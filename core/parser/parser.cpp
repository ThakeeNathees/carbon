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

		while (true) {
			const TokenData* base = &tokenizer->next();

			// TODO: base could be builtin class (Object, File)
			if (base->type != Token::IDENTIFIER) {
				THROW_UNEXP_TOKEN("an identifier");
			}
			// TODO: check identifier predefined.
			class_node->inherits.push_back(base->identifier);

			base = &tokenizer->peek();
			if (base->type == Token::SYM_DOT) tokenizer->next(); // eat "."
			else break;			
		}

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
				class_node->enums.push_back(_parse_enum(class_node));
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
				for (const std::pair<String, ptr<Node>>& value : enum_node->values) {
					if (value.first == token.identifier) {
						THROW_PARSER_ERR(Error::ALREADY_DEFINED, String::format("enum name \"%s\" is already defined", token.identifier.c_str()), Vect2i());
					}
				}
				
				const TokenData* tk = &tokenizer->peek();
				if (tk->type == Token::OP_EQ) {
					tk = &tokenizer->next(); // eat "=".
					enum_node->values[token.identifier] = _parse_expression(enum_node, false);
				} else {
					enum_node->values[token.identifier] = nullptr;
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
			ptr<Node> expr = _parse_expression(p_node, false);
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

	for (const std::pair<String, ptr<Parser::Node>>& value : p_enum->values) {
		PRINT_INDENT(p_indent + 1);
		printf("%s = ", value.first.c_str());
		if (value.second != nullptr) {
			if (value.second->type == Parser::Node::Type::CONST_VALUE) {
				ptr<Parser::ConstValueNode> enum_val = ptrcast<Parser::ConstValueNode>(value.second);
				ASSERT(enum_val->value.get_type() == var::INT);
				printf("%lli\n", enum_val->value.operator int64_t());
			} else {
				PRINT_COLOR("[TODO: reduce expression]\n", Logger::Color::L_GRAY);
			}
		} else {
			PRINT_COLOR("[TODO: auto increase]\n", Logger::Color::L_GRAY);
		}
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

	print_block_node(p_func->body.get(), p_indent + 1);
}

static void print_class_node(Parser::ClassNode* p_class, int p_indent) {
	PRINT_INDENT(p_indent);
	PRINT_COLOR(TokenData(Token::KWORD_CLASS).to_string().c_str(), KEYWORD_COLOR);
	PRINT_COLOR((String(" ") + p_class->name).c_str(), TYPE_COLOR);
	if (p_class->inherits.size() != 0) {
		printf(" inherits ");
		for (int i = 0; i < (int)p_class->inherits.size(); i++) {
			if (i > 0) printf(".");
			PRINT_COLOR(p_class->inherits[i].c_str(), TYPE_COLOR);
		} 
	}
	printf("\n");

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
	_print_id = 0;
	print_file_node(file_node.get(), 0);
}
#endif

}