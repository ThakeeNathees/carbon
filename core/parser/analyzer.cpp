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

#include "parser.h"

namespace carbon {
	
void Parser::_reduce_expression(ptr<Node>& p_expr) {
	switch (p_expr->type) {
		case Node::Type::BUILTIN_FUNCTION: {
		} break;
		case Node::Type::ARRAY: {
			ptr<ArrayNode> arr = ptrcast<ArrayNode>(p_expr);
			for (int i = 0; i < arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
			}
		} break;
		case Node::Type::MAP: { // TODO: no literal for map.
			ptr<MapNode> map = ptrcast<MapNode>(p_expr);
			for (int i = 0; i < map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: key should be hashable.
				_reduce_expression(map->elements[i].value);
			}
		} break;
		case Node::Type::OPERATOR: {
			ptr<OperatorNode> op = ptrcast<OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Node::Type::CONST_VALUE) {
					if (i == 0 && (op->args[0]->type == Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Node::Type::BUILTIN_TYPE)) {
						// Could be all const.
					} else {
						all_const = false;
					}
				}
			}

			switch (op->op_type) {
				case OperatorNode::OpType::OP_CALL: {
					// reduce builtin function
					if ((op->args[0]->type == Node::Type::BUILTIN_FUNCTION) && all_const) {
						ptr<BuiltinFunctionNode> bf = ptrcast<BuiltinFunctionNode>(op->args[0]);
						if (bf->func != BuiltinFunctions::Type::PRINT) { // can't call print, TODO input at compile time
							stdvec<var> args;
							for (int i = 1; i < op->args.size(); i++) {
								args.push_back(ptrcast<ConstValueNode>(op->args[i])->value);
							}
							var ret;
							try {
								BuiltinFunctions::call(bf->func, args, ret);
							} catch (Error& err) {
								throw err
									.set_file(file_node->path)
									.set_line(file_node->source.get_line(op->pos.x))
									.set_pos(op->pos)
									.set_err_len((uint32_t)String(BuiltinFunctions::get_func_name(bf->func)).size())
								;
							}
							ptr<ConstValueNode> cv = new_node<ConstValueNode>(ret);
							cv->pos = op->pos;
							p_expr = cv;
						}
					}
					// reduce builtin type construction
					if ((op->args[0]->type == Node::Type::BUILTIN_TYPE) && all_const) {
						ptr<BuiltinTypeNode> bt = ptrcast<BuiltinTypeNode>(op->args[0]);
						stdvec<var> args;
						for (int i = 1; i < op->args.size(); i++) {
							args.push_back(ptrcast<ConstValueNode>(op->args[i])->value);
						}
						var ret;
						try {
							//BuiltinTypes::call(bf->func, args, ret); // TODO:
						} catch (Error& err) {
							throw err
								.set_file(file_node->path)
								.set_line(file_node->source.get_line(op->pos.x))
								.set_pos(op->pos)
								.set_err_len((uint32_t)String(BuiltinTypes::get_type_name(bt->builtin_type)).size())
								;
						}
						ptr<ConstValueNode> cv = new_node<ConstValueNode>(ret);
						cv->pos = op->pos;
						p_expr = cv;
					}

				} break;
				case OperatorNode::OpType::OP_INDEX: {
					ASSERT(op->args.size() == 2); // TODO: throw internal bug
					// TODO:
					switch (op->args[0]->type) {
						case Node::Type::THIS:
						case Node::Type::SUPER:
						case Node::Type::BUILTIN_TYPE:
						case Node::Type::CONST_VALUE:
						case Node::Type::IDENTIFIER: {

						}
						default:
							break;
							// RUNTIME.
					}
				} break;
				case OperatorNode::OpType::OP_INDEX_MAPPED: {
					// Can't reduce at compile time.
				} break;
				default: {
					// TODO: binary, unary operators, and others
				}
			}

		} break;
		case Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			// ASSERT ?
		}

	}
}

}