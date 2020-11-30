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
#include "carbon_function.h"

namespace carbon {

CompileTimeError Analyzer::_analyzer_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source, pos, err_len), p_dbg_info);
}

Warning Analyzer::_analyzer_warning(Warning::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	return Warning(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source, pos, err_len), p_dbg_info);
}

void Analyzer::analyze(ptr<Parser> p_parser) {

	parser = p_parser;
	file_node = parser->file_node;

	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_func = nullptr;
	parser->parser_context.current_block = nullptr;
	parser->parser_context.current_enum = nullptr;

	for (int i = 0; i < (int)file_node->classes.size(); i++) {
		_resolve_inheritance(file_node->classes[i].get());
	}

	// File/class level constants.
	for (size_t i = 0; i < file_node->constants.size(); i++) {
		parser->parser_context.current_const = file_node->constants[i].get();
		_resolve_constant(file_node->constants[i].get());
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->constants.size(); j++) {
				parser->parser_context.current_const = file_node->classes[i]->constants[j].get();
				_resolve_constant(file_node->classes[i]->constants[j].get());
		}
	}
	parser->parser_context.current_const = nullptr;
	parser->parser_context.current_class = nullptr;

	// File/class enums/unnamed_enums.
	for (size_t i = 0; i < file_node->enums.size(); i++) {
		parser->parser_context.current_enum = file_node->enums[i].get();
		int _possible_value = 0;
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->enums[i]->values) {
			_resolve_enumvalue(file_node->enums[i]->values[pair.first], &_possible_value);
		}
	}
	if (file_node->unnamed_enum != nullptr) {
		parser->parser_context.current_enum = file_node->unnamed_enum.get();
		int _possible_value = 0;
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->unnamed_enum->values) {
			_resolve_enumvalue(file_node->unnamed_enum->values[pair.first], &_possible_value);
		}
	}
	parser->parser_context.current_enum = nullptr;

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->enums.size(); j++) {
			parser->parser_context.current_enum = file_node->classes[i]->enums[j].get();
			int _possible_value = 0;
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->enums[j]->values) {
				_resolve_enumvalue(file_node->classes[i]->enums[j]->values[pair.first], &_possible_value);
			}
		}
		if (file_node->classes[i]->unnamed_enum != nullptr) {
			parser->parser_context.current_enum = file_node->classes[i]->unnamed_enum.get();
			int _possible_value = 0;
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->unnamed_enum->values) {
				_resolve_enumvalue(file_node->classes[i]->unnamed_enum->values[pair.first], &_possible_value);
			}
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_enum = nullptr;

	// call compile time functions.
	_resolve_compiletime_funcs(file_node->compiletime_functions);
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		_resolve_compiletime_funcs(file_node->classes[i]->compiletime_functions);
	}
	parser->parser_context.current_class = nullptr;

	// File/class level variables.
	for (size_t i = 0; i < file_node->vars.size(); i++) {
		if (file_node->vars[i]->assignment != nullptr) {
			parser->parser_context.current_var = file_node->vars[i].get();
			_reduce_expression(file_node->vars[i]->assignment);
		}
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->vars.size(); j++) {
			if (file_node->classes[i]->vars[j]->assignment != nullptr) {
				parser->parser_context.current_var = file_node->classes[i]->vars[j].get();
				_reduce_expression(file_node->classes[i]->vars[j]->assignment);
			}
		}
	}
	parser->parser_context.current_var = nullptr;
	parser->parser_context.current_class = nullptr;

	// resolve parameters.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		Parser::FunctionNode* fn = file_node->functions[i].get();
		_resolve_parameters(fn);
	}

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			Parser::FunctionNode* fn = file_node->classes[i]->functions[j].get();
			_resolve_parameters(fn);
		}
		parser->parser_context.current_class = nullptr;
	}

	// File level function.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		parser->parser_context.current_func = file_node->functions[i].get();
		Parser::FunctionNode* fn = file_node->functions[i].get();

		if (fn->name == "main") { // TODO: string literal.
			if (fn->args.size() >= 2) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "main function takes at most 1 argument.", fn->pos);
		}

		_reduce_block(file_node->functions[i]->body);
	}
	parser->parser_context.current_func = nullptr;

	// class function.
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			parser->parser_context.current_func = file_node->classes[i]->functions[j].get();
			_reduce_block(file_node->classes[i]->functions[j]->body);
		}

		// add default constructor
		Parser::ClassNode* cls = file_node->classes[i].get();
		if (!cls->has_super_ctor_call && cls->base_type != Parser::ClassNode::NO_BASE) {

			bool can_add_default_ctor = true;
			switch (cls->base_type) {
				case Parser::ClassNode::BASE_LOCAL: {
					const Parser::FunctionNode* ctor = cls->base_class->constructor;
					if (ctor && ctor->args.size() - ctor->default_args.size() != 0) can_add_default_ctor = false;
				} break;
				case Parser::ClassNode::BASE_NATIVE: {
					const StaticFuncBind* ctor = NativeClasses::singleton()->get_constructor(cls->base_class_name);
					if (ctor && ctor->get_method_info()->get_arg_count() - ctor->get_method_info()->get_default_arg_count() - 1 != 0) can_add_default_ctor = false; // -1 for self argument
				} break;
				case Parser::ClassNode::BASE_EXTERN: {
					const CarbonFunction* ctor = cls->base_binary->get_constructor();
					if (ctor && ctor->get_arg_count() - ctor->get_default_args().size() != 0) can_add_default_ctor = false;
				} break;
			}
			if (!can_add_default_ctor)
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "super constructor call needed since base class doesn't have a default constructor.", cls->pos);

			Parser::FunctionNode* fn = cls->constructor;
			if (fn == nullptr) {
				ptr<Parser::FunctionNode> new_fn = newptr<Parser::FunctionNode>();
				new_fn->name = cls->name;
				new_fn->is_reduced = true;
				new_fn->parent_node = cls;
				new_fn->pos = cls->pos;
				new_fn->body = newptr<Parser::BlockNode>();
				cls->functions.push_back(new_fn);
				cls->constructor = new_fn.get();
				fn = new_fn.get();
			}
			ptr<Parser::CallNode> super_call = newptr<Parser::CallNode>(); super_call->pos = cls->pos;
			super_call->base = newptr<Parser::SuperNode>(); super_call->base->pos = cls->pos;
			fn->body->statements.insert(fn->body->statements.begin(), super_call);
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_func = nullptr;
}

