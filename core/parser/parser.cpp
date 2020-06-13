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

#define IF_IDF_ALREADY_FOUND_RET_ERR(m_identifier, m_node)                                                                                \
	do {                                                                                                                                  \
		IdentifierLocation loc = _find_identifier_location(m_identifier, m_node);                                                         \
		if (loc.found) {                                                                                                                  \
			_throw(Error::ALREADY_DEFINED, String::format("Identifier %s already defined at %s:%i", loc.file_path, loc.line));            \
		}                                                                                                                                 \
	} while (false)

void Parser::_throw(Error::Type p_type, const String& p_msg, int p_line) {
    /*TODO: msg += __LINE__, __FUNCTION__*/
	if (p_line > 0) {
		throw Error(p_type, p_msg, Vect2i(p_line, 0));
	} else {
		throw Error(p_type, p_msg, Vect2i(tokenizer->get_line(), 0));
	}
}

void Parser::_throw_unexp_token(const String& p_exp) {
	if (p_exp != String()) {
		_throw(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\"). expected \"%s\"", "<tk_name>", p_exp));
	} else {
		_throw(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\").", "<tk_name>"));
	}
}

Parser::IdentifierLocation Parser::_find_identifier_location(const String& p_name, const Ptr<Node> p_node) const {
	ASSERT(p_node == nullptr || p_node->type == Node::Type::BLOCK || p_node->type == Node::Type::STRUCT);

	// if struct scope no need to check outer scope
	if (p_node && p_node->type == Node::Type::STRUCT) {
		for (const Ptr<VarNode>& lv : ptr_cast(StructNode, p_node)->members) {
			if (lv->name == p_name) {
				return IdentifierLocation(p_node, file_path);
			}
		}
		return IdentifierLocation();
	}

	Ptr<Node> outer_node = p_node;
	while (outer_node) {
		switch (outer_node->type) {

			case Node::Type::BLOCK: {
				for (const Ptr<VarNode>& local_var : ptr_cast(BlockNode, outer_node)->local_vars) {
					if (local_var->name == p_name) {
						return IdentifierLocation(outer_node, file_path);
					}
				}
			}
			case Node::Type::FUNCTION: {
				for (const String& arg : ptr_cast(FunctionNode, outer_node)->args) {
					if (arg == p_name) {
						return IdentifierLocation(outer_node, file_path);
					}
				}
			}
		}
		outer_node = outer_node->parern_node;
	}

	for (const Ptr<StructNode>& struct_node : file_node->structs) {
		if (struct_node->name == p_name) {
			return IdentifierLocation(struct_node, file_path);
		}
	}
	for (const Ptr<EnumNode>& enum_node : file_node->enums) {
		if (enum_node->name == p_name) {
			return IdentifierLocation(enum_node, file_path);
		}
	}
	for (const Ptr<FunctionNode>& func_node : file_node->functions) {
		if (func_node->name == p_name) {
			return IdentifierLocation(func_node, file_path);
		}
	}

	// TODO: find from import lib, binary

	return IdentifierLocation();
}

void Parser::parse(String p_source, String p_file_path) {

	source = p_source;
	file_path = p_file_path;
	file_node = newptr(FileNode);
	tokenizer->tokenize(source); // this will throw

	while (true) {
	
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case  Token::_EOF:
				return;
			case Token::KWORD_IMPORT:
				// TODO:
				break;
			case Token::KWORD_STRUCT:
				_parse_struct();
				break;
			case Token::KWORD_ENUM:
				_parse_enum();
				break;
			case Token::KWORD_FUNC:
				_parse_func();
				break;
			case Token::KWORD_VAR:
				_parse_var();
				break;
			// Ignore.
			case Token::SYM_SEMI_COLLON: 
				break;
			default:
				_throw_unexp_token();
		}

	} // while true

}


void Parser::_parse_struct() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_STRUCT);
	Ptr<StructNode> struct_node = newptr(StructNode);

	const TokenData& struct_name = tokenizer->next();
	if (struct_name.type != Token::IDENTIFIER) {
		_throw_unexp_token("<identifier>");
	}
	struct_node->name = struct_name.identifier;

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		_throw_unexp_token("{");
	}
	
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case Token::_EOF:
				_throw(Error::UNEXPECTED_EOF, "Unexpected end of file.");

			case Token::BRACKET_RCUR:
				file_node->structs.push_back(struct_node);
				return;

			case Token::SYM_SEMI_COLLON: // ignore
				break;

			case Token::KWORD_VAR:
				_parse_var(struct_node);
				break;

			default:
				_throw_unexp_token("var");

		}
	}

}

