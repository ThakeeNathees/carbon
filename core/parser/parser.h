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

#define THROW_PARSER_ERR(m_err_type, m_msg, m_pos)                                                                         \
	do {                                                                                                                   \
		uint32_t err_len = 1;                                                                                              \
		String token_str = "";                                                                                             \
		if (m_pos.x > 0 && m_pos.y > 0) token_str = tokenizer->get_token_at(m_pos).to_string();                            \
		else token_str = tokenizer->peek(-1, true).to_string();                                                            \
		if (token_str.size() > 1 && token_str[0] == '<' && token_str[token_str.size() - 1] == '>') err_len = 1;            \
		else err_len = (uint32_t)token_str.size();                                                                         \
																														   \
		if (m_pos.x > 0 && m_pos.y > 0) {                                                                                  \
			String line = file_node->source.get_line(m_pos.x);                                                             \
			throw Error(m_err_type, m_msg, file_node->path, line, m_pos, err_len);                                         \
		} else {                                                                                                           \
			String line = file_node->source.get_line(tokenizer->get_pos().x);                                              \
			throw Error(m_err_type, m_msg, file_node->path, line, tokenizer->get_pos(), err_len);                          \
		}                                                                                                                  \
	} while (false)

#define THROW_UNEXP_TOKEN(m_tk)                                                                                            \
	do {                                                                                                                   \
		if (m_tk != "") {                                                                                                  \
			THROW_PARSER_ERR(Error::SYNTAX_ERROR,                                                                          \
				String::format("Unexpected token(\"%s\"). expected %s.",                                                   \
					Tokenizer::get_token_name(tokenizer->peek(-1, true).type), m_tk).c_str(), Vect2i());                   \
		} else {                                                                                                           \
			THROW_PARSER_ERR(Error::SYNTAX_ERROR, String::format("Unexpected token(\"%s\").",                              \
				Tokenizer::get_token_name(tokenizer->peek(-1, true).type)).c_str(), Vect2i());                             \
		}                                                                                                                  \
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
		Vect2i pos;
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
		String source;
		stdvec<ptr<FileNode>> imports;
		stdvec<ptr<VarNode>> vars; // Global vars.
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
		bool is_static = false;
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
		BuiltinFunctions::Type func;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
		BuiltinFunctionNode(BuiltinFunctions::Type p_func) {
			type = Type::BUILTIN_FUNCTION;
			func = p_func;
		}
	};

	struct BuiltinClassNode : public Node {
		BuiltinTypes::Type cls;
		BuiltinClassNode() {
			type = Type::BUILTIN_CLASS;
		}
		BuiltinClassNode(BuiltinTypes::Type p_cls) {
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

			OP_POSITIVE,
			OP_NEGATIVE,

			__OP_MAX__,
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
			SWITCH,
			WHILE,
			BREAK,
			CONTINUE,
			RETURN,
		};
		CfType cf_type;
		stdvec<ptr<Node>> args;
		ptr<BlockNode> body;
		ptr<BlockNode> body_else; // For if node.
		ControlFlowNode() {
			type = Type::CONTROL_FLOW;
		}
	};

	// Methods.
	void parse(String p_source, String p_file_path);

protected:

private:
	struct Expr {
		Expr(OperatorNode::OpType p_op, const Vect2i& p_pos) { _is_op = true; op = p_op; pos = p_pos; }
		Expr(const ptr<Node>& p_node) { _is_op = false; expr = p_node; pos = p_node->pos; }
		Expr(const Expr& p_other) {
			if (p_other._is_op) { _is_op = true; op = p_other.op; } else { _is_op = false; expr = p_other.expr; }
			pos = p_other.pos;
		}
		Expr& operator=(const Expr& p_other) {
			if (p_other._is_op) { _is_op = true; op = p_other.op; } else { _is_op = false; expr = p_other.expr; }
			pos = p_other.pos;
			return *this;
		}
		~Expr() { if (!_is_op) { expr = nullptr; } }

		bool is_op() const { return _is_op; }
		Vect2i get_pos() const { return pos; }
		OperatorNode::OpType get_op() const { return op; }
		ptr<Node>& get_expr() { return expr; }

	private:
		bool _is_op = true;
		Vect2i pos;
		OperatorNode::OpType op;
		ptr<Node> expr;
	};

	struct ParserContext {
		ClassNode* current_class = nullptr;
		FunctionNode* current_func = nullptr;
	};

	// Methods.

	template<typename T=Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		ret->pos = tokenizer->get_pos();
		return ret;
	}

	ptr<ClassNode> _parse_class();
	ptr<EnumNode> _parse_enum(ptr<Node> p_parent = nullptr);
	stdvec<ptr<VarNode>> _parse_var(ptr<Node> p_parent);
	ptr<FunctionNode> _parse_func(ptr<Node> p_parent);
	ptr<BlockNode> _parse_block(const ptr<Node>& p_parent);

	ptr<Node> _parse_expression(const ptr<Node>& p_parent);
	stdvec<ptr<Node>> _parse_arguments(const ptr<Node>& p_parent);
	void _reduce_expression(ptr<Node>& p_expr);

	ptr<Node> _reduce_operator_tree(stdvec<Expr>& p_expr);
	static int _get_operator_precedence(OperatorNode::OpType p_op);

	// Members.
	ptr<FileNode> file_node;
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ParserContext parser_context;
};


}
#endif // PARSER_H