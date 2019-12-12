echo off

gcc -c mylib.c
gcc -shared -o mylib.dll mylib.o
rem or use them both gcc -shared -o mylib.dll mylib.c

gcc test.c -o test.exe

test.exe mylib.dll

echo press any key to clean
pause

rem clean
del mylib.o mylib.dll test.exe

