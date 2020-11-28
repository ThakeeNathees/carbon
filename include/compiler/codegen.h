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

#ifndef CODEGEN_H
#define CODEGEN_H

#include "core/core.h"
#include "analyzer.h"
#include "bytecode.h"
#include "carbon_function.h"

#define _POP_ADDR_IF_TEMP(m_addr)if (m_addr.is_temp()) _context.pop_stack_temp();

namespace carbon {

struct CGContext {
	// members
	Bytecode* bytecode = nullptr;
	const Parser::ClassNode* curr_class = nullptr;
	CarbonFunction* function = nullptr;

	std::stack<stdmap<String, uint32_t>> stack_locals_frames;
	stdmap<String, uint32_t> stack_locals;
	stdvec<String> parameters;
	uint32_t curr_stack_temps = 0;
	uint32_t stack_max_size = 0;

	ptr<Opcodes> opcodes;

	void clear();
	void push_stack_locals();
	void pop_stack_locals();
	void pop_stack_temp();

	Address add_stack_local(const String& p_name);
	Address get_stack_local(const String& p_name);
	Address get_parameter(const String& p_name);
	Address add_stack_temp();

};

class CodeGen {

private: // members
	CGContext _context;
	Bytecode* _bytecode = nullptr; // the file node version.
	const Parser::FileNode* _file_node = nullptr;

public:
	ptr<Bytecode> generate(ptr<Analyzer> p_analyzer);

private:
	void _generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode);

	ptr<CarbonFunction> _generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode);
	ptr<CarbonFunction> _generate_initializer(bool p_static, Bytecode* p_bytecode, Parser::MemberContainer* p_container);
	void _generate_block(const Parser::BlockNode* p_block);
	void _generate_control_flow(const Parser::ControlFlowNode* p_cflow);
	Address _generate_expression(const Parser::Node* p_expr, Address* p_dst = nullptr);

	Address add_global_const_value(const var& p_value);
	uint32_t add_global_name(const String& p_name);
};

}

#endif // CODEGEN_H
