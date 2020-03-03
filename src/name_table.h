#ifndef NAME_TABLE_H
#define NAME_TABLE_H

#include "ast/ast.h"

// TODO: create a new header for carbon object
// struct CarbonObject
// {
// 
// };

#define FOREACH_NAMETABLE_ENTRY_TYPE(func) \
	func(NT_UNKNOWN)		\
	func(NT_FUNCTION_DEFN)	\
	func(NT_CLASS_DEFN)		\
	func(NT_VAR)

enum NameTableEntryType 
{
	FOREACH_NAMETABLE_ENTRY_TYPE(GENERATE_ENUM)
};

struct NameTableEntry
{
	enum NameTableEntryType type;
	struct NameTableEntry* next;
	struct NameTableEntry* previous;
	struct Statement* stmn;
	
};

struct NameTable
{
	struct NameTableEntry* first_entry;
	struct NameTableEntry* last_entry;
};

/******************** <PUBLIC API> *****************************/
const char* enumNameTableEntry_toString(enum NameTableEntryType self );

// nametable
struct NameTable* structNameTable_new(); // static method
void structNameTable_init(struct NameTable* self);
void structNameTable_free(struct NameTable* self);
void structNameTable_addEntry( struct NameTable* self, struct NameTableEntry* entry);

// nametable entry
struct NameTableEntry* structNameTableEntry_new(); // static method
void structNameTableEntry_free( struct NameTableEntry* self );
/******************** </PUBLIC API> *****************************/

#endif 