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

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "core.h"
#include "builtin_functions.h"
#include "builtin_types.h"

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
	OP_LT,
	OP_LTEQ,
	OP_GT,
	OP_GTEQ,
	OP_AND,
	OP_OR,
	OP_NOT,
	OP_NOTEQ,

	OP_BIT_NOT,
	OP_BIT_LSHIFT,
	OP_BIT_LSHIFT_EQ,
	OP_BIT_RSHIFT,
	OP_BIT_RSHIFT_EQ,
	OP_BIT_OR,
	OP_BIT_OR_EQ,
	OP_BIT_AND,
	OP_BIT_AND_EQ,
	OP_BIT_XOR,
	OP_BIT_XOR_EQ,

	IDENTIFIER,
	BUILTIN_FUNC, // also identifier

	KWORD_IMPORT,
	KWORD_CLASS,
	KWORD_ENUM,
	KWORD_FUNC,
	KWORD_VAR,
	KWORD_NULL,
	KWORD_TRUE,
	KWORD_FALSE,
	KWORD_IF,
	KWORD_ELSE,
	KWORD_WHILE,
	//KWORD_FOR, 
/*  var i = 0; while(i < 10) { i += 1;
		print(i);
	} 
*/
	KWORD_SWITCH,
	KWORD_BREAK,
	KWORD_CONTINUE,
	KWORD_STATIC,
	KWORD_THIS,
	KWORD_SUPER,
	KWORD_RETURN,

	VALUE_STRING,
	VALUE_INT,
	VALUE_FLOAT,
	_TK_MAX_,
};

struct TokenData {
	Token type = Token::UNKNOWN;
	int line = 0, col = 0;
	
	// Constant int, float, string values.
	var constant;

	// Identifiers.
	String identifier;
	BuiltinFunctions::Type builtin_func = BuiltinFunctions::Type::UNKNOWN;
	BuiltinTypes::Type builtin_class = BuiltinTypes::Type::_NULL;
	var::Type biltin_type = var::Type::_NULL;

	String to_string() const;
};


class Tokenizer {
public:
	// Methods.
	const void tokenize(const String& p_source);

	const TokenData& next(int p_offset = 0) { 
		if (token_ptr + p_offset >= (int)tokens.size()) { throw Error(Error::INVALID_INDEX); }
		token_ptr += p_offset;
		tk_last = &tokens[token_ptr++];
		cur_line = tk_last->line; cur_col = tk_last->col;
		return *tk_last;
	}
	const TokenData& peek(int p_offset = 0) {
		if (token_ptr + p_offset >= (int)tokens.size()) { throw Error(Error::INVALID_INDEX); }
		tk_last = &tokens[token_ptr+p_offset];
		return *tk_last;
	}

	const TokenData& last() const {
		if (tk_last == nullptr) throw Error(Error::INTERNAL_BUG, "The pointer tk_last was nullptr.");
		return *tk_last;
	}

	int get_line() const { return cur_line; }
	int get_col() const { return cur_col; }

	static const char* get_token_name(Token p_tk);

protected:

private:
	// Methods.
	void _eat_escape(String& p_str);
	void _eat_token(Token p_tk, int p_eat_size = 1);
	void _eat_eof();
	void _eat_const_value(const var& p_value, int p_eat_size = 0);
	void _eat_identifier(const String& p_idf, int p_eat_size = 0);

	// Members.
	String source;
	stdvec<TokenData> tokens;
	TokenData* tk_last = nullptr; // Last seen token from next() or peek()
	int cur_line = 1, cur_col = 1;
	int char_ptr = 0;
	int token_ptr = 0; // for next()

};

}

#endif // TOKENIZER_H