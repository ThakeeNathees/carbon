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
	if (file.is_open()) {
		file.close();
	}
}

void File::open(const String& p_path, int p_mode) {

	path = p_path;
	mode = p_mode;

	int mode = 0;
	if (p_mode & READ) {
		mode |= std::ios::in;
	}
	if (p_mode & WRITE) {
		mode |= std::ios::out;
	}
	if (p_mode & APPEND) {
		mode |= std::ios::app;
	}
	if (p_mode & BINARY) {
		mode |= std::ios::binary;
	}

	file.open(path, mode);
	if (!file.is_open()) {
		throw Error(Error::CANT_OPEN_FILE, String::format("can't open \"%s\"", path));
	}
}

size_t File::size() {
	std::streampos begin, end;
	file.seekg(0, std::ios::beg);
	begin = file.tellg();
	file.seekg(0, std::ios::end);
	end = file.tellg();
	return end - begin;
}

String File::read_text() {
	if (!is_open()) throw Error(Error::IO_INVALID_OPERATORN, "can't read on a closed file");

	std::stringstream sstream;
	std::string line;
	while (std::getline(file, line)) {
		sstream << line << '\n';
	}
	return sstream.str();
}

ptr<Buffer> File::read_bytes() {
	if (!is_open()) throw Error(Error::IO_INVALID_OPERATORN, "can't read on a closed file");

	size_t file_size = size();
	ptr<Buffer> buff = newptr<Buffer>(file_size);
	file.seekg(0, std::ios::beg);
	file.read(buff->front(), file_size);
	return buff;
}

Array File::get_lines() {
	if (!is_open()) throw Error(Error::IO_INVALID_OPERATORN, "can't read on a closed file");
	Array arr;
	std::string line;
	while (std::getline(file, line)) {
		arr.append(String(line));
	}
	return arr;
}

var File::read() {
	if (mode & BINARY) {
		return read_bytes();
	} else {
		return read_text();
	}
}

File::File() {}

File::~File() {
	close();
}

}

