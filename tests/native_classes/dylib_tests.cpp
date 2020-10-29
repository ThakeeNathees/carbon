#include "tests/carbon_tests.h"

TEST_CASE("[native_classes:dylib]") {

	// // compile msvc : cl.exe /LD mylib.c
	// #include<stdio.h>
	// 
	// #ifdef __cplusplus
	// extern "C" {
	// #endif
	// 
	// 	__declspec(dllexport) int r0_func_a0() {
	// 		printf("called : r0_func_a0\n");
	// 		return 0;
	// 	}
	// 	__declspec(dllexport) int ra1_func_a1(int ret) {
	// 		printf("called : ra1_func_a1 with: %i\n", ret);
	// 		return ret;
	// 	}
	// 
	// 	__declspec(dllexport) int r0_func_a3(int a1, float a2, const char* a3) {
	// 		printf("called : ra1_func_a1 with: %i, %f, %s\n", a1, a2, a3);
	// 		return 0;
	// 	}
	// 
	// #ifdef __cplusplus
	// }
	// #endif

#if defined(PLATFORM_WINDOWS)
	DynamicLibrary lib("bin/mylib.dll");
#elif defined(PLATFORM_X11)
	DynamicLibrary lib("bin/mylib.so");
#else
#error "dynamic library is not implemented in this platform yet"
#endif

	var i = 42, f = 3.14, s = "hello";
	int ret = 0;
	ret = lib.call("r0_func_a0");
	CHECK(ret == 0);

	ret = lib.call("ra1_func_a1", i);
	CHECK(ret == (int)i);

	lib.close();

	var lib2 = NativeClasses::construct(DynamicLibrary::get_class_name_s(), make_stdvec<var>("bin/mylib.dll"));
	ret = lib2.call_method("call", "r0_func_a3", i, f, s);
	CHECK(ret == 0);
}

