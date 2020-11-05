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

namespace carbon {

var Bytecode::__call_method(const String& p_method_name, stdvec<var*>& p_args) {
	return nullptr; // TODO:
}

var Bytecode::__get_member(const String& p_member_name) {
	stdmap<String, var>::iterator it = _constants.find(p_member_name);
	if (it != _constants.end()) return it->second;

	it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) return it->second;

	stdmap<String, ptr<_EnumBytes>>::iterator it_en = _enums.find(p_member_name);
	if (it_en != _enums.end()) return it_en->second;

	stdmap<String, int64_t>::iterator it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end()) return it_uen->second;

	THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("%s %s has no member named \"%s\".", ((_is_class)?"type":"file at"), _name.c_str(), p_member_name.c_str()));
}

void Bytecode::__set_member(const String& p_member_name, var& p_value) {
	stdmap<String, var>::iterator it = _constants.find(p_member_name);
	it = _static_vars.find(p_member_name);
	if (it != _static_vars.end()) {
		it->second = p_value;
		return;
	}

	// TODO: check other members only when debugging is enabled.
	it = _constants.find(p_member_name);
	if (it != _constants.end()) THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to a constant value named \"%s\".", it->first.c_str()));

	stdmap<String, ptr<_EnumBytes>>::iterator it_en = _enums.find(p_member_name);
	if (it_en != _enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum type named \"%s\".", it->first.c_str()));

	stdmap<String, int64_t>::iterator it_uen = _unnamed_enums.find(p_member_name);
	if (it_uen != _unnamed_enums.end())  THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("cannot assign to an enum value named \"%s\".", it->first.c_str()));

	THROW_VARERROR(VarError::ATTRIBUTE_ERROR,
		String::format("%s %s has no member named \"%s\".", ((_is_class) ? "type" : "file at"), _name.c_str(), p_member_name.c_str()));
}

}