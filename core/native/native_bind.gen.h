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

// !!! AUTO GENERATED DO NOT EDIT !!!

#ifndef NATIVE_BIND_GEN_H
#define NATIVE_BIND_GEN_H

namespace carbon {


class BindData {
protected:
	const char* name;
	const char* class_name;

public:
	enum Type {
		METHOD,
		STATIC_FUNC,
		MEMBER_VAR,
		STATIC_VAR,
		STATIC_CONST,
		ENUM,
		ENUM_VALUE,
	};
	virtual Type get_type() const = 0;
	virtual const char* get_name() const { return name; }
	virtual const char* get_class_name() const { return class_name; }
	virtual int get_argc() const { THROW_BUG("invalid call"); }
};

class MethodBind : public BindData {
protected:
	int argc = 0;

public:
	virtual BindData::Type get_type() const { return BindData::METHOD; }
	virtual int get_argc() const { return argc; }

	virtual var call(ptr<Object> self, stdvec<var>& args) = 0;
};

class StaticFuncBind : public BindData {
protected:
	int argc;

public:
	virtual BindData::Type get_type()   const { return BindData::STATIC_FUNC; }
	virtual int get_argc()              const { return argc; }

	virtual var call(stdvec<var>& args) = 0;
};

// ---------------- MEMBER BIND START --------------------------------------
class MemberBind : public BindData {
public:
	virtual BindData::Type get_type() const { return BindData::MEMBER_VAR; }
	virtual var& get(ptr<Object> self) = 0;
};

template<typename Class>
class _MemberBind : public MemberBind {
	typedef var Class::* member_ptr_t;
	member_ptr_t member_ptr;
public:
	_MemberBind(const char* p_name, const char* p_class_name, member_ptr_t p_member_ptr) {
		name = p_name;
		class_name = p_class_name;
		member_ptr = p_member_ptr;
	}

	virtual var& get(ptr<Object> self) override {
		return ptrcast<Class>(self).get()->*member_ptr;
	}
};

template<typename Class>
ptr<MemberBind> _bind_member(const char* p_name, const char* p_class_name, var Class::* p_member_ptr) {
	var Class::* member_ptr = p_member_ptr;
	return newptr<_MemberBind<Class>>(p_name, p_class_name, member_ptr);
}
// ------------------------------------------------------------------------


// ---------------- STATIC MEMBER BIND START ------------------------------
class StaticMemberBind : public BindData {
	var* member = nullptr;
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_VAR; }

	StaticMemberBind(const char* p_name, const char* p_class_name, var* p_member) {
		name = p_name;
		class_name = p_class_name;
		member = p_member;
	}
	virtual var& get() { return *member; }
};

inline ptr<StaticMemberBind> _bind_static_member(const char* p_name, const char* p_class_name, var* p_member) {
	return newptr<StaticMemberBind>(p_name, p_class_name, p_member);
}
// ------------------------------------------------------------------------

// ---------------- STATIC CONST BIND START ------------------------------
class ConstantBind : public BindData {
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_CONST; }
	virtual var get() = 0;
};

template<typename T>
class _ConstantBind : public ConstantBind {
	T* _const = nullptr;
public:
	_ConstantBind(const char* p_name, const char* p_class_name, T* p_const) {
		name = p_name;
		class_name = p_class_name;
		_const = p_const;
	}

	virtual var get() override {
		return *_const;
	}
};

template<typename T>
ptr<ConstantBind> _bind_static_const(const char* p_name, const char* p_class_name, T* p_const) {
	return newptr<_ConstantBind<T>>(p_name, p_class_name, p_const);
}
// ------------------------------------------------------------------------

// ---------------- ENUM BIND START ------------------------------

class EnumBind : public BindData {
	stdvec<std::pair<String, int64_t>> values;
public:
	EnumBind(const char* p_name, const char* p_class_name, const stdvec<std::pair<String, int64_t>>& p_values) {
		name = p_name;
		class_name = p_class_name;
		values = p_values;
	}
	virtual BindData::Type get_type() const { return BindData::ENUM; }
	int64_t get(const String& p_value_name) const {
		for (int i = 0; i < (int)values.size(); i++) {
			if (values[i].first == p_value_name) {
				return values[i].second;
			}
		}
		throw Error(Error::INVALID_GET_INDEX,
			String::format("value \"%s\" isn't exists on enum \"%s\"", p_value_name.c_str(), name)
		);
	}
};
inline ptr<EnumBind> _bind_enum(const char* p_name, const char* p_class_name, const stdvec<std::pair<String, int64_t>>& p_values) {
	return newptr<EnumBind>(p_name, p_class_name, p_values);
}

