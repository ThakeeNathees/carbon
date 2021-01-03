//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

#include "core/type_info.h"
#include "core/_error.h"
#include "core/native.h"

namespace carbon {

const ptr<MemberInfo> TypeInfo::get_member_info(const var& p_var, const String& p_name) {
	if (p_var.get_type() == var::OBJECT) return get_member_info_object(p_var.operator carbon::ptr<carbon::Object>().get(), p_name);
	else return get_member_info(p_var.get_type(), p_name);
}

const ptr<MemberInfo> TypeInfo::get_member_info(var::Type p_type, const String& p_name) {
	// TODO: not using member info in var lib for encaptulation
	THROW_BUG("TODO: not completed yet.");
	switch (p_type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return nullptr;
		case var::STRING: return get_member_info_string(p_name);
		case var::ARRAY:  return get_member_info_array(p_name);
		case var::MAP:    return get_member_info_map(p_name);
		case var::OBJECT: return get_member_info_object(nullptr, p_name);
			break;
	}
	MISSED_ENUM_CHECK(var::_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list(const var& p_var) {
	if (p_var.get_type() == var::OBJECT) return get_member_info_list_object(p_var.operator carbon::ptr<carbon::Object>().get());
	else return get_member_info_list(p_var.get_type());
}

const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list(var::Type p_type) {
	static stdmap<size_t, ptr<MemberInfo>> _null;
	switch (p_type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return _null;
		case var::STRING:
			return get_member_info_list_string();
		case var::ARRAY:
			return get_member_info_list_array();
		case var::MAP:
			return get_member_info_list_map();
		case var::OBJECT:
			return get_member_info_list_object(nullptr);
	}
	MISSED_ENUM_CHECK(var::_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}


const ptr<MemberInfo> TypeInfo::get_member_info_object(const Object* p_instance, const String& p_member) {
	if (p_instance) return NativeClasses::singleton()->get_member_info(p_instance->get_type_name(), p_member);
	else return NativeClasses::singleton()->get_member_info(Object::get_type_name_s(), p_member);
}
const stdmap<size_t, ptr<MemberInfo>>& TypeInfo::get_member_info_list_object(const Object* p_instance) {
	if (p_instance) return NativeClasses::singleton()->get_member_info_list(p_instance->get_type_name());
	else return NativeClasses::singleton()->get_member_info_list(Object::get_type_name_s());
}


const ptr<MemberInfo> TypeInfo::get_member_info_string(const String& p_member) {
	auto& list = get_member_info_list_string();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

const ptr<MemberInfo> TypeInfo::get_member_info_array(const String& p_member) {
	auto& list = get_member_info_list_array();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

const ptr<MemberInfo> TypeInfo::get_member_info_map(const String& p_member) {
	auto& list = get_member_info_list_map();
	auto it = list.find(p_member.hash());
	if (it != list.end()) return it->second;
	return nullptr;
}

}
