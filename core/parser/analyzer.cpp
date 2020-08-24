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
	

void Parser::analyze() {
	for (size_t i = 0; i < file_node->vars.size(); i++) {
		if (file_node->vars[i]->assignment != nullptr) {
			_reduce_expression(file_node->vars[i]->assignment);
		}
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		for (size_t j = 0; j < file_node->classes[i]->vars.size(); j++) {
			if (file_node->classes[i]->vars[j]->assignment != nullptr) {
				_reduce_expression(file_node->classes[i]->vars[j]->assignment);
			}
		}
	}
}

void Parser::_reduce_expression(ptr<Node>& p_expr) {
	switch (p_expr->type) {
		case Node::Type::BUILTIN_FUNCTION: {
		} break;
		case Node::Type::ARRAY: {
			ptr<ArrayNode> arr = ptrcast<ArrayNode>(p_expr);
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
			}
		} break;
		case Node::Type::MAP: { // TODO: no literal for map.
			ptr<MapNode> map = ptrcast<MapNode>(p_expr);
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: key should be hashable.
				_reduce_expression(map->elements[i].value);
			}
		} break;
		case Node::Type::OPERATOR: {
			ptr<OperatorNode> op = ptrcast<OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Node::Type::CONST_VALUE) {
					if (i == 0 && (op->args[0]->type == Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Node::Type::BUILTIN_TYPE)) {
						// Could be all const.
					} else {
						all_const = false;
					}
				}
			}

			stdvec<var> args;
			int initial_argument = (op->args[0]->type == Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Node::Type::BUILTIN_TYPE) ? 1 : 0;
			for (int i = initial_argument; i < (int)op->args.size(); i++) {
				args.push_back(ptrcast<ConstValueNode>(op->args[i])->value);
			}
#define SET_EXPR_CONST_NODE(m_var)  /* Can't use as a single statement and without a scope in switch case */ \
	ptr<ConstValueNode> cv = new_node<ConstValueNode>(m_var);                                                \
	cv->pos = op->pos;										                                                 \
	p_expr = cv
			switch (op->op_type) {
				case OperatorNode::OpType::OP_CALL: {
					// reduce builtin function
					if ((op->args[0]->type == Node::Type::BUILTIN_FUNCTION) && all_const) {
						ptr<BuiltinFunctionNode> bf = ptrcast<BuiltinFunctionNode>(op->args[0]);
						if (BuiltinFunctions::can_const_fold(bf->func)) {
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
							SET_EXPR_CONST_NODE(ret);
						}
					}
					// reduce builtin type construction
					if ((op->args[0]->type == Node::Type::BUILTIN_TYPE) && all_const) {
						ptr<BuiltinTypeNode> bt = ptrcast<BuiltinTypeNode>(op->args[0]);
						var ret;
						try {
							ret = BuiltinTypes::construct(bt->builtin_type, args);
						} catch (Error& err) {
							throw err
								.set_file(file_node->path)
								.set_line(file_node->source.get_line(op->pos.x))
								.set_pos(op->pos)
								.set_err_len((uint32_t)String(BuiltinTypes::get_type_name(bt->builtin_type)).size())
								;
						}
						SET_EXPR_CONST_NODE(ret);
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

			/***************** BINARY AND UNARY OPERATORS  *****************/

				case OperatorNode::OpType::OP_EQ:
				case OperatorNode::OpType::OP_PLUSEQ:
				case OperatorNode::OpType::OP_MINUSEQ:
				case OperatorNode::OpType::OP_MULEQ:
				case OperatorNode::OpType::OP_DIVEQ:
				case OperatorNode::OpType::OP_MOD_EQ:
				case OperatorNode::OpType::OP_LTEQ:
				case OperatorNode::OpType::OP_GTEQ:
				case OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
				case OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
				case OperatorNode::OpType::OP_BIT_OR_EQ:
				case OperatorNode::OpType::OP_BIT_AND_EQ:
				case OperatorNode::OpType::OP_BIT_XOR_EQ: {
					// TODO:
					// Can't assign to self, constant, and if LSH is operator only index (named, mapped) are
					// supported for assignment.
				} break;

				case OperatorNode::OpType::OP_EQEQ: {
					SET_EXPR_CONST_NODE(args[0] == args[1]);
				}
				case OperatorNode::OpType::OP_PLUS: {
					SET_EXPR_CONST_NODE(args[0] + args[1]);
				} break;
				case OperatorNode::OpType::OP_MINUS: {
					SET_EXPR_CONST_NODE(args[0] - args[1]);
				} break;
				case OperatorNode::OpType::OP_MUL: {
					SET_EXPR_CONST_NODE(args[0] * args[1]);
				} break;
				case OperatorNode::OpType::OP_DIV: {
					SET_EXPR_CONST_NODE(args[0] / args[1]);
				} break;
				case OperatorNode::OpType::OP_MOD: {
					SET_EXPR_CONST_NODE(args[0] % args[1]);
				} break;
				case OperatorNode::OpType::OP_LT: {
					SET_EXPR_CONST_NODE(args[0] < args[1]);
				} break;
				case OperatorNode::OpType::OP_GT: {
					SET_EXPR_CONST_NODE(args[0] > args[1]);
				} break;
				case OperatorNode::OpType::OP_AND: {
					SET_EXPR_CONST_NODE(args[0].operator bool() && args[1].operator bool());
				} break;
				case OperatorNode::OpType::OP_OR: {
					SET_EXPR_CONST_NODE(args[0].operator bool() || args[1].operator bool());
				} break;
				case OperatorNode::OpType::OP_NOTEQ: {
					SET_EXPR_CONST_NODE(args[0] != args[1]);
				} break;
				case OperatorNode::OpType::OP_BIT_LSHIFT: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() << args[1].operator int64_t());
				} break;
				case OperatorNode::OpType::OP_BIT_RSHIFT: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() >> args[1].operator int64_t());
				} break;
				case OperatorNode::OpType::OP_BIT_OR: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() | args[1].operator int64_t());
				} break;
				case OperatorNode::OpType::OP_BIT_AND: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() & args[1].operator int64_t());
				} break;
				case OperatorNode::OpType::OP_BIT_XOR: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() ^ args[1].operator int64_t());
				} break;


				case OperatorNode::OpType::OP_NOT: {
				} break;
				case OperatorNode::OpType::OP_BIT_NOT: {
				} break;
				case OperatorNode::OpType::OP_POSITIVE: {
				} break;
				case OperatorNode::OpType::OP_NEGATIVE: {
				} break;

				default: {
					ASSERT(false); // TODO: throw internal bug.
				}
				MISSED_ENUM_CHECK(OperatorNode::OpType::_OP_MAX_, 36);
			}
#undef SET_EXPR_CONST_NODE
		} break;
		case Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			ASSERT(false); // ???
		}

	}
}

void Parser::_reduce_block(ptr<BlockNode>& p_block) {
	// TODO:
}

}