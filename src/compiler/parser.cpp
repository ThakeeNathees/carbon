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

#include "compiler/parser.h"

#include "compiler/analyzer.h"
#include "compiler/compiler.h"
#include "native/path.h"

/******************************************************************************************************************/
/*                                         PARSER                                                                 */
/******************************************************************************************************************/

namespace carbon {

CompileTimeError Parser::_unexp_token_error(const char* p_exptected, const DBGSourceInfo& p_dbg_info) const {
	Error::Type err_type = Error::SYNTAX_ERROR;
	if (tokenizer->peek(-1, true).type == Token::_EOF) err_type = Error::UNEXPECTED_EOF;
	if (p_exptected != nullptr) {
		return _parser_error(err_type, String::format("unexpected token(\"%s\"). expected %s.",
				Tokenizer::get_token_name(tokenizer->peek(-1, true).type), p_exptected).c_str(), Vect2i(), p_dbg_info);
	} else {
		return _parser_error(err_type, String::format("unexpected token(\"%s\").",
			Tokenizer::get_token_name(tokenizer->peek(-1, true).type)).c_str(), Vect2i(), p_dbg_info);
	}    
}

CompileTimeError Parser::_parser_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	String token_str = "";
	if (p_pos.x > 0 && p_pos.y > 0) token_str = tokenizer->get_token_at(p_pos).to_string();
	else token_str = tokenizer->peek(-1, true).to_string();
	uint32_t err_len = (uint32_t)token_str.size();

	Vect2i pos = (p_pos.x > 0 && p_pos.y > 0) ? p_pos : tokenizer->peek(-1, true).get_pos();
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source,
		std::pair<int, int>((int)pos.x, (int)pos.y), err_len), p_dbg_info);
}

CompileTimeError Parser::_predefined_error(const String& p_what, const String& p_name, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	return _parser_error(Error::NAME_ERROR,
		String::format((p_what + " named \"%s\" already exists at (line:%i, col:%i)").c_str(), p_name.c_str(), p_pos.x, p_pos.y),
		Vect2i(), p_dbg_info);
}

void Parser::parse(ptr<Tokenizer> p_tokenizer) {
	
	tokenizer = p_tokenizer;
	file_node = new_node<FileNode>();

	// TODO: maybe redundant
	file_node->source = tokenizer->get_source();
	file_node->path = tokenizer->get_source_path();

	while (true) {
	
		const TokenData& token = tokenizer->next();
		switch (token.type) {
			case  Token::_EOF:
				return;

			case Token::KWORD_IMPORT: {
				file_node->imports.push_back(_parse_import());
			} break;

			case Token::KWORD_CLASS: {
				file_node->classes.push_back(_parse_class());
			} break;

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
			} break;

			case Token::KWORD_VAR: {
				stdvec<ptr<VarNode>> vars = _parse_var(file_node);
				for (ptr<VarNode>& _var : vars) {
					file_node->vars.push_back(_var);
				}
			} break;

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
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					ptr<CallNode> call = new_node<CallNode>();
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					call->args = _parse_arguments(file_node);
					call->is_compilttime = true;
					file_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]

			default:
				throw UNEXP_TOKEN_ERROR(nullptr);
		}

	} // while true
}

