
#include "tests/carbon_tests.h"

#define CHECK_THROWS_VARH__ANALYZE(m_type, m_source)            \
do {												            \
	parser->parse(m_source, NO_PATH);				            \
	CHECK_THROWS_VARH_ERR(m_type, analyzer.analyze(parser));    \
} while(false)

#define CHECK_THROWS_CARBON__ANALYZE(m_type, m_source)          \
do {												            \
	parser->parse(m_source, NO_PATH);				            \
	CHECK_THROWS_CARBON_ERR(m_type, analyzer.analyze(parser));  \
} while(false)

#define CHECK_NOTHROW__ANALYZE(m_source)                       \
do {														   \
	parser->parse(m_source, NO_PATH);						   \
	CHECK_NOTHROW(analyzer.analyze(parser));				   \
} while (false)

TEST_CASE("[parser_tests]:analyzer_test") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 + \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::ZERO_DIVISION, "var x = 1 / 0;");
	CHECK_THROWS_VARH__ANALYZE(VarError::ZERO_DIVISION, "var x = 1 / 0.00;");

	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "const C = Array();");
	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "enum { A = 3.14 };");

	CHECK_THROWS_CARBON__ANALYZE(Error::NOT_DEFINED, "const C = identifier;");
	CHECK_THROWS_CARBON__ANALYZE(Error::NOT_DEFINED, "var v = identifier;");
	CHECK_THROWS_CARBON__ANALYZE(Error::NOT_DEFINED, "enum { VAL = identifier, }");
	CHECK_THROWS_CARBON__ANALYZE(Error::NOT_DEFINED, "func fn() { var x = identifier; }");
	// TODO: implement fix the error types, may need to implement more types.
	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "class C1 : C2 {} class C2 : C1 {}");
	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "const C1 = C2; const C2 = C1;");


	// to test if they are cleaned and optimized.
	CHECK_NOTHROW__ANALYZE("func fn(arg) { \"literal\"; arg; Array(1, 2); }");

	CHECK_NOTHROW__ANALYZE("enum E { V1 = 1 + 2, }");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = - 2, }");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = 1 << 1, V2 = 1 << 2, V3 = V1 | V2 }");
	CHECK_NOTHROW__ANALYZE("const C = 1; enum E { V1 = C, }");
	CHECK_NOTHROW__ANALYZE("const C = 1 + 2; enum E { V1 = 1 + C, }");

	CHECK_NOTHROW__ANALYZE("const C1 = C2; const C2 = 2;");
	CHECK_NOTHROW__ANALYZE("const C1 = C2; const C2 = C3; const C3 = 3;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = V2, V2 = 2, }");
	CHECK_NOTHROW__ANALYZE("enum { V1 = V2, V2 = V3, V3 = 3}");

	// indexing reduced at compile time.
	CHECK_NOTHROW__ANALYZE("enum E { V = 42 } const C = E.V;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = C, V2 = 42 } const C = E.V2;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = C, V2 = 42, V3 = 1 } const C = E.V2 + E.V3;");
	CHECK_NOTHROW__ANALYZE("class Name { enum E { V = 42 } } const C = Name.E.V;");
	CHECK_NOTHROW__ANALYZE("class Name { enum E { V = C } } const C = 42;");
	//CHECK_NOTHROW__ANALYZE("class ClassName { enum E {V = 42,} func fn(arg) { this.E.V; } }");
	
}