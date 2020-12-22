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

#include "core/console.h"
#include "core/platform.h"

namespace carbon {

void Console::initialize() {}
void Console::cleanup() {}

void Console::set_cursor(int p_line, int p_column) {
	_Platform::console_set_cursor(p_line, p_column);
}

void Console::get_cursor(int* p_line, int* p_column) {
	_Platform::console_get_cursor(p_line, p_column);
}

std::string Console::getline() {
	std::string ret;
	std::getline(std::cin, ret);
	return ret;
}

void Console::logf(const char* p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	vprintf(p_fmt, args);
	va_end(args);
}

void Console::logf_stderr(const char* p_fmt, ...) {
	va_list args;
	va_start(args, p_fmt);
	vfprintf(stderr, p_fmt, args);
	va_end(args);
}

void Console::logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	_Platform::console_logf(p_fmt, p_args, p_stderr, p_forground, p_background);
}

void Console::log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	_Platform::console_log(p_message, p_stderr, p_forground, p_background);
}

}
