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

#include "analyzer.h"
#include "compiler.h"
#include "native/path.h"

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
	return CompileTimeError(p_type, p_msg, DBGSourceInfo(file_node->path, file_node->source, pos, err_len), p_dbg_info);
}

CompileTimeError Parser::_predefined_error(const String& p_what, const String& p_name, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const {
	return _parser_error(Error::NAME_ERROR,
		String::format((p_what + " named \"%s\" already exists at (line:%i, col:%i)").c_str(), p_name.c_str(), p_pos.x, p_pos.y),
		Vect2i(), p_dbg_info);
}

void Parser::parse(const String& p_source, const String& p_file_path) {
	
	tokenizer = newptr<Tokenizer>();
	file_node = new_node<FileNode>();
	file_node->source = p_source;
	file_node->path = Path::absolute(p_file_path);

	tokenizer->tokenize(file_node->source, file_node->path);

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

// -----------------------------------------------------------------------------

#if DEBUG_BUILD
// properly implement this.
#define KEYWORD_COLOR Console::Color::L_YELLOW
#define TYPE_COLOR Console::Color::L_GREEN

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
			PRINT_COLOR(String::format("[expr:%i]\n", _print_id).c_str(), Console::Color::L_GRAY);
	}
	else PRINT_COLOR("nullptr\n", Console::Color::L_GRAY);
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
				PRINT_COLOR(String::format("[Array:%i]\n", _print_id).c_str(), Console::Color::L_GRAY);
			} break;
			//case Parser::Node::Type::MAP: {
			//	PRINT_INDENT(p_indent);
			//	PRINT_COLOR("[MAP]\n", Console::Color::L_GRAY);
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
				PRINT_COLOR(String::format("[%s:%i]\n", op_name, _print_id).c_str(), Console::Color::L_GRAY);
			} break;

			case Parser::Node::Type::CONTROL_FLOW: {
				PRINT_INDENT(p_indent);
				Parser::ControlFlowNode* dbg = ptrcast<Parser::ControlFlowNode>(node).get();
				const char* cf_name = Parser::ControlFlowNode::get_cftype_name(ptrcast<Parser::ControlFlowNode>(node)->cf_type);
				PRINT_COLOR(String::format("[%s:%i]\n", cf_name, _print_id).c_str(), Console::Color::L_GRAY);
			} break;
		}
	}
}

static void print_enum_node(const Parser::EnumNode* p_enum, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR("enum", KEYWORD_COLOR);
	if (p_enum->named_enum) PRINT_COLOR(String(String(" ") + p_enum->name + "\n").c_str(), TYPE_COLOR);
	else PRINT_COLOR(" [not named]\n", Console::Color::L_GRAY);

	for (const std::pair<String, Parser::EnumValueNode>& value : p_enum->values) {
		PRINT_INDENT(p_indent + 1);
		printf("%s = ", value.first.c_str());
		//if (value.second != nullptr) {
		//	if (value.second->type == Parser::Node::Type::CONST_VALUE) {
		//		ptr<Parser::ConstValueNode> enum_val = ptrcast<Parser::ConstValueNode>(value.second);
		//		ASSERT(enum_val->value.get_type() == var::INT);
		//		printf("%lli\n", enum_val->value.operator int64_t());
		//	} else {
		//		PRINT_COLOR("[TODO: reduce expression]\n", Console::Color::L_GRAY);
		//	}
		//} else {
		//	PRINT_COLOR("[TODO: auto increase]\n", Console::Color::L_GRAY);
		//}
		printf("%li\n", value.second.value);
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
			PRINT_COLOR(p_class->base_class_name.c_str(), TYPE_COLOR);
		} else { // BASE_EXTERN
			//PRINT_COLOR(p_class->base_file_name.c_str(), TYPE_COLOR);
			//printf(".");
			//PRINT_COLOR(p_class->base_class_name.c_str(), TYPE_COLOR);
			PRINT_COLOR("[TODO]", TYPE_COLOR);
			
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