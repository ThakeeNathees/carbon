#include "tests/carbon_tests.h"

class A : public Object {
	INHERITS_OBJECT(A, Object);
public:
	static int A_static_func() { return 1; }
	virtual int A_virtual_func() { return 2; }
	static void _bind_data() {
		BIND_STATIC_FUNC("A_static_func", &A::A_static_func);
		BIND_METHOD("A_virtual_func", &A::A_virtual_func);
	}
};

class B1 : public A {
	INHERITS_OBJECT(B1, A);
public:
	static int A_static_func() { return 3; }
	virtual int A_virtual_func() override { return 4; }
	static void _bind_data() {
		BIND_STATIC_FUNC("A_static_func", &B1::A_static_func);
		BIND_METHOD("A_virtual_func", &B1::A_virtual_func);
	}
};
class B2 : public A {
	INHERITS_OBJECT(B2, A);
public:
	static String B2_static_func() { return "B2-static"; }
	static void _bind_data() {
		BIND_STATIC_FUNC("B2_static_func", &B2::B2_static_func);
	}
};

class C : public B2 {
	INHERITS_OBJECT(C, B2);
public:
	var C_member_func(var v) { return v; }
	static void _bind_data() {
		BIND_METHOD("C_member_func", &C::C_member_func);
	}
};
/////////////////////////////////////////////////////////////////

// POSITIVE TESTS
TEST_CASE("[native_classes:method_bind]") {

	var a  = newptr<A>();
	var b1 = newptr<B1>();
	var b2 = newptr<B2>();
	var c  = newptr<C>();

	A::_register_class();
	B1::_register_class();
	B2::_register_class();
	C::_register_class();

	var r;
	r = a.call_method("A_virtual_func");
	REQUIRE(r.get_type() == var::INT);
	CHECK(r.operator int() == 2);

	r = b1.call_method("A_virtual_func");
	REQUIRE(r.get_type() == var::INT);
	CHECK(r.operator int() == 4);
	r = b2.call_method("A_virtual_func");
	REQUIRE(r.get_type() == var::INT);
	CHECK(r.operator int() == 2);

	r = a.call_method("A_static_func");
	REQUIRE(r.get_type() == var::INT);
	CHECK(r.operator int() == 1);

	r = b1.call_method("A_static_func");
	REQUIRE(r.get_type() == var::INT);
	CHECK(r.operator int() == 3);

	r = b2.call_method("B2_static_func");
	REQUIRE(r.get_type() == var::STRING);
	CHECK(r.to_string() == "B2-static");

	Array arr(1, 2.1, String("test"));
	r = c.call_method("C_member_func", arr);
	REQUIRE(r.get_type() == var::ARRAY);
	CHECK(r.operator Array()[0].operator int() == 1);
	CHECK(r.operator Array()[1].operator double() == 2.1);
	CHECK(r.operator Array()[2].to_string() == "test");

}
