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
MISSED_ENUM_CHECK(Token::_TK_MAX_, 75);
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

		"VALUE_STRING",
		"VALUE_INT",
		"VALUE_FLOAT",
		"VALUE_BOOL",
		nullptr, //_TK_MAX_
	};
	return token_names[(int)p_tk];
MISSED_ENUM_CHECK(Token::_TK_MAX_, 75);
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
MISSED_ENUM_CHECK(Parser::Node::Type::_NODE_MAX_, 21);
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
		"BREAK",
		"CONTINUE",
		"RETURN",
		nullptr, // _CF_MAX_
	};
	return cf_names[p_type];

MISSED_ENUM_CHECK(Parser::ControlFlowNode::_CF_MAX_, 7);
}


stdmap<BuiltinFunctions::Type, String> BuiltinFunctions::_func_list = {

	{ BuiltinFunctions::__ASSERT,  "__assert" },
	{ BuiltinFunctions::__FUNC,    "__func"   },
	{ BuiltinFunctions::__LINE,    "__line"   },
	{ BuiltinFunctions::__FILE,    "__file"   },

	{ BuiltinFunctions::PRINT,     "print"    },
	{ BuiltinFunctions::INPUT,     "input"    },
	{ BuiltinFunctions::MATH_MIN,  "min"      },
	{ BuiltinFunctions::MATH_MAX,  "max"      },
	{ BuiltinFunctions::MATH_POW,  "pow"      },

};
MISSED_ENUM_CHECK(BuiltinFunctions::Type::_FUNC_MAX_, 10);

stdmap<BuiltinTypes::Type, String> BuiltinTypes::_type_list = {
	//{ "", BuiltinTypes::UNKNOWN    },
	//{ "", BuiltinTypes::_TYPE_MAX_ },

	{ BuiltinTypes::_NULL,  "null",      },
	{ BuiltinTypes::BOOL,   "bool",      },
	{ BuiltinTypes::INT,    "int",       },
	{ BuiltinTypes::FLOAT,  "float",     },
	{ BuiltinTypes::STRING, "String",    },
	{ BuiltinTypes::ARRAY,  "Array",     },
	{ BuiltinTypes::MAP,    "Map",       },

};
MISSED_ENUM_CHECK(BuiltinTypes::Type::_TYPE_MAX_, 8);



}