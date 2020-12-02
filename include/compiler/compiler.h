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

#ifndef COMPILER_H
#define COMPILER_H

#include "core/core.h"
#include "native/path.h"
#include "native/file.h"
#include "codegen.h"

namespace carbon {

class Compiler {
public:
	enum CompileFlags {
		// TODO: bitfield
	};

private:
	static Compiler* _singleton;
	struct _Cache {
		bool compiling = true;
		ptr<Bytecode> bytecode = nullptr;
	};
	stdmap<String, _Cache> _cache;
	uint32_t _flags;
	stdvec<String> _include_dirs;
	std::stack<String> _cwd;

	Compiler() {} // private constructor singleton;

public:
	static Compiler* singleton();
	static void cleanup();

	void add_flag(CompileFlags p_flag);
	void add_include_dir(const String& p_dir);
	ptr<Bytecode> compile(const String& p_path);
	ptr<Bytecode> compile_source(const String& p_source, const String& p_path = "<string-soruce>");


};

}

#endif // COMPILER_H