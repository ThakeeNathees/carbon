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

#include "var/var.h"
#include "native/iterators.h"

namespace carbon {

std::ostream& operator<<(std::ostream& p_ostream, const String& p_str) {
	p_ostream << p_str.operator std::string();
	return p_ostream;
}
std::istream& operator>>(std::istream& p_istream, String& p_str) {
	p_istream >> *p_str._data;
	return p_istream;
}
std::ostream& operator<<(std::ostream& p_ostream, const var& p_var) {
	p_ostream << p_var.to_string();
	return p_ostream;
}
std::ostream& operator<<(std::ostream& p_ostream, const Array& p_arr) {
	p_ostream << p_arr.to_string();
	return p_ostream;
}
std::ostream& operator<<(std::ostream& p_ostream, const Map& p_map) {
	p_ostream << p_map.to_string();
	return p_ostream;
}

String var::get_op_name_s(Operator op) {
	static const char* _names[] = {
		"OP_ASSIGNMENT",
		"OP_ADDITION",
		"OP_SUBTRACTION",
		"OP_MULTIPLICATION",
		"OP_DIVISION",
		"OP_MODULO",
		"OP_POSITIVE",
		"OP_NEGATIVE",
		"OP_EQ_CHECK",
		"OP_NOT_EQ_CHECK",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_RSHIFT",
		"OP_BIT_AND",
		"OP_BIT_OR",
		"OP_BIT_XOR",
		"OP_BIT_NOT",
		"_OP_MAX_",
	};
	MISSED_ENUM_CHECK(Operator::_OP_MAX_, 23);
	return _names[op];
}

var::Type var::get_type() const { return type; }

void* var::get_data() {
	switch (type) {
		case _NULL:  return nullptr;
		case BOOL:   return (void*)&_data._bool;
		case INT:    return (void*)&_data._int;
		case FLOAT:  return (void*)&_data._float;
		case STRING: return _data._string.get_data();
		case ARRAY:  return _data._arr.get_data();
		case MAP:    return _data._map.get_data();
		case OBJECT: return _data._obj->get_data();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::BUG, "can't reach here.");
}

size_t var::hash() const {
	switch (type) {
		case _NULL:
			THROW_ERROR(Error::NULL_POINTER, "");
		case BOOL:   return std::hash<bool>{}(_data._bool);
		case INT:    return std::hash<int64_t>{}(_data._int);
		case FLOAT:  return std::hash<double>{}(_data._float);
		case STRING: return _data._string.hash();
		case ARRAY:
		case MAP:
			THROW_ERROR(Error::TYPE_ERROR, String::format("key of typt %s is unhashable.", get_type_name().c_str()));
		case OBJECT: DEBUG_BREAK(); // TODO: add hash method for objects?
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool var::is_hashable(var::Type p_type) {
	switch (p_type) {
		case _NULL:  return false;
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
			return true;
		case ARRAY:
		case MAP:
			return false;
		case OBJECT:
			return true;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

const char* var::get_type_name_s(var::Type p_type) {
	switch (p_type) {
		case var::_NULL:  return "null";
		case var::VAR:    return "var";
		case var::BOOL:   return "bool";
		case var::INT:    return "int";
		case var::FLOAT:  return "float";
		case var::STRING: return "String";
		case var::ARRAY:  return "Array";
		case var::MAP:    return "Map";
		case var::OBJECT: return "Object";
		default:
			return "";
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

void var::clear() {
	clear_data();
	type = _NULL;
}

var var::copy(bool p_deep) const {
	switch (type) {
		case _NULL:
		case BOOL:
		case INT:
		case FLOAT:
		case STRING:
			return *this;
		case ARRAY: return _data._arr.copy(p_deep);
		case MAP: return _data._map.copy(p_deep);
		case OBJECT: return _data._obj->copy(p_deep);
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::BUG, "can't reach here.");
}


/* constructors */
var::var() {
	_data._bool = false;
	type = _NULL;
}

var::var(const var& p_copy) {
	copy_data(p_copy);
	type = p_copy.type;
}

var::var(bool p_bool) {
	type = BOOL;
	_data._bool = p_bool;
}

var::var(int p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(size_t p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(int64_t p_int) {
	type = INT;
	_data._int = p_int;
}

var::var(float p_float) {
	type = FLOAT;
	_data._float = p_float;
}

var::var(double p_double) {
	type = FLOAT;
	_data._float = p_double;
}

var::var(const char* p_cstring) {
	if (p_cstring == nullptr) {
		type = _NULL;
	} else {
		type = STRING;
		new(&_data._string) String(p_cstring);
	}
}

var::var(const String& p_string) {
	type = STRING;
	new(&_data._string) String(p_string);
}

var::var(const Array& p_array) {
	type = ARRAY;
	new(&_data._arr) Array(p_array);
}

var::var(const Map& p_map) {
	type = MAP;
	new(&_data._map) Map(p_map);
}

var::var(const ptr<Object>& p_other) {
	type = OBJECT;
	new(&_data._obj) ptr<Object>(p_other);
}

var::~var() {
	clear();
}

/* operator overloading */

#define VAR_OP_PRE_INCR_DECR(m_op)                                                                      \
var var::operator m_op () {                                                                             \
	switch (type) {                                                                                     \
		case INT:  return m_op _data._int;                                                              \
		case FLOAT: return m_op _data._float;                                                           \
		default: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                             \
			String::format("operator " #m_op " not supported on base %s.", get_type_name().c_str()));   \
	}                                                                                                   \
	return var();                                                                                       \
}

#define VAR_OP_POST_INCR_DECR(m_op)                                                                     \
var var::operator m_op(int) {                                                                           \
	switch (type) {                                                                                     \
		case INT: return _data._int m_op;                                                               \
		case FLOAT: return _data._float m_op;                                                           \
		default: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                             \
			String::format("operator " #m_op " not supported on base %s.", get_type_name().c_str()));   \
	}                                                                                                   \
	return var();                                                                                       \
}
VAR_OP_PRE_INCR_DECR(++)
	VAR_OP_PRE_INCR_DECR(--)
	VAR_OP_POST_INCR_DECR(++)
	VAR_OP_POST_INCR_DECR(--)
#undef VAR_OP_PRE_INCR_DECR
#undef VAR_OP_POST_INCR_DECR

	var& var::operator=(const var& p_other) {
	copy_data(p_other);
	return *this;
}

var var::operator[](const var& p_key) const {
	switch (type) {
		// strings can't return char as var&
		case STRING: return _data._string[p_key.operator int64_t()];
		case ARRAY:  return _data._arr[p_key.operator int64_t()];
		case MAP:    return _data._map[p_key];
		case OBJECT: return _data._obj->__get_mapped(p_key);
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

var var::operator[](int index) const {
	return operator[](var(index));
}

var var::__get_mapped(const var& p_key) const {
	switch (type) {
		case STRING: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			int64_t index = p_key;
			return String(_data._string[index]);
		} break;
		case ARRAY: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			int64_t index = p_key;
			return _data._arr[index];
		} break;
		case MAP:
			if (!_data._map.has(p_key)) THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("key %s does not exists on base Map.", p_key.to_string()));
			return _data._map[p_key];
		case OBJECT:
			return _data._obj->__get_mapped(p_key);
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

void var::__set_mapped(const var& p_key, const var& p_value) {
	switch (type) {
		case STRING: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			if (p_value.get_type() != var::STRING) THROW_ERROR(Error::TYPE_ERROR, "expected a string value to assign");
			if (p_value.operator String().size() != 1) THROW_ERROR(Error::TYPE_ERROR, "expected a string of size 1 to assign");
			_data._string[p_key.operator int64_t()] = p_value.operator String()[0];
			return;
		} break;
		case ARRAY: {
			if (p_key.get_type() != var::INT) THROW_ERROR(Error::TYPE_ERROR, "expected a numeric value for indexing.");
			_data._arr[p_key.operator int64_t()] = p_value;
			return;
		} break;
		case MAP:
			_data._map[p_key] = p_value;
			return;
		case OBJECT:
			_data._obj->__set_mapped(p_key, p_value);
			return;
	}
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, String::format("operator[] not supported on base %s", get_type_name().c_str()));
}

var var::__iter_begin() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    return newptr<_Iterator_int>(_data._int);
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: return newptr<_Iterator_String>(&_data._string);
		case var::ARRAY:  return newptr<_Iterator_Array>(&_data._arr);
		case var::MAP:    return newptr<_Iterator_Map>(&_data._map);
		case var::OBJECT: return _data._obj.get()->__iter_begin();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

bool var::__iter_has_next() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not iterable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String instance is not iterable (use __iter_begin on strings for string iterator).");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array instance is not iterable (use __iter_begin on array for array iterator).");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Mpa instance is not iterable (use __iter_begin on map for map iterator).");
		case var::OBJECT: return _data._obj.get()->__iter_has_next();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::__iter_next() {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not iterable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not iterable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not iterable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String instance is not iterable (use __iter_begin on strings for string iterator).");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array instance is not iterable (use __iter_begin on array for array iterator).");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Mpa instance is not iterable (use __iter_begin on map for map iterator).");
		case var::OBJECT: return _data._obj.get()->__iter_next();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::__call(stdvec<var*>& p_args) {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "boolean is not callable.");
		case var::INT:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "integer is not callable.");
		case var::FLOAT:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "float is not callable.");
		case var::STRING: THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "String is not callable.");
		case var::ARRAY:  THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Array is not callable.");
		case var::MAP:    THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED, "Map is not callable.");
		case var::OBJECT: return _data._obj.get()->__call(p_args);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::call_method(const String& p_method, stdvec<var*>& p_args) {

	// check var methods.
	switch (p_method.const_hash()) {
		case "to_string"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return to_string();
		case "copy"_hash:
			if (p_args.size() >= 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at maximum 1 argument.");
			if (p_args.size() == 0) return copy();
			return copy(p_args[0]->operator bool());
		case "hash"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return (int64_t)hash();
		case "get_type_name"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return get_type_name();

		// operators.
		case "__iter_begin"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_begin();
		case "__iter_has_next"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_has_next();
		case "__iter_next"_hash:
			if (p_args.size() != 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 0 argument.");
			return __iter_next();
		case "__get_mapped"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return __get_mapped(*p_args[0]);
		case "__set_mapped"_hash:
			if (p_args.size() != 2) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			__set_mapped(*p_args[0], *p_args[1]); return var();
		case "__add"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator +(*p_args[0]);
		case "__sub"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator -(*p_args[0]);
		case "__mul"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator *(*p_args[0]);
		case "__div"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator /(*p_args[0]);
		case "__add_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator +=(*p_args[0]);
		case "__sub_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator -=(*p_args[0]);
		case "__mul_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator *=(*p_args[0]);
		case "__div_eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator /=(*p_args[0]);
		case "__gt"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator >(*p_args[0]);
		case "__lt"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator <(*p_args[0]);
		case "__eq"_hash:
			if (p_args.size() != 1) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly 1 argument.");
			return operator ==(*p_args[0]);

		case "__call"_hash:
			return __call(p_args);
	}

	// type methods.
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_method.c_str()));
		case var::INT:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_method.c_str()));
		case var::FLOAT:  THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_method.c_str()));
		case var::STRING: return _data._string.call_method(p_method, p_args);
		case var::ARRAY:  return _data._arr.call_method(p_method, p_args);
		case var::MAP:    return _data._map.call_method(p_method, p_args);
		case var::OBJECT: return Object::call_method_s(_data._obj, p_method, p_args);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

var var::get_member(const String& p_name) {
	switch (type) {
		case var::_NULL:   THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_name.c_str()));
		case var::INT:     THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_name.c_str()));
		case var::FLOAT:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_name.c_str()));
		case var::STRING:
		case var::ARRAY:
		case var::MAP:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on base %s.", p_name.c_str(), get_type_name().c_str()));
		case var::OBJECT: return Object::get_member_s(_data._obj, p_name);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

