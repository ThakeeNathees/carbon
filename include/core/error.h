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
#define THROW_BUG(m_msg)                                                                \
	do{																	                \
		Logger::log_error((String("BUG : ") + m_msg + " (please report)\n").c_str());	\
		DEBUG_BREAK();													                \
		THROW_ERROR(Error::BUG, m_msg);									                \
	} while (false)

#include "logger.h"
#include "var/internal.h"

namespace carbon {

struct DBGSourceInfo {
	std::string func;
	std::string file;
	uint32_t line = 0;

	std::string line_before;
	std::string line_str;
	std::string line_after;

	std::pair<int, int> pos;
	uint32_t width = 0;

	DBGSourceInfo();
	DBGSourceInfo(const std::string& p_file, uint32_t p_line, const std::string& p_func);
	DBGSourceInfo(const std::string& p_file, const std::string& p_line_str, std::pair<int, int> p_pos, uint32_t p_width, const std::string& p_func = "");
	std::string get_pos_str() const;
};

class Throwable : public std::exception {
public:
	enum Kind {
		ERROR,
		COMPILE_TIME,
		WARNING,
		TRACEBACK,
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

		// runtime error
		RETHROW,
		STACK_OVERFLOW,

		_ERROR_MAX_,
	};

	Throwable(Type p_type, const std::string& p_what, const DBGSourceInfo& p_source_info = DBGSourceInfo());
	static std::string get_err_name(Throwable::Type p_type);

	const char* what() const noexcept override { return _what.c_str(); }
	virtual Kind get_kind() const = 0;
	virtual void console_log() const = 0;

	void set_source_info(const DBGSourceInfo& p_source_info);
	Type get_type() const { return _type; }

	void _add_nested(ptr<Throwable> p_err) { _nested = p_err; }
	Throwable* _get_nested() const { return _nested.get(); }
	//void _set_owner(Throwable* p_owner) { _owner = p_owner; }
	//Throwable* _get_owner()  const { return _owner; }

protected:
	Type _type;
	std::string _what;
	DBGSourceInfo source_info;

	mutable ptr<Throwable> _nested;
	//mutable Throwable* _owner = nullptr;
};

// ---------------------------------------------------

class Error : public Throwable {
public:
	Error(Type p_type, const std::string& p_what, const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return ERROR; }
	void console_log() const override;
};

class CompileTimeError : public Throwable {
public:
	CompileTimeError(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return COMPILE_TIME; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};

class Warning : public Throwable {
public:
	Warning(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	virtual Kind get_kind() const { return WARNING; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};

class TraceBack : public Throwable {
public:
	TraceBack(Type p_type, const std::string& p_what,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());
	TraceBack(const ptr<Throwable> p_nested,
		const DBGSourceInfo& p_cb_info = DBGSourceInfo(), const DBGSourceInfo& p_dbg_info = DBGSourceInfo());

	virtual Kind get_kind() const { return TRACEBACK; }
	void console_log() const override;

private:
	DBGSourceInfo _cb_dbg_info;
};


}

#endif // ERRORS_H
