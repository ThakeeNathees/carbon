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

#ifndef BYTECODE_H
#define BYTECODE_H

#include "binary.h"

namespace carbon {

class CarbonFunction;

class Bytecode : public Object {
	REGISTER_CLASS(Bytecode, Object) {}

public:
	Bytecode() {}

	var __call(stdvec<var*>& p_args) override; // constructor
	var __call_method(const String& p_method_name, stdvec<var*>& p_args) override; // static methods.
	var __get_member(const String& p_member_name) override; // static member, constants, enums, functions ...
	void __set_member(const String& p_member_name, var& p_value) override; // static members.

	inline int get_member_count() const {
		return get_member_offset() + (int)_members.size();
	}
	inline int get_member_offset() const {
		return (_base != nullptr ? _base->get_member_count() : 0);
	}

	uint32_t get_member_index(const String& p_name) {
		auto it = _members.find(p_name);
		if (it == _members.end()) {
			ASSERT(_base != nullptr); // TODO: _base==nullptr -> throw runtime error here <-- no member named p_name
			return _base->get_member_index(p_name);
		} else {
			return get_member_offset() + it->second;
		}
	}

	const ptr<MemberInfo> get_member_info(const String& p_member_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();

	bool is_class() const { return _is_class; }
	String get_name() const { return _name; }
	var* _get_member_var_ptr(const String& p_member_name);

	const stdmap<String, ptr<CarbonFunction>>& get_functions() const { return _functions; }
	stdmap<String, ptr<Bytecode>>& get_classes() { return _classes; }
	stdmap<String, ptr<Bytecode>>& get_externs() { return _externs; }

	bool has_base() const { ASSERT(_is_class); return _has_base; }
	bool is_base_native() const { ASSERT(_is_class); return _is_base_native; }
	const ptr<Bytecode>& get_base_binary() const { ASSERT(_is_class); return _base; }
	const String& get_base_native() const { return _base_native; }

	const ptr<Bytecode>& get_file() const { ASSERT(_is_class); return _file; }
	const CarbonFunction* get_main() const { ASSERT(!_is_class); return _main; }
	const CarbonFunction* get_constructor() const { ASSERT(_is_class); return _constructor; }

	const String& get_global_name(uint32_t p_pos) {
		THROW_INVALID_INDEX(_global_names_array.size(), p_pos);
		return _global_names_array[p_pos];
	}

	var* get_global_const_value(uint32_t p_index) {
		THROW_INVALID_INDEX(_global_const_values.size(), p_index);
		return &_global_const_values[p_index];
	}

private:
	friend class CodeGen;
	friend struct CGContext;
	bool _is_class = false;

	String _name; // name for class, path for file.
	
	// for ClassNode
	bool _has_base = false;
	bool _is_base_native = false;
	ptr<Bytecode> _base = nullptr;
	String _base_native;
	ptr<Bytecode> _file = nullptr;

	stdmap<String, ptr<Bytecode>> _classes; // for FileNode
	stdmap<String, ptr<Bytecode>> _externs; // imported for FileNode

	stdmap<String, uint32_t> _members; // member index. offset willbe added for inherited instances.
	stdmap<String, var> _static_vars;
	stdmap<String, var> _constants;
	stdmap<String, int64_t> _unnamed_enums;
	stdmap<String, ptr<EnumInfo>> _enums;
	stdmap<String, ptr<CarbonFunction>> _functions;

	stdvec<String> _global_names_array;
	stdmap<String, uint32_t> _global_names;
	stdvec<var> _global_const_values;

	bool _member_info_built = false; // set to true after _member_info is built
	stdmap<size_t, ptr<MemberInfo>> _member_info;
	stdmap<String, var> _member_vars; // all members as var (constructed at runtime)

	union {
		CarbonFunction* _main = nullptr; // file
		CarbonFunction* _constructor;    // class
	};
	CarbonFunction* _static_initializer;
	CarbonFunction* _member_initialization; // class

	bool _is_compiled = false;
	bool _is_compiling = false;

	uint32_t _global_name_get(const String& p_name) {
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

	void _build_global_names_array() {
		_global_names_array.resize(_global_names.size());
		for (auto& it : _global_names) {
			_global_names_array[it.second] = it.first;
		}
	}

	uint32_t _global_const_value_get(const var& p_value) {
		stdvec<var>::iterator it = std::find(_global_const_values.begin(), _global_const_values.end(), p_value);
		uint32_t pos = 0;
		if (it == _global_const_values.end()) {
			pos = (uint32_t)_global_const_values.size();
			_global_const_values.push_back(p_value);
		} else {
			pos = (uint32_t)std::distance(_global_const_values.begin(), it);
		}

		return pos;
	}

};

}

#endif // BYTECODE_H