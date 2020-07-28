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

#include "core.h"
#include "io/console_logger.h"
#include <Windows.h>

namespace carbon {


class ConsoleLoggerWindows :public ConsoleLogger
{
private:
	static HANDLE h_console;

protected:
	virtual void log_impl(const char* p_msg) {
		log(p_msg, false);
	}
	virtual void log_info_impl(const char* p_msg) {
		log(p_msg, false, ConsoleColor::D_WHITE);
	}
	virtual void log_warning_impl(const char* p_msg) {
		log(p_msg, true, ConsoleColor::D_YELLOW);
	}
	virtual void log_error_impl(const char* p_msg) {
		log(p_msg, true, ConsoleColor::D_RED);
	}

	virtual void logf_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, false);
	}
	virtual void logf_info_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, false, ConsoleColor::D_WHITE);
	}
	virtual void logf_warning_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, true, ConsoleColor::D_YELLOW);
	}
	virtual void logf_error_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, true, ConsoleColor::D_RED);
	}

public:
	enum class ConsoleColor {
		BLACK = 0,
		L_BLUE = 1,
		L_GREEN = 2,
		L_SKYBLUE = 3,
		L_RED = 4,
		L_PINK = 5,
		L_YELLOW = 6,
		L_WHITE = 7,
		L_GRAY = 8,

		D_BLUE = 9,
		D_GREEN = 10,
		D_SKYBLUE = 11,
		D_RED = 12,
		D_PINK = 13,
		D_YELLOW = 14,
		D_WHITE = 15,
	};
	static void _set_console_color(ConsoleColor p_forground, ConsoleColor p_background = ConsoleColor::BLACK) {
			SetConsoleTextAttribute(h_console, (int)p_background << 4 | (int)p_forground);
	}
	static void log(const char* p_message, bool p_err, 
			ConsoleColor p_forground = ConsoleColor::L_WHITE, ConsoleColor p_background = ConsoleColor::BLACK) {
		_set_console_color(p_forground, p_background);
		if (p_err) {
			fprintf(stderr, p_message);
		} else {
			fprintf(stdout, p_message);
		}
		_set_console_color(ConsoleColor::D_WHITE);
	}

	static void logf(const char* p_fmt, va_list p_args, bool p_err,
			ConsoleColor p_forground = ConsoleColor::L_WHITE, ConsoleColor p_background = ConsoleColor::BLACK) {
		static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		_set_console_color(p_forground, p_background);
		if (p_err) fprintf(stderr, "%s", buf);
		else fprintf(stdout, "%s", buf);
		_set_console_color(ConsoleColor::D_WHITE);
	}
};

HANDLE ConsoleLoggerWindows::h_console = GetStdHandle(STD_OUTPUT_HANDLE);
ptr<ConsoleLogger> ConsoleLogger::singleton = newptr<ConsoleLoggerWindows>();

}