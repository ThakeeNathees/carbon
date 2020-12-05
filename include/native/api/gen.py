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

TARGET_FILE   = 'api.gen.h'
ROOT_HEADER   = 'wrapper.h'
LIB_NAME      = 'CARBON_API'
EXCLUDE_FILES = []

import sys, os, re
def main():
    target = TARGET_FILE
    dir_root = '.'
    if len(sys.argv) >= 3:
        dir_root = sys.argv[1]
        target = sys.argv[2]
    header = generate_header(dir_root)
    inc = generate_inc(header)
    generate_file(target, inc)

####################################################

def generate_header(dir_root):
    src     = dict() ## file_name : source
    headers = dict() ## file_name : [source, included?]
            
    for dir in os.listdir(dir_root):
        if os.path.isfile(os.path.join(dir_root, dir)):
            if dir.endswith('.cpp'):
                with open(dir, 'r') as f:
                    src[dir] = f.read()
            elif dir.endswith('.h') or dir.endswith('.inc'):
                with open(os.path.join(dir_root, dir), 'r') as f:
                    headers[dir] = [f.read(), False]
    
    gen = headers[ROOT_HEADER][0] ## generated source
    done = False
    while not done:
        done = True
        for include in re.findall(r'#include ".+"', gen):
            done = False
            file = include[include.find('"')+1:-1]
            if not headers[file][1]:
                gen = gen.replace(include, headers[file][0], 1)
                headers[file][1] = True;
            else:
                gen = gen.replace(include, include.replace('#', '//'))
                
    ##gen += '\n\n//--------------- IMPLEMENTATION -------------------\n\n'
    ##
    ##gen += '#if defined(%s_IMPLEMENTATION)\n' % LIB_NAME
    ##for file in src:
    ##    if file in EXCLUDE_FILES: continue
    ##    gen += src[file] + '\n'
    ##gen += '\n#endif //  %s_IMPLEMENTATION' % LIB_NAME
    
    for include in re.findall(r'#include ".+"', gen):
        gen = gen.replace(include, include.replace('#', '//'))
    gen = USAGE + '\n' + gen
    gen = LICENSE + gen.replace(LICENSE, '')

    return gen


def generate_inc(header):
    gen = "";
    gen += "const char* NATIVE_API_STR = \n"
    for line in header.splitlines():
    	gen += '"' + line.rstrip().replace('"', '\\"') + '\\n"\n'
    gen += ";\n\n"
    return gen

def generate_file(target, inc):
    with open(target, 'w') as file:
    	file.write(inc)
    print('%s generated' % target)

#######################################################################

if __name__ == '__main__':
    main()
