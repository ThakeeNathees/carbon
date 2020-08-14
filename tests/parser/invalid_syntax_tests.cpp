
#include "tests/carbon_tests.h"

TEST_CASE("[parser_tests]:invalid_syntax_test") {
	Parser parser;

	// Eof
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("class", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("class MyClass", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var x", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var x =", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var x,", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var x = 1", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("var x = \"abc\"[0", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("func", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("func fn(", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("func fn() { if", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("func fn() { if (false", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("func fn() { if (false) { while(false) { } ", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("enum // comment", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("enum {", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::UNEXPECTED_EOF, parser.parse("enum E ", NO_PATH));

	// unexp token
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var = 1; ", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = 1 2", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = this", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = super", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = \"abc\"[]", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = \"abc\"[0] = \"A\"", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = print(1 2);", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x = print(1,, 2);", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var x += 1;", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("static func fn(){}", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("class {}", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("func fn(a)) {}", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("blah_blah", NO_PATH));

}