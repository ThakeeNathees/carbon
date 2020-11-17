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
					Address member_addr = Address(Address::STACK, _bytecode->_global_name_get(id->name));
					_context.opcodes->write_get_member(member_addr, get_dst);
					return get_dst;
				} break;

				case Parser::IdentifierNode::REF_NATIVE_CLASS:
				case Parser::IdentifierNode::REF_EXTERN:
					break;
			}
		} break;

		case Parser::Node::Type::VAR:
		case Parser::Node::Type::CONST: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;

		case Parser::Node::Type::CONST_VALUE: {
			return _context.add_global_const_value(static_cast<const Parser::ConstValueNode*>(p_expr)->value);
		} break;

		case Parser::Node::Type::ARRAY: {

		} break;
		case Parser::Node::Type::MAP: {

		} break;

		case Parser::Node::Type::THIS: {
		} break;
		case Parser::Node::Type::SUPER: {
		} break;
		case Parser::Node::Type::BUILTIN_FUNCTION: {
		} break;
		case Parser::Node::Type::BUILTIN_TYPE: {
		} break;

		case Parser::Node::Type::CALL: {

		} break;
		case Parser::Node::Type::INDEX: {

		} break;
		case Parser::Node::Type::MAPPED_INDEX: {

		} break;
		case Parser::Node::Type::OPERATOR: {

		} break;

		case Parser::Node::Type::CONTROL_FLOW: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;
	}


	return Address();
}

}
