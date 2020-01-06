#include "ast.h"



int main(int argc, char** argv){

	if (argc < 2  ){

		//printf("usage: <file_path>\n");
		//return 1;

		// clean the below peace of code and move to clinterp
		struct String* src = structString_new();
		struct Ast ast; structAst_init(&ast, src, "<stdin>");
		struct CarbonError* err;
		size_t stmnt_count = 0, token_count=0;
		size_t begin_pos = 0; // for eof
		bool is_eof = false;

		printf("Carbon v1.0.0 (https://github.com/ThakeeNathees/Carbon)\n");
		printf("Copyright (c) 2019 ThakeeNathees.\n");
		printf("License GPLv3 : GNU GPL version 3\n");

		while (true) {

			char buff[100];
			if (!is_eof) printf(">>> ");
			else printf("... ");

			fgets(buff, 100, stdin);
			if (buff[0] == '\n') continue;
			structString_strcat(src, buff);

			stmnt_count = ast.stmn_list->count;
			token_count = ast.tokens->count;
			begin_pos = ast.pos;

			err = structAst_scaneTokens(&ast); 
			ast.token_scanner->pos = strlen(ast.src->buffer); // if error at the middle set it to last
			if (err->type == ERROR_SUCCESS) {
				structCarbonError_free(err);

				err = structAst_makeTree(&ast, ast.stmn_list, STMNEND_EOF); 
				ast.pos = ast.tokens->count - 1; // if error at the middle set it to last
				
				if (ast.tokens->count > 0 && ast.tokens->list[ast.pos]->group == TKG_EOF) {
					structTokenList_deleteLast(ast.tokens);
				}
				//structTokenList_print(ast.tokens);

				if (err->type == ERROR_SUCCESS) {
					structCarbonError_free(err);
					is_eof = false;
					for (size_t i = 0; i < ast.stmn_list->count - stmnt_count; i++)
						structStatement_print(ast.stmn_list->list[ast.stmn_list->count - (i+(size_t)1)], 0); // execute it
				}
				else {
					if (err->type == ERROR_UNEXP_EOF) {
						is_eof = true;
						ast.pos = begin_pos;
					}
					else {
						is_eof = false;
						printf("%s\n", err->message.buffer);
					}

					structCarbonError_free(err);

					if (stmnt_count != ast.stmn_list->count)
						structAst_deleteLastStatement(&ast);

				}
			}
			else {
				structCarbonError_free(err);
				printf("%s\n", err->message.buffer);
				ast.pos += (ast.tokens->count - token_count); // skip those error tokens
			}
			//structStatementList_print(ast.stmn_list);
			
		}

	}

	char* text;
	int error = utils_read_file(&text, argv[1]);
	if (error){
		printf("Error: can't read the file : %s\n", argv[1]);
		return 1;
	}

	// debug print src
	//printf("%s\n", text);


	struct CarbonError* err;

	// read tokens
	struct String* source_code = structString_new();
	free(source_code->buffer); source_code->buffer = text;
	struct Ast ast; structAst_init(&ast, source_code, argv[1]);
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
	// structTokenList_print(ast.tokens);

	// debug print statements
	structStatementList_print( ast.stmn_list );

	printf("main runned successfully!\n");
	return 0;
}