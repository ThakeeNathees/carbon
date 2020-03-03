#include "name_table.h"

static const char* NAMETABLE_ENTRY_TYPE_STRING[] = {
	FOREACH_NAMETABLE_ENTRY_TYPE(GENERATE_STRING)
};

// public api
const char* enumNameTableEntry_toString(enum NameTableEntryType self ){
	return NAMETABLE_ENTRY_TYPE_STRING[self];
}

/***************** <NameTable> *************/
void structNameTable_init(struct NameTable* self){
    self->first_entry = NULL;
    self->last_entry = NULL;
}
struct NameTable* structNameTable_new(){
    struct NameTable* name_table = (struct NameTable*) malloc(sizeof(struct NameTable));
    structNameTable_init(name_table);
    return name_table;
}

void structNameTable_addEntry( struct NameTable* self, struct NameTableEntry* entry) {
    if (self->first_entry == NULL) {
        self->first_entry = entry;
        self->last_entry = entry;
        return;
    }
    self->last_entry->next = entry;
    entry->previous = self->last_entry;
    self->last_entry = entry;
}

void structNameTable_free(struct NameTable* self){
    // TODO: 
}
/***************** </NameTable> *************/


/***************** <NameTableEntry> *************/
void structNameTableEntry_init(struct NameTableEntry* self) {
    self->next = NULL;
    self->previous = NULL;
    self->stmn = NULL;
    self->type = NT_UNKNOWN;
}

struct NameTableEntry* structNameTableEntry_new() { // static method
    struct NameTableEntry* entry = (struct NameTableEntry*) malloc( sizeof( struct NameTableEntry ) );
    structNameTableEntry_init(entry);
    return entry;
}

void structNameTableEntry_free(struct NameTableEntry* self) {
    // TODO:
}
/***************** </NameTableEntry> *************/