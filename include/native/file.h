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

#include "core/core.h"
#include "buffer.h"

namespace carbon {

class File : public Object {

	REGISTER_CLASS(File, Object) {
		BIND_STATIC_FUNC("File", &File::_File, PARAMS("self", "path", "mode"), DEFVALUES("", DEFAULT));

		BIND_ENUM_VALUE("READ",   READ);
		BIND_ENUM_VALUE("WRITE",  WRITE);
		BIND_ENUM_VALUE("APPEND", APPEND);
		BIND_ENUM_VALUE("BINARY", BINARY);
		BIND_ENUM_VALUE("EXTRA",  EXTRA);
		//BIND_ENUM_VALUE("DEFAULT", DEFAULT);

		BIND_METHOD("open",  &File::open,  PARAMS("path", "mode"), DEFVALUES(DEFAULT));
		BIND_METHOD("read",  &File::read);
		BIND_METHOD("write", &File::write, PARAMS("what"));
		BIND_METHOD("close", &File::close);
	}

public:
	enum {
		READ   = 1 << 0, // "r"
		WRITE  = 1 << 1, // "w"
		APPEND = 1 << 2, // "a"
		BINARY = 1 << 3, // "b"
		EXTRA  = 1 << 4, // "+"

		DEFAULT = READ,
	};

	File(const String& p_path = "", int p_mode = DEFAULT);
	~File();
	static void _File(ptr<Object> p_self, const String& p_path = "", int p_mode = DEFAULT);

	// Methods.
	inline bool is_open() const { return _file != NULL; }
	void open(const String& p_path, int p_mode = DEFAULT);
	void close();
	long size();
	String get_path() const { return path; }
	int get_mode() const { return mode; }


	String read_text();
	void write_text(const String& p_text);

	ptr<Buffer> read_bytes();
	void write_bytes(const ptr<Buffer>& p_bytes);

	var read();
	void write(const var& p_what);


protected:

private:
	//std::fstream file;
	FILE* _file = NULL;
	String path;
	int mode = DEFAULT;

};
}

#endif // FILE_H

