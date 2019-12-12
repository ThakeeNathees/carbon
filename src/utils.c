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

bool utils_char_in_str(char c, const char* string){
	for (unsigned int i=0; i<strlen(string); i++){
		if (string[i] == c) return true;
	}
	return false;
}
int utils_char_count_in_str(char c, const char* string){
	int count = 0;
	for (unsigned int i=0; i<strlen(string); i++){
		if (string[i] == c) count++;
	}
	return count;
}

// error_len = number of ^^^^
int utils_pos_to_line(int pos, const char* src, char* buffer, char* location_str, size_t error_len ){

	int line_no = 1; int loc_ptr = 0;
	int line_begin_pos = 0;
	for ( int i=0; i<pos; i++){
		if (src[i] == '\t' && location_str != NULL) location_str[loc_ptr++] = '\t';
		else if (location_str != NULL) location_str[loc_ptr++] = ' ';


		if (src[i] == '\n'){ 
			if (location_str != NULL) { loc_ptr =0; location_str[loc_ptr] = '\0'; } 
			line_begin_pos = i+1;
			(line_no)++;
		}
	}
	if (location_str != NULL) { for (size_t i = 0; i < error_len; i++) location_str[loc_ptr++] = '^'; location_str[loc_ptr++] = '\0'; }
	
	int i=0;
	while(true){
		if (src[line_begin_pos+i]=='\n' || src[line_begin_pos+i]=='\0'){ buffer[i] = '\0'; break; }
		buffer[i] = src[line_begin_pos+i]; i++;
	}

	return line_no;

}

void utils_error_exit(const char* err_msg, size_t pos, const char* src, const char* file_name){
	char location_str[ERROR_LINE_SIZE];
	char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(pos, src, buff, location_str, 1);
	printf("%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff);
	printf("%s\n", location_str);
	exit(1);
}

struct CarbonError* utils_make_error(char* err_msg, enum ErrorType err_type, size_t pos, const char* src, const char* file_name, bool free_msg, size_t error_len){
	char location_str[ERROR_LINE_SIZE];
	char buff[ERROR_LINE_SIZE];  int line_no = utils_pos_to_line(pos, src, buff, location_str, error_len);
	struct CarbonError* err = structCarbonError_new(); err->type = err_type;
	int msg_min_size = snprintf(NULL, 0,                                 "%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff); structString_minSize(&(err->message), msg_min_size+100); // TOOD: 100 is for ^ printing
	int msg_size = snprintf(err->message.buffer, err->message.buff_size, "%s @%s:%i\n%s\n",err_msg, file_name, line_no, buff);
	err->message.buff_pos += msg_size;

	int i=0; 
	while( true ){
		char c = location_str[i++];
		err->message.buffer[(err->message.buff_pos)++]= c;
		if (c == '\0') break;
	}
	
	if (free_msg) free(err_msg);
	return err;
}

void utils_warning_print(const char* warn_msg, size_t pos, const char* src, const char* file_name, size_t err_len) {
	char location_str[ERROR_LINE_SIZE]; char buff[ERROR_LINE_SIZE];
	int line_no = utils_pos_to_line(pos, src, buff, location_str, err_len);
	printf("%s ", warn_msg);
	printf("@%s:%i\n%s\n%s\n", file_name, line_no, buff, location_str);
}