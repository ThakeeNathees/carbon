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
#include "builtin/builtin_functions.h"
#include "builtin/builtin_types.h"

#define THROW_TOKENIZE_ERROR(m_err_type, m_msg)                                                                      \
	do {                                                                                                             \
		uint32_t err_len = 1;                                                                                        \
		String token_str = peek(-1, true).to_string();                                                               \
		if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;      \
		else err_len = (uint32_t)token_str.size();                                                                   \
		throw Error(m_err_type, m_msg, source_path, source.get_line(cur_line), Vect2i(cur_line, cur_col), err_len)   \
		_ERR_ADD_DBG_VARS;                                                                                           \
	} while (false)


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
	//BUILTIN_FUNC, // also identifier
	BUILTIN_TYPE,  // bool, int, String, ... cant be identifiers
	//native type, // also identifier

	KWORD_IMPORT,
	KWORD_CLASS,
	KWORD_ENUM,
	KWORD_FUNC,
	KWORD_VAR,
	KWORD_CONST,
	KWORD_NULL,
	KWORD_TRUE,
	KWORD_FALSE,
	KWORD_IF,
	KWORD_ELSE,
	KWORD_WHILE,
	KWORD_FOR,
	KWORD_SWITCH,
	KWORD_CASE,
	KWORD_DEFAULT,
	KWORD_BREAK,
	KWORD_CONTINUE,
	KWORD_STATIC,
	KWORD_THIS,
	KWORD_SUPER,
	KWORD_RETURN,

	VALUE_STRING,
	VALUE_INT,
	VALUE_FLOAT,
	VALUE_BOOL,

	_TK_MAX_,
};

struct TokenData {
	Token type = Token::UNKNOWN;
	int line = 0, col = 0;
	
	var constant;
	String identifier;
	BuiltinTypes::Type builtin_type = BuiltinTypes::UNKNOWN;

	TokenData() {}
	TokenData(Token p_type) { type = p_type; }

	String to_string() const;
	Vect2i get_pos() const {  return Vect2i(line, col);  }
};


class Tokenizer {
public:
	// Methods.
	const void tokenize(const String& p_source, const String& p_source_path = "<PATH-NOT-SET>");

	const TokenData& next(int p_offset = 0) { 
		if (token_ptr + p_offset >= (int)tokens.size()) { THROW_BUG("invalid index."); }
		token_ptr += p_offset;
		cur_line = tokens[token_ptr].line; cur_col = tokens[token_ptr].col;
		return tokens[token_ptr++];
	}

	const TokenData& peek(int p_offset = 0, bool p_safe = false) const {
		static TokenData tmp = { Token::_EOF };
		if (token_ptr + p_offset  < 0 || token_ptr + p_offset >= (int)tokens.size()) {
			if (p_safe) return tmp;
			else THROW_TOKENIZE_ERROR(Error::INVALID_INDEX, "Internal Bug: TokenData::peek() index out of bounds");
		}
		return tokens[token_ptr + p_offset];
	}

	Vect2i get_pos() const { return Vect2i(cur_line, cur_col); }
	const TokenData& get_token_at(const Vect2i& p_pos) const {
		for (size_t i = 0; i < tokens.size(); i++) {
			if (tokens[i].line == p_pos.x && tokens[i].col == p_pos.y) {
				return tokens[i];
			}
		}
		THROW_TOKENIZE_ERROR(Error::BUG, "TokenData::get_token_at() called with invalid position.");
	}

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
	String source_path;
	stdvec<TokenData> tokens;

	int cur_line = 1, cur_col = 1;
	
	int char_ptr = 0;
	int token_ptr = 0; // for next()

	// The float .3 length must be 2 but constant.to_string() result a longer size
	// and it'll set wrong token column. here is a dirty way to prevent that.
	int __const_val_token_len = 0;

};

}

#endif // TOKENIZER_H