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
#ifndef NATIVE_API_H
#define NATIVE_API_H

#include <stdint.h>

namespace carbon {

struct nativeapi {
	/* target argument is the pointer of the owner for the return value to
	   keep the application memory safe, and maintain the ownership.
	*/

	// string api
	void(*new_string)(uint8_t* self, const char* str);
	int (*str_size)(uint8_t* self);
	const char* (*str_c_str)(uint8_t* self);
	long long (*str_to_int)(uint8_t* self);
	double (*str_to_float)(uint8_t* self);
	long long (*str_hash)(uint8_t* self);
	void (*str_upper)(uint8_t* self, uint8_t* target);
	void (*str_lower)(uint8_t* self, uint8_t* target);
	void (*str_substr)(uint8_t* self, int start, int end, uint8_t* target);
	bool(*str_endswith)(uint8_t* self, uint8_t* with);
	bool(*str_startswith)(uint8_t* self, uint8_t* with);
	void (*str_split)(uint8_t* self, uint8_t* delimiter, uint8_t* target);
	void (*str_join)(uint8_t* self, uint8_t* arr, uint8_t* target);

	// array api
	void (*new_array)(uint8_t* self);
	int (*arr_size)(uint8_t* self);
	uint8_t* (*arr_at)(uint8_t* self, int index);
	void (*arr_push_back)(uint8_t* self, uint8_t* val);
	void (*arr_pop_back)(uint8_t* self);
	void (*arr_clear)(uint8_t* self);

	// map api
	void (*new_map)(uint8_t* self);
	int (*map_size)(uint8_t* self);
	void (*map_insert)(uint8_t* self, uint8_t* key, uint8_t* value);
	void (*map_clear)(uint8_t* self);
	bool (*map_has)(uint8_t* self, uint8_t* key);
	void (*map_at)(uint8_t* self, uint8_t* key, uint8_t* target);

	// object api

	// var api
	void (*new_var)(uint8_t* self);
	void (*new_var_from_bool)(uint8_t* self, bool value);
	void (*new_var_from_int)(uint8_t* self, int64_t value);
	void (*new_var_from_float)(uint8_t* self, double value);
	void (*new_var_from_cstr)(uint8_t* self, const char* value);
	void (*new_var_from_str)(uint8_t* self, uint8_t* str);
	void (*new_var_from_array)(uint8_t* self, uint8_t* arr);
	void (*new_var_from_map)(uint8_t* self, uint8_t* map);
	bool (*var_operator_bool)(uint8_t* self);
	uint64_t (*var_operator_int)(uint8_t* self);
	double (*var_operator_float)(uint8_t* self);
	void (*var_operator_string)(uint8_t* self, uint8_t* target);
	void (*var_operator_array)(uint8_t* self, uint8_t* target);
	void (*var_operator_map)(uint8_t* self, uint8_t* target);

	void (*var_call)(uint8_t* self, uint8_t* args, uint8_t* target);
	void (*var_call_method)(uint8_t* self, const char* name, uint8_t* args, uint8_t* target);
	void (*var_get_member)(uint8_t* self, const char* name, uint8_t* target);
	void (*var_set_member)(uint8_t* self, const char* name, uint8_t* value);
	void* (*var_get_data)(uint8_t* self);
	int (*var_get_type)(uint8_t* self);


	void (*arr_to_string)(uint8_t* self, uint8_t* target);
	void (*var_to_string)(uint8_t* self, uint8_t* target);

	// TODO: call static rename prefix
	void (*api_construct)(const char* name, uint8_t* args, uint8_t* target);
	void (*api_call_static)(const char* name, const char* func, uint8_t* args, uint8_t* target);

	// global
	void (*free_var)(uint8_t* vptr);
};

} // namespace carbon

/******************************************************************************************************************/
/*                                         CARBON_WRAPPER                                                         */
/******************************************************************************************************************/

#ifndef NO_CARBON_WRAPPER

#if defined(_MSC_VER) //  Microsoft 
	#define EXPORT __declspec(dllexport)
#elif defined(__GNUC__) //  GCC
	#define EXPORT __attribute__((visibility("default")))
#else // unknown
	#define EXPORT
#endif

#define CLASS_SIZE_STRING   8
#define CLASS_SIZE_ARRAY   16
#define CLASS_SIZE_MAP     16
#define CLASS_SIZE_OBJECT   8
#define CLASS_SIZE_VAR     24

#ifdef CARBON_API_IMPLEMENTATION
	nativeapi* api;
#else
	extern nativeapi* api;
#endif

namespace carbon {

class String;
class Array;
class Map;
class var;

class String {
public:
	uint8_t __bytes[CLASS_SIZE_STRING];

	String(const char* str = "") { api->new_string(__bytes, str); }

	const char* c_str();
	int size();
	long long to_int();
	double to_float();
	long long hash();
	String upper();
	String lower();
	String substr(int start, int end);
	bool endswith(String& with);
	bool startswith(String& with);
	Array split(String& delimeter);
	String join(Array& arr);
};

class Array {
public:
	uint8_t __bytes[CLASS_SIZE_ARRAY];

	Array() { api->new_array(__bytes); }

	int size();
	var* at(int index);
	void push_back(var& val);
	void pop_back();
	void clear();

	String to_string();
};

class Map {
public:
	uint8_t __bytes[CLASS_SIZE_MAP];

	Map() { api->new_map(__bytes); }

