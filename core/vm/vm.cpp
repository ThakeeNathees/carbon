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

	printf("%s\n", bytecode->get_function_opcodes_as_string("main").c_str());

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
	static var _null;

	uint32_t index = p_addr.get_index();
	switch (p_addr.get_type()) {
		case Address::_NULL: {
			_null.clear();
			return &_null;
		} break;
		case Address::STACK: {
			return stack->get_at(index);
		} break;
		case Address::THIS: {
			return &self;
		} break;
		case Address::EXTERN: {
			const String& name = _file->get_global_name(index);
			ASSERT(_file->get_externs().find(name) != _file->get_externs().end());
			return _file->_get_member_var_ptr(name);
		} break;
		case Address::NATIVE_CLASS: {
			const String& name = _file->get_global_name(index);
			return vm->_get_native_ref(name);
		} break;
		case Address::BUILTIN_FUNC: {
			return vm->_get_builtin_func_ref(index);
		} break;
		case Address::BUILTIN_TYPE: {
			return vm->_get_builtin_type_ref(index);
		} break;
		case Address::MEMBER_VAR: {
			// ASSERT self is runtime instance
			stdvec<var>& members = self.cast_to<RuntimeInstance>()->members;
			THROW_INVALID_INDEX(members.size(), index);
			return &members[index];
		} break;
		case Address::STATIC: {
			const String& name = _file->get_global_name(index);
			var* member = nullptr;
			if (self.get_type() == var::OBJECT) member = self.cast_to<RuntimeInstance>()->blueprint->_get_member_var_ptr(name);
			if (member == nullptr) member = _file->_get_member_var_ptr(name);
			return member;
		} break;
		case Address::CONST_VALUE: {
			return _file->get_global_const_value(index);
		} break;

		MISSED_ENUM_CHECK(Address::CONST_VALUE, 9);
	}
	THROW_BUG("can't reach here");
}

var* VM::_get_native_ref(const String& p_name) {
	auto it = _native_ref.find(p_name);
	if (it != _native_ref.end()) { return &it->second; }

	ASSERT(NativeClasses::singleton()->is_class_registered(p_name));
	var new_ref = newptr<NativeClassRef>(p_name);
	_native_ref[p_name] = new_ref;
	return &_native_ref[p_name];
}

var* VM::_get_builtin_func_ref(uint32_t p_type) {
	ASSERT(p_type < BuiltinFunctions::Type::_FUNC_MAX_);

	auto it = _builtin_func_ref.find(p_type);
	if (it != _builtin_func_ref.end()) { return &it->second; }

	var new_ref = newptr<BuiltinFuncRef>((BuiltinFunctions::Type)p_type);
	_builtin_func_ref[p_type] = new_ref;
	return &_builtin_func_ref[p_type];
}

var* VM::_get_builtin_type_ref(uint32_t p_type) {
	ASSERT(p_type < BuiltinTypes::Type::_TYPE_MAX_);

	auto it = _builtin_type_ref.find(p_type);
	if (it != _builtin_type_ref.end()) { return &it->second; }

	var new_ref = newptr<BuiltinTypeRef>((BuiltinTypes::Type)p_type);
	_builtin_type_ref[p_type] = new_ref;
	return &_builtin_type_ref[p_type];
}


var VM::call_carbon_function(const CarbonFunction* p_func, ptr<Bytecode> p_bytecode, ptr<RuntimeInstance> p_self, stdvec<var*> p_args) {
	Stack stack(p_func->get_stack_size());

	RuntimeContext context;
	context.vm = this;
	context.stack = &stack;
	if (p_self != nullptr) context.self = p_self;
	context.bytecode = p_bytecode;

	context.init();

	uint32_t ip = 0; // instruction pointer

	const stdvec<uint32_t>& opcodes = p_func->get_opcodes();

#define CHECK_OPCODE_SIZE(m_size) ASSERT(ip + m_size < opcodes.size())
	while (ip < opcodes.size()) {

		switch (opcodes[ip]) {
			case Opcode::GET: {
				CHECK_OPCODE_SIZE(4);

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
				CHECK_OPCODE_SIZE(3);
				var* dst = context.get_var_at(opcodes[++ip]);
				var* src = context.get_var_at(opcodes[++ip]);

				*dst = *src;
				ip++;
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
				CHECK_OPCODE_SIZE(3);

				uint32_t func = opcodes[++ip];
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* ret = context.get_var_at(opcodes[++ip]);
				ASSERT(func < BuiltinFunctions::_FUNC_MAX_);
				BuiltinFunctions::call((BuiltinFunctions::Type)func, args, *ret);
				ip++;
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
				return var();
			} break;

		}
		MISSED_ENUM_CHECK(Opcode::END, 23);

	}
	THROW_BUG("can't reach here");
}

}