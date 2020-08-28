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

HEADER_GUARD = 'NATIVE_BIND_GEN_H'

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
		ret += f'args[{j}]'
	return ret

def generage_method_calls(path, num):
	f = open(path, 'w')
	f.write(LICENSE)
	f.write('\n// !!! AUTO GENERATED DO NOT EDIT !!!\n\n')
	f.write(f'#ifndef {HEADER_GUARD}\n#define {HEADER_GUARD}\n\n')
	f.write('namespace carbon {\n\n')

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
		// TODO: enum, const, ...
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

	virtual var get(ptr<Object> self) = 0;
};

template<typename T, typename Class>
class _MemberBind : public MemberBind {
	typedef T Class::* member_ptr_t;
	member_ptr_t member_ptr;
public:
	_MemberBind(const char* p_name, const char* p_class_name, member_ptr_t p_member_ptr) {
		name = p_name;
		class_name = p_class_name;
		member_ptr = p_member_ptr;
	}

	virtual var get(ptr<Object> self) override {
		return ptrcast<Class>(self).get()->*member_ptr;
	}
};

template<typename T, typename Class>
ptr<MemberBind> _bind_member(const char* p_name, const char* p_class_name, T Class::* p_member_ptr) {
	T Class::* member_ptr = p_member_ptr;
	return newptr<_MemberBind<T, Class>>(p_name, p_class_name, member_ptr);
}
// ---------------- MEMBER BIND END --------------------------------------

''')
	## method pointers
	for i in range(num):
		write_template_symbol(f, i, True) ## template<...>
		f.write(f'\nusing M{i} = R(T::*)({get_args_symbol(i, False)});\n\n')

	## function pointers
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'\nusing F{i} = R(*)({get_args_symbol(i, False)});\n\n')

	## class class _MethodBind_M{i}
	for i in range(num):
		write_template_symbol(f, i, True)
		f.write(f'''\n\
class _MethodBind_M{i} : public MethodBind {{
	M{i}<T, R{get_args_symbol(i, True)}> method;
public:
	_MethodBind_M{i}(const char* p_name, const char* p_class_name, int p_argc, M{i}<T, R{get_args_symbol(i, True)}> p_method) {{
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		method = p_method;
	}}
	virtual var call(ptr<Object> self, stdvec<var>& args) override {{
		if (args.size() != {i}) {{
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("method %s takes {i} arguments but %i was given", get_name(), (int)args.size())
			);
		}}
		if constexpr (std::is_same_v<R, void>) {{
			(ptrcast<T>(self).get()->*method)({get_args_call_symbol(i)}); return var();
		}} else {{
			return (ptrcast<T>(self).get()->*method)({get_args_call_symbol(i)});
		}}
	}}
}};

''')
	## class class _StaticFuncBind_F{i}
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'''\n\
class _StaticFuncBind_F{i} : public StaticFuncBind {{
	F{i}<R{get_args_symbol(i, True)}> static_func;
public:
	_StaticFuncBind_F{i}(const char* p_name, const char* p_class_name, int p_argc, F{i}<R{get_args_symbol(i, True)}> p_func) {{
		name = p_name;
		class_name = p_class_name;
		argc = p_argc;
		static_func = p_func;
	}}
	virtual var call(stdvec<var>& args) override {{
		if (args.size() != {i}) {{
			throw Error(Error::INVALID_ARG_COUNT, 
				String::format("function %s takes {i} arguments but %i was given", get_name(), (int)args.size())
			);
		}}
		if constexpr (std::is_same_v<R, void>) {{
			static_func({get_args_call_symbol(i)}); return var();
		}} else {{
			return static_func({get_args_call_symbol(i)});
		}}
	}}
}};

''')

	## _bind_method()
	for i in range(num):
		write_template_symbol(f, i, True)
		f.write(f'''\n\
ptr<MethodBind> _bind_method(const char* method_name, const char* p_class_name, M{i}<T, R{get_args_symbol(i, True)}> m) {{
	return newptr<_MethodBind_M{i}<T, R{get_args_symbol(i, True)}>>(method_name, p_class_name, {i}, m);
}}

''')

	## _bind_func()
	for i in range(num):
		write_template_symbol(f, i, False)
		f.write(f'''\n\
ptr<StaticFuncBind> _bind_static_func(const char* func_name, const char* p_class_name, F{i}<R{get_args_symbol(i, True)}> f) {{
	return newptr<_StaticFuncBind_F{i}<R{get_args_symbol(i, True)}>>(func_name, p_class_name, {i}, f);
}}

''')
	############ VA_ARGS ############
	f.write('''\

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
	''')
	f.write('} // namespace\n\n')
	f.write(f'#endif // {HEADER_GUARD}\n')
	f.close()

if __name__ == '__main__':
	generage_method_calls('core/native_bind.gen.h', 8)