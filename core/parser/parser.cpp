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

#include "parser.h"

#include "analyzer/analyzer.h"

#define THROW_PREDEFINED(m_what, m_name, m_pos)             \
	THROW_PARSER_ERR(Error::NAME_ERROR,                     \
	String::format(m_what " named \"%s\" already exists at (line:%i, col:%i)", m_name.c_str(), m_pos.x, m_pos.y), Vect2i())

#define THROW_IF_NAME_DEFINED(m_parent, m_what, m_identifier, m_members)                                 \
	for (int i = 0; i < (int)m_parent->m_members.size(); i++) {                                          \
		if (m_parent->m_members[i]->name == m_identifier) {                                              \
			THROW_PREDEFINED(m_what, m_parent->m_members[i]->name, m_parent->m_members[i]->pos);         \
		}										                                                         \
	}


#define THROW_IF_NAME_DEFINED_ENUMVALUES(m_parent)                                                               \
if (m_parent->unnamed_enum != nullptr) {																		 \
	for (auto it = m_parent->unnamed_enum->values.begin(); it != m_parent->unnamed_enum->values.end(); it++) {	 \
		if (it->first == tk->identifier) {																		 \
			THROW_PREDEFINED("an enum value", tk->identifier, it->second.pos);									 \
		}																										 \
	}																											 \
}

#define THROW_IF_NAME_NATIVE(m_name)                                                                                         \
	if (NativeClasses::singleton()->is_class_registered(m_name)) {                                                           \
		THROW_PARSER_ERR(Error::NAME_ERROR, String::format("a native type named \"%s\" already exists at (line:%i, col:%i)", \
			m_name.c_str(), tk->line, tk->col), Vect2i());                                                                   \
	}

namespace carbon {

void Parser::parse(String p_source, String p_file_path) {
	tokenizer = newptr<Tokenizer>();
	file_node = new_node<FileNode>();
	file_node->source = p_source;
	file_node->path = p_file_path;

	tokenizer->tokenize(file_node->source, file_node->path);

	while (true) {
	
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case  Token::_EOF:
				return;
			case Token::KWORD_IMPORT: {
				// TODO: file_node->imports.push_back(_parse_import());
				break;
			}
			case Token::KWORD_CLASS: {
				file_node->classes.push_back(_parse_class());
				break;
			}
			case Token::KWORD_ENUM: {
				ptr<EnumNode> _enum = _parse_enum(file_node);
				if (_enum->named_enum) {
					file_node->enums.push_back(_enum);
				} else {
					if (file_node->unnamed_enum == nullptr) {
						file_node->unnamed_enum = _enum;
					} else {
						for (auto it = _enum->values.begin(); it != _enum->values.end(); it++) {
							file_node->unnamed_enum->values[it->first] = it->second;
						}
					}
				}
			} break;
			case Token::KWORD_FUNC: {
				ptr<FunctionNode> func = _parse_func(file_node);
				file_node->functions.push_back(func);
				break;
			}
			case Token::KWORD_VAR: {
				stdvec<ptr<VarNode>> vars = _parse_var(file_node);
				for (ptr<VarNode>& _var : vars) {
					file_node->vars.push_back(_var);
				}
				break;
			}
			case Token::KWORD_CONST: {
				ptr<ConstNode> _const = _parse_const(file_node);
				file_node->constants.push_back(_const);
			} break;

			// Ignore.
			case Token::SYM_SEMI_COLLON: 
			case Token::VALUE_STRING:
				break;

			// compile time function call.
			case Token::IDENTIFIER: {
				ptr<CallNode> call = new_node<CallNode>();
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					if (tokenizer->next().type != Token::BRACKET_LPARAN) THROW_UNEXP_TOKEN("symbol \"(\"");
					call->r_args = _parse_arguments(file_node);
					file_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]
			default:
				THROW_UNEXP_TOKEN("");
		}

	} // while true
}

