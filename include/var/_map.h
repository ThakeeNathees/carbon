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

#include "core/internal.h"

namespace carbon {

class String;
class var;

class Map {
	friend class var;
public:
	struct _KeyValue;
	typedef stdmap<size_t, _KeyValue> _map_internal_t;

	constexpr static  const char* get_type_name_s() { return "Map"; }

	// methods.
	Map();
	Map(const ptr<_map_internal_t>& p_data);
	Map(const Map& p_copy);

	void* get_data() const;
	Map copy(bool p_deep = true) const;
	var call_method(const String& p_method, const stdvec<var*>& p_args);

	// Wrappers.
	size_t size() const;
	bool empty() const;
	void insert(const var& p_key, const var& p_value);
	void clear();
	bool has(const var& p_key) const;
	var at(const var& p_key) const;

	String to_string() const;
	operator bool() const;
	bool operator ==(const Map& p_other) const;
	Map& operator=(const Map& p_other);
	var operator[](const var& p_key) const;
	var& operator[](const var& p_key);
	var operator[](const char* p_key) const;
	var& operator[](const char* p_key);

private:
	_map_internal_t::iterator begin() const;
	_map_internal_t::iterator end() const;
	_map_internal_t::iterator find(const var& p_key) const;

	ptr<_map_internal_t> _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const Map& p_dict);
};

}

#endif // MAP_H