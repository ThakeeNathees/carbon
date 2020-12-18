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

namespace carbon {

// constructors
Path::Path(const String& p_path) :_path(p_path) {}
void Path::_Path(ptr<Path> p_self, const String& p_path) {
	p_self->_path = p_path;
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

String Path::to_string() {
	return _path;
}

Path::operator const String& () {
	return _path;
}

}
