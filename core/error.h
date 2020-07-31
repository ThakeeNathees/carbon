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

#ifndef ERRORS_H
#define ERRORS_H


#define ARG_1 (_1,...) _1
#define ARG_2 (_1,_2,...) _2
#define ARG_3 (_1,_2,_3,...) _3
#define ARG_4 (_1,_2,_3,_4,...) _4
#define ARG_5 (_1,_2,_3,_4,_5,...) _5
#define ARG_6 (_1,_2,_3,_4,_5,_6,...) _6
#define ARG_7 (_1,_2,_3,_4,_5,_6,_7,...) _7
#define ARG_8 (_1,_2,_3,_4,_5,_6,_7,_8,...) _8
#define ARG_9 (_1,_2,_3,_4,_5,_6,_7,_8,_9,...) _9
#define ARG_10(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,...) _10

#define STRCAT2(m_1, m_2) m_1##m_2
#define STRCAT3(m_1, m_2, m_3) m_1##m_2##m_3
#define STRCAT4(m_1, m_2, m_3, m_4) m_1##m_2##m_3##m_4
#define STRCAT5(m_1, m_2, m_3, m_4, m_5) m_1##m_2##m_3##m_4##m_5

#if DEBUG_BUILD
#define _ERR_ADD_DBG_VARS .set_deg_variables(__FUNCTION__, __FILE__, __LINE__)
#else
#define _ERR_ADD_DBG_VARS 
#endif

#define THROW_INVALID_INDEX(m_size, m_ind)                                                            \
if (m_ind < 0 || m_size <= m_ind) {                                                                   \
	throw Error(Error::INVALID_INDEX, String::format("Index %s = %lli is out of bounds (%s = %lli)",  \
		STRINGIFY(m_size), m_size, STRINGIFY(m_ind), m_ind))_ERR_ADD_DBG_VARS;                        \
} else ((void)0)

#define THROW_IF_NULLPTR(m_ptr)                                                                       \
if (m_ptr == nullptr){                                                                                \
	throw Error(Error::NULL_POINTER, String::format("The pointer \"%s\" is null", STRINGIFY(m_ptr)))  \
	_ERR_ADD_DBG_VARS;                                                                                \
} else ((void)0)

#define THROW_INTERNAL(m_type, m_msg) throw Error(m_type, m_msg)_ERR_ADD_DBG_VARS

#include "var.h/_var.h"
using namespace varh;

namespace carbon {

class Error : public std::exception {
public:
	enum Type {
		OK = 0,

		// Compiletime errors.
		SYNTAX_ERROR,
		UNEXPECTED_EOF,
		ALREADY_DEFINED,

		// Runtime errors.
		NULL_POINTER,
		INVALID_INDEX,
		INVALID_CASTING,
		NOT_IMPLEMENTED,
		ZERO_DIVISION,
		INVALID_ARGUMENT,
		INVALID_ARG_COUNT,

		IO_ERROR,
		CANT_OPEN_FILE,
		IO_INVALID_OPERATORN,

		// for debugging
		INTERNAL_BUG,

	};

	const char* what() const noexcept override { return msg.c_str(); }

	Type get_type() const { return type; }
	Vect2i get_pos() const { return pos; }

	String get_file() const noexcept { return file.c_str(); }
	String get_line() const { // TODO: refactor line with line.rstrip("\n") when setting it.
		if (line[line.size() - 1] == '\n') {
			return line.substr(0, line.size() - 1);
		}
		return line;
	}
	String get_line_pos() const {
		std::stringstream ss_pos;
		size_t cur_col = 0;
		for (size_t i = 0; i < line.size(); i++) {
			cur_col++;
			if (cur_col == pos.y) {
				for (uint32_t i = 0; i < err_len; i++) {
					ss_pos << '^';
				}
				break;
			} else if (line[i] != '\t') {
				ss_pos << ' ';
			} else {
				ss_pos << '\t';
			}
		}
		//return String(line + ((line[line.size()-1] != '\n')? "\n" : "") + ss_pos.str());
		return ss_pos.str();
	}

#if DEBUG_BUILD
	const String& get_dbg_func() const { return __dbg_func__; }
	const String& get_dbg_file() const { return __dbg_file__; }
	int get_dbg_line() const { return __dbg_line__; }
#endif

	Error() {}
	Error(Type p_type) { type = p_type; }
	Error(Type p_type, const String& p_msg) { type = p_type; msg = p_msg; }

	Error(Type p_type, const String& p_msg, const Vect2i p_pos, uint32_t p_err_len = 1) { 
		type = p_type; msg = p_msg; pos = p_pos; err_len = p_err_len; 
	}
	Error(Type p_type, const String& p_msg, const String& p_file, const String& p_line, const Vect2i p_pos, uint32_t p_err_len = 1) {
		type = p_type; msg = p_msg; file = p_file; line = p_line; pos = p_pos; err_len = p_err_len;
	}

	Error& set_file(const String& p_file) { file = p_file;    return *this; }
	Error& set_line(const String& p_line) { line = p_line;    return *this; }
	Error& set_pos(const Vect2i& p_pos)   { pos = p_pos;      return *this; }
	Error& set_err_len(uint32_t p_len)    { err_len = p_len;  return *this; }

#if DEBUG_BUILD
	Error& set_deg_variables(const String& p_func, const String& p_file, uint32_t p_line) {
		__dbg_func__ = p_func;
		__dbg_file__ = p_file;
		__dbg_line__ = p_line;
		return *this;
	}
#endif

private:
	Type type = OK;
	String msg = "<NO-ERROR-MSG-SET>";
	String file = "<NO-FILE-SET>";
	String line = "<NO-LINE-SET>";
	Vect2i pos = Vect2i(-1, -1);
	uint32_t err_len = 1;

#if DEBUG_BUILD
	String __dbg_func__ = "<NO-FUNC-SET>";
	String __dbg_file__ = "<NO-FILE-SET>";
	uint32_t __dbg_line__ = 0;
#endif
};

}

#endif // ERRORS_H