class EnumValueBind : public BindData {
	int64_t value;
public:
	EnumValueBind(const char* p_name, const char* p_class_name, int64_t p_value) {
		name = p_name;
		class_name = p_class_name;
		value = p_value;
	}
	virtual BindData::Type get_type() const { return BindData::ENUM_VALUE; }
	int64_t get() { return value; }
};

// -----------------------------------------------------------------------

template<typename T, typename R>
using M0 = R(T::*)();

template<typename T, typename R>
using M0_c = R(T::*)() const;

template<typename T, typename R, typename a0>
using M1 = R(T::*)(a0);

template<typename T, typename R, typename a0>
using M1_c = R(T::*)(a0) const;

template<typename T, typename R, typename a0, typename a1>
using M2 = R(T::*)(a0, a1);

template<typename T, typename R, typename a0, typename a1>
using M2_c = R(T::*)(a0, a1) const;

template<typename T, typename R, typename a0, typename a1, typename a2>
using M3 = R(T::*)(a0, a1, a2);

template<typename T, typename R, typename a0, typename a1, typename a2>
using M3_c = R(T::*)(a0, a1, a2) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
using M4 = R(T::*)(a0, a1, a2, a3);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
using M4_c = R(T::*)(a0, a1, a2, a3) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using M5 = R(T::*)(a0, a1, a2, a3, a4);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using M5_c = R(T::*)(a0, a1, a2, a3, a4) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using M6 = R(T::*)(a0, a1, a2, a3, a4, a5);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using M6_c = R(T::*)(a0, a1, a2, a3, a4, a5) const;

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using M7 = R(T::*)(a0, a1, a2, a3, a4, a5, a6);

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using M7_c = R(T::*)(a0, a1, a2, a3, a4, a5, a6) const;

template<typename R>
using F0 = R(*)();

template<typename R, typename a0>
using F1 = R(*)(a0);

template<typename R, typename a0, typename a1>
using F2 = R(*)(a0, a1);

template<typename R, typename a0, typename a1, typename a2>
using F3 = R(*)(a0, a1, a2);

template<typename R, typename a0, typename a1, typename a2, typename a3>
using F4 = R(*)(a0, a1, a2, a3);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
using F5 = R(*)(a0, a1, a2, a3, a4);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
using F6 = R(*)(a0, a1, a2, a3, a4, a5);

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
using F7 = R(*)(a0, a1, a2, a3, a4, a5, a6);

template<typename T, typename R>
class _MethodBind_M0 : public MethodBind {
	M0<T, R> method;
public:
	_MethodBind_M0(const char* p_name, const char* p_class_name, int p_argc, M0<T, R> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 0) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 0 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)();
		}
	}
};

template<typename T, typename R>
class _MethodBind_M0_c : public MethodBind {
	M0_c<T, R> method;
public:
	_MethodBind_M0_c(const char* p_name, const char* p_class_name, int p_argc, M0_c<T, R> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 0) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 0 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)();
		}
	}
};

template<typename T, typename R, typename a0>
class _MethodBind_M1 : public MethodBind {
	M1<T, R, a0> method;
public:
	_MethodBind_M1(const char* p_name, const char* p_class_name, int p_argc, M1<T, R, a0> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 1) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 1 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0]);
		}
	}
};

