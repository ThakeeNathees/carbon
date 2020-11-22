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

Address CodeGen::_generate_expression(const Parser::Node* p_expr, Address* p_dst) {

#define ADDR_DST() (p_dst) ? * p_dst : _context.add_stack_temp()

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

				case Parser::IdentifierNode::REF_PARAMETER: {
					return _context.get_parameter(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_VAR: {
					return _context.get_stack_local(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_CONST:
					THROW_BUG("identifier to local const should be reduced at analyzing phace");

				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					Address get_dst = ADDR_DST();
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
					Address get_dst = ADDR_DST();
					Address member_addr = Address(Address::STATIC_MEMBER, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					Address get_dst = ADDR_DST();
					Address member_addr = Address(Address::NATIVE_CLASS, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					Address get_dst = ADDR_DST();
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

			Address arr_dst = ADDR_DST();

			stdvec<Address> values;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				Address val = _generate_expression(arr->elements[i].get());
				values.push_back(val);
			}

			_context.opcodes->write_array_literal(arr_dst, values);

			for (Address& addr : values) {
				_POP_ADDR_IF_TEMP(addr);
			}

			return arr_dst;
		} break;

		case Parser::Node::Type::MAP: {
			const Parser::MapNode* map = static_cast<const Parser::MapNode*>(p_expr);

			Address map_dst = ADDR_DST();

			stdvec<Address> keys, values;
			for (auto& pair : map->elements) {
				Address key = _generate_expression(pair.key.get());
				Address value = _generate_expression(pair.value.get());

				keys.push_back(key);
				values.push_back(value);
			}

			_context.opcodes->write_map_literal(map_dst, keys, values);

			for (Address& addr : keys) _POP_ADDR_IF_TEMP(addr);
			for (Address& addr : values) _POP_ADDR_IF_TEMP(addr);

			return map_dst;
		} break;

		case Parser::Node::Type::THIS: {
			return Address(Address::THIS, 0);
		} break;
		case Parser::Node::Type::SUPER: {
			THROW_BUG("TODO:");
			// TODO: super cannot be a stand alone expression (throw error)
		} break;

		case Parser::Node::Type::BUILTIN_FUNCTION: {
			Address get_dst = ADDR_DST();
			Address func_addr = Address(Address::BUILTIN_FUNC, (uint32_t)static_cast<const Parser::BuiltinFunctionNode*>(p_expr)->func);
			_context.opcodes->write_get_member(func_addr, get_dst);
			return get_dst;
		} break;

		case Parser::Node::Type::BUILTIN_TYPE: {
			Address get_dst = ADDR_DST();
			Address type_addr = Address(Address::BUILTIN_TYPE, (uint32_t)static_cast<const Parser::BuiltinTypeNode*>(p_expr)->builtin_type);
			_context.opcodes->write_get_member(type_addr, get_dst);
			return get_dst;
		} break;

		case Parser::Node::Type::CALL: {
			const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_expr);
			Address ret = ADDR_DST();

			stdvec<Address> args;
			for (int i = 0; i < (int)call->args.size(); i++) {
				Address arg = _generate_expression(call->args[i].get());
				args.push_back(arg);
			}

			switch (call->base->type) {

				// print(); builtin func call
				case Parser::Node::Type::BUILTIN_FUNCTION: {
					if (call->method == nullptr) { // print(...);
						_context.opcodes->write_call_builtin(ret, static_cast<const Parser::BuiltinFunctionNode*>(call->base.get())->func, args);
					} else { // print.member(...);
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.opcodes->write_call_method(ret, base, name,  args);
						_POP_ADDR_IF_TEMP(base);
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
						_POP_ADDR_IF_TEMP(base);
					}
				} break;

				case Parser::Node::Type::SUPER: {

					if (call->method == nullptr) { // super(...); if used in constructor -> super constructor else call same func on super.
						// TODO:
					} else { // super.f();

					}

				} break;

				case Parser::Node::Type::UNKNOWN: {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					const Parser::IdentifierNode* func = ptrcast<Parser::IdentifierNode>(call->method).get();
					uint32_t name = add_global_name(func->name);
					switch (func->ref) {
						case  Parser::IdentifierNode::REF_FUNCTION: {
							_context.opcodes->write_call_func(ret, name, args);
						} break;
						case  Parser::IdentifierNode::REF_CARBON_CLASS: {
							_context.opcodes->write_construct_carbon(ret, name, args);
						} break;
						case Parser::IdentifierNode::REF_NATIVE_CLASS: {
							_context.opcodes->write_construct_native(ret, name, args);
						} break;
						default: {
							THROW_BUG("can't reach here"); // TODO: refactor
						}
					}
				} break;

				default: {
					Address base = _generate_expression(call->base.get());
					if (call->method != nullptr) {
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.opcodes->write_call_method(ret, base, name, args);
					} else {
						_context.opcodes->write_call(ret, base, args);
					}
					_POP_ADDR_IF_TEMP(base);
				} break;
			}
			for (Address& addr : args) _POP_ADDR_IF_TEMP(addr);
			return ret;
		} break;

		case Parser::Node::Type::INDEX: {
			Address dst = ADDR_DST();
			const Parser::IndexNode* index_node = static_cast<const Parser::IndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			uint32_t name = add_global_name(index_node->member->name);
			_context.opcodes->write_get_index(on, name, dst);
			_POP_ADDR_IF_TEMP(on);
			return dst;
		} break;

		case Parser::Node::Type::MAPPED_INDEX: {
			Address dst = ADDR_DST();
			const Parser::MappedIndexNode* index_node = static_cast<const Parser::MappedIndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			Address key = _generate_expression(index_node->key.get());

			_context.opcodes->write_get_mapped(on, key, dst);

			_POP_ADDR_IF_TEMP(on);
			_POP_ADDR_IF_TEMP(key);
			return dst;
		} break;
		case Parser::Node::Type::OPERATOR: {
			const Parser::OperatorNode* op = static_cast<const Parser::OperatorNode*>(p_expr);

			var::Operator var_op = var::_OP_MAX_;
			switch (op->op_type) {
				case Parser::OperatorNode::OP_EQ:
				case Parser::OperatorNode::OP_PLUSEQ:        var_op = var::OP_ADDITION;       goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MINUSEQ:       var_op = var::OP_SUBTRACTION;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MULEQ:         var_op = var::OP_MULTIPLICATION; goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_DIVEQ:         var_op = var::OP_DIVISION;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MOD_EQ:        var_op = var::OP_MODULO;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_LSHIFT_EQ: var_op = var::OP_BIT_LSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_RSHIFT_EQ: var_op = var::OP_BIT_RSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_OR_EQ:     var_op = var::OP_BIT_OR;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_AND_EQ:    var_op = var::OP_BIT_AND;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_XOR_EQ: {  var_op = var::OP_BIT_XOR;
				_addr_operator_assign_:
					// indexing, mapped indexing is special case.
					if (op->args[0]->type == Parser::Node::Type::INDEX) {
						//	TODO:
					} else if (op->args[0]->type == Parser::Node::Type::MAPPED_INDEX) {
						const Parser::MappedIndexNode* mapped = static_cast<const Parser::MappedIndexNode*>(op->args[0].get());
						Address value = _generate_expression(op->args[1].get());
						Address on = _generate_expression(mapped->base.get());
						Address key = _generate_expression(mapped->key.get());

						_context.opcodes->write_set_mapped(on, key, value);

						_POP_ADDR_IF_TEMP(on);
						_POP_ADDR_IF_TEMP(key);
						return value;
					} else {

						Address left = _generate_expression(op->args[0].get());
						if (var_op != var::_OP_MAX_) {
							Address right = _generate_expression(op->args[1].get());
							_context.opcodes->write_operator(left, var_op, left, right);
							_POP_ADDR_IF_TEMP(right);
						} else {
							Address right = _generate_expression(op->args[1].get(), &left);
							if (left != right) _context.opcodes->write_assign(left, right);
						}
						return left;
					}
				} break;

				case Parser::OperatorNode::OP_AND: {
					Address dst = ADDR_DST();
					_context.opcodes->write_assign_bool(dst, false);
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_and_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.opcodes->write_and_right(right, dst);
					_POP_ADDR_IF_TEMP(left);
					_POP_ADDR_IF_TEMP(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_OR: {
					Address dst = ADDR_DST();
					_context.opcodes->write_assign_bool(dst, true);
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_or_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.opcodes->write_or_right(right, dst);
					_POP_ADDR_IF_TEMP(left);
					_POP_ADDR_IF_TEMP(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_EQEQ:       var_op = var::OP_EQ_CHECK;       goto _addr_operator_;
				case Parser::OperatorNode::OP_NOTEQ:      var_op = var::OP_NOT_EQ_CHECK;   goto _addr_operator_;
				case Parser::OperatorNode::OP_LTEQ:       var_op = var::OP_LTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_GTEQ:       var_op = var::OP_GTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_PLUS:       var_op = var::OP_ADDITION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MINUS:      var_op = var::OP_SUBTRACTION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MUL:        var_op = var::OP_MULTIPLICATION; goto _addr_operator_;
				case Parser::OperatorNode::OP_DIV:        var_op = var::OP_DIVISION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MOD:        var_op = var::OP_MODULO;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_LT:         var_op = var::OP_LT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_GT:         var_op = var::OP_GT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_LSHIFT: var_op = var::OP_BIT_LSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_RSHIFT: var_op = var::OP_BIT_RSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_OR:     var_op = var::OP_BIT_OR;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_AND:    var_op = var::OP_BIT_AND;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_XOR: {  var_op = var::OP_BIT_XOR;
					_addr_operator_:
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					Address right = _generate_expression(op->args[1].get());
					_context.opcodes->write_operator(dst, var_op, left, right);
					_POP_ADDR_IF_TEMP(left);
					_POP_ADDR_IF_TEMP(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_operator(dst, var::OP_NOT, left, Address());
					_POP_ADDR_IF_TEMP(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_BIT_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_operator(dst, var::OP_BIT_NOT, left, Address());
					_POP_ADDR_IF_TEMP(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_POSITIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_operator(dst, var::OP_POSITIVE, left, Address());
					_POP_ADDR_IF_TEMP(left);
					return dst;
				} break;
				case Parser::OperatorNode::OP_NEGATIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.opcodes->write_operator(dst, var::OP_NEGATIVE, left, Address());
					_POP_ADDR_IF_TEMP(left);
					return dst;
				} break;
			}
		} break;

		case Parser::Node::Type::CONTROL_FLOW: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;
	}


	return Address();
}

}