void var::set_member(const String& p_name, var& p_value) {
	switch (type) {
		case var::_NULL:  THROW_ERROR(Error::NULL_POINTER, "");
		case var::BOOL:   THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("boolean has no attribute \"%s\".", p_name.c_str()));
		case var::INT:    THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("int has no attribute \"%s\".", p_name.c_str()));
		case var::FLOAT:  THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("float has no attribute \"%s\".", p_name.c_str()));
		case var::STRING:
		case var::ARRAY:
		case var::MAP:
			THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on \"%s\".", p_name.c_str(), get_type_name().c_str()));
		case var::OBJECT:
			Object::set_member_s(_data._obj, p_name, p_value);
			return;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

String var::to_string() const {
	switch (type) {
		case _NULL: return "null";
		case BOOL: return (_data._bool) ? "true" : "false";
		case INT: return String(_data._int);
		case FLOAT: return String(_data._float);
		case STRING: return _data._string;
		case ARRAY: return _data._arr.to_string();
		case MAP: return _data._map.to_string();
		case OBJECT: return _data._obj->to_string();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	DEBUG_BREAK(); THROW_ERROR(Error::BUG, "can't reach here.");
}

/* casting */
var::operator bool() const {
	switch (type) {
		case _NULL:  return false;
		case BOOL:   return _data._bool;
		case INT:    return _data._int != 0;
		case FLOAT:  return _data._float != 0;
		case STRING: return _data._string.size() != 0;
		case ARRAY:  return !_data._arr.empty();
		case MAP:    return !_data._map.empty();
		case OBJECT: return _data._obj.operator bool();
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"bool\".", get_type_name().c_str()));
}

