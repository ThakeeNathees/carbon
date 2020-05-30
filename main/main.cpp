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

#include <iostream>
#include <string>
#define PRINT(x) std::cout << (x) << std::endl

#include "tokenizer.h"

void logv(const char* fmt, va_list list, bool err) {

	const unsigned int BUFFER_SIZE = 1024;
	char buf[BUFFER_SIZE + 1]; // +1 for the terminating character
	int len = vsnprintf(buf, BUFFER_SIZE, fmt, list);
	if (len <= 0)
		return;
	if ((unsigned int)len >= BUFFER_SIZE)
		len = BUFFER_SIZE; // Output is too big, will be truncated
	buf[len] = 0;
	if (err)
		fprintf(stderr, "%s", buf);
	else
		fprintf(stdout, "%s", buf);

}

void logf(const char* fmt, ...) {
	va_list argp;

	va_start(argp, fmt);

	logv(fmt, argp, false);

	va_end(argp);
}

int _main(int argc, char** argv)
{
	{
		var s = Array();
		{
			carbon::Tokenizer tk;
			s = "testing asdf";
		}
		//s = Dictionary();
		s = Dictionary();
	}

	PRINT("It's goint to crash");
	char* invalid_ptr = NULL;
	*invalid_ptr = 0xff;
	return 0;
}