void Parser::_parse_import() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IMPORT);
	ptr<FileNode> import_file = new_node<FileNode>();

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");

	if (tokenizer->next().type != Token::OP_EQ) THROW_UNEXP_TOKEN("symbol \"=\"");
	tk = &tokenizer->next();
	if (tk->type != Token::VALUE_STRING) THROW_UNEXP_TOKEN("string path to source");
	// TODO:

	return;
}

ptr<Parser::ClassNode> Parser::_parse_class() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_CLASS);
	ptr<ClassNode> class_node = new_node<ClassNode>();
	class_node->parernt_node = file_node;

	parser_context.current_class = class_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_class = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
	
	// TODO: check identifier from import.
	THROW_IF_NAME_NATIVE(tk->identifier);
	THROW_IF_NAME_DEFINED(file_node, "a class", tk->identifier, classes);
	THROW_IF_NAME_DEFINED(file_node, "a variable", tk->identifier, vars);
	THROW_IF_NAME_DEFINED(file_node, "a constants", tk->identifier, constants);
	THROW_IF_NAME_DEFINED(file_node, "a function", tk->identifier, functions);
	THROW_IF_NAME_DEFINED(file_node, "an enum", tk->identifier, enums);
	THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);

	class_node->name = tk->identifier;

	// Inheritance.
	tk = &tokenizer->next();
	if (tk->type == Token::SYM_COLLON) {

		tk = &tokenizer->next();
		if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
		class_node->base_class = tk->identifier;

		tk = &tokenizer->next();
		if (tk->type == Token::SYM_DOT) {
			tk = &tokenizer->next();
			if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");

			class_node->base_file = class_node->base_class;
			class_node->base_class = tk->identifier;
			class_node->base_type = ClassNode::BASE_EXTERN;
			ASSERT(false); // TODO: inherits a binary base.

			tk = &tokenizer->next();
		} else {
			if (NativeClasses::singleton()->is_class_registered(class_node->base_class)) {
				class_node->base_type = ClassNode::BASE_NATIVE;
			} else {
				if (class_node->base_class == class_node->name) THROW_PARSER_ERR(Error::SYNTAX_ERROR, "", tokenizer->peek(-2, true).get_pos());
				class_node->base_type = ClassNode::BASE_LOCAL;
			}
		} // TODO: what if inherits a builtin type like Array, Map, String, ...
	}

	if (tk->type != Token::BRACKET_LCUR) THROW_UNEXP_TOKEN("symbol \"{\"");
	
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				THROW_PARSER_ERR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::BRACKET_RCUR: {
				return class_node;
			}

			case Token::SYM_SEMI_COLLON: { // ignore
			} break;

			case Token::KWORD_ENUM: {
				ptr<EnumNode> _enum = _parse_enum(class_node);
				if (_enum->named_enum) {
					class_node->enums.push_back(_enum);
				} else {
					if (class_node->unnamed_enum == nullptr) {
						class_node->unnamed_enum = _enum;
					} else {
						for (auto it = _enum->values.begin(); it != _enum->values.end(); it++) {
							class_node->unnamed_enum->values[it->first] = it->second;
						}
					}
				}
			} break;

			case Token::KWORD_STATIC: {
				if (tokenizer->peek().type != Token::KWORD_FUNC && tokenizer->peek().type != Token::KWORD_VAR) {
					THROW_PARSER_ERR(Error::SYNTAX_ERROR, "expected keyword \"func\" or \"var\" after static", Vect2i());
				}

			} break;

			case Token::KWORD_FUNC: {
				ptr<FunctionNode> func = _parse_func(class_node);
				class_node->functions.push_back(func);
			} break;

			case Token::KWORD_VAR: {
				stdvec<ptr<VarNode>> vars = _parse_var(class_node);
				for (ptr<VarNode>& _var : vars) {
					class_node->vars.push_back(_var);
				}
			} break;

			case Token::KWORD_CONST: {
				ptr<ConstNode> _const = _parse_const(class_node);
				class_node->constants.push_back(_const);
			} break;

			// compile time function call.
			case Token::IDENTIFIER: {

				ptr<CallNode> call = new_node<CallNode>();
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					if (tokenizer->next().type != Token::BRACKET_LPARAN) THROW_UNEXP_TOKEN("symbol \"(\"");
					call->r_args = _parse_arguments(class_node);
					class_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]
			default: {
				THROW_UNEXP_TOKEN("");
			}
		}
	}
}

