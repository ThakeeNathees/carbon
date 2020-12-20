
#include "tests/carbon_tests.h"

TEST_CASE("[parser_tests]:valid_syntax_test") {
	ptr<Tokenizer> tokenizer = newptr<Tokenizer>();
	ptr<Parser> parser = newptr<Parser>();

	// variables.
	CHECK_NOTHROW(_PARSE("var v1; var v2 = 2; var v3 = 3, v4 = 4, v5 = 5;"));
	CHECK_NOTHROW(_PARSE("var v1 = \"string\";"));
	CHECK_NOTHROW(_PARSE("var v1 = 1 + 2 * ((3 / 4) - (5 + 6));"));
	CHECK_NOTHROW(_PARSE("var v1 = [1, 2, 3].append(Array(4, 5, 6));"));

	// arrays and maps.
	CHECK_NOTHROW(_PARSE("var arr = [];"));
	CHECK_NOTHROW(_PARSE("var arr = [1, \"2\", 3.14, 4,];"));
	CHECK_NOTHROW(_PARSE("var arr = [1, \"2\", 3.14, 4];"));
	CHECK_NOTHROW(_PARSE("var arr = [1, [], [[]], Array(Map())];"));
	CHECK_NOTHROW(_PARSE("var arr = [ [\"testing\"[0]], [42].pop() ];"));
	CHECK_NOTHROW(_PARSE("var dict = { \"key\":\"value\", 1:2, \"3\":Array(), };"));
	CHECK_NOTHROW(_PARSE("var dict = {};"));
	CHECK_NOTHROW(_PARSE("var dict = {0:{},};"));
	CHECK_NOTHROW(_PARSE("var dict = {0:{ 1:2, }};"));

	// enums.
	CHECK_NOTHROW(_PARSE("enum { }"));
	CHECK_NOTHROW(_PARSE("enum En { }"));
	CHECK_NOTHROW(_PARSE("enum { V1 }"));
	CHECK_NOTHROW(_PARSE("enum { V1, }"));
	CHECK_NOTHROW(_PARSE("enum En { V1, V2 = 1 , V3 = 0, }"));

	// functions.
	CHECK_NOTHROW(_PARSE("func fn() {}"));
	CHECK_NOTHROW(_PARSE("func fn(a) {}"));
	CHECK_NOTHROW(_PARSE("func fn(a, b, c) {}"));
	CHECK_NOTHROW(_PARSE("func fn(a, b) = a + 2*b;"));

	// classes.
	CHECK_NOTHROW(_PARSE("class cls {}"));
	CHECK_NOTHROW(_PARSE("class cls { static func fn(){}}"));
	CHECK_NOTHROW(_PARSE("class cls { static func fn(a, b, c){}}"));
	CHECK_NOTHROW(_PARSE("class cls { var v = 1; func f(){} }"));
	CHECK_NOTHROW(_PARSE("class cls { enum {} }"));
	CHECK_NOTHROW(_PARSE("class cls { enum E {} }"));
	CHECK_NOTHROW(_PARSE("class cls { enum {V1, V2 = 1, } }"));
	CHECK_NOTHROW(_PARSE("class cls {} class cls2 {}"));

	CHECK_NOTHROW(_PARSE("func fn(arg1, arg2) { print(arg1); }"));
	CHECK_NOTHROW(_PARSE("func fn(arg1, arg2 = 42) {}"));

	// if statement.
	CHECK_NOTHROW(_PARSE(R"(
	func fn() {
		if (false);

		if (false) print("false");
		else      { print(true); }

		if (true) {
		} else if (false) {
			print("testing");
		} else if (true) {
		} else;
	}
	)"));

	// for loop.
	CHECK_NOTHROW(_PARSE(R"(
	func fn() {
		for(;;);
		for(var i = 1 + 2; i < 10; i+=1) print(i);
		for(var i = 0; i <= 10; i+=1) { var in = input(); print(in); }
	}
	)"));

	// foreach loop
	CHECK_NOTHROW(_PARSE(R"(
	func fn() {
		for (var i : 10);
		for (var c : "string") println(c);
		for (var e : [1, 2, 3]);

		var maybe_iterable;
		for (var i : maybe_iterable) {}
	}
	)"));

	// while loop.
	CHECK_NOTHROW(_PARSE(R"(
	func fn() {
		// a comment.
		while(1 + 2);
		while(false) print("test");
		while(false) { while (true) print("test"); }
	}	
	)"));

	CHECK_NOTHROW(_PARSE("func f(x) { while (x) { if (false) break; } }"));

	// switch statement.
	CHECK_NOTHROW(_PARSE(R"(
	func fn(c) {
		switch(c) {
			case 0:
		/*
			multi line comment.
		*/
			case 1:
				var x = "all case body are blocks";
				break;
			case 2:
			case 1 + 2 :
				print("case 0, 1");
				break;
			default:
				print("testing");
		}
	}
	)"));

	// multi line string
	CHECK_NOTHROW(_PARSE(R"(
	var s = "\\
	line 1
	line 2
	";
	)"));

}