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

#include "compiler/codegen.h"

/******************************************************************************************************************/
/*                                         CODEGEN                                                                */
/******************************************************************************************************************/

namespace carbon {

void CGContext::insert_dbg(const Parser::Node* p_node) {
	//if (p_node == nullptr) return;
	opcodes->insert_dbg((uint32_t)p_node->pos.x);
}

void CGContext::clear() {
	curr_class = nullptr;
	function = nullptr;
	while (!stack_locals_frames.empty()) stack_locals_frames.pop();
	stack_locals.clear();
	parameters.clear();
	curr_stack_temps = 0;
	stack_max_size = 0;
	opcodes = newptr<Opcodes>();
}

void CGContext::push_stack_locals() {
	stack_locals_frames.push(stack_locals);
}

void CGContext::pop_stack_locals() {
	stack_locals = stack_locals_frames.top();
	stack_locals_frames.pop();
}

void CGContext::pop_stack_temp() {
	curr_stack_temps--;
}

Address CGContext::add_stack_local(const String& p_name) {
	ASSERT(stack_locals.find(p_name) == stack_locals.end());

	uint32_t stack_size = (uint32_t)stack_locals.size() + curr_stack_temps;
	stack_locals[p_name] = stack_size;
	stack_max_size = std::max(stack_max_size, stack_size + 1);
	return Address(Address::STACK, stack_size);
}

Address CGContext::get_stack_local(const String& p_name) {
	ASSERT(stack_locals.find(p_name) != stack_locals.end());
	return Address(Address::STACK, stack_locals[p_name]);
}

Address CGContext::get_parameter(const String& p_name) {
	for (int i = 0; i < (int)parameters.size(); i++) {
		if (parameters[i] == p_name) {
			return Address(Address::PARAMETER, i);
		}
	}
	THROW_BUG("parameter not found.");
}

Address CGContext::add_stack_temp() {
	uint32_t temp_pos = (uint32_t)stack_locals.size() + (curr_stack_temps++);
	stack_max_size = std::max(stack_max_size, temp_pos + 1);
	return Address(Address::STACK, temp_pos, true);
}
//--------------------------------------------------------------------

void CodeGen::_pop_addr_if_temp(const Address& m_addr) {
	if (m_addr.is_temp()) _context.pop_stack_temp();
}

Address CodeGen::add_global_const_value(const var& p_value) {
	uint32_t pos = _bytecode->_global_const_value_get(p_value);
	return Address(Address::CONST_VALUE, pos);
}

uint32_t CodeGen::add_global_name(const String& p_name) {
	return _bytecode->_global_name_get(p_name);
}

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
		ptr<Function> cfn = _generate_function(fn.get(), class_node, p_bytecode);
		p_bytecode->_functions[cfn->_name] = cfn;

		if (fn->name == GlobalStrings::main) p_bytecode->_main = cfn.get();
		if (class_node && class_node->constructor == fn.get()) p_bytecode->_constructor = cfn.get();
	}
}


ptr<Function> CodeGen::_generate_initializer(bool p_static, Bytecode* p_bytecode, Parser::MemberContainer* p_container) {

	ptr<Function> cfn = newptr<Function>();

	const Parser::ClassNode* class_node = nullptr;
	if (p_container->type == Parser::Node::Type::CLASS) class_node = static_cast<const Parser::ClassNode*>(p_container);

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = class_node;
	_context.opcodes->op_dbg = &cfn->op_dbg;

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
			if (member != value) {
				_context.insert_dbg(var_node.get());
				_context.opcodes->write_assign(member, value);
			}
			_pop_addr_if_temp(value);
		}
	}

	_context.opcodes->insert(Opcode::END);

	cfn->_name = (p_static) ? "@static_initializer" : "@member_initializer";
	cfn->_is_static = p_static;
	cfn->_owner = _context.bytecode;
	cfn->_opcodes = _context.opcodes->opcodes;
	cfn->_stack_size = _context.stack_max_size;

	return cfn;
}

