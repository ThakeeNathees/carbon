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

#include "compiler/opcodes.h"

namespace carbon {

String Opcodes::get_opcode_name(Opcode p_opcode) {
	static const char* _names[] = {
		"GET",
		"SET",
		"GET_MAPPED",
		"SET_MAPPED",
		"SET_TRUE",
		"SET_FALSE",
		"OPERATOR",
		"ASSIGN",
		"CONSTRUCT_BUILTIN",
		"CONSTRUCT_NATIVE",
		"CONSTRUCT_CARBON",
		"CONSTRUCT_LITERAL_ARRAY",
		"CONSTRUCT_LITERAL_DICT",
		"CALL",
		"CALL_FUNC",
		"CALL_METHOD",
		"CALL_BUILTIN",
		"CALL_SUPER_CTOR",
		"CALL_SUPER_METHOD",
		"JUMP",
		"JUMP_IF",
		"JUMP_IF_NOT",
		"RETURN",
		"ITER_BEGIN",
		"ITER_NEXT",
		"END",
	};
	MISSED_ENUM_CHECK(END, 25);
	return _names[p_opcode];
}

String Address::get_type_name_s(Address::Type p_type) {
	static const char* _names[] = {
		"Address::_NULL",
		"Address::STACK",
		"Address::PARAMETER",
		"Address::THIS",
		"Address::EXTERN",
		"Address::NATIVE_CLASS",
		"Address::BUILTIN_FUNC",
		"Address::BUILTIN_TYPE",
		"Address::MEMBER_VAR",
		"Address::STATIC_MEMBER",
		"Address::CONST_VALUE",
	};
	MISSED_ENUM_CHECK(CONST_VALUE, 10);
	return _names[p_type];
}

String Address::as_string(const stdvec<String>* _global_names_array, const stdvec<var>* _global_const_values) const {

	switch (type) {
		case _NULL:
		case THIS:
			return get_type_name_s(type);

		case STACK:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";

		case PARAMETER:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";

		case EXTERN:
		case NATIVE_CLASS:
		case STATIC_MEMBER:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")" + (
				(_global_names_array) ? String(" // \"") + (*_global_names_array)[index] + "\"" : "");
		case CONST_VALUE:
			if (_global_const_values) {
				if ((*_global_const_values)[index].get_type() == var::STRING) {
					return get_type_name_s(type) + "(" + std::to_string(index) + ")" + String(" // \"") + (*_global_const_values)[index].to_string() + "\"";
				} else {
					return get_type_name_s(type) + "(" + std::to_string(index) + ")" + String(" // ") + (*_global_const_values)[index].to_string();
				}
			} else {
				return get_type_name_s(type) + "(" + std::to_string(index) + ")";
			}

		case BUILTIN_FUNC:
			return get_type_name_s(type) + "(" + std::to_string(index) + ") // " + BuiltinFunctions::get_func_name((BuiltinFunctions::Type)index);
		case BUILTIN_TYPE:
			return get_type_name_s(type) + "(" + std::to_string(index) + ") // " + BuiltinTypes::get_type_name((BuiltinTypes::Type)index);

		case MEMBER_VAR:
			return get_type_name_s(type) + "(" + std::to_string(index) + ")";
	}
	MISSED_ENUM_CHECK(CONST_VALUE, 10);
	THROW_BUG("can't reach here");
}

void Opcodes::insert_dbg(uint32_t p_line) {
	(*op_dbg)[next()] = p_line;
}

uint32_t Opcodes::last() { return (uint32_t)opcodes.size() - 1; }
uint32_t Opcodes::next() { return (uint32_t)opcodes.size(); }

void Opcodes::insert(uint32_t p_opcode) {
	opcodes.push_back(p_opcode);
}
void Opcodes::insert(const Address& p_addr) {
	opcodes.push_back(p_addr.get_address());
}
void Opcodes::insert(Opcode p_opcode) {
	opcodes.push_back((uint32_t)p_opcode);
}

void Opcodes::write_assign(const Address& dst, const Address& src) {
	insert(Opcode::ASSIGN);
	insert(dst);
	insert(src);
}

void Opcodes::write_if(const Address& p_cond) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_cond);
	insert(0); // addr to jump out of if.

	jump_out_if.push(last());
}

void Opcodes::write_else() {
	insert(Opcode::JUMP);
	insert(0); // addr to jump out of else

	// if not jump to next();
	opcodes[jump_out_if.top()] = next();
	jump_out_if.pop();
	jump_out_if.push(last());
}

void Opcodes::write_endif() {
	opcodes[jump_out_if.top()] = next();
	jump_out_if.pop();
}

void Opcodes::write_while(const Address& p_cond) {
	//jump_to_continue.push(next()); <-- above the condition experssion.
	jump_out_break.push(stdvec<uint32_t>());

	insert(Opcode::JUMP_IF_NOT);
	insert(p_cond);
	insert(0); // addr to jump out of while.

	jump_out_while.push(last());
}