var Analyzer::_call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var*>& args) {
	switch (p_func->func) {
		case BuiltinFunctions::__ASSERT: {
			if (args.size() != 1) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.", p_func->pos);
			if (!args[0]->operator bool()) throw ANALYZER_ERROR(Error::ASSERTION, "assertion failed.", p_func->pos);
		} break;
		case BuiltinFunctions::__FUNC: {
			if (!parser->parser_context.current_func) throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "__func() must be called inside a function.", p_func->pos);
			if (parser->parser_context.current_class) return parser->parser_context.current_class->name + "." + parser->parser_context.current_func->name;
			else  return parser->parser_context.current_func->name;
		} break;
		case BuiltinFunctions::__LINE: {
			return p_func->pos.x;
		} break;
		case BuiltinFunctions::__FILE: {
			return parser->file_node->path;
		} break;
		default:
			ASSERT(false);
	}
	return var();
}

void Analyzer::_resolve_compiletime_funcs(const stdvec<ptr<Parser::CallNode>>& p_funcs) {
	for (int i = 0; i < (int)p_funcs.size(); i++) {
		Parser::CallNode* call = p_funcs[i].get();
		ASSERT(call->base->type == Parser::Node::Type::BUILTIN_FUNCTION);
		Parser::BuiltinFunctionNode* bf = ptrcast<Parser::BuiltinFunctionNode>(call->base).get();
		stdvec<var*> args;
		for (int j = 0; j < (int)call->args.size(); j++) {
			_reduce_expression(call->args[j]);
			if (call->args[j]->type != Parser::Node::Type::CONST_VALUE) {
				throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("compiletime function arguments must be compile time known values."), p_funcs[i]->args[j]->pos);
			}
			args.push_back(&ptrcast<Parser::ConstValueNode>(call->args[j])->value);
		}
		_call_compiletime_func(bf, args);
	}
}

