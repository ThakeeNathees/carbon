#ifndef NAME_TABLE_H
#define NAME_TABLE_H

#include "ast/ast.h"

// TODO: create a new header for carbon object
struct CarbonObject
{

};


enum NameTableEntryType 
{
	NT_FUNCTION_DEFN,
	NT_CLASS_DEFN,
	NT_VAR,		// which is a ponter to carbon object
};
struct NameTableValue
{
	enum NameTableEntryType type;
	union {
		struct CarbonObject* obj;
		struct Statement* stmn; // can be func defn, class defn
	};
};


#endif 