
#include "tests/carbon_tests.h"

#define CHECK_THROWS_VARH__ANALYZE(m_type, m_source) \
do {												 \
	parser.parse(m_source, NO_PATH);				 \
	CHECK_THROWS_VARH_ERR(m_type, parser.analyze()); \
} while(false)

#define CHECK_THROWS_CARBON__ANALYZE(m_type, m_source) \
do {												   \
	parser.parse(m_source, NO_PATH);				   \
	CHECK_THROWS_CARBON_ERR(m_type, parser.analyze()); \
} while(false)

TEST_CASE("[parser_tests]:analyzer_test") {
	Parser parser;

	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 + \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS_VARH__ANALYZE(VarError::ZERO_DIVISION, "var x = 1 / 0;");
	CHECK_THROWS_VARH__ANALYZE(VarError::ZERO_DIVISION, "var x = 1 / 0.00;");

	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "const C = identifier;");
	CHECK_THROWS_CARBON__ANALYZE(Error::INVALID_TYPE, "const C = Array();");

}