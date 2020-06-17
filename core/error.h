//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c), 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),, to deal
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

#ifndef ERRORS_H
#define ERRORS_H

#include "var.h/_var.h"
using namespace varh;

namespace carbon {

class Error : public std::exception {
public:
	enum Type {
		OK = 0,

		// Compiletime errors.
		SYNTAX_ERROR,
		UNEXPECTED_EOF,
		ALREADY_DEFINED,

		// Runtime errors.
		NULL_POINTER,
		INVALID_INDEX,
		INVALID_CASTING,
		NOT_IMPLEMENTED,
		ZERO_DIVISION,

		IO_ERROR,
		CANT_OPEN_FILE,
		IO_INVALID_OPERATORN,

		// for debugging
		INTERNAL_BUG,

	};

	const char* what() const noexcept override { return msg.c_str(); }
	Type get_type() const { return type; }
	Vect2i get_pos() const { return pos; }

	Error() {}
	Error(Type p_type) { type = p_type; }
	Error(Type p_type, const String& p_msg) { type = p_type; msg = p_msg; }
	Error(Type p_type, const String& p_msg, const Vect2i p_pos) { type = p_type; msg = p_msg; pos = p_pos; }

private:
	Type type = OK;
	String msg;
	Vect2i pos = Vect2i(-1, -1);
};

}

#endif // ERRORS_H