template<typename T, typename R, typename a0>
class _MethodBind_M1_c : public MethodBind {
	M1_c<T, R, a0> method;
public:
	_MethodBind_M1_c(const char* p_name, const char* p_class_name, int p_argc, M1_c<T, R, a0> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 1) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 1 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1>
class _MethodBind_M2 : public MethodBind {
	M2<T, R, a0, a1> method;
public:
	_MethodBind_M2(const char* p_name, const char* p_class_name, int p_argc, M2<T, R, a0, a1> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 2) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 2 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1>
class _MethodBind_M2_c : public MethodBind {
	M2_c<T, R, a0, a1> method;
public:
	_MethodBind_M2_c(const char* p_name, const char* p_class_name, int p_argc, M2_c<T, R, a0, a1> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 2) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 2 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2>
class _MethodBind_M3 : public MethodBind {
	M3<T, R, a0, a1, a2> method;
public:
	_MethodBind_M3(const char* p_name, const char* p_class_name, int p_argc, M3<T, R, a0, a1, a2> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 3) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 3 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2>
class _MethodBind_M3_c : public MethodBind {
	M3_c<T, R, a0, a1, a2> method;
public:
	_MethodBind_M3_c(const char* p_name, const char* p_class_name, int p_argc, M3_c<T, R, a0, a1, a2> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 3) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 3 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
class _MethodBind_M4 : public MethodBind {
	M4<T, R, a0, a1, a2, a3> method;
public:
	_MethodBind_M4(const char* p_name, const char* p_class_name, int p_argc, M4<T, R, a0, a1, a2, a3> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 4) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 4 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
class _MethodBind_M4_c : public MethodBind {
	M4_c<T, R, a0, a1, a2, a3> method;
public:
	_MethodBind_M4_c(const char* p_name, const char* p_class_name, int p_argc, M4_c<T, R, a0, a1, a2, a3> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 4) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 4 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _MethodBind_M5 : public MethodBind {
	M5<T, R, a0, a1, a2, a3, a4> method;
public:
	_MethodBind_M5(const char* p_name, const char* p_class_name, int p_argc, M5<T, R, a0, a1, a2, a3, a4> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 5) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 5 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _MethodBind_M5_c : public MethodBind {
	M5_c<T, R, a0, a1, a2, a3, a4> method;
public:
	_MethodBind_M5_c(const char* p_name, const char* p_class_name, int p_argc, M5_c<T, R, a0, a1, a2, a3, a4> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 5) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 5 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _MethodBind_M6 : public MethodBind {
	M6<T, R, a0, a1, a2, a3, a4, a5> method;
public:
	_MethodBind_M6(const char* p_name, const char* p_class_name, int p_argc, M6<T, R, a0, a1, a2, a3, a4, a5> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 6) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 6 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _MethodBind_M6_c : public MethodBind {
	M6_c<T, R, a0, a1, a2, a3, a4, a5> method;
public:
	_MethodBind_M6_c(const char* p_name, const char* p_class_name, int p_argc, M6_c<T, R, a0, a1, a2, a3, a4, a5> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 6) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 6 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _MethodBind_M7 : public MethodBind {
	M7<T, R, a0, a1, a2, a3, a4, a5, a6> method;
public:
	_MethodBind_M7(const char* p_name, const char* p_class_name, int p_argc, M7<T, R, a0, a1, a2, a3, a4, a5, a6> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 7) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 7 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		}
	}
};

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _MethodBind_M7_c : public MethodBind {
	M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> method;
public:
	_MethodBind_M7_c(const char* p_name, const char* p_class_name, int p_argc, M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if (args.size() != 7) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes 7 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		}
	}
};

template<typename R>
class _StaticFuncBind_F0 : public StaticFuncBind {
	F0<R> static_func;
public:
	_StaticFuncBind_F0(const char* p_name, const char* p_class_name, int p_argc, F0<R> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 0) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 0 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(); return var();
		} else {
			return static_func();
		}
	}
};

template<typename R, typename a0>
class _StaticFuncBind_F1 : public StaticFuncBind {
	F1<R, a0> static_func;
public:
	_StaticFuncBind_F1(const char* p_name, const char* p_class_name, int p_argc, F1<R, a0> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 1) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 1 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0]); return var();
		} else {
			return static_func(args[0]);
		}
	}
};

template<typename R, typename a0, typename a1>
class _StaticFuncBind_F2 : public StaticFuncBind {
	F2<R, a0, a1> static_func;
public:
	_StaticFuncBind_F2(const char* p_name, const char* p_class_name, int p_argc, F2<R, a0, a1> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 2) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 2 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1]); return var();
		} else {
			return static_func(args[0], args[1]);
		}
	}
};

template<typename R, typename a0, typename a1, typename a2>
class _StaticFuncBind_F3 : public StaticFuncBind {
	F3<R, a0, a1, a2> static_func;
public:
	_StaticFuncBind_F3(const char* p_name, const char* p_class_name, int p_argc, F3<R, a0, a1, a2> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 3) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 3 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1], args[2]); return var();
		} else {
			return static_func(args[0], args[1], args[2]);
		}
	}
};

