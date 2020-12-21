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

#include "core/core.h"

// compilation pipeline
#include "compiler/tokenizer.h"
#include "compiler/parser.h"
#include "compiler/analyzer.h"
#include "compiler/codegen.h"
#include "compiler/vm.h"
#include "compiler/compiler.h"
#include "compiler/function.h"
#include "compiler/bytecode.h"

// native imports
#include "native/file.h"
#include "native/nativelib.h"
#include "native/api/native_struct.h"
#include "native/path.h"
#include "native/os.h"

namespace carbon {

inline void carbon_initialize() {
	Logger::initialize();
	Console::initialize();

	// Register native classes.
	NativeClasses::singleton()->register_class<Object>();
	NativeClasses::singleton()->register_class<OS>();
	NativeClasses::singleton()->register_class<File>();
	NativeClasses::singleton()->register_class<NativeLib>();
	NativeClasses::singleton()->register_class<_NativeStruct>();
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

// =================================================================

inline void log_copyright_and_license() {
	Logger::log(1 + R"(
Carbon 1.0.0 (https://github.com/ThakeeNathees/carbon/)
Copyright (c) 2020 ThakeeNathees.
Free and open source software under the terms of the MIT license.

)");
}

inline void log_help() {
	Logger::log(1 + R"(
usage: carbon [options] file ...
Options:
	--native-api [path] : generate a native api file.
    -h, --help          : Display this help message.
    -v, --version       : Display the version.
    -o                  : Output path.
    -w                  : Warnings are treated as errors.
    -I(path)            : Import search path.
)");
}

}


#endif // CARBON_H