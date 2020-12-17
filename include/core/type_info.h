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

#ifndef _TYPE_INFO_H
#define _TYPE_INFO_H

#include "var/var.h"

namespace carbon {

class MemberInfo {
public:
	enum Type {
		METHOD,
		PROPERTY,
		ENUM,
		ENUM_VALUE,
		CLASS,
	};

	virtual Type get_type() const = 0;
	virtual const String& get_name() const = 0;
};

struct VarTypeInfo {
	var::Type type = var::_NULL;
	const char* class_name = nullptr;
	VarTypeInfo(var::Type p_type = var::VAR) : type(p_type) {}
	VarTypeInfo(var::Type p_type, const char* p_class_name) : type(p_type), class_name(p_class_name) {}

	bool operator==(const VarTypeInfo p_other) const {
		if (type != var::OBJECT) return type == p_other.type;
		return strcmp(class_name, p_other.class_name) == 0;
	}
	bool operator!=(const VarTypeInfo p_other) const { return !(operator==(p_other)); }
};

class ClassInfo : public MemberInfo, public Object {
	REGISTER_CLASS(ClassInfo, Object);
	public: ClassInfo() {} // default constructor needed for inherit Object

private:
	String name;
	var _class; // compiled version of the class.
public:
	ClassInfo(const String& p_name, const var& p_class) :name(p_name), _class(p_class) { }
	virtual Type get_type() const { return  CLASS; };
	virtual const String& get_name() const { return name; };

	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "_class"_hash: return _class;
			default: return Super::get_member(p_name);
		}
	}
};

class MethodInfo : public MemberInfo, public Object {
	REGISTER_CLASS(MethodInfo, Object);
public: MethodInfo() {} // default constructor needed for inherit Object

private:
	String name;
	bool _is_static = false;
	int arg_count = 0; // -1 is va args, -2 is unknown
	stdvec<String> arg_names;
	stdvec<var> default_args;
	stdvec<VarTypeInfo> arg_types;
	VarTypeInfo return_type;

	// to call a function using method info
	// we need method bind
	void* _bind = nullptr;

public:
	virtual Type get_type() const override { return Type::METHOD; }
	virtual const String& get_name() const override { return name; }

	// complete constructor.
	MethodInfo(
		String p_name,
		stdvec<String> p_arg_names,
		stdvec<VarTypeInfo> p_arg_types = stdvec<VarTypeInfo>(),
		VarTypeInfo p_return_type = var::_NULL,
		bool p__is_static = false,
		stdvec<var> p_default_args = stdvec<var>(),
		int p_arg_count = -2
	) {
		name = p_name;
		arg_names = p_arg_names;
		arg_types = p_arg_types;
		return_type = p_return_type;
		_is_static = p__is_static;
		default_args = p_default_args;

		// if va_arg it should be set manually to -1
		if (p_arg_count == -2) arg_count = (int)arg_names.size();
		else arg_count = p_arg_count;
	}

	// zero parameter constructor
	MethodInfo(
		String p_name,
		VarTypeInfo p_return_type = var::_NULL,
		bool p__is_static = false
	) {
		name = p_name;
		return_type = p_return_type;
		_is_static = p__is_static;
	}

	void _set_bind(void* p_bind) { _bind = p_bind; }
	void* _get_bind() { return _bind; }

	// defined in native.cpp
	var __call(stdvec<var*>& p_args) override;

	int get_arg_count() const { return arg_count; }
	int get_default_arg_count() const { return (int)default_args.size(); }
	bool is_static() const { return _is_static; }
	const stdvec<String>& get_arg_names() const { return arg_names; }
	const stdvec<var>& get_default_args() const { return default_args; }
	const stdvec<VarTypeInfo>& get_arg_types() const { return arg_types; }
	VarTypeInfo get_return_type() const { return return_type; }

	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "is_static"_hash: return _is_static;
			case "arg_count"_hash: return arg_count;
			case "default_arg_count"_hash: return default_args.size();
			default: return Super::get_member(p_name);
		}
	}
};

class PropertyInfo : public MemberInfo, public Object {
	REGISTER_CLASS(PropertyInfo, Object);
	public: PropertyInfo() {} // default constructor needed for inherit Object

private:
	String name;
	VarTypeInfo datatype;
	var value;
	bool _is_const = false;
	bool _is_static = false;

	void* _bind = nullptr;

public:
	virtual Type get_type() const override { return Type::PROPERTY; }
	virtual const String& get_name() const override { return name; }

	PropertyInfo(
		const String& p_name,
		VarTypeInfo p_datatype = var::VAR,
		var p_value = var(),
		bool p__is_const = false,
		bool p__is_static = false
	) {
		name = p_name;
		datatype = p_datatype;
		value = p_value;
		_is_const = p__is_const;
		_is_static = p__is_static;
	}

	void _set_bind(void* p_bind) { _bind = p_bind; }
	void* _get_bind() { return _bind; }

	// defined in native.cpp
	var __call(stdvec<var*>& p_args) override;

	bool is_static() const { return _is_static; }
	bool is_const() const { return _is_const; }
	VarTypeInfo get_datatype() const { return datatype; }
	const var& get_value() const { return value; }         // value for constants.
	const var& get_default_value() const { return value; } // defalut_value for variables.

	var get_member(const String& p_name) override {
		switch (p_name.const_hash()) {
			case "name"_hash: return name;
			case "value"_hash: return value;
			default: return Super::get_member(p_name);
		}
	}
};

class EnumInfo : public MemberInfo, public Object {
	REGISTER_CLASS(EnumInfo, Object);
	public: EnumInfo() {} // default constructor needed for inherit Object

private:
	String _name;
	stdmap<String, int64_t> _values;

public:
	virtual Type get_type() const override { return Type::ENUM; }
	virtual const String& get_name() const override { return _name; }

	EnumInfo(const String& p_name) : _name(p_name) {}
	EnumInfo(const String& p_name, const stdvec<std::pair<String, int64_t>>& p_values) {
		_name = p_name;
		for (const std::pair<String, int64_t>& p : p_values) {
			_values[p.first] = p.second;
		}
	}

	var get_member(const String& p_name) override {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) return it->second;
		return Super::get_member(p_name);
	}
	void set_member(const String& p_name, var& p_value) override {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) throw "TODO:";// TODO: more throw_error to _type_info.cpp => THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot assign a value to enum value."));
		else Super::set_member(p_name, p_value);
	}

	const stdmap<String, int64_t>& get_values() const { return _values; }
	stdmap<String, int64_t>& get_edit_values() { return _values; }
};

class EnumValueInfo : public MemberInfo {
private:
	String name;
	int64_t value;

public:
	virtual Type get_type() const override { return Type::ENUM_VALUE; }
	virtual const String& get_name() const override { return name; }

	EnumValueInfo(const String& p_name, int64_t p_value) {
		name = p_name;
		value = p_value;
	}

	int64_t get_value() const { return value; }
};


// ---------------------------------------------------------------------------------

class TypeInfo {

public:
	static const ptr<MemberInfo> get_member_info(const var& p_var, const String& p_name);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(const var& p_var);

	static const ptr<MemberInfo> get_member_info(var::Type p_type, const String& p_name);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list(var::Type p_type);

	static const ptr<MemberInfo> get_member_info_object(const Object* p_instance, const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_object(const Object* p_instance);

	static const ptr<MemberInfo> get_member_info_string(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_string();

	static const ptr<MemberInfo> get_member_info_array(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_array();

	static const ptr<MemberInfo> get_member_info_map(const String& p_member);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_map();

};




}

#endif // _TYPE_INFO_H