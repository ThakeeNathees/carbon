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

#include "platform_windows.h"
#include "core/logger.h"

namespace carbon {

class LoggerWindows :public Logger {

protected:
	virtual void log_impl(const char* p_msg, Console::Color p_fg, Console::Color p_bg) const override { log(p_msg, false, p_fg, p_bg); }

	virtual void log_verbose_impl(const char* p_msg) const override { log(p_msg, false); }
	virtual void log_info_impl(const char* p_msg)    const override { log(p_msg, false, Console::Color::L_WHITE); }
	virtual void log_success_impl(const char* p_msg) const override { log(p_msg, false, Console::Color::L_GREEN); }
	virtual void log_warning_impl(const char* p_msg) const override { log(p_msg, true, Console::Color::D_YELLOW); }
	virtual void log_error_impl(const char* p_msg)   const override { log(p_msg, true, Console::Color::D_RED); }

	virtual void logf_verbose_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, false); }
	virtual void logf_info_impl(const char* p_fmt, va_list p_list)    const override { logf(p_fmt, p_list, false, Console::Color::L_WHITE); }
	virtual void logf_success_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, false, Console::Color::L_GREEN); }
	virtual void logf_warning_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, true, Console::Color::D_YELLOW); }
	virtual void logf_error_impl(const char* p_fmt, va_list p_list)   const override { logf(p_fmt, p_list, true, Console::Color::D_RED); }

public:
	
	static void log(const char* p_message, bool p_err, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT) {
		if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
		if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
		Console::singleton()->set_console_color(p_forground, p_background);
		if (p_err) {
			fprintf(stderr, p_message);
		} else {
			fprintf(stdout, p_message);
		}
		Console::singleton()->set_console_color(Console::Color::L_WHITE);
	}

	static void logf(const char* p_fmt, va_list p_args, bool p_err, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT) {
		if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
		if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
		static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		Console::singleton()->set_console_color(p_forground, p_background);
		if (p_err) fprintf(stderr, "%s", buf);
		else fprintf(stdout, "%s", buf);
		Console::singleton()->set_console_color(Console::Color::L_WHITE);
	}
};

Logger* Logger::singleton = nullptr;

void Logger::initialize() {
	singleton = new LoggerWindows();
}

void Logger::cleanup() {
	delete singleton;
}

}