LICENSE = '''\
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
'''

## this file heavily modified for this project
## use https://gist.github.com/ThakeeNathees/774be053c916d3883038ed7085388198
## for single header generation.

USAGE = '''
/* ------------------------ USAGE ---------------------------

#define CARBON_API_IMPLEMENTATION
#include "carbon_api.h"
using namespace carbon;

#ifdef __cplusplus
extern "C" {
#endif

EXPORT var* your_function(int argc, var** argv) {
    return nullptr;
}

#ifdef __cplusplus
}
#endif

---------------------------------------------------------*/
'''

import sys, os, re

TARGET_FILE   = 'api.gen.inc'
PATH = 'native_api.h'

def generate_api(path, out):
    gen = "const char* NATIVE_API_STR = \n"
    with open(path, 'r') as header:
        for line in header:
            gen += '"' + line.rstrip().replace('"', '\\"') + '\\n"\n'
    gen += ";\n\n"
    with open(out, 'w') as file:
    	file.write(gen)
    print('[api_gen] %s generated' % out)


if __name__ == '__main__':
    generate_api(PATH, TARGET_FILE)
