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

#include "compiler/vm.h"

namespace carbon {

VM* VM::_singleton = nullptr;
VM* VM::singleton() {
	if (_singleton == nullptr) _singleton = new VM();
	return _singleton;
}

void VM::cleanup() {
	if (_singleton != nullptr) delete _singleton;
}

VMStack::VMStack(uint32_t p_max_size) {
	_stack = newptr<stdvec<var>>(p_max_size);
}
var* VMStack::get_at(uint32_t p_pos) {
	THROW_INVALID_INDEX(_stack->size(), p_pos);
	return &(*_stack)[p_pos];
}

var VM::call_function(const String& p_func_name, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args) {
	THROW_IF_NULLPTR(p_bytecode);
	Function* p_func = p_bytecode->get_function(p_func_name).get();
	return call_function(p_func, p_bytecode, p_self, p_args);
}

var VM::call_function(const Function* p_func, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*>& p_args, int __stack) {

	THROW_IF_NULLPTR(p_func);
	THROW_IF_NULLPTR(p_bytecode);
	if (__stack >= STACK_MAX) THROW_ERROR(Error::STACK_OVERFLOW, "stack was overflowed.");

	p_bytecode->initialize();

	VMStack stack(p_func->get_stack_size());
	RuntimeContext context;
	context.vm = this;
	context.stack = &stack;
	context.args = &p_args;
	if (p_self != nullptr) context.self = p_self;
	context.curr_fn = p_func;
	for (int i = 0; i < p_func->get_is_args_ref().size(); i++) {
		if (!p_func->get_is_args_ref()[i]) context.value_args.push_back(*(p_args[i]));
	}
	if (p_bytecode->is_class()) {
		context.bytecode_class = p_bytecode;
		context.bytecode_file = p_bytecode->get_file().get();
	} else {
		context.bytecode_file = p_bytecode;
	}

	uint32_t ip = 0; // instruction pointer

	const stdvec<uint32_t>& opcodes = p_func->get_opcodes();

	// check argc and add default args
	stdvec<var> default_args_copy;
	if (p_args.size() > p_func->get_arg_count()) {
		THROW_ERROR(Error::INVALID_ARG_COUNT,
			String::format("too many arguments were provided, expected at most %i argument(s).", p_func->get_arg_count()));
	} else if (p_args.size() < p_func->get_arg_count()) {
		const stdvec<var>& defaults = p_func->get_default_args();
		if (p_args.size() + defaults.size() < p_func->get_arg_count()) {
			THROW_ERROR(Error::INVALID_ARG_COUNT,
				String::format("too few arguments were provided, expected at least %i argument(s).", p_func->get_arg_count() - (int)defaults.size()));
		}

		int args_needed = p_func->get_arg_count() - (int)p_args.size();
		while (args_needed > 0) {
			default_args_copy.push_back(defaults[defaults.size() - args_needed--]);
		}
		for (var& v : default_args_copy) p_args.push_back(&v);
	}

#define CHECK_OPCODE_SIZE(m_size) ASSERT(ip + m_size < opcodes.size())
	while (ip < opcodes.size()) {
		ASSERT(opcodes[ip] <= Opcode::END);
		uint32_t last_ip = ip;
		try {
			/* TODO: debugging  -------------------*/
			//if (debugger.is_active()) {
			//	auto it = p_func->get_op_dbg().lower_bound(last_ip);
			//	if (it != p_func->get_op_dbg().end()) {
			//		String file = context.bytecode_file->get_name();
			//		//if (p_func->get_owner() != nullptr && p_func->get_owner()->is_class()) {
			//		//	func = p_func->get_owner()->get_name() + "." + p_func->get_name();
			//		//} else {
			//		//	func = p_func->get_name();
			//		//}
			//		uint32_t line = (uint32_t)it->second;
			//		debugger.debug(file, line);
			//	}
			//}
			/* ---------------------------------- */

		switch (opcodes[ip]) {
			case Opcode::GET: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				const String& name = context.get_name_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = on->get_member(name);
			} break;

			case Opcode::SET: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				const String& name = context.get_name_at(opcodes[++ip]);
				var* value = context.get_var_at(opcodes[++ip]);
				ip++;

				on->set_member(name, *value);
			} break;

			case Opcode::GET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				var* key = context.get_var_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = on->__get_mapped(*key);
			} break;

			case Opcode::SET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				var* key = context.get_var_at(opcodes[++ip]);
				var* value = context.get_var_at(opcodes[++ip]);
				ip++;

				on->__set_mapped(*key, *value);
			} break;

