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

#ifndef CARBON_H
#define CARBON_H

// core imports
#include "core/core.h"

// compilation pipeline
#include "core/tokenizer/tokenizer.h"
#include "core/parser/parser.h"
#include "core/analyzer/analyzer.h"
#include "core/codegen/codegen.h"

#include "core/binary/carbon_function.h"
#include "core/binary/bytecode.h"

// native imports
#include "io/logger.h"
#include "io/file.h"
#include "io/dynamic_library.h"
#include "os/os.h"

namespace carbon {

inline void initialize(NativeClasses* p_singleton) {
	NativeClasses::_set_singleton(p_singleton);

	// Register native classes.
	NativeClasses::singleton()->register_class<Object>();
	NativeClasses::singleton()->register_class<_Iterator_String>();
	NativeClasses::singleton()->register_class<_Iterator_Array>();
	NativeClasses::singleton()->register_class<_Iterator_Map>();
	NativeClasses::singleton()->register_class<Bytecode>();

	NativeClasses::singleton()->register_class<OS>();
	NativeClasses::singleton()->register_class<File>();
	NativeClasses::singleton()->register_class<Buffer>();
	NativeClasses::singleton()->register_class<DynamicLibrary>();
	NativeClasses::singleton()->register_class<NativeLib>();

}

}


#endif // CARBON_H