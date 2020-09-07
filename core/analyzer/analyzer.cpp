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

// TODO: class variable name can't shadow from base.

namespace carbon {

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
		_resolve_constant(file_node->constants[i].get());
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->constants.size(); j++) {
				_resolve_constant(file_node->classes[i]->constants[j].get());
		}
	}
	parser->parser_context.current_class = nullptr;

	// File/class enums/unnamed_enums.
	for (size_t i = 0; i < file_node->enums.size(); i++) {
		parser->parser_context.current_enum = file_node->enums[i].get();
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->enums[i]->values) {
			_resolve_enumvalue(file_node->enums[i]->values[pair.first]);
		}
	}
	if (file_node->unnamed_enum != nullptr) {
		parser->parser_context.current_enum = file_node->unnamed_enum.get();
		for (std::pair<String, Parser::EnumValueNode> pair : file_node->unnamed_enum->values) {
			_resolve_enumvalue(file_node->unnamed_enum->values[pair.first]);
		}
	}
	parser->parser_context.current_enum = nullptr;

	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->enums.size(); j++) {
			parser->parser_context.current_enum = file_node->classes[i]->enums[j].get();
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->enums[j]->values) {
				_resolve_enumvalue(file_node->classes[i]->enums[j]->values[pair.first]);
			}
		}
		if (file_node->classes[i]->unnamed_enum != nullptr) {
			parser->parser_context.current_enum = file_node->classes[i]->unnamed_enum.get();
			for (std::pair<String, Parser::EnumValueNode> pair : file_node->classes[i]->unnamed_enum->values) {
				_resolve_enumvalue(file_node->classes[i]->unnamed_enum->values[pair.first]);
			}
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_enum = nullptr;

	// call compile time functions.
	_resolve_compiletime_funcs(file_node->compiletime_functions);
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		_resolve_compiletime_funcs(file_node->classes[i]->compiletime_functions);
	}

	// File/class level variables.
	for (size_t i = 0; i < file_node->vars.size(); i++) {
		if (file_node->vars[i]->assignment != nullptr) {
			_reduce_expression(file_node->vars[i]->assignment);
		}
	}
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->vars.size(); j++) {
			if (file_node->classes[i]->vars[j]->assignment != nullptr) {
				_reduce_expression(file_node->classes[i]->vars[j]->assignment);
			}
		}
	}
	parser->parser_context.current_class = nullptr;

	// File level function.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		parser->parser_context.current_func = file_node->functions[i].get();
		_resolve_parameters(file_node->functions[i].get());
		_reduce_block(file_node->functions[i]->body);
	}
	parser->parser_context.current_func = nullptr;

	// Inner class function.
	for (size_t i = 0; i < file_node->classes.size(); i++) {
		parser->parser_context.current_class = file_node->classes[i].get();
		for (size_t j = 0; j < file_node->classes[i]->functions.size(); j++) {
			parser->parser_context.current_func = file_node->classes[i]->functions[j].get();
			_reduce_block(file_node->classes[i]->functions[j]->body);
		}
	}
	parser->parser_context.current_class = nullptr;
	parser->parser_context.current_func = nullptr;
}

