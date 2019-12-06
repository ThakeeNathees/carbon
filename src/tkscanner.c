
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tkscanner.h"
#include "utils.h"

#define ERROR_LINE_SIZE 80
#define TOKEN_LIST_SIZE 100

static const char* TOKEN_TYPE_STRING[] = {
	FOREACH_TOKEN_TYPE(GENERATE_STRING)
};
static const char* NUMBER_TYPE_STRING[] = {
	FOREACH_NUMBER_TYPE(GENERATE_STRING)
};

// public api
const char* enumTokenType_toString(enum TokenType self ){
	return TOKEN_TYPE_STRING[self];
}

const char* enumNumberType_toString(enum NumberType self){
	return NUMBER_TYPE_STRING[self];
}


/***************** <Token> *************/
// public
void structToken_init(struct Token* self){
	self->_name_len 		= TOKEN_NAME_SIZE;
	self->_name_ptr 		= 0;
	self->token_name 		= (char*)malloc(TOKEN_NAME_SIZE); // free after use
	self->token_name[0] 	= '\0';
	self->token_type 		= UNKNOWN;
	self->number_value.l 	= 0;
	self->number_type 		= INT;
}
int structToken_toString(struct Token* self, char* buffer){
	return sprintf(buffer, "Token %s : %s", enumTokenType_toString(self->token_type), self->token_name);
}
void structToken_print(struct Token* self){
	if (self->token_type == NUMBER){
		if 		(self->number_type == CHAR)  printf("Token %10s : %-10s | (%s, %c)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.c);
		else if (self->number_type == SHORT) printf("Token %10s : %-10s | (%s, %i)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.s);
		else if (self->number_type == INT)   printf("Token %10s : %-10s | (%s, %i)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.i);
		else if (self->number_type == FLOAT) printf("Token %10s : %-10s | (%s, %f)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.f);
		else if (self->number_type == DOUBLE)printf("Token %10s : %-10s | (%s, %f)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.d);
		else if (self->number_type == LONG)  printf("Token %10s : %-10s | (%s, %ld)\n", enumTokenType_toString(self->token_type), self->token_name, enumNumberType_toString(self->number_type), self->number_value.l);
	} else{
		printf("Token %10s : %s\n", enumTokenType_toString(self->token_type), self->token_name);
	}
}
void structToken_clear(struct Token* self){
	self->_name_ptr 	= 0;
	self->token_name[0] = '\0';
	self->token_type 	= UNKNOWN;
	// TODO: free token_name and allocate new mem
}
//private
void structToken_addChar(struct Token* self, char c){
	// TODO: if name_ptr > _name_len
	self->token_name[(self->_name_ptr)++] = c;
	self->token_name[(self->_name_ptr)] = '\0';
}
/***************** </Token> *************/

/***************** <TokenList> *************/
// private
//public
void structTokenList_init(struct TokenList* self){
	self->list  = (struct Token**)malloc( sizeof(struct Token) * TOKEN_LIST_SIZE );
	self->count = 0;
	self->size  = TOKEN_LIST_SIZE;
}
void structTokenList_addToken(struct TokenList* self, struct Token* token){
	if (self->count >= self->size){
		struct Token** new_list = (struct Token**)malloc( sizeof(struct Token)*(self->size + TOKEN_LIST_SIZE) ) ;
		self->size += TOKEN_LIST_SIZE;
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
	for (int i=0; i < arr_size; i++){
		if ( c == C_WHITE_SPACE[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharNumber(char c){
	size_t arr_size = ARR_SIZE(C_NUMBER);
	for (int i=0; i < arr_size; i++){
		if ( c == C_NUMBER[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharSymbol(char c){
	size_t arr_size = ARR_SIZE(C_SYMBOL);
	for (int i=0; i < arr_size; i++){
		if ( c == C_SYMBOL[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharBracket(char c){
	size_t arr_size = ARR_SIZE(C_BRACKET);
	for (int i=0; i < arr_size; i++){
		if ( c == C_BRACKET[i] ) return true;
	} return false;
}
bool structTokenScanner_isCharOperator(char c){
	size_t arr_size = ARR_SIZE(C_OPERATOR);
	for (int i=0; i<arr_size; i++){
		if (c == C_OPERATOR[i]) return true;
	} return false;
}
bool structTokenScanner_isCharIdentifier(char c){
	size_t arr_size = ARR_SIZE(C_IDENTIFIER);
	for (int i=0; i < arr_size; i++){
		if (c == C_IDENTIFIER[i]) return true;
	} return false;
}

// private
void structTokenScanner_checkIdentifier(struct TokenScanner* self){
	if (strcmp( self->current_token->token_name, KWORD_NULL )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_TRUE )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_FALSE )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_IF )==0)		{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_ELSE )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_WHILE )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWARD_FOR )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_BREAK )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_CONTINUE )==0){ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_AND )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_OR )==0)		{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_NOT )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_RETURN )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_STATIC )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_FUNCTION )==0){ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_CLASS )==0)	{ self->current_token->token_type = KEYWORD; return;}
	if (strcmp( self->current_token->token_name, KWORD_IMPORT )==0)	{ self->current_token->token_type = KEYWORD; return;}

	if (strcmp( self->current_token->token_name, DTYPE_VOID )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_BOOL )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_CAHR )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_SHORT )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_INT )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_LONG )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_FLOAT )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_DOUBLE )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_LIST )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_ARRAY )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_MAP )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_STRING )==0)	{ self->current_token->token_type = DTYPE; return;}
	if (strcmp( self->current_token->token_name, DTYPE_FUNC )==0)	{ self->current_token->token_type = DTYPE; return;}

	if (strcmp( self->current_token->token_name, BUILTIN_PRINT )==0){ self->current_token->token_type = BUILTIN; return;}
	if (strcmp( self->current_token->token_name, BUILTIN_INPUT )==0){ self->current_token->token_type = BUILTIN; return;}
	if (strcmp( self->current_token->token_name, BUILTIN_MIN )==0)	{ self->current_token->token_type = BUILTIN; return;}
	if (strcmp( self->current_token->token_name, BUILTIN_MAX )==0)	{ self->current_token->token_type = BUILTIN; return;}
	if (strcmp( self->current_token->token_name, BUILTIN_RAND )==0)	{ self->current_token->token_type = BUILTIN; return;}
}

