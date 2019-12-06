#include <stdio.h>

#include "tkscanner.h"
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

	struct TokenList    tl; structTokenList_init(&tl);
	struct TokenScanner ts; structTokenScanner_init(&ts, text, argv[1]);

	bool eof = false;
	while (!eof){
		
		struct Token* tk = structTokenList_createToken(&tl);
		structTokenScanner_setToken(&ts, tk);
		eof = structTokenScanner_scaneToken(&ts);
		
	}

	for (int i=0; i<tl.count; i++){
		structToken_print( tl.list[i] );
	}


	printf("main runned successfully!\n");
	return 0;
}