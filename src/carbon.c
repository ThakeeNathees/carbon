#include <stdio.h>
#include "lparse.h"

int main(int argc, char** argv){

	enum TokenType ttype = SYMBOL;
	printf("%s\n", TokenType_toString(ttype));

	printf("main runned successfully!\n");
	return 0;
}