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

#include "native/api/native_struct.h"
#include "native/path.h"

namespace carbon {

//stdmap<String, ptr<NativeLib>> _NativeStruct::lib_cache;

//void _NativeStruct::alloc(size_t p_size) {
//	_size = p_size;
//	_data = ptr<void>(malloc(_size), free_delete());
//}

void* _NativeStruct::get_data() {
	return _data.get();
}

void _NativeStruct::_set_lib(var p_lib) {
	if (_lib != nullptr) THROW_ERROR(Error::ASSERTION, "lib already initialized");
	_lib = p_lib.cast_to<NativeLib>();
}

void _NativeStruct::_set_new_delete(const String& p_ctor, const String& p_destruct) {
	THROW_IF_NULLPTR(_lib);
	fp_new = (NativeLib::func_ptr)_lib->_get_function(p_ctor);
	fp_delete = (_native_struct_delete)_lib->_get_function(p_destruct);
}

void _NativeStruct::_init(stdvec<var*>& p_va_args) {
	void* _new_obj = fp_new((int)p_va_args.size(), (uint8_t**)p_va_args.data());
	_data = ptr<void>(_new_obj, native_destruct(fp_delete));
}

ptr<NativeLib> _NativeStruct::get_lib() { return _lib; }

}