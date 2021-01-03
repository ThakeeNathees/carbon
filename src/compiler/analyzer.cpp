//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020-2021 Thakee Nathees
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

#include "compiler/analyzer.h"
#include "compiler/function.h"

/******************************************************************************************************************/
/*                                         ANALYZER                                                               */
/******************************************************************************************************************/

namespace carbon {

CompileTimeError Analyzer::_analyzer_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source,
		std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
}

void Analyzer::_analyzer_warning(Warning::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) {
	uint32_t err_len = 1;
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = parser->tokenizer->get_token_at(p_pos).to_string();
	else token_str = parser->tokenizer->peek(-1, true).to_string();
	if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;
	else err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : parser->tokenizer->peek(-1, true).get_pos();
	Warning warning(p_type, p_msg,
		DBGSourceInfo(file_node->path, file_node->source,
			std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
	
	warnings.push_back(warning);
}

const stdvec<Warning>& Analyzer::get_warnings() const {
	return warnings;
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
	for (auto& func : file_node->compiletime_functions) _resolve_compiletime_funcs(func);
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (auto& func : file_node->classes[i]->compiletime_functions)
			_resolve_compiletime_funcs(func);
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

	// file level function.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		parser->parser_context.current_func = file_node->functions[i].get();
		Parser::FunctionNode* fn = file_node->functions[i].get();

		if (fn->name == GlobalStrings::main) {
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
					if (ctor && ctor->get_method_info()->get_arg_count()-1/*self*/ - ctor->get_method_info()->get_default_arg_count() != 0)
						can_add_default_ctor = false;
				} break;
				case Parser::ClassNode::BASE_EXTERN: {
					const Function* ctor = cls->base_binary->get_constructor();
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

void Analyzer::_resolve_compiletime_funcs(const ptr<Parser::CallNode>& p_func) {
	Parser::CallNode* call = p_func.get();
	ASSERT(call->is_compilttime);
	ASSERT(call->base->type == Parser::Node::Type::BUILTIN_FUNCTION);
	Parser::BuiltinFunctionNode* bf = ptrcast<Parser::BuiltinFunctionNode>(call->base).get();
	stdvec<var*> args;
	for (int j = 0; j < (int)call->args.size(); j++) {
		_reduce_expression(call->args[j]);
		if (call->args[j]->type != Parser::Node::Type::CONST_VALUE) {
			throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("compiletime function arguments must be compile time known values."), p_func->args[j]->pos);
		}
		args.push_back(&ptrcast<Parser::ConstValueNode>(call->args[j])->value);
	}
	_call_compiletime_func(bf, args);
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
			const Function* constructor = current_class->base_binary->get_constructor();
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
	if (p_argc == -1 /*va args*/) return;

	// TODO: error message 
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
		// no argument is required -> check if argc exceeding
	} else if (p_args_given > p_argc) {
		throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT,
			String::format("expected at most %i argument(s) for super constructor call", p_argc), p_err_pos);
	}
}

}

/******************************************************************************************************************/
/*                                         REDUCE BLOCK                                                           */
/******************************************************************************************************************/

namespace carbon {

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
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
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
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE:
				ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

