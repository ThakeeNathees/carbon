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

#define GET_ARGS(m_nodes)                                                             \
	stdvec<var*> args;                                                                \
	for (int i = 0; i < (int)m_nodes.size(); i++) {                                   \
	    args.push_back(&ptrcast<Parser::ConstValueNode>(m_nodes[i])->value);          \
	}

#define SET_EXPR_CONST_NODE(m_var, m_pos)                                             \
do {                                                                                  \
	ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(m_var);         \
	cv->pos = m_pos, p_expr = cv;                                                     \
} while (false)


void Analyzer::_reduce_indexing(ptr < Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::INDEX);

	ptr<Parser::IndexNode> index = ptrcast<Parser::IndexNode>(p_expr);
	_reduce_expression(index->base);
	ASSERT(index->member->type == Parser::Node::Type::IDENTIFIER);
	ptr<Parser::IdentifierNode> member = ptrcast<Parser::IdentifierNode>(index->member);

	switch (index->base->type) {

		// String.prop; index base on built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			Parser::BuiltinTypeNode* bt = ptrcast<Parser::BuiltinTypeNode>(index->base).get();
			const MemberInfo* mi = TypeInfo::get_member_info(BuiltinTypes::get_var_type(bt->builtin_type), member->name).get();
			if (!mi) throw _analyzer_error(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), BuiltinTypes::get_type_name(bt->builtin_type).c_str()), member->pos);

			switch (mi->get_type()) {
				// var x = String.format;
				case MemberInfo::METHOD:
					break;

				// var x = int.something ? <-- is this even valid
				case MemberInfo::PROPERTY: {
					PropertyInfo* pi = (PropertyInfo*)mi;
					if (pi->is_const()) SET_EXPR_CONST_NODE(pi->get_value(), index->pos);
					else THROW_BUG("can't be."); // builtin types can only have a constant property
				} break;

				// built in types cannot contain enum inside.
				case MemberInfo::ENUM:
				case MemberInfo::ENUM_VALUE:
					THROW_BUG("can't be.");
			}
		} break;

		// "string".member; // TODO: check 1.2 .prop ?
		case Parser::Node::Type::CONST_VALUE: {
			Parser::ConstValueNode* base = ptrcast<Parser::ConstValueNode>(index->base).get();
			try {
				ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(base->value.get_member(member->name));
				cv->pos = member->pos; p_expr = cv;
			} catch (Error& err) {
				throw _analyzer_error(err.get_type(), err.what(), index->pos);
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
				} else if (parser->parser_context.current_class->base_type == Parser::ClassNode::BASE_EXTERN) {
					ptr<Parser::IdentifierNode> _id = newptr<Parser::IdentifierNode>(parser->parser_context.current_class->base_class->name);
					_id->ref = Parser::IdentifierNode::REF_EXTERN;
					_id->_bytecode = parser->parser_context.current_class->base_binary.get();
					index->base = _id;
				}
				_base_class_ref = _SUPER;
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

				// EnumClass.prop; <-- TODO: could the prop be a method?
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
					throw _analyzer_error(Error::OPERATOR_NOT_SUPPORTED, "enum value doesn't support attribute access.", member->pos);

				// Aclass.prop;
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					Parser::IdentifierNode _id = _find_member(base->_class, member->name);
					_id.pos = member->pos;

					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							throw _analyzer_error(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
							THROW_BUG("can't be.");

						// Aclass.a_var <-- only valid if the base is this `this.a_var`
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							_id.pos = member->pos;
							if (_base_class_ref != _THIS && !_id._var->is_static) {
								throw _analyzer_error(Error::ATTRIBUTE_ERROR, String::format("non-static attribute \"%s\" cannot be access with a class reference \"%s\".", member->name.c_str(), base->name.c_str()), member->pos);
							}
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

						// Aclass.CONST;
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							var value;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._const->value;
							else value = _id._prop_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

						// Aclass.EnumClass;
						case Parser::IdentifierNode::REF_ENUM_NAME: {
							_id.pos = member->pos;
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

						// Aclass.ENUM_VALUE
						case Parser::IdentifierNode::REF_ENUM_VALUE: {
							uint64_t value = 0;
							if (_id.ref_base == Parser::IdentifierNode::BASE_LOCAL) value = _id._enum_value->value;
							else value = _id._enum_value_info->get_value();
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(value);
							cv->pos = member->pos; p_expr = cv;
						} break;

						// Aclass.a_function;
						case Parser::IdentifierNode::REF_FUNCTION: {
							_id.pos = member->pos;
							if (_base_class_ref == _THIS) {
								p_expr = newptr<Parser::IdentifierNode>(_id);
							} else {
								index->member = newptr<Parser::IdentifierNode>(_id);
								index->_ref_reduced = true;
							}
						} break;

						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} break;

				// File.prop;
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(base->name));
					BindData* bd = NativeClasses::singleton()->find_bind_data(base->name, member->name).get();
					if (!bd) throw _analyzer_error(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s.", member->name.c_str(), base->name.c_str()), member->pos);
					switch (bd->get_type()) {
						case BindData::METHOD:
						case BindData::STATIC_FUNC:
						case BindData::MEMBER_VAR:
						case BindData::STATIC_VAR:
						case BindData::ENUM:
							break;

						// NativeClass.CONT_VALUE
						case BindData::STATIC_CONST: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((ConstantBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;

						// File.READ
						case BindData::ENUM_VALUE: {
							ptr<Parser::ConstValueNode> cv = new_node<Parser::ConstValueNode>(((EnumValueBind*)bd)->get());
							cv->pos = member->pos; p_expr = cv;
						} break;
					}

				} break;

				// f.attrib;
				case Parser::IdentifierNode::REF_FUNCTION: {
					// at runtime it'll return a ptr<CarbonFunction> reference.
					// TODO: check attribute.
				} break;

				// extern_class.prop;
				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check attrib
				} break;
			}
		}

		default:
			break;
			// RUNTIME.
	}
}


}