void Parser::_check_identifier_predefinition(const String& p_name, Node* p_scope) const {
	const TokenData* tk = &tokenizer->peek(-1, true);

	if (NativeClasses::singleton()->is_class_registered(p_name)) {
		throw PARSER_ERROR(Error::NAME_ERROR, String::format("a native type named %s already exists", p_name.c_str()), Vect2i());
	}

	for (ptr<ImportNode>& in : file_node->imports) {
		if (p_name == in->name) throw PREDEFINED_ERROR("an imported file", p_name, in->pos);
	}

	if (p_scope == nullptr || p_scope->type == Node::Type::CLASS || p_scope->type == Node::Type::FILE) {
		const MemberContainer* scope = nullptr;

		if (p_scope == nullptr) scope = file_node.get();
		else scope = static_cast<const MemberContainer*>(p_scope);

		for (int i = 0; i < (int)scope->vars.size(); i++) {
			if (scope->vars[i]->name == p_name) {
				throw PREDEFINED_ERROR("a var", scope->vars[i]->name, scope->vars[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->constants.size(); i++) {
			if (scope->constants[i]->name == p_name) {
				throw PREDEFINED_ERROR("a constant", scope->constants[i]->name, scope->constants[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->functions.size(); i++) {
			if (scope->functions[i]->name == p_name) {
				throw PREDEFINED_ERROR("a function", scope->functions[i]->name, scope->functions[i]->pos);
			}
		}

		for (int i = 0; i < (int)scope->enums.size(); i++) {
			if (scope->enums[i]->name == p_name) {
				throw PREDEFINED_ERROR("an enum", scope->enums[i]->name, scope->enums[i]->pos);
			}
		}

		if (scope->unnamed_enum != nullptr) {
			for (auto it = scope->unnamed_enum->values.begin(); it != scope->unnamed_enum->values.end(); it++) {
				if (it->first == p_name) {
					throw PREDEFINED_ERROR("an enum value", p_name, it->second.pos);
				}
			}
		}

		if (scope->type == Node::Type::FILE) {
			const Parser::FileNode* p_file = static_cast<const Parser::FileNode*>(scope);
			for (int i = 0; i < (int)p_file->classes.size(); i++) {
				if (p_file->classes[i]->name == p_name) {
					throw PREDEFINED_ERROR("a classe", p_file->classes[i]->name, p_file->classes[i]->pos);
				}
			}
		}
	} else if (p_scope->type == Node::Type::BLOCK) {
		ASSERT(parser_context.current_func != nullptr);
		for (int i = 0; i < (int)parser_context.current_func->args.size(); i++) {
			if (parser_context.current_func->args[i].name == p_name) {
				throw PREDEFINED_ERROR("an argument", p_name, parser_context.current_func->args[i].pos);
			}
		}
		const BlockNode* block = static_cast<const BlockNode*>(p_scope);
		while (block) {
			for (int i = 0; i < (int)block->local_vars.size(); i++) {
				if (block->local_vars[i]->name == p_name) {
					throw  PREDEFINED_ERROR("a variable", p_name, block->local_vars[i]->pos);
				}
			}
			if (block->parernt_node->type == Node::Type::FUNCTION) break;
			block = ptrcast<BlockNode>(block->parernt_node).get();
		}
	} else {
		ASSERT(false);
	}
}

ptr<Parser::ImportNode> Parser::_parse_import() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IMPORT);

	ptr<ImportNode> import_node = new_node<ImportNode>();

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");

	String name = tk->identifier;
	_check_identifier_predefinition(name, nullptr);
	import_node->name = name;

	if (tokenizer->next().type != Token::OP_EQ) throw UNEXP_TOKEN_ERROR("symbol \"=\"");
	tk = &tokenizer->next();
	if (tk->type != Token::VALUE_STRING) throw UNEXP_TOKEN_ERROR("string path to source");
	String path = tk->constant.operator String();

	import_node->bytecode = Compiler::singleton()->compile(path);

	tk = &tokenizer->next();
	if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

	return import_node;
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
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, nullptr);

	class_node->name = tk->identifier;

	// Inheritance.
	tk = &tokenizer->next();
	if (tk->type == Token::SYM_COLLON) {

		tk = &tokenizer->next();
		if (tk->type == Token::BUILTIN_TYPE) throw PARSER_ERROR(Error::TYPE_ERROR, "cannot inherit a builtin type.", Vect2i());
		if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
		class_node->base_class_name = tk->identifier;

		tk = &tokenizer->next();
		if (tk->type == Token::SYM_DOT) {
			tk = &tokenizer->next();
			if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");

			String base_file_name  = class_node->base_class_name;
			String base_class_name = tk->identifier;
			class_node->base_type = ClassNode::BASE_EXTERN;

			Bytecode* base_file = nullptr;
			for (ptr<ImportNode>& in : file_node->imports) {
				if (in->name == base_file_name) {
					base_file = in->bytecode.get();
					break;
				}
			}
			if (!base_file)
				throw PARSER_ERROR(Error::NAME_ERROR,
					String::format("base file name \"%s\" not found from the imported libs.", base_file_name.c_str()), Vect2i());

			ptr<Bytecode> base_binary = nullptr;
			for (const std::pair<String, ptr<Bytecode>>& cls : base_file->get_classes()) {
				if (cls.first == base_class_name) {
					base_binary = cls.second;
					break;
				}
			}
			if (!base_binary) {
				throw PARSER_ERROR(Error::NAME_ERROR,
					String::format("base class name \"%s\" not found from the imported lib \"%s\".", base_class_name.c_str(), base_file_name.c_str()),
					Vect2i());
			}

			class_node->base_binary = base_binary;
			class_node->base_class_name = base_class_name;

			tk = &tokenizer->next();
		} else {
			if (NativeClasses::singleton()->is_class_registered(class_node->base_class_name)) {
				class_node->base_type = ClassNode::BASE_NATIVE;
			} else {
				if (class_node->base_class_name == class_node->name)
					throw PARSER_ERROR(Error::TYPE_ERROR, "cyclic inheritance. class inherits itself isn't allowed.", tokenizer->peek(-2, true).get_pos());
				class_node->base_type = ClassNode::BASE_LOCAL;
			}
		}
	}

	if (tk->type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");
	
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
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
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "expected keyword \"func\" or \"var\" after static", Vect2i());
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
					if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					call->args = _parse_arguments(class_node);
					call->is_compilttime = true;
					class_node->compiletime_functions.push_back(call);
					break;
				}
			} // [[fallthrough]]
			default: {
				throw UNEXP_TOKEN_ERROR(nullptr);
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
		throw UNEXP_TOKEN_ERROR("an identifier or symbol \"{\"");	

	if (tk->type == Token::IDENTIFIER) {
		_check_identifier_predefinition(tk->identifier, p_parent.get());

		enum_node->name = tk->identifier;
		enum_node->named_enum = true;
		tk = &tokenizer->next();
	}

	if (tk->type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");

	bool comma_valid = false;
	int64_t next_value = 0;
	while (true) {
		const TokenData& token = tokenizer->next();
		switch (token.type) {

			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::BRACKET_RCUR: {
				return enum_node;
			} break;

			case Token::SYM_COMMA: {
				if (!comma_valid) throw UNEXP_TOKEN_ERROR("an identifier or symbol \"}\"");
				comma_valid = false;
			} break;

			case Token::IDENTIFIER: {
				for (const std::pair<String, EnumValueNode>& value : enum_node->values) {
					if (value.first == token.identifier) throw PREDEFINED_ERROR("an enum value", value.first, value.second.pos);
				}

				if (!enum_node->named_enum) {

					// TODO: check if it's compile time function.
					//BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(token.identifier);
					//if (builtin_func != BuiltinFunctions::UNKNOWN && BuiltinFunctions::is_compiletime(builtin_func)) {
					//	ptr<CallNode> call = new_node<CallNode>();
					//	call->base = new_node<BuiltinFunctionNode>(builtin_func);
					//	if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
					//	call->args = _parse_arguments(file_node);
					//	call->is_compilttime = true;
					//	if (parser_context.current_class != nullptr) parser_context.current_class->compiletime_functions.push_back(call);
					//	else file_node->compiletime_functions.push_back(call);
					//	break;
					//}

					_check_identifier_predefinition(token.identifier, p_parent.get());
				}
				
				const TokenData* tk = &tokenizer->peek();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next(); // eat "=".
					ptr<Node> expr = _parse_expression(enum_node, false);
					enum_node->values[token.identifier] = EnumValueNode(expr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				} else {
					enum_node->values[token.identifier] = EnumValueNode(nullptr, token.get_pos(), (enum_node->named_enum) ? enum_node.get() : nullptr);
				}

				comma_valid = true;
			} break;

			default: {
				throw UNEXP_TOKEN_ERROR("an identifier");
			} break;
		}
	}
}

stdvec<ptr<Parser::VarNode>> Parser::_parse_var(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);
	ASSERT(p_parent != nullptr);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::BLOCK || p_parent->type == Node::Type::CLASS);

	bool _static = p_parent->type == Node::Type::FILE || tokenizer->peek(-2, true).type == Token::KWORD_STATIC;

	const TokenData* tk;
	stdvec<ptr<VarNode>> vars;

	while (true) {
		tk = &tokenizer->next();

		if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
		_check_identifier_predefinition(tk->identifier, p_parent.get());

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
			var_node->assignment = expr;

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
			} else if (tk->type == Token::SYM_SEMI_COLLON) {
				vars.push_back(var_node);
				break;
			} else {
				throw UNEXP_TOKEN_ERROR("symbol \",\" or \";\"");
			}
		} else if (tk->type == Token::SYM_COMMA) {
		} else if (tk->type == Token::SYM_SEMI_COLLON) {
			vars.push_back(var_node);
			break;
		} else {
			throw UNEXP_TOKEN_ERROR("symbol \",\" or \";\"");
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

	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, p_parent.get());

	ptr<ConstNode> const_node = new_node<ConstNode>();
	const_node->parernt_node = p_parent;
	const_node->name = tk->identifier;

	parser_context.current_const = const_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_const = nullptr;
		}
	};
	ScopeDestruct destruct = ScopeDestruct(&parser_context);

	tk = &tokenizer->next();
	if (tk->type != Token::OP_EQ) throw UNEXP_TOKEN_ERROR("symbol \"=\"");
	ptr<Node> expr = _parse_expression(p_parent, false);
	const_node->assignment = expr;

	tk = &tokenizer->next();
	if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

	return const_node;
}

