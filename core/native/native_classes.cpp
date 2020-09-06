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

#include "native_classes.h"

namespace carbon {

stdhashtable<size_t, NativeClasses::ClassEntries> NativeClasses::classes;

void NativeClasses::bind_data(ptr<BindData> p_bind_data) {
	String class_name = p_bind_data->get_class_name();
	String data_name = p_bind_data->get_name();

	ClassEntries& entries = classes[class_name.hash()];

	if (entries.bind_data[data_name.hash()] != nullptr) {
		THROW_ERROR(Error::NAME_ERROR, 
			String::format("entry \"%s\" already exists on class \"%s\".", p_bind_data->get_name(), p_bind_data->get_class_name())
		);
	}
	// check initializer.
	if (class_name == data_name) {
		if (p_bind_data->get_type() != BindData::STATIC_FUNC)
			THROW_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" can't be the same as type name.", data_name.c_str()));
		const MethodInfo* mi = ptrcast<StaticFuncBind>(p_bind_data)->get_method_info();
		if (mi->get_return_type().type != var::_NULL) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must not return anything.");
		if (mi->get_arg_count() < 1 || mi->get_arg_types()[0].type != var::OBJECT) THROW_ERROR(Error::TYPE_ERROR, "constructor initializer must take the instance as the first argument.");
		entries.__initializer = ptrcast<StaticFuncBind>(p_bind_data).get();		
	}
	entries.bind_data[data_name.hash()] = p_bind_data;
}

ptr<BindData> NativeClasses::get_bind_data(const String& cls, const String& attrib) {
	if (!is_class_registered(cls))
		THROW_ERROR(Error::NAME_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", cls.c_str()));
	return classes[cls.hash()].bind_data[attrib.hash()];
}

ptr<BindData> NativeClasses::find_bind_data(const String& cls, const String& attrib) {
	String class_name = cls;
	while (class_name.size() != 0) {
		ptr<BindData> bind_data = NativeClasses::get_bind_data(class_name, attrib);
		if (bind_data != nullptr) return bind_data;
		class_name = NativeClasses::get_inheritance(class_name);
	}
	return nullptr;
}

const MemberInfo* NativeClasses::get_member_info(const String& p_class_name, const String& attrib) {
	return get_bind_data(p_class_name, attrib)->get_member_info();
}

void NativeClasses::set_inheritance(const String& p_class_name, const String& p_parent_class_name) {
	if (is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("class \"%s\" already exists on NativeClasses entries.", p_class_name.c_str()));

	classes[p_class_name.hash()].class_name = p_class_name;
	classes[p_class_name.hash()].parent_class_name = p_parent_class_name;
}

void NativeClasses::set_constructor(const String& p_class_name, __constructor_f p__constructor) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("class \"%s\" not registered on NativeClasses entries.", p_class_name.c_str()));
	classes[p_class_name.hash()].__constructor = p__constructor;
}

String NativeClasses::get_inheritance(const String& p_class_name) {
	if (classes[p_class_name.hash()].class_name.size() == 0)
		THROW_ERROR(Error::NAME_ERROR, String::format("class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].parent_class_name;
}

bool NativeClasses::is_class_registered(const String& p_class_name) {
	return classes[p_class_name.hash()].class_name.size() != 0;
}

ptr<Object> NativeClasses::construct(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__constructor();
}

const StaticFuncBind* NativeClasses::get_initializer(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	return classes[p_class_name.hash()].__initializer;
}

const stdvec<const BindData*> NativeClasses::get_bind_data_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	stdvec<const BindData*> ret;
	stdhashtable<size_t, ptr<BindData>>& bind_data_list = classes[p_class_name.hash()].bind_data;
	stdhashtable<size_t, ptr<BindData>>::iterator it = bind_data_list.begin();
	while (it != bind_data_list.end()) {
		const BindData* bd = (*it).second.get();
		ret.push_back(bd);
		it++;
	}
	return ret;
}

const stdvec<const MemberInfo*> NativeClasses::get_member_info_list(const String& p_class_name) {
	if (!is_class_registered(p_class_name))
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", p_class_name.c_str()));
	stdvec<const MemberInfo*> ret;
	stdhashtable<size_t, ptr<BindData>>& bind_data_list = classes[p_class_name.hash()].bind_data;
	stdhashtable<size_t, ptr<BindData>>::iterator it = bind_data_list.begin();
	while (it != bind_data_list.end()) {
		const BindData* bd = (*it).second.get();
		ret.push_back(bd->get_member_info());
		it++;
	}
	return ret;
}

}

