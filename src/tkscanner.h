#ifndef TKSCANNER_H
#define TKSCANNER_H

#include "utils.h"

#define TOKEN_LIST_SIZE 100
#define TOKEN_NAME_SIZE 100

#define GENERATE_ENUM(enum_name) enum_name,
#define GENERATE_STRING(enum_name) #enum_name,

// token types
// tk_pass : if >> to > and > use tk_pass
#define FOREACH_TOKEN_TYPE(func) \
	func(UNKNOWN)		\
	func(TK_EOF)		\
	func(TK_PASS)		\
	func(COMMENT)		\
	func(SYMBOL) 		\
	func(BRACKET) 		\
	func(OPERATOR)		\
	func(KEYWORD)		\
	func(DTYPE)			\
	func(BUILTIN)		\
	func(NUMBER)		\
	func(STRING) 		\
	func(FUNCTION) 		\
	func(VARIABLE)		\
	func(IDENTIFIER) // variable, function when tkscan, ...

#define FOREACH_NUMBER_TYPE(func) \
	func(NT_UNKNOWN)\
	func(CHAR)		\
	func(SHORT)		\
	func(INT)		\
	func(FLOAT)		\
	func(DOUBLE)	\
	func(LONG)		


/**************** CLASSES **********************/
enum TokenType
{
	FOREACH_TOKEN_TYPE(GENERATE_ENUM)
};

union NumberValue
{
	char 	c;
	short 	s;
	int 	i;
	float 	f;
	double 	d;
	long 	l;
};
enum NumberType
{
	FOREACH_NUMBER_TYPE(GENERATE_ENUM)
};

struct Token
{
	enum  TokenType 	type;
	// name
	char* 				name;
	int 				_name_len;
	int 				_name_ptr;
	int 				pos;
	// for number type
	union NumberValue 	number_value;
	enum  NumberType	number_type;

	// for function type
	bool func_is_method; // insts.method() : comes after . operator and after a '('
	int  func_args_count;

	// for identifier
	bool idf_is_field; // instance.field : comes after . operator no brecket after

	// for minus operator
	bool minus_is_single_op;
};

struct TokenList
{
	struct Token** list;
	int count;
	int size;
	int growth_size;
};

struct TokenScanner
{
	char* src;
	char* file_name;
	struct Token* current_token;
	int pos;
};

/****************** PUBLIC API ************************************/
const char* enumTokenType_toString(enum TokenType self);
const char* enumNumberType_toString(enum NumberType self);

// token
void structToken_init(struct Token* self);
int  structToken_toString(struct Token* self, char* buffer);
void structToken_print(struct Token* self);

// token list
void structTokenList_init(struct TokenList* self, int growth_size);
void structTokenList_addToken(struct TokenList* self, struct Token* token);
struct Token* structTokenList_createToken(struct TokenList* self);
void structTokenList_print(struct TokenList* self);
struct TokenList* structTokenList_new(); // static method

// token scanner
void structTokenScanner_init(struct TokenScanner* self, char* src, char* file_name);
bool structTokenScanner_setToken(struct TokenScanner* self, struct Token* current_token);
bool structTokenScanner_scaneToken(struct TokenScanner* self); // return true if eof
struct TokenScanner* structTokenScanner_new(char* src, char* file_name); // static method

/****************************************************************/


// symbols
#define SYM_DOT 		"." // or maybe operator
#define SYM_COMMA 		","
#define SYM_COLLON		":"
#define SYM_SEMI_COLLON	";"
#define SYM_DQUOTE		"\""
#define SYM_SQUOTE 		"'"

// brackets
#define LPARN 		 	"("
#define RPARN 		 	")"
#define LCRU_BRACKET 	"{"
#define RCRU_BRACKET 	"}"
#define RSQ_BRACKET 	"["
#define LSQ_BRACKET 	"]"
#define RTRI_BRACKET	"<"
#define LTRI_BRACKET	">"

// data types
#define DTYPE_VOID 		"void"
#define DTYPE_BOOL 	 	"bool"
#define DTYPE_CAHR 	 	"char"
#define DTYPE_SHORT	 	"short"
#define DTYPE_INT 	 	"int"
#define DTYPE_LONG 	 	"long"
#define DTYPE_FLOAT	 	"float"
#define DTYPE_DOUBLE  	"double"

#define DTYPE_LIST 		"list"
#define DTYPE_MAP 		"map"
#define DTYPE_STRING 	"string"
//#define DTYPE_FUNC 	"func" function is not a type anymore

// arithmetic operators
#define OP_EQ 			"="
#define OP_PLUS			"+"   // operator ++, and --
#define OP_PLUSEQ 		"+="
#define OP_MINUS		"-"   // can applied for negative numbers <num> - <num> or - <num>
#define OP_MINUSEQ 		"-="
#define OP_MUL			"*"
#define OP_MUEQ 		"*="
#define OP_DIV 			"/"
#define OP_DIVEQ 		"/="
#define OP_MOD			"%"
#define OP_NOT 			"!" // simmiler of not
#define OP_POW			"**"
// #define OP_DIV_FLOOR	"//" not an operator because // is for comment

// bool operators
#define OP_EQEQ 		"=="
#define OP_NOTEQ		"!="
#define OP_GT 			">"
#define OP_LT 			"<"
#define OP_EQ 			"="
#define OP_GTEQ 		">="
#define OP_LTEQ 		"<="


// bitwise operators
#define OP_LSHIFT		"<<"
#define OP_RSHIFT 		">>"
#define OP_OR 			"|"
#define OP_AND 			"&"
#define OP_XOR 			"^"

// keywords
#define KWORD_NULL 		"null"
#define KWORD_SELF 		"self"
#define KWORD_TRUE 		"true"
#define KWORD_FALSE		"false"
#define KWORD_IF 		"if"
#define KWORD_ELSE 		"else"
#define KWORD_WHILE 	"while"
#define KWARD_FOR 		"for"
#define KWARD_FOREACH	"foreach"
#define KWORD_BREAK 	"break"
#define KWORD_CONTINUE 	"continue"
#define KWORD_AND 		"and"
#define KWORD_OR 		"or"
#define KWORD_NOT 		"not"
#define KWORD_RETURN 	"return"
#define KWORD_STATIC 	"static"
#define KWORD_FUNCTION 	"function"
#define KWORD_CLASS 	"class"
#define KWORD_IMPORT 	"import"

// built in func
#define BUILTIN_PRINT 	"print"
#define BUILTIN_INPUT 	"input"
#define BUILTIN_MIN   	"min"
#define BUILTIN_MAX   	"max"
#define BUILTIN_RAND  	"rand" // 0 <= rand(x) < x, x is an int


#endif