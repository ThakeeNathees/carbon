#ifndef CARBON_CONF_H
#define CARBON_CONF_H

// std includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ARR_SIZE(arr) sizeof(arr) / sizeof(*arr)

typedef unsigned char bool;
#define true  1
#define false 0

#define GENERATE_ENUM(enum_name) enum_name,
#define GENERATE_STRING(enum_name) #enum_name,

#define STRING_BUFFER_SIZE 100

struct String
{
	char* buffer;
	int buff_size;
	int buff_pos;
};

// Error success means no error
#define FOREACH_ERROR_TYPE(func) \
	func(ERROR_SUCCESS)\
	func(ERROR_INTERNAL)\
	func(ERROR_FILE_NOT_FOUND)\
	func(ERROR_UNEXP_EOF)\
	func(ERROR_IMPORT)\
	func(ERROR_SYNTAX)\
	func(ERROR_TYPE)\
	func(ERROR_INDEX)\
	func(ERROR_NAME)\
	func(ERROR_ZERO_DIVISION)

enum ErrorType
{
	FOREACH_ERROR_TYPE(GENERATE_ENUM)
};

struct CarbonError
{
	enum ErrorType type;
	struct String message;
};

// for windows dll define CARBON_BUILD_DLL, CARBON_LIB
#if defined(CARBON_BUILD_DLL)

	#if defined(CARBON_LIB)
		#define CARBON_API __declspec(dllexport)
	#else
		#define CARBON_API __declspec(dllimport)
	#endif

#else
	#define CARBON_API
#endif


/************** public api ******************/
const char* enumErrorType_toString(enum ErrorType self );

// string
void structString_init(struct String* self);
void structString_minSize(struct String* self, int min_size); // min size to sprinf
struct String* structString_new(); // static method

// carbon error
void structCarbonError_init(struct CarbonError* self);
struct CarbonError* structCarbonError_new(); // static method
/********************************************/


// CARBON_CONF_H
#endif



/********* include order ****************


[stdio.h, string.h, stdlib.h]
	|
	carbon_conf.h
	|
	utils.h 
	|
	tkscanner.h
	|
	ast.h
	|
	carbon.c


******************************************/