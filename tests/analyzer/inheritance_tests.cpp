
#include "tests/carbon_tests.h"

TEST_CASE("[analyzer_tests]:inheritance_test+") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

	//CHECK_NOTHROW__ANALYZE(R"()");

	CHECK_NOTHROW__ANALYZE(R"( class MyFile : File {
		func open(path, mode = MyFile.READ) { super.open(path, mode); }
	})");

	// TODO: not sure the below use of super should be valid.
	//CHECK_NOTHROW__ANALYZE(R"( class Base { func f() = 1; } class Derived : Base { var x = super.f(); } )");

	CHECK_NOTHROW__ANALYZE(R"( class A  { func f() {} }
	class B : A {} class C : B { func f() {  super.f(); } }
	)");

	CHECK_NOTHROW__ANALYZE(R"( class A  { const C = "CONST_STR"; }
	class B : A { const C2 = C; }  __assert(B.C2 == "CONST_STR");
	)");


}

TEST_CASE("[analyzer_tests]:inheritance_test+") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

	// ctor required
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, R"(
	class Base { func Base(arg1) {} }
	class Dirived : Base { }
)");

	// ctor argument size
	CHECK_THROWS__ANALYZE(Error::INVALID_ARG_COUNT, R"(
	class Dirived : Base { func Dirived() { super(1); } }
	class Base { func Base(arg1, arg2, arg3="default") {} }
)");

	// cyclic inheritance
	CHECK_THROWS__ANALYZE(Error::TYPE_ERROR, R"( class A : B {} class B : C {} class C : A {} )");



}