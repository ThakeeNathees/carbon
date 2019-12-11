
#include "tkscanner.h"

static const char* TOKEN_GROUP_STRING[] = {
	FOREACH_TOKEN_GROUP(GENERATE_STRING)
};
static const char* TOKEN_TYPE_STRING[] = {
	FOREACH_TOKEN_TYPE(GENERATE_STRING)
};
/*
static const char* NUMBER_TYPE_STRING[] = {
	FOREACH_NUMBER_TYPE(GENERATE_STRING)
};
*/

// public api
const char* enumTokenGroup_toString(enum TokenGroup self ){
	return TOKEN_GROUP_STRING[self];
}
const char* enumTokenType_toString(enum TokenType self) {
	return TOKEN_TYPE_STRING[self];
}
/*
const char* enumNumberType_toString(enum NumberType self){
	return NUMBER_TYPE_STRING[self];
}
*/


/***************** <Token> *************/
// public
void structToken_init(struct Token* self){
	self->_name_len 		 = TOKEN_NAME_SIZE;
	self->_name_ptr 		 = 0;
	self->name 				 = (char*)malloc(TOKEN_NAME_SIZE); // free after use
	self->name[0] 			 = '\0';
	self->type				 = TK_UNKNOWN;
	self->group 			 = TKG_UNKNOWN;
	self->number_value.l 	 = 0;
	//self->number_type 		 = NT_INT;
	self->func_args_count    = 0;
	self->func_args_given    = 0;
	self->func_is_method     = false;
	self->idf_is_field       = false;
	self->op_is_single = false;
	self->op_is_pre			 = false;
}
void structToken_print(struct Token* self){
	if (self->group == TKG_NUMBER){
		if 		(self->type == TK_VALUE_CHAR)  printf("Token %-20s : %-10s | value = %c\n", enumTokenType_toString(self->type), self->name, self->number_value.c);
		else if (self->type == TK_VALUE_SHORT) printf("Token %-20s : %-10s | value = %i\n", enumTokenType_toString(self->type), self->name, self->number_value.s);
		else if (self->type == TK_VALUE_INT)   printf("Token %-20s : %-10s | value = %i\n", enumTokenType_toString(self->type), self->name, self->number_value.i);
		else if (self->type == TK_VALUE_FLOAT) printf("Token %-20s : %-10s | value = %f\n", enumTokenType_toString(self->type), self->name, self->number_value.f);
		else if (self->type == TK_VALUE_DOUBLE)printf("Token %-20s : %-10s | value = %f\n", enumTokenType_toString(self->type), self->name, self->number_value.d);
		else if (self->type == TK_VALUE_LONG)  printf("Token %-20s : %-10s | value = %ld\n",enumTokenType_toString(self->type), self->name, self->number_value.l);	
	} 
	else if (self->group == TKG_FUNCTION || self->group == TKG_BUILTIN)		printf("Token %-20s : %-10s | method=%i, args_given=%i\n",	enumTokenType_toString(self->type), self->name, self->func_is_method, self->func_args_given);
	else if (self->group == TKG_OPERATOR && strcmp(self->name, OP_MINUS)==0)printf("Token %-20s : %-10s | single_op=%i\n",				enumTokenType_toString(self->type), self->name, self->op_is_single);
	else																	printf("Token %-20s : %s\n",									enumTokenType_toString(self->type), self->name);
	
}
void structToken_clear(struct Token* self){
	self->_name_ptr 	= 0;
	self->name[0] = '\0';
	self->group 	= TKG_UNKNOWN;
	// TODO: free name and allocate new mem
}
bool structToken_isAssignmentOperator(struct Token* self){
	if (self->group != TKG_OPERATOR){ return false; }
	if ( self->type == TK_OP_EQ      || 
		 self->type == TK_OP_PLUSEQ  ||  
		 self->type == TK_OP_MINUSEQ || 
		 self->type == TK_OP_MULEQ   ||
		 self->type == TK_OP_DIVEQ 
	) return true;
	return false;
}

