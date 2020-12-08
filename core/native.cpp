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

#include "native.h"

namespace carbon {

NativeClasses* NativeClasses::_singleton = nullptr;

NativeClasses* NativeClasses::singleton() {
	if (_singleton == nullptr) _singleton = new NativeClasses();
	return _singleton;
}

void NativeClasses::cleanup() {
	delete _singleton;
}

void NativeClasses::bind_data(ptr<BindData> p_bind_data) {
	String class_name = p_bind_data->get_type_name();
	String data_name = p_bind_data->get_name();

	ClassEntries& entries = classes[class_name.hash()];

	if (entries.bind_data[data_name.hash()] != nullptr) {
		THROW_ERROR(Error::ATTRIBUTE_ERROR, 
			String::format("entry \"%s\" already exists on class \"%s\".", p_bind_data->get_name(), p_bind_data->get_type_name())
		);
	}
	// check initializer.
	if (class_name == data_name) {
		if (p_bind_data->get_type() != BindData::STATIC_FUNC)
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" can't be the same as type name.", data_name.c_str()));
		const MethodInfo* mi = ptrcast<StaticFuncBind>(p_bind_data)->get_method_info();
		if (mi->get_return_type().type != var::_NULL) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must not return anything.");
		if (mi->get_arg_count() < 1 || mi->get_arg_types()[0].type != var::OBJECT) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must take the instance as the first argument.");
		entries.__constructor = ptrcast<StaticFuncBind>(p_bind_data).get();		
	}
	entries.bind_data[data_name.hash()] = p_bind_data;
}

ptr<BindData> NativeClasses::get_bind_data(const String& cls, const String& attrib) {
	if (!is_class_registered(cls))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", cls.c_str()));
	auto it = classes[cls.hash()].bind_data.find(attrib.hash());
	if (it == classes[cls.hash()].bind_data.end()) return nullptr;
	return it->second;
}

// TODO: rename to get_bind_data and add p_include_base as a parameter
ptr<BindData> NativeClasses::find_bind_data(const String& cls, const String& attrib) {
	String class_name = cls;
	while (class_name.size() != 0) {
		ptr<BindData> bind_data = NativeClasses::get_bind_data(class_name, attrib);
		if (bind_data != nullptr) return bind_data;
		class_name = NativeClasses::get_inheritance(class_name);
	}
	return nullptr;
}

const ptr<MemberInfo> NativeClasses::get_member_info(const String& p_class_name, const String& attrib) {
	ptr<BindData> bd = find_bind_data(p_class_name, attrib);
	if (bd == nullptr) return nullptr;
	return bd->get_member_info();
}

void NativeClasses::set_parent(const String& p_class_name, const String& p_parent_class_name) {
	if (is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" already exists on NativeClasses entries.", p_class_name.c_str()));

	classes[p_class_name.hash()].class_name = p_class_name;
	classes[p_class_name.hash()].parent_class_name = p_parent_class_name;
}

void NativeClasses::set_new_function(const String& p_class_name, __new_f p__new) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", p_class_name.c_str()));
	classes[p_class_name.hash()].__new = p__new;
}

String NativeClasses::get_inheritance(const String& p_class_name) {
	if (classes[p_class_name.hash()].class_name.size() == 0)
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].parent_class_name;
}

bool NativeClasses::is_class_registered(const String& p_class_name) {
	return classes[p_class_name.hash()].class_name.size() != 0;
}

ptr<Object> NativeClasses::_new(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__new();
}

const StaticFuncBind* NativeClasses::get_constructor(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__constructor;
}

ptr<Object> NativeClasses::construct(const String& p_class_name, stdvec<var*>& p_args) {
	var instance = _new(p_class_name);
	p_args.insert(p_args.begin(), &instance);
	const StaticFuncBind* constructor = get_constructor(p_class_name);
	if (constructor) constructor->call(p_args);
	return instance;
}

const stdmap<size_t, ptr<BindData>>& NativeClasses::get_bind_data_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].bind_data;
}

const stdmap<size_t, ptr<MemberInfo>>& NativeClasses::get_member_info_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].member_info;
}

var NativeClasses::call_static(const String& p_base, const String& p_attrib, stdvec<var*>& p_args) {
	ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(p_base, p_attrib);
	if (bd != nullptr) {
		switch (bd->get_type()) {
			case BindData::METHOD:
			case BindData::MEMBER_VAR:
				THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot access non static member \"%s\" statically on base %s", p_attrib.c_str(), p_base.c_str()));
			case BindData::STATIC_FUNC:
				return static_cast<StaticFuncBind*>(bd.get())->call(p_args);
			case BindData::STATIC_VAR:
				return static_cast<StaticPropertyBind*>(bd.get())->get().__call(p_args);
			case BindData::STATIC_CONST:
			case BindData::ENUM:
			case BindData::ENUM_VALUE:
				THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("member \"%s\" on base %s isn't callable", p_attrib.c_str(), p_base.c_str()));
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no attribute named \"%s\" base %s", p_attrib.c_str(), p_base.c_str()));
}

var NativeClasses::call_method_on(ptr<Object>& p_on, const String& p_attrib, stdvec<var*>& p_args) {
	// TODO: this is no more needed see Object::call_method_s <-- move that to here
	BindData* bd = find_bind_data(p_on->get_type_name(), p_attrib).get();
	if (bd != nullptr) {
		switch (bd->get_type()) {
			case BindData::METHOD:
				return static_cast<MethodBind*>(bd)->call(p_on, p_args);
			case BindData::STATIC_FUNC:
				return static_cast<StaticFuncBind*>(bd)->call(p_args);
			case BindData::MEMBER_VAR:
				return static_cast<PropertyBind*>(bd)->get(p_on).__call(p_args);
			case BindData::STATIC_VAR:
				return static_cast<StaticPropertyBind*>(bd)->get().__call(p_args);
			case BindData::STATIC_CONST:
				return static_cast<ConstantBind*>(bd)->get().__call(p_args);
			case BindData::ENUM:
				return static_cast<EnumBind*>(bd)->get()->__call(p_args);
			case BindData::ENUM_VALUE:
				THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enums are not callable.");
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no attribute named \"%s\" base %s", p_attrib.c_str(), p_on->get_type_name()));
	return var();
}

}
