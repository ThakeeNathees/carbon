bin_out = ./_bin/Linux/
obj_out = ./_obj/Linux/

all: carbon

carbon: main.o tkscanner.o utils.o ast.o carbon_conf.o
	gcc -o $(bin_out)carbon $(obj_out)main.o $(obj_out)tkscanner.o $(obj_out)utils.o $(obj_out)ast.o $(obj_out)carbon_conf.o

main.o : ./src/carbon.c
	gcc -c ./src/carbon.c -o $(obj_out)main.o 

carbon_conf.o : ./src/carbon_conf.c ./src/carbon_conf.h
	gcc -c ./src/carbon_conf.c -o $(obj_out)carbon_conf.o

tkscanner.o: ./src/tkscanner.c ./src/tkscanner.h
	gcc -c ./src/tkscanner.c -o $(obj_out)tkscanner.o

utils.o: ./src/utils.c ./src/utils.h
	gcc -c ./src/utils.c -o $(obj_out)utils.o

ast.o: ./src/ast.c ./src/ast.h
	gcc -c ./src/ast.c -o $(obj_out)ast.o

test: ./tests/file_read.c ./tests/infix.c
	gcc ./tests/file_read.c -o $(bin_out)file_read.so
	gcc ./tests/infix.c -o $(bin_out)infix.so

clean:
	rm -rf $(obj_out)*.o
	rm -rf $(bin_out)file_read.so $(bin_out)carbon.so
