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

	stdmap<Bytecode*, String> pending_inheritance;
	for (ptr<Parser::ClassNode>& class_node : root->classes) {
		ptr<Bytecode>_class = newptr<Bytecode>();
		_class->_file = bytecode;
		_class->_is_class = true;
		_class->_name = class_node->name;

		// set inheritance.
		_class->_has_base = class_node->base_type != Parser::ClassNode::NO_BASE;
		switch (class_node->base_type) {
			case Parser::ClassNode::NO_BASE:
				break;
			case Parser::ClassNode::BASE_LOCAL:
				pending_inheritance[_class.get()] = class_node->base_class->name;
				_class->_pending_base = (void*)class_node->base_class;
				break;
			case Parser::ClassNode::BASE_NATIVE:
				_class->_is_base_native = true;
				_class->_base_native = class_node->base_class_name;
				break;
			case Parser::ClassNode::BASE_EXTERN:
				_class->_is_base_native = false;
				_class->_base = class_node->base_binary;
				break;
		}
		_generate_members(static_cast<Parser::MemberContainer*>(class_node.get()), _class.get());
		_class->_pending_base = nullptr;
		bytecode->_classes[_class->_name] = _class;
	}

	for (auto it : pending_inheritance) {
		it.first->_base = bytecode->_classes.at(it.second);
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
			p_bytecode->_static_vars[var_node->name] = default_value;
			if (var_node->assignment != nullptr) static_var_init_fn_need = true;
		} else {
			p_bytecode->_members[var_node->name] = member_index++;
			if (var_node->assignment != nullptr) member_var_init_fn_need = true;
		}
	}

	// constants
	for (ptr<Parser::ConstNode>& const_node : p_container->constants) {
		p_bytecode->_constants[const_node->name] = const_node->value;
	}

	// unnamed enums
	if (p_container->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> value : p_container->unnamed_enum->values) {
			p_bytecode->_unnamed_enums[value.first] = value.second.value;
		}
	}

	// named enums
	for (ptr<Parser::EnumNode> en : p_container->enums) {
		ptr<EnumInfo> ei = newptr<EnumInfo>(en->name);
		for (std::pair<String, Parser::EnumValueNode> value : en->values) {
			ei->get_edit_values()[value.first] = value.second.value;
		}
		p_bytecode->_enums[en->name] = ei;
	}

	if (member_var_init_fn_need) {
		ASSERT(p_bytecode->is_class());
		p_bytecode->_member_initializer = _generate_initializer(false, p_bytecode, p_container);
	}
	if (static_var_init_fn_need) {
		p_bytecode->_static_initializer = _generate_initializer(true, p_bytecode, p_container);
	}

	// functions
	for (ptr<Parser::FunctionNode> fn : p_container->functions) {
		const Parser::ClassNode* class_node = nullptr;
		if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);
		ptr<CarbonFunction> cfn = _generate_function(fn.get(), class_node, p_bytecode);
		cfn->_stack_size = _context.stack_max_size;
		p_bytecode->_functions[cfn->_name] = cfn;

		if (fn->name == "main") p_bytecode->_main = cfn.get(); // TODO: move literal string "main" to constants.
		if (class_node && class_node->constructor == fn.get()) p_bytecode->_constructor = cfn.get();
	}
}


ptr<CarbonFunction> CodeGen::_generate_initializer(bool p_static, Bytecode* p_bytecode, Parser::MemberContainer* p_container) {

	ptr<CarbonFunction> cfn = newptr<CarbonFunction>();

	const Parser::ClassNode* class_node = nullptr;
	if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = class_node;


	cfn->_name = (p_static) ? "@static_initializer" : "@member_initializer";
	cfn->_is_static = p_static;
	cfn->_owner = _context.bytecode;

	for (ptr<Parser::VarNode>& var_node : p_container->vars) {
		if (var_node->is_static == p_static && var_node->assignment != nullptr) {
			Address member;
			if (p_static) {
				Bytecode* bytecode_file = (p_bytecode->is_class()) ? p_bytecode->get_file().get() : p_bytecode;
				member = Address(Address::STATIC_MEMBER, bytecode_file->_global_name_get(var_node->name));
			} else {
				member = Address(Address::MEMBER_VAR, _context.bytecode->get_member_index(var_node->name));
			}
			Address value = _generate_expression(var_node->assignment.get(), &member);
			if (member != value) _context.opcodes->write_assign(member, value);
			_POP_ADDR_IF_TEMP(value);
		}
	}

	_context.opcodes->insert(Opcode::END);
	cfn->_opcodes = _context.opcodes->opcodes;
	return cfn;
}

ptr<CarbonFunction> CodeGen::_generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode) {
	ptr<CarbonFunction> cfn = newptr<CarbonFunction>();

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = p_class;
	for (int i = 0; i < (int)p_func->args.size(); i++) _context.parameters.push_back(p_func->args[i].name);

	cfn->_name = p_func->name;
	cfn->_is_static = p_func->is_static;
	cfn->_arg_count = (int)p_func->args.size();
	cfn->_default_args = p_func->default_args;
	cfn->_owner = _context.bytecode;
	cfn->_default_args = p_func->default_args;

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
					Address assign_value = _generate_expression(var_node->assignment.get(), &local_var);
					if (assign_value != local_var) {
						_context.opcodes->write_assign(local_var, assign_value);
					}
					_POP_ADDR_IF_TEMP(assign_value);
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
				_POP_ADDR_IF_TEMP(expr);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				_generate_control_flow(static_cast<const Parser::ControlFlowNode*>(statement));
			} break;
		}

	}

	_context.pop_stack_locals();

}

}