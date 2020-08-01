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

ptr<Parser::BlockNode> Parser::_parse_block(const ptr<Node>& p_parent, bool p_single_statement) {
	ptr<BlockNode> block_node = newptr<BlockNode>();

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

			case Token::BRACKET_RCUR: {
				tokenizer->next(); // eat "}"
				return block_node;
			} break;

			case Token::KWORD_VAR: {
				tokenizer->next(); // eat "var"
				stdvec<ptr<VarNode>> vars = _parse_var(file_node);
				for (ptr<VarNode>& _var : vars) {
					block_node->local_vars.push_back(_var);
				}
			} break;

			// Ignore.
			case Token::SYM_SEMI_COLLON:
			case Token::VALUE_STRING:
				tokenizer->next(); // eat ";" or "string literal"
				break;

			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				block_node->statements.push_back(_parse_if_block(block_node));
			} break;

			// TODO: case keyword switch
			// TODO: case keyword while
			// TODO: case keyword break
			// TODO: case keyword continue
			// TODO: case keyword return

			default: {
				ptr<Node> expr = _parse_expression(block_node);
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
	ptr<Node> cond = _parse_expression(p_parent);
	if_block->args.push_back(cond);

	const TokenData* tk = &tokenizer->peek();
	ptr<BlockNode> body;
	if (tk->type == Token::BRACKET_LCUR) {
		tokenizer->next(); // eat "{"
		body = _parse_block(p_parent);
		if (tokenizer->peek(-1).type != Token::BRACKET_RCUR) {
			THROW_UNEXP_TOKEN("symbol \"}\"");
		}
	} else {
		body = _parse_block(p_parent, true);
	}
	if_block->body = body;

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
				if (tokenizer->peek(-1).type != Token::BRACKET_RCUR)  THROW_UNEXP_TOKEN("symbol \"}\"");
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
