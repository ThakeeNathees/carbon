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

#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

namespace carbon {

#define THROW_PARSER_ERR(m_err_type, m_msg, m_line)                                                                                       \
	if (m_line > 0) {                                                                                                                     \
		throw Error(m_err_type, String(m_msg), Vect2i(m_line, 0));                                                                        \
	} else {                                                                                                                              \
		throw Error(m_err_type, String(m_msg), Vect2i(tokenizer->get_line(), 0));                                                         \
	}

#define THROW_UNEXP_TOKEN(m_tk)                                                                                                           \
	if (m_tk != "") {                                                                                                                     \
		THROW_PARSER_ERR(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\"). expected \"%s\"", "<tk_name>", m_tk), -1);        \
	} else {                                                                                                                              \
		THROW_PARSER_ERR(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\").", "<tk_name>"), -1);                              \
	}

#define THROW_IF_ALREADY_FOUND(m_identifier, m_node)                                                                                      \
	do {                                                                                                                                  \
		IdentifierLocation loc = _find_identifier_location(m_identifier, m_node);                                                         \
		if (loc.found) {                                                                                                                  \
			THROW_PARSER_ERR(Error::ALREADY_DEFINED, String::format("Identifier %s already defined at %s:%i", loc.file_path, loc.line));  \
		}                                                                                                                                 \
	} while (false)


class Parser {
public:
	
	struct Node {
		enum class Type {
			UNKNOWN,

			FILE,
			CLASS,
			ENUM,
			FUNCTION,
			BLOCK,
			IDENTIFIER,
			VAR,
			CONST_VALUE,
			ARRAY,
			MAP,
			THIS,
			SUPER,
			BUILTIN_FUNCTION,
			BUILTIN_CLASS,
			OPERATOR,
			CONTROL_FLOW,
		};
		Type type;
		int line, col;
		ptr<Node> parernt_node;
	};

	struct ClassNode;
	struct EnumNode;
	struct BuiltinFunctionNode;
	struct BuiltinClassNode;
	struct FunctionNode;
	struct BlockNode;
	struct IdentifierNode;
	struct VarNode;
	struct ConstValueNode;
	struct ArrayNode;
	struct MapNode;
	struct OperatorNode;
	struct ControlFlowNode;

	struct FileNode : public Node {
		String path;
		stdvec<ptr<FileNode>> imports;
		stdvec<ptr<VarNode>> file_vars;
		stdvec<ptr<ClassNode>> classes;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<FunctionNode>> functions;

		FileNode() {
			type = Type::FILE;
		}

	};

	struct ClassNode : public Node {
		String name;
		String base;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<FunctionNode>> functions;
		stdvec<ptr<FunctionNode>> static_functions;
		stdvec<ptr<VarNode>> members;
		stdvec<ptr<VarNode>> static_members;
		ClassNode() {
			type = Type::CLASS;
		}
	};

