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

ptr<Parser::BlockNode> Parser::_parse_block(const ptr<Node>& p_parent, bool p_single_statement, stdvec<Token> p_termination) {
	ptr<BlockNode> block_node = newptr<BlockNode>();
	block_node->parernt_node = p_parent;

	parser_context.current_block = block_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_block = nullptr;
		}
	};
	ScopeDestruct distruct = ScopeDestruct(&parser_context);

	const TokenData* tk;

	while (true) {
		tk = &tokenizer->peek();
		switch (tk->type) {
			case Token::_EOF: {
				THROW_PARSER_ERR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::KWORD_VAR: {
				tokenizer->next(); // eat "var"
				stdvec<ptr<VarNode>> vars = _parse_var(block_node);
				for (ptr<VarNode>& _var : vars) {
					block_node->local_vars.push_back(_var); // for quick access.
					block_node->statements.push_back(_var);
				}
			} break;

			// Ignore.
			case Token::SYM_SEMI_COLLON:
			case Token::VALUE_STRING: // should I include VALUE_INT, VALUE_FLOAT ??
				tokenizer->next(); // eat ";" or "string literal"
				break;

			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				block_node->statements.push_back(_parse_if_block(block_node));
			} break;

			case Token::KWORD_SWITCH: {
				tk = &tokenizer->next(); // eat "switch"
				ptr<ControlFlowNode> switch_block = new_node<ControlFlowNode>(ControlFlowNode::SWITCH);

				switch_block->args.push_back(_parse_expression(block_node, false));
				if (tokenizer->next().type != Token::BRACKET_LCUR) THROW_UNEXP_TOKEN("symbol \"{\"");

				while (true) {
					tk = &tokenizer->next();
					if (tk->type == Token::KWORD_CASE) {
						ControlFlowNode::SwitchCase _case;
						_case.value = _parse_expression(block_node, false); // TODO: const int.
						if (tokenizer->next().type != Token::SYM_COLLON) THROW_UNEXP_TOKEN("symbol \":\"");

						// COMMENTED: `case VALUE: { expr; expr; }` <--- curly brackets are not allowed.
						//tk = &tokenizer->peek();
						//if (tk->type == Token::BRACKET_LCUR) {
						//	tokenizer->next(); // eat "{"
						//	_case.body = _parse_block(block_node);
						//	if (tokenizer->next().type != Token::BRACKET_RCUR) THROW_UNEXP_TOKEN("symbol \"}\"");
						//} else {
						_case.body = _parse_block(block_node, false, { Token::KWORD_CASE, Token::BRACKET_RCUR });
						//}
						switch_block->switch_cases.push_back(_case);

					} else if (tk->type == Token::BRACKET_RCUR) {
						break;
					} else {
						THROW_UNEXP_TOKEN("keyword \"case\" or symbol \"}\"");
					}
				}
				block_node->statements.push_back(switch_block);

			} break;

			case Token::KWORD_WHILE: {
				tk = &tokenizer->next(); // eat "while"
				ptr<ControlFlowNode> while_block = new_node<ControlFlowNode>(ControlFlowNode::WHILE);
				while_block->args.push_back(_parse_expression(block_node, false));
				tk = &tokenizer->peek();
				if (tk->type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					while_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) THROW_UNEXP_TOKEN("symbol \"}\"");
				} else {
					while_block->body = _parse_block(block_node, true);
				}
				block_node->statements.push_back(while_block);
			} break;

			case Token::KWORD_FOR: {
				tk = &tokenizer->next(); // eat "for"
				ptr<ControlFlowNode> for_block = new_node<ControlFlowNode>(ControlFlowNode::FOR);
				if (tokenizer->next().type != Token::BRACKET_LPARAN) THROW_UNEXP_TOKEN("symbol \"(\"");

				if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
					tokenizer->next(); // eat ";"
					for_block->args.push_back(nullptr);
				} else {
					if (tokenizer->peek().type == Token::KWORD_VAR) {
						tokenizer->next(); // eat "var"
						stdvec<ptr<VarNode>> vars = _parse_var(block_node);
						if (vars.size() > 1) THROW_PARSER_ERR(Error::SYNTAX_ERROR, "multiple assignment not allowed here", vars[1]->pos);
						for_block->args.push_back(vars[0]);
					} else {
						for_block->args.push_back(_parse_expression(block_node, true));
						if (tokenizer->next().type != Token::SYM_SEMI_COLLON) THROW_UNEXP_TOKEN("symbol \";\"");
					}
				}

				if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
					tokenizer->next(); // eat ";"
					for_block->args.push_back(nullptr);
				} else {
					for_block->args.push_back(_parse_expression(block_node, false));
					if (tokenizer->next().type != Token::SYM_SEMI_COLLON) THROW_UNEXP_TOKEN("symbol \";\"");
				}

				if (tokenizer->peek().type == Token::BRACKET_RPARAN) {
					tokenizer->next(); // eat ")"
					for_block->args.push_back(nullptr);
				} else {
					for_block->args.push_back(_parse_expression(block_node, true));
					if (tokenizer->next().type != Token::BRACKET_RPARAN) THROW_UNEXP_TOKEN("symbol \")\"");
				}

				if (tokenizer->peek().type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					for_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) THROW_UNEXP_TOKEN("symbol \"}\"");
				} else {
					for_block->body = _parse_block(block_node, true);
				}
				block_node->statements.push_back(for_block);
			} break;

			case Token::KWORD_BREAK: {
				tk = &tokenizer->next(); // eat "break"
				// TODO: check if inside loop/switch
				ptr<ControlFlowNode> _break = new_node<ControlFlowNode>(ControlFlowNode::BREAK);
				block_node->statements.push_back(_break);
			} break;

			case Token::KWORD_CONTINUE: {
				tk = &tokenizer->next(); // eat "continue"
				// TODO: check if inside loop
				ptr<ControlFlowNode> _continue = new_node<ControlFlowNode>(ControlFlowNode::CONTINUE);
				block_node->statements.push_back(_continue);
			} break;

			case Token::KWORD_RETURN: {
				tk = &tokenizer->next(); // eat "return"
				if (!parser_context.current_func) {
					THROW_PARSER_ERR(Error::SYNTAX_ERROR, "can't use return outside a function", Vect2i());
				}
				ptr<ControlFlowNode> _return = new_node<ControlFlowNode>(ControlFlowNode::RETURN);
				block_node->statements.push_back(_return);
			} break;

			default: {
				for (Token termination : p_termination) {
					if (tk->type == termination) {
						return block_node;
					}
				}
				ptr<Node> expr = _parse_expression(block_node, true);
				if (tokenizer->next().type != Token::SYM_SEMI_COLLON) THROW_UNEXP_TOKEN("symbol \";\"");
				block_node->statements.push_back(expr);
			}
		}

		if (p_single_statement)
			break;
	}

	return block_node;
}


