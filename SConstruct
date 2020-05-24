#!python
import os, subprocess, sys

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['debug', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", False))
opts.Add(BoolVariable('vsproj', "make a visual studio project", False))
opts.Add(PathVariable('target_path', 'The path to the output library.', 'bin/', PathVariable.PathAccept))
opts.Add(PathVariable('target_name', 'The library/application name.', 'carbon', PathVariable.PathAccept))
opts.Add(EnumVariable('bits', 'output program bits', '64', ['32', '64']))

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

# debug macro for all platforms
if env['target'] == 'debug':
    env.Append(CPPDEFINES=['_DEBUG'])
else:
    env.Append(CPPDEFINES=['NDEBUG'])

# find platform
if env['platform'] == 'linux':
    env['platform'] = 'x11'
if env['platform'] == '':
    if sys.platform == 'win32':
        env['platform'] = 'windows'
    elif sys.platform in ('linux', 'linux2'):
        env['platform'] = 'x11'
    elif sys.platform == 'darwin':
        env['platform'] = 'osx'
    else:
        print("platform(%s) not supported." % sys.platform)
        quit()

# add include dir from outer libs
def add_include_dir(_path):
    dir  = Dir('.').abspath
    path = os.path.join(dir, _path)
    env.Append(CPPPATH=[path])
env.add_include_dir = add_include_dir

def add_lib(name, path=None):
    if path:
        env.Append(LIBPATH=[path])
    env.Append(LIBS=[name])
env.add_lib = add_lib

# output name suffix, dir
def get_suffix(platform, target, bits):
    return '.%s.%s.%s' % (platform, target, bits)

env["out_suffix"]  = get_suffix(
    env['platform'], env['target'], env['bits'])

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# Check our platform specifics
if env['platform'] == "osx":
    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-g', '-O2', '-arch', 'x86_64'])
        env.Append(LINKFLAGS=['-arch', 'x86_64'])
    else:
        env.Append(CCFLAGS=['-g', '-O3', '-arch', 'x86_64'])
        env.Append(LINKFLAGS=['-arch', 'x86_64'])

elif env['platform'] == 'x11':
    env.Append(LIBS=['GL', 'GLU', 'dl', 'X11', 'pthread']) 
    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-fPIC', '-g3', '-Og'])
        env.Append(CXXFLAGS=['-std=c++17'])
    else:
        env.Append(CCFLAGS=['-fPIC', '-g', '-O3'])
        env.Append(CXXFLAGS=['-std=c++17'])

elif env['platform'] == "windows":
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV=os.environ)

    env.Append(CPPDEFINES=['WIN32', '_WIN32', '_WINDOWS', '_CRT_SECURE_NO_WARNINGS'])
    env.Append(CCFLAGS=['-W3', '-GR'])
    env.Append(LINKFLAGS='-SUBSYSTEM:CONSOLE')

    env.Append(LIBS=[
        #'opengl32.lib',
        #'user32.lib',
        #'gdi32.lib',
        #'shell32.lib',
    ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-EHsc', '-MDd', '-ZI'])
        env.Append(LINKFLAGS=['-DEBUG'])
    else:
        env.Append(CCFLAGS=['-O2', '-EHsc', '-MD'])

# includes and libs
env.includes = []
env.sources  = [       # cpp files
    'main/main_%s.cpp' % env['platform'],
    'main/main.cpp'
]
env.Append(CPPPATH=[]) # include files
env.Append(LIBPATH=[]) # static lib dir

Export('env')
SConscript('core/SConstruct')
SConscript('os/SConstruct')

for header in env.includes:
    env.Prepend(CPPPATH=[header])


target = env.Program(target=os.path.join(
    env['target_path'], env['target_name'] + env['out_suffix']),
    source=env.sources,
)

# visual studio targets
def get_vsproj_context():
    vsproj_targets = []
    variants = [] # ["debug|Win32", "debug|x64", "release|Win32", "release|x64"]
    for target in 'debug', 'release':
        for bits in '32', '64':
            vsproj_targets.append(
                os.path.join(
                    env['target_path'],
                    env['target_name'] + env["out_suffix"]
                    + '.exe'
                )
            )
            variants.append(
                target+'|'+('Win32' if bits=='32' else 'x64')
            )
    return vsproj_targets, variants

def msvs_collect_header():
    ret = []
    for _dir in env['CPPPATH']:
        _dir = str(_dir)
        for file in os.listdir(_dir):
            if os.path.isfile(os.path.join(_dir, file)) and (file.endswith('.h') or file.endswith('.hpp')):
                abspath = os.path.join(_dir, file)
                ret.append('$(ProjectDir)' + os.path.relpath(str(abspath)))
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
    env.MSVSProject(target = env['target_name'] + env['MSVSPROJECTSUFFIX'],
        srcs = env.sources,
        incs = msvs_collect_header(),
        variant = variants,
        runfile = targets,
        buildtarget = targets,
    )


# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
