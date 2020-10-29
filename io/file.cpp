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


#include "file.h"

namespace carbon {

void File::close() {
	if (is_open()) {
		fclose(_file);
		_file = NULL;
	} else {
		// TODO: Warn here ??
	}
}

void File::open(const String& p_path, int p_mode) {
	if (p_mode < READ || (p_mode > (READ | WRITE | APPEND | BINARY | EXTRA))) THROW_ERROR(Error::IO_ERROR, "invalid mode flag set in file.");

	// TODO: fix the error message, print given combination.
	if ((p_mode & READ) && (p_mode & WRITE) && (p_mode & APPEND)) THROW_ERROR(Error::IO_ERROR, "invalid combination of flags (WRITE & APPEND) in file.");
	if (!(p_mode & READ) && !(p_mode & WRITE) && !(p_mode & APPEND)) THROW_ERROR(Error::IO_ERROR, "invalid combination of flags (WRITE & APPEND) in file.");

	path = p_path;
	mode = p_mode;

	String _strmode;
	bool binary = (p_mode & BINARY);
	bool extra = (p_mode & EXTRA);
	if (p_mode & READ) {
		if (binary && extra) _strmode = "rb+";
		else if (extra)      _strmode = "r+";
		else if (binary)     _strmode = "rb";
		else                 _strmode = "r";
		
	} else if (p_mode & WRITE) {
		if (binary && extra) _strmode = "wb+";
		else if (extra)      _strmode = "w+";
		else if (binary)     _strmode = "wb";
		else                 _strmode = "w";

	} else { // APPEND
		if (binary && extra) _strmode = "ab+";
		else if (extra)      _strmode = "a+";
		else if (binary)     _strmode = "ab";
		else                 _strmode = "a";
	}
	_file = fopen(path.c_str(), _strmode.c_str());
	
	if (_file == NULL) {
		THROW_ERROR(Error::IO_ERROR, String::format("can't open file at \"%s\".", path.c_str()));
	}
}

long File::size() {
	fseek(_file, 0, SEEK_END);
	long _size = ftell(_file);
	fseek(_file, 0, SEEK_SET);
	return _size;
}

String File::read_text() {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't read on a closed file.");
	if (!(mode & READ) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for reading.");
	if (mode & BINARY) THROW_ERROR(Error::IO_ERROR, "opened file mode is binary not supported for text.");
	long _file_size = size();
	if (_file_size == 0) return String();
	
	char* buff = new char[_file_size + 1];
	size_t read = fread(buff, sizeof(char), _file_size, _file);
	buff[read] = '\0';
	String text = buff;
	delete[] buff;
	
	return text;
}

void File::write_text(const String& p_text) {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't write on a closed file.");
	if (!(mode & WRITE) && !(mode & APPEND) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for writing.");
	fprintf(_file, p_text.c_str());
}

ptr<Buffer> File::read_bytes() {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't read on a closed file.");
	if (!(mode & READ) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for reading.");
	if (!(mode & BINARY)) THROW_ERROR(Error::IO_ERROR, "opened file mode is text not supported for binary.");
	long file_size = size();
	ptr<Buffer> buff = newptr<Buffer>(file_size);
	fseek(_file, 0, SEEK_SET);
	fread(buff->front(), sizeof(byte_t), file_size, _file);
	return buff;
}

void File::write_bytes(const ptr<Buffer>& p_bytes) {
	if (!is_open()) THROW_ERROR(Error::IO_ERROR, "can't write on a closed file.");
	if (!(mode & WRITE) && !(mode & APPEND) && !(mode & EXTRA)) THROW_ERROR(Error::IO_ERROR, "opened file mode isn't supported for writing.");
	fwrite(p_bytes->front(), sizeof(byte_t), p_bytes->size(), _file);
}

var File::read() {
	if (mode & BINARY) {
		return read_bytes();
	} else {
		return read_text();
	}
}

void File::write(const var& p_what) {
	if (mode & BINARY) {
		if (p_what.get_type() != var::OBJECT || p_what.get_type_name() != Buffer::get_class_name_s()) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("expected type %s at argument 0.", Buffer::get_class_name_s()));
		}
		return write_bytes(ptrcast<Buffer>(p_what.operator ptr<Object>()));
	} else {
		if (p_what.get_type() != var::STRING) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("expected type %s at argument 0.", var::get_type_name_s(var::STRING)));
		}
		return write_text(p_what.to_string());
	}
}

File::File(const String& p_path, int p_mode) {
	path = p_path;
	if (path.size() != 0) open(path, p_mode);
}

File::~File() {
	close();
}

void File::_File(ptr<Object> p_self, const String& p_path, int p_mode) {
	File* file = ptrcast<File>(p_self).get();
	file->path = p_path;
	if (file->path.size() != 0) file->open(file->path, p_mode);
}

}

