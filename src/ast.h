#include "tkscanner.h"

#define FOREACH_EXPRESSION_TYPE(func) \
	func(EXPR_UNKNOWN)	   \
	func(EXPR_IMPORT)	   \
	func(EXPR_VAR_INIT)    \
	func(EXPR_ASSIGN) 	   \
	func(EXPR_FUNC_CALL)   \
	func(EXPR_METHOD_CALL)

#define FOREACH_STATEMENT_TYPE(func) \
	func(STMNT_UNKNOWN)	   \
	func(STMNT_IF)		   \
	func(STMNT_WHILE) 	   \
	func(STMNT_FOR)		   \
	func(STMNT_FOREACH)	   \
	func(STMNT_FUNC_DEFN)  \
	func(STMNT_CLASS_DEFN)

enum ExpressionType
{
	FOREACH_EXPRESSION_TYPE(GENERATE_ENUM)
};
enum StatementType
{
	FOREACH_STATEMENT_TYPE(GENERATE_ENUM)
};

/* expression always ends with a semi collon */
struct Expression
{
	struct TokenList* token_list;
	int begin_pos;
	int end_pos;
};

struct Statement
{
	enum StatementType type;
	// union of different sattement structs
};

/****************** PUBLIC API ************************************/
const char* enumExpressionType_toString(enum ExpressionType self );
const char* enumStatementType_toString(enum StatementType self);

// expression
void structExpression_init(struct Expression* self, struct TokenList* token_list, int begin_pos, int end_pos);

// statement
//void structStatement_init(struct Statement* self);

/****************************************************************/