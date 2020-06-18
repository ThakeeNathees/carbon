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

#ifndef DL_LOADER_H
#define DL_lOADER_H

#include "core.h"

#ifdef PLATFORM_WINDOWS
#include "dlfcn-win32/dlfcn.h"
#else
#include <dlfcn.h>
#endif

namespace carbon {

class DlLoader : public Object {
public:
	// Object overrides.
	virtual String get_class_name() const override { return "DlLoader"; }

	// Methods.
	void dl_open(const char* p_lib_name) {
		if (handle) {
			throw Error(Error::IO_INVALID_OPERATORN, "lib already opened (close before reopening).");
		}
		handle = dlopen(p_lib_name, RTLD_LAZY);
		if (!handle) { /* fail to load the library */
			throw Error(Error::IO_ERROR, String::format("%s", dlerror()));
		}
		lib_name = p_lib_name;
	}

#define VISIT_ARGS(...) GET_ARG_COUNT(__VA_ARGS__), __VA_ARGS__, var()
	int dl_call(const char* p_func) {
		return _call(p_func);
	}
	int dl_call(const char* p_func, var& arg0) {
		return _visit(p_func, VISIT_ARGS(arg0)); 
	}
	int dl_call(const char* p_func, var& arg0, var& arg1) {
		return _visit(p_func, VISIT_ARGS(arg0, arg1));
	}
	int dl_call(const char* p_func, var& arg0, var& arg1, var& arg3) {
		return _visit(p_func, VISIT_ARGS(arg0, arg1, arg3));
	}
	int dl_call(const char* p_func, var& arg0, var& arg1, var& arg3, var& arg4) {
		return _visit(p_func, VISIT_ARGS(arg0, arg1, arg3, arg4));
	}
	int dl_call(const char* p_func, var& arg0, var& arg1, var& arg3, var& arg4, var& arg5) {
		return _visit(p_func, VISIT_ARGS(arg0, arg1, arg3, arg4, arg5));
	}
	// !!!!!!!!! NO MORE => NUMBER OF RECURSIVE VARIADIC TEMPLATE WILL EXPLODE !!!!!!!!!
#undef VISIT_ARGS

	void dl_close() {
		if (handle) {
			dlclose(handle);
			handle = nullptr;
		}
	}

	DlLoader(){}
	DlLoader(const char* p_lib_name) {
		dl_open(p_lib_name);
	}
	~DlLoader(){
		dl_close();
	}

protected:

private:
	void* handle = nullptr;
	String lib_name;

	template<typename... T>
	int _call(const char* p_func_name, T... val) {

		if (!handle) {
			throw Error(Error::IO_INVALID_OPERATORN, "handle was NULL");
		}

		typedef int(*func_ptr)(T...);

		func_ptr fp;
		fp = (func_ptr) dlsym(handle, p_func_name);

		if (!fp) { /* no such symbol */
			dlclose(handle);
			throw Error(Error::IO_ERROR, String::format("%s", dlerror()));
		}

		int ret = fp(val...);
		return ret;
	}

	template<typename... Targs>
	int _visit(const char* p_func_name, int p_argn, var p_arg0, Targs... p_args) {

		if (p_argn == 0) {
			return _call(p_func_name, p_args...);
		}

		if (p_arg0.get_type() == var::Type::BOOL) {
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0.operator bool());

		} else if (p_arg0.get_type() == var::Type::INT) {
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0.operator int());

		} else if (p_arg0.get_type() == var::Type::FLOAT) {
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0.operator float());

		} else if (p_arg0.get_type() == var::Type::STRING) {
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0.to_string().c_str());
		
		} else if (p_arg0.get_type() == var::Type::OBJECT) {
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0.operator ptr<varh::Object>());
		
		} else { // else var
			return _visit(p_func_name, p_argn - 1, p_args..., p_arg0);
		}

		throw Error(Error::INTERNAL_BUG, "Please Bug Report.");
	}

};

}

#endif // DL_LOADER_H