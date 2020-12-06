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

namespace carbon {

class _NativeStruct : public Buffer {
	REGISTER_CLASS(_NativeStruct, Buffer) {
		BIND_METHOD("load", &_NativeStruct::load, PARAMS("path"));
		BIND_METHOD("get_lib", &_NativeStruct::get_lib);
	}

public:
	void load(const String& p_path);
	ptr<NativeLib> get_lib();
	// TODO: defind DataType abstraction to define the layout.


private:
	// TODO: move this to a generatl native location if any in the future.
	static stdmap<String, ptr<NativeLib>> lib_cache;
	ptr<NativeLib> _lib;
};


}

#endif // NATIVE_STRUCT_H
