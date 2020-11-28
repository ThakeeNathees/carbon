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

#ifndef _VAR_H
#define _VAR_H

#include "_string.h"
#include "_array.h"
#include "_map.h"
#include "_object.h"
#include "_vector.h" // not a part of var anymore

#define DATA_PTR_CONST(T) reinterpret_cast<const T *>(_data._mem)
#define DATA_PTR_OTHER_CONST(T) reinterpret_cast<const T *>(p_other._data._mem)

#define DATA_PTR(T) reinterpret_cast<T *>(_data._mem)
#define DATA_PTR_OTHER(T) reinterpret_cast<T *>(p_other._data._mem)

namespace carbon {

class var {
public:
	enum Type {
		_NULL, // not initialized.
		VAR,   // any type used only for member info.

		BOOL,
		INT,
		FLOAT,
		STRING,

		// misc types
		ARRAY,
		MAP,
		OBJECT,

		_TYPE_MAX_,
	};

	/* constructors */
	var();
	var(const var& p_copy);
	var(bool p_bool);
	var(int p_int);
	var(size_t p_int);
	var(int64_t p_int);
	var(float p_float);
	var(double p_double);
	var(const char* p_cstring);
	var(const String& p_string);
	var(const Array& p_array);
	var(const Map& p_map);
	~var();
	
	template <typename T=Object>
	var(const ptr<T>& p_ptr) {
		type = OBJECT;
		new(&_data._obj) ptr<Object>(p_ptr);
	}

	template <typename... Targs>
	var operator()(Targs... p_args) {
		return __call(p_args...);
	}

	// Methods.
	inline Type get_type() const { return type; }
	String get_type_name() const;
	//const char* get_parent_class_name() const;
	size_t hash() const;
	static bool is_hashable(var::Type p_type);
	void clear();
	var copy(bool p_deep = false) const;

	constexpr static const char* get_type_name_s(var::Type p_type) {
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
	}

	// Operators.
	operator bool() const;
	operator int64_t() const;
	operator int() const { return (int)operator int64_t(); }
	operator size_t() const { return (size_t)operator int64_t(); }
	operator float() const { return (float)operator double(); }
	operator double() const;
	operator String() const;   // int.operator String() is invalid casting.
	String to_string() const;  // int.to_string() is valid.
	// this treated as: built-in C++ operator[](const char *, int), conflict with operator[](size_t)
	// operator const char* () const;
	operator Array() const;
	operator Map() const;
	operator ptr<Object>() const;

	template <typename T>
	ptr<T> cast_to() const {
		return ptrcast<T>(operator ptr<Object>());
	}

	enum Operator {

		// methamatical operators
		OP_ASSIGNMENT,
		OP_ADDITION,
		OP_SUBTRACTION,
		OP_MULTIPLICATION,
		OP_DIVISION,
		OP_MODULO,
		OP_POSITIVE, // unary
		OP_NEGATIVE, // unary

		// locical operators
		OP_EQ_CHECK,
		OP_NOT_EQ_CHECK,
		OP_LT,
		OP_LTEQ,
		OP_GT,
		OP_GTEQ,
		OP_AND,
		OP_OR,
		OP_NOT, // unary

		// bitwise operators
		OP_BIT_LSHIFT,
		OP_BIT_RSHIFT,
		OP_BIT_AND,
		OP_BIT_OR,
		OP_BIT_XOR,
		OP_BIT_NOT, // unary

		_OP_MAX_,
	};
	static String get_op_name_s(Operator op);

#define _VAR_OP_DECL(m_ret, m_op, m_access)                                                        \
	m_ret operator m_op (bool p_other) m_access { return operator m_op (var(p_other)); }           \
	m_ret operator m_op (int64_t p_other) m_access { return operator m_op (var(p_other)); }        \
	m_ret operator m_op (int p_other)     m_access { return operator m_op (var(p_other)); }        \
	m_ret operator m_op (double p_other) m_access { return operator m_op (var(p_other)); }         \
	m_ret operator m_op (const char* p_other) m_access { return operator m_op (var(p_other)); }    \
	m_ret operator m_op (const var& p_other) m_access
#define VAR_OP_DECL(m_ret, m_op, m_access) _VAR_OP_DECL(m_ret, m_op, m_access)

