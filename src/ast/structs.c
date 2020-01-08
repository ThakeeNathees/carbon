#include "ast.h"


static const char* STATEMENT_TYPE_STRING[] = {
	FOREACH_STATEMENT_TYPE(GENERATE_STRING)
};

// public api
const char* enumStatementType_toString(enum StatementType self) {
	return STATEMENT_TYPE_STRING[self];
}

/***************** <Expression> *************/
// public
void structExpression_free(struct Expression* self) {
	if (self != NULL) free(self);
}
void structExpression_init(struct Expression* self, struct TokenList* token_list) {
	self->token_list = token_list;
	self->begin_pos = -1;
	self->end_pos = -1;
}
void structExpression_print(struct Expression* self, int indent, bool new_line) {
	PRINT_INDENT(indent);
	for (size_t i = self->begin_pos; i <= self->end_pos; i++) {
		if (self->token_list->list[i]->type == TK_STRING) {
			utils_print_str_without_esc(self->token_list->list[i]->name, false, true);
		}
		else printf("%s", self->token_list->list[i]->name);
	}
	if (new_line)printf("\n");
}
struct Expression* structExpression_new(struct TokenList* token_list) {
	struct Expression* expr = (struct Expression*)malloc(sizeof(struct Expression));
	structExpression_init(expr, token_list);
	return expr;
}
/***************** </Expression> *************/


/***************** <ExprDtype> *************/
void structExprDtype_free(struct ExprDtype* self) {
	if (self != NULL) free(self);
}
void structExprDtype_init(struct ExprDtype* self, struct Token* dtype) {
	self->dtype = dtype;
	self->is_map = false;
	self->is_list = false;
}
void structExprDtype_print(struct ExprDtype* self, int indent) { // no new line after the print
	//if (print_type) { PRINT_INDENT(indent); printf("<type> %s", self->dtype->name); } else
	PRINT_INDENT(indent); printf("%s", self->dtype->name);
	if (self->is_list) {
		printf("<"); structExprDtype_print(self->value, 0); printf(">");

	}
	else if (self->is_map) {
		printf("<key:%s, value:", self->key->name); structExprDtype_print(self->value, 0); printf(">");
	}
}
struct ExprDtype* structExprDtype_new(struct Token* dtype) {
	struct ExprDtype* new_dtype = (struct ExprDtype*)malloc(sizeof(struct ExprDtype));
	structExprDtype_init(new_dtype, dtype);
	return new_dtype;
}
/***************** </ExprDtype> *************/


