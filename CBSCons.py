import os

## USAGE:
##   sys.path.append('path/to/carbon/')
##   import CbSCons as cb_scnos
##   lib = cb_scons.GET_CARBON_LIB(env)

CARBON_DIR = os.path.dirname(__file__)

def ALL_SOURCES(env):
	ALL_SOURCES = [
		## core
		'var/*.cpp',
		'core/%s/*.cpp' % env['platform'],
		'core/*.cpp',
		'native/*.cpp',

		## compiler
		'compiler/analyzer/*.cpp',
		'compiler/binary/*.cpp',
		'compiler/codegen/*.cpp',
		'compiler/compiler/*.cpp',
		'compiler/debugger/*.cpp',
		'compiler/parser/*.cpp',
		'compiler/tokenizer/*.cpp',
		'compiler/types/*.cpp',
		'compiler/vm/*.cpp',
	]
	if env['platform'] == 'windows':
			ALL_SOURCES.append('thirdparty/dlfcn-win32/dlfcn.c')
	return ALL_SOURCES

def INTERNAL_INCLUDE(env):
	INTERNAL_INCLUDE = [
		'thirdparty/dlfcn-win32/',
		'include/core/',
		'include/var/',
		'include/native/',
		'include/compiler/',
	]
	return INTERNAL_INCLUDE


def GET_CARBON_LIB(env):
	SOURCES = []
	cbenv = env.Clone();
	cbenv.Append(CPPPATH=[os.path.join(CARBON_DIR, 'include/')])
	for src in ALL_SOURCES(cbenv):
		SOURCES.append(
			cbenv.Glob(os.path.join(CARBON_DIR, src)))
	for dir in INTERNAL_INCLUDE(cbenv):
		cbenv.Append(CPPPATH=[os.path.join(CARBON_DIR, dir)])
		
	lib = cbenv.Library(
		target = os.path.join(CARBON_DIR, 'bin/carbon'),
		source = SOURCES)
	return lib