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

#include "carbon.h"
using namespace carbon;

int _main(int argc, char** argv) {

	carbon_initialize();
	log_copyright_and_license();

	try {
		if (argc < 2) {
			log_help();
		} else {

			// TODO: properly parse command line args
			if (strcmp(argv[1], "--native-api") == 0) {
				String path = Path::get_cwd();
				if (argc >= 3) path = Path::absolute(argv[2]);
				NativeLib::generate_api(path);
				printf("%s was generated.\n", path.c_str());
			} else {

				stdvec<String> args;
				for (int i = 1; i < argc; i++) args.push_back(argv[i]);

				ptr<Bytecode> bytecode = Compiler::singleton()->compile(argv[1]);
				VM::singleton()->run(bytecode, args);
			}
		}
	} catch (Throwable& err) {
		err.console_log();
	}

	carbon_cleanup();
	return 0;
}
