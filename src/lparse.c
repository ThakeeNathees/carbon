#include <stdio.h>

#include "carbon_conf.h"
#include "lparse.h"

static const char* TOKEN_TYPE_STRING[] = {
	FOREACH_ENUM(GENERATE_STRING)
};

// public api
const char* enumTokenType_toString(enum TokenType self ){
	return TOKEN_TYPE_STRING[self];
}


struct Token
{
	char* 			token_name;
	enum TokenType 	token_type;
};

// private

int structToken_toString(struct Token self, char* buffer){
	return sprintf(buffer, "Token %s : %s", self.token_name, enumTokenType_toString(self.token_type));
}