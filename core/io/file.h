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

	// Object methods.
	virtual bool get(const String& p_name, var& r_val)       const override { return false; }
	virtual bool set(const String& p_name, const var& p_val)       override { return false; }
	virtual bool has(const String& p_name)                   const override { return false; }
	virtual ptr<Object> copy(bool p_deep)                    const          { return ptrcast<Object>(newptr<File>()); }
	virtual String get_class_name()                          const          { return "File"; }

	enum {
		READ   = 1 << 0,
		WRITE  = 1 << 1,
		APPEND = 1 << 2,
		BINARY = 1 << 3,
	};

private:
	std::fstream _file;
	String       _path;
	int          _mode;

public:
	File();
	~File();

	bool is_open() const { return _file.is_open(); }
	void open(const String& p_path, int p_mode = READ);
	void close();
	size_t size();

	String read_text();
	ptr<Buffer> read_bytes();
	Array get_lines();

	String get_path() const { return _path; }
	int get_mode() const { return _mode; }

	// carbon api.
	var read();

};
}

#endif // FILE_H

