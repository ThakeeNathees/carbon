#include "ast.h"

// semicollon or other end type not included in expr
// after the scane position points to exprend_
struct CarbonError* structAst_scaneExpr(struct Ast* self, struct Expression* expr, enum structAst_ExprEndType end_type) {
	//struct Expression* expr = structExpression_new(self->tokens);
	expr->begin_pos = self->pos;
	int bracket_ptr = 0; // if open bracket increase, close bracket degrease
	int curbracket_ptr = 0, sqbracket_ptr = 0;
	while (true) {

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
			if (curbracket_ptr < 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		else if (token->type == TK_BRACKET_RSQ) {
			sqbracket_ptr--;
			if (sqbracket_ptr < 0)return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}

		if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, 1);
		else if ((end_type != EXPREND_SEMICOLLON) && (token->type == TK_SYM_SEMI_COLLON))
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);

		if (end_type == EXPREND_SEMICOLLON && token->type == TK_SYM_SEMI_COLLON) {
			if (bracket_ptr != 0 || curbracket_ptr != 0 || sqbracket_ptr != 0) return utils_make_error("SyntaxError: brackets mismatch", ERROR_SYNTAX, self->tokens->list[expr->begin_pos]->pos, self->src->buffer, self->file_name, false, token->pos - self->tokens->list[expr->begin_pos]->pos);
			expr->end_pos = self->pos - 1; break;
		}
		else if (end_type == EXPREND_COMMA && token->type == TK_SYM_COMMA && bracket_ptr == 0) { expr->end_pos = self->pos - 1; break; }
		// else if (end_type == EXPREND_RPRAN && token->type == TK_BRACKET_RPARAN && bracket_ptr == 0){ expr->end_pos = self->pos-1; break;}
		else if (end_type == EXPREND_ASSIGN && structToken_isAssignmentOperator(token) && bracket_ptr == 0) { expr->end_pos = self->pos - 1; break; }
		else if (end_type == EXPREND_COMMA_OR_RPRAN && (token->type == TK_BRACKET_RPARAN) || (token->type == TK_SYM_COMMA) && bracket_ptr == 0) {
			expr->end_pos = self->pos - 1; break;
		}