ptr<Parser::EnumNode> Parser::_parse_enum(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_ENUM);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::CLASS);

	ptr<EnumNode> enum_node = new_node<EnumNode>();
	enum_node->parernt_node = p_parent;

	parser_context.current_enum = enum_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_enum = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER && tk->type != Token::BRACKET_LCUR)
		THROW_UNEXP_TOKEN("an identifier or symbol \"{\"");	

	if (tk->type == Token::IDENTIFIER) {
		THROW_IF_NAME_NATIVE(tk->identifier);
		// TODO: check identifier from import.
		if (p_parent->type == Node::Type::FILE) {
			THROW_IF_NAME_DEFINED(file_node, "a class", tk->identifier, classes);
			THROW_IF_NAME_DEFINED(file_node, "a variable", tk->identifier, vars);
			THROW_IF_NAME_DEFINED(file_node, "a constant", tk->identifier, constants);
			THROW_IF_NAME_DEFINED(file_node, "a function", tk->identifier, functions);
			THROW_IF_NAME_DEFINED(file_node, "an enum", tk->identifier, enums);
			THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);
		} else { // class enum.
			ptr<ClassNode> cn = ptrcast<ClassNode>(p_parent);
			THROW_IF_NAME_DEFINED(cn, "a variable", tk->identifier, vars);
			THROW_IF_NAME_DEFINED(cn, "a constant", tk->identifier, constants);
			THROW_IF_NAME_DEFINED(cn, "a function", tk->identifier, functions);
			THROW_IF_NAME_DEFINED(cn, "an enum", tk->identifier, enums);
			THROW_IF_NAME_DEFINED_ENUMVALUES(cn);
			// TODO: shadowed warning ?
		}

		enum_node->name = tk->identifier;
		enum_node->named_enum = true;
		tk = &tokenizer->next();
	}

	if (tk->type != Token::BRACKET_LCUR) THROW_UNEXP_TOKEN("symbol \"{\"");

	bool comma_valid = false;
	int64_t next_value = 0;
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				THROW_PARSER_ERR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::BRACKET_RCUR: {
				return enum_node;
			} break;

			case Token::SYM_COMMA: {
				if (!comma_valid) THROW_UNEXP_TOKEN("an identifier or symbol \"}\"");
				comma_valid = false;
			} break;

			case Token::IDENTIFIER: {
				// TODO: check identifier with include
				for (const std::pair<String, EnumValueNode>& value : enum_node->values) {
					if (value.first == token.identifier) {
						THROW_PREDEFINED("an enum value", value.first, value.second.pos);
					}
				}

				if (!enum_node->named_enum) {
					if (p_parent->type == Node::Type::FILE) {
						THROW_IF_NAME_DEFINED(file_node, "a variable", token.identifier, vars);
						THROW_IF_NAME_DEFINED(file_node, "a constant", tk->identifier, constants);
						THROW_IF_NAME_DEFINED(file_node, "a function", token.identifier, functions);
						THROW_IF_NAME_DEFINED(file_node, "an enum", token.identifier, enums);
						THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);
						
					} else { // CLASS
						ptr<ClassNode> cn = ptrcast<ClassNode>(p_parent);
						THROW_IF_NAME_DEFINED(cn, "a variable", token.identifier, vars);
						THROW_IF_NAME_DEFINED(cn, "a constant", tk->identifier, constants);
						THROW_IF_NAME_DEFINED(cn, "a function", token.identifier, functions);
						THROW_IF_NAME_DEFINED(cn, "an enum", token.identifier, enums);
						THROW_IF_NAME_DEFINED_ENUMVALUES(cn);
					}
				}
				
				const TokenData* tk = &tokenizer->peek();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next(); // eat "=".
					ptr<Node> expr = _parse_expression(enum_node, false);
					//if (expr->type != Node::Type::CONST_VALUE || ptrcast<ConstValueNode>(expr)->value.get_type() != var::INT) {
					//	THROW_PARSER_ERR(Error::INVALID_TYPE, "enum value must be a constant integer", Vect2i());
					//}
					//next_value = ptrcast<ConstValueNode>(expr)->value.operator int64_t();
					enum_node->values[token.identifier] = EnumValueNode(expr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				} else {
					enum_node->values[token.identifier] = EnumValueNode(nullptr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				}

				comma_valid = true;
			} break;

			default: {
				THROW_UNEXP_TOKEN("an identifier");
			} break;
		}
	}
}