			case Opcode::SET_TRUE: {
				CHECK_OPCODE_SIZE(2);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = true;
			} break;

			case Opcode::SET_FALSE: {
				CHECK_OPCODE_SIZE(2);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = false;
			} break;

			case Opcode::OPERATOR: {
				CHECK_OPCODE_SIZE(5);
				ASSERT(opcodes[ip + 1] < var::_OP_MAX_);
				var::Operator op = (var::Operator)opcodes[++ip];
				var* left = context.get_var_at(opcodes[++ip]);
				var* right = context.get_var_at(opcodes[++ip]);
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				switch (op) {
					case var::OP_ASSIGNMENT: {
						THROW_BUG("assignment operations should be under ASSIGN opcode");
					};
					case var::OP_ADDITION: { *dst = *left + *right; } break;
					case var::OP_SUBTRACTION: { *dst = *left - *right; } break;
					case var::OP_MULTIPLICATION: { *dst = *left * *right; } break;
					case var::OP_DIVISION: { *dst = *left / *right; } break;
					case var::OP_MODULO: { *dst = *left % *right; } break;
					case var::OP_POSITIVE: { *dst = *left; /* is it okey? */ } break;
					case var::OP_NEGATIVE: {
						if (left->get_type() == var::INT) {
							*dst = -left->operator int64_t();
						} else if (left->get_type() == var::FLOAT) {
							*dst = -left->operator double();
						} else {
							THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,
								String::format("operator (-) not supported on base %s.", left->get_type_name().c_str()));
						}
					} break;
					case var::OP_EQ_CHECK: { *dst = *left == *right; } break;
					case var::OP_NOT_EQ_CHECK: { *dst = *left != *right; } break;
					case var::OP_LT: { *dst = *left < *right; } break;
					case var::OP_LTEQ: { *dst = *left <= *right; } break;
					case var::OP_GT: { *dst = *left > * right; } break;
					case var::OP_GTEQ: { *dst = *left >= *right; } break;
					case var::OP_AND: { *dst = *left && *right; } break;
					case var::OP_OR: { *dst = *left || *right; } break;
					case var::OP_NOT: { *dst = !*left; } break;
					case var::OP_BIT_LSHIFT: { *dst = left->operator int64_t() << right->operator int64_t(); } break;
					case var::OP_BIT_RSHIFT: { *dst = left->operator int64_t() >> right->operator int64_t(); } break;
					case var::OP_BIT_AND: { *dst = left->operator int64_t() & right->operator int64_t(); } break;
					case var::OP_BIT_OR: { *dst = left->operator int64_t() | right->operator int64_t(); } break;
					case var::OP_BIT_XOR: { *dst = left->operator int64_t() ^ right->operator int64_t(); } break;
					case var::OP_BIT_NOT: { *dst = ~left->operator int64_t(); } break;
				}
			} break;

			case Opcode::ASSIGN: {
				CHECK_OPCODE_SIZE(3);
				var* dst = context.get_var_at(opcodes[++ip]);
				var* src = context.get_var_at(opcodes[++ip]);

				*dst = *src;
				ip++;
			} break;

			case Opcode::CONSTRUCT_BUILTIN: {
				CHECK_OPCODE_SIZE(4);
				uint32_t b_type = opcodes[++ip];
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				ASSERT(b_type < BuiltinTypes::_TYPE_MAX_);
				*dst = BuiltinTypes::construct((BuiltinTypes::Type)b_type, args);
			} break;

			case Opcode::CONSTRUCT_NATIVE: {
				CHECK_OPCODE_SIZE(4);
				const String& class_name = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = NativeClasses::singleton()->construct(class_name, args);
			} break;

			case Opcode::CONSTRUCT_CARBON: {
				CHECK_OPCODE_SIZE(4);
				const String& name = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				stdmap<String, ptr<Bytecode>>* classes;
				if (p_bytecode->is_class()) classes = &p_bytecode->get_file()->get_classes();
				else classes = &p_bytecode->get_classes();
				if (classes->find(name) == classes->end()) THROW_BUG("cannot find classes the class");

				ptr<Bytecode> blueprint = classes->at(name);
				ptr<Instance> instance = newptr<Instance>(blueprint);

				const Function* member_initializer = blueprint->get_member_initializer();
				stdvec<var*> _args;
				if (member_initializer) call_function(member_initializer, blueprint.get(), instance, _args, __stack + 1);

				const Function* constructor = blueprint->get_constructor();
				if (constructor) call_function(constructor, blueprint.get(), instance, args, __stack + 1);

				*dst = instance;
			} break;

			case Opcode::CONSTRUCT_LITERAL_ARRAY: {
				CHECK_OPCODE_SIZE(3);
				uint32_t size = opcodes[++ip];

				Array arr;
				for (int i = 0; i < (int)size; i++) {
					var* value = context.get_var_at(opcodes[++ip]);
					arr.push_back(*value);
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = arr;
			} break;

			case Opcode::CONSTRUCT_LITERAL_MAP: {
				CHECK_OPCODE_SIZE(3);
				uint32_t size = opcodes[++ip];

				Map map;
				for (int i = 0; i < (int)size; i++) {
					var* key = context.get_var_at(opcodes[++ip]);
					var* value = context.get_var_at(opcodes[++ip]);
					map[*key] = *value;
				}
				var* dst = context.get_var_at(opcodes[++ip]);
				ip++;

				*dst = map;
			} break;

			case Opcode::CALL: {
				CHECK_OPCODE_SIZE(4);
				var* on = context.get_var_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				*ret_value = on->__call(args);
			} break;

			case Opcode::CALL_FUNC: {
				CHECK_OPCODE_SIZE(4);

				const String& func = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				Bytecode* call_base;
				ptr<Function> func_ptr;

				// first search through inheritance
				if (p_self != nullptr) {
					call_base = p_self->blueprint.get();
					while (call_base != nullptr) {
						auto it = call_base->get_functions().find(func);
						if (it != call_base->get_functions().end()) {
							func_ptr = it->second;
							break;
						}
						call_base = call_base->get_base_binary().get();
					}

				// search in static class functions
				} else if (p_bytecode->is_class()) {
					call_base = p_bytecode;
					func_ptr = call_base->get_function(func);
					if (func_ptr != nullptr && !func_ptr->is_static() && p_func->is_static()) {
						THROW_BUG("can't call a non static function from static function");
					}
				}

				// search in the file
				if (func_ptr == nullptr) {
					if (p_bytecode->is_class()) call_base = p_bytecode->get_file().get();
					else call_base = p_bytecode;
				
					auto& functions = call_base->get_functions();
					auto it = functions.find(func);
					if (it != functions.end()) func_ptr = it->second;
				}

				//if (func_ptr == nullptr) THROW_BUG(String::format("can't find the function \"%s\"", func.c_str()));
				if (func_ptr == nullptr) {
					//*ret_value = NativeClasses::singleton()->call_method_on(p_self->native_instance, func, args);
					*ret_value = Object::call_method_s(p_self->native_instance, func, args);
				} else {
					*ret_value = call_function(func_ptr.get(), call_base, (func_ptr->is_static()) ? nullptr : p_self, args, __stack + 1);
				}


			} break;

			case Opcode::CALL_METHOD: {
				CHECK_OPCODE_SIZE(5);

				var* on = context.get_var_at(opcodes[++ip]);
				const String& method = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				*ret_value = on->call_method(method, args);

			} break;

			case Opcode::CALL_BUILTIN: {
				CHECK_OPCODE_SIZE(4);

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

			case Opcode::CALL_SUPER_CTOR: {
				CHECK_OPCODE_SIZE(2);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;

				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				ip++;

				ASSERT(p_bytecode->is_class());
				if (p_bytecode->is_base_native()) {
					p_self->native_instance = NativeClasses::singleton()->construct(p_bytecode->get_base_native(), args);
				} else {

					const Function* member_initializer = p_bytecode->get_base_binary()->get_member_initializer();
					if (member_initializer) {
						stdvec<var*> _args;
						call_function(member_initializer, p_bytecode->get_base_binary().get(), p_self, _args, __stack + 1);
					}

					const Function* ctor = p_bytecode->get_base_binary()->get_constructor();
					if (ctor) call_function(ctor, p_bytecode->get_base_binary().get(), p_self, args, __stack + 1);
				}

			} break;

			case Opcode::CALL_SUPER_METHOD: {
				CHECK_OPCODE_SIZE(4);

				const String& method = context.get_name_at(opcodes[++ip]);
				uint32_t argc = opcodes[++ip];
				stdvec<var*> args;
				for (int i = 0; i < (int)argc; i++) {
					var* arg = context.get_var_at(opcodes[++ip]);
					args.push_back(arg);
				}
				var* ret_value = context.get_var_at(opcodes[++ip]);
				ip++;

				ASSERT(p_self->blueprint->has_base());

				if (p_self->blueprint->is_base_native()) {
					//ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(p_self->blueprint->get_base_native(), method);
					*ret_value = NativeClasses::singleton()->call_method_on(p_self->native_instance, method, args);
				} else {
					ptr<Function> fn = p_self->blueprint->get_base_binary()->get_function(method);
					var* sv = nullptr; if (fn == nullptr) sv = p_self->blueprint->get_base_binary()->get_static_var(method);

					if (fn != nullptr) {
						if (fn->is_static()) {
							*ret_value = call_function(fn.get(), p_self->blueprint.get(), p_self, args, __stack + 1);
						} else {
							if (p_self == nullptr) THROW_BUG("can't call non-static method statically");
							*ret_value = call_function(fn.get(), p_self->blueprint.get(), nullptr, args, __stack + 1);
						}
					} else if (sv != nullptr) {
						*ret_value = sv->__call(args);
					} else {
						THROW_BUG("attribute not found in super");
					}
				}

			} break;

			case Opcode::JUMP: {
				CHECK_OPCODE_SIZE(2);
				uint32_t addr = opcodes[++ip];
				ip = addr;
			} break;

			case Opcode::JUMP_IF: {
				CHECK_OPCODE_SIZE(3);
				var* cond = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];
				if (cond->operator bool()) ip = addr;
				else ip++;
			} break;

			case Opcode::JUMP_IF_NOT: {
				CHECK_OPCODE_SIZE(3);
				var* cond = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];
				if (!cond->operator bool()) ip = addr;
				else ip++;
			} break;

			case Opcode::RETURN: {
				CHECK_OPCODE_SIZE(2);
				var* val = context.get_var_at(opcodes[++ip]);
				return *val;
			} break;

			case Opcode::ITER_BEGIN: {
				CHECK_OPCODE_SIZE(3);
				var* iterator = context.get_var_at(opcodes[++ip]);
				var* on = context.get_var_at(opcodes[++ip]);
				ip++;

				*iterator = on->__iter_begin();
			} break;
			case Opcode::ITER_NEXT: {
				CHECK_OPCODE_SIZE(4);
				var* iter_value = context.get_var_at(opcodes[++ip]);
				var* iterator = context.get_var_at(opcodes[++ip]);
				uint32_t addr = opcodes[++ip];

				if (iterator->__iter_has_next()) {
					*iter_value = iterator->__iter_next();
					ip++;
				} else {
					ip = addr;
				}

			} break;
			case Opcode::END: {
				return var();
			} break;
			MISSED_ENUM_CHECK(Opcode::END, 25);

		}} catch (Throwable& err) {
			ptr<Throwable> nested;
			switch (err.get_kind()) {
				case Throwable::ERROR:
					nested = newptr<Error>(static_cast<Error&>(err));
					break;
				case Throwable::COMPILE_TIME:
					nested = newptr<CompileTimeError>(static_cast<CompileTimeError&>(err));
					break;
				case Throwable::WARNING:
					nested = newptr<Warning>(static_cast<Warning&>(err));
					break;
				case Throwable::TRACEBACK:
					nested = newptr<TraceBack>(static_cast<TraceBack&>(err));
					break;
			}

			auto it = p_func->get_op_dbg().lower_bound(last_ip);
			uint32_t line = (it != p_func->get_op_dbg().end()) ? line = (uint32_t)it->second : 0;
			String func;
			if (p_func->get_owner() != nullptr && p_func->get_owner()->is_class()) {
				func = p_func->get_owner()->get_name() + "." + p_func->get_name();
			} else {
				func = p_func->get_name();
			}

			throw TraceBack(nested, DBGSourceInfo(context.bytecode_file->get_name(), line, func), _DBG_SOURCE);
		}

	}
	THROW_BUG("can't reach here");
}

