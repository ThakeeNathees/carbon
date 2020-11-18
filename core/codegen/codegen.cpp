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

ptr<Bytecode> CodeGen::generate(ptr<Analyzer> p_analyzer) {
	ptr<Bytecode> bytecode = newptr<Bytecode>();
	_bytecode = bytecode.get();
	Parser::FileNode* root = p_analyzer->parser->file_node.get();

	_file_node = root;
	_context.bytecode = bytecode.get();

	bytecode->_name = root->path;
	_generate_members(static_cast<Parser::MemberContainer*>(root), bytecode.get());

	for (ptr<Parser::ImportNode>& import_node : root->imports) {
		_bytecode->_externs[import_node->name] = import_node->bytecode;
	}

	for (ptr<Parser::ClassNode>& class_node : root->classes) {
		ptr<Bytecode>_class;
		_class->_file = bytecode;
		_generate_members(static_cast<Parser::MemberContainer*>(class_node.get()), _class.get());
	}
	_context.curr_class = nullptr;

	bytecode->_build_global_names_array();
	return bytecode;
}

void CodeGen::_generate_members(Parser::MemberContainer* p_container, Bytecode* p_bytecode) {

	// members/ static vars
	bool static_var_init_fn_need = false, member_var_init_fn_need = false;
	int member_index = 0;
	for (ptr<Parser::VarNode>& var_node : p_container->vars) {
		if (var_node->is_static) {
			var default_value; // default value set at runtime `static var x = f();`
			p_bytecode->_static_vars[_bytecode->_global_name_get(var_node->name)] = default_value;
			if (var_node->assignment != nullptr) static_var_init_fn_need = true;
		} else {
			p_bytecode->_members[_bytecode->_global_name_get(var_node->name)] = member_index++;
			if (var_node->assignment != nullptr) member_var_init_fn_need = true;
		}
	}

	// constants
	for (ptr<Parser::ConstNode>& const_node : p_container->constants) {
		p_bytecode->_constants[_bytecode->_global_name_get(const_node->name)] = const_node->value;
	}

	// unnamed enums
	if (p_container->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> value : p_container->unnamed_enum->values) {
			p_bytecode->_unnamed_enums[_bytecode->_global_name_get(value.first)] = value.second.value;
		}
	}

	// named enums
	for (ptr<Parser::EnumNode> en : p_container->enums) {
		ptr<EnumInfo> ei = newptr<EnumInfo>(en->name);
		for (std::pair<String, Parser::EnumValueNode> value : en->values) {
			ei->get_edit_values()[value.first] = value.second.value;
		}
		p_bytecode->_enums[_bytecode->_global_name_get(en->name)] = ei;
	}


	// TODO: static var initialization function (called when the byte code initialized) Binary::_static_var_initializer_f
	// TODO: member var initialization function (called before construction)            Binary::_member_var_initializer_f
	//if (static_var_init_fn_need) {}

	// functions
	for (ptr<Parser::FunctionNode> fn : p_container->functions) {
		const Parser::ClassNode* class_node = nullptr;
		if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);
		ptr<CarbonFunction> cfn = _generate_function(fn.get(), class_node, p_bytecode);
		cfn->_stack_size = _context.stack_max_size;
		p_bytecode->_functions[cfn->_name] = cfn;
	}
}

ptr<CarbonFunction> CodeGen::_generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode) {
	ptr<CarbonFunction> cfn = newptr<CarbonFunction>();

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = p_class;

	cfn->_name = p_func->name;
	cfn->_is_static = p_func->is_static;
	cfn->_arg_count = (int)p_func->args.size();
	cfn->_default_args = p_func->default_args;
	cfn->_owner = _context.bytecode;
	cfn->_default_args = p_func->default_args;

	// add parameters to stack locals.
	for (int i = 0; i < (int)p_func->args.size(); i++) {
		_context.add_stack_local(p_func->args[i].name);
	}

	_generate_block(p_func->body.get());
	_context.opcodes->insert(Opcode::END);

	cfn->_opcodes = _context.opcodes->opcodes;
	return cfn;
}

void CodeGen::_generate_block(const Parser::BlockNode* p_block) {

	_context.push_stack_locals();

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		const Parser::Node* statement = p_block->statements[i].get();

		switch (statement->type) {
			case Parser::Node::Type::UNKNOWN:
			case Parser::Node::Type::FILE:
			case Parser::Node::Type::CLASS:
			case Parser::Node::Type::ENUM:
			case Parser::Node::Type::FUNCTION:
			case Parser::Node::Type::BLOCK:
				THROW_BUG("invalid statement type in codegen.");

			case Parser::Node::Type::IDENTIFIER: {
				THROW_BUG("standalone expression didn't cleaned.");
			} break;

			case Parser::Node::Type::VAR: {
				const Parser::VarNode* var_node = static_cast<const Parser::VarNode*>(statement);
				Address local_var = _context.add_stack_local(var_node->name);
				if (var_node->assignment != nullptr) {
					Address assign_value = _generate_expression(var_node->assignment.get());
					_context.opcodes->write_assign(local_var, assign_value);
					if (assign_value.is_temp()) _context.pop_stack_temp();
				}
			} break;

			case Parser::Node::Type::CONST: {
				THROW_BUG("local constants should be cleaned by now");
			} break;

			case Parser::Node::Type::CONST_VALUE:
			case Parser::Node::Type::ARRAY:
			case Parser::Node::Type::MAP:
			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE: {
				THROW_BUG("stand alone expression didn't cleaned.");
			} break;

			case Parser::Node::Type::CALL:
			case Parser::Node::Type::INDEX:
			case Parser::Node::Type::MAPPED_INDEX:
			case Parser::Node::Type::OPERATOR: {
				Address expr = _generate_expression(statement);
				if (expr.is_temp()) _context.pop_stack_temp();
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				_generate_control_flow(static_cast<const Parser::ControlFlowNode*>(statement));
			} break;
		}

	}

	_context.pop_stack_locals();

}

}