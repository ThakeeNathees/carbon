
#include "tests/carbon_tests.h"


#define CHECK_NOTHROW__ANALYZE(m_source)                       \
do {														   \
	parser->parse(m_source, NO_PATH);						   \
	CHECK_NOTHROW(analyzer.analyze(parser));				   \
} while (false)

TEST_CASE("[analyzer_tests]:analyzer_test") {
	ptr<Parser> parser = newptr<Parser>();
	Analyzer analyzer;

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

	CHECK_NOTHROW__ANALYZE("const C = \"string\".hash();");
	CHECK_NOTHROW__ANALYZE("const C = \"string\".size();");
	CHECK_NOTHROW__ANALYZE("const C = \"3.14\".to_float();");
	CHECK_NOTHROW__ANALYZE("const C = [1, 2, 3].append(42)[-1];");
	CHECK_NOTHROW__ANALYZE("const C = [1, [2, 3]].pop()[-1];");

	// mapped index.
	CHECK_NOTHROW__ANALYZE("const C = \"string\"[0];");
	CHECK_NOTHROW__ANALYZE("const C = \"string\"[-1];");
	CHECK_NOTHROW__ANALYZE("const C = [1, 2, 3][0];");
	CHECK_NOTHROW__ANALYZE("const C = {\"key\":\"value\"}[\"key\"];");

	// compiletime functions.
	CHECK_NOTHROW__ANALYZE("__assert(true);");
	CHECK_NOTHROW__ANALYZE("__assert(!false);");
	CHECK_NOTHROW__ANALYZE("__assert(__line() == 1);");
	CHECK_NOTHROW__ANALYZE("__assert(__file() == \"" NO_PATH "\");");
	CHECK_NOTHROW__ANALYZE("func fn() { __assert(__func() == \"fn\"); }");
	CHECK_NOTHROW__ANALYZE("class Aclass { func fn() { __assert(__func() == \"Aclass.fn\"); } }");
	CHECK_NOTHROW__ANALYZE("enum E { V = 1 } __assert(E.V == 1);");
	CHECK_NOTHROW__ANALYZE("const C = __file();");

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
	//CHECK_NOTHROW__ANALYZE("class Class { static func f() { } } func g() { Class.f(); } "); // remove static for failure test.

	CHECK_NOTHROW__ANALYZE("var file = File();");
	CHECK_NOTHROW__ANALYZE("var file = File(\"the/path/to/file.txt\");");
	CHECK_NOTHROW__ANALYZE("var buffer = Buffer();");
	CHECK_NOTHROW__ANALYZE("var buffer = Buffer(1000);");

}