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

	int pos = 0;
	struct Token tk; structToken_init(&tk);
	struct TokenScanner ts; structTokenScanner_init(&ts, &tk, text, argv[1]);

	int i=0;
	struct Token* tokens[100];

	bool eof = false;
	while (!eof){
		// new token
		struct Token* new_tk = (struct Token*)malloc(sizeof(struct Token));
		structToken_init(new_tk);
		tokens[i++] = new_tk;
		structTokenScanner_setToken(&ts, new_tk);

		eof = structTokenScanner_scaneToken(&ts, &pos);
	}

	for (int j=0; j<i-1; j++){
		structToken_print( tokens[j] );
	}


	printf("main runned successfully!\n");
	return 0;
}