ptr<Parser::ControlFlowNode> Parser::_parse_if_block(const ptr<BlockNode>& p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IF);

	ptr<ControlFlowNode> if_block = new_node<ControlFlowNode>(ControlFlowNode::IF);
	ptr<Node> cond = _parse_expression(p_parent, false);
	if_block->args.push_back(cond);

	const TokenData* tk = &tokenizer->peek();
	if (tk->type == Token::BRACKET_LCUR) {
		tokenizer->next(); // eat "{"
		if_block->body = _parse_block(p_parent);
		if (tokenizer->next().type != Token::BRACKET_RCUR) THROW_UNEXP_TOKEN("symbol \"}\"");
	} else {
		if_block->body = _parse_block(p_parent, true);
	}

	tk = &tokenizer->peek(0);
	while (tk->type == Token::KWORD_ELSE) {
		tk = &tokenizer->next(1);
		switch (tk->type) {
			case Token::KWORD_IF: {
				if_block->body_else = new_node<BlockNode>();
				if_block->body_else->statements.push_back(_parse_if_block(p_parent));
			} break;
			case Token::BRACKET_LCUR: {
				if_block->body_else = _parse_block(p_parent);
				if (tokenizer->next().type != Token::BRACKET_RCUR) THROW_UNEXP_TOKEN("symbol \"}\"");
			} break;
			default: {
				if_block->body_else = _parse_block(p_parent, true);
			}
		}
		tk = &tokenizer->peek();
	}

	return if_block;
}

}