void Analyzer::_check_member_var_shadow(void* p_base, Parser::ClassNode::BaseType p_base_type, stdvec<ptr<Parser::VarNode>>& p_vars) {
	switch (p_base_type) {
		case Parser::ClassNode::NO_BASE: // can't be
			return;
		case Parser::ClassNode::BASE_NATIVE: {
			String* base = (String*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				ptr<MemberInfo> mi = NativeClasses::singleton()->get_member_info(*base, v->name);
				if (mi == nullptr) continue;
				if (mi->get_type() == MemberInfo::PROPERTY) {
					const PropertyInfo* pi = static_cast<const PropertyInfo*>(mi.get());
					if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->c_str()), v->pos);
				}
			}
			String parent = NativeClasses::singleton()->get_inheritance(*base);
			if (parent != "") _check_member_var_shadow((void*)&parent, Parser::ClassNode::BASE_NATIVE, p_vars);
		} break;
		case Parser::ClassNode::BASE_EXTERN: {
			Bytecode* base = (Bytecode*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				const ptr<MemberInfo> mi = base->get_member_info(v->name);
				if (mi == nullptr) continue;
				if (mi->get_type() == MemberInfo::PROPERTY) {
					const PropertyInfo* pi = static_cast<const PropertyInfo*>(mi.get());
					if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->get_name().c_str()), v->pos);
				}
			}
			if (base->has_base()) {
				if (base->is_base_native()) _check_member_var_shadow((void*)&base->get_base_native(), Parser::ClassNode::BASE_NATIVE, p_vars);
				else _check_member_var_shadow(base->get_base_binary().get(), Parser::ClassNode::BASE_EXTERN, p_vars);
			}
		} break;
		case Parser::ClassNode::BASE_LOCAL: {
			Parser::ClassNode* base = (Parser::ClassNode*)p_base;
			for (const ptr<Parser::VarNode>& v : p_vars) {
				if (v->is_static) continue;
				for (const ptr<Parser::VarNode>& _v : base->vars) {
					if (_v->is_static) continue;
					if (_v->name == v->name) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR,
						String::format("member named \"%s\" already exists in base \"%s\"", v->name.c_str(), base->name.c_str()), v->pos);
				}
			}
			if (base->base_type == Parser::ClassNode::BASE_LOCAL) _check_member_var_shadow((void*)base->base_class, Parser::ClassNode::BASE_LOCAL, p_vars);
			else if (base->base_type == Parser::ClassNode::BASE_EXTERN) _check_member_var_shadow((void*)base->base_binary.get(), Parser::ClassNode::BASE_EXTERN, p_vars);
			else if (base->base_type == Parser::ClassNode::BASE_NATIVE) _check_member_var_shadow((void*)&base->base_class_name, Parser::ClassNode::BASE_NATIVE, p_vars);
		} break;

	}
}

void Analyzer::_resolve_inheritance(Parser::ClassNode* p_class) {

	if (p_class->is_reduced) return;
	if (p_class->_is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic inheritance. class inherits itself isn't allowed.", p_class->pos);
	p_class->_is_reducing = true;

	// resolve inheritance.
	if (p_class->base_type == Parser::ClassNode::BASE_LOCAL) {
		bool found = false;
		for (int i = 0; i < (int)file_node->classes.size(); i++) {
			if (p_class->base_class_name == file_node->classes[i]->name) {
				found = true;
				_resolve_inheritance(file_node->classes[i].get());
				p_class->base_class = file_node->classes[i].get();
			}
		}
		if (!found) throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("base class \"%s\" not found.", p_class->base_class_name.c_str()), p_class->pos);
	}

	// check if a member is already exists in the parent class.
	if (p_class->base_type == Parser::ClassNode::BASE_LOCAL) _check_member_var_shadow((void*)p_class->base_class, Parser::ClassNode::BASE_LOCAL, p_class->vars);
	else if (p_class->base_type == Parser::ClassNode::BASE_EXTERN) _check_member_var_shadow((void*)p_class->base_binary.get(), Parser::ClassNode::BASE_EXTERN, p_class->vars);
	else if (p_class->base_type == Parser::ClassNode::BASE_NATIVE) _check_member_var_shadow((void*)&p_class->base_class_name, Parser::ClassNode::BASE_NATIVE, p_class->vars);

	p_class->_is_reducing = false;
	p_class->is_reduced = true;
}

