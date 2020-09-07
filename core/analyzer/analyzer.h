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

#include "core.h"
#include "parser/parser.h"


#define THROW_ANALYZER_ERROR(m_err_type, m_msg, m_pos)                                                                      \
	do {																													\
		uint32_t err_len = 1;																								\
		String token_str = "";																								\
		if (m_pos.x > 0 && m_pos.y > 0) token_str = parser->tokenizer->get_token_at(m_pos).to_string();						\
		else token_str = parser->tokenizer->peek(-1, true).to_string();														\
		if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;				\
		else err_len = (uint32_t)token_str.size();																			\
																															\
		if (m_pos.x > 0 && m_pos.y > 0) {																					\
			String line = file_node->source.get_line(m_pos.x);																\
			throw Error(m_err_type, m_msg, file_node->path, line, m_pos, err_len)_ERR_ADD_DBG_VARS;							\
		} else {																											\
			String line = file_node->source.get_line(parser->tokenizer->get_pos().x);										\
			throw Error(m_err_type, m_msg, file_node->path, line, parser->tokenizer->peek(-1, true).get_pos(), err_len)		\
				_ERR_ADD_DBG_VARS;																							\
		}																													\
	} while (false)


namespace carbon {

class Analyzer {
public:
	void analyze(ptr<Parser> p_parser);

protected:

private:
	ptr<Parser> parser;
	ptr<Parser::FileNode> file_node; // Quick access.

	template<typename T = Parser::Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		return ret;
	}

	Parser::IdentifierNode _get_member(const Parser::ClassNode* p_class, const String& p_name);

	var _call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var>& args);
	void _resolve_compiletime_funcs(const stdvec<ptr<Parser::OperatorNode>>& p_funcs);

	void _resolve_inheritance(Parser::ClassNode* p_class);
	void _resolve_constant(Parser::ConstNode* p_const);
	void _resolve_parameters(Parser::FunctionNode* p_func);
	void _resolve_enumvalue(Parser::EnumValueNode& p_enumvalue);

	void _reduce_expression(ptr<Parser::Node>& p_expr);
	void _reduce_block(ptr<Parser::BlockNode> p_block);

};

}

#endif // ANALYZER_H