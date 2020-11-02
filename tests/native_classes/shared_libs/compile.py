import os

CC = 'mycl' ## my configured cl.exe
SOURCES = [
    ['native.cpp'],
    ['dynamic.cpp'],
]

INCLUDE_DIRS = [
    '/I..\\..\\..\\core/var.h',
]

def clean_int():
    for file in ('native', 'dynamic'):
        os.system(f'del {file}.exp & del {file}.ilk & del {file}.lib & del {file}.obj & del {file}.pdb')

def compile(sources):
    opts = [
        '/bigobj',
        '/std:c++17',
        '/W3', '/GR',
        '/EHsc',
        '/MDd',
        '/ZI',
    ]
    
    os.system(f"{CC} {' '.join(INCLUDE_DIRS)} /LD {' '.join(opts)} {' '.join(sources)}")
    clean_int()

for sources in SOURCES:
    compile(sources)
