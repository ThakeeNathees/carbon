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
#include "varptr.h"

namespace carbon {

struct nativeapi {
	/* target argument is the pointer of the owner for the return value to
	   keep the application memory safe, and maintain the ownership.
	*/

	// string api
	int (*str_size)(varptr* self);
	long long (*str_to_int)(varptr* self);
	double (*str_to_float)(varptr* self);
	long long (*str_hash)(varptr* self);
	void (*str_upper)(varptr* self, varptr* target);
	void (*str_lower)(varptr* self, varptr* target);
	void (*str_substr)(varptr* self, int start, int end, varptr* target);
	bool(*str_endswith)(varptr* self, varptr* with);
	bool(*str_startswith)(varptr* self, varptr* with);
	void (*str_split)(varptr* self, varptr* delimiter, varptr* target);
	void (*str_join)(varptr* self, varptr* arr, varptr* target);

	// array api
	int (*arr_size)(varptr* self);
	varptr(*arr_at)(varptr* self, int index);
	void (*arr_push_back)(varptr* self, varptr* val);
	void (*arr_pop_back)(varptr* self);
	void (*arr_clear)(varptr* self);

	// map api
	int (*map_size)(varptr* self);
	void (*map_insert)(varptr* self, varptr* key, varptr* value);
	void (*map_clear)(varptr* self);
	bool (*map_has)(varptr* self, varptr* key);
	void (*map_at)(varptr* self, varptr* key, varptr* target);

	// object/var api
	void (*var_call)(varptr* self, varptr* args, varptr* target);
	void (*var_call_method)(varptr* self, const char* name, varptr* args, varptr* target);
	void (*var_get_member)(varptr* self, const char* name, varptr* target);
	void (*var_set_member)(varptr* self, const char* name, varptr* value);

	// TODO: call static rename prefix
	void (*api_construct)(const char* name, varptr* args, varptr* target);
	void (*api_call_static)(const char* name, const char* func, varptr* args, varptr* target);

	// global
	varptr(*new_var)();
	varptr(*new_string)();
	varptr(*new_array)();
	varptr(*new_map)();
	void (*free_var)(varptr* vptr);
};

#ifdef SINGLE_IMPLEMENTATION
nativeapi* api;
#else
extern nativeapi* api;
#endif // SINGLE_IMPLEMENTATION


}
#endif // NATIVE_API_H

