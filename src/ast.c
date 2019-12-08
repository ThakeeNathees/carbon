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
	if (self->tokens->count < self->pos + 3 ) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_UNEXP_EOF, self->tokens->list[self->pos]->pos, self->src, self->file_name, false); 
	if (self->tokens->list[self->pos+1]->type != BRACKET || strcmp(self->tokens->list[self->pos+1]->name,LPARN)!=0 ) utils_error_exit("InternalError: expected bracket '('", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
	if (self->tokens->list[self->pos+2]->type == SYMBOL && strcmp(self->tokens->list[self->pos+2]->name, SYM_COMMA)==0 )return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->tokens->list[self->pos+2]->pos, self->src, self->file_name, false); 
	struct Token* token = self->tokens->list[self->pos + 2 ];
	if (token->type == BRACKET && strcmp(token->name, RPARN)==0) {*count = 0; return structCarbonError_new(); }

	int i = 0, arg_count = 1;
	while (true){
		token = self->tokens->list[self->pos + i++ ];
		if (token->type == BRACKET && strcmp(token->name, RPARN)==0){ *count = arg_count; return structCarbonError_new(); }
		else if (token->type == SYMBOL && strcmp(token->name, SYM_COMMA)==0 ){ 
			arg_count++; 
			token = self->tokens->list[self->pos + i ];
			if (token->type == BRACKET) return utils_make_error("SyntaxError: unexpected character", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);  // func(arg , )
		}
		else if (token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
		else if (token->type == TK_EOF) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
		else if (structToken_isAssignmentOperator(token)) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
	}
}

// semicollon or other end type not included in expr
struct CarbonError* structAst_scaneExpr(struct Ast* self, struct Expression* expr, enum structAst_ExprEndType end_type){
	//struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	while(true){
		
		struct Token* token = self->tokens->list[(self->pos)];
		if (token->type == TK_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false); 
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
			if (token->type != IDENTIFIER ) return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
			if ( (self->tokens->list[self->pos+1])->type == BRACKET && strcmp((self->tokens->list[self->pos+1])->name, LPARN )==0 ){
				token->type = FUNCTION; token->func_is_method  = true;
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_count)); if (err->type != ERROR_SUCCESS){ return err; }
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
			struct CarbonError* err = structAst_countArgs(self, &args_count); if (err->type != ERROR_SUCCESS){ return err; }
			if (token->func_args_count != args_count){
				char* err_msg = (char*)malloc(ERROR_LINE_SIZE);
				snprintf(err_msg, ERROR_LINE_SIZE, "TypeError: func:%s takes %i arguments (%i given)", token->name, token->func_args_count, args_count);
				return utils_make_error(err_msg, ERROR_TYPE, token->pos, self->src, self->file_name, true);
			}
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
				return utils_make_error("TypeError: numbers aren't callable", ERROR_TYPE, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false);
		}
		// two numbers can't come next to each other
		if (token->type == NUMBER && self->tokens->list[self->pos+1]->type == NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false);
		// after a close bracket number cant come
		if (token->type == BRACKET && strcmp(token->name, RPARN)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false);
		if (token->type == BRACKET && strcmp(token->name, RCRU_BRACKET)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false);
		if (token->type == BRACKET && strcmp(token->name, LSQ_BRACKET)==0 && self->tokens->list[self->pos+1]->type == NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false);
		/********************************************************/


		(self->pos)++;
	}
	return structCarbonError_new();
}

struct CarbonError* structAst_isNextStmnAssign(struct Ast* self, bool* ret, int* assign_op_pos){
	struct CarbonError* err = structCarbonError_new();
	int i = 0;
	while(true){
		struct Token* token = self->tokens->list[ self->pos + i ];
		if (token->type == TK_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false); 
		if (token->type == SYMBOL && strcmp(token->name, SYM_SEMI_COLLON)==0){ *ret = false; return err; }
		if (structToken_isAssignmentOperator(token)){ 
			if (assign_op_pos != NULL) *assign_op_pos = token->pos;
			*ret = true; return err;
		}
		i++;
	}
	return err;
}

