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

ptr<Parser::Node> Parser::_parse_expression(const ptr<Node>& p_parent) {
	ASSERT(p_parent != nullptr);

	stdvec<Expr> expressions;

	while (true) {

		const TokenData* tk = &tokenizer->next();
		ptr<Node> expr = nullptr;
		
		if (tk->type == Token::BRACKET_LPARAN) {
			expr = _parse_expression(p_parent);

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_RPARAN) {
				THROW_UNEXP_TOKEN("symbol \")\"");
			}

		} else if (tk->type == Token::KWORD_THIS) {
			if (parser_context.current_class == nullptr || (parser_context.current_func && parser_context.current_func->is_static))
				THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Keyword \"this\" only be used in non-static member function.", Vect2i());
			expr = new_node<ThisNode>();

		} else if (tk->type == Token::KWORD_SUPER) {
			if (parser_context.current_class == nullptr || (parser_context.current_func && parser_context.current_func->is_static))
				THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Keyword \"super\" only be used in non-static member function.", Vect2i());
			expr = new_node<SuperNode>();

		} else if (tk->type == Token::VALUE_FLOAT || tk->type == Token::VALUE_INT || tk->type == Token::VALUE_STRING) {
			expr = new_node<ConstValueNode>(tk->constant);

		} else if (tk->type == Token::OP_PLUS || tk->type == Token::OP_MINUS || tk->type == Token::OP_NOT || tk->type == Token::OP_BIT_NOT) {
			switch (tk->type) {
				case Token::OP_PLUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_POSITIVE, tokenizer->get_pos()));
					break;
				case Token::OP_MINUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NEGATIVE, tokenizer->get_pos()));
					break;
				case Token::OP_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NOT, tokenizer->get_pos()));
					break;
				case Token::OP_BIT_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_BIT_NOT, tokenizer->get_pos()));
					break;
			}
			continue;
		} else if ((tk->type == Token::IDENTIFIER || tk->type == Token::BUILTIN_FUNC) && tokenizer->peek().type == Token::BRACKET_LPARAN) {
			ptr<OperatorNode> call = new_node<OperatorNode>(OperatorNode::OpType::OP_CALL);

			if (tk->type == Token::BUILTIN_FUNC) {
				call->args.push_back(new_node<BuiltinFunctionNode>(tk->builtin_func));
			} else {
				// Identifier node could be builtin class like File(), another static method, ...
				// will know when reducing.
				call->args.push_back(new_node<Node>()); // UNKNOWN on may/may-not be self
				call->args.push_back(new_node<IdentifierNode>(tk->identifier));
			}

			tk = &tokenizer->next();
			ASSERT(tk->type == Token::BRACKET_LPARAN);
			call->args = _parse_arguments(p_parent);
			expr = call;

		} else if (tk->type == Token::IDENTIFIER) {
			expr = new_node<IdentifierNode>(tk->identifier);

		} else if (tk->type == Token::BRACKET_LCUR) {
			// No literal for dictionary.
			ptr<ArrayNode> arr = new_node<ArrayNode>();
			bool done = false;
			bool comma_valid = false;
			while (!done) {
				tk = &tokenizer->peek();
				switch (tk->type) {
					case Token::_EOF:
						tk = &tokenizer->next(); // eat eof
						THROW_UNEXP_TOKEN("");
						break;
					case Token::SYM_COMMA:
						tk = &tokenizer->next(); // eat comma
						if (!comma_valid) {
							THROW_UNEXP_TOKEN("");
						}
						comma_valid = false;
						break;
					case Token::BRACKET_RCUR:
						tk = &tokenizer->next(); // eat '}'
						done = true;
						break;
					default:
						ptr<Node> subexpr = _parse_expression(p_parent);
						arr->elements.push_back(subexpr);
						comma_valid = true;
				}
			}
			expr = arr;
		} else {
			THROW_UNEXP_TOKEN("");
		}

		// -- PARSE INDEXING -------------------------------------------------------

		while (true) {

			tk = &tokenizer->peek();
			// .named_index
			if (tk->type == Token::SYM_DOT) {
				tk = &tokenizer->next(1);

				if (tk->type != Token::IDENTIFIER && tk->type != Token::BUILTIN_FUNC) {
					THROW_UNEXP_TOKEN("");
				}

				// call
				if (tokenizer->peek().type == Token::BRACKET_LPARAN) {
					ptr<OperatorNode> call = new_node<OperatorNode>(OperatorNode::OpType::OP_CALL);
					
					call->args.push_back(expr);
					call->args.push_back(new_node<IdentifierNode>(tk->identifier));
					tk = &tokenizer->next();
					call->args = _parse_arguments(p_parent);

					expr = call;

				// Just indexing.
				} else {
					ptr<OperatorNode> ind = new_node<OperatorNode>(OperatorNode::OpType::OP_INDEX);

					ind->args.push_back(expr);
					ind->args.push_back(new_node<IdentifierNode>(tk->identifier));
					expr = ind;
				}


			// [mapped_index]
			} else if (tk->type == Token::BRACKET_LSQ) {
				ptr<OperatorNode> ind_mapped = new_node<OperatorNode>(OperatorNode::OpType::OP_INDEX_MAPPED);

				tk = &tokenizer->next();
				ptr<Node> key = _parse_expression(p_parent);
				tk = &tokenizer->next();
				if (tk->type != Token::BRACKET_RSQ) {
					THROW_UNEXP_TOKEN("symbol \"}\"");
				}

				ind_mapped->args.push_back(expr);
				ind_mapped->args.push_back(key);
				expr = ind_mapped;

			} else {
				break;
			}

		}

		expressions.push_back(Expr(expr));

		// -- PARSE OPERATOR -------------------------------------------------------
		tk = &tokenizer->peek();

		OperatorNode::OpType op;
		bool valid = true;
		
		switch (tk->type) {
		#define OP_CASE(m_op) case Token::m_op: op = OperatorNode::OpType::m_op; break
			OP_CASE(OP_EQ);
			OP_CASE(OP_EQEQ);
			OP_CASE(OP_PLUS);
			OP_CASE(OP_PLUSEQ);
			OP_CASE(OP_MINUS);
			OP_CASE(OP_MINUSEQ);
			OP_CASE(OP_MUL);
			OP_CASE(OP_MULEQ);
			OP_CASE(OP_DIV);
			OP_CASE(OP_DIVEQ);
			OP_CASE(OP_MOD);
			OP_CASE(OP_MOD_EQ);
			OP_CASE(OP_LT);
			OP_CASE(OP_LTEQ);
			OP_CASE(OP_GT);
			OP_CASE(OP_GTEQ);
			OP_CASE(OP_AND);
			OP_CASE(OP_OR);
			OP_CASE(OP_NOT);
			OP_CASE(OP_NOTEQ);
			OP_CASE(OP_BIT_NOT);
			OP_CASE(OP_BIT_LSHIFT);
			OP_CASE(OP_BIT_LSHIFT_EQ);
			OP_CASE(OP_BIT_RSHIFT);
			OP_CASE(OP_BIT_RSHIFT_EQ);
			OP_CASE(OP_BIT_OR);
			OP_CASE(OP_BIT_OR_EQ);
			OP_CASE(OP_BIT_AND);
			OP_CASE(OP_BIT_AND_EQ);
			OP_CASE(OP_BIT_XOR);
			OP_CASE(OP_BIT_XOR_EQ);
		#undef OP_CASE

			default: valid = false;
		}
		MISSED_ENUM_CHECK(OperatorNode::OpType::__OP_MAX__, 36);

		if (valid) {
			tokenizer->next(); // Eat peeked token.
			expressions.push_back(Expr(op, tokenizer->get_pos()));
		} else {
			break;
		}
	}

	return _build_operator_tree(expressions);

}