void Analyzer::_resolve_constant(Parser::ConstNode* p_const) {
	if (p_const->is_reduced) return;
	if (p_const->_is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic constant value dependancy found.", p_const->pos);
	p_const->_is_reducing = true;

	ASSERT(p_const->assignment != nullptr);
	_reduce_expression(p_const->assignment);

	if (p_const->assignment->type == Parser::Node::Type::CONST_VALUE) {
		ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_const->assignment);
		if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
			cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING &&
			cv->value.get_type() != var::_NULL) {
			throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_const->assignment->pos);
		}
		p_const->value = cv->value;
		p_const->_is_reducing = false;
		p_const->is_reduced = true;

	} else throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_const->assignment->pos);
}

void Analyzer::_resolve_parameters(Parser::FunctionNode* p_func) {
	for (int i = 0; i < p_func->args.size(); i++) {
		if (p_func->args[i].default_value != nullptr) {
			_reduce_expression(p_func->args[i].default_value);
			if (p_func->args[i].default_value->type != Parser::Node::Type::CONST_VALUE) 
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_func->args[i].default_value->pos);
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_func->args[i].default_value);
			if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
				cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING &&
				cv->value.get_type() != var::_NULL) {
				throw ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_func->args[i].default_value->pos);
			}
			p_func->default_args.push_back(cv->value);
		}
	}
}

void Analyzer::_resolve_enumvalue(Parser::EnumValueNode& p_enumvalue, int* p_possible) {
	if (p_enumvalue.is_reduced) return;
	if (p_enumvalue._is_reducing) throw ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic enum value dependancy found.", p_enumvalue.expr->pos);
	p_enumvalue._is_reducing = true;

	if (p_enumvalue.expr != nullptr) {
		_reduce_expression(p_enumvalue.expr);
		if (p_enumvalue.expr->type != Parser::Node::Type::CONST_VALUE)
			throw ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
		ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_enumvalue.expr);
		if (cv->value.get_type() != var::INT) throw ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
		p_enumvalue.value = cv->value;
	} else {
		p_enumvalue.value = (p_possible) ? *p_possible: -1;
	}
	if (p_possible) *p_possible = (int)p_enumvalue.value + 1;

	p_enumvalue._is_reducing = false;
	p_enumvalue.is_reduced = true;
}

void Analyzer::_check_super_constructor_call(const Parser::BlockNode* p_block) {
	int constructor_argc = 0;
	int default_argc = 0;

	Parser::ClassNode* current_class = parser->parser_context.current_class;
	switch (parser->parser_context.current_class->base_type) {
		case Parser::ClassNode::BASE_LOCAL: {
			Parser::FunctionNode* constructor = current_class->base_class->constructor;
			if (constructor == nullptr) return;
			constructor_argc = (int)constructor->args.size();
			default_argc = (int)constructor->default_args.size();
		} break;
		case Parser::ClassNode::BASE_EXTERN: {
			const CarbonFunction* constructor = current_class->base_binary->get_constructor();
			if (constructor == nullptr) return;
			constructor_argc = (int)constructor->get_arg_count();
			default_argc = (int)constructor->get_default_args().size();
		} break;
		case Parser::ClassNode::BASE_NATIVE: {
			const StaticFuncBind* constructor = NativeClasses::singleton()->get_constructor(current_class->base_class_name);
			constructor_argc = (constructor) ? constructor->get_argc() : 0;
			default_argc = (constructor) ? constructor->get_method_info()->get_default_arg_count() : 0;
		} break;
	}

	if (constructor_argc - default_argc > 0) { // super call needed.
		if ((p_block->statements.size() == 0) || (p_block->statements[0]->type != Parser::Node::Type::CALL))
			throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, "super constructor call expected since base class doesn't have a default constructor.", p_block->pos);
		const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_block->statements[0].get());
		if (call->base->type != Parser::Node::Type::SUPER || call->method != nullptr)
			throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, "super constructor call expected since base class doesn't have a default constructor.", call->pos);
		current_class->has_super_ctor_call = true;
		_check_arg_count(constructor_argc, default_argc, (int)call->args.size(), call->pos);
	}

	if ((p_block->statements.size() > 0) && (p_block->statements[0]->type == Parser::Node::Type::CALL)) {
		const Parser::CallNode* call = static_cast<const Parser::CallNode*>(p_block->statements[0].get());
		if (call->base->type == Parser::Node::Type::SUPER && call->method == nullptr) current_class->has_super_ctor_call = true;
	}
}

