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
#include "vm/vm.h"

namespace carbon {

var Bytecode::__call(stdvec<var*>& p_args) {
	throw "TODO:";
	return var(); // TODO:
}

var Bytecode::__call_method(const String& p_method_name, stdvec<var*>& p_args) {

	auto it_f = _functions.find(p_method_name);
	if (it_f != _functions.end()) {
		if (!it_f->second->is_static()) throw "TODO: throw error here";
		return VM::singleton()->call_carbon_function(it_f->second.get(), this, nullptr, p_args);
	}

	auto it_sm = _static_vars.find(p_method_name);
	if (it_sm != _static_vars.end()) {
		return it_sm->second.__call(p_args);
	}

	throw "TODO: throw error here";
}

var Bytecode::__get_member(const String& p_member_name) {

	var* _member = _get_member_var_ptr(p_member_name);
	if (_member != nullptr) return *_member;
	
	//return _base->__get_member(p_member_name);
	THROW_ERROR(Error::ATTRIBUTE_ERROR,
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
	if (it_const != _constants.end()) THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to a constant value named \"%s\".", p_member_name.c_str()));

	auto it_en = _enums.find(p_member_name);
	if (it_en != _enums.end())  THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum type named \"%s\".", p_member_name.c_str()));

	auto it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end())  THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum value named \"%s\".", p_member_name.c_str()));

	auto it_fn = _functions.find(p_member_name);
	if (it_fn != _functions.end()) THROW_ERROR(Error::ATTRIBUTE_ERROR,
		String::format("cannot assign to a function pointer named \"%s\".", p_member_name.c_str()));

	if (_base != nullptr) {
		_base->__set_member(p_member_name, p_value);
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR,
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

	if (!_is_class) {
		auto it_cls = _classes.find(p_member_name);
		if (it_cls != _classes.end()) { _member_vars[p_member_name] = it_cls->second; return &_member_vars[p_member_name]; }

		auto it_ex = _externs.find(p_member_name);
		if (it_ex != _externs.end()) { _member_vars[p_member_name] = it_ex->second; return &_member_vars[p_member_name]; }
	}

	if (_base != nullptr) { return _base->_get_member_var_ptr(p_member_name); }
	return nullptr;
}

const stdmap<size_t, ptr<MemberInfo>>& Bytecode::get_member_info_list() {
	if (_member_info_built) return _member_info;

	for (auto& pair : _classes) {
		ptr<ClassInfo> class_info = newptr<ClassInfo>(pair.first, pair.second);
		_member_info[pair.first.hash()] = class_info;
	}

	for (auto& pair : _members) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _static_vars) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first, var::VAR, pair.second, false, true);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _constants) {
		ptr<PropertyInfo> prop_info = newptr<PropertyInfo>(pair.first, var::VAR, pair.second, true, true);
		_member_info[pair.first.hash()] = prop_info;
	}
	for (auto& pair : _unnamed_enums) {
		ptr<EnumValueInfo> ev_info = newptr<EnumValueInfo>(pair.first, pair.second);
		_member_info[pair.first.hash()] = ev_info;
	}
	for (auto& pair : _enums) {
		ptr<EnumInfo> en_info = pair.second;
		_member_info[pair.first.hash()] = en_info;
	}

	// TODO: method info

	_member_info_built = true;
	return _member_info;
}

const ptr<MemberInfo> Bytecode::get_member_info(const String& p_member_name) {

	if (!_member_info_built) get_member_info_list(); // this will build.

	auto it = _member_info.find(p_member_name.hash());
	if (it != _member_info.end()) return it->second;
	
	return nullptr;
}

}