/***************** <ExpressionList> *************/
void structExpressionList_init(struct ExpressionList* self, int growth_size, struct TokenList* token_list) {
	self->count = 0;
	self->growth_size = growth_size;
	self->size = self->growth_size;
	self->list = (struct Expression**)malloc(sizeof(struct Expression) * self->growth_size);
}
void structExpressionList_addExpression(struct ExpressionList* self, struct Expression* expr) {
	if (self->count >= self->size) {
		struct Expression** new_list = (struct Expression**)malloc(sizeof(struct Expression) * (self->size + self->growth_size));
		self->size += self->growth_size;
		for (size_t i = 0; i < self->count; i++) {
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = expr;
}
struct Expression* structExpressionList_createExpression(struct ExpressionList* self) {
	struct Expression* new_expr = structExpression_new(self->token_list);
	structExpressionList_addExpression(self, new_expr);
	return new_expr;
}
struct ExpressionList* structExpressionList_new(struct TokenList* token_list) {
	struct ExpressionList* expr_list = (struct ExpressionList*)malloc(sizeof(struct ExpressionList));
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
void structStatement_init(struct Statement* self, struct Statement* parent, enum StatementType type) {
	self->type = type;
	self->parent = parent;
	self->statement.unknown.expr = NULL;
	self->statement.init.expr = NULL;
	self->statement.stm_while.stmn_list = NULL;

	self->statement.stm_if.stmn_list = NULL;
	self->statement.stm_if.else_if_list = NULL;
	self->statement.stm_if.stmn_list_else = NULL;
	self->statement.stmn_else_if.stmn_list = NULL;

	self->statement.stm_for.stmn_ini = NULL;
	self->statement.stm_for.expr_bool = NULL;
	self->statement.stm_for.expr_end = NULL;
	self->statement.stm_for.stmn_list = NULL;

	self->statement.stm_foreach.stmn_ini = NULL;
	self->statement.stm_foreach.expr_itter = NULL;
	self->statement.stm_foreach.stmn_list = NULL;

	self->statement.func_defn.args = NULL;
	self->statement.func_defn.ret_type = NULL;
	self->statement.func_defn.stmn_list = NULL;

	self->statement.class_defn.supers = NULL;
	self->statement.class_defn.stmn_list = NULL;

	self->statement.stmn_return.expr = NULL;

}
void structStatement_print(struct Statement* self, int indent) {

	if (self->type == STMNT_UNKNOWN) {
		if (self->statement.unknown.expr != NULL)structExpression_print(self->statement.unknown.expr, indent, true);
	}
	else if (self->type == STMNT_IMPORT) {
		PRINT_INDENT(indent); printf("<import> "); printf("path  : %s\n", self->statement.import.path->name);
	}
	else if (self->type == STMNT_VAR_INI) {
		PRINT_INDENT(indent);
		printf("<ini> %s%s",
			(self->statement.init.idf->is_static) ? "<static> " : "",
			(self->statement.init.idf->idf_is_const) ? "<const> " : ""
		);
		structExprDtype_print(self->statement.init.dtype, 0);
		printf(" %s", self->statement.init.idf->name);
		if (self->statement.init.expr != NULL) { printf(" = "); structExpression_print(self->statement.init.expr, 0, true); }
		else printf("\n");

	}
	else if (self->type == STMNT_ASSIGN) {
		PRINT_INDENT(indent); printf("<assign> ");
		printf("expr="); structExpression_print(self->statement.assign.idf, 0, false); printf(" ");
		printf("op=%s expr=", self->statement.assign.op->name);
		structExpression_print(self->statement.assign.expr, 0, true);
	}
	else if (self->type == STMNT_BREAK) {
		PRINT_INDENT(indent); printf("<break>\n");
	}
	else if (self->type == STMNT_CONTINUE) {
		PRINT_INDENT(indent); printf("<continue>\n");
	}
	else if (self->type == STMNT_WHILE) {
		PRINT_INDENT(indent); printf("<while> "); structExpression_print(self->statement.stm_while.expr_bool, 0, true);
		if (self->statement.stm_while.stmn_list != NULL) structStatementList_print(self->statement.stm_while.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FOR) {
		PRINT_INDENT(indent); printf("<for>\n");
		if (self->statement.stm_for.stmn_ini != NULL) { structStatement_print(self->statement.stm_for.stmn_ini, indent + 1); }
		else { PRINT_INDENT(indent + 1); printf("(No IniStmn)\n"); }
		if (self->statement.stm_for.expr_bool != NULL) { structExpression_print(self->statement.stm_for.expr_bool, indent + 1, true); }
		else { PRINT_INDENT(indent + 1); printf("(No ExprBool)\n"); }
		if (self->statement.stm_for.expr_end != NULL) { structExpression_print(self->statement.stm_for.expr_end, indent + 1, true); }
		else { PRINT_INDENT(indent + 1); printf("(No ExprEnd)\n"); }
		if (self->statement.stm_for.stmn_list != NULL) { structStatementList_print(self->statement.stm_for.stmn_list); }
		else { PRINT_INDENT(indent + 1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FOREACH) {
		PRINT_INDENT(indent); printf("<foreach>\n");
		structStatement_print(self->statement.stm_foreach.stmn_ini, indent + 1);
		structExpression_print(self->statement.stm_foreach.expr_itter, indent + 1, true);
		if (self->statement.stm_foreach.stmn_list != NULL) { structStatementList_print(self->statement.stm_foreach.stmn_list); }
		else { PRINT_INDENT(indent + 1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_IF) {
		PRINT_INDENT(indent); printf("<if> "); structExpression_print(self->statement.stm_if.expr_bool, 0, true);
		if (self->statement.stm_if.stmn_list != NULL) structStatementList_print(self->statement.stm_if.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No StmnList)\n"); }

		if (self->statement.stm_if.else_if_list != NULL)  structStatementList_print(self->statement.stm_if.else_if_list);

		if (self->statement.stm_if.stmn_list_else != NULL) { PRINT_INDENT(indent); printf("<else>\n"); structStatementList_print(self->statement.stm_if.stmn_list_else); }

	}
	else if (self->type == STMNT_ELSE_IF) {
		PRINT_INDENT(indent); printf("<elseif> "); structExpression_print(self->statement.stmn_else_if.expr_bool, 0, true);
		if (self->statement.stm_while.stmn_list != NULL) structStatementList_print(self->statement.stm_while.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No StmnList)\n"); }
	}
	else if (self->type == STMNT_FUNC_DEFN) {
		PRINT_INDENT(indent); printf("%s%s%s%s %s\n",
			(self->statement.func_defn.idf->is_abstract) ? "<abs> " : "", // static
			(self->statement.func_defn.idf->is_override) ? "<override> " : "", // static
			(self->statement.func_defn.idf->is_static) ? "<static> " : "", // static
			(self->statement.func_defn.idf->func_is_method) ? "<method> " : "<func> ", // static
			self->statement.func_defn.idf->name // idf
		); // TODO: add abstract also
		if (self->statement.func_defn.args != NULL) structStatementList_print(self->statement.func_defn.args);
		if (self->statement.func_defn.ret_type != NULL) { PRINT_INDENT(indent + 1); printf("<ret> "); structExprDtype_print(self->statement.func_defn.ret_type, 0); printf("\n"); }
		else { PRINT_INDENT(indent + 1); printf("(Return Type Void)\n"); }
		if (self->statement.func_defn.stmn_list != NULL) structStatementList_print(self->statement.func_defn.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No Func Body)\n"); }
	}
	else if (self->type == STMNT_RETURN) {
		PRINT_INDENT(indent); printf("<return> ");
		if (self->statement.stmn_return.expr != NULL) structExpression_print(self->statement.stmn_return.expr, 0, true);
		else printf("\n");
	}

	else if (self->type == STMNT_CLASS_DEFN) {
		PRINT_INDENT(indent); printf("%s<class> %s <%s>\n",
			(self->statement.class_defn.idf->is_abstract) ? "<abs> " : "",
			self->statement.class_defn.idf->name,
			(self->statement.class_defn.idf->is_class_generic) ? self->statement.class_defn.idf->generic_type->name : ""
		);
		if (self->statement.class_defn.supers != NULL) {
			PRINT_INDENT(indent + 1); printf("<supers> ");
			for (int i = 0; i < self->statement.class_defn.supers->count; i++) {
				printf("%s, ", self->statement.class_defn.supers->list[i]->name);
			} printf("\n");
		}
		if (self->statement.class_defn.stmn_list != NULL) structStatementList_print(self->statement.class_defn.stmn_list);
		else { PRINT_INDENT(indent + 1); printf("(No Class Body)\n"); }
	}
}
struct Statement* structStatement_new(enum StatementType type, struct Statement* parent) {
	struct Statement* new_stmn = (struct Statement*)malloc(sizeof(struct Statement));
	structStatement_init(new_stmn, parent, type);
	return new_stmn;
}
/***************** </Statement> *************/






/***************** <StatementList> *************/
void structStatementList_init(struct StatementList* self, struct Statement* parent, int growth_size) {
	self->parent = parent;
	self->count = 0;
	self->growth_size = growth_size;
	self->size = self->growth_size;
	self->list = (struct Statement**)malloc(sizeof(struct Statement) * self->growth_size);
	self->indent = 0;
	self->name_table = structNameTable_new();
}
void structStatementList_addStatement(struct StatementList* self, struct Statement* statement) {
	if (self->count >= self->size) {
		struct Statement** new_list = (struct Statement**)malloc(sizeof(struct Statement) * (self->size + self->growth_size));
		self->size += self->growth_size;
		for (size_t i = 0; i < self->count; i++) {
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = statement;
}
struct Statement* structStatementList_createStatement(struct StatementList* self, enum StatementType type, struct Statement* parent) {
	struct Statement* new_stmn = structStatement_new(type, parent);
	structStatementList_addStatement(self, new_stmn);
	return new_stmn;
}
void structStatementList_print(struct StatementList* self) {
	for (size_t i = 0; i < self->count; i++) {
		structStatement_print(self->list[i], self->indent);
	}
}
struct StatementList* structStatementList_new(struct Statement* parent) {
	struct StatementList* stmn_list = (struct StatementList*)malloc(sizeof(struct StatementList));
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




/***************** <NameTableEntry> *************/

void structNameTableEntry_free(struct NameTableEntry* self) {
	free(self);
}
void structNameTableEntry_init(struct NameTableEntry* self, struct Token* idf, enum IdfType type, struct Statement* stmn) {
	self->idf = idf; 
	self->type = type;
	self->stmn = stmn; // can be NULL 
	self->is_class_generic = false;
}
void structNameTableEntry_print(struct NameTableEntry* self) {
	// do nothing
}
struct NameTableEntry* structNameTableEntry_new(struct Token* idf, enum IdfType type, struct Statement* stmn) {
	struct NameTableEntry* ret = (struct NameTableEntry*)malloc(sizeof(struct NameTableEntry));
	structNameTableEntry_init(ret, idf, type, stmn);
}

/***************** </NameTableEntry> *************/




/***************** <NameTable> *************/

void structNameTable_free(struct NameTable* self) {

}
void structNameTable_init(struct NameTable* self, int growth_size) {
	self->growth_size = growth_size;
	self->count = 0;
	self->size = growth_size;
	self->entries = (struct NameTableEntry**)malloc( (sizeof(struct NameTableEntry))*self->growth_size );
}
void structNameTable_print(struct NameTable* self){
	for (size_t i = 0; i < self->count; i++) {
		printf("%s : %i\n", self->entries[i]->idf->name, self->entries[i]->type);
	}
}
void structNameTable_addEntry(struct NameTable* self, struct NameTableEntry* entry) {
	if (self->count >= self->size) {
		struct NameTableEntry** new_entries = (struct NameTableEntry**)malloc(sizeof(struct NameTableEntry) * (self->size + self->growth_size));
		self->size += self->growth_size;
		for (size_t i = 0; i < self->count; i++) {
			new_entries[i] = self->entries[i];
		}
		free(self->entries);
		self->entries = new_entries;
	}
	self->entries[(self->count)++] = entry;
}
struct NameTableEntry* structNameTable_createEntry(struct NameTable* self, struct Token* idf, enum IdfType type, struct Statement* stmn) {
	struct NameTableEntry* new_entry = structNameTableEntry_new(idf, type, stmn);
	structNameTable_addEntry(self, new_entry);
	return new_entry;
}
struct NameTable* structNameTable_new() {
	struct NameTable* new_table = (struct NameTable*)malloc(sizeof(struct NameTable));
	structNameTable_init(new_table, NAMETABLE_LIST_SIZE);
	return new_table;
}

/***************** </NameTable> *************/