	struct EnumNode : public Node {
		String name;
		std::map<String, int> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct FunctionNode : public Node {
		String name;
		bool is_static = true; // All functions are static by default.
		stdvec<String> args;
		ptr<BlockNode> body;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : public Node {
		stdvec<ptr<Node>> statements;
		stdvec<ptr<VarNode>> local_vars;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : public Node {
		String name;
		IdentifierNode() {
			type = Type::IDENTIFIER;
		}
		IdentifierNode(const String& p_name) {
			type = Type::IDENTIFIER;
			name = p_name;
		}

	};

	struct VarNode : public Node {
		String name;
		// all variables are static by default.
		// but static variables are only allows inside class
		// not inside any scope.
		bool is_static = true; 
		ptr<Node> assignment;
		VarNode() {
			type = Type::VAR;
		}
	};

	struct ConstValueNode : public Node {
		var value;
		ConstValueNode() {
			type = Type::CONST_VALUE;
		}
		ConstValueNode(const var& p_value) {
			type = Type::CONST_VALUE;
			value = p_value;
		}
	};

	struct ArrayNode : public Node {
		stdvec<ptr<Node>> elements;
		ArrayNode() {
			type = Type::ARRAY;
		}
	};

	struct MapNode : public Node {
		struct Pair {
			ptr<Node> key;
			ptr<Node> value;
		};
		stdvec<Pair> elements;
		MapNode() {
			type = Type::MAP;
		}
	};

	struct ThisNode : public Node {
		ThisNode() {
			type = Node::Type::THIS;
		}
	};

	struct SuperNode : public Node {
		SuperNode() {
			type = Node::Type::SUPER;
		}
	};

	struct BuiltinFunctionNode : public Node {
		BuiltinFunctions::Function func;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
		BuiltinFunctionNode(BuiltinFunctions::Function p_func) {
			type = Type::BUILTIN_FUNCTION;
			func = p_func;
		}
	};

	struct BuiltinClassNode : public Node {
		BuiltinClasses::Class cls;
		BuiltinClassNode() {
			type = Type::BUILTIN_CLASS;
		}
		BuiltinClassNode(BuiltinClasses::Class p_cls) {
			type = Type::BUILTIN_CLASS;
			cls = p_cls;
		}
	};



	struct OperatorNode : public Node {
		enum class OpType {
			OP_CALL,
			OP_INDEX,
			OP_INDEX_MAPPED,
			
			OP_EQ,
			OP_EQEQ,
			OP_PLUS,
			OP_PLUSEQ,
			OP_MINUS,
			OP_MINUSEQ,
			OP_MUL,
			OP_MULEQ,
			OP_DIV,
			OP_DIVEQ,
			OP_MOD,
			OP_MOD_EQ,
			OP_INCR,
			OP_DECR,
			OP_LT,
			OP_LTEQ,
			OP_GT,
			OP_GTEQ,
			OP_AND,
			OP_OR,
			OP_NOT,
			OP_NOTEQ,

			OP_BIT_NOT,
			OP_BIT_LSHIFT,
			OP_BIT_LSHIFT_EQ,
			OP_BIT_RSHIFT,
			OP_BIT_RSHIFT_EQ,
			OP_BIT_OR,
			OP_BIT_OR_EQ,
			OP_BIT_AND,
			OP_BIT_AND_EQ,
			OP_BIT_XOR,
			OP_BIT_XOR_EQ,
		};
		OpType op_type;
		stdvec<ptr<Node>> args;
		OperatorNode() {
			type = Type::OPERATOR;
		}
		OperatorNode(OpType p_type) {
			type = Type::OPERATOR;
			op_type = p_type;
		}
	};

	struct ControlFlowNode : public Node {
		enum class CfType {
			IF,
			FOR,      // for ( var i = 0; i < 10; i++ ) { }
			FOR_EACH, // for ( var i : a_list ) { }
			WHILE,
			BREAK,
			CONTINUE,
			RETURN,
		};
		CfType cf_type;
		stdvec<ptr<Node>> args;
		ptr<BlockNode> body;
		ptr<BlockNode> body_else; // for cf if node
		ControlFlowNode() {
			type = Type::CONTROL_FLOW;
		}
	};

	// Methods.
	void parse(String p_source, String p_file_path);

protected:

private:
	struct IdentifierLocation {
		bool found = false;
		int line = 0, col = 0;
		String file_path;
		Node::Type type = Node::Type::UNKNOWN;
		IdentifierLocation() {}
		IdentifierLocation(const ptr<Node>& p_node, const String& p_file_path) {
			found = true;
			file_path = p_file_path;
			line = p_node->line;
			col = p_node->col;
			type = p_node->type;
		}
	};

	// Methods.
	//void _throw(Error::Type p_type, const String& p_msg, int line = -1);
	//void _throw_unexp_token(const String& p_exp = "");

	template<typename T=Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		ret->line = tokenizer->get_line();
		ret->col = tokenizer->get_col();
		return ret;
	}

	IdentifierLocation _find_identifier_location(const String& p_name, const ptr<Node> p_node) const;

	ptr<ClassNode> _parse_class();
	ptr<EnumNode> _parse_enum(ptr<Node> p_parent = nullptr);
	stdvec<ptr<VarNode>> _parse_var(ptr<Node> p_parent, bool p_static);
	ptr<FunctionNode> _parse_func(ptr<Node> p_parent, bool p_static);
	ptr<BlockNode> _parse_block(const ptr<Node>& p_parent);
	ptr<Node> _parse_expression(const ptr<Node>& p_parent, bool p_static);
	void _parse_arguments(stdvec<ptr<Node>>& p_args, const ptr<Node>& p_parent, bool p_static);
	void _reduce_expression(ptr<Node>& p_expr);

	// Members.
	String file_path;
	String source;
	ptr<FileNode> file_node;
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();


};


}
#endif // PARSER_H