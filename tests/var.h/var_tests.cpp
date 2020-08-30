
#include "tests/carbon_tests.h"

TEST_CASE("[parser_tests]:var.h_test") {
	var v;

	v = "abc"; v *= 2;
	CHECK(v == "abcabc");
	CHECK(v.call_method("endswith", "bc").operator bool());

	v = -1;
	CHECK(v * "test" == "");

	v = 1; v += 3.14;
	CHECK(v.get_type() == var::FLOAT);

	v = Array(1, "2", 3.14);
	CHECK(v.operator Array()[-1] == 3.14);
	CHECK(v[-2] == "2");
	CHECK(v == var(Array(1, "2", 3.14)));

	CHECK(v.call_method("at", 0) == 1);

	// TODO: v.call_method("append", 10);
}