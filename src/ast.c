#include "ast.h"

static const char* STATEMENT_TYPE_STRING[] = {
	FOREACH_STATEMENT_TYPE(GENERATE_STRING)
};

// public api
const char* enumStatementType_toString(enum StatementType self){
	return STATEMENT_TYPE_STRING[self];
}

/***************** <Expression> *************/
// public
void structExpression_init(struct Expression* self, struct TokenList* token_list){
	self->token_list = token_list;
	self->begin_pos  = -1;
	self->end_pos    = -1;
}
void structExpression_print(struct Expression* self, int indent){
	for (int i=0; i< indent; i++) printf("\t"); printf("Expression:\n");
	for (int i=self->begin_pos; i<=self->end_pos; i++){
		for (int i=0; i< indent; i++) printf("\t"); printf("  "); structToken_print(self->token_list->list[i]);
	}
}
struct Expression* structExpression_new(struct TokenList* token_list){
	struct Expression* expr = (struct Expression*)malloc( sizeof(struct Expression) );
	structExpression_init(expr, token_list);
	return expr;
}
/***************** </Expression> *************/

/***************** <ExpressionList> *************/
void structExpressionList_init(struct ExpressionList* self, int growth_size, struct TokenList* token_list){
	self->count = 0;
	self->growth_size = growth_size;
	self->size  = self->growth_size;
	self->list  = (struct Expression**)malloc( sizeof(struct Expression) * self->growth_size );
}
void structExpressionList_addExpression(struct ExpressionList* self, struct Expression* expr){
	if (self->count >= self->size){
		struct Expression** new_list = (struct Expression**)malloc( sizeof(struct Expression)*(self->size + self->growth_size) ) ;
		self->size += self->growth_size;
		for ( int i=0; i< self->count; i++){
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = expr;	
}
struct Expression* structExpressionList_createExpression(struct ExpressionList* self){
	struct Expression* new_expr = structExpression_new(self->token_list);
	structExpressionList_addExpression(self, new_expr);
	return new_expr;
}
struct ExpressionList* structExpressionList_new(struct TokenList* token_list){
	struct ExpressionList* expr_list = (struct ExpressionList*)malloc( sizeof(struct ExpressionList) );
	structExpressionList_init(expr_list, EXPRESSION_LIST_SIZE, token_list);
	return expr_list;
}
/***************** </ExpressionList> *************/


/***************** <Statement> *************/
void structStatement_init(struct Statement* self){
	self->type = STMNT_UNKNOWN;
}
struct Statement* structStatement_new(){
	struct Statement* new_stmn = (struct Statement*)malloc(sizeof(struct Statement));
	structStatement_init(new_stmn);
	return new_stmn;
}
/***************** </Statement> *************/

/***************** <StatementList> *************/
void structStatementList_init(struct StatementList* self, int growth_size){
	self->count = 0;
	self->growth_size = growth_size;
	self->size  = self->growth_size;
	self->list  = (struct Statement**)malloc( sizeof(struct Statement) * self->growth_size );
}
void structStatementList_addStatement(struct StatementList* self, struct Statement* statement){
	if (self->count >= self->size){
		struct Statement** new_list = (struct Statement**)malloc( sizeof(struct Statement)*(self->size + self->growth_size) ) ;
		self->size += self->growth_size;
		for ( int i=0; i< self->count; i++){
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = statement;	
}
struct Statement* structStatementList_createStatement(struct StatementList* self){
	struct Statement* new_stmn = structStatement_new();
	structStatementList_addStatement(self, new_stmn);
	return new_stmn;
}
struct StatementList* structStatementList_new(){
	struct StatementList* stmn_list = (struct StatementList*)malloc( sizeof(struct StatementList) );
	structStatementList_init(stmn_list, STATEMENT_LIST_SIZE);
	return stmn_list;
}
/***************** </StatementList> *************/


/***************** <Ast> *************/

// private
enum structAst_ExprEndType
{
	EXPREND_SEMICOLLON,
	EXPREND_COMMA, // ',' function
	EXPREND_RPRAN, // ')'
	EXPREND_COMMA_OR_RPRAN,
};

int structAst_countArgs(struct Ast* self){

	// check -> idf ( ) : no args
	if (self->tokens->count < self->pos + 3 ) utils_error_exit("Error: unexpected EOF, expected ')'", self->tokens->list[self->pos]->pos, self->src, self->file_name); 
	struct Token* token = self->tokens->list[self->pos + 2 ];
	if (token->type == BRACKET && strcmp(token->name, RPARN)==0) return 0;

	int i = 0, arg_count = 1;
	while (true){
		token = self->tokens->list[self->pos + i++ ];
		if (token->type == BRACKET && strcmp(token->name, RPARN)==0){ return arg_count; }
		else if (token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0 ){ 
			arg_count++; 
			token = self->tokens->list[self->pos + i ];
			if (token->type == BRACKET) utils_error_exit("Error: unexpected character", token->pos, self->src, self->file_name);  // func(arg , )
		}
		else if (token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0) utils_error_exit("Error: unexpected semicollon, expected ')'", token->pos, self->src, self->file_name); 
		else if (token->type == TK_EOF) utils_error_exit("Error: unexpected EOF, expected ')'", token->pos, self->src, self->file_name); 
	}
}

struct Expression* structAst_scaneExpr(struct Ast* self, enum structAst_ExprEndType end_type){
	struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	while(true){
		struct Token* token = self->tokens->list[++self->pos];
		if (token->type == TK_EOF ) utils_error_exit("Error: unexpected EOF", token->pos, self->src, self->file_name); 
		if (end_type == EXPREND_SEMICOLLON && token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_COMMA && token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_RPRAN && token->type == BRACKET && strcmp(token->name,RPARN)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_COMMA_OR_RPRAN && ( (token->type == BRACKET && strcmp(token->name,RPARN)==0) || (token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0) ) ){ 
			expr->end_pos = self->pos-1; break;
		}

		// token = "." check if expr is method or field
		if (token->type == SYMBOL && strcmp(token->name, SYM_DOT)==0){
			token = self->tokens->list[++self->pos];
			if (token->type != IDENTIFIER ) utils_error_exit("Error: expected an identifier", token->pos, self->src, self->file_name);
			if ( (self->tokens->list[self->pos+1])->type == BRACKET && strcmp((self->tokens->list[self->pos+1])->name, LPARN )==0 ){
				token->type = FUNCTION;
				token->func_is_method  = true;
				token->func_args_count = structAst_countArgs(self);
			}
			else { token->idf_is_field = true; }
		}
		
		// token = idf check if expr is function
		if (token->type == IDENTIFIER){
			if ( (self->tokens->list[self->pos+1])->type == BRACKET && strcmp((self->tokens->list[self->pos+1])->name, LPARN )==0 ){
				token->type = FUNCTION; // not method
				// TODO: check if function is registered else error!
				token->func_args_count = structAst_countArgs(self); // TODO: args count already known : assert
			}
			else { /* TODO: check if the variable is registered */ }
		}

		// if builtin count arg count and assert

	}
}

// public
void structAst_init(struct Ast* self, char* src, char* file_name){
	self->src 			= src;
	self->file_name 	= file_name;
	self->pos 			= 0;
	self->tokens 		= structTokenList_new();
	self->token_scanner = structTokenScanner_new(src, file_name);
	self->stmn_list 	= structStatementList_new();
}
void structAst_scane(struct Ast* self){
	bool eof = false;
	while (!eof){
		struct Token* tk = structTokenList_createToken(self->tokens);
		structTokenScanner_setToken(self->token_scanner, tk);
		eof = structTokenScanner_scaneToken(self->token_scanner);
		if (eof) tk->type = TK_EOF;
	}
}

void structAst_makeTree(struct Ast* self){
	while ( true ){

		struct Token* token = self->tokens->list[self->pos];
		

		if (self->pos == self->tokens->count -1){
			if (token->type != TK_EOF) { printf("CompilerError: expected token TK_EOF\nfound: "); structToken_print(token); exit(1); }
			break;
		}


		if (token->type == COMMENT); // do nothing

		else if (token->type == DTYPE){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_VAR_INIT;
			stmn->statement.init.dtype = token;
			token = self->tokens->list[++self->pos];
			if (token->type != IDENTIFIER){ utils_error_exit("Error: expected an identifier", token->pos, self->src, self->file_name); }
			// if already defined error
			stmn->statement.init.idf = token;
			token = self->tokens->list[++self->pos];
			if ( token->type == SYMBOL && strcmp(token->name , SYM_SEMI_COLLON)==0 ){
				stmn->statement.init.has_expr = false;
			} else { // scane expr
				if (token->type != OPERATOR && strcmp(token->name, OP_EQ)!=0){ utils_error_exit("Error: expected operator =", token->pos, self->src, self->file_name); }
				++self->pos; // scane for expr ends with ;
				struct Expression* expr = structAst_scaneExpr(self, EXPREND_SEMICOLLON);
				stmn->statement.init.expr = expr;

				structExpression_print(expr, 0);
			}
		}

		self->pos++;
	}
}
/***************** </Ast> *************/