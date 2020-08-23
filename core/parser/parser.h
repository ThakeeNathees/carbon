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
#include "io/logger.h"

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
			throw Error(m_err_type, m_msg, file_node->path, line, m_pos, err_len)_ERR_ADD_DBG_VARS;                        \
		} else {                                                                                                           \
			String line = file_node->source.get_line(tokenizer->get_pos().x);                                              \
			throw Error(m_err_type, m_msg, file_node->path, line, tokenizer->peek(-1, true).get_pos(), err_len)            \
			_ERR_ADD_DBG_VARS;                                                                                             \
		}                                                                                                                  \
	} while (false)

#define THROW_UNEXP_TOKEN(m_tk)                                                                                            \
	do {                                                                                                                   \
		Error::Type err_type = Error::SYNTAX_ERROR;                                                                        \
		if (tokenizer->peek(-1, true).type == Token::_EOF) err_type = Error::UNEXPECTED_EOF;                               \
		if (m_tk != "") {                                                                                                  \
			THROW_PARSER_ERR(err_type, String::format("Unexpected token(\"%s\"). expected %s.",                            \
					Tokenizer::get_token_name(tokenizer->peek(-1, true).type), m_tk).c_str(), Vect2i());                   \
		} else {                                                                                                           \
			THROW_PARSER_ERR(err_type, String::format("Unexpected token(\"%s\").",                                         \
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
			MAP,   // <-- TODO: should be literal map ?
			THIS,
			SUPER,
			BUILTIN_FUNCTION,
			BUILTIN_TYPE,
			OPERATOR,
			CONTROL_FLOW,

			_NODE_MAX_,
		};
		Type type = Type::UNKNOWN;
		Vect2i pos;
		ptr<Node> parernt_node; // TODO: need to set to all nodes.
		static const char* get_node_type_name(Type p_type);
	};

	struct ClassNode;
	struct EnumNode;
	struct BuiltinFunctionNode;
	struct BuiltinTypeNode;
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
		String name; // import name = "path/to/source.cb";
		stdvec<ptr<FileNode>> imports;
		stdvec<ptr<VarNode>> vars; // Global vars.
		stdvec<ptr<ClassNode>> classes;
		ptr<EnumNode> unnamed_enum;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<FunctionNode>> functions;

		FileNode() {
			type = Type::FILE;
		}

	};

	struct ClassNode : public Node {
		String name;
		stdvec<String> inherits;
		//String base; // TODO: file_node.file_node.ClassName ??
		ptr<EnumNode> unnamed_enum;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<VarNode>> vars;
		stdvec<ptr<FunctionNode>> functions;
		// TODO: implement const.
		ClassNode() {
			type = Type::CLASS;
		}
	};

	struct EnumValueNode {
		Vect2i pos = Vect2i(-1, -1);
		int64_t value = 0;
		EnumValueNode() {}
		EnumValueNode(int64_t p_value, Vect2i p_pos) {
			pos = p_pos;
			value = p_value;
		}
	};
	struct EnumNode : public Node {
		String name;
		bool named_enum = false;
		std::map<String, EnumValueNode> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct ArgumentNode {
		Vect2i pos = Vect2i(-1, -1);
		String name;
		ArgumentNode() {}
		ArgumentNode(String p_name, Vect2i p_pos) {
			name = p_name;
			pos = p_pos;
		}
	};
	struct FunctionNode : public Node {
		String name;
		bool is_static = false;
		stdvec<ArgumentNode> args;
		ptr<BlockNode> body;
		ptr<Node> parent_node;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : public Node {
		stdvec<ptr<Node>> statements;
		// quick reference instead of searching from statement (change to VarNode* maybe).
		stdvec<ptr<VarNode>> local_vars;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : public Node {
		String name;
		int arg_index = -1; // For argument identifier.
		BlockNode* declared_block = nullptr; // For local vars.
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
		bool is_static = false;
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

	struct BuiltinTypeNode : public Node {
		BuiltinTypes::Type builtin_type;
		BuiltinTypeNode() {
			type = Type::BUILTIN_TYPE;
		}
		BuiltinTypeNode(BuiltinTypes::Type p_cls) {
			type = Type::BUILTIN_TYPE;
			builtin_type = p_cls;
		}
	};



	struct OperatorNode : public Node {
		enum OpType {
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

			_OP_MAX_,
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
		static bool is_assignment(OpType p_op_type) {
			return
				p_op_type == OperatorNode::OpType::OP_EQ ||
				p_op_type == OperatorNode::OpType::OP_PLUSEQ ||
				p_op_type == OperatorNode::OpType::OP_MINUSEQ ||
				p_op_type == OperatorNode::OpType::OP_MULEQ ||
				p_op_type == OperatorNode::OpType::OP_DIVEQ ||
				p_op_type == OperatorNode::OpType::OP_MOD_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_LSHIFT_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_RSHIFT_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_OR_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_AND_EQ ||
				p_op_type == OperatorNode::OpType::OP_BIT_XOR_EQ;
			}
		static const char* get_op_name(OpType p_op);
	};

	struct ControlFlowNode : public Node {
		enum CfType {
			IF,
			SWITCH,
			WHILE,
			FOR,
			BREAK,
			CONTINUE,
			RETURN,

			_CF_MAX_,
		};
		struct SwitchCase {
			ptr<Node> value; // TODO: any 2 values can't be same.
			ptr<BlockNode> body;
			bool default_case = false;
		};
		CfType cf_type;
		stdvec<ptr<Node>> args;
		ptr<BlockNode> body;
		ptr<BlockNode> body_else;
		stdvec<SwitchCase> switch_cases;
		ControlFlowNode() {
			type = Type::CONTROL_FLOW;
		}
		ControlFlowNode(CfType p_cf_type) {
			type = Type::CONTROL_FLOW;
			cf_type = p_cf_type;
		}
		static const char* get_cftype_name(CfType p_type);
	};

	/* How if block parsed
	-----------------------------------------
	if (c1){
	} else if (c2) {
	} else if (c3) {
	} else {}
	---- the above parsed into --------------
	if (c1){
	} else {
		if (c2){
		} else {
			if (c3) {
			} else {}
		}
	}
	-----------------------------------------
	*/

	// Methods.
	void parse(String p_source, String p_file_path);
#if DEBUG_BUILD
	void print_tree() const;
#endif

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
		BlockNode* current_block = nullptr;
	};

	// Methods.

	template<typename T=Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		ret->pos = tokenizer->get_pos();
		return ret;
	}

	ptr<FileNode> _parse_import(); // TODO: must return codegen.
	ptr<ClassNode> _parse_class();
	ptr<EnumNode> _parse_enum(ptr<Node> p_parent);
	stdvec<ptr<VarNode>> _parse_var(ptr<Node> p_parent);
	ptr<FunctionNode> _parse_func(ptr<Node> p_parent);

	ptr<BlockNode> _parse_block(const ptr<Node>& p_parent, bool p_single_statement = false, stdvec<Token> p_termination = { Token::BRACKET_RCUR } );
	ptr<ControlFlowNode> _parse_if_block(const ptr<BlockNode>& p_parent);

	ptr<Node> _parse_expression(const ptr<Node>& p_parent, bool p_allow_assign);
	stdvec<ptr<Node>> _parse_arguments(const ptr<Node>& p_parent);

	ptr<Node> _build_operator_tree(stdvec<Expr>& p_expr);
	static int _get_operator_precedence(OperatorNode::OpType p_op);


	void _reduce_expression(ptr<Node>& p_expr);

	// Members.
	ptr<FileNode> file_node;
	ptr<Tokenizer> tokenizer;
	ParserContext parser_context;
};


}
#endif // PARSER_H