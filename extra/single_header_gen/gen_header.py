
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

import os

USAGE = '''\
// TODO: write a better usage here:
// #define CARBON_IMPLEMENTATION
// #include "carbon.h"
// 
// define CARBON_NO_MAIN before including "carbon.h" if you don't want to
// implement the carbon entrypoint and embed carbon into your project.
'''

GEN = ""

## TODO: if api.gen.inc, method_bind.gen.h not exists
##       generate them first

ALL_INCLUDES = []

## from path : file it'll generate the source
def gen_file(file): ## file is the complete path
	global GEN
	with open(file, 'r') as fp:
		source = fp.read().replace(LICENSE, "");
				
		if 'main.cpp' in file:
			with open('src/main/crash_handler.h') as ch_source:
				source = source.replace('#include "crash_handler.h"', '\n' + ch_source.read() + '\n')
			
		
		file_macro = "_FILE_%s_" % file \
			.replace('.', '_')   \
			.replace('-', '_')   \
			.replace('/', '_')   \
			.replace('\\', '_')  \
			.upper()
		GEN += f"\n#ifndef {file_macro}\n"
		
		for line in source.splitlines():
			if line.strip().startswith('#include "'):
				ALL_INCLUDES.append(line)
			else:
				GEN += line + '\n'
		
		GEN += f"\n#endif //{file_macro}\n"

## recursivly collect headers
INCLUDED = []
def recursive_header_gen(_path):
	global INCLUDED
	header = ''
	with open(_path, 'r') as file:
		header_source = file.read().replace(LICENSE, '')
		
		for line in header_source.splitlines():
			#line = line.strip()
				
			if line.startswith('#include "'):
				path = line.split('"')[1]
				
				if '/' in path:
					path = 'include/' + path
				else:
					path = os.path.dirname(_path) + '/' + path
				
				if path not in INCLUDED:
					INCLUDED.append(path)
					header += recursive_header_gen(path) + '\n'
				else:
					header += '//' + line + '\n'
			else:
				header += line + '\n'
	return header

## add all internal headers above the source
def append_all_headers():
	global GEN, ALL_INCLUDES
	all_headers = recursive_header_gen('include/carbon.h')
	GEN = LICENSE + '\n'   +                     \
		USAGE + '\n' +                           \
		all_headers + '\n' +                     \
		'#ifdef CARBON_IMPLEMENTATION\n\n' +     \
		GEN + '\n\n' +                           \
		'#endif // CARBON_IMPLEMENTATION\n\n'

def main():
	os.chdir('../../')
	gen_last = [
		'src/main/main.cpp',
	]
	
	for dir in ['src/core', 'src/var', 'src/native', 'src/compiler']:
		for file in os.listdir(dir):
			if not file.endswith('.cpp'):
				continue
				
			file = os.path.join(dir, file)
			if 'platform_' in file:
				gen_last.append(file)
				continue
			gen_file(file)
			
	for last in gen_last:
		gen_file(last)
		
	append_all_headers()
	all_headers = recursive_header_gen('include/carbon.h')
			
	with open('extra/single_header_gen/carbon.h', 'w') as target:
		target.write(GEN)
			
			
if __name__ == '__main__':
	main()