void Opcodes::write_endwhile() {
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

void Opcodes::write_foreach(const Address& p_iter_value, const Address& p_iterator, const Address& p_on) {

	insert(Opcode::ITER_BEGIN);
	insert(p_iterator);
	insert(p_on);

	jump_to_continue.push(next());
	jump_out_break.push(stdvec<uint32_t>());

	// check if can iterate more
	insert(Opcode::ITER_NEXT);
	insert(p_iter_value);
	insert(p_iterator);
	insert(0); // addr to jump out of foreach;

	jump_out_foreach.push(last());
}

void Opcodes::write_endforeach() {
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

void Opcodes::write_for(const Address& p_cond) {

	// jump_to_continue.push(next()); // <-- above the condition expression
	jump_out_break.push(stdvec<uint32_t>());

	if (p_cond.get_type() != Address::_NULL) { // for (;;) {}
		insert(Opcode::JUMP_IF_NOT);
		insert(p_cond);
		insert(0); // addr to jump out of for;

		jump_out_for.push(last());
	}
}

void Opcodes::write_endfor(bool p_has_cond) {
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

void Opcodes::write_break() {
	insert(Opcode::JUMP);
	insert(0); // addr to jump out.

	jump_out_break.top().push_back(last());
}

void Opcodes::write_continue() {
	insert(Opcode::JUMP);
	insert(jump_to_continue.top());
}

void Opcodes::write_return(const Address& p_ret_value) {
	insert(Opcode::RETURN);
	insert(p_ret_value);
}

void Opcodes::write_assign_bool(const Address& dst, bool value) {
	if (value) insert(Opcode::SET_TRUE);
	else insert(Opcode::SET_FALSE);
	insert(dst);
}

void Opcodes::write_and_left(const Address& p_left) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_left);
	insert(0); // addr to jump out if false

	jump_out_and.push(last());
}

void Opcodes::write_and_right(const Address& p_right, const Address& p_dst) {
	insert(Opcode::JUMP_IF_NOT);
	insert(p_right);
	insert(0); // <-- if false jump over the set_true below

	uint32_t above = last();

	insert(Opcode::SET_TRUE);
	insert(p_dst);

	opcodes[above] = next();
	opcodes[jump_out_and.top()] = next();
	jump_out_and.pop();
}


void Opcodes::write_or_left(const Address& p_left) {
	insert(Opcode::JUMP_IF);
	insert(p_left);
	insert(0); // <-- if true jump out

	jump_out_or.push(last());
}
void Opcodes::write_or_right(const Address& p_right, const Address& p_dst) {
	insert(Opcode::JUMP_IF);
	insert(p_right);
	insert(0); // <-- if true jump over the set_false

	uint32_t above = last();

	insert(Opcode::SET_FALSE);
	insert(p_dst);

	opcodes[above] = next();
	opcodes[jump_out_or.top()] = next();
	jump_out_or.pop();
}

/// expressions

//void Opcodes::write_get_member(const Address& p_member, const Address& p_dst) {
//	insert(Opcode::GET_MEMBER);
//	insert(p_member);
//	insert(p_dst);
//}

void Opcodes::write_get_index(const Address& p_on, uint32_t p_name, const Address& p_dst) {
	insert(Opcode::GET);
	insert(p_on);
	insert(p_name);
	insert(p_dst);
}

void Opcodes::write_set_index(const Address& p_on, uint32_t p_name, const Address& p_value) {
	insert(Opcode::SET);
	insert(p_on);
	insert(p_name);
	insert(p_value);
}

void Opcodes::write_get_mapped(const Address& p_on, const Address& p_key, const Address& p_dst) {
	insert(Opcode::GET_MAPPED);
	insert(p_on);
	insert(p_key);
	insert(p_dst);
}

void Opcodes::write_set_mapped(const Address& p_on, const Address& p_key, const Address& p_value) {
	insert(Opcode::SET_MAPPED);
	insert(p_on);
	insert(p_key);
	insert(p_value);
}

void Opcodes::write_array_literal(const Address& p_dst, const stdvec<Address>& p_values) {
	insert(Opcode::CONSTRUCT_LITERAL_ARRAY);
	insert((uint32_t)p_values.size());
	for (const Address& addr : p_values) insert(addr);
	insert(p_dst);
}

void Opcodes::write_map_literal(const Address& p_dst, const stdvec<Address>& p_keys, const stdvec<Address>& p_values) {
	ASSERT(p_keys.size() == p_values.size());

	insert(Opcode::CONSTRUCT_LITERAL_MAP);
	insert((uint32_t)p_keys.size());
	for (int i = 0; i < (int)p_keys.size(); i++) {
		insert(p_keys[i]);
		insert(p_values[i]);
	}
	insert(p_dst);
}

void Opcodes::write_construct_builtin_type(const Address& p_dst, BuiltinTypes::Type p_type, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_BUILTIN);
	insert((uint32_t)p_type);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}

void Opcodes::write_construct_native(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_NATIVE);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}
void Opcodes::write_construct_carbon(const Address& p_dst, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CONSTRUCT_CARBON);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_dst);
}

void Opcodes::write_call_builtin(const Address& p_ret, BuiltinFunctions::Type p_func, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_BUILTIN);
	insert((uint32_t)p_func);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call(const Address& p_ret, const Address& p_on, const stdvec<Address>& p_args) {
	insert(Opcode::CALL);
	insert(p_on);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_func(const Address& p_ret, uint32_t p_name, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_FUNC);
	insert(p_name);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_method(const Address& p_ret, Address& p_on, uint32_t p_method, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_METHOD);
	insert(p_on);
	insert(p_method);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_call_super_constructor(const stdvec<Address>& p_args) {
	insert(Opcode::CALL_SUPER_CTOR);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
}

void Opcodes::write_call_super_method(const Address& p_ret, uint32_t p_method, const stdvec<Address>& p_args) {
	insert(Opcode::CALL_SUPER_METHOD);
	insert(p_method);
	insert((uint32_t)p_args.size());
	for (const Address& addr : p_args) {
		insert(addr);
	}
	insert(p_ret);
}

void Opcodes::write_operator(const Address& p_dst, var::Operator p_op, const Address& p_left, const Address& p_right) {
	insert(Opcode::OPERATOR);
	insert((uint32_t)p_op);
	insert(p_left);
	insert(p_right);
	insert(p_dst);
}


}