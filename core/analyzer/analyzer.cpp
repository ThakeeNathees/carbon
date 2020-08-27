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

#include "analyzer.h"

#define THROW_ANALYZER_ERROR(m_err_type, m_msg, m_pos)                                                                      \
	do {																													\
		uint32_t err_len = 1;																								\
		String token_str = "";																								\
		if (m_pos.x > 0 && m_pos.y > 0) token_str = parser->tokenizer->get_token_at(m_pos).to_string();						\
		else token_str = parser->tokenizer->peek(-1, true).to_string();														\
		if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;				\
		else err_len = (uint32_t)token_str.size();																			\
																															\
		if (m_pos.x > 0 && m_pos.y > 0) {																					\
			String line = file_node->source.get_line(m_pos.x);																\
			throw Error(m_err_type, m_msg, file_node->path, line, m_pos, err_len)_ERR_ADD_DBG_VARS;							\
		} else {																											\
			String line = file_node->source.get_line(parser->tokenizer->get_pos().x);										\
			throw Error(m_err_type, m_msg, file_node->path, line, parser->tokenizer->peek(-1, true).get_pos(), err_len)		\
				_ERR_ADD_DBG_VARS;																							\
		}																													\
	} while (false)
	

namespace carbon {

void Analyzer::analyze(ptr<Parser> p_parser) {

	parser = p_parser;
	file_node = parser->file_node;

	// File/class level constants.
	for (size_t i = 0; i < file_node->constants.size(); i++) {
		if (file_node->constants[i]->assignment != nullptr) {
			_reduce_expression(file_node->constants[i]->assignment);
			if (file_node->constants[i]->assignment->type != Parser::Node::Type::CONST_VALUE) {
				THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "Expected a contant expression.", file_node->constants[i]->assignment->pos);
			}
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(file_node->constants[i]->assignment);
			if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
				cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING) {
				THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "Expected a constant expression.", file_node->constants[i]->assignment->pos);

			}
			file_node->constants[i]->value = cv->value;
		}
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		for (size_t j = 0; j < file_node->classes[i]->constants.size(); j++) {
			if (file_node->classes[i]->constants[j]->assignment != nullptr) {
				_reduce_expression(file_node->classes[i]->constants[j]->assignment);
				if (file_node->classes[i]->constants[j]->assignment->type != Parser::Node::Type::CONST_VALUE) {
					THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "Expected a contant expression.", file_node->classes[i]->constants[j]->assignment->pos);
				}
				ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(file_node->classes[i]->constants[j]->assignment);
				if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
					cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING) {
					THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "Expected a constant expression.", file_node->classes[i]->constants[j]->assignment->pos);
				}
				file_node->classes[i]->constants[j]->value = cv->value;
			}
		}
	}

	// File/class enums/unnamed_enums.
	for (size_t i = 0; i < file_node->enums.size(); i++) {
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->enums[i]->values) {
			_reduce_expression(pair.second.expr);
			if (pair.second.expr->type != Parser::Node::Type::CONST_VALUE)
				THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(pair.second.expr);
			if (cv->value.get_type() != var::INT) THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
			pair.second.value = cv->value;
		}
	}
	if (file_node->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->unnamed_enum->values) {
			_reduce_expression(pair.second.expr);
			if (pair.second.expr->type != Parser::Node::Type::CONST_VALUE)
				THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(pair.second.expr);
			if (cv->value.get_type() != var::INT) THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
			pair.second.value = cv->value;
		}
	}

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		for (size_t j = 0; j < file_node->classes[i]->enums.size(); j++) {
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->enums[j]->values) {
				_reduce_expression(pair.second.expr);
				if (pair.second.expr->type != Parser::Node::Type::CONST_VALUE, pair.second.expr->pos)
					THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
				ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(pair.second.expr);
				if (cv->value.get_type() != var::INT) THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
				pair.second.value = cv->value;
			}
		}
		if (file_node->classes[i]->unnamed_enum != nullptr) {
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->unnamed_enum->values) {
				_reduce_expression(pair.second.expr);
				if (pair.second.expr->type != Parser::Node::Type::CONST_VALUE)
					THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
				ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(pair.second.expr);
				if (cv->value.get_type() != var::INT)
					THROW_ANALYZER_ERROR(Error::INVALID_TYPE, "enum value must be a constant integer", pair.second.expr->pos);
				pair.second.value = cv->value;
			}
		}
	}

	// File/class level variables.
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

	// File level function body.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		_reduce_block(file_node->functions[i]->body);
	}

	// Inner class function body.
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			_reduce_block(file_node->classes[i]->functions[j]->body);
		}
	}
}