ptr<Parser::FunctionNode> Parser::_parse_func(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_FUNC);
	ASSERT(p_parent->type == Node::Type::FILE || p_parent->type == Node::Type::CLASS);

	ptr<FunctionNode> func_node = new_node<FunctionNode>();
	func_node->parent_node = p_parent.get();
	if (p_parent->type == Node::Type::FILE || tokenizer->peek(-2, true).type == Token::KWORD_STATIC) {
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

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
	_check_identifier_predefinition(tk->identifier, p_parent.get());

	func_node->name = tk->identifier;
	if (parser_context.current_class && parser_context.current_class->name == tk->identifier) {
		func_node->is_constructor = true;
		parser_context.current_class->constructor = func_node.get();
	}

	tk = &tokenizer->next();
	if (tk->type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
	tk = &tokenizer->next();

	bool has_default = false;
	if (tk->type != Token::BRACKET_RPARAN) {
		while (true) {
			if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
			for (int i = 0; i < (int)func_node->args.size(); i++) {
				if (func_node->args[i].name == tk->identifier)
					throw PARSER_ERROR(Error::NAME_ERROR, String::format("identifier \"%s\" already defined in arguments", tk->identifier.c_str()), Vect2i());
			}

			ParameterNode parameter = ParameterNode(tk->identifier, tk->get_pos());
			tk = &tokenizer->next();

			if (tk->type == Token::OP_BIT_AND) {
				parameter.is_reference = true;
				tk = &tokenizer->next();
			}

			if (tk->type == Token::OP_EQ) {
				has_default = true;
				parameter.default_value = _parse_expression(p_parent, false);
				tk = &tokenizer->next();
			} else {
				if (has_default)
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "default parameter expected.", Vect2i());
			}
			func_node->args.push_back(parameter);

			if (tk->type == Token::SYM_COMMA) {
				tk = &tokenizer->next(); // eat ','
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				throw UNEXP_TOKEN_ERROR(nullptr);
			}
		}
	}

	const TokenData& _next = tokenizer->next();
	bool _single_expr = false;

	if (_next.type == Token::OP_EQ) {
		_single_expr = true;
	} else if (_next.type != Token::BRACKET_LCUR) {
		throw UNEXP_TOKEN_ERROR("symbol \"{\"");
	}

	// TODO: this could be used to inline.
	if (_single_expr) {

		if (parser_context.current_class && parser_context.current_class->constructor) {
			if (parser_context.current_class->constructor == parser_context.current_func) {
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "constructor can't return anything.", _next.get_pos());
			}
		}

		ptr<BlockNode> block_node = newptr<BlockNode>();
		block_node->parernt_node = func_node;

		ptr<ControlFlowNode> _return = new_node<ControlFlowNode>(ControlFlowNode::RETURN);
		_return->args.push_back(_parse_expression(func_node, false));
		_return->parernt_node = func_node;
		_return->_return = parser_context.current_func;
		parser_context.current_func->has_return = true;
		block_node->statements.push_back(_return);

		tk = &tokenizer->next();
		if (tk->type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");

		func_node->body = block_node;

	} else {
		func_node->body = _parse_block(func_node);
		if (tokenizer->next().type != Token::BRACKET_RCUR) {
			throw UNEXP_TOKEN_ERROR("symbol \"}\"");
		}
		func_node->end_line = (uint32_t)tokenizer->get_pos().x;
	}

	return func_node;
}

}

/******************************************************************************************************************/
/*                                         BLOCK                                                                  */
/******************************************************************************************************************/

