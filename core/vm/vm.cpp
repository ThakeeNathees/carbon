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

#include "vm.h"

namespace carbon {


int VM::run(ptr<Bytecode> bytecode, stdvec<String> args) {
	
	ptr<CarbonFunction> main = bytecode->get_main();
	if (main == nullptr) {
		ASSERT(false); // TODO: error no main function to run.
	}

	ASSERT(main->get_arg_count() <= 1); // main() or main(args)

	stdvec<var*> call_args; var argv = Array();
	if (main->get_arg_count() == 1) {
		for (const String& str : args) argv.operator Array().push_back(str);
		call_args.push_back(&argv);
	}
	call_carbon_function(main.get(), bytecode, nullptr, call_args);

	return 0;
}

void RuntimeContext::init() {
	_file = bytecode.cast_to<Bytecode>().get();
}

var* RuntimeContext::get_var_at(const Address& p_addr) {
	static var v;

	uint32_t index = p_addr.get_index();
	switch (p_addr.get_type()) {
		case Address::_NULL: {

		} break;
		case Address::STACK: {
			return stack->get_at(index);
		} break;
		case Address::THIS: {
			return &self;
		} break;
		case Address::EXTERN: {
			const String& name = _file->get_global_name(index);
			_file->get_externs()[name];
		} break;
		case Address::NATIVE_CLASS: {

		} break;
		case Address::BUILTIN_FUNC: {

		} break;
		case Address::BUILTIN_TYPE: {

		} break;
		case Address::MEMBER_VAR: {

		} break;
		case Address::STATIC: {

		} break;
		case Address::CONST_VALUE: {

		} break;

		MISSED_ENUM_CHECK(Address::CONST_VALUE, 9);
	}
}

var VM::call_carbon_function(const CarbonFunction* p_func, ptr<Bytecode> p_bytecode, ptr<RuntimeInstance> p_self, stdvec<var*> p_args) {
	Stack stack(p_func->get_stack_size());

	RuntimeContext context;
	context.stack = &stack;
	context.self = p_self;
	context.bytecode = p_bytecode;

	context.init();

	uint32_t ip = 0; // instruction pointer

	const stdvec<uint32_t>& opcodes = p_func->get_opcodes();

	while (ip < opcodes.size()) {

		switch (opcodes[ip]) {
			case Opcode::GET: {
				ASSERT(ip + 3 < opcodes.size());

			} break;
			case Opcode::SET: {
			} break;
			case Opcode::GET_MAPPED: {
			} break;
			case Opcode::SET_MAPPED: {
			} break;
			case Opcode::GET_MEMBER: {
			} break;
			case Opcode::SET_MEMBER: {
			} break;
			case Opcode::SET_TRUE: {
			} break;
			case Opcode::SET_FALSE: {
			} break;
			case Opcode::OPERATOR: {
			} break;
			case Opcode::ASSIGN: {
				ASSERT(ip + 2 < opcodes.size());
				Address src = opcodes[++ip];
				Address dst = opcodes[++ip];



			} break;
			case Opcode::CONSTRUCT_BUILTIN: {
			} break;
			case Opcode::CONSTRUCT_LITERAL_ARRAY: {
			} break;
			case Opcode::CONSTRUCT_LITERAL_DICT: {
			} break;
			case Opcode::CALL_FUNC: {
			} break;
			case Opcode::CALL_METHOD: {
			} break;
			case Opcode::CALL_BUILTIN: {
			} break;
			case Opcode::CALL_SUPER: {
			} break;
			case Opcode::JUMP: {
			} break;
			case Opcode::JUMP_IF: {
			} break;
			case Opcode::JUMP_IF_NOT: {
			} break;
			case Opcode::RETURN: {
			} break;
			case Opcode::ITER_BEGIN: {
			} break;
			case Opcode::ITER_NEXT: {
			} break;
			case Opcode::END: {
			} break;

		}
		MISSED_ENUM_CHECK(Opcode::END, 23);

	}
}

}