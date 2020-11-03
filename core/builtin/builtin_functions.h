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

#ifndef BUILTIN_FUNCTIONS_H
#define BUILTIN_FUNCTIONS_H

#include "core.h"

namespace carbon {

class BuiltinFunctions {
public:
	enum Type {
		UNKNOWN,

		// __compiletime_functions.
		__ASSERT,
		__FUNC,
		__LINE,
		__FILE,

		PRINT,
		INPUT,

		MATH_MIN,
		MATH_MAX,
		MATH_POW,

		_FUNC_MAX_,
	};

	// Methods.
	static String get_func_name(Type p_func);
	static Type get_func_type(const String& p_func); // returns UNKNOWN if not valid 
	static int get_arg_count(Type p_func); // returns -1 if variadic.
	static bool can_const_fold(Type p_func);
	static bool is_compiletime(Type p_func);
	static void call(Type p_func, const stdvec<var>& p_args, var& r_ret);

private:
	static stdmap<Type, String> _func_list;
};

}

#endif // BUILTIN_FUNCTIONS_H