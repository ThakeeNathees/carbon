//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

#ifndef VAR_PRIVATE_H
#define VAR_PRIVATE_H
namespace carbon {

#define _PARAMS(...) make_stdvec<String>(__VA_ARGS__)
#define _TYPES(...) make_stdvec<VarTypeInfo>(__VA_ARGS__)
#define _DEFVALS(...) make_stdvec<var>(__VA_ARGS__)
#define _NEW_METHOD_INFO(m_name, ...) { String(m_name).hash(), newptr<MethodInfo>(m_name, __VA_ARGS__) }

template <typename T>
static void _check_method_and_args(const String& p_method, const stdvec<var*>& p_args, ptr<MemberInfo> p_mi) {
	if (p_mi != nullptr) {
		if (p_mi->get_type() != MemberInfo::METHOD)
			THROW_ERROR(Error::TYPE_ERROR, String::format("member \"%s\" is not callable.", p_method.c_str()));
		const MethodInfo* mp = static_cast<const MethodInfo*>(p_mi.get());
		int arg_count = mp->get_arg_count();
		int default_arg_count = mp->get_default_arg_count();
		if (arg_count != -1) {
			if (p_args.size() + default_arg_count < arg_count) { /* Args not enough. */
				if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at exactly %i argument(s).", arg_count));
				else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", arg_count - default_arg_count));
			} else if (p_args.size() > arg_count) { /* More args proveded.    */
				if (default_arg_count == 0) THROW_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at exactly %i argument(s).", arg_count));
				else THROW_ERROR(Error::INVALID_ARG_COUNT, String::format(
					"expected minimum of %i argument(s) and maximum of %i argument(s).", arg_count - default_arg_count, arg_count));
			}
		}
		for (int j = 0; j < mp->get_arg_types().size(); j++) {
			if (mp->get_arg_types()[j] == VarTypeInfo(var::VAR)) continue; /* can't be _NULL. */
			if (p_args.size() == j) break; /* rest are default args. */
			if (mp->get_arg_types()[j] != VarTypeInfo(p_args[j]->get_type(), p_args[j]->get_type_name().c_str()))
				THROW_ERROR(Error::TYPE_ERROR, String::format(
					"expected type %s at argument %i.", var::get_type_name_s(mp->get_arg_types()[j].type), j));
		}
	} else {
		THROW_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" not exists on base %s.", p_method.c_str(), T::get_type_name_s()));
	}
}


}
#endif // VAR_PRIVATE_H
