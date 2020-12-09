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

#ifndef CARBON_TESTS_H
#define	CARBON_TESTS_H

#include "carbon.h"
using namespace carbon;

#include <doctest/doctest.h>

#define NO_PATH "<NO-PATH-SET>"
#define _PARSE(m_source) parser.parse(m_source, NO_PATH)

#define CHECK_THROWS_ERR(m_type, m_statement)																		        \
	do {																													\
		try {																												\
			m_statement;																									\
		} catch (Throwable& err) {																							\
			CHECK_MESSAGE(err.get_type() == m_type, String::format("expected error: \"%s\" got \"%s\"\n     msg: %s",		\
				Error::get_err_name(m_type).c_str(), Error::get_err_name(err.get_type()).c_str(), err.what()));             \
			break;																											\
		}																													\
		CHECK_MESSAGE(false, String::format("expected error: \"%s\" but no error has thrown",								\
			Error::get_err_name(m_type).c_str()));																			\
	} while (false)


#define CHECK_NOTHROW__ANALYZE(m_source)                 \
do {													 \
	parser->parse(m_source, NO_PATH);					 \
	CHECK_NOTHROW(analyzer.analyze(parser));			 \
} while (false)

#define CHECK_THROWS__ANALYZE(m_type, m_source)          \
do {												     \
	parser->parse(m_source, NO_PATH);				     \
	CHECK_THROWS_ERR(m_type, analyzer.analyze(parser));  \
} while(false)

#define CHECK_NOTHROW__CODEGEN(m_source)                   \
do {													   \
	parser->parse(m_source, NO_PATH);					   \
	analyzer->analyze(parser);							   \
	CHECK_NOTHROW(bytecode = codegen.generate(analyzer));  \
} while (false)

#define CHECK_NOTHROW__VM(m_source)                        \
do {										               \
	parser->parse(m_source, NO_PATH);					   \
	analyzer->analyze(parser);				               \
	bytecode = codegen->generate(analyzer);	               \
	VM::singleton()->run(bytecode, argv);	               \
} while (false)


template<typename... Targs>
var call_method(var & p_var, const String & p_method, Targs... p_args) {
	stdvec<var> _args = make_stdvec<var>(p_args...);
	stdvec<var*> args; for (var& v : _args) args.push_back(&v);
	return p_var.call_method(p_method, args);
}

#endif // CARBON_TESTS_H
