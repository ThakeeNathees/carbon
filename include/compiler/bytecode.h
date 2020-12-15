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

#include "core/core.h"
#include "binary.h"

namespace carbon {

class CarbonFunction;

class Bytecode : public Object {
	REGISTER_CLASS(Bytecode, Object) {}

	friend class CodeGen;
	friend struct CGContext;
	friend class CarbonFunction;

private: // members.
	bool _is_class = false;
	String _name;                            // name for class, path for file.

	// for ClassNode
	bool _has_base = false;
	bool _is_base_native = false;
	ptr<Bytecode> _base = nullptr;
	void* _pending_base = nullptr;           // incase base isn't fully compiled yet. Parser::ClassNode*
	String _base_native;
	ptr<Bytecode> _file = nullptr;

	stdmap<String, ptr<Bytecode>> _classes;  // for FileNode
	stdmap<String, ptr<Bytecode>> _externs;  // imported for FileNode

	stdmap<String, uint32_t> _members;       // member index. offset willbe added for inherited instances.
	stdmap<String, var> _static_vars;        // TODO : change to stdvec<String> since var is stored _member_vars
	stdmap<String, var> _constants;			 // TODO : same as above
	stdmap<String, int64_t> _unnamed_enums;
	stdmap<String, ptr<EnumInfo>> _enums;
	stdmap<String, ptr<CarbonFunction>> _functions;

	stdvec<String> _global_names_array;
	stdmap<String, uint32_t> _global_names;
	stdvec<var> _global_const_values;

	bool _member_info_built = false;          // set to true after _member_info is built
	stdmap<size_t, ptr<MemberInfo>> _member_info;
	stdmap<String, var> _member_vars;         // all members as var (constructed at runtime)

	union {
		CarbonFunction* _main = nullptr;      // file
		CarbonFunction* _constructor;         // class
	};
	ptr<CarbonFunction> _static_initializer = nullptr;
	ptr<CarbonFunction> _member_initializer = nullptr; // class
	bool _static_initialized = false;

	bool _is_compiled = false;
	bool _is_compiling = false;

//------------------------------------------------------------------------

public:
	Bytecode() {}

	void initialize();

	var __call(stdvec<var*>& p_args) override;                                    // constructor
	var call_method(const String& p_method_name, stdvec<var*>& p_args) override;  // static methods.
	var get_member(const String& p_member_name) override;                         // static member, constants, enums, functions ...
	void set_member(const String& p_member_name, var& p_value) override;          // static members.

	bool is_class() const;
	const String& get_name() const;
	var* _get_member_var_ptr(const String& p_member_name);

	bool has_base() const;
	bool is_base_native() const;
	const ptr<Bytecode>& get_base_binary() const;
	const String& get_base_native() const;

	inline int get_member_count() const;
	int get_member_offset() const;
	uint32_t get_member_index(const String& p_name);
	const ptr<MemberInfo> get_member_info(const String& p_member_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();

	stdmap<String, ptr<Bytecode>>& get_classes();
	stdmap<String, ptr<Bytecode>>& get_externs();
	const stdmap<String, ptr<CarbonFunction>>& get_functions() const;
	const stdmap<String, var>& get_constants();
	stdmap<String, var>& get_static_vars();

	ptr<Bytecode> get_class(const String& p_name);
	ptr<Bytecode> get_import(const String& p_name);
	ptr<CarbonFunction> get_function(const String& p_name);
	var* get_static_var(const String& p_name);
	var get_constant(const String& p_name);

	const ptr<Bytecode>&  get_file() const;
	const CarbonFunction* get_main() const;
	const CarbonFunction* get_constructor() const;
	const CarbonFunction* get_member_initializer() const;
	const CarbonFunction* get_static_initializer() const;

	const String& get_global_name(uint32_t p_pos);
	var* get_global_const_value(uint32_t p_index);

private:
	uint32_t _global_name_get(const String& p_name);
	void _build_global_names_array();
	uint32_t _global_const_value_get(const var& p_value);

};

}

#endif // BYTECODE_H