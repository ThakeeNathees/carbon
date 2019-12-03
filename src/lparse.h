#ifndef LPARSE_H
#define LPARSE_H


// token types
#define FOREACH_ENUM(func) \
	func(SYMBOL) 		\
	func(BRACKET) 		\
	func(DTYPE)			\
	func(OPERATOR)		\
	func(KEYWORD)		\
	func(BUILTIN)		\
	func(IDENTIFIER) // variable, function, class name

#define GENERATE_ENUM(enum_name) enum_name,
#define GENERATE_STRING(enum_name) #enum_name,


enum TokenType
{
	FOREACH_ENUM(GENERATE_ENUM)
};
// public api
const char* TokenType_toString(enum TokenType self);



// symbols
#define SYM_DOT 		"."
#define SYM_COMMA 		","
#define SYM_COLLON		":"

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
#define DTYPE_BOOL 	 	"bool"
#define DTYPE_CAHR 	 	"char"
#define DTYPE_SHORT	 	"short"
#define DTYPE_INT 	 	"int"
#define DTYPE_LONG 	 	"long"
#define DTYPE_FLOAT	 	"float"
#define DTYPE_DOUBLE  	"double"

#define DTYPE_LIST 		"list"
#define DTYPE_ARRAY 	"array"
#define DTYPE_MAP 		"map"
#define DTYPE_STRING 	"string"
#define DTYPE_FUNC 		"func"

// arithmetic operators
#define OP_PLUS			"+"
#define OP_MINUS		"-"
#define OP_MUL			"*"
#define OP_DIV 			"/"
#define OP_MOD			"%"
#define OP_DIV_FLOOR	"//"

// bitwise operators
#define OP_LSHIFT		"<<"
#define OP_RSHIFT 		">>"
#define OP_OR 			"|"
#define OP_AND 			"&"
#define OP_XOR 			"^"

// keywords
#define KWORD_NULL 		"null"
#define KWORD_TRUE 		"true"
#define KWORD_FALSE		"false"
#define KWORD_IF 		"if"
#define KWORD_ELSE 		"else"
#define KWORD_WHILE 	"while"
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
#define BUILTIN_PRINT "print"
#define BUILTIN_MIN   "min"
#define BUILTIN_MAX   "max"
#define BUILTIN_RAND  "rand"


#endif