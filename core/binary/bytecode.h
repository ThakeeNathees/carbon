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

class _EnumBytes : public Object {
	REGISTER_CLASS(_EnumBytes, Object) { }

	var __get_member(const String& p_name) override {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) return it->second;
		else THROW_VARERROR(VarError::ATTRIBUTE_ERROR, String::format("enum %s has no value named \"%s\".", _name.c_str(), p_name.c_str()));
	}
	void __set_member(const String& p_name, var& p_value) {
		stdmap<String, int64_t>::iterator it = _values.find(p_name);
		if (it != _values.end()) THROW_VARERROR(VarError::ATTRIBUTE_ERROR, String::format("cannot assign a value to enum value."));
		else THROW_VARERROR(VarError::ATTRIBUTE_ERROR, String::format("enum %s has no member named \"%s\".", _name.c_str(), p_name.c_str()));
	}

	_EnumBytes() {}
	_EnumBytes(const String& p_name) :_name(p_name) {}

public: // like a struct.
	String _name; // points to the name in enums map.
	stdmap<String, int64_t> _values;
};

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

	const stdmap<String, ptr<Bytecode>>& get_classes() const { return _classes; }

private:
	friend class Compiler;
	bool _is_class = false;

	String _path; // for FileNode
	String _name; // for ClassNode
	
	stdmap<String, ptr<Bytecode>> _classes; // for FileNode
	ptr<Bytecode> _base = nullptr; // for ClassNode

	struct _MemberData {
		int index; // member index. offset willbe added for inherited instances.
		var default_value;
	}; stdmap<String, _MemberData> _members;

	stdmap<String, var> _static_vars;
	stdmap<String, var> _constants;
	stdmap<String, int64_t> _unnamed_enums;
	stdmap<String, ptr<_EnumBytes>> _enums;
	// TODO: function pointers

	bool _is_compiled = false;
	bool _is_compinling = false;
};

}

#endif // BYTECODE_H