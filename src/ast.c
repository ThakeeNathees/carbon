#include "ast.h"

#define INDENT_STR "    "
#define PRINT_INDENT(indent) for (int i = 0; i < indent; i++) printf("%s", INDENT_STR)

static const char* STATEMENT_TYPE_STRING[] = {
	FOREACH_STATEMENT_TYPE(GENERATE_STRING)
};

// public api
const char* enumStatementType_toString(enum StatementType self){
	return STATEMENT_TYPE_STRING[self];
}

/***************** <Expression> *************/
// public
void structExpression_free(struct Expression* self) {
	if (self != NULL) free(self);
}
void structExpression_init(struct Expression* self, struct TokenList* token_list){
	self->token_list = token_list;
	self->begin_pos  = -1;
	self->end_pos    = -1;
}
void structExpression_print(struct Expression* self, int indent, bool new_line){
	PRINT_INDENT(indent);
	for (size_t i=self->begin_pos; i<=self->end_pos; i++){
		if (self->token_list->list[i]->type == TK_STRING) {
			utils_print_str_without_esc(self->token_list->list[i]->name, false, true);
		}
		else printf("%s", self->token_list->list[i]->name);
	}
	if (new_line)printf("\n");
}
struct Expression* structExpression_new(struct TokenList* token_list){
	struct Expression* expr = (struct Expression*)malloc( sizeof(struct Expression) );
	structExpression_init(expr, token_list);
	return expr;
}
/***************** </Expression> *************/

