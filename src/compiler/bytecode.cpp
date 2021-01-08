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

#include "compiler/bytecode.h"

#include "compiler/parser.h"
#include "compiler/function.h"
#include "compiler/vm.h"

namespace carbon {

void Bytecode::initialize() {
	if (_static_initialized) return;
	_static_initialized = true;
	stdvec<var*> _args;
	if (is_class()) {
		_file->initialize();
		if (_static_initializer) VM::singleton()->call_function(_static_initializer.get(), this, nullptr, _args);
	} else {
		for (auto p : _externs) p.second->initialize();
		if (_static_initializer) VM::singleton()->call_function(_static_initializer.get(), this, nullptr, _args);
		for (auto p : _classes) p.second->initialize();
	}
}
bool Bytecode::is_class() const { return _is_class; }
const String& Bytecode::get_name() const { return _name; }

bool Bytecode::has_base() const { ASSERT(_is_class); return _has_base; }
bool Bytecode::is_base_native() const { ASSERT(_is_class); return _is_base_native; }
const ptr<Bytecode>& Bytecode::get_base_binary() const { ASSERT(_is_class); return _base; }
const String& Bytecode::get_base_native() const { return _base_native; }

int Bytecode::get_member_count() const { return get_member_offset() + (int)_members.size(); }
stdmap<String, ptr<Bytecode>>& Bytecode::get_classes() { ASSERT(!_is_class); return _classes; }
stdmap<String, ptr<Bytecode>>& Bytecode::get_externs() { ASSERT(!_is_class); return _externs; }
const stdmap<String, var>& Bytecode::get_constants() { return _constants; }
const stdmap<String, ptr<Function>>& Bytecode::get_functions() const { return _functions; }
stdmap<String, var>& Bytecode::get_static_vars() { return _static_vars; }

const ptr<Bytecode>&  Bytecode::get_file() const { ASSERT(_is_class); return _file; }
const Function* Bytecode::get_main() const { ASSERT(!_is_class); return _main; }
const Function* Bytecode::get_constructor() const { ASSERT(_is_class); return _constructor; }
const Function* Bytecode::get_member_initializer() const { ASSERT(_is_class); return _member_initializer.get(); }
const Function* Bytecode::get_static_initializer() const { return _static_initializer.get(); }

const String& Bytecode::get_global_name(uint32_t p_pos) {
	THROW_INVALID_INDEX(_global_names_array.size(), p_pos);
	return _global_names_array[p_pos];
}

var* Bytecode::get_global_const_value(uint32_t p_index) {
	THROW_INVALID_INDEX(_global_const_values.size(), p_index);
	return &_global_const_values[p_index];
}

#define _GET_OR_NULL(m_map, m_addr)         \
	auto it = m_map.find(p_name);			\
	if (it == m_map.end()) return nullptr;	\
	return m_addr it->second
ptr<Bytecode> Bytecode::get_class(const String& p_name) { ASSERT(!_is_class); _GET_OR_NULL(_classes, PLACE_HOLDER_MACRO); }
ptr<Bytecode> Bytecode::get_import(const String& p_name) { ASSERT(!_is_class); _GET_OR_NULL(_externs, PLACE_HOLDER_MACRO); }
ptr<Function> Bytecode::get_function(const String& p_name) { _GET_OR_NULL(_functions, PLACE_HOLDER_MACRO); }
var* Bytecode::get_static_var(const String& p_name) { _GET_OR_NULL(_static_vars, &); }
var Bytecode::get_constant(const String& p_name) { _GET_OR_NULL(_constants, PLACE_HOLDER_MACRO); }

var Bytecode::__call(stdvec<var*>& p_args) {
	if (!is_class())
		THROW_ERROR(Error::ATTRIBUTE_ERROR, "Bytecode module is not callable");

	ptr<Instance> instance = newptr<Instance>(shared_from_this());
	const Function* member_initializer = get_member_initializer();
	stdvec<var*> _args;
	if (member_initializer) VM::singleton()->call_function(member_initializer, this, instance, _args);
	const Function* constructor = get_constructor();
	if (constructor) VM::singleton()->call_function(constructor, this, instance, p_args);
	return instance;
}

var Bytecode::call_method(const String& p_method_name, stdvec<var*>& p_args) {

	auto it_f = _functions.find(p_method_name);
	if (it_f != _functions.end()) {
		if (!it_f->second->is_static()) throw "TODO: throw error here";
		return VM::singleton()->call_function(it_f->second.get(), this, nullptr, p_args);
	}

	auto it_sm = _static_vars.find(p_method_name);
	if (it_sm != _static_vars.end()) {
		return it_sm->second.__call(p_args);
	}

	if (_has_base) {
		if (_is_base_native) {
			ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(_base_native, p_method_name);
			if (bd != nullptr) {
				switch (bd->get_type()) {
					case BindData::METHOD:
					case BindData::MEMBER_VAR:
						THROW_ERROR(Error::ATTRIBUTE_ERROR, "cannot call a non static attribute statically");

					case BindData::STATIC_FUNC: {
						const StaticFuncBind* sf = static_cast<const StaticFuncBind*>(bd.get());
						return sf->call(p_args);
					} break;

					case BindData::STATIC_VAR: {
						const StaticPropertyBind* pb = static_cast<const StaticPropertyBind*>(bd.get());
						return pb->get().__call(p_args);
					} break;

					case BindData::STATIC_CONST:
					case BindData::ENUM:
					case BindData::ENUM_VALUE:
						THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("%s isn't callable on base %s", p_method_name.c_str(), get_name().c_str()));
				}
			}
		} else {
			_base->call_method(p_method_name, p_args);
		}
	}

