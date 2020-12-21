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

#ifndef CONSOLE_H
#define CONSOLE_H

#include "internal.h"

namespace carbon {

class Console {
public:
	enum class Color {
		DEFAULT = -1,
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

		__COLOR_MAX__,
	};

	static void initialize();
	static void cleanup();

	static void set_cursor(int p_line, int p_column);
	static void get_cursor(int* p_line, int* p_column);

	static std::string getline();

	static void logf(const char* p_fmt, ...);
	static void logf_stderr(const char* p_fmt, ...);
	static void logf(const char* p_fmt, va_list p_args, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);
	static void log(const char* p_message, bool p_stderr = false, Console::Color p_forground = Console::Color::DEFAULT, Console::Color p_background = Console::Color::DEFAULT);

private:

};


}

#endif // CONSOLE_H