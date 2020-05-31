//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c), 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),, to deal
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

#ifndef TOKENS_H
#define TOKENS_H

#include "core.h"
#include "builtin_functions.h"

namespace carbon {

enum class Token {
	UNKNOWN,
	_EOF, // EOF already a macro in <stdio.h>
	SYM_DOT,
	SYM_COMMA,
	SYM_COLLON,
	SYM_SEMI_COLLON,
	SYM_AT,
	SYM_HASH,
	SYM_DOLLAR,
	SYM_QUESTION,
	// SYM_DQUOTE, // Token string value
	// SYM_SQUOTE, // Token string value
	BRACKET_LPARAN,
	BRACKET_RPARAN,
	BRACKET_LCUR,
	BRACKET_RCUR,
	BRACKET_RSQ,
	BRACKET_LSQ,
	OP_EQ,
	OP_EQEQ,
	OP_PLUS,
	OP_PLUSEQ,
	OP_MINUS,
	OP_MINUSEQ,
	OP_MUL,
	OP_MULEQ,
	OP_DIV,
	OP_DIVEQ,
	OP_MOD,
	OP_MOD_EQ,
	OP_NOT,
	OP_NOTEQ,
	OP_LT,
	OP_LTEQ,
	OP_GT,
	OP_GTEQ,
	OP_BIT_NOT,
	OP_LSHIFT,
	OP_LSHIFT_EQ,
	OP_RSHIFT,
	OP_RSHIFT_EQ,
	OP_OR,
	OP_OR_EQ,
	OP_AND,
	OP_AND_EQ,
	OP_XOR,
	OP_XOR_EQ,
	IDENTIFIER,
	KWORD_NULL,
	KWORD_VAR,
	KWORD_TRUE,
	KWORD_FALSE,
	KWORD_IF,
	KWORD_ELSE,
	KWORD_WHILE,
	KWORD_FOR,
	KWORD_FOREACH,
	KWORD_BREAK,
	KWORD_CONTINUE,
	KWORD_AND,
	KWORD_OR,
	KWORD_NOT,
	KWORD_RETURN,
	KWORD_FUNC,
	KWORD_STRUCT,
	KWORD_IMPORT,
	VALUE_STRING,
	VALUE_INT,
	VALUE_FLOAT,
	_TK_MAX_,
};

struct TokenData
{
	Token type = Token::UNKNOWN;
	String identifier; // for identifier
	var constant;      // for constant int, float, string values
	BuiltinFunctions::Function builtin_func;
	int line = 0, col = 0;

	static const char* get_token_name(Token p_tk) {
		return ""; // TODO:
		//static const char* token_names[] = {
		//};
		//return token_names[(int)p_tk];
	}

};

}

#endif // TOKENS_H
