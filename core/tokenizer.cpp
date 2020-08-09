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

#include "tokenizer.h"

namespace carbon {

#define GET_CHAR(m_off) \
( ((size_t)char_ptr + m_off >= source.size())? '\0': source[(size_t)char_ptr + m_off] )

#define EAT_CHAR(m_num)      \
{	char_ptr += m_num;       \
	cur_col += m_num;        \
}

#define EAT_LINE()    \
{	char_ptr++;       \
	cur_col = 1;      \
	cur_line++;       \
}

#define IS_NUM(c)     \
( ('0' <= c && c <= '9') )

#define IS_TEXT(c)    \
( (c == '_') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') )


/*TODO: msg += __LINE__, __FUNCTION__*/

struct KeywordName { const char* name; Token tk; };
static KeywordName _keyword_name_list[] = {
	{ "import",   Token::KWORD_IMPORT	     },
	{ "class",    Token::KWORD_CLASS	     },
	{ "enum",     Token::KWORD_ENUM	         },
	{ "func",     Token::KWORD_FUNC	         },
	{ "var",      Token::KWORD_VAR		     },
	{ "null",     Token::KWORD_NULL	         },
	{ "true",     Token::KWORD_TRUE	         },
	{ "false",    Token::KWORD_FALSE	     },
	{ "if",       Token::KWORD_IF		     },
	{ "else",     Token::KWORD_ELSE	         },
	{ "while",    Token::KWORD_WHILE	     },
	{ "for",      Token::KWORD_FOR		     },
	{ "switch",   Token::KWORD_SWITCH		 },
	{ "case",     Token::KWORD_CASE		     },
	{ "break",    Token::KWORD_BREAK	     },
	{ "continue", Token::KWORD_CONTINUE      },
	{ "static",   Token::KWORD_STATIC        },
	{ "this",     Token::KWORD_THIS          },
	{ "super",    Token::KWORD_SUPER         },
	{ "return",   Token::KWORD_RETURN	     },
};
MISSED_ENUM_CHECK(Token::_TK_MAX_, 73);

void Tokenizer::_eat_escape(String& p_str) {
	char c = GET_CHAR(0);
	ASSERT(c == '\\');
	c = GET_CHAR(1);
	switch (c) {
		case 0:
			THROW_TOKENIZE_ERROR(Error::UNEXPECTED_EOF, "");
			break;
		case '\\': p_str += '\\'; EAT_CHAR(2); break;
		case '\'': p_str += '\''; EAT_CHAR(2); break;
		case 't':  p_str += '\t'; EAT_CHAR(2); break;
		case 'n':  p_str += '\n'; EAT_CHAR(2); break;
		case '"':  p_str += '"';  EAT_CHAR(2); break;
		case 'r':  p_str += '\r'; EAT_CHAR(2); break;
		case '\n': EAT_CHAR(1); EAT_LINE(); break;
		default: p_str += c;
	}
}

void Tokenizer::_eat_token(Token p_tk, int p_eat_size) {
	TokenData tk;
	tk.type = p_tk;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

void Tokenizer::_eat_eof() {
	TokenData tk;
	tk.type = Token::_EOF;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(1);
}

// TODO: eat const value, ... cur_line, cur_col are not at the end of the token
// make the position to be at the start
void Tokenizer::_eat_const_value(const var& p_value, int p_eat_size) {
	TokenData tk;
	tk.line = cur_line;
	tk.col = cur_col - __const_val_token_len;
	tk.constant = p_value;

	switch (p_value.get_type()) {
		case var::STRING:
			tk.type = Token::VALUE_STRING;
			break;
		case var::INT:
			tk.type = Token::VALUE_INT;
			break;
		case var::FLOAT:
			tk.type = Token::VALUE_FLOAT;
			break;
		default:
			THROW_TOKENIZE_ERROR(Error::INTERNAL_BUG, "Internal Bug: Report!");
			break;
	}

	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

void Tokenizer::_eat_identifier(const String& p_idf, int p_eat_size) {
	
	TokenData tk;
	tk.type = Token::IDENTIFIER;
	tk.identifier = p_idf; // method name may be builtin func
	tk.col = cur_col - (int)p_idf.size();
	tk.line = cur_line;

	for (const KeywordName& kw : _keyword_name_list) {
		if (kw.name == p_idf) {
			tk.type = kw.tk;
			if (tk.type == Token::KWORD_TRUE) {
				tk.type = Token::VALUE_BOOL;
				tk.constant = var(true);
			} else if (tk.type == Token::KWORD_FALSE) {
				tk.type = Token::VALUE_BOOL;
				tk.constant = var(false);
			}
			break;
		}
	}

	if (tk.type == Token::IDENTIFIER) {
		BuiltinTypes::Type bf_type = BuiltinTypes::get_type_type(tk.identifier);
		if (bf_type != BuiltinTypes::UNKNOWN) {
			tk.type = Token::BUILTIN_TYPE;
			tk.builtin_type = bf_type;
		}
	}

	tokens.push_back(tk);
	EAT_CHAR(p_eat_size);
}

const void Tokenizer::tokenize(const String& p_source, const String& p_source_path) {

	source = p_source;
	source_path = p_source_path;
	cur_line = cur_col = 1;
	char_ptr = 0;
	tokens.clear();

	while (char_ptr < (int)source.size()) {

		switch (GET_CHAR(0)) {
			case 0:
				_eat_eof();
				break;
			case ' ':
			case '\t':
				EAT_CHAR(1);
				break;
			case '\n':
				EAT_LINE();
				break;
			case '/':
			{
				if (GET_CHAR(1) == '/') { // comment

					while (GET_CHAR(0) != '\n' && GET_CHAR(0) != 0 ) {
						EAT_CHAR(1);
					}
					if (GET_CHAR(0) == '\n') {
						EAT_LINE();
					} else if (GET_CHAR(0) == 0) {
						_eat_eof();
					}
					
				} else if (GET_CHAR(1) == '*') { // multi line comment

					EAT_CHAR(2);
					while (true) {
						if (GET_CHAR(0) == '*' && GET_CHAR(1) == '/') {
							EAT_CHAR(2);
							break;
						} else if (GET_CHAR(0) == 0) {
							THROW_TOKENIZE_ERROR(Error::UNEXPECTED_EOF, ""); // TODO: Error message.
						} else if (GET_CHAR(0) == '\n') {
							EAT_LINE();
						} else {
							EAT_CHAR(1);
						}
					}

				} else {
					if (GET_CHAR(1) == '=') _eat_token(Token::OP_DIVEQ, 2);
					else _eat_token(Token::OP_DIV);
				}
				break;
			}
			// symbols
			case ',': _eat_token(Token::SYM_COMMA); break;
			case ':': _eat_token(Token::SYM_COLLON); break;
			case ';': _eat_token(Token::SYM_SEMI_COLLON); break;
			case '@': _eat_token(Token::SYM_AT); break;
			case '#': _eat_token(Token::SYM_HASH); break;
			case '$': _eat_token(Token::SYM_DOLLAR); break;
			case '?': _eat_token(Token::SYM_QUESTION); break;

			// brackets
			case '(': _eat_token(Token::BRACKET_LPARAN); break;
			case ')': _eat_token(Token::BRACKET_RPARAN); break;
			case '{': _eat_token(Token::BRACKET_LCUR); break;
			case '}': _eat_token(Token::BRACKET_RCUR); break;
			case '[': _eat_token(Token::BRACKET_LSQ); break;
			case ']': _eat_token(Token::BRACKET_RSQ); break;

			// op
			case '=': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_EQEQ, 2);
				else _eat_token(Token::OP_EQ);
				break;
			}
			case '+': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_PLUSEQ, 2);
				else _eat_token(Token::OP_PLUS);
				break;
			}
			case '-': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MINUSEQ, 2);
				else _eat_token(Token::OP_MINUS);
				break;
			}
			case '*': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MULEQ, 2);
				else _eat_token(Token::OP_MUL);
				break;
			}
			// case '/': { } // already hadled
			case '\\':
				THROW_TOKENIZE_ERROR(Error::SYNTAX_ERROR, "Invalid character '\\'");
				break;
			case '%': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MOD_EQ, 2);
				else _eat_token(Token::OP_MOD);
				break;
			}
			case '<': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_LTEQ, 2);
				else if (GET_CHAR(1) == '<') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_BIT_LSHIFT_EQ, 3);
					else _eat_token(Token::OP_BIT_LSHIFT, 2);
				}
				else _eat_token(Token::OP_LT);
				break;
			}
			case '>': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_GTEQ, 2);
				else if (GET_CHAR(1) == '>') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_BIT_RSHIFT_EQ, 3);
					else _eat_token(Token::OP_BIT_RSHIFT, 2);
				}
				else _eat_token(Token::OP_GT);
				break;
			}
			case '!': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_NOTEQ, 2);
				else _eat_token(Token::OP_NOT);
				break;
			}
			case '~':
				_eat_token(Token::OP_BIT_NOT);
				break;
			case '|': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_OR_EQ, 2);
				else if (GET_CHAR(1) == '|') _eat_token(Token::OP_OR, 2);
				else _eat_token(Token::OP_BIT_OR);
				break;
			}
			case '&': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_AND_EQ, 2);
				else if (GET_CHAR(1) == '&') _eat_token(Token::OP_AND, 2);
				else _eat_token(Token::OP_BIT_AND);
				break;
			}
			case '^': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_BIT_XOR_EQ, 2);
				else _eat_token(Token::OP_BIT_XOR);
				break;
			}

			// double quote string value single quote not supported yet
			case '"': {
				EAT_CHAR(1);
				String str;
				while (GET_CHAR(0) != '"') {
					if (GET_CHAR(0) == '\\') {
						_eat_escape(str);
					} else if (GET_CHAR(0) == 0) {
						THROW_TOKENIZE_ERROR(Error::UNEXPECTED_EOF, ""); // TODO: Error message.
						break;
					} else if(GET_CHAR(0) == '\n'){
						THROW_TOKENIZE_ERROR(Error::SYNTAX_ERROR, "Unexpected EOL while parsing string");
						break;
					} else {
						str += GET_CHAR(0);
						EAT_CHAR(1);
					}
				}
				EAT_CHAR(1);
				__const_val_token_len = (int)str.size() + 2;
				_eat_const_value(str);
				break;
			}
			case '\'':
				THROW_TOKENIZE_ERROR(Error::SYNTAX_ERROR, "Invalid character '\\''.");
				break;
			default: {
				
				// NOTE: 1.2.3 => float=1.2 float=.3 is this okey?
				// TODO: 1.2e3 => is a valid float number
				// TODO: hex/binary/octal numbers

				// float value begins with '.'
				if (GET_CHAR(0) == '.' && IS_NUM(GET_CHAR(1)) ) {
					String float_str = '.';
					EAT_CHAR(1);
					while (IS_NUM(GET_CHAR(0))) {
						float_str += GET_CHAR(0);
						EAT_CHAR(1);
					}
					double float_val = float_str.to_float();
					__const_val_token_len = (int)float_str.size();
					_eat_const_value(float_val);
					break;
				}
				// integer/float value
				if (IS_NUM(GET_CHAR(0))) {
					String num = GET_CHAR(0);
					EAT_CHAR(1);
					bool is_float = false;
					while (IS_NUM(GET_CHAR(0)) || GET_CHAR(0) == '.' ) {
						if (GET_CHAR(0) == '.' && is_float)
							break;
						if (GET_CHAR(0) == '.')
							is_float = true;
						num += GET_CHAR(0);
						EAT_CHAR(1);
					}

					// "1." parsed as 1.0 which should be error.
					if (num[num.size() - 1] == '.') THROW_TOKENIZE_ERROR(Error::SYNTAX_ERROR, "Invalid numeric value.");

					__const_val_token_len = (int)num.size();
					if (is_float)
						_eat_const_value(num.to_float());
					else
						_eat_const_value(num.to_int());
					break;
				}
				// identifier
				if (IS_TEXT(GET_CHAR(0))) {
					String identifier = GET_CHAR(0);
					EAT_CHAR(1);
					while (IS_TEXT(GET_CHAR(0)) || IS_NUM(GET_CHAR(0))) {
						identifier += GET_CHAR(0);
						EAT_CHAR(1);
					}
					_eat_identifier(identifier);
					break;
				}

				if (GET_CHAR(0) == '.') {
					_eat_token(Token::SYM_DOT);
					break;
				}

				DEBUG_BREAK(); // TODO: Unknown character.

			} // default case

		} // switch
	} // while

	_eat_eof();
}

}