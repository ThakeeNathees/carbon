//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

#include "codegen.h"

namespace carbon {

Address CodeGen::_generate_expression(const Parser::Node* p_expr) {

	switch (p_expr->type) {
		case Parser::Node::Type::IMPORT:
		case Parser::Node::Type::FILE:
		case Parser::Node::Type::CLASS:
		case Parser::Node::Type::ENUM:
		case Parser::Node::Type::FUNCTION:
		case Parser::Node::Type::BLOCK:
			THROW_BUG("invalid expression type found in codegen");

		case Parser::Node::Type::IDENTIFIER: {
			const Parser::IdentifierNode* id = static_cast<const Parser::IdentifierNode*>(p_expr);
			switch (id->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN:
					THROW_BUG("unknown identifier should be analyzer error (can't reach to codegen)");

				case Parser::IdentifierNode::REF_PARAMETER: 
				case Parser::IdentifierNode::REF_LOCAL_VAR: {
					return _context.get_stack_local(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_CONST:
					THROW_BUG("identifier to local const should be reduced at analyzing phace");

				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					Address get_dst = _context.add_stack_temp();
					Address member_addr = Address(Address::MEMBER_VAR, _context.bytecode->get_member_index(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_CONST:
				case Parser::IdentifierNode::REF_ENUM_NAME:
				case Parser::IdentifierNode::REF_ENUM_VALUE:
				case Parser::IdentifierNode::REF_FUNCTION:
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					Address get_dst = _context.add_stack_temp();
					Address member_addr = Address(Address::STATIC, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					Address get_dst = _context.add_stack_temp();
					Address member_addr = Address(Address::NATIVE_CLASS, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					Address get_dst = _context.add_stack_temp();
					Address member_addr = Address(Address::EXTERN, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;
			}
		} break;

		case Parser::Node::Type::VAR:
		case Parser::Node::Type::CONST: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;

		case Parser::Node::Type::CONST_VALUE: {
			return add_global_const_value(static_cast<const Parser::ConstValueNode*>(p_expr)->value);
		} break;

		case Parser::Node::Type::ARRAY: {
			const Parser::ArrayNode* arr = static_cast<const Parser::ArrayNode*>(p_expr);

			Address arr_dst = _context.add_stack_temp();

			stdvec<Address> values;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				Address val = _generate_expression(arr->elements[i].get());
				values.push_back(val);
			}

			_context.opcodes->write_array_literal(arr_dst, values);

			for (Address& addr : values) {
				if (addr.is_temp()) _context.pop_stack_temp();
			}

			return arr_dst;
		} break;

		case Parser::Node::Type::MAP: {
			const Parser::MapNode* map = static_cast<const Parser::MapNode*>(p_expr);

			Address map_dst = _context.add_stack_temp();

			stdvec<Address> keys, values;
			for (auto& pair : map->elements) {
				Address key = _generate_expression(pair.key.get());
				Address value = _generate_expression(pair.value.get());

				keys.push_back(key);
				values.push_back(value);
			}

			_context.opcodes->write_map_literal(map_dst, keys, values);

			for (Address& addr : keys) if (addr.is_temp()) _context.pop_stack_temp();
			for (Address& addr : values) if (addr.is_temp()) _context.pop_stack_temp();

			return map_dst;
		} break;

		case Parser::Node::Type::THIS: {
			return Address(Address::THIS, 0);
		} break;
		case Parser::Node::Type::SUPER: {
			// TODO: super cannot be a stand alone expression (throw error)
		} break;

		case Parser::Node::Type::BUILTIN_FUNCTION: {
			Address get_dst = _context.add_stack_temp();
			Address func_addr = Address(Address::BUILTIN_FUNC, (uint32_t)static_cast<const Parser::BuiltinFunctionNode*>(p_expr)->func);
			_context.opcodes->write_get_member(func_addr, get_dst);
			return get_dst;
		} break;

		case Parser::Node::Type::BUILTIN_TYPE: {
			Address get_dst = _context.add_stack_temp();
			Address type_addr = Address(Address::BUILTIN_TYPE, (uint32_t)static_cast<const Parser::BuiltinTypeNode*>(p_expr)->builtin_type);
			_context.opcodes->write_get_member(type_addr, get_dst);
			return get_dst;
		} break;

		case Parser::Node::Type::CALL: {
			const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_expr);
			Address ret = _context.add_stack_temp();

			stdvec<Address> args;
			for (int i = 0; i < (int)call->args.size(); i++) {
				Address arg = _generate_expression(call->args[i].get());
				args.push_back(arg);
			}

			switch (call->base->type) {

				// print(); builtin func call
				case Parser::Node::Type::BUILTIN_FUNCTION: {
					if (call->method == nullptr) { // print(...);
						_context.opcodes->write_call_builtin_func(ret, static_cast<const Parser::BuiltinFunctionNode*>(call->base.get())->func, args);
					} else { // print.member(...);
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.opcodes->write_call_method(ret, base, name,  args);
						if (base.is_temp()) _context.pop_stack_temp();
					}
				} break;

				// Array(); builtin type constructor
				case Parser::Node::Type::BUILTIN_TYPE: {
					if (call->method == nullptr) { // Array(); constructor
						_context.opcodes->write_construct_builtin_type(ret, static_cast<const Parser::BuiltinTypeNode*>(call->base.get())->builtin_type, args);
					} else { // String.format(); // static method call on builtin type
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.opcodes->write_call_method(ret, base, name, args);
						if (base.is_temp()) _context.pop_stack_temp();
					}
				} break;

				case Parser::Node::Type::SUPER: {

					if (call->method == nullptr) { // super(...); if used in constructor -> super constructor else call same func on super.
						// TODO:
					} else { // super.f();

					}

				} break;

				default: {
					Address base = _generate_expression(call->base.get());
					if (call->method != nullptr) {
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.opcodes->write_call_method(ret, base, name, args);
					} else {
						_context.opcodes->write_call_func(ret, base, args);
					}
					if (base.is_temp()) _context.pop_stack_temp();
				} break;
			}
			for (Address& addr : args) if (addr.is_temp()) _context.pop_stack_temp();
			return ret;
		} break;

		case Parser::Node::Type::INDEX: {
			Address dst = _context.add_stack_temp();
			const Parser::IndexNode* index_node = static_cast<const Parser::IndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			uint32_t name = add_global_name(index_node->member->name);
			_context.opcodes->write_get_index(on, name, dst);
			if (on.is_temp()) _context.pop_stack_temp();
			return dst;
		} break;

		case Parser::Node::Type::MAPPED_INDEX: {
			Address dst = _context.add_stack_temp();
			const Parser::MappedIndexNode* index_node = static_cast<const Parser::MappedIndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			Address key = _generate_expression(index_node->key.get());

			_context.opcodes->write_get_mapped(on, key, dst);

			if (on.is_temp()) _context.pop_stack_temp();
			if (key.is_temp()) _context.pop_stack_temp();
			return dst;
		} break;
		case Parser::Node::Type::OPERATOR: {
			// TODO:
		} break;

		case Parser::Node::Type::CONTROL_FLOW: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;
	}


	return Address();
}

}