ptr<Function> CodeGen::_generate_function(const Parser::FunctionNode* p_func, const Parser::ClassNode* p_class, Bytecode* p_bytecode) {
	ptr<Function> cfn = newptr<Function>();

	_context.clear();
	_context.function = cfn.get();
	_context.bytecode = p_bytecode;
	_context.curr_class = p_class;
	_context.opcodes->op_dbg = &cfn->op_dbg;
	for (int i = 0; i < (int)p_func->args.size(); i++) {
		cfn->_is_reference.push_back(p_func->args[i].is_reference);
		_context.parameters.push_back(p_func->args[i].name);
	}

	_generate_block(p_func->body.get());

	// Opcode::END dbg position
	_context.opcodes->insert_dbg(p_func->end_line);
	_context.opcodes->insert(Opcode::END);

	cfn->_name = p_func->name;
	cfn->_is_static = p_func->is_static;
	cfn->_arg_count = (int)p_func->args.size();
	cfn->_default_args = p_func->default_args;
	cfn->_owner = _context.bytecode;
	cfn->_default_args = p_func->default_args;
	cfn->_opcodes = _context.opcodes->opcodes;
	cfn->_stack_size = _context.stack_max_size;

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
						_context.insert_dbg(var_node);
						_context.opcodes->write_assign(local_var, assign_value);
					}
					_pop_addr_if_temp(assign_value);
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
				_pop_addr_if_temp(expr);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				_generate_control_flow(static_cast<const Parser::ControlFlowNode*>(statement));
			} break;
		}

	}

	_context.pop_stack_locals();

}

}

/******************************************************************************************************************/
/*                                         GEN CONTROLFLOAT                                                       */
/******************************************************************************************************************/

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

/******************************************************************************************************************/
/*                                         GEN EXPRESSION                                                         */
/******************************************************************************************************************/

namespace carbon {

Address CodeGen::_generate_expression(const Parser::Node* p_expr, Address* p_dst) {

#define ADDR_DST() (p_dst) ? * p_dst : _context.add_stack_temp()

	switch (p_expr->type) {
		case Parser::Node::Type::IMPORT:
		case Parser::Node::Type::FILE:
		case Parser::Node::Type::CLASS:
		case Parser::Node::Type::ENUM:
		case Parser::Node::Type::FUNCTION:
		case Parser::Node::Type::BLOCK:
			THROW_BUG("invalid expression type found in codegen");

		case Parser::Node::Type::IDENTIFIER: {
			const Parser::IdentifierNode* id = static_cast<const Parser::IdentifierNode*>(p_expr);
			switch (id->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN:
					THROW_BUG("unknown identifier should be analyzer error (can't reach to codegen)");

				case Parser::IdentifierNode::REF_PARAMETER: {
					return _context.get_parameter(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_VAR: {
					return _context.get_stack_local(id->name);
				} break;

				case Parser::IdentifierNode::REF_LOCAL_CONST:
					THROW_BUG("identifier to local const should be reduced at analyzing phace");

				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					return Address(Address::MEMBER_VAR, _context.bytecode->get_member_index(id->name));
				} break;

				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_CONST:
				case Parser::IdentifierNode::REF_ENUM_NAME:
				case Parser::IdentifierNode::REF_ENUM_VALUE:
				case Parser::IdentifierNode::REF_FUNCTION:
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					return Address(Address::STATIC_MEMBER, _bytecode->_global_name_get(id->name));
				} break;

				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					return Address(Address::NATIVE_CLASS, _bytecode->_global_name_get(id->name));
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					return Address(Address::EXTERN, _bytecode->_global_name_get(id->name));
				} break;
			}
		} break;

		case Parser::Node::Type::VAR:
		case Parser::Node::Type::CONST: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;

		case Parser::Node::Type::CONST_VALUE: {
			return add_global_const_value(static_cast<const Parser::ConstValueNode*>(p_expr)->value);
		} break;

		case Parser::Node::Type::ARRAY: {
			const Parser::ArrayNode* arr = static_cast<const Parser::ArrayNode*>(p_expr);

			Address arr_dst = ADDR_DST();

			stdvec<Address> values;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				Address val = _generate_expression(arr->elements[i].get());
				values.push_back(val);
			}
			_context.insert_dbg(p_expr);
			_context.opcodes->write_array_literal(arr_dst, values);

			for (Address& addr : values) {
				_pop_addr_if_temp(addr);
			}

			return arr_dst;
		} break;

