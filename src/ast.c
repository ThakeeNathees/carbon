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
	for (int i=0; i< indent; i++) printf("\t"); printf("Expr  : ");
	for (int i=self->begin_pos; i<=self->end_pos; i++){
		printf("%s ", self->token_list->list[i]->name); //structToken_print(self->token_list->list[i]);
	}printf("\n");
}
struct Expression* structExpression_new(struct TokenList* token_list){
	struct Expression* expr = (struct Expression*)malloc( sizeof(struct Expression) );
	structExpression_init(expr, token_list);
	return expr;
}
/***************** </Expression> *************/

/***************** <ExprDtype> *************/
void structExprDtype_init(struct ExprDtype* self, struct Token* dtype){
	self->dtype   = dtype;
	self->is_map  = false;
	self->is_list = false;
}
void structExprDtype_print(struct ExprDtype* self, int indent, bool new_line){
	if (new_line) { for (int i=0; i< indent; i++) printf("\t"); printf("Type  : "); printf("%s", self->dtype->name); }
	else{ printf("%s", self->dtype->name); }
	if (self->is_list){
		printf("<");structExprDtype_print(self->value, 0, false);printf(">");

	} else if (self->is_map){
		printf("<key:%s, value:", self->key->name ); structExprDtype_print(self->value, 0, false);printf(">");
	}
	if (new_line) printf("\n");
}
struct ExprDtype* structExprDtype_new(struct Token* dtype){
	struct ExprDtype* new_dtype = (struct ExprDtype*)malloc(sizeof(struct ExprDtype));
	structExprDtype_init(new_dtype, dtype);
	return new_dtype;
}
/***************** </ExprDtype> *************/

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
	self->statement.unknown.has_expr = false;
	self->indent = 0;
}
void structStatement_print(struct Statement* self){
	if (self->type == STMNT_UNKNOWN){ 
		for (int i=0; i< self->indent; i++)  printf("\t");printf("Stmnt : Unknown\n"); 
		if (self->statement.unknown.has_expr)structExpression_print(self->statement.unknown.expr, self->indent+1);
	}
	else if (self->type == STMNT_IMPORT){
		for (int i=0; i< self->indent; i++) printf("\t");printf("Stmnt : IMPORT\n");
		for (int i=0; i< self->indent+1; i++) printf("\t");printf("path  : %s\n", self->statement.import.path->name );
	}
	else if (self->type == STMNT_VAR_INIT){
		for (int i=0; i< self->indent; i++) printf("\t");printf("Stmnt : VAR_INIT\n");
		structExprDtype_print(self->statement.init.dtype,self->indent+1, true);
		for (int i=0; i< self->indent+1; i++) printf("\t");printf("idf   : %-6s\n", self->statement.init.idf->name);
		if (self->statement.init.has_expr) structExpression_print(self->statement.init.expr, self->indent+1);
	}
	else if (self->type == STMNT_ASSIGN){
		for (int i=0; i< self->indent; i++) printf("\t");printf("Stmnt : ASSIGN\n");
		structExpression_print(self->statement.assign.idf, self->indent+1);
		for (int i=0; i< self->indent+1; i++) printf("\t");printf("op    : %-6s\n", self->statement.assign.op->name);
		structExpression_print(self->statement.assign.expr, self->indent+1);
	}
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
void structStatementList_print(struct StatementList* self){
	for (int i=0; i< self->count; i++){
		structStatement_print(self->list[i]);
	}
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
	EXPREND_ASSIGN, // for assignment statement
};

struct CarbonError* structAst_countArgs(struct Ast* self, int* count){

	// check -> idf ( ) : no args
	//utils_make_error("Error: unexpected EOF, expected ')'", ERROR_UNEXP_EOF, self->tokens->list[self->pos]->pos, self->src, self->file_name);
	if (self->tokens->count < self->pos + 3 ) utils_error_exit("Error: unexpected EOF, expected ')'", self->tokens->list[self->pos]->pos, self->src, self->file_name); 
	if (self->tokens->list[self->pos+1]->type != BRACKET || strcmp(self->tokens->list[self->pos+1]->name,LPARN)!=0 ) utils_error_exit("Error: expected bracket '('", self->tokens->list[self->pos+1]->pos, self->src, self->file_name); 
	struct Token* token = self->tokens->list[self->pos + 2 ];
	if (token->type == BRACKET && strcmp(token->name, RPARN)==0) {*count = 0; return structCarbonError_new(); }

	int i = 0, arg_count = 1;
	while (true){
		token = self->tokens->list[self->pos + i++ ];
		if (token->type == BRACKET && strcmp(token->name, RPARN)==0){ *count = arg_count; return structCarbonError_new(); }
		else if (token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0 ){ 
			arg_count++; 
			token = self->tokens->list[self->pos + i ];
			if (token->type == BRACKET) utils_error_exit("Error: unexpected character", token->pos, self->src, self->file_name);  // func(arg , )
		}
		else if (token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0) utils_error_exit("Error: unexpected semicollon, expected ')' or args", token->pos, self->src, self->file_name); 
		else if (token->type == TK_EOF) utils_error_exit("Error: unexpected EOF, expected ')'", token->pos, self->src, self->file_name); 
		else if (structToken_isAssignmentOperator(token)) utils_error_exit("Error: unexpected operator", token->pos, self->src, self->file_name); 
	}
}

// semicollon or other end type not included in expr
struct Expression* structAst_scaneExpr(struct Ast* self, enum structAst_ExprEndType end_type){
	struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	while(true){
		struct Token* token = self->tokens->list[(self->pos)];
		if (token->type == TK_EOF ) utils_error_exit("Error: unexpected EOF", token->pos, self->src, self->file_name); 
		if (end_type == EXPREND_SEMICOLLON && token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_COMMA && token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_RPRAN && token->type == BRACKET && strcmp(token->name,RPARN)==0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_ASSIGN && structToken_isAssignmentOperator(token) ){ expr->end_pos = self->pos-1; break; }
		else if (end_type == EXPREND_COMMA_OR_RPRAN && ( (token->type == BRACKET && strcmp(token->name,RPARN)==0) || (token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0) ) ){ 
			expr->end_pos = self->pos-1; break;
		}

		// token = "." check if expr is method or field
		if (token->type == SYMBOL && strcmp(token->name, SYM_DOT)==0){
			token = self->tokens->list[++self->pos];
			if (token->type != IDENTIFIER ) utils_error_exit("Error: expected an identifier", token->pos, self->src, self->file_name);
			if ( (self->tokens->list[self->pos+1])->type == BRACKET && strcmp((self->tokens->list[self->pos+1])->name, LPARN )==0 ){
				token->type = FUNCTION; token->func_is_method  = true;
				structAst_countArgs(self, &(token->func_args_count)); // TODO: err_handle
			}
			else { token->idf_is_field = true; /* not conform */ }
		}

		// token = idf check if expr is function
		if (token->type == IDENTIFIER){
			if ( (self->tokens->list[self->pos+1])->type == BRACKET && strcmp((self->tokens->list[self->pos+1])->name, LPARN )==0 ){
				// TODO: check if function is registered else error!
				token->type = FUNCTION; // not method
				// token->func_args_count == structAst_countArgs(self); // TODO: args count already known : assert
			}
			else {
				// TODO: check if the variable is registered else error!
				token->type = VARIABLE;
			}
		}

		// if builtin count arg count and assert
		if (token->type == BUILTIN){
			int args_count;
			structAst_countArgs(self, &args_count); // TODO: ERROR handle
			if (token->func_args_count != args_count){ 
				printf("TypeError: func:%s takes %i arguments (%i given)", token->name, token->func_args_count, args_count); 
				utils_error_exit("", token->pos, self->src, self->file_name);}
		}

		// if token == '-' check it's single operator or binary
		if (token->type == OPERATOR && strcmp(token->name, OP_MINUS)==0){
			struct Token* before = self->tokens->list[self->pos-1]; if (before->type == TK_PASS) before = self->tokens->list[self->pos-2]; // skip TK_PASS
			if (token->pos == expr->begin_pos) token->minus_is_single_op = true; 									   // begining minus is single op
			else if ( before->type == OPERATOR ) token->minus_is_single_op = true; 									   // before minus is operator
			else if ( before->type == BRACKET && strcmp(before->name, LPARN)==0 )  token->minus_is_single_op = true;    // before (
			else if ( before->type == BRACKET && strcmp(before->name, RSQ_BRACKET)==0)token->minus_is_single_op = true; // before [
			else if ( before->type == SYMBOL  && strcmp(before->name, SYM_COMMA)==0) token->minus_is_single_op = true;  // before ,
			// after bool_op bool expected
			//else if ( before->type == KEYWORD && strcmp(before->name, KWORD_AND)==0) token->minus_is_single_op = true;  // before and 
			//else if ( before->type == KEYWORD && strcmp(before->name, KWORD_OR )==0) token->minus_is_single_op = true;  // before or
			//else if ( before->type == KEYWORD && strcmp(before->name, KWORD_NOT)==0) token->minus_is_single_op = true;  // before not
		}

		/**************** INVALID SYNTAX ***********************/
		// if number and next is '(' : numbers anen't callable error
		if (token->type == NUMBER){
			if (self->tokens->list[self->pos+1]->type == BRACKET && strcmp(self->tokens->list[self->pos+1]->name, LPARN)==0)
				utils_error_exit("Error: numbers aren't callable", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
		}
		// two numbers can't come next to each other
		if (token->type == NUMBER && self->tokens->list[self->pos+1]->type == NUMBER)
			utils_error_exit("Error: invalid syntax", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
		// after a close bracket number cant come
		if (token->type == BRACKET && strcmp(token->name, RPARN)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			utils_error_exit("Error: invalid syntax", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
		if (token->type == BRACKET && strcmp(token->name, RCRU_BRACKET)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			utils_error_exit("Error: invalid syntax", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
		if (token->type == BRACKET && strcmp(token->name, LSQ_BRACKET)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			utils_error_exit("Error: invalid syntax", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
		/********************************************************/


		(self->pos)++;
	}
}

bool structAst_isNextStmnAssign(struct Ast* self, int* assign_op_pos){
	int i = 0;
	while(true){
		struct Token* token = self->tokens->list[ self->pos + i ];
		if (token->type == TK_EOF ) utils_error_exit("Error: unexpected EOF", token->pos, self->src, self->file_name); 
		if (token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0){ return false;}
		if (structToken_isAssignmentOperator(token)){ 
			if (assign_op_pos != NULL) *assign_op_pos = token->pos;
			return true; 
		}
		i++;
	}
}

struct ExprDtype* structAst_scaneDtype(struct Ast* self){

	struct Token* token = self->tokens->list[self->pos];
	if (token->type != DTYPE) utils_error_exit("Error: expected a data type", token->pos, self->src, self->file_name); 
	struct ExprDtype* ret =  structExprDtype_new(token);

	// for map<dtype, dtype> list<dtype> 
	if (strcmp(token->name, DTYPE_LIST)==0){
		ret->is_list = true;
		token = self->tokens->list[++self->pos];
		if (token->type != OPERATOR || strcmp(token->name, RTRI_BRACKET)!=0) utils_error_exit("Error: expected bracket '<'", token->pos, self->src, self->file_name); 
		token->type = BRACKET;
		++self->pos; ret->value = structAst_scaneDtype(self);
		token = self->tokens->list[self->pos];

		if (token->type == OPERATOR && strcmp(token->name, OP_RSHIFT)==0 && (self->tokens->list[self->pos+1])->type == TK_PASS ){
			token->name[1] = '\0'; // now token is '>'
			(self->tokens->list[self->pos+1])->type = OPERATOR; (self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0';
		}
		else if (token->type != OPERATOR || strcmp(token->name, LTRI_BRACKET)!=0 ) utils_error_exit("Error: exprcted bracket '>'", token->pos, self->src, self->file_name);
		token->type = BRACKET;
	}
	else if (strcmp(token->name, DTYPE_MAP)==0){
		ret->is_map = true;
		token = self->tokens->list[++self->pos];
		if (token->type != OPERATOR || strcmp(token->name, RTRI_BRACKET)!=0) utils_error_exit("Error: expected bracket '<'", token->pos, self->src, self->file_name); 
		token->type = BRACKET;
		token = self->tokens->list[++self->pos];
		if (token->type != DTYPE) utils_error_exit("Error: expected a data type", token->pos, self->src, self->file_name); 
		if (strcmp(token->name, DTYPE_LIST)==0 ) utils_error_exit("Error: list objects can't be a key", token->pos, self->src, self->file_name); 
		if (strcmp(token->name, DTYPE_MAP)==0 ) utils_error_exit("Error: map objects can't be a key", token->pos, self->src, self->file_name);
		ret->key = token;
		token = self->tokens->list[++self->pos];
		if (token->type != SYMBOL || strcmp(token->name, SYM_COMMA)!=0 ) utils_error_exit("Error: exprcted symbol ','", token->pos, self->src, self->file_name);
		token = self->tokens->list[++self->pos];
		ret->value = structAst_scaneDtype(self);
		token = self->tokens->list[self->pos];
		if (token->type == OPERATOR && strcmp(token->name, OP_RSHIFT)==0 && (self->tokens->list[self->pos+1])->type == TK_PASS ){
			token->name[1] = '\0'; // now token is '>'
			(self->tokens->list[self->pos+1])->type = OPERATOR; (self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0';
		}
		else if (token->type != OPERATOR || strcmp(token->name, LTRI_BRACKET)!=0 ) utils_error_exit("Error: exprcted bracket '>'", token->pos, self->src, self->file_name);
		token->type = BRACKET;
	}

	++self->pos;
	return ret;

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
		if ( tk->type == OPERATOR && strcmp(tk->name, OP_RSHIFT)==0){ // if tk == >> add pass to make it > > for close bracket : list<list<char>>
			tk = structTokenList_createToken(self->tokens);
			tk->type = TK_PASS;
		}
	}
	// set tk_eof position = last token
	if (self->tokens->count > 1) self->tokens->list[self->tokens->count -1]->pos = self->tokens->list[self->tokens->count -2]->pos;
}



void structAst_makeTree(struct Ast* self, struct StatementList* statement_list) {
	while ( true ){
		
		struct Token* token = self->tokens->list[self->pos];

		if (self->pos == self->tokens->count -1){
			if (token->type != TK_EOF) { printf("InternalError: expected token TK_EOF\nfound: "); structToken_print(token); exit(1); }
			break;
		}


		if (token->type == COMMENT); // do nothing

		else if (token->type == DTYPE){

			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_VAR_INIT;
			stmn->statement.init.dtype = structAst_scaneDtype(self);

			token = self->tokens->list[self->pos];
			if (token->type != IDENTIFIER){ utils_error_exit("Error: expected an identifier", token->pos, self->src, self->file_name); }
			stmn->statement.init.idf = token;
			token = self->tokens->list[++self->pos];
			if ( token->type == SYMBOL && strcmp(token->name , SYM_SEMI_COLLON)==0 ){
				stmn->statement.init.has_expr = false;
			} else { // scane expr
				if (token->type != OPERATOR && strcmp(token->name, OP_EQ)!=0){ utils_error_exit("Error: expected '=' or ';'", token->pos, self->src, self->file_name); }
				++self->pos;
				struct Expression* expr = structAst_scaneExpr(self, EXPREND_SEMICOLLON);
				stmn->statement.init.expr = expr;
				stmn->statement.init.has_expr = true;
			}
			structStatementList_addStatement(statement_list, stmn);
		}

		else if (token->type == IDENTIFIER){ // could be variable, function, 
			// assignment statement
			if (structAst_isNextStmnAssign(self, NULL)){
				struct Statement* stmn = structStatement_new();
				stmn->type = STMNT_ASSIGN;

				stmn->statement.assign.idf = structAst_scaneExpr(self, EXPREND_ASSIGN);
				token = self->tokens->list[self->pos++];
				if (!structToken_isAssignmentOperator(token)) utils_error_exit("InternalError: expected an assignment operator", token->pos, self->src, self->file_name);
				stmn->statement.assign.op = token;
				stmn->statement.assign.expr = structAst_scaneExpr(self, EXPREND_SEMICOLLON);
				structStatementList_addStatement(statement_list, stmn);
			} else {
				// if have globals : identifier is known or undefined, if idf is func stmn is func call // TODO: 
				struct Statement* stmn = structStatement_new();
				stmn->statement.unknown.expr = structAst_scaneExpr(self, EXPREND_SEMICOLLON);
				stmn->statement.unknown.has_expr = true;
				structStatementList_addStatement(statement_list, stmn);
			}
		}

		else if ( token->type == NUMBER || token->type == STRING ){
			struct Statement* stmn = structStatement_new();
			int err_pos = 0; if (structAst_isNextStmnAssign(self, &err_pos)) utils_error_exit("Error: can't assign to literls", err_pos, self->src, self->file_name);
			stmn->statement.unknown.expr = structAst_scaneExpr(self, EXPREND_SEMICOLLON);
			stmn->statement.unknown.has_expr = true;
			structStatementList_addStatement(statement_list, stmn);
		}

		else if ( token->type == KEYWORD && strcmp(token->name, KWORD_IMPORT)==0){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_IMPORT;
			token = self->tokens->list[++self->pos];
			if (token->type != STRING)utils_error_exit("Error: expected import path string", token->pos, self->src, self->file_name);
			// TODO: validate token path
			stmn->statement.import.path = token;
			token = self->tokens->list[++self->pos];
			if (token->type != SYMBOL || strcmp(token->name, SYM_SEMI_COLLON)!=0 )utils_error_exit("Error: expected a semicollon", token->pos, self->src, self->file_name);
			structStatementList_addStatement(statement_list, stmn);
		}

		// if token == static && ast state != reading class error!



		self->pos++;
	}
	


}
/***************** </Ast> *************/