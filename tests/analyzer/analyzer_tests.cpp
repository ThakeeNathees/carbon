
#include "tests/carbon_tests.h"


#define CHECK_NOTHROW__ANALYZE(m_source)                       \
do {														   \
	parser->parse(m_source, NO_PATH);						   \
	CHECK_NOTHROW(analyzer.analyze(parser));				   \
} while (false)

TEST_CASE("[analyzer_tests]:analyzer_test") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

	CHECK_NOTHROW__ANALYZE(R"(
		__assert(0xc0ffee == 12648430); __assert(0xA == 10); __assert(0X12AaBcDeFf457C == 5254277752636796); 
		__assert(0b10 == 2); __assert(0B10100100010010010 == 84114); __assert(0b1 + 1 == 0B10); __assert((1 << 3) == 0b1000);
		__assert((0b10 | 0b100) == 0b110); __assert((0b1110 & 0b1101) == 0b1100); // C++ like operator precedence.
	)");

	// to test if they are cleaned and optimized.
	CHECK_NOTHROW__ANALYZE("func fn(arg) { \"literal\"; arg; Array(1, 2); }");

	CHECK_NOTHROW__ANALYZE("enum E { V1 = 1 + 2, }");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = - 2, }");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = 1 << 1, V2 = 1 << 2, V3 = V1 | V2 } __assert(E.V3 == 6);");
	CHECK_NOTHROW__ANALYZE("const C = 1; enum E { V1 = C, }");
	CHECK_NOTHROW__ANALYZE("const C = 1 + 2; enum E { V1 = 1 + C, }");

	CHECK_NOTHROW__ANALYZE("const C1 = C2; const C2 = 2;");
	CHECK_NOTHROW__ANALYZE("const C1 = C2; const C2 = C3; const C3 = 3;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = V2, V2 = 2, }");
	CHECK_NOTHROW__ANALYZE("enum { V1 = V2, V2 = V3, V3 = 3}");

	CHECK_NOTHROW__ANALYZE("const C = \"string\".hash();            __assert(C == \"string\".hash());");
	CHECK_NOTHROW__ANALYZE("const C = \"string\".size();            __assert(C == 6);");
	CHECK_NOTHROW__ANALYZE("const C = \"3.14\".to_float();          __assert(C == 3.14);");
	CHECK_NOTHROW__ANALYZE("const C = [1, 2, 3].append(42)[-1];     __assert(C == 42);");
	CHECK_NOTHROW__ANALYZE("const C = [1, [2, 3]].pop()[-1];        __assert(C == 3);");
	CHECK_NOTHROW__ANALYZE("const C = [42].at(0);                   __assert(C == 42);");

	// mapped index.
	CHECK_NOTHROW__ANALYZE("const C = \"string\"[0];                __assert(C == \"s\");");
	CHECK_NOTHROW__ANALYZE("const C = \"string\"[-1];               __assert(C == \"g\");");
	CHECK_NOTHROW__ANALYZE("const C = [1, 2, 3][0];                 __assert(C == 1);");
	CHECK_NOTHROW__ANALYZE("const C = {\"key\":\"value\"}[\"key\"]; __assert(C == \"value\");");
	CHECK_NOTHROW__ANALYZE("class Aclass { const C = \"string\"; } const C = Aclass.C.hash();");

	CHECK_NOTHROW__ANALYZE("func fn() { String; }"); // will throw warning.

	// compiletime functions.
	CHECK_NOTHROW__ANALYZE("__assert(true);");
	CHECK_NOTHROW__ANALYZE("__assert(!false);");
	CHECK_NOTHROW__ANALYZE("__assert(__line() == 1);");
	CHECK_NOTHROW__ANALYZE("__assert(__file() == \"" NO_PATH "\");");
	CHECK_NOTHROW__ANALYZE("func fn() { __assert(__func() == \"fn\"); }");
	CHECK_NOTHROW__ANALYZE("class Aclass { func fn() { __assert(__func() == \"Aclass.fn\"); } }");
	CHECK_NOTHROW__ANALYZE("enum E { V = 1 } __assert(E.V == 1);");
	CHECK_NOTHROW__ANALYZE("const C = __file(); __assert(C == \"" NO_PATH "\")");

	// indexing reduced at compile time.
	CHECK_NOTHROW__ANALYZE("enum E { V = 42 } const C = E.V;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = C, V2 = 42 } const C = E.V2;");
	CHECK_NOTHROW__ANALYZE("enum E { V1 = C, V2 = 42, V3 = 1 } const C = E.V2 + E.V3;");
	CHECK_NOTHROW__ANALYZE("class Name { enum E { V = 42 } } const C = Name.E.V;");
	CHECK_NOTHROW__ANALYZE("class Name { enum E { V = C } } const C = 42;");

	CHECK_NOTHROW__ANALYZE("enum E {V = 42} var arr  = [ E.V ];");
	CHECK_NOTHROW__ANALYZE("enum E {V = 42} var dict = { \"E.V\":E.V };");

	CHECK_NOTHROW__ANALYZE("class ClassName { enum E { V = 42 } func fn(arg) { const C = this.E.V; } }");
	CHECK_NOTHROW__ANALYZE(R"(
		class A {
			func fn() { const A_C = B.E.V + B.B_C1; }
		}
		class B {
			const B_C1 = 2; // TODO: implement 0b10101, 0xc0ffee
			const B_C2 = 8;
			enum E { V = B_C1 & B_C2 }
		}
	)");
	
	// switch case.
	CHECK_NOTHROW__ANALYZE(R"(
		func fn(arg) {
			switch (arg) {
				case "str1".hash(): break;
				case "str2".hash(): break;
				default: break;
			}
		}
	)");


	// function signature
	CHECK_NOTHROW__ANALYZE("func f() { } func g() { f(); } ");
	CHECK_NOTHROW__ANALYZE("func f(a0, a1) { } func g() { f(1, 2); } ");
	CHECK_NOTHROW__ANALYZE("class Class { static func f() { } } func g() { Class.f(); }");
	//CHECK_NOTHROW__ANALYZE("class Class { func f(){}  func g() { Class.f(); }  }"); // should be valid ??

	CHECK_NOTHROW__ANALYZE("var file = File();");
	CHECK_NOTHROW__ANALYZE("var file = File(\"the/path/to/file.txt\");");
	CHECK_NOTHROW__ANALYZE("var buffer = Buffer();");
	CHECK_NOTHROW__ANALYZE("var buffer = Buffer(1000);");

	// TODO:
	CHECK_NOTHROW__ANALYZE("func call(f) { f(); } func fn(){ print(\"fn() called.\"); } var tmp = call(fn);");
	CHECK_NOTHROW__ANALYZE("func f(){} var v = f;");
	CHECK_NOTHROW__ANALYZE("class Aclass { func f(){} } var v = Aclass.f;");
	//CHECK_NOTHROW__ANALYZE("func fn() {} var v = fn.get_name();");
	// if ref of identifier is func pass it as callable object.
	

	// Native classes.
	CHECK_NOTHROW__ANALYZE("const C = File.READ;");
	CHECK_NOTHROW__ANALYZE("const OPEN_MODE = File.READ | File.BINARY;");

}