			case Parser::Node::Type::CALL: {
				ptr<Parser::CallNode> call = ptrcast<Parser::CallNode>(p_block->statements[i]);
				if (call->is_compilttime) {
					_resolve_compiletime_funcs(call);
					p_block->statements.erase(p_block->statements.begin() + i--);
					break;
				}
			} // [[FALLTHROUGH]]
			case Parser::Node::Type::INDEX:
			case Parser::Node::Type::MAPPED_INDEX:
			case Parser::Node::Type::OPERATOR: {
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
							ANALYZER_WARNING(Warning::MISSED_ENUM_IN_SWITCH, "", cf_node->pos);
						}

					} break;

					case Parser::ControlFlowNode::WHILE: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);
						if (cf_node->args[0]->type == Parser::Node::Type::CONST_VALUE) {
							if (ptrcast<Parser::ConstValueNode>(cf_node->args[0])->value.operator bool()) {
								if (!cf_node->has_break) {
									ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->args[0]->pos);
								}
							} else {
								ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", cf_node->args[0]->pos);
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
						} else _reduce_expression(cf_node->args[0]);
						_reduce_expression(cf_node->args[1]);
						_reduce_expression(cf_node->args[2]);
						parser->parser_context.current_block = parent_block;

						_reduce_block(cf_node->body);
						// TODO: if it's evaluvated to compile time constant it could be optimized/warned.
						if (cf_node->args[0] == nullptr && cf_node->args[1] == nullptr && cf_node->args[2] == nullptr) {
							if (!cf_node->has_break) {
								ANALYZER_WARNING(Warning::NON_TERMINATING_LOOP, "", cf_node->pos);
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
			ANALYZER_WARNING(Warning::STAND_ALONE_EXPRESSION, "", p_block->statements[i]->pos);
			p_block->statements.erase(p_block->statements.begin() + i--);

			// remove all statements after return
		} else if (p_block->statements[i]->type == Parser::Node::Type::CONTROL_FLOW) {
			Parser::ControlFlowNode* cf = ptrcast<Parser::ControlFlowNode>(p_block->statements[i]).get();
			if (cf->cf_type == Parser::ControlFlowNode::RETURN) {
				if (i != p_block->statements.size() - 1) {
					ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i + 1]->pos);
					p_block->statements.erase(p_block->statements.begin() + i + 1, p_block->statements.end());
				}
			} else if (cf->cf_type == Parser::ControlFlowNode::IF) {
				if (cf->args[0]->type == Parser::Node::Type::CONST_VALUE && ptrcast<Parser::ConstValueNode>(cf->args[0])->value.operator bool() == false) {
					if (cf->body_else == nullptr) {
						ANALYZER_WARNING(Warning::UNREACHABLE_CODE, "", p_block->statements[i]->pos);
						p_block->statements.erase(p_block->statements.begin() + i--);
					}
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

} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE EXPRESSION                                                      */
/******************************************************************************************************************/


namespace carbon {

void Analyzer::_reduce_expression(ptr<Parser::Node>& p_expr) {

	if (p_expr == nullptr) return;

	// Prevent stack overflow.
	if (p_expr->is_reduced) return;
	p_expr->is_reduced = true;

	switch (p_expr->type) {

		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER:
		case Parser::Node::Type::BUILTIN_TYPE:
		case Parser::Node::Type::BUILTIN_FUNCTION:
			break;

		case Parser::Node::Type::IDENTIFIER: {
			_reduce_identifier(p_expr);
			_check_identifier(p_expr);
		} break;

			// reduce ArrayNode
		case Parser::Node::Type::ARRAY: {
			ptr<Parser::ArrayNode> arr = ptrcast<Parser::ArrayNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
				if (arr->elements[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			arr->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Array arr_v;
				for (int i = 0; i < (int)arr->elements.size(); i++) {
					arr_v.push_back(ptrcast<Parser::ConstValueNode>(arr->elements[i])->value);
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(arr_v);
				cv->pos = arr->pos; p_expr = cv;
			}
		} break;

			// reduce MapNode
		case Parser::Node::Type::MAP: {
			ptr<Parser::MapNode> map = ptrcast<Parser::MapNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: if key is const value and two keys are the same throw error.
				if (map->elements[i].key->type == Parser::Node::Type::CONST_VALUE) {
					var& key_v = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					if (!var::is_hashable(key_v.get_type())) throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("unhasnable type %s used as map key.", key_v.get_type_name().c_str()), map->pos);
				}
				_reduce_expression(map->elements[i].value);

				if (map->elements[i].key->type != Parser::Node::Type::CONST_VALUE || map->elements[i].value->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			map->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Map map_v;
				for (int i = 0; i < (int)map->elements.size(); i++) {
					var& _key = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					var& _val = ptrcast<Parser::ConstValueNode>(map->elements[i].value)->value;
					map_v[_key] = _val;
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(map_v);
				cv->pos = map->pos; p_expr = cv;
			}
		} break;

		case Parser::Node::Type::CALL: {
			_reduce_call(p_expr);
		} break;

		case Parser::Node::Type::INDEX: {
			_reduce_indexing(p_expr);
		} break;

			// reduce MappedIndexNode
		case Parser::Node::Type::MAPPED_INDEX: {
			ptr<Parser::MappedIndexNode> mapped_index = ptrcast<Parser::MappedIndexNode>(p_expr);
			_reduce_expression(mapped_index->base);
			_reduce_expression(mapped_index->key);
			if (mapped_index->base->type == Parser::Node::Type::CONST_VALUE && mapped_index->key->type == Parser::Node::Type::CONST_VALUE) {
				Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(mapped_index->base).get();
				Parser::ConstValueNode* key = ptrcast<Parser::ConstValueNode>(mapped_index->key).get();
				try {
					ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.__get_mapped(key->value));
					cv->pos = base->pos; p_expr = cv;
				} catch (Error& err) {
					throw ANALYZER_ERROR(err.get_type(), err.what(), key->pos);
				}
			}
		} break;

			///////////////////////////////////////////////////////////////////////////////////////////////

		case Parser::Node::Type::OPERATOR: {
			ptr<Parser::OperatorNode> op = ptrcast<Parser::OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Parser::Node::Type::CONST_VALUE) all_const = false;
			}

			switch (op->op_type) {

				case Parser::OperatorNode::OpType::OP_EQ:
				case Parser::OperatorNode::OpType::OP_PLUSEQ:
				case Parser::OperatorNode::OpType::OP_MINUSEQ:
				case Parser::OperatorNode::OpType::OP_MULEQ:
				case Parser::OperatorNode::OpType::OP_DIVEQ:
				case Parser::OperatorNode::OpType::OP_MOD_EQ:
				case Parser::OperatorNode::OpType::OP_LTEQ:
				case Parser::OperatorNode::OpType::OP_GTEQ:
				case Parser::OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_OR_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_AND_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_XOR_EQ: {

					if (op->args[0]->type == Parser::Node::Type::IDENTIFIER) {
						switch (ptrcast<Parser::IdentifierNode>(op->args[0])->ref) {
							case Parser::IdentifierNode::REF_PARAMETER:
							case Parser::IdentifierNode::REF_LOCAL_VAR:
							case Parser::IdentifierNode::REF_MEMBER_VAR:
							case Parser::IdentifierNode::REF_STATIC_VAR:
								break;
							case Parser::IdentifierNode::REF_LOCAL_CONST:
							case Parser::IdentifierNode::REF_MEMBER_CONST:
							case Parser::IdentifierNode::REF_ENUM_NAME:
							case Parser::IdentifierNode::REF_ENUM_VALUE:
							case Parser::IdentifierNode::REF_FUNCTION:
							case Parser::IdentifierNode::REF_CARBON_CLASS:
							case Parser::IdentifierNode::REF_NATIVE_CLASS:
							case Parser::IdentifierNode::REF_EXTERN:
								throw ANALYZER_ERROR(Error::TYPE_ERROR, "invalid assignment (only assignment on variables/parameters are valid).", op->args[0]->pos);
						}
					} else if (op->args[0]->type == Parser::Node::Type::THIS) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"this\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::SUPER) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"super\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to constant values.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::ARRAY) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to array literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::MAP) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to map literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to builtin function.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE) {
						throw ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to builtin type.", op->args[0]->pos);
					}
				} break;

				default: { // Remaining binary/unary operators.
					if (!all_const) break;
					stdvec<var*> args;
					for (int i = 0; i < (int)op->args.size(); i++) args.push_back(&ptrcast<Parser::ConstValueNode>(op->args[i])->value);
				#define SET_EXPR_CONST_NODE(m_expr, m_pos)											      \
					do {                                                                                  \
						var value;																		  \
						try {																			  \
							value = (m_expr);														      \
						} catch (Throwable& err) {														  \
							throw ANALYZER_ERROR(err.get_type(), err.what(), op->pos);					  \
						}																				  \
						ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);         \
						cv->pos = m_pos;                                                                  \
						p_expr = cv;                                                                      \
					} while (false)

					switch (op->op_type) {
						case Parser::OperatorNode::OpType::OP_EQEQ:
							SET_EXPR_CONST_NODE(*args[0] == *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_PLUS:
							SET_EXPR_CONST_NODE(*args[0] + *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MINUS:
							SET_EXPR_CONST_NODE(*args[0] - *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MUL:
							SET_EXPR_CONST_NODE(*args[0] * *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_DIV:
							SET_EXPR_CONST_NODE(*args[0] / *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MOD:
							SET_EXPR_CONST_NODE(*args[0] % *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_LT:
							SET_EXPR_CONST_NODE(*args[0] < *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_GT:
							SET_EXPR_CONST_NODE(*args[0] > * args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_AND:
							SET_EXPR_CONST_NODE(args[0]->operator bool() && args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_OR:
							SET_EXPR_CONST_NODE(args[0]->operator bool() || args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_NOTEQ:
							SET_EXPR_CONST_NODE(*args[0] != *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_LSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() << args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_RSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() >> args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_OR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() | args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_AND:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() & args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_XOR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() ^ args[1]->operator int64_t(), op->pos);
							break;

						case Parser::OperatorNode::OpType::OP_NOT:
							SET_EXPR_CONST_NODE(!args[0]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_NOT:
							SET_EXPR_CONST_NODE(~args[0]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_POSITIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
								case var::FLOAT: {
									SET_EXPR_CONST_NODE(*args[0], op->pos);
								} break;
								default:
									throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"+\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
						case Parser::OperatorNode::OpType::OP_NEGATIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
									SET_EXPR_CONST_NODE(-args[0]->operator int64_t(), op->pos);
									break;
								case var::FLOAT:
									SET_EXPR_CONST_NODE(-args[0]->operator double(), op->pos);
									break;
								default:
									throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"-\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
					}
					#undef SET_EXPR_CONST_NODE
					MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 33);
				}
			}
		} break;

		case Parser::Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			THROW_BUG("Invalid expression node.");
		}
	}
}
} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE IDENTIFIER                                                      */
/******************************************************************************************************************/

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

} // namespace carbon


/******************************************************************************************************************/
/*                                         REDUCE INDEXING                                                        */
/******************************************************************************************************************/

namespace carbon {

void Analyzer::_reduce_indexing(ptr < Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::INDEX);

	ptr<Parser::IndexNode> index = ptrcast<Parser::IndexNode>(p_expr);
	_reduce_expression(index->base);
	ASSERT(index->member->type == Parser::Node::Type::IDENTIFIER);
	ptr<Parser::IdentifierNode> member = ptrcast<Parser::IdentifierNode>(index->member);

	switch (index->base->type) {

		// String.prop; index base on built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			Parser::BuiltinTypeNode* bt = ptrcast<Parser::BuiltinTypeNode>(index->base).get();
			const MemberInfo* mi = TypeInfo::get_member_info(BuiltinTypes::get_var_type(bt->builtin_type), member->name).get();
			if (!mi) throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), BuiltinTypes::get_type_name(bt->builtin_type).c_str()), member->pos);

			switch (mi->get_type()) {
				// var x = String.format;
				case MemberInfo::METHOD:
					break;

					// var x = int.something ? <-- is this even valid
				case MemberInfo::PROPERTY: {
					PropertyInfo* pi = (PropertyInfo*)mi;
					if (pi->is_const()) {
						ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(pi->get_value());
						cv->pos = index->pos, p_expr = cv;
					}
					else THROW_BUG("can't be."); // builtin types can only have a constant property
				} break;

					// built in types cannot contain enum inside.
				case MemberInfo::ENUM:
				case MemberInfo::ENUM_VALUE:
					THROW_BUG("can't be.");
			}
		} break;

			// "string".member;
		case Parser::Node::Type::CONST_VALUE: {
			Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(index->base).get();
			try {
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.get_member(member->name));
				cv->pos = member->pos; p_expr = cv;
			} catch (Error& err) {
				throw ANALYZER_ERROR(err.get_type(), err.what(), index->pos);
			}
		} break;

			// this.member; super.member; idf.member;
		case Parser::Node::Type::INDEX: { // <-- base is index node but reference reduced.
			Parser::IndexNode* _ind = ptrcast<Parser::IndexNode>(index->base).get();
			if (!_ind->_ref_reduced) break;
		}  // [[ FALLTHROUGH ]]
		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER:
		case Parser::Node::Type::IDENTIFIER: {

			Parser::IdentifierNode* base;
			enum _BaseClassRef { _THIS, _SUPER, _NEITHER };
			_BaseClassRef _base_class_ref = _NEITHER;

			if (index->base->type == Parser::Node::Type::THIS) {
				ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->name);
				_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
				_id->_class = parser->parser_context.current_class;
				index->base = _id;
				_base_class_ref = _THIS;
			} else if (index->base->type == Parser::Node::Type::SUPER) {
				if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_LOCAL) {
					ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
					_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
					_id->_class = parser->parser_context.current_class->base_class;
					index->base = _id;
				} else if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_EXTERN) {
					ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
					_id->ref = Parser::IdentifierNode::REF_EXTERN;
					_id->_bytecode = parser->parser_context.current_class->base_binary.get();
					index->base = _id;
				}
				_base_class_ref = _SUPER;
			}

			if (index->base->type == Parser::Node::Type::INDEX) {
				base = ptrcast<Parser::IndexNode>(index->base)->member.get();
			} else {
				base = ptrcast<Parser::IdentifierNode>(index->base).get();
			}

			switch (base->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN: {
					THROW_BUG("base can't be unknown.");
				} break;

				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR:
					break; // Can't reduce anymore.

				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					THROW_BUG("there isn't any contant value currently support attribute access and most probably in the future");
				} break;

					// EnumClass.prop; <-- TODO: could the prop be a method?
				case Parser::IdentifierNode::REF_ENUM_NAME: {
					if (base->ref_base == Parser::IdentifierNode::BASE_LOCAL) {
						stdmap<String, Parser::EnumValueNode>::iterator it = base->_enum_node->values.find(member->name);
						if (it != base->_enum_node->values.end()) {
							member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							member->ref_base = Parser::IdentifierNode::BASE_LOCAL;
							member->_enum_value = &(it->second);
							_resolve_enumvalue(base->_enum_node->values[it->first]);
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->_enum_node->values[it->first].value);
							cv->pos = member->pos; p_expr = cv;
						} else {
							throw ANALYZER_ERROR(Error::NAME_ERROR,
								String::format("\"%s\" doesn't exists on base enum \"%s\"%.", member->name.c_str(), base->_enum_node->name.c_str()),
								member->pos);
						}
					} else { // ref on base native/extern.
						stdmap<String, int64_t>::const_iterator it = base->_enum_info->get_values().find(member->name);
						if (it != base->_enum_info->get_values().end()) {
							member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							member->ref_base = base->ref_base;
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(it->second);
							cv->pos = member->pos; p_expr = cv;
						} else {
							throw ANALYZER_ERROR(Error::NAME_ERROR,
								String::format("\"%s\" doesn't exists on base enum \"%s\"%.", member->name.c_str(), base->_enum_info->get_name().c_str()),
								member->pos);
						}
					}
				} break;

				case Parser::IdentifierNode::REF_ENUM_VALUE:
					throw ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enum value doesn't support attribute access.", member->pos);

					// Aclass.prop;
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					Parser::IdentifierNode _id = _find_member(base->_class, member->name);
					_id.pos = member->pos;

					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
							THROW_BUG("can't be.");

							// Aclass.a_var <-- only valid if the base is this `this.a_var`
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							_id.pos = member->pos;
							if (_base_class_ref != _THIS && !_id._var->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
							}
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

							// Aclass.CONST;
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							var value;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._const->value;
							else value = _id._prop_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

							// Aclass.EnumClass;
						case Parser::IdentifierNode::REF_ENUM_NAME: {
							_id.pos = member->pos;
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

							// Aclass.ENUM_VALUE
						case Parser::IdentifierNode::REF_ENUM_VALUE: {
							int64_t value = 0;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._enum_value->value;
							else value = _id._enum_value_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

							// Aclass.a_function;
						case Parser::IdentifierNode::REF_FUNCTION: {
							_id.pos = member->pos;

							if (_base_class_ref != _THIS && !_id._func->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
							}

							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} break;

					// File.prop;
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(base->name));
					BindData* bd = NativeClasses::singleton()->find_bind_data(base->name, member->name).get();
					if (!bd) throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), base->name.c_str()), member->pos);
					switch (bd->get_type()) {
						case BindData::METHOD:
						case BindData::STATIC_FUNC:
						case BindData::MEMBER_VAR:
						case BindData::STATIC_VAR:
						case BindData::ENUM:
							break;

							// NativeClass.CONT_VALUE
						case BindData::STATIC_CONST: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((ConstantBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;

							// File.READ
						case BindData::ENUM_VALUE: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((EnumValueBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;
					}

				} break;

					// f.attrib;
				case Parser::IdentifierNode::REF_FUNCTION: {
					// at runtime it'll return a ptr<CarbonFunction> reference.
					// TODO: check attribute.
				} break;

					// extern_class.prop;
				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check attrib
				} break;
			}
		}

		default:
			break;
			// RUNTIME.
	}
}

} // namespace carbon