template<typename R, typename a0, typename a1, typename a2, typename a3>
class _StaticFuncBind_F4 : public StaticFuncBind {
	F4<R, a0, a1, a2, a3> static_func;
public:
	_StaticFuncBind_F4(const char* p_name, const char* p_class_name, int p_argc, F4<R, a0, a1, a2, a3> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 4) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 4 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1], args[2], args[3]); return var();
		} else {
			return static_func(args[0], args[1], args[2], args[3]);
		}
	}
};

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
class _StaticFuncBind_F5 : public StaticFuncBind {
	F5<R, a0, a1, a2, a3, a4> static_func;
public:
	_StaticFuncBind_F5(const char* p_name, const char* p_class_name, int p_argc, F5<R, a0, a1, a2, a3, a4> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 5) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 5 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1], args[2], args[3], args[4]); return var();
		} else {
			return static_func(args[0], args[1], args[2], args[3], args[4]);
		}
	}
};

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
class _StaticFuncBind_F6 : public StaticFuncBind {
	F6<R, a0, a1, a2, a3, a4, a5> static_func;
public:
	_StaticFuncBind_F6(const char* p_name, const char* p_class_name, int p_argc, F6<R, a0, a1, a2, a3, a4, a5> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 6) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 6 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1], args[2], args[3], args[4], args[5]); return var();
		} else {
			return static_func(args[0], args[1], args[2], args[3], args[4], args[5]);
		}
	}
};

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
class _StaticFuncBind_F7 : public StaticFuncBind {
	F7<R, a0, a1, a2, a3, a4, a5, a6> static_func;
public:
	_StaticFuncBind_F7(const char* p_name, const char* p_class_name, int p_argc, F7<R, a0, a1, a2, a3, a4, a5, a6> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if (args.size() != 7) {
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes 7 arguments but %i was given", get_name(), (int)args.size())
			);
		}
		if constexpr (std::is_same_v<R, void>) {
			static_func(args[0], args[1], args[2], args[3], args[4], args[5], args[6]); return var();
		} else {
			return static_func(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		}
	}
};

template<typename T, typename R>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M0<T, R> m) {
	return newptr<_MethodBind_M0<T, R>>(method_name, p_class_name, 0, m);
}

template<typename T, typename R>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M0_c<T, R> m) {
	return newptr<_MethodBind_M0_c<T, R>>(method_name, p_class_name, 0, m);
}

template<typename T, typename R, typename a0>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M1<T, R, a0> m) {
	return newptr<_MethodBind_M1<T, R, a0>>(method_name, p_class_name, 1, m);
}

template<typename T, typename R, typename a0>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M1_c<T, R, a0> m) {
	return newptr<_MethodBind_M1_c<T, R, a0>>(method_name, p_class_name, 1, m);
}

template<typename T, typename R, typename a0, typename a1>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M2<T, R, a0, a1> m) {
	return newptr<_MethodBind_M2<T, R, a0, a1>>(method_name, p_class_name, 2, m);
}

