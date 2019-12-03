#include <stdio.h>

#include "lparse.h"
#include "utils.h"

int main(int argc, char** argv){

	if (argc < 2){
		printf("usage: <file_path>\n");
		return 1;
	}

	char* text;
	int error = utils_read_file(&text, argv[1]);
	if (error){
		printf("Error: can't read the file : %s\n", argv[1]);
		return 1;
	}
	printf("%s\n", text);

	printf("main runned successfully!\n");
	return 0;
}