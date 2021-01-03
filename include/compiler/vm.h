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

#ifndef VM_H
#define VM_H

#include "var/var.h"
#include "builtin.h"
#include "bytecode.h"
#include "function.h"
#include "instance.h"

namespace carbon {

class VM;

class VMStack {
private: // members
	ptr<stdvec<var>> _stack;

public:
	VMStack(uint32_t p_max_size = 0);
	var* get_at(uint32_t p_pos);
};

struct RuntimeContext {
	VM* vm = nullptr;
	VMStack* stack = nullptr;
	stdvec<var*>* args = nullptr;
	var self;
	Bytecode* bytecode_class = nullptr;  // static member reference
	Bytecode* bytecode_file = nullptr;   // file node blueprint

	const Function* curr_fn = nullptr; // current function
	stdvec<var> value_args;                  // for pass by value args
	int get_arg_value_index(int p_arg_ind) const;

	var* get_var_at(const Address& p_addr);
	const String& get_name_at(uint32_t p_pos);
};

class VM {
	friend struct RuntimeContext;

private: // members
	stdmap<String, var> _native_ref;
	stdmap<uint32_t, var> _builtin_func_ref;
	stdmap<uint32_t, var> _builtin_type_ref;

public:
	int run(ptr<Bytecode> bytecode, stdvec<String> args);
	var call_function(const String& p_func_name, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args);
	var call_function(const Function* p_func, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args, int __stack = 0);

	static VM* singleton();
	static void cleanup();

private:
	VM() {} // singleton
	var* _get_native_ref(const String& p_name);
	var* _get_builtin_func_ref(uint32_t p_type);
	var* _get_builtin_type_ref(uint32_t p_type);

	static VM* _singleton;
	const int STACK_MAX = 1024; // TODO: increase

};

}

#endif // VM_H