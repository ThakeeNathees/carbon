LICENSE = '''\
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
'''

## TODO: throw error with __FILE__, __LINE__, __FUNCTION__ in bind data.

HEADER_GUARD = 'NATIVE_BIND_GEN_H'

DEFINE_DECLARE_VAR_TYPE = '''\
template<typename T> struct is_shared_ptr : std::false_type {};
template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

#define DECLARE_VAR_TYPE(m_var_type, m_T)                                                                                     \\
	VarTypeInfo m_var_type;																						              \\
	if constexpr (std::is_same<m_T, void>::value) {																              \\
		m_var_type = var::_NULL;																				              \\
	} else if constexpr (std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, bool>::value) {              \\
		m_var_type = var::BOOL;																					              \\
	} else if constexpr (std::numeric_limits<m_T>::is_integer) {												              \\
		m_var_type = var::INT;																					              \\
	} else if constexpr (std::is_floating_point<m_T>::value) {													              \\
		m_var_type = var::FLOAT;																				              \\
	} else if constexpr (std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, String>::value ||            \\
			std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, const char*>::value) {			          \\
		m_var_type = var::STRING;																				              \\
	} else if constexpr (std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, Array>::value) {             \\
		m_var_type = var::ARRAY;																				              \\
	} else if constexpr (std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, Map>::value) {               \\
		m_var_type = var::MAP;																					              \\
	} else if constexpr (std::is_same<std::remove_const<std::remove_reference<m_T>::type>::type, var>::value) {               \\
		m_var_type = var::VAR;																					              \\
	} else if constexpr (is_shared_ptr<m_T>::value) {																	      \\
		m_var_type = { var::OBJECT, m_T::element_type::get_type_name_s() };                                                  \\
	}

'''

## template<typename T, typename R, typename a0, ...>
def write_template_symbol(f, i, include_T):
	f.write('template<%stypename R'%( 'typename T, ' if include_T else '' ))
	for j in range(i):
		f.write(f', typename a{j}')
	f.write('>')

def get_args_symbol(i, leading_comma):
	ret = ''
	for j in range(i):
		if j > 0 or leading_comma : ret += ', '
		ret += f'a{j}'
	return ret

def get_args_call_symbol(i):
	ret = ''
	for j in range(i):
		if j > 0 : ret += ', '
		ret += f'*args[{j}]'
	return ret

