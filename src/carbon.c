#include <stdio.h>

#include "tkscanner.h"
#include "utils.h"

int main(int argc, char** argv){

	/*
	char c = 'G';
	bool ws = structTokenScanner_isCharNumber(c);
	if (ws) printf("%c is num\n", c);
	else printf("%c is not num\n", c);
	//*/

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

	int pos = 0;
	struct TokenScanner ts;
	structTokenScanner_init(&ts, text);

	bool eof = structTokenScanner_scaneToken(&ts, &pos);
	while (!eof){
		structToken_print(&(ts.current_token));
		eof = structTokenScanner_scaneToken(&ts, &pos);
	}


	printf("main runned successfully!\n");
	return 0;
}