// return true if EOF
bool structTokenScanner_skipWhiteSpaces(struct TokenScanner* self, int* pos){
	while (true){
		if ( *pos >= strlen(self->src) ) return true;
		char c = self->src[ *pos ];
		if (!structTokenScanner_isCharWhiteSpace(c)) return false;
		(*pos)++;
	}
}
bool structTokenScanner_skipComments(struct TokenScanner* self, int* pos){
	if ( *pos >= strlen(self->src) ) return true;
	char c = self->src[ *pos ];
	if (c == '/'){
		if ( (*pos)+1 >= strlen(self->src) ){ printf("Error: unexpected EOF\n"); exit(1);}
		char next = self->src[(*pos)+1];
		// //
		if (next == '/'){ 
			self->current_token->token_type = COMMENT;
			while(c != '\n'){
				structToken_addChar( self->current_token, c );
				(*pos)++;
				if ( (*pos) >= strlen(self->src) ) { printf("Error: unexpected EOF\n"); exit(1);}
				c = self->src[*pos];
			} return false;
		}

		// /**/
		if (next == '*'){
			structToken_addChar( self->current_token, '/' );structToken_addChar( self->current_token, '*' );
			self->current_token->token_type = COMMENT;(*pos)+=2;
			while (true){
				if ( (*pos)+1 >= strlen(self->src)  ) { printf("Error: unexpected EOF\n"); exit(1);}
				if ( self->src[*pos] == '*' && self->src[(*pos)+1] == '/' ){
					structToken_addChar( self->current_token, '*' );structToken_addChar( self->current_token, '/' );
					(*pos)+=2; return false;
				}
				c = self->src[*pos]; structToken_addChar( self->current_token, c );
				(*pos)++;

			}
		}

	}
	if (!structTokenScanner_isCharWhiteSpace(c)) return false;
	
}

