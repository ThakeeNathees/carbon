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

#include "bytecode.h"

namespace carbon {

var Bytecode::__call_method(const String& p_method_name, stdvec<var*>& p_args) {
	return nullptr; // TODO:
}

var Bytecode::__get_member(const String& p_member_name) {
	stdmap<String, var>::iterator it = _constants.find(p_member_name);
	if (it != _constants.end()) return it->second;

	it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) return it->second;

	stdmap<String, ptr<_EnumBytes>>::iterator it_en = _enums.find(p_member_name);
	if (it_en != _enums.end()) return it_en->second;

	stdmap<String, int64_t>::iterator it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end()) return it_uen->second;

	if (_base != nullptr) return _base->__get_member(p_member_name);

	THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));

}

void Bytecode::__set_member(const String& p_member_name, var& p_value) {
	stdmap<String, var>::iterator it = _constants.find(p_member_name);
	it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) {
		it->second = p_value;
		return;
	}

	// TODO: check other members only when debugging is enabled.
	it = _constants.find(p_member_name);
	if (it != _constants.end()) THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to a constant value named \"%s\".", it->first.c_str()));

	stdmap<String, ptr<_EnumBytes>>::iterator it_en = _enums.find(p_member_name);
	if (it_en != _enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum type named \"%s\".", it->first.c_str()));

	stdmap<String, int64_t>::iterator it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum value named \"%s\".", it->first.c_str()));

	if (_base != nullptr) {
		_base->__set_member(p_member_name, p_value);
	} else {
		THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}

}

ptr<MemberInfo> Bytecode::get_member_info(const String& p_member_name) {
	stdmap<String, ptr<MemberInfo>>::iterator it = _member_info.find(p_member_name);
	if (it != _member_info.end()) return it->second;

	for (auto& cls : _classes) {
		if (cls.first == p_member_name) {
			ptr<ClassInfo> class_info = newptr<ClassInfo>(p_member_name, cls.second);
			_member_info[p_member_name] = class_info;
			return class_info;
		}
	}
	for (auto& mem : _members) {
		if (mem.first == p_member_name) {
			ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(p_member_name);
			_member_info[p_member_name] = prop_info;
			return prop_info;
		}
	}
	for (auto& svar : _static_vars) {
		if (svar.first == p_member_name) {
			ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(p_member_name, var::VAR, svar.second, false, true);
			_member_info[p_member_name] = prop_info;
			return prop_info;
		}
	}
	for (auto& con : _constants) {
		if (con.first == p_member_name) {
			ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(p_member_name, var::VAR, con.second, true, true);
			_member_info[p_member_name] = prop_info;
			return prop_info;
		}
	}
	for (auto& unen : _unnamed_enums) {
		if (unen.first == p_member_name) {
			ptr<EnumValueInfo> ev_info = newptr<EnumValueInfo>(p_member_name, unen.second);
			_member_info[p_member_name] = ev_info;
			return ev_info;
		}
	}
	for (auto& en : _enums) {
		if (en.first == p_member_name) {
			ptr<EnumInfo> en_info = newptr<EnumInfo>(en.second);
			_member_info[p_member_name] = en_info;
			return en_info;
		}
	}

	if (_base != nullptr) return _base->get_member_info(p_member_name);

	return nullptr;
	//THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
	//	String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));

}

}