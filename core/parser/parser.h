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

class Parser 
{
private:

public:
	
	struct Node
	{
		enum class Type {
			UNKNOWN,

			FILE,
			STRUCT,
			ENUM,
			BUILTIN_FUNCTION,
			FUNCTION,
			BLOCK,
			IDENTIFIER,
			VAR,
			ARRAY,
			DICTIONARY,
			OPERATOR,
			CONTROL_FLOW,
		};
		Type type;
		int line, col;
		Ptr<Node> parern_node;
	};

	struct StructNode;
	struct EnumNode;
	struct BuiltinFunctionNode;
	struct FunctionNode;
	struct BlockNode;
	struct IdentifierNode;
	struct VarNode;
	struct ArrayNode;
	struct DictionaryNode;
	struct OperatorNode;
	struct ControlFlowNode;

	struct FileNode : public Node {
		String path;
		std::vector<Ptr<FileNode>> imports;
		std::vector<Ptr<VarNode>> static_vars;
		std::vector<Ptr<StructNode>> structs;
		std::vector<Ptr<EnumNode>> enums;
		std::vector<Ptr<FunctionNode>> functions;

		FileNode() {
			type = Type::FILE;
		}

	};

	struct StructNode : public Node {
		String name;
		std::vector<Ptr<VarNode>> members;
		StructNode() {
			type = Type::STRUCT;
		}
	};

	struct EnumNode : public Node {
		String name;
		std::map<String, int> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct BuiltinFunctionNode : public Node {
		BuiltinFunctions::Function func;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
	};

	struct FunctionNode : public Node {
		String name;
		std::vector<String> args;
		Ptr<BlockNode> body;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : public Node {
		std::vector<Ptr<Node>> statements;
		std::vector<Ptr<VarNode>> local_vars;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : public Node {
		String name;
		IdentifierNode() {
			type = Type::IDENTIFIER;
		}
	};

	struct VarNode : public Node {
		String name;
		Ptr<Node> assignment;
		VarNode() {
			type = Type::VAR;
		}
	};

	struct ArrayNode : public Node {
		std::vector<Ptr<Node>> elements;
		ArrayNode() {
			type = Type::ARRAY;
		}
	};

	struct DictionaryNode : public Node {
		struct Pair {
			Ptr<Node> key;
			Ptr<Node> value;
		};
		std::vector<Pair> elements;
		DictionaryNode() {
			type = Type::DICTIONARY;
		}
	};

	struct OperatorNode : public Node {
		enum class OpType {
			OP_CALL_FUNC,
			OP_INDEX,
			OP_INDEX_NAMED,
			
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
			OP_NOT,
			OP_NOTEQ,
			OP_LT,
			OP_LTEQ,
			OP_GT,
			OP_GTEQ,
			OP_BIT_NOT,
			OP_LSHIFT,
			OP_LSHIFT_EQ,
			OP_RSHIFT,
			OP_RSHIFT_EQ,
			OP_OR,
			OP_OR_EQ,
			OP_AND,
			OP_AND_EQ,
			OP_XOR,
			OP_XOR_EQ,
		};
		OpType op_type;
		std::vector<Ptr<Node>> args;
		OperatorNode() {
			type = Type::OPERATOR;
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
		std::vector<Ptr<Node>> args;
		Ptr<BlockNode> body;
		Ptr<BlockNode> body_else; // for cf if node
		ControlFlowNode() {
			type = Type::CONTROL_FLOW;
		}
	};

private:

	Error err;

	String file_path;
	String source;

	Ptr<FileNode> file_node;
	Ptr<Tokenizer> tokenizer = newptr(Tokenizer);

	struct IdentifierLocation {
		bool found = false;
		int line = 0, col = 0;
		String file_path;
		Node::Type type = Node::Type::UNKNOWN;
		IdentifierLocation() {}
		IdentifierLocation(const Ptr<Node>& p_node, const String& p_file_path) {
			found = true;
			file_path = p_file_path;
			line = p_node->line;
			col = p_node->col;
			type = p_node->type;
		}
	};

	void _throw(Error::Type p_type, const String& p_msg, int line = -1);
	void _throw_unexp_token(const String& p_exp = "");
	IdentifierLocation _find_identifier_location(const String& p_name, const Ptr<Node> p_node) const;

	void _parse_struct();
	void _parse_enum();
	void _parse_var(Ptr<Node> p_node = nullptr);
	void _parse_func();

	void _parse_block(Ptr<BlockNode>& p_block, const Ptr<Node>& p_parent);
	void _parse_expression(Ptr<Node>& p_expr);
	void _reduce_expression(Ptr<Node>& p_expr);

public:
	void parse(String p_source, String p_file_path);

};


}
#endif // PARSER_H