bool structTokenScanner_isEof(struct TokenScanner* self, int* pos){
	if ( *pos >= strlen(self->src) ){
		return true;
	} return false;
}
void structTokenScanner_validateNumber(struct TokenScanner* self, int* pos){

	// invalid number
	char* numstr = self->current_token->token_name;
	int   numlen = strlen(numstr);
	int count = utils_char_count_in_str('.', numstr);
	if (count > 1){ if(self->src[ *pos ] == '\n') (*pos)--;
		char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
		printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
	}

	size_t arr_size;

	if (utils_char_in_str('x', numstr)){
		count = utils_char_count_in_str('x', numstr);
		char invalid_chars_hex[] = { '_', 'g',  'h',  'i',  'j',  'k',  'l', 'm',  'n',  'o',  'p',  'q',  'r',  't',  'u',  'v',  'w',  'y',  'z', 
		'A',  'B',  'C',  'D',  'E',  'F', 'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
		};
		arr_size = ARR_SIZE(invalid_chars_hex);
			for (int i=0; i<arr_size; i++){
			if (utils_char_in_str(invalid_chars_hex[i], numstr)) { if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		}
		if (count > 1 || numstr[1] != 'x' || numstr[0] != '0' || utils_char_in_str('.', numstr) ){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		char hexstring[10]; // TODO: 10
		strcpy(hexstring, numstr+2); // ignore 0x
		self->current_token->number_type 	= INT;
		self->current_token->number_value.i 	= (int)strtol(hexstring, NULL, 16);
		return;
	}

	if(utils_char_in_str('s', numstr)){
		count = utils_char_count_in_str('s', numstr);
		if (count > 1 || numstr[numlen-1] != 's' || utils_char_in_str('.', numstr)){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		self->current_token->number_type 	= SHORT;
		self->current_token->number_value.s 	= (short)atoi(numstr);
		return;
	}
	if(utils_char_in_str('l', numstr )){
		count = utils_char_count_in_str('l', numstr);
		if (count > 1 || numstr[numlen-1] != 'l' || utils_char_in_str('.', numstr)){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		self->current_token->number_type 		= LONG;
		self->current_token->number_value.l 	= atof(numstr);
		return;
	}
	if (utils_char_in_str('f', numstr)){
		count = utils_char_count_in_str('f', numstr);
		if (count > 1 || numstr[numlen-1] != 'f'){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		self->current_token->number_type 		= FLOAT;
		self->current_token->number_value.f 	= atof(numstr);
		return;
	}
	if (utils_char_in_str('d', numstr)){
		count = utils_char_count_in_str('d', numstr);
		if (count > 1 || numstr[numlen-1] != 'd'){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		self->current_token->number_type 		= DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return;
	}

	char invalid_chars[] = { '_', 'a',  'b',  'c',  'e',  'g',  'h',  'i',  'j',  'k',  'm',  'n',  'o',  'p',  'q',  'r',  't',  'u',  'v',  'w',  'y',  'z', 
	'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
	};
	arr_size = ARR_SIZE(invalid_chars);
	for (int i=0; i<arr_size; i++){
		if (utils_char_in_str(invalid_chars[i], numstr)) { if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: invalid number @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
	}

	if (utils_char_in_str('.', numstr)){
		self->current_token->number_type 	= DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return;
	} else {
		self->current_token->number_type 	= INT;
		self->current_token->number_value.i 	= atoi(numstr);
		return;
	}
}

// public
void structTokenScanner_init(struct TokenScanner* self, char* src, char* file_name){
	self->src = src;
	self->file_name = file_name;
	// self->current_token = current_token; // not initialized, use setToken
}
bool structTokenScanner_setToken(struct TokenScanner* self, struct Token* current_token){
	self->current_token = current_token;
}

// return true if eof
bool structTokenScanner_scaneToken(struct TokenScanner* self, int* pos){

	structToken_clear(self->current_token);
	bool eof = structTokenScanner_skipWhiteSpaces(self, pos); if (eof) return true;
	eof = structTokenScanner_skipComments(self, pos); if (eof) return true;
	char c = self->src[ *pos ];

	// identifier
	if (structTokenScanner_isCharIdentifier(c)){
		while (true){
			structToken_addChar( self->current_token, c ); (*pos)++;
			if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
				char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
				printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
			}
			c = self->src[ *pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || structTokenScanner_isCharSymbol(c) || structTokenScanner_isCharBracket(c) || structTokenScanner_isCharOperator(c) ){
				self->current_token->token_type = IDENTIFIER;
				structTokenScanner_checkIdentifier(self);
				return false;
			}
		}
	}


	// symbol
	if ( structTokenScanner_isCharSymbol(c) ){
		
		// string
		if (c == '"'){
			while(true){
				(*pos)++;
				if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
					char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
					printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
				}

				c = self->src[ *pos ];
				if (c == '\\'){
					(*pos)++;
					if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
						char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
						printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
					}
					c = self->src[*pos];
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
					self->current_token->token_type = STRING; (*pos)++;
					return false;
				}
				structToken_addChar( self->current_token, c );
			}
		}

		// character
		if (c == '\''){
			(*pos)++;
			c = self->src[*pos];
			if (c == '\\'){
				(*pos)++;
				if ( structTokenScanner_isEof(self, pos) ){
					char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
					printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1);
				}
				c = self->src[*pos];
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
					char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
					printf("Warning: unknown escaping @%s:%i\n%s\n", self->file_name, line_no, buff);
				}
			}
			structToken_addChar( self->current_token, c ); (*pos)++;
			self->current_token->token_type = NUMBER;
			self->current_token->number_type = CHAR;
			self->current_token->number_value.c = c;
			if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
				char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
				printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1);
			}
			if (self->src[(*pos)++] != '\''){  if(self->src[ *pos ] == '\n') (*pos)--;
				char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
				printf("Error: expected ' @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
			} 
			return false;
		}

		// symbol not a string
		structToken_addChar( self->current_token, c); (*pos)++;
		self->current_token->token_type = SYMBOL;
		return false;
	}

	// bracket
	if ( structTokenScanner_isCharBracket(c) ){
		structToken_addChar( self->current_token, c ); (*pos)++;		
		self->current_token->token_type = BRACKET;
		return false;
	}

	// operator TODO: negative numbers 
	if ( structTokenScanner_isCharOperator(c) ){
		structToken_addChar( self->current_token, c ); (*pos)++;
		self->current_token->token_type = OPERATOR;
		if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
			char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
			printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
		}
		char next = self->src[(*pos)++];
		if (c == '=' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '!' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '+' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '-' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '*' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '*' && next == '*'){ structToken_addChar( self->current_token, next); return false; }
		if (c == '/' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '<' && next == '<'){ structToken_addChar( self->current_token, next); return false; }
		if (c == '>' && next == '>'){ structToken_addChar( self->current_token, next); return false; }
		if (c == '>' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		if (c == '<' && next == '='){ structToken_addChar( self->current_token, next); return false; }
		(*pos)--;
		return false;
	}


	// number: negative numbers starts with -, can contain alphabat
	if ( structTokenScanner_isCharNumber(c) ){
		
		while (true){
			structToken_addChar( self->current_token, c ); (*pos)++;
			if ( structTokenScanner_isEof(self, pos) ){ if(self->src[ *pos ] == '\n') (*pos)--;
				char buff[ERROR_LINE_SIZE]; int line_no = utils_pos_to_line(*pos, self->src, buff);
				printf("Error: unexpected EOF @%s:%i\n%s\n", self->file_name, line_no, buff); exit(1); 
			}
			c = self->src[ *pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || (structTokenScanner_isCharSymbol(c) && c != '.' ) || structTokenScanner_isCharBracket(c) || structTokenScanner_isCharOperator(c) ){
				self->current_token->token_type = NUMBER;
				structTokenScanner_validateNumber(self, pos);
				return false;
			}
		}

	}


}

/***************** </TokenScanner> *************/