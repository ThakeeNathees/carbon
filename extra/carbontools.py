
import os, sys
import shutil

CARBON_DIR = os.path.dirname(__file__)

USAGE = '''\
'''

## USAGE:
##   sys.path.append('path/to/carbon/')
##   import carbontools.py as cbtools
##   lib = cbtools.GET_CARBON_LIB(env)
def GET_CARBON_LIB(env):
	## TODO: generate "*.gen.h" files
	SOURCES = []
	cbenv = env.Clone();
	cbenv.Append(CPPPATH=[os.path.join(CARBON_DIR, 'include/')])
	ALL_SOURCES = [
		'src/var/*.cpp',
		'src/core/*.cpp',
		'src/native/*.cpp',
		'src/compiler/*.cpp',
		'src/thirdparty/dlfcn-win32/*.c',
	]
	for src in ALL_SOURCES(cbenv):
		SOURCES.append(cbenv.Glob(os.path.join(CARBON_DIR, src)))
		
	lib = cbenv.Library(
		target = os.path.join(CARBON_DIR, 'bin/carbon'),
		source = SOURCES)
	return lib


def main():
	argcount = len(sys.argv)
	if argcount < 2:
		print(USAGE_STRING)
		exit()

	## switch commands
	if sys.argv[1] == 'clean':
		cleanall = False
		for i in range(2, argcount):
			if sys.argv[i] in ('--all', '-a'):
				cleanall = True
			else:
				error_command(sys.argv[i])
		clean(cleanall)

	else:
		error_command(sys.argv[1])


## Internal methods ####
def error_command(cmd):
	print('[*]: ERROR: unknown command "'+ cmd + '"\n' + USAGE)
	exit(-1)

def error_exit(msg):
	print('[*]: ERROR: ' + msg + '"\n' + USAGE)
	exit(-1)

def get_platform():
	if sys.platform == 'win32': return 'windows'
	elif sys.platform in ('linux', 'linux2'): return 'x11'
	elif sys.platform == 'darwin': return 'osx'
	else: error_exit("platform(%s) not supported." % sys.platform)

def clean():
	CLEAN_DIRS = [
		'x64/',
		'debug/'
		'release/',
		'debug/',
		'bin/',
	
		'.vs',
		'.vscode',
	]
	
	CLEAN_FILES = [
		'.pdb',
		'.idb',
		'.ilk',
		'.obj',
		'.sln',
		'.vcxproj',
		'.vcxproj.filters',
		'.vcxproj.user',
		'.sconsign.dblite',
	]

	os.system('scons -c')
	print('\n[*]: cleaning all files ...')
	for _dir in CLEAN_DIRS:
		try:
			shutil.rmtree(_dir)
			print('[*]: Removed - %s' % _dir)
		except:
			pass
	for path, dirs, files in os.walk('.'):
		for file in files:
			for suffix in CLEAN_FILES:
				if file.endswith(suffix):
					os.remove(os.path.join(path, file))
					print('[*]: Removed - %s' % os.path.join(path, file))
	print('[*]: done cleaning targets.')


if __name__ == '__main__':
	main()