stdvec<ptr<Parser::Node>> Parser::_parse_arguments(const ptr<Node>& p_parent) {
	const TokenData* tk = &tokenizer->peek();
	stdvec<ptr<Node>> args;

	if (tk->type == Token::BRACKET_RPARAN) {
		tokenizer->next(); // eat BRACKET_RPARAN
	} else {
		while (true) {

			ptr<Node> arg = _parse_expression(p_parent);
			args.push_back(arg);

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
				// pass
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				THROW_UNEXP_TOKEN("");
			}
		}
	}

	return args;
}

int Parser::_get_operator_precedence(OperatorNode::OpType p_op) {
	switch (p_op) {
		case OperatorNode::OpType::OP_NOT:
		case OperatorNode::OpType::OP_BIT_NOT:
		case OperatorNode::OpType::OP_POSITIVE:
		case OperatorNode::OpType::OP_NEGATIVE:
			return 0;
		case OperatorNode::OpType::OP_MUL:
		case OperatorNode::OpType::OP_DIV:
		case OperatorNode::OpType::OP_MOD:
			return 1;
		case OperatorNode::OpType::OP_PLUS:
		case OperatorNode::OpType::OP_MINUS:
			return 2;
		case OperatorNode::OpType::OP_BIT_LSHIFT:
		case OperatorNode::OpType::OP_BIT_RSHIFT:
			return 3;
		case OperatorNode::OpType::OP_LT:
		case OperatorNode::OpType::OP_LTEQ:
		case OperatorNode::OpType::OP_GT:
		case OperatorNode::OpType::OP_GTEQ:
			return 4;
		case OperatorNode::OpType::OP_EQEQ:
		case OperatorNode::OpType::OP_NOTEQ:
			return 5;
		case OperatorNode::OpType::OP_BIT_AND:
			return 6;
		case OperatorNode::OpType::OP_BIT_XOR:
			return 7;
		case OperatorNode::OpType::OP_BIT_OR:
			return 8;
		case OperatorNode::OpType::OP_AND:
			return 9;
		case OperatorNode::OpType::OP_OR:
			return 10;
		case OperatorNode::OpType::OP_EQ:
		case OperatorNode::OpType::OP_PLUSEQ:
		case OperatorNode::OpType::OP_MINUSEQ:
		case OperatorNode::OpType::OP_MULEQ:
		case OperatorNode::OpType::OP_DIVEQ:
		case OperatorNode::OpType::OP_MOD_EQ:
		case OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_AND_EQ:
		case OperatorNode::OpType::OP_BIT_XOR_EQ:
		case OperatorNode::OpType::OP_BIT_OR_EQ:
			return 11;
		default:
			ASSERT(false);
			return -1;
	}
	MISSED_ENUM_CHECK(OperatorNode::OpType::__OP_MAX__, 36);
}

