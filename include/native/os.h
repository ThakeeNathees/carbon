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

#ifndef OS_H
#define OS_H

#include "core/native.h"

namespace carbon {

class OS : public Object {
	REGISTER_CLASS(OS, Object) {
		BIND_STATIC_FUNC("unix_time", &OS::unix_time);
		BIND_STATIC_FUNC("system", &OS::syscall, PARAMS("command"));


		// TODO: implement cross-platform manner
		BIND_STATIC_FUNC("getcwd", &OS::getcwd);
		BIND_STATIC_FUNC("chdir", &OS::chdir, PARAMS("path"));
		//BIND_STATIC_FUNC("mkdir", &OS::mkdir, PARAMS("path", "recursive"), DEFVALUES(false));
		//BIND_STATIC_FUNC("copy_file", &OS::copy_file, PARAMS("from", "to"));
		//BIND_STATIC_FUNC("copy_tree", &OS::copy_tree, PARAMS("from", "to"));
		//BIND_STATIC_FUNC("remove", &OS::remove, PARAMS("path", "recursive"), DEFVALUES(false));
		//BIND_STATIC_FUNC("rename", &OS::rename, PARAMS("old", "new")); // move and rename are the same.
		//BIND_STATIC_FUNC("listdir", &OS::listdir, PARAMS("path", "recursive"), DEFVALUES(false));
	}

public:
	static int64_t unix_time();
	static int syscall(const String& p_cmd);

	// dir/path related
	static String getcwd();
	static void chdir(const String& p_path);
	//static void mkdir(const String& p_path, bool p_recursive = false);
	//static void copy_file(const String& p_from, const String& p_to);
	//static void copy_tree(const String& p_from, const String& p_to);
	//static void remove(const String& p_path, bool p_recursive = false);
	//static void rename(const String& p_path, const String& p_new);
	//static Array listdir(const String& p_path, bool p_recursive = false);

};

}
#endif // OS_H