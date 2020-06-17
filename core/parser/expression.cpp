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

ptr<Parser::Node> Parser::_parse_expression(const ptr<Node>& p_parent, bool p_static) {
	ASSERT(p_parent != nullptr);

	struct Expr {
		Expr(OperatorNode::OpType p_op) { _is_op = true; op = p_op; }
		Expr(const ptr<Node>& p_node) { _is_op = false; expr = p_node; }
		Expr(const Expr& p_other) {
			if (p_other._is_op) { _is_op = true; op = p_other.op; } 
			else { _is_op = false; expr = p_other.expr; }
		}
		~Expr() { if (!_is_op) { expr = nullptr; } }

		bool is_op() const { return _is_op; }
		OperatorNode::OpType get_op() const { return op; }
		ptr<Node>& get_expr() { return expr; }
	private:
		bool _is_op = true;
		union {
			OperatorNode::OpType op;
			ptr<Node> expr;
		};
	};
	stdvec<Expr> expressions;

	while (true) {

		const TokenData* tk = &tokenizer->next();
		ptr<Node> expr = nullptr;
		
		if (tk->type == Token::BRACKET_LPARAN) {
			expr = _parse_expression(p_parent, p_static);

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_RPARAN) {
				THROW_UNEXP_TOKEN(")");
			}

		} else if (tk->type == Token::KWORD_THIS) {
			if (p_static) {
				THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Invalid use of \"self\" in static scope.", -1);
			}
			expr = new_node<ThisNode>();

		} else if (tk->type == Token::KWORD_SUPER) {
			if (p_static) {
				THROW_PARSER_ERR(Error::SYNTAX_ERROR, "Invalid use of \"super\" in static scope.", -1);
			}
			expr = new_node<SuperNode>();

		} else if (tk->type == Token::VALUE_FLOAT || tk->type == Token::VALUE_INT || tk->type == Token::VALUE_STRING) {
			expr = new_node<ConstValueNode>(tk->constant);

		} else if (tk->type == Token::OP_PLUS || tk->type == Token::OP_MINUS || tk->type == Token::OP_NOT || tk->type == Token::OP_BIT_NOT) {
			switch (tk->type) {
				case Token::OP_PLUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_PLUS));
					break;
				case Token::OP_MINUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_MINUS));
					break;
				case Token::OP_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NOT));
					break;
				case Token::OP_BIT_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_BIT_NOT));
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

			_parse_arguments(call->args, p_parent, p_static);
			
			tk = &tokenizer->next();
			expr = call;

		} else if (tk->type == Token::IDENTIFIER) {
			expr = new_node<IdentifierNode>(tk->identifier);

		} else if (tk->type == Token::BRACKET_LCUR) {
			// No literal for dictionary.
			ptr<ArrayNode> arr = new_node<ArrayNode>();
			bool done = false;
			while (!done) {
				tk = &tokenizer->next();
				bool comma_valid = false;
				switch (tk->type) {
					case Token::_EOF:
						THROW_UNEXP_TOKEN("");
					case Token::SYM_COMMA:
						if (!comma_valid) {
							THROW_UNEXP_TOKEN("");
						}
						comma_valid = false;
					case Token::BRACKET_RCUR:
						done = true;
						break;
					default:
						ptr<Node> subexpr = _parse_expression(p_parent, p_static);
						arr->elements.push_back(subexpr);
						comma_valid = true;
				}
			}
			expr = arr;
		} else {
			THROW_UNEXP_TOKEN("");
		}

		tk = &tokenizer->peek(); // if next not valid, position doesn't increase

		// -- PARSE INDEXING -------------------------------------------------------

		while (true) {

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
					tk = &tokenizer->next(1);
					_parse_arguments(call->args, p_parent, p_static);

					expr = call;

				// Just indexing.
				} else {

				}


			// [mapped_index]
			} else if (tk->type == Token::BRACKET_LSQ) {
				tk = &tokenizer->next(1);

			}

		}

		// TODO: parse operator

		// TODO: call reduce operator

	}



}

void Parser::_parse_arguments(stdvec<ptr<Node>>& p_args, const ptr<Node>& p_parent, bool p_static) {
	// TODO:
}


void Parser::_reduce_expression(ptr<Node>& p_expr) {
}

}
