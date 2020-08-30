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
		THROW_ERROR(Error::ALREADY_DEFINED, 
			String::format("entry \"%s\" already exists on class \"%s\"", p_bind_data->get_name(), p_bind_data->get_class_name())
		);
	}
	entries.bind_data[data_name.hash()] = p_bind_data;
}

ptr<BindData> NativeClasses::get_bind_data(const String& cls, const String& attrib) {
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

void NativeClasses::set_inheritance(const String& p_class_name, const String& p_parent_class_name) {
	if (classes[p_class_name.hash()].class_name.size() != 0) {
		THROW_ERROR(Error::ALREADY_DEFINED, String::format("class \"%s\" already exists on NativeClasses entries", p_class_name));
	}
	classes[p_class_name.hash()].class_name = p_class_name;
	classes[p_class_name.hash()].parent_class_name = p_parent_class_name;
}

String NativeClasses::get_inheritance(const String& p_class_name) {
	if (classes[p_class_name.hash()].class_name.size() == 0) {
		THROW_ERROR(Error::NULL_POINTER, String::format("the class \"%s\" isn't registered in native class entries", p_class_name.c_str()));
	}
	return classes[p_class_name.hash()].parent_class_name;
}

bool NativeClasses::is_class_registered(const String& p_class_name) {
	return classes[p_class_name.hash()].class_name.size() != 0;
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
		THROW_ERROR(Error::NULL_POINTER, String::format("the class \"%s\" isn't registered in native class entries", class_name.c_str()));
	}

	ptr<BindData> bind_data = NativeClasses::find_bind_data(class_name, p_name);
	if (bind_data) {
		if (bind_data->get_type() == BindData::METHOD) {
			return ptrcast<MethodBind>(bind_data)->call(p_self, p_args);
	
		} else if (bind_data->get_type() == BindData::STATIC_FUNC) {
			return ptrcast<StaticFuncBind>(bind_data)->call(p_args);
	
		} else {
			THROW_ERROR(Error::INVALID_GET_INDEX,
				String::format("attribute named \"%s\" on type \"%s\" is not callable", method_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::INVALID_GET_INDEX, String::format("type \"%s\" has no method named \"%s\"", p_self->get_class_name(), method_name.c_str()));
}

var Object::get_member(ptr<Object> p_self, const String& p_name) {
	String class_name = p_self->get_class_name();
	String member_name = p_name;

	if (!NativeClasses::is_class_registered(class_name)) {
		THROW_ERROR(Error::NULL_POINTER, String::format("the class \"%s\" isn't registered in native class entries", class_name.c_str()));
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
			THROW_ERROR(Error::INVALID_GET_INDEX, String::format("attribute named \"%s\" on type \"%s\" is not a property", member_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::INVALID_GET_INDEX, String::format("type \"%s\" has no member named \"%s\"", p_self->get_class_name(), member_name.c_str()));
}

void Object::set_member(ptr<Object> p_self, const String& p_name, var& p_value) {
	String class_name = p_self->get_class_name();
	String member_name = p_name;

	if (!NativeClasses::is_class_registered(class_name)) {
		THROW_ERROR(Error::NULL_POINTER, String::format("the class \"%s\" isn't registered in native class entries", class_name.c_str()));
	}

	ptr<BindData> bind_data = NativeClasses::find_bind_data(class_name, member_name);
	if (bind_data) {
		if (bind_data->get_type() == BindData::MEMBER_VAR) {
			ptrcast<PropertyBind>(bind_data)->get(p_self) = p_value;
		} else if (bind_data->get_type() == BindData::STATIC_VAR) {
			ptrcast<StaticPropertyBind>(bind_data)->get() = p_value;

		} else if (bind_data->get_type() == BindData::STATIC_CONST) {
			THROW_ERROR(Error::INVALID_TYPE, String::format("can't assign a value to constant named \"%s\" on type \"%s\".", member_name.c_str(), p_self->get_class_name()));
		} else if (bind_data->get_type() == BindData::ENUM_VALUE) {
			THROW_ERROR(Error::INVALID_TYPE, String::format("can't assign a value to enum value named \"%s\" on type \"%s\".", member_name.c_str(), p_self->get_class_name()));
		} else {
			THROW_ERROR(Error::INVALID_GET_INDEX, String::format("attribute named \"%s\" on type \"%s\" is not a property", member_name.c_str(), p_self->get_class_name()));
		}
	}
	THROW_ERROR(Error::INVALID_GET_INDEX, String::format("type \"%s\" has no member named \"%s\"", p_self->get_class_name(), member_name.c_str()));
}

#endif // _VAR_H_EXTERN_IMPLEMENTATIONS

} // namespace varh