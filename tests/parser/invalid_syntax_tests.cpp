
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
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("var v1 = {1 2};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::SYNTAX_ERROR, parser.parse("func fn(arg1, arg2 = 42, arg3) {}", NO_PATH));

	// predefined name
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("func name() {}  class name {}", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("func name() {}  var name;", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("func name() {}  enum name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("func name() {}  func name() {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("class name {}   var name;", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("class name {}   func name() {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("class name {}   class name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("var name;       func name() {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("var name;       class name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("var name;       enum name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("enum name {};   enum name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("enum name {};   class name {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("enum {ENUM_NAME,};  class ENUM_NAME {};", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse("func fn(arg_name) { var arg_name; }", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse(R"(
		class Aclass {
			enum { ENUM_NAME = 0 }
			static func ENUM_NAME(){}
		}
	)", NO_PATH));
	CHECK_THROWS_CARBON_ERR(Error::NAME_ERROR, parser.parse(R"(
		func f() {
			var x;
			while (false) {
				var x;
			}
		}
	)", NO_PATH));
}