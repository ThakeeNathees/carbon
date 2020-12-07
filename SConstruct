#!python
import os, subprocess, sys

###### USER DATA #############################################################################
def USER_DATA(env):
	env.PROJECT_NAME = 'carbon'
	
	## generate files TODO: use builders
	os.system('python include/core/native_gen.py include/core/native_bind.gen.h')
	os.system('python %s %s %s' % ("include/native/api/gen.py", "include/native/api/", "native/io/api.gen.inc") )
	
	env.Append(CPPPATH=[Dir(path) for path in [
			"./thirdparty",
			"./",
			"./include/",
			"./include/core/",
			"./include/var/",
			"./include/native/",
			"./include/compiler/",
	]])

	env.SOURCES_CORE     = []
	env.SOURCES_COMPILER = []
	env.SOURCES_MAIN = [
		Glob('main/platform/%s/*.cpp' % cbenv['platform'] ),
		Glob('main/*.cpp'),
	]
	env.SOURCES_TEST     = [
		Glob('main/platform/%s/*.cpp' % cbenv['platform'] ),
		Glob('tests/*.cpp'),
		Glob('tests/var/*.cpp'),
		Glob('tests/native_classes/*.cpp'),
		Glob('tests/parser/*.cpp'),
		Glob('tests/analyzer/*.cpp'),
	]

	env.SCONSCRIPTS = [
		'thirdparty/SConstruct',

		'var/SConstruct',
		'core/SConstruct',
		'native/SConstruct',
		'compiler/SConstruct',
	]

	# if env['libs'] true -> sources will compile as separate libs as below.
	env.LIBS = { "cb_core" : env.SOURCES_CORE, "cb_compiler" : env.SOURCES_COMPILER }
	env.DEBUG_TESTS = True ## if true vsproj run tests to debug.

	if env['target'] == 'debug':
		env.Append(CPPDEFINES=['DEBUG_BUILD'])
	else:
		env.Append(CPPDEFINES=['RELEASE_BUILD'])

	## set stack size 
	if env['platform'] == 'windows':
		env.Append(CPPPATH=[Dir('./thirdparty/dlfcn-win32/')])
		env.Append(LINKFLAGS=['/STACK:%s'% (40 * 1024 * 1024) ])
	## TODO: not sure about other platforms
		

#################################################################################################

## Gets the standard flags CC, CCX, etc.
cbenv = DefaultEnvironment()

