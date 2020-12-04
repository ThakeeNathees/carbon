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

#include "internal.h"

namespace carbon {

class String;

class Array {
	friend class var;
public:
	constexpr static  const char* get_type_name_s() { return "Array"; }

	Array();
	Array(const ptr<stdvec<var>>& p_data);
	Array(const Array& p_copy);
	Array(const stdvec<var>& p_data);

	//template <typename... Targs>
	//Array(Targs... p_args) {
	//	_data = newptr<stdvec<var>>();
	//	_make_va_arg_array(p_args...);
	//}

	Array copy(bool p_deep = true) const;
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	const stdvec<var>* get_stdvec() const;
	void* get_data() const;

	// wrappers. TODO: throw errors
	size_t size() const;
	bool empty() const;
	void push_back(const var& p_var);
	void pop_back();
	Array& append(const var& p_var);
	var pop();
	void clear();
	void insert(int64_t p_index, const var& p_var);
	var& at(int64_t p_index);
	void resize(size_t p_size);
	void reserve(size_t p_size);
	Array& sort();
	var& back();
	var& front();

	String to_string() const;
	operator bool() const;
	bool operator ==(const Array& p_other) const;
	Array& operator=(const Array& p_other);
	Array operator+(const Array& p_other) const;
	Array& operator+=(const Array& p_other);
	var& operator[](int64_t p_index) const;
	var& operator[](int64_t p_index);

private:
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