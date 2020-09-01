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

	var _call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var>& args);
	void _resolve_compiletime_funcs(const stdvec<ptr<Parser::OperatorNode>>& p_funcs);

	void _resolve_inheritance(Parser::ClassNode* p_class);
	void _resolve_constant(Parser::ConstNode* p_const);
	void _resolve_enumvalue(Parser::EnumValueNode& p_enumvalue);

	void _reduce_expression(ptr<Parser::Node>& p_expr);
	void _reduce_block(ptr<Parser::BlockNode>& p_block, Parser::BlockNode* p_parent_block = nullptr);

};

}

#endif // ANALYZER_H