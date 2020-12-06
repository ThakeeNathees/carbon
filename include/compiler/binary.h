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

#ifndef BINARY_H
#define BINARY_H

#include "core/core.h"
#include "builtin_functions.h"
#include "builtin_types.h"

namespace carbon {


enum Opcode {
	GET,
	SET,
	GET_MAPPED,
	SET_MAPPED,
	SET_TRUE,
	SET_FALSE,

	OPERATOR,
	ASSIGN,

	CONSTRUCT_BUILTIN,
	CONSTRUCT_NATIVE,
	CONSTRUCT_CARBON,
	CONSTRUCT_LITERAL_ARRAY,
	CONSTRUCT_LITERAL_MAP,

	// Native and other types constructed from calling
	CALL,                // a_var(...); -> a_var.__call(...);
	CALL_FUNC,           // f(...); calling a function
	CALL_METHOD,         // a.method(...)
	CALL_BUILTIN,        // pritnln(...)
	CALL_SUPER_CTOR,     // super();
	CALL_SUPER_METHOD,   // super.method(...);

	JUMP,
	JUMP_IF,
	JUMP_IF_NOT,
	RETURN,
	ITER_BEGIN,
	ITER_NEXT,

	END,
};

struct Address {
	static constexpr int ADDR_BITS = 32;
	static constexpr int ADDR_TYPE_BITS = 8;
	static constexpr int ADDR_INDEX_BITS = ADDR_BITS - ADDR_TYPE_BITS;
	static constexpr int ADDR_TYPE_MASK = ((1 << ADDR_TYPE_BITS) - 1) << ADDR_INDEX_BITS;
	static constexpr int ADDR_INDEX_MASK = (1 << ADDR_INDEX_BITS) - 1;

	enum Type {
		_NULL = 0,
		STACK,
		PARAMETER,
		THIS,

		EXTERN,         // current translation unit or imported one
		NATIVE_CLASS,   // native class ref
		BUILTIN_FUNC,   // builtin function ref
		BUILTIN_TYPE,   // builtin type ref

		MEMBER_VAR,     // only member variables with index with offset
		STATIC_MEMBER,  // constant, function, enums, enum value, static vars, static function ... are static var

		CONST_VALUE, // searched in _global_const_values

	};

private:
	Type type = _NULL;
	bool temp = false;
	uint32_t index = 0;

public:
	Address() {}
	Address(Type p_type, uint32_t p_index, bool p_temp = false) :type(p_type), index(p_index), temp(p_temp) {}
	Address(uint32_t p_addr) :type(get_type_s(p_addr)), index(get_index_s(p_addr)) {}

	static Type get_type_s(uint32_t p_addr) { return  (Type)((p_addr & ADDR_TYPE_MASK) >> ADDR_INDEX_BITS); }
	static uint32_t get_index_s(uint32_t p_addr) { return p_addr & ADDR_INDEX_MASK; }
	static String get_type_name_s(Type p_type);

	Type get_type() const { return type; }
	uint32_t get_index() const { return index; }
	uint32_t get_address() const { return index | (type << ADDR_INDEX_BITS); }
	bool is_temp() const { return temp; }
	String as_string(const stdvec<String>* _global_names_array = nullptr, const stdvec<var>* _global_const_values = nullptr) const;

	bool operator==(const Address& p_other) const { return type == p_other.type && index == p_other.index; }
	bool operator!=(const Address& p_other) const { return !operator==(p_other); }
};

struct Opcodes {
	stdvec<uint32_t> opcodes;

	stdmap<uint32_t, uint32_t>* op_dbg; // opcode index to line

	std::stack<uint32_t> jump_out_if;
	std::stack<uint32_t> jump_out_switch;
	std::stack<uint32_t> jump_out_while;
	std::stack<uint32_t> jump_out_for;
	std::stack<uint32_t> jump_out_foreach;
	std::stack<uint32_t> jump_to_continue;
	std::stack<uint32_t> jump_out_and;
	std::stack<uint32_t> jump_out_or;
	std::stack<stdvec<uint32_t>> jump_out_break; // multiple break statement jump out to one addr.

	inline uint32_t last(); // last instruction
	inline uint32_t next(); // next instruction

	static String get_opcode_name(Opcode p_opcode);
	void insert_dbg(uint32_t p_line);

	inline void insert(uint32_t p_opcode);
	inline void insert(const Address& p_addr);
	inline void insert(Opcode p_opcode);

	void write_assign(const Address& dst, const Address& src);
	void write_if(const Address& p_cond);
	void write_else();
	void write_endif();
	void write_while(const Address& p_cond);
	void write_endwhile();
	void write_foreach(const Address& p_iter_value, const Address& p_iterator, const Address& p_on);
	void write_endforeach();
	void write_for(const Address& p_cond);
	void write_endfor(bool p_has_cond);
	void write_break();
	void write_continue();
	void write_return(const Address& p_ret_value);

	void write_assign_bool(const Address& dst, bool value);
	void write_and_left(const Address& p_left);
	void write_and_right(const Address& p_right, const Address& p_dst);
	void write_or_left(const Address& p_left);
	void write_or_right(const Address& p_right, const Address& p_dst);

	//void write_get_member(const Address& p_name, const Address& p_dst);
	void write_get_index(const Address& p_on, uint32_t p_name, const Address& p_dst);
	void write_set_index(const Address& p_on, uint32_t p_name, const Address& p_value);
	void write_get_mapped(const Address& p_on, const Address& p_key, const Address& p_dst);
	void write_set_mapped(const Address& p_on, const Address& p_key, const Address& p_value);
	void write_array_literal(const Address& p_dst, const stdvec<Address>& p_values);
	void write_map_literal(const Address& p_dst, const stdvec<Address>& p_keys, const stdvec<Address>& p_values);
	void write_construct_builtin_type(const Address& p_dst, BuiltinTypes::Type p_type, const stdvec<Address>& p_args);
	void write_construct_native(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args);
	void write_construct_carbon(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args);
	void write_call_builtin(const Address& p_ret, BuiltinFunctions::Type p_func, const stdvec<Address>& p_args);
	void write_call(const Address& p_ret, const Address& p_on, const stdvec<Address>& p_args);
	void write_call_func(const Address& p_ret, uint32_t p_name, const stdvec<Address>& p_args);
	void write_call_method(const Address& p_ret, Address& p_on, uint32_t p_method, const stdvec<Address>& p_args);
	void write_call_super_constructor(const stdvec<Address>& p_args);
	void write_call_super_method(const Address& p_ret, uint32_t p_method, const stdvec<Address>& p_args);
	void write_operator(const Address& p_dst, var::Operator p_op, const Address& p_left, const Address& p_right);

};

}

#endif // BINARY_H