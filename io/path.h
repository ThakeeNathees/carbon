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

#include "core.h"

namespace carbon {

namespace fs = std::filesystem;
class Path : public Object {

	REGISTER_CLASS(Path, Object) {
		BIND_STATIC_FUNC("absolute", &Path::absolute, PARAMS("path"));
		BIND_STATIC_FUNC("relative", &Path::relative, PARAMS("path", "to"), DEFVALUES(""));
		BIND_STATIC_FUNC("get_cwd",  &Path::get_cwd);
		BIND_STATIC_FUNC("set_cwd",  &Path::set_cwd, PARAMS("path"));
		BIND_STATIC_FUNC("join", &Path::join, PARAMS("path1", "path2"));
		BIND_STATIC_FUNC("parent", &Path::parent, PARAMS("path"));
		BIND_STATIC_FUNC("filename", &Path::filename, PARAMS("path"));
		BIND_STATIC_FUNC("extension", &Path::extension, PARAMS("path"));
		BIND_STATIC_FUNC("exists", &Path::exists, PARAMS("path"));
		BIND_STATIC_FUNC("is_dir", &Path::is_dir, PARAMS("path"));

	}

private:
public:
	static String absolute(const String& p_path) {
		return fs::absolute(p_path.operator std::string()).string();
	}

	static String relative(const String& p_path, const String& p_to = "") {
		std::filesystem::path to;
		if (p_to == "") to = std::filesystem::current_path();
		else to = p_to.operator std::string();
		return fs::relative(p_path.operator std::string(), to).string();
	}

	static String get_cwd() {
		return fs::current_path().string();
	}

	static void set_cwd(const String& p_path) {
		// TODO: error handle here (catch and throw the Error() object)
		fs::current_path(p_path.operator std::string());
	}

	static String join(const String& p_path1, const String& p_path2) {
		return (fs::path(p_path1.operator std::string()) / fs::path(p_path2.operator std::string())).string();
	}

	static String parent(const String& p_path) {
		return fs::path(p_path.operator std::string()).parent_path().string();
	}

	static String filename(const String& p_path) {
		return fs::path(p_path.operator std::string()).filename().string();
	}

	static String extension(const String& p_path) {
		return fs::path(p_path.operator std::string()).extension().string();
	}

	static bool exists(const String& p_path) {
		return fs::exists(p_path.operator std::string());
	}

	static bool is_dir(const String& p_path) {
		return fs::is_directory(p_path.operator std::string());
	}
};

}

#endif // PATH_H