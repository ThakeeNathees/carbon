//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c), 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"),, to deal
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

#ifndef ANALYZER_H
#define ANALYZER_H

#include "core/core.h"
#include "parser.h"

#define ANALYZER_ERROR(m_type, m_msg, m_pos) _analyzer_error(m_type, m_msg, m_pos, _DBG_SOURCE)
#define ANALYZER_WARNING(m_type, m_msg, m_pos) _analyzer_warning(m_type, m_msg, m_pos, _DBG_SOURCE)

namespace carbon {

class Analyzer {
public:
	void analyze(ptr<Parser> p_parser);
	const stdvec<Warning>& get_warnings() const;
private:
	friend class CodeGen;
	ptr<Parser> parser;
	ptr<Parser::FileNode> file_node; // Quick access.
	stdvec<Warning> warnings;

	template<typename T = Parser::Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		return ret;
	}

	CompileTimeError _analyzer_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;
	 void _analyzer_warning(Warning::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info);

	var _call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var*>& args);
	void _resolve_compiletime_funcs(const ptr<Parser::CallNode>& p_func);

	void _check_identifier(ptr<Parser::Node>& p_expr);
	void _check_member_var_shadow(void* p_base, Parser::ClassNode::BaseType p_base_type, stdvec<ptr<Parser::VarNode>>& p_vars);
	void _check_super_constructor_call(const Parser::BlockNode* p_block);
	void _check_arg_count(int p_argc, int p_default_argc, int p_args_given, Vect2i p_err_pos = Vect2i(0, 0));

	void _resolve_inheritance(Parser::ClassNode* p_class);
	void _resolve_constant(Parser::ConstNode* p_const);
	void _resolve_parameters(Parser::FunctionNode* p_func);
	void _resolve_enumvalue(Parser::EnumValueNode& p_enumvalue, int* p_possible = nullptr);

	void _reduce_expression(ptr<Parser::Node>& p_expr);
	void _reduce_block(ptr<Parser::BlockNode>& p_block);
	void _reduce_identifier(ptr<Parser::Node>& p_expr);
	void _reduce_call(ptr<Parser::Node>& p_expr);
	void _reduce_indexing(ptr<Parser::Node>& p_expr);


	Parser::IdentifierNode _find_member(const Parser::MemberContainer* p_member, const String& p_name);
};

}

#endif // ANALYZER_H