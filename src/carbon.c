#include "ast.h"

/* TODO:
****** handle pass ******
while (cond) PASS  { printf();   }
^^^^^^^^^^^^^  ^^  ^^^^^^^^^^^   ^
while     pass!={  single_expr;  ignored
******************
impl eof : idf, kwords, find at all place where syntax errors!
rebuild inpl line by line interpriter using the above code
impl try catch, throw, class.
------------
eval expr
globals, locals table
execute 
*/


int main(int argc, char** argv){

	if (argc < 2 || true){

		//printf("usage: <file_path>\n");
		//return 1;

		// DONT COPY THIS rebuild it using this
		struct String* src = structString_new();
		struct Ast ast; structAst_init(&ast, src, "module");
		struct CarbonError* err;
		int stmnt_count = 0;
		int begin_pos = 0; // for eof
		bool is_eof = false;

		while (true) {

			char buff[100];
			if (!is_eof) printf(">>> ");
			else printf("... ");

			fgets(buff, 100, stdin);
			if (buff[0] == '\n') continue;
			structString_strcat(src, buff);

			stmnt_count = ast.stmn_list->count;
			begin_pos = ast.pos;

			err = structAst_scaneTokens(&ast); 
			if (err->type == ERROR_SUCCESS) {
				structCarbonError_free(err);

				err = structAst_makeTree(&ast, ast.stmn_list, STMNEND_EOF); 
				ast.pos = ast.tokens->count - 1;
				if (ast.tokens->count > 0 && ast.tokens->list[ast.pos]->group == TKG_EOF) {
					structTokenList_deleteLast(ast.tokens);
					// ast.tokens->list[ast.pos]->type		= TK_PASS;
					// ast.tokens->list[ast.pos]->group	= TKG_PASS;
				}
				// structTokenList_print(ast.tokens);

				if (err->type == ERROR_SUCCESS) {
					is_eof = false;
					structStatement_print(ast.stmn_list->list[ast.stmn_list->count - 1], 0); // execute it
					// pass
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

					if (stmnt_count != ast.stmn_list->count)
						structAst_deleteLastStatement(&ast);

				}
			}
			else {
				printf("%s\n", err->message.buffer);
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
	printf("%s\n", text);


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
	structTokenList_print(ast.tokens);

	// debug print statements
	structStatementList_print( ast.stmn_list );

	printf("main runned successfully!\n");
	return 0;
}