namespace varh {
using namespace carbon;
#ifdef _VAR_H_EXTERN_IMPLEMENTATIONS
void Object::_bind_data() {
	BIND_METHOD("get_class_name", &Object::get_class_name);
	BIND_METHOD("get_parent_class_name", &Object::get_parent_class_name);
}

// call_method() should call it's parent if method not exists.
var Object::call_method(ptr<Object> p_self, const String& p_name, stdvec<var>& p_args) {
	String class_name = p_self->get_class_name();
	String method_name = p_name;

	if (!NativeClasses::is_class_registered(class_name)) {
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", class_name.c_str()));
	}

	ptr<BindData> bind_data = NativeClasses::find_bind_data(class_name, p_name);
	if (bind_data) {
		if (bind_data->get_type() == BindData::METHOD) {
			return ptrcast<MethodBind>(bind_data)->call(p_self, p_args);
	
		} else if (bind_data->get_type() == BindData::STATIC_FUNC) {
			return ptrcast<StaticFuncBind>(bind_data)->call(p_args);
	
		} else {
			THROW_ERROR(Error::TYPE_ERROR,
				String::format("attribute named \"%s\" on type %s is not callable.", method_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::NAME_ERROR, String::format("type %s has no method named \"%s\".", p_self->get_class_name(), method_name.c_str()));
}

var Object::get_member(ptr<Object> p_self, const String& p_name) {
	String class_name = p_self->get_class_name();
	String member_name = p_name;

	if (!NativeClasses::is_class_registered(class_name)) {
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", class_name.c_str()));
	}
	
	ptr<BindData> bind_data = NativeClasses::find_bind_data(class_name, member_name);
	if (bind_data) {
		if (bind_data->get_type() == BindData::MEMBER_VAR) {
			return ptrcast<PropertyBind>(bind_data)->get(p_self);
		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			return ptrcast<StaticPropertyBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::STATIC_CONST) {
			return ptrcast<ConstantBind>(bind_data)->get();
		} else if (bind_data->get_type() == BindData::ENUM_VALUE) {
			return ptrcast<EnumValueBind>(bind_data)->get();

		} else {
			THROW_ERROR(Error::TYPE_ERROR, String::format("attribute named \"%s\" on type %s is not a property.", member_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::NAME_ERROR, String::format("type %s has no member named \"%s\"", p_self->get_class_name(), member_name.c_str()));
}

void Object::set_member(ptr<Object> p_self, const String& p_name, var& p_value) {
	String class_name = p_self->get_class_name();
	String member_name = p_name;

	if (!NativeClasses::is_class_registered(class_name)) {
		THROW_ERROR(Error::NAME_ERROR, String::format("the class \"%s\" isn't registered in native class entries.", class_name.c_str()));
	}

	ptr<BindData> bind_data = NativeClasses::find_bind_data(class_name, member_name);
	if (bind_data) {
		if (bind_data->get_type() == BindData::MEMBER_VAR) {
			ptrcast<PropertyBind>(bind_data)->get(p_self) = p_value;
		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			ptrcast<StaticPropertyBind>(bind_data)->get() = p_value;

		} else if (bind_data->get_type() == BindData::STATIC_CONST) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't assign a value to constant named \"%s\" on type \"%s\".", member_name.c_str(), p_self->get_class_name()));
		} else if (bind_data->get_type() == BindData::ENUM_VALUE) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't assign a value to enum value named \"%s\" on type \"%s\".", member_name.c_str(), p_self->get_class_name()));
		} else {
			THROW_ERROR(Error::TYPE_ERROR, String::format("attribute named \"%s\" on type \"%s\" is not a property.", member_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::NAME_ERROR, String::format("type %s has no member named \"%s\".", p_self->get_class_name(), member_name.c_str()));
}

const stdvec<const MemberInfo*> Object::get_member_info_list(const Object* p_instance) {
	if (p_instance) return NativeClasses::get_member_info_list(p_instance->get_class_name());
	else return NativeClasses::get_member_info_list(Object::get_class_name_s());
}
const MemberInfo* Object::get_member_info(const Object* p_instance, const String& p_member) {
	if (p_instance) return NativeClasses::get_member_info(p_instance->get_class_name(), p_member);
	else return NativeClasses::get_member_info(Object::get_class_name_s(), p_member);

}

#endif // _VAR_H_EXTERN_IMPLEMENTATIONS

} // namespace varh