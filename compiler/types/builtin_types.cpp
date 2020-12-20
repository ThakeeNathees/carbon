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

#include "builtin_types.h"

namespace carbon {

String BuiltinTypes::get_type_name(Type p_type) {
	return _type_list[p_type];
}

BuiltinTypes::Type BuiltinTypes::get_type_type(const String& p_type) {
	for (const std::pair<Type, String>& pair : _type_list) {
		if (pair.second == p_type) {
			return pair.first;
		}
	}
	return BuiltinTypes::UNKNOWN;
}

var::Type BuiltinTypes::get_var_type(Type p_type) {
	switch (p_type) {
		case UNKNOWN:
		case _NULL:  return var::_NULL;
		case BOOL:   return var::BOOL;
		case INT:    return var::INT;
		case FLOAT:  return var::FLOAT;
		case STR: // [[FALLTHROUGH]]
		case STRING: return var::STRING;
		case ARRAY:  return var::ARRAY;
		case MAP:    return var::MAP;
	}
	THROW_BUG("can't reach here.");
}
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

bool BuiltinTypes::can_construct_compile_time(Type p_type) {
	switch (p_type) {
		case UNKNOWN:
		case _NULL:
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
		case STR:
			return true;
		case ARRAY:
		case MAP:
			return false;
	}
	THROW_BUG("can't reach here.");
}
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

var BuiltinTypes::construct(Type p_type, const stdvec<var*>& p_args) {
	switch (p_type) {
		case _NULL:
			THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "can't construct null instance.");
		case BOOL:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			return p_args[0]->operator bool();
		case INT:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			switch (p_args[0]->get_type()) {
				case var::INT:
				case var::FLOAT:
					return p_args[0]->operator int64_t();
				case  var::STRING:
					return p_args[0]->operator String().to_int();
				default: {
					THROW_ERROR(Error::TYPE_ERROR, String::format("cannot construct integer from type %s", p_args[0]->get_type_name().c_str()));
				}
			}
		case FLOAT:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.");
			switch (p_args[0]->get_type()) {
				case var::INT:
				case var::FLOAT:
					return p_args[0]->operator double();
				case  var::STRING:
					return p_args[0]->operator String().to_float();
				default: {
					THROW_ERROR(Error::TYPE_ERROR, String::format("cannot construct float from type %s", p_args[0]->get_type_name().c_str()));
				}
			}

		case STR: // [[FALLTHROUGH]]
		case STRING: {
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at most 1 argument."); // TODO: what if multiple args??
			if (p_args.size() == 0) return String();
			return p_args[0]->to_string();

		} break;
		case ARRAY: {
			Array ret;
			for (size_t i = 0; i < p_args.size(); i++) {
				ret.push_back(*p_args[i]);
			}
			return ret;
		} break;

		case MAP: {
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exactly 0 argument.");
			return Map();
		}
		default: {
			ASSERT(false); // TODO: throw internal bug.
		}
	}

	MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);
	THROW_BUG("can't reach here");
}

}
