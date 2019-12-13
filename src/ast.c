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
	for (size_t i=self->begin_pos; i<=self->end_pos; i++){
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
void structStatement_init(struct Statement* self){
	self->type = STMNT_UNKNOWN;
	self->statement.unknown.expr			= NULL;
	self->statement.init.expr				= NULL;
	self->statement.stm_if.stmn_list		= NULL;
	self->statement.stm_while.stmn_list		= NULL;
	
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

}
void structStatement_print(struct Statement* self, int indent){
	if (self->type == STMNT_UNKNOWN){ 
		for (int i=0; i< indent; i++)  printf("\t");printf("Stmnt : "); 
		if (self->statement.unknown.expr != NULL)structExpression_print(self->statement.unknown.expr, 0);
	}
	else if (self->type == STMNT_IMPORT){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : IMPORT| "); printf("path  : %s\n", self->statement.import.path->name );
	}
	else if (self->type == STMNT_VAR_INIT){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : VA_INI\n");
		structExprDtype_print(self->statement.init.dtype, indent+1, true);
		for (int i=0; i< indent+1; i++) printf("\t");printf("idf   : %-6s\n", self->statement.init.idf->name);
		if (self->statement.init.expr != NULL) structExpression_print(self->statement.init.expr, indent+1);
	}
	else if (self->type == STMNT_ASSIGN){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : ASSIGN\n");
		structExpression_print(self->statement.assign.idf, indent+1);
		for (int i=0; i< indent+1; i++) printf("\t");printf("op    : %-6s\n", self->statement.assign.op->name);
		structExpression_print(self->statement.assign.expr, indent+1);
	}
	else if (self->type == STMNT_BREAK){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : BREAK\n");
	}
	else if (self->type == STMNT_CONTINUE){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : CONTINUE\n");
	}
	else if (self->type == STMNT_WHILE){
		for (int i=0; i< indent; i++) printf("\t");printf("Stmnt : WHILE | "); structExpression_print(self->statement.stm_while.expr_bool, 0);
		if (self->statement.stm_while.stmn_list != NULL) structStatementList_print(self->statement.stm_while.stmn_list);

	}
	else if (self->type == STMNT_FOR) {
		for (int i = 0; i < indent; i++) printf("\t"); printf("Stmnt : FOR\n");
		if (self->statement.stm_for.stmn_ini  != NULL) { structStatement_print(self->statement.stm_for.stmn_ini, indent + 1); }
		else { for (int i = 0; i < indent + 1; i++) printf("\t"); printf("(No IniStmn)\n"); }
		if (self->statement.stm_for.expr_bool != NULL) { structExpression_print(self->statement.stm_for.expr_bool, indent + 1); }
		else { for (int i = 0; i < indent + 1; i++) printf("\t"); printf("(No ExprBool)\n"); }
		if (self->statement.stm_for.expr_end  != NULL) { structExpression_print(self->statement.stm_for.expr_end, indent + 1); }
		else { for (int i = 0; i < indent + 1; i++) printf("\t"); printf("(No ExprEnd)\n"); }
		if (self->statement.stm_for.stmn_list != NULL) { structStatementList_print(self->statement.stm_for.stmn_list); }
		else { for (int i = 0; i < indent + 1; i++) printf("\t"); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FOREACH) {
		for (int i = 0; i < indent; i++) printf("\t"); printf("Stmnt : FOREACH\n");
		structStatement_print(self->statement.stm_foreach.stmn_ini, indent + 1);
		structExpression_print(self->statement.stm_foreach.expr_itter, indent+1);
		if (self->statement.stm_foreach.stmn_list != NULL) {  structStatementList_print(self->statement.stm_foreach.stmn_list);}
		else { for (int i = 0; i < indent + 1; i++) printf("\t"); printf("(No StmnList)\n"); }
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
	self->parent = NULL;
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
		structStatement_print(self->list[i], self->indent);
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
	EXPREND_COMMA_OR_RPRAN, // for func args
	EXPREND_ASSIGN, // for assignment statement
};


struct CarbonError* structAst_countArgs(struct Ast* self, int* count, size_t* end_pos){

	// check -> idf ( ) : no args
	if (self->tokens->count < self->pos + 3 ) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_UNEXP_EOF, self->tokens->list[self->pos]->pos, self->src, self->file_name, false, 1); 
	if (self->tokens->list[self->pos+1]->type != TK_BRACKET_LPARAN) utils_error_exit("InternalError: expected bracket '('", self->tokens->list[self->pos+1]->pos, self->src, self->file_name);
	if (self->tokens->list[self->pos+2]->type == TK_SYM_COMMA )return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->tokens->list[self->pos+2]->pos, self->src, self->file_name, false, 1); 
	struct Token* token = self->tokens->list[self->pos + 2 ];
	if (token->type == TK_BRACKET_RPARAN) { *count = 0; if(end_pos!=NULL)*end_pos = token->pos; return structCarbonError_new(); }

	int i = 0, arg_count = 1, bracket_ptr = 0;
	while (true){
		token = self->tokens->list[self->pos + i++ ];

		if (token->type == TK_BRACKET_LPARAN) bracket_ptr++;
		else if (token->type == TK_BRACKET_RPARAN) {
			bracket_ptr--;
			if (bracket_ptr == 0){ *count = arg_count; if (end_pos != NULL)*end_pos = token->pos; return structCarbonError_new(); }
			if (bracket_ptr < 0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1);
		}

		else if (token->type == TK_SYM_COMMA ){ 
			token->comma_is_valid = true;
			if (bracket_ptr == 1) arg_count++; // open bracket for the function
			token = self->tokens->list[self->pos + i ];
			if (token->group == TKG_BRACKET) return utils_make_error("SyntaxError: unexpected character", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);  // func(arg , )
		}
		else if (token->type == TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
		else if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
		else if (structToken_isAssignmentOperator(token)) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
	}
}

// semicollon or other end type not included in expr, TODO: handle unexpected comma
// after the scane position points to exprend_
struct CarbonError* structAst_scaneExpr(struct Ast* self, struct Expression* expr, enum structAst_ExprEndType end_type){
	//struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	int bracket_ptr = 0; // if open bracket increase, close bracket degrease
	while(true){
		
		struct Token* token = self->tokens->list[(self->pos)];
		if (token->type == TK_BRACKET_LPARAN) bracket_ptr++;
		else if (token->type == TK_BRACKET_RPARAN){
			if (end_type == EXPREND_RPRAN && bracket_ptr == 0) { expr->end_pos = self->pos - 1; break; } // open bracket already ommited
			bracket_ptr--;
			if (bracket_ptr < 0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1);
		}

		if (token->group == TKG_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1);
		else if ( (end_type !=EXPREND_SEMICOLLON) && (token->type == TK_SYM_SEMI_COLLON) ) 
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1);

		if (end_type == EXPREND_SEMICOLLON && token->type == TK_SYM_SEMI_COLLON){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_COMMA && token->type == TK_SYM_COMMA && bracket_ptr==0){ expr->end_pos = self->pos-1; break;}
		// else if (end_type == EXPREND_RPRAN && token->type == TK_BRACKET_RPARAN && bracket_ptr == 0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_ASSIGN && structToken_isAssignmentOperator(token) ){ expr->end_pos = self->pos-1; break; }
		else if (end_type == EXPREND_COMMA_OR_RPRAN && ( (token->type == TK_BRACKET_RPARAN && bracket_ptr==0) || (token->type == TK_SYM_COMMA) ) ){
			expr->end_pos = self->pos-1; break;
		}

		// token = "." check if expr is method or field
		if (token->type == TK_SYM_DOT){

			// dot cant be at the begining
			if (self->pos == expr->begin_pos || self->pos == 0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->pos, self->src, self->file_name, false, 1);
			struct Token* before = self->tokens->list[self->pos - 1]; 
			if ( ! (before->group == TKG_FUNCTION || structToken_isCloseBracket(before)) ) // TODO: this logic may fails
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);

			token = self->tokens->list[++self->pos];
			if (token->group != TKG_IDENTIFIER ) return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
			if ( (self->tokens->list[self->pos+1])->type == TK_BRACKET_LPARAN ){
				token->group = TKG_FUNCTION; token->func_is_method  = true;
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), NULL); if (err->type != ERROR_SUCCESS){ return err; }
			}
			else { token->idf_is_field = true; /* not conform */ }
		}

		// token = idf check if expr is function
		if (token->group == TKG_IDENTIFIER){
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
				return utils_make_error(err_msg, ERROR_TYPE, token->pos, self->src, self->file_name, true, call_end_pos - token->pos + 1);
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
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src, self->file_name, false, 2);
				// (++) or (--)
				if (structToken_isCloseBracket(before) && structToken_isOpenBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src, self->file_name, false, 2);
				if (structToken_isOpenBracket(before) && structToken_isCloseBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src, self->file_name, false, 2);
				// idf ++ idf | ; ++ ; handled by ast_makeTree
				if (before->group == TKG_IDENTIFIER && next->group ==  TKG_IDENTIFIER)
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src, self->file_name, false, 2);
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
				// TODO: raise an error here and check unhandled logic
			}
		}

		/**************** INVALID SYNTAX ***********************/
		// expr cant contain an assignment operator
		if (structToken_isAssignmentOperator(token)) {
			return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}
		// illegal symbols in an expr // TODO: add \@#$ 
		if (token->type == TK_SYM_COLLON) {
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
		}
		// invalid comma -> expr = 2,2
		if (token->type == TK_SYM_COMMA && !token->comma_is_valid)
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
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
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}

		// if number and next is '(' : numbers anen't callable error
		if (token->group == TKG_NUMBER){
			if (self->tokens->list[self->pos+1]->type == TK_BRACKET_LPARAN)
				return utils_make_error("TypeError: numbers aren't callable", ERROR_TYPE, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false, 1);
		}
		// two numbers can't come next to each other
		if (token->group == TKG_NUMBER && self->tokens->list[self->pos+1]->group == TKG_NUMBER)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr );
		
		// calling a number
		if (structToken_isCloseBracket(token)) {
			if (self->tokens->list[self->pos+1]->group == TKG_NUMBER ||  self->tokens->list[self->pos + 1]->group == TKG_IDENTIFIER )
				return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos+1]->pos, self->src, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr);
		}

		// binary operator
		if (structToken_isBinaryOperator(token)) {

			// binary operator can't be at the beggining of an expr, after an open bracket
			if (self->pos == expr->begin_pos) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			if ( structToken_isOpenBracket(self->tokens->list[self->pos-1]) ) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			
			// two binary operators can't be next to each other 
			struct Token* next = self->tokens->list[self->pos + 1]; if (next->type == TK_PASS) next = self->tokens->list[self->pos + 2];
			if (structToken_isBinaryOperator(next))
				return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, next->pos, self->src, self->file_name, false, next->_name_ptr);
		}

		// illegal before ending
		struct Token* tk_next = self->tokens->list[self->pos + 1]; if (tk_next->group == TKG_PASS) tk_next = self->tokens->list[self->pos + 2];
		if (tk_next->type == TK_SYM_SEMI_COLLON || structToken_isCloseBracket(tk_next)) {
			if (structToken_isBinaryOperator(token))  return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			if (token->type == TK_OP_PLUS   || token->type == TK_OP_MINUS ||
				token->type == TK_SYM_COMMA || token->type == TK_SYM_COLLON
			) 
				return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}
		/********************************************************/


		(self->pos)++;
	}
	return structCarbonError_new();
}

