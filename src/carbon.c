#include "ast.h"



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

	// debug print
	printf("%s\n", text);

	struct Ast ast; structAst_init(&ast, text, argv[1]);
	structAst_scane(&ast);

	// debug print
	//structTokenList_print(ast.tokens); // not classified

	structAst_makeTree(&ast);
	
	// debug print
	//structTokenList_print(ast.tokens); // classified
	structStatementList_print( ast.stmn_list );

	printf("main runned successfully!\n");
	return 0;
}