namespace carbon {

ptr<Parser::BlockNode> Parser::_parse_block(const ptr<Node>& p_parent, bool p_single_statement, stdvec<Token> p_termination) {
	ptr<BlockNode> block_node = newptr<BlockNode>();
	block_node->parernt_node = p_parent;

	parser_context.current_block = block_node.get();
	class ScopeDestruct {
	public:
		Parser::ParserContext* context = nullptr;
		ScopeDestruct(Parser::ParserContext* p_context) {
			context = p_context;
		}
		~ScopeDestruct() {
			context->current_block = nullptr;
		}
	};
	ScopeDestruct distruct = ScopeDestruct(&parser_context);

	const TokenData* tk;

	while (true) {
		tk = &tokenizer->peek();
		switch (tk->type) {
			case Token::_EOF: {
				throw PARSER_ERROR(Error::UNEXPECTED_EOF, "Unexpected end of file.", Vect2i());
			} break;

			case Token::KWORD_VAR: {
				tokenizer->next(); // eat "var"
				stdvec<ptr<VarNode>> vars = _parse_var(block_node);
				for (ptr<VarNode>& _var : vars) {
					block_node->local_vars.push_back(_var); // for quick access.
					block_node->statements.push_back(_var);
				}
			} break;

			case Token::KWORD_CONST: {
				tokenizer->next(); // ear "const"
				ptr<ConstNode> _const = _parse_const(block_node);
				block_node->local_const.push_back(_const);
				block_node->statements.push_back(_const);
			} break;

				// Ignore.
			case Token::SYM_SEMI_COLLON:
				tokenizer->next(); // eat ";"
				break;

				// could be 1/2 or something, default case
				//case Token::VALUE_NULL:
				//case Token::VALUE_BOOL:
				//case Token::VALUE_INT:
				//case Token::VALUE_FLOAT:
				//case Token::VALUE_STRING: {
				//	tk = &tokenizer->next(); // will be ignored by analyzer
				//	ptr<ConstValueNode> value = new_node<ConstValueNode>(tk->constant);
				//	block_node->statements.push_back(value);
				//} break;

			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				block_node->statements.push_back(_parse_if_block(block_node));
			} break;

			case Token::KWORD_SWITCH: {
				tk = &tokenizer->next(); // eat "switch"
				ptr<ControlFlowNode> switch_block = new_node<ControlFlowNode>(ControlFlowNode::SWITCH);
				switch_block->parernt_node = p_parent;
				ControlFlowNode* outer_break = parser_context.current_break;
				parser_context.current_break = switch_block.get();

				switch_block->args.push_back(_parse_expression(block_node, false));
				if (tokenizer->next().type != Token::BRACKET_LCUR) throw UNEXP_TOKEN_ERROR("symbol \"{\"");

				while (true) {
					tk = &tokenizer->next();
					if (tk->type == Token::KWORD_CASE) {
						ControlFlowNode::SwitchCase _case;
						_case.pos = tk->get_pos();
						_case.expr = _parse_expression(block_node, false);
						if (tokenizer->next().type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");

						// COMMENTED: `case VALUE: { expr; expr; }` <--- curly brackets are not allowed.
						//tk = &tokenizer->peek();
						//if (tk->type == Token::BRACKET_LCUR) {
						//	tokenizer->next(); // eat "{"
						//	_case.body = _parse_block(block_node);
						//	if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN("symbol \"}\"");
						//} else {
						_case.body = _parse_block(block_node, false, { Token::KWORD_CASE, Token::KWORD_DEFAULT, Token::BRACKET_RCUR });
						//}
						switch_block->switch_cases.push_back(_case);

					} else if (tk->type == Token::KWORD_DEFAULT) {
						ControlFlowNode::SwitchCase _case;
						_case.default_case = true;
						if (tokenizer->next().type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");
						_case.body = _parse_block(block_node, false);
					} else if (tk->type == Token::BRACKET_RCUR) {
						break;
					} else {
						throw UNEXP_TOKEN_ERROR("keyword \"case\" or symbol \"}\"");
					}
				}
				block_node->statements.push_back(switch_block);
				parser_context.current_break = outer_break;
			} break;

			case Token::KWORD_WHILE: {
				tk = &tokenizer->next(); // eat "while"
				ptr<ControlFlowNode> while_block = new_node<ControlFlowNode>(ControlFlowNode::WHILE);

				ControlFlowNode* outer_break = parser_context.current_break;
				ControlFlowNode* outer_continue = parser_context.current_continue;
				parser_context.current_break = while_block.get();
				parser_context.current_continue = while_block.get();

				while_block->parernt_node = p_parent;
				while_block->args.push_back(_parse_expression(block_node, false));
				tk = &tokenizer->peek();
				if (tk->type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					while_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
				} else {
					while_block->body = _parse_block(block_node, true);
				}
				block_node->statements.push_back(while_block);
				parser_context.current_break = outer_break;
				parser_context.current_continue = outer_continue;
			} break;

			case Token::KWORD_FOR: {
				tk = &tokenizer->next(); // eat "for"
				ptr<ControlFlowNode> for_block = new_node<ControlFlowNode>(ControlFlowNode::FOR);
				ControlFlowNode* outer_break = parser_context.current_break;
				ControlFlowNode* outer_continue = parser_context.current_continue;
				parser_context.current_break = for_block.get();
				parser_context.current_continue = for_block.get();

				for_block->parernt_node = p_parent;
				if (tokenizer->next().type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");

				if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
					tokenizer->next(); // eat ";"
					for_block->args.push_back(nullptr);
				} else {
					if (tokenizer->peek().type == Token::KWORD_VAR) {
						tokenizer->next(); // eat "var"

						tk = &tokenizer->next();
						if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR("an identifier");
						_check_identifier_predefinition(tk->identifier, block_node.get());

						ptr<VarNode> var_node = new_node<VarNode>();
						var_node->parernt_node = p_parent;
						var_node->name = tk->identifier;

						tk = &tokenizer->next();
						if (tk->type == Token::OP_EQ) {
							parser_context.current_var = var_node.get();
							ptr<Node> expr = _parse_expression(p_parent, false);
							parser_context.current_var = nullptr;
							var_node->assignment = expr;
							if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
						} else if (tk->type == Token::SYM_SEMI_COLLON) {

						} else if (tk->type == Token::SYM_COLLON) {
							for_block->cf_type = ControlFlowNode::CfType::FOREACH;
						}
						for_block->args.push_back(var_node);
					} else {
						for_block->args.push_back(_parse_expression(block_node, true));
						if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
					}
				}

				if (for_block->cf_type == ControlFlowNode::CfType::FOREACH) {
					for_block->args.push_back(_parse_expression(block_node, false));
					if (tokenizer->next().type != Token::BRACKET_RPARAN) throw UNEXP_TOKEN_ERROR("symbol \")\"");
				} else {
					if (tokenizer->peek().type == Token::SYM_SEMI_COLLON) {
						tokenizer->next(); // eat ";"
						for_block->args.push_back(nullptr);
					} else {
						for_block->args.push_back(_parse_expression(block_node, false));
						if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
					}

					if (tokenizer->peek().type == Token::BRACKET_RPARAN) {
						tokenizer->next(); // eat ")"
						for_block->args.push_back(nullptr);
					} else {
						for_block->args.push_back(_parse_expression(block_node, true));
						if (tokenizer->next().type != Token::BRACKET_RPARAN) throw UNEXP_TOKEN_ERROR("symbol \")\"");
					}
				}

				if (tokenizer->peek().type == Token::BRACKET_LCUR) {
					tokenizer->next(); // eat "{"
					for_block->body = _parse_block(block_node);
					if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
				} else {
					for_block->body = _parse_block(block_node, true);
				}

				// add loop counter initialization to local vars.
				if (for_block->args[0] != nullptr && for_block->args[0]->type == Node::Type::VAR) {
					for_block->body->local_vars.insert(for_block->body->local_vars.begin(), ptrcast<VarNode>(for_block->args[0]));
				}

				block_node->statements.push_back(for_block);
				parser_context.current_break = outer_break;
				parser_context.current_continue = outer_continue;
			} break;

			case Token::KWORD_BREAK: {
				tk = &tokenizer->next(); // eat "break"
				if (!parser_context.current_break) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use break outside a loop/switch.", tk->get_pos());
				ptr<ControlFlowNode> _break = new_node<ControlFlowNode>(ControlFlowNode::BREAK);
				_break->break_continue = parser_context.current_break;
				parser_context.current_break->has_break = true;
				_break->parernt_node = p_parent;
				block_node->statements.push_back(_break);
			} break;

			case Token::KWORD_CONTINUE: {
				tk = &tokenizer->next(); // eat "continue"
				if (!parser_context.current_continue) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use continue outside a loop.", tk->get_pos());
				ptr<ControlFlowNode> _continue = new_node<ControlFlowNode>(ControlFlowNode::CONTINUE);
				_continue->break_continue = parser_context.current_continue;
				parser_context.current_continue->has_continue = true;
				_continue->parernt_node = p_parent;
				block_node->statements.push_back(_continue);
			} break;

			case Token::KWORD_RETURN: {
				tk = &tokenizer->next(); // eat "return"
				if (!parser_context.current_func) throw PARSER_ERROR(Error::SYNTAX_ERROR, "can't use return outside a function.", tk->get_pos());
				if (parser_context.current_class && parser_context.current_class->constructor) {
					if (parser_context.current_class->constructor == parser_context.current_func) {
						throw PARSER_ERROR(Error::SYNTAX_ERROR, "constructor can't return anything.", tk->get_pos());
					}
				}
				ptr<ControlFlowNode> _return = new_node<ControlFlowNode>(ControlFlowNode::RETURN);
				if (tokenizer->peek().type != Token::SYM_SEMI_COLLON)  _return->args.push_back(_parse_expression(block_node, false));
				_return->parernt_node = p_parent;
				_return->_return = parser_context.current_func;
				if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
				parser_context.current_func->has_return = true;
				block_node->statements.push_back(_return);
			} break;

			default: {
				for (Token termination : p_termination) {
					if (tk->type == termination) {
						return block_node;
					}
				}
				ptr<Node> expr = _parse_expression(block_node, true);
				if (tokenizer->next().type != Token::SYM_SEMI_COLLON) throw UNEXP_TOKEN_ERROR("symbol \";\"");
				block_node->statements.push_back(expr);
			}
		}

		if (p_single_statement)
			break;
	}

	return block_node;
}


ptr<Parser::ControlFlowNode> Parser::_parse_if_block(const ptr<BlockNode>& p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_IF);

	ptr<ControlFlowNode> if_block = new_node<ControlFlowNode>(ControlFlowNode::IF);
	if_block->parernt_node = p_parent;
	ptr<Node> cond = _parse_expression(p_parent, false);
	if_block->args.push_back(cond);

	const TokenData* tk = &tokenizer->peek();
	if (tk->type == Token::BRACKET_LCUR) {
		tokenizer->next(); // eat "{"
		if_block->body = _parse_block(p_parent);
		if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
	} else {
		if_block->body = _parse_block(p_parent, true);
	}

	tk = &tokenizer->peek(0);
	while (tk->type == Token::KWORD_ELSE) {
		tokenizer->next(); // eat "else"
		tk = &tokenizer->peek(0);
		switch (tk->type) {
			case Token::KWORD_IF: {
				tokenizer->next(); // eat "if"
				if_block->body_else = new_node<BlockNode>();
				if_block->body_else->parernt_node = p_parent;
				if_block->body_else->statements.push_back(_parse_if_block(p_parent));
			} break;
			case Token::BRACKET_LCUR: {
				tokenizer->next(); // eat "{"
				if_block->body_else = _parse_block(p_parent);
				if (tokenizer->next().type != Token::BRACKET_RCUR) throw UNEXP_TOKEN_ERROR("symbol \"}\"");
			} break;
			default: {
				if_block->body_else = _parse_block(p_parent, true);
			}
		}
		tk = &tokenizer->peek();
	}

	return if_block;
}

} // namespace carbon

/******************************************************************************************************************/
/*                                         EXPRESSION                                                             */
/******************************************************************************************************************/

