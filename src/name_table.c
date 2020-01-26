#include "name_table.h"

static const char* NAMETABLE_ENTRY_TYPE_STRING[] = {
	FOREACH_NAMETABLE_ENTRY_TYPE(GENERATE_STRING)
};

// public api
const char* enumNameTableEntry_toString(enum NameTableEntryType self ){
	return NAMETABLE_ENTRY_TYPE_STRING[self];
}

/***************** <NameTable> *************/
void structNameTable_init(struct NameTable* self, unsigned int growth_size){
    self->growth_size = growth_size;
    self->count = 0;
    self->size = self->growth_size;
    self->entry_list = (struct NameTable**) malloc(sizeof(struct NameTable) * self->size );
}
struct NameTable* structNameTable_new(){
    struct NameTable* name_table = (struct NameTable*) malloc(sizeof(struct NameTable));
    structNameTable_init(name_table, NAME_TABLE_SIZE);
    return name_table;
}
void structNameTable_free(){
    // TODO: 
}
/***************** </NameTable> *************/