struct CarbonError* structAst_isNextStmnAssign(struct Ast* self, bool* ret, size_t* assign_op_pos){
	struct CarbonError* err = structCarbonError_new();
	int i = 0;
	while(true){
		struct Token* token = self->tokens->list[ self->pos + i ];
		if (token->group == TKG_EOF ) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1); 
		if (token->type == TK_SYM_SEMI_COLLON){ *ret = false; return err; }
		if (structToken_isAssignmentOperator(token)){ 
			if (assign_op_pos != NULL) *assign_op_pos = token->pos;
			*ret = true; return err;
		}
		i++;
	}
	return err;
}

// after scan pos = idf
struct CarbonError* structAst_scaneDtype(struct Ast* self, struct ExprDtype** ret){

	struct Token* token = self->tokens->list[self->pos];

	if ( (token->group != TKG_DTYPE) && (token->group != TKG_IDENTIFIER) /*token may be a class name*/ ) return utils_make_error("SyntexError: expected a data type", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr); 
	*ret =  structExprDtype_new(token);

	// for map<dtype, dtype> list<dtype> 
	if (token->type == TK_DT_LIST){
		(*ret)->is_list = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) token->type = TK_BRACKET_LTRI;
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_LTRI;
		++self->pos; 
		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if (err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;

		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos+1])->group == TKG_PASS ){
			token->name[1] = '\0'; // now token is '>'
			token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
			(self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0'; 
			(self->tokens->list[self->pos + 1])->group = TKG_BRACKET; (self->tokens->list[self->pos + 1])->type = TK_BRACKET_RTRI;
		}
		else if (token->type != TK_BRACKET_RTRI ) return utils_make_error("SyntexError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
	}
	else if (token->type == TK_DT_MAP){
		(*ret)->is_map = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) token->type = TK_BRACKET_LTRI;
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
		token = self->tokens->list[++self->pos];
		if (token->group != TKG_DTYPE) return utils_make_error("SyntaxError: expected a data type", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_LIST ) return utils_make_error("TypeError: list objects can't be a key", ERROR_TYPE, token->pos, self->src, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_MAP ) return utils_make_error("TypeError: map objects can't be a key", ERROR_TYPE, token->pos, self->src, self->file_name, false, token->_name_ptr);
		(*ret)->key = token;
		token = self->tokens->list[++self->pos];
		if (token->type != TK_SYM_COMMA ) return utils_make_error("SyntaxError: exprcted symbol ','", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		token = self->tokens->list[++self->pos];

		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value)); if(err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;
		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos+1])->group == TKG_PASS ){
			token->name[1] = '\0'; // now token is '>'
			(self->tokens->list[self->pos+1])->group = TKG_OPERATOR; (self->tokens->list[self->pos + 1])->type = TK_BRACKET_RTRI;
			(self->tokens->list[self->pos+1])->name[0] = '>'; (self->tokens->list[self->pos+1])->name[1] = '\0';
			(self->tokens->list[self->pos + 1])->group = TKG_BRACKET; (self->tokens->list[self->pos + 1])->type = TK_BRACKET_RTRI;
		}
		else if ( token->type != TK_BRACKET_RTRI ) return utils_make_error("SyntaxError: exprcted bracket '>'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
	}

	++self->pos;
	return structCarbonError_new(); //return ret;
}

