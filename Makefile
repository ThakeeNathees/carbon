bin_out = ./_bin/
obj_out = ./_obj/

all: carbon

carbon: main.o lparse.o utils.o
	gcc -o $(bin_out)carbon $(obj_out)main.o $(obj_out)lparse.o $(obj_out)utils.o

main.o : ./src/carbon.c
	gcc -c ./src/carbon.c -o $(obj_out)main.o 

lparse.o: ./src/lparse.c ./src/lparse.h
	gcc -c ./src/lparse.c -o $(obj_out)lparse.o

utils.o: ./src/utils.c ./src/utils.h
	gcc -c ./src/utils.c -o $(obj_out)utils.o


test: ./tests/file_read.c
	gcc ./tests/file_read.c -o $(bin_out)file_read.so

clean:
	rm -rf $(obj_out)*.o
	rm -rf $(bin_out)file_read.so $(bin_out)carbon.so