struct CarbonError* structAst_scaneDtype(struct Ast* self, struct ExprDtype** ret){

	struct Token* token = self->tokens->list[self->pos];
	if (token->type != DTYPE) return utils_make_error("SyntexError: expected a data type", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
	*ret =  structExprDtype_new(token);

	// for map<dtype, dtype> list<dtype> 
	if (strcmp(token->name, DTYPE_LIST)==0){
		(*ret)->is_list = true;
		token = self->tokens->list[++self->pos];
		if (token->type != OPERATOR || strcmp(token->name, RTRI_BRACKET)!=0) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
		token->type = BRACKET;
		++self->pos; 
		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if (err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos];

		if (token->type == OPERATOR && strcmp(token->name, OP_RSHIFT)==0 && (self->tokens->list[self->pos+1])->type == TK_PASS ){
			token->name[1] = '\0'; // now token is '>'
			(self->tokens->list[self->pos+1])->type = OPERATOR; (self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0';
		}
		else if (token->type != OPERATOR || strcmp(token->name, LTRI_BRACKET)!=0 ) return utils_make_error("SyntexError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
		token->type = BRACKET;
	}
	else if (strcmp(token->name, DTYPE_MAP)==0){
		(*ret)->is_map = true;
		token = self->tokens->list[++self->pos];
		if (token->type != OPERATOR || strcmp(token->name, RTRI_BRACKET)!=0) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
		token->type = BRACKET;
		token = self->tokens->list[++self->pos];
		if (token->type != DTYPE) return utils_make_error("SyntaxError: expected a data type", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); 
		if (strcmp(token->name, DTYPE_LIST)==0 ) return utils_make_error("TypeError: list objects can't be a key", ERROR_TYPE, token->pos, self->src, self->file_name, false); 
		if (strcmp(token->name, DTYPE_MAP)==0 ) return utils_make_error("TypeError: map objects can't be a key", ERROR_TYPE, token->pos, self->src, self->file_name, false);
		(*ret)->key = token;
		token = self->tokens->list[++self->pos];
		if (token->type != SYMBOL || strcmp(token->name, SYM_COMMA)!=0 ) return utils_make_error("SyntaxError: exprcted symbol ','", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
		token = self->tokens->list[++self->pos];

		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if(err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos];
		if (token->type == OPERATOR && strcmp(token->name, OP_RSHIFT)==0 && (self->tokens->list[self->pos+1])->type == TK_PASS ){
			token->name[1] = '\0'; // now token is '>'
			(self->tokens->list[self->pos+1])->type = OPERATOR; (self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0';
		}
		else if (token->type != OPERATOR || strcmp(token->name, LTRI_BRACKET)!=0 ) return utils_make_error("SyntaxError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
		token->type = BRACKET;
	}

	++self->pos;
	return structCarbonError_new(); //return ret;

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

struct CarbonError* structAst_scane(struct Ast* self){
	bool eof = false;
	while (!eof){
		struct Token* tk = structTokenList_createToken(self->tokens);
		structTokenScanner_setToken(self->token_scanner, tk);
		struct CarbonError* err = structTokenScanner_scaneToken(self->token_scanner, &eof); if (err->type != ERROR_SUCCESS) return err;
		if (eof) tk->type = TK_EOF;
		if ( tk->type == OPERATOR && strcmp(tk->name, OP_RSHIFT)==0){ // if tk == >> add pass to make it > > for close bracket : list<list<char>>
			struct Token* pass_tk = structTokenList_createToken(self->tokens); pass_tk->pos = tk->pos+1;
			pass_tk->type = TK_PASS;
		}
	}
	// set tk_eof position = last token
	if (self->tokens->count > 1) self->tokens->list[self->tokens->count -1]->pos = self->tokens->list[self->tokens->count -2]->pos;
	return structCarbonError_new();
}



struct CarbonError* structAst_makeTree(struct Ast* self, struct StatementList* statement_list) {
	struct CarbonError* err;

	while ( true ) {
		
		struct Token* token = self->tokens->list[self->pos];

		if (self->pos == self->tokens->count -1){
			if (token->type != TK_EOF) { printf("InternalError: expected token TK_EOF\nfound: "); structToken_print(token); exit(1); }
			return err; //break;
		}

		if (token->type == COMMENT); // do nothing

		else if (token->type == DTYPE){

			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_VAR_INIT;
			err = structAst_scaneDtype(self, &(stmn->statement.init.dtype)); if (err->type != ERROR_SUCCESS) return err;
			token = self->tokens->list[self->pos];
			if (token->type != IDENTIFIER){ return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); }
			stmn->statement.init.idf = token;
			token = self->tokens->list[++self->pos];
			if ( token->type == SYMBOL && strcmp(token->name , SYM_SEMI_COLLON)==0 ){
				stmn->statement.init.has_expr = false;
			} else { // scane expr
				if (token->type != OPERATOR && strcmp(token->name, OP_EQ)!=0){ return utils_make_error("SyntaxError: expected '=' or ';'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false); }
				++self->pos;
				struct Expression* expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err;
				stmn->statement.init.expr = expr;
				stmn->statement.init.has_expr = true;
			}
			structStatementList_addStatement(statement_list, stmn);
		}

		// TODO: builtin (or just add to identifier with || )

		else if (token->type == IDENTIFIER || token->type == BUILTIN){ // could be variable, function, 
			// assignment statement
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err;
			if (is_next_assign){
				struct Statement* stmn = structStatement_new();
				stmn->type = STMNT_ASSIGN;
				stmn->statement.assign.idf = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.assign.idf, EXPREND_ASSIGN);  if (err->type != ERROR_SUCCESS) return err;
				token = self->tokens->list[self->pos++];
				if (!structToken_isAssignmentOperator(token)) utils_error_exit("InternalError: expected an assignment operator", token->pos, self->src, self->file_name); // COMPLIER INTERNAL ERROR EXIT(1)
				stmn->statement.assign.op = token;
				stmn->statement.assign.expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.assign.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err;
				structStatementList_addStatement(statement_list, stmn);
			} else {
				// if have globals : identifier is known or undefined, if idf is func/builtin stmn is func call // TODO: 
				struct Statement* stmn = structStatement_new();
				stmn->statement.unknown.expr = structExpression_new(self->tokens);
				structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON);
				stmn->statement.unknown.has_expr = true;
				structStatementList_addStatement(statement_list, stmn);
			}
		}

		else if ( token->type == NUMBER || token->type == STRING ){
			struct Statement* stmn = structStatement_new();
			int err_pos = 0; 
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err;
			if (is_next_assign) return utils_make_error("TypeError: can't assign to literls", ERROR_TYPE, err_pos, self->src, self->file_name, false);
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err;
			stmn->statement.unknown.has_expr = true;
			structStatementList_addStatement(statement_list, stmn);
		}

		else if ( token->type == KEYWORD && strcmp(token->name, KWORD_IMPORT)==0){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_IMPORT;
			token = self->tokens->list[++self->pos];
			if (token->type != STRING)return utils_make_error("SyntaxError: expected import path string", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
			// TODO: validate token path
			stmn->statement.import.path = token;
			token = self->tokens->list[++self->pos];
			if (token->type != SYMBOL || strcmp(token->name, SYM_SEMI_COLLON)!=0 )return utils_make_error("SyntaxError: expected a semicollon", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
			structStatementList_addStatement(statement_list, stmn);
		}

		/*
		else if (token->type == KEYWORD && strcmp(token->name, KWORD_WHILE)==0){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_WHILE;
			token = self->tokens->list[++self->pos];
			if (token->type != BRACKET || strcmp(token->name, LPARN)|=0)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src, self->file_name, false);
			++self->pos;
			struct Expression* expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err;
			stmn->statement.stmn_while.expr_bool = expr;
			stmn->statement.stmn_while.stmn_list = structStatementList_new();
			// TODO: scane 
		}
		*/
		
		// if token == static && ast state != reading class error!

		self->pos++;
	}

	// code never reaches here

}
/***************** </Ast> *************/