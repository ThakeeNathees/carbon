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

#include "compiler.h"

namespace carbon {

void Compiler::_generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode) {

	// members/ static vars
	int member_index = 0;
	for (ptr<Parser::VarNode>& var_node : p_container->vars) {
		var default_value;
		if (var_node->assignment->type == Parser::Node::Type::CONST_VALUE)
			default_value = ptrcast<Parser::ConstValueNode>(var_node->assignment)->value;

		if (var_node->is_static) {
			p_bytecode->_static_vars[var_node->name] = default_value;
		} else {
			p_bytecode->_members[var_node->name] = member_index++;
		}
	}

	// constants
	for (ptr<Parser::ConstNode>& const_node : p_container->constants) {
		p_bytecode->_constants[const_node->name] = const_node->value;
	}

	// unnamed enums
	for (std::pair<String, Parser::EnumValueNode> value : p_container->unnamed_enum->values) {
		p_bytecode->_unnamed_enums[value.first] = value.second.value;
	}

	// named enums
	for (ptr<Parser::EnumNode> en : p_container->enums) {
		//ptr<_EnumBytes> eb = newptr<_EnumBytes>(en->name);
		//for (std::pair<String, Parser::EnumValueNode> value : en->values) {
		//	eb->_values[value.first] = value.second.value;
		//}
		//p_bytecode->_enums[en->name] = eb;
	}

	// functions
	// TODO:
}

ptr<Bytecode> Compiler::compile(ptr<Analyzer> p_analyzer) {
	ptr<Bytecode> bytecode = newptr<Bytecode>();
	Parser::FileNode* root = p_analyzer->parser->file_node.get();

	bytecode->_name = root->path;
	_generate_members(static_cast<Parser::MemberContainer*>(root), bytecode.get());

	for (ptr<Parser::ClassNode>& class_node : root->classes) {
		ptr<Bytecode>_class;
		_generate_members(static_cast<Parser::MemberContainer*>(class_node.get()), _class.get());
	}

	return bytecode;
}

}