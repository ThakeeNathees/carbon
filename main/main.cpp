//------------------------------------------------------------------------------
// MIT License
//------------------------------------------------------------------------------
// 
// Copyright (c) 2020 Thakee Nathees
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

//#include <iostream>
#include <string>
#define PRINT(x) std::cout << (x) << std::endl


#include "parser/parser.h"
#include "io/console_logger.h"
#include "io/file.h"
#include "io/dynamic_library.h"
using namespace carbon;

void dl_test();
void parser_test();
void crash_handler_test();

#define _CATCH_

int _main(int argc, char** argv) {
	
	//dl_test();
	//crash_handler_test();
#ifdef _CATCH_
	try {
#endif
		parser_test();
		ConsoleLogger::logf_info("Parsing success.");

#ifdef _CATCH_
	} catch (const Error & err) {
		ConsoleLogger::logf_error("Error:\n%s\n", err.what());
		DEBUG_BREAK();
	} catch (...) {
		DEBUG_BREAK();
	}
#endif

	int c = getchar();
	return 0;
}

// --------	TESTS -----------------------------------

void parser_test() {
	String path = "bin/main.cb";
	File file;
	file.open(path);
	String source = file.read();

	Parser p;
	p.parse(source, path);
}

void dl_test() {
	// dl loader test
	// the source:
	// /*** use extern "C" ***/
	// #include<stdio.h>
	// 
	// extern "C" {
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
	// }
	// /**********************/
#if defined(PLATFORM_WINDOWS)
	DynamicLibrary lib("bin/mylib.dll");
#elif defined(PLATFORM_X11)
	DynamicLibrary lib("bin/mylib.so");
#else
	#error ""
#endif

	var i = 42, f = 3.14, s = "hello";
	int ret =0;
	ret = lib.call("r0_func_a0");
	PRINT(ret);
	ret = lib.call("ra1_func_a1", i);
	PRINT(ret);
	ret = lib.call("r0_func_a3", i, f, s);
	PRINT(ret);
}

void crash_handler_test() {
	// crash handler test
	ConsoleLogger::logf_error("Error: %s\n", "Debug break ...");
	char* invalid_ptr = NULL;
	*invalid_ptr = 0xff;
	//DEBUG_BREAK();
}