stdvec<ptr<Parser::VarNode>> Parser::_parse_var(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);
	ASSERT(p_parent != nullptr);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::BLOCK || p_parent->type == Node::Type::CLASS);

	bool _static = tokenizer->peek(-2, true).type == Token::KWORD_STATIC;

	const TokenData* tk;
	stdvec<ptr<VarNode>> vars;

	while (true) {
		tk = &tokenizer->next();
		if (tk->type != Token::IDENTIFIER) {
			THROW_UNEXP_TOKEN("an identifier");
		}

		// TODO: check identifier include import
		if (p_parent->type == Node::Type::FILE) {
			THROW_IF_NAME_DEFINED(file_node, "a class", tk->identifier, classes);
			THROW_IF_NAME_DEFINED(file_node, "a variable", tk->identifier, vars);
			THROW_IF_NAME_DEFINED(file_node, "a constant", tk->identifier, constants);
			THROW_IF_NAME_DEFINED(file_node, "a function", tk->identifier, functions);
			THROW_IF_NAME_DEFINED(file_node, "an enum", tk->identifier, enums);
			THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);
		} else if (p_parent->type == Node::Type::CLASS) {
			ptr<ClassNode> cn = ptrcast<ClassNode>(p_parent);
			THROW_IF_NAME_DEFINED(cn, "a variable", tk->identifier, vars);
			THROW_IF_NAME_DEFINED(cn, "a constant", tk->identifier, constants);
			THROW_IF_NAME_DEFINED(cn, "a function", tk->identifier, functions);
			THROW_IF_NAME_DEFINED(cn, "an enum", tk->identifier, enums);
			THROW_IF_NAME_DEFINED_ENUMVALUES(cn);
		} else { // BLOCK local var
			ASSERT(parser_context.current_func != nullptr);
			for (int i = 0; i < (int)parser_context.current_func->args.size(); i++) {
				if (parser_context.current_func->args[i].name == tk->identifier) {
					THROW_PREDEFINED("an argument", tk->identifier, parser_context.current_func->args[i].pos);
				}
			}
			BlockNode* block = ptrcast<BlockNode>(p_parent).get();
			while (block) {
				for (int i = 0; i < (int)block->local_vars.size(); i++) {
					if (block->local_vars[i]->name == tk->identifier) {
						THROW_PREDEFINED("a variable", tk->identifier, block->local_vars[i]->pos);
					}
				}
				if (block->parernt_node->type == Node::Type::FUNCTION) {
					break;
				}
				block = ptrcast<BlockNode>(block->parernt_node).get();
			}
		}

		ptr<VarNode> var_node = new_node<VarNode>();
		var_node->parernt_node = p_parent;
		var_node->is_static = _static;
		var_node->name = tk->identifier;

		parser_context.current_var = var_node.get();
		class ScopeDestruct {
		public:
			Parser::ParserContext* context = nullptr;
			ScopeDestruct(Parser::ParserContext* p_context) {
				context = p_context;
			}
			~ScopeDestruct() {
				context->current_var = nullptr;
			}
		};
		ScopeDestruct destruct = ScopeDestruct(&parser_context);

		tk = &tokenizer->next();
		if (tk->type == Token::OP_EQ) {
			ptr<Node> expr = _parse_expression(p_parent, false);
			//_reduce_expression(expr); TODO: reduce after all are parsed.
			var_node->assignment = expr;

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
			} else if (tk->type == Token::SYM_SEMI_COLLON) {
				vars.push_back(var_node);
				break;
			} else {
				THROW_UNEXP_TOKEN("symbol \",\" or \";\"");
			}
		} else if (tk->type == Token::SYM_COMMA) {
		} else if (tk->type == Token::SYM_SEMI_COLLON) {
			vars.push_back(var_node);
			break;
		} else {
			THROW_UNEXP_TOKEN("symbol \",\" or \";\"");
		}
		vars.push_back(var_node);
	}
	return vars;
}

