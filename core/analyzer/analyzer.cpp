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

#define THROW_ANALYZER_ERROR(m_err_type, m_msg, m_pos)                                                                      \
	do {																													\
		uint32_t err_len = 1;																								\
		String token_str = "";																								\
		if (m_pos.x > 0 && m_pos.y > 0) token_str = parser->tokenizer->get_token_at(m_pos).to_string();						\
		else token_str = parser->tokenizer->peek(-1, true).to_string();														\
		if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;				\
		else err_len = (uint32_t)token_str.size();																			\
																															\
		if (m_pos.x > 0 && m_pos.y > 0) {																					\
			String line = file_node->source.get_line(m_pos.x);																\
			throw Error(m_err_type, m_msg, file_node->path, line, m_pos, err_len)_ERR_ADD_DBG_VARS;							\
		} else {																											\
			String line = file_node->source.get_line(parser->tokenizer->get_pos().x);										\
			throw Error(m_err_type, m_msg, file_node->path, line, parser->tokenizer->peek(-1, true).get_pos(), err_len)		\
				_ERR_ADD_DBG_VARS;																							\
		}																													\
	} while (false)


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

	// File level function body.
	for (size_t i = 0; i < file_node->functions.size(); i++) {
		parser->parser_context.current_func = file_node->functions[i].get();
		_reduce_block(file_node->functions[i]->body);
	}
	parser->parser_context.current_func = nullptr;

	// Inner class function body.
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
	if (p_const->assignment->type != Parser::Node::Type::CONST_VALUE) {
		THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a contant expression.", p_const->assignment->pos);
	}
	ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(p_const->assignment);
	if (cv->value.get_type() != var::INT && cv->value.get_type() != var::FLOAT &&
		cv->value.get_type() != var::BOOL && cv->value.get_type() != var::STRING) {
		THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "expected a constant expression.", p_const->assignment->pos);
	}
	p_const->value = cv->value;
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
			ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(p_expr);
			do { // do ... while() loop is for jump out from the middle.
				if (parser->parser_context.current_func) {
					for (int i = 0; i < (int)parser->parser_context.current_func->args.size(); i++) {
						if (parser->parser_context.current_func->args[i].name == id->name) {
							id->ref = Parser::IdentifierNode::REF_PARAMETER;
							id->param_index = i;
							break;
						}
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				Parser::BlockNode* outer_block = parser->parser_context.current_block;
				while (outer_block && id->ref != Parser::IdentifierNode::REF_UNKNOWN) {
					for (int i = 0; i < (int)outer_block->local_vars.size(); i++) {
						if (outer_block->local_vars[i]->name == id->name) {
							id->ref = Parser::IdentifierNode::REF_LOCAL_VAR;
							id->_var = outer_block->local_vars[i].get();
							break;
						}
					}
					if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

					for (int i = 0; i < (int)outer_block->local_const.size(); i++) {
						if (outer_block->local_const[i]->name == id->name) {
							id->ref = Parser::IdentifierNode::REF_LOCAL_CONST;
							_resolve_constant(outer_block->local_const[i].get());
							id->_const = outer_block->local_const[i].get();
							break;
						}
					}
					if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

					if (outer_block->parernt_node->type == Parser::Node::Type::BLOCK) {
						outer_block = ptrcast<Parser::BlockNode>(outer_block->parernt_node).get();
					} else {
						outer_block = nullptr;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				if (parser->parser_context.current_class) { // TODO: abstract this and search in parent classes too.
					for (int i = 0; i < (int)parser->parser_context.current_class->vars.size(); i++) {
						if (parser->parser_context.current_class->vars[i]->name == id->name) {
							id->ref = Parser::IdentifierNode::REF_MEMBER_VAR;
							id->_var = parser->parser_context.current_class->vars[i].get();
							break;
						}
					}
					for (int i = 0; i < (int)parser->parser_context.current_class->constants.size(); i++) {
						if (parser->parser_context.current_class->constants[i]->name == id->name) {
							id->ref = Parser::IdentifierNode::REF_MEMBER_CONST;
							_resolve_constant(parser->parser_context.current_class->constants[i].get());
							id->_const = parser->parser_context.current_class->constants[i].get();
							break;
						}
					}
					if (parser->parser_context.current_class->unnamed_enum != nullptr) {
						for (std::pair<String, Parser::EnumValueNode> pair : parser->parser_context.current_class->unnamed_enum->values) {
							if (pair.first == id->name) {
								id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
								_resolve_enumvalue(parser->parser_context.current_class->unnamed_enum->values[pair.first]);
								id->enum_value = &parser->parser_context.current_class->unnamed_enum->values[pair.first];
								break;
							}
						}
						if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;
					}
					for (int i = 0; i < (int)parser->parser_context.current_class->enums.size(); i++) {
						if (parser->parser_context.current_class->enums[i]->name == id->name) {
							id->ref = Parser::IdentifierNode::REF_ENUM_NAME;
							id->enum_node = parser->parser_context.current_class->enums[i].get();
							break;
						}
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				for (int i = 0; i < (int)file_node->vars.size(); i++) {
					if (file_node->vars[i]->name == id->name) {
						id->ref = Parser::IdentifierNode::REF_MEMBER_VAR;
						id->_var = file_node->vars[i].get();
						break;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				for (int i = 0; i < (int)file_node->constants.size(); i++) {
					if (file_node->constants[i]->name == id->name) {
						id->ref = Parser::IdentifierNode::REF_MEMBER_CONST;
						_resolve_constant(file_node->constants[i].get());
						id->_const = file_node->constants[i].get();
						break;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				if (parser->parser_context.current_enum != nullptr) {
					for (std::pair<String, Parser::EnumValueNode> pair : parser->parser_context.current_enum->values) {
						if (pair.first == id->name) {
							id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							_resolve_enumvalue(parser->parser_context.current_enum->values[pair.first]);
							id->enum_value = &pair.second;
							break;
						}
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				if (file_node->unnamed_enum != nullptr) {
					for (std::pair<String, Parser::EnumValueNode> pair : file_node->unnamed_enum->values) {
						if (pair.first == id->name) {
							id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
							_resolve_enumvalue(file_node->unnamed_enum->values[pair.first]);
							id->enum_value = &pair.second;
							break;
						}
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				for (int i = 0; i < (int)file_node->enums.size(); i++) {
					if (file_node->enums[i]->name == id->name) {
						id->ref = Parser::IdentifierNode::REF_ENUM_NAME;
						id->enum_node = file_node->enums[i].get();
						break;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				for (int i = 0; i < (int)file_node->classes.size(); i++) {
					if (file_node->classes[i]->name == id->name) {
						id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
						id->_class = file_node->classes[i].get();
						break;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				for (int i = 0; i < (int)file_node->functions.size(); i++) {
					if (file_node->functions[i]->name == id->name) {
						id->ref = Parser::IdentifierNode::REF_CARBON_FUNCTION;
						id->_func = file_node->functions[i].get();
						break;
					}
				}
				if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

				if (NativeClasses::is_class_registered(id->name)) {
					id->ref = Parser::IdentifierNode::REF_NATIVE_CLASS;
					break;
				}

				// TODO: REF_FILE for import.
			} while (false);

			switch (id->ref) {
				case Parser::IdentifierNode::REF_UNKNOWN:
					THROW_ANALYZER_ERROR(Error::NAME_ERROR, String::format("identifier \"%s\" isn't defined.", id->name.c_str()), id->pos);
				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->_const->value);
					cv->pos = id->pos; p_expr = cv;
				} break;
				case Parser::IdentifierNode::REF_ENUM_VALUE: {
					ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(id->enum_value->value);
					cv->pos = id->pos; p_expr = cv;
				} break;
				default: { // variable, parameter, function name, ...
					p_expr = id;
					break;
				}
			}
		} break;

		case Parser::Node::Type::ARRAY: {
			ptr<Parser::ArrayNode> arr = ptrcast<Parser::ArrayNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
				if (arr->elements[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			if (all_const) {
				Array arr_value;
				for (int i = 0; i < (int)arr->elements.size(); i++) {
					arr_value.push_back(ptrcast<Parser::ConstValueNode>(arr->elements[i])->value);
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(arr_value);
				cv->pos = p_expr->pos; p_expr = cv;
			}
		} break;

		case Parser::Node::Type::MAP: {
			ptr<Parser::MapNode> map = ptrcast<Parser::MapNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: if key is const value and two keys are the same throw error.
				if (map->elements[i].key->type == Parser::Node::Type::CONST_VALUE) {
					var& key_v = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;

					switch (key_v.get_type()) { // TODO: implement is_hashable() in var. what if null as key?
						case var::BOOL:
						case var::INT:
						case var::FLOAT:
						case var::STRING:
						case var::VECT2F:
						case var::VECT2I:
						case var::VECT3F:
						case var::VECT3I:
							break;
						default: //_NULL, ARRAY, MAP, OBJECT:
							THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("unhasnable type %s used as map key.", key_v.get_type_name().c_str()), map->pos);
					}
				}
				_reduce_expression(map->elements[i].value);

				if (map->elements[i].key->type != Parser::Node::Type::CONST_VALUE || map->elements[i].value->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			if (all_const) {
				Map map_value;
				for (int i = 0; i < (int)map->elements.size(); i++) {
					var& _key = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					var& _val = ptrcast<Parser::ConstValueNode>(map->elements[i].value)->value;
					map_value[_key] = _val;
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(map_value);
				cv->pos = p_expr->pos; p_expr = cv;
			}
		} break;

		case Parser::Node::Type::OPERATOR: {
			ptr<Parser::OperatorNode> op = ptrcast<Parser::OperatorNode>(p_expr);

			// TODO: instead of checking operator type and casting everywhere, implmemnt IndexingNode, CallNode, ...

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				if (i == 0 && (op->op_type == Parser::OperatorNode::OP_CALL) && (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION || op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE)) {
					// _don't_reduce_expression__and_could_be_all_const();
					continue;
				} else {
					// don't_reduce();
					if (i != 0 && op->op_type == Parser::OperatorNode::OP_INDEX) continue;  // don't_reduce_attribute_on_base();
					if (op->op_type == Parser::OperatorNode::OP_CALL) {                     // don't_reduce_method_name_on_base();
						if (i == 0 && op->args[0]->type == Parser::Node::Type::UNKNOWN) continue; // don't_reduce_unknown_base(); could be this or super.
						if (i == 1 && op->args[0]->type != Parser::Node::Type::BUILTIN_FUNCTION && op->args[0]->type != Parser::Node::Type::BUILTIN_TYPE) {
							// args[0] : call on identifier, const_value, ... if UNKNOWN compiler is not sure if it's called on self or static func.
							// args[1] : method name -> ignore here and reduce on base.
							// args[2...] : method arguments.
							continue;
						}
					}
					
					_reduce_expression(op->args[i]);
				}
				if (op->args[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}

#define GET_ARGS(m_initial_arg)														  \
	stdvec<var> args;																  \
	for (int i = m_initial_arg; i < (int)op->args.size(); i++) {					  \
		args.push_back(ptrcast<Parser::ConstValueNode>(op->args[i])->value);		  \
	}

#define SET_EXPR_CONST_NODE(m_var)                                                    \
do {                                                                                  \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);         \
	cv->pos = op->pos;                                                                \
	p_expr = cv;                                                                      \
} while (false)

			switch (op->op_type) {
				case Parser::OperatorNode::OpType::OP_CALL: {

					// reduce builtin function
					if (op->args[0]->type == Parser::Node::Type::BUILTIN_FUNCTION) {
						if (all_const) {
							ptr<Parser::BuiltinFunctionNode> bf = ptrcast<Parser::BuiltinFunctionNode>(op->args[0]);
							if (BuiltinFunctions::can_const_fold(bf->func)) {
								GET_ARGS(1);
								if (BuiltinFunctions::is_compiletime(bf->func)) {
									var ret = _call_compiletime_func(bf.get(), args);
									SET_EXPR_CONST_NODE(ret);
								} else {
									try {
										var ret;
										BuiltinFunctions::call(bf->func, args, ret);
										SET_EXPR_CONST_NODE(ret);
									} catch (Error& err) {
										throw err
											.set_file(file_node->path)
											.set_line(file_node->source.get_line(op->pos.x))
											.set_pos(op->pos)
											.set_err_len((uint32_t)String(BuiltinFunctions::get_func_name(bf->func)).size())
											;
									}
								}
							}
						}

					// reduce builtin type construction
					} else if (op->args[0]->type == Parser::Node::Type::BUILTIN_TYPE) {
						if (all_const) {
							ptr<Parser::BuiltinTypeNode> bt = ptrcast<Parser::BuiltinTypeNode>(op->args[0]);
							try {
								GET_ARGS(1);
								var ret = BuiltinTypes::construct(bt->builtin_type, args);
								SET_EXPR_CONST_NODE(ret);
							} catch (Error& err) {
								throw err
									.set_file(file_node->path)
									.set_line(file_node->source.get_line(op->pos.x))
									.set_pos(op->pos)
									.set_err_len((uint32_t)String(BuiltinTypes::get_type_name(bt->builtin_type)).size())
									;
							}
						}

					// base const value.
					} else if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {
						if (all_const) {
							try {
								ASSERT(op->args.size() >= 2);
								ASSERT(op->args[1]->type == Parser::Node::Type::IDENTIFIER);
								GET_ARGS(2); // 0 : const value, 1: name, ... args.
								var ret = ptrcast<Parser::ConstValueNode>(op->args[0])->value.call_method(ptrcast<Parser::IdentifierNode>(op->args[1])->name, args);
								SET_EXPR_CONST_NODE(ret);
							} catch (.../*VarError& err*/) {
								ASSERT(false && "TODO: catch and throw var error as cb error");
							}
						}

					// call on this, super, super.super ...
					} else if (op->args[0]->type == Parser::Node::Type::UNKNOWN) {
						ASSERT(op->args.size() >= 2);
						_reduce_expression(op->args[1]);
						ASSERT(op->args[1]->type == Parser::Node::Type::IDENTIFIER);
						Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(op->args[1]).get();
						switch (id->ref) {

							// call `__call` method on the variable.
							case Parser::IdentifierNode::REF_PARAMETER:
							case Parser::IdentifierNode::REF_LOCAL_VAR:
							case Parser::IdentifierNode::REF_MEMBER_VAR: {
								ptr<Parser::IdentifierNode> __call = newptr<Parser::IdentifierNode>("__call");
								__call->pos = id->pos;
								op->args[0] = op->args[1];
								op->args[1] = __call;
							} break;

							// check arguments.
							case Parser::IdentifierNode::REF_CARBON_FUNCTION: {
								// TODO: implement default arguments.
								if (op->args.size() - 2 != id->_func->args.size())
									THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", (int)id->_func->args.size()), id->pos);
							} break;

							// call constructor.
							case Parser::IdentifierNode::REF_CARBON_CLASS: {
								if (id->_class->constructor) {
									// TODO: default arguments.
									if (op->args.size() - 2 != id->_class->constructor->args.size()) {
										THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", (int)id->_class->constructor->args.size()), id->pos);
									}
								} else {
									if (op->args.size() - 2 != 0)
										THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
								}
							} break;

							case Parser::IdentifierNode::REF_NATIVE_CLASS: {
								ASSERT(NativeClasses::is_class_registered(id->name));
								const StaticFuncBind* initializer = NativeClasses::get_initializer(id->name);
								if (initializer) {
									// check arg counts.
									int argc = initializer->get_method_info()->get_arg_count() - 1; // -1 for self argument.
									int argc_default = initializer->get_method_info()->get_default_arg_count();
									int argc_given = (int) op->args.size() - 2;
									if (argc_given + argc_default < argc) {
										if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
										else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
									} else if (argc_given > argc) {
										if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
										else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
									}
									// check arg types.
									const stdvec<VarTypeInfo>& arg_types = initializer->get_method_info()->get_arg_types();
									for (int i = 2; i < op->args.size(); i++) {
										if (op->args[i]->type == Parser::Node::Type::CONST_VALUE) {
											var value = ptrcast<Parser::ConstValueNode>(op->args[i])->value;
											if (value.get_type() != arg_types[i - 2 + 1].type) // +1 for skip self argument.
												THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(arg_types[i - 2].type), i-2), op->args[i-2]->pos);
										}
									}
								} else {
									if (op->args.size() - 2 != 0)
										THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
								}
							} break;

							// invalid.
							//case Parser::IdentifierNode::REF_ENUM_NAME:
							//case Parser::IdentifierNode::REF_ENUM_VALUE:
							//case Parser::IdentifierNode::REF_FILE:
							//case Parser::IdentifierNode::REF_LOCAL_CONST:
							//case Parser::IdentifierNode::REF_MEMBER_CONST:
							default: {
								THROW_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" is not callable.", id->name.c_str()));
							}
						}

					} else {
						ASSERT(false && "TODO: resolve op->args[1] on base op->args[0].");

					}

				} break;

				case Parser::OperatorNode::OpType::OP_INDEX: {
					ASSERT(op->args.size() == 2);
					ASSERT(op->args[1]->type == Parser::Node::Type::IDENTIFIER);

					ptr<Parser::IdentifierNode> id = ptrcast<Parser::IdentifierNode>(op->args[1]);

					switch (op->args[0]->type) {
						case Parser::Node::Type::BUILTIN_TYPE: // used in: Vect2.ZERO;
							break;
						case Parser::Node::Type::CONST_VALUE: {
							Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(op->args[0]).get();
							try {
								ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.get_member(id->name));
								cv->pos = id->pos; p_expr = cv;
							} catch (VarError& err) {
								if (err.get_type() == VarError::NULL_POINTER) {
									THROW_ANALYZER_ERROR(Error::NULL_POINTER, "", op->pos);
								} else if (err.get_type() == VarError::ATTRIBUTE_ERROR) {
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("name \"%s\" isn't exists on base %s.", id->name.c_str(), base->value.get_type_name().c_str()), op->pos);
								} else {
									ASSERT(false);
								}
							}
						} break;
						case Parser::Node::Type::THIS:
						case Parser::Node::Type::SUPER:
						case Parser::Node::Type::IDENTIFIER: {

							Parser::IdentifierNode* base;
							ptr<Parser::IdentifierNode> _keep_alive;

							if (op->args[0]->type == Parser::Node::Type::THIS) {
								_keep_alive = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->name);
								_keep_alive->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
								_keep_alive->_class = parser->parser_context.current_class;
								base = _keep_alive.get();
							} else if (op->args[0]->type == Parser::Node::Type::SUPER) {
								if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_LOCAL) {
									_keep_alive = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_local->name);
									_keep_alive->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
									_keep_alive->_class = parser->parser_context.current_class->base_local;
									base = _keep_alive.get();
								} else if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_EXTERN) {
									ASSERT(false); // TODO:
								}
							} else {
								base = ptrcast<Parser::IdentifierNode>(op->args[0]).get();
							}

							switch (base->ref) {
								case Parser::IdentifierNode::REF_UNKNOWN: {
									THROW_BUG("base can't be unknown.");
								} break;

								case Parser::IdentifierNode::REF_PARAMETER:
								case Parser::IdentifierNode::REF_LOCAL_VAR:
									break; // Can't reduce anymore.
								case Parser::IdentifierNode::REF_LOCAL_CONST: {
								} break;
								case Parser::IdentifierNode::REF_MEMBER_VAR:
									break; // Can't reduce anymore.

								case Parser::IdentifierNode::REF_MEMBER_CONST: {
									try {
										base->_const->value.get_member(id->name);
									} catch (VarError& err) {
										THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, err.what(), id->pos);
									}
									ASSERT(false && "there isn't any contant value currently support attribute access and most probably in the future");
								} break;

								case Parser::IdentifierNode::REF_ENUM_NAME: {
									for (std::pair<String, Parser::EnumValueNode> pair : base->enum_node->values) {
										if (pair.first == id->name) {
											id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
											id->enum_value = &pair.second;
											_resolve_enumvalue(base->enum_node->values[pair.first]);
											ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->enum_node->values[pair.first].value);
											cv->pos = id->pos; p_expr = cv;
											break;
										}
									}
								} break;

								case Parser::IdentifierNode::REF_ENUM_VALUE:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enum value doesn't support attribute access.", id->pos);

								case Parser::IdentifierNode::REF_CARBON_CLASS: {
									for (int i = 0; i < (int)base->_class->constants.size(); i++) {
										if (base->_class->constants[i]->name == id->name) {
											id->ref = Parser::IdentifierNode::REF_MEMBER_CONST;
											id->_const = base->_class->constants[i].get();
											_resolve_constant(base->_class->constants[i].get());
											ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->_class->constants[i]->value);
											cv->pos = id->pos; p_expr = cv;
											break;
										}
									}
									if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

									for (int i = 0; i < (int)base->_class->enums.size(); i++) {
										if (base->_class->enums[i]->name == id->name) {
											id->ref = Parser::IdentifierNode::REF_ENUM_NAME;
											id->enum_node = base->_class->enums[i].get();
											id->name = base->name + "." + id->name; p_expr = id;
											break;
										}
									}
									if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

									if (base->_class->unnamed_enum != nullptr) {
										for (std::pair<String, Parser::EnumValueNode> pair : base->_class->unnamed_enum->values) {
											if (pair.first == id->name) {
												id->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
												id->enum_value = &pair.second;
												_resolve_enumvalue(base->_class->unnamed_enum->values[pair.first]);
												ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(pair.second.value);
												cv->pos = id->pos; p_expr = cv;
												break;
											}
										}
									}
									if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

									for (int i = 0; i < (int)base->_class->vars.size(); i++) {
										if (base->_class->vars[i]->name == id->name) {
											if (!base->_class->vars[i]->is_static) {
												THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static members can only be accessed from instances.").c_str(), id->pos);
											}
											id->ref = Parser::IdentifierNode::REF_MEMBER_VAR;
											id->_var = base->_class->vars[i].get();
											id->name = base->name + "." + id->name; p_expr = id;
											break;
										}
									}
									if (id->ref != Parser::IdentifierNode::REF_UNKNOWN) break;

									// TODO: also check in functions, static function for better error message.
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".",
										id->name.c_str(), base->name.c_str()), id->pos);

								} break;

								case Parser::IdentifierNode::REF_NATIVE_CLASS: {
									ASSERT(false && "TODO:");
								} break;

								case Parser::IdentifierNode::REF_CARBON_FUNCTION:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "function object doesn't support attribute access.", id->pos);

								case Parser::IdentifierNode::REF_FILE: { // TODO: change the name.
									ASSERT(false && "TODO:");
								} break;

								// TODO: REF binary version of everything above.
							}
						}
						default:
							break;
							// RUNTIME.
					}
				} break;

				case Parser::OperatorNode::OpType::OP_INDEX_MAPPED: {
					if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {

						Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(op->args[0]).get();
						ASSERT(base->value.get_type() != var::OBJECT); // Objects can't be const value.
						try {
							GET_ARGS(1);
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.__get_mapped(args[0]));
							cv->pos = base->pos; p_expr = cv;
						} catch (.../*VarError& err*/) {
							ASSERT(false && "TODO: var error to carbon error");
						}
					}
				} break;

			/***************** BINARY AND UNARY OPERATORS  *****************/

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
								break;
							case Parser::IdentifierNode::REF_LOCAL_CONST:
							case Parser::IdentifierNode::REF_MEMBER_CONST:
							case Parser::IdentifierNode::REF_ENUM_NAME:
							case Parser::IdentifierNode::REF_ENUM_VALUE:
							case Parser::IdentifierNode::REF_CARBON_CLASS:
							case Parser::IdentifierNode::REF_NATIVE_CLASS:
							case Parser::IdentifierNode::REF_CARBON_FUNCTION:
							case Parser::IdentifierNode::REF_FILE:
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "invalid assignment (only assignment on variables/parameters are valid).", op->args[0]->pos);
						}
					} else if (op->args[0]->type == Parser::Node::Type::THIS) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"this\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::SUPER) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"super\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to constant values.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::ARRAY) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to array literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::MAP) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to map literal.", op->args[0]->pos);
					}
				} break;
				default: { // Remaining binary/unary operators.
					if (!all_const) break;
					GET_ARGS(0);
					switch (op->op_type) {
						case Parser::OperatorNode::OpType::OP_EQEQ:
							SET_EXPR_CONST_NODE(args[0] == args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_PLUS:
							SET_EXPR_CONST_NODE(args[0] + args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_MINUS:
							SET_EXPR_CONST_NODE(args[0] - args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_MUL:
							SET_EXPR_CONST_NODE(args[0] * args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_DIV:
							SET_EXPR_CONST_NODE(args[0] / args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_MOD:
							SET_EXPR_CONST_NODE(args[0] % args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_LT:
							SET_EXPR_CONST_NODE(args[0] < args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_GT:
							SET_EXPR_CONST_NODE(args[0] > args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_AND:
							SET_EXPR_CONST_NODE(args[0].operator bool() && args[1].operator bool());
							break;
						case Parser::OperatorNode::OpType::OP_OR:
							SET_EXPR_CONST_NODE(args[0].operator bool() || args[1].operator bool());
							break;
						case Parser::OperatorNode::OpType::OP_NOTEQ:
							SET_EXPR_CONST_NODE(args[0] != args[1]);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_LSHIFT:
							SET_EXPR_CONST_NODE(args[0].operator int64_t() << args[1].operator int64_t());
							break;
						case Parser::OperatorNode::OpType::OP_BIT_RSHIFT:
							SET_EXPR_CONST_NODE(args[0].operator int64_t() >> args[1].operator int64_t());
							break;
						case Parser::OperatorNode::OpType::OP_BIT_OR:
							SET_EXPR_CONST_NODE(args[0].operator int64_t() | args[1].operator int64_t());
							break;
						case Parser::OperatorNode::OpType::OP_BIT_AND:
							SET_EXPR_CONST_NODE(args[0].operator int64_t() & args[1].operator int64_t());
							break;
						case Parser::OperatorNode::OpType::OP_BIT_XOR:
							SET_EXPR_CONST_NODE(args[0].operator int64_t() ^ args[1].operator int64_t());
							break;

						case Parser::OperatorNode::OpType::OP_NOT:
							SET_EXPR_CONST_NODE(!args[0].operator bool());
							break;
						case Parser::OperatorNode::OpType::OP_BIT_NOT:
							SET_EXPR_CONST_NODE(~args[0].operator int64_t());
							break;
						case Parser::OperatorNode::OpType::OP_POSITIVE:
							switch (args[0].get_type()) {
								case var::BOOL:
								case var::INT:
								case var::FLOAT: {
									SET_EXPR_CONST_NODE(args[0]);
								} break;
								default:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"+\" not supported on %s.", args[0].get_type_name()), op->pos);
							}
							break;
						case Parser::OperatorNode::OpType::OP_NEGATIVE:
							switch (args[0].get_type()) {
								case var::BOOL:
								case var::INT:
									SET_EXPR_CONST_NODE(-args[0].operator int64_t());
									break;
								case var::FLOAT:
									SET_EXPR_CONST_NODE(-args[0].operator double());
									break;
								default:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"+\" not supported on %s.", args[0].get_type_name()), op->pos);
							}
							break;
					}
				}
				MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 36);
			}
#undef SET_EXPR_CONST_NODE
		} break;
		case Parser::Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			ASSERT(false); // ???
		}
	}
}

