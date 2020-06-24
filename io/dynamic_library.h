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

#ifndef DYNAMIC_LYBRARY_H
#define DYNAMIC_LYBRARY_H

#include "core.h"

#ifdef PLATFORM_WINDOWS
#include "dlfcn-win32/dlfcn.h"
#else
#include <dlfcn.h>
#endif

namespace carbon {

class DynamicLibrary : public Object {
public:
	// Object overrides.
	virtual String get_class_name() const override { return "DynamicLibrary"; }

	// Methods.
	void open(const char* p_lib_name) {
		if (handle) {
			throw Error(Error::IO_INVALID_OPERATORN, "lib already opened (close before reopening).");
		}
		handle = dlopen(p_lib_name, RTLD_LAZY);
		if (!handle) { /* fail to load the library */
			throw Error(Error::IO_ERROR, String::format("%s", dlerror()));
		}
		lib_name = p_lib_name;
	}

	int call(const char* p_func) {
		//return _Visit<0>::_visit(this, p_func, nullptr);
		return _call(p_func);
	}
	int call(const char* p_func, var& arg0) {
		return _Visit<1>::_visit(this, p_func, &arg0, nullptr); 
	}
	int call(const char* p_func, var& arg0, var& arg1) {
		return _Visit<2>::_visit(this, p_func, &arg0, &arg1, nullptr);
	}
	int call(const char* p_func, var& arg0, var& arg1, var& arg3) {
		return _Visit<3>::_visit(this, p_func, &arg0, &arg1, &arg3, nullptr);
	}
	int call(const char* p_func, var& arg0, var& arg1, var& arg3, var& arg4) {
		return _Visit<4>::_visit(this, p_func, &arg0, &arg1, &arg3, &arg4, nullptr);
	}
	int call(const char* p_func, var& arg0, var& arg1, var& arg3, var& arg4, var& arg5) {
		return _Visit<5>::_visit(this, p_func, &arg0, &arg1, &arg3, &arg4, &arg5, nullptr);
	}
	// !!!!!!!!! NO MORE => NUMBER OF RECURSIVE VARIADIC TEMPLATE WILL EXPLODE !!!!!!!!!
#undef VISIT_ARGS

	void close() {
		if (handle) {
			dlclose(handle);
			handle = nullptr;
		}
	}

	DynamicLibrary(){}
	DynamicLibrary(const char* p_lib_name) {
		open(p_lib_name);
	}
	~DynamicLibrary(){
		close();
	}

protected:

private:
	void* handle = nullptr;
	String lib_name;

	template<typename... T>
	int _call(const char* p_func_name, T... p_args) {

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

		int ret = fp(p_args...);
		return ret;
	}

	template<unsigned int t_argn> struct _Visit {
	template<typename... Targs>
	static int _visit(DynamicLibrary* p_lib, const char* p_func_name, var* p_arg0, Targs... p_args) {

		if (p_arg0->get_type() == var::Type::BOOL) {
			return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator bool());

		} else if (p_arg0->get_type() == var::Type::INT) {
			return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator int());

		} else if (p_arg0->get_type() == var::Type::FLOAT) {
			return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator float());

		} else if (p_arg0->get_type() == var::Type::STRING) {
			return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->to_string().c_str());
		
		//} else if (p_arg0->get_type() == var::Type::OBJECT) {
		//	return _visit(p_func_name, p_argn - 1, p_args..., p_arg0->operator ptr<varh::Object>());
		
		} else { // else var*
			return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0);
		}

		throw Error(Error::INTERNAL_BUG, "Please Bug Report.");
	}};

	template<> struct _Visit<0> {
	template<typename... Targs>
	static int _visit(DynamicLibrary* p_lib, const char* p_func_name, var* p_nullptr, Targs... p_args) {
		return p_lib->_call(p_func_name, p_args...);
	}};

};

}

#endif // DYNAMIC_LYBRARY_H