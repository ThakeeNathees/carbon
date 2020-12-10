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

#include "analyzer.h"

namespace carbon {

Parser::IdentifierNode Analyzer::_find_member(const Parser::MemberContainer* p_container, const String& p_name) {

	Parser::IdentifierNode id; id.name = p_name;
	if (!p_container) return id;

	id.ref_base = Parser::IdentifierNode::BASE_LOCAL;

	for (int i = 0; i < (int)p_container->vars.size(); i++) {
		if (p_container->vars[i]->name == id.name) {
			if (p_container->vars[i]->is_static) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
			else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
			id._var = p_container->vars[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_container->functions.size(); i++) {
		// constructors are REF_CARBON_CLASS 
		if (!p_container->functions[i]->is_constructor && p_container->functions[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_FUNCTION;
			id._func = p_container->functions[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_container->constants.size(); i++) {
		if (p_container->constants[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
			_resolve_constant(p_container->constants[i].get());
			id._const = p_container->constants[i].get();
			return id;
		}
	}
	if (p_container->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : p_container->unnamed_enum->values) {
			if (pair.first == id.name) {
				id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
				_resolve_enumvalue(p_container->unnamed_enum->values[pair.first]);
				id._enum_value = &p_container->unnamed_enum->values[pair.first];
				return id;
			}
		}
	}
	for (int i = 0; i < (int)p_container->enums.size(); i++) {
		if (p_container->enums[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
			id._enum_node = p_container->enums[i].get();
			return id;
		}
	}

	ASSERT(id.ref == Parser::IdentifierNode::REF_UNKNOWN);
	if (p_container->type == Parser::Node::Type::CLASS) {
		const Parser::ClassNode* _class = static_cast<const Parser::ClassNode*>(p_container);

		switch (_class->base_type) {
			case Parser::ClassNode::BASE_LOCAL:
				return _find_member(_class->base_class, p_name);
			case Parser::ClassNode::BASE_NATIVE: {
				ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(_class->base_class_name, p_name);
				if (bd) {
					id.ref_base = Parser::IdentifierNode::BASE_NATIVE;
					switch (bd->get_member_info()->get_type()) {
						case MemberInfo::Type::METHOD: {
							id.ref = Parser::IdentifierNode::REF_FUNCTION;
							id._method_info = ptrcast<MethodBind>(bd)->get_method_info().get();
							return id;
						} break;
						case MemberInfo::Type::PROPERTY: {
							id._prop_info = ptrcast<PropertyBind>(bd)->get_prop_info().get();
							if (id._prop_info->is_const()) id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
							else if (id._prop_info->is_static()) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
							else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
							return id;
						} break;
						case MemberInfo::Type::ENUM: {
							id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
							id._enum_info = ptrcast<EnumBind>(bd)->get_enum_info().get();
							return id;
						} break;
						case MemberInfo::Type::ENUM_VALUE: {
							id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							id._enum_value_info = ptrcast<EnumValueBind>(bd)->get_enum_value_info().get();
							return id;
						} break;
					}
				}
			} break;
			case Parser::ClassNode::BASE_EXTERN:
				id.ref_base = Parser::IdentifierNode::BASE_EXTERN;
				MemberInfo* mi = _class->base_binary->get_member_info(p_name).get();
				if (mi == nullptr) break;
				switch (mi->get_type()) {
					case MemberInfo::Type::CLASS: {
						id.ref = Parser::IdentifierNode::REF_EXTERN; // extern class
						id._class_info = static_cast<ClassInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::METHOD: {
						id.ref = Parser::IdentifierNode::REF_FUNCTION;
						id._method_info = static_cast<MethodInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::PROPERTY: {
						id._prop_info = static_cast<PropertyInfo*>(mi);
						if (id._prop_info->is_const()) id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
						else if (id._prop_info->is_static()) id.ref = Parser::IdentifierNode::REF_STATIC_VAR;
						else id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
						return id;
					} break;
					case MemberInfo::Type::ENUM: {
						id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
						id._enum_info = static_cast<EnumInfo*>(mi);
						return id;
					} break;
					case MemberInfo::Type::ENUM_VALUE: {
						id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
						id._enum_value_info = static_cast<EnumValueInfo*>(mi);
						return id;
					} break;
				}
		}

	} else { // container is FileNode
		const Parser::FileNode* file_node = static_cast<const Parser::FileNode*>(p_container);

		for (int i = 0; i < (int)file_node->classes.size(); i++) {
			if (file_node->classes[i]->name == id.name) {
				id.ref = Parser::IdentifierNode::REF_CARBON_CLASS;
				id._class = file_node->classes[i].get();
				return id;
			}
		}

		if (NativeClasses::singleton()->is_class_registered(id.name)) {
			id.ref = Parser::IdentifierNode::REF_NATIVE_CLASS;
			id.ref_base = Parser::IdentifierNode::BASE_NATIVE;
			return id;
		}
	}

	id.ref_base = Parser::IdentifierNode::BASE_UNKNOWN;
	return id;
}

void Analyzer::_reduce_identifier(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::IDENTIFIER);

	// search parameters.
	ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(p_expr);
	if (parser->parser_context.current_func) {
		for (int i = 0; i < (int)parser->parser_context.current_func->args.size(); i++) {
			if (parser->parser_context.current_func->args[i].name == id->name) {
				id->ref = Parser::IdentifierNode::REF_PARAMETER;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				id->param_index = i;
				return;
			}
		}
	}

	// search in locals (var, const)
	Parser::BlockNode* outer_block = parser->parser_context.current_block;
	while (outer_block != nullptr && id->ref == Parser::IdentifierNode::REF_UNKNOWN) {
		for (int i = 0; i < (int)outer_block->local_vars.size(); i++) {
			Parser::VarNode* local_var = outer_block->local_vars[i].get();
			if (local_var->name == id->name) {

				if (p_expr->pos.x < local_var->pos.x || (p_expr->pos.x == local_var->pos.x && p_expr->pos.y < local_var->pos.y))
					throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("local variable \"%s\" referenced before assigned", local_var->name.c_str()), id->pos);
				id->ref = Parser::IdentifierNode::REF_LOCAL_VAR;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				id->_var = outer_block->local_vars[i].get();
				return;
			}
		}

		for (int i = 0; i < (int)outer_block->local_const.size(); i++) {
			if (outer_block->local_const[i]->name == id->name) {
				id->ref = Parser::IdentifierNode::REF_LOCAL_CONST;
				id->ref_base = Parser::IdentifierNode::BASE_LOCAL;
				_resolve_constant(outer_block->local_const[i].get());
				id->_const = outer_block->local_const[i].get();
				return;
			}
		}

		if (outer_block->parernt_node->type == Parser::Node::Type::BLOCK) {
			outer_block = ptrcast<Parser::BlockNode>(outer_block->parernt_node).get();
		} else {
			outer_block = nullptr;
		}
	}

	// if analyzing enum search in enums
	if (parser->parser_context.current_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : parser->parser_context.current_enum->values) {
			if (pair.first == id->name) {
				id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
				_resolve_enumvalue(parser->parser_context.current_enum->values[pair.first]);
				id->_enum_value = &pair.second;
				return;
			}
		}
	}

	// search in current class.
	Parser::IdentifierNode _id = _find_member(parser->parser_context.current_class, id->name);
	if (_id.ref != Parser::IdentifierNode::REF_UNKNOWN) {
		_id.pos = id->pos; p_expr = newptr<Parser::IdentifierNode>(_id);
		return;
	}

	// search in current file.
	_id = _find_member(parser->file_node.get(), id->name);
	if (_id.ref != Parser::IdentifierNode::REF_UNKNOWN) {
		_id.pos = id->pos; p_expr = newptr<Parser::IdentifierNode>(_id);
		return;
	}

	// search in imports.
	for (int i = 0; i < (int)parser->file_node->imports.size(); i++) {
		if (parser->file_node->imports[i]->name == id->name) {
			id->ref = Parser::IdentifierNode::REF_EXTERN;
			id->_bytecode = parser->file_node->imports[i]->bytecode.get();
			return;
		}
	}
}


void Analyzer::_check_identifier(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::IDENTIFIER);

	ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(p_expr);
	switch (id->ref) {
		case Parser::IdentifierNode::REF_UNKNOWN:
			throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("identifier \"%s\" isn't defined.", id->name.c_str()), id->pos);
		case Parser::IdentifierNode::REF_LOCAL_CONST:
		case Parser::IdentifierNode::REF_MEMBER_CONST: {
			ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->_const->value);
			cv->pos = id->pos; p_expr = cv;
		} break;
		case Parser::IdentifierNode::REF_ENUM_VALUE: {
			ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->_enum_value->value);
			cv->pos = id->pos; p_expr = cv;
		} break;

		case Parser::IdentifierNode::REF_LOCAL_VAR:
		case Parser::IdentifierNode::REF_STATIC_VAR:
		case Parser::IdentifierNode::REF_MEMBER_VAR: {
			if (id->ref_base == Parser::IdentifierNode::BASE_LOCAL && parser->parser_context.current_var) {
				if (parser->parser_context.current_var->name == id->name) {
					throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("invalid attribute access \"%s\" can't be used in it's own initialization.", id->name.c_str()), id->pos);
				}
			}
		} // [[fallthrought]]
		default: { // variable, parameter, function name, ...
			p_expr = id;
			break;
		}
	}
}

}
