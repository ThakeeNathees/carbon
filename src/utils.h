
#include "carbon_conf.h"

#define ERROR_LINE_SIZE 100

/* 	return 0 if success, text_p has result 
	don't forget to free(*text_p)
*/
int utils_read_file(char** text_p, const char* file_path); 

// string util
bool utils_char_in_str(char c, char* string);
int  utils_char_count_in_str(char c, char* string);
//int  utils_pos_to_line(int pos, char* src, char* buffer, char* location_str);


// dipricated
void utils_error_exit(char* err_msg, size_t pos, char* src, char* file_name);

// free_msg -> free the error message after writing to new error obj
struct CarbonError* utils_make_error(char* err_msg, enum ErrorType err_type, size_t pos, char* src, char* file_name, bool free_msg, size_t error_len);