		case Parser::Node::Type::MAP: {
			const Parser::MapNode* map = static_cast<const Parser::MapNode*>(p_expr);

			Address map_dst = ADDR_DST();

			stdvec<Address> keys, values;
			for (auto& pair : map->elements) {
				Address key = _generate_expression(pair.key.get());
				Address value = _generate_expression(pair.value.get());

				keys.push_back(key);
				values.push_back(value);
			}
			_context.insert_dbg(p_expr);
			_context.opcodes->write_map_literal(map_dst, keys, values);

			for (Address& addr : keys) _pop_addr_if_temp(addr);
			for (Address& addr : values) _pop_addr_if_temp(addr);

			return map_dst;
		} break;

		case Parser::Node::Type::THIS: {
			return Address(Address::THIS, 0);
		} break;
		case Parser::Node::Type::SUPER: {
			THROW_BUG("TODO:");
			// TODO: super cannot be a stand alone expression (throw error)
		} break;

		case Parser::Node::Type::BUILTIN_FUNCTION: {
			return Address(Address::BUILTIN_FUNC, (uint32_t)static_cast<const Parser::BuiltinFunctionNode*>(p_expr)->func);
		} break;

		case Parser::Node::Type::BUILTIN_TYPE: {
			return Address(Address::BUILTIN_TYPE, (uint32_t)static_cast<const Parser::BuiltinTypeNode*>(p_expr)->builtin_type);
		} break;

		case Parser::Node::Type::CALL: {
			const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_expr);
			Address ret = ADDR_DST();

			stdvec<Address> args;
			for (int i = 0; i < (int)call->args.size(); i++) {
				Address arg = _generate_expression(call->args[i].get());
				args.push_back(arg);
			}

			switch (call->base->type) {

				// print(); builtin func call
				case Parser::Node::Type::BUILTIN_FUNCTION: {
					if (call->method == nullptr) { // print(...);
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call_builtin(ret, static_cast<const Parser::BuiltinFunctionNode*>(call->base.get())->func, args);
					} else { // print.member(...);
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name,  args);
						_pop_addr_if_temp(base);
					}
				} break;

