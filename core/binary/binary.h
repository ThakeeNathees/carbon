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

#include "core.h"
#include "builtin/builtin_functions.h"
#include "builtin/builtin_types.h"

namespace carbon {

struct Address {
	static constexpr int ADDR_BITS = 32;
	static constexpr int ADDR_TYPE_BITS = 8;
	static constexpr int ADDR_INDEX_BITS = ADDR_BITS - ADDR_TYPE_BITS;
	static constexpr int ADDR_TYPE_MASK = ((1 << ADDR_TYPE_BITS) - 1) << ADDR_INDEX_BITS;
	static constexpr int ADDR_INDEX_MASK = (1 << ADDR_INDEX_BITS) - 1;

	enum Type {
		_NULL = 0,
		STACK,
		THIS,

		//CLASS,       // another local class ref <-- STATIC
		EXTERN,       // cruurent translation unit or imported one
		NATIVE_CLASS, // native class ref
		BUILTIN_FUNC, // builtin function ref
		BUILTIN_TYPE, // builtin type ref

		MEMBER_VAR,   // only member variables with index with offset
		STATIC,       // constant, function, enums, enum value, static vars, static function ... are static var

		CONST_VALUE, // searched in _global_const_values

	};

	Address() {}
	Address(Type p_type, uint32_t p_index, bool p_temp = false) :type(p_type), index(p_index), temp(p_temp) {}

	Address(uint32_t p_addr) {
		type = get_type_s(p_addr);
		index = get_index_s(p_addr);
	}

	static Type get_type_s(uint32_t p_addr) { return  (Type)((p_addr & ADDR_TYPE_MASK) >> ADDR_INDEX_BITS); }
	static uint32_t get_index_s(uint32_t p_addr) { return p_addr & ADDR_INDEX_MASK; }
	static String get_type_name_s(Type p_type);

	Type get_type() const { return type; }
	uint32_t get_index() const { return index; }
	uint32_t get_address() const { return index | (type << ADDR_INDEX_BITS); }
	bool is_temp() const { return temp; }

	String as_string(const stdvec<String>* _global_names_array = nullptr, const stdvec<var>* _global_const_values = nullptr) const;

private:
	Type type = _NULL;
	bool temp = false;
	uint32_t index = 0;
};

enum Opcode {
	GET,
	SET,
	GET_MAPPED,
	SET_MAPPED,
	GET_MEMBER,
	SET_MEMBER,
	SET_TRUE,
	SET_FALSE,

	OPERATOR,
	ASSIGN,

	CONSTRUCT_BUILTIN,
	CONSTRUCT_LITERAL_ARRAY,
	CONSTRUCT_LITERAL_DICT,
	// Native and other types constructed from calling
	CALL_FUNC,
	CALL_METHOD,
	CALL_BUILTIN,
	CALL_SUPER,

	JUMP,
	JUMP_IF,
	JUMP_IF_NOT,
	RETURN,
	ITER_BEGIN,
	ITER_NEXT,

	END,
};

struct Opcodes {
	stdvec<uint32_t> opcodes;

	std::stack<uint32_t> jump_out_if;
	std::stack<uint32_t> jump_out_switch;
	std::stack<uint32_t> jump_out_while;
	std::stack<uint32_t> jump_out_for;
	std::stack<uint32_t> jump_out_foreach;
	std::stack<uint32_t> jump_to_continue;

	std::stack<stdvec<uint32_t>> jump_out_break; // multiple break statement jump out to one addr.

	uint32_t last() { return (uint32_t)opcodes.size() - 1; } // last instruction
	uint32_t next() { return (uint32_t)opcodes.size(); }     // next instruction

	static String get_opcode_name(Opcode p_opcode);

	void insert(uint32_t p_opcode) {
		opcodes.push_back(p_opcode);
	}
	void insert(const Address& p_addr) {
		opcodes.push_back(p_addr.get_address());
	}
	void insert(Opcode p_opcode) {
		opcodes.push_back((uint32_t)p_opcode);
	}

	void write_assign(const Address& dst, const Address& src) {
		insert(Opcode::ASSIGN);
		insert(dst);
		insert(src);
	}

	void write_if(const Address& p_cond) {
		insert(Opcode::JUMP_IF_NOT);
		insert(p_cond);
		insert(0); // addr to jump out of if.

		jump_out_if.push(last());
	}

	void write_else() {
		insert(Opcode::JUMP);
		insert(0); // addr to jump out of else

		// if not jump to next();
		opcodes[jump_out_if.top()] = next();
		jump_out_if.pop();
		jump_out_if.push(last());
	}

	void write_endif() {
		opcodes[jump_out_if.top()] = next();
		jump_out_if.pop();
	}

	void write_while(const Address& p_cond) {
		jump_to_continue.push(next());
		jump_out_break.push(stdvec<uint32_t>());

		insert(Opcode::JUMP_IF_NOT);
		insert(p_cond);
		insert(0); // addr to jump out of while.

		jump_out_while.push(last());
	}