var::operator int64_t() const {
	switch (type) {
		case BOOL: return _data._bool;
		case INT: return _data._int;
		case FLOAT: return (int)_data._float;
		//case STRING: return  _data._string.to_int(); // throws std::exception
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"int\".", get_type_name().c_str()));
}

var::operator double() const {
	switch (type) {
		case BOOL: return (double)_data._bool;
		case INT: return (double)_data._int;
		case FLOAT: return _data._float;
		//case STRING: return  _data._string.to_float();
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"float\".", get_type_name().c_str()));
}

var::operator String() const {
	if (type != STRING)
		THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String\".", get_type_name().c_str()));
	return _data._string;
}

var::operator Array() const {
	if (type == ARRAY) {
		return _data._arr;
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array\".", get_type_name().c_str()));
}

var::operator Map() const {
	if (type == MAP) {
		return _data._map;
	}
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map\".", get_type_name().c_str()));
}

var::operator int()    { return (int)operator int64_t(); }
var::operator size_t() { return (size_t)operator int64_t(); }
var::operator float()  { return (float)operator double(); }

var::operator int()    const { return (int)operator int64_t(); }
var::operator size_t() const { return (size_t)operator int64_t(); }
var::operator float()  const { return (float)operator double(); }

// casting to pointer
var::operator bool* () {
	if (type == BOOL) return &_data._bool;
	return nullptr;
}

