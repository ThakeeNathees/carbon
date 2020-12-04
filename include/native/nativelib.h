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

#ifndef NATIVELIB_H
#define NATIVELIB_H

#include "core/core.h"
#include <dlfcn.h>

#include "api/varptr.h"
#include "api/nativeapi.h"

#include "file.h"
#include "path.h"
#include "os.h"

namespace carbon {

class NativeLib : public Object {
	REGISTER_CLASS(NativeLib, Object) {
		BIND_STATIC_FUNC("NativeLib", &NativeLib::_NativeLib, PARAMS("self", "path"), DEFVALUES(""));

		BIND_STATIC_FUNC("generate_api", &NativeLib::generate_api, PARAMS("path"), DEFVALUES(""));
		BIND_METHOD("open", &NativeLib::open, PARAMS("path"));
		BIND_METHOD("close", &NativeLib::close);

		_varapi_init();
	}

	NativeLib(const String& p_lib_name = "");
	~NativeLib();
	static void _NativeLib(ptr<Object> p_self, const String& p_lib_name = "");

	var call_method(const String& p_name, stdvec<var*>& p_args) override;

	void open(const String& p_path);
	void close();

	static void _varapi_init();
	static void generate_api(const String& p_path = "");
	static nativeapi* get_varapi();
	static varptr var_to_varptr(var* p_val);
	static var varptr_to_var(varptr* api);
	static void varptr_assign(varptr* p_vptr, var p_var);
	static varptr new_var_from(const var& p_from);
	static varptr new_var();
	static varptr new_string();
	static varptr new_array();
	static varptr new_map();
	static void free_var(varptr* p_vptr);

private:
	static nativeapi api;
	static unsigned int _alloc_id;
	static stdmap<int, var> allocate; // for lib

	void*  _handle = nullptr;
	String _path;

};

}
#endif // NATIVELIB_H
