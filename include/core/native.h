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

#include "var/var.h"
#include "type_info.h"
#include "native_bind.gen.h"

#define DEFVAL(m_val) m_val
#define DEFVALUES(...) make_stdvec<var>(__VA_ARGS__)
#define PARAMS(...) __VA_ARGS__

#define BIND_METHOD(m_name, m_method, ...)    p_native_classes->bind_data(_bind_method(m_name, get_type_name_s(), m_method, ##__VA_ARGS__))
#define BIND_METHOD_VA(m_name, m_method)      p_native_classes->bind_data(_bind_va_method(m_name, get_type_name_s(), m_method))
#define BIND_STATIC_FUNC(m_name, m_func, ...) p_native_classes->bind_data(_bind_static_func(m_name, get_type_name_s(), m_func, ##__VA_ARGS__))
#define BIND_STATIC_FUNC_VA(m_name, m_func)   p_native_classes->bind_data(_bind_va_static_func(m_name, get_type_name_s(), m_func))
#define BIND_MEMBER(m_name, m_member, ...)    p_native_classes->bind_data(_bind_member(m_name, get_type_name_s(), m_member, ##__VA_ARGS__))
#define BIND_STATIC_MEMBER(m_name, m_member)  p_native_classes->bind_data(_bind_static_member(m_name, get_type_name_s(), m_member))
#define BIND_CONST(m_name, m_const)           p_native_classes->bind_data(_bind_static_const(m_name, get_type_name_s(), m_const))
#define BIND_ENUM(m_name, ...)                p_native_classes->bind_data(_bind_enum(m_name, get_type_name_s(), ##__VA_ARGS__));
#define BIND_ENUM_VALUE(m_name, m_value)      p_native_classes->bind_data(newptr<EnumValueBind>(m_name, get_type_name_s(), m_value, newptr<EnumValueInfo>(m_name, m_value)));


namespace carbon {
typedef ptr<Object>(*__new_f)();

class NativeClasses {
	struct ClassEntries {
		String class_name;
		String parent_class_name;
		__new_f __new = nullptr;
		const StaticFuncBind* __constructor = nullptr;
		stdmap<size_t, ptr<BindData>> bind_data;
		stdmap<size_t, ptr<MemberInfo>> member_info;
	};

private:
	NativeClasses() {}
	static NativeClasses* _singleton;
	stdhashtable<size_t, ClassEntries> classes;

public:
	static NativeClasses* singleton();
	static void cleanup();

	void bind_data(ptr<BindData> p_bind_data);
	void set_parent(const String& p_class_name, const String& p_parent_class_name);
	void set_new_function(const String& p_class_name, __new_f p__new);

	ptr<BindData> get_bind_data(const String& p_class_name, const String& attrib);
	ptr<BindData> find_bind_data(const String& p_class_name, const String& attrib);
	const ptr<MemberInfo> get_member_info(const String& p_class_name, const String& attrib);
	String get_inheritance(const String& p_class_name);
	bool is_class_registered(const String& p_class_name);
	const stdmap<size_t, ptr<BindData>>& get_bind_data_list(const String& p_class_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(const String& p_class_name);

	ptr<Object> _new(const String& p_class_name);
	const StaticFuncBind* get_constructor(const String& p_class_name);
	ptr<Object> construct(const String& p_class_name, stdvec<var*>& p_args);
	var call_static(const String& p_base_name, const String& p_attrib, stdvec<var*>& p_args);
	var call_method_on(ptr<Object>& p_on, const String& p_attrib, stdvec<var*>& p_args);

	template<typename T>
	void register_class() {
		set_parent(T::get_type_name_s(), T::get_base_type_name_s());
		set_new_function(T::get_type_name_s(), &T::__new);
		T::_bind_data(this);
	}

	template<typename T>
	void unregister_class() {
		throw "TODO:";
	}

};


}

#endif // NATIVE_CLASSES_H