ptr<Parser::ConstNode> Parser::_parse_const(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_CONST);
	ASSERT(p_parent != nullptr);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::BLOCK || p_parent->type == Node::Type::CLASS);

	const TokenData* tk;
	tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) {
		THROW_UNEXP_TOKEN("an identifier");
	}

	// TODO: check identifier include import
	if (p_parent->type == Node::Type::FILE) {
		THROW_IF_NAME_DEFINED(file_node, "a class", tk->identifier, classes);
		THROW_IF_NAME_DEFINED(file_node, "a variable", tk->identifier, vars);
		THROW_IF_NAME_DEFINED(file_node, "a constant", tk->identifier, constants);
		THROW_IF_NAME_DEFINED(file_node, "a function", tk->identifier, functions);
		THROW_IF_NAME_DEFINED(file_node, "an enum", tk->identifier, enums);
		THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);
	} else if (p_parent->type == Node::Type::CLASS) {
		ptr<ClassNode> cn = ptrcast<ClassNode>(p_parent);
		THROW_IF_NAME_DEFINED(cn, "a variable", tk->identifier, vars);
		THROW_IF_NAME_DEFINED(cn, "a constant", tk->identifier, constants);
		THROW_IF_NAME_DEFINED(cn, "a function", tk->identifier, functions);
		THROW_IF_NAME_DEFINED(cn, "an enum", tk->identifier, enums);
		THROW_IF_NAME_DEFINED_ENUMVALUES(cn);
	} else { // BLOCK local var
		ASSERT(parser_context.current_func != nullptr);
		for (int i = 0; i < (int)parser_context.current_func->args.size(); i++) {
			if (parser_context.current_func->args[i].name == tk->identifier) {
				THROW_PREDEFINED("an argument", tk->identifier, parser_context.current_func->args[i].pos);
			}
		}
		BlockNode* block = ptrcast<BlockNode>(p_parent).get();
		while (block) {
			for (int i = 0; i < (int)block->local_vars.size(); i++) {
				if (block->local_vars[i]->name == tk->identifier) {
					THROW_PREDEFINED("a variable", tk->identifier, block->local_vars[i]->pos);
				}
			}
			if (block->parernt_node->type == Node::Type::FUNCTION) {
				break;
			}
			block = ptrcast<BlockNode>(block->parernt_node).get();
		}
	}

	ptr<ConstNode> const_node = new_node<ConstNode>();
	const_node->parernt_node = p_parent;
	const_node->name = tk->identifier;

	tk = &tokenizer->next();
	if (tk->type != Token::OP_EQ) THROW_UNEXP_TOKEN("symbol \"=\"");
	ptr<Node> expr = _parse_expression(p_parent, false);
	const_node->assignment = expr;

	tk = &tokenizer->next();
	if (tk->type != Token::SYM_SEMI_COLLON) THROW_UNEXP_TOKEN("symbol \";\"");

	return const_node;
}

