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

#ifndef  MAP_H
#define  MAP_H

#include "core_internal.h"

namespace carbon {

class var;
class String;
class MemberInfo;

class Map {
public:
	struct _KeyValue;
	typedef stdmap<size_t, _KeyValue> _map_internal_t;

	// Mehtods.
	Map();
	Map(const ptr<_map_internal_t>& p_data);
	Map(const Map& p_copy);
	constexpr static  const char* get_type_name_s() { return "Map"; }

	_map_internal_t* get_data() { return  _data.operator->(); }
	_map_internal_t* get_data() const { return _data.operator->(); }

	Map copy(bool p_deep = true) const;

	// reflection methods.
	var call_method(const String& p_method, const stdvec<var*>& p_args);
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list();
	static bool has_member(const String& p_member);
	static const ptr<MemberInfo> get_member_info(const String& p_member);

	// Wrappers.
	size_t size() const { return _data->size(); }
	bool empty() const { return _data->empty(); }
	void insert(const var& p_key, const var& p_value);
	var operator[](const var& p_key) const;
	var& operator[](const var& p_key);
	var operator[](const char* p_key) const;
	var& operator[](const char* p_key);
	_map_internal_t::iterator begin() const;
	_map_internal_t::iterator end() const;
	_map_internal_t::iterator find(const var& p_key) const;
	void clear();
	bool has(const var& p_key) const;
	// TODO: add more

	// Operators.
	operator bool() const { return empty(); }
	String to_string() const; // operator String() const;
	bool operator ==(const Map& p_other) const;
	Map& operator=(const Map& p_other);

private:
	friend class var;
	
	//ptr<stdmap<var, var>> _data;
	ptr<_map_internal_t> _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const Map& p_dict);
};

}

#endif // MAP_H