template<typename T, typename R, typename a0, typename a1>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M2_c<T, R, a0, a1> m) {
	return newptr<_MethodBind_M2_c<T, R, a0, a1>>(method_name, p_class_name, 2, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M3<T, R, a0, a1, a2> m) {
	return newptr<_MethodBind_M3<T, R, a0, a1, a2>>(method_name, p_class_name, 3, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M3_c<T, R, a0, a1, a2> m) {
	return newptr<_MethodBind_M3_c<T, R, a0, a1, a2>>(method_name, p_class_name, 3, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M4<T, R, a0, a1, a2, a3> m) {
	return newptr<_MethodBind_M4<T, R, a0, a1, a2, a3>>(method_name, p_class_name, 4, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M4_c<T, R, a0, a1, a2, a3> m) {
	return newptr<_MethodBind_M4_c<T, R, a0, a1, a2, a3>>(method_name, p_class_name, 4, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M5<T, R, a0, a1, a2, a3, a4> m) {
	return newptr<_MethodBind_M5<T, R, a0, a1, a2, a3, a4>>(method_name, p_class_name, 5, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M5_c<T, R, a0, a1, a2, a3, a4> m) {
	return newptr<_MethodBind_M5_c<T, R, a0, a1, a2, a3, a4>>(method_name, p_class_name, 5, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M6<T, R, a0, a1, a2, a3, a4, a5> m) {
	return newptr<_MethodBind_M6<T, R, a0, a1, a2, a3, a4, a5>>(method_name, p_class_name, 6, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M6_c<T, R, a0, a1, a2, a3, a4, a5> m) {
	return newptr<_MethodBind_M6_c<T, R, a0, a1, a2, a3, a4, a5>>(method_name, p_class_name, 6, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M7<T, R, a0, a1, a2, a3, a4, a5, a6> m) {
	return newptr<_MethodBind_M7<T, R, a0, a1, a2, a3, a4, a5, a6>>(method_name, p_class_name, 7, m);
}

template<typename T, typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M7_c<T, R, a0, a1, a2, a3, a4, a5, a6> m) {
	return newptr<_MethodBind_M7_c<T, R, a0, a1, a2, a3, a4, a5, a6>>(method_name, p_class_name, 7, m);
}

template<typename R>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F0<R> f) {
	return newptr<_StaticFuncBind_F0<R>>(func_name, p_class_name, 0, f);
}

template<typename R, typename a0>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F1<R, a0> f) {
	return newptr<_StaticFuncBind_F1<R, a0>>(func_name, p_class_name, 1, f);
}

template<typename R, typename a0, typename a1>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F2<R, a0, a1> f) {
	return newptr<_StaticFuncBind_F2<R, a0, a1>>(func_name, p_class_name, 2, f);
}

template<typename R, typename a0, typename a1, typename a2>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F3<R, a0, a1, a2> f) {
	return newptr<_StaticFuncBind_F3<R, a0, a1, a2>>(func_name, p_class_name, 3, f);
}

template<typename R, typename a0, typename a1, typename a2, typename a3>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F4<R, a0, a1, a2, a3> f) {
	return newptr<_StaticFuncBind_F4<R, a0, a1, a2, a3>>(func_name, p_class_name, 4, f);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F5<R, a0, a1, a2, a3, a4> f) {
	return newptr<_StaticFuncBind_F5<R, a0, a1, a2, a3, a4>>(func_name, p_class_name, 5, f);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F6<R, a0, a1, a2, a3, a4, a5> f) {
	return newptr<_StaticFuncBind_F6<R, a0, a1, a2, a3, a4, a5>>(func_name, p_class_name, 6, f);
}

template<typename R, typename a0, typename a1, typename a2, typename a3, typename a4, typename a5, typename a6>
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F7<R, a0, a1, a2, a3, a4, a5, a6> f) {
	return newptr<_StaticFuncBind_F7<R, a0, a1, a2, a3, a4, a5, a6>>(func_name, p_class_name, 7, f);
}


template<typename T, typename R>
using MVA = R(T::*)(stdvec<var>&);

template<typename R>
using FVA = R(*)(stdvec<var>&);

template<typename T, typename R>
class _MethodBind_MVA : public MethodBind {
	MVA<T, R> method;
public:
	_MethodBind_MVA(const char* p_name, const char* p_class_name, MVA<T, R> p_method) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		method = p_method;
	}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {
		if constexpr (std::is_same_v<R, void>) {
			(ptrcast<T>(self).get()->*method)(args); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args);
		}
	}
};

template<typename R>
class _StaticFuncBind_FVA : public StaticFuncBind {
	FVA<R> static_func;
public:
	_StaticFuncBind_FVA(const char* p_name, const char* p_class_name, FVA<R> p_func) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		static_func = p_func;
	}
	virtual var call(stdvec<var>& args) override {
		if constexpr (std::is_same_v<R, void>) {
			static_func(args); return var();
		} else {
			return static_func(args);
		}
	}
};

template<typename T, typename R>
ptr<MethodBind> _bind_va_method(const char* method_name, const char* p_class_name, MVA<T, R> m) {
	return newptr<_MethodBind_MVA<T, R>>(method_name, p_class_name, m);
}

template<typename R>
ptr<StaticFuncBind> _bind_va_static_func(const char* func_name, const char* p_class_name, FVA<R> f) {
	return newptr<_StaticFuncBind_FVA<R>>(func_name, p_class_name, f);
}
	} // namespace

#endif // NATIVE_BIND_GEN_H
