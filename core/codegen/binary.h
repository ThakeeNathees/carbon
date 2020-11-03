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

namespace carbon {


class Binary {
public:
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
			CLASS,
			FILE,
			MEMBER_STATIC,
			MEMBER_NONSTATIC,
			MEMBER_FILE,
			CONSTANT_CLASS,
			CONSTANT_FILE,
		};

		Address() {}
		Address(Type p_type, uint32_t p_index, bool p_temp = false) :type(p_type), index(p_index), temp(p_temp) {}

		static Type get_type_s(uint32_t p_addr) { return  (Type)((p_addr & ADDR_TYPE_MASK) >> ADDR_INDEX_BITS); }
		static uint32_t get_index_s(uint32_t p_addr) { return p_addr & ADDR_INDEX_MASK; }

		Type get_type() const { return type; }
		uint32_t get_index() const { return index; }
		uint32_t get_address() const { return index | (type << ADDR_INDEX_BITS); }
		bool is_temp() const { return temp; }

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
		SET_TRUE,
		SET_FALSE,
		OPERATOR,
		ASSIGN,
		CONSTRUCT_BUILTIN,
		CONSTRUCT_LITERAL_ARRAY,
		CONSTRUCT_LITERAL_DICT,
		// Native and other types constructed from calling
		CALL,
		CALL_BUILTIN,
		CALL_SUPER,
		JUMP,
		JUMP_IF,
		JUMP_IF_NOT,
		RETURN,

		// ITER_BEGIN
		// ITER_NEXT

		_OPCODE_END_,
	};

};

}

#endif // BINARY_H