// after end pos = semicollon, don't use this for function arguments (because stmn end type = comma or ')' )
struct CarbonError* structAst_getVarInitStatement(struct Ast* self, struct Statement* stmn, bool var_ini_only) { // foreach( var_ini_only; expr_itter ){}
	stmn->type = STMNT_VAR_INIT;
	struct CarbonError* err = structAst_scaneDtype(self, &(stmn->statement.init.dtype)); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
	struct Token* token = self->tokens->list[self->pos];
	if (token->group != TKG_IDENTIFIER) { return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr); }
	stmn->statement.init.idf = token;
	token = self->tokens->list[++self->pos];
	if (token->type == TK_SYM_SEMI_COLLON) {

	}
	else { // scane expr
		if (var_ini_only) return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		if (token->type != TK_OP_EQ) { return utils_make_error("SyntaxError: expected symbol '=' or ';'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr); }
		++self->pos;
		struct Expression* expr = structExpression_new(self->tokens);
		err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		stmn->statement.init.expr = expr;
	}
	return structCarbonError_new();
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

struct CarbonError* structAst_scaneTokens(struct Ast* self){
	bool eof = false;
	while (!eof){
		struct Token* tk = structTokenList_createToken(self->tokens);
		structTokenScanner_setToken(self->token_scanner, tk);
		struct CarbonError* err = structTokenScanner_scaneToken(self->token_scanner, &eof); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
		if (eof) { 
			tk->group = TKG_EOF; tk->type = TK_EOF; 
			tk->_name_ptr = 1; tk->name[0] = 'e'; tk->name[1] = '\0'; // need _name_ptr to print error!
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
			if (end_type != STMNEND_EOF) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src, self->file_name, false, 1);
			return structCarbonError_new();
		}
		if (token->type == TK_BRACKET_RCUR && end_type == STMNEND_BRACKET_RCUR){
			return structCarbonError_new();
		}

		/******************************************************************/

		if (token->group == TKG_COMMENT); // do nothing

		// datatype init
		else if (token->group == TKG_DTYPE){
			struct Statement* stmn = structStatement_new();
			err = structAst_getVarInitStatement(self, stmn, false); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			structStatementList_addStatement(statement_list, stmn);
		}

		// identifier
		else if (token->group == TKG_IDENTIFIER || token->group == TKG_BUILTIN){ // could be variable, function, 
			// assignment statement
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (is_next_assign){
				struct Statement* stmn = structStatement_new();
				stmn->type = STMNT_ASSIGN;
				stmn->statement.assign.idf = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.assign.idf, EXPREND_ASSIGN);  if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				token = self->tokens->list[self->pos++];
				if (!structToken_isAssignmentOperator(token)) utils_error_exit("InternalError: expected an assignment operator", token->pos, self->src, self->file_name); // COMPLIER INTERNAL ERROR EXIT(1)
				stmn->statement.assign.op = token;
				stmn->statement.assign.expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.assign.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				structStatementList_addStatement(statement_list, stmn);
			} else {
				// if have globals : identifier is known or undefined, if idf is func/builtin stmn is func call // TODO: 
				struct Statement* stmn = structStatement_new();
				stmn->statement.unknown.expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				structStatementList_addStatement(statement_list, stmn);
			}
		}

		// number or string
		else if ( token->group == TKG_NUMBER || token->group == TKG_STRING ){
			struct Statement* stmn = structStatement_new();
			int err_pos = 0; 
			bool is_next_assign; err = structAst_isNextStmnAssign(self, &is_next_assign, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (is_next_assign) return utils_make_error("TypeError: can't assign to literls", ERROR_TYPE, err_pos, self->src, self->file_name, false, token->_name_ptr);
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			structStatementList_addStatement(statement_list, stmn);
		}

		// TODO: open curly bracket may be a list

		// SINGLE operators
		else if (token->group == TKG_OPERATOR && !structToken_isBinaryOperator(token)) {
			struct Token* next = self->tokens->list[self->pos + 1];
			if (next->type == TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			struct Statement* stmn = structStatement_new();
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			structStatementList_addStatement(statement_list, stmn);
		}

		// open bracket (
		else if (token->type == TK_BRACKET_LPARAN){
			++self->pos;
			struct Statement* stmn = structStatement_new();
			stmn->statement.unknown.expr = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, stmn->statement.unknown.expr, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			structStatementList_addStatement(statement_list, stmn);
		}

		// import
		else if ( token->type == TK_KWORD_IMPORT){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_IMPORT;
			token = self->tokens->list[++self->pos];
			if (token->group != TKG_STRING)return utils_make_error("SyntaxError: expected import path string", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			// TODO: validate token path
			stmn->statement.import.path = token;
			token = self->tokens->list[++self->pos];
			if (token->type != TK_SYM_SEMI_COLLON )return utils_make_error("SyntaxError: expected a semicollon", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			structStatementList_addStatement(statement_list, stmn);
		}

		// while
		else if (token->type == TK_KWORD_WHILE){
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_WHILE;
			token = self->tokens->list[++self->pos];
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			++self->pos;
			struct Expression* expr_bool = structExpression_new(self->tokens);
			err = structAst_scaneExpr(self, expr_bool, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			stmn->statement.stm_while.expr_bool = expr_bool;
			token = self->tokens->list[++self->pos];

			// if curly bracket - multiple statements else single statement ends with semi collon
			if (token->type == TK_SYM_SEMI_COLLON) {
				structStatementList_addStatement(statement_list, stmn); /* while(cond); */
			} 
			else if (token->type == TK_BRACKET_LCUR) {
				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RCUR) {
					structStatementList_addStatement(statement_list, stmn); /* while(cond){} */
				} else { 
					struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
					stmn->statement.stm_while.stmn_list = stmn_list;
					err = structAst_makeTree(self, stmn->statement.stm_while.stmn_list, STMNEND_BRACKET_RCUR);
					if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
					structStatementList_addStatement(statement_list, stmn);
				}
			}
			else{ // single statement
				struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
				stmn->statement.stm_while.stmn_list = stmn_list;
				struct Expression* expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				struct Statement* stmn_while_body = structStatement_new(); stmn_while_body->statement.unknown.expr = expr;
				structStatementList_addStatement(stmn->statement.stm_while.stmn_list, stmn_while_body);
				structStatementList_addStatement(statement_list, stmn);
			}
		}

		// break
		else if (token->type == TK_KWORD_BREAK){
			token = self->tokens->list[++self->pos];
			if (token->type != TK_SYM_SEMI_COLLON){ return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr); }
			struct Statement* stmn = structStatement_new(); stmn->type = STMNT_BREAK;
			structStatementList_addStatement(statement_list, stmn);
		}

		// continue
		else if (token->type == TK_KWORD_CONTINUE){
			token = self->tokens->list[++self->pos];
			if (token->type != TK_SYM_SEMI_COLLON){ return utils_make_error("SyntaxError: expected symbol ';'", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr); }
			struct Statement* stmn = structStatement_new(); stmn->type = STMNT_CONTINUE;
			structStatementList_addStatement(statement_list, stmn);
		}

		// for loop
		else if (token->type == TK_KWORD_FOR) {
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_FOR;
			token = self->tokens->list[++self->pos];
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);

			token = self->tokens->list[++self->pos];
			if (token->type == TK_SYM_SEMI_COLLON) {
				// no stmn_ini
			} else {
				struct Statement* stmn_ini = structStatement_new();
				err = structAst_getVarInitStatement(self, stmn_ini, false); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
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

			token = self->tokens->list[++self->pos];
			if (token->type == TK_SYM_SEMI_COLLON) {
				structStatementList_addStatement(statement_list, stmn); /* for(;;); */
			}
			else if (token->type == TK_BRACKET_LCUR) {
				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RCUR) {
					structStatementList_addStatement(statement_list, stmn); /* for(;;){} */
				}
				else {
					struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
					stmn->statement.stm_for.stmn_list = stmn_list;
					err = structAst_makeTree(self, stmn->statement.stm_for.stmn_list, STMNEND_BRACKET_RCUR);
					if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
					structStatementList_addStatement(statement_list, stmn);
				}
			}
			else { // single statement
				struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
				stmn->statement.stm_for.stmn_list = stmn_list;
				struct Expression* expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				struct Statement* stmn_for_body = structStatement_new(); stmn_for_body->statement.unknown.expr = expr;
				structStatementList_addStatement(stmn->statement.stm_for.stmn_list, stmn_for_body);
				structStatementList_addStatement(statement_list, stmn);
			}
			
		}

		// foreach loop
		else if (token->type == TK_KWORD_FOREACH) {
			struct Statement* stmn = structStatement_new();
			stmn->type = STMNT_FOREACH;
			token = self->tokens->list[++self->pos];
			if (token->type != TK_BRACKET_LPARAN)return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);

			token = self->tokens->list[++self->pos];
			if (token->type == TK_SYM_SEMI_COLLON) {
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			}
			else {
				struct Statement* stmn_ini = structStatement_new();
				err = structAst_getVarInitStatement(self, stmn_ini, true); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_foreach.stmn_ini = stmn_ini;
			}

			token = self->tokens->list[++self->pos];
			if (token->type == TK_BRACKET_RPARAN) {
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
			}
			else {
				struct Expression* expr_itter = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr_itter, EXPREND_RPRAN); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				stmn->statement.stm_foreach.expr_itter = expr_itter;
			}

			token = self->tokens->list[++self->pos];
			if (token->type == TK_SYM_SEMI_COLLON) {
				structStatementList_addStatement(statement_list, stmn); /* for(;;); */
			}
			else if (token->type == TK_BRACKET_LCUR) {
				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RCUR) {
					structStatementList_addStatement(statement_list, stmn); /* for(;;){} */
				}
				else {
					struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
					stmn->statement.stm_foreach.stmn_list = stmn_list;
					err = structAst_makeTree(self, stmn->statement.stm_foreach.stmn_list, STMNEND_BRACKET_RCUR);
					if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
					structStatementList_addStatement(statement_list, stmn);
				}
			}
			else { // single statement
				struct StatementList* stmn_list = structStatementList_new(); stmn_list->indent = statement_list->indent + 1;
				stmn->statement.stm_foreach.stmn_list = stmn_list;
				struct Expression* expr = structExpression_new(self->tokens);
				err = structAst_scaneExpr(self, expr, EXPREND_SEMICOLLON); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				struct Statement* stmn_foreach_body = structStatement_new(); stmn_foreach_body->statement.unknown.expr = expr;
				structStatementList_addStatement(stmn->statement.stm_for.stmn_list, stmn_foreach_body);
				structStatementList_addStatement(statement_list, stmn);
			}
		}


		/*** Illegal tokens at the begining of an statement **********************************************/
		// TODO: close brackets, collon, dot, binary opeartors, 

		// unexpected symbols
		else if (structToken_isCloseBracket(token) || token->type == TK_SYM_COLLON || token->type == TK_SYM_COMMA || token->type == TK_SYM_COLLON ) {
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, 1);
		}

		// binary operator cant be at the begining of a statement
		else if (structToken_isBinaryOperator(token)) {
			return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}
		// after open bracket
		else if (structToken_isOpenBracket(token)) {
			// binary opeartor can't be
			struct Token* next = self->tokens->list[self->pos + 1];
			if(structToken_isBinaryOperator(next)) utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, next->pos, self->src, self->file_name, false, next->_name_ptr);
		}

		// else cant be at the begining of a statement
		else if (token->type == TK_KWORD_ELSE) {
		return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}

		
		/*************************************************/

		else {
			return utils_make_error("InternalError: unhandled token", ERROR_SYNTAX, token->pos, self->src, self->file_name, false, token->_name_ptr);
		}

		
		// if token == static && ast state != reading class error!

		self->pos++;
	}

	// code never reaches here

}
/***************** </Ast> *************/