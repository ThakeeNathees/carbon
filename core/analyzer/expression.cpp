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

#include "analyzer.h"

namespace carbon {

void Analyzer::_reduce_expression(ptr<Parser::Node>& p_expr) {

	if (p_expr == nullptr) return;

	// Prevent stack overflow.
	if (p_expr->is_reduced) return;
	p_expr->is_reduced = true;

	switch (p_expr->type) {

		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER:
		case Parser::Node::Type::BUILTIN_TYPE:
		case Parser::Node::Type::BUILTIN_FUNCTION:
			break;

		case Parser::Node::Type::IDENTIFIER: {
			_reduce_identifier(p_expr);
			_analyze_identifier(p_expr);
		} break;

		// reduce ArrayNode
		case Parser::Node::Type::ARRAY: {
			ptr<Parser::ArrayNode> arr = ptrcast<Parser::ArrayNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)arr->elements.size(); i++) {
				_reduce_expression(arr->elements[i]);
				if (arr->elements[i]->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			arr->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Array arr_v;
				for (int i = 0; i < (int)arr->elements.size(); i++) {
					arr_v.push_back(ptrcast<Parser::ConstValueNode>(arr->elements[i])->value);
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(arr_v);
				cv->pos = arr->pos; p_expr = cv;
			}
		} break;

		// reduce MapNode
		case Parser::Node::Type::MAP: {
			ptr<Parser::MapNode> map = ptrcast<Parser::MapNode>(p_expr);
			bool all_const = true;
			for (int i = 0; i < (int)map->elements.size(); i++) {
				_reduce_expression(map->elements[i].key);
				// TODO: if key is const value and two keys are the same throw error.
				if (map->elements[i].key->type == Parser::Node::Type::CONST_VALUE) {
					var& key_v = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					if (!var::is_hashable(key_v.get_type())) THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("unhasnable type %s used as map key.", key_v.get_type_name().c_str()), map->pos);
				}
				_reduce_expression(map->elements[i].value);

				if (map->elements[i].key->type != Parser::Node::Type::CONST_VALUE || map->elements[i].value->type != Parser::Node::Type::CONST_VALUE) {
					all_const = false;
				}
			}
			map->_can_const_fold = all_const;

			if (all_const && parser->parser_context.current_const != nullptr) {
				Map map_v;
				for (int i = 0; i < (int)map->elements.size(); i++) {
					var& _key = ptrcast<Parser::ConstValueNode>(map->elements[i].key)->value;
					var& _val = ptrcast<Parser::ConstValueNode>(map->elements[i].value)->value;
					map_v[_key] = _val;
				}
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(map_v);
				cv->pos = map->pos; p_expr = cv;
			}
		} break;

#define GET_ARGS(m_nodes)                                                             \
	stdvec<var*> args;                                                                \
	for (int i = 0; i < (int)m_nodes.size(); i++) {                                   \
	    args.push_back(&ptrcast<Parser::ConstValueNode>(m_nodes[i])->value);          \
	}

#define SET_EXPR_CONST_NODE(m_var, m_pos)                                             \
do {                                                                                  \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);         \
	cv->pos = m_pos;                                                                  \
	p_expr = cv;                                                                      \
} while (false)

		case Parser::Node::Type::CALL: {
			_reduce_call(p_expr);
		} break;