bool structToken_isOpenBracket(struct Token* self) {
	if (self->group != TKG_BRACKET) return false;
	if (self->type == TK_BRACKET_LPARAN ||
		self->type == TK_BRACKET_LCUR	||
		self->type == TK_BRACKET_LSQ	||
		self->type == TK_BRACKET_LTRI
		) return true;
	return false;
}
bool structToken_isCloseBracket(struct Token* self) {
	if (self->group != TKG_BRACKET) return false;
	if (self->type == TK_BRACKET_RPARAN ||
		self->type == TK_BRACKET_RCUR	||
		self->type == TK_BRACKET_RSQ	||
		self->type == TK_BRACKET_RTRI
		) return true;
	return false;
}
// value <op> value -> binary operator, plus and minus are not binary operator
bool structToken_isBinaryOperator(struct Token* self) {
	if (self->group != TKG_OPERATOR) return false;
	if (
		self->type == TK_OP_EQ		||
		self->type == TK_OP_PLUSEQ	||
		self->type == TK_OP_MINUSEQ ||
		self->type == TK_OP_MUL		||
		self->type == TK_OP_MULEQ	||
		self->type == TK_OP_DIV		||
		self->type == TK_OP_DIVEQ	||
		self->type == TK_OP_MOD		||
		self->type == TK_OP_POW		||
		self->type == TK_OP_EQEQ	||
		self->type == TK_OP_NOTEQ	||
		self->type == TK_OP_GT		||
		self->type == TK_OP_LT		||
		self->type == TK_OP_GTEQ	||
		self->type == TK_OP_LTEQ	||
		self->type == TK_OP_LSHIFT	||
		self->type == TK_OP_RSHIFT	||
		self->type == TK_OP_OR		||
		self->type == TK_OP_AND		||
		self->type == TK_OP_XOR
		) return true;
	// TK_OP_PLUS, TK_OP_MINUS, TK_OP_NOT, TK_OP_INCR, TK_OP_DECR
	return false;
}

//private
void structToken_addChar(struct Token* self, char c){
	
	if (self->_name_ptr >= self->_name_len-1){ // one space for '\0'
		int growth_size = TOKEN_NAME_SIZE; if (self->group == TKG_STRING) growth_size = TOKEN_STRING_GROWTH;
		char* new_name = (char*)malloc( self->_name_len + growth_size ) ;
		self->_name_len += growth_size;
		for ( int i=0; i < self->_name_ptr; i++){
			new_name[i] = self->name[i];
		}
		free(self->name);
		self->name = new_name;
	}
	self->name[(self->_name_ptr)++] = c;
	self->name[(self->_name_ptr)] = '\0';
}
/***************** </Token> *************/

/***************** <TokenList> *************/
// private
//public
void structTokenList_init(struct TokenList* self, int growth_size){
	self->count = 0;
	self->growth_size = growth_size;
	self->size  = self->growth_size;
	self->list  = (struct Token**)malloc( sizeof(struct Token) * self->growth_size );
}
void structTokenList_addToken(struct TokenList* self, struct Token* token){
	if (self->count >= self->size){
		struct Token** new_list = (struct Token**)malloc( sizeof(struct Token)*(self->size + self->growth_size) ) ;
		self->size += self->growth_size;
		for ( int i=0; i< self->count; i++){
			new_list[i] = self->list[i];
		}
		free(self->list);
		self->list = new_list;
	}
	self->list[(self->count)++] = token;	
}
struct Token* structTokenList_createToken(struct TokenList* self){
	struct Token* new_tk = (struct Token*)malloc(sizeof(struct Token));
	structToken_init(new_tk);
	structTokenList_addToken(self, new_tk);
	return new_tk;
}
void structTokenList_print(struct TokenList* self){
	for (int i=0; i< self->count; i++){
		printf("%03i ", i ); structToken_print( self->list[i] );
	}
}
struct TokenList* structTokenList_new(){
	struct TokenList* token_list = (struct TokenList*)malloc( sizeof(struct TokenList) );
	structTokenList_init(token_list, TOKEN_LIST_SIZE);
	return token_list;
}
/***************** </TokenList> *************/




/***************** <TokenScanner> *************/

