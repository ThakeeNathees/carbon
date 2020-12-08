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

// TODO: move all to path.cpp

namespace carbon {
namespace fs = std::filesystem;
class Path : public Object {

	REGISTER_CLASS(Path, Object) {
		BIND_STATIC_FUNC("absolute",  &Path::absolute,  PARAMS("path"));
		BIND_STATIC_FUNC("relative",  &Path::relative,  PARAMS("path", "to"), DEFVALUES(""));
		BIND_STATIC_FUNC("get_cwd",   &Path::get_cwd);
		BIND_STATIC_FUNC("set_cwd",   &Path::set_cwd,   PARAMS("path"));
		BIND_STATIC_FUNC("join",      &Path::join,      PARAMS("path1", "path2"));
		BIND_STATIC_FUNC("parent",    &Path::parent,    PARAMS("path"));
		BIND_STATIC_FUNC("filename",  &Path::filename,  PARAMS("path"));
		BIND_STATIC_FUNC("extension", &Path::extension, PARAMS("path"));
		BIND_STATIC_FUNC("exists",    &Path::exists,    PARAMS("path"));
		BIND_STATIC_FUNC("is_dir",    &Path::is_dir,    PARAMS("path"));
		BIND_STATIC_FUNC("mkdir",     &Path::mkdir,     PARAMS("path", "recursive"), DEFVALUES(false));
		BIND_STATIC_FUNC("copy_file", &Path::copy_file, PARAMS("from", "to"));
		BIND_STATIC_FUNC("copy_tree", &Path::copy_tree, PARAMS("from", "to"));
		BIND_STATIC_FUNC("remove",    &Path::remove,    PARAMS("path", "recursive"), DEFVALUES(false));
		BIND_STATIC_FUNC("rename",    &Path::rename,    PARAMS("old", "new")); // move and rename are the same.
		BIND_STATIC_FUNC("move",      &Path::rename,    PARAMS("old", "new"));
		BIND_STATIC_FUNC("listdir",   &Path::listdir,   PARAMS("path", "recursive"), DEFVALUES(false));

	}

public:

#define PATH_TRY(m_statement)                              \
do {													   \
	try {												   \
		m_statement;									   \
	} catch (std::exception& err) {						   \
		THROW_ERROR(Error::IO_ERROR, err.what());		   \
	}													   \
} while (false)

	static String absolute(const String& p_path) { return fs::absolute(p_path.operator std::string()).string(); }
	static String relative(const String& p_path, const String& p_to = "") {
		std::filesystem::path to;
		if (p_to == "") to = std::filesystem::current_path();
		else to = p_to.operator std::string();
		return fs::relative(p_path.operator std::string(), to).string();
	}
	static String parent(const String& p_path) { return fs::path(p_path.operator std::string()).parent_path().string(); }
	static String filename(const String& p_path) { return fs::path(p_path.operator std::string()).filename().string(); }
	static String extension(const String& p_path) { return fs::path(p_path.operator std::string()).extension().string(); }

	static String get_cwd() { return fs::current_path().string(); }
	static void set_cwd(const String& p_path) {
		PATH_TRY(fs::current_path(p_path.operator std::string()));
	}

	static String join(const String& p_path1, const String& p_path2) {
		return (fs::path(p_path1.operator std::string()) / fs::path(p_path2.operator std::string())).string();
	}

	static bool exists(const String& p_path) { return fs::exists(p_path.operator std::string()); }
	static bool is_dir(const String& p_path) { return fs::is_directory(p_path.operator std::string()); }
	static void mkdir(const String& p_path, bool p_recursive = false) {
		if (p_recursive) PATH_TRY(fs::create_directories(p_path.operator std::string()));
		else PATH_TRY(fs::create_directory(p_path.operator std::string()));
	}

	static void copy_file(const String& p_from, const String& p_to) {
		PATH_TRY(fs::copy_file(p_from.operator std::string(), p_to.operator std::string()));
	}

	static void copy_tree(const String& p_from, const String& p_to) {
		PATH_TRY(fs::copy(p_from.operator std::string(), p_to.operator std::string(), fs::copy_options::recursive));
	}

	static void remove(const String& p_path, bool p_recursive = false) {
		if (p_recursive) PATH_TRY(fs::remove_all(p_path.operator std::string()));
		else PATH_TRY(fs::remove(p_path.operator std::string()));
	}

	static void rename(const String& p_path, const String& p_new) {
		PATH_TRY(fs::rename(p_path.operator std::string(), p_new.operator std::string()));
	}

	static Array listdir(const String& p_path, bool p_recursive = false) {
		Array ret;
		try {
			if (p_recursive) {
				for (auto entry : fs::recursive_directory_iterator(p_path.operator std::string()))
					ret.append(String(entry.path().string()));
			} else {
				for (auto entry : fs::directory_iterator(p_path.operator std::string()))
					ret.append(String(entry.path().string()));
			}
		} catch (std::exception& err) {
			THROW_ERROR(Error::IO_ERROR, err.what());
		}
		return ret;
	}

};

#undef PATH_TRY

}

#endif // PATH_H