void Analyzer::_reduce_expression(ptr<Parser::Node>& p_expr) {

	if (p_expr == nullptr) return;

	// Prevent stack overflow.
	if (p_expr->is_reduced) return;
	p_expr->is_reduced = true;

	switch (p_expr->type) {

		// Assigning function to a variable ?
		case Parser::Node::Type::BUILTIN_TYPE:
		case Parser::Node::Type::BUILTIN_FUNCTION:
			break;

		case Parser::Node::Type::IDENTIFIER: {
			// TODO:
			// could be parameter, local constant, local var, constant, member var,
			// class name, native class name, (not builtin class/builtin function name)
			// function name
		} break;

		case Parser::Node::Type::ARRAY: {
			ptr<Parser::ArrayNode> arr = ptrcast<Parser::ArrayNode>(p_expr);
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
			}
		} break;

		case Parser::Node::Type::MAP: { // TODO: no literal for map.
			ptr<Parser::MapNode> map = ptrcast<Parser::MapNode>(p_expr);
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: key should be hashable.
				_reduce_expression(map->elements[i].value);
			}
		} break;

		case Parser::Node::Type::OPERATOR: {
			ptr<Parser::OperatorNode> op = ptrcast<Parser::OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				if (i == 0 && (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE)) {
					// _don't_reduce_expression();
					continue;
				} else {
					_reduce_expression(op->args[i]);
				}
				if (op->args[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}

			stdvec<var> args;
			int initial_argument = (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE) ? 1 : 0;
			for (int i = initial_argument; i < (int)op->args.size(); i++) {
				args.push_back(ptrcast<Parser::ConstValueNode>(op->args[i])->value);
			}
#define SET_EXPR_CONST_NODE(m_var)  /* Can't use as a single statement and without a scope in switch case */ \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);                                \
	cv->pos = op->pos;										                                                 \
	p_expr = cv
			switch (op->op_type) {
				case Parser::OperatorNode::OpType::OP_CALL: {
					// reduce builtin function
					if ((op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION) && all_const) {
						ptr<Parser::BuiltinFunctionNode> bf = ptrcast<Parser::BuiltinFunctionNode>(op->args[0]);
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
					if ((op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE) && all_const) {
						ptr<Parser::BuiltinTypeNode> bt = ptrcast<Parser::BuiltinTypeNode>(op->args[0]);
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
				case Parser::OperatorNode::OpType::OP_INDEX: {
					ASSERT(op->args.size() == 2); // TODO: throw internal bug
					// TODO:
					switch (op->args[0]->type) {
						case Parser::Node::Type::THIS:
						case Parser::Node::Type::SUPER:
						case Parser::Node::Type::BUILTIN_TYPE:
						case Parser::Node::Type::CONST_VALUE:
						case Parser::Node::Type::IDENTIFIER: {

						}
						default:
							break;
							// RUNTIME.
					}
				} break;
				case Parser::OperatorNode::OpType::OP_INDEX_MAPPED: {
					// Can't reduce at compile time.
				} break;

			/***************** BINARY AND UNARY OPERATORS  *****************/

				case Parser::OperatorNode::OpType::OP_EQ:
				case Parser::OperatorNode::OpType::OP_PLUSEQ:
				case Parser::OperatorNode::OpType::OP_MINUSEQ:
				case Parser::OperatorNode::OpType::OP_MULEQ:
				case Parser::OperatorNode::OpType::OP_DIVEQ:
				case Parser::OperatorNode::OpType::OP_MOD_EQ:
				case Parser::OperatorNode::OpType::OP_LTEQ:
				case Parser::OperatorNode::OpType::OP_GTEQ:
				case Parser::OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_OR_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_AND_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_XOR_EQ: {
					// TODO:
					// Can't assign to self, constant, and if LSH is operator only index (named, mapped) are
					// supported for assignment.
				} break;

				case Parser::OperatorNode::OpType::OP_EQEQ: {
					SET_EXPR_CONST_NODE(args[0] == args[1]);
				}
				case Parser::OperatorNode::OpType::OP_PLUS: {
					SET_EXPR_CONST_NODE(args[0] + args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_MINUS: {
					SET_EXPR_CONST_NODE(args[0] - args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_MUL: {
					SET_EXPR_CONST_NODE(args[0] * args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_DIV: {
					SET_EXPR_CONST_NODE(args[0] / args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_MOD: {
					SET_EXPR_CONST_NODE(args[0] % args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_LT: {
					SET_EXPR_CONST_NODE(args[0] < args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_GT: {
					SET_EXPR_CONST_NODE(args[0] > args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_AND: {
					SET_EXPR_CONST_NODE(args[0].operator bool() && args[1].operator bool());
				} break;
				case Parser::OperatorNode::OpType::OP_OR: {
					SET_EXPR_CONST_NODE(args[0].operator bool() || args[1].operator bool());
				} break;
				case Parser::OperatorNode::OpType::OP_NOTEQ: {
					SET_EXPR_CONST_NODE(args[0] != args[1]);
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_LSHIFT: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() << args[1].operator int64_t());
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_RSHIFT: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() >> args[1].operator int64_t());
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_OR: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() | args[1].operator int64_t());
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_AND: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() & args[1].operator int64_t());
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_XOR: {
					SET_EXPR_CONST_NODE(args[0].operator int64_t() ^ args[1].operator int64_t());
				} break;


				case Parser::OperatorNode::OpType::OP_NOT: {
				} break;
				case Parser::OperatorNode::OpType::OP_BIT_NOT: {
				} break;
				case Parser::OperatorNode::OpType::OP_POSITIVE: {
				} break;
				case Parser::OperatorNode::OpType::OP_NEGATIVE: {
				} break;

				default: {
					ASSERT(false); // TODO: throw internal bug.
				}
				MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 36);
			}
#undef SET_EXPR_CONST_NODE
		} break;
		case Parser::Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			ASSERT(false); // ???
		}

	}
}

void Analyzer::_reduce_block(ptr<Parser::BlockNode>& p_block) {
	// TODO:
}

}