	VAR_OP_DECL(bool, ==, const);
	VAR_OP_DECL(bool, !=, const);
	VAR_OP_DECL(bool, < , const);
	VAR_OP_DECL(bool, > , const);
	VAR_OP_DECL(bool, <=, const);
	VAR_OP_DECL(bool, >=, const);

	var operator++();
	var operator++(int);
	var operator--();
	var operator--(int);
	bool operator !() const { return !operator bool(); }
	var operator[](const var& p_key) const;
	// var& operator[](const var& p_key); // TODO:

	var __get_mapped(const var& p_key) const;
	void __set_mapped(const var& p_key, const var& p_value);

	var __iter_begin();
	bool __iter_has_next();
	var __iter_next();

	template <typename... Targs>
	var __call(Targs... p_args) {
		stdvec<var> _args = make_stdvec<var>(p_args...);
		stdvec<var*> args; for (var& v : _args) args.push_back(&v);
		return __call_internal(args);
	}
	var __call(stdvec<var*>& p_args) { return __call_internal(p_args); }

	template <typename... Targs>
	var call_method(const String& p_method, Targs... p_args) {
		stdvec<var> _args = make_stdvec<var>(p_args...);
		stdvec<var*> args; for (var& v : _args) args.push_back(&v);
		return call_method_internal(p_method, args);
	}
	var call_method(const String& p_method, stdvec<var*>& p_args) { return call_method_internal(p_method, p_args); }

	var get_member(const String& p_name);
	void set_member(const String& p_name, var& p_value);

	const ptr<MemberInfo> get_member_info(const String& p_name) const;
	static const ptr<MemberInfo> get_member_info_s(var::Type p_type, const String& p_name);
	const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list() const;
	static const stdmap<size_t, ptr<MemberInfo>>& get_member_info_list_s(var::Type p_type);

private:
	var __call_internal(stdvec<var*>& p_args);
	var call_method_internal(const String& p_method, stdvec<var*>& p_args);
public:

	VAR_OP_DECL(var, +, const);
	VAR_OP_DECL(var, -, const);
	VAR_OP_DECL(var, *, const);
	VAR_OP_DECL(var, /, const);
	VAR_OP_DECL(var, %, const);

	/* assignments */
	var& operator=(const var& p_other);
	template<typename T=Object>
	var& operator=(const ptr<T>& p_other) {
		clear_data();
		type = OBJECT;
		new(&_data._obj) ptr<Object>(ptrcast<Object>(p_other));
		return *this;
	}
	VAR_OP_DECL(var&, +=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, -=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, *=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, /=, PLACE_HOLDER_MACRO);
	VAR_OP_DECL(var&, %=, PLACE_HOLDER_MACRO);

private:
	struct VarData {
		VarData() : _float(.0f) {}
		~VarData() {}


		union {
			ptr<Object> _obj;
			Map _map;
			Array _arr;
			String _string;

			bool _bool = false;
			int64_t _int;
			double _float;
		};
	};

	// Methods.
	void copy_data(const var& p_other);
	void clear_data();

	// Members.
	static var tmp;
	Type type = _NULL;
	VarData _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const var& p_var);
};

// Map internal pair struct
struct Map::_KeyValue {
	var key;
	var value;
	_KeyValue() {}
	_KeyValue(const var& p_key, const var& p_value) : key(p_key), value(p_value) {}
};
}


// ******** MEMBER INFO IMPLEMENTATIONS ******************* //
//#include "runtime_types.h"
//#include "type_info.h"
//#include "native.h"

#endif // _VAR_H