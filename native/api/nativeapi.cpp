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
#include "native/api/nativeapi.h"

#define _TYPE_CHECK(m_var, m_type)                                                             \
do {																						   \
	THROW_IF_NULLPTR(m_var);															       \
	if (m_var->type != varptr::m_type)														   \
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"%s\".",		   \
			NativeLib::varptr_to_var(m_var).get_type_name().c_str(), var::get_type_name_s(var::m_type))); \
} while (false)

namespace carbon {

nativeapi NativeLib::api;
unsigned int NativeLib::_alloc_id = 0;
stdmap<int, var> NativeLib::allocate;

// STRING //////////////////////////////////////////////////

static int str_size(varptr* self) {
	_TYPE_CHECK(self, STRING);
	return (int)String(self->_str).size();
}

static long long str_to_int(varptr* self) {
	_TYPE_CHECK(self, STRING);
	return String(self->_str).to_int();
}

static double  str_to_float(varptr* self) {
	_TYPE_CHECK(self, STRING);
	return String(self->_str).to_float();
}

static long long  str_hash(varptr* self) {
	_TYPE_CHECK(self, STRING);
	return String(self->_str).hash();
}

static void str_upper(varptr* self, varptr* target) {
	_TYPE_CHECK(self, STRING);
	*((var*)target->_ptr) = String(self->_str).upper();
}

static void str_lower(varptr* self, varptr* target) {
	_TYPE_CHECK(self, STRING);
	NativeLib::varptr_assign(target, String(self->_str).lower());
}

static void str_substr(varptr* self, int start, int end, varptr* target) {
	_TYPE_CHECK(self, STRING);
	NativeLib::varptr_assign(target, String(self->_str).substr(start, end));
}

static bool str_endswith(varptr* self, varptr* with) {
	_TYPE_CHECK(self, STRING);
	_TYPE_CHECK(with, STRING);
	return String(self->_str).endswith(String(with->_str));
}

static bool str_startswith(varptr* self, varptr* with) {
	_TYPE_CHECK(self, STRING);
	_TYPE_CHECK(with, STRING);
	return String(self->_str).startswith(String(with->_str));
}

static void str_split(varptr* self, varptr* delimiter, varptr* target) {
	_TYPE_CHECK(self, STRING);
	NativeLib::varptr_assign(target, String(self->_str).split(String(delimiter->_str)));
}

static void str_join(varptr* self, varptr* arr, varptr* target) {
	_TYPE_CHECK(self, STRING);
	_TYPE_CHECK(arr, ARRAY);
	NativeLib::varptr_assign(target, String(self->_str).join(((var*)arr->_ptr)->operator Array()));
}

// ARRAY //////////////////////////////////////////////////

static varptr arr_at(varptr* self, int p_index) {
	_TYPE_CHECK(self, ARRAY);
	var& elem = ((var*)self->_ptr)->operator Array()[p_index];
	return NativeLib::var_to_varptr(&elem);
}

static int arr_size(varptr* self) {
	_TYPE_CHECK(self, ARRAY);
	return (int)((var*)self->_ptr)->operator Array().size();
}

static void arr_push_back(varptr* self, varptr* val) {
	_TYPE_CHECK(self, ARRAY);
	THROW_IF_NULLPTR(val);
	((var*)self->_ptr)->operator Array().push_back(NativeLib::varptr_to_var(val));
}

static void arr_pop_back(varptr* self) {
	_TYPE_CHECK(self, ARRAY);
	((var*)self->_ptr)->operator Array().pop_back();
}

static void arr_clear(varptr* self) {
	_TYPE_CHECK(self, ARRAY);
	((var*)self->_ptr)->operator Array().clear();
}

// MAP //////////////////////////////////////////////////

static int map_size(varptr* self) {
	_TYPE_CHECK(self, MAP);
	return (int)((var*)self->_ptr)->operator Map().size();
}

static void map_insert(varptr* self, varptr* key, varptr* value) {
	_TYPE_CHECK(self, MAP);
	((var*)self->_ptr)->operator Map().insert(NativeLib::varptr_to_var(key), NativeLib::varptr_to_var(value));
}
static void map_clear(varptr* self) {
	_TYPE_CHECK(self, MAP);
	((var*)self->_ptr)->operator Map().clear();
}
static bool map_has(varptr* self, varptr* key) {
	_TYPE_CHECK(self, MAP);
	return ((var*)self->_ptr)->operator Map().has(NativeLib::varptr_to_var(key));
}
static void map_at(varptr* self, varptr* key, varptr* target) {
	_TYPE_CHECK(self, MAP);
	NativeLib::varptr_assign(target, ((var*)self->_ptr)->operator Map().at(NativeLib::varptr_to_var(key)));
}

// VAR //////////////////////////////////////////////////


static void var_call(varptr* self, varptr* args, varptr* target) {
	_TYPE_CHECK(args, ARRAY);
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)args->_data) + i;
	}
	NativeLib::varptr_assign(target, ((var*)self->_ptr)->__call(_args));
}

