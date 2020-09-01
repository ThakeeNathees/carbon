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

#include "builtin_functions.h"

namespace carbon {

String BuiltinFunctions::get_func_name(BuiltinFunctions::Type p_func) {
	return _func_list[p_func];
}

BuiltinFunctions::Type BuiltinFunctions::get_func_type(const String& p_func) {
	for (const std::pair<Type, String>& pair: _func_list) {
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
		case Type::INPUT:
		case Type::MATH_MAX:
		case Type::MATH_MIN:
			return -1;
		case Type::MATH_POW:
			return 2;
	}
	return 0;
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 10);
}

bool BuiltinFunctions::can_const_fold(Type p_func) {
	switch (p_func) {
		case Type::PRINT:
		case Type::INPUT:
			return false;
		default:
			return true;
	}
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 10);
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
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 10);
}

// TODO: change this to return r_ret for consistancy.
void BuiltinFunctions::call(Type p_func, const stdvec<var>& p_args, var& r_ret) {
	switch (p_func) {

		case Type::__ASSERT:
		case Type::__FUNC:
		case Type::__LINE:
		case Type::__FILE:
			THROW_ERROR(Error::INTERNAL_BUG, "the compile time func should be called by the analyzer.");

		case Type::PRINT: {
			for (int i = 0; i < (int)p_args.size(); i++) {
				printf("%s", p_args[i].operator String().c_str());
			}
			printf("\n");
		} break;

		case Type::INPUT: {
			// Not safe to use scanf() possibly lead to buffer overflow.
			String input;
			std::cin >> input;
			r_ret = input;
		} break;

		case Type::MATH_MAX: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at least 2 arguments.");
			var min = p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (p_args[i] < min) {
					min = p_args[i];
				}
			}
			r_ret = min;
		} break;

		case Type::MATH_MIN: {
			if (p_args.size() <= 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected at least 2 arguments.");
			var max = p_args[0];
			for (int i = 1; i < (int)p_args.size(); i++) {
				if (p_args[i] > max) {
					max = p_args[i];
				}
			}
			r_ret = max;
		} break;

		case Type::MATH_POW: {
			if (p_args.size() != 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "Expected exactly 2 arguments.");
			if (p_args[0].get_type() != var::INT && p_args[1].get_type() != var::FLOAT)
				THROW_ERROR(Error::INVALID_ARGUMENT, "Expected a numeric value at argument 0.");
			if (p_args[1].get_type() != var::INT && p_args[1].get_type() != var::FLOAT)
				THROW_ERROR(Error::INVALID_ARGUMENT, "Expected a numeric value at argument 1.");
			r_ret = pow(p_args[0].operator double(), p_args[1].operator double());
		} break;

	}
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 10);
}

}