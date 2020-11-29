
import os, sys
import shutil

USAGE_STRING = '''\
Usage:
$ python tool.py command [...]

clean - clean the generated files
    --all, -a       if all, clean all generated (includes project) files

run - runs the output binary debug x86_64 by default
    --gdb, -g        debug the output binary with gdb
'''
#    --debug, -d      run the debug version
#    --release, -r    run the release version
#    --x86_64         run the 64 bit binary
#    --x86            run the 32 bit binary

NAME = 'tool.py'

REMOVE_DIRS = [

    'x64/',
    'debug/'
    'release/',
    'debug/',
    'bin/',
]

IGNORE_FILES = [
    '.pdb',
    '.idb',
    '.ilk',
    
    '.obj',
]

VS_DIRS = [
    '.vs',
    '.vscode',
]

VS_FILES = [
    '.sln',
    '.vcxproj',
    '.vcxproj.filters',
    '.vcxproj.user',
]

REMOVE_FILES = IGNORE_FILES

def error_command(cmd):
    print(NAME + ': ERROR: unknown command "'+ cmd + '"\n' + USAGE_STRING)
    exit(-1)

def error_exit(msg):
    print(NAME + ': ERROR: ' + msg + '"\n' + USAGE_STRING)
    exit(-1)

def get_platform():
    if sys.platform == 'win32': return 'windows'
    elif sys.platform in ('linux', 'linux2'): return 'x11'
    elif sys.platform == 'darwin': return 'osx'
    else: error_exit("platform(%s) not supported." % sys.platform)

def clean(cleanll = False, REMOVE_DIRS=REMOVE_DIRS, REMOVE_FILES=REMOVE_FILES, VS_FILES=VS_FILES):

    if cleanll:
        REMOVE_DIRS += VS_DIRS
        REMOVE_FILES += VS_FILES + ['.sconsign.dblite']

    os.system('scons -c')
    print('\n%s: cleaning all files ...' % NAME)
    for _dir in REMOVE_DIRS:
        try:
            shutil.rmtree(_dir)
            print('%s: Removed - %s' % (NAME, _dir))
        except:
            pass
    for path, dirs, files in os.walk('.'):
        for file in files:
            for suffix in REMOVE_FILES:
                if file.endswith(suffix):
                    os.remove(os.path.join(path, file))
                    print('%s: Removed - %s' % (NAME, os.path.join(path, file)))
    print(NAME + ': done cleaning targets.')

def run(is_gdb):
    out_file = 'carbon'
    run_target = os.path.join(os.getcwd(), 'bin', out_file)
    if is_gdb:
        os.system('gdb "%s"' % run_target)
    else:
        os.system('"%s"' % run_target)
    pass

if __name__ == '__main__':
    argcount = len(sys.argv)

    if argcount < 2:
        print(USAGE_STRING)
        exit()

    if sys.argv[1] == 'clean':
        cleanall = False
        for i in range(2, argcount):
            if sys.argv[i] in ('--all', '-a'):
                cleanall = True
            else:
                error_command(sys.argv[i])
        clean(cleanall)

    elif sys.argv[1] == 'run':
        target = None
        bits = None
        gdb = None
        for i in range(2, argcount):

            #if sys.argv[i] in ('--debug', '-d'):
            #    if not target: target = 'debug'
            #    else: error_exit('multiple definition argument target')
            #elif sys.argv[i] in ('--release', '-r'):
            #    if not target: target = 'release'
            #    else: error_exit('multiple definition argument target')
            #elif sys.argv[i] == '--x86_64':
            #    if not bits: bits = '64'
            #    else: error_exit('multiple definition bits')
            #elif sys.argv[i] == '--x86':
            #    if not bits: bits = '32'
            #    else: error_exit('multiple definition bits')
            
            if sys.argv[i] in ('--gdb', '-g'):
                if not gdb: gdb = True
                else: error_exit('multiple definition argument gdb')
            else:
                error_command(sys.argv[i])

        if not gdb: gdb = False

        run(gdb)

    else:
        error_command(sys.argv[1])