void Parser::_parse_enum() { 
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_ENUM);
	Ptr<EnumNode> enum_node = newptr(EnumNode);
	int cur_value = -1;

	const TokenData& enum_name = tokenizer->next();
	if (enum_name.type != Token::IDENTIFIER) {
		_throw_unexp_token("<identifier>");
	}
	enum_node->name = enum_name.identifier;

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		_throw_unexp_token("{");
	}

	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case Token::_EOF:
				_throw(Error::UNEXPECTED_EOF, "Unexpected end of file.");

			case Token::BRACKET_RCUR:
				file_node->enums.push_back(enum_node);
				return;

			case Token::SYM_SEMI_COLLON: // ignore
				break;

			case Token::IDENTIFIER: {

				// TODO: check identifier with struct name, enum name, static var name, and from import, import-> import , ...
				enum_node->values[token.identifier] = ++cur_value;
				
				const TokenData* tk = &tokenizer->next();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next();
					if (tk->type != Token::VALUE_INT) {
						_throw_unexp_token("<integer constant>");
					}
					ASSERT(tk->constant.get_type() == var::INT);
					cur_value = tk->constant;
					enum_node->values[token.identifier] = cur_value;
				}

				tk = &tokenizer->next();
				if (tk->type != Token::SYM_COMMA) {
					_throw_unexp_token(",");
				}
				break;
			}

			default:
				_throw_unexp_token("<identifier>");
		}
	}
}

void Parser::_parse_var(Ptr<Node> p_node) {

	// TODO: how about var x, y; -> peek(-1) == kword_var or sym_comma
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);
	ASSERT(p_node == nullptr || p_node->type == Node::Type::BLOCK || p_node->type == Node::Type::STRUCT);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) {
		_throw_unexp_token("<identifier>");
	}

	// check identifier when reducing
	// IF_IDF_ALREADY_FOUND_RET_ERR(tk->identifier, p_node);

#define PARSE_EXPR_VAR()                                          \
	tk = &tokenizer->next();                                      \
	if (tk->type == Token::OP_EQ) {                               \
		Ptr<Node> expr = newptr(Node);                            \
		_parse_expression(expr);                                  \
		                                                          \
		if (tokenizer->peek(-1).type != Token::SYM_SEMI_COLLON) { \
			_throw_unexp_token(";");                              \
		}                                                         \
		var_node->assignment = expr;                              \
	}                                                             \
	else if (tokenizer->next().type != Token::SYM_SEMI_COLLON) {  \
		_throw_unexp_token(";");                                  \
	}


	Ptr<VarNode> var_node = newptr(VarNode);
	var_node->name = tk->identifier;

	if (p_node) {
		switch (p_node->type) {
			case Node::Type::STRUCT: {
				PARSE_EXPR_VAR();
				ptr_cast(StructNode, p_node)->members.push_back(var_node);
				return;
			}
			case Node::Type::BLOCK: {
				PARSE_EXPR_VAR();
				ptr_cast(BlockNode, p_node)->local_vars.push_back(var_node);
				return;
			}
			default:
				ASSERT(false); // TODO: parser bug
				break;
		}
	} else { // class var
		PARSE_EXPR_VAR();
		file_node->static_vars.push_back(var_node);
		return;
	}
}

// TODO: newptr to new_node<T> which sets the node's line, col

void Parser::_parse_func() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_FUNC);
	Ptr<FunctionNode> func_node = newptr(FunctionNode);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) {
		_throw_unexp_token("<identifier>");
	}
	func_node->name = tk->identifier;

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		_throw_unexp_token("{");
	}

	Ptr<BlockNode> body = newptr(BlockNode);
	_parse_block(body, func_node);
	if (tokenizer->peek(-1).type != Token::BRACKET_RCUR) {
		_throw_unexp_token("}");
	}
	func_node->body = body;
	file_node->functions.push_back(func_node);
}

}