
// compile msvc : cl.exe {COMPILE_FLAGS} /LD /std:c++17 mylib.c

#include<stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	__declspec(dllexport) int r0_func_a0() {
		printf("called : r0_func_a0\n");
		return 0;
	}
	__declspec(dllexport) int ra1_func_a1(int ret) {
		printf("called : ra1_func_a1 with: %i\n", ret);
		return ret;
	}

	__declspec(dllexport) int r0_func_a3(int a1, float a2, const char* a3) {
		printf("called : ra1_func_a1 with: %i, %f, %s\n", a1, a2, a3);
		return 0;
	}

#ifdef __cplusplus
}
#endif