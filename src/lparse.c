#include <stdio.h>

#include "carbon_conf.h"
#include "lparse.h"

static const char* TOKEN_TYPE_STRING[] = {
	FOREACH_ENUM(GENERATE_STRING)
};

struct Token
{
	char* 			token_name;
	enum TokenType 	token_type;
};

// public api
const char* TokenType_toString(enum TokenType self ){
	return TOKEN_TYPE_STRING[self];
}

// private

int structToken_toString(struct Token self, char* buffer){
	return sprintf(buffer, "Token %s : %s", self.token_name, TOKEN_TYPE_STRING[self.token_type]);
}