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

#ifndef INSTANCE_H
#define INSTANCE_H

#include "core/core.h"
#include "globals.h"
#include "bytecode.h"

namespace carbon {

class Instance : public Object, public std::enable_shared_from_this<Instance> {
	friend class VM;
	friend struct RuntimeContext;
	
	//REGISTER_CLASS(Instance, Object) {}
public:
	typedef Object Super;
	static ptr<Object> __new() { return newptr<Instance>(); }
	static constexpr const char* get_base_type_name_s() { return STR(Object); }
	virtual const char* get_base_type_name() const override { return get_base_type_name_s(); }
	static constexpr const char* get_type_name_s() { return STR(Instance); }
	static void _bind_data(NativeClasses* p_native_classes) {}
	/* overriding from regsiter class */
	virtual const char* get_type_name() const override {
		return blueprint->get_name().c_str();
	}

private: // members
	ptr<Bytecode> blueprint;
	ptr<Object> native_instance;
	stdvec<var> members;

public:
	Instance();
	Instance(ptr<Bytecode>& p_blueprint);

	bool _is_registered() const override;
	var call_method(const String& p_method_name, stdvec<var*>& p_args) override;
	var get_member(const String& p_name) override;
	void set_member(const String& p_name, var& p_value) override;

	ptr<Object> copy(bool p_deep) /*const*/ override;

	// TODO: implement all the operator methods here.
	String to_string() override;
	var __call(stdvec<var*>& p_args) override;

	var __iter_begin() override;
	bool __iter_has_next() override;
	var __iter_next() override;

	var __get_mapped(const var& p_key) /*const*/ override;
	void __set_mapped(const var& p_key, const var& p_val) override;
	int64_t __hash() /*const*/ override;

	var __add(const var& p_other) /*const*/ override;
	var __sub(const var& p_other) /*const*/ override;
	var __mul(const var& p_other) /*const*/ override;
	var __div(const var& p_other) /*const*/ override;

	// += are change into (+) and (=) at compile time and can't return var& here
	//var& __add_eq(const var& p_other) override;
	//var& __sub_eq(const var& p_other) override;
	//var& __mul_eq(const var& p_other) override;
	//var& __div_eq(const var& p_other) override;

	bool __gt(const var& p_other) /*const*/ override;
	bool __lt(const var& p_other) /*const*/ override;
	bool __eq(const var& p_other) /*const*/ override;
};

}

#endif // INSTANCE_H