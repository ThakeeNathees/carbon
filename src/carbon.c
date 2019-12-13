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

	// debug print src
	printf("%s\n", text);

	struct CarbonError* err;

	// read tokens
	struct Ast ast; structAst_init(&ast, text, argv[1]);
	err = structAst_scaneTokens(&ast); if (err->type != ERROR_SUCCESS){ printf("%s\n", err->message.buffer ); exit(-1); }
	structCarbonError_free(err);

	// debug print tokens
	//structTokenList_print(ast.tokens);

	// create tree
	err = structAst_makeTree(&ast, ast.stmn_list, STMNEND_EOF); if (err->type != ERROR_SUCCESS){ printf("%s\n", err->message.buffer ); exit(-1);}
	structCarbonError_free(err);
	struct Token* token = ast.tokens->list[ast.pos];
	if (token->group != TKG_EOF) {
		if (token->group == TKG_BRACKET && strcmp(token->name, RCUR_BRACKET) == 0) {
			struct CarbonError* err = utils_make_error("SyntaxError: unexpected bracket", ERROR_SYNTAX, token->pos, ast.src, ast.file_name, false, 1);
			printf("%s\n", err->message.buffer); exit(-1);
		}
		else {
			printf("InternalError: expected token TK_EOF\nfound: "); structToken_print(token); exit(-1);
		}
	}

	// debug print tokens
	// structTokenList_print(ast.tokens);

	// debug print statements
	structStatementList_print( ast.stmn_list );

	printf("main runned successfully!\n");
	return 0;
}