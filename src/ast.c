#include "ast.h"

static const char* EXPRESSION_TYPE_STRING[] = {
	FOREACH_EXPRESSION_TYPE(GENERATE_STRING)
};
static const char* STATEMENT_TYPE_STRING[] = {
	FOREACH_STATEMENT_TYPE(GENERATE_STRING)
};

// public api
const char* enumExpressionType_toString(enum ExpressionType self ){
	return EXPRESSION_TYPE_STRING[self];
}

const char* enumStatementType_toString(enum StatementType self){
	return STATEMENT_TYPE_STRING[self];
}

/***************** <Expression> *************/
// public
void structExpression_init(struct Expression* self, struct TokenList* token_list, int begin_pos, int end_pos){
	self->token_list = token_list;
	self->begin_pos  = begin_pos;
	self->end_pos    = end_pos;
}
/***************** </Expression> *************/