
import os, sys
import shutil

'''
usage:
> python cleanall.py         <-- clear all generated files
> python cleanall.py true    <-- include project files
'''

REMOVE_DIRS = [

    'x64/',
    'debug/'
    'release',
    
    'bin/',
    'core/bin',
    'thirdparty/bin/'
]

IGNORE_FILES = [
    '.pdb',
    '.idb',
    '.ilk',
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
if len(sys.argv) == 2 and sys.argv[1] == 'true':
    REMOVE_DIRS += VS_DIRS
    REMOVE_FILES += VS_FILES + ['.sconsign.dblite']

def main():
    os.system('scons -c')
    print('\ncleanall.py: cleaning all files ...')
    for _dir in REMOVE_DIRS:
        try:
            shutil.rmtree(_dir)
            print('Removed: %s' % _dir)
        except:
            pass
    for path, dirs, files in os.walk('.'):
        for file in files:
            for suffix in REMOVE_FILES:
                if file.endswith(suffix):
                    os.remove(os.path.join(path, file))
                    print('Removed: %s' % os.path.join(path, file))
    print('cleanall.py: done cleaning targets.')

if __name__ == '__main__':
    main()
