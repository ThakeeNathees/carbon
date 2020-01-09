#include "ast.h"

struct CarbonError* structAst_countArgs(struct Ast* self, int* count, size_t* end_pos) {

	// check -> idf ( ) : no args
	if (self->tokens->count < self->pos + 3) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->tokens->list[self->pos]->pos, self->src->buffer, self->file_name, false, 1);
	if (self->tokens->list[self->pos + 1]->type != TK_BRACKET_LPARAN) return utils_make_error("SyntaxError: expected symbol '('", ERROR_SYNTAX, self->tokens->list[self->pos + 1]->pos, self->src->buffer, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr);
	if (self->tokens->list[self->pos + 2]->type == TK_SYM_COMMA)return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->tokens->list[self->pos + 2]->pos, self->src->buffer, self->file_name, false, 1);
	struct Token* token = self->tokens->list[self->pos + 2];
	if (token->type == TK_BRACKET_RPARAN) { *count = 0; if (end_pos != NULL)*end_pos = token->pos; return structCarbonError_new(); }

	int i = 0, arg_count = 1, bracket_ptr = 0;
	bool default_arg_begin = false;
	while (true) {
		token = self->tokens->list[self->pos + i++];

		if (token->type == TK_BRACKET_LPARAN) bracket_ptr++;
		else if (token->type == TK_BRACKET_RPARAN) {
			bracket_ptr--;
			if (bracket_ptr == 0) { *count = arg_count; if (end_pos != NULL)*end_pos = token->pos; return structCarbonError_new(); }
			if (bracket_ptr < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		else if (token->type == TK_SYM_COMMA) {
			token->comma_is_valid = true;
			if (bracket_ptr == 1) arg_count++; // open bracket for the function
			token = self->tokens->list[self->pos + i];
			if (token->type == TK_BRACKET_RPARAN || token->type == TK_SYM_COMMA || token->type == TK_OP_EQ) return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);  // func(arg , )
		}
		else if (token->type == TK_SYM_SEMI_COLLON) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		else if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected EOF, expected ')'", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
		else if (structToken_isAssignmentOperator(token)) {
			if (token->type == TK_OP_EQ) {
				struct Token* next = self->tokens->list[self->pos + i];
				if (next->type == TK_SYM_COMMA)  return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
				default_arg_begin = true;
				token->eq_is_valid = true;
			}
			else return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		else if ((token->group == TKG_IDENTIFIER || token->group == TKG_NUMBER)) {
			if (default_arg_begin) {
				struct Token* next = self->tokens->list[self->pos + i];
				struct Token* before = self->tokens->list[self->pos + i - 2];
				if (before->type == TK_SYM_COMMA) {
					if (next->type != TK_OP_EQ) return utils_make_error("SyntaxError: positional argument follows keyword argument", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
				}
			}
		}
	}
}




// assign_op_pos : to get the position of the operator's position
struct CarbonError* structAst_isNextStmnAssign(struct Ast* self, bool* ret, size_t* assign_op_pos) {
	struct CarbonError* err = structCarbonError_new();
	int i = 0;
	int pbrcket = 0, curbracket = 0, sqbracket = 0;
	while (true) {
		struct Token* token = self->tokens->list[self->pos + i];

		// bracket mismatch
		if (token->type == TK_BRACKET_LPARAN) pbrcket++;
		else if (token->type == TK_BRACKET_LCUR) curbracket++;
		else if (token->type == TK_BRACKET_LSQ) sqbracket++;

		else if (token->type == TK_BRACKET_RPARAN) {
			pbrcket--;
			if (pbrcket < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RCUR) {
			curbracket--;
			if (curbracket < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RSQ) {
			sqbracket--;
			if (sqbracket < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
		if (token->type == TK_SYM_SEMI_COLLON) {
			if (pbrcket != 0 || curbracket != 0 || sqbracket != 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, self->tokens->list[self->pos]->pos, self->src->buffer, self->file_name, false, token->pos - self->tokens->list[self->pos]->pos);
			*ret = false; return err;
		}
		if (structToken_isAssignmentOperator(token) && pbrcket == 0) {
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

bool structAst_isStmnLoop(struct Statement* stmn) { // static method
	if (stmn == NULL) return false;
	if (
		stmn->type == STMNT_WHILE ||
		stmn->type == STMNT_FOR ||
		stmn->type == STMNT_FOREACH
		) return true;
	return false;
}


struct CarbonError* structAst_scaneDtype(struct Ast* self, struct ExprDtype** ret, struct StatementList* statement_list) {
	struct Token* token = self->tokens->list[self->pos];
	structNameTable_checkEntry(statement_list, token);

	if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	if ((token->group != TKG_DTYPE) && token->type != TK_CLASS && token->type != TK_GENERIC_TYPE ) { // func f() : T {} // here T is identifier TODO: set token type to generic
		if (token->type == TK_IDENTIFIER) return utils_make_error("NameError: undefined identifier", ERROR_NAME, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		return utils_make_error("SyntexError: expected a data type", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	}
	*ret = structExprDtype_new(token);

	// for map<dtype, dtype> list<dtype> 
	if (token->type == TK_DT_LIST || ( token->type == TK_CLASS && token->is_class_generic )) {
		if (token->type == TK_DT_LIST) (*ret)->is_list = true;
		else if (token->type == TK_CLASS) (*ret)->is_generic = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) { token->type = TK_BRACKET_LTRI; token->group = TKG_BRACKET; }
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_LTRI;
		++self->pos;
		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value), statement_list); if (err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;

		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos + 1])->group == TKG_PASS) {
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
	else if (token->type == TK_DT_MAP) {
		(*ret)->is_map = true;
		token = self->tokens->list[++self->pos]; if (token->type == TK_OP_LT) { token->type = TK_BRACKET_LTRI; token->group = TKG_BRACKET; }
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_BRACKET_LTRI) return utils_make_error("SyntaxError: expected bracket '<'", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token->group = TKG_BRACKET; token->type = TK_BRACKET_RTRI;
		token = self->tokens->list[++self->pos];
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->group != TKG_DTYPE) return utils_make_error("SyntaxError: expected a data type", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_LIST) return utils_make_error("TypeError: list objects can't be a key", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type == TK_DT_MAP) return utils_make_error("TypeError: map objects can't be a key", ERROR_TYPE, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		(*ret)->key = token;
		token = self->tokens->list[++self->pos];
		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		if (token->type != TK_SYM_COMMA) return utils_make_error("SyntaxError: exprcted symbol ','", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		token = self->tokens->list[++self->pos];

		struct CarbonError* err = structAst_scaneDtype(self, &((*ret)->value), statement_list); if (err->type != ERROR_SUCCESS) return err;
		token = self->tokens->list[self->pos]; if (token->type == TK_OP_GT) token->type = TK_BRACKET_RTRI;
		if (token->type == TK_OP_RSHIFT && (self->tokens->list[self->pos + 1])->group == TKG_PASS) {
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
struct CarbonError* structAst_getVarInitStatement(struct Ast* self, struct Statement* stmn, enum VarIniEndType end_type, struct StatementList* statement_list) { // foreach( var_ini_only; expr_itter ){}
	struct CarbonError* err = structAst_scaneDtype(self, &(stmn->statement.init.dtype), statement_list); if (err->type != ERROR_SUCCESS) return err; structCarbonError_free(err);
	struct Token* token = self->tokens->list[self->pos];
	if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
	if (token->group != TKG_IDENTIFIER) { return utils_make_error("SyntaxError: expected an identifier", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr); }
	stmn->statement.init.idf = token;
	token = self->tokens->list[++self->pos];
	if (end_type == VARINIEND_NORMAL) {
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
		if (token->type != TK_SYM_COLLON)  return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

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
			}
			else {
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