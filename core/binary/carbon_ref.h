#pragma once//------------------------------------------------------------------------------
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

#ifndef CARBON_REF_H
#define CARBON_REF_H

#include "binary.h"
#include "builtin/builtin_functions.h"
#include "builtin/builtin_types.h"

namespace carbon {

class NativeClassRef : public Object {
	REGISTER_CLASS(NativeClassRef, Object) {}

private:
	String _name;

public:
	NativeClassRef() {}
	NativeClassRef(const String& p_native_class);

	bool _is_native_ref() const override { return true; }
	String _get_native_ref() const { return _name; }

	var __call(stdvec<var*>& p_args) override; // construct
	var call_method(const String& p_name, stdvec<var*>& p_args) override; // static method call
	var get_member(const String& p_name) override; // static member, constants, functions, ...
	void set_member(const String& p_name, var& p_value) override; // static members
};

class BuiltinFuncRef : public Object {
	REGISTER_CLASS(BuiltinFuncRef, Object) {}
private:
	BuiltinFunctions::Type _type;
public:
	BuiltinFuncRef() {}
	BuiltinFuncRef(BuiltinFunctions::Type p_type) : _type(p_type) {}

	var __call(stdvec<var*>& p_args) override {
		var ret;
		BuiltinFunctions::call(_type, p_args, ret);
		return ret;
	}

	String to_string() const override {
		return String("[builtin_function:") + BuiltinFunctions::get_func_name(_type) + "]";
	}

	// TODO: add wrapper methods.
};

class BuiltinTypeRef : public Object {
	REGISTER_CLASS(BuiltinTypeRef, Object) {}

private:
	BuiltinTypes::Type _type;

public:
	BuiltinTypeRef() {}
	BuiltinTypeRef(BuiltinTypes::Type p_type) :_type(p_type) {}

	var __call(stdvec<var*>& p_args) override {
		return BuiltinTypes::construct(_type, p_args);
	}

	String to_string() const override {
		return String("[builtin_type:") + BuiltinTypes::get_type_name(_type) + "]";
	}

};

}

#endif // CARBON_REF_H