
#include "tests/carbon_tests.h"

__assert("testing");
func f() {

}

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

TEST_CASE("[parser_tests]:analyzer_failure") {
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

	// compiletime functions
	CHECK_THROWS_CARBON__ANALYZE(Error::SYNTAX_ERROR, "__func(); // can't call outside a function");
	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "__assert(false);"); // TODO: implement assertion error.
}