Parser::IdentifierNode Analyzer::_get_member(const Parser::ClassNode* p_class, const String& p_name) {

	Parser::IdentifierNode id; id.name = p_name;
	if (!p_class) return id;

	for (int i = 0; i < (int)p_class->vars.size(); i++) {
		if (p_class->vars[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_MEMBER_VAR;
			id._var = p_class->vars[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_class->functions.size(); i++) {
		if (p_class->functions[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_CARBON_FUNCTION;
			id._func = p_class->functions[i].get();
			return id;
		}
	}
	for (int i = 0; i < (int)p_class->constants.size(); i++) {
		if (p_class->constants[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_MEMBER_CONST;
			_resolve_constant(p_class->constants[i].get());
			id._const = p_class->constants[i].get();
			return id;
		}
	}
	if (p_class->unnamed_enum != nullptr) {
		for (std::pair<String, Parser::EnumValueNode> pair : p_class->unnamed_enum->values) {
			if (pair.first == id.name) {
				id.ref = Parser::IdentifierNode::REF_ENUM_VALUE;
				_resolve_enumvalue(p_class->unnamed_enum->values[pair.first]);
				id.enum_value = &p_class->unnamed_enum->values[pair.first];
				return id;
			}
		}
	}
	for (int i = 0; i < (int)p_class->enums.size(); i++) {
		if (p_class->enums[i]->name == id.name) {
			id.ref = Parser::IdentifierNode::REF_ENUM_NAME;
			id.enum_node = p_class->enums[i].get();
			return id;
		}
	}
	// TODO: REF_FILE for import.

	ASSERT(id.ref == Parser::IdentifierNode::REF_UNKNOWN);
	switch (p_class->base_type) {
		case Parser::ClassNode::BASE_LOCAL:
			return _get_member(p_class->base_local, p_name);
		case Parser::ClassNode::BASE_EXTERN:
			ASSERT(false && "TODO:");
	}
	return id;
}

var Analyzer::_call_compiletime_func(Parser::BuiltinFunctionNode* p_func, stdvec<var>& args) {
	switch (p_func->func) {
		case BuiltinFunctions::__ASSERT: {
			if (args.size() != 1) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "expected exactly 1 argument.", p_func->pos);
			if (!args[0].operator bool()) THROW_ANALYZER_ERROR(Error::ASSERTION, "assertion failed.", p_func->pos);
		} break;
		case BuiltinFunctions::__FUNC: {
			if (!parser->parser_context.current_func) THROW_ANALYZER_ERROR(Error::SYNTAX_ERROR, "__func() must be called inside a function.", p_func->pos);
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

void Analyzer::_resolve_compiletime_funcs(const stdvec<ptr<Parser::OperatorNode>>& p_funcs) {
	for (int i = 0; i < (int)p_funcs.size(); i++) {
		ASSERT(p_funcs[i]->op_type == Parser::OperatorNode::OP_CALL);
		ptr<Parser::OperatorNode> op = p_funcs[i];
		ASSERT(op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION);
		Parser::BuiltinFunctionNode* bf = ptrcast<Parser::BuiltinFunctionNode>(op->args[0]).get();
		stdvec<var> args;
		for (int j = 1; j < (int)op->args.size(); j++) {
			_reduce_expression(op->args[j]);
			if (op->args[j]->type != Parser::Node::Type::CONST_VALUE) {
				THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("compiletime function arguments must be compile time known values."), p_funcs[i]->args[j]->pos);
			}
			args.push_back(ptrcast<Parser::ConstValueNode>(op->args[j])->value);
		}
		_call_compiletime_func(bf, args);
	}
}

void Analyzer::_resolve_inheritance(Parser::ClassNode* p_class) {

	if (p_class->is_reduced) return;
	p_class->is_reduced = true;

	switch (p_class->base_type) {
		case Parser::ClassNode::NO_BASE:
			break;
		case Parser::ClassNode::BASE_LOCAL:
			for (int i = 0; i < (int)file_node->classes.size(); i++) {
				if (p_class->base_class == file_node->classes[i]->name) {
					_resolve_inheritance(file_node->classes[i].get());
					p_class->base_local = file_node->classes[i].get();

					Parser::ClassNode* base = p_class->base_local;
					while (base) {
						if (base == p_class)
							THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "cyclic inheritance. class inherits itself isn't allowed.", p_class->pos);
						base = base->base_local;
					}
					break;
				}
			}
			break;
		case Parser::ClassNode::BASE_EXTERN:
			DEBUG_PRINT("TODO"); // TODO: load ptr<CarbonByteCode> from path
			break;
	}
}

void Analyzer::_resolve_constant(Parser::ConstNode* p_const) {
	if (p_const->is_reduced) return;
	p_const->is_reduced = true;

	ASSERT(p_const->assignment != nullptr);
	_reduce_expression(p_const->assignment);
	if (p_const->assignment->type != Parser::Node::Type::CONST_VALUE)
		THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_const->assignment->pos);
	ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_const->assignment);
	if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
		cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING) {
		THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_const->assignment->pos);
	}
	p_const->value = cv->value;
}

void Analyzer::_resolve_parameters(Parser::FunctionNode* p_func) {
	for (int i = 0; i < p_func->args.size(); i++) {
		if (p_func->args[i].default_value != nullptr) {
			_reduce_expression(p_func->args[i].default_value);
			if (p_func->args[i].default_value->type != Parser::Node::Type::CONST_VALUE) 
				THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_func->args[i].default_value->pos);
			ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_func->args[i].default_value);
			if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
				cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING) {
				THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_func->args[i].default_value->pos);
			}
			p_func->default_parameters.push_back(cv->value);
		}
	}
}

void Analyzer::_resolve_enumvalue(Parser::EnumValueNode& p_enumvalue) {
	if (p_enumvalue.is_reduced) return;
	p_enumvalue.is_reduced = true;

	_reduce_expression(p_enumvalue.expr);
	if (p_enumvalue.expr->type != Parser::Node::Type::CONST_VALUE)
		THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
	ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_enumvalue.expr);
	if (cv->value.get_type() != var::INT) THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", p_enumvalue.expr->pos);
	p_enumvalue.value = cv->value;
}

