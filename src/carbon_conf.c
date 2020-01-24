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

void structString_addChar(struct String* self, char c) {
	if (self->buff_size <= self->buff_pos + 1) {
		char* new_buff = (char*)malloc(self->buff_size + STRING_BUFFER_SIZE);
		for (int i = 0; i < self->buff_size; i++) {
			new_buff[i] = self->buffer[i];
		}
		self->buff_size += STRING_BUFFER_SIZE;
		free(self->buffer);
		self->buffer = new_buff;
	}
	self->buffer[self->buff_pos++] = c;
	self->buffer[self->buff_pos] = '\0';
}

void structString_strcat(struct String* self, const char* src) {
	size_t i = 0;
	char c = src[i++];
	while (c != '\0' && c != '\n') {
		structString_addChar(self, c);
		c = src[i++];
	}
	structString_addChar(self, c);

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