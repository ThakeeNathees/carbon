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

#include "native/nativelib.h"
#include "native_api.gen.inc"

namespace carbon {

NativeLib::NativeLib(const String& p_lib_name) { if (p_lib_name.size() != 0) open(p_lib_name); }
NativeLib::~NativeLib() { close(); }
void NativeLib::_NativeLib(ptr<Object> p_self, const String& p_lib_name) {
	NativeLib* self = ptrcast<NativeLib>(p_self).get();
	if (p_lib_name.size() != 0) self->open(p_lib_name.c_str());
}

void NativeLib::open(const String& path) {
	_path = Path::absolute(path);
	if (_handle) THROW_ERROR(Error::IO_ERROR, "lib already opened (close before reopening).");
	_handle = dlopen(_path.c_str(), RTLD_LAZY);
	if (!_handle) THROW_ERROR(Error::IO_ERROR, String::format("failed to load lib : %s.", dlerror()));

	// TODO: string literal
	typedef void (*init_varapi_f)(nativeapi*);
	void* init_varapi_sym = dlsym(_handle, "varapi_init");
	if (init_varapi_sym == nullptr) return;
	init_varapi_f init_varapi_fn = (init_varapi_f)init_varapi_sym;
	init_varapi_fn(&api);
}

void NativeLib::close() {
	if (_handle) {
		dlclose(_handle);
		_handle = nullptr;
	}
}

var NativeLib::call_method(const String& p_name, stdvec<var*>& p_args) {
	if (!_handle) THROW_ERROR(Error::IO_ERROR, "handle was NULL.");

	typedef varptr* (*func_ptr)(int argc, varptr* argv);
	func_ptr fp;
	fp = (func_ptr)dlsym(_handle, p_name.c_str());
	if (!fp) THROW_ERROR(Error::IO_ERROR, String::format("%s.", dlerror()));

	stdvec<varptr> args_api;
	for (var* v : p_args) args_api.push_back(var_to_varptr(v));

	varptr* ret = fp((int)p_args.size(), args_api.data());
	return varptr_to_var(ret);
}

void NativeLib::generate_api(const String& p_path) {
	String path;
	if (p_path != "") {
		if (!Path::is_dir(p_path)) THROW_ERROR(Error::IO_ERROR, String::format("expected a directory path, got : %s", p_path.c_str()));
	} else path = Path::get_cwd();

	path += "carbon/";
	Path::mkdir(path, true);

	File target;

	stdvec<std::pair<const char*, const char*>> ctx = {
		{VARPTR_SOURCE, "varptr.h"},
		{NATIVEAPI_SOURCE, "nativeapi.h"},
		{WRAPPERS_SOURCE, "wrappers.h"},
		{API_SOURCE, "api.h"},
	};
	for (size_t i = 0; i < ctx.size(); i++) {
		target.open(Path::join(path, ctx[i].second), File::WRITE);
		target.write_text(ctx[i].first);
		target.close();
	}

}

nativeapi* NativeLib::get_varapi() {
	return &api;
}

// this method might be unnecessary (code reuse)
void NativeLib::varptr_assign(varptr* p_vptr, var p_var) {
	*((var*)p_vptr->_ptr) = p_var;
	p_vptr->_data = ((var*)p_vptr->_ptr)->get_data();

	switch (p_var.get_type()) {
		case var::_NULL: {
			p_vptr->type = varptr::_NULL;
		} break;
		case var::BOOL: {
			p_vptr->type = varptr::BOOL;
			p_vptr->_bool = p_var.operator bool();
		} break;
		case var::INT: {
			p_vptr->type = varptr::INT;
			p_vptr->_int = p_var.operator int64_t();
		} break;
		case var::FLOAT: {
			p_vptr->type = varptr::FLOAT;
			p_vptr->_float = p_var.operator double();
		} break;
		case var::STRING: {
			p_vptr->type = varptr::STRING;
			// should be points to copyied now.
			p_vptr->_str = (const char*)(p_vptr->_data);
		} break;
		case var::ARRAY: {
			p_vptr->type = varptr::ARRAY;
		} break;
		case var::MAP: {
			p_vptr->type = varptr::MAP;
		} break;
		default: {
			p_vptr->type = varptr::OBJECT;
		} break;
	}
}

varptr NativeLib::var_to_varptr(var* p_val) {
	varptr vptr;
	vptr._ptr = p_val;
	vptr._data = p_val->get_data();

	switch (p_val->get_type()) {
		case var::_NULL: {
			vptr.type = varptr::_NULL;
		} break;
		case var::BOOL: {
			vptr.type = varptr::BOOL;
			vptr._bool = p_val->operator bool();
		} break;
		case var::INT: {
			vptr.type = varptr::INT;
			vptr._int = p_val->operator int64_t();
		} break;
		case var::FLOAT: {
			vptr.type = varptr::FLOAT;
			vptr._float = p_val->operator double();
		} break;
		case var::STRING: {
			vptr.type = varptr::STRING;
			vptr._str = (const char*)p_val->get_data();
		} break;
		case var::ARRAY: {
			vptr.type = varptr::ARRAY;
		} break;
		case var::MAP: {
			vptr.type = varptr::MAP;
		} break;
		default: {
			vptr.type = varptr::OBJECT;
		} break;

	}
	return vptr;
}

var NativeLib::varptr_to_var(varptr* pvar) {
	if (pvar == nullptr) return var();
	switch (pvar->type) {
		case varptr::_NULL: return var();
		case varptr::BOOL: return pvar->_bool;
		case varptr::INT: return pvar->_int;
		case varptr::FLOAT: return pvar->_float;
		case varptr::STRING: return pvar->_str;
		default:
			THROW_IF_NULLPTR(pvar->_ptr);
			return *(var*)pvar->_ptr;
	}
}


}