	int map_size();
	void map_insert(var& key, var& value);
	void map_clear();
	bool map_has(var& key);
	var map_at(var key);
};


class var {
public:
	uint8_t __bytes[CLASS_SIZE_VAR];

	enum Type {
		_NULL,
		___, // <- internal type DO not use.
		BOOL,
		INT,
		FLOAT,
		STRING,
		ARRAY,
		MAP,
		OBJECT,
		_TYPE_MAX_,
	};

	var() { api->new_var(__bytes); }
	var(bool val) { api->new_var_from_bool(__bytes, val); }
	var(int64_t val) { api->new_var_from_int(__bytes, val); }
	var(double val) { api->new_var_from_float(__bytes, val); }
	var(const char* val) { api->new_var_from_cstr(__bytes, val); }
	var(String val) { api->new_var_from_str(__bytes, val.__bytes); }
	var(Array val) { api->new_var_from_array(__bytes, val.__bytes); }
	var(Map val) { api->new_var_from_map(__bytes, val.__bytes); }

	operator bool() { return api->var_operator_bool(__bytes); }
	operator int64_t() { return api->var_operator_int(__bytes); }
	operator int() { return (int) operator int64_t(); }
	operator unsigned int() { return (unsigned int) operator int64_t(); }
	operator double() { return api->var_operator_float(__bytes); }
	operator float() { return (float) operator double(); }
	operator String() { String target; api->var_operator_string(__bytes, target.__bytes); return target; }
	operator Array() { Array target; api->var_operator_array(__bytes, target.__bytes); return target; }
	operator Map() { Map target; api->var_operator_map(__bytes, target.__bytes); return target; }

	var call(Array& args);
	var call_method(const char* name, Array& args);
	var get_member(const char* name);
	void set_member(const char* name, var& value);
	void* get_data();
	Type get_type();

	String to_string();
};


class CarbonApi {
public:
	static var construct(const char* name, Array& args) { var target;  api->api_construct(name, args.__bytes, target.__bytes); return target; }
	static var call_static(const char* base, const char* attrib, Array& args) {
		var target;
		api->api_call_static(base, attrib, args.__bytes, target.__bytes);
		return target;
	}
};

} // namespace carbon

// --------------- IMPLEMENTATION -------------------

#if defined(CARBON_API_IMPLEMENTATION)

namespace carbon {

#ifdef __cplusplus
extern "C" {
#endif
	EXPORT void varapi_init(carbon::nativeapi* _api) {
		carbon::api = _api;
	}
#ifdef __cplusplus
}
#endif

	const char* String::c_str() { return api->str_c_str(__bytes); }
	int String::size() { return api->str_size(__bytes); }
	long long String::to_int() { return api->str_to_int(__bytes); }
	double String::to_float() { return api->str_to_float(__bytes); }
	long long String::hash() { return api->str_hash(__bytes); }
	String String::upper() { String ret; api->str_upper(__bytes, ret.__bytes); return ret; }
	String String::lower() { String ret; api->str_lower(__bytes, ret.__bytes); return ret; }
	String String::substr(int start, int end) { String ret; api->str_substr(__bytes, start, end, ret.__bytes); return ret; }
	bool String::endswith(String& with) { return api->str_endswith(__bytes, with.__bytes); }
	bool String::startswith(String& with) { return api->str_startswith(__bytes, with.__bytes); }
	Array String::split(String& delimeter) { Array target; api->str_split(__bytes, delimeter.__bytes, target.__bytes); return target; }
	String String::join(Array& arr) { String target; api->str_join(__bytes, arr.__bytes, target.__bytes); return target; }

	int  Array::size() { return api->arr_size(__bytes); }
	var* Array::at(int index) { return (var*)api->arr_at(__bytes, index); }
	void Array::push_back(var& val) { api->arr_push_back(__bytes, val.__bytes); }
	void Array::pop_back() { api->arr_pop_back(__bytes); }
	void Array::clear() { api->arr_clear(__bytes); }
	String Array::to_string() { String target; api->arr_to_string(__bytes, target.__bytes); return target; }

	int  Map::map_size() { return api->map_size(__bytes); }
	void Map::map_insert(var& key, var& value) { api->map_insert(__bytes, key.__bytes, value.__bytes); }
	void Map::map_clear() { api->map_clear(__bytes); }
	bool Map::map_has(var& key) { return api->map_has(__bytes, key.__bytes); }
	var  Map::map_at(var key) { var target; api->map_at(__bytes, key.__bytes, target.__bytes); return target; }


	var  var::call(Array& args) { var target; api->var_call(__bytes, args.__bytes, target.__bytes); return target; }
	var  var::call_method(const char* name, Array& args) { var target; api->var_call_method(__bytes, name, args.__bytes, target.__bytes); return target; }
	var  var::get_member(const char* name) { var target; api->var_get_member(__bytes, name, target.__bytes); return target; }
	void var::set_member(const char* name, var& value) { api->var_set_member(__bytes, name, value.__bytes); }
	String var::to_string() { String target; api->var_to_string(__bytes, target.__bytes); return target; }
	void* var::get_data() { return api->var_get_data(__bytes); }
	var::Type var::get_type() { return (var::Type)api->var_get_type(__bytes); }

} // carbon
#endif // CARBON_API_IMPLEMENTATION
#endif // NO_CARBON_WRAPPER

#endif // NATIVE_API_H

