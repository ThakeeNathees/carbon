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

#include "core/platform.h"
#include "var/var.h"

#ifdef PLATFORM_LINUX

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace carbon {

void _Platform::console_get_cursor(int* p_line, int* p_column) {
	THROW_BUG("TODO:");
}

void _Platform::console_set_cursor(int p_line, int p_column) {
	printf("\033[%d;%dH", p_column + 1, p_line + 1);
}

void _Platform::console_logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
		char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
		int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);
		
		if (len <= 0) return;
		if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
		buf[len] = 0;

		console_log(buf, p_stderr, p_forground, p_background);
}

void _Platform::console_log(const char* p_msg, bool p_stderr, Console::Color p_forground, Console::Color p_background) {

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

	#define MAP_COLOR(m_target, m_offset)                                                         \
	switch (p_##m_target) {                                                                       \
		case Console::Color::DEFAULT:   /*[[fallthrought]]*/                                      \
		case Console::Color::BLACK:     m_target = ANSI_Codes::DEFAULT + m_offset;        break;  \
		case Console::Color::L_BLUE:    m_target = ANSI_Codes::BLUE + m_offset;           break;  \
		case Console::Color::L_GREEN:   m_target = ANSI_Codes::GREEN + m_offset;          break;  \
		case Console::Color::L_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Console::Color::L_RED:     m_target = ANSI_Codes::RED + m_offset;            break;  \
		case Console::Color::L_PINK:    m_target = ANSI_Codes::MAGENTA + m_offset;        break;  \
		case Console::Color::L_YELLOW:  m_target = ANSI_Codes::YELLOW + m_offset;         break;  \
		case Console::Color::L_WHITE:   m_target = ANSI_Codes::WHITE + m_offset;          break;  \
		case Console::Color::L_GRAY:    m_target = ANSI_Codes::GRAY + m_offset;           break;  \
		case Console::Color::D_BLUE:    m_target = ANSI_Codes::BRIGHT_BLUE + m_offset;    break;  \
		case Console::Color::D_GREEN:   m_target = ANSI_Codes::BRIGHT_GREEN + m_offset;   break;  \
		case Console::Color::D_SKYBLUE: m_target = ANSI_Codes::CYAN + m_offset;           break;  \
		case Console::Color::D_RED:     m_target = ANSI_Codes::BRIGHT_RED + m_offset;     break;  \
		case Console::Color::D_PINK:    m_target = ANSI_Codes::BRIGHT_MEGENTA + m_offset; break;  \
		case Console::Color::D_YELLOW:  m_target = ANSI_Codes::BRIGHT_YELLOW + m_offset;  break;  \
		case Console::Color::D_WHITE:   m_target = ANSI_Codes::BRIGHT_WHITE + m_offset;   break;  \
	}
	MISSED_ENUM_CHECK(Console::Color::__COLOR_MAX__, 16);

		int forground = ANSI_Codes::WHITE + ANSI_Codes::FG_OFFSET;
		int background = ANSI_Codes::BLACK + ANSI_Codes::BG_OFFSET;
		MAP_COLOR(forground, ANSI_Codes::FG_OFFSET);
		MAP_COLOR(background, ANSI_Codes::BG_OFFSET);
		if (!p_stderr) {
			fprintf(stdout, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, forground, background, p_msg, ANSI_Codes::FMT_DEFAULT);
		} else {
			fprintf(stderr, "\033[%i;%i;%im%s\033[%im", ANSI_Codes::FMT_DEFAULT, forground, background, p_msg, ANSI_Codes::FMT_DEFAULT);
		}
}


std::string _Platform::os_getcwd() {
	char buffer[VSNPRINTF_BUFF_SIZE];
    if (getcwd(buffer, sizeof(buffer)) == NULL) {
        THROW_ERROR(Error::IO_ERROR, "getcwd() returned NULL.");
    }
    return buffer;
}

void _Platform::os_chdir(const std::string& p_path) {
	if (chdir(p_path.c_str()) != 0){
        THROW_ERROR(Error::IO_ERROR, "chdir() failed.");
    }
}

std::string _Platform::path_absolute(const std::string& p_path) {
	char* path = realpath(p_path.c_str(), NULL);
    String ret;
    if (path != NULL) {
        ret = path;
        free(path);
    }
    return ret;
}

bool _Platform::path_exists(const std::string& p_path) {
	struct stat _stat;
	return stat(p_path.c_str(), &_stat) == 0;
}

bool _Platform::path_isdir(const std::string& p_path) {
	struct stat _stat;
	if (stat(p_path.c_str(), &_stat) == 0 && (_stat.st_mode & S_IFMT == S_IFDIR))
        return true;
    return false;
}

} // namespace carbon

#endif // PLATFORM_WINDOWS