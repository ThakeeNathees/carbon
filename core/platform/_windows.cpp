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
#include "core/core.h"

#define NOMINMAX
#include <Windows.h>
#undef ERROR
#include <direct.h>


namespace carbon {

static void _set_console_color(Console::Color p_forground, Console::Color p_background = Console::Color::DEFAULT) {
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (int)p_background << 4 | (int)p_forground);
}

static std::string get_last_error_as_string() {
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return ""; //No error message has been recorded
	} else {
		return std::system_category().message(errorMessageID);
	}
}

// ------------------------------------------------------------------------

void _Platform::console_get_cursor(int* p_line, int* p_column) {
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	COORD pos;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi)) {
		pos = cbsi.dwCursorPosition;
	} else {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
	*p_line = pos.X;
	*p_column = pos.Y;
}

void _Platform::console_set_cursor(int p_line, int p_column) {
	COORD coord;
	coord.X = p_line;
	coord.Y = p_column;

	if (!SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord)) {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
}

void _Platform::console_logf(const char* p_fmt, va_list p_args, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	static const unsigned int BUFFER_SIZE = VSNPRINTF_BUFF_SIZE;
	char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
	int len = vsnprintf(buf, BUFFER_SIZE, p_fmt, p_args);

	if (len <= 0) return;
	if ((unsigned int)len >= BUFFER_SIZE) len = BUFFER_SIZE; // Output is too big, will be truncated
	buf[len] = 0;

	_set_console_color(p_forground, p_background);
	if (p_stderr) fprintf(stderr, "%s", buf);
	else fprintf(stdout, "%s", buf);
	_set_console_color(Console::Color::L_WHITE);
}

void _Platform::console_log(const char* p_message, bool p_stderr, Console::Color p_forground, Console::Color p_background) {
	if (p_forground == Console::Color::DEFAULT) p_forground = Console::Color::L_WHITE;
	if (p_background == Console::Color::DEFAULT) p_background = Console::Color::BLACK;
	_set_console_color(p_forground, p_background);
	if (p_stderr) {
		fprintf(stderr, p_message);
	} else {
		fprintf(stdout, p_message);
	}
	_set_console_color(Console::Color::L_WHITE);
}


std::string _Platform::os_getcwd() {
	char* buffer = _getcwd(NULL, 0);
	if (buffer == NULL) {
		THROW_ERROR(Error::IO_ERROR, "_getcwd returned NULL");
	}
	String cwd = buffer;
	free(buffer);
	return cwd;
}

void _Platform::os_chdir(const std::string& p_path) {
	if (_chdir(p_path.c_str())) {
		switch (errno) {
			case ENOENT:
				THROW_ERROR(Error::IO_ERROR, String::format("Unable to locate the directory: %s.", p_path.c_str()));
			case EINVAL:
				THROW_ERROR(Error::IO_ERROR, "Invalid buffer.");
			default:
				THROW_ERROR(Error::IO_ERROR, "Unknown error.");
		}
	}
}

std::string _Platform::path_absolute(const std::string& p_path) {
	char buffer[VSNPRINTF_BUFF_SIZE];
	if (GetFullPathNameA(p_path.c_str(), VSNPRINTF_BUFF_SIZE, buffer, NULL) == 0) {
		THROW_ERROR(Error::IO_ERROR, get_last_error_as_string());
	}
	return buffer;
}

bool _Platform::path_exists(const std::string& p_path) {
	DWORD ftyp = GetFileAttributesA(p_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return true;
}

bool _Platform::path_isdir(const std::string& p_path) {
	DWORD ftyp = GetFileAttributesA(p_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return false;									// this is not a directory!
}

}