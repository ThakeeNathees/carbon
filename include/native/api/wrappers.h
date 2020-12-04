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

#ifndef NATIVE_WRAPPERS_H
#define NATIVE_WRAPPERS_H

#include "nativeapi.h"

namespace carbon {

class var {
	varptr self;
	friend class String;
	friend class Array;
	friend class Map;
	friend class CarbonApi;

public:
	varptr& ptr() { return self; }
	static var _new() { return var(api->new_var()); }
	void free() { api->free_var(&self); }
	var(varptr vptr) {
		if (vptr._ptr == nullptr) throw "TODO: invalid";
		this->self = vptr;
	}
	// constructors from other types
	var(bool val) { self.type = varptr::BOOL; self._bool = val; }
	var(int val) { self.type = varptr::INT; self._int = val; }
	var(double val) { self.type = varptr::FLOAT; self._float = val; }
	//var(const char* val);
	var(String val);
	var(Array val);
	var(Map val);

	void call(var args, var& target) { api->var_call(&args.self, &args.self, &target.self); }
	void call_method(const char* name, var args, var& target) { api->var_call_method(&self, name, &args.self, &target.self); }
	void get_member(const char* name, var& target) { api->var_get_member(&self, name, &target.self); }
	void set_member(const char* name, var value) { api->var_set_member(&self, name, &value.self); }
};

class String {
	varptr self;
	friend class var;
public:
	varptr& ptr() { return self; }
	static String _new() { return String(api->new_string()); }
	void free() { api->free_var(&self); }
	String(const var& val) : String(val.self) {}
	String(varptr vptr) {
		if (vptr.type != varptr::STRING) throw "TODO: invalid";
		this->self = vptr;
	}

	const char* c_str() { return self._str; }

	int size() { return api->str_size(&self); }
	long long to_int() { return api->str_to_int(&self); }
	double to_float() { return api->str_to_float(&self); }
	long long hash() { return api->str_hash(&self); }
	void upper(var& target) { api->str_upper(&self, &target.self); }
	void lower(var& target) { api->str_lower(&self, &target.self); }
	void substr(int start, int end, var& target) { return api->str_substr(&self, start, end, &target.self); }
	bool endswith(var with) { return api->str_endswith(&self, &with.self); }
	bool startswith(var with) { return api->str_startswith(&self, &with.self); }
	void split(var delimeter, var& target) { api->str_split(&self, &delimeter.self, &target.self); }
	void join(var arr, var& target) { api->str_join(&self, &arr.self, &target.self); }
};


class Array {
	varptr self;
	friend class var;
public:
	varptr& ptr() { return self; }
	static Array _new() { return Array(api->new_array()); }
	void free() { api->free_var(&self); }
	Array(const var& val) : Array(val.self) {}
	Array(varptr vptr) {
		if (vptr.type != varptr::ARRAY) throw "TODO: invalid";
		this->self = vptr;
	}

	int size() { return api->arr_size(&self); }
	var at(int index) { return api->arr_at(&self, index); }
	void push_back(var val) { api->arr_push_back(&self, &val.self); }
	void pop_back() { api->arr_pop_back(&self); }
	void clear() { api->arr_clear(&self); }
};


class Map {
	varptr self;
	friend class var;
public:
	varptr& ptr() { return self; }
	static Map _new() { return Map(api->new_map()); }
	void free() { api->free_var(&self); }
	Map(const var& val) : Map(val.self) {}
	Map(varptr vptr) {
		if (vptr.type != varptr::MAP) throw "TODO: invalid";
		this->self = vptr;
	}

	int map_size() { return api->map_size(&self); }
	void map_insert(var key, var value) { api->map_insert(&self, &key.self, &value.self); }
	void map_clear() { api->map_clear(&self); }
	bool map_has(var key) { api->map_has(&self, &key.self); }
	void map_at(var key, var& target) { api->map_at(&self, &key.self, &target.self); }
};

class CarbonApi {
public:
	static void construct(const char* name, var args, var& target) { api->api_construct(name, &args.self, &target.self); }
	static void call_static(const char* base, const char* attrib, var args, var& target) {
		api->api_call_static(base, attrib, &args.self, &target.self);
	}
};

#ifdef SINGLE_IMPLEMENTATION
//var::var(const char* val) {}
var::var(String val) { self = val.self; }
var::var(Array val) { self = val.self; }
var::var(Map val) { self = val.self; }
#endif // SINGLE_IMPLEMENTATION

}
#endif // NATIVE_WRAPPERS_H
