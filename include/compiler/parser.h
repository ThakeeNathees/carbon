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

#include "core/core.h"
#include "core/logger.h"
#include "tokenizer.h"
#include "bytecode.h"

namespace carbon {

#define PARSER_ERROR(m_type, m_msg, m_pos) _parser_error(m_type, m_msg, m_pos, _DBG_SOURCE)
#define UNEXP_TOKEN_ERROR(m_expected) _unexp_token_error(m_expected, _DBG_SOURCE)
#define PREDEFINED_ERROR(m_what, m_name, m_pos) _predefined_error(m_what, m_name, m_pos, _DBG_SOURCE)

class Parser {
public:
	struct Node {
		enum class Type {
			UNKNOWN,

			IMPORT,
			FILE,
			CLASS,
			ENUM,
			FUNCTION,
			BLOCK,
			IDENTIFIER,
			VAR,
			CONST,
			CONST_VALUE, // evaluvated to compile time constants ex: "str", 3.14, Array(1, 2), ...
			ARRAY,       // literal array ex: [1, 2, [3]]
			MAP,         // literal map   ex: { "key":"value", 1:[2, 3] }
			THIS,
			SUPER,
			BUILTIN_FUNCTION,
			BUILTIN_TYPE,
			CALL,
			INDEX,
			MAPPED_INDEX,
			OPERATOR,
			CONTROL_FLOW,

			_NODE_MAX_,
		};
		Type type = Type::UNKNOWN;
		Vect2i pos;
		uint32_t width = 1; // width of the node ^^^^ (dbg).
		ptr<Node> parernt_node;
		bool is_reduced = false;
		bool _is_reducing = false;
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
	struct ConstNode;
	struct ConstValueNode;
	struct ArrayNode;
	struct MapNode;
	struct CallNode;
	struct IndexNode;
	struct MappedIndexNode;
	struct OperatorNode;
	struct ControlFlowNode;

	struct MemberContainer : public Node {
		MemberContainer(Type type) { this->type = type; }

		ptr<EnumNode> unnamed_enum;
		stdvec<ptr<EnumNode>> enums;
		stdvec<ptr<VarNode>> vars;
		stdvec<ptr<ConstNode>> constants;
		stdvec<ptr<FunctionNode>> functions;
		stdvec<ptr<CallNode>> compiletime_functions;
	};

	struct ImportNode : public Node {
		String name;
		ptr<Bytecode> bytecode;
	};

	struct FileNode : public MemberContainer {
		String path, source;

		stdvec<ptr<ClassNode>> classes;
		stdvec<ptr<ImportNode>> imports;

		FileNode() : MemberContainer(Type::FILE) { }

	};

	struct ClassNode : public MemberContainer {
		String name;

		enum BaseType {
			NO_BASE,
			BASE_LOCAL,
			BASE_NATIVE,
			BASE_EXTERN
		};
		BaseType base_type = NO_BASE;

		String base_class_name;
		ClassNode* base_class = nullptr;
		ptr<Bytecode> base_binary = nullptr;

		bool has_super_ctor_call = false;
		FunctionNode* constructor = nullptr;

		uint32_t get_member_offset() {
			if (base_type == BASE_EXTERN) return base_binary->get_member_count();
			else if (base_type == BASE_LOCAL) return base_class->get_member_offset() + (uint32_t)base_class->vars.size();
			else return 0;
		}

		uint32_t get_member_index(const String& p_name) {
			for (uint32_t i = 0; i < (uint32_t)vars.size(); i++) {
				if (vars[i]->name == p_name) return get_member_offset() + i;
			}
			if (base_type == BASE_EXTERN) return base_binary->get_member_index(p_name);
			else if (base_type == BASE_LOCAL) return base_class->get_member_index(p_name);

			THROW_BUG("member not found");
		}

		ClassNode() : MemberContainer(Type::CLASS) { }
	};

	struct EnumValueNode {
		Vect2i pos = Vect2i(-1, -1);
		ptr<Node> expr;
		bool is_reduced = false;
		bool _is_reducing = false; // for cyclic dependancy.
		int64_t value = 0;
		EnumNode* _enum = nullptr; // if not named enum it'll be nullptr.
		EnumValueNode() {}
		EnumValueNode(ptr<Node> p_expr, Vect2i p_pos, EnumNode* p_enum = nullptr) {
			pos = p_pos;
			expr = p_expr;
			_enum = p_enum;
		}
	};
	struct EnumNode : public Node {
		String name;
		bool named_enum = false;
		// EnumValueNode could be nullptr if no custom value.
		std::map<String, EnumValueNode> values;
		EnumNode() {
			type = Type::ENUM;
		}
	};

