#ifndef TKSCANNER_H
#define TKSCANNER_H

#include "utils.h"

#define TOKEN_LIST_SIZE 100
#define TOKEN_NAME_SIZE 10
#define TOKEN_STRING_GROWTH 100


/* add list
new keyword:
	expr scanner : illegal keyword 
	token scanner
	enum
new symbol:
	expr scanner : illegal before ending, illegal in an expr
*/

// token types
// tk_pass : if >> to > and > use tk_pass
#define FOREACH_TOKEN_GROUP(func) \
	func(TKG_UNKNOWN)		\
	func(TKG_EOF)			\
	func(TKG_PASS)			\
	func(TKG_COMMENT)		\
	func(TKG_SYMBOL) 		\
	func(TKG_BRACKET) 		\
	func(TKG_OPERATOR)		\
	func(TKG_KEYWORD)		\
	func(TKG_DTYPE)			\
	func(TKG_NUMBER)		\
	func(TKG_STRING) 		\
	func(TKG_IDENTIFIER) // variable, function when tkscan, ...

	//func(TKG_VARIABLE)		\
	//func(TKG_BUILTIN)		\
	//func(TKG_FUNCTION) 		\

#define FOREACH_TOKEN_TYPE(func)\
	func(TK_UNKNOWN)	\
	func(TK_EOF)		\
	func(TK_PASS)		\
	func(TK_STRING)		\
	func(TK_IDENTIFIER)			\
	func(TK_VARIABLE)			\
	func(TK_FUNCTION)			\
	func(TK_CLASS)				\
	func(TK_SYM_DOT)		\
	func(TK_SYM_COMMA)		\
	func(TK_SYM_COLLON)		\
	func(TK_SYM_SEMI_COLLON)\
	func(TK_SYM_DQUOTE)		\
	func(TK_SYM_SQUOTE)		\
	func(TK_SYM_AT)			\
	func(TK_SYM_HASH)		\
	func(TK_SYM_DOLLAR)		\
	func(TK_SYM_DILDO)		\
	func(TK_BRACKET_LPARAN)		\
	func(TK_BRACKET_RPARAN)		\
	func(TK_BRACKET_LCUR)		\
	func(TK_BRACKET_RCUR)		\
	func(TK_BRACKET_RSQ)		\
	func(TK_BRACKET_LSQ)		\
	func(TK_BRACKET_RTRI)		\
	func(TK_BRACKET_LTRI)		\
	func(TK_DT_VOID)		\
	func(TK_DT_BOOL)		\
	func(TK_DT_CHAR)		\
	func(TK_DT_SHORT)		\
	func(TK_DT_INT)			\
	func(TK_DT_LONG)		\
	func(TK_DT_FLOAT)		\
	func(TK_DT_DOUBLE)		\
	func(TK_DT_LIST)		\
	func(TK_DT_MAP)			\
	func(TK_DT_STRING)		\
	func(TK_OP_EQ)		\
	func(TK_OP_PLUS)	\
	func(TK_OP_PLUSEQ)	\
	func(TK_OP_MINUS)	\
	func(TK_OP_MINUSEQ)	\
	func(TK_OP_MUL)		\
	func(TK_OP_MULEQ)	\
	func(TK_OP_DIV)		\
	func(TK_OP_DIVEQ)	\
	func(TK_OP_MOD)		\
	func(TK_OP_NOT)		\
	func(TK_OP_POW)		\
	func(TK_OP_EQEQ)	\
	func(TK_OP_NOTEQ)	\
	func(TK_OP_GT)		\
	func(TK_OP_LT)		\
	func(TK_OP_GTEQ)	\
	func(TK_OP_LTEQ)	\
	func(TK_OP_LSHIFT)	\
	func(TK_OP_RSHIFT)	\
	func(TK_OP_OR)		\
	func(TK_OP_AND)		\
	func(TK_OP_XOR)		\
	func(TK_OP_INCR)	\
	func(TK_OP_DECR)	\
	func(TK_KWORD_NULL)			\
	func(TK_KWORD_SELF)			\
	func(TK_KWORD_TRUE)			\
	func(TK_KWORD_FALSE)		\
	func(TK_KWORD_IF)			\
	func(TK_KWORD_ELSE)			\
	func(TK_KWORD_WHILE)		\
	func(TK_KWORD_FOR)			\
	func(TK_KWORD_FOREACH)		\
	func(TK_KWORD_BREAK)		\
	func(TK_KWORD_CONTINUE)		\
	func(TK_KWORD_AND)			\
	func(TK_KWORD_OR)			\
	func(TK_KWORD_NOT)			\
	func(TK_KWORD_RETURN)		\
	func(TK_KWORD_FUNCTION)		\
	func(TK_KWORD_CLASS)		\
	func(TK_KWORD_IMPORT)		\
	func(TK_KWORD_STATIC)		\
	func(TK_KWORD_CONST)		\
	func(TK_KWORD_ABSTRACT)		\
	func(TK_KWORD_OVERRIDE)		\
	func(TK_BUILTIN_PRINT)	\
	func(TK_BUILTIN_INPUT)	\
	func(TK_BUILTIN_MIN)	\
	func(TK_BUILTIN_MAX)	\
	func(TK_BUILTIN_RAND)	\
	func(TK_VALUE_CHAR)			\
	func(TK_VALUE_SHORT)		\
	func(TK_VALUE_INT)			\
	func(TK_VALUE_LONG)			\
	func(TK_VALUE_FLOAT)		\
	func(TK_VALUE_DOUBLE)


