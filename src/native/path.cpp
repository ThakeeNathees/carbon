//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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


#include "native/path.h"
#include "core/platform.h"

namespace carbon {

// constructors
Path::Path(const String& p_path) :_path(p_path) {}
void Path::_Path(ptr<Path> p_self, const String& p_path) {
	p_self->_path = p_path;
}

String Path::absolute() {
	return _Platform::path_absolute(_path);
}

bool Path::exists() {
	return _Platform::path_exists(_path);
}

bool Path::isdir() {
	return _Platform::path_isdir(_path);
}

Array Path::listdir() {
	Array ret;
	stdvec<std::string> dirs = _Platform::path_listdir(_path);
	for (const std::string& dir : dirs) {
		if (dir == "." || dir == "..") continue;
		ret.push_back(newptr<Path>(dir));
	}
	return ret;
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

ptr<Path> Path::join(const String& p_path) const {
	// TODO: this is temp and try corss platform libraries (boost/filesystem?)
	if (p_path.size() == 0) return newptr<Path>(_path);
	if (_path.size() == 0 || p_path[0] == '\\' || p_path[0] == '/') newptr<Path>(p_path);
	if (_path[0] == '\\' || p_path[0] == '/') {
		return newptr<Path>(_path + p_path);
	}
	return newptr<Path>(_path + '/' + p_path);
}

ptr<Path> Path::operator /(const Path& p_other) const {
	return join(p_other);
}

var Path::__div(const var& p_other) {
	if (p_other.get_type() == var::STRING) {
		return join(p_other.operator String());
	}
	if (p_other.get_type_name() != get_type_name_s()) {
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Object\".", p_other.get_type_name().c_str()));
	}
	return join(p_other.operator ptr<Path>()->operator const String &());
}

String Path::to_string() {
	return _path;
}

Path::operator const String& () const {
	return _path;
}

}
