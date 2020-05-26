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

#define FOREACH_TOKEN_TYPE(func) \
	func(TK_UNKNOWN),    \
	func(TK_EOF),        \
	func(TK_IDENTIFIER),        \
	func(TK_SYM_DOT),           \
	func(TK_SYM_COMMA),         \
	func(TK_SYM_COLLON),        \
	func(TK_SYM_SEMI_COLLON),   \
	func(TK_SYM_DQUOTE),        \
	func(TK_SYM_SQUOTE),        \
	func(TK_SYM_AT),            \
	func(TK_SYM_HASH),          \
	func(TK_SYM_DOLLAR),        \
	func(TK_SYM_DILDO),         \
	func(TK_BRACKET_LPARAN),        \
	func(TK_BRACKET_RPARAN),        \
	func(TK_BRACKET_LCUR),          \
	func(TK_BRACKET_RCUR),          \
	func(TK_BRACKET_RSQ),           \
	func(TK_BRACKET_LSQ),           \
	func(TK_BRACKET_RTRI),          \
	func(TK_BRACKET_LTRI),          \
	func(TK_OP_EQ),        \
	func(TK_OP_PLUS),      \
	func(TK_OP_PLUSEQ),    \
	func(TK_OP_MINUS),     \
	func(TK_OP_MINUSEQ),   \
	func(TK_OP_MUL),       \
	func(TK_OP_MULEQ),     \
	func(TK_OP_DIV),       \
	func(TK_OP_DIVEQ),     \
	func(TK_OP_MOD),       \
	func(TK_OP_NOT),       \
	func(TK_OP_POW),       \
	func(TK_OP_EQEQ),      \
	func(TK_OP_NOTEQ),     \
	func(TK_OP_GT),        \
	func(TK_OP_LT),        \
	func(TK_OP_GTEQ),      \
	func(TK_OP_LTEQ),      \
	func(TK_OP_LSHIFT),    \
	func(TK_OP_RSHIFT),    \
	func(TK_OP_OR),        \
	func(TK_OP_AND),       \
	func(TK_OP_XOR),       \
	func(TK_OP_INCR),      \
	func(TK_OP_DECR),      \
	func(TK_KWORD_NULL),         \
	func(TK_KWORD_VAR),          \
	func(TK_KWORD_TRUE),         \
	func(TK_KWORD_FALSE),        \
	func(TK_KWORD_IF),           \
	func(TK_KWORD_ELSE),         \
	func(TK_KWORD_WHILE),        \
	func(TK_KWORD_FOR),          \
	func(TK_KWORD_FOREACH),      \
	func(TK_KWORD_BREAK),        \
	func(TK_KWORD_CONTINUE),     \
	func(TK_KWORD_AND),          \
	func(TK_KWORD_OR),           \
	func(TK_KWORD_NOT),          \
	func(TK_KWORD_RETURN),       \
	func(TK_KWORD_FUNC),         \
	func(TK_KWORD_STRUCT),       \
	func(TK_KWORD_IMPORT),       \
	func(TK_VALUE_STRING),    \
	func(TK_VALUE_INT),       \
	func(TK_VALUE_FLOAT),     \
	func(_TK_MAX_)

enum class Token {
	FOREACH_TOKEN_TYPE(NOEFFECT)
};

struct TokenData
{
	Token type = Token::TK_UNKNOWN;
	String identifier; // for identifier
	var constant; // for constants

	// TODO: builtin function ref
	int line = 0, col = 0;

	static const char* get_token_name(Token p_tk);

};

#endif // TOKENS_H
