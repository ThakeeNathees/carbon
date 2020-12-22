
#include "../carbon_tests.h"

TEST_CASE("[parser_tests]:var_test") {
	var v;

	v = "abc"; v *= 2;
	CHECK(v == "abcabc");
	CHECK(call_method(v, "endswith", "bc").operator bool());

	v = -1;
	CHECK(v * "test" == "");

	v = 1; v += 3.14;
	CHECK(v.get_type() == var::FLOAT);

	v = Array(1, "2", 3.14);
	CHECK(v.operator Array()[-1] == 3.14);
	CHECK(v[-2] == "2");

	CHECK(call_method(v, "at", 0) == 1);
	call_method(v, "append", 10);
	CHECK(call_method(v, "pop") == 10);

	v = Array(1);
	CHECK((v * 3).operator Array() == Array(1, 1, 1));

	Map m = Map();
	m["key"] = "value";
	CHECK(m["key"].operator String() == "value");

	// iterator tests.
	var it_arr = Array(1, 2, 3, 4);
	var it1 = it_arr.__iter_begin();
	while (it1.__iter_has_next()) {
		static int i = 0;
		CHECK(it1.__iter_next() == ++i);
	} //printf("\n");

	m[42] = "life"; m["pi"] = 3.14;
	var it2 = var(m).__iter_begin();
	while (it2.__iter_has_next()) {
		var _pair = it2.__iter_next();
		if (_pair.get_member("key") == 42) CHECK(_pair.get_member("value") == "life");
		else if (_pair.get_member("key") == "pi") CHECK(_pair.get_member("value") == 3.14);
	}

	{
		const char* str = "testing";
		var keep_alive = str; // var(str).__iter_begin() will delete it.
		var it4 = keep_alive.__iter_begin();
		int i = 0;
		while (it4.__iter_has_next()) {
			var char_c = it4.__iter_next();
			CHECK_EQ(str[i], *char_c.to_string().c_str());
			i++;
		}
	}
}
