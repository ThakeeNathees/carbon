#include "carbon_conf.h"

static const char* ERROR_TYPE_STRING[] = {
	FOREACH_ERROR_TYPE(GENERATE_STRING)
};

// public api
const char* enumErrorType_toString(enum ErrorType self ){
	return ERROR_TYPE_STRING[self];
}

/***************** <String> *************/
void structString_init(struct String* self){
	self->buffer = (char*)malloc(STRING_BUFFER_SIZE);
	self->buffer[0] = '\0';
	self->buff_size = STRING_BUFFER_SIZE;
	self->buff_pos = 0;
}
void structString_minSize(struct String* self, int min_size){
	if (self->buff_size < min_size){
		char* new_buff = (char*)malloc(min_size);
		self->buff_size = min_size; self->buff_pos = 0;
		free(self->buffer);
		self->buffer = new_buff;
	}
}
void structString_free(struct String* self) {
	free(self->buffer);
	free(self);
}

struct String* structString_new(){
	struct String* new_str = (struct String*)malloc(sizeof(struct String));
	structString_init(new_str);
	return new_str;
}
/***************** </String> *************/

/***************** <CarbonError> *************/
void structCarbonError_init(struct CarbonError* self){
	self->type = ERROR_SUCCESS;
	structString_init( &(self->message) );
}
void structCarbonError_free(struct CarbonError* self) {
	free(self->message.buffer);
	free(self);
}
struct CarbonError* structCarbonError_new(){
	struct CarbonError* new_err = (struct CarbonError*)malloc(sizeof(struct CarbonError));
	structCarbonError_init(new_err);
	return new_err;
}
/***************** </CarbonError> *************/