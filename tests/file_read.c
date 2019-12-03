#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char** argv){

	if (argc < 2){
		printf("usage <filename>\n");
		return 1;
	}
	FILE* fptr = fopen(argv[1], "r");
	if (fptr == NULL){
		printf("Error: can't read the file\n");
		return 1;
	}

	fseek(fptr, 0L, SEEK_END);
	long size = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	char* text = (char*)malloc(size);

	char ch = fgetc(fptr);
	long i = 0;
	while( ch != EOF){
		text[i++] = ch;
		ch = fgetc(fptr);
	}
	printf("%s", text);
	free(text);
	return 0;
}
