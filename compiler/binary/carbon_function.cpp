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

#include "carbon_function.h"
#include "vm.h"

namespace carbon {

#define CHECK_OPCODE_SIZE(m_size) ASSERT(ip + m_size < _opcodes.size())
#define ADD_ADDR() ret += Address(_opcodes[++ip]).as_string(_global_names_array, _global_const_values) + '\n'
#define ADD_GLOBAL_NAME()																					\
	do {                                                                                                    \
		uint32_t index = _opcodes[++ip];																	\
		if (_global_names_array) {																			\
			THROW_INVALID_INDEX(_global_names_array->size(), index);										\
			ret += String(std::to_string(index)) + " // \"" + (*_global_names_array)[index] + "\"\n";		\
		} else {																							\
			ret += std::to_string(index) +"\n";																\
		}																									\
	} while (false)

#define ADD_ADDR_LIST()                                  \
	do {									             \
		uint32_t argc = _opcodes[++ip];		             \
		ret += std::to_string(argc) + " // argc\n";		 \
		for (int i = 0; i < (int)argc; i++) {	         \
			ADD_ADDR();						             \
		}									             \
	} while (false)

#define ADD_ADDR_LIST_MAP()                              \
	do {									             \
		uint32_t argc = _opcodes[++ip];		             \
		ret += std::to_string(argc) + " // argc\n";		 \
		for (int i = 0; i < (int)argc; i++) {	         \
			ADD_ADDR();						             \
			ADD_ADDR();						             \
		}									             \
	} while (false)

//---------------------------------------------------------------------------------------------------


const String& CarbonFunction::get_name() const { return _name; }
bool CarbonFunction::is_static() const { return _is_static; }
int CarbonFunction::get_arg_count() const { return _arg_count; }
const stdvec<var>& CarbonFunction::get_default_args() const { return _default_args; }
uint32_t CarbonFunction::get_stack_size() const { return _stack_size; }
const Bytecode* CarbonFunction::get_owner() const { return _owner; }

const stdvec<uint32_t>& CarbonFunction::get_opcodes() const { return _opcodes; }
const stdmap<uint32_t, uint32_t>& CarbonFunction::get_op_dbg() const { return op_dbg; }


var CarbonFunction::__call(stdvec<var*>& p_args) {
	return VM::singleton()->call_function(this, _owner, nullptr, p_args);
}

String CarbonFunction::get_opcodes_as_string() const {

	Bytecode* _bytecode_file = nullptr;
	if (_owner->is_class()) _bytecode_file = _owner->get_file().get();
	else _bytecode_file = _owner;

	const stdvec<String>* _global_names_array = &_bytecode_file->_global_names_array;
	const stdvec<var>* _global_const_values = &_bytecode_file->_global_const_values;

	String ret;
	uint32_t ip = 0;

	while (ip < _opcodes.size()) {

		if (_opcodes[ip] > Opcode::END) std::cout << ret << std::endl;
		ASSERT(_opcodes[ip] <= Opcode::END);
		ret += String("---- addr:") + std::to_string(ip) + " ----\n";
		ret += Opcodes::get_opcode_name((Opcode)_opcodes[ip]) + '\n';

		switch (_opcodes[ip]) {
			case Opcode::GET: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_GLOBAL_NAME();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::SET: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_GLOBAL_NAME();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::GET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_ADDR();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::SET_MAPPED: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_ADDR();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::SET_TRUE: {
				CHECK_OPCODE_SIZE(2);
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::SET_FALSE: {
				CHECK_OPCODE_SIZE(2);
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::OPERATOR: {
				uint32_t op_type = _opcodes[++ip]; ASSERT(op_type < var::_OP_MAX_);
				ret += String(std::to_string(op_type)) + " // " + var::get_op_name_s((var::Operator)op_type) + '\n';
				ADD_ADDR(); ADD_ADDR(); ADD_ADDR();
				ip++;
			} break;
			case Opcode::ASSIGN: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CONSTRUCT_BUILTIN: {
				CHECK_OPCODE_SIZE(4);
				uint32_t b_type = _opcodes[++ip]; ASSERT(b_type < BuiltinTypes::_TYPE_MAX_);
				ret += String(std::to_string(b_type)) + " // " + BuiltinTypes::get_type_name((BuiltinTypes::Type)b_type) + '\n';
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;

			case Opcode::CONSTRUCT_NATIVE:
			case Opcode::CONSTRUCT_CARBON: {
				CHECK_OPCODE_SIZE(4);
				ADD_GLOBAL_NAME();
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;

			case Opcode::CONSTRUCT_LITERAL_ARRAY: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CONSTRUCT_LITERAL_MAP: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR_LIST_MAP();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CALL: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CALL_FUNC: {
				CHECK_OPCODE_SIZE(4);
				ADD_GLOBAL_NAME();
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CALL_METHOD: {
				CHECK_OPCODE_SIZE(5);
				ADD_ADDR();
				ADD_GLOBAL_NAME();
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CALL_BUILTIN: {
				CHECK_OPCODE_SIZE(3);
				uint32_t func = _opcodes[++ip]; ASSERT(func < BuiltinFunctions::_FUNC_MAX_);
				ret += String(std::to_string(func)) + " // " + BuiltinFunctions::get_func_name((BuiltinFunctions::Type)func) + '\n';
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::CALL_SUPER_CTOR: {
				CHECK_OPCODE_SIZE(2);
				ADD_ADDR_LIST();
				ip++;
			} break;
			case Opcode::CALL_SUPER_METHOD: {
				CHECK_OPCODE_SIZE(5);
				ADD_GLOBAL_NAME();
				ADD_ADDR_LIST();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::JUMP: {
				CHECK_OPCODE_SIZE(2);
				ret += String(std::to_string(_opcodes[++ip])) + " // addr\n";
				ip++;
			} break;
			case Opcode::JUMP_IF: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR();
				ret += String(std::to_string(_opcodes[++ip])) + " // addr\n" ;
				ip++;
			} break;
			case Opcode::JUMP_IF_NOT: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR();
				ret += String(std::to_string(_opcodes[++ip])) + " // addr\n";
				ip++;
			} break;
			case Opcode::RETURN: {
				CHECK_OPCODE_SIZE(2);
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::ITER_BEGIN: {
				CHECK_OPCODE_SIZE(3);
				ADD_ADDR();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::ITER_NEXT: {
				CHECK_OPCODE_SIZE(4);
				ADD_ADDR();
				ADD_ADDR();
				ADD_ADDR();
				ip++;
			} break;
			case Opcode::END: {
				ip++;
			} break;

		}
		ret += "\n";
		MISSED_ENUM_CHECK(Opcode::END, 25);
	}
	return ret;
}

}