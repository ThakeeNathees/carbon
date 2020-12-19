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

#ifndef	PATH_H
#define	PATH_H

#include "core/core.h"

// TODO: split path to path and dir

// TODO: add copy options for overwrite

namespace carbon {
//namespace fs = std::filesystem;
class Path : public Object {

	REGISTER_CLASS(Path, Object) {
		BIND_STATIC_FUNC("Path",   &Path::_Path,     PARAMS("self", "path"), DEFVALUES(""));

		BIND_METHOD("absolute",   &Path::absolute);
		BIND_METHOD("join",       &Path::join,      PARAMS("path"));
		BIND_METHOD("parent",     &Path::parent);
		BIND_METHOD("filename",   &Path::filename);
		BIND_METHOD("extension",  &Path::extension);
		BIND_METHOD("exists",     &Path::exists);
		BIND_METHOD("isdir",      &Path::isdir);

	}

public:
	Path(const String& p_path = "");
	static void _Path(ptr<Path> p_self, const String& p_path);

	String absolute(); // TODO: return Path instance (ptr<Path>)
	String parent();   // TODO: return Path instance
	String filename();
	String extension();
	bool exists();
	bool isdir();

	ptr<Path> join(const String& p_path) const;
	ptr<Path> operator /(const Path& p_other) const;
	var __div(const var& p_other) override;

	String to_string() override;
	operator const String& () const;

private:
	String _path;

};

#undef PATH_TRY

}

#endif // PATH_H