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


#include "path.h"

#include "core/platform/windows/platform_windows.h"

#define PATH_TRY(m_statement)                              \
do {													   \
	try {												   \
		m_statement;									   \
	} catch (std::exception& err) {						   \
		THROW_ERROR(Error::IO_ERROR, err.what());		   \
	}													   \
} while (false)


namespace carbon {

// constructors
Path::Path(const String& p_path) :_path(p_path) {}
void Path::_Path(ptr<Path> p_self, const String& p_path) {
	p_self->_path = p_path;
}

String Path::absolute() {
	char buffer[VSNPRINTF_BUFF_SIZE];
	if (GetFullPathNameA(_path.c_str(), VSNPRINTF_BUFF_SIZE, buffer, NULL) == 0) {
		THROW_ERROR(Error::IO_ERROR, windows::get_last_error_as_string());
	}
	return buffer;
}

String Path::parent() {
	const std::string& str = _path;
	size_t found = str.find_last_of("/\\");
	if (found == std::string::npos) return "";
	return str.substr(0, found);
}

String Path::filename() {
	const std::string& str = _path;
	size_t found = str.find_last_of("/\\");
	if (found == std::string::npos) return "";
	return str.substr(found + 1);
}

String Path::extension() {
	const std::string& str = _path;
	size_t found = str.rfind('.');
	if (found == std::string::npos) return "";
	return str.substr(found);
}


String Path::join(const String& p_path2) {
	THROW_BUG("Not imlpemented");
}

bool Path::exists() {
	DWORD ftyp = GetFileAttributesA(_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return true;                                   // this is not a directory!
}

bool Path::isdir() {
	DWORD ftyp = GetFileAttributesA(_path.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES) return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;   // this is a directory!
	else return false;									// this is not a directory!
}

String Path::to_string() {
	return _path;
}

Path::operator const String& () {
	return _path;
}

}
