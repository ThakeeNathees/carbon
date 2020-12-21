
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

#ifndef BUILTIN_H
#define BUILTIN_H

#include "core/core.h"

/******************************************************************************************************************/
/*                                          BUILTIN TYPES                                                         */
/******************************************************************************************************************/

namespace carbon {

class BuiltinTypes {
public:
	enum Type {
		UNKNOWN,

		_NULL,
		BOOL,
		INT,
		FLOAT,
		STRING,
		STR,
		ARRAY,
		MAP,
		//OBJECT,

		_TYPE_MAX_,
	};

private: // members
	static stdmap<Type, String> _type_list;

public:

	static String get_type_name(Type p_type);
	static Type get_type_type(const String& p_type);
	static var::Type get_var_type(Type p_type);
	static var construct(Type p_type, const stdvec<var*>& p_args);
	static bool can_construct_compile_time(Type p_type);
};

}

/******************************************************************************************************************/
/*                                          BUILTIN FUNCTIONS                                                     */
/******************************************************************************************************************/

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

			// runtime functions.
			PRINT,
			PRINTLN,
			INPUT,

			HEX,
			BIN,

			MATH_MIN,
			MATH_MAX,
			MATH_POW,

			_FUNC_MAX_,
		};

	private: // members
		static stdmap<Type, String> _func_list;

	public:

		// Methods.
		static String get_func_name(Type p_func);
		static Type get_func_type(const String& p_func); // returns UNKNOWN if not valid 
		static int get_arg_count(Type p_func); // returns -1 if variadic.
		static bool can_const_fold(Type p_func);
		static bool is_compiletime(Type p_func);
		static void call(Type p_func, const stdvec<var*>& p_args, var& r_ret);
	};

}

/******************************************************************************************************************/
/*                                          NATIVE REFERENCE                                                      */
/******************************************************************************************************************/

namespace carbon {

	class NativeClassRef : public Object {
		REGISTER_CLASS(NativeClassRef, Object) {}

	private:
		String _name;

	public:
		NativeClassRef() {}
		NativeClassRef(const String& p_native_class);

		bool _is_native_ref() const override { return true; }
		String _get_native_ref() const { return _name; }

		var __call(stdvec<var*>& p_args) override; // construct
		var call_method(const String& p_name, stdvec<var*>& p_args) override; // static method call
		var get_member(const String& p_name) override; // static member, constants, functions, ...
		void set_member(const String& p_name, var& p_value) override; // static members
	};

	class BuiltinFuncRef : public Object {
		REGISTER_CLASS(BuiltinFuncRef, Object) {}

	private: // members
		BuiltinFunctions::Type _type;

	public:
		BuiltinFuncRef();
		BuiltinFuncRef(BuiltinFunctions::Type p_type);

		var __call(stdvec<var*>& p_args) override;
		String to_string() override;

		// TODO: add wrapper methods.
	};

	class BuiltinTypeRef : public Object {
		REGISTER_CLASS(BuiltinTypeRef, Object) {}

	private: // members
		BuiltinTypes::Type _type;

	public:
		BuiltinTypeRef();
		BuiltinTypeRef(BuiltinTypes::Type p_type);

		var __call(stdvec<var*>& p_args) override;
		String to_string() override;

	};

}

#endif // BUILTIN_H
