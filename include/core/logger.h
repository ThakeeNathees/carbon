//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

#include <stdarg.h> // for va_list
#include "console.h"

// TODO: implement log to file, redirect error logs, ...

namespace carbon {

class Logger {
public:
	enum LogLevel 
	{
		JUST_LOG = 0,

		VERBOSE = 1,
		INFO    = 2,
		SUCCESS = 3,
		WARNING = 4,
		ERROR   = 5,
	};

	static void initialize();
	static void cleanup();

	static void set_level(LogLevel p_level);
	static void reset_level();
	static LogLevel get_level();
	static bool is_level(LogLevel p_level);
	
	static void log(const char* p_msg, Console::Color p_fg = Console::Color::L_WHITE, Console::Color p_bg = Console::Color::BLACK);
	static void log(const char* p_msg, LogLevel p_level, Console::Color p_fg = Console::Color::L_WHITE, Console::Color p_bg = Console::Color::BLACK);

	static void log_verbose(const char* p_msg);
	static void log_info(const char* p_msg);
	static void log_success(const char* p_msg);
	static void log_warning(const char* p_msg);
	static void log_error(const char* p_msg);

	static void logf_verbose(const char* p_fmt, ...);
	static void logf_info(const char* p_fmt, ...);
	static void logf_success(const char* p_fmt, ...);
	static void logf_warning(const char* p_fmt, ...);
	static void logf_error(const char* p_fmt, ...);

private:
	static LogLevel level;
	static LogLevel last_level;

	static void _log(const char* p_message, bool p_stderr, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void _logf(const char* p_fmt, va_list p_args, bool p_err, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
};

}

#endif // LOGGER_H