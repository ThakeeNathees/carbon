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

#ifndef  ARRAY_H
#define  ARRAY_H

#include "core_internal.h"

namespace carbon {

class String;
class var;
class MemberInfo;

class Array {
public:
	Array() {
		_data = newptr<stdvec<var>>();
	}
	Array(const ptr<stdvec<var>>& p_data) {
		_data = p_data;
	}
	Array(const Array& p_copy) {
		_data = p_copy._data;
	}
	Array(const stdvec<var>& p_data);

	template <typename... Targs>
	Array(Targs... p_args) {
		_data = newptr<stdvec<var>>();
		_make_va_arg_array(p_args...);
	}

	stdvec<var>* get_data() {
		return _data.operator->();
	}
	stdvec<var>* get_data() const {
		return _data.operator->();
	}
	constexpr static  const char* get_type_name_s() { return "Array"; }

	Array copy(bool p_deep = true) const;

	// reflection methods.
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();
	static bool has_member(const String& p_member);
	static const ptr<MemberInfo> get_member_info(const String& p_member);

	// Wrappers.
	// TODO: throw all errors with VarError
	size_t size() const { return _data->size(); }
	bool empty() const { return _data->empty(); }
	void push_back(const var& p_var) { _data->push_back(p_var); }
	void pop_back() { _data->pop_back(); }
	Array& append(const var& p_var) { push_back(p_var); return *this; }
	var pop();
	void clear() { (*_data).clear(); }
	void insert(int64_t p_index, const var& p_var) { _data->insert(_data->begin() + p_index, p_var); }
	var& at(int64_t p_index);
	void resize(size_t p_size) { _data->resize(p_size); }
	void reserve(size_t p_size) { _data->reserve(p_size); }
	Array& sort() { std::sort(_data->begin(), _data->end()); return *this; }

	std::vector<var>::const_iterator begin() const { return (*_data).begin(); }
	std::vector<var>::const_iterator end() const { return (*_data).end(); }
	var& back() { return (*_data).back(); }
	var& front() { return (*_data).front(); }

	var& operator[](int64_t p_index) const;
	var& operator[](int64_t p_index);
	// TODO: 

	// Operators.
	operator bool() const { return empty(); }
	String to_string() const; // operator String() const;
	bool operator ==(const Array& p_other) const;
	Array& operator=(const Array& p_other);
	Array operator+(const Array& p_other) const;
	Array& operator+=(const Array& p_other);

private:
	friend class var;
	ptr<std::vector<var>> _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const Array& p_arr);

	// va_args constructor internal
	template <typename T, typename... Targs>
	constexpr void _make_va_arg_array(T p_val, Targs... p_args) {
		push_back(p_val);
		_make_va_arg_array(p_args...);
	}
	void _make_va_arg_array() { return; }
};

}

#endif // ARRAY_H