/***************** <ExprDtype> *************/
void structExprDtype_free(struct ExprDtype* self) {
	if (self != NULL) free(self);
}
void structExprDtype_init(struct ExprDtype* self, struct Token* dtype){
	self->dtype   = dtype;
	self->is_map  = false;
	self->is_list = false;
}
void structExprDtype_print(struct ExprDtype* self, int indent, bool print_type){ // no new line after the print
	if (print_type) { PRINT_INDENT(indent); printf("<type> %s", self->dtype->name); }
	else{ printf("%s", self->dtype->name); }
	if (self->is_list){
		printf("<");structExprDtype_print(self->value, 0, false);printf(">");

	} else if (self->is_map){
		printf("<key:%s, value:", self->key->name ); structExprDtype_print(self->value, 0, false);printf(">");
	}
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
		for ( size_t i=0; i< self->count; i++){
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

void structStatement_free(struct Statement* self) {
	if (self != NULL) {
		
		if (self->type == STMNT_UNKNOWN) {
			structExpression_free(self->statement.unknown.expr);
		}
		else if (self->type == STMNT_IMPORT) {
			// pass
		}
		else if (self->type == STMNT_VAR_INI) {
			structExpression_free(self->statement.init.expr);
			structExprDtype_free(self->statement.init.dtype);
		}
		else if (self->type == STMNT_ASSIGN) {
			structExpression_free(self->statement.assign.idf);
			structExpression_free(self->statement.assign.expr);
		}
		else if (self->type == STMNT_IF) {
			structExpression_free(self->statement.stm_if.expr_bool);
			structStatementList_free(self->statement.stm_if.stmn_list);
			structStatementList_free(self->statement.stm_if.else_if_list);
			structStatementList_free(self->statement.stm_if.stmn_list_else);
		}
		else if (self->type == STMNT_ELSE_IF) {
			structExpression_free(self->statement.stmn_else_if.expr_bool);
			structStatementList_free(self->statement.stmn_else_if.stmn_list);
		}
		else if (self->type == STMNT_RETURN) {
			structExpression_free(self->statement.stmn_return.expr);
		}
		else if (self->type == STMNT_BREAK || self->type == STMNT_CONTINUE) {
			// pass
		}
		else if (self->type == STMNT_WHILE) {
			structExpression_free(self->statement.stm_while.expr_bool);
			structStatementList_free(self->statement.stm_while.stmn_list);
		}
		else if (self->type == STMNT_FOR) {
			structStatement_free(self->statement.stm_for.stmn_ini);
			structExpression_free(self->statement.stm_for.expr_bool);
			structExpression_free(self->statement.stm_for.expr_end);
			structStatementList_free(self->statement.stm_for.stmn_list);
		}
		else if (self->type == STMNT_FOREACH) {
			structStatement_free(self->statement.stm_foreach.stmn_ini);
			structExpression_free(self->statement.stm_foreach.expr_itter);
			structStatementList_free(self->statement.stm_foreach.stmn_list);
		}
		else if (self->type == STMNT_FUNC_DEFN) {
			structStatementList_free(self->statement.func_defn.args);
			structExprDtype_free(self->statement.func_defn.ret_type);
			structStatementList_free(self->statement.func_defn.stmn_list);
		}
		else if (self->type == STMNT_CLASS_DEFN) {
			structStatementList_free(self->statement.class_defn.stmn_list);
		}

		else utils_error_exit("InternalError: unknown statement type for delete", 0, "", "");

		free(self);
		
	}
}
void structStatement_init(struct Statement* self, struct Statement* parent, enum StatementType type){
	self->type = type;
	self->parent = parent;
	self->statement.unknown.expr			= NULL;
	self->statement.init.expr				= NULL;
	self->statement.stm_while.stmn_list		= NULL;

	self->statement.stm_if.stmn_list		= NULL;
	self->statement.stm_if.else_if_list		= NULL;
	self->statement.stm_if.stmn_list_else	= NULL;
	self->statement.stmn_else_if.stmn_list	= NULL;
	
	self->statement.stm_for.stmn_ini		= NULL;
	self->statement.stm_for.expr_bool		= NULL;
	self->statement.stm_for.expr_end		= NULL;
	self->statement.stm_for.stmn_list		= NULL;

	self->statement.stm_foreach.stmn_ini	= NULL;
	self->statement.stm_foreach.expr_itter	= NULL;
	self->statement.stm_foreach.stmn_list	= NULL;

	self->statement.func_defn.args			= NULL;
	self->statement.func_defn.ret_type		= NULL;
	self->statement.func_defn.stmn_list		= NULL;

	self->statement.class_defn.par			= NULL;
	self->statement.class_defn.stmn_list	= NULL;

	self->statement.stmn_return.expr		= NULL;

}
void structStatement_print(struct Statement* self, int indent){
	
	if (self->type == STMNT_UNKNOWN){ 
		if (self->statement.unknown.expr != NULL)structExpression_print(self->statement.unknown.expr, indent, true);
	}
	else if (self->type == STMNT_IMPORT){
		PRINT_INDENT(indent); printf("<import> "); printf("path  : %s\n", self->statement.import.path->name );
	}
	else if (self->type == STMNT_VAR_INI){
		PRINT_INDENT(indent); printf("<var_ini> ");
		structExprDtype_print(self->statement.init.dtype, 0, true);
		printf(" idf=%s", self->statement.init.idf->name);
		if (self->statement.init.expr != NULL) { printf(" expr="); structExpression_print(self->statement.init.expr, 0, true); }
		else printf("\n");
		
	}
	else if (self->type == STMNT_ASSIGN){
		PRINT_INDENT(indent); printf("<assign> ");
		printf("expr="); structExpression_print(self->statement.assign.idf, 0, false); printf(" ");
		printf("op=%s expr=", self->statement.assign.op->name);
		structExpression_print(self->statement.assign.expr, 0, true);
	}
	else if (self->type == STMNT_BREAK){
		PRINT_INDENT(indent); printf("<break>\n");
	}
	else if (self->type == STMNT_CONTINUE){
		PRINT_INDENT(indent); printf("<continue>\n");
	}
	else if (self->type == STMNT_WHILE){
		PRINT_INDENT(indent); printf("<while> "); structExpression_print(self->statement.stm_while.expr_bool, 0, true);
		if (self->statement.stm_while.stmn_list != NULL) structStatementList_print(self->statement.stm_while.stmn_list);
		else { PRINT_INDENT(indent+1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FOR) {
		PRINT_INDENT(indent); printf("<FOR>\n");
		if (self->statement.stm_for.stmn_ini  != NULL) { structStatement_print(self->statement.stm_for.stmn_ini, indent + 1); }
		else { PRINT_INDENT(indent+1); printf("(No IniStmn)\n"); }
		if (self->statement.stm_for.expr_bool != NULL) { structExpression_print(self->statement.stm_for.expr_bool, indent + 1, true); }
		else { PRINT_INDENT(indent+1); printf("(No ExprBool)\n"); }
		if (self->statement.stm_for.expr_end  != NULL) { structExpression_print(self->statement.stm_for.expr_end, indent + 1, true); }
		else { PRINT_INDENT(indent+1); printf("(No ExprEnd)\n"); }
		if (self->statement.stm_for.stmn_list != NULL) { structStatementList_print(self->statement.stm_for.stmn_list); }
		else { PRINT_INDENT(indent+1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FOREACH) {
		PRINT_INDENT(indent); printf("<foreach>\n");
		structStatement_print(self->statement.stm_foreach.stmn_ini, indent + 1);
		structExpression_print(self->statement.stm_foreach.expr_itter, indent+1, true);
		if (self->statement.stm_foreach.stmn_list != NULL) {  structStatementList_print(self->statement.stm_foreach.stmn_list);}
		else { PRINT_INDENT(indent+1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_IF) {
		PRINT_INDENT(indent); printf("<if> "); structExpression_print(self->statement.stm_if.expr_bool, 0, true);
		if (self->statement.stm_if.stmn_list != NULL) structStatementList_print(self->statement.stm_if.stmn_list);
		else { PRINT_INDENT(indent+1); printf("(No StmnList)\n"); }

		if (self->statement.stm_if.else_if_list != NULL)  structStatementList_print(self->statement.stm_if.else_if_list);
		
		if (self->statement.stm_if.stmn_list_else != NULL) { PRINT_INDENT(indent); printf("<else>\n"); structStatementList_print(self->statement.stm_if.stmn_list_else); }

	}
	else if (self->type == STMNT_ELSE_IF) {
		PRINT_INDENT(indent); printf("<else_if> "); structExpression_print(self->statement.stmn_else_if.expr_bool, 0, true);
		if (self->statement.stm_while.stmn_list != NULL) structStatementList_print(self->statement.stm_while.stmn_list);
		else { PRINT_INDENT(indent+1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FUNC_DEFN) {
		PRINT_INDENT(indent); printf("<func defn> %s\n", self->statement.func_defn.idf->name);
		if (self->statement.func_defn.args != NULL) structStatementList_print(self->statement.func_defn.args);
		if (self->statement.func_defn.ret_type != NULL) { structExprDtype_print(self->statement.func_defn.ret_type, indent + 1, true); printf("\n"); }
		else { PRINT_INDENT(indent + 1); printf("(Return Type Void)\n"); }
		if (self->statement.func_defn.stmn_list != NULL) structStatementList_print(self->statement.func_defn.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No Func Body)\n"); }
	}
	else if (self->type == STMNT_RETURN) {
		PRINT_INDENT(indent); printf("<return> "); 
		if (self->statement.stmn_return.expr != NULL) structExpression_print(self->statement.stmn_return.expr, 0, true);
		else printf("\n");
	}
}
struct Statement* structStatement_new(enum StatementType type, struct Statement* parent){
	struct Statement* new_stmn = (struct Statement*)malloc(sizeof(struct Statement));
	structStatement_init(new_stmn, parent, type);
	return new_stmn;
}
/***************** </Statement> *************/

/***************** <StatementList> *************/
void structStatementList_init(struct StatementList* self, struct Statement* parent, int growth_size){
	self->parent = parent;
	self->count = 0;
	self->growth_size = growth_size;
	self->size  = self->growth_size;
	self->list  = (struct Statement**)malloc( sizeof(struct Statement) * self->growth_size );
	self->indent = 0;
}
void structStatementList_addStatement(struct StatementList* self, struct Statement* statement){
	if (self->count >= self->size){
		struct Statement** new_list = (struct Statement**)malloc( sizeof(struct Statement)*(self->size + self->growth_size) ) ;
		self->size += self->growth_size;
		for ( size_t i=0; i< self->count; i++){
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = statement;	
}
struct Statement* structStatementList_createStatement(struct StatementList* self, enum StatementType type, struct Statement* parent){
	struct Statement* new_stmn = structStatement_new(type, parent);
	structStatementList_addStatement(self, new_stmn);
	return new_stmn;
}
void structStatementList_print(struct StatementList* self){
	for (size_t i=0; i< self->count; i++){
		structStatement_print(self->list[i], self->indent);
	}
}
struct StatementList* structStatementList_new(struct Statement* parent){
	struct StatementList* stmn_list = (struct StatementList*)malloc( sizeof(struct StatementList) );
	structStatementList_init(stmn_list, parent, STATEMENT_LIST_SIZE);
	return stmn_list;
}
void structStatementList_deleteLast(struct StatementList* self) {
	if (self->count == 0) return;
	struct Statement* last = self->list[self->count - 1];
	structStatement_free(last);
	self->count--;
}
void structStatementList_free(struct StatementList* self) {
	if (self != NULL) {
		while (self->count != 0) {
			structStatementList_deleteLast(self);
		}
		free(self->list);
		free(self);
	}
}
/***************** </StatementList> *************/


/***************** <Ast> *************/

// private
enum structAst_ExprEndType
{
	EXPREND_SEMICOLLON,
	EXPREND_COMMA, // ',' function
	EXPREND_RPRAN, // ')'
	EXPREND_COMMA_OR_RPRAN, // for func args
	EXPREND_ASSIGN, // for assignment statement
};


struct CarbonError* structAst_countArgs(struct Ast* self, int* count, size_t* end_pos){

	// check -> idf ( ) : no args
	if (self->tokens->count < self->pos + 3 ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->tokens->list[self->pos]->pos, self->src->buffer, self->file_name, false, 1); 
	if (self->tokens->list[self->pos+1]->type != TK_BRACKET_LPARAN) return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src->buffer, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr);
	if (self->tokens->list[self->pos+2]->type == TK_SYM_COMMA )return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->tokens->list[self->pos+2]->pos, self->src->buffer, self->file_name, false, 1); 
	struct Token* token = self->tokens->list[self->pos + 2 ];
	if (token->type == TK_BRACKET_RPARAN) { *count = 0; if(end_pos!=NULL)*end_pos = token->pos; return structCarbonError_new(); }

	int i = 0, arg_count = 1, bracket_ptr = 0;
	while (true){
		token = self->tokens->list[self->pos + i++ ];

		if (token->type == TK_BRACKET_LPARAN) bracket_ptr++;
		else if (token->type == TK_BRACKET_RPARAN) {
			bracket_ptr--;
			if (bracket_ptr == 0){ *count = arg_count; if (end_pos != NULL)*end_pos = token->pos; return structCarbonError_new(); }
			if (bracket_ptr < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		else if (token->type == TK_SYM_COMMA ){ 
			token->comma_is_valid = true;
			if (bracket_ptr == 1) arg_count++; // open bracket for the function
			token = self->tokens->list[self->pos + i ];
			if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected character", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);  // func(arg , )
		}
		else if (token->type == TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		else if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
		else if (structToken_isAssignmentOperator(token)) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	}
}

// semicollon or other end type not included in expr
// after the scane position points to exprend_
struct CarbonError* structAst_scaneExpr(struct Ast* self, struct Expression* expr, enum structAst_ExprEndType end_type){
	//struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	int bracket_ptr = 0; // if open bracket increase, close bracket degrease
	int curbracket_ptr = 0, sqbracket_ptr=0;
	while(true){
		
		struct Token* token = self->tokens->list[(self->pos)];
		if (token->type == TK_BRACKET_LPARAN) bracket_ptr++;
		else if (token->type == TK_BRACKET_LCUR) curbracket_ptr++;
		else if (token->type == TK_BRACKET_LSQ) sqbracket_ptr++;
		else if (token->type == TK_BRACKET_RPARAN) {
			bracket_ptr--;
			if ((end_type == EXPREND_RPRAN || end_type == EXPREND_COMMA_OR_RPRAN) && bracket_ptr == -1) { expr->end_pos = self->pos - 1; break; } // open bracket already ommited
			if (bracket_ptr < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RCUR) {
			curbracket_ptr--;
			if (curbracket_ptr <0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RSQ) {
			sqbracket_ptr--;
			if (sqbracket_ptr<0)return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		if (token->group == TKG_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
		else if ( (end_type !=EXPREND_SEMICOLLON) && (token->type == TK_SYM_SEMI_COLLON) ) 
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);

		if (end_type == EXPREND_SEMICOLLON && token->type == TK_SYM_SEMI_COLLON){ 
			if (bracket_ptr != 0 || curbracket_ptr != 0 || sqbracket_ptr != 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, self->tokens->list[expr->begin_pos]->pos, self->src->buffer, self->file_name, false, token->pos- self->tokens->list[expr->begin_pos]->pos);
			expr->end_pos = self->pos-1; break;
		}
		else if (end_type == EXPREND_COMMA && token->type == TK_SYM_COMMA && bracket_ptr==0){ expr->end_pos = self->pos-1; break;}
		// else if (end_type == EXPREND_RPRAN && token->type == TK_BRACKET_RPARAN && bracket_ptr == 0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_ASSIGN && structToken_isAssignmentOperator(token) && bracket_ptr == 0 ){ expr->end_pos = self->pos-1; break; }
		else if (end_type == EXPREND_COMMA_OR_RPRAN &&  (token->type == TK_BRACKET_RPARAN ) || (token->type == TK_SYM_COMMA) && bracket_ptr == 0 ) {
			expr->end_pos = self->pos-1; break;
		}

		// token = "." check if expr is method or field
		if (token->type == TK_SYM_DOT){

			// dot cant be at the begining
			if (self->pos == expr->begin_pos || self->pos == 0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 1);
			struct Token* before = self->tokens->list[self->pos - 1]; if (before->group == TKG_PASS) before = self->tokens->list[self->pos - 2];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if ( ! (before->group == TKG_VARIABLE || structToken_isCloseBracket(before) || before->type == TK_KWORD_SELF ) ) // TODO: this logic may fails
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_IDENTIFIER ) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
			if ( (self->tokens->list[self->pos+1])->type == TK_BRACKET_LPARAN ){
				token->group = TKG_FUNCTION; token->func_is_method  = true;
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), NULL); if (err->type != ERROR_SUCCESS){ return err; }
			}
			else { token->idf_is_field = true; /* not conform */ }
		}

		// token = idf check if expr is function
		if (token->group == TKG_IDENTIFIER || token->group == TKG_FUNCTION){ // line by line interp, already change to tkg_function
			if ( (self->tokens->list[self->pos+1])->type == TK_BRACKET_LPARAN ){
				token->group = TKG_FUNCTION; // not method
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), NULL); if (err->type != ERROR_SUCCESS) return err;
			}
			else  token->group = TKG_VARIABLE;
			
		}

		// if builtin count arg count and assert
		if (token->group == TKG_BUILTIN){
			size_t call_end_pos = token->pos;
			struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), &call_end_pos); if (err->type != ERROR_SUCCESS){ return err; }
			if (token->func_args_given != token->func_args_count){
				char* err_msg = (char*)malloc(ERROR_LINE_SIZE);
				snprintf(err_msg, ERROR_LINE_SIZE, "TypeError: func:%s takes %i arguments (%i given)", token->name, token->func_args_count, token->func_args_given);
				return utils_make_error(err_msg, ERROR_TYPE, token->pos, self->src->buffer, self->file_name, true, call_end_pos - token->pos + 1);
			}
		}

		// if token == '+' or '-' check it is a single operator or binary
		if (token->type == TK_OP_MINUS || token->type == TK_OP_PLUS){
			struct Token* before = self->tokens->list[self->pos-1]; if (before->group == TKG_PASS) before = self->tokens->list[self->pos-2]; // skip TK_PASS
			if (token->pos == expr->begin_pos)				token->op_is_single = true; // begining minus is single op
			else if ( before->group == TKG_OPERATOR )		token->op_is_single = true; // before minus is operator
			else if ( before->type == TK_BRACKET_LPARAN )	token->op_is_single = true;	// before (
			else if ( before->type == TK_BRACKET_RSQ)		token->op_is_single = true;	// before [
			else if ( before->type == TK_SYM_COMMA)			token->op_is_single = true;	// before ,
		}

		// if token == ++ or -- check it is a pre or post
		if (token->type == TK_OP_INCR || token->type == TK_OP_DECR) {
			if (self->pos == 0 || self->pos == expr->begin_pos) token->op_is_pre = true; // very first token
			else {
				// errors ***************************************
				struct Token* before = self->tokens->list[self->pos - 1]; if (before->type == TK_PASS) before = self->tokens->list[self->pos - 2];
				struct Token* next   = self->tokens->list[self->pos + 1]; if (before->type == TK_PASS) next = self->tokens->list[self->pos + 2];
				if (before->group == TKG_OPERATOR && next->group == TKG_OPERATOR) // before and after are operator
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// (++) or (--)
				if (structToken_isCloseBracket(before) && structToken_isOpenBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				if (structToken_isOpenBracket(before) && structToken_isCloseBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// idf ++ idf | ; ++ ; handled by ast_makeTree
				if (before->group == TKG_IDENTIFIER && next->group ==  TKG_IDENTIFIER)
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// ***********************************************

				// for post | default is post
				if (before->group == TKG_IDENTIFIER || structToken_isCloseBracket(before) || 
					next->group == TKG_OPERATOR ||next->type == TK_SYM_COMMA || next->type == TK_SYM_SEMI_COLLON
				) 
					token->op_is_pre = false;

				// for pre
				else if (before->group == TKG_OPERATOR || structToken_isOpenBracket(before) ||
					before->type == TK_SYM_COMMA || next->group == TKG_IDENTIFIER || before->type == TK_SYM_SEMI_COLLON
					) 
					token->op_is_pre = true;
			}
		}

		/**************** INVALID SYNTAX ***********************/
		// expr cant contain an assignment operator
		if (structToken_isAssignmentOperator(token)) {
			return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		// illegal symbols in an expr // TODO: add '\' symbol
		if (token->type == TK_SYM_COLLON || token->type == TK_SYM_AT || token->type == TK_SYM_HASH || token->type == TK_SYM_DOLLAR) {
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		// invalid comma -> expr = 2,2
		if (token->type == TK_SYM_COMMA && !token->comma_is_valid)
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		// illegal keywords in an expr
		if (token->group == TKG_KEYWORD) {
			if (
				token->type == TK_KWORD_IF			||
				token->type == TK_KWORD_ELSE		||
				token->type == TK_KWORD_WHILE		||
				token->type == TK_KWORD_FOR			||
				token->type == TK_KWORD_FOREACH		||
				token->type == TK_KWORD_BREAK		||
				token->type == TK_KWORD_CONTINUE	||
				token->type == TK_KWORD_RETURN		||
				token->type == TK_KWORD_STATIC		||
				token->type == TK_KWORD_FUNCTION	||
				token->type == TK_KWORD_CLASS		||
				token->type == TK_KWORD_IMPORT
				)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}

		// after close bracket illegal tokens
		if (token->type == TK_BRACKET_RPARAN) {
			struct Token* next = self->tokens->list[self->pos + 1];
			if (next->group == TKG_IDENTIFIER || next->group == TKG_BUILTIN || next->group == TKG_FUNCTION|| next->group == TKG_NUMBER ) 
				return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);
		}

		// if number and next is '(' : numbers anen't callable error
		if (token->group == TKG_NUMBER){
			if (self->tokens->list[self->pos+1]->type == TK_BRACKET_LPARAN)
				return utils_make_error("TypeError: numbers aren't callable", ERROR_TYPE, self->tokens->list[self->pos+1]->pos, self->src->buffer, self->file_name, false, 1);
		}
		// two numbers can't come next to each other
		if (token->group == TKG_NUMBER && self->tokens->list[self->pos+1]->group == TKG_NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src->buffer, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr );

		// binary operator
		if (structToken_isBinaryOperator(token)) {

			// binary operator can't be at the beggining of an expr, after an open bracket
			if (self->pos == expr->begin_pos) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if ( structToken_isOpenBracket(self->tokens->list[self->pos-1]) ) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			
			// two binary operators can't be next to each other 
			struct Token* next = self->tokens->list[self->pos + 1]; if (next->type == TK_PASS) next = self->tokens->list[self->pos + 2];
			if (structToken_isBinaryOperator(next))
				return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);
		}

		// illegal before ending ) and ;
		struct Token* tk_next = self->tokens->list[self->pos + 1]; if (tk_next->group == TKG_PASS) tk_next = self->tokens->list[self->pos + 2];
		if (tk_next->type == TK_SYM_SEMI_COLLON || structToken_isCloseBracket(tk_next)) {
			if (structToken_isBinaryOperator(token))  return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type == TK_OP_PLUS || token->type == TK_OP_MINUS ||
				token->type == TK_SYM_COMMA || token->type == TK_SYM_COLLON ||
				token->type == TK_SYM_AT || token->type == TK_SYM_HASH ||
				token->type == TK_SYM_DOLLAR || token->type == TK_SYM_DILDO
			) 
				return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		/********************************************************/


		(self->pos)++;
	}
	return structCarbonError_new();
}

struct CarbonError* structAst_isNextStmnAssign(struct Ast* self, bool* ret, size_t* assign_op_pos){
	struct CarbonError* err = structCarbonError_new();
	int i = 0;
	int pbrcket = 0, curbracket=0, sqbracket = 0;
	while(true){
		struct Token* token = self->tokens->list[ self->pos + i ];

		// bracket mismatch
		if (token->type == TK_BRACKET_LPARAN) pbrcket++;
		else if (token->type == TK_BRACKET_LCUR) curbracket++;
		else if (token->type == TK_BRACKET_LSQ) sqbracket++;

		else if (token->type == TK_BRACKET_RPARAN) {
			pbrcket--;
			if (pbrcket < 0) return utils_make_error("SyntaxError: brackets mismatch (did you forget semicollon?)", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RCUR) {
			curbracket--;
			if (curbracket < 0) return utils_make_error("SyntaxError: brackets mismatch (did you forget semicollon?)", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RSQ) {
			sqbracket--;
			if (sqbracket< 0) return utils_make_error("SyntaxError: brackets mismatch (did you forget semicollon?)", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		if (token->group == TKG_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1); 
		if (token->type == TK_SYM_SEMI_COLLON){ 
			if (pbrcket!=0 || curbracket!=0 || sqbracket!=0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, self->tokens->list[self->pos]->pos, self->src->buffer, self->file_name, false, token->pos - self->tokens->list[self->pos]->pos);
			*ret = false; return err; 
		}
		if (structToken_isAssignmentOperator(token)){ 
			if (assign_op_pos != NULL) *assign_op_pos = token->pos;
			*ret = true; return err;
		}
		i++;
	}
	return err;
}

// now self->pos is ';' or '}' and won't change the pos
bool structAst_isNextElseIf(struct Ast* self) {
	struct Token* next = self->tokens->list[self->pos + 1];
	if (next->group == TKG_EOF) return false;
	struct Token* after_that = self->tokens->list[self->pos + 2];
	if (next->type == TK_KWORD_ELSE && after_that->type == TK_KWORD_IF) return true;
	return false;
}

bool structAst_isStmnLoop( struct Statement* stmn) { // static method
	if (stmn == NULL) return false;
	if (
		stmn->type == STMNT_WHILE	||
		stmn->type == STMNT_FOR		||
		stmn->type == STMNT_FOREACH
		) return true;
	return false;
}


// after scan pos = idf
struct CarbonError* structAst_scaneDtype(struct Ast* self, struct ExprDtype** ret){

	struct Token* token = self->tokens->list[self->pos];

	if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	if ( (token->group != TKG_DTYPE) && (token->group != TKG_IDENTIFIER) /*token may be a class name*/ ) return utils_make_error("SyntexError: expected a data type", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); 
	*ret =  structExprDtype_new(token);

	// for map<dtype, dtype> list<dtype> 
	if (token->type == TK_DT_LIST){
		(*ret)->is_list = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) { token->type = TK_BRACKET_LTRI; token->group = TKG_BRACKET; }
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_LTRI;
		++self->pos; 
		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if (err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;

		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos+1])->group == TKG_PASS ){
			token->name[1] = '\0'; // now token is '>'
			token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
			struct Token* tk_pass = self->tokens->list[self->pos + 1];
			structToken_addChar(tk_pass, '>'); tk_pass->group = TKG_BRACKET; self->tokens->list[self->pos + 1]->type = TK_BRACKET_RTRI;
		}
		else if (token->type != TK_BRACKET_RTRI) {
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			return utils_make_error("SyntexError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
	}
	else if (token->type == TK_DT_MAP){
		(*ret)->is_map = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) { token->type = TK_BRACKET_LTRI; token->group = TKG_BRACKET; }
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
		token = self->tokens->list[++self->pos];
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->group != TKG_DTYPE) return utils_make_error("SyntaxError: expected a data type", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_LIST ) return utils_make_error("TypeError: list objects can't be a key", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_MAP ) return utils_make_error("TypeError: map objects can't be a key", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		(*ret)->key = token;
		token = self->tokens->list[++self->pos];
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_SYM_COMMA ) return utils_make_error("SyntaxError: exprcted symbol ','", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token = self->tokens->list[++self->pos];

		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if(err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;
		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos+1])->group == TKG_PASS ){
			token->name[1] = '\0'; // now token is '>'
			struct Token* tk_pass = self->tokens->list[self->pos + 1];
			structToken_addChar(tk_pass, '>'); tk_pass->group = TKG_BRACKET; self->tokens->list[self->pos + 1]->type = TK_BRACKET_RTRI;
		}
		else if (token->type != TK_BRACKET_RTRI) {
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			return utils_make_error("SyntaxError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
	}

	++self->pos;
	return structCarbonError_new(); //return ret;
}

enum VarIniEndType
{
	VARINIEND_NORMAL,  // semicollon, can contain =
	VARINIEND_FOREACH, // '=' not allowed <dtype> <idf> ;
	VARINIEND_FUNC,    // '=' not allowed for now (TODO:), end ')' or ','
};

struct CarbonError* structAst_getAssignStatement(struct Ast* self, struct Statement* stmn) {
	stmn->statement.assign.idf = structExpression_new(self->tokens);
	struct CarbonError* err = structAst_scaneExpr(self, stmn->statement.assign.idf, EXPREND_ASSIGN);  if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
	struct Token* token = self->tokens->list[self->pos++];
	if (!structToken_isAssignmentOperator(token)) utils_error_exit("InternalError: expected an assignment operator", token->pos, self->src->buffer, self->file_name); // COMPLIER INTERNAL ERROR EXIT(1)
	stmn->statement.assign.op = token;
	stmn->statement.assign.expr = structExpression_new(self->tokens);
	err = structAst_scaneExpr(self, stmn->statement.assign.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);

	return structCarbonError_new();
}

// after end pos = semicollon or other end type
struct CarbonError* structAst_getVarInitStatement(struct Ast* self, struct Statement* stmn, enum VarIniEndType end_type) { // foreach( var_ini_only; expr_itter ){}
	struct CarbonError* err = structAst_scaneDtype(self, &(stmn->statement.init.dtype)); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
	struct Token* token = self->tokens->list[self->pos];
	if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	if (token->group != TKG_IDENTIFIER) { return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
	stmn->statement.init.idf = token;
	token = self->tokens->list[++self->pos];
	if (end_type == VARINIEND_NORMAL){
		if (token->type == TK_SYM_SEMI_COLLON) {

		}
		else { // scane expr
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_OP_EQ) { return utils_make_error("SyntaxError: expected symbol '=' or ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
			++self->pos;
			struct Expression* expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			stmn->statement.init.expr = expr;
		}
	}
	else if (end_type == VARINIEND_FOREACH) {
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_SYM_SEMI_COLLON)  return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

	}
	else if (end_type == VARINIEND_FUNC) {
		if (token->type == TK_SYM_COMMA || token->type == TK_BRACKET_RPARAN) { // TODO: END WITH '='

		}
		else { // scane expr
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_OP_EQ) { return utils_make_error("SyntaxError: expected symbol '=' or ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
			++self->pos;
			struct Expression* expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr, EXPREND_COMMA_OR_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			stmn->statement.init.expr = expr;
		}

		//if (!(token->type == TK_SYM_COMMA || token->type == TK_BRACKET_RPARAN)) 
		//	return utils_make_error("SyntaxError: expected symbol ',' or ')'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	}
	
	
	return structCarbonError_new();
}

struct CarbonError* structAst_getStmnListBody(struct Ast* self, int indent, struct StatementList** body_ptr, bool cur_bracket_must, struct Statement* parent_of_stmn_list) { // for func defn and try ... cur bracket is must
	struct Token* token = self->tokens->list[self->pos];
	if (token->type == TK_SYM_SEMI_COLLON) { /* while(cond); */
		if (cur_bracket_must) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	}
	else if (token->type == TK_BRACKET_LCUR) {
		token = self->tokens->list[++self->pos];
		if (token->type == TK_BRACKET_RCUR) { /* while(cond){} */

		}
		else {
			struct StatementList* stmn_list = structStatementList_new(parent_of_stmn_list); stmn_list->indent = indent;
			*body_ptr = stmn_list;
			struct CarbonError* err = structAst_makeTree(self, *body_ptr, STMNEND_BRACKET_RCUR);
			if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}
	}
	else { // single statement
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (cur_bracket_must) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		struct StatementList* stmn_list = structStatementList_new(parent_of_stmn_list); stmn_list->indent = indent;
		*body_ptr = stmn_list;
		struct Expression* expr = structExpression_new(self->tokens);

		// single statement may expr(stmn unknown), break, continue, return   other are invalid
		if (token->type == TK_KWORD_BREAK) {
			struct Statement* par = parent_of_stmn_list->parent;
			while (true) {
				if (par == NULL) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (structAst_isStmnLoop(par)) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			struct Statement* stmn_body = structStatement_new(STMNT_BREAK, parent_of_stmn_list);
			structStatementList_addStatement(*body_ptr, stmn_body);
			
		}
		else if (token->type == TK_KWORD_CONTINUE) {
			struct Statement* par = parent_of_stmn_list->parent;
			while (true) {
				if (par == NULL) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (structAst_isStmnLoop(par)) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			struct Statement* stmn_body = structStatement_new(STMNT_CONTINUE, parent_of_stmn_list);
			structStatementList_addStatement(*body_ptr, stmn_body);
		}
		else if (token->type == TK_KWORD_RETURN) {
			struct Statement* par = parent_of_stmn_list->parent;
			while (true) {
				if (par == NULL)  return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (par->type == STMNT_FUNC_DEFN) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			struct Statement* stmn_body = structStatement_new(STMNT_RETURN, parent_of_stmn_list);

			if (token->type == TK_SYM_SEMI_COLLON) {
				// if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				// return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			} else {
					stmn_body->statement.stmn_return.expr = structExpression_new(self->tokens);
					struct CarbonError* err = structAst_scaneExpr(self, stmn_body->statement.stmn_return.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
					structStatementList_addStatement(*body_ptr, stmn_body);
			}
		}
		
		else {
			bool is_next_assign; struct CarbonError* err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (is_next_assign) {
				struct Statement* stmn = structStatementList_createStatement(*body_ptr, STMNT_ASSIGN, parent_of_stmn_list);
				err = structAst_getAssignStatement(self, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}
			else {
				struct CarbonError* err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				struct Statement* stmn_body = structStatement_new(STMNT_UNKNOWN, parent_of_stmn_list); stmn_body->statement.unknown.expr = expr;
				structStatementList_addStatement(*body_ptr, stmn_body);
			}
		}
	}
	return structCarbonError_new();
}

// public
void structAst_init(struct Ast* self, struct String* src, char* file_name){
	self->src 			= src;
	self->file_name 	= file_name;
	self->pos 			= 0;
	self->tokens 		= structTokenList_new();
	self->token_scanner = structTokenScanner_new(src, file_name);
	self->stmn_list 	= structStatementList_new(NULL);
}

struct CarbonError* structAst_scaneTokens(struct Ast* self){
	bool eof = false;
	while (!eof){
		struct Token* tk = structTokenList_createToken(self->tokens);
		structTokenScanner_setToken(self->token_scanner, tk);
		struct CarbonError* err = structTokenScanner_scaneToken(self->token_scanner, &eof); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		if (eof) { 
			tk->group = TKG_EOF; tk->type = TK_EOF;  structToken_addChar(tk, ' ');
		}
		if ( tk->type == TK_OP_RSHIFT){ // if tk == >> add pass to make it > > for close bracket : list<list<char>>
			struct Token* pass_tk = structTokenList_createToken(self->tokens); pass_tk->pos = tk->pos+1;
			pass_tk->group = TKG_PASS; pass_tk->type = TK_PASS;
		}
	}
	// set tk_eof position = last token
	if (self->tokens->count > 1) {
		struct Token* tk_eof  = self->tokens->list[self->tokens->count - 1];
		struct Token* tk_last = self->tokens->list[self->tokens->count - 2];
		tk_eof->pos = tk_last->pos + tk_last->_name_ptr;
	}
	return structCarbonError_new();
}

void structAst_deleteLastStatement(struct Ast* self) {
	structStatementList_deleteLast(self->stmn_list);
}

/*
makeTree terminates for tk_eof, and rcur_bracket
*/
struct CarbonError* structAst_makeTree(struct Ast* self, struct StatementList* statement_list, enum structAst_StmnEndType end_type) {
	struct CarbonError* err;

	while ( true ) {
		
		struct Token* token = self->tokens->list[self->pos];
		// scane terminations
		if (self->pos == self->tokens->count -1){
			if (token->group != TKG_EOF) { printf("InternalError: expected token TK_EOF\nfound: "); structToken_print(token); exit(-1); }
			if (end_type != STMNEND_EOF) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
			return structCarbonError_new();
		}
		if (token->type == TK_BRACKET_RCUR && end_type == STMNEND_BRACKET_RCUR){
			return structCarbonError_new();
		}

		/******************************************************************/
		struct Statement* parent = statement_list->parent;

		if (token->group == TKG_COMMENT); // do nothing
		else if (token->type == TK_SYM_SEMI_COLLON); // do nothing
		else if (token->group == TKG_PASS); // do nothing

		// datatype init
		else if (token->group == TKG_DTYPE){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_VAR_INI, parent);
			err = structAst_getVarInitStatement(self, stmn, VARINIEND_NORMAL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// identifier
		else if (token->group == TKG_IDENTIFIER || token->group == TKG_FUNCTION || token->group == TKG_VARIABLE || token->group == TKG_BUILTIN || token->type == TK_KWORD_SELF){ // could be variable, function, 
			// assignment statement
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (is_next_assign){
				struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_ASSIGN, parent);
				err = structAst_getAssignStatement(self, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			} else { 
				struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_UNKNOWN, parent);
				stmn->statement.unknown.expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}
		}

		// number or string
		else if ( token->group == TKG_NUMBER || token->group == TKG_STRING ){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_UNKNOWN, parent);
			int err_pos = 0; 
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (is_next_assign) return utils_make_error("TypeError: can't assign to literls", ERROR_TYPE, err_pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// TODO: open curly bracket may be a list

		// SINGLE operators
		else if (token->group == TKG_OPERATOR && !structToken_isBinaryOperator(token)) {
			struct Token* next = self->tokens->list[self->pos + 1];
			if (next->type == TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_UNKNOWN, parent);
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// open bracket (
		else if (token->type == TK_BRACKET_LPARAN){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_UNKNOWN, parent);
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// import
		else if ( token->type == TK_KWORD_IMPORT){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_IMPORT, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->group != TKG_STRING)return utils_make_error("SyntaxError: expected import path string", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			// TODO: validate token path
			stmn->statement.import.path = token;
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_SYM_SEMI_COLLON )return utils_make_error("SyntaxError: expected a semicollon", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}

		// while
		else if (token->type == TK_KWORD_WHILE){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_WHILE, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			
			struct Expression* expr_bool = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr_bool, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			stmn->statement.stm_while.expr_bool = expr_bool;
			self->pos++;

			// add body
			err = structAst_getStmnListBody(self, statement_list->indent+1, &(stmn->statement.stm_while.stmn_list), false, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			//if (stmn->statement.stm_while.stmn_list != NULL) stmn->statement.stm_while.stmn_list->parent = stmn;
		}

		// for loop
		else if (token->type == TK_KWORD_FOR) {
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_FOR, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			if (token->type == TK_SYM_SEMI_COLLON) {
				// no stmn_ini
			} else {
				struct Statement* stmn_ini = structStatement_new(STMNT_VAR_INI, NULL);
				err = structAst_getVarInitStatement(self, stmn_ini, VARINIEND_NORMAL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_for.stmn_ini = stmn_ini;
			}

			token = self->tokens->list[++self->pos];
			if (token->type == TK_SYM_SEMI_COLLON) {
				// no bool expr
			} else {
				struct Expression* expr_bool = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr_bool, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_for.expr_bool = expr_bool;
			}

			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RPARAN) {
				// no expr_end
			} else {
				struct Expression* expr_end = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr_end, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_for.expr_end = expr_end;
			}
			self->pos++;
			
			// add body
			err = structAst_getStmnListBody(self, statement_list->indent+1, &(stmn->statement.stm_for.stmn_list), false, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// foreach loop
		else if (token->type == TK_KWORD_FOREACH) {
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_FOREACH, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			if (token->type == TK_SYM_SEMI_COLLON) {
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			}
			else {
				struct Statement* stmn_ini = structStatement_new(STMNT_VAR_INI, NULL);
				err = structAst_getVarInitStatement(self, stmn_ini, VARINIEND_FOREACH); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_foreach.stmn_ini = stmn_ini;
			}

			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RPARAN) {
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			}
			else {
				struct Expression* expr_itter = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr_itter, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_foreach.expr_itter = expr_itter;
			}
			self->pos++;
			// add body
			err = structAst_getStmnListBody(self, statement_list->indent + 1, &(stmn->statement.stm_foreach.stmn_list), false, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		}

		// if statement
		else if (token->type == TK_KWORD_IF) {
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_IF, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			struct Expression* expr_bool = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr_bool, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			stmn->statement.stm_if.expr_bool = expr_bool;
			self->pos++;

			// body for if
			err = structAst_getStmnListBody(self, statement_list->indent +1, &(stmn->statement.stm_if.stmn_list), false, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);

			if (structAst_isNextElseIf(self)) {
				stmn->statement.stm_if.else_if_list = structStatementList_new(stmn);
				stmn->statement.stm_if.else_if_list->indent = statement_list->indent; // for printing if, else if are same indent
			}
			while (structAst_isNextElseIf(self)) {
				token = self->tokens->list[(self->pos += 3)]; // pos should be '('
				if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
				if (token->type != TK_BRACKET_LPARAN) return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				struct Expression* expr_bool = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr_bool, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);

				struct Statement* stmn_else_if = structStatementList_createStatement(stmn->statement.stm_if.else_if_list, STMNT_ELSE_IF, parent);
				stmn_else_if->statement.stmn_else_if.expr_bool = expr_bool;

				// else if body
				self->pos++;
				err = structAst_getStmnListBody(self, stmn->statement.stm_if.else_if_list->indent+1, &(stmn_else_if->statement.stmn_else_if.stmn_list), false, stmn_else_if); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}
			
			struct Token* next = self->tokens->list[self->pos + 1];
			if (next->type == TK_KWORD_ELSE) {
				self->pos += 2;
				err = structAst_getStmnListBody(self, statement_list->indent+1, &(stmn->statement.stm_if.stmn_list_else), false, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}

			
		}

		
		else if (token->type == TK_KWORD_FUNCTION) {
			
			// TODO: check parent is null or inside a class
			//if (parent != NULL) return utils_make_error("SyntaxError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_FUNC_DEFN, parent);

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->group != TKG_IDENTIFIER) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			stmn->statement.func_defn.idf = token;

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LPARAN) return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			token = self->tokens->list[++self->pos];

			if (token->type == TK_BRACKET_RPARAN) { // 0 args

			} else {
				if (token->type == TK_SYM_COMMA) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				stmn->statement.func_defn.args = structStatementList_new(NULL); stmn->statement.func_defn.args->indent = statement_list->indent + 1;
				bool default_arg_begined = false; // f(int x = 0, int y){} // illegal 
				while (true) {
					struct Statement* stmn_ini = structStatementList_createStatement(stmn->statement.func_defn.args, STMNT_VAR_INI, NULL);
					err = structAst_getVarInitStatement(self, stmn_ini, VARINIEND_FUNC);  if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
					if (stmn_ini->statement.init.expr != NULL) default_arg_begined = true;
					else if (default_arg_begined) return utils_make_error("SyntaxError: non default argument comes before default arguments", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr );
					token = self->tokens->list[self->pos]; 
					if(token->type == TK_BRACKET_RPARAN) break;
					token = self->tokens->list[++self->pos]; 
					if (token->type == TK_BRACKET_RPARAN) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				}
				// 2 args cant have same name
				for (size_t i = 0; i < stmn->statement.func_defn.args->count-1; i++) {
					for (size_t j = i + 1; j < stmn->statement.func_defn.args->count; j++) {
						struct Statement* next = stmn->statement.func_defn.args->list[j];
						if (strcmp(stmn->statement.func_defn.args->list[i]->statement.init.idf->name, next->statement.init.idf->name) == 0) {
							return utils_make_error("SyntaxError: duplicate argument names in function definition", ERROR_SYNTAX, next->statement.init.idf->pos, self->src->buffer, self->file_name, false, next->statement.init.idf->_name_ptr);
						}
					}
				}
				
			}
			// now at ')'

			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (!(token->type == TK_SYM_COLLON || token->type == TK_BRACKET_LCUR)) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type == TK_SYM_COLLON) {
				self->pos++;
				err = structAst_scaneDtype(self, &(stmn->statement.func_defn.ret_type)); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				token = self->tokens->list[self->pos];
			}
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_BRACKET_LCUR) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			
			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RCUR) {
				// no func body
			} else {
				self->pos--; // at '{'
				err = structAst_getStmnListBody(self, statement_list->indent + 1, &(stmn->statement.func_defn.stmn_list), true, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}
			
		}

		// break
		else if (token->type == TK_KWORD_BREAK) {
			struct Statement* par = parent;
			while (true) {
				if (par == NULL) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (structAst_isStmnLoop(par)) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_SYM_SEMI_COLLON) { return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_BREAK, parent);
		}
		// continue
		else if (token->type == TK_KWORD_CONTINUE) {
			struct Statement* par = parent;
			while (true) {
				if (par == NULL) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (structAst_isStmnLoop(par)) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_SYM_SEMI_COLLON) { return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_CONTINUE, parent);
		}
		// return 
		else if (token->type == TK_KWORD_RETURN) {
			struct Statement* par = parent;
			while (true) {
				if (par == NULL)  return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (par->type == STMNT_FUNC_DEFN) break;
				par = par->parent;
			}
			token = self->tokens->list[++self->pos];
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_RETURN, parent);
			if (token->type == TK_SYM_SEMI_COLLON) { // return;
				//return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); 
			}
			else {
				stmn->statement.stmn_return.expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.stmn_return.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			}
		}

		/*** Illegal tokens at the begining of an statement **********************************************/

		// unexpected symbols
		else if (structToken_isCloseBracket(token) || 
			token->type == TK_SYM_COLLON || token->type == TK_SYM_COMMA || token->type == TK_SYM_COLLON ||
			token->type == TK_SYM_AT || token->type == TK_SYM_HASH || token->type == TK_SYM_DOLLAR || token->type == TK_SYM_DILDO
		) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		

		// binary operator cant be at the begining of a statement
		else if (structToken_isBinaryOperator(token))
			return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		

		/* check this at expr scane
		// after open bracket
		else if (structToken_isOpenBracket(token)) {
			// binary opeartor can't be
			struct Token* next = self->tokens->list[self->pos + 1];
			if(structToken_isBinaryOperator(next)) utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);
		}
		*/

		// else cant be at the begining of a statement
		else if (token->type == TK_KWORD_ELSE)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		

		
		/*************************************************/

		else {
			return utils_make_error("InternalError: unhandled token", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}

		
		// if token == static && ast state != reading class error!

		self->pos++;
	}

	// code never reaches here

}
/***************** </Ast> *************/