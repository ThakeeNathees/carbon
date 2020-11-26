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

VM* VM::_singleton = nullptr;
VM* VM::singleton() {
	if (_singleton == nullptr) _singleton = new VM();
	return _singleton;
}
void VM::cleanup() {
	if (_singleton != nullptr) delete _singleton;
}

var Instance::call_method(const String& p_method_name, stdvec<var*>& p_args) {

	CarbonFunction* fn = nullptr;
	Bytecode* _class = blueprint.get();
	while (_class) {
		auto& functions = _class->get_functions();
		auto it = functions.find(p_method_name);
		if (it != functions.end()) {
			fn = it->second.get();
			break;
		} else {
			if (!_class->has_base()) THROW_ERROR(Error::ATTRIBUTE_ERROR, "TODO: method not found msg"); // TODO: throw error with debug info.
			if (_class->is_base_native()) {
				BindData* bd = NativeClasses::singleton()->get_bind_data(_class->get_base_native(), p_method_name).get();
				switch (bd->get_type()) {
					case BindData::METHOD:
						return static_cast<MethodBind*>(bd)->call(native_instance, p_args);
					case BindData::STATIC_FUNC:
						return static_cast<StaticFuncBind*>(bd)->call(p_args);
					case BindData::MEMBER_VAR:
						return static_cast<PropertyBind*>(bd)->get(native_instance).__call(p_args);
					case BindData::STATIC_VAR:
						return static_cast<StaticPropertyBind*>(bd)->get().__call(p_args);
					case BindData::STATIC_CONST:
						return static_cast<ConstantBind*>(bd)->get().__call(p_args);
					case BindData::ENUM:
						return static_cast<EnumBind*>(bd)->get()->__call(p_args);
					case BindData::ENUM_VALUE:
						THROW_BUG("TODO: enum value not callable error msg here.");
				}
				THROW_BUG("can't reach here");
				return var();
			} else {
				_class = _class->get_base_binary().get();
			}
		}
	}

	ASSERT(fn != nullptr);
	if (fn->is_static()) { // calling static method using instance (acceptable)
		return VM::singleton()->call_carbon_function(fn, _class, nullptr, p_args);
	} else {
		return VM::singleton()->call_carbon_function(fn, _class, shared_from_this(), p_args);
	}
}

var Instance::__call(stdvec<var*>& p_args) {
	//	TODO:
	return var();
}

String Instance::to_string() {
	ptr<CarbonFunction> fn = blueprint->get_function("to_string");
	if (fn != nullptr) return VM::singleton()->call_carbon_function(fn.get(), blueprint.get(), shared_from_this(), stdvec<var*>());
	return Super::to_string();
}

var VM::call_carbon_function(const CarbonFunction* p_func, Bytecode* p_bytecode, ptr<Instance> p_self, stdvec<var*> p_args) {

	p_bytecode->initialize();

	Stack stack(p_func->get_stack_size());
	RuntimeContext context;
	context.vm = this;
	context.stack = &stack;
	context.args = &p_args;
	if (p_self != nullptr) context.self = p_self;
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
		THROW_BUG("TODO: too many arguments error msg here");
	} else if (p_args.size() < p_func->get_arg_count()) {
		const stdvec<var>& defaults = p_func->get_default_args();
		if (p_args.size() + defaults.size() < p_func->get_arg_count()) {
			THROW_BUG("TODO: too few arguments error msg here");
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
					case var::OP_POSITIVE: { *dst = *left; /* TODO: anything? */ } break;
					case var::OP_NEGATIVE: {
						if (left->get_type() == var::INT) {
							*dst = -left->operator int64_t();
						} else if (left->get_type() == var::FLOAT) {
							*dst = -left->operator double();
						} else {
							THROW_BUG("TODO: throw runtime error.");
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

				const CarbonFunction* member_initializer = blueprint->get_member_initializer();
				if (member_initializer) call_carbon_function(member_initializer, blueprint.get(), instance, stdvec<var*>());

				const CarbonFunction* constructor = blueprint->get_constructor();
				if (constructor) call_carbon_function(constructor, blueprint.get(), instance, args);

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
				ptr<CarbonFunction> func_ptr;

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
					auto it = call_base->get_functions().find(func);
					if (it != call_base->get_functions().end() && it->second->is_static()) {
						func_ptr = it->second;
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

				if (func_ptr == nullptr) THROW_BUG("can't find the function");

				*ret_value = call_carbon_function(func_ptr.get(), call_base, (func_ptr->is_static()) ? nullptr : p_self, args);

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

					const CarbonFunction* member_initializer = p_bytecode->get_base_binary()->get_member_initializer();
					if (member_initializer) call_carbon_function(member_initializer, p_bytecode->get_base_binary().get(), p_self, stdvec<var*>());

					const CarbonFunction* ctor = p_bytecode->get_base_binary()->get_constructor();
					if (ctor) call_carbon_function(ctor, p_bytecode->get_base_binary().get(), p_self, args);
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

		}
		MISSED_ENUM_CHECK(Opcode::END, 24);

	}
	THROW_BUG("can't reach here");
}

int VM::run(ptr<Bytecode> bytecode, stdvec<String> args) {
	
	const CarbonFunction* main = bytecode->get_main();
	if (main == nullptr) {
		THROW_ERROR(Error::NULL_POINTER, "entry point was null");
	}

	//printf("%s\n", main->get_opcodes_as_string().c_str());

	ASSERT(main->get_arg_count() <= 1); // main() or main(args)

	stdvec<var*> call_args; var argv = Array();
	if (main->get_arg_count() == 1) {
		for (const String& str : args) argv.operator Array().push_back(str);
		call_args.push_back(&argv);
	}
	var main_ret = call_carbon_function(main, bytecode.get(), nullptr, call_args);

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
			return (*args)[index];
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
			// TODO: ASSERT self is runtime instance
			stdvec<var>& members = self.cast_to<Instance>()->members;
			THROW_INVALID_INDEX(members.size(), index);
			return &members[index];
		} break;
		case Address::STATIC_MEMBER: {
			const String& name = get_name_at(index);
			var* member = nullptr;
			if (self.get_type() != var::_NULL) member = self.cast_to<Instance>()->blueprint->_get_member_var_ptr(name);
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