void Analyzer::_reduce_block(ptr<Parser::BlockNode> p_block) {
	
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

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
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
				// TODO: just an identifier -> should add a warning and remove the statement.
				p_block->statements.erase(p_block->statements.begin() + i--);
			} break;

			case Parser::Node::Type::VAR: {
				ptr<Parser::VarNode> var_node = ptrcast<Parser::VarNode>(p_block->statements[i]);
				if (var_node->assignment != nullptr) {
					_reduce_expression(var_node->assignment);
				}
			} break;

			case Parser::Node::Type::CONST: {
				// TODO: this could be removed after all others are resolved.
				ptr<Parser::ConstNode> const_node = ptrcast<Parser::ConstNode>(p_block->statements[i]);
				_resolve_constant(const_node.get());
			} break;

			case Parser::Node::Type::CONST_VALUE:
			case Parser::Node::Type::ARRAY:
			case Parser::Node::Type::MAP:
				// TODO: add warning here.
				p_block->statements.erase(p_block->statements.begin() + i--);
			break;

			case Parser::Node::Type::THIS:
			case Parser::Node::Type::SUPER:
				// TODO: add warning here.
				break;

			case Parser::Node::Type::BUILTIN_FUNCTION:
			case Parser::Node::Type::BUILTIN_TYPE:
				// TODO: add warning here. `func fn() { String; }`
				p_block->statements.erase(p_block->statements.begin() + i--);
				break;

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
						}
					} break;

					case Parser::ControlFlowNode::SWITCH: {
						ASSERT(cf_node->args.size() == 1);
						// TODO: if it's evaluvated to compile time constant integer it could be optimized/warned.
						_reduce_expression(cf_node->args[0]);

						for (int j = 0; j < (int)cf_node->switch_cases.size(); j++) {
							_reduce_expression(cf_node->switch_cases[j].expr);
							if (cf_node->switch_cases[j].expr->type != Parser::Node::Type::CONST_VALUE)
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "switch case value must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(cf_node->switch_cases[j].expr);
							if (cv->value.get_type() != var::INT)
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "switch case must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							cf_node->switch_cases[j].value = cv->value;

							for (int _j = 0; _j < j; _j++) {
								if (cf_node->switch_cases[_j].value == cf_node->switch_cases[j].value) {
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("case value %lli has already defined at line %lli.",
										cf_node->switch_cases[j].value, cf_node->switch_cases[_j].pos.y), cf_node->switch_cases[j].pos);
								}
							}

							_reduce_block(cf_node->switch_cases[j].body);
						}

					} break;

					case Parser::ControlFlowNode::WHILE: {
						ASSERT(cf_node->args.size() == 1);
						_reduce_expression(cf_node->args[0]);
						if (cf_node->args[0]->type == Parser::Node::Type::CONST_VALUE) {
							if (ptrcast<Parser::ConstValueNode>(cf_node->args[0])->value.operator bool()) {
								if (!cf_node->has_break) { /* TODO: add warning -> infinite loop */ }
							} else {
								// TODO: add warning
								p_block->statements.erase(p_block->statements.begin() + i--);
							}
						}
						_reduce_block(cf_node->body);
					} break;

					case Parser::ControlFlowNode::FOR: {
						ASSERT(cf_node->args.size() == 3);
						ASSERT(cf_node->body->statements.size() >= 3); // arguments are in the body.
						_reduce_block(cf_node->body);
						// TODO: if it's evaluvated to compile time constant it could be optimized/warned.
						if (cf_node->args[0] == nullptr && cf_node->args[1] == nullptr && cf_node->args[2] == nullptr) {
							if (!cf_node->has_break) { /* TODO: add warning -> infinit loop */ }
						}
					} break;

					case Parser::ControlFlowNode::BREAK:
					case Parser::ControlFlowNode::CONTINUE: {
						ASSERT(cf_node->args.size() == 0);
					} break;
					case Parser::ControlFlowNode::RETURN: {
						ASSERT(cf_node->args.size() <= 1);
					} break;
				}
			} break;
		} // statement switch ends.

	}

	for (int i = 0; i < (int)p_block->statements.size(); i++) {
		// remove all local constant statments. no need anymore.
		if (p_block->statements[i]->type == Parser::Node::Type::CONST)
			p_block->statements.erase(p_block->statements.begin() + i--);

		// remove all compile time functions.
		else if (p_block->statements[i]->type == Parser::Node::Type::OPERATOR) {
			Parser::OperatorNode* op = ptrcast<Parser::OperatorNode>(p_block->statements[i]).get();
			if (op->op_type == Parser::OperatorNode::OP_CALL && op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION) {
				if (BuiltinFunctions::is_compiletime(ptrcast<Parser::BuiltinFunctionNode>(op->args[0])->func)) {
					p_block->statements.erase(p_block->statements.begin() + i--);
				}
			}
		}
	}
}

}