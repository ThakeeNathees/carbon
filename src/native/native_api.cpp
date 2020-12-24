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

#include "native/nativelib.h"
#include "native/native_api.h"

#define _TYPE_CHECK(m_var, m_type)                                                             \
do {																						   \
	THROW_IF_NULLPTR(m_var);															       \
	if (m_var->type != varptr::m_type)														   \
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"%s\".",		   \
			NativeLib::varptr_to_var(m_var).get_type_name().c_str(), var::get_type_name_s(var::m_type))); \
} while (false)

namespace carbon {

// STRING //////////////////////////////////////////////////

static const char* str_c_str(uint8_t* self) {
	return ((String*)self)->c_str();
}

static int str_size(uint8_t* self) {
	return (int)((String*)self)->size();
}

static long long str_to_int(uint8_t* self) {
	return ((String*)self)->to_int();
}

static double  str_to_float(uint8_t* self) {
	return ((String*)self)->to_float();
}

static long long  str_hash(uint8_t* self) {
	return ((String*)self)->hash();
}

static void str_upper(uint8_t* self, uint8_t* target) {
	new (target) String(((String*)self)->upper());
}

static void str_lower(uint8_t* self, uint8_t* target) {
	new (target) String(((String*)self)->lower());
}

static void str_substr(uint8_t* self, int start, int end, uint8_t* target) {
	new (target) String(((String*)self)->substr(start, end));
}

static bool str_endswith(uint8_t* self, uint8_t* with) {
	return ((String*)self)->endswith(*((String*)with));
}

static bool str_startswith(uint8_t* self, uint8_t* with) {
	return ((String*)self)->startswith(*((String*)with));
}

static void str_split(uint8_t* self, uint8_t* delimiter, uint8_t* target) {
	new (target) Array(((String*)self)->split(*((String*)delimiter)));
}

static void str_join(uint8_t* self, uint8_t* arr, uint8_t* target) {
	new (target)String(((String*)self)->join(*((Array*)arr)));
}

// ARRAY //////////////////////////////////////////////////

static uint8_t* arr_at(uint8_t* self, int index) {
	return (uint8_t*)&((Array*)self)->at(index);
}

static int arr_size(uint8_t* self) {
	return (int)((Array*)self)->size();
}

static void arr_push_back(uint8_t* self, uint8_t* val) {
	((Array*)self)->push_back(*((var*)val));
}

static void arr_pop_back(uint8_t* self) {
	((Array*)self)->pop_back();
}

static void arr_clear(uint8_t* self) {
	((Array*)self)->clear();
}

// MAP //////////////////////////////////////////////////

static int map_size(uint8_t* self) {
	return (int)((Map*)self)->size();
}

static void map_insert(uint8_t* self, uint8_t* key, uint8_t* value) {
	((Map*)self)->insert(*(var*)key, *(var*)value);
}
static void map_clear(uint8_t* self) {
	((Map*)self)->clear();
}
static bool map_has(uint8_t* self, uint8_t* key) {
	return ((Map*)self)->has(*(var*)key);
}
static void map_at(uint8_t* self, uint8_t* key, uint8_t* target) {
	new (target) var(((Map*)self)->at(*(var*)key));
}

// VAR //////////////////////////////////////////////////


static void var_call(uint8_t* self, uint8_t* args, uint8_t* target) {
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)((Array*)args)->get_data()) + i;
	}
	new (target) var(((var*)self)->__call(_args));
}

static void var_call_method(uint8_t* self, const char* name, uint8_t* args, uint8_t* target) {
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)((Array*)args)->get_data()) + i;
	}
	new (target) var(((var*)self)->call_method(name, _args));
}

static void var_get_member(uint8_t* self, const char* name, uint8_t* target) {
	new (target) var(((var*)self)->get_member(name));
}

static void var_set_member(uint8_t* self, const char* name, uint8_t* value) {
	((var*)self)->set_member(name, *(var*)value);
}

static void* var_get_data(uint8_t* self) {
	return ((var*)self)->get_data();
}

static int var_get_type(uint8_t* self) {
	return (int)((var*)self)->get_type();
}

/////////////////////////////////////////////////////////////
static void new_string(uint8_t* self, const char* str) {
	new (self) String(str);
}

static void new_array(uint8_t* self) {
	new (self) Array();
}

static void new_map(uint8_t* self) {
	new (self) Map();
}

static void new_var(uint8_t* self) {
	new (self) var();
}