		// token = "." check if expr is method or field
		if (token->type == TK_SYM_DOT) {

			// dot cant be at the begining
			if (self->pos == expr->begin_pos || self->pos == 0) return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 1);
			struct Token* before = self->tokens->list[self->pos - 1]; if (before->group == TKG_PASS) before = self->tokens->list[self->pos - 2];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (!(before->type == TK_VARIABLE || structToken_isCloseBracket(before) || before->type == TK_KWORD_SELF)) // TODO: this logic may fails
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);


			token = self->tokens->list[++self->pos];
			if (token->group == TKG_EOF) return utils_make_error("EofError: unexpected eof", ERROR_UNEXP_EOF, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (token->group != TKG_IDENTIFIER && token->group != TKG_BUILTIN) // if print is a method of the class
				return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
			if ((self->tokens->list[self->pos + 1])->type == TK_BRACKET_LPARAN) {
				token->type = TK_FUNCTION; token->func_is_method = true;
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), NULL); if (err->type != ERROR_SUCCESS) { return err; }
			}
			else { token->idf_is_field = true; /* not conform */ }
		}

		// token = idf check if expr is function
		if (token->group == TKG_IDENTIFIER) {
			if ((self->tokens->list[self->pos + 1])->type == TK_BRACKET_LPARAN) {
				token->type = TK_FUNCTION; // assumption, not method
				struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), NULL); if (err->type != ERROR_SUCCESS) return err;
			}
			else  token->type = TK_VARIABLE;

		}

		// if builtin count arg count and assert
		if (token->group == TKG_BUILTIN) {
			size_t call_end_pos = token->pos;
			struct CarbonError* err = structAst_countArgs(self, &(token->func_args_given), &call_end_pos); if (err->type != ERROR_SUCCESS) { return err; }
			if (token->func_args_given < token->func_args_count_min || ((token->func_args_count_max != -1) ? token->func_args_given > token->func_args_count_max : false)) {
				char* err_msg = (char*)malloc(ERROR_LINE_SIZE);
				if (token->func_args_count_max != -1)
					snprintf(err_msg, ERROR_LINE_SIZE, "TypeError: func:%s takes min %i, max %i arguments (%i given)", token->name, token->func_args_count_min, token->func_args_count_max, token->func_args_given);
				else
					snprintf(err_msg, ERROR_LINE_SIZE, "TypeError: func:%s takes min %i, max INF arguments (%i given)", token->name, token->func_args_count_min, token->func_args_given);
				return utils_make_error(err_msg, ERROR_TYPE, token->pos, self->src->buffer, self->file_name, true, call_end_pos - token->pos + 1);
			}
		}

		// if token == '+' or '-' check it is a single operator or binary
		if (token->type == TK_OP_MINUS || token->type == TK_OP_PLUS) {
			struct Token* before = self->tokens->list[self->pos - 1]; if (before->group == TKG_PASS) before = self->tokens->list[self->pos - 2]; // skip TK_PASS
			if (token->pos == expr->begin_pos)				token->op_is_single = true; // begining minus is single op
			else if (before->group == TKG_OPERATOR)		token->op_is_single = true; // before minus is operator
			else if (before->type == TK_BRACKET_LPARAN)	token->op_is_single = true;	// before (
			else if (before->type == TK_BRACKET_RSQ)		token->op_is_single = true;	// before [
			else if (before->type == TK_SYM_COMMA)			token->op_is_single = true;	// before ,
		}

		// if token == ++ or -- check it is a pre or post
		if (token->type == TK_OP_INCR || token->type == TK_OP_DECR) {
			if (self->pos == 0 || self->pos == expr->begin_pos) token->op_is_pre = true; // very first token
			else {
				// errors ***************************************
				struct Token* before = self->tokens->list[self->pos - 1]; if (before->type == TK_PASS) before = self->tokens->list[self->pos - 2];
				struct Token* next = self->tokens->list[self->pos + 1]; if (before->type == TK_PASS) next = self->tokens->list[self->pos + 2];
				if (before->group == TKG_OPERATOR && next->group == TKG_OPERATOR) // before and after are operator
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// (++) or (--)
				if (structToken_isCloseBracket(before) && structToken_isOpenBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				if (structToken_isOpenBracket(before) && structToken_isCloseBracket(next))
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// idf ++ idf | ; ++ ; handled by ast_makeTree
				if (before->group == TKG_IDENTIFIER && next->group == TKG_IDENTIFIER)
					return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, self->pos, self->src->buffer, self->file_name, false, 2);
				// ***********************************************

				// for post | default is post
				if (before->group == TKG_IDENTIFIER || structToken_isCloseBracket(before) ||
					next->group == TKG_OPERATOR || next->type == TK_SYM_COMMA || next->type == TK_SYM_SEMI_COLLON
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
			if (token->type == TK_OP_EQ && token->eq_is_valid); // ok
			else return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		// illegal symbols in an expr // TODO: add '\' symbol
		if (token->type == TK_SYM_COLLON || token->type == TK_SYM_AT || token->type == TK_SYM_HASH || token->type == TK_SYM_DOLLAR) {
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		}
		// invalid comma -> expr = 2,2
		if (token->type == TK_SYM_COMMA && !token->comma_is_valid)
			return utils_make_error("SyntaxError: unexpected symbol", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, 1);
		// illegal keywords in an expr
		if (token->group == TKG_KEYWORD) { // self keyword is allowed
			if (
				token->type == TK_KWORD_VAR			||	
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
		// if (token->group == TKG_DTYPE)
		// 	return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

		// after close bracket illegal tokens
		if (token->type == TK_BRACKET_RPARAN) {
			struct Token* next = self->tokens->list[self->pos + 1];
			if (next->group == TKG_IDENTIFIER || next->group == TKG_NUMBER)
				return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);
		}

		// if number and next is '(' : numbers anen't callable error
		if (token->group == TKG_NUMBER) {
			if (self->tokens->list[self->pos + 1]->type == TK_BRACKET_LPARAN)
				return utils_make_error("TypeError: numbers aren't callable", ERROR_TYPE, self->tokens->list[self->pos + 1]->pos, self->src->buffer, self->file_name, false, 1);
		}

		// two numbers, idfs, num and idf can't come next to each other -> x y; 1 2; 1 x; y 2;
		if (token->group == TKG_NUMBER || token->group == TKG_IDENTIFIER) {
			struct Token* next = self->tokens->list[self->pos + 1]; if (next->type == TK_PASS) next = self->tokens->list[self->pos + 2]; // TODO: next name collision check
			if (next->group == TKG_NUMBER || next->group == TKG_IDENTIFIER)
				return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, self->tokens->list[self->pos + 1]->pos, self->src->buffer, self->file_name, false, self->tokens->list[self->pos + 1]->_name_ptr);
		}


		// binary operator
		if (structToken_isBinaryOperator(token)) {

			// binary operator can't be at the beggining of an expr, after an open bracket
			if (self->pos == expr->begin_pos) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
			if (structToken_isOpenBracket(self->tokens->list[self->pos - 1])) return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);

			// two binary operators can't be next to each other 
			struct Token* next = self->tokens->list[self->pos + 1]; if (next->type == TK_PASS) next = self->tokens->list[self->pos + 2];
			if (structToken_isBinaryOperator(next))
				return utils_make_error("SyntaxError: unexpected operator", ERROR_SYNTAX, next->pos, self->src->buffer, self->file_name, false, next->_name_ptr);
		}

		// illegal before ending ) and ;
		struct Token* tk_next = self->tokens->list[self->pos + 1]; if (tk_next->group == TKG_PASS) tk_next = self->tokens->list[self->pos + 2];
		if (tk_next->type == TK_SYM_SEMI_COLLON || structToken_isCloseBracket(tk_next)) {
			if ( // symbols, binary operator
				token->type == TK_OP_PLUS || token->type == TK_OP_MINUS ||
				token->type == TK_SYM_COMMA || token->type == TK_SYM_COLLON ||
				token->type == TK_SYM_AT || token->type == TK_SYM_HASH ||
				token->type == TK_SYM_DOLLAR || token->type == TK_SYM_DILDO ||
				token->type == TK_OP_NOT || structToken_isBinaryOperator(token) ||

				// and or not 
				token->type == TK_KWORD_OR || token->type == TK_KWORD_AND || token->type == TK_KWORD_NOT
				)
				return utils_make_error("SyntaxError: invalid syntax", ERROR_SYNTAX, token->pos, self->src->buffer, self->file_name, false, token->_name_ptr);
		}
		/********************************************************/


		(self->pos)++;
	}
	return structCarbonError_new();
}

