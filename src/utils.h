

#include "carbon_conf.h"
/* 	return 0 if success, text_p has result 
	don't forget to free(*text_p)
*/
int utils_read_file(char** text_p, const char* file_path); 

// string util
bool utils_char_in_str(char c, char* string);
int  utils_char_count_in_str(char c, char* string);
int  utils_pos_to_line(int pos, char* src, char* buffer );