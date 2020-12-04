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

#ifndef OBJECT_H
#define OBJECT_H

#include "internal.h"

#define REGISTER_CLASS(m_class, m_inherits)                                                          \
public:                                                                                              \
	typedef m_inherits Super;                                                                        \
	static ptr<Object> __new() { return newptr<m_class>(); }                                         \
	static constexpr const char* get_type_name_s() { return STR(m_class); }                          \
	static constexpr const char* get_base_type_name_s() { return STR(m_inherits); }                  \
	virtual const char* get_base_type_name() const override { return get_base_type_name_s(); }       \
	virtual const char* get_type_name() const override { return get_type_name_s(); }                 \
	static void _bind_data(NativeClasses* p_native_classes)

namespace carbon {

class var;
class String;
class NativeClasses;

class Object {
public:

	// REGISTER_CLASS methods.
	static ptr<Object> __new() { return newptr<Object>(); }
	constexpr static  const char* get_type_name_s() { return "Object"; }
	constexpr static  const char* get_base_type_name_s() { return ""; }
	virtual const char* get_type_name() const { return get_type_name_s(); }
	virtual const char* get_base_type_name() const { return get_base_type_name_s(); }
	static void _bind_data(NativeClasses* p_native_classes);

	// Operators.
	Object& operator=(const Object& p_copy) = default;
	operator String(); // const
	var operator()(stdvec<var*>& p_args);

	bool operator==(const var& p_other) /*const*/;
	bool operator!=(const var& p_other) /*const*/;
	bool operator<=(const var& p_other) /*const*/;
	bool operator>=(const var& p_other) /*const*/;
	bool operator< (const var& p_other) /*const*/;
	bool operator> (const var& p_other) /*const*/;

	var operator+(const var& p_other) /*const*/;
	var operator-(const var& p_other) /*const*/;
	var operator*(const var& p_other) /*const*/;
	var operator/(const var& p_other) /*const*/;

	var& operator+=(const var& p_other);
	var& operator-=(const var& p_other);
	var& operator*=(const var& p_other);
	var& operator/=(const var& p_other);

	var operator[](const var& p_key) const;
	var& operator[](const var& p_key);

	// TODO: move them to native
	static var call_method_s(ptr<Object> p_self, const String& p_name, stdvec<var*>& p_args);
	static var get_member_s(ptr<Object> p_self, const String& p_name);
	static void set_member_s(ptr<Object> p_self, const String& p_name, var& p_value);

	virtual var call_method(const String& p_method_name, stdvec<var*>& p_args);
	virtual var get_member(const String& p_member_name);
	virtual void set_member(const String& p_member_name, var& p_value);

	virtual ptr<Object> copy(bool p_deep) /*const*/;
	virtual void* get_data();

	// operators.
	virtual var __call(stdvec<var*>& p_vars);

	virtual var __iter_begin();
	virtual bool __iter_has_next();
	virtual var __iter_next();

	virtual var __get_mapped(const var& p_key) /*const*/;
	virtual void __set_mapped(const var& p_key, const var& p_val);
	virtual int64_t __hash() /*const*/;

	virtual var __add(const var& p_other) /*const*/;
	virtual var __sub(const var& p_other) /*const*/;
	virtual var __mul(const var& p_other) /*const*/;
	virtual var __div(const var& p_other) /*const*/;

	virtual var& __add_eq(const var& p_other);
	virtual var& __sub_eq(const var& p_other);
	virtual var& __mul_eq(const var& p_other);
	virtual var& __div_eq(const var& p_other);

	virtual bool __gt(const var& p_other) /*const*/;
	virtual bool __lt(const var& p_other) /*const*/;
	virtual bool __eq(const var& p_other) /*const*/;

	virtual String to_string() /*const*/;

	// this could be a bad design but a better workaround for now instead of using is_instance_of<Type>();
	virtual bool _is_native_ref() const;
	virtual String _get_native_ref() const;
	virtual bool _is_registered() const;

private:
	friend class var;
};

}


#endif //OBJECT_H