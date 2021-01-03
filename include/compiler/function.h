//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "core/native.h"
#include "opcodes.h"

namespace carbon {

class Bytecode;

class Function : public Object {
	REGISTER_CLASS(Function, Object) {}
	friend class CodeGen;

private: // members
	Bytecode* _owner;
	String _name;
	bool _is_static;
	int _arg_count; // TODO: = _is_reference.size(); maybe remove this
	stdvec<bool> _is_reference;
	stdvec<var> _default_args;
	stdvec<uint32_t> _opcodes;
	stdmap<uint32_t, uint32_t> op_dbg; // opcode line to pos
	uint32_t _stack_size;
	
public:
	const String& get_name() const;
	bool is_static() const;
	int get_arg_count() const;
	const stdvec<var>& get_default_args() const;
	const stdvec<bool>& get_is_args_ref() const;
	// TODO: parameter names : only for debugging
	uint32_t get_stack_size() const;
	const Bytecode* get_owner() const;
	
	const stdvec<uint32_t>& get_opcodes() const;
	const stdmap<uint32_t, uint32_t>& get_op_dbg() const;
	String get_opcodes_as_string() const;

	var __call(stdvec<var*>& p_args) override;
};

}

#endif // _FUNCTION_H