	if (!is_class()) {
		ptr<Bytecode> _class = get_class(p_method_name);
		if (_class != nullptr) {
			// TODO: abstract construction and everything.
			ptr<Instance> instance = newptr<Instance>(_class);
			const Function* member_initializer = _class->get_member_initializer();
			stdvec<var*> _args;
			if (member_initializer) VM::singleton()->call_function(member_initializer, _class.get(), instance, _args);
			const Function* constructor = _class->get_constructor();
			if (constructor) VM::singleton()->call_function(constructor, _class.get(), instance, p_args);
			return instance;
		}
	}
	THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists on base %s", p_method_name.c_str(), get_name().c_str()));
}

var Bytecode::get_member(const String& p_member_name) {

	var* _member = _get_member_var_ptr(p_member_name);
	if (_member != nullptr) return *_member;
	
	if (_base != nullptr) {
		return _base->get_member(p_member_name);
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}
}

void Bytecode::set_member(const String& p_member_name, var& p_value) {

	auto it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) {
		it->second = p_value;
		return;
	}

	// check other members for better error message.
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
		_base->set_member(p_member_name, p_value);
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR,
			String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
	}
}

var* Bytecode::_get_member_var_ptr(const String& p_member_name) {

	// if already constructed var* return it
	auto it = _member_vars.find(p_member_name);
	if (it != _member_vars.end()) { return &it->second; }

	// search and construct var*

	auto it_static = _static_vars.find(p_member_name);
	if (it_static != _static_vars.end()) return &it_static->second;

	auto it_const = _constants.find(p_member_name);
	if (it_const != _constants.end()) return &it_const->second; // { _member_vars[p_member_name] = it_const->second; return &_member_vars[p_member_name]; }

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

int Bytecode::get_member_offset() const {
	if (_base != nullptr) return _base->get_member_count();
	if (_pending_base != nullptr) {
		Parser::ClassNode* cls = (Parser::ClassNode*)_pending_base;
		uint32_t member_count = 0;
		for (auto& v : cls->vars) if (!v->is_static) member_count++;
		return cls->get_member_offset() + member_count;
	}
	return 0;
}

uint32_t Bytecode::get_member_index(const String& p_name) {
	auto it = _members.find(p_name);
	if (it == _members.end()) {
		if (_base == nullptr) {
			if (_pending_base != nullptr) return ((Parser::ClassNode*)_pending_base)->get_member_index(p_name);
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no member named \"%s\" on base %s.",
				p_name.c_str(), _name.c_str()));
		} else {
			return _base->get_member_index(p_name);
		}
	} else {
		return get_member_offset() + it->second;
	}
}

const ptr<MemberInfo> Bytecode::get_member_info(const String& p_member_name) {

	if (!_member_info_built) get_member_info_list(); // this will build.

	auto it = _member_info.find(p_member_name.hash());
	if (it != _member_info.end()) return it->second;
	
	return nullptr;
}

// ------------ private -------------------

uint32_t Bytecode::_global_name_get(const String& p_name) {
	ASSERT(!_is_class); // global names only available at file.
	stdmap<String, uint32_t>::iterator it = _global_names.find(p_name);
	if (it == _global_names.end()) {
		uint32_t pos = (uint32_t)_global_names.size();
		_global_names[p_name] = pos;
		return pos;
	} else {
		return it->second;
	}
}

void Bytecode::_build_global_names_array() {
	_global_names_array.resize(_global_names.size());
	for (auto& it : _global_names) {
		_global_names_array[it.second] = it.first;
	}
}

uint32_t Bytecode::_global_const_value_get(const var& p_value) {
	for (int i = 0; i < (int)_global_const_values.size(); i++) {
		if (_global_const_values[i].get_type() == p_value.get_type() && _global_const_values[i] == p_value) {
			return i;
		}
	}
	_global_const_values.push_back(p_value);
	return (uint32_t)(_global_const_values.size() - 1);
}

}