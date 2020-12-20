
#include "tests/carbon_tests.h"

TEST_CASE("[parser_tests]:invalid_syntax_test") {
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();

	// Eof
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("class"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("class MyClass"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var x"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var x ="));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var x,"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var x = 1"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("var x = \"abc\"[0"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("func"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("func fn("));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("func fn() { if"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("func fn() { if (false"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("func fn() { if (false) { while(false) { } "));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("enum // comment"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("enum {"));
	CHECK_THROWS_ERR(Error::UNEXPECTED_EOF, _PARSE("enum E "));

	// unexp token
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var = 1; "));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = 1 2"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = this"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = super"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("func f() { super; }"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = \"abc\"[]"));
	// CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = \"abc\"[0] = \"A\"")); // assign in expr is valid now
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = print(1 2);"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x = print(1,, 2);"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var x += 1;"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("static func fn(){}"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("class {}"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("func fn(a)) {}"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("blah_blah"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("var v1 = [1 2];"));
	CHECK_THROWS_ERR(Error::SYNTAX_ERROR, _PARSE("func fn(arg1, arg2 = 42, arg3) {}"));

	// predefined name
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("func name() {}  class name {}"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("func name() {}  var name;"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("func name() {}  enum name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("func name() {}  func name() {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("class name {}   var name;"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("class name {}   func name() {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("class name {}   class name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("var name;       func name() {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("var name;       class name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("var name;       enum name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("enum name {};   enum name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("enum name {};   class name {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("enum {ENUM_NAME,};  class ENUM_NAME {};"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE("func fn(arg_name) { var arg_name; }"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE(R"(
		class Aclass {
			enum { ENUM_NAME = 0 }
			static func ENUM_NAME(){}
		}
	)"));
	CHECK_THROWS_ERR(Error::NAME_ERROR, _PARSE(R"(
		func f() {
			var x;
			while (false) {
				var x;
			}
		}
	)"));
}