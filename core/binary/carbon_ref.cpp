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

#include "carbon_ref.h"

namespace carbon {


NativeClassRef::NativeClassRef(const String& p_native_class) {
	_name = p_native_class;
	if (!NativeClasses::singleton()->is_class_registered(_name))
		THROW_BUG(String::format("class \"%s\" not registered in native classes entries.", _name.c_str()));
}

var NativeClassRef::__call(stdvec<var*>& p_args) {
	return NativeClasses::singleton()->construct(_name, p_args);
}

// TODO:
var  NativeClassRef::call_method(const String& p_name, stdvec<var*>& p_args) { return var(); }
var  NativeClassRef::get_member(const String& p_name) { return var(); }
void NativeClassRef::set_member(const String& p_name, var& p_value) {}

//---------------------------------------------------------------------------

BuiltinFuncRef::BuiltinFuncRef() {}
BuiltinFuncRef::BuiltinFuncRef(BuiltinFunctions::Type p_type) : _type(p_type) {}

var BuiltinFuncRef::__call(stdvec<var*>& p_args) {
	var ret;
	BuiltinFunctions::call(_type, p_args, ret);
	return ret;
}

String BuiltinFuncRef::to_string() {
	return String("[builtin_function:") + BuiltinFunctions::get_func_name(_type) + "]";
}

//---------------------------------------------------------------------------

BuiltinTypeRef::BuiltinTypeRef() {}
BuiltinTypeRef::BuiltinTypeRef(BuiltinTypes::Type p_type) :_type(p_type) {}

var BuiltinTypeRef::__call(stdvec<var*>& p_args) {
	return BuiltinTypes::construct(_type, p_args);
}

String BuiltinTypeRef::to_string() {
	return String("[builtin_type:") + BuiltinTypes::get_type_name(_type) + "]";
}



}
