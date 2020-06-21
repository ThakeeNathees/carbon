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

#ifndef FILE_H
#define FILE_H

#include "core.h"
#include "buffer.h"

namespace carbon {

class File : public Object {
public:
	enum {
		READ   = 1 << 0,
		WRITE  = 1 << 1,
		APPEND = 1 << 2,
		BINARY = 1 << 3,
	};

	// Object overrides.
	virtual String get_class_name() const override { return "File"; }

	File();
	~File();

	// Methods.
	bool is_open() const { return file.is_open(); }
	void open(const String& p_path, int p_mode = READ);
	void close();
	size_t size();
	String get_path() const { return path; }
	int get_mode() const { return mode; }

	String read_text();
	ptr<Buffer> read_bytes();
	Array get_lines();
	var read();


protected:

private:
	std::fstream file;
	String path;
	int mode;

};
}

#endif // FILE_H