var::operator int64_t* () {
	if (type == INT) return &_data._int;
	return nullptr;
}

var::operator double* () {
	if (type == FLOAT) return &_data._float;
	return nullptr;
}

var::operator String* () {
	if (type == STRING) return &_data._string;
	return nullptr;
}

var::operator Array* () {
	if (type == ARRAY) return &_data._arr;
	return nullptr;
}

var::operator Map* () {
	if (type == MAP) return &_data._map;
	return nullptr;
}

// casting to reference // TODO: move error messages to a general location
var::operator bool&() {
	if (type == BOOL) return _data._bool;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"bool&\".", get_type_name().c_str()));
}

var::operator int64_t&() {
	if (type == INT) return _data._int;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"int64_t&\".", get_type_name().c_str()));
}

var::operator double&() {
	if (type == FLOAT) return _data._float;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"double&\".", get_type_name().c_str()));
}

var::operator String&() {
	if (type == STRING) return _data._string;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String&\".", get_type_name().c_str()));
}

var::operator Array&() {
	if (type == ARRAY) return _data._arr;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array&\".", get_type_name().c_str()));
}

var::operator Map&() {
	if (type == MAP) return _data._map;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map&\".", get_type_name().c_str()));
}

// - const& -
var::operator const String&() const {
	if (type == STRING) return _data._string;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"String&\".", get_type_name().c_str()));
}

var::operator const Array&() const {
	if (type == ARRAY) return _data._arr;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Array&\".", get_type_name().c_str()));
}

var::operator const Map&() const {
	if (type == MAP) return _data._map;
	THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Map&\".", get_type_name().c_str()));
}

/* operator overloading */
		/* comparison */

