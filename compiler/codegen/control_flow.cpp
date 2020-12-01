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

#include "codegen.h"

namespace carbon {

void CodeGen::_generate_control_flow(const Parser::ControlFlowNode* p_cflow) {
	switch (p_cflow->cf_type) {
		case Parser::ControlFlowNode::CfType::IF: {
			ASSERT(p_cflow->args.size() == 1);
			Address cond = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_if(cond);
			if (cond.is_temp()) _context.pop_stack_temp();

			_generate_block(p_cflow->body.get());

			if (p_cflow->body_else != nullptr) {
				_context.insert_dbg(p_cflow->body_else.get());
				_context.opcodes->write_else();
				_generate_block(p_cflow->body_else.get());
			}

			_context.opcodes->write_endif();
		} break;

		case Parser::ControlFlowNode::CfType::SWITCH: {
			// TODO:
		} break;

		case Parser::ControlFlowNode::CfType::WHILE: {
			ASSERT(p_cflow->args.size() == 1);
			_context.opcodes->jump_to_continue.push(_context.opcodes->next());
			Address cond = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_while(cond);
			if (cond.is_temp()) _context.pop_stack_temp();
			_generate_block(p_cflow->body.get());
			_context.opcodes->write_endwhile();
		} break;

		case Parser::ControlFlowNode::CfType::FOR: {
			ASSERT(p_cflow->args.size() == 3);
			_context.push_stack_locals();

			// iterator
			if (p_cflow->args[0] != nullptr) {
				const Parser::VarNode* var_node = static_cast<const Parser::VarNode*>(p_cflow->args[0].get());
				Address iterator = _context.add_stack_local(var_node->name);
				if (var_node->assignment != nullptr) {
					Address assign_value = _generate_expression(var_node->assignment.get());
					_context.insert_dbg(var_node);
					_context.opcodes->write_assign(iterator, assign_value);
					if (assign_value.is_temp()) _context.pop_stack_temp();
				}
			}

			// condition.
			Address cond;
			_context.opcodes->jump_to_continue.push(_context.opcodes->next());
			if (p_cflow->args[1] != nullptr) {
				const Parser::Node* cond_node = p_cflow->args[1].get();
				cond = _generate_expression(cond_node);
			}
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_for(cond);
			if (cond.is_temp()) _context.pop_stack_temp();

			// body
			_generate_block(p_cflow->body.get());

			// end
			Address end_statement;
			if (p_cflow->args[2] != nullptr) {
				const Parser::Node* end_statement_node = p_cflow->args[2].get();
				end_statement = _generate_expression(end_statement_node);
				if (end_statement.is_temp()) _context.pop_stack_temp();
			}
			_context.opcodes->write_endfor(cond.get_type() != Address::_NULL);

			_context.pop_stack_locals();
		} break;

		case Parser::ControlFlowNode::CfType::FOREACH: {
			ASSERT(p_cflow->args.size() == 2);
			_context.push_stack_locals();

			const String& iterator_name = static_cast<const Parser::VarNode*>(p_cflow->args[0].get())->name;
			Address iter_value = _context.add_stack_local(iterator_name);
			Address iterator = _context.add_stack_temp();
			Address on = _generate_expression(p_cflow->args[1].get());

			_context.insert_dbg(p_cflow);
			_context.opcodes->write_foreach(iter_value, iterator, on);
			_generate_block(p_cflow->body.get());
			_pop_addr_if_temp(iterator);
			_context.opcodes->write_endforeach();
			if (on.is_temp()) _context.pop_stack_temp();

			_context.pop_stack_locals();
		} break;

		case Parser::ControlFlowNode::CfType::BREAK: {
			ASSERT(p_cflow->args.size() == 0);
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_break();
		} break;
		case Parser::ControlFlowNode::CfType::CONTINUE: {
			ASSERT(p_cflow->args.size() == 0);
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_continue();
		} break;
		case Parser::ControlFlowNode::CfType::RETURN: {
			ASSERT(p_cflow->args.size() <= 1);
			Address ret;
			if (p_cflow->args.size() == 1) ret = _generate_expression(p_cflow->args[0].get());
			_context.insert_dbg(p_cflow);
			_context.opcodes->write_return(ret);
			if (ret.is_temp()) _context.pop_stack_temp();
			
		} break;
	}
	MISSED_ENUM_CHECK(Parser::ControlFlowNode::CfType::_CF_MAX_, 8);
}

}
