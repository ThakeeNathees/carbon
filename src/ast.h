// https://en.wikipedia.org/wiki/Shunting-yard_algorithm
#include "tkscanner.h"

#define EXPRESSION_LIST_SIZE 10
#define STATEMENT_LIST_SIZE  50

// statement unknowns are: 3; "some str"; func_call(); method.call();
#define FOREACH_STATEMENT_TYPE(func) \
	func(STMNT_UNKNOWN)	    \
	func(STMNT_IMPORT)	    \
	func(STMNT_VAR_INI)    \
	func(STMNT_ASSIGN) 	    \
	func(STMNT_IF)		    \
	func(STMNT_ELSE_IF)	    \
	func(STMNT_RETURN)	    \
	func(STMNT_BREAK)		\
	func(STMNT_CONTINUE) 	\
	func(STMNT_WHILE) 	    \
	func(STMNT_FOR)		    \
	func(STMNT_FOREACH)	    \
	func(STMNT_FUNC_DEFN)   \
	func(STMNT_CLASS_DEFN)
	//func(STMNT_FUNC_CALL) \ it's unknonwn

enum StatementType
{
	FOREACH_STATEMENT_TYPE(GENERATE_ENUM)
};

/* expression always ends with a semi collon */
struct Expression
{
	struct TokenList* token_list;
	size_t begin_pos;
	size_t end_pos;
};

struct ExprDtype // to store dtype int, string, map<int, map<string, list<string>>>
{
	struct Token* dtype;

	bool is_list;
	bool is_map;

	struct Token*     key; // key cant be list or map
	struct ExprDtype* value; // value is for both list and map
};

/*********************************************/
struct _StatementImport
{
	struct Token* path; // import path; path is string
};
struct _StatementVarInit
{
	struct ExprDtype* dtype;
	struct Token* idf;
	struct Expression* expr; // can be NULL
};

struct _StatementVarAssign
{
	struct Expression* idf; // idf can be : my_ins.get_position().x = 2;
	struct Token* op; 		// op can be =, +=. -=. *=, ...
	struct Expression* expr;
};
struct _StatementUnknown
{
	struct Expression* expr; // just execute the expr, CAN BE NULL
};
struct _StatementIf
{
	struct Expression* expr_bool;
	struct StatementList* stmn_list;	// can be NULL
	struct StatementList* else_if_list; // can be NULL, list of statements type = else_if
	struct StatementList* stmn_list_else; // can be NULL
};
struct _StatementElseIf
{
	struct Expression* expr_bool;
	struct StatementList* stmn_list;	// can be NULL
};

struct _StatementReturn
{
	struct Expression* expr;			// can be NULL
};

struct _StatementWhile
{
	struct Expression* expr_bool;
	struct StatementList* stmn_list;	// can be NULL
};
struct _StatementFor
{
	struct Statement* stmn_ini;      // can be NULL
	struct Expression* expr_bool;    // can be NULL, if not boolean = error
	struct Expression* expr_end;     // can be NULL
	struct StatementList* stmn_list; // can be NULL
};
struct _StatementForEach
{
	struct Statement* stmn_ini;      // can be NULL
	struct Expression* expr_itter;   // can be NULL
	struct StatementList* stmn_list; // can be NULL
};
struct _StatementFuncDefn
{
	struct Token* idf;
	struct StatementList* args;      // can be NULL, null=0 args, statement list of stmn_ini,
	struct ExprDtype* ret_type;      // can be NULL = void
	struct StatementList* stmn_list; // can be NULL
};
struct _StatementClassDefn
{
	struct Token* idf;
	struct Token* par;				 // can be NULL
	struct StatementList* stmn_list; // can be NULL
};
union _Statement
{
	struct _StatementUnknown	unknown;
	struct _StatementImport 	import;
	struct _StatementVarInit 	init;
	struct _StatementVarAssign	assign;
	struct _StatementIf			stm_if;
	struct _StatementElseIf		stmn_else_if;
	struct _StatementReturn		stmn_return;
	struct _StatementWhile		stm_while;
	struct _StatementFor 		stm_for;
	struct _StatementForEach 	stm_foreach;
	struct _StatementFuncDefn 	func_defn;
	struct _StatementClassDefn	class_defn;
};
/*********************************************/

enum structAst_StmnEndType
{
	STMNEND_EOF,
	STMNEND_BRACKET_RCUR,
};

struct Statement
{
	enum StatementType type;
	struct Statement* parent;
	union _Statement statement;

};

struct ExpressionList
{
	struct TokenList* token_list;
	struct Expression** list;
	size_t count;
	size_t size;
	size_t growth_size;
};
struct StatementList
{
	struct Statement** list;
	struct Statement* parent;
	size_t count;
	size_t size;
	size_t growth_size;
	int indent; // for printing
};

/*********************************************/

struct Ast
{
	struct String* src;
	char* file_name;
	size_t pos;
	struct TokenScanner* token_scanner;
	struct TokenList* tokens;
	struct StatementList* stmn_list;
};



/****************** PUBLIC API ************************************/
const char* enumStatementType_toString(enum StatementType self);

// expression
void structExpression_free(struct Expression* self);
void structExpression_init(struct Expression* self, struct TokenList* token_list);
void structExpression_print(struct Expression* self, int indent, bool new_line);
struct Expression* structExpression_new(struct TokenList* token_list); // static

// expression dtype
void structExprDtype_free(struct Expression* self);
void structExprDtype_init(struct ExprDtype* self, struct Token* dtype);
void structExprDtype_print(struct ExprDtype* self, int indent, bool new_line); // call with new_line true; false internal
struct ExprDtype* structExprDtype_new(struct Token* dtype);

// expression list
void structExpressionList_init(struct ExpressionList* self, int growth_size, struct TokenList* token_list);
void structExpressionList_addExpression(struct ExpressionList* self, struct Expression* expr);
struct Expression* structExpressionList_createExpression(struct ExpressionList* self);
struct ExpressionList* structExpressionList_new(struct TokenList* token_list); // static method

// statement
void structStatement_free(struct Statement* self);
void structStatement_init(struct Statement* self, struct Statement* parent, enum StatementType type);
void structStatement_print(struct Statement* self, int indent);
struct Statement* structStatement_new(enum StatementType type, struct Statement* parent); // static method

// statement list
void structStatementList_deleteLast(struct StatementList* self );
void structStatementList_free(struct StatementList* self);
void structStatementList_init(struct StatementList* self, struct Statement* parent, int growth_size);
void structStatementList_print(struct StatementList* self);
void structStatementList_addStatement(struct StatementList* self, struct Statement* statement);
struct Statement* structStatementList_createStatement(struct StatementList* self, enum StatementType type, struct Statement* parent);
struct StatementList* structStatementList_new(struct Statement* parent); // static method

// ast
void structAst_init(struct Ast* self, struct String* src, char* fiel_name);
struct CarbonError* structAst_scaneTokens(struct Ast* self);
struct CarbonError* structAst_makeTree(struct Ast* self, struct StatementList* statement_list, enum structAst_StmnEndType end_type);
void structAst_deleteLastStatement(struct Ast* self);

/****************************************************************/