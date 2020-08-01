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

namespace carbon {

void Parser::parse(String p_source, String p_file_path) {

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
				// TODO:
				break;
			}
			case Token::KWORD_CLASS: {
				file_node->classes.push_back(_parse_class());
				break;
			}
			case Token::KWORD_ENUM: {
				file_node->enums.push_back(_parse_enum());
				break;
			}
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
			// Ignore.
			case Token::SYM_SEMI_COLLON: 
			case Token::VALUE_STRING:
				break;
			default:
				THROW_UNEXP_TOKEN("");
		}

	} // while true

}

ptr<Parser::ClassNode> Parser::_parse_class() {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_CLASS);
	ptr<ClassNode> class_node = new_node<ClassNode>();

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
	ScopeDestruct distruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();

	if (tk->type != Token::IDENTIFIER) {
		THROW_UNEXP_TOKEN("an identifier");
	}
	// TODO: check identifier predefined.
	class_node->name = tk->identifier;

	tk = &tokenizer->next();

	if (tk->type == Token::SYM_COLLON) {
		const TokenData& base = tokenizer->next();
		// TODO: base could be builtin class (Object, File, Map)
		if (base.type != Token::IDENTIFIER) {
			THROW_UNEXP_TOKEN("an identifier");
		}
		// TODO: check identifier predefined.
		class_node->base = base.identifier;

		tk = &tokenizer->next();
	}

	if (tk->type != Token::BRACKET_LCUR) {
		THROW_UNEXP_TOKEN("symbol \"{\"");
	}
	
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
				_parse_enum(class_node);
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
					class_node->members.push_back(_var);
				}
			} break;

			default: {
				THROW_UNEXP_TOKEN("");
			}
		}
	}
}

ptr<Parser::EnumNode> Parser::_parse_enum(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_ENUM);

	ptr<EnumNode> enum_node = new_node<EnumNode>();
	int64_t cur_value = -1;

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER && tk->type != Token::BRACKET_LCUR)
		THROW_UNEXP_TOKEN("an identifier or symbol \"{\"");	

	if (tk->type == Token::IDENTIFIER) {
		// TODO: identifier check.
		enum_node->name = tk->identifier;
		enum_node->named_enum = true;
		tk = &tokenizer->next();
	}

	if (tk->type != Token::BRACKET_LCUR) THROW_UNEXP_TOKEN("symbol \"{\"");

	bool comma_valid = false;
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
				// TODO: check identifier with class name, enum name, static var name, and from import, import-> import , ...
				for (const std::pair<String, int64_t>& value : enum_node->values) {
					if (value.first == token.identifier) {
						THROW_PARSER_ERR(Error::ALREADY_DEFINED, String::format("enum name \"%s\" is already defined", token.identifier.c_str()), Vect2i());
					}
				}
				enum_node->values[token.identifier] = ++cur_value;
				
				const TokenData* tk = &tokenizer->peek();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next(1);
					// TODO: this could be a constant expression evaluvated to int.
					if (tk->type != Token::VALUE_INT) {
						THROW_UNEXP_TOKEN("an integer constant");
					}
					ASSERT(tk->constant.get_type() == var::INT);
					cur_value = tk->constant.operator int64_t();
					enum_node->values[token.identifier] = cur_value;
				}

				comma_valid = true;
			} break;

			default: {
				THROW_UNEXP_TOKEN("an identifier");
			} break;
		}
	}
}

