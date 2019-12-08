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

int utils_pos_to_line(int pos, char* src, char* buffer, int* err_pos ){
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

	if (err_pos != NULL) *err_pos = pos - line_begin_pos;

	return line_no;

}

void utils_error_exit(char* err_msg, int pos, char* src, char* file_name){
	int line_pos = 0;
	char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(pos, src, buff, &line_pos);
	printf("%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff);
	for(int i=0; i<line_pos;i++) printf(" ");printf("^\n");exit(1); 
}

struct CarbonError* utils_make_error(char* err_msg, enum ErrorType err_type, int pos, char* src, char* file_name, bool free_msg){
	int line_pos = 0; struct CarbonError* err = structCarbonError_new(); err->type = err_type;
	char buff[ERROR_LINE_SIZE];  int line_no = utils_pos_to_line(pos, src, buff, &line_pos);
	int msg_min_size = snprintf(NULL, 0,                                 "%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff); structString_minSize(&(err->message), msg_min_size+100); // TOOD: 100 is for ^ printing
	int msg_size = snprintf(err->message.buffer, err->message.buff_size, "%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff);
	err->message.buff_pos += msg_size;
	for(int i=0; i<line_pos;i++) err->message.buffer[err->message.buff_pos++]= ' ';err->message.buffer[err->message.buff_pos++]='^';
	if (free_msg) free(err_msg);
	return err;
}