/**************** CLASSES **********************/
enum TokenGroup
{
	FOREACH_TOKEN_GROUP(GENERATE_ENUM)
};

enum TokenType {
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
/*
enum NumberType
{
	FOREACH_NUMBER_TYPE(GENERATE_ENUM)
};
*/

struct Token
{
	enum TokenType		type;
	enum TokenGroup 	group;
	// name
	char* 				name;
	size_t 				_name_len;
	size_t 				_name_ptr;
	size_t 				pos;

	// for number type
	union NumberValue 	number_value;
	// for function type
	bool func_is_method; // insts.method() : comes after . operator and after a '(', also used for idf of func_defn statement
	int  func_args_count_min;
	int  func_args_count_max;
	int  func_args_given; // for check count == given
	// for identifier
	bool idf_is_field; // instance.field : comes after . operator no brecket after
	
	bool idf_is_const;
	bool is_static; // for both func and idf
	bool is_abstract; // for both class and it't abstract functions
	bool is_override;

	bool is_class_generic;
	struct Token* generic_type;
	
	// for minus operator
	bool op_is_single;
	// for incr and decr operator
	bool op_is_pre;
	// for comma
	bool comma_is_valid; // expr = 2,2 <-- invalid comma, when count args or scaning lists comma is valid, in expr scan if comma invalid error!
	// for equal
	bool eq_is_valid;
};

struct TokenList
{
	struct Token** list;
	size_t count;
	size_t size;
	int growth_size;
};

struct TokenScanner
{
	struct String* src;
	char* file_name;
	struct Token* current_token;
	size_t pos;
};

/****************** PUBLIC API ************************************/
const char* enumTokenGroup_toString(enum TokenGroup self);
const char* enumTokenType_toString(enum TokenType self);
//const char* enumNumberType_toString(enum NumberType self);

// token
void structToken_free(struct Token* self);
void structToken_init(struct Token* self);
void structToken_print(struct Token* self);
bool structToken_isAssignmentOperator(struct Token* self);
bool structToken_isOpenBracket(struct Token* self);
bool structToken_isCloseBracket(struct Token* self);
bool structToken_isBinaryOperator(struct Token* self);
bool structToken_isBuiltin(struct Token* self);
void structToken_addChar(struct Token* self, char c);

// token list
void structTokenList_deleteLast(struct TokenList* self);
void structTokenList_free(struct TokenList* self);
void structTokenList_init(struct TokenList* self, int growth_size);
void structTokenList_addToken(struct TokenList* self, struct Token* token);
struct Token* structTokenList_createToken(struct TokenList* self);
void structTokenList_print(struct TokenList* self);
struct TokenList* structTokenList_new(); // static method

// token scanner
void structTokenScanner_init(struct TokenScanner* self, struct String* src, char* file_name);
void structTokenScanner_setToken(struct TokenScanner* self, struct Token* current_token);
struct CarbonError* structTokenScanner_scaneToken(struct TokenScanner* self, bool* is_eof); // return true if eof
struct TokenScanner* structTokenScanner_new(struct String* src, char* file_name); // static method

/****************************************************************/


// symbols
#define SYM_DOT 		"." // or maybe operator
#define SYM_COMMA 		","
#define SYM_COLLON		":"
#define SYM_SEMI_COLLON	";"
#define SYM_DQUOTE		"\""
#define SYM_SQUOTE 		"'"
#define SYM_AT			"@"
#define SYM_HASH		"#"
#define SYM_DOLLAR		"$"
#define SYM_DILDO		"`"

// brackets
#define LPARN 		 	"("
#define RPARN 		 	")"
#define LCUR_BRACKET 	"{"
#define RCUR_BRACKET 	"}"
#define LSQ_BRACKET 	"["
#define RSQ_BRACKET 	"]"
#define LTRI_BRACKET	"<"
#define RTRI_BRACKET	">"

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
// #define DTYPE_FUNC 	"func" function is not a type anymore

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
#define OP_INCR			"++"
#define OP_DECR			"--"
// #define OP_DIV_FLOOR	"//" not an operator because // is for comment

// bool operators
#define OP_EQEQ 		"=="
#define OP_NOTEQ		"!="
#define OP_GT 			">"
#define OP_LT 			"<"
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
#define KWORD_FOR 		"for"
#define KWORD_FOREACH	"foreach"
#define KWORD_BREAK 	"break"
#define KWORD_CONTINUE 	"continue"
#define KWORD_AND 		"and"
#define KWORD_OR 		"or"
#define KWORD_NOT 		"not"
#define KWORD_RETURN 	"return"
#define KWORD_FUNCTION 	"func"
#define KWORD_CLASS 	"class"
#define KWORD_IMPORT 	"import"

#define KWORD_STATIC    "static"
#define KWORD_CONST		"const"
#define KWORD_ABSTRACT  "abstract"
#define KWORD_OVERRIDE  "override"

// built in func
#define BUILTIN_PRINT 	"print"
#define BUILTIN_INPUT 	"input"
#define BUILTIN_MIN   	"min"
#define BUILTIN_MAX   	"max"
#define BUILTIN_RAND  	"rand" // 0 <= rand(x) < x, x is an int


#endif