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

#include "_error.h"
#include "_string.h"
#include "_array.h"
#include "_map.h"
#include "_object.h"
#include "_vector.h" // not a part of var anymore

#define DATA_PTR_CONST(T) reinterpret_cast<const T *>(_data._mem)
#define DATA_PTR_OTHER_CONST(T) reinterpret_cast<const T *>(p_other._data._mem)

#define DATA_PTR(T) reinterpret_cast<T *>(_data._mem)
#define DATA_PTR_OTHER(T) reinterpret_cast<T *>(p_other._data._mem)

#define _TRY_VAR_STL(m_statement)                    \
do {												 \
	try {											 \
		m_statement;								 \
	} catch (std::exception& err) {					 \
		THROW_ERROR(Error::VALUE_ERROR, err.what()); \
	}												 \
} while (false)

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
	var(const ptr<Object>& p_other);
	~var();
	
	template <typename T=Object>
	var(const ptr<T>& p_ptr) {
		type = OBJECT;
		new(&_data._obj) ptr<Object>(p_ptr);
	}

	static bool is_hashable(var::Type p_type);
	static const char* get_type_name_s(var::Type p_type);
	static String get_op_name_s(Operator op);

	// methods.
	Type get_type() const;
	String get_type_name() const;
	void* get_data();
	size_t hash() const;
	void clear();
	var copy(bool p_deep = false) const;
	String to_string() const;

	// Operators.
	operator bool() const;
	operator int64_t() const;
	operator double() const;
	operator String() const;
	operator Array() const;
	operator Map() const;
	// operator const char* () const; <-- never implement this

	template <typename T>
	operator ptr<T>() const {
		if (type != OBJECT) {
			THROW_ERROR(Error::TYPE_ERROR, String::format("can't cast \"%s\" to \"Object\".", get_type_name().c_str()));
		}

		// TODO: check if type compatable
		return ptrcast<T>(_data._obj);
	}

	operator int();
	operator size_t();
	operator float();
	operator int () const;
	operator size_t () const;
	operator float() const;

	operator bool* ();
	operator int64_t* ();
	operator double* ();
	operator String* ();
	operator Array* ();
	operator Map* ();

	operator bool&();
	operator int64_t&();
	operator double&();
	operator String&();
	operator Array&();
	operator Map&();

	operator const String& () const;
	operator const Array& () const;
	operator const Map& () const;
	

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
	var operator[](int index) const;
	// var& operator[](const var& p_key); // TODO:

	var __get_mapped(const var& p_key) const;
	void __set_mapped(const var& p_key, const var& p_value);

	var __iter_begin();
	bool __iter_has_next();
	var __iter_next();

	//template <typename... Targs>
	//var __call(Targs... p_args) {
	//	stdvec<var> _args = make_stdvec<var>(p_args...);
	//	stdvec<var*> args; for (var& v : _args) args.push_back(&v);
	//	return __call_internal(args);
	//}
	//template <typename... Targs>
	//var operator()(Targs... p_args) {
	//	return __call(p_args...);
	//}

	//template <typename... Targs>
	//var call_method(const String& p_method, Targs... p_args) {
	//	stdvec<var> _args = make_stdvec<var>(p_args...);
	//	stdvec<var*> args; for (var& v : _args) args.push_back(&v);
	//	return call_method_internal(p_method, args);
	//}

	var __call(stdvec<var*>& p_args);
	var call_method(const String& p_method, stdvec<var*>& p_args);

	var get_member(const String& p_name);
	void set_member(const String& p_name, var& p_value);

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

	// private methods.
	void copy_data(const var& p_other);
	void clear_data();

	// private members.
	Type type = _NULL;
	VarData _data;
	friend std::ostream& operator<<(std::ostream& p_ostream, const var& p_var);
};


// THIS NEEDS TO BE DEFINED HERE (NEED VAR DEFINITION BEFORE)
struct Map::_KeyValue {
	var key;
	var value;
	_KeyValue() {}
	_KeyValue(const var& p_key, const var& p_value) : key(p_key), value(p_value) {}
};

}

#endif // _VAR_H
