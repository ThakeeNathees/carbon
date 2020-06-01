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

Error::Type Parser::_set_error(Error::Type p_type, const String& p_msg, int p_line = -1) {
	if (err.type != Error::OK) return;
	err.type = p_type;
	err.msg = p_msg;

	if (p_line > 0) {
		err.line = p_line;
	} else {
		err.line = tokenizer->get_line();
	}

	return err.type;
}

const Error& Parser::parse(String p_source, String p_file_path) {

	source = p_source;
	file_path = p_file_path;
	file_node = newptr(FileNode);
	tokenizer->tokenize(source);

	while (true) {
	
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case  Token::_EOF:
				return;
			case Token::KWORD_IMPORT:
				// TODO:
				break;
			case Token::KWORD_STRUCT:
				if (_parse_struct() != Error::OK)
					return;
				break;
			case Token::KWORD_ENUM:
				if (_parse_enum() != Error::OK)
					return;
				break;
			case Token::KWORD_FUNC:
				if (_parse_func() != Error::OK)
					return;
				break;
			case Token::KWORD_VAR:
				if (_parse_var() != Error::OK)
					return;
				break;
			case Token::SYM_SEMI_COLLON: // ignore
				break;
			default:
				DEBUG_BREAK(); // TODO:
		}

	} // while true

}

#define UNEXP_TOKEN_ERROR(m_exp)  \
	_set_error(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\"). expected \"%s\"", "", m_exp/*TODO: token to name*/))

Error::Type Parser::_parse_struct() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_STRUCT);
	Ptr<StructNode> struct_node = newptr(StructNode);

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		return UNEXP_TOKEN_ERROR("{");
	}
	
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case Token::_EOF:
				return _set_error(Error::UNEXPECTED_EOF, "Unexpected end of file.");

			case Token::BRACKET_RCUR:
				file_node->structs.push_back(struct_node);
				return err.type;

			case Token::SYM_SEMI_COLLON: // ignore
				break;

			case Token::KWORD_VAR:
				if (_parse_var(struct_node) != Error::OK) {
					return err.type;
				}
				break;

			default:
				return UNEXP_TOKEN_ERROR("var");

		}
	}

}

Error::Type Parser::_parse_enum() { 
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_ENUM);
	Ptr<EnumNode> enum_node = newptr(EnumNode);
	int cur_value = -1;

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		return UNEXP_TOKEN_ERROR("{");
	}

	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case Token::_EOF:
				return _set_error(Error::UNEXPECTED_EOF, "Unexpected end of file.");

			case Token::BRACKET_RCUR:
				file_node->enums.push_back(enum_node);
				return err.type;

			case Token::SYM_SEMI_COLLON: // ignore
				break;

			case Token::IDENTIFIER:
				// TODO: check identifier with struct name, enum name, static var name, and from import, import-> import , ...
				enum_node->values[token.identifier] = ++cur_value;
				
				const TokenData* tk = &tokenizer->next();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next();
					if (tk->type != Token::VALUE_INT) {
						UNEXP_TOKEN_ERROR("<integer constant>");
					}
					ASSERT(tk->constant.get_type() == var::INT);
					cur_value = tk->constant;
					enum_node->values[token.identifier] = cur_value;
				}

				tk = &tokenizer->next();
				if (tk->type != Token::SYM_COMMA) {
					UNEXP_TOKEN_ERROR(",");
				}
				break;

			default:
				return UNEXP_TOKEN_ERROR("<identifier>");
		}
	}
}

Error::Type Parser::_parse_var(Ptr<Node> p_struct) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) {
		UNEXP_TOKEN_ERROR("<identifier>");
	}

	Ptr<VarNode> var_node = newptr(VarNode);

	var_node->name = tk->identifier;

	if (p_struct) { // class var
		// TODO: check identifier
	} else {
		switch (p_struct->type) {
			case Node::Type::STRUCT:
				// TODO: check identifier
				// TODO:
				break;
			case Node::Type::BLOCK:
				// TODO: check identifier
				// TODO:
				break;
			default:
				ASSERT(false); // TODO: parser bug
				break;
		}
	}
}

Error::Type Parser::_parse_func() { }

}