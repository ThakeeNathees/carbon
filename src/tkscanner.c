
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tkscanner.h"

static const char* TOKEN_TYPE_STRING[] = {
	FOREACH_ENUM(GENERATE_STRING)
};

// public api
const char* enumTokenType_toString(enum TokenType self ){
	return TOKEN_TYPE_STRING[self];
}


/***************** <Token> *************/
// public
void structToken_init(struct Token* self){
	self->_name_len = TOKEN_NAME_SIZE;
	self->_name_ptr = 0;
	self->token_name = (char*)malloc(TOKEN_NAME_SIZE); // free after use
	for (int i=0; i<TOKEN_NAME_SIZE; i++){
		self->token_name[i] = 0;
	}
	self->token_type = UNKNOWN;
}
int structToken_toString(struct Token* self, char* buffer){
	return sprintf(buffer, "Token %s : %s", enumTokenType_toString(self->token_type), self->token_name);
}
void structToken_print(struct Token* self){
	printf("Token %s : %s\n", enumTokenType_toString(self->token_type), self->token_name);
}
//private
void structToken_addChar(struct Token* self, char c){
	// TODO: if name_ptr > _name_len
	self->token_name[(self->_name_ptr)++] = c;
}
/***************** </Token> *************/



/***************** <TokenScanner> *************/

// private
static const char C_WHITE_SPACE[] 	= { ' ', '\t', '\n', '\v', '\f', '\r' };
static const char C_NUMBER[]		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static const char C_SYMBOL[]     	= { '.', ',', ':', ';', '"', '\'' };			// symbols are only one character
static const char C_BRACKET[]		= { '(', ')', '{', '}', '[', ']', '<', '>'  };  // warning : tri bracket and bitwise shift are same
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
	}
	return false;
}
bool structTokenScanner_isCharNumber(char c){
	size_t arr_size = ARR_SIZE(C_NUMBER);
	for (int i=0; i < arr_size; i++){
		if ( c == C_NUMBER[i] ) return true;
	}
	return false;
}
bool structTokenScanner_isCharSymbol(char c){
	size_t arr_size = ARR_SIZE(C_SYMBOL);
	for (int i=0; i < arr_size; i++){
		if ( c == C_SYMBOL[i] ) return true;
	}
	return false;
}
bool structTokenScanner_isCharBracket(char c){
	size_t arr_size = ARR_SIZE(C_BRACKET);
	for (int i=0; i < arr_size; i++){
		if ( c == C_BRACKET[i] ) return true;
	}
	return false;
}
bool structTokenScanner_isCharIdentifier(char c){
	size_t arr_size = ARR_SIZE(C_IDENTIFIER);
	for (int i=0; i < arr_size; i++){
		if (c == C_IDENTIFIER[i]) return true;
	}
	return false;
}

// private
// return true if EOF
bool structTokenScanner_skipWhiteSpaces(struct TokenScanner* self, char* src, int* pos){
	while (true){
		if ( *pos >= strlen(src) ) return true;
		char c = src[ *pos ];
		if (!structTokenScanner_isCharWhiteSpace(c)) return false;
		(*pos)++;
	}
}

// public
void structTokenScanner_init(struct TokenScanner* self, char* src){
	self->src = src;
	structToken_init(&(self->current_token));
}

// return true if eof
bool structTokenScanner_scaneToken(struct TokenScanner* self, int* pos){
	
	bool eof = structTokenScanner_skipWhiteSpaces(self, self->src, pos);
	if (eof) return true;
	char c = self->src[ *pos ]; 
	//int pos_start = *pos;

	// identifier
	if (structTokenScanner_isCharIdentifier(c)){
		while (true){
			//self->current_token.token_name[ ((*pos)++) - pos_start ] = c;
			structToken_addChar( &(self->current_token), c ); (*pos)++;
			if ( *pos == strlen(self->src) ){ // end of file
				self->current_token.token_type = IDENTIFIER; // TODO: this may an error
				return true;
			}
			c = self->src[ *pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || structTokenScanner_isCharSymbol(c) || structTokenScanner_isCharBracket(c)){
				self->current_token.token_type = IDENTIFIER;
				return false;
			}
		}
	}


	// symbol
	if ( structTokenScanner_isCharSymbol(c) ){
		// string
		if (c == '"'){
		}

		//self->current_token.token_name[0] = c;
		structToken_addChar( &(self->current_token), c);
		self->current_token.token_type = SYMBOL;
		return false;
	}

	// bracket : special case < and >
	if ( structTokenScanner_isCharBracket(c) ){
		// TODO: check <,>
		structToken_addChar( &(self->current_token), c );
		self->current_token.token_type = BRACKET;
		return false;
	}


	// number: negative numbers starts with -, can contain alphabat


	/*
	*/


}

/***************** </TokenScanner> *************/