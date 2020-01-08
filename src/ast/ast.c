#include "ast.h"

/***************** <Ast> *************/

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

struct CarbonError* structAst_scaneClasses(struct Ast* self) {
	if (self->tokens == NULL) return utils_make_error("InternalError: scane classes called with NULL tokens", ERROR_INTERNAL, 0, self->src->buffer, self->file_name, false, 1);
	struct Token* token, *idf;
	for (size_t i = 0; i < self->tokens->count; i++) {
		token = self->tokens->list[i];
		if (token->type == TK_KWORD_CLASS) {
			idf = self->tokens->list[i + 1];
			if (idf->type != TK_IDENTIFIER) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, idf->pos, self->src->buffer, self->file_name, false, idf->_name_ptr);
			struct NameTableEntry* entry = structNameTable_createEntry(self->stmn_list->name_table, idf, IDF_CLASS_NAME, NULL);
			struct Token* next = self->tokens->list[i + 2];
			if (next->type == TK_OP_LT || next->type == TK_BRACKET_LTRI) { entry->is_class_generic = true; }
		}
	}
	return structCarbonError_new();
}

void structAst_deleteLastStatement(struct Ast* self) {
	structStatementList_deleteLast(self->stmn_list);
}

// makeTree terminates for tk_eof, and rcur_bracket
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

		// ------------- const and static
		struct Statement* parent = statement_list->parent;

		//bool is_next_const = false;  // these bools set false when used -> if true at the end of the loop = error!
		//bool is_next_static = false;
		struct Token* _const = NULL; 
		struct Token* _static = NULL;
		struct Token* _abstract = NULL;
		struct Token* _override = NULL;

		while (true) {

			if (token->type == TK_KWORD_STATIC) {
				if (_static != NULL) return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				struct Statement* par = parent;
				if (par == NULL) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (par->type != STMNT_CLASS_DEFN) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				struct Token* next = self->tokens->list[self->pos + 1];  // after next : if (next->type == TK_PASS) next = self->tokens->list[self->pos + 2];
				if (next->type == TK_KWORD_CONST) next = self->tokens->list[self->pos + 2];
				if (next->type == TK_EOF) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
				if (next->type == TK_KWORD_FUNCTION); // ok
				else if (next->group == TKG_DTYPE);  // ok
				else if (next->group == TKG_IDENTIFIER && !structToken_isBuiltin(next)) {
					// can't decide but handled below, at the end of loop
				}
				else
					return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);

				_static = token;
				token = self->tokens->list[++self->pos];
			}

			else if (token->type == TK_KWORD_CONST) {
				if (_const != NULL) return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				bool is_next_assignment = false;
				err = structAst_isNextStmnAssign(self, &is_next_assignment, NULL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				if (!is_next_assignment) return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				_const = token;
				token = self->tokens->list[++self->pos];
			}

			else if (token->type == TK_KWORD_ABSTRACT) {
				if ( 
					(_abstract != NULL) ||
					(_const != NULL)	||
					(_static != NULL)	||
					(_override != NULL)	
					)
					return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				struct Statement* par = parent;
				if (par != NULL) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				//if (par->type != STMNT_CLASS_DEFN) return utils_make_error("SyntaxError: unexpected keyword", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);


				struct Token* next = self->tokens->list[self->pos + 1];
				if (next->type != TK_KWORD_CLASS) utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				_abstract = token;
				token = self->tokens->list[++self->pos];
			}

			else if (token->type == TK_KWORD_OVERRIDE) {
				if ( 
					(_abstract != NULL) ||
					(_const != NULL)	||
					(_static != NULL)	||
					(_override != NULL)	
					)
					return utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				struct Statement* par = parent;
				if (par == NULL) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (par->type != STMNT_CLASS_DEFN) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				struct Token* next = self->tokens->list[self->pos + 1];
				if (next->type != TK_KWORD_FUNCTION) utils_make_error("SyntaxError: invalid syntax", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				_override = token;
				token = self->tokens->list[++self->pos];
			}

			else break;
		}
		// -------------------

		if (token->group == TKG_COMMENT); // do nothing
		else if (token->type == TK_SYM_SEMI_COLLON); // do nothing
		else if (token->group == TKG_PASS); // do nothing


		// datatype init
		else if (token->group == TKG_DTYPE){
			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_VAR_INI, parent);
			err = structAst_getVarInitStatement(self, stmn, VARINIEND_NORMAL); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (_const != NULL) {
				stmn->statement.init.idf->idf_is_const = true; // statement is assignment 
				_const = NULL; 
			}
			if (_static != NULL) {
				stmn->statement.init.idf->is_static = true;
				_static = NULL;
			}
		}

		// TODO: identifier may be a dtype, impl static, const

		// identifier
		else if (token->group == TKG_IDENTIFIER || token->type == TK_KWORD_SELF){ // could be variable, function, 
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

		// function defn
		else if (token->type == TK_KWORD_FUNCTION) {
			
			// check parent is null -> function , class -> method
			if (parent == NULL || parent->type == STMNT_CLASS_DEFN); // ok
			else return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_FUNC_DEFN, parent);


			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_IDENTIFIER) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			stmn->statement.func_defn.idf = token;
			if (parent != NULL && parent->type == STMNT_CLASS_DEFN) { stmn->statement.func_defn.idf->func_is_method = true; }
			if (_override != NULL) { stmn->statement.func_defn.idf->is_override = true; _override = NULL; } // par = class already chacked

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
			if (!(token->type == TK_SYM_COLLON || token->type == TK_BRACKET_LCUR || token->type == TK_SYM_SEMI_COLLON)) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type == TK_SYM_COLLON) {
				self->pos++;
				err = structAst_scaneDtype(self, &(stmn->statement.func_defn.ret_type)); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				token = self->tokens->list[self->pos];
			}
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			
			// not at '{' or ';' <- for abstract methods
			if (token->type == TK_SYM_SEMI_COLLON) {
				if (!parent->statement.class_defn.idf->is_abstract) 
					return utils_make_error("SyntaxError: abstract method defined on non abstract class", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				if (stmn->statement.func_defn.idf->is_static || stmn->statement.func_defn.idf->is_override)
					return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				
				stmn->statement.func_defn.idf->is_abstract = true;
			}
			else {
				if (token->type != TK_BRACKET_LCUR) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RCUR) {
					// no func body
				}
				else {
					self->pos--; // at '{'
					err = structAst_getStmnListBody(self, statement_list->indent + 1, &(stmn->statement.func_defn.stmn_list), true, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
				}

				if (_static != NULL) {
					stmn->statement.func_defn.idf->is_static = true;
					_static = NULL;
				}
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

		// class defn
		else if (token->type == TK_KWORD_CLASS) {

			if (parent != NULL) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			struct Statement* stmn = structStatementList_createStatement(statement_list, STMNT_CLASS_DEFN, parent);
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->type != TK_IDENTIFIER) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			stmn->statement.class_defn.idf = token; // class name
			if (_abstract != NULL) { stmn->statement.class_defn.idf->is_abstract = true; _abstract = NULL; }
			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			// if open tribracket has generic type
			if (token->type == TK_OP_LT) {
				token->type = TK_BRACKET_LTRI;
				token = self->tokens->list[++self->pos];
				if (token->type != TK_IDENTIFIER)
					return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				
				stmn->statement.class_defn.idf->is_class_generic = true;
				stmn->statement.class_defn.idf->generic_type = token;

				token = self->tokens->list[++self->pos];
				if (token->type != TK_OP_GT)
					return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				token->type = TK_BRACKET_RTRI;
				token = self->tokens->list[++self->pos];
			}

			// if open bracket has parents
			if (token->type == TK_BRACKET_LPARAN) {
				token = self->tokens->list[++self->pos];
				if (token->type == TK_BRACKET_RPARAN) {} // no parents
				else {
					if (token->type == TK_SYM_COMMA) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
					struct TokenList* supers = structTokenList_new();
					token = self->tokens->list[self->pos];
					while (true) {
						if (token->type != TK_IDENTIFIER)  return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
						structTokenList_addToken(supers, token);
						token = self->tokens->list[++self->pos];
						if (token->type == TK_BRACKET_RPARAN) break;
						if (token->type != TK_SYM_COMMA) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
						token = self->tokens->list[++self->pos];
					}
					stmn->statement.class_defn.supers = supers;
					// not at ')'
				}
			token = self->tokens->list[++self->pos]; 
			}

			// not at '{'
			if (token->type != TK_BRACKET_LCUR) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			// add body
			err = structAst_getStmnListBody(self, statement_list->indent + 1, &(stmn->statement.class_defn.stmn_list), true, stmn); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
			if (stmn->statement.class_defn.stmn_list != NULL) stmn->statement.class_defn.stmn_list->parent = stmn;
		}


		/*** Illegal tokens at the begining of an statement **********************************************/

		// unexpected symbols
		else if (structToken_isCloseBracket(token) || token->type == TK_BRACKET_LCUR || token->type == TK_BRACKET_LSQ ||
			token->type == TK_SYM_COLLON || token->type == TK_SYM_COMMA ||
			token->type == TK_SYM_AT || token->type == TK_SYM_HASH || token->type == TK_SYM_DOLLAR || token->type == TK_SYM_DILDO
		) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		

		// binary operator cant be at the begining of a statement
		else if (structToken_isBinaryOperator(token))
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		

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

		// static const unused
		if (_const != NULL)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, _const->pos, self->src->buffer, self->file_name, false, _const->_name_ptr);
		if (_static != NULL)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, _static->pos, self->src->buffer, self->file_name, false, _static->_name_ptr);
		if (_abstract != NULL)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, _abstract->pos, self->src->buffer, self->file_name, false, _abstract->_name_ptr);
		if (_override != NULL)
			return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, _override->pos, self->src->buffer, self->file_name, false, _override->_name_ptr);

		
		// if token == static && ast state != reading class error!

		self->pos++;
	}

	// code never reaches here

}
/***************** </Ast> *************/