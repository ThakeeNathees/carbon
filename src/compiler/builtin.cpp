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

#include "compiler/builtin.h"
#include "core/native.h"

/******************************************************************************************************************/
/*                                          BUILTIN TYPES                                                         */
/******************************************************************************************************************/

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

} // namespace carbon

/******************************************************************************************************************/
/*                                          BUILTIN FUNCTIONS                                                     */
/******************************************************************************************************************/

namespace carbon {

String BuiltinFunctions::get_func_name(BuiltinFunctions::Type p_func) {
	return _func_list[p_func];
}

BuiltinFunctions::Type BuiltinFunctions::get_func_type(const String& p_func) {
	for (const std::pair<Type, String>& pair : _func_list) {
		if (pair.second == p_func) {
			return pair.first;
		}
	}
	return BuiltinFunctions::UNKNOWN;
}

int BuiltinFunctions::get_arg_count(BuiltinFunctions::Type p_func) {
	switch (p_func) {
		case Type::__ASSERT:
			return 1;
		case Type::__FUNC:
		case Type::__LINE:
		case Type::__FILE:
		case Type::PRINT:
		case Type::PRINTLN:
		case Type::INPUT:
			return -1;

		case Type::HEX:
		case Type::BIN:
			return 1;

		case Type::MATH_MAX:
		case Type::MATH_MIN:
			return -1;
		case Type::MATH_POW:
			return 2;
	}
	return 0;
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

bool BuiltinFunctions::can_const_fold(Type p_func) {
	switch (p_func) {
		case Type::PRINT:
		case Type::PRINTLN:
		case Type::INPUT:
			return false;
		default:
			return true;
	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

bool BuiltinFunctions::is_compiletime(Type p_func) {
	switch (p_func) {
		case __ASSERT:
		case __FUNC:
		case __LINE:
		case __FILE:
			return true;
		default:
			return false;
	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

// TODO: change this to return r_ret for consistancy.
void BuiltinFunctions::call(Type p_func, const stdvec<var*>& p_args, var& r_ret) {
	switch (p_func) {

		case Type::__ASSERT:
		case Type::__FUNC:
		case Type::__LINE:
		case Type::__FILE:
			THROW_BUG("the compile time func should be called by the analyzer.");

		case Type::PRINT: // [[FALLTHROUGH]]
		case Type::PRINTLN: {
			for (int i = 0; i < (int)p_args.size(); i++) {
				if (i > 0) Console::log(" ");
				Console::log(p_args[i]->to_string().c_str());
			}

			if (p_func == Type::PRINTLN) Console::log("\n");

		} break;

		case Type::INPUT: {
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at most 1 argument.");
			if (p_args.size() == 1) Console::log(p_args[0]->operator String().c_str());
			r_ret = String(Console::getline());
		} break;

		case Type::HEX: {
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exctly 1 argument.");

			std::stringstream ss;
			ss << "0x" << std::hex << p_args[0]->operator int64_t();
			r_ret = String(ss.str());
		} break;

		case Type::BIN: {
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exctly 1 argument.");

			std::stringstream ss;
			ss << "0b" << std::bitset<sizeof(int64_t)>(p_args[0]->operator int64_t());
			r_ret = String(ss.str());
		} break;

		case Type::MATH_MAX: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at least 2 arguments.");
			var min = *p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (*p_args[i] < min) {
					min = *p_args[i];
				}
			}
			r_ret = min;
		} break;

		case Type::MATH_MIN: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at least 2 arguments.");
			var max = p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (*p_args[i] > max) {
					max = *p_args[i];
				}
			}
			r_ret = max;
		} break;

		case Type::MATH_POW: {
			if (p_args.size() != 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exactly 2 arguments.");
			if (p_args[0]->get_type() != var::INT && p_args[1]->get_type() != var::FLOAT)
				THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value at argument 0.");
			if (p_args[1]->get_type() != var::INT && p_args[1]->get_type() != var::FLOAT)
				THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value at argument 1.");
			r_ret = pow(p_args[0]->operator double(), p_args[1]->operator double());
		} break;

	}
	MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);
}

} // namespace carbon

/******************************************************************************************************************/
/*                                          NATIVE REFERENCE                                                      */
/******************************************************************************************************************/

namespace carbon {

NativeClassRef::NativeClassRef(const String& p_native_class) {
	_name = p_native_class;
	if (!NativeClasses::singleton()->is_class_registered(_name))
		THROW_BUG(String::format("class \"%s\" not registered in native classes entries.", _name.c_str()));
}

var NativeClassRef::__call(stdvec<var*>& p_args) {
	return NativeClasses::singleton()->construct(_name, p_args);
}

// TODO:
var  NativeClassRef::call_method(const String& p_name, stdvec<var*>& p_args) { return var(); }
var  NativeClassRef::get_member(const String& p_name) { return var(); }
void NativeClassRef::set_member(const String& p_name, var& p_value) {}

//---------------------------------------------------------------------------

BuiltinFuncRef::BuiltinFuncRef() {}
BuiltinFuncRef::BuiltinFuncRef(BuiltinFunctions::Type p_type) : _type(p_type) {}

var BuiltinFuncRef::__call(stdvec<var*>& p_args) {
	var ret;
	BuiltinFunctions::call(_type, p_args, ret);
	return ret;
}

String BuiltinFuncRef::to_string() {
	return String("[builtin_function:") + BuiltinFunctions::get_func_name(_type) + "]";
}

//---------------------------------------------------------------------------

BuiltinTypeRef::BuiltinTypeRef() {}
BuiltinTypeRef::BuiltinTypeRef(BuiltinTypes::Type p_type) :_type(p_type) {}

var BuiltinTypeRef::__call(stdvec<var*>& p_args) {
	return BuiltinTypes::construct(_type, p_args);
}

String BuiltinTypeRef::to_string() {
	return String("[builtin_type:") + BuiltinTypes::get_type_name(_type) + "]";
}

}
