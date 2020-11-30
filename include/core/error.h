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

#define _DBG_SOURCE DBGSourceInfo(__FILE__, __LINE__, __FUNCTION__)

#define THROW_INVALID_INDEX(m_size, m_ind)                                                              \
if (m_ind < 0 || m_size <= (size_t)m_ind) {                                                             \
	throw Error(Error::INVALID_INDEX, String::format("index %s = %lli is out of bounds (%s = %lli).",   \
				STRINGIFY(m_size), m_size, STRINGIFY(m_ind), m_ind),                                    \
		_DBG_SOURCE);                                                                                   \
} else ((void)0)

#define THROW_IF_NULLPTR(m_ptr)                                                                           \
if (m_ptr == nullptr){                                                                                    \
	throw Error(Error::INVALID_INDEX, String::format("the pointer \"%s\" is null.", STRINGIFY(m_ptr)),    \
		_DBG_SOURCE);                                                                                     \
} else ((void)0)


#define THROW_ERROR(m_type, m_msg) throw Error(m_type, m_msg, _DBG_SOURCE)
#define THROW_BUG(m_msg) do{ DEBUG_BREAK(); THROW_ERROR(Error::BUG, m_msg); } while (false)

#include "var/var.h"

// TODO: create a common parent for warning and error first

namespace carbon {

struct DBGSourceInfo {
	String func;
	String file;
	uint32_t line = 0;

	String line_before;
	String line_str;
	String line_after;

	Vect2i pos;
	uint32_t width = 0;

	DBGSourceInfo();
	DBGSourceInfo(const String& p_file, uint32_t p_line, const String& p_func);
	DBGSourceInfo(const String& p_file, const String& p_line_str, Vect2i& p_pos, uint32_t p_width, const String& p_func = "");
	String get_pos_str() const;
};

class Throwable : public std::exception {
public:
	enum Kind {
		ERROR,
		COMPILE_TIME,
		RUN_TIME,
		WARNING,
	};

	enum Type {
		OK = 0,
		BUG,

		NULL_POINTER,
		OPERATOR_NOT_SUPPORTED,
		NOT_IMPLEMENTED,
		ZERO_DIVISION,
		TYPE_ERROR,
		ATTRIBUTE_ERROR,
		INVALID_ARG_COUNT,
		INVALID_INDEX,
		IO_ERROR,

		// compile time only errors
		SYNTAX_ERROR,
		ASSERTION,
		UNEXPECTED_EOF,
		NAME_ERROR,

		// warnings
		VARIABLE_SHADOWING,
		MISSED_ENUM_IN_SWITCH,
		NON_TERMINATING_LOOP,
		UNREACHABLE_CODE,
		STAND_ALONE_EXPRESSION,

		_ERROR_MAX_,
	};

	Throwable(Type p_type, const String& p_what = "", const DBGSourceInfo& p_source_info = DBGSourceInfo());
	static String get_err_name(Throwable::Type p_type);

	const char* what() const noexcept override { return _what.c_str(); }
	virtual Kind get_kind() const = 0;
	virtual void console_log() const = 0;

	void set_source_info(const DBGSourceInfo& p_source_info);
	Type get_type() const { return _type; }

protected:
	Type _type;
	String _what;
	DBGSourceInfo source_info;
};

// ---------------------------------------------------

class Error : public Throwable {
public:
	Error(Type p_type = BUG, const String& p_what = "", const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return ERROR; }
	void console_log() const override;
};

class CompileTimeError : public Throwable {
public:

	CompileTimeError(Type p_type, const String& p_what = "",
		const DBGSourceInfo& p_dbg_info = DBGSourceInfo(), const DBGSourceInfo& p_cb_dbg = DBGSourceInfo());
	virtual Kind get_kind() const { return COMPILE_TIME; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};

class Warning : public Throwable {
public:

	Warning(Type p_type, const String& p_what = "",
		const DBGSourceInfo& p_dbg_info = DBGSourceInfo(), const DBGSourceInfo& p_cb_dbg = DBGSourceInfo());
	virtual Kind get_kind() const { return WARNING; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
	
};


}

#endif // ERRORS_H