namespace carbon {

ptr<Parser::Node> Parser::_parse_expression(const ptr<Node>& p_parent, bool p_allow_assign) {
	p_allow_assign = true; // all expressions suport assignment now (test for any bugs)
	ASSERT(p_parent != nullptr);

	stdvec<Expr> expressions;

	while (true) {

		const TokenData* tk = &tokenizer->next();
		ptr<Node> expr = nullptr;

		if (tk->type == Token::BRACKET_LPARAN) {
			expr = _parse_expression(p_parent, false);

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_RPARAN) {
				throw UNEXP_TOKEN_ERROR("symbol \")\"");
			}

		} else if (tk->type == Token::KWORD_THIS) {
			if (parser_context.current_class == nullptr || (parser_context.current_func && parser_context.current_func->is_static) ||
				(parser_context.current_var && parser_context.current_var->is_static))
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "keyword \"this\" only be used in non-static member function.", Vect2i());
			if (tokenizer->peek().type == Token::BRACKET_LPARAN) { // super();
				tk = &tokenizer->next(); // eat "("
				ptr<CallNode> call = new_node<CallNode>();
				call->base = new_node<ThisNode>();
				call->method = nullptr;
				call->args = _parse_arguments(p_parent);
				expr = call;
			} else {
				expr = new_node<ThisNode>();
			}
		} else if (tk->type == Token::KWORD_SUPER) {
			// if super is inside class function, it calls the same function in it's base.
			if (parser_context.current_class == nullptr || (parser_context.current_func == nullptr))
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "keyword \"super\" can only be used in class function.", Vect2i());
			if (parser_context.current_class->base_type == ClassNode::NO_BASE) {
				throw PARSER_ERROR(Error::SYNTAX_ERROR, "invalid use of \"super\". Can only used inside classes with a base type.", Vect2i());
			}
			if (tokenizer->peek().type == Token::BRACKET_LPARAN) { // super();
				tk = &tokenizer->next(); // eat "("
				ptr<CallNode> call = new_node<CallNode>();
				call->base = new_node<SuperNode>();
				call->method = nullptr;
				call->args = _parse_arguments(p_parent);
				expr = call;
			} else {
				expr = new_node<SuperNode>();
			}

		} else if (tk->type == Token::VALUE_FLOAT || tk->type == Token::VALUE_INT || tk->type == Token::VALUE_STRING || tk->type == Token::VALUE_BOOL || tk->type == Token::VALUE_NULL) {
			expr = new_node<ConstValueNode>(tk->constant);

		} else if (tk->type == Token::OP_PLUS || tk->type == Token::OP_MINUS || tk->type == Token::OP_NOT || tk->type == Token::OP_BIT_NOT) {
			switch (tk->type) {
				case Token::OP_PLUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_POSITIVE, tokenizer->get_pos()));
					break;
				case Token::OP_MINUS:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NEGATIVE, tokenizer->get_pos()));
					break;
				case Token::OP_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_NOT, tokenizer->get_pos()));
					break;
				case Token::OP_BIT_NOT:
					expressions.push_back(Expr(OperatorNode::OpType::OP_BIT_NOT, tokenizer->get_pos()));
					break;
			}
			continue;
		} else if ((tk->type == Token::IDENTIFIER || tk->type == Token::BUILTIN_TYPE) && tokenizer->peek().type == Token::BRACKET_LPARAN) {
			ptr<CallNode> call = new_node<CallNode>();

			if (tk->type == Token::IDENTIFIER) {
				BuiltinFunctions::Type builtin_func = BuiltinFunctions::get_func_type(tk->identifier);
				if (builtin_func != BuiltinFunctions::UNKNOWN) {
					call->is_compilttime = BuiltinFunctions::is_compiletime(builtin_func);
					call->base = new_node<BuiltinFunctionNode>(builtin_func);
					call->method = nullptr;
				} else {
					// Identifier node could be builtin class like File(), another static method, ...
					// will know when reducing.
					call->base = new_node<Node>(); // UNKNOWN on may/may-not be self
					call->method = new_node<IdentifierNode>(tk->identifier);
				}
			} else {
				call->base = new_node<BuiltinTypeNode>(tk->builtin_type);
				call->method = nullptr;
			}

			tk = &tokenizer->next();
			if (tk->type != Token::BRACKET_LPARAN) throw UNEXP_TOKEN_ERROR("symbol \"(\"");
			call->args = _parse_arguments(p_parent);
			expr = call;

		} else if (tk->type == Token::IDENTIFIER) {
			BuiltinFunctions::Type bif_type = BuiltinFunctions::get_func_type(tk->identifier);
			if (bif_type != BuiltinFunctions::UNKNOWN) {
				ptr<BuiltinFunctionNode> bif = new_node<BuiltinFunctionNode>(bif_type);
				expr = bif;
			} else {
				ptr<IdentifierNode> id = new_node<IdentifierNode>(tk->identifier);
				id->declared_block = parser_context.current_block;
				expr = id;
			}

		} else if (tk->type == Token::BUILTIN_TYPE) { // String.format(...);
			ptr<BuiltinTypeNode> bt = new_node<BuiltinTypeNode>(tk->builtin_type);
			expr = bt;

		} else if (tk->type == Token::BRACKET_LSQ) {
			ptr<ArrayNode> arr = new_node<ArrayNode>();
			bool done = false;
			bool comma_valid = false;
			while (!done) {
				tk = &tokenizer->peek();
				switch (tk->type) {
					case Token::_EOF:
						tk = &tokenizer->next(); // eat eof
						throw UNEXP_TOKEN_ERROR(nullptr);
						break;
					case Token::SYM_COMMA:
						tk = &tokenizer->next(); // eat comma
						if (!comma_valid) {
							throw UNEXP_TOKEN_ERROR(nullptr);
						}
						comma_valid = false;
						break;
					case Token::BRACKET_RSQ:
						tk = &tokenizer->next(); // eat ']'
						done = true;
						break;
					default:
						if (comma_valid) throw UNEXP_TOKEN_ERROR("symbol \",\"");

						ptr<Node> subexpr = _parse_expression(p_parent, false);
						arr->elements.push_back(subexpr);
						comma_valid = true;
				}
			}
			expr = arr;

		} else if (tk->type == Token::BRACKET_LCUR) {
			ptr<MapNode> map = new_node<MapNode>();
			bool done = false;
			bool comma_valid = false;
			while (!done) {
				tk = &tokenizer->peek();
				switch (tk->type) {
					case Token::_EOF:
						tk = &tokenizer->next(); // eat eof
						throw UNEXP_TOKEN_ERROR(nullptr);
						break;
					case Token::SYM_COMMA:
						tk = &tokenizer->next(); // eat comma
						if (!comma_valid) throw UNEXP_TOKEN_ERROR(nullptr);
						comma_valid = false;
						break;
					case Token::BRACKET_RCUR:
						tk = &tokenizer->next(); // eat '}'
						done = true;
						break;
					default:
						if (comma_valid) throw UNEXP_TOKEN_ERROR("symbol \",\"");

						ptr<Node> key = _parse_expression(p_parent, false);
						tk = &tokenizer->next();
						if (tk->type != Token::SYM_COLLON) throw UNEXP_TOKEN_ERROR("symbol \":\"");
						ptr<Node> value = _parse_expression(p_parent, false);
						map->elements.push_back(Parser::MapNode::Pair(key, value));
						comma_valid = true;
				}
			}
			expr = map;
		} else {
			throw UNEXP_TOKEN_ERROR(nullptr);
		}

		// -- PARSE INDEXING -------------------------------------------------------

		while (true) {

			tk = &tokenizer->peek();
			// .named_index
			if (tk->type == Token::SYM_DOT) {
				tk = &tokenizer->next(1);

				if (tk->type != Token::IDENTIFIER) throw UNEXP_TOKEN_ERROR(nullptr);

				// call
				if (tokenizer->peek().type == Token::BRACKET_LPARAN) {
					ptr<CallNode> call = new_node<CallNode>();

					call->base = expr;
					call->method = new_node<IdentifierNode>(tk->identifier);
					tk = &tokenizer->next(); // eat "("
					call->args = _parse_arguments(p_parent);
					expr = call;

					// Just indexing.
				} else {
					ptr<IndexNode> ind = new_node<IndexNode>();
					ind->base = expr;
					ind->member = new_node<IdentifierNode>(tk->identifier);
					expr = ind;
				}


				// [mapped_index]
			} else if (tk->type == Token::BRACKET_LSQ) {
				ptr<MappedIndexNode> ind_mapped = new_node<MappedIndexNode>();

				tk = &tokenizer->next(); // eat "["
				ptr<Node> key = _parse_expression(p_parent, false);
				tk = &tokenizer->next();
				if (tk->type != Token::BRACKET_RSQ) {
					throw UNEXP_TOKEN_ERROR("symbol \"]\"");
				}

				ind_mapped->base = expr;
				ind_mapped->key = key;
				expr = ind_mapped;

				// get_func()(...);
			} else if (tk->type == Token::BRACKET_LPARAN) {
				ptr<CallNode> call = new_node<CallNode>();

				call->base = expr;
				call->method = nullptr;
				tk = &tokenizer->next(); // eat "("
				call->args = _parse_arguments(p_parent);
				expr = call;

			} else {
				break;
			}

		}

		expressions.push_back(Expr(expr));

		// -- PARSE OPERATOR -------------------------------------------------------
		tk = &tokenizer->peek();

		OperatorNode::OpType op;
		bool valid = true;

		switch (tk->type) {
		#define OP_CASE(m_op) case Token::m_op: op = OperatorNode::OpType::m_op; break
			OP_CASE(OP_EQ);
			OP_CASE(OP_EQEQ);
			OP_CASE(OP_PLUS);
			OP_CASE(OP_PLUSEQ);
			OP_CASE(OP_MINUS);
			OP_CASE(OP_MINUSEQ);
			OP_CASE(OP_MUL);
			OP_CASE(OP_MULEQ);
			OP_CASE(OP_DIV);
			OP_CASE(OP_DIVEQ);
			OP_CASE(OP_MOD);
			OP_CASE(OP_MOD_EQ);
			OP_CASE(OP_LT);
			OP_CASE(OP_LTEQ);
			OP_CASE(OP_GT);
			OP_CASE(OP_GTEQ);
			OP_CASE(OP_AND);
			OP_CASE(OP_OR);
			OP_CASE(OP_NOT);
			OP_CASE(OP_NOTEQ);
			OP_CASE(OP_BIT_NOT);
			OP_CASE(OP_BIT_LSHIFT);
			OP_CASE(OP_BIT_LSHIFT_EQ);
			OP_CASE(OP_BIT_RSHIFT);
			OP_CASE(OP_BIT_RSHIFT_EQ);
			OP_CASE(OP_BIT_OR);
			OP_CASE(OP_BIT_OR_EQ);
			OP_CASE(OP_BIT_AND);
			OP_CASE(OP_BIT_AND_EQ);
			OP_CASE(OP_BIT_XOR);
			OP_CASE(OP_BIT_XOR_EQ);
		#undef OP_CASE

			default: valid = false;
		}
		MISSED_ENUM_CHECK(OperatorNode::OpType::_OP_MAX_, 33);

		if (valid) {
			tokenizer->next(); // Eat peeked token.

			expressions.push_back(Expr(op, tokenizer->get_pos()));
		} else {
			break;
		}
	}

	ptr<Node> op_tree = _build_operator_tree(expressions);
	if (op_tree->type == Node::Type::OPERATOR) {
		if (!p_allow_assign && OperatorNode::is_assignment(ptrcast<OperatorNode>(op_tree)->op_type)) {
			throw PARSER_ERROR(Error::SYNTAX_ERROR, "assignment is not allowed inside expression.", op_tree->pos);
		}
	}
	return op_tree;

}

