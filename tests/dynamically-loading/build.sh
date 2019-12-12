gcc -c mylib.c
gcc -shared -o mylib.so mylib.o
## gcc -shared -o mylib.so mylib.c

gcc -o test.out test.c -ldl
## -ldl flat needed for linking shared object

./test.out ./mylib.so
sleep 1

## clean
rm ./mylib.o ./mylib.so ./test.out