				// Array(); builtin type constructor
				case Parser::Node::Type::BUILTIN_TYPE: {
					if (call->method == nullptr) { // Array(); constructor
						_context.insert_dbg(p_expr);
						_context.opcodes->write_construct_builtin_type(ret, static_cast<const Parser::BuiltinTypeNode*>(call->base.get())->builtin_type, args);
					} else { // String.format(); // static method call on builtin type
						Address base = _generate_expression(call->base.get());
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name, args);
						_pop_addr_if_temp(base);
					}
				} break;

				case Parser::Node::Type::SUPER: {

					if (call->method == nullptr) { // super(...); if used in constructor -> super constructor else call same func on super.
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call_super_constructor(args);
					} else { // super.f();
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						const Parser::IdentifierNode* method = ptrcast<Parser::IdentifierNode>(call->method).get();
						uint32_t name = add_global_name(method->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_super_method(ret, name, args);
					}

				} break;

				case Parser::Node::Type::UNKNOWN: {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					const Parser::IdentifierNode* func = ptrcast<Parser::IdentifierNode>(call->method).get();
					uint32_t name = add_global_name(func->name);
					switch (func->ref) {
						case  Parser::IdentifierNode::REF_FUNCTION: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_call_func(ret, name, args);
						} break;
						case  Parser::IdentifierNode::REF_CARBON_CLASS: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_construct_carbon(ret, name, args);
						} break;
						case Parser::IdentifierNode::REF_NATIVE_CLASS: {
							_context.insert_dbg(p_expr);
							_context.opcodes->write_construct_native(ret, name, args);
						} break;
						default: {
							THROW_BUG("can't reach here"); // TODO: refactor
						}
					}
				} break;

				default: {
					Address base = _generate_expression(call->base.get());
					if (call->method != nullptr) {
						ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
						uint32_t name = add_global_name(static_cast<const Parser::IdentifierNode*>(call->method.get())->name);
						_context.insert_dbg(call->method.get());
						_context.opcodes->write_call_method(ret, base, name, args);
					} else {
						_context.insert_dbg(p_expr);
						_context.opcodes->write_call(ret, base, args);
					}
					_pop_addr_if_temp(base);
				} break;
			}
			for (Address& addr : args) _pop_addr_if_temp(addr);
			return ret;
		} break;

		case Parser::Node::Type::INDEX: {
			Address dst = ADDR_DST();
			const Parser::IndexNode* index_node = static_cast<const Parser::IndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			uint32_t name = add_global_name(index_node->member->name);
			_context.insert_dbg(index_node->member.get());
			_context.opcodes->write_get_index(on, name, dst);
			_pop_addr_if_temp(on);
			return dst;
		} break;

		case Parser::Node::Type::MAPPED_INDEX: {
			Address dst = ADDR_DST();
			const Parser::MappedIndexNode* index_node = static_cast<const Parser::MappedIndexNode*>(p_expr);
			Address on = _generate_expression(index_node->base.get());
			Address key = _generate_expression(index_node->key.get());

			_context.insert_dbg(index_node->key.get());
			_context.opcodes->write_get_mapped(on, key, dst);

			_pop_addr_if_temp(on);
			_pop_addr_if_temp(key);
			return dst;
		} break;
		case Parser::Node::Type::OPERATOR: {
			const Parser::OperatorNode* op = static_cast<const Parser::OperatorNode*>(p_expr);

			var::Operator var_op = var::_OP_MAX_;
			switch (op->op_type) {
				case Parser::OperatorNode::OP_EQ:                                             goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_PLUSEQ:        var_op = var::OP_ADDITION;       goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MINUSEQ:       var_op = var::OP_SUBTRACTION;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MULEQ:         var_op = var::OP_MULTIPLICATION; goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_DIVEQ:         var_op = var::OP_DIVISION;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_MOD_EQ:        var_op = var::OP_MODULO;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_LSHIFT_EQ: var_op = var::OP_BIT_LSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_RSHIFT_EQ: var_op = var::OP_BIT_RSHIFT;	  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_OR_EQ:     var_op = var::OP_BIT_OR;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_AND_EQ:    var_op = var::OP_BIT_AND;		  goto _addr_operator_assign_;
				case Parser::OperatorNode::OP_BIT_XOR_EQ: {  var_op = var::OP_BIT_XOR;
				_addr_operator_assign_:
					// indexing, mapped indexing is special case.
					if (op->args[0]->type == Parser::Node::Type::INDEX) {
						const Parser::IndexNode* index = static_cast<Parser::IndexNode*>(op->args[0].get());
						Address on = _generate_expression(index->base.get());
						uint32_t name = add_global_name(ptrcast<Parser::IdentifierNode>(index->member)->name);
						Address value = _generate_expression(op->args[1].get());

						_context.insert_dbg(index->member.get());
						_context.opcodes->write_set_index(on, name, value);

						_pop_addr_if_temp(on);
						return value;

					} else if (op->args[0]->type == Parser::Node::Type::MAPPED_INDEX) {
						const Parser::MappedIndexNode* mapped = static_cast<const Parser::MappedIndexNode*>(op->args[0].get());
						Address on = _generate_expression(mapped->base.get());
						Address key = _generate_expression(mapped->key.get());
						Address value = _generate_expression(op->args[1].get());

						_context.insert_dbg(mapped->key.get());
						_context.opcodes->write_set_mapped(on, key, value);

						_pop_addr_if_temp(on);
						_pop_addr_if_temp(key);
						return value;

					} else {
						Address left = _generate_expression(op->args[0].get());
						if (left.is_temp()) THROW_ERROR(Error::SYNTAX_ERROR, "invalid assignment to an expression"); // f() = 12; TODO: throw with dbg info.
						if (var_op != var::_OP_MAX_) {
							Address right = _generate_expression(op->args[1].get());
							_context.insert_dbg(p_expr);
							_context.opcodes->write_operator(left, var_op, left, right);
							_pop_addr_if_temp(right);
						} else {
							Address right = _generate_expression(op->args[1].get(), &left);
							_context.insert_dbg(p_expr);
							if (left != right) _context.opcodes->write_assign(left, right);
							_pop_addr_if_temp(right);
						}
						return left;
					}
				} break;

				case Parser::OperatorNode::OP_AND: {
					Address dst = ADDR_DST();
					_context.insert_dbg(p_expr);
					_context.opcodes->write_assign_bool(dst, false);
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_and_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_and_right(right, dst);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_OR: {
					Address dst = ADDR_DST();
					_context.insert_dbg(p_expr);
					_context.opcodes->write_assign_bool(dst, true);
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_or_left(left);
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_or_right(right, dst);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_EQEQ:       var_op = var::OP_EQ_CHECK;       goto _addr_operator_;
				case Parser::OperatorNode::OP_NOTEQ:      var_op = var::OP_NOT_EQ_CHECK;   goto _addr_operator_;
				case Parser::OperatorNode::OP_LTEQ:       var_op = var::OP_LTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_GTEQ:       var_op = var::OP_GTEQ;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_PLUS:       var_op = var::OP_ADDITION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MINUS:      var_op = var::OP_SUBTRACTION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MUL:        var_op = var::OP_MULTIPLICATION; goto _addr_operator_;
				case Parser::OperatorNode::OP_DIV:        var_op = var::OP_DIVISION;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_MOD:        var_op = var::OP_MODULO;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_LT:         var_op = var::OP_LT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_GT:         var_op = var::OP_GT;			   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_LSHIFT: var_op = var::OP_BIT_LSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_RSHIFT: var_op = var::OP_BIT_RSHIFT;	   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_OR:     var_op = var::OP_BIT_OR;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_AND:    var_op = var::OP_BIT_AND;		   goto _addr_operator_;
				case Parser::OperatorNode::OP_BIT_XOR: {  var_op = var::OP_BIT_XOR;
					_addr_operator_:
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					Address right = _generate_expression(op->args[1].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var_op, left, right);
					_pop_addr_if_temp(left);
					_pop_addr_if_temp(right);
					return dst;
				} break;

				case Parser::OperatorNode::OP_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_NOT, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_BIT_NOT: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_BIT_NOT, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;

				case Parser::OperatorNode::OP_POSITIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_POSITIVE, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;
				case Parser::OperatorNode::OP_NEGATIVE: {
					Address dst = ADDR_DST();
					Address left = _generate_expression(op->args[0].get());
					_context.insert_dbg(p_expr);
					_context.opcodes->write_operator(dst, var::OP_NEGATIVE, left, Address());
					_pop_addr_if_temp(left);
					return dst;
				} break;
			}
		} break;

		case Parser::Node::Type::CONTROL_FLOW: {
			THROW_BUG("invalid expression node found in codegen.");
		} break;
	}


	return Address();
}

}
