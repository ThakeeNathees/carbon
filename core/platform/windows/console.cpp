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
#include "core/console.h"

// console input handle reference: https://docs.microsoft.com/en-us/windows/console/reading-input-buffer-events

namespace carbon {

class ConsoleWindows : public Console {
private:
	HANDLE hStdout;
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	DWORD cNumRead, fdwMode, dispatchInd;
	INPUT_RECORD irInBuf[128];

	//VOID ErrorReset(LPSTR);
	//VOID KeyEventProc(KEY_EVENT_RECORD);
	//VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

public:
	static ConsoleWindows* singleton() {
		if (_singleton == nullptr) _singleton = new ConsoleWindows();
		return _singleton;
	}
	static void initialize() {}
	static void cleanup() {
		delete _singleton;
	}

	void* get_handle() const override {
		return hStdout;
	}

	void get_console_size(int* p_columns, int* p_rows) const override {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hStdout, &csbi);
		*p_columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		*p_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}

	void set_cursor(int p_line, int p_column) const override {
		COORD coord;
		coord.X = p_line;
		coord.Y = p_column;

		if (!SetConsoleCursorPosition(hStdout, coord)) {
			_error_reset(_get_last_error_as_string());
		}
	}

	void get_cursor(int* p_line, int* p_column) const override {
		CONSOLE_SCREEN_BUFFER_INFO cbsi;
		COORD pos;
		if (GetConsoleScreenBufferInfo(hStdout, &cbsi)) {
			pos = cbsi.dwCursorPosition;
		} else {
			_error_reset(_get_last_error_as_string());
			return;
		}
		*p_line = pos.X;
		*p_column = pos.Y;
	}

	void set_console_color(Color p_forground, Color p_background = Color::DEFAULT) const override {
		if (p_background == Color::DEFAULT) p_background = Color::BLACK;
		SetConsoleTextAttribute(hStdout, (int)p_background << 4 | (int)p_forground);
	}

	void enable_input_mode() override {
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) _error_reset("GetConsoleMode");
	}

	void listen_input_event() override {
		if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            _error_reset("ReadConsoleInput");
		dispatchInd = 0;
	}

	bool get_input_event(KeyEvent& p_event) override {
		do {
			if (irInBuf[dispatchInd].EventType == KEY_EVENT) break;
			dispatchInd++;
			if (dispatchInd >= cNumRead) return false;
		} while (true);
		p_event.is_down = irInBuf[dispatchInd].Event.KeyEvent.bKeyDown;
		p_event.keycode = irInBuf[dispatchInd].Event.KeyEvent.wVirtualKeyCode;
		p_event.repeat_count = irInBuf[dispatchInd].Event.KeyEvent.wRepeatCount;
		p_event.ascii = irInBuf[dispatchInd].Event.KeyEvent.uChar.AsciiChar;
		p_event.ctrl_key_state = irInBuf[dispatchInd].Event.KeyEvent.dwControlKeyState;
		dispatchInd++;
		return true;
	}

private:
	ConsoleWindows() {
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		if (hStdin == INVALID_HANDLE_VALUE) _error_reset("GetStdHandle");
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) _error_reset("GetConsoleMode");
	}

	void _error_reset(std::string msg) const {
		fprintf(stderr, "%s\n", msg.c_str());
		SetConsoleMode(hStdin, fdwSaveOldMode);
	}

	std::string _get_last_error_as_string() const {
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0) {
			return ""; //No error message has been recorded
		} else {
			return std::system_category().message(errorMessageID);
		}
	}

	static ConsoleWindows* _singleton;
};

// --------------------------------------------------------------

ConsoleWindows* ConsoleWindows::_singleton = nullptr;

Console* Console::singleton() {
	return ConsoleWindows::singleton();
}

void Console::initialize() {
}

void Console::cleanup() {
	ConsoleWindows::cleanup();
}

}
