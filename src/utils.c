#include "utils.h"

int utils_read_file(char** text_p, const char* file_path){

	FILE* fptr = fopen(file_path, "r");
	if (fptr == NULL){
		return 1;
	}

	fseek(fptr, 0L, SEEK_END);
	long size = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	char* text = (char*)malloc(size);
	char c = fgetc(fptr); long i = 0;
	while( c != EOF){
		text[i++] = c; c = fgetc(fptr);
	}

	*text_p = text;
	return 0;

}