stdvec<ptr<Parser::Node>> Parser::_parse_arguments(const ptr<Node>& p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::BRACKET_LPARAN);

	const TokenData* tk = &tokenizer->peek();
	stdvec<ptr<Node>> args;

	if (tk->type == Token::BRACKET_RPARAN) {
		tokenizer->next(); // eat BRACKET_RPARAN
	} else {
		while (true) {

			ptr<Node> arg = _parse_expression(p_parent, false);
			args.push_back(arg);

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
				// pass
			} else if (tk->type == Token::BRACKET_RPARAN) {
				break;
			} else {
				throw UNEXP_TOKEN_ERROR(nullptr);
			}
		}
	}

	return args;
}

int Parser::_get_operator_precedence(OperatorNode::OpType p_op) {
	switch (p_op) {
		case OperatorNode::OpType::OP_NOT:
		case OperatorNode::OpType::OP_BIT_NOT:
		case OperatorNode::OpType::OP_POSITIVE:
		case OperatorNode::OpType::OP_NEGATIVE:
			return 0;
		case OperatorNode::OpType::OP_MUL:
		case OperatorNode::OpType::OP_DIV:
		case OperatorNode::OpType::OP_MOD:
			return 1;
		case OperatorNode::OpType::OP_PLUS:
		case OperatorNode::OpType::OP_MINUS:
			return 2;
		case OperatorNode::OpType::OP_BIT_LSHIFT:
		case OperatorNode::OpType::OP_BIT_RSHIFT:
			return 3;
		case OperatorNode::OpType::OP_LT:
		case OperatorNode::OpType::OP_LTEQ:
		case OperatorNode::OpType::OP_GT:
		case OperatorNode::OpType::OP_GTEQ:
			return 4;
		case OperatorNode::OpType::OP_EQEQ:
		case OperatorNode::OpType::OP_NOTEQ:
			return 5;
		case OperatorNode::OpType::OP_BIT_AND:
			return 6;
		case OperatorNode::OpType::OP_BIT_XOR:
			return 7;
		case OperatorNode::OpType::OP_BIT_OR:
			return 8;
		case OperatorNode::OpType::OP_AND:
			return 9;
		case OperatorNode::OpType::OP_OR:
			return 10;
		case OperatorNode::OpType::OP_EQ:
		case OperatorNode::OpType::OP_PLUSEQ:
		case OperatorNode::OpType::OP_MINUSEQ:
		case OperatorNode::OpType::OP_MULEQ:
		case OperatorNode::OpType::OP_DIVEQ:
		case OperatorNode::OpType::OP_MOD_EQ:
		case OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
		case OperatorNode::OpType::OP_BIT_AND_EQ:
		case OperatorNode::OpType::OP_BIT_XOR_EQ:
		case OperatorNode::OpType::OP_BIT_OR_EQ:
			return 11;
		default:
			ASSERT(false);
			return -1;
	}
	MISSED_ENUM_CHECK(OperatorNode::OpType::_OP_MAX_, 33);
}

