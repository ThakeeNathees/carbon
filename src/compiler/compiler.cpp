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

#include "compiler/compiler.h"

#include "native/path.h"
#include "native/file.h"
#include "native/os.h"

namespace carbon {

Compiler* Compiler::_singleton = nullptr;
Compiler* Compiler::singleton() {
	if (_singleton == nullptr) _singleton = new Compiler();
	return _singleton;
}

void Compiler::cleanup() {
	if (_singleton != nullptr) delete _singleton;
}

void Compiler::add_flag(CompileFlags p_flag) { _flags |= p_flag; }
void Compiler::add_include_dir(const String& p_dir) {
	if (!Path(p_dir).isdir()) {
		// TODO: throw error / warning (ignore for now)
	} else {
		_include_dirs.push_back(Path(p_dir).absolute());
	}
}

ptr<Bytecode> Compiler::compile_file(const String& p_path) {

	// TODO: remove this
	Logger::log(String::format("compiling: %s\n", p_path.c_str()).c_str());

	class ScopeDestruct {
	public:
		std::stack<String>* _cwd_ptr = nullptr;
		ScopeDestruct(std::stack<String>* p_cwd_ptr) {
			_cwd_ptr = p_cwd_ptr;
		}
		~ScopeDestruct() {
			OS::chdir(_cwd_ptr->top());
			_cwd_ptr->pop();
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&_cwd);
	_cwd.push(OS::getcwd());
	OS::chdir(Path(p_path).parent());

	ptr<File> file = newptr<File>();
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();
	ptr<Analyzer> analyzer = newptr<Analyzer>();
	ptr<CodeGen> codegen = newptr<CodeGen>();
	ptr<Bytecode> bytecode;

	file->open(p_path, File::READ);
	tokenizer->tokenize(file);
	parser->parse(tokenizer);
	analyzer->analyze(parser);
	bytecode = codegen->generate(analyzer);

	file->close();
	for (const Warning& warning : analyzer->get_warnings()) {
		warning.console_log(); // TODO: it shouldn't print, add to warnings list instead.
	}

	return bytecode;
}

ptr<Bytecode> Compiler::compile(const String& p_path) {

	if (!Path(p_path).exists()) THROW_ERROR(Error::IO_ERROR, String::format("path \"%s\" does not exists.", p_path.c_str()));

	String path = Path(p_path).absolute();
	auto it = _cache.find(path);
	if (it != _cache.end()) {
		if (it->second.compiling)  throw "TODO: cyclic import found";
		else return it->second.bytecode;

	} else {
		_cache[path] = _Cache();
	}

	//String extension = Path(path).extension();
	// TODO: check endswith .cb

	ptr<Bytecode> bytecode = compile_file(path);

	_cache[path].bytecode = bytecode;
	return bytecode;
}

}