/******************************************************************************************************************/
/*                                         REDUCE CALL                                                            */
/******************************************************************************************************************/

namespace carbon {

#define GET_ARGS(m_nodes)                                                             \
	stdvec<var*> args;                                                                \
	for (int i = 0; i < (int)m_nodes.size(); i++) {                                   \
	    args.push_back(&ptrcast<Parser::ConstValueNode>(m_nodes[i])->value);          \
	}

#define SET_EXPR_CONST_NODE(m_var, m_pos)                                             \
do {                                                                                  \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);         \
	cv->pos = m_pos, p_expr = cv;                                                     \
} while (false)


void Analyzer::_reduce_call(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::CALL);

	ptr<Parser::CallNode> call = ptrcast<Parser::CallNode>(p_expr);

	// reduce arguments.
	bool all_const = true;
	for (int i = 0; i < (int)call->args.size(); i++) {
		_reduce_expression(call->args[i]);
		if (call->args[i]->type != Parser::Node::Type::CONST_VALUE) {
			all_const = false;
		}
	}

	// reduce base.
	if (call->base->type == Parser::Node::Type::BUILTIN_FUNCTION || call->base->type == Parser::Node::Type::BUILTIN_TYPE) {
		// don't_reduce_anything();
	} else {
		if (call->base->type == Parser::Node::Type::UNKNOWN) {
			_reduce_expression(call->method);
			if (call->method->type == Parser::Node::Type::CONST_VALUE)
				throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable."), call->pos);
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
		} else {
			_reduce_expression(call->base);
		}
	}

	switch (call->base->type) {

		// print.a_method(); print(); call base is builtin function.
		case Parser::Node::Type::BUILTIN_FUNCTION: {

			if (call->method == nullptr) { // print();
				if (all_const) {
					ptr<Parser::BuiltinFunctionNode> bf = ptrcast<Parser::BuiltinFunctionNode>(call->base);
					if (BuiltinFunctions::can_const_fold(bf->func)) {
						GET_ARGS(call->args);
						if (BuiltinFunctions::is_compiletime(bf->func)) {
							var ret = _call_compiletime_func(bf.get(), args);
							SET_EXPR_CONST_NODE(ret, call->pos);
						} else {
							try {
								var ret;
								BuiltinFunctions::call(bf->func, args, ret);
								SET_EXPR_CONST_NODE(ret, call->pos);
							} catch (Error& err) {
								throw ANALYZER_ERROR(err.get_type(), err.what(), call->pos);
							}
						}
					}
				}
			} else { // print.a_method();
				// TODO: check method exists, if (all_const) reduce();
			}

		} break;

			// String(); String.format(...); method call on base built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			if (call->method == nullptr) { // String(...); constructor.
				Parser::BuiltinTypeNode* bt = static_cast<Parser::BuiltinTypeNode*>(call->base.get());
				if (all_const && BuiltinTypes::can_construct_compile_time(bt->builtin_type)) {
					try {
						GET_ARGS(call->args);
						var ret = BuiltinTypes::construct(bt->builtin_type, args);
						SET_EXPR_CONST_NODE(ret, call->pos);
					} catch (Error& err) {
						throw ANALYZER_ERROR(err.get_type(), err.what(), call->base->pos);
					}
				}
			} else { // String.format(); static func call.
				// TODO: check if exists, reduce if compile time callable.
			}

		} break;

			// method call on base const value.
		case Parser::Node::Type::CONST_VALUE: {
			if (all_const) {
				try {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					GET_ARGS(call->args); // 0 : const value, 1: name, ... args.
					var ret = ptrcast<Parser::ConstValueNode>(call->base)->value.call_method(ptrcast<Parser::IdentifierNode>(call->method)->name, args);
					SET_EXPR_CONST_NODE(ret, call->pos);
				} catch (const Error& err) {
					throw ANALYZER_ERROR(err.get_type(), err.what(), call->method->pos);
				}
			}
		} break;

			// call base is unknown. search method from this to super, or static function.
		case Parser::Node::Type::UNKNOWN: {

			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();
			switch (id->ref) {

				// a_var(); call `__call` method on the variable.
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					call->base = call->method; // param(args...); -> will call param.__call(args...);
					call->method = nullptr;
				} break;

					// f(); calling a local carbon function.
				case Parser::IdentifierNode::REF_FUNCTION: {

					bool is_illegal_call = false;
					int argc = 0;
					int argc_default = 0;
					int argc_given = (int)call->args.size();

					switch (id->ref_base) {
						case Parser::IdentifierNode::BASE_UNKNOWN:
							THROW_BUG("can't be"); // call base is empty.
						case Parser::IdentifierNode::BASE_EXTERN:
							THROW_BUG("TODO:");
						case Parser::IdentifierNode::BASE_NATIVE: {
							// is_illegal_call = //TODO: impl
							argc = id->_method_info->get_arg_count();
							argc_default = id->_method_info->get_default_arg_count();
						} break;
						case Parser::IdentifierNode::BASE_LOCAL: {
							// TODO: this logic may be false.
							is_illegal_call = parser->parser_context.current_class && !id->_func->is_static;
							argc = (int)id->_func->args.size();
							argc_default = (int)id->_func->default_args.size();
						} break;
					}

					if (is_illegal_call) { // can't call a non-static function.
						if ((parser->parser_context.current_func && parser->parser_context.current_func->is_static) ||
							(parser->parser_context.current_var && parser->parser_context.current_var->is_static)) {
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
						}
					}

					_check_arg_count(argc, argc_default, argc_given, call->pos);

				} break;

					// Aclass(...); calling carbon class constructor.
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					if (id->_class->constructor) {
						int argc = (int)id->_class->constructor->args.size();
						int argc_default = (int)id->_class->constructor->default_args.size();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
					} else {
						if (call->args.size() != 0)
							throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

					// File(...); calling a native class constructor.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(id->name));
					const StaticFuncBind* initializer = NativeClasses::singleton()->get_constructor(id->name);
					if (initializer) {
						// check arg counts.
						int argc = initializer->get_method_info()->get_arg_count() - 1; // -1 for self argument.
						int argc_default = initializer->get_method_info()->get_default_arg_count();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
						// check arg types.
						const stdvec<VarTypeInfo>& arg_types = initializer->get_method_info()->get_arg_types();
						for (int i = 0; i < argc_given; i++) {
							if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
								var value = ptrcast<Parser::ConstValueNode>(call->args[i])->value;
								if (!var::is_compatible(value.get_type(), arg_types[i + 1].type)) // +1 for skip self argument.
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(arg_types[i + 1].type), i), call->args[i]->pos);
							}
						}
					} else {
						if (call->args.size() != 0)
							throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

					// invalid callables.
					//case Parser::IdentifierNode::REF_ENUM_NAME:
					//case Parser::IdentifierNode::REF_ENUM_VALUE:
					//case Parser::IdentifierNode::REF_FILE:
					//case Parser::IdentifierNode::REF_LOCAL_CONST:
					//case Parser::IdentifierNode::REF_MEMBER_CONST:
				default: {
					throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" is not callable.", id->name.c_str()), id->pos);
				}
			}

		} break;

			// this.method(); super.method();
		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER: {
			const Parser::ClassNode* curr_class = parser->parser_context.current_class;

			if (call->method == nullptr) {
				if (call->base->type == Parser::Node::Type::THIS) { // this(); = __call() = operator ()()
					const Parser::FunctionNode* func = nullptr;
					for (const ptr<Parser::FunctionNode>& fn : curr_class->functions) {
						if (fn->name == GlobalStrings::__call) {
							func = fn.get(); break;
						}
					}
					if (func == nullptr) throw ANALYZER_ERROR(Error::NOT_IMPLEMENTED, String::format("operator method __call not implemented on base %s", curr_class->name.c_str()), call->pos);
					_check_arg_count((int)func->args.size(), (int)func->default_args.size(), (int)call->args.size(), call->pos);
				} else { // super();
					if (parser->parser_context.current_class == nullptr || parser->parser_context.current_class->base_type == Parser::ClassNode::NO_BASE ||
						(parser->parser_context.current_class->constructor != parser->parser_context.current_func))
						throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "invalid super call.", call->pos);
					if ((parser->parser_context.current_statement_ind != 0) || (parser->parser_context.current_block->statements[0].get() != p_expr.get()))
						throw ANALYZER_ERROR(Error::SYNTAX_ERROR, "super call should be the first and stand-alone statement of a constructor.", call->pos);

					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE:
							THROW_BUG("it should be an analyzer error");
						case Parser::ClassNode::BASE_LOCAL: {
							const Parser::FunctionNode* base_constructor = curr_class->base_class->constructor;
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count((int)base_constructor->args.size(), (int)base_constructor->default_args.size(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_NATIVE: {
							const StaticFuncBind* base_constructor = NativeClasses::singleton()->get_constructor(curr_class->base_class_name);
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_argc(), base_constructor->get_method_info()->get_default_arg_count(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_EXTERN: {
							const Function* base_constructor = curr_class->base_binary->get_constructor();
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_arg_count(), (int)base_constructor->get_default_args().size(), (int)call->args.size(), call->pos);
						} break;
					}
				}

			} else {
				const String& method_name = ptrcast<Parser::IdentifierNode>(call->method)->name;
				if (call->base->type == Parser::Node::Type::THIS) {
					// this.method();
					Parser::IdentifierNode _id = _find_member(curr_class, method_name);
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

							// this.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							call->base = call->method;
							call->method = nullptr;
						} break;

							// this.CONST(); inavlid callables.
						case Parser::IdentifierNode::REF_MEMBER_CONST:
						case Parser::IdentifierNode::REF_ENUM_NAME:
						case Parser::IdentifierNode::REF_ENUM_VALUE:
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

							// this.f(); // function call on this.
						case Parser::IdentifierNode::REF_FUNCTION: {
							if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), _id.pos);
							}

							int argc = (int)_id._func->args.size();
							int argc_default = (int)_id._func->default_args.size();
							_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

						} break;

						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} else { // super.method();


					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE: THROW_BUG("it should be an analyzer error");


						case Parser::ClassNode::BASE_LOCAL: {
							Parser::IdentifierNode _id = _find_member(curr_class->base_class, method_name);
							switch (_id.ref) {
								case Parser::IdentifierNode::REF_UNKNOWN:
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

									// super.a_var();
								case Parser::IdentifierNode::REF_MEMBER_VAR: {
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;

									// super.CONST(); inavlid callables.
								case Parser::IdentifierNode::REF_MEMBER_CONST:
								case Parser::IdentifierNode::REF_ENUM_NAME:
								case Parser::IdentifierNode::REF_ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

									// super.f(); // function call on super.
								case Parser::IdentifierNode::REF_FUNCTION: {
									if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
										throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), call->pos);
									}

									int argc = (int)_id._func->args.size();
									int argc_default = (int)_id._func->default_args.size();
									_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

								} break;

								case Parser::IdentifierNode::REF_PARAMETER:
								case Parser::IdentifierNode::REF_LOCAL_VAR:
								case Parser::IdentifierNode::REF_LOCAL_CONST:
								case Parser::IdentifierNode::REF_CARBON_CLASS:
								case Parser::IdentifierNode::REF_NATIVE_CLASS:
								case Parser::IdentifierNode::REF_EXTERN:
									THROW_BUG("can't be");
							}
						} break;

							// super.method(); // super is native
						case Parser::ClassNode::BASE_NATIVE: {
							// TODO: can also check types at compile time it arg is constvalue.
							ptr<BindData> bd = NativeClasses::singleton()->find_bind_data(curr_class->base_class_name, method_name);
							if (bd == nullptr) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (bd->get_type()) {
								case BindData::METHOD: { // super.method();
									if (parser->parser_context.current_func->is_static) { // calling super method from static function.
										throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", method_name.c_str()), call->pos);
									}
									const MethodInfo* mi = ptrcast<MethodBind>(bd)->get_method_info().get();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::STATIC_FUNC: { // super.sfunc();
									const MethodInfo* mi = ptrcast<StaticFuncBind>(bd)->get_method_info().get();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::MEMBER_VAR: { // super.a_var();
									throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case BindData::STATIC_VAR:
									break; // OK
								case BindData::STATIC_CONST:
								case BindData::ENUM:
								case BindData::ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
									break;
							}
						} break;

							// super.method() // super is extern
						case Parser::ClassNode::BASE_EXTERN: {
							const MemberInfo* info = curr_class->base_binary->get_member_info(method_name).get();
							if (info == nullptr) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (info->get_type()) {
								case MemberInfo::METHOD: {
									const MethodInfo* mi = static_cast<const MethodInfo*>(info);
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case MemberInfo::PROPERTY: {
									const PropertyInfo* pi = static_cast<const PropertyInfo*>(info);
									if (!pi->is_static()) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case MemberInfo::ENUM:
								case MemberInfo::ENUM_VALUE:
									throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
								case MemberInfo::CLASS:
									THROW_BUG("can't be");
							}
						} break;
					}
				}
			}

		} break;

			// get_func()(); arr[0](); (a + b)(); base().method(); [o1, o2][1].method(); (x + y).method();
		case Parser::Node::Type::CALL:
		case Parser::Node::Type::INDEX:
		case Parser::Node::Type::MAPPED_INDEX:
		case Parser::Node::Type::OPERATOR:
			ASSERT(call->method == nullptr || call->method->type == Parser::Node::Type::IDENTIFIER);
			break;

		case Parser::Node::Type::ARRAY: // TODO: the method could be validated.
		case Parser::Node::Type::MAP:   // TODO: the method could be validated.
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			break;


			// idf.method(); method call on base with identifier id.
		case Parser::Node::Type::IDENTIFIER: {
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			Parser::IdentifierNode* base = ptrcast<Parser::IdentifierNode>(call->base).get();
			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();

			switch (base->ref) {

				// IF IDENTIFIER IS UNKNOWN IT'S ALREADY A NAME ERROR BY NOW.
				case Parser::IdentifierNode::REF_UNKNOWN: THROW_BUG("can't be");

					// p_param.method(); a_var.method(); a_member.method();
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_STATIC_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
				} break;

					// IF AN IDENTIFIER IS REFERENCE TO A CONSTANT IT'LL BE `ConstValueNode` BY NOW.
				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					THROW_BUG("can't be.");
				} break;

					// Aclass.id();
				case Parser::IdentifierNode::REF_CARBON_CLASS: {

					Parser::IdentifierNode _id = _find_member(base->_class, id->name);
					_id.pos = id->pos; id = &_id;
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s", id->name.c_str(), base->_class->name.c_str()), id->pos);

							// Aclass.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							bool _is_member_static = false;
							switch (id->ref_base) {
								case Parser::IdentifierNode::BASE_UNKNOWN: ASSERT(false && "I must forgotten here");
								case Parser::IdentifierNode::BASE_LOCAL:
									_is_member_static = id->_var->is_static;
									break;
								case Parser::IdentifierNode::BASE_NATIVE:
								case Parser::IdentifierNode::BASE_EXTERN:
									_is_member_static = id->_prop_info->is_static();
									break;
							}

							if (_is_member_static) {
								break; // Class.var(args...);
							} else {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

							// Aclass.CONST(args...);
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;

							// Aclass.Enum();
						case Parser::IdentifierNode::REF_ENUM_NAME: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case Parser::IdentifierNode::REF_ENUM_VALUE: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);

						case Parser::IdentifierNode::REF_FUNCTION: {

							bool _is_func_static = false;
							switch (id->ref_base) {
								case Parser::IdentifierNode::BASE_UNKNOWN: ASSERT(false && "I must forgotten here");
								case Parser::IdentifierNode::BASE_LOCAL:
									_is_func_static = _id._func->is_static;
									break;
								case Parser::IdentifierNode::BASE_NATIVE:
								case Parser::IdentifierNode::BASE_EXTERN:
									_is_func_static = id->_method_info->is_static();
									break;
							}

							if (_is_func_static) {
								break; // Class.static_func(args...);
							} else {
								throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't call non-static method\"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

							// Aclass.Lib();
						case Parser::IdentifierNode::REF_EXTERN: {
							throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("external libraries (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;
					}
				} break;

					// File.method(); base is a native class.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {

					BindData* bd = NativeClasses::singleton()->find_bind_data(base->name, id->name).get();
					if (!bd) throw ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on base %s.", id->name.c_str(), base->name.c_str()), id->pos);
					switch (bd->get_type()) {
						case BindData::STATIC_FUNC: {

							const MemberInfo* memi = bd->get_member_info().get();
							if (memi->get_type() != MemberInfo::METHOD) THROW_BUG("native member reference mismatch.");
							const MethodInfo* mi = (const MethodInfo*)memi;
							if (!mi->is_static()) THROW_BUG("native method reference mismatch.");

							int argc_given = (int)call->args.size();
							int argc = mi->get_arg_count(), argc_default = mi->get_default_arg_count();
							if (argc_given + argc_default < argc) {
								if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
							} else if (argc_given > argc) {
								if (argc_default == 0) throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else throw ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
							}

							for (int i = 0; i < (int)call->args.size(); i++) {
								if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
									if (mi->get_arg_types()[i].type != ptrcast<Parser::ConstValueNode>(call->args[i])->value.get_type()) {
										throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(mi->get_arg_types()[i].type), i), call->args[i]->pos);
									}
								}
							}

							// TODO: check if the static function is ok to call at compile time
							//       ex: read a file at a location, print something... are runtime.
							//if (all_const) {
							//	try {
							//		GET_ARGS(call->r_args);
							//		var ret = ptrcast<StaticFuncBind>(bd)->call(args);
							//		SET_EXPR_CONST_NODE(ret, call->pos);
							//	} catch (Error& err) {
							//		throw ANALYZER_ERROR(err.get_type(), err.what(), call->pos);
							//	}
							//}

						} break;
						case BindData::STATIC_VAR: break; // calls the "__call" at runtime.
						case BindData::STATIC_CONST: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value \"%s.%s()\" is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::METHOD: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static method \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::MEMBER_VAR:  throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static member \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM:  throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM_VALUE: throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
					}

				} break;

					// fn.get_default_args(), fn.get_name(), ...
				case Parser::IdentifierNode::REF_FUNCTION: {
					// TODO: check if method exists and args.
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check if function exists and check args.
				} break;

				case Parser::IdentifierNode::REF_ENUM_NAME: {
					// TODO: check if method exists
				} break;

					// TODO: EnumType.get_value_count();
					//case Parser::IdentifierNode::REF_ENUM_VALUE:
				default: {
					throw ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" doesn't support method calls.", base->name.c_str()), base->pos);
				}
			}

		} break;

		default: {
			THROW_BUG("can't reach here.");
		}

	}
#undef SET_EXPR_CONST_NODE
#undef GET_ARGS
}

} // namespace carbon