	struct ParameterNode {
		Vect2i pos = Vect2i(-1, -1);
		String name;
		bool is_reference = false;
		ptr<Node> default_value;
		ParameterNode() {}
		ParameterNode(String p_name, Vect2i p_pos) {
			name = p_name;
			pos = p_pos;
		}
	};
	struct FunctionNode : public Node {
		String name;
		bool is_static = false;
		bool has_return = false;
		bool is_constructor = false;
		uint32_t end_line = -1; // needed for debugger, it's where destructor called
		stdvec<ParameterNode> args;
		stdvec<var> default_args;
		ptr<BlockNode> body;
		Node* parent_node;
		FunctionNode() {
			type = Type::FUNCTION;
		}
	};

	struct BlockNode : public Node {
		stdvec<ptr<Node>> statements;
		// quick reference instead of searching from statement (change to VarNode* maybe).
		stdvec<ptr<VarNode>> local_vars;
		stdvec<ptr<ConstNode>> local_const;
		BlockNode() {
			type = Type::BLOCK;
		}
	};

	struct IdentifierNode : public Node {
		String name;
		// TODO: declared_block haven't added.
		BlockNode* declared_block = nullptr; // For search in local vars.

		enum IdentifierReferenceBase {
			BASE_UNKNOWN,
			BASE_LOCAL,
			BASE_NATIVE,
			BASE_EXTERN,
		};

		enum IdentifierReference {
			REF_UNKNOWN,
			REF_PARAMETER,
			REF_LOCAL_VAR,
			REF_LOCAL_CONST,

			// these will have base.
			REF_MEMBER_VAR,
			REF_STATIC_VAR,
			REF_MEMBER_CONST,
			REF_ENUM_NAME,
			REF_ENUM_VALUE,
			REF_FUNCTION,

			REF_CARBON_CLASS,
			REF_NATIVE_CLASS,
			REF_EXTERN, // TODO: extern class and extern file.
		};

		IdentifierReferenceBase ref_base = BASE_UNKNOWN;
		IdentifierReference ref = REF_UNKNOWN;
		union {
			int param_index = 0;

			// reference from carbon local.
			VarNode* _var;
			ConstNode* _const;
			EnumValueNode* _enum_value;
			EnumNode* _enum_node;
			const ClassNode* _class;
			const FunctionNode* _func;
			BuiltinFunctions::Type _bi_func;
			
			// reference from native.
			const MethodInfo* _method_info;
			const PropertyInfo* _prop_info;
			const EnumInfo* _enum_info;
			const EnumValueInfo* _enum_value_info;
			const ClassInfo* _class_info;
			const Bytecode* _bytecode;
		};

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

	struct ConstNode : public Node {
		String name; // Every const are static.
		ptr<Node> assignment;
		var value;
		ConstNode() {
			type = Type::CONST;
		}
	};

	// Note: ConstValueNode isn't constant values but they are compile time known variables.
	//       and could be Array(1, 2, 3), Map if it has a literal.
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
		bool _can_const_fold = false;
		ArrayNode() {
			type = Type::ARRAY;
		}
	};

	struct MapNode : public Node {
		struct Pair {
			ptr<Node> key;
			ptr<Node> value;
			Pair() {}
			Pair(ptr<Node>& p_key, ptr<Node>& p_value) { key = p_key; value = p_value; }
		};
		stdvec<Pair> elements;
		bool _can_const_fold = false;
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
		BuiltinFunctions::Type func = BuiltinFunctions::UNKNOWN;
		BuiltinFunctionNode() {
			type = Type::BUILTIN_FUNCTION;
		}
		BuiltinFunctionNode(BuiltinFunctions::Type p_func) {
			type = Type::BUILTIN_FUNCTION;
			func = p_func;
		}
	};

	struct BuiltinTypeNode : public Node {
		BuiltinTypes::Type builtin_type = BuiltinTypes::UNKNOWN;
		BuiltinTypeNode() {
			type = Type::BUILTIN_TYPE;
		}
		BuiltinTypeNode(BuiltinTypes::Type p_cls) {
			type = Type::BUILTIN_TYPE;
			builtin_type = p_cls;
		}
	};