		case Parser::Node::Type::INDEX: {
			ptr<Parser::IndexNode> index = ptrcast<Parser::IndexNode>(p_expr);
			_reduce_expression(index->base);
			ASSERT(index->member->type == Parser::Node::Type::IDENTIFIER);
			ptr<Parser::IdentifierNode> member = ptrcast<Parser::IdentifierNode>(index->member);

			switch (index->base->type) {

				// used in: Vect2.ZERO;
				case Parser::Node::Type::BUILTIN_TYPE: {
					Parser::BuiltinTypeNode* bt = ptrcast<Parser::BuiltinTypeNode>(index->base).get();
					const MemberInfo* mi = var::get_member_info_s(BuiltinTypes::get_var_type(bt->builtin_type), member->name);
					if (!mi) THROW_ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), BuiltinTypes::get_type_name(bt->builtin_type).c_str()), member->pos);
					switch (mi->get_type()) {
						case MemberInfo::METHOD:
							break; // TODO: FuncRef?
						case MemberInfo::PROPERTY: {
							PropertyInfo* pi = (PropertyInfo*)mi;
							if (pi->is_const()) SET_EXPR_CONST_NODE(pi->get_value(), index->pos);
							else THROW_BUG("can't be.");
						} break;
						case MemberInfo::ENUM:
						case MemberInfo::ENUM_VALUE:
							THROW_BUG("can't be.");
					}
				} break;

				// [1, 2].member;
				case Parser::Node::Type::CONST_VALUE: {
					Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(index->base).get();
					try {
						ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.get_member(member->name));
						cv->pos = member->pos; p_expr = cv;
					} catch (VarError& verr) {
						THROW_ANALYZER_ERROR(Error(verr).get_type(), verr.what(), index->pos);
					}
				} break;

				// this.member; super.member; idf.member;
				case Parser::Node::Type::INDEX: { // <-- base is index node but reference reduced.
					Parser::IndexNode* _ind = ptrcast<Parser::IndexNode>(index->base).get();
					if (!_ind->_ref_reduced) break;
				}  // [[ FALLTHROUGH ]]
				case Parser::Node::Type::THIS:
				case Parser::Node::Type::SUPER:
				case Parser::Node::Type::IDENTIFIER: {

					Parser::IdentifierNode* base;
					enum _BaseClassRef { _THIS, _SUPER, _NEITHER };
					_BaseClassRef _base_class_ref = _NEITHER;

					if (index->base->type == Parser::Node::Type::THIS) {
						ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->name);
						_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
						_id->_class = parser->parser_context.current_class;
						index->base = _id;
						_base_class_ref = _THIS;
					} else if (index->base->type == Parser::Node::Type::SUPER) {
						if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_LOCAL) {
							ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
							_id->ref = Parser::IdentifierNode::REF_CARBON_CLASS;
							_id->_class = parser->parser_context.current_class->base_class;
							index->base = _id;
							_base_class_ref = _SUPER;
						} else if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_EXTERN) {
							THROW_BUG("TODO:");
						}
					}

					if (index->base->type == Parser::Node::Type::INDEX) {
						base = ptrcast<Parser::IndexNode>(index->base)->member.get();
					} else {
						base = ptrcast<Parser::IdentifierNode>(index->base).get();
					}

					switch (base->ref) {
						case Parser::IdentifierNode::REF_UNKNOWN: {
							THROW_BUG("base can't be unknown.");
						} break;

						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_MEMBER_VAR:
							break; // Can't reduce anymore.

						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							THROW_BUG("there isn't any contant value currently support attribute access and most probably in the future");
						} break;

						case Parser::IdentifierNode::REF_ENUM_NAME: {
							if (base->ref_base == Parser::IdentifierNode::BASE_LOCAL) {
								stdmap<String, Parser::EnumValueNode>::iterator it = base->_enum_node->values.find(member->name);
								if (it != base->_enum_node->values.end()) {
									member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
									member->ref_base = Parser::IdentifierNode::BASE_LOCAL;
									member->_enum_value = &(it->second);
									_resolve_enumvalue(base->_enum_node->values[it->first]);
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->_enum_node->values[it->first].value);
									cv->pos = member->pos; p_expr = cv;
								} else {
									ASSERT(false && "bug unresolved reference");
								}
							} else { // ref on base native/extern.
								stdmap<String, int64_t>::const_iterator it = base->_enum_info->get_values().find(member->name);
								if (it != base->_enum_info->get_values().end()) {
									member->ref = Parser::IdentifierNode::REF_ENUM_VALUE;
									member->ref_base = base->ref_base;
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(it->second);
									cv->pos = member->pos; p_expr = cv;
								} else {
									ASSERT(false && "bug unresolved reference");
								}
							}
						} break;

						case Parser::IdentifierNode::REF_ENUM_VALUE:
							THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "enum value doesn't support attribute access.", member->pos);

						case Parser::IdentifierNode::REF_CARBON_CLASS: {
							Parser::IdentifierNode _id = _find_member(base->_class, member->name);
							_id.pos = member->pos;
							switch (_id.ref) {
								case Parser::IdentifierNode::REF_UNKNOWN:
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
								case Parser::IdentifierNode::REF_PARAMETER:
								case Parser::IdentifierNode::REF_LOCAL_VAR:
								case Parser::IdentifierNode::REF_LOCAL_CONST:
									THROW_BUG("can't be.");

								case Parser::IdentifierNode::REF_MEMBER_VAR: {
									_id.pos = member->pos;
									if (_base_class_ref != _THIS && !_id._var->is_static) {
										THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
									}
									index->member = newptr<Parser::IdentifierNode>(_id);
									index->_ref_reduced = true;
								} break;

								case Parser::IdentifierNode::REF_MEMBER_CONST: {
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(_id._const->value);
									cv->pos = member->pos; p_expr = cv;
								} break;

								case Parser::IdentifierNode::REF_ENUM_NAME: {
									_id.pos = member->pos;
									index->member = newptr<Parser::IdentifierNode>(_id);
									index->_ref_reduced = true;
								} break;

								case Parser::IdentifierNode::REF_ENUM_VALUE: {
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(_id._enum_value);
									cv->pos = member->pos; p_expr = cv;
								} break;

								case Parser::IdentifierNode::REF_FUNCTION: {
									_id.pos = member->pos;
									index->member = newptr<Parser::IdentifierNode>(_id);
									index->_ref_reduced = true;
								} break;

								case Parser::IdentifierNode::REF_CARBON_CLASS:
								case Parser::IdentifierNode::REF_NATIVE_CLASS:
								case Parser::IdentifierNode::REF_EXTERN:
									THROW_BUG("can't be");
							}
						} break;

						case Parser::IdentifierNode::REF_NATIVE_CLASS: {
							ASSERT(NativeClasses::singleton()->is_class_registered(base->name));
							BindData* bd = NativeClasses::singleton()->get_bind_data(base->name, member->name).get();
							if (!bd) THROW_ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), base->name.c_str()), member->pos);
							switch (bd->get_type()) {
								case BindData::METHOD:
								case BindData::STATIC_FUNC:
								case BindData::MEMBER_VAR:
								case BindData::STATIC_VAR:
								case BindData::ENUM:
									break;

								case BindData::STATIC_CONST: {
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((ConstantBind*)bd)->get());
									cv->pos = member->pos; p_expr = cv;
								} break;
								case BindData::ENUM_VALUE: {
									ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((EnumValueBind*)bd)->get());
									cv->pos = member->pos; p_expr = cv;
								} break;
							}

						} break;

							// TODO: fn.NAME, fn.ARGC, fn.DEFAULT_ARGC, ... TODO: implement FuncRef
						case Parser::IdentifierNode::REF_FUNCTION: {
							THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED, "function object doesn't support attribute access.", member->pos);
						} break;

						case Parser::IdentifierNode::REF_EXTERN: { // TODO: change the name.
							THROW_BUG("TODO:");
						} break;
					}
				}

				default:
					break;
					// RUNTIME.
			}

		} break; /// reduce IndexNode ///////////////////////////////////

		case Parser::Node::Type::MAPPED_INDEX: {
			ptr<Parser::MappedIndexNode> mapped_index = ptrcast<Parser::MappedIndexNode>(p_expr);
			_reduce_expression(mapped_index->base);
			_reduce_expression(mapped_index->key);
			if (mapped_index->base->type == Parser::Node::Type::CONST_VALUE && mapped_index->key->type == Parser::Node::Type::CONST_VALUE) {
				Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(mapped_index->base).get();
				Parser::ConstValueNode* key = ptrcast<Parser::ConstValueNode>(mapped_index->key).get();
				try {
					ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.__get_mapped(key->value));
					cv->pos = base->pos; p_expr = cv;
				} catch (VarError& verr) {
					THROW_ANALYZER_ERROR(Error(verr).get_type(), verr.what(), key->pos);
				}
			}
		} break; /// reduce MappedIndexNode ///////////////////////////////////

		///////////////////////////////////////////////////////////////////////////////////////////////

		case Parser::Node::Type::OPERATOR: {
			ptr<Parser::OperatorNode> op = ptrcast<Parser::OperatorNode>(p_expr);

			bool all_const = true;
			for (int i = 0; i < (int)op->args.size(); i++) {
				_reduce_expression(op->args[i]);
				if (op->args[i]->type != Parser::Node::Type::CONST_VALUE) all_const = false;
			}

			switch (op->op_type) {

				case Parser::OperatorNode::OpType::OP_EQ:
				case Parser::OperatorNode::OpType::OP_PLUSEQ:
				case Parser::OperatorNode::OpType::OP_MINUSEQ:
				case Parser::OperatorNode::OpType::OP_MULEQ:
				case Parser::OperatorNode::OpType::OP_DIVEQ:
				case Parser::OperatorNode::OpType::OP_MOD_EQ:
				case Parser::OperatorNode::OpType::OP_LTEQ:
				case Parser::OperatorNode::OpType::OP_GTEQ:
				case Parser::OperatorNode::OpType::OP_BIT_LSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_RSHIFT_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_OR_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_AND_EQ:
				case Parser::OperatorNode::OpType::OP_BIT_XOR_EQ: {

					if (op->args[0]->type == Parser::Node::Type::IDENTIFIER) {
						switch (ptrcast<Parser::IdentifierNode>(op->args[0])->ref) {
							case Parser::IdentifierNode::REF_PARAMETER:
							case Parser::IdentifierNode::REF_LOCAL_VAR:
							case Parser::IdentifierNode::REF_MEMBER_VAR:
								break;
							case Parser::IdentifierNode::REF_LOCAL_CONST:
							case Parser::IdentifierNode::REF_MEMBER_CONST:
							case Parser::IdentifierNode::REF_ENUM_NAME:
							case Parser::IdentifierNode::REF_ENUM_VALUE:
							case Parser::IdentifierNode::REF_FUNCTION:
							case Parser::IdentifierNode::REF_CARBON_CLASS:
							case Parser::IdentifierNode::REF_NATIVE_CLASS:
							case Parser::IdentifierNode::REF_EXTERN:
								THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "invalid assignment (only assignment on variables/parameters are valid).", op->args[0]->pos);
						}
					} else if (op->args[0]->type == Parser::Node::Type::THIS) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"this\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::SUPER) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to \"super\".", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::CONST_VALUE) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to constant values.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::ARRAY) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to array literal.", op->args[0]->pos);
					} else if (op->args[0]->type == Parser::Node::Type::MAP) {
						THROW_ANALYZER_ERROR(Error::TYPE_ERROR, "can't assign anything to map literal.", op->args[0]->pos);
					}
				} break;
				default: { // Remaining binary/unary operators.
					if (!all_const) break;
					GET_ARGS(op->args);
					switch (op->op_type) {
						case Parser::OperatorNode::OpType::OP_EQEQ:
							SET_EXPR_CONST_NODE(*args[0] == *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_PLUS:
							SET_EXPR_CONST_NODE(*args[0] + *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MINUS:
							SET_EXPR_CONST_NODE(*args[0] - *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MUL:
							SET_EXPR_CONST_NODE(*args[0] * *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_DIV:
							SET_EXPR_CONST_NODE(*args[0] / *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_MOD:
							SET_EXPR_CONST_NODE(*args[0] % *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_LT:
							SET_EXPR_CONST_NODE(*args[0] < *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_GT:
							SET_EXPR_CONST_NODE(*args[0] > *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_AND:
							SET_EXPR_CONST_NODE(args[0]->operator bool() && args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_OR:
							SET_EXPR_CONST_NODE(args[0]->operator bool() || args[1]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_NOTEQ:
							SET_EXPR_CONST_NODE(*args[0] != *args[1], op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_LSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() << args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_RSHIFT:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() >> args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_OR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() | args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_AND:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() & args[1]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_XOR:
							SET_EXPR_CONST_NODE(args[0]->operator int64_t() ^ args[1]->operator int64_t(), op->pos);
							break;

						case Parser::OperatorNode::OpType::OP_NOT:
							SET_EXPR_CONST_NODE(!args[0]->operator bool(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_BIT_NOT:
							SET_EXPR_CONST_NODE(~args[0]->operator int64_t(), op->pos);
							break;
						case Parser::OperatorNode::OpType::OP_POSITIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
								case var::FLOAT: {
									SET_EXPR_CONST_NODE(*args[0], op->pos);
								} break;
								default:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"+\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
						case Parser::OperatorNode::OpType::OP_NEGATIVE:
							switch (args[0]->get_type()) {
								case var::BOOL:
								case var::INT:
									SET_EXPR_CONST_NODE(-args[0]->operator int64_t(), op->pos);
									break;
								case var::FLOAT:
									SET_EXPR_CONST_NODE(-args[0]->operator double(), op->pos);
									break;
								default:
									THROW_ANALYZER_ERROR(Error::OPERATOR_NOT_SUPPORTED,
										String::format("unary operator \"-\" not supported on %s.", args[0]->get_type_name()), op->pos);
							}
							break;
					}
				}
				MISSED_ENUM_CHECK(Parser::OperatorNode::OpType::_OP_MAX_, 33);
			}
		} break;

		case Parser::Node::Type::CONST_VALUE: {
		} break; // Can't reduce anymore.

		default: {
			THROW_BUG("Invalid expression node.");
		}
	}
}


}