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
( (char_ptr + m_off >= source.size())? 0: source[char_ptr+m_off] )

#define EAT_CHAR(m_num)      \
{	char_ptr += m_num;       \
	cur_col += m_num;        \
}

#define EAT_LINE()    \
{	char_ptr++;       \
	cur_col = 1;      \
	cur_line++;       \
}

void Tokenizer::_eat_token(Token p_tk, int char_size) {
	TokenData tk;
	tk.type = p_tk;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(char_size);
}

void Tokenizer::_eat_eof() {
	TokenData tk;
	tk.type = Token::_EOF;
	tk.line = cur_line;
	tk.col = cur_col;
	tokens.push_back(tk);
	EAT_CHAR(1);
}


void Tokenizer::set_source(const String& p_source) {
	source = p_source;
	cur_line = cur_col = 1;
	char_ptr = 0;
	tokens.clear();

	while (char_ptr < source.size()) {
		if (has_error) return;

		switch (GET_CHAR(0)) {
			case 0:
				_eat_eof();
				break;
			case ' ':
			case '\t':
				EAT_CHAR(1);
				break;
			case '\n':
				EAT_LINE(1);
				break;
			case '/':
			{
				if (GET_CHAR(1) == '/') { // comment

					while (GET_CHAR(0) != '\n' || GET_CHAR(0) != 0 ) {
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
						} else if (GET_CHAR(0) == 0) {
							// EAT_CHAR(1); // can't eat it's error!
							has_error = true;
							error_msg = "Unexpected EOF.";
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
			// TODO: case SYM_DOT could be float value
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
			case '%': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_MOD_EQ, 2);
				else _eat_token(Token::OP_MOD);
				break;
			}
			case '!': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_NOTEQ, 2);
				else _eat_token(Token::OP_NOT);
				break;
			}
			case '<': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_LTEQ, 2);
				else if (GET_CHAR(1) == '<') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_LSHIFT_EQ, 3);
					else _eat_token(Token::OP_LSHIFT, 2);
				}
				else _eat_token(Token::OP_LT);
				break;
			}
			case '>': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_GTEQ, 2);
				else if (GET_CHAR(1) == '>') {
					if (GET_CHAR(2) == '=') _eat_token(Token::OP_RSHIFT_EQ, 3);
					else _eat_token(Token::OP_RSHIFT, 2);
				}
				else _eat_token(Token::OP_GT);
				break;
			}
			case '~':
				_eat_token(Token::OP_BIT_NOT);
				break;
			case '|': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_OR_EQ, 2);
				else _eat_token(Token::OP_OR);
				break;
			}
			case '&': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_AND_EQ, 2);
				else _eat_token(Token::OP_AND);
				break;
			}
			case '^': {
				if (GET_CHAR(1) == '=') _eat_token(Token::OP_XOR_EQ, 2);
				else _eat_token(Token::OP_XOR);
				break;
			}

			// identifier





		}

	}
	_eat_eof();

}

}