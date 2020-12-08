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
#ifndef NATIVE_STRUCT_H
#define NATIVE_STRUCT_H

#include "native/buffer.h"
#include "native/nativelib.h"

/* // carbon ///////////////////////
	class Aclass : _NativeStruct {
		func Aclass(args) {
			_set_lib(lib);
			_set_new_delete("new_Aclass", "delete_Aclass");
			_init(args);
		}
	}
	// c++ ////////////////////////////
	void* new_Aclass(int argc, var** argv) { return new Aclass(args); }
	void delete_Aclass(void* o) { delete (Aclass*)o; }
*/

namespace carbon {
	// TODO: this class is incomplete

class _NativeStruct : public Object, public std::enable_shared_from_this<_NativeStruct> {
	REGISTER_CLASS(_NativeStruct, Object) {

		BIND_METHOD("_set_lib", &_NativeStruct::_set_lib, PARAMS("lib"));
		BIND_METHOD("_set_new_delete", &_NativeStruct::_set_new_delete, PARAMS("constructor", "destructor"));
		BIND_METHOD_VA("_init", &_NativeStruct::_init);
		BIND_METHOD("get_lib", &_NativeStruct::get_lib);
	}

	typedef void* (*_native_struct_ctor)();
	typedef void (*_native_struct_delete)(void*);
	struct native_destruct {
		native_destruct(_native_struct_delete p_fp) {
			fp = p_fp;
		}
		_native_struct_delete fp;
		void operator()(void* o) {
			fp(o);
		}
	};

public:
	void _set_lib(var p_lib);
	void _set_new_delete(const String& p_ctor, const String& p_destruct);
	void _init(stdvec<var*>& p_va_args);

	void* get_data() override;
	ptr<NativeLib> get_lib();



private:
	// TODO: move this to a generatl native location if any in the future.
	//static stdmap<String, ptr<NativeLib>> lib_cache;

	ptr<NativeLib> _lib;

	ptr<void> _data;
	NativeLib::func_ptr fp_new;
	_native_struct_delete fp_delete;
};


}

#endif // NATIVE_STRUCT_H
