
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
	v.call_method("append", 10);
	CHECK(v.call_method("pop") == 10);

	v = Array(1);
	CHECK((v * 3).operator Array() == Array(1, 1, 1));

	Map m = Map();
	m["key"] = "value";
	CHECK(m["key"].operator String() == "value");

	// iterator tests.
	var it_arr = Array(1, 2, 3, 4);
	var it1 = it_arr.__iter_begin();
	while (it1.call_method("__iter_has_next").operator bool()) {
		static int i = 0;
		CHECK(it1.call_method("__iter_next").operator int() == ++i);
	} printf("\n");

	m[42] = "life"; m["pi"] = 3.14;
	var it2 = var(m).__iter_begin();
	while (it2.call_method("__iter_has_next").operator bool()) {
		var _pair = it2.call_method("__iter_next");
		if (_pair.get_member("key") == 42) CHECK(_pair.get_member("value") == "life");
		else if (_pair.get_member("key") == "pi") CHECK(_pair.get_member("value") == 3.14);
	}

	// TODO: this will cause a crash since the type shouldn't be allowed to construct.
	// var it3 = NativeClasses::singleton()->construct("_Iterator_Array");
	// CHECK(it3.call_method("__iter_next").operator int() == 1);

	var str1 = "testing";
	var it4 = str1.__iter_begin();
	while (it4.call_method("__iter_has_next").operator bool()) {
		var char_c = it4.call_method("__iter_next");
		//printf("%s ", char_c.to_string().c_str());
	} printf("\n");
}