ptr<Parser::Node> Parser::_build_operator_tree(stdvec<Expr>& p_expr) {
	
	ASSERT(p_expr.size() > 0);

	while (p_expr.size() > 1) {

		int next_op = -1;
		int min_precedence = 0xFFFFF;
		bool unary = false;

		for (int i = 0; i < p_expr.size(); i++) {
			if (!p_expr[i].is_op()) {
				continue;
			}

			int precedence = _get_operator_precedence(p_expr[i].get_op());
			if (precedence < min_precedence) {
				next_op = i;
				OperatorNode::OpType op = p_expr[i].get_op();
				unary = (
					op == OperatorNode::OpType::OP_NOT ||
					op == OperatorNode::OpType::OP_BIT_NOT ||
					op == OperatorNode::OpType::OP_POSITIVE ||
					op == OperatorNode::OpType::OP_NEGATIVE );
			}
		}

		ASSERT(next_op >= 0);

		if (unary) {

			int next_expr = next_op;
			while (p_expr[next_expr].is_op()) {
				if (++next_expr == p_expr.size()) {
					THROW_PARSER_ERR(Error::SYNTAX_ERROR, "", Vect2i());
				}
			}

			for (int i = next_expr - 1; i >= next_op; i--) {
				ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[i].get_op());
				op_node->pos = p_expr[i].get_pos();
				op_node->args.push_back(p_expr[(size_t)i + 1].get_expr());
				p_expr.at(i) = Expr(op_node);
				p_expr.erase(p_expr.begin() + i + 1);
			}

		} else {
			ASSERT(next_op >= 1 && next_op < p_expr.size()-1);
			ASSERT(!p_expr[(size_t)next_op - 1].is_op() && !p_expr[(size_t)next_op + 1].is_op());

			ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[(size_t)next_op].get_op());
			op_node->pos = p_expr[next_op].get_pos();

			if (p_expr[(size_t)next_op - 1].get_expr()->type == Node::Type::OPERATOR) {
				switch (ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->op_type) {
					case OperatorNode::OpType::OP_EQ:
					case OperatorNode::OpType::OP_PLUSEQ:
					case OperatorNode::OpType::OP_MINUSEQ:
					case OperatorNode::OpType::OP_MULEQ:
					case OperatorNode::OpType::OP_DIVEQ:
					case OperatorNode::OpType::OP_MOD_EQ: {
						Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->pos;
						THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Unexpected assignment.", Vect2i(pos.x, pos.y));
					}
				}
			}

			if (p_expr[(size_t)next_op + 1].get_expr()->type == Node::Type::OPERATOR) {
				switch (ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->op_type) {
					case OperatorNode::OpType::OP_EQ:
					case OperatorNode::OpType::OP_PLUSEQ:
					case OperatorNode::OpType::OP_MINUSEQ:
					case OperatorNode::OpType::OP_MULEQ:
					case OperatorNode::OpType::OP_DIVEQ:
					case OperatorNode::OpType::OP_MOD_EQ: {
						Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->pos;
						THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Unexpected assignment.", Vect2i(pos.x, pos.y));
					}
				}
			}

			op_node->args.push_back(p_expr[(size_t)next_op - 1].get_expr());
			op_node->args.push_back(p_expr[(size_t)next_op + 1].get_expr());

			p_expr.at((size_t)next_op - 1) = Expr(op_node);
			p_expr.erase(p_expr.begin() + next_op);
			p_expr.erase(p_expr.begin() + next_op);
		}
	}
	ASSERT(!p_expr[0].is_op());
	return p_expr[0].get_expr();
}

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
		case Node::Type::MAP: {
			ptr<MapNode> map = ptrcast<MapNode>(p_expr);
			for (int i = 0; i < map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: key should be hashable?
				_reduce_expression(map->elements[i].value);
			}
		} break;
		case Node::Type::OPERATOR: {
			ptr<OperatorNode> op = ptrcast<OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Node::Type::CONST_VALUE) {
					if (i == 0 && op->args[i]->type == Node::Type::BUILTIN_FUNCTION) {
						// Could be all const.
					} else {
						all_const = false;
					}
				}
			}

			switch (op->op_type) {
				case OperatorNode::OpType::OP_CALL: {

					// TODO: add built in class for Array(), Map(), ...
					if (op->args[0]->type == Node::Type::BUILTIN_FUNCTION && all_const) {
						ptr<BuiltinFunctionNode> bf = ptrcast<BuiltinFunctionNode>(op->args[0]);
						if (bf->func != BuiltinFunctions::Type::PRINT) {
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
				} break;
				case OperatorNode::OpType::OP_INDEX: {
					// TODO: const_node_var.x or idf_node_enum.VALUE
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
