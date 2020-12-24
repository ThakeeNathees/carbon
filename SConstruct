#!python
import os, subprocess, sys

## TODO: debug/release compiler flags for mingw
##       refactor *.gen.h generation

###### USER DATA #############################################################################
def USER_DATA(env):
	env.PROJECT_NAME = 'carbon'
	
	## generate files TODO: use builders
	py = 'python' if env['platform'] == 'windows' else 'python3'
	sys.path.append('include/core')
	from native_bind_gen import generage_method_binds
	generage_method_binds('include/core/native_bind.gen.h')
	
	env.Append(CPPPATH=[Dir("./include/")])

	env.SOURCES_MAIN = [
		Glob('src/main/*.cpp'),
	]

	env.SOURCES_TEST     = [
		Glob('tests/*.cpp'),
		Glob('tests/var/*.cpp'),
		Glob('tests/native_classes/*.cpp'),
		Glob('tests/parser/*.cpp'),
		Glob('tests/analyzer/*.cpp'),
		Glob('tests/codegen/*.cpp'),
		Glob('tests/vm/*.cpp'),
	]
	env.SOURCES_TEST     = []

	## skip test/main sources if not found
	## this will only generate the carbon.lib
	if not os.path.exists('./src/main/'):
		env.SOURCES_MAIN = []
	if not os.path.exists('./tests/'):
		env.SOURCES_TEST = []

	## env.SCONSCRIPTS = []

	env.LIBS = {
	"cb_core" : [
		Glob('src/core/*.cpp'),
		Glob('src/var/*.cpp'),
		Glob('src/native/*.cpp'),
	],
	"cb_compiler" : [
		Glob('src/compiler/*.cpp'),
	]}
	env.DEBUG_TESTS = False

	if env['target'] == 'debug':
		env.Append(CPPDEFINES=['DEBUG_BUILD'])

	if env['platform'] == 'windows':
		if env['CC'] == 'cl':
			env.Append(LIBS=[
				'psapi', 'dbghelp', ## for crash handler
			])
	elif env['platform'] == 'linux':
		env.Append(LIBS=['dl', 'pthread']) 
		pass

	## set stack size 
	if env['CC'] == 'cl':
		env.Append(LINKFLAGS=['/STACK:%s'% (40 * 1024 * 1024) ])
	## TODO: not sure about other compilers
		

#################################################################################################