def generage_method_calls(path, num):
	f = open(path, 'w')
	f.write(LICENSE)
	f.write('\n// !!! AUTO GENERATED DO NOT EDIT !!!\n\n')
	f.write(f'#ifndef {HEADER_GUARD}\n#define {HEADER_GUARD}\n\n')
	f.write('namespace carbon {\n\n')
	f.write(DEFINE_DECLARE_VAR_TYPE)
	f.write('''\

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
	virtual const char* get_type_name() const { return class_name; }
	virtual int get_argc() const { DEBUG_BREAK(); THROW_ERROR(Error::BUG, "invalid call"); }
	virtual const ptr<MemberInfo> get_member_info() const = 0;
};

class MethodBind : public BindData {
protected:
	int argc = 0;
	ptr<MethodInfo> mi;

public:
	virtual BindData::Type get_type() const { return BindData::METHOD; }
	virtual int get_argc() const { return argc; }

	virtual var call(ptr<Object> self, stdvec<var*>& args) const = 0;
	const MethodInfo* get_method_info() const { return mi.get(); }
	const ptr<MemberInfo> get_member_info() const override { return mi; }
};

class StaticFuncBind : public BindData {
protected:
	int argc;
	ptr<MethodInfo> mi;

public:
	virtual BindData::Type get_type()   const { return BindData::STATIC_FUNC; }
	virtual int get_argc()              const { return argc; }

	virtual var call(stdvec<var*>& args) const = 0;
	const MethodInfo* get_method_info() const { return mi.get(); }
	const ptr<MemberInfo> get_member_info() const override { return mi; }
};

// ---------------- MEMBER BIND START --------------------------------------
class PropertyBind : public BindData {
protected:
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::MEMBER_VAR; }
	virtual var& get(ptr<Object> self) const = 0;

	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

template<typename Class>
class _PropertyBind : public PropertyBind {
protected:
	typedef var Class::* member_ptr_t;
	member_ptr_t member_ptr;
public:
	_PropertyBind(const char* p_name, const char* p_class_name, member_ptr_t p_member_ptr, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		member_ptr = p_member_ptr;
		pi = p_pi;
	}

	virtual var& get(ptr<Object> self) const override {
		return ptrcast<Class>(self).get()->*member_ptr;
	}
};

template<typename Class>
ptr<PropertyBind> _bind_member(const char* p_name, const char* p_class_name, var Class::* p_member_ptr, const var& p_value = var()) {
	var Class::* member_ptr = p_member_ptr;
	return newptr<_PropertyBind<Class>>(p_name, p_class_name, member_ptr, newptr<PropertyInfo>(p_name, var::VAR, p_value));
}
// ------------------------------------------------------------------------


// ---------------- STATIC MEMBER BIND START ------------------------------
class StaticPropertyBind : public BindData {
	var* member = nullptr;
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_VAR; }

	StaticPropertyBind(const char* p_name, const char* p_class_name, var* p_member, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		member = p_member;
		pi = p_pi;
	}
	virtual var& get() const { return *member; }
	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

inline ptr<StaticPropertyBind> _bind_static_member(const char* p_name, const char* p_class_name, var* p_member) {
	return newptr<StaticPropertyBind>(p_name, p_class_name, p_member, newptr<PropertyInfo>(p_name, var::VAR, *p_member));
}
// ------------------------------------------------------------------------

// ---------------- STATIC CONST BIND START ------------------------------
class ConstantBind : public BindData {
protected:
	ptr<PropertyInfo> pi;
public:
	virtual BindData::Type get_type() const { return BindData::STATIC_CONST; }
	virtual var get() const = 0;

	const ptr<PropertyInfo> get_prop_info() const { return pi; }
	const ptr<MemberInfo> get_member_info() const override { return pi; }
};

template<typename T>
class _ConstantBind : public ConstantBind {
	T* _const = nullptr;
public:
	_ConstantBind(const char* p_name, const char* p_class_name, T* p_const, ptr<PropertyInfo> p_pi) {
		name = p_name;
		class_name = p_class_name;
		_const = p_const;
		pi = p_pi;
	}

	virtual var get() const override {
		return *_const;
	}
};

template<typename T>
ptr<ConstantBind> _bind_static_const(const char* p_name, const char* p_class_name, T* p_const) {
	DECLARE_VAR_TYPE(datatype, T);
	return newptr<_ConstantBind<T>>(p_name, p_class_name, p_const, newptr<PropertyInfo>(p_name, datatype, *p_const));
}
// ------------------------------------------------------------------------

// ---------------- ENUM BIND START ------------------------------

class EnumBind : public BindData {
	ptr<EnumInfo> ei;
public:
	EnumBind(const char* p_name, const char* p_class_name, ptr<EnumInfo> p_ei) {
		name = p_name;
		class_name = p_class_name;
		ei = p_ei;
	}
	virtual BindData::Type get_type() const { return BindData::ENUM; }
	int64_t get(const String& p_value_name) const {
		const stdmap<String, int64_t>& values = ei->get_values();
		stdmap<String, int64_t>::const_iterator it = values.find(p_value_name);
		if (it != values.end()) return it->second;
		throw Error(Error::ATTRIBUTE_ERROR, String::format("value \\"%s\\" isn't exists on enum %s.", p_value_name.c_str(), name));
	}
	const ptr<EnumInfo> get() const { return ei; }
	const ptr<EnumInfo> get_enum_info() const { return ei; }
	const ptr<MemberInfo> get_member_info() const override { return ei; }
};
inline ptr<EnumBind> _bind_enum(const char* p_name, const char* p_class_name, const stdvec<std::pair<String, int64_t>>& p_values) {
	return newptr<EnumBind>(p_name, p_class_name, newptr<EnumInfo>(p_name, p_values));
}

class EnumValueBind : public BindData {
	ptr<EnumValueInfo> evi;
	int64_t value;
public:
	EnumValueBind(const char* p_name, const char* p_class_name, int64_t p_value, ptr<EnumValueInfo> p_evi) {
		name = p_name;
		class_name = p_class_name;
		value = p_value;
		evi = newptr<EnumValueInfo>(p_name, p_value);
	}
	virtual BindData::Type get_type() const { return BindData::ENUM_VALUE; }
	int64_t get() const { return value; }

	const ptr<EnumValueInfo> get_enum_value_info() const { return evi; }
	const ptr<MemberInfo> get_member_info() const override { return evi; }
};

// -----------------------------------------------------------------------

''')
	## method pointers
	for i in range(num):
		write_template_symbol(f, i, True) ## template<...>
		f.write(f'\nusing M{i} = R(T::*)({get_args_symbol(i, False)});\n\n')
		write_template_symbol(f, i, True) ## template<...>
		f.write(f'\nusing M{i}_c = R(T::*)({get_args_symbol(i, False)}) const;\n\n')

	## function pointers
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'\nusing F{i} = R(*)({get_args_symbol(i, False)});\n\n')

	## class class _MethodBind_M{i}
	for i in range(num):
		method_bind_body = f'''\n\
class _MethodBind_M{i}$_c$ : public MethodBind {{
	M{i}$_c$<T, R{get_args_symbol(i, True)}> method;
public:
	_MethodBind_M{i}$_c$(const char* p_name, const char* p_class_name, int p_argc, M{i}$_c$<T, R{get_args_symbol(i, True)}> p_method, ptr<MethodInfo> p_mi) {{
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
		mi = p_mi;
	}}
	virtual var call(ptr<Object> self, stdvec<var*>& args) const override {{

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < {i}) {{ /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly {i} argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", {i} - default_arg_count));
		}} else if (args_given > {i}) {{ /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly {i} argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of {i} argument(s).", {i} - default_arg_count));
		}}

		stdvec<var> default_args_copy;
		for (int i = {i} - args_given; i > 0 ; i--) {{
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}}
		for (var& v : default_args_copy) args.push_back(&v);

		if constexpr (std::is_same<R, void>::value) {{
			(ptrcast<T>(self).get()->*method)({get_args_call_symbol(i)}); return var();
		}} else {{
			return (ptrcast<T>(self).get()->*method)({get_args_call_symbol(i)});
		}}
	}}
}};

'''
		write_template_symbol(f, i, True)
		f.write(method_bind_body.replace("$_c$",  ""))
		write_template_symbol(f, i, True)
		f.write(method_bind_body.replace("$_c$",  "_c"))
	## class class _StaticFuncBind_F{i}
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'''\n\
class _StaticFuncBind_F{i} : public StaticFuncBind {{
	F{i}<R{get_args_symbol(i, True)}> static_func;
public:
	_StaticFuncBind_F{i}(const char* p_name, const char* p_class_name, int p_argc, F{i}<R{get_args_symbol(i, True)}> p_func, ptr<MethodInfo> p_mi) {{
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
		mi = p_mi;
	}}
	virtual var call(stdvec<var*>& args) const override {{

		int default_arg_count = mi->get_default_arg_count();
		int args_given = (int)args.size();
		if (args_given + default_arg_count < {i}) {{ /* Args not enough. */
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly {i} argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", {i} - default_arg_count));
		}} else if (args_given > {i}) {{ /* More args proveded.	*/
			if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, "expected at exactly {i} argument(s).");
			else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format( "expected minimum of %i argument(s) and maximum of {i} argument(s).", {i} - default_arg_count));
		}}

		stdvec<var> default_args_copy;
		for (int i = {i} - args_given; i > 0 ; i--) {{
			default_args_copy.push_back(mi->get_default_args()[default_arg_count - i]);
		}}
		for (var& v : default_args_copy) args.push_back(&v);

		if constexpr (std::is_same<R, void>::value) {{
			static_func({get_args_call_symbol(i)}); return var();
		}} else {{
			return static_func({get_args_call_symbol(i)});
		}}
	}}
}};

''')

	## _bind_method()
	for i in range(num):
		method_bind_func = f'''\n\
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M{i}$_c$<T, R{get_args_symbol(i, True)}> m,
		{' '.join([ "const String& name%s,"%j for j in range(i)])} stdvec<var> default_args = {{}}) {{
		{' '.join(["DECLARE_VAR_TYPE(vt%s, a%s);"%(j, j) for j in range(i)])}
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(method_name, make_stdvec<String>({', '.join(["name%s"%j for j in range(i)])} ), make_stdvec<VarTypeInfo>({', '.join(["vt%s"%j for j in range(i)])}), ret, false, default_args, {i} );
	return newptr<_MethodBind_M{i}$_c$<T, R{get_args_symbol(i, True)}>>(method_name, p_class_name, {i}, m, mi);
}}

'''
		write_template_symbol(f, i, True)
		f.write(method_bind_func.replace("$_c$", ""))
		write_template_symbol(f, i, True)
		f.write(method_bind_func.replace("$_c$", "_c"))

	## _bind_func()
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'''\n\
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F{i}<R{get_args_symbol(i, True)}> f,
		{' '.join([ "const String& name%s,"%j for j in range(i)])} stdvec<var> default_args = {{}}) {{
		{' '.join(["DECLARE_VAR_TYPE(vt%s, a%s);"%(j, j) for j in range(i)])}
		DECLARE_VAR_TYPE(ret, R);
		ptr<MethodInfo> mi = newptr<MethodInfo>(func_name, make_stdvec<String>({', '.join(["name%s"%j for j in range(i)])} ), make_stdvec<VarTypeInfo>({', '.join(["vt%s"%j for j in range(i)])}), ret, true, default_args, {i} );
	return newptr<_StaticFuncBind_F{i}<R{get_args_symbol(i, True)}>>(func_name, p_class_name, {i}, f, mi);
}}

''')
	############ VA_ARGS ############
	f.write('''\

template<typename T, typename R>
using MVA = R(T::*)(stdvec<var*>&);

template<typename R>
using FVA = R(*)(stdvec<var*>&);

template<typename T, typename R>
class _MethodBind_MVA : public MethodBind {
	MVA<T, R> method;
public:
	_MethodBind_MVA(const char* p_name, const char* p_class_name, MVA<T, R> p_method, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		method = p_method;
		mi = p_mi;
	}
	virtual var call(ptr<Object> self, stdvec<var*>& args) const override {
		if constexpr (std::is_same<R, void>::value) {
			(ptrcast<T>(self).get()->*method)(args); return var();
		} else {
			return (ptrcast<T>(self).get()->*method)(args);
		}
	}
	const MethodInfo* get_method_info() const { return mi.get(); }
};

template<typename R>
class _StaticFuncBind_FVA : public StaticFuncBind {
	FVA<R> static_func;
public:
	_StaticFuncBind_FVA(const char* p_name, const char* p_class_name, FVA<R> p_func, ptr<MethodInfo> p_mi) {
		name = p_name;
		class_name = p_class_name;
		argc = -1;
		static_func = p_func;
		mi = p_mi;
	}
	virtual var call(stdvec<var*>& args) const override {
		if constexpr (std::is_same<R, void>::value) {
			static_func(args); return var();
		} else {
			return static_func(args);
		}
	}
	const MethodInfo* get_method_info() const { return mi.get(); }
};

template<typename T, typename R>
ptr<MethodBind> _bind_va_method(const char* method_name, const char* p_class_name, MVA<T, R> m) {
	DECLARE_VAR_TYPE(ret, R);
	ptr<MethodInfo> mi = newptr<MethodInfo>( method_name, make_stdvec<String>(), make_stdvec<VarTypeInfo>(), ret, false, make_stdvec<var>(), -1);
	return newptr<_MethodBind_MVA<T, R>>(method_name, p_class_name, m, mi);
}

template<typename R>
ptr<StaticFuncBind> _bind_va_static_func(const char* func_name, const char* p_class_name, FVA<R> f) {
	DECLARE_VAR_TYPE(ret, R);
	ptr<MethodInfo> mi = newptr<MethodInfo>( func_name, make_stdvec<String>(), make_stdvec<VarTypeInfo>(), ret, true, make_stdvec<var>(), -1);
	return newptr<_StaticFuncBind_FVA<R>>(func_name, p_class_name, f, mi);
}
''')
	f.write('} // namespace\n\n')
	f.write(f'#endif // {HEADER_GUARD}\n')
	f.close()

if __name__ == '__main__':
	generage_method_calls('native_bind.gen.h', 8)
	print("[source gen] native bind source generated!");