	struct CallNode : public Node {
		ptr<Node> base;
		// should be Node (instead of identifier node) for reduce the identifier.
		// if the method is nullptr and base is a var `a_var(...)` -> `a_var.__call(...)` will be called.
		ptr<Node> method;
		bool is_compilttime = false;
		stdvec<ptr<Node>> args;
		CallNode() {
			type = Node::Type::CALL;
		}
	};

	struct IndexNode : public Node {
		ptr<Node> base;
		ptr<IdentifierNode> member = nullptr;

		bool _ref_reduced = false;
		//ptr<IdentifierNode> _ref = nullptr; // reduced index node reference.
		IndexNode() {
			type = Node::Type::INDEX;
		}
	};

	struct MappedIndexNode : public Node {
		ptr<Node> base;
		ptr<Node> key;
		MappedIndexNode() {
			type = Node::Type::MAPPED_INDEX;
		}
	};

	struct OperatorNode : public Node {
		enum OpType {
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
			FOREACH,
			BREAK,
			CONTINUE,
			RETURN,

			_CF_MAX_,
		};
		struct SwitchCase {
			Vect2i pos;
			ptr<Node> expr;
			int64_t value;
			ptr<BlockNode> body;
			bool default_case = false;
		};
		CfType cf_type;
		stdvec<ptr<Node>> args;
		ptr<BlockNode> body;
		ptr<BlockNode> body_else;
		stdvec<SwitchCase> switch_cases;
		
		ControlFlowNode* break_continue = nullptr;
		FunctionNode* _return = nullptr;
		bool has_break = false;
		bool has_continue = false;

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
	//void parse(const String& p_source, const String& p_file_path);
	void parse(ptr<Tokenizer> p_tokenizer);
#ifdef DEBUG_BUILD
	void print_tree() const;
#endif

private:
	friend class Analyzer;
	friend class CodeGen;
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
		VarNode* current_var = nullptr;
		ConstNode* current_const = nullptr;
		FunctionNode* current_func = nullptr;
		BlockNode* current_block = nullptr;
		EnumNode* current_enum = nullptr;
		int current_statement_ind = -1;

		ControlFlowNode* current_break = nullptr;
		ControlFlowNode* current_continue = nullptr;
	};

	// members.
	ptr<FileNode> file_node;
	ptr<Tokenizer> tokenizer;
	ParserContext parser_context;

	// methods.
	template<typename T=Node, typename... Targs>
	ptr<T> new_node(Targs... p_args) {
		ptr<T> ret = newptr<T>(p_args...);
		ret->pos = tokenizer->get_pos();
		ret->width = tokenizer->get_pos();
		return ret;
	}

	CompileTimeError _parser_error(Error::Type p_type, const String& p_msg, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;
	CompileTimeError _unexp_token_error(const char* p_expected, const DBGSourceInfo& p_dbg_info) const;
	CompileTimeError _predefined_error(const String& p_what, const String& p_name, Vect2i p_pos, const DBGSourceInfo& p_dbg_info) const;

	ptr<ImportNode> _parse_import();
	ptr<ClassNode> _parse_class();
	ptr<EnumNode> _parse_enum(ptr<Node> p_parent);
	stdvec<ptr<VarNode>> _parse_var(ptr<Node> p_parent);
	ptr<ConstNode> _parse_const(ptr<Node> p_parent);
	ptr<FunctionNode> _parse_func(ptr<Node> p_parent);

	ptr<BlockNode> _parse_block(const ptr<Node>& p_parent, bool p_single_statement = false, stdvec<Token> p_termination = { Token::BRACKET_RCUR } );
	ptr<ControlFlowNode> _parse_if_block(const ptr<BlockNode>& p_parent);

	ptr<Node> _parse_expression(const ptr<Node>& p_parent, bool p_allow_assign);
	stdvec<ptr<Node>> _parse_arguments(const ptr<Node>& p_parent);

	ptr<Node> _build_operator_tree(stdvec<Expr>& p_expr);
	static int _get_operator_precedence(OperatorNode::OpType p_op);
	void _check_identifier_predefinition(const String& p_name, Node* p_scope) const;
};


}
#endif // PARSER_H