// private
static const char C_WHITE_SPACE[] 	= { ' ', '\t', '\n', '\v', '\f', '\r' };
static const char C_NUMBER[]		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static const char C_SYMBOL[]     	= { '.', ',', ':', ';', '"', '\'' };			// symbols are only one character
static const char C_BRACKET[]		= { '(', ')', '{', '}', '[', ']', /*'<', '>'*/  };
static const char C_OPERATOR[] 		= { '+', '-', '*', '/', '%', '!', '=', '<', '>', '&', '|', '^' };
static const char C_IDENTIFIER[]  	= { 
	'_',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',	
 };

// private static methods
bool structTokenScanner_isCharWhiteSpace(char c){
	size_t arr_size = ARR_SIZE(C_WHITE_SPACE);
	for (unsigned int i=0; i < arr_size; i++){
		if ( c == C_WHITE_SPACE[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharNumber(char c){
	size_t arr_size = ARR_SIZE(C_NUMBER);
	for (unsigned int i=0; i < arr_size; i++){
		if ( c == C_NUMBER[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharSymbol(char c){
	size_t arr_size = ARR_SIZE(C_SYMBOL);
	for (unsigned int i=0; i < arr_size; i++){
		if ( c == C_SYMBOL[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharBracket(char c){
	size_t arr_size = ARR_SIZE(C_BRACKET);
	for (unsigned int i=0; i < arr_size; i++){
		if ( c == C_BRACKET[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharOperator(char c){
	size_t arr_size = ARR_SIZE(C_OPERATOR);
	for (unsigned int i=0; i<arr_size; i++){
		if (c == C_OPERATOR[i]) return true;
	} return false;
}
bool structTokenScanner_isCharIdentifier(char c){
	size_t arr_size = ARR_SIZE(C_IDENTIFIER);
	for (unsigned int i=0; i < arr_size; i++){
		if (c == C_IDENTIFIER[i]) return true;
	} return false;
}

// private
void structTokenScanner_checkSymbol(struct TokenScanner* self) {
	if (self->current_token->group != TKG_SYMBOL)  utils_error_exit("InternalError: expected token group symbol", self->pos, self->src, self->file_name);
	
	if (strcmp(self->current_token->name, SYM_DOT) == 0)				{ self->current_token->type = TK_SYM_DOT;			return; }
	else if (strcmp(self->current_token->name, SYM_COMMA) == 0)			{ self->current_token->type = TK_SYM_COMMA;			return; }
	else if (strcmp(self->current_token->name, SYM_COLLON) == 0)		{ self->current_token->type = TK_SYM_COLLON;		return; }
	else if (strcmp(self->current_token->name, SYM_SEMI_COLLON) == 0)	{ self->current_token->type = TK_SYM_SEMI_COLLON;	return; }
	else if (strcmp(self->current_token->name, SYM_DQUOTE) == 0)		{ self->current_token->type = TK_SYM_DQUOTE;		return; }
	else if (strcmp(self->current_token->name, SYM_SQUOTE) == 0)		{ self->current_token->type = TK_SYM_SQUOTE;		return; }

	utils_error_exit("InternalError: unknown symbol", self->pos, self->src, self->file_name);
}
void structTokenScanner_ckeckBracket(struct TokenScanner* self) { // for < and > assign at scane expr
	if (self->current_token->group != TKG_BRACKET)  utils_error_exit("InternalError: expected token group bracket", self->pos, self->src, self->file_name);
	
	if (strcmp(self->current_token->name, LPARN) == 0)				{ self->current_token->type = TK_BRACKET_LPARAN;	return; }
	else if (strcmp(self->current_token->name, RPARN) == 0)			{ self->current_token->type = TK_BRACKET_RPARAN;	return; }
	else if (strcmp(self->current_token->name, LCUR_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_LCUR;		return; }
	else if (strcmp(self->current_token->name, RCUR_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_RCUR;		return; }
	else if (strcmp(self->current_token->name, RSQ_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_RSQ;		return; }
	else if (strcmp(self->current_token->name, LSQ_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_LSQ;		return; }
	else if (strcmp(self->current_token->name, LTRI_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_RTRI;		return; }
	else if (strcmp(self->current_token->name, RTRI_BRACKET) == 0)	{ self->current_token->type = TK_BRACKET_LTRI;		return; }
	
	utils_error_exit("InternalError: unknown bracket", self->pos, self->src, self->file_name);
}
void structTokenScanner_checkOperator(struct TokenScanner* self) {
	if (self->current_token->group != TKG_OPERATOR)  utils_error_exit("InternalError: expected token group operator", self->pos, self->src, self->file_name);

	if (strcmp(self->current_token->name, OP_EQ) == 0)		{ self->current_token->type = TK_OP_EQ;			return; }
	if (strcmp(self->current_token->name, OP_PLUS) == 0)	{ self->current_token->type = TK_OP_PLUS;		return; }
	if (strcmp(self->current_token->name, OP_PLUSEQ) == 0)	{ self->current_token->type = TK_OP_PLUSEQ;		return; }
	if (strcmp(self->current_token->name, OP_MINUS) == 0)	{ self->current_token->type = TK_OP_MINUS;		return; }
	if (strcmp(self->current_token->name, OP_MINUSEQ) == 0) { self->current_token->type = TK_OP_MINUSEQ;	return; }
	if (strcmp(self->current_token->name, OP_MUL) == 0)		{ self->current_token->type = TK_OP_MUL;		return; }
	if (strcmp(self->current_token->name, OP_MUEQ) == 0)	{ self->current_token->type = TK_OP_MULEQ;		return; }
	if (strcmp(self->current_token->name, OP_DIV) == 0)		{ self->current_token->type = TK_OP_DIV;		return; }
	if (strcmp(self->current_token->name, OP_DIVEQ) == 0)	{ self->current_token->type = TK_OP_DIVEQ;		return; }
	if (strcmp(self->current_token->name, OP_MOD) == 0)		{ self->current_token->type = TK_OP_MOD;		return; }
	if (strcmp(self->current_token->name, OP_NOT) == 0)		{ self->current_token->type = TK_OP_NOT;		return; }
	if (strcmp(self->current_token->name, OP_POW) == 0)		{ self->current_token->type = TK_OP_POW;		return; }
	if (strcmp(self->current_token->name, OP_EQEQ) == 0)	{ self->current_token->type = TK_OP_EQEQ;		return; }
	if (strcmp(self->current_token->name, OP_NOTEQ) == 0)	{ self->current_token->type = TK_OP_NOTEQ;		return; }
	if (strcmp(self->current_token->name, OP_GT) == 0)		{ self->current_token->type = TK_OP_GT;			return; }
	if (strcmp(self->current_token->name, OP_LT) == 0)		{ self->current_token->type = TK_OP_LT;			return; }
	if (strcmp(self->current_token->name, OP_GTEQ) == 0)	{ self->current_token->type = TK_OP_GTEQ;		return; }
	if (strcmp(self->current_token->name, OP_LTEQ) == 0)	{ self->current_token->type = TK_OP_LTEQ;		return; }
	if (strcmp(self->current_token->name, OP_LSHIFT) == 0)	{ self->current_token->type = TK_OP_LSHIFT;		return; }
	if (strcmp(self->current_token->name, OP_RSHIFT) == 0)	{ self->current_token->type = TK_OP_RSHIFT;		return; }
	if (strcmp(self->current_token->name, OP_OR) == 0)		{ self->current_token->type = TK_OP_OR;			return; }
	if (strcmp(self->current_token->name, OP_AND) == 0)		{ self->current_token->type = TK_OP_AND;		return; }
	if (strcmp(self->current_token->name, OP_XOR) == 0)		{ self->current_token->type = TK_OP_XOR;		return; }
	if (strcmp(self->current_token->name, OP_INCR) == 0)	{ self->current_token->type = TK_OP_INCR;		return; }
	if (strcmp(self->current_token->name, OP_XOR) == 0)		{ self->current_token->type = TK_OP_DECR;		return; }
	
	utils_error_exit("InternalError: unknown operator", self->pos, self->src, self->file_name);
}
void structTokenScanner_checkIdentifier(struct TokenScanner* self){
	if (self->current_token->group != TKG_IDENTIFIER) utils_error_exit("InternalError: expected token group identifier", self->pos, self->src, self->file_name);

	if (strcmp( self->current_token->name, KWORD_NULL) == 0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_NULL;		return; }
	else if (strcmp( self->current_token->name, KWORD_SELF )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_SELF;		return; }
	else if (strcmp( self->current_token->name, KWORD_TRUE )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_TRUE;		return; }
	else if (strcmp( self->current_token->name, KWORD_FALSE )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_FALSE;		return; }
	else if (strcmp( self->current_token->name, KWORD_IF )==0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_IF;		return; }
	else if (strcmp( self->current_token->name, KWORD_ELSE )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_ELSE;		return; }
	else if (strcmp( self->current_token->name, KWORD_WHILE )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_WHILE;		return; }
	else if (strcmp( self->current_token->name, KWORD_FOR )==0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_FOR;		return; }
	else if (strcmp( self->current_token->name, KWORD_FOREACH )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_FOREACH;	return; }
	else if (strcmp( self->current_token->name, KWORD_BREAK )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_BREAK;		return; }
	else if (strcmp( self->current_token->name, KWORD_CONTINUE )==0){ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_CONTINUE;	return; }
	else if (strcmp( self->current_token->name, KWORD_AND )==0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_AND;		return; }
	else if (strcmp( self->current_token->name, KWORD_OR )==0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_OR;		return; }
	else if (strcmp( self->current_token->name, KWORD_NOT )==0)		{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_NOT;		return; }
	else if (strcmp( self->current_token->name, KWORD_RETURN )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_RETURN;	return; }
	else if (strcmp( self->current_token->name, KWORD_STATIC )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_STATIC;	return; }
	else if (strcmp( self->current_token->name, KWORD_FUNCTION )==0){ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_FUNCTION;	return; }
	else if (strcmp( self->current_token->name, KWORD_CLASS )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_CLASS;		return; }
	else if (strcmp( self->current_token->name, KWORD_IMPORT )==0)	{ self->current_token->group = TKG_KEYWORD; self->current_token->type = TK_KWORD_IMPORT;	return; }

	else if (strcmp( self->current_token->name, DTYPE_VOID )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_VOID;	return; }
	else if (strcmp( self->current_token->name, DTYPE_BOOL )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_BOOL;	return; }
	else if (strcmp( self->current_token->name, DTYPE_CAHR )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_CHAR;	return; }
	else if (strcmp( self->current_token->name, DTYPE_SHORT )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_SHORT;	return; }
	else if (strcmp( self->current_token->name, DTYPE_INT )==0)		{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_INT;	return; }
	else if (strcmp( self->current_token->name, DTYPE_LONG )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_LONG;	return; }
	else if (strcmp( self->current_token->name, DTYPE_FLOAT )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_FLOAT;	return; }
	else if (strcmp( self->current_token->name, DTYPE_DOUBLE )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_DOUBLE; return; }
	else if (strcmp( self->current_token->name, DTYPE_LIST )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_LIST;	return; }
	else if (strcmp( self->current_token->name, DTYPE_MAP )==0)		{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_MAP;	return; }
	//if (strcmp( self->current_token->name, DTYPE_ARRAY )==0)	{ self->current_token->type = DTYPE; return;}
	else if (strcmp( self->current_token->name, DTYPE_STRING )==0)	{ self->current_token->group = TKG_DTYPE; self->current_token->type = TK_DT_STRING; return;}
	//if (strcmp( self->current_token->name, DTYPE_FUNC )==0)	{ self->current_token->type = DTYPE; return;}

	else if (strcmp(self->current_token->name, BUILTIN_PRINT) == 0)	{ self->current_token->group = TKG_BUILTIN; self->current_token->type = TK_BUILTIN_PRINT;	self->current_token->func_args_count = 1; return; }
	else if (strcmp(self->current_token->name, BUILTIN_INPUT) == 0)	{ self->current_token->group = TKG_BUILTIN; self->current_token->type = TK_BUILTIN_INPUT;	self->current_token->func_args_count = 1; return; }
	else if (strcmp(self->current_token->name, BUILTIN_MIN) == 0)	{ self->current_token->group = TKG_BUILTIN; self->current_token->type = TK_BUILTIN_MIN;		self->current_token->func_args_count = 2; return; }
	else if (strcmp(self->current_token->name, BUILTIN_MAX) == 0)	{ self->current_token->group = TKG_BUILTIN; self->current_token->type = TK_BUILTIN_MAX;		self->current_token->func_args_count = 2; return; }
	else if (strcmp(self->current_token->name, BUILTIN_RAND) == 0)	{ self->current_token->group = TKG_BUILTIN; self->current_token->type = TK_BUILTIN_RAND;	self->current_token->func_args_count = 1; return; }

	else { self->current_token->type = TK_IDENTIFIER; return; }
}

struct CarbonError* structTokenScanner_skipComments(struct TokenScanner* self){
	//utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
	if ( self->pos >= strlen(self->src) ) return structCarbonError_new();
	char c = self->src[ self->pos ];
	if (c == '/'){
		if ( (self->pos)+1 >= strlen(self->src) ){ return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->pos, self->src, self->file_name, false);}
		char next = self->src[(self->pos)+1];
		// //
		if (next == '/'){ 
			self->current_token->group = TKG_COMMENT;
			while(c != '\n'){
				structToken_addChar( self->current_token, c );
				if ( (++self->pos) >= strlen(self->src) ){ return structCarbonError_new(); } 
				c = self->src[self->pos];
			} 
			self->pos++;
			return structCarbonError_new();
		}

		// /**/
		if (next == '*'){
			structToken_addChar( self->current_token, '/' );structToken_addChar( self->current_token, '*' );
			self->current_token->group = TKG_COMMENT;(self->pos)+=2;
			while (true){
				if ( (self->pos)+1 >= strlen(self->src)  ) { return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->pos, self->src, self->file_name, false);}
				if ( self->src[self->pos] == '*' && self->src[(self->pos)+1] == '/' ){
					structToken_addChar( self->current_token, '*' );structToken_addChar( self->current_token, '/' );
					(self->pos)+=2; return structCarbonError_new();
				}
				c = self->src[self->pos]; structToken_addChar( self->current_token, c );
				(self->pos)++;
			}
		}
	}
	//if (!structTokenScanner_isCharWhiteSpace(c)) *is_eof = false; 
	return structCarbonError_new();
	
}

bool structTokenScanner_isEof(struct TokenScanner* self){
	if ( self->pos >= strlen(self->src) ){
		return true;
	} return false;
}

// return true if EOF
struct CarbonError* structTokenScanner_skipWhiteSpaceAndComments(struct TokenScanner* self, bool* is_eof){
	while (true){
		if ( self->pos >= strlen(self->src) ){ *is_eof = true; return structCarbonError_new();}
		
		char c = self->src[ self->pos ];
		struct CarbonError* err = structTokenScanner_skipComments(self); if (err->type != ERROR_SUCCESS) return err; 
		structCarbonError_free(err);
		if (structTokenScanner_isEof(self)) { *is_eof = true; return structCarbonError_new(); }

		c = self->src[ self->pos ];
		if (!structTokenScanner_isCharWhiteSpace(c)){*is_eof = false; return structCarbonError_new();}
		(self->pos)++;
	}
}

struct CarbonError* structTokenScanner_validateNumber(struct TokenScanner* self){

	// invalid number
	char* numstr = self->current_token->name;
	size_t   numlen = strlen(numstr);
	int count = utils_char_count_in_str('.', numstr);
	if (count > 1){ if(self->src[ self->pos ] == '\n') (self->pos)--;
		return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
	}

	size_t arr_size;

	if (utils_char_in_str('x', numstr)){
		count = utils_char_count_in_str('x', numstr);
		char invalid_chars_hex[] = { '_', 'g',  'h',  'i',  'j',  'k',  'l', 'm',  'n',  'o',  'p',  'q',  'r',  't',  'u',  'v',  'w',  'y',  'z', 
		'A',  'B',  'C',  'D',  'E',  'F', 'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
		};
		arr_size = ARR_SIZE(invalid_chars_hex);
			for (unsigned int i=0; i<arr_size; i++){
			if (utils_char_in_str(invalid_chars_hex[i], numstr)) { if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		}
		if (count > 1 || numstr[1] != 'x' || numstr[0] != '0' || utils_char_in_str('.', numstr) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		char hexstring[10]; // TODO: 10
		strcpy(hexstring, numstr+2); // ignore 0x
		self->current_token->type 				= TK_VALUE_INT;
		self->current_token->number_value.i 	= (int)strtol(hexstring, NULL, 16);
		return structCarbonError_new();
	}

	if(utils_char_in_str('s', numstr)){
		count = utils_char_count_in_str('s', numstr);
		if (count > 1 || numstr[numlen-1] != 's' || utils_char_in_str('.', numstr)){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->type 				= TK_VALUE_SHORT;
		self->current_token->number_value.s 	= (short)atoi(numstr);
		return structCarbonError_new();
	}
	if(utils_char_in_str('l', numstr )){
		count = utils_char_count_in_str('l', numstr);
		if (count > 1 || numstr[numlen-1] != 'l' || utils_char_in_str('.', numstr)){ if(self->src[self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->type 				= TK_VALUE_LONG;
		self->current_token->number_value.l 	= atol(numstr);
		return structCarbonError_new();
	}
	if (utils_char_in_str('f', numstr)){
		count = utils_char_count_in_str('f', numstr);
		if (count > 1 || numstr[numlen-1] != 'f'){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->type 				= TK_VALUE_FLOAT;
		self->current_token->number_value.f 	= (float)atof(numstr);
		return structCarbonError_new();
	}
	if (utils_char_in_str('d', numstr)){
		count = utils_char_count_in_str('d', numstr);
		if (count > 1 || numstr[numlen-1] != 'd'){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->type 				= TK_VALUE_DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return structCarbonError_new();
	}

	char invalid_chars[] = { '_', 'a',  'b',  'c',  'e',  'g',  'h',  'i',  'j',  'k',  'm',  'n',  'o',  'p',  'q',  'r',  't',  'u',  'v',  'w',  'y',  'z', 
	'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
	};
	arr_size = ARR_SIZE(invalid_chars);
	for (unsigned int i=0; i<arr_size; i++){
		if (utils_char_in_str(invalid_chars[i], numstr)) { if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
	}

	if (utils_char_in_str('.', numstr)){
		self->current_token->type 				= TK_VALUE_DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return structCarbonError_new();
	} else {
		self->current_token->type 				= TK_VALUE_INT;
		self->current_token->number_value.i 	= atoi(numstr);
		return structCarbonError_new();
	}
}

// public
void structTokenScanner_init(struct TokenScanner* self, char* src, char* file_name){
	self->src = src;
	self->file_name = file_name;
	self->pos = 0;
	// self->current_token = current_token; // not initialized, use setToken
}
void structTokenScanner_setToken(struct TokenScanner* self, struct Token* current_token){
	self->current_token = current_token;
}

struct CarbonError* structTokenScanner_scaneToken(struct TokenScanner* self, bool* is_eof){

	*is_eof = false;
	structToken_clear(self->current_token);
	struct CarbonError* err = structTokenScanner_skipWhiteSpaceAndComments(self, is_eof); if (err->type != ERROR_SUCCESS) return err;
	structCarbonError_free(err);
	if (*is_eof) return structCarbonError_new();
	char c = self->src[ self->pos ];

	self->current_token->pos = self->pos;

	// identifier
	if (structTokenScanner_isCharIdentifier(c)){
		while (true){
			structToken_addChar( self->current_token, c ); (self->pos)++;
			if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
				return utils_make_error("EofError: s unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
			}
			c = self->src[ self->pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || structTokenScanner_isCharSymbol(c) || structTokenScanner_isCharBracket(c) || structTokenScanner_isCharOperator(c) ){
				self->current_token->group = TKG_IDENTIFIER;
				structTokenScanner_checkIdentifier(self);
				return structCarbonError_new();
			}
		}
	}


	// symbol
	if ( structTokenScanner_isCharSymbol(c) ){
		
		// string
		if (c == '"'){
			self->current_token->group = TKG_STRING; self->current_token->type = TK_STRING;
			while(true){
				(self->pos)++;
				if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
					return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
				}

				c = self->src[ self->pos ];
				if (c == '\\'){
					(self->pos)++;
					if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
						return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
					}
					c = self->src[self->pos];
					if 		(c == '"'); // do nothing
					else if (c == '\\') c = '\\';
					else if (c == 't') c = '\t';
					else if (c == 'b') c = '\b';
					else if (c == 'f') c = '\f';
					else if (c == 'r') c = '\r';
					else if (c == 'v') c = '\v';
					else if (c == 'n') c = '\n';
					else structToken_addChar( self->current_token, '\\' ); // \ is not escaping 

				} else if (c=='"'){
					(self->pos)++;
					return structCarbonError_new();
				}
				structToken_addChar( self->current_token, c );
			}
		}

		// character
		if (c == '\''){
			(self->pos)++;
			c = self->src[self->pos];
			if (c == '\\'){
				(self->pos)++;
				if ( structTokenScanner_isEof(self) ){
						return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
				}
				c = self->src[self->pos];
				if 		(c == '"'); // donothing
				else if (c == '\\') c = '\\';
				else if (c == '\'') c = '\'';
				else if (c == 't')  c = '\t';
				else if (c == 'b')  c = '\b';
				else if (c == 'f')  c = '\f';
				else if (c == 'r')  c = '\r';
				else if (c == 'v')  c = '\v';
				else if (c == 'n')  c = '\n';
				else { 
					char location_str[ERROR_LINE_SIZE];
					char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(self->pos, self->src, buff, location_str);
					printf("Warning: unknown escaping @%s:%i\n%s\n", self->file_name, line_no, buff);
					printf("%s\n", location_str);
				}
			} 
			else if (c == '\''){
				return utils_make_error("TypeError: invalid char", ERROR_TYPE, self->current_token->pos, self->src, self->file_name, false);
			}
			structToken_addChar( self->current_token, c ); (self->pos)++;
			self->current_token->group			= TKG_NUMBER;
			self->current_token->type			= TK_VALUE_CHAR;
			self->current_token->number_value.c = c;
			if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
				return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
			}
			if (self->src[(self->pos)++] != '\''){  if(self->src[ self->pos ] == '\n') (self->pos)--;
				return utils_make_error("SyntaxError: expected symbol ' (use \" for strings)", ERROR_SYNTAX, self->pos-1, self->src, self->file_name, false);
			} 
			return structCarbonError_new();
		}

		// symbol not a string
		structToken_addChar( self->current_token, c); (self->pos)++;
		self->current_token->group = TKG_SYMBOL; 
		structTokenScanner_checkSymbol(self);
		return structCarbonError_new();
	}

	// bracket
	if ( structTokenScanner_isCharBracket(c) ){
		structToken_addChar( self->current_token, c ); (self->pos)++;		
		self->current_token->group = TKG_BRACKET;
		structTokenScanner_ckeckBracket(self);
		return structCarbonError_new();
	}

	// operator TODO:
	if ( structTokenScanner_isCharOperator(c) ){
		structToken_addChar( self->current_token, c ); (self->pos)++;
		self->current_token->group = TKG_OPERATOR;
		if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
		}
		char next = self->src[(self->pos)++];
		if (c == '=' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '!' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '+' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '-' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '*' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '*' && next == '*'){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '/' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '<' && next == '<'){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '>' && next == '>'){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '>' && next == '='){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '<' && next == '=') { structToken_addChar(self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '+' && next == '+') { structToken_addChar(self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		if (c == '-' && next == '-'){ structToken_addChar( self->current_token, next); structTokenScanner_checkOperator(self); return structCarbonError_new(); }
		structTokenScanner_checkOperator(self);
		(self->pos)--;
		return structCarbonError_new();
	}

	if ( structTokenScanner_isCharNumber(c) ){
		
		while (true){
			structToken_addChar( self->current_token, c ); (self->pos)++;
			if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
				return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
			}
			c = self->src[ self->pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || (structTokenScanner_isCharSymbol(c) && c != '.' ) || structTokenScanner_isCharBracket(c) || structTokenScanner_isCharOperator(c) ){
				self->current_token->group = TKG_NUMBER;
				return structTokenScanner_validateNumber(self);
			}
		}
	}
	utils_error_exit("InternalError: scane token reached an invalid point", self->current_token->pos, self->src, self->file_name);
	return structCarbonError_new(); // for complier warning
}

struct TokenScanner* structTokenScanner_new(char* src, char* file_name){
	struct TokenScanner* token_scanner = (struct TokenScanner*)malloc( sizeof(struct TokenScanner) );
	structTokenScanner_init(token_scanner, src, file_name);
	return token_scanner;
}


/***************** </TokenScanner> *************/