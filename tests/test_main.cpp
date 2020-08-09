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

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>


int _test_main(int argc, char** argv) {
	doctest::Context context;

	// !!! THIS IS JUST AN EXAMPLE SHOWING HOW DEFAULTS/OVERRIDES ARE SET !!!

	// defaults
	context.addFilter("test-case-exclude", "[crash_handler]"); // exclude test cases
	context.addFilter("test-case-exclude", "[native:dll]");    // exclude test cases
	context.setOption("rand-seed", 324);                       // if order-by is set to "rand" use this seed
	context.setOption("order-by", "file");                     // sort the test cases by file and line

	context.applyCommandLine(argc, argv);

	// overrides
	context.setOption("no-breaks", true); // don't break in the debugger when assertions fail

	int res = context.run(); // run queries, or run tests unless --no-run is specified

	if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
		return res;          // propagate the result of the tests

	context.clearFilters(); // removes all filters added up to this point

	return res;
}