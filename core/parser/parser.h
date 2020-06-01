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

	struct FileNode : Node {
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

	struct StructNode : Node {
		String name;
		std::vector<Ptr<VarNode>> members;
		StructNode() {
			type = Type::STRUCT;
		}
	};

	struct EnumNode : Node {
		String name;
		std::map<String, int> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct BuiltinFunctionNode : Node {
		BuiltinFunctions::Function func;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
	};

	struct FunctionNode : Node {
		String name;
		std::vector<String> args;
		Ptr<BlockNode> body;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : Node {
		Ptr<BlockNode> outer_block;
		std::vector<Ptr<Node>> statements;
		std::vector<Ptr<VarNode>> local_vars;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : Node {
		String name;
		IdentifierNode() {
			type = Type::IDENTIFIER;
		}
	};

	struct VarNode : Node {
		String name;
		Ptr<Node> assignment;
		VarNode() {
			type = Type::VAR;
		}
	};

	struct ArrayNode : Node {
		std::vector<Ptr<Node>> elements;
		ArrayNode() {
			type = Type::ARRAY;
		}
	};

	struct DictionaryNode : Node {
		struct Pair {
			Ptr<Node> key;
			Ptr<Node> value;
		};
		std::vector<Pair> elements;
		DictionaryNode() {
			type = Type::DICTIONARY;
		}
	};

	struct OperatorNode : Node {
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

	struct ControlFlowNode : Node {
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

	Error::Type _set_error(Error::Type p_type, const String& p_msg, int line = -1);

	Error::Type _parse_struct();
	Error::Type _parse_enum();
	Error::Type _parse_func();
	Error::Type _parse_var(Ptr<Node> p_struct = nullptr);

	Error::Type _parse_expression(Ptr<Node>& p_expr);
	Error::Type _reduce_expression(Ptr<Node>& p_expr);
	Error::Type _parse_and_reduce_expression(Ptr<Node>& p_expr);

public:
	const Error& parse(String p_source, String p_file_path);

};


}
#endif // PARSER_H