#define THROW_OPERATOR_NOT_SUPPORTED(m_op)                                                         \
do {                                                                                               \
	THROW_ERROR(Error::OPERATOR_NOT_SUPPORTED,                                                \
		String::format("operator \"" STR(m_op) "\" not supported on operands \"%s\" and \"%s\".",  \
			get_type_name().c_str(), p_other.get_type_name().c_str())                              \
	);                                                                                             \
} while(false)

bool var::operator==(const var& p_other) const {

	switch (type) {
		case _NULL: return p_other.type == _NULL;
		case BOOL:   {
			switch (p_other.type) {
				case BOOL:  return _data._bool == p_other._data._bool;
				case INT:   return _data._bool == (bool) p_other._data._int;
				case FLOAT: return _data._bool == (bool) p_other._data._float;
			}
		} break;
		case INT:    {
			switch (p_other.type) {
				case BOOL:  return (bool) _data._int == p_other._data._bool;
				case INT:   return _data._int        == p_other._data._int;
				case FLOAT: return _data._int        == p_other._data._float;
			}
		} break;
		case FLOAT:  {
			switch (p_other.type) {
				case BOOL:  return (bool)_data._float == p_other._data._bool;
				case INT:   return _data._float       == p_other._data._int;
				case FLOAT: return _data._float       == p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string == p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				return _data._arr == p_other.operator Array();
			}
			break;
		}
		case MAP: {
			if (p_other.type == MAP) {
				return _data._map == p_other.operator Map();
			}
			break;
		}
		case OBJECT: {
			return _data._obj->__eq(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	return false; // different types.
}

bool var::operator!=(const var& p_other) const {
	return !operator==(p_other);
}

bool var::operator<(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int)    _data._bool < (int)p_other._data._bool;
				case INT:   return (int)    _data._bool < p_other._data._int;
				case FLOAT: return (double) _data._bool < p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int < (int)p_other._data._bool;
				case INT:   return _data._int < p_other._data._int;
				case FLOAT: return _data._int < p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float < (double) p_other._data._bool;
				case INT:   return _data._float < p_other._data._int;
				case FLOAT: return _data._float < p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string < p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY)
				return *_data._arr._data.get() < *p_other.operator Array()._data.get();
			break;
		}
		case MAP: {
			break;
		}
		case OBJECT: {
			return _data._obj->__lt(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(<);
}

bool var::operator>(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int)    _data._bool > (int) p_other._data._bool;
				case INT:   return (int)    _data._bool > p_other._data._int;
				case FLOAT: return (double) _data._bool > p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int > (int) p_other._data._bool;
				case INT:   return _data._int > p_other._data._int;
				case FLOAT: return _data._int > p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float > (int)p_other._data._bool;
				case INT:   return _data._float > p_other._data._int;
				case FLOAT: return _data._float > p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string < p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY)
				return *_data._arr._data.get() > *p_other.operator Array()._data.get();
			break;
		}
		case MAP:
			break;
		case OBJECT: {
			return _data._obj->__gt(p_other);
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(>);
}

bool var::operator<=(const var& p_other) const {
	return *this == p_other || *this < p_other;
}
bool var::operator>=(const var& p_other) const {
	return *this == p_other || *this > p_other;
}

var var::operator +(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t) _data._bool + (int64_t) p_other._data._bool;
				case INT:   return (int64_t) _data._bool + p_other._data._int;
				case FLOAT: return (double) _data._bool  + p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int          + (int64_t) p_other._data._bool;
				case INT:   return _data._int          + p_other._data._int;
				case FLOAT: return (double) _data._int + p_other._data._float;
			}

		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float + (double) p_other._data._bool;
				case INT:   return _data._float + (double) p_other._data._int;
				case FLOAT: return _data._float + p_other._data._float;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING)
				return _data._string + p_other._data._string;
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				return _data._arr + p_other._data._arr;
			}
			break;
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__add(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(+);
}

var var::operator-(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t) _data._bool - (int64_t) p_other._data._bool;
				case INT:   return (int64_t) _data._bool - p_other._data._int;
				case FLOAT: return (double) _data._bool  - p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int         - (int64_t)p_other._data._bool;
				case INT:   return _data._int         - p_other._data._int;
				case FLOAT: return (double)_data._int - p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float - (double)p_other._data._bool;
				case INT:   return _data._float - (double)p_other._data._int;
				case FLOAT: return _data._float - p_other._data._float;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__sub(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(-);
}