ptr<Parser::FunctionNode> Parser::_parse_func(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_FUNC);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::CLASS);

	ptr<FunctionNode> func_node = new_node<FunctionNode>();
	func_node->parent_node = p_parent;
	if (tokenizer->peek(-2, true).type == Token::KWORD_STATIC) {
		func_node->is_static = true;
	}

	parser_context.current_func = func_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_func = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	// TODO: check identifier from import
	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
	if (p_parent->type == Node::Type::FILE) {
		THROW_IF_NAME_DEFINED(file_node, "a class", tk->identifier, classes);
		THROW_IF_NAME_DEFINED(file_node, "a variable", tk->identifier, vars);
		THROW_IF_NAME_DEFINED(file_node, "a constant", tk->identifier, constants);
		THROW_IF_NAME_DEFINED(file_node, "a function", tk->identifier, functions);
		THROW_IF_NAME_DEFINED(file_node, "an enum", tk->identifier, enums);
		THROW_IF_NAME_DEFINED_ENUMVALUES(file_node);
	} else { // CLASS
		ptr<ClassNode> cn = ptrcast<ClassNode>(p_parent);
		THROW_IF_NAME_DEFINED(cn, "a variable", tk->identifier, vars);
		THROW_IF_NAME_DEFINED(cn, "a constant", tk->identifier, constants);
		THROW_IF_NAME_DEFINED(cn, "a function", tk->identifier, functions);
		THROW_IF_NAME_DEFINED(cn, "an enum", tk->identifier, enums);
		THROW_IF_NAME_DEFINED_ENUMVALUES(cn);
	}

	func_node->name = tk->identifier;
	if (parser_context.current_class && parser_context.current_class->name == tk->identifier) {
		parser_context.current_class->constructor = func_node.get();
	}

	tk = &tokenizer->next();
	if (tk->type != Token::BRACKET_LPARAN) THROW_UNEXP_TOKEN("symbol \"(\"");
	tk = &tokenizer->next();

	bool has_default = false;
	if (tk->type != Token::BRACKET_RPARAN) {
		while (true) {
			if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
			for (int i = 0; i < (int)func_node->args.size(); i++) {
				if (func_node->args[i].name == tk->identifier)
					THROW_PARSER_ERR(Error::NAME_ERROR, String::format("identifier \"%s\" already defined in arguments", tk->identifier.c_str()), Vect2i());
			}

			ParameterNode parameter = ParameterNode(tk->identifier, tk->get_pos());
			tk = &tokenizer->next();
			if (tk->type == Token::OP_EQ) {
				has_default = true;
				parameter.default_value = _parse_expression(p_parent, false);
				tk = &tokenizer->next();
			} else {
				if (has_default)
					THROW_PARSER_ERR(Error::SYNTAX_ERROR, "default parameter expected.", Vect2i());
			}
			func_node->args.push_back(parameter);

			if (tk->type == Token::SYM_COMMA) {
				tk = &tokenizer->next(); // eat ','
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				THROW_UNEXP_TOKEN("");
			}
		}
	}

	if (tokenizer->next().type != Token::BRACKET_LCUR) {
		THROW_UNEXP_TOKEN("symbol \"{\"");
	}

	func_node->body = _parse_block(func_node);
	if (tokenizer->next().type != Token::BRACKET_RCUR) {
		THROW_UNEXP_TOKEN("symbol \"}\"");
	}

	return func_node;
}

// -----------------------------------------------------------------------------

#if DEBUG_BUILD
// properly implement this.
#define KEYWORD_COLOR Logger::Color::L_YELLOW
#define TYPE_COLOR Logger::Color::L_GREEN

static int _print_id = 0; // dirty way for debugging.
#define PRINT_INDENT(m_indent)                           \
do {                                                     \
	for (int i = 0; i < m_indent; i++)  printf("    ");  \
	_print_id++;                                         \
} while (false)
#define PRINT_COLOR(m_log, m_color) Logger::log(m_log, Logger::VERBOSE, m_color)

