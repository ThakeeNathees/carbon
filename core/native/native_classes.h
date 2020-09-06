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

#ifndef NATIVE_CLASSES_H
#define NATIVE_CLASSES_H

#include "core.h"


/* USAGE:
class Aclass : public Object {
	REGISTER_CLASS(Aclass, Object) {
		BIND_ENUM_VALUE("EVAL1", EVAL1);                          // Bind unnamed enums
		BIND_ENUM_VALUE("EVAL2", EVAL2);

		BIND_ENUM("MyEnum", {                                     // enum type
			{ "V1", MyEnum::V1 },
			{ "V2", MyEnum::V2 },
			{ "V3", MyEnum::V3 },
		});

		BIND_METHOD("method1",  &Aclass::method1);                // bind method with 0 arguments
		BIND_METHOD("method2",  &Aclass::method2,  PARAMS("arg1", "arg2"), DEFVALUES("arg2_defval")); // method with 2 arguments, 1 default arg.
		BIND_STATIC_FUNC("static_func", &Aclass::static_fun);     // similerly for static functions.

		BIND_MEMBER("member", &Aclass::member);                   // bind member, type MUST BE var.
		BIND_MEMBER("member", &Aclass::member, DEFVAL(2));        // bind member with default initial value.

		BIND_STATIC_MEMBER("s_member", &Aclass::s_member);        // no need to set default value, could be resolved statically. type MUST BE var.
		BIND_STATIC_MEMBER("s_member", &Aclass::s_member);        // no need to set default value, could be resolved statically.

		BIND_CONST("CONST_VAL", &Aclass::CONST_VAL);              // similerly as static member but it could be any type since it won't change.

		BIND_METHOD_VA("va_method", &Aclass::va_method);          // method must be `Ret va_method(stdvec<var>& p_args) {}`
		BIND_STATIC_FUNC_VA("va_func_s", &Aclass::va_func_s);     // similerly static method.
	}
};
*/


#define DEFVAL(m_val) m_val
#define DEFVALUES(...) make_stdvec<var>(__VA_ARGS__)
#define PARAMS(...) __VA_ARGS__

#define BIND_METHOD(m_name, m_method, ...)    NativeClasses::bind_data(_bind_method(m_name, get_class_name_s(), m_method, __VA_ARGS__))
#define BIND_METHOD_VA(m_name, m_method)      NativeClasses::bind_data(_bind_va_method(m_name, get_class_name_s(), m_method))
#define BIND_STATIC_FUNC(m_name, m_func, ...) NativeClasses::bind_data(_bind_static_func(m_name, get_class_name_s(), m_func, __VA_ARGS__))
#define BIND_STATIC_FUNC_VA(m_name, m_func)   NativeClasses::bind_data(_bind_va_static_func(m_name, get_class_name_s(), m_func))
#define BIND_MEMBER(m_name, m_member, ...)    NativeClasses::bind_data(_bind_member(m_name, get_class_name_s(), m_member, __VA_ARGS__))
#define BIND_STATIC_MEMBER(m_name, m_member)  NativeClasses::bind_data(_bind_static_member(m_name, get_class_name_s(), m_member))
#define BIND_CONST(m_name, m_const)           NativeClasses::bind_data(_bind_static_const(m_name, get_class_name_s(), m_const))
#define BIND_ENUM(m_name, ...)                NativeClasses::bind_data(_bind_enum(m_name, get_class_name_s(), __VA_ARGS__));
#define BIND_ENUM_VALUE(m_name, m_value)      NativeClasses::bind_data(newptr<EnumValueBind>(m_name, get_class_name_s(), m_value, newptr<EnumValueInfo>(m_name, m_value)));


namespace carbon {
		typedef ptr<Object>(*__constructor_f)();

class NativeClasses {
	struct ClassEntries {
		String class_name;
		String parent_class_name;
		__constructor_f __constructor = nullptr;
		const StaticFuncBind* __initializer = nullptr;
		stdhashtable<size_t, ptr<BindData>> bind_data;
	};

private:
	static stdhashtable<size_t, ClassEntries> classes;

public:
	static void bind_data(ptr<BindData> p_bind_data);
	static void set_inheritance(const String& p_class_name, const String& p_parent_class_name);
	static void set_constructor(const String& p_class_name, __constructor_f p__constructor);

	static ptr<BindData> get_bind_data(const String& p_class_name, const String& attrib);
	static ptr<BindData> find_bind_data(const String& p_class_name, const String& attrib);
	static const MemberInfo* get_member_info(const String& p_class_name, const String& attrib);
	static String get_inheritance(const String& p_class_name);
	static bool is_class_registered(const String& p_class_name);
	static const stdvec<const BindData*> get_bind_data_list(const String& p_class_name);
	static const stdvec<const MemberInfo*> get_member_info_list(const String& p_class_name);

	static ptr<Object> construct(const String& p_class_name);
	static const StaticFuncBind* get_initializer(const String& p_class_name);
};

}

#endif // NATIVE_CLASSES_H