static void var_call_method(varptr* self, const char* name, varptr* args, varptr* target) {
	_TYPE_CHECK(args, ARRAY);
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)args->_data) + i;
	}
	NativeLib::varptr_assign(target, ((var*)self->_ptr)->call_method(name, _args));
}

static void var_get_member(varptr* self, const char* name, varptr* target) {
	THROW_IF_NULLPTR(self);
	NativeLib::varptr_assign(target, ((var*)self->_ptr)->get_member(name));
}
static void var_set_member(varptr* self, const char* name, varptr* value) {
	THROW_IF_NULLPTR(self);
	((var*)self->_ptr)->set_member(name, *((var*)value->_ptr));
}


/////////////////////////////////////////////////////////////
static void api_construct(const char* name, varptr* args, varptr* target) {
	_TYPE_CHECK(args, ARRAY);
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)args->_data) + i;
	}
	NativeLib::varptr_assign(target, NativeClasses::singleton()->construct(name, _args));
}

static void api_call_static(const char* name, const char* func, varptr* args, varptr* target) {
	_TYPE_CHECK(args, ARRAY);
	int size = arr_size(args);
	stdvec<var*> _args(size);
	for (int i = 0; i < size; i++) {
		_args[i] = ((var*)args->_data) + i;
	}
	// TODO: move this to native classes and abstract (replace all places checking the same over and over again)
	ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(name, func);
	if (bd == nullptr) THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("no attribute named \"%s\" base %s", func, name));
	switch (bd->get_type()) {
		case BindData::METHOD:
		case BindData::MEMBER_VAR:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("cannot access non static member \"%s\" statically on base %s", func, name));
		case BindData::STATIC_FUNC:
			NativeLib::varptr_assign(target, static_cast<StaticFuncBind*>(bd.get())->call(_args));
			break;
		case BindData::STATIC_VAR:
			NativeLib::varptr_assign(target,
				static_cast<StaticPropertyBind*>(bd.get())->get().__call(_args));
			break;
		case BindData::STATIC_CONST:
		case BindData::ENUM:
		case BindData::ENUM_VALUE:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("member \"%s\" on base %s isn't callable", func, name));
	}
}

varptr NativeLib::new_var() {
	allocate[_alloc_id] = var();
	varptr vptr = var_to_varptr(&(allocate[_alloc_id]));
	vptr.__id_dont_touch = _alloc_id++;
	return vptr;
}

varptr NativeLib::new_var_from(const var& p_from) {
	allocate[_alloc_id] = p_from;
	varptr vptr = var_to_varptr(&(allocate[_alloc_id]));
	vptr.__id_dont_touch = _alloc_id++;
	return vptr;
}

varptr NativeLib::new_string() {
	varptr vptr = new_var_from(String());
	return vptr;
}

varptr NativeLib::new_array() {
	varptr vptr = new_var_from(Array());
	return vptr;
}

varptr NativeLib::new_map() {
	varptr vptr = new_var_from(Map());
	return vptr;
}

void NativeLib::free_var(varptr* p_vptr) {
	if (p_vptr->__id_dont_touch == -1) {
		THROW_ERROR(Error::NULL_POINTER, String::format("varptr %x already freed or never allocated with native api."));
	}
	allocate.erase(p_vptr->__id_dont_touch);
	p_vptr->__id_dont_touch = -1;
	p_vptr->_ptr = nullptr;
	p_vptr->_data = nullptr;
	p_vptr->_str = nullptr;
}

/////////////////////////////////////////////////////////////

void NativeLib::_varapi_init() {

	// string api
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
	api.arr_at = &arr_at;
	api.arr_size = &arr_size;
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

	api.api_construct = &api_construct;
	api.api_call_static = &api_call_static;
	// TODO:

	api.new_var = &NativeLib::new_var;
	api.free_var = &NativeLib::free_var;
	api.new_string = &NativeLib::new_string;
	api.new_array = &NativeLib::new_array;
	api.new_map = &NativeLib::new_map;
	
}

}