static void print_var_node(const Parser::VarNode* p_var, int p_indent) {
	PRINT_INDENT(p_indent);
	if (p_var->is_static) {
		PRINT_COLOR(TokenData(Token::KWORD_STATIC).to_string().c_str(), KEYWORD_COLOR); printf(" ");
	}
	PRINT_COLOR(TokenData(Token::KWORD_VAR).to_string().c_str(), KEYWORD_COLOR);
	printf(" %s = ", p_var->name.c_str());
	if (p_var->assignment != nullptr) {
		if (p_var->assignment->type == Parser::Node::Type::CONST_VALUE)
			printf("%s\n", ptrcast<Parser::ConstValueNode>(p_var->assignment)->value.to_string().c_str());
		else
			PRINT_COLOR(String::format("[expr:%i]\n", _print_id).c_str(), Logger::Color::L_GRAY);
	}
	else PRINT_COLOR("nullptr\n", Logger::Color::L_GRAY);
}


static void print_block_node(const Parser::BlockNode* p_block, int p_indent) {
	for (ptr<Parser::Node> node : p_block->statements) {
		switch (node->type) {
			//BLOCK,
			case Parser::Node::Type::VAR: {
				print_var_node(ptrcast<Parser::VarNode>(node).get(), p_indent);
			} break;

			case Parser::Node::Type::IDENTIFIER: {
				PRINT_INDENT(p_indent);
				printf("%s\n", ptrcast<Parser::IdentifierNode>(node)->name.c_str());
			} break;

			case Parser::Node::Type::CONST_VALUE: {
				PRINT_INDENT(p_indent);
				printf("%s\n", ptrcast<Parser::ConstValueNode>(node)->value.to_string().c_str());
			} break;

			case Parser::Node::Type::ARRAY: {
				PRINT_INDENT(p_indent);
				PRINT_COLOR(String::format("[Array:%i]\n", _print_id).c_str(), Logger::Color::L_GRAY);
			} break;
			//case Parser::Node::Type::MAP: {
			//	PRINT_INDENT(p_indent);
			//	PRINT_COLOR("[MAP]\n", Logger::Color::L_GRAY);
			//} break;
			case Parser::Node::Type::THIS: {
				PRINT_INDENT(p_indent);
				PRINT_COLOR("this\n", KEYWORD_COLOR);
			} break;
			case Parser::Node::Type::SUPER: {
				PRINT_INDENT(p_indent);
				PRINT_COLOR("super\n", KEYWORD_COLOR);
			} break;

			// TODO: function call is operator node. create seperate function for print_expr();
			//case Parser::Node::Type::BUILTIN_FUNCTION: {
			//	PRINT_INDENT(p_indent);
			//	const char* func_name = BuiltinFunctions::get_func_name(ptrcast<Parser::BuiltinFunctionNode>(node)->func);
			//	int arg_count = BuiltinFunctions::get_arg_count(ptrcast<Parser::BuiltinFunctionNode>(node)->func);
			//	printf("%s(", func_name);
			//	if (arg_count != 0) printf("...");
			//	printf(");\n");
			//}
			//BUILTIN_FUNCTION,
			//BUILTIN_CLASS,

			case Parser::Node::Type::OPERATOR: {
				PRINT_INDENT(p_indent);
				const char* op_name = Parser::OperatorNode::get_op_name(ptrcast<Parser::OperatorNode>(node)->op_type);
				PRINT_COLOR(String::format("[%s:%i]\n", op_name, _print_id).c_str(), Logger::Color::L_GRAY);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				PRINT_INDENT(p_indent);
				Parser::ControlFlowNode* dbg = ptrcast<Parser::ControlFlowNode>(node).get();
				const char* cf_name = Parser::ControlFlowNode::get_cftype_name(ptrcast<Parser::ControlFlowNode>(node)->cf_type);
				PRINT_COLOR(String::format("[%s:%i]\n", cf_name, _print_id).c_str(), Logger::Color::L_GRAY);
			} break;
		}
	}
}

