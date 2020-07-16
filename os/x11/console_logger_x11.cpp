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

namespace carbon {


class ConsoleLoggerX11 :public ConsoleLogger
{
private:

protected:
	virtual void log_impl(const char* p_msg) {
		log(p_msg, false);
	}
	virtual void log_info_impl(const char* p_msg) {
		log(p_msg, false, ColorCode::FG_DEFAULT);
	}
	virtual void log_warning_impl(const char* p_msg) {
		log(p_msg, true, ColorCode::FG_YELLOW);
	}
	virtual void log_error_impl(const char* p_msg) {
		log(p_msg, true, ColorCode::FG_RED);
	}

	virtual void logf_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, false);
	}
	virtual void logf_info_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, false, ColorCode::FG_DEFAULT);
	}
	virtual void logf_warning_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, true, ColorCode::FG_YELLOW);
	}
	virtual void logf_error_impl(const char* p_fmt, va_list p_list) {
		logf(p_fmt, p_list, true, ColorCode::FG_RED);
	}

public:
	enum ColorCode {
		FMT_DEFAULT      = 0,
		FMT_BOLD         = 1,
		FMT_DIM          = 2,
		FMT_UNDERLINED   = 3,
		FMT_BLINK        = 5,
		FMT_REVERSE      = 7,
		FMT_HIDDEN       = 8,

		FG_RED     = 31,
		FG_GREEN   = 32,
		FG_YELLOW  = 33,
		FG_BLUE	   = 34,
		FG_DEFAULT = 39,

		BG_RED	   = 41,
		BG_GREEN   = 42,
		BG_BLUE	   = 44,
		BG_YELLOW  = 43,
		BG_DEFAULT = 49,
	};

	static void log(const char* p_msg, bool p_err,
			ColorCode p_fg = ColorCode::FG_DEFAULT, ColorCode p_bg = ColorCode::BG_DEFAULT, ColorCode p_fmt = ColorCode::FMT_DEFAULT) {
		if (!p_err) {
			fprintf(stdout, "\033[%i;%i;%im%s\033[%im", p_fmt, p_fg, p_bg, p_msg, ColorCode::FMT_DEFAULT);
		} else {
			fprintf(stderr, "\033[%i;%i;%im%s\033[%im", p_fmt, p_fg, p_bg, p_msg, ColorCode::FMT_DEFAULT);
		}
	}

	static void logf(const char* p_fmt, va_list p_args, bool p_err,
			ColorCode p_fg = ColorCode::FG_DEFAULT, ColorCode p_bg = ColorCode::BG_DEFAULT, ColorCode p_cfmt = ColorCode::FMT_DEFAULT) {
		static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		log(buf, p_err, p_fg, p_bg, p_cfmt);
	}
};

ptr<ConsoleLogger> ConsoleLogger::singleton = newptr<ConsoleLoggerX11>();

}