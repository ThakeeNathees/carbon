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

#ifdef _WIN32
// FIXME: including like this is to reduce buil workload
//        for a newbie trying to compile the source
#include "../thirdparty/dlfcn-win32/dlfcn.h"
#else
#include <dlfcn.h>
#endif

#if 1 // TODO:
#include "api.gen.inc"
#else
static const char* NATIVE_API_STR = "";
#endif

namespace carbon {

nativeapi NativeLib::api;

NativeLib::NativeLib(const String& p_lib_name) { if (p_lib_name.size() != 0) open(p_lib_name); }
NativeLib::~NativeLib() { close(); }
void NativeLib::_NativeLib(ptr<Object> p_self, const String& p_lib_name) {
	// TODO: use cache to prevent re-load
	NativeLib* self = ptrcast<NativeLib>(p_self).get();
	if (p_lib_name.size() != 0) self->open(p_lib_name.c_str());
}

void NativeLib::open(const String& path) {
	_path = Path(path).absolute();
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

void* NativeLib::_get_function(const String& p_name) {
	if (!_handle) THROW_ERROR(Error::IO_ERROR, "handle was NULL.");

	void* fp = dlsym(_handle, p_name.c_str());
	if (!fp) THROW_ERROR(Error::IO_ERROR, String::format("%s.", dlerror()));
	return fp;
}

var NativeLib::call_method(const String& p_name, stdvec<var*>& p_args) {
	func_ptr fp = (func_ptr) _get_function(p_name);
	return fp((int)p_args.size(), (uint8_t**)p_args.data());
}

void NativeLib::generate_api(const String& p_path) {
	String path;
	if (p_path != "") {
		if (!Path(p_path).isdir()) THROW_ERROR(Error::IO_ERROR, String::format("expected a directory path, got : %s", p_path.c_str()));
		path = p_path;
	} else path = OS::getcwd();

	File target;
	target.open(*Path(path).join("carbon_api.h"), File::WRITE);
	target.write_text(NATIVE_API_STR);
	target.close();
}

nativeapi* NativeLib::get_varapi() {
	return &api;
}

}