ptr<Parser::Node> Parser::_build_operator_tree(stdvec<Expr>& p_expr) {
	ASSERT(p_expr.size() > 0);

	while (p_expr.size() > 1) {

		int next_op = -1;
		int min_precedence = 0xFFFFF;
		bool unary = false;

		for (int i = 0; i < (int)p_expr.size(); i++) {
			if (!p_expr[i].is_op()) {
				continue;
			}

			int precedence = _get_operator_precedence(p_expr[i].get_op());
			if (precedence < min_precedence) {
				min_precedence = precedence;
				next_op = i;
				OperatorNode::OpType op = p_expr[i].get_op();
				unary = (
					op == OperatorNode::OpType::OP_NOT ||
					op == OperatorNode::OpType::OP_BIT_NOT ||
					op == OperatorNode::OpType::OP_POSITIVE ||
					op == OperatorNode::OpType::OP_NEGATIVE);
			}
		}

		ASSERT(next_op >= 0);

		if (unary) {

			int next_expr = next_op;
			while (p_expr[next_expr].is_op()) {
				if (++next_expr == p_expr.size()) {
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "expected an expression.", Vect2i());
				}
			}

			for (int i = next_expr - 1; i >= next_op; i--) {
				ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[i].get_op());
				op_node->pos = p_expr[i].get_pos();
				op_node->args.push_back(p_expr[(size_t)i + 1].get_expr());
				p_expr.at(i) = Expr(op_node);
				p_expr.erase(p_expr.begin() + i + 1);
			}

		} else {
			ASSERT(next_op >= 1 && next_op < (int)p_expr.size() - 1);
			ASSERT(!p_expr[(size_t)next_op - 1].is_op() && !p_expr[(size_t)next_op + 1].is_op());

			ptr<OperatorNode> op_node = new_node<OperatorNode>(p_expr[(size_t)next_op].get_op());
			op_node->pos = p_expr[next_op].get_pos();

			if (p_expr[(size_t)next_op - 1].get_expr()->type == Node::Type::OPERATOR) {
				if (OperatorNode::is_assignment(ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->op_type)) {
					Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op - 1].get_expr())->pos;
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "unexpected assignment.", Vect2i(pos.x, pos.y));
				}
			}

			if (p_expr[(size_t)next_op + 1].get_expr()->type == Node::Type::OPERATOR) {
				if (OperatorNode::is_assignment(ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->op_type)) {
					Vect2i pos = ptrcast<OperatorNode>(p_expr[(size_t)next_op + 1].get_expr())->pos;
					throw PARSER_ERROR(Error::SYNTAX_ERROR, "unexpected assignment.", Vect2i(pos.x, pos.y));
				}
			}

			op_node->args.push_back(p_expr[(size_t)next_op - 1].get_expr());
			op_node->args.push_back(p_expr[(size_t)next_op + 1].get_expr());

			p_expr.at((size_t)next_op - 1) = Expr(op_node);
			p_expr.erase(p_expr.begin() + next_op);
			p_expr.erase(p_expr.begin() + next_op);
		}
	}
	ASSERT(!p_expr[0].is_op());
	return p_expr[0].get_expr();
}

} // namespace carbon


/******************************************************************************************************************/
/*                                         STRINGS                                                               */
/******************************************************************************************************************/

