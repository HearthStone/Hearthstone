/***
 * Demonstrike Core
 */

#include "StdAfx.h"

/////////////////////////////////////////////////
// Database Modifying Commands
/////////////////////////////////////////////////

bool ChatHandler::HandleDBItemCreateCommand(const char* args, WorldSession *m_session)
{
	uint32 entry = atol(args);
	if(entry == 0)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto)
		RedSystemMessage(m_session, "That item already exists");
	else
	{
		BlueSystemMessage(m_session, "Created item %u", entry);
		WorldDatabase.Execute("INSERT INTO items(entry, name1) VALUES(%u, \"New Item\")", entry);
		ItemPrototypeStorage.Reload();
	}

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetClassCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, iclass = 0;
	if(sscanf(args, "%u %u", &entry, &iclass) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item class from %u to %u", proto->Class, iclass);
	proto->Class = iclass;
	WorldDatabase.Execute("UPDATE items SET class = '%u' WHERE entry = '%u'", iclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSubClassCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass = 0;
	if(sscanf(args, "%u %u", &entry, &subclass) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetNameCommand(const char* args, WorldSession *m_session)
{
	uint32 entry;
	char name[255];
	if(sscanf(args, "%u %s", &entry, &name) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item name from %s to %s", proto->Name1, (char*)name);
	proto->Name1 = name;
	WorldDatabase.Execute("UPDATE items SET name1 = '%s' WHERE entry = '%u'", (char*)name, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDisplayIdCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, display = 0;
	if(sscanf(args, "%u %u", &entry, &display) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item display id from %u to %u", proto->DisplayInfoID, display);
	proto->DisplayInfoID = display;
	WorldDatabase.Execute("UPDATE items SET displayid = '%u' WHERE entry = '%u'", display, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetQualityCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, quality = 0;
	if(sscanf(args, "%u %u", &entry, &quality) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item quality from %u to %u", proto->Quality, quality);
	proto->Quality = quality;
	WorldDatabase.Execute("UPDATE items SET quality = '%u' WHERE entry = '%u'", quality, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetFlagsCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, flags = 0;
	if(sscanf(args, "%u %u", &entry, &flags) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item flags from %u to %u", proto->Flags, flags);
	proto->Flags = flags;
	WorldDatabase.Execute("UPDATE items SET flags = '%u' WHERE entry = '%u'", flags, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetBuyPriceCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, buyprice = 0;
	if(sscanf(args, "%u %u", &entry, &buyprice) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item buy price from %u to %u", proto->BuyPrice, buyprice);
	proto->BuyPrice = buyprice;
	WorldDatabase.Execute("UPDATE items SET buyprice = '%u' WHERE entry = '%u'", buyprice, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSellPriceCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, sellprice = 0;
	if(sscanf(args, "%u %u", &entry, &sellprice) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item sell price from %u to %u", proto->SellPrice, sellprice);
	proto->SellPrice = sellprice;
	WorldDatabase.Execute("UPDATE items SET sellprice = '%u' WHERE entry = '%u'", sellprice, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetInventoryTypeCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, inventorytype = 0;
	if(sscanf(args, "%u %u", &entry, &inventorytype) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item inventory type from %u to %u", proto->InventoryType, inventorytype);
	proto->InventoryType = inventorytype;
	WorldDatabase.Execute("UPDATE items SET inventorytype = '%u' WHERE entry = '%u'", inventorytype, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetAllowableClassCommand(const char* args, WorldSession *m_session)
{
	uint32 entry;
	int32 allowableclass = 0;
	if(sscanf(args, "%u %i", &entry, &allowableclass) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	bool masked = false;
	if(allowableclass <= DRUID && allowableclass > 0) // MASK IT
	{
		masked = true;
		allowableclass = (1 << (allowableclass - 1));
	}

	if(allowableclass == -1)
	{
		m_lock.Acquire();
		BlueSystemMessage(m_session, "Changing item allowable class from %u to all classes", proto->AllowableClass);
		proto->AllowableClass = -1;
		WorldDatabase.Execute("UPDATE items SET allowableclass = '%i' WHERE entry = '%u'", -1, entry);
		m_lock.Release();
	}
	else if(masked && (!(proto->AllowableClass & allowableclass) || proto->AllowableClass == -1))
	{
		m_lock.Acquire();
		if(proto->AllowableClass == -1)
			proto->AllowableClass = 0;

		BlueSystemMessage(m_session, "Changing item allowable class from %i to %i", proto->AllowableClass, proto->AllowableClass | allowableclass);
		proto->AllowableClass |= allowableclass;
		WorldDatabase.Execute("UPDATE items SET allowableclass = '%i' WHERE entry = '%u'", proto->AllowableClass, entry);
		m_lock.Release();
	}
	else if(!masked)
	{
		m_lock.Acquire();
		BlueSystemMessage(m_session, "Changing item allowable class from %i to %i", proto->AllowableClass, allowableclass);
		proto->AllowableClass = allowableclass;
		WorldDatabase.Execute("UPDATE items SET allowableclass = '%i' WHERE entry = '%u'", allowableclass, entry);
		m_lock.Release();
	}
	else
		BlueSystemMessage(m_session, "Item allowable class: %i", proto->AllowableClass);

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetAllowableRaceCommand(const char* args, WorldSession *m_session)
{
	uint32 entry;
	int32 allowablerace = 0;
	if(sscanf(args, "%u %u", &entry, &allowablerace) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	bool masked = false;
	if(allowablerace <= DRUID && allowablerace > 0) // MASK IT
	{
		masked = true;
		allowablerace = (1 << (allowablerace - 1));
	}

	if(allowablerace == -1)
	{
		m_lock.Acquire();
		BlueSystemMessage(m_session, "Changing item allowable race from %u to all classes", proto->AllowableRace);
		proto->AllowableRace = -1;
		WorldDatabase.Execute("UPDATE items SET allowablerace = '%i' WHERE entry = '%u'", -1, entry);
		m_lock.Release();
	}
	else if(masked && (!(proto->AllowableRace & allowablerace) || proto->AllowableRace == -1))
	{
		m_lock.Acquire();
		if(proto->AllowableRace == -1)
			proto->AllowableRace = 0;

		BlueSystemMessage(m_session, "Changing item allowable race from %i to %i", proto->AllowableRace, proto->AllowableRace | allowablerace);
		proto->AllowableRace |= allowablerace;
		WorldDatabase.Execute("UPDATE items SET allowablerace = '%i' WHERE entry = '%u'", proto->AllowableRace, entry);
		m_lock.Release();
	}
	else if(!masked)
	{
		m_lock.Acquire();
		BlueSystemMessage(m_session, "Changing item allowable race from %i to %i", proto->AllowableRace, allowablerace);
		proto->AllowableRace = allowablerace;
		WorldDatabase.Execute("UPDATE items SET allowablerace = '%i' WHERE entry = '%u'", allowablerace, entry);
		m_lock.Release();
	}
	else
		BlueSystemMessage(m_session, "Item allowable class: %i", proto->AllowableRace);

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetItemLevelCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, itemlevel = 0;
	if(sscanf(args, "%u %u", &entry, &itemlevel) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item item level from %u to %u", proto->ItemLevel, itemlevel);
	proto->ItemLevel = itemlevel;
	WorldDatabase.Execute("UPDATE items SET itemlevel = '%u' WHERE entry = '%u'", itemlevel, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredLevelCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, requiredlevel = 0;
	if(sscanf(args, "%u %u", &entry, &requiredlevel) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item required level from %u to %u", proto->RequiredLevel, requiredlevel);
	proto->RequiredLevel = requiredlevel;
	WorldDatabase.Execute("UPDATE items SET requiredlevel = '%u' WHERE entry = '%u'", requiredlevel, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredSkillCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, requiredskill = 0;
	if(sscanf(args, "%u %u", &entry, &requiredskill) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item required skill from %u to %u", proto->RequiredSkill, requiredskill);
	proto->RequiredSkill = requiredskill;
	WorldDatabase.Execute("UPDATE items SET requiredskill = '%u' WHERE entry = '%u'", requiredskill, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredSkillRankCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, requiredskillrank = 0;
	if(sscanf(args, "%u %u", &entry, &requiredskillrank) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item required skill rank from %u to %u", proto->RequiredSkillRank, requiredskillrank);
	proto->RequiredSkillRank = requiredskillrank;
	WorldDatabase.Execute("UPDATE items SET requiredskillrank = '%u' WHERE entry = '%u'", requiredskillrank, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredSpellCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, requiredspell = 0;
	if(sscanf(args, "%u %u", &entry, &requiredspell) < 1)
		return false;

	if(!entry)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item required spell from %u to %u", proto->RequiredSpell, requiredspell);
	proto->RequiredSpell = requiredspell;
	WorldDatabase.Execute("UPDATE items SET requiredspell = '%u' WHERE entry = '%u'", requiredspell, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredRank1Command(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredRank2Command(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredFactionCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredFactionStandingCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetUniqueCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetMaxCountCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetContainerSlotsCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetStatTypeCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, index, stattype;
	if(sscanf(args, "%u %u %u", &entry, &index, &stattype) != 3)
		return false;

	if(index < 1 || index > 10)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item stat type %u from %u to %u", index, proto->Stats[index].Type, stattype);
	proto->Stats[index].Type = stattype;
	WorldDatabase.Execute("UPDATE items SET stat_type%u = '%u' WHERE entry = '%u'", index, stattype, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetStatValueCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, index, statvalue;
	if(sscanf(args, "%u %u %u", &entry, &index, &statvalue) != 3)
		return false;

	if(index < 1 || index > 10)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item stat value %u from %u to %u", index, proto->Stats[index].Value, statvalue);
	proto->Stats[index].Value = statvalue;
	WorldDatabase.Execute("UPDATE items SET stat_value%u = '%u' WHERE entry = '%u'", index, statvalue, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetScalingStatIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetScalingStatFlagsCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDamageMinCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDamageMaxCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDamageTypeCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetArmorCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetResistanceCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDelayCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetAmmoTypeCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRangeCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSpellInfoCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetBondingCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetDescriptionCommand(const char* args, WorldSession *m_session)
{
	RedSystemMessage(m_session, "Description changes are not allowed via commands.");
	return true;
}

bool ChatHandler::HandleDBItemSetPageIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetPageLanguageCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetPageMaterialCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetQuestIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetLockIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSheathIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRandomPropertyIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRandomSuffixIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetBlockCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetItemSetCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetMaxDurabilityCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetZoneNameIDCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetMapIdCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetBagFamilyCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetTotemCategoryCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSocketInfoCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetSocketBonusCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetGemPropertiesCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetRequiredDisenchantSkillCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetLootGoldCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}

bool ChatHandler::HandleDBItemSetArmorDamageModifierCommand(const char* args, WorldSession *m_session)
{
	uint32 entry, subclass;
	if(sscanf(args, "%u %u", &entry, &subclass) != 2)
		return false;

	ItemPrototype* proto = ItemPrototypeStorage.LookupEntry(entry);
	if(proto == NULL)
	{
		RedSystemMessage(m_session, "Item does not exist.");
		return true;
	}

	m_lock.Acquire();
	BlueSystemMessage(m_session, "Changing item subclass from %u to %u", proto->SubClass, subclass);
	proto->SubClass = subclass;
	WorldDatabase.Execute("UPDATE items SET subclass = '%u' WHERE entry = '%u'", subclass, entry);
	m_lock.Release();

	m_session->SendItemInfo(entry);
	return true;
}
