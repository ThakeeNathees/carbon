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

#include "../analyzer.h"
#include "binary/carbon_function.h"

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


void Analyzer::_reduce_call(ptr<Parser::Node>& p_expr) {
	ASSERT(p_expr->type == Parser::Node::Type::CALL);

	ptr<Parser::CallNode> call = ptrcast<Parser::CallNode>(p_expr);

	// reduce arguments.
	bool all_const = true;
	for (int i = 0; i < (int)call->args.size(); i++) {
		_reduce_expression(call->args[i]);
		if (call->args[i]->type != Parser::Node::Type::CONST_VALUE) {
			all_const = false;
		}
	}

	// reduce base.
	if (call->base->type == Parser::Node::Type::BUILTIN_FUNCTION || call->base->type == Parser::Node::Type::BUILTIN_TYPE) {
		// don't_reduce_anything();
	} else {
		if (call->base->type == Parser::Node::Type::UNKNOWN) {
			_reduce_expression(call->method);
			if (call->method->type == Parser::Node::Type::CONST_VALUE)
				THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable."), call->pos);
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
		} else {
			_reduce_expression(call->base);
		}
	}

	switch (call->base->type) {

		// print.a_method(); print(); call base is builtin function.
		case Parser::Node::Type::BUILTIN_FUNCTION: {

			if (call->method == nullptr) { // print();
				if (all_const) {
					ptr<Parser::BuiltinFunctionNode> bf = ptrcast<Parser::BuiltinFunctionNode>(call->base);
					if (BuiltinFunctions::can_const_fold(bf->func)) {
						GET_ARGS(call->args);
						if (BuiltinFunctions::is_compiletime(bf->func)) {
							var ret = _call_compiletime_func(bf.get(), args);
							SET_EXPR_CONST_NODE(ret, call->pos);
						} else {
							try {
								var ret;
								BuiltinFunctions::call(bf->func, args, ret);
								SET_EXPR_CONST_NODE(ret, call->pos);
							} catch (Error& err) {
								THROW_ANALYZER_ERROR(err.get_type(), err.get_msg(), call->pos);
							}
						}
					}
				}
			} else { // print.a_method();
				// TODO: check method exists, if (all_const) reduce();
			}
			
		} break;

		// String(); String.format(...); method call on base built in type
		case Parser::Node::Type::BUILTIN_TYPE: {
			if (call->method == nullptr) { // String(...); constructor.
				// CONSTRUCTED OBJECT CANNOT BE A COMPILE TIME CONST VALUE (like Map())
				//if (all_const) {
				//	ptr<Parser::BuiltinTypeNode> bt = ptrcast<Parser::BuiltinTypeNode>(call->base);
				//	try {
				//		GET_ARGS(call->r_args);
				//		var ret = BuiltinTypes::construct(bt->builtin_type, args);
				//		SET_EXPR_CONST_NODE(ret, call->pos);
				//	} catch (Error& err) {
				//		THROW_ANALYZER_ERROR(err.get_type(), err.get_msg(), call->base->pos);
				//	}
				//}
			} else { // String.format(); static func call.
				// TODO: check if exists, reduce if compile time callable.
			}

		} break;

		// method call on base const value.
		case Parser::Node::Type::CONST_VALUE: {
			if (all_const) {
				try {
					ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
					GET_ARGS(call->args); // 0 : const value, 1: name, ... args.
					var ret = ptrcast<Parser::ConstValueNode>(call->base)->value.call_method(ptrcast<Parser::IdentifierNode>(call->method)->name, args);
					SET_EXPR_CONST_NODE(ret, call->pos);
				} catch (const Error& err) {
					THROW_ANALYZER_ERROR(err.get_type(), err.what(), call->method->pos);
				}
			}
		} break;

		// call base is unknown. search method from this to super, or static function.
		case Parser::Node::Type::UNKNOWN: {

			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();
			switch (id->ref) {

				// a_var(); call `__call` method on the variable.
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
					call->base = call->method; // param(args...); -> will call param.__call(args...);
					call->method = nullptr;
				} break;

				// f(); calling a local carbon function.
				case Parser::IdentifierNode::REF_FUNCTION: {

					bool is_illegal_call = false;
					switch (id->ref_base) {
						case Parser::IdentifierNode::BASE_UNKNOWN:
						case Parser::IdentifierNode::BASE_EXTERN:
						case Parser::IdentifierNode::BASE_NATIVE:
							THROW_BUG("can't be"); // call base is empty.

						case Parser::IdentifierNode::BASE_LOCAL: {
							is_illegal_call = parser->parser_context.current_class && !id->_func->is_static;
						} break;
					}

					if (is_illegal_call) { // can't call a non-static function.
						if ((parser->parser_context.current_func && parser->parser_context.current_func->is_static) ||
							(parser->parser_context.current_var && parser->parser_context.current_var->is_static)) {
							THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
						}
					}

					int argc = (int)id->_func->args.size();
					int argc_default = (int)id->_func->default_args.size();
					int argc_given = (int)call->args.size();
					if (argc_given + argc_default < argc) {
						if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
						else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
					} else if (argc_given > argc) {
						if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
						else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
					}
				} break;

				// Aclass(...); calling carbon class constructor.
				case Parser::IdentifierNode::REF_CARBON_CLASS: {
					if (id->_class->constructor) {
						int argc = (int)id->_class->constructor->args.size();
						int argc_default = (int)id->_class->constructor->default_args.size();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
					} else {
						if (call->args.size() != 0)
							THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

				// File(...); calling a native class constructor.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {
					ASSERT(NativeClasses::singleton()->is_class_registered(id->name));
					const StaticFuncBind* initializer = NativeClasses::singleton()->get_constructor(id->name);
					if (initializer) {
						// check arg counts.
						int argc = initializer->get_method_info()->get_arg_count() - 1; // -1 for self argument.
						int argc_default = initializer->get_method_info()->get_default_arg_count();
						int argc_given = (int)call->args.size();
						if (argc_given + argc_default < argc) {
							if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
						} else if (argc_given > argc) {
							if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
							else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
						}
						// check arg types.
						const stdvec<VarTypeInfo>& arg_types = initializer->get_method_info()->get_arg_types();
						for (int i = 0; i < (int)call->args.size(); i++) {
							if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
								var value = ptrcast<Parser::ConstValueNode>(call->args[i])->value;
								if (value.get_type() != arg_types[i + 1].type) // +1 for skip self argument.
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(arg_types[i].type), i), call->args[i]->pos);
							}
						}
					} else {
						if (call->args.size() != 0)
							THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, "default constructor takes exactly 0 argument.", id->pos);
					}
				} break;

				// invalid callables.
				//case Parser::IdentifierNode::REF_ENUM_NAME:
				//case Parser::IdentifierNode::REF_ENUM_VALUE:
				//case Parser::IdentifierNode::REF_FILE:
				//case Parser::IdentifierNode::REF_LOCAL_CONST:
				//case Parser::IdentifierNode::REF_MEMBER_CONST:
				default: {
					THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" is not callable.", id->name.c_str()), id->pos);
				}
			}

		} break;

		// this.method(); super.method();
		case Parser::Node::Type::THIS:
		case Parser::Node::Type::SUPER: {
			const Parser::ClassNode* curr_class = parser->parser_context.current_class;

			if (call->method == nullptr) {
				if (call->base->type == Parser::Node::Type::THIS) { // this(); = __call() = operator ()()
					const Parser::FunctionNode* func = nullptr;
					for (const ptr<Parser::FunctionNode>& fn : curr_class->functions) {
						if (fn->name == "__call") { // TODO: move string literal "__call" to constants
							func = fn.get(); break;
						}
					}
					if (func == nullptr) THROW_ANALYZER_ERROR(Error::NOT_IMPLEMENTED, String::format("operator method __call not implemented on base %s", curr_class->name.c_str()), call->pos);
					_check_arg_count((int)func->args.size(), (int)func->default_args.size(), (int)call->args.size(), call->pos);
				} else { // super();
					if (parser->parser_context.current_class == nullptr || parser->parser_context.current_class->base_type == Parser::ClassNode::NO_BASE ||
						(parser->parser_context.current_class->constructor != parser->parser_context.current_func))
						THROW_ANALYZER_ERROR(Error::SYNTAX_ERROR, "invalid super call.", call->pos);
					if ((parser->parser_context.current_statement_ind != 0) || (parser->parser_context.current_block->statements[0].get() != p_expr.get()))
						THROW_ANALYZER_ERROR(Error::SYNTAX_ERROR, "super call should be the first and stand-alone statement of a constructor.", call->pos);

					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE:
							THROW_BUG("it should be an analyzer error");
						case Parser::ClassNode::BASE_LOCAL: {
							const Parser::FunctionNode* base_constructor = curr_class->base_class->constructor;
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count((int)base_constructor->args.size(), (int)base_constructor->default_args.size(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_NATIVE: {
							const StaticFuncBind* base_constructor = NativeClasses::singleton()->get_constructor(curr_class->base_class_name);
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_argc(), base_constructor->get_method_info()->get_default_arg_count(), (int)call->args.size(), call->pos);
						} break;
						case Parser::ClassNode::BASE_EXTERN: {
							const CarbonFunction* base_constructor = curr_class->base_binary->get_constructor();
							if (base_constructor == nullptr) _check_arg_count(0, 0, (int)call->args.size(), call->pos);
							else _check_arg_count(base_constructor->get_arg_count(), (int)base_constructor->get_default_args().size(), (int)call->args.size(), call->pos);
						} break;
					}
				}

			} else {
				const String& method_name = ptrcast<Parser::IdentifierNode>(call->method)->name;
				if (call->base->type == Parser::Node::Type::THIS) {
					// this.method();
					Parser::IdentifierNode _id = _find_member(curr_class, method_name);
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

						// this.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							call->base = call->method;
							call->method = nullptr;
						} break;

						// this.CONST(); inavlid callables.
						case Parser::IdentifierNode::REF_MEMBER_CONST:
						case Parser::IdentifierNode::REF_ENUM_NAME:
						case Parser::IdentifierNode::REF_ENUM_VALUE:
							THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

						// this.f(); // function call on this.
						case Parser::IdentifierNode::REF_FUNCTION: {
							if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
								THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), _id.pos);
							}

							int argc = (int)_id._func->args.size();
							int argc_default = (int)_id._func->default_args.size();
							_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

						} break;

						case Parser::IdentifierNode::REF_PARAMETER:
						case Parser::IdentifierNode::REF_LOCAL_VAR:
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_CARBON_CLASS:
						case Parser::IdentifierNode::REF_NATIVE_CLASS:
						case Parser::IdentifierNode::REF_EXTERN:
							THROW_BUG("can't be");
					}
				} else { // super.method();


					switch (curr_class->base_type) {
						case Parser::ClassNode::NO_BASE: THROW_BUG("it should be an analyzer error");


						case Parser::ClassNode::BASE_LOCAL: {
							Parser::IdentifierNode _id = _find_member(curr_class->base_class, method_name);
							switch (_id.ref) {
								case Parser::IdentifierNode::REF_UNKNOWN:
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->name.c_str()), call->pos);

								// super.a_var();
								case Parser::IdentifierNode::REF_MEMBER_VAR: {
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;

								// super.CONST(); inavlid callables.
								case Parser::IdentifierNode::REF_MEMBER_CONST:
								case Parser::IdentifierNode::REF_ENUM_NAME:
								case Parser::IdentifierNode::REF_ENUM_VALUE:
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);

								// super.f(); // function call on super.
								case Parser::IdentifierNode::REF_FUNCTION: { // TODO: static function and super -> super.f(); vs SuperClass.f();
									if (parser->parser_context.current_func->is_static && !_id._func->is_static) {
										THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", _id.name.c_str()), call->pos);
									}

									int argc = (int)_id._func->args.size();
									int argc_default = (int)_id._func->default_args.size();
									_check_arg_count(argc, argc_default, (int)call->args.size(), call->pos);

								} break;

								case Parser::IdentifierNode::REF_PARAMETER:
								case Parser::IdentifierNode::REF_LOCAL_VAR:
								case Parser::IdentifierNode::REF_LOCAL_CONST:
								case Parser::IdentifierNode::REF_CARBON_CLASS:
								case Parser::IdentifierNode::REF_NATIVE_CLASS:
								case Parser::IdentifierNode::REF_EXTERN:
									THROW_BUG("can't be");
							}
						} break;

						// super.method(); // super is native
						case Parser::ClassNode::BASE_NATIVE: {
							// TODO: can also check types at compile time it arg is constvalue.
							ptr<BindData> bd = NativeClasses::singleton()->get_bind_data(curr_class->base_class_name, method_name);
							if (bd == nullptr) THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (bd->get_type()) {
								case BindData::METHOD: { // super.method();
									if (parser->parser_context.current_func->is_static) { // calling super method from static function.
										THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", method_name.c_str()), call->pos);
									}
									const MethodInfo* mi = ptrcast<MethodBind>(bd)->get_method_info();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::STATIC_FUNC: { // super.sfunc();
									const MethodInfo* mi = ptrcast<StaticFuncBind>(bd)->get_method_info();
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case BindData::MEMBER_VAR: { // super.a_var();
									THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case BindData::STATIC_VAR:
									break; // OK
								case BindData::STATIC_CONST:
								case BindData::ENUM:
								case BindData::ENUM_VALUE:
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
									break;
							}
						} break;

						// super.method() // super is extern
						case Parser::ClassNode::BASE_EXTERN: {
							const MemberInfo* info = curr_class->base_binary->get_member_info(method_name).get();
							if (info == nullptr) THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" isn't exists in base \"%s\".", method_name.c_str(), curr_class->base_class_name.c_str()), call->pos);
							switch (info->get_type()) {
								case MemberInfo::METHOD: {
									const MethodInfo* mi = static_cast<const MethodInfo*>(info);
									_check_arg_count(mi->get_arg_count(), mi->get_default_arg_count(), (int)call->args.size(), call->pos);
								} break;
								case MemberInfo::PROPERTY: {
									const PropertyInfo* pi = static_cast<const PropertyInfo*>(info);
									if (!pi->is_static()) THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" cannot be access with with \"super\" use \"this\" instead.", method_name.c_str()), call->pos);
								} break;
								case MemberInfo::ENUM:
								case MemberInfo::ENUM_VALUE:
									THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value is not callable.", method_name.c_str()), call->pos);
								case MemberInfo::CLASS:
									THROW_BUG("can't be");
							}
						} break;
					}
				}
			}
			
		} break;

		// get_func()(); arr[0](); (a + b)(); base().method(); [o1, o2][1].method(); (x + y).method();
		case Parser::Node::Type::CALL:
		case Parser::Node::Type::INDEX:
		case Parser::Node::Type::MAPPED_INDEX:
		case Parser::Node::Type::OPERATOR:
			ASSERT(call->method == nullptr || call->method->type == Parser::Node::Type::IDENTIFIER);
			break;

		case Parser::Node::Type::ARRAY: // TODO: the method could be validated.
		case Parser::Node::Type::MAP:   // TODO: the method could be validated.
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			break;


		// idf.method(); method call on base with identifier id.
		case Parser::Node::Type::IDENTIFIER: {
			ASSERT(call->method->type == Parser::Node::Type::IDENTIFIER);
			Parser::IdentifierNode* base = ptrcast<Parser::IdentifierNode>(call->base).get();
			Parser::IdentifierNode* id = ptrcast<Parser::IdentifierNode>(call->method).get();

			switch (base->ref) {

				// IF IDENTIFIER IS UNKNOWN IT'S ALREADY A NAME ERROR BY NOW.
				case Parser::IdentifierNode::REF_UNKNOWN: THROW_BUG("can't be");

				// p_param.method(); a_var.method(); a_member.method();
				case Parser::IdentifierNode::REF_PARAMETER:
				case Parser::IdentifierNode::REF_LOCAL_VAR:
				case Parser::IdentifierNode::REF_MEMBER_VAR: {
				} break;

				// IF AN IDENTIFIER IS REFERENCE TO A CONSTANT IT'LL BE `ConstValueNode` BY NOW.
				case Parser::IdentifierNode::REF_LOCAL_CONST:
				case Parser::IdentifierNode::REF_MEMBER_CONST: {
					THROW_BUG("can't be.");
				} break;

				// Aclass.id();
				case Parser::IdentifierNode::REF_CARBON_CLASS: {

					Parser::IdentifierNode _id = _find_member(base->_class, id->name); _id.pos = id->pos;
					switch (_id.ref) {
						case Parser::IdentifierNode::REF_UNKNOWN:
							THROW_ANALYZER_ERROR(Error::NAME_ERROR, String::format("attribute \"%s\" doesn't exists on base %s", id->name.c_str(), base->_class->name.c_str()), id->pos);

						// Aclass.a_var();
						case Parser::IdentifierNode::REF_MEMBER_VAR: {
							bool _is_member_static = false;
							switch (id->ref_base) {
								case Parser::IdentifierNode::BASE_UNKNOWN: ASSERT(false && "I must forgotten here");
								case Parser::IdentifierNode::BASE_LOCAL:
									_is_member_static = id->_var->is_static;
									break;
								case Parser::IdentifierNode::BASE_NATIVE:
								case Parser::IdentifierNode::BASE_EXTERN:
									_is_member_static = id->_prop_info->is_static();
									break;
							}

							if (_id._var->is_static) {
								break; // Class.var(args...);
							} else {
								THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't access non-static attribute \"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

						// Aclass.CONST(args...);
						case Parser::IdentifierNode::REF_LOCAL_CONST:
						case Parser::IdentifierNode::REF_MEMBER_CONST: {
							THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;

						// Aclass.Enum();
						case Parser::IdentifierNode::REF_ENUM_NAME: THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case Parser::IdentifierNode::REF_ENUM_VALUE: THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") is not callable.", base->name.c_str(), id->name.c_str()), id->pos);

						case Parser::IdentifierNode::REF_FUNCTION: {
							if (_id._func->is_static) {
								break; // Class.static_func(args...);
							} else {
								THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("can't call non-static method\"%s\" statically", id->name.c_str()), id->pos);
							}
						} break;

						// Aclass.Lib();
						case Parser::IdentifierNode::REF_EXTERN: {
							THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("external libraries (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						} break;
					}
				} break;

				// File.method(); base is a native class.
				case Parser::IdentifierNode::REF_NATIVE_CLASS: {

					BindData* bd = NativeClasses::singleton()->get_bind_data(base->name, id->name).get();
					if (!bd) THROW_ANALYZER_ERROR(Error::ATTRIBUTE_ERROR, String::format("attribute \"%s\" does not exists on base %s.", id->name.c_str(), base->name.c_str()), id->pos);
					switch (bd->get_type()) {
						case BindData::STATIC_FUNC: {

							const MemberInfo* memi = bd->get_member_info().get();
							if (memi->get_type() != MemberInfo::METHOD) THROW_BUG("native member reference mismatch.");
							const MethodInfo* mi = (const MethodInfo*)memi;
							if (!mi->is_static()) THROW_BUG("native method reference mismatch.");

							int argc_given = (int)call->args.size();
							int argc = mi->get_arg_count(), argc_default = mi->get_default_arg_count();
							if (argc_given + argc_default < argc) {
								if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected at least %i argument(s).", argc - argc_default), id->pos);
							} else if (argc_given > argc) {
								if (argc_default == 0) THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected exactly %i argument(s).", argc), id->pos);
								else THROW_ANALYZER_ERROR(Error::INVALID_ARG_COUNT, String::format("expected minimum of %i argument(s) and maximum of %i argument(s).", argc - argc_default, argc), id->pos);
							}

							for (int i = 0; i < (int)call->args.size(); i++) {
								if (call->args[i]->type == Parser::Node::Type::CONST_VALUE) {
									if (mi->get_arg_types()[i].type != ptrcast<Parser::ConstValueNode>(call->args[i])->value.get_type()) {
										THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("expected type \"%s\" at argument %i.", var::get_type_name_s(mi->get_arg_types()[i].type), i), call->args[i]->pos);
									}
								}
							}

							// TODO: check if the static function is ok to call at compile time
							//       ex: read a file at a location, print something... are runtime.
							//if (all_const) {
							//	try {
							//		GET_ARGS(call->r_args);
							//		var ret = ptrcast<StaticFuncBind>(bd)->call(args);
							//		SET_EXPR_CONST_NODE(ret, call->pos);
							//	} catch (Error& err) {
							//		THROW_ANALYZER_ERROR(err.get_type(), err.what(), call->pos);
							//	}
							//}

						} break;
						case BindData::STATIC_VAR: break; // calls the "__call" at runtime.
						case BindData::STATIC_CONST: THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("constant value \"%s.%s()\" is not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::METHOD: THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static method \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::MEMBER_VAR:  THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("can't call non-static member \"%s.%s()\" statically.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM:  THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enums (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
						case BindData::ENUM_VALUE: THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("enum value (\"%s.%s()\") are not callable.", base->name.c_str(), id->name.c_str()), id->pos);
					}

				} break;

				// fn.get_default_args(), fn.get_name(), ...
				case Parser::IdentifierNode::REF_FUNCTION: {
					// TODO: check if method exists and args.
				} break;

				case Parser::IdentifierNode::REF_EXTERN: {
					// TODO: check if function exists and check args.
				} break;

				// TODO: EnumType.get_value_count();
				//case Parser::IdentifierNode::REF_ENUM_NAME:
				//case Parser::IdentifierNode::REF_ENUM_VALUE:
				default: {
					THROW_ANALYZER_ERROR(Error::TYPE_ERROR, String::format("attribute \"%s\" doesn't support method calls.", base->name.c_str()), base->pos);
				}
			}

		} break;

		default: {
			THROW_BUG("can't reach here.");
		}
	}
}

}