namespace carbon {
	
String TokenData::to_string() const {
	switch (type) {
		case Token::UNKNOWN: return "<unknown>";
		case Token::_EOF:    return "<eof>";

		case Token::SYM_DOT:         return ".";
		case Token::SYM_COMMA:       return ",";
		case Token::SYM_COLLON:      return ":";
		case Token::SYM_SEMI_COLLON: return ";";
		case Token::SYM_AT:          return "@";
		case Token::SYM_HASH:        return "#";
		case Token::SYM_DOLLAR:      return "$";
		case Token::SYM_QUESTION:    return "?";
		case Token::BRACKET_LPARAN:  return "(";
		case Token::BRACKET_RPARAN:  return ")";
		case Token::BRACKET_LCUR:    return "{";
		case Token::BRACKET_RCUR:    return "}";
		case Token::BRACKET_RSQ:     return "[";
		case Token::BRACKET_LSQ:     return "]";

		case Token::OP_EQ:      return "=";
		case Token::OP_EQEQ:    return "==";
		case Token::OP_PLUS:    return "+";
		case Token::OP_PLUSEQ:  return "+=";
		case Token::OP_MINUS:   return "-";
		case Token::OP_MINUSEQ: return "-=";
		case Token::OP_MUL:     return "*";
		case Token::OP_MULEQ:   return "*=";
		case Token::OP_DIV:     return "/";
		case Token::OP_DIVEQ:   return "/=";
		case Token::OP_MOD:     return "%";
		case Token::OP_MOD_EQ:  return "%=";
		case Token::OP_LT:      return "<";
		case Token::OP_LTEQ:    return "<=";
		case Token::OP_GT:      return ">";
		case Token::OP_GTEQ:    return ">=";
		case Token::OP_AND:     return "&&";
		case Token::OP_OR:      return "||";
		case Token::OP_NOT:     return "!";
		case Token::OP_NOTEQ:   return "!=";

		case Token::OP_BIT_NOT:       return "~";
		case Token::OP_BIT_LSHIFT:    return "<<";
		case Token::OP_BIT_LSHIFT_EQ: return "<<=";
		case Token::OP_BIT_RSHIFT:    return ">>";
		case Token::OP_BIT_RSHIFT_EQ: return ">>=";
		case Token::OP_BIT_OR:        return "|";
		case Token::OP_BIT_OR_EQ:     return "|=";
		case Token::OP_BIT_AND:       return "&";
		case Token::OP_BIT_AND_EQ:    return "&=";
		case Token::OP_BIT_XOR:       return "^";
		case Token::OP_BIT_XOR_EQ:    return "^=";

		case Token::IDENTIFIER:     return identifier;
		case Token::BUILTIN_TYPE:   return BuiltinTypes::get_type_name(builtin_type);

		case Token::KWORD_IMPORT:   return "import";
		case Token::KWORD_CLASS:    return "class";
		case Token::KWORD_ENUM:     return "enum";
		case Token::KWORD_FUNC:     return "func";
		case Token::KWORD_VAR:      return "var";
		case Token::KWORD_CONST:    return "const";
		case Token::KWORD_NULL:     return "null";
		case Token::KWORD_TRUE:     return "true";
		case Token::KWORD_FALSE:    return "false";
		case Token::KWORD_IF:       return "if";
		case Token::KWORD_ELSE:     return "else";
		case Token::KWORD_WHILE:    return "while";
		case Token::KWORD_FOR:      return "for";
		case Token::KWORD_SWITCH:   return "switch";
		case Token::KWORD_CASE:     return "case";
		case Token::KWORD_DEFAULT:  return "default";
		case Token::KWORD_BREAK:    return "break";
		case Token::KWORD_CONTINUE: return "continue";
		case Token::KWORD_STATIC:   return "static";
		case Token::KWORD_THIS:     return "this";
		case Token::KWORD_SUPER:    return "super";
		case Token::KWORD_RETURN:   return "return";
		case Token::KWORD_AND:      return "and";
		case Token::KWORD_OR:       return "or";
		case Token::KWORD_NOT:      return "not";
			
		case Token::VALUE_NULL:     return "null";
		case Token::VALUE_STRING: 
			return String("\"") + constant.operator String() + "\"";
		case Token::VALUE_INT: 
		case Token::VALUE_FLOAT: 
			return constant.to_string();
		case Token::VALUE_BOOL:
			return (constant.operator bool()) ? "true" : "false";

		case Token::_TK_MAX_: return "<_TK_MAX_>";
	}
	THROW_BUG(String::format("missed enum in switch case."));
MISSED_ENUM_CHECK(Token::_TK_MAX_, 79);
}


const char* Tokenizer::get_token_name(Token p_tk) {
	static const char* token_names[] = {
		"UNKNOWN",
		"_EOF",

		"SYM_DOT",
		"SYM_COMMA",
		"SYM_COLLON",
		"SYM_SEMI_COLLON",
		"SYM_AT",
		"SYM_HASH",
		"SYM_DOLLAR",
		"SYM_QUESTION",
		"BRACKET_LPARAN",
		"BRACKET_RPARAN",
		"BRACKET_LCUR",
		"BRACKET_RCUR",
		"BRACKET_RSQ",
		"BRACKET_LSQ",

		"OP_EQ",
		"OP_EQEQ",
		"OP_PLUS",
		"OP_PLUSEQ",
		"OP_MINUS",
		"OP_MINUSEQ",
		"OP_MUL",
		"OP_MULEQ",
		"OP_DIV",
		"OP_DIVEQ",
		"OP_MOD",
		"OP_MOD_EQ",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_NOTEQ",

		"OP_BIT_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_LSHIFT_EQ",
		"OP_BIT_RSHIFT",
		"OP_BIT_RSHIFT_EQ",
		"OP_BIT_OR",
		"OP_BIT_OR_EQ",
		"OP_BIT_AND",
		"OP_BIT_AND_EQ",
		"OP_BIT_XOR",
		"OP_BIT_XOR_EQ",

		"IDENTIFIER",
		"BUILTIN_TYPE",

		"KWORD_IMPORT",
		"KWORD_CLASS",
		"KWORD_ENUM",
		"KWORD_FUNC",
		"KWORD_VAR",
		"KWORD_CONST",
		"KWORD_NULL",
		"KWORD_TRUE",
		"KWORD_FALSE",
		"KWORD_IF",
		"KWORD_ELSE",
		"KWORD_WHILE",
		"KWORD_FOR",
		"KWORD_SWITCH",
		"KWORD_CASE",
		"KWORD_DEFAULT",
		"KWORD_BREAK",
		"KWORD_CONTINUE",
		"KWORD_STATIC",
		"KWORD_THIS",
		"KWORD_SUPER",
		"KWORD_RETURN",
		"KWORD_AND",
		"KWORD_OR",
		"KWORD_NOT",

		"VALUE_NULL",
		"VALUE_STRING",
		"VALUE_INT",
		"VALUE_FLOAT",
		"VALUE_BOOL",
		nullptr, //_TK_MAX_
	};
	return token_names[(int)p_tk];
MISSED_ENUM_CHECK(Token::_TK_MAX_, 79);
}

const char* Parser::Node::get_node_type_name(Type p_type) {
	static const char* type_names[] = {
		"UNKNOWN",
		"FILE",
		"CLASS",
		"ENUM",
		"FUNCTION",
		"BLOCK",
		"IDENTIFIER",
		"VAR",
		"CONST",
		"CONST_VALUE",
		"ARRAY",
		"MAP",
		"THIS",
		"SUPER",
		"BUILTIN_FUNCTION",
		"BUILTIN_TYPE",
		"CALL",
		"INDEX",
		"MAPPED_INDEX",
		"OPERATOR",
		"CONTROL_FLOW",
		nullptr, // _NODE_MAX_
	};
	return type_names[(int)p_type];
MISSED_ENUM_CHECK(Parser::Node::Type::_NODE_MAX_, 22);
}

const char* Parser::OperatorNode::get_op_name(OpType p_op) {
	static const char* op_names[] = {

		"OP_EQ",
		"OP_EQEQ",
		"OP_PLUS",
		"OP_PLUSEQ",
		"OP_MINUS",
		"OP_MINUSEQ",
		"OP_MUL",
		"OP_MULEQ",
		"OP_DIV",
		"OP_DIVEQ",
		"OP_MOD",
		"OP_MOD_EQ",
		"OP_LT",
		"OP_LTEQ",
		"OP_GT",
		"OP_GTEQ",
		"OP_AND",
		"OP_OR",
		"OP_NOT",
		"OP_NOTEQ",

		"OP_BIT_NOT",
		"OP_BIT_LSHIFT",
		"OP_BIT_LSHIFT_EQ",
		"OP_BIT_RSHIFT",
		"OP_BIT_RSHIFT_EQ",
		"OP_BIT_OR",
		"OP_BIT_OR_EQ",
		"OP_BIT_AND",
		"OP_BIT_AND_EQ",
		"OP_BIT_XOR",
		"OP_BIT_XOR_EQ",

		"OP_POSITIVE",
		"OP_NEGATIVE",

		nullptr, // _OP_MAX_
	};
	return op_names[p_op];
MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 33);
}

const char* Parser::ControlFlowNode::get_cftype_name(CfType p_type) {
	static const char* cf_names[] = {
		"IF",
		"SWITCH",
		"WHILE",
		"FOR",
		"FOREACH",
		"BREAK",
		"CONTINUE",
		"RETURN",
		nullptr, // _CF_MAX_
	};
	return cf_names[p_type];

MISSED_ENUM_CHECK(Parser::ControlFlowNode::_CF_MAX_, 8);
}


stdmap<BuiltinFunctions::Type, String> BuiltinFunctions::_func_list = {

	{ BuiltinFunctions::__ASSERT,  "__assert" },
	{ BuiltinFunctions::__FUNC,    "__func"   },
	{ BuiltinFunctions::__LINE,    "__line"   },
	{ BuiltinFunctions::__FILE,    "__file"   },

	{ BuiltinFunctions::PRINT,     "print"    },
	{ BuiltinFunctions::PRINTLN,   "println"  },
	{ BuiltinFunctions::INPUT,     "input"    },
	{ BuiltinFunctions::HEX,       "hex"      },
	{ BuiltinFunctions::BIN,       "bin"      },
	{ BuiltinFunctions::MATH_MIN,  "min"      },
	{ BuiltinFunctions::MATH_MAX,  "max"      },
	{ BuiltinFunctions::MATH_POW,  "pow"      },

};
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 13);

stdmap<BuiltinTypes::Type, String> BuiltinTypes::_type_list = {
	//{ "", BuiltinTypes::UNKNOWN    },
	//{ "", BuiltinTypes::_TYPE_MAX_ },

	{ BuiltinTypes::_NULL,  "null",      },
	{ BuiltinTypes::BOOL,   "bool",      },
	{ BuiltinTypes::INT,    "int",       },
	{ BuiltinTypes::FLOAT,  "float",     },
	{ BuiltinTypes::STRING, "String",    },
	{ BuiltinTypes::STR,    "str",       },
	{ BuiltinTypes::ARRAY,  "Array",     },
	{ BuiltinTypes::MAP,    "Map",       },

};
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 9);

} // namespace carbon