stdvec<ptr<Parser::VarNode>> Parser::_parse_var(ptr<Node> p_node) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_VAR);
	ASSERT(p_node != nullptr);
	ASSERT(p_node->type == Node::Type::FILE || p_node->type == Node::Type::BLOCK || p_node->type == Node::Type::CLASS);

	bool _static = tokenizer->peek(-2, true).type == Token::KWORD_STATIC;

	const TokenData* tk;
	stdvec<ptr<VarNode>> vars;

	while (true) {
		tk = &tokenizer->next();
		if (tk->type != Token::IDENTIFIER) {
			THROW_UNEXP_TOKEN("an identifier");
		}
		ptr<VarNode> var_node = new_node<VarNode>();
		var_node->is_static = _static;
		var_node->name = tk->identifier;

		if (parser_context.current_func) {
			stdvec<String>* args = &parser_context.current_func->args;
			if (std::find(args->begin(), args->end(), var_node->name) != args->end()) {
				THROW_PARSER_ERR(Error::ALREADY_DEFINED, 
					String::format("identifier \"%s\" already defined in arguments", var_node->name.c_str()), Vect2i());
			}
		}
		// TODO: identifier check.

		tk = &tokenizer->next();
		if (tk->type == Token::OP_EQ) {
			ptr<Node> expr = _parse_expression(p_node);
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

ptr<Parser::FunctionNode> Parser::_parse_func(ptr<Node> p_parent) {
	ASSERT(tokenizer->peek(-1).type == Token::KWORD_FUNC);

	ptr<FunctionNode> func_node = new_node<FunctionNode>();
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
	ScopeDestruct distruct = ScopeDestruct(&parser_context);

	const TokenData* tk = &tokenizer->next();
	if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
	func_node->name = tk->identifier;

	tk = &tokenizer->next();
	if (tk->type != Token::BRACKET_LPARAN) THROW_UNEXP_TOKEN("symbol \"(\"");
	tk = &tokenizer->next();

	if (tk->type != Token::BRACKET_RPARAN) {
		while (true) {
			if (tk->type != Token::IDENTIFIER) THROW_UNEXP_TOKEN("an identifier");
			if (std::find(func_node->args.begin(), func_node->args.end(), tk->identifier) != func_node->args.end()) {
				THROW_PARSER_ERR(Error::ALREADY_DEFINED, 
					String::format("identifier \"%s\" already defined in arguments", tk->identifier.c_str()), Vect2i());
			}
			// TODO: identifier shadow check.
			func_node->args.push_back(tk->identifier);

			tk = &tokenizer->next();
			if (tk->type == Token::SYM_COMMA) {
				tk = &tokenizer->next();
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

	ptr<BlockNode> body = _parse_block(func_node);
	if (tokenizer->peek(-1).type != Token::BRACKET_RCUR) {
		THROW_UNEXP_TOKEN("symbol \"}\"");
	}
	func_node->body = body;
	return func_node;
}

// -----------------------------------------------------------------------------

#if DEBUG_BUILD
// properly implement this.
#define KEYWORD_COLOR Logger::Color::L_YELLOW
#define TYPE_COLOR Logger::Color::L_GREEN

#define PRINT_INDENT(m_indent) for (int i = 0; i < m_indent; i++)  printf("    ")
#define PRINT_COLOR(m_log, m_color) Logger::log(m_log, Logger::VERBOSE, m_color)

static void print_var_node(const Parser::VarNode* p_var, int p_indent) {
	PRINT_INDENT(p_indent);
	if (p_var->is_static) {
		PRINT_COLOR(TokenData(Token::KWORD_STATIC).to_string().c_str(), KEYWORD_COLOR); printf(" ");
	}
	PRINT_COLOR(TokenData(Token::KWORD_VAR).to_string().c_str(), KEYWORD_COLOR);
	printf(" %s = ", p_var->name.c_str());
	if (p_var->assignment != nullptr) PRINT_COLOR(" [expr]\n", Logger::Color::L_GRAY);
	else PRINT_COLOR(" nullptr\n", Logger::Color::L_GRAY);
}

static void print_enum_node(const Parser::EnumNode* p_enum, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR("enum", KEYWORD_COLOR);
	if (p_enum->named_enum) PRINT_COLOR(String(String(" ") + p_enum->name + "\n").c_str(), TYPE_COLOR);
	else PRINT_COLOR(" [not named]\n", Logger::Color::L_GRAY);

	for (const std::pair<String, int64_t>& value : p_enum->values) {
		PRINT_INDENT(p_indent + 1);
		printf("%s = %lli\n", value.first.c_str(), value.second);
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
		printf("%s", p_func->args[j].c_str());
	}
	printf(")\n");
}

static void print_class_node(Parser::ClassNode* p_class, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR(TokenData(Token::KWORD_CLASS).to_string().c_str(), KEYWORD_COLOR);
	PRINT_COLOR((String(" ") + p_class->name).c_str(), TYPE_COLOR);
	if (p_class->base != "") {
		printf(" inherits ");
		PRINT_COLOR((p_class->base + "\n").c_str(), TYPE_COLOR);
	}

	for (int i = 0; i < (int)p_class->enums.size(); i++) {
		print_enum_node(p_class->enums[i].get(), p_indent + 1);
	}

	for (int i = 0; i < (int)p_class->members.size(); i++) {
		print_var_node(p_class->members[i].get(), p_indent + 1);
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
	print_file_node(file_node.get(), 0);
}
#endif

}