opts = Variables([], ARGUMENTS)
## Define our options
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'linux', 'osx']))
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['debug', 'release']))
opts.Add(EnumVariable('bits', 'output program bits', '64', ['32', '64']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", False))
opts.Add(BoolVariable('use_mingw', "Use the MinGW compiler", False))

opts.Add(BoolVariable('vsproj', "make a visual studio project", False))
opts.Add(PathVariable('target_path', 'The path to the output library.', 'bin/', PathVariable.PathAccept))
opts.Add(BoolVariable('verbose', "Use verbose build command", False))

opts.Add(BoolVariable('libs', "Only build the libs defined in the user data", False))

if 'use_llvm' in opts.args.keys() and opts.args['use_llvm']:
	env['CC'] = 'clang'
	env['CXX'] = 'clang++'

elif 'use_mingw' in opts.args.keys() and opts.args['use_mingw']:
	env = Environment(tools = ['mingw'], ENV = {'PATH' : os.environ['PATH']})
	env['tools'] = ['mingw']

else:
	## Gets the standard flags CC, CCX, etc.
	env = DefaultEnvironment()
	## scons can't find "cl.exe"
	os.environ['PATH'] = env['ENV']['PATH']

## Updates the environment with the option variables.
opts.Update(env)

## find platform
if env['platform'] == '':
	if sys.platform == 'win32':
		env['platform'] = 'windows'
	elif sys.platform in ('x11', 'linux', 'linux2'):
		env['platform'] = 'linux'
	elif sys.platform == 'darwin':
		env['platform'] = 'osx'
	else:
		print("platform(%s) not supported." % sys.platform)
		quit()

## For the reference:
## - CCFLAGS are compilation flags shared between C and C++
## - CFLAGS are for C-specific compilation flags
## - CXXFLAGS are for C++-specific compilation flags
## - CPPFLAGS are for pre-processor flags
## - CPPDEFINES are for pre-processor defines
## - LINKFLAGS are for linking flags

## Check our platform specifics
if env['platform'] == "osx":
	env.Append(CXXFLAGS=['-std=c++11'])
	if env['target'] == 'debug':
		env.Append(CCFLAGS=['-g', '-O2', '-arch', 'x86_64'])
		env.Append(LINKFLAGS=['-arch', 'x86_64'])
	else:
		env.Append(CCFLAGS=['-g', '-O3', '-arch', 'x86_64'])
		env.Append(LINKFLAGS=['-arch', 'x86_64'])

elif env['platform'] == 'linux':
	env.Append(CXXFLAGS=['-std=c++11'])
	if env['target'] == 'debug':
		env.Append(CCFLAGS=['-fPIC', '-g3', '-Og'])
	else:
		env.Append(CCFLAGS=['-fPIC', '-g', '-O3'])

elif env['platform'] == "windows":
	## This makes sure to keep the session environment variables on windows,
	## that way you can run scons in a vs 2017 prompt and it will find all the required tools
	env.Append(ENV=os.environ)

	## MSVC
	if env['CC'] == 'cl':
		env.Append(CXXFLAGS=['/bigobj'])
		env.Append(CPPDEFINES=['_CRT_SECURE_NO_WARNINGS'])
		env.Append(CCFLAGS=['-W3', '-GR'])
		env.Append(LINKFLAGS='-SUBSYSTEM:CONSOLE')
		if env['target'] == 'debug':
			env.Append(CPPDEFINES=['_DEBUG'])
			env.Append(CCFLAGS=['-EHsc', '-MDd', '-ZI'])
			env.Append(LINKFLAGS=['-DEBUG'])
		else:
			env.Append(CPPDEFINES=['NDEBUG'])
			env.Append(CCFLAGS=['-O2', '-EHsc', '-MD'])

	## MINGW
	if env['CC'] == 'gcc':
		pass

## --------------------------------------------------------------------------------

## reference : https://github.com/godotengine/godot/blob/master/methods.py
if not env['verbose']:
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
	env.Append(CXXCOMSTR=[compile_source_message])
	env.Append(CCCOMSTR=[compile_source_message])
	env.Append(SHCCCOMSTR=[compile_shared_source_message])
	env.Append(SHCXXCOMSTR=[compile_shared_source_message])
	env.Append(ARCOMSTR=[link_library_message])
	env.Append(RANLIBCOMSTR=[ranlib_library_message])
	env.Append(SHLINKCOMSTR=[link_shared_library_message])
	env.Append(LINKCOMSTR=[link_program_message])
	env.Append(JARCOMSTR=[java_library_message])
	env.Append(JAVACCOMSTR=[java_compile_source_message])

## update user data
USER_DATA(env)

if env['verbose']:
	def dbg_print(expr)	:
		value = ""
		try:
			value = eval(expr)
		except Exception as err:
			pass
		if type(value) == type([]):
			value = '[' + ", ".join(map(lambda i : "'%s'"%str(i), value)) + ']'
		print(expr, '=', str(value))

	dbg_print("env['platform']")
	dbg_print("env['CC']")
	dbg_print("env['CXX']")
	dbg_print("env['CCFLAGS']")
	dbg_print("env['CXXFLAGS']")
	dbg_print("env['LINKFLAGS']")
	dbg_print("env['CPPPATH']")
	dbg_print("env['LIBS']")
	dbg_print("env['LIBPATH']")

##Export('env')
##for script in env.SCONSCRIPTS:
##	SConscript(script)

## compile the libs.
if env['libs']:
	for lib_name in env.LIBS:
		lib = env.Library(
			target = os.path.join(env['target_path'], lib_name),
			source = env.LIBS[lib_name])
		env.Prepend(LIBS=[lib])
else:
	LIB_SOURCES = []
	for sources in env.LIBS.values(): LIB_SOURCES += sources
	if len(LIB_SOURCES) > 0:
		lib = env.Library(
			target = os.path.join(env['target_path'], env.PROJECT_NAME),
			source = LIB_SOURCES)
		env.Prepend(LIBS=[lib])

if not env['libs']:
	## the main application
	if len(env.SOURCES_MAIN) > 0:
		main_program = env.Program(
			target = os.path.join(env['target_path'], env.PROJECT_NAME),
			source = env.SOURCES_MAIN
		)

	## tests
	if len(env.SOURCES_TEST) > 0:
		tests_program = env.Program(
			target = os.path.join(env['target_path'], "tests"),
			source = env.SOURCES_TEST
		)

## --------------------------------------------------------------------------------

## visual studio targets
def get_vsproj_context():
	vsproj_targets = []
	variants = [] ## ["debug|Win32", "debug|x64", "release|Win32", "release|x64"]
	for target in 'debug', 'release':
		for bits in '32', '64':
			run_target = "tests" if env.DEBUG_TESTS and len(env.SOURCES_TEST) > 0 else env.PROJECT_NAME
			vsproj_targets.append(os.path.join( env['target_path'], run_target + '.exe'))
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
	env.Append(CPPPATH=[]) ## A fail safe if CPPPATH doesn't exists on env.
	for dir in env['CPPPATH']:
		ret += recursive_collect(str(dir))
	return ret

def msvc_collect_sources():
	ret = []
	all_sources = env.SOURCES_MAIN + env.SOURCES_TEST
	for sources in env.LIBS.values(): all_sources += sources
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


if env['vsproj']:
	env["MSVSBUILDCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" platform=windows target=$(Configuration) bits=!bits!"
	)
	env["MSVSREBUILDCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" platform=windows target=$(Configuration) bits=!bits! vsproj=yes"
	)
	env["MSVSCLEANCOM"] = msvc_build_commandline(
		"scons --directory=\"$(ProjectDir.TrimEnd('\\'))\" --clean platform=windows bits=!bits! target=$(Configuration)"
	)
	targets, variants = get_vsproj_context()
	env.MSVSProject(target = env.PROJECT_NAME + env['MSVSPROJECTSUFFIX'],
		srcs = msvc_collect_sources(),
		incs = msvs_collect_header(),
		variant = variants,
		runfile = targets,
		buildtarget = targets,
	)


## Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))