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

#ifndef CARBON_FUNCTION_H
#define CARBON_FUNCTION_H

#include "binary.h"

namespace carbon {

class Bytecode;

class CarbonFunction : public Object {
	REGISTER_CLASS(CarbonFunction, Object) {
	}

private:
	friend class CodeGen;
	Bytecode* _owner;

	String _name;
	bool _is_static;
	int _arg_count;
	stdvec<var> _default_args;

	stdvec<uint32_t> _opcodes;
	
public:
	const String& get_name() const { return _name; }
	bool is_static() const { return _is_static; }
	int get_arg_count() const { return _arg_count; }
	const stdvec<var>& get_default_args() const { return _default_args; }
	// parameter names : only in debug build

	// var call(Bytecode* p_self, ...) <-- VM
};

}

#endif // CARBON_FUNCTION_H