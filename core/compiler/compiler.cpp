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

#include "compiler.h"

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
	if (!Path::exists(p_dir) || !Path::is_dir(p_dir)) {
		// TODO: throw error / warning (ignore for now)
	} else {
		_include_dirs.push_back(Path::absolute(p_dir));
	}
}

ptr<Bytecode> Compiler::compile(const String& p_path) {

	if (!Path::exists(p_path)) THROW_ERROR(Error::IO_ERROR, String::format("path \"%s\" does not exists.", p_path.c_str()));

	String path = Path::absolute(p_path);
	auto it = _cache.find(path);
	if (it != _cache.end()) {
		if (it->second.compiling)  throw "TODO: cyclic import found";
		else return it->second.bytecode;

	} else {
		_cache[path] = _Cache();
	}

	String extension = Path::extension(path);
	// TODO: check endswith .cb

	class ScopeDestruct {
	public:
		std::stack<String>* _cwd_ptr = nullptr;
		ScopeDestruct(std::stack<String>* p_cwd_ptr) {
			_cwd_ptr = p_cwd_ptr;
		}
		~ScopeDestruct() {
			Path::set_cwd(_cwd_ptr->top());
			_cwd_ptr->pop();
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&_cwd);

	_cwd.push(Path::get_cwd());
	Path::set_cwd(Path::parent(path)); // TODO: error handle

	ptr<Parser> parser = newptr<Parser>();
	ptr<Analyzer> analyzer = newptr<Analyzer>();
	ptr<CodeGen> codegen = newptr<CodeGen>();

	File file(path, File::READ);
	parser->parse(file.read_text(), path);
	analyzer->analyze(parser);
	ptr<Bytecode> bytecode = codegen->generate(analyzer);

	_cache[path].bytecode = bytecode;
	return bytecode;
}

}