	void write_endwhile() {
		insert(Opcode::JUMP);
		insert(jump_to_continue.top());

		jump_to_continue.pop();
		opcodes[jump_out_while.top()] = next();
		jump_out_while.pop();

		for (uint32_t pos : jump_out_break.top()) {
			opcodes[pos] = next();
		}
		jump_out_break.pop();
	}

	void write_foreach(const Address& p_iterator, const Address& p_on) {

		insert(Opcode::ITER_BEGIN);
		insert(p_iterator);
		insert(p_on);

		jump_to_continue.push(next());
		jump_out_break.push(stdvec<uint32_t>());

		// check if can iterate more
		insert(Opcode::ITER_NEXT);
		insert(p_iterator);
		insert(p_on);
		insert(0); // addr to jump out of foreach;

		jump_out_foreach.push(last());
	}

	void write_endforeach() {
		insert(Opcode::JUMP);
		insert(jump_to_continue.top());

		jump_to_continue.pop();
		opcodes[jump_out_foreach.top()] = next();
		jump_out_foreach.pop();

		for (uint32_t pos : jump_out_break.top()) {
			opcodes[pos] = next();
		}
		jump_out_break.pop();
	}

	void write_for(const Address& p_cond) {

		jump_to_continue.push(next());
		jump_out_break.push(stdvec<uint32_t>());

		if (p_cond.get_type() != Address::_NULL) { // for (;;) {}
			insert(Opcode::JUMP_IF_NOT);
			insert(p_cond);
			insert(0); // addr to jump out of for;

			jump_out_for.push(last());
		}
	}

	void write_endfor(bool p_has_cond) {
		insert(Opcode::JUMP);
		insert(jump_to_continue.top());

		jump_to_continue.pop();

		if (p_has_cond) {
			opcodes[jump_out_for.top()] = next();
			jump_out_for.pop();
		}

		for (uint32_t pos : jump_out_break.top()) {
			opcodes[pos] = next();
		}
		jump_out_break.pop();
	}

	void write_break() {
		insert(Opcode::JUMP);
		insert(0); // addr to jump out.

		jump_out_break.top().push_back(last());
	}

	void write_continue() {
		insert(Opcode::JUMP);
		insert(jump_to_continue.top());
	}

	void write_return(const Address& p_ret_value) {
		insert(Opcode::RETURN);
		insert(p_ret_value);
	}


	/// expressions

	void write_get_member(const Address& p_name, const Address& p_dst) {
		insert(Opcode::GET_MEMBER);
		insert(p_name);
		insert(p_dst);
	}

	void write_get_index(const Address& p_on, uint32_t p_name, const Address& p_dst) {
		insert(Opcode::GET);
		insert(p_on);
		insert(p_name);
		insert(p_dst);
	}

	void write_get_mapped(const Address& p_on, const Address& p_key, const Address& p_dst) {
		insert(Opcode::GET_MAPPED);
		insert(p_on);
		insert(p_key);
		insert(p_dst);
	}

	void write_array_literal(const Address& p_dst, const stdvec<Address>& p_values) {
		insert(Opcode::CONSTRUCT_LITERAL_ARRAY);
		insert((uint32_t)p_values.size());
		for (const Address& addr : p_values) insert(addr);
		insert(p_dst);
	}

	void write_map_literal(const Address& p_dst, const stdvec<Address>& p_keys, const stdvec<Address>& p_values) {
		ASSERT(p_keys.size() == p_values.size());

		insert(Opcode::CONSTRUCT_LITERAL_DICT);
		insert((uint32_t)p_keys.size());
		for (int i = 0; i < (int)p_keys.size(); i++) {
			insert(p_keys[i]);
			insert(p_values[i]);
		}
		insert(p_dst);
	}

	void write_construct_builtin_type(const Address& p_dst, BuiltinTypes::Type p_type, const stdvec<Address>& p_args) {
		insert(Opcode::CONSTRUCT_BUILTIN);
		insert((uint32_t)p_type);
		insert((uint32_t)p_args.size());
		for (const Address& addr : p_args) {
			insert(addr);
		}
		insert(p_dst);
	}

	void write_call_builtin_func(const Address& p_ret, BuiltinFunctions::Type p_func, const stdvec<Address>& p_args) {
		insert(Opcode::CALL_BUILTIN);
		insert((uint32_t)p_func);
		insert((uint32_t)p_args.size());
		for (const Address& addr : p_args) {
			insert(addr);
		}
		insert(p_ret);
	}

	void write_call_func(const Address& p_ret, const Address& p_on, const stdvec<Address>& p_args) {
		insert(Opcode::CALL_FUNC);
		insert(p_on);
		insert((uint32_t)p_args.size());
		for (const Address& addr : p_args) {
			insert(addr);
		}
	}

	void write_call_method(const Address& p_ret, Address& p_on, uint32_t p_method, const stdvec<Address>& p_args) {
		insert(Opcode::CALL_METHOD);
		insert(p_on);
		insert(p_method);
		insert((uint32_t)p_args.size());
		for (const Address& addr : p_args) {
			insert(addr);
		}
	}

};

}

#endif // BINARY_H