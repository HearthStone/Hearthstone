/***
 * Demonstrike Core
 */

//
// NameTables.h
//

#pragma once

struct NameTableEntry
{
	uint32 id;
	const char *name;
};

static inline const char* LookupName(uint32 id, NameTableEntry *table)
{
	for(uint32 i = 0; table[i].name != 0; i++)
	{
		if (table[i].id == id)
			return table[i].name;
	}

	return "UNKNOWN";
}

//extern NameTableEntry g_worldOpcodeNames[];
extern NameTableEntry g_logonOpcodeNames[];
extern NameTableEntry g_pluginOpcodeNames[];