static void new_var_from_bool(uint8_t* self, bool value) {
	new (self) var(value);
}

static void new_var_from_int(uint8_t* self, int64_t value) {
	new (self) var(value);
}

static void new_var_from_float(uint8_t* self, double value) {
	new (self) var(value);
}

static void new_var_from_cstr(uint8_t* self, const char* value) {
	new (self) var(value);
}

static void new_var_from_str(uint8_t* self, uint8_t* str) {
	new (self) var(*(String*)str);
}

static void new_var_from_array(uint8_t* self, uint8_t* arr) {
	new (self) var(*(Array*)arr);
}

static void new_var_from_map(uint8_t* self, uint8_t* map) {
	new (self) var(*(Map*)map);
}


static void arr_to_string(uint8_t* self, uint8_t* target) {
	new (target) String(((Array*)self)->to_string());
}

static void var_to_string(uint8_t* self, uint8_t* target) {
	new (target) String(((var*)self)->to_string());
}

static bool var_operator_bool(uint8_t* self) {
	return ((var*)self)->operator bool();
}

static uint64_t var_operator_int(uint8_t* self) {
	return ((var*)self)->operator int64_t();
}

static double var_operator_float(uint8_t* self) {
	return ((var*)self)->operator double();
}

static void var_operator_string(uint8_t* self, uint8_t* target) {
	new (target) var(((var*)self)->operator String());
}

static void var_operator_array(uint8_t* self, uint8_t* target) {
	new (target) var(((var*)self)->operator Array());
}

static void var_operator_map(uint8_t* self, uint8_t* target) {
	new (target) var(((var*)self)->operator Map());
}

/////////////////////////////////////////////////////////////
static void api_construct(const char* name, uint8_t* args, uint8_t* target) {
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)((Array*)args)->get_data()) + i;
	}
	new (target) var(NativeClasses::singleton()->construct(name, _args));
}

static void api_call_static(const char* base, const char* attrib, uint8_t* args, uint8_t* target) {
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)((Array*)args)->get_data()) + i;
	}
	new (target) var(NativeClasses::singleton()->call_static(base, attrib, _args));
}


/////////////////////////////////////////////////////////////

void NativeLib::_native_api_init() {

	// string api
	api.str_c_str = &str_c_str;
	api.str_size = &str_size;;
	api.str_to_int = &str_to_int;
	api.str_to_float = &str_to_float;
	api.str_hash = &str_hash;
	api.str_upper = &str_upper;
	api.str_lower = &str_lower;
	api.str_substr = &str_substr;
	api.str_endswith = &str_endswith;
	api.str_startswith = &str_startswith;
	api.str_split = &str_split;
	api.str_join = &str_join;

	// array api
	api.arr_size = &arr_size;
	api.arr_at = &arr_at;
	api.arr_push_back = &arr_push_back;
	api.arr_pop_back = &arr_pop_back;
	api.arr_clear = &arr_clear;
	
	// map
	api.map_size = &map_size;
	api.map_insert = &map_insert;
	api.map_clear = &map_clear;
	api.map_has = &map_has;
	api.map_at = &map_at;
	
	// var / object
	api.var_call = &var_call;
	api.var_call_method = &var_call_method;
	api.var_get_member = &var_get_member;
	api.var_set_member = &var_set_member;
	api.var_get_data = &var_get_data;
	api.var_get_type = &var_get_type;

	api.new_string = &new_string;
	api.new_array = &new_array;
	api.new_map = &new_map;
	api.new_var = &new_var;
	api.new_var_from_bool = &new_var_from_bool;
	api.new_var_from_int = &new_var_from_int;
	api.new_var_from_float = &new_var_from_float;
	api.new_var_from_cstr = &new_var_from_cstr;
	api.new_var_from_str = &new_var_from_str;
	api.new_var_from_array = &new_var_from_array;
	api.new_var_from_map = &new_var_from_map;
	api.var_operator_bool = &var_operator_bool;
	api.var_operator_int = &var_operator_int;
	api.var_operator_float = &var_operator_float;
	api.var_operator_string = &var_operator_string;
	api.var_operator_array = &var_operator_array;
	api.var_operator_map = &var_operator_map;

	api.var_to_string = &var_to_string;
	api.arr_to_string = &arr_to_string;

	api.api_construct = &api_construct;
	api.api_call_static = &api_call_static;
	// TODO:

	
	
}

}