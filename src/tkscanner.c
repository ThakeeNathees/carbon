
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
	self->token_name[0] = '\0';
	self->token_type = UNKNOWN;
}
int structToken_toString(struct Token* self, char* buffer){
	return sprintf(buffer, "Token %s : %s", enumTokenType_toString(self->token_type), self->token_name);
}
void structToken_print(struct Token* self){
	printf("Token %10s : %s\n", enumTokenType_toString(self->token_type), self->token_name);
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



/***************** <TokenScanner> *************/

// private
static const char C_WHITE_SPACE[] 	= { ' ', '\t', '\n', '\v', '\f', '\r' };
static const char C_NUMBER[]		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
static const char C_SYMBOL[]     	= { '.', ',', ':', ';', '"', '\'' };			// symbols are only one character
static const char C_BRACKET[]		= { '(', ')', '{', '}', '[', ']', /*'<', '>'*/  };
static const char C_OPERATOR[] 		= { '+', '-', '*', '/', '%', '=', '<', '>', '&', '|', '^' };
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
		if ( (*pos)+1 >= strlen(self->src) ) return true; // TODO: ERROR
		char next = self->src[(*pos)+1];
		// //
		if (next == '/'){ 
			self->current_token.token_type = COMMENT;
			while(c != '\n'){
				(*pos)++;
				if ( (*pos) >= strlen(self->src) ) return true; // TODO: ERROR
				c = self->src[*pos];
			} return false;
		}

		// /**/
		if (next == '*'){
			self->current_token.token_type = COMMENT;(*pos)+=2;
			while (true){
				if ( (*pos)+1 >= strlen(self->src)  ) return true; // TODO: ERROR
				if ( self->src[*pos] == '*' && self->src[(*pos)+1] == '/' ){
					(*pos)+=2; return false;
				}
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

// public
void structTokenScanner_init(struct TokenScanner* self, char* src){
	self->src = src;
	structToken_init(&(self->current_token));
}

// return true if eof
bool structTokenScanner_scaneToken(struct TokenScanner* self, int* pos){

	// TODO: skip comments


	structToken_clear(&(self->current_token));
	bool eof = structTokenScanner_skipWhiteSpaces(self, pos); if (eof) return true;
	eof = structTokenScanner_skipComments(self, pos); if (eof) return true;
	char c = self->src[ *pos ];
	//exit(0);

	// identifier TODO: check if identifier is keyword
	if (structTokenScanner_isCharIdentifier(c)){
		while (true){
			structToken_addChar( &(self->current_token), c ); (*pos)++;
			if ( structTokenScanner_isEof(self, pos) ){ 
				// TODO: error print, exit
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
			while(true){
				(*pos)++;
				if ( structTokenScanner_isEof(self, pos) ){
					// TODO: error print, exit
					return true;
				}

				c = self->src[ *pos ];
				if (c == '\\'){
					(*pos)++;
					if ( structTokenScanner_isEof(self, pos) ){
						// TODO: error print, exit
						return true;
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
					else if (c == 't') c = '\t';
					else structToken_addChar( &(self->current_token), '\\' ); // \ is not escaping 

				} else if (c=='"'){
					self->current_token.token_type = STRING; (*pos)++;
					return false;
				}
				structToken_addChar( &(self->current_token), c );
			}
		}

		// symbol not a string
		structToken_addChar( &(self->current_token), c); (*pos)++;
		self->current_token.token_type = SYMBOL;
		return false;
	}

	// bracket : special case < and >
	if ( structTokenScanner_isCharBracket(c) ){
		structToken_addChar( &(self->current_token), c ); (*pos)++;		
		self->current_token.token_type = BRACKET;
		return false;
	}

	// operator TODO: negative numbers 
	if ( structTokenScanner_isCharOperator(c) ){
		structToken_addChar( &(self->current_token), c ); (*pos)++;
		self->current_token.token_type = OPERATOR;
		if ( structTokenScanner_isEof(self, pos) ){
			// TODO: error print, exit
			return true;
		}
		char next = self->src[(*pos)++];
		if (c == '=' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '+' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '-' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '*' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '/' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		//if (c == '/' && next == '/'){ structToken_addChar(&(self->current_token), next); return false; } // means comment not operator
		if (c == '<' && next == '<'){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '>' && next == '>'){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '>' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		if (c == '<' && next == '='){ structToken_addChar(&(self->current_token), next); return false; }
		(*pos)--; // single char operator
		return false;
	}


	// number: negative numbers starts with -, can contain alphabat
	if ( structTokenScanner_isCharNumber(c) ){
		
		while (true){
			structToken_addChar( &(self->current_token), c ); (*pos)++;
			if ( structTokenScanner_isEof(self, pos) ){ 
				// TODO: error print, exit
				return true;
			}
			c = self->src[ *pos ];
			if ( structTokenScanner_isCharWhiteSpace(c) || structTokenScanner_isCharSymbol(c) || structTokenScanner_isCharBracket(c) || structTokenScanner_isCharOperator(c) ){
				self->current_token.token_type = NUMBER;
				return false;
			}
		}

	}

	/*
	*/


}

/***************** </TokenScanner> *************/