opts = Variables([], ARGUMENTS)
## Define our options
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['debug', 'release']))
opts.Add(EnumVariable('bits', 'output program bits', '64', ['32', '64']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", False))

opts.Add(BoolVariable('vsproj', "make a visual studio project", False))
opts.Add(PathVariable('target_path', 'The path to the output library.', 'bin/', PathVariable.PathAccept))
opts.Add(BoolVariable('verbose', "Use verbose build command", False))

opts.Add(BoolVariable('libs', "Only build the libs defined in the user data", False))

## Updates the environment with the option variables.
opts.Update(cbenv)

## find platform
if cbenv['platform'] == 'linux':
	cbenv['platform'] = 'x11'

if cbenv['platform'] == '':
	if sys.platform == 'win32':
		cbenv['platform'] = 'windows'
	elif sys.platform in ('linux', 'linux2'):
		cbenv['platform'] = 'x11'
	elif sys.platform == 'darwin':
		cbenv['platform'] = 'osx'
	else:
		print("platform(%s) not supported." % sys.platform)
		quit()

## Process some arguments
if cbenv['use_llvm']:
	cbenv['CC'] = 'clang'
	cbenv['CXX'] = 'clang++'

## For the reference:
## - CCFLAGS are compilation flags shared between C and C++
## - CFLAGS are for C-specific compilation flags
## - CXXFLAGS are for C++-specific compilation flags
## - CPPFLAGS are for pre-processor flags
## - CPPDEFINES are for pre-processor defines
## - LINKFLAGS are for linking flags

## Check our platform specifics
if cbenv['platform'] == "osx":
	cbenv.Append(CXXFLAGS=['-std=c++17'])
	if cbenv['target'] == 'debug':
		cbenv.Append(CCFLAGS=['-g', '-O2', '-arch', 'x86_64'])
		cbenv.Append(LINKFLAGS=['-arch', 'x86_64'])
	else:
		cbenv.Append(CCFLAGS=['-g', '-O3', '-arch', 'x86_64'])
		cbenv.Append(LINKFLAGS=['-arch', 'x86_64'])

elif cbenv['platform'] == 'x11':
	cbenv.Append(LIBS=['dl', 'pthread']) 
	cbenv.Append(CXXFLAGS=['-std=c++17'])
	if cbenv['target'] == 'debug':
		cbenv.Append(CCFLAGS=['-fPIC', '-g3', '-Og'])
	else:
		cbenv.Append(CCFLAGS=['-fPIC', '-g', '-O3'])

elif cbenv['platform'] == "windows":
	## This makes sure to keep the session environment variables on windows,
	## that way you can run scons in a vs 2017 prompt and it will find all the required tools
	cbenv.Append(ENV=os.environ)

	cbenv.Append(CXXFLAGS=['/std:c++17', '/bigobj'])
	cbenv.Append(CPPDEFINES=['WIN32', '_WIN32', '_WINDOWS', '_CRT_SECURE_NO_WARNINGS'])
	#cbenv.Append(CPPDEFINES=['_HAS_CXX17']) ## for autocompletion in vs.
	cbenv.Append(CCFLAGS=['-W3', '-GR'])
	cbenv.Append(LINKFLAGS='-SUBSYSTEM:CONSOLE')

	cbenv.Append(LIBS=[
		'psapi', 'dbghelp', ## for crash handler
		'User32',           ## console debugger
	])

	if cbenv['target'] == 'debug':
		cbenv.Append(CPPDEFINES=['_DEBUG'])
		cbenv.Append(CCFLAGS=['-EHsc', '-MDd', '-ZI'])
		cbenv.Append(LINKFLAGS=['-DEBUG'])
	else:
		cbenv.Append(CPPDEFINES=['NDEBUG'])
		cbenv.Append(CCFLAGS=['-O2', '-EHsc', '-MD'])

## --------------------------------------------------------------------------------

## reference : https://github.com/godotengine/godot/blob/master/methods.py
if not cbenv['verbose']:
	colors = {}
	# Colors are disabled in non-TTY environments such as pipes. This means
	# that if output is redirected to a file, it will not contain color codes
	isatty = sys.stdout.isatty()
	colors["cyan"]   = "\033[96m" if isatty else ""
	colors["purple"] = "\033[95m" if isatty else ""
	colors["blue"]   = "\033[94m" if isatty else ""
	colors["green"]  = "\033[92m" if isatty else ""
	colors["yellow"] = "\033[93m" if isatty else ""
	colors["red"]    = "\033[91m" if isatty else ""
	colors["end"]    = "\033[0m"  if isatty else ""
	compile_source_message = "{}Compiling {}==> {}$SOURCE{}".format(
		colors["blue"], colors["purple"], colors["yellow"], colors["end"]
	)
	java_compile_source_message = "{}Compiling {}==> {}$SOURCE{}".format(
		colors["blue"], colors["purple"], colors["yellow"], colors["end"]
	)
	compile_shared_source_message = "{}Compiling shared {}==> {}$SOURCE{}".format(
		colors["blue"], colors["purple"], colors["yellow"], colors["end"]
	)
	link_program_message = "{}Linking Program {}==> {}$TARGET{}".format(
		colors["red"], colors["purple"], colors["yellow"], colors["end"]
	)
	link_library_message = "{}Linking Static Library {}==> {}$TARGET{}".format(
		colors["red"], colors["purple"], colors["yellow"], colors["end"]
	)
	ranlib_library_message = "{}Ranlib Library {}==> {}$TARGET{}".format(
		colors["red"], colors["purple"], colors["yellow"], colors["end"]
	)
	link_shared_library_message = "{}Linking Shared Library {}==> {}$TARGET{}".format(
		colors["red"], colors["purple"], colors["yellow"], colors["end"]
	)
	java_library_message = "{}Creating Java Archive {}==> {}$TARGET{}".format(
		colors["red"], colors["purple"], colors["yellow"], colors["end"]
	)
	cbenv.Append(CXXCOMSTR=[compile_source_message])
	cbenv.Append(CCCOMSTR=[compile_source_message])
	cbenv.Append(SHCCCOMSTR=[compile_shared_source_message])
	cbenv.Append(SHCXXCOMSTR=[compile_shared_source_message])
	cbenv.Append(ARCOMSTR=[link_library_message])
	cbenv.Append(RANLIBCOMSTR=[ranlib_library_message])
	cbenv.Append(SHLINKCOMSTR=[link_shared_library_message])
	cbenv.Append(LINKCOMSTR=[link_program_message])
	cbenv.Append(JARCOMSTR=[java_library_message])
	cbenv.Append(JAVACCOMSTR=[java_compile_source_message])

## update user data
USER_DATA(cbenv)

Export('cbenv')
for script in cbenv.SCONSCRIPTS:
	SConscript(script)

## compile the libs.
if cbenv['libs']:
	for lib_name in cbenv.LIBS:
		lib = cbenv.Library(
			target = os.path.join(cbenv['target_path'], lib_name),
			source = cbenv.LIBS[lib_name])
		cbenv.Prepend(LIBS=[lib])
else:
	LIB_SOURCES = []
	for sources in cbenv.LIBS.values(): LIB_SOURCES += sources
	if len(LIB_SOURCES) > 0:
		lib = cbenv.Library(
			target = os.path.join(cbenv['target_path'], cbenv.PROJECT_NAME),
			source = LIB_SOURCES)
		cbenv.Prepend(LIBS=[lib])

if not cbenv['libs']:
	## the main application
	main_program = cbenv.Program(
		target = os.path.join(cbenv['target_path'], cbenv.PROJECT_NAME),
		source = cbenv.SOURCES_MAIN
	)

	## tests
	if len(cbenv.SOURCES_TEST) > 0:
		tests_program = cbenv.Program(
			target = os.path.join(cbenv['target_path'], "tests"),
			source = cbenv.SOURCES_TEST
		)

## --------------------------------------------------------------------------------

## visual studio targets
def get_vsproj_context():
	vsproj_targets = []
	variants = [] ## ["debug|Win32", "debug|x64", "release|Win32", "release|x64"]
	for target in 'debug', 'release':
		for bits in '32', '64':
			run_target = "tests" if cbenv.DEBUG_TESTS and len(cbenv.SOURCES_TEST) > 0 else cbenv.PROJECT_NAME
			vsproj_targets.append(os.path.join( cbenv['target_path'], run_target + '.exe'))
			variants.append(target+'|'+('Win32' if bits=='32' else 'x64'))
	return vsproj_targets, variants

def msvs_collect_header():

	def recursive_collect(dir):
		ret = []
		for file in os.listdir(dir):
			file = os.path.join(dir, file)
			if os.path.isfile(file):
				if (file.endswith('.h') or file.endswith('.hpp')):
					ret.append('$(ProjectDir)' + os.path.relpath(file))
			elif os.path.isdir(file):
				ret += recursive_collect(file)
		return ret

	ret = []
	cbenv.Append(CPPPATH=[]) ## A fail safe if CPPPATH doesn't exists on env.
	for dir in cbenv['CPPPATH']:
		ret += recursive_collect(str(dir))
	return ret

def msvc_collect_sources():
	ret = []
	all_sources = cbenv.SOURCES_MAIN + cbenv.SOURCES_TEST
	for sources in cbenv.LIBS.values(): all_sources += sources
	for src in all_sources:
		if (str(src).endswith('.c')  or str(src).endswith('.cpp') or 
			str(src).endswith('.cc') or str(src).endswith('.cxx')):
			ret.append(str(src))
		else: ## Glob
			for _src in src:
				assert(str(_src).endswith('.c')  or str(_src).endswith('.cpp') or 
					   str(_src).endswith('.cc') or str(_src).endswith('.cxx'))
				ret.append(str(_src))
	return ret

def msvc_build_commandline(commands):
	common_build_prefix = [
		'cmd /V /C set "bits=64"',
		'(if "$(PlatformTarget)"=="x86" (set "bits=32"))',
	]
	return " ^& ".join(common_build_prefix + [commands])


if cbenv['vsproj']:
	cbenv["MSVSBUILDCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" platform=windows target=$(Configuration) bits=!bits!"
	)
	cbenv["MSVSREBUILDCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" platform=windows target=$(Configuration) bits=!bits! vsproj=yes"
	)
	cbenv["MSVSCLEANCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" --clean platform=windows bits=!bits! target=$(Configuration)"
	)
	targets, variants = get_vsproj_context()
	cbenv.MSVSProject(target = cbenv.PROJECT_NAME + cbenv['MSVSPROJECTSUFFIX'],
		srcs = msvc_collect_sources(),
		incs = msvs_collect_header(),
		variant = variants,
		runfile = targets,
		buildtarget = targets,
	)


## Generates help for the -h scons option.
Help(opts.GenerateHelpText(cbenv))