void Analyzer::_reduce_block(ptr<Parser::BlockNode>& p_block, Parser::BlockNode* p_parent_block) {
	
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
	ScopeDestruct destruct = ScopeDestruct(&parser->parser_context, p_parent_block);

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
				THROW_BUG("invalid statement type in analyzer."); // TODO: check what throws : `func fn() { String; }`
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
						_reduce_block(cf_node->body, p_block.get());
						if (cf_node->body_else != nullptr) {
							_reduce_block(cf_node->body_else, p_block.get());
						}
					} break;

					case Parser::ControlFlowNode::SWITCH: {
						ASSERT(cf_node->args.size() == 1);
						// TODO: if it's evaluvated to compile time constant integer it could be optimized/warned.
						_reduce_expression(cf_node->args[0]);

						for (int j = 0; j < (int)cf_node->switch_cases.size(); j++) {
							_reduce_expression(cf_node->switch_cases[j].expr);
							if (cf_node->switch_cases[j].expr->type != Parser::Node::Type::CONST_VALUE)
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							ptr<Parser::ConstValueNode> cv = ptrcast<Parser::ConstValueNode>(cf_node->switch_cases[j].expr);
							if (cv->value.get_type() != var::INT)
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "enum value must be a constant integer.", cf_node->switch_cases[j].expr->pos);
							cf_node->switch_cases[j].value = cv->value;

							for (int _j = 0; _j < j; _j++) {
								if (cf_node->switch_cases[_j].value == cf_node->switch_cases[j].value) {
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("case value %lli has already defined at line %lli.",
										cf_node->switch_cases[j].value, cf_node->switch_cases[_j].pos.y), cf_node->switch_cases[j].pos);
								}
							}

							_reduce_block(cf_node->switch_cases[j].body, p_block.get());
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
						_reduce_block(cf_node->body, p_block.get());
					} break;

					case Parser::ControlFlowNode::FOR: {
						ASSERT(cf_node->args.size() == 3);
						// TODO: if it's evaluvated to compile time constant it could be optimized/warned.
						if (cf_node->args[0] != nullptr) _reduce_expression(cf_node->args[0]);
						if (cf_node->args[1] != nullptr) _reduce_expression(cf_node->args[1]);
						if (cf_node->args[2] != nullptr) _reduce_expression(cf_node->args[2]);

						if (cf_node->args[0] == nullptr && cf_node->args[1] == nullptr && cf_node->args[2] == nullptr) {
							if (!cf_node->has_break) { /* TODO: add warning -> infinit loop */ }
						}
						_reduce_block(cf_node->body, p_block.get());
					} break;

					case Parser::ControlFlowNode::BREAK:
					case Parser::ControlFlowNode::CONTINUE:
					case Parser::ControlFlowNode::RETURN: {
						ASSERT(cf_node->args.size() == 0);
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