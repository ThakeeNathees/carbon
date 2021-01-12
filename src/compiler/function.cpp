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

#include "compiler/function.h"
#include "compiler/vm.h"

namespace carbon {

const String& Function::get_name() const { return _name; }
bool Function::is_static() const { return _is_static; }
int Function::get_arg_count() const { return _arg_count; }
const stdvec<var>& Function::get_default_args() const { return _default_args; }
const stdvec<bool>& Function::get_is_args_ref() const { return _is_reference; }
uint32_t Function::get_stack_size() const { return _stack_size; }
const Bytecode* Function::get_owner() const { return _owner; }

const stdvec<uint32_t>& Function::get_opcodes() const { return _opcodes; }
const stdmap<uint32_t, uint32_t>& Function::get_op_dbg() const { return op_dbg; }


var Function::__call(stdvec<var*>& p_args) {
	return VM::singleton()->call_function(this, _owner, nullptr, p_args);
}

}