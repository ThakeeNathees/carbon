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
#include "core/internal.h"
#include "core/_error.h"
#include "core/logger.h"
#include "core/console.h"
#include "core/platform.h"
#include "core/type_info.h"
#include "core/native.h"

// var imports
#include "var/var_internal.h"
#include "var/_string.h"
#include "var/_array.h"
#include "var/_map.h"
#include "var/_object.h"
#include "var/var.h"

// native imports
#include "native/iterators.h"
#include "native/file.h"
#include "native/path.h"
#include "native/os.h"

// compilation pipeline
#include "compiler/tokenizer.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "compiler/vm.h"
#include "compiler/compiler.h"
#include "compiler/function.h"
#include "compiler/bytecode.h"


namespace carbon {

inline void carbon_initialize() {
	Logger::initialize();
	Console::initialize();

	// Register native classes.
	NativeClasses::singleton()->register_class<Object>();
	NativeClasses::singleton()->register_class<OS>();
	NativeClasses::singleton()->register_class<File>();
	NativeClasses::singleton()->register_class<Path>();
	NativeClasses::singleton()->register_class<Buffer>();

	NativeClasses::singleton()->register_class<Bytecode>();
	NativeClasses::singleton()->register_class<Function>();
}

inline void carbon_cleanup() {
	Logger::cleanup();
	Console::cleanup();
	NativeClasses::cleanup();
	VM::cleanup();
	Compiler::cleanup();
}

}


#endif // CARBON_H