var var::operator *(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:  return (int64_t)_data._bool * (int64_t)p_other._data._bool;
				case INT:   return (int64_t)_data._bool * p_other._data._int;
				case FLOAT: return (double)_data._bool  * p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:  return _data._int         * (int64_t)p_other._data._bool;
				case INT:   return _data._int         * p_other._data._int;
				case FLOAT: return (double)_data._int * p_other._data._float;
				case STRING: {
					String ret;
					for (int64_t i = 0; i < _data._int; i++) {
						ret += p_other._data._string;
					}
					return ret;
				}
				case ARRAY: {
					Array ret;
					ret._data->reserve(ret._data->size() * _data._int);
					for (int64_t i = 0; i < _data._int; i++) {
						ret._data->insert(ret._data->end(), p_other._data._arr._data->begin(), p_other._data._arr._data->end());
					}
					return ret;
					break;
				}
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  return _data._float * (double)p_other._data._bool;
				case INT:   return _data._float * (double)p_other._data._int;
				case FLOAT: return _data._float * p_other._data._float;
			}
		} break;
		case STRING:
			if (p_other.type == INT) {
				String ret;
				for (int64_t i = 0; i < p_other._data._int; i++) {
					ret += _data._string;
				}
				return ret;
			}
			break;
		case ARRAY:
			if (p_other.type == INT) {
				Array ret;
				ret._data->reserve(_data._arr.size() * p_other._data._int);
				for (int64_t i = 0; i < p_other._data._int; i++) {
					ret._data->insert(ret._data->end(), _data._arr._data->begin(), _data._arr._data->end());
				}
				return ret;
			}
			break;
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(*);
}

var var::operator /(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL:  {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (int64_t)_data._bool / (int64_t)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (int64_t)_data._bool / p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (double)_data._bool  / p_other._data._float;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._int         / (int64_t)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._int         / p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return (double)_data._int / p_other._data._float;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / (double)p_other._data._bool;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / (double)p_other._data._int;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					return _data._float / p_other._data._float;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__div(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(/);
}

var var::operator %(const var& p_other) const {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL: return (int)(_data._bool) % (int)(p_other._data._bool);
				case INT: return (int)_data._bool % p_other._data._int;
			}
		}
		case INT: {
			switch (p_other.type) {
				case BOOL: return _data._int % (int)(p_other._data._bool);
				case INT: return _data._int % p_other._data._int;
			}
		}
		case STRING: {
			THROW_ERROR(Error::NOT_IMPLEMENTED, "TODO: implement this ASAP");
			//switch (p_other.type) {
			//	// check if array or a single var
			//}
		}
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(%%);
}

var& var::operator+=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int)_data._bool    + (int)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int)_data._bool    + p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool + p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   =         _data._int + (int)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int + p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int + p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float + (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float + (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float + p_other._data._float;         return *this;
			}
		} break;
		case STRING: {
			if (p_other.type == STRING) {
				_data._string += p_other._data._string;
				return *this;
			}
			break;
		}
		case ARRAY: {
			if (p_other.type == ARRAY) {
				_data._arr += p_other._data._arr;
				return *this;
			}
			break;
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__add_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(+=);
}

var& var::operator-=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int)_data._bool    - (int)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int)_data._bool    - p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool - p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   = (int)   _data._int - (int)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int - p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int - p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float - (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float - (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float - p_other._data._float;         return *this;
			}
		} break;
		case STRING:
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__sub_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(-=);
}


