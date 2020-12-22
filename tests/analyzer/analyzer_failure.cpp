
#include "../carbon_tests.h"

TEST_CASE("[analyzer_tests]:analyzer_failure") {
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

	CHECK_THROWS__ANALYZE(Error::OPERATOR_NOT_SUPPORTED, "var x = 1 + \"1\";");
	CHECK_THROWS__ANALYZE(Error::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS__ANALYZE(Error::OPERATOR_NOT_SUPPORTED, "var x = 1 / \"1\";");
	CHECK_THROWS__ANALYZE(Error::ZERO_DIVISION, "var x = 1 / 0;");
	CHECK_THROWS__ANALYZE(Error::ZERO_DIVISION, "var x = 1 / 0.00;");

	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "const C = Array();");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "enum { A = 3.14 };");

	CHECK_THROWS__ANALYZE(Error::NAME_ERROR, "const C = identifier;");
	CHECK_THROWS__ANALYZE(Error::NAME_ERROR, "var v = identifier;");
	CHECK_THROWS__ANALYZE(Error::NAME_ERROR, "enum { VAL = identifier, }");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "class Aclass { enum { V = V, } }");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "enum { V = V, }");
	CHECK_THROWS__ANALYZE(Error::NAME_ERROR, "func fn() { var x = identifier; }");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "class C1 : C2 {} class C2 : C1 {}");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "const C1 = C2; const C2 = C1;");

	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "var y = y * 2;");
	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "func f() { var y = y * 2; }");

	// TODO: this should be valid after implementing switch
	CHECK_THROWS__ANALYZE(Error::NAME_ERROR, R"( // TODO: change name error to attribute error.
	func f() { __assert(__func() == "f");
		var x; switch (x) {
			case 1:
				var y = 0;
			case 2:
				print(y);
		}
	})");

	// functions and arguments.
	CHECK_THROWS__ANALYZE(Error::INVALID_ARG_COUNT, "func f(arg1, arg2 = \"default\"){} func g(){ f(); }");
	CHECK_THROWS__ANALYZE(Error::INVALID_ARG_COUNT, "func f(arg1, arg2 = \"default\"){} func g(){ f(1, false, -3.14); }");
	CHECK_THROWS__ANALYZE(Error::INVALID_ARG_COUNT, "func f(arg1 = 1){} class Aclass { func g(){ f(false, true); } }");
	CHECK_THROWS__ANALYZE(Error::INVALID_ARG_COUNT, "func f(){ var f = File(\"path\", File.WRITE, false); }");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "func f() { var f = File(1); }");
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, "func f() { var b = Buffer(false); }");

	// compiletime functions
	CHECK_THROWS__ANALYZE(Error::SYNTAX_ERROR, "__func(); // can't call outside a function");
	CHECK_THROWS__ANALYZE(Error::ASSERTION, "__assert(false);");

	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "class Class { func f() {} } func g() { Class.f(); } ");
	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "class Class { func f(){}  func g() { Class.f(); }  }");
	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "class A{ func f(){} } class B:A{ static func g(){ f(); } }");
	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "class A{ func f(){} } class B:A{ static func g(){ super.f(); } }");
	CHECK_THROWS__ANALYZE(Error::ATTRIBUTE_ERROR, "class A{ /*non static*/ func f(){} } var x = A.f;");

	// super errors

	// TODO: more invalid attribute access tests
}
