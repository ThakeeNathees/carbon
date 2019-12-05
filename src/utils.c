#include "utils.h"

int utils_read_file(char** text_p, const char* file_path){

	FILE* fptr = fopen(file_path, "r");
	if (fptr == NULL){
		return 1;
	}

	fseek(fptr, 0L, SEEK_END);
	long size = ftell(fptr);
	fseek(fptr, 0L, SEEK_SET);

	char* text = (char*)malloc(size+1);
	char c = fgetc(fptr); long i = 0;
	while( c != EOF){
		text[i++] = c; c = fgetc(fptr);
	}
	text[i] = '\0';

	*text_p = text;
	return 0;

}

bool utils_char_in_str(char c, char* string){
	for (int i=0; i<strlen(string); i++){
		if (string[i] == c) return true;
	}
	return false;
}
int utils_char_count_in_str(char c, char* string){
	int count = 0;
	for (int i=0; i<strlen(string); i++){
		if (string[i] == c) count++;
	}
	return count;
}

int utils_pos_to_line(int pos, char* src, char* buffer ){
	int line_no = 1;
	int line_begin_pos = 0;
	for ( int i=0; i<pos; i++){
		if (src[i] == '\n'){ 
			line_begin_pos = i+1;
			(line_no)++;
		}
	}

	int i=0;
	while(true){
		if (src[line_begin_pos+i]=='\n' || src[line_begin_pos+i]=='\0'){ buffer[i] = '\0'; break; }
		buffer[i] = src[line_begin_pos+i]; i++;
	}

	return line_no;

}