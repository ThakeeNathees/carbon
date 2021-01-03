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

#include "internal.h"
#include "console.h"
#include "logger.h"

#ifndef _PLATFORM_H
#define _PLATFORM_H

namespace carbon {

class _Platform {

public:
	static void console_get_cursor(int* p_line, int* p_column);
	static void console_set_cursor(int p_line, int p_column);
	static void console_logf(const char* p_fmt, va_list p_args, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void console_log(const char* p_message, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);

	static std::string os_getcwd();
	static void os_chdir(const std::string& p_path);

	static std::string path_absolute(const std::string& p_path);
	static bool path_exists(const std::string& p_path);
	static bool path_isdir(const std::string& p_path);
};

}

#endif //_PLATFORM_H
