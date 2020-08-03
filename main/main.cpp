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

#include "native_classes.h"
#include "parser/parser.h"
#include "io/logger.h"

#include "os/os.h"
#include "io/file.h"
#include "io/dynamic_library.h"
using namespace carbon;

#define _CATCH_
void dl_test();
void parser_test();
void crash_handler_test();
void method_bind_test();


int _main(int argc, char** argv) {
	File::_register_class();
	Buffer::_register_class();
	OS::_register_class();
	DynamicLibrary::_register_class();

	//dl_test();
	//crash_handler_test();
	parser_test();
	//method_bind_test();

	getchar(); // pause
	return 0;
}

// --------	TESTS -----------------------------------

void parser_test() {

#ifdef _CATCH_
	try {
#endif
		String path = "bin/main.cb";
		File file;
		file.open(path);
		String source = file.read();

		Parser p;
		p.parse(source, path);
		p.print_tree();

		Logger::logf_success("\nSuccess: parsing completed.");

#ifdef _CATCH_
	} catch (const Error& err) {
		Logger::logf_error("Error: %s at: %s(%lli, %lli)\n", err.what(), err.get_file().c_str(), err.get_pos().x, err.get_pos().y);
	#if DEBUG_BUILD
		Logger::logf_error("\tat %s (%s:%i)\n", err.get_dbg_func().c_str(), err.get_dbg_file().c_str(), err.get_dbg_line());
	#endif
		Logger::logf_info("%s\n%s\n", err.get_line().c_str(), err.get_line_pos().c_str());

		DEBUG_BREAK();
	} catch (...) {
		DEBUG_BREAK();
	}
#endif

	
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

void method_bind_test() {
#ifdef _CATCH_
	try {
#endif

		var f = newptr<File>();
		f.call_method("open", "bin/main.cb", File::READ);
		String text = f.call_method("read");
		printf("%s\n", text.c_str());
		f.call_method("close");

		var d = newptr<DynamicLibrary>();
		d.call_method("open", "bin/mylib.dll");
		d.call_method("call", "r0_func_a0");
		d.call_method("close");

		//stdvec<var> args = { "bin/main.cb", File::READ };
		//ptrcast<MethodBind>(NativeClasses::get_bind_data(f.get_class_name(), "open"))->call(f, args);
		//args.clear();
		//var text = ptrcast<MethodBind>(NativeClasses::get_bind_data(f.get_class_name(), "read"))->call(f, args);
		//printf("%s\n", text.to_string().c_str());
		//ptrcast<MethodBind>(NativeClasses::get_bind_data(f.get_class_name(), "close"))->call(f, args);

#ifdef _CATCH_
	} catch (const Error& err) {
		Logger::logf_error("Error: %s at: %s(%lli, %lli)\n", err.what(), err.get_file().c_str(), err.get_pos().x, err.get_pos().y);
#if DEBUG_BUILD
		Logger::logf_error("\tat %s (%s:%i)\n", err.get_dbg_func().c_str(), err.get_dbg_file().c_str(), err.get_dbg_line());
#endif
		Logger::logf_info("%s\n%s\n", err.get_line().c_str(), err.get_line_pos().c_str());

		DEBUG_BREAK();
	} catch (...) {
		DEBUG_BREAK();
	}
#endif
}

void crash_handler_test() {
	// crash handler test
	Logger::logf_error("Error: %s\n", "Debug break ...");
	char* invalid_ptr = NULL;
	*invalid_ptr = 0xff;
	//DEBUG_BREAK();
}