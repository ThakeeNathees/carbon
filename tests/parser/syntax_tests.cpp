
#include "tests/carbon_tests.h"

TEST_CASE("[parser_tests]:syntax_test") {
	Parser parser;

	// variables.
	CHECK_NOTHROW(parser.parse("var v1; var v2 = 2; var v3 = 3, v4 = 4, v5 = 5;", NO_PATH));
	CHECK_NOTHROW(parser.parse("var v1 = \"string\";", NO_PATH));
	CHECK_NOTHROW(parser.parse("var v1 = 1 + 2 * ((3 / 4) - (5 + 6));", NO_PATH));
	CHECK_NOTHROW(parser.parse("var v1 = {1, 2, 3}.append(Array(4, 5, 6));", NO_PATH));

	// enums.
	CHECK_NOTHROW(parser.parse("enum { }", NO_PATH));
	CHECK_NOTHROW(parser.parse("enum En { }", NO_PATH));
	CHECK_NOTHROW(parser.parse("enum { V1 }", NO_PATH));
	CHECK_NOTHROW(parser.parse("enum { V1, }", NO_PATH));
	CHECK_NOTHROW(parser.parse("enum En { V1, V2 = 1 , V3 = 0, }", NO_PATH));
	// TODO: reduce expr: CHECK_NOTHROW(parser.parse("enum { V1, V2 = 1 * (2 + 3), V3 }", NO_PATH));

	// functions.
	CHECK_NOTHROW(parser.parse("func fn(){}", NO_PATH));
	CHECK_NOTHROW(parser.parse("func fn(a){}", NO_PATH));
	CHECK_NOTHROW(parser.parse("func fn(a, b, c){}", NO_PATH));

	// classes.
	CHECK_NOTHROW(parser.parse("class cls {}", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { static func fn(){}}", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { static func fn(a, b, c){}}", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { var v = 1; func f(){} }", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { enum {} }", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { enum E {} }", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls { enum {V1, V2 = 1, } }", NO_PATH));
	CHECK_NOTHROW(parser.parse("class cls {} class cls2 {}", NO_PATH));

	// if statement.
	CHECK_NOTHROW(parser.parse(R"(
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
	)", NO_PATH));

	// for loop.
	CHECK_NOTHROW(parser.parse(R"(
	func fn() {
		for(;;);
		for(var i = 1 + 2; i < 10; i+=1) print(i);
		for(var i = 0; i <= 10; i+=1) { var in = input(); print(in); }
	}	
	)", NO_PATH));

	// while loop.
	CHECK_NOTHROW(parser.parse(R"(
	func fn() {
		// a comment.
		while(1 + 2);
		while(false) print("test");
		while(false) { while (true) print("test"); }
	}	
	)", NO_PATH));

	// switch statement.
	CHECK_NOTHROW(parser.parse(R"(
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
	)", NO_PATH));


}