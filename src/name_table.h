#ifndef NAME_TABLE_H
#define NAME_TABLE_H

#include "ast/ast.h"

#define NAME_TABLE_SIZE 100

// TODO: create a new header for carbon object
struct CarbonObject
{

};

#define FOREACH_NAMETABLE_ENTRY_TYPE(func) \
	func(NT_FUNCTION_DEFN) \
	func(NT_CLASS_DEFN) \
	func(NT_VAR)

enum NameTableEntryType 
{
	FOREACH_NAMETABLE_ENTRY_TYPE(GENERATE_ENUM)
};

struct NameTableEntry
{
	enum NameTableEntryType type;
	union {
		struct CarbonObject* obj;
		struct Statement* stmn; // can be func defn, class defn
	};
};

struct NameTable
{
	unsigned int growth_size;
	size_t size;
	size_t count;
	struct NameTableEntry** entry_list;
};

/******************** <PUBLIC API> *****************************/
const char* enumNameTableEntry_toString(enum NameTableEntryType self );

// nametable
struct NameTable* structNameTable_new(); // static method
void structNameTable_init(struct NameTable* self, unsigned int growth_size);
void structNameTable_free();
/******************** </PUBLIC API> *****************************/

#endif 