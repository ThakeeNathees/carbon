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

#include "analyzer/analyzer.h"
#include "binary/bytecode.h"
#include "binary/carbon_function.h"

namespace carbon {

struct CGContext {
	Bytecode* bytecode = nullptr;
	const Parser::ClassNode* curr_class = nullptr;
	CarbonFunction* function = nullptr;

	std::stack<stdmap<String, uint32_t>> stack_locals_frames;
	stdmap<String, uint32_t> stack_locals;
	uint32_t curr_stack_temps = 0;
	//curr_stack_size = curr_stack_temps + stack_frame.size();
	uint32_t stack_max_size = 0;

	ptr<Opcodes> opcodes;

	void clear() {
		curr_class = nullptr;
		function = nullptr;
		while (!stack_locals_frames.empty()) stack_locals_frames.pop();
		stack_locals.clear();
		curr_stack_temps = 0;
		stack_max_size = 0;
		opcodes = newptr<Opcodes>();
	}

	void push_stack_locals() {
		stack_locals_frames.push(stack_locals);
	}

	void pop_stack_locals() {
		stack_locals = stack_locals_frames.top();
		stack_locals_frames.pop();
	}

	void pop_stack_temp() {
		curr_stack_temps--;
	}

	Address add_stack_local(const String& p_name) {
		ASSERT(stack_locals.find(p_name) == stack_locals.end());

		uint32_t stack_size = (uint32_t)stack_locals.size();
		stack_locals[p_name] = stack_size;
		stack_max_size = std::max(stack_max_size, (stack_size + curr_stack_temps) + 1);
		return Address(Address::STACK, stack_size);
	}

	Address get_stack_local(const String& p_name) {
		ASSERT(stack_locals.find(p_name) != stack_locals.end());
		return Address(Address::STACK, stack_locals[p_name]);
	}

	Address add_stack_temp() {
		uint32_t temp_pos = (uint32_t)stack_locals.size() + (curr_stack_temps++);
		stack_max_size = std::max(stack_max_size, temp_pos + 1);
		return Address(Address::STACK, temp_pos, true);
	}

};

class CodeGen {
public:
	ptr<Bytecode> generate(ptr<Analyzer> p_analyzer);

private:
	CGContext _context;
	Bytecode* _bytecode = nullptr; // the file node version.
	const Parser::FileNode* _file_node = nullptr;

	void _generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode);

	ptr<CarbonFunction> _generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode);
	void _generate_block(const Parser::BlockNode* p_block);
	void _generate_control_flow(const Parser::ControlFlowNode* p_cflow);
	Address _generate_expression(const Parser::Node* p_expr);

	Address add_global_const_value(const var& p_value) {
		uint32_t pos = _bytecode->_global_const_value_get(p_value);
		return Address(Address::CONST_VALUE, pos);
	}

	uint32_t add_global_name(const String& p_name) {
		return _bytecode->_global_name_get(p_name);
	}
};

}

#endif // CODEGEN_H
