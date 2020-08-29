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

#include "native/native_classes.h"

#ifdef PLATFORM_WINDOWS
#include "dlfcn-win32/dlfcn.h"
#else
#include <dlfcn.h>
#endif

namespace carbon {

// FOR g++ this template should be in namespace scope
// error: explicit specialization in non-namespace scope
class DynamicLibrary;
template<unsigned int t_argn> struct _Visit {
template<typename... Targs>
static int _visit(DynamicLibrary* p_lib, const String& p_func_name, var* p_arg0, Targs... p_args) {

	if (p_arg0->get_type() == var::Type::BOOL) {
		return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator bool());

	} else if (p_arg0->get_type() == var::Type::INT) {
		return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator int64_t());

	} else if (p_arg0->get_type() == var::Type::FLOAT) {
		return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->operator float());

	} else if (p_arg0->get_type() == var::Type::STRING) {
		return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0->to_string().c_str());
		
	//} else if (p_arg0->get_type() == var::Type::OBJECT) {
	//	return _visit(p_func_name, p_argn - 1, p_args..., p_arg0->operator ptr<varh::Object>());
		
	} else { // else var*
		return _Visit<t_argn - 1>::_visit(p_lib, p_func_name, p_args..., p_arg0);
	}
	THROW_BUG("Please Bug Report.");
}};

class DynamicLibrary : public Object {
	REGISTER_CLASS(DynamicLibrary, Object) {
		BIND_METHOD("open", &DynamicLibrary::open);
		BIND_METHOD_VA("call", &DynamicLibrary::_call_va_args);
		BIND_METHOD("close", &DynamicLibrary::close);
	}

public:
	// Methods.
	void open(const String& p_lib_name) {
		if (handle) {
			THROW_ERROR(Error::IO_INVALID_OPERATORN, "lib already opened (close before reopening).");
		}
		handle = dlopen(p_lib_name.c_str(), RTLD_LAZY);
		if (!handle) { /* fail to load the library */
			THROW_ERROR(Error::IO_ERROR, String::format("%s", dlerror()));
		}
		lib_name = p_lib_name;
	}

	int _call_va_args(stdvec<var>& p_args) {
		if (p_args.size() == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "p_args.size() is 0, need at least 1 as the function name");
		if (p_args[0].get_type() != var::STRING) // TODO: better error msg
			THROW_ERROR(Error::INVALID_ARGUMENT, "first argument of call() must be string (name of the function)");
		const String& func = p_args[0];
		switch ((int)(p_args.size()-1)) {
			case 0: return _call(func.c_str());
			case 1: return  call(func, p_args[1]);
			case 2: return  call(func, p_args[1], p_args[2]);
			case 3: return  call(func, p_args[1], p_args[2], p_args[3]);
			case 4: return  call(func, p_args[1], p_args[2], p_args[3], p_args[4]);
			case 5: return  call(func, p_args[1], p_args[2], p_args[3], p_args[4], p_args[5]);
			default:
				THROW_ERROR(Error::INVALID_ARG_COUNT, "dynamic library call argument count must be less than 5");
		}
	}

	int call(const String& p_func) {
		//return _Visit<0>::_visit(this, p_func, nullptr);
		return _call(p_func.c_str());
	}
	int call(const String& p_func, var& arg0) {
		return _Visit<1>::_visit(this, p_func, &arg0, nullptr); 
	}
	int call(const String& p_func, var& arg0, var& arg1) {
		return _Visit<2>::_visit(this, p_func, &arg0, &arg1, nullptr);
	}
	int call(const String& p_func, var& arg0, var& arg1, var& arg2) {
		return _Visit<3>::_visit(this, p_func, &arg0, &arg1, &arg2, nullptr);
	}
	int call(const String& p_func, var& arg0, var& arg1, var& arg2, var& arg3) {
		return _Visit<4>::_visit(this, p_func, &arg0, &arg1, &arg2, &arg3, nullptr);
	}
	int call(const String& p_func, var& arg0, var& arg1, var& arg2, var& arg3, var& arg4) {
		return _Visit<5>::_visit(this, p_func, &arg0, &arg1, &arg2, &arg3, &arg4, nullptr);
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
			THROW_ERROR(Error::IO_INVALID_OPERATORN, "handle was NULL");
		}

		typedef int(*func_ptr)(T...);

		func_ptr fp;
		fp = (func_ptr) dlsym(handle, p_func_name);

		if (!fp) { /* no such symbol */
			dlclose(handle);
			THROW_ERROR(Error::IO_ERROR, String::format("%s", dlerror()));
		}

		int ret = fp(p_args...);
		return ret;
	}

	template<unsigned int t_argn> friend struct _Visit;

};

template<> struct _Visit<0> {
template<typename... Targs>
static int _visit(DynamicLibrary* p_lib, const String& p_func_name, var* p_nullptr, Targs... p_args) {
	return p_lib->_call(p_func_name.c_str(), p_args...);
}};


}

#endif // DYNAMIC_LYBRARY_H