#include "ast.h"



int main(int argc, char** argv){

	if (argc < 2){

		printf("usage: <file_path>\n");
		return 1;


		struct String* src = structString_new();
		struct Ast ast; structAst_init(&ast, src->buffer, "module");
		struct CarbonError* err;

		while (true) {

			char* buff[100];
			printf(">>> ");
			fgets(buff, 100, stdin);
			structString_strcat(src, buff);

			err = structAst_scaneTokens(&ast); 
			if (err->type == ERROR_SUCCESS) {
				structCarbonError_free(err);

				err = structAst_makeTree(&ast, ast.stmn_list, STMNEND_EOF); 
				ast.pos = ast.tokens->count - 1;
				if (ast.tokens->count > 0 && ast.tokens->list[ast.pos]->group == TKG_EOF) {
					ast.tokens->list[ast.pos]->type		= TK_PASS;
					ast.tokens->list[ast.pos]->group	= TKG_PASS;
				}
				structTokenList_print(ast.tokens);

				if (err->type == ERROR_SUCCESS) {
					printf("success...\n");
				}
				else {
					if (err->type == ERROR_UNEXP_EOF) {
						printf("... \n");
					}
					else {
						printf("%s\n", err->message.buffer);
					}

					structAst_deleteLastStatement(&ast);

				}
			}
			else {
				printf("%s\n", err->message.buffer);
			}

			structStatementList_print(ast.stmn_list);
			
		}

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
		printf("InternalError: expected token TK_EOF\nfound: "); structToken_print(token); exit(-1);
	}

	// debug print tokens
	structTokenList_print(ast.tokens);

	// debug print statements
	structStatementList_print( ast.stmn_list );

	printf("main runned successfully!\n");
	return 0;
}