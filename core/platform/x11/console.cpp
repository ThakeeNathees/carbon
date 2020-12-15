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

#include <stdio.h>
#include "core/console.h"

namespace carbon {

class ConsoleX11 : public Console {
private:

public:
	static ConsoleX11* singleton() {
		if (_singleton == nullptr) _singleton = new ConsoleX11();
		return _singleton;
	}
	static void initialize() {}
	static void cleanup() {
		delete _singleton;
	}

	void* get_handle() const override {
		return (void*)nullptr; // TODO:
	}

	void get_console_size(int* p_columns, int* p_rows) const override {
		//  TODO:
	}

	void set_cursor(int p_line, int p_column) const override {
		printf("\033[%d;%dH", p_column + 1, p_line + 1);
	}

	void get_cursor(int* p_line, int* p_column) const override {
		// TODO:
	}

	void set_console_color(Color p_forground, Color p_background = Color::DEFAULT) const override {
		// TODO: move from logger to here
	}

	void enable_input_mode() override {
        // TODO: remove this api
	}

	void listen_input_event() override {
		// TODO: remove this api
	}

	bool get_input_event(KeyEvent& p_event) override {
		// TODO: remove this api
        return false;
	}

private:
	ConsoleX11() {}

	static ConsoleX11* _singleton;
};

// --------------------------------------------------------------

ConsoleX11* ConsoleX11::_singleton = nullptr;

Console* Console::singleton() {
	return ConsoleX11::singleton();
}

void Console::initialize() {
}

void Console::cleanup() {
	ConsoleX11::cleanup();
}

}
