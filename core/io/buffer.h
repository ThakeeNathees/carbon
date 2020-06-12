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

#ifndef BUFFER_H
#define BUFFER_H

#include "core.h"

namespace carbon {

class Buffer : Object {
private:
	Ptr<char[]> _buffer;

public:
	// Object overrides
	virtual bool get(const String& p_name, var& r_val) const override { return false; }
	virtual bool set(const String& p_name, const var& p_val) override { return false; }
	virtual bool has(const String& p_name) const override { return false; }

	virtual void copy(Object* r_ret, bool p_deep) const override { /* TODO: */ }
	String get_class_name() const { return "Buffer"; }

	// Buffer methods
	void alloc(size_t p_size) {
		_buffer = Ptr<char[]>(new char[p_size]);
	}

	char& operator[](size_t p_size) {
		// TODO: error handle
		static char c = 0;
		return c;
	}

	Buffer() {}
	Buffer(size_t p_size) { alloc(p_size); }

};

}

#endif // BUFFER_H