int VM::run(ptr<Bytecode> bytecode, stdvec<String> args) {

	const Function* main = bytecode->get_main();
	if (main == nullptr) {
		THROW_ERROR(Error::NULL_POINTER, "entry point was null");
	}

	// TODO: temp debugging code
	//printf("%s\n", main->get_opcodes_as_string().c_str());

	ASSERT(main->get_arg_count() <= 1); // main() or main(args)

	stdvec<var*> call_args; var argv = Array();
	if (main->get_arg_count() == 1) {
		for (const String& str : args) argv.operator Array().push_back(str);
		call_args.push_back(&argv);
	}
	var main_ret = call_function(main, bytecode.get(), nullptr, call_args);

	if (main_ret.get_type() == var::_NULL) return 0;
	if (main_ret.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "main function returned a non integer value");
	return main_ret.operator int();
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

		case Address::PARAMETER: {
			ASSERT(curr_fn != nullptr && curr_fn->get_is_args_ref().size() > index);
			int value_index = get_arg_value_index(index);
			if (value_index < 0) { // argument is reference
				return (*args)[index];
			} else { // argument is reference
				return &value_args[value_index];
			}
		} break;

		case Address::THIS: {
			return &self;
		} break;
		case Address::EXTERN: {
			const String& name = get_name_at(index);
			ASSERT(bytecode_file->get_externs().find(name) != bytecode_file->get_externs().end());
			return bytecode_file->_get_member_var_ptr(name);
		} break;
		case Address::NATIVE_CLASS: {
			const String& name = get_name_at(index);
			return vm->_get_native_ref(name);
		} break;
		case Address::BUILTIN_FUNC: {
			return vm->_get_builtin_func_ref(index);
		} break;
		case Address::BUILTIN_TYPE: {
			return vm->_get_builtin_type_ref(index);
		} break;
		case Address::MEMBER_VAR: {
			stdvec<var>& members = self.operator ptr<Instance>()->members;
			THROW_INVALID_INDEX(members.size(), index);
			return &members[index];
		} break;
		case Address::STATIC_MEMBER: {
			const String& name = get_name_at(index);
			var* member = nullptr;
			if (self.get_type() != var::_NULL) member = self.operator ptr<Instance>()->blueprint->_get_member_var_ptr(name);
			if (!member && bytecode_class) member = bytecode_class->_get_member_var_ptr(name);
			if (!member) member = bytecode_file->_get_member_var_ptr(name);
			return member;
		} break;
		case Address::CONST_VALUE: {
			return bytecode_file->get_global_const_value(index);
		} break;

		MISSED_ENUM_CHECK(Address::CONST_VALUE, 10);
	}
	THROW_BUG("can't reach here");
}

int RuntimeContext::get_arg_value_index(int p_arg_ind) const {
	auto& is_ref = curr_fn->get_is_args_ref();
	int value_index = -1;
	for (int i = 0; i < is_ref.size(); i++) {
		if (!is_ref[i]) value_index++;
		if (i == p_arg_ind) return (is_ref[i]) ? -1 : value_index;
	}
	return -1;
}

const String& RuntimeContext::get_name_at(uint32_t p_pos) {
	return bytecode_file->get_global_name(p_pos);
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

}