var& var::operator*=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:  type = INT;   _data._int   = (int64_t)_data._bool    * (int64_t)p_other._data._bool; return *this;
				case INT:   type = INT;   _data._int   = (int64_t)_data._bool    * p_other._data._int;       return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._bool     * p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:                _data._int   = (int64_t)_data._int * (int64_t)p_other._data._bool;  return *this;
				case INT:                 _data._int   =         _data._int  * p_other._data._int;        return *this;
				case FLOAT: type = FLOAT; _data._float = (double)_data._int  * p_other._data._float;      return *this;
				case STRING: {
					String self;
					for (int64_t i = 0; i < _data._int; i++) {
						self += p_other._data._string;
					}
					type = STRING; new(&_data._string) String(self); return *this;
				}
				case ARRAY: {
					Array self;
					for (int64_t i = 0; i < _data._int; i++) {
						self._data->insert(self._data->end(), p_other._data._arr._data->begin(), p_other._data._arr._data->end());
					}
					clear();
					type = ARRAY; _data._arr = self; return *this;
				}
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:  _data._float = _data._float * (double)p_other._data._bool;  return *this;
				case INT:   _data._float = _data._float * (double)p_other._data._int;   return *this;
				case FLOAT: _data._float = _data._float * p_other._data._float;         return *this;
			}
		} break;
		case STRING:
			if (p_other.type == INT) {
				String self;
				for (int64_t i = 0; i < p_other._data._int; i++) {
					self += _data._string;
				}
				_data._string = self; return *this;
			}
			break;
		case ARRAY: {
			if (p_other.type == INT) {
				_data._arr.reserve(_data._arr.size() * p_other._data._int);
				for (int64_t i = 0; i < p_other._data._int -1; i++) {
					_data._arr._data->insert(_data._arr._data->end(), _data._arr._data->begin(), _data._arr._data->end());
				}
				return *this;
			}
		}
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(*=);
}

var& var::operator/=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		case BOOL: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = INT;   _data._int   = (int)_data._bool    / (int)p_other._data._bool; return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = INT;   _data._int   = (int)_data._bool    / p_other._data._int;       return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = FLOAT; _data._float = (double)_data._bool / p_other._data._float;     return *this;
			}
		} break;
		case INT:   {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._int   = (int)   _data._int / (int)p_other._data._bool;  return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._int   =         _data._int / p_other._data._int;        return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					type = FLOAT; _data._float = (double)_data._int / p_other._data._float;      return *this;
			}
		} break;
		case FLOAT: {
			switch (p_other.type) {
				case BOOL:
					if (p_other._data._bool == false) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / (double)p_other._data._bool;  return *this;
				case INT:
					if (p_other._data._int == 0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / (double)p_other._data._int;   return *this;
				case FLOAT:
					if (p_other._data._float == 0.0) THROW_ERROR(Error::ZERO_DIVISION, "");
					_data._float = _data._float / p_other._data._float;         return *this;
			}
		} break;
		case STRING:
			break;
		case ARRAY:
		case MAP:
			break;
		case OBJECT:
			return _data._obj->__mul_eq(p_other);
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
	THROW_OPERATOR_NOT_SUPPORTED(/=);
}

var& var::operator %=(const var& p_other) {
	switch (type) {
		case _NULL: break;
		//case BOOL: break;
		case INT: {
			switch (p_other.type) {
				//case BOOL: _data._int %= (int)(p_other._data._bool); return *this;
				case INT: _data._int %= p_other._data._int; return *this;
			}
		}
	}
	THROW_OPERATOR_NOT_SUPPORTED(%=);
}

void var::copy_data(const var& p_other) {
	clear_data();
	type = p_other.type;
	switch (p_other.type) {
		case var::_NULL: break;
		case var::BOOL:
			_data._bool = p_other._data._bool;
			break;
		case var::INT:
			_data._int = p_other._data._int;
			break;
		case var::FLOAT:
			_data._float = p_other._data._float;
			break;
		case var::STRING:
			new(&_data._string) String(p_other._data._string);
			break;
		case var::ARRAY:
			new(&_data._arr) Array(p_other._data._arr);
			break;
		case var::MAP:
			new(&_data._map) Map(p_other._data._map);
			break;
		case var::OBJECT:
			new(&_data._obj) ptr<Object>(p_other._data._obj);
			return;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

void var::clear_data() {
	switch (type) {
		case var::_NULL:
		case var::BOOL:
		case var::INT:
		case var::FLOAT:
			return;
		case var::STRING:
			_data._string.~String();
			return;
		case var::ARRAY:
			_data._arr.~Array();
			break;
		case var::MAP:
			_data._map.~Map();
			break;
		case var::OBJECT:
			_data._obj = nullptr;
			break;
	}
	MISSED_ENUM_CHECK(_TYPE_MAX_, 9);
}

String var::get_type_name() const {
	if (type == OBJECT) {
		return _data._obj->get_type_name();
	} else {
		return get_type_name_s(type);
	}
}

}
