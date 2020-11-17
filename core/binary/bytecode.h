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
	REGISTER_CLASS(Bytecode, Object) {
		//BIND_STATIC_FUNC("Bytecode", &Bytecode::_Bytecode, PARAMS("self",  ...));

		//BIND_METHOD("method_name", &Bytecode::method_name, PARAMS("param_name"));
		//BIND_METHOD("method_name", &Bytecode::method_name, PARAMS("param_name"));
	}

public:
	Bytecode() {}

	var __call_method(const String& p_method_name, stdvec<var*>& p_args) override; // static methods.
	var __get_member(const String& p_member_name) override; // static member, constants, enums.
	void __set_member(const String& p_member_name, var& p_value) override; // static members.

	int get_member_offset() const {
		return (_base != nullptr ? _base->get_member_offset() : 0) + (int)_members.size();
	}

	uint32_t get_member_index(const String& p_name) {
		ASSERT(_is_class && _file != nullptr);
		auto it = _file->_global_names.find(p_name);

		if (it == _file->_global_names.end()) {
			ASSERT(_base != nullptr);
			return _base->get_member_index(p_name);
		} else {
			ASSERT(_members.find(it->second) != _members.end());
			return get_member_offset() + _members[it->second];
		}
	}

	const stdmap<String, ptr<Bytecode>>& get_classes() const { return _classes; }
	ptr<MemberInfo> get_member_info(const String& p_member_name);

private:
	friend class CodeGen;
	friend struct CGContext;
	bool _is_class = false;

	String _name; // name for class, path for file.
	
	stdmap<String, ptr<Bytecode>> _classes; // for FileNode
	ptr<Bytecode> _base = nullptr; // for ClassNode
	Bytecode* _file = nullptr; // for ClassNode

	typedef uint32_t global_name_pos; // instead of string.
	stdmap<global_name_pos, uint32_t> _members; // member index. offset willbe added for inherited instances.
	stdmap<global_name_pos, var> _static_vars;
	stdmap<global_name_pos, var> _constants;
	stdmap<global_name_pos, int64_t> _unnamed_enums;
	stdmap<global_name_pos, ptr<EnumInfo>> _enums;
	stdmap<global_name_pos, ptr<CarbonFunction>> _functions;

	stdmap<String, uint32_t> _global_names;
	stdvec<var> _global_const_values;

	stdmap<String, ptr<MemberInfo>> _member_info;

	bool _is_compiled = false;
	bool _is_compinling = false;

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