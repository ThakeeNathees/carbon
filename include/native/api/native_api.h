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
	void (*new_var_from_int)(uint8_t* self, long long value);
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


	void (*arr_to_string)(uint8_t* self, uint8_t* target);
	void (*var_to_string)(uint8_t* self, uint8_t* target);

	// TODO: call static rename prefix
	void (*api_construct)(const char* name, uint8_t* args, uint8_t* target);
	void (*api_call_static)(const char* name, const char* func, uint8_t* args, uint8_t* target);

	// global
	void (*free_var)(uint8_t* vptr);
};


}
#endif // NATIVE_API_H

