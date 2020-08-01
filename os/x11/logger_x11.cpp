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
#include "io/logger.h"

namespace carbon {


class LoggerX11 :public Logger
{
private:

protected:
	virtual void log_impl(const char* p_msg, Color p_fg, Color p_bg) const override {
		log(p_msg, false, p_fg, p_bg);
	}
	virtual void set_cursor_impl(int p_line, int p_column) const override {
		printf("\033[%d;%dH", p_column + 1, p_line + 1);
	}

	virtual void log_verbose_impl(const char* p_msg) const override { log(p_msg, false); }
	virtual void log_info_impl(const char* p_msg)    const override { log(p_msg, false, Color::L_WHITE); }
	virtual void log_success_impl(const char* p_msg) const override { log(p_msg, false, Color::D_GREEN); }
	virtual void log_warning_impl(const char* p_msg) const override { log(p_msg, true, Color::D_YELLOW); }
	virtual void log_error_impl(const char* p_msg)   const override { log(p_msg, true, Color::D_RED); }

	virtual void logf_verbose_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, false); }
	virtual void logf_info_impl(const char* p_fmt, va_list p_list)    const override { logf(p_fmt, p_list, false, Color::L_WHITE); }
	virtual void logf_success_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, false, Color::D_GREEN); }
	virtual void logf_warning_impl(const char* p_fmt, va_list p_list) const override { logf(p_fmt, p_list, true, Color::D_YELLOW); }
	virtual void logf_error_impl(const char* p_fmt, va_list p_list)   const override { logf(p_fmt, p_list, true, Color::D_RED); }


	enum ANSI_Codes {
		FG_OFFSET      = 0,
		BG_OFFSET      = 10, // fg, bg offset = 10.

		FMT_DEFAULT    = 0,
		FMT_BOLD       = 1,
		FMT_DIM        = 2,
		FMT_UNDERLINED = 3,
		FMT_BLINK      = 5,
		FMT_REVERSE    = 7,
		FMT_HIDDEN     = 8,

		DEFAULT        = 39,

		BLACK          = 30,
		RED            = 31,
		GREEN          = 32,
		YELLOW         = 33,
		BLUE           = 34,
		MAGENTA        = 35,
		WHITE          = 37,
		GRAY           = 90,
		BRIGHT_RED     = 91,
		BRIGHT_GREEN   = 92,
		BRIGHT_YELLOW  = 93,
		BRIGHT_BLUE    = 94,
		BRIGHT_MEGENTA = 95,
		CYAN           = 96,
		BRIGHT_WHITE   = 97,
	};

public:

	static void log(const char* p_msg, bool p_err, Color p_fg = Color::DEFAULT, Color p_bg = Color::DEFAULT) {

#define MAP_COLOR(m_target, m_offset)                                                    \
	switch (p_##m_target) {                                                              \
		case Color::DEFAULT:   m_target = ANSI_Codes::DEFAULT + m_offset;        break;  \
		case Color::BLACK:     m_target = ANSI_Codes::BLACK + m_offset;          break;  \
		case Color::L_BLUE:    m_target = ANSI_Codes::BLUE + m_offset;           break;  \
		case Color::L_GREEN:   m_target = ANSI_Codes::GREEN + m_offset;          break;  \
		case Color::L_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Color::L_RED:     m_target = ANSI_Codes::RED + m_offset;            break;  \
		case Color::L_PINK:    m_target = ANSI_Codes::MAGENTA + m_offset;        break;  \
		case Color::L_YELLOW:  m_target = ANSI_Codes::YELLOW + m_offset;         break;  \
		case Color::L_WHITE:   m_target = ANSI_Codes::WHITE + m_offset;          break;  \
		case Color::L_GRAY:    m_target = ANSI_Codes::GRAY + m_offset;           break;  \
		case Color::D_BLUE:    m_target = ANSI_Codes::BRIGHT_BLUE + m_offset;    break;  \
		case Color::D_GREEN:   m_target = ANSI_Codes::BRIGHT_GREEN + m_offset;   break;  \
		case Color::D_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Color::D_RED:     m_target = ANSI_Codes::BRIGHT_RED + m_offset;     break;  \
		case Color::D_PINK:    m_target = ANSI_Codes::BRIGHT_MEGENTA + m_offset; break;  \
		case Color::D_YELLOW:  m_target = ANSI_Codes::BRIGHT_YELLOW + m_offset;  break;  \
		case Color::D_WHITE:   m_target = ANSI_Codes::BRIGHT_WHITE + m_offset;   break;  \
	}
	MISSED_ENUM_CHECK(Color::__COLOR_MAX__, 16);

		int fg = ANSI_Codes::WHITE + ANSI_Codes::FG_OFFSET;
		int bg = ANSI_Codes::BLACK + ANSI_Codes::BG_OFFSET;
		MAP_COLOR(fg, ANSI_Codes::FG_OFFSET);
		MAP_COLOR(bg, ANSI_Codes::BG_OFFSET);
		if (!p_err) {
			fprintf(stdout, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, fg, bg, p_msg, ANSI_Codes::FMT_DEFAULT);
		} else {
			fprintf(stderr, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, fg, bg, p_msg, ANSI_Codes::FMT_DEFAULT);
		}
	}

	static void logf(const char* p_fmt, va_list p_args, bool p_err, Color p_fg = Color::DEFAULT, Color p_bg = Color::DEFAULT) {
		static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		log(buf, p_err, p_fg, p_bg);
	}
};

ptr<Logger> Logger::singleton = newptr<LoggerX11>();

}