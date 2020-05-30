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

#ifndef LOGGER_H
#define LOGGER_H

#include "core.h"

namespace carbon {

class ConsoleLogger
{
private:
	static Ptr<ConsoleLogger> singleton;
protected:
	virtual void log_impl(const char* p_msg) = 0;
	virtual void log_info_impl(const char* p_msg) = 0;
	virtual void log_warning_impl(const char* p_msg) = 0;
	virtual void log_error_impl(const char* p_msg) = 0;

	virtual void logf_impl(const char* p_fmt, va_list p_list) = 0;
	virtual void logf_info_impl(const char* p_fmt, va_list p_list) = 0;
	virtual void logf_warning_impl(const char* p_fmt, va_list p_list) = 0;
	virtual void logf_error_impl(const char* p_fmt, va_list p_list) = 0;

public:
	enum class LogLevel 
	{
		VERBOSE,
		INFO,
		WARNING,
		ERROR,
	};
	static void set_level(LogLevel p_level) { level = p_level; }
	static LogLevel set_level() { return level; }
	
	static void log(const char* p_msg) { singleton->log_impl(p_msg); }
	static void log_info(const char* p_msg) { singleton->log_info_impl(p_msg); }
	static void log_warning(const char* p_msg) { singleton->log_warning_impl(p_msg); }
	static void log_error(const char* p_msg) { singleton->log_error_impl(p_msg); }

#define LOG_METHODS(func)                               \
static void func(const char* p_fmt, ...) {              \
	va_list args;                                       \
	va_start(args, p_fmt);                              \
	singleton->STRCAT2(func, _impl) (p_fmt, args);      \
	va_end(args);                                       \
}
	LOG_METHODS(logf)
	LOG_METHODS(logf_info)
	LOG_METHODS(logf_warning)
	LOG_METHODS(logf_error)

private:
	static LogLevel level;
};

}

#endif // LOGGER_H