void Analyzer::_check_arg_count(int p_argc, int p_default_argc, int p_args_given, Vect2i p_err_pos) {
	int required_min_argc = p_argc - p_default_argc;
	if (required_min_argc > 0) {
		if (p_default_argc == 0) {
			if (p_args_given != p_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected excatly %i argument(s) for super constructor call", p_argc), p_err_pos);
		} else {
			if (p_args_given < required_min_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected at least %i argument(s) for super constructor call", required_min_argc), p_err_pos);
			else if (p_args_given > p_argc) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
				String::format("expected at most %i argument(s) for super constructor call", p_argc), p_err_pos);
		}
	}
}

void Analyzer::_reduce_block(ptr<Parser::BlockNode>& p_block) {
	
	Parser::BlockNode* parent_block = parser->parser_context.current_block;
	parser->parser_context.current_block = p_block.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		Parser::BlockNode* parent_block = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context, Parser::BlockNode* p_parent_block) {
			context = p_context;
			parent_block = p_parent_block;
		}
		~ScopeDestruct() {
			context->current_block = parent_block;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser->parser_context, parent_block);

	// if reducing constructor -> check super() call
	if (parser->parser_context.current_class && parser->parser_context.current_class->base_type != Parser::ClassNode::NO_BASE) {
		if (parser->parser_context.current_class->constructor == parser->parser_context.current_func) {
			_check_super_constructor_call(p_block.get());
		}
	}

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		parser->parser_context.current_statement_ind = i;

		switch (p_block->statements[i]->type) {
			case Parser::Node::Type::UNKNOWN:
			case Parser::Node::Type::FILE:
			case Parser::Node::Type::CLASS:
			case Parser::Node::Type::ENUM:
			case Parser::Node::Type::FUNCTION:
			case Parser::Node::Type::BLOCK:
				THROW_BUG("invalid statement type in analyzer.");

			case Parser::Node::Type::IDENTIFIER: {
				_reduce_expression(p_block->statements[i]); // to check if the identifier exists.
				warnings.push_back(ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos));
				p_block->statements.erase(p_block->statements.begin() + i--);
			} break;

			case Parser::Node::Type::VAR: {
				ptr<Parser::VarNode> var_node = ptrcast<Parser::VarNode>(p_block->statements[i]);
				if (var_node->assignment != nullptr) {
					parser->parser_context.current_var = var_node.get();
					_reduce_expression(var_node->assignment);
					parser->parser_context.current_var = nullptr;
				}
			} break;

			case Parser::Node::Type::CONST: {
				ptr<Parser::ConstNode> const_node = ptrcast<Parser::ConstNode>(p_block->statements[i]);
				parser->parser_context.current_const = const_node.get();
				_resolve_constant(const_node.get());
				parser->parser_context.current_const = nullptr;
			} break;

			case Parser::Node::Type::CONST_VALUE:
			case Parser::Node::Type::ARRAY:
			case Parser::Node::Type::MAP:
				warnings.push_back(ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos));
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
				warnings.push_back(ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos));
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE:
				warnings.push_back(ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos));
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::CALL:
			case Parser::Node::Type::INDEX:
			case Parser::Node::Type::MAPPED_INDEX:
			case Parser::Node::Type::OPERATOR: {
				// TODO: if it's reduced to compile time constant it could be removed?
				_reduce_expression(p_block->statements[i]);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				ptr<Parser::ControlFlowNode> cf_node = ptrcast<Parser::ControlFlowNode>(p_block->statements[i]);
				switch (cf_node->cf_type) {

					case Parser::ControlFlowNode::IF: {
						ASSERT(cf_node->args.size() == 1);
						// TODO: if it's evaluvated to compile time constant true/false it could be optimized/warned.
						_reduce_expression(cf_node->args[0]);
						_reduce_block(cf_node->body);
						if (cf_node->body_else != nullptr) {
							_reduce_block(cf_node->body_else);
							// if it's statements cleared it needto be removed.
							if (cf_node->body_else->statements.size() == 0) cf_node->body_else = nullptr;
						}
					} break;

					case Parser::ControlFlowNode::SWITCH: {
						ASSERT(cf_node->args.size() == 1);
						// TODO: if it's evaluvated to compile time constant integer it could be optimized/warned.
						_reduce_expression(cf_node->args[0]);

						Parser::EnumNode* _switch_enum = nullptr;
						int _enum_case_count = 0; bool _check_missed_enum = true;
						if (cf_node->switch_cases.size() > 1 && cf_node->switch_cases[0].expr->type == Parser::Node::Type::IDENTIFIER) {
							Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(cf_node->switch_cases[0].expr).get();
							if (id->ref == Parser::IdentifierNode::REF_ENUM_VALUE) {
								_switch_enum = id->_enum_value->_enum;
							}
						}

						for (int j = 0; j < (int)cf_node->switch_cases.size(); j++) {
							if (cf_node->switch_cases[j].default_case) _check_missed_enum = false;
							if (_check_missed_enum && cf_node->switch_cases[j].expr->type == Parser::Node::Type::IDENTIFIER) {
								Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(cf_node->switch_cases[j].expr).get();
								if (id->ref == Parser::IdentifierNode::REF_ENUM_VALUE) {
									if (id->_enum_value->_enum == _switch_enum) _enum_case_count++;
								} else _check_missed_enum = false;
							} else _check_missed_enum = false;

							_reduce_expression(cf_node->switch_cases[j].expr);
							if (cf_node->switch_cases[j].expr->type != Parser::Node::Type::CONST_VALUE)
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "switch case value must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(cf_node->switch_cases[j].expr);
							if (cv->value.get_type() != var::INT)
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "switch case must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							cf_node->switch_cases[j].value = cv->value;

							for (int _j = 0; _j < j; _j++) {
								if (cf_node->switch_cases[_j].value == cf_node->switch_cases[j].value) {
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("case value %lli has already defined at line %lli.",
										cf_node->switch_cases[j].value, cf_node->switch_cases[_j].pos.y), cf_node->switch_cases[j].pos);
								}
							}

							_reduce_block(cf_node->switch_cases[j].body);
						}

						if (_check_missed_enum && _enum_case_count != _switch_enum->values.size()) {
							warnings.push_back(ANALYZER_WARNING(Warning::MISSED_ENUM_IN_SWITCH, "", cf_node->pos));
						}

					} break;

					case Parser::ControlFlowNode::WHILE: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);
						if (cf_node->args[0]->type == Parser::Node::Type::CONST_VALUE) {
							if (ptrcast<Parser::ConstValueNode>(cf_node->args[0])->value.operator bool()) {
								if (!cf_node->has_break) {
									warnings.push_back(ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->args[0]->pos));
								}
							} else {
								warnings.push_back(ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", cf_node->args[0]->pos));
								p_block->statements.erase(p_block->statements.begin() + i--);
							}
						}
						_reduce_block(cf_node->body);
					} break;

					case Parser::ControlFlowNode::FOR: {
						ASSERT(cf_node->args.size() == 3);

						// reduce loop arguments.
						Parser::BlockNode* parent_block = parser->parser_context.current_block;
						parser->parser_context.current_block = cf_node->body.get();
						if (cf_node->args[0] != nullptr && cf_node->args[0]->type == Parser::Node::Type::VAR) {
							cf_node->body->local_vars.push_back(ptrcast<Parser::VarNode>(cf_node->args[0]));
							_reduce_expression(ptrcast<Parser::VarNode>(cf_node->args[0])->assignment);
						}
						else _reduce_expression(cf_node->args[0]);
						_reduce_expression(cf_node->args[1]);
						_reduce_expression(cf_node->args[2]);
						parser->parser_context.current_block = parent_block;

						_reduce_block(cf_node->body);
						// TODO: if it's evaluvated to compile time constant it could be optimized/warned.
						if (cf_node->args[0] == nullptr && cf_node->args[1] == nullptr && cf_node->args[2] == nullptr) {
							if (!cf_node->has_break) {
								warnings.push_back(ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->pos));
							}
						}
					} break;

					case Parser::ControlFlowNode::FOREACH: {
						ASSERT(cf_node->args.size() == 2);

						// reduce loop arguments.
						Parser::BlockNode* parent_block = parser->parser_context.current_block;
						parser->parser_context.current_block = cf_node->body.get();
						cf_node->body->local_vars.push_back(ptrcast<Parser::VarNode>(cf_node->args[0]));
						_reduce_expression(ptrcast<Parser::VarNode>(cf_node->args[0])->assignment);
						_reduce_expression(cf_node->args[1]);
						parser->parser_context.current_block = parent_block;

						_reduce_block(cf_node->body);
					} break;

					case Parser::ControlFlowNode::BREAK:
					case Parser::ControlFlowNode::CONTINUE: {
						ASSERT(cf_node->args.size() == 0);
					} break;
					case Parser::ControlFlowNode::RETURN: {
						ASSERT(cf_node->args.size() <= 1);
						if (cf_node->args.size() == 1) {
							_reduce_expression(cf_node->args[0]);
						}
					} break;
				}
			} break;
		} // statement switch ends.
	}
	parser->parser_context.current_statement_ind = -1;

	// remove reduced && un-wanted statements.
	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		// remove all local constant statments. no need anymore.
		if (p_block->statements[i]->type == Parser::Node::Type::CONST) {
			p_block->statements.erase(p_block->statements.begin() + i--);

		} else if (p_block->statements[i]->type == Parser::Node::Type::CONST_VALUE) {
			warnings.push_back(ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos));
			p_block->statements.erase(p_block->statements.begin() + i--);

		// remove all statements after return
		} else if (p_block->statements[i]->type == Parser::Node::Type::CONTROL_FLOW) {
			Parser::ControlFlowNode* cf = ptrcast<Parser::ControlFlowNode>(p_block->statements[i]).get();
			if (cf->cf_type == Parser::ControlFlowNode::RETURN) {
				if (i != p_block->statements.size() - 1) {
					warnings.push_back(ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i + 1]->pos));
					p_block->statements.erase(p_block->statements.begin() + i + 1, p_block->statements.end());
				}
			} else if (cf->cf_type == Parser::ControlFlowNode::IF) {
				if (cf->args[0]->type == Parser::Node::Type::CONST_VALUE && ptrcast<Parser::ConstValueNode>(cf->args[0])->value.operator bool() == false) {
					warnings.push_back(ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i]->pos));
					if (cf->body_else == nullptr) p_block->statements.erase(p_block->statements.begin() + i--);
					// TODO: move else body to if body and set condition to true.
				}
			}

		// remove all compile time functions.
		} else if (p_block->statements[i]->type == Parser::Node::Type::CALL) {
			Parser::CallNode* call = ptrcast<Parser::CallNode>(p_block->statements[i]).get();
			if (call->base->type == Parser::Node::Type::BUILTIN_FUNCTION) {
				if (BuiltinFunctions::is_compiletime(ptrcast<Parser::BuiltinFunctionNode>(call->base)->func)) {
					p_block->statements.erase(p_block->statements.begin() + i--);
				}
			}
		}
	}
}

}