static void print_enum_node(const Parser::EnumNode* p_enum, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR("enum", KEYWORD_COLOR);
	if (p_enum->named_enum) PRINT_COLOR(String(String(" ") + p_enum->name + "\n").c_str(), TYPE_COLOR);
	else PRINT_COLOR(" [not named]\n", Logger::Color::L_GRAY);

	for (const std::pair<String, Parser::EnumValueNode>& value : p_enum->values) {
		PRINT_INDENT(p_indent + 1);
		printf("%s = ", value.first.c_str());
		//if (value.second != nullptr) {
		//	if (value.second->type == Parser::Node::Type::CONST_VALUE) {
		//		ptr<Parser::ConstValueNode> enum_val = ptrcast<Parser::ConstValueNode>(value.second);
		//		ASSERT(enum_val->value.get_type() == var::INT);
		//		printf("%lli\n", enum_val->value.operator int64_t());
		//	} else {
		//		PRINT_COLOR("[TODO: reduce expression]\n", Logger::Color::L_GRAY);
		//	}
		//} else {
		//	PRINT_COLOR("[TODO: auto increase]\n", Logger::Color::L_GRAY);
		//}
		printf("%lli\n", value.second.value);
	}
}

static void print_func_node(const Parser::FunctionNode* p_func, int p_indent) {
	PRINT_INDENT(p_indent);
	if (p_func->is_static) {
		PRINT_COLOR(TokenData(Token::KWORD_STATIC).to_string().c_str(), KEYWORD_COLOR); printf(" ");
	}
	PRINT_COLOR(TokenData(Token::KWORD_FUNC).to_string().c_str(), KEYWORD_COLOR);
	printf(" %s(", p_func->name.c_str());
	for (int j = 0; j < (int)p_func->args.size(); j++) {
		if (j > 0) printf(", ");
		printf("%s", p_func->args[j].name.c_str());
	}
	printf(")\n");

	print_block_node(p_func->body.get(), p_indent + 1);
}

static void print_class_node(Parser::ClassNode* p_class, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR(TokenData(Token::KWORD_CLASS).to_string().c_str(), KEYWORD_COLOR);
	PRINT_COLOR((String(" ") + p_class->name).c_str(), TYPE_COLOR);
	if (p_class->base_type != Parser::ClassNode::NO_BASE) {
		printf(" inherits ");
		if (p_class->base_type == Parser::ClassNode::BASE_LOCAL) {
			PRINT_COLOR(p_class->base_class.c_str(), TYPE_COLOR);
		} else { // BASE_EXTERN
			PRINT_COLOR(p_class->base_file.c_str(), TYPE_COLOR);
			printf(".");
			PRINT_COLOR(p_class->base_class.c_str(), TYPE_COLOR);
		}
	}
	printf("\n");

	for (int i = 0; i < (int)p_class->enums.size(); i++) {
		print_enum_node(p_class->enums[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_class->vars.size(); i++) {
		print_var_node(p_class->vars[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_class->functions.size(); i++) {
		print_func_node(p_class->functions[i].get(), p_indent + 1);
	}
}

static void print_file_node(const Parser::FileNode* p_fn, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR("file", KEYWORD_COLOR);
	printf(" (%s)\n", p_fn->path.c_str());

	for (int i = 0; i < (int)p_fn->enums.size(); i++) {
		print_enum_node(p_fn->enums[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_fn->vars.size(); i++) {
		print_var_node(p_fn->vars[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_fn->functions.size(); i++) {
		print_func_node(p_fn->functions[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_fn->classes.size(); i++) {
		print_class_node(p_fn->classes[i].get(), p_indent + 1);
	}
}
void Parser::print_tree() const {
	_print_id = 0;
	print_file_node(file_node.get(), 0);
}
#endif

}