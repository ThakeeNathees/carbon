
#include "tkscanner.h"

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
	self->_name_len 		 = TOKEN_NAME_SIZE;
	self->_name_ptr 		 = 0;
	self->name 				 = (char*)malloc(TOKEN_NAME_SIZE); // free after use
	self->name[0] 			 = '\0';
	self->type 				 = UNKNOWN;
	self->number_value.l 	 = 0;
	self->number_type 		 = INT;
	self->func_args_count    = 0;
	self->func_args_given    = 0;
	self->func_is_method     = false;
	self->idf_is_field       = false;
	self->minus_is_single_op = false;
}
void structToken_print(struct Token* self){
	if (self->type == NUMBER){
		if 		(self->number_type == CHAR)  printf("Token %10s : %-10s | (%s, %c)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.c);
		else if (self->number_type == SHORT) printf("Token %10s : %-10s | (%s, %i)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.s);
		else if (self->number_type == INT)   printf("Token %10s : %-10s | (%s, %i)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.i);
		else if (self->number_type == FLOAT) printf("Token %10s : %-10s | (%s, %f)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.f);
		else if (self->number_type == DOUBLE)printf("Token %10s : %-10s | (%s, %f)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.d);
		else if (self->number_type == LONG)  printf("Token %10s : %-10s | (%s, %ld)\n", enumTokenType_toString(self->type), self->name, enumNumberType_toString(self->number_type), self->number_value.l);	
	} 
	else if (self->type == FUNCTION || self->type == BUILTIN)  printf("Token %10s : %-10s | method=%i, args_given=%i\n", enumTokenType_toString(self->type), self->name, self->func_is_method, self->func_args_given);
	else if (self->type == OPERATOR && strcmp(self->name, OP_MINUS)==0)printf ("Token %10s : %-10s | single_op=%i\n", enumTokenType_toString(self->type), self->name, self->minus_is_single_op);
	else{
		printf("Token %10s : %s\n", enumTokenType_toString(self->type), self->name);
	}
}
void structToken_clear(struct Token* self){
	self->_name_ptr 	= 0;
	self->name[0] = '\0';
	self->type 	= UNKNOWN;
	// TODO: free name and allocate new mem
}
bool structToken_isAssignmentOperator(struct Token* self){
	if (self->type != OPERATOR){ return false; }
	if ( strcmp(self->name, OP_EQ)==0      || strcmp(self->name, OP_PLUSEQ)==0 ||  
		 strcmp(self->name, OP_MINUSEQ)==0 || strcmp(self->name, OP_MUEQ)==0   ||
		 strcmp(self->name, OP_DIVEQ)==0 ){
		return true;
	}
	return false;
}
//private
void structToken_addChar(struct Token* self, char c){
	
	if (self->_name_ptr >= self->_name_len-1){ // one space for '\0'
		int growth_size = TOKEN_NAME_SIZE; if (self->type == STRING) growth_size = TOKEN_STRING_GROWTH;
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
	if (strcmp( self->current_token->name, KWORD_NULL )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_SELF )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_TRUE )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_FALSE )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_IF )==0)		{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_ELSE )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_WHILE )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWARD_FOR )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWARD_FOREACH )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_BREAK )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_CONTINUE )==0){ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_AND )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_OR )==0)		{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_NOT )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_RETURN )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_STATIC )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_FUNCTION )==0){ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_CLASS )==0)	{ self->current_token->type = KEYWORD; return;}
	if (strcmp( self->current_token->name, KWORD_IMPORT )==0)	{ self->current_token->type = KEYWORD; return;}

	if (strcmp( self->current_token->name, DTYPE_VOID )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_BOOL )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_CAHR )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_SHORT )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_INT )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_LONG )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_FLOAT )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_DOUBLE )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_LIST )==0)	{ self->current_token->type = DTYPE; return;}
	//if (strcmp( self->current_token->name, DTYPE_ARRAY )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_MAP )==0)	{ self->current_token->type = DTYPE; return;}
	if (strcmp( self->current_token->name, DTYPE_STRING )==0)	{ self->current_token->type = DTYPE; return;}
	//if (strcmp( self->current_token->name, DTYPE_FUNC )==0)	{ self->current_token->type = DTYPE; return;}

	if (strcmp( self->current_token->name, BUILTIN_PRINT )==0)  { self->current_token->type = BUILTIN; self->current_token->func_args_count = 1; return;}
	if (strcmp( self->current_token->name, BUILTIN_INPUT )==0)  { self->current_token->type = BUILTIN; self->current_token->func_args_count = 1; return;}
	if (strcmp( self->current_token->name, BUILTIN_MIN )==0)	{ self->current_token->type = BUILTIN; self->current_token->func_args_count = 2; return;}
	if (strcmp( self->current_token->name, BUILTIN_MAX )==0)	{ self->current_token->type = BUILTIN; self->current_token->func_args_count = 2; return;}
	if (strcmp( self->current_token->name, BUILTIN_RAND )==0)	{ self->current_token->type = BUILTIN; self->current_token->func_args_count = 1; return;}
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
			self->current_token->type = COMMENT;
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
			self->current_token->type = COMMENT;(self->pos)+=2;
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
		if (structTokenScanner_isEof(self)) { *is_eof = true; return structCarbonError_new(); }

		c = self->src[ self->pos ];
		if (!structTokenScanner_isCharWhiteSpace(c)){*is_eof = false; return structCarbonError_new();}
		(self->pos)++;
	}
}

struct CarbonError* structTokenScanner_validateNumber(struct TokenScanner* self){

	// invalid number
	char* numstr = self->current_token->name;
	int   numlen = strlen(numstr);
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
			for (int i=0; i<arr_size; i++){
			if (utils_char_in_str(invalid_chars_hex[i], numstr)) { if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		}
		if (count > 1 || numstr[1] != 'x' || numstr[0] != '0' || utils_char_in_str('.', numstr) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		char hexstring[10]; // TODO: 10
		strcpy(hexstring, numstr+2); // ignore 0x
		self->current_token->number_type 	= INT;
		self->current_token->number_value.i 	= (int)strtol(hexstring, NULL, 16);
		return structCarbonError_new();
	}

	if(utils_char_in_str('s', numstr)){
		count = utils_char_count_in_str('s', numstr);
		if (count > 1 || numstr[numlen-1] != 's' || utils_char_in_str('.', numstr)){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->number_type 	= SHORT;
		self->current_token->number_value.s 	= (short)atoi(numstr);
		return structCarbonError_new();
	}
	if(utils_char_in_str('l', numstr )){
		count = utils_char_count_in_str('l', numstr);
		if (count > 1 || numstr[numlen-1] != 'l' || utils_char_in_str('.', numstr)){ if(self->src[self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->number_type 		= LONG;
		self->current_token->number_value.l 	= atof(numstr);
		return structCarbonError_new();
	}
	if (utils_char_in_str('f', numstr)){
		count = utils_char_count_in_str('f', numstr);
		if (count > 1 || numstr[numlen-1] != 'f'){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->number_type 		= FLOAT;
		self->current_token->number_value.f 	= atof(numstr);
		return structCarbonError_new();
	}
	if (utils_char_in_str('d', numstr)){
		count = utils_char_count_in_str('d', numstr);
		if (count > 1 || numstr[numlen-1] != 'd'){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
		self->current_token->number_type 		= DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return structCarbonError_new();
	}

	char invalid_chars[] = { '_', 'a',  'b',  'c',  'e',  'g',  'h',  'i',  'j',  'k',  'm',  'n',  'o',  'p',  'q',  'r',  't',  'u',  'v',  'w',  'y',  'z', 
	'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',
	};
	arr_size = ARR_SIZE(invalid_chars);
	for (int i=0; i<arr_size; i++){
		if (utils_char_in_str(invalid_chars[i], numstr)) { if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("SyntaxError: invalid number", ERROR_SYNTAX, self->current_token->pos, self->src, self->file_name, false);
		}
	}

	if (utils_char_in_str('.', numstr)){
		self->current_token->number_type 	= DOUBLE;
		self->current_token->number_value.d 	= strtod(numstr, NULL);
		return structCarbonError_new();
	} else {
		self->current_token->number_type 	= INT;
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
	if (*is_eof) return err;
	//*is_eof = structTokenScanner_skipWhiteSpaces(self); if(*is_eof) return structCarbonError_new();
	//*is_eof = structTokenScanner_skipComments(self); if (*is_eof) return structCarbonError_new();
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
				self->current_token->type = IDENTIFIER;
				structTokenScanner_checkIdentifier(self);
				return structCarbonError_new();
			}
		}
	}


	// symbol
	if ( structTokenScanner_isCharSymbol(c) ){
		
		// string
		if (c == '"'){
			self->current_token->type = STRING;
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
			self->current_token->type = NUMBER;
			self->current_token->number_type = CHAR;
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
		self->current_token->type = SYMBOL;
		return structCarbonError_new();
	}

	// bracket
	if ( structTokenScanner_isCharBracket(c) ){
		structToken_addChar( self->current_token, c ); (self->pos)++;		
		self->current_token->type = BRACKET;
		return structCarbonError_new();
	}

	// operator TODO:
	if ( structTokenScanner_isCharOperator(c) ){
		structToken_addChar( self->current_token, c ); (self->pos)++;
		self->current_token->type = OPERATOR;
		if ( structTokenScanner_isEof(self) ){ if(self->src[ self->pos ] == '\n') (self->pos)--;
			return utils_make_error("EofError: unexpected EOF", ERROR_UNEXP_EOF, self->current_token->pos, self->src, self->file_name, false);
		}
		char next = self->src[(self->pos)++];
		if (c == '=' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '!' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '+' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '-' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '*' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '*' && next == '*'){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '/' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '<' && next == '<'){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '>' && next == '>'){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '>' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
		if (c == '<' && next == '='){ structToken_addChar( self->current_token, next); return structCarbonError_new(); }
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
				self->current_token->type = NUMBER;
				return structTokenScanner_validateNumber(self);
			}
		}

	}
}

struct TokenScanner* structTokenScanner_new(char* src, char* file_name){
	struct TokenScanner* token_scanner = (struct TokenScanner*)malloc( sizeof(struct TokenScanner) );
	structTokenScanner_init(token_scanner, src, file_name);
	return token_scanner;
}

/***************** </TokenScanner> *************/