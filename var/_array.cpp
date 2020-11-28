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

#include "_array.h"
#include "var.h"
#include "error.h"
#include "type_info.h"
#include "var_private.h"

namespace carbon {
const stdmap<size_t, ptr<MemberInfo>>& Array::get_member_info_list() {
	static stdmap<size_t, ptr<MemberInfo>> member_info_s = {
		_NEW_METHOD_INFO("size",                                                    var::INT   ),
		_NEW_METHOD_INFO("empty",                                                   var::BOOL  ),
		_NEW_METHOD_INFO("push_back",    _PARAMS("element"),  _TYPES(var::VAR),     var::_NULL ),
		_NEW_METHOD_INFO("pop_back",                                                var::_NULL ),
		_NEW_METHOD_INFO("append",       _PARAMS("element"),  _TYPES(var::VAR),     var::ARRAY ),
		_NEW_METHOD_INFO("pop",                                                     var::VAR   ),
		_NEW_METHOD_INFO("clear",                                                   var::_NULL ),
		_NEW_METHOD_INFO("at",           _PARAMS("index"),    _TYPES(var::INT),     var::VAR   ),
		_NEW_METHOD_INFO("resize",       _PARAMS("size" ),    _TYPES(var::INT),     var::_NULL ),
		_NEW_METHOD_INFO("reserve",      _PARAMS("size" ),    _TYPES(var::INT),     var::_NULL ),
		_NEW_METHOD_INFO("sort",                                                    var::ARRAY ),
	};
	return member_info_s;
}

var Array::call_method(const String& p_method, const stdvec<var*>& p_args) {
	_check_method_and_args<Array>(p_method, p_args);
	switch (p_method.const_hash()) {
		case "size"_hash:      return (int64_t)size();
		case "empty"_hash:     return empty();
		case "push_back"_hash: { push_back(*p_args[0]); return var(); }
		case "pop_back"_hash: { pop_back(); return var(); }
		case "append"_hash:    return append(*p_args[0]);
		case "pop"_hash:       return pop();
		case "clear"_hash: { clear(); return var(); }
		case "insert"_hash: { insert(*p_args[0], *p_args[1]); return var(); }
		case "at"_hash:        return at(p_args[0]->operator int64_t());
		case "resize"_hash: { resize(p_args[0]->operator int64_t()); return var(); }
		case "reserve"_hash: { reserve(p_args[0]->operator int64_t()); return var(); }
		case "sort"_hash: { return sort(); }
	}
	// TODO: add more.
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool Array::has_member(const String& p_member) { return _has_member_impl<Array>(p_member); }
const ptr<MemberInfo> Array::get_member_info(const String& p_member) { return _get_member_info_impl<Array>(p_member); }

var& Array::at(int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return (*_data).at(p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return (*_data).at(size() + p_index);
	throw Error(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}

var& Array::operator[](int64_t p_index) const {
	if (0 <= p_index && p_index < (int64_t)size())
		return _data->operator[](p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return _data->operator[](size() + p_index);
	throw Error(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}
var& Array::operator[](int64_t p_index) {
	if (0 <= p_index && p_index < (int64_t)size())
		return _data->operator[](p_index);
	if ((int64_t)size() * -1 <= p_index && p_index < 0)
		return _data->operator[](size() + p_index);
	throw Error(Error::INVALID_INDEX, String::format("Array index %i is invalid.", p_index));
}



Array::Array(const stdvec<var>& p_data) {
	_data = newptr<stdvec<var>>();
	for (const var& v : p_data) _data->push_back(v);
}

String Array::to_string() const {
	// TODO: if the array contains itself it'll stack overflow.
	std::stringstream ss;
	ss << "[ ";
	for (unsigned int i = 0; i < _data->size(); i++) {
		ss << _data->operator[](i).to_string();
		if (i != _data->size() - 1) ss << ", ";
		else ss << " ";
	}
	ss << "]";
	return ss.str();
}

var Array::pop() { var ret = this->operator[](size() - 1); pop_back(); return ret; }

bool Array::operator ==(const Array& p_other) const {
	if (size() != p_other.size())
		return false;
	for (size_t i = 0; i < size(); i++) {
		if (operator[](i) != p_other[i])
			return false;
	}
	return true;
}

Array Array::copy(bool p_deep) const {
	Array ret;
	for (size_t i = 0; i < size(); i++) {
		if (p_deep)
			ret.push_back(operator[](i).copy(true));
		else
			ret.push_back(operator[](i));
	}
	return ret;
}

Array Array::operator+(const Array& p_other) const {
	Array ret = copy();
	for (size_t i = 0; i < p_other.size(); i++) {
		ret.push_back(p_other[i].copy());
	}
	return ret;
}

Array& Array::operator+=(const Array& p_other) {
	for (size_t i = 0; i < p_other.size(); i++) {
		push_back(p_other[i].copy());
	}
	return *this;
}

Array& Array::operator=(const Array& p_other) {
	_data = p_other._data;
	return *this;
}


} // namespace carbon