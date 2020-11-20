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

#include "carbon_function.h"

namespace carbon {

var Bytecode::__call(stdvec<var*>& p_args) {
	return var(); // TODO:
}

var Bytecode::__call_method(const String& p_method_name, stdvec<var*>& p_args) {
	return nullptr; // TODO:
}

var Bytecode::__get_member(const String& p_member_name) {

	var* _member = _get_member_var_ptr(p_member_name);
	if (_member != nullptr) return *_member;
	
	//return _base->__get_member(p_member_name);
	THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
}

void Bytecode::__set_member(const String& p_member_name, var& p_value) {

	auto it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) {
		it->second = p_value;
		return;
	}

	// TODO: check other members only when debugging is enabled.
	auto it_const = _constants.find(p_member_name);
	if (it_const != _constants.end()) THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to a constant value named \"%s\".", p_member_name.c_str()));

	auto it_en = _enums.find(p_member_name);
	if (it_en != _enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum type named \"%s\".", p_member_name.c_str()));

	auto it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum value named \"%s\".", p_member_name.c_str()));

	auto it_fn = _functions.find(p_member_name);
	if (it_fn != _functions.end()) THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to a function pointer named \"%s\".", p_member_name.c_str()));

	if (_base != nullptr) {
		_base->__set_member(p_member_name, p_value);
	} else {
		THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}
}

var* Bytecode::_get_member_var_ptr(const String& p_member_name) {

	auto it = _member_vars.find(p_member_name);
	if (it != _member_vars.end()) { return &it->second; }

	auto it_static = _static_vars.find(p_member_name);
	if (it_static != _static_vars.end()) return &it_static->second;

	auto it_const = _constants.find(p_member_name);
	if (it_const != _constants.end()) { _member_vars[p_member_name] = it_const->second; return &_member_vars[p_member_name]; }

	auto it_en = _enums.find(p_member_name);
	if (it_en != _enums.end()) { _member_vars[p_member_name] = it_en->second; return &_member_vars[p_member_name]; }

	auto it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end()) { _member_vars[p_member_name] = it_uen->second; return &_member_vars[p_member_name]; }

	auto it_fn = _functions.find(p_member_name);
	if (it_fn != _functions.end()) { _member_vars[p_member_name] = it_fn->second; return &_member_vars[p_member_name]; }

	if (_base != nullptr) { return _base->_get_member_var_ptr(p_member_name); }
	return nullptr;
}

ptr<MemberInfo> Bytecode::get_member_info(const String& p_member_name) {
	stdmap<String, ptr<MemberInfo>>::iterator it = _member_info.find(p_member_name);
	if (it != _member_info.end()) return it->second;

	// TODO: for loop iterate with O(n) use map.find() method with O(log(n))
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
			ptr<EnumInfo> en_info = en.second;
			_member_info[p_member_name] = en_info; // doesn't really necessary.
			return en_info;
		}
	}

	// TODO: method info

	if (_base != nullptr) return _base->get_member_info(p_member_name);

	return nullptr;
	//THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
	//	String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));

}

String Bytecode::get_function_opcodes_as_string(const String& p_name) const {
	auto it = _functions.find(p_name);
	if (it == _functions.end()) THROW_ERROR(Error::NAME_ERROR, "function name not found in _functions map"); // TODO: refactor error.
	return it->second->get_opcodes_as_string(&_global_names_array, &_global_const_values);
}

}