/***
 * Demonstrike Core
 */

#include "StdAfx.h"

//////////////////////////////////////////////////////////////
/// This function handles CMSG_NAME_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleNameQueryOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 8);
	uint64 guid;
	recv_data >> guid;

	PlayerInfo *pn = objmgr.GetPlayerInfo( (uint32)guid );
	if(pn == NULL)
		return;

	// We query our own name on player create so check to send MOTD
	if(!_player->sentMOTD)
	{
		_player->sendMOTD();
		_player->sentMOTD = true;
	}

	DEBUG_LOG("WorldSession","Received CMSG_NAME_QUERY for: %s", pn->name );
	WorldPacket data(SMSG_NAME_QUERY_RESPONSE, 10000);
	data << WoWGuid(guid);
	data << uint8(0);
	data << pn->name;
//	if(blablabla)
//		data << std::string("");
//	else
		data << uint8(0);
	data << uint8(pn->race);
	data << uint8(pn->gender);
	data << uint8(pn->_class);
	data << uint8(0);
	SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_QUERY_TIME:
//////////////////////////////////////////////////////////////
void WorldSession::HandleQueryTimeOpcode( WorldPacket & recv_data )
{
	WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 8);
	data << uint32(UNIXTIME);
	data << uint32(0);
	SendPacket(&data);
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_CREATURE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCreatureQueryOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 12);
	uint32 entry;
	uint64 guid;

	recv_data >> entry;
	recv_data >> guid;

	WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 150);
	if(entry == 300000)
	{
		data << (uint32)entry;
		data << "WayPoint";
		data << uint8(0) << uint8(0) << uint8(0);
		data << "Level is WayPoint ID";
		for(uint32 i = 0; i < 10; i++)
			data << uint32(0);
		data << float(0.0f);
		data << float(0.0f);
		data << uint8(0);
		for(uint32 i = 0; i < 7; i++)
			data << uint32(0);
		SendPacket( &data );
		return;
	}

	CreatureInfo* ci = CreatureNameStorage.LookupEntry(entry);
	if(ci == NULL)
		return;

	DEBUG_LOG("WORLD","HandleCreatureQueryOpcode CMSG_CREATURE_QUERY '%s'", ci->Name);
	data << entry;
	data << ci->Name;
	data << uint8(0) << uint8(0) << uint8(0);
	data << ci->SubName;
	data << ci->info_str; //!!! this is a string in 2.3.0 Example: stormwind guard has : "Direction"
	data << ci->Flags1;
	data << ci->Type;
	data << ci->Family;
	data << ci->Rank;
	data << ci->Unknown1;
	data << ci->SpellDataID;
	data << ci->Male_DisplayID;
	data << ci->Female_DisplayID;
	data << ci->Male_DisplayID2;
	data << ci->Female_DisplayID2;
	data << ci->unkfloat1;
	data << ci->unkfloat2;
	data << ci->Leader;
	CreatureQuestLoot* CtrQuestLoot = lootmgr.GetCreatureQuestLoot(entry);
	for(uint32 i = 0; i < 6; i++)
		data << uint32(CtrQuestLoot ? CtrQuestLoot->QuestLoot[i] : 0); // QuestItems
	data << uint32(0);	// CreatureMovementInfo.dbc
	SendPacket( &data );
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 12);
	WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 150);

	uint32 entryID;
	uint64 guid;
	recv_data >> entryID;
	recv_data >> guid;

	DEBUG_LOG("WORLD","HandleGameObjectQueryOpcode CMSG_GAMEOBJECT_QUERY '%u'", entryID);

	GameObjectInfo* goinfo = GameObjectNameStorage.LookupEntry(entryID);
	if(goinfo == NULL)
		return;

	data << entryID;
	data << goinfo->Type;
	data << goinfo->DisplayID;
	data << goinfo->Name;
	data << uint8(0);
	data << uint8(0);
	data << uint8(0);
	data << goinfo->Icon;
	data << goinfo->CastBarText;
	data << uint8(0);
	for(uint32 d = 0; d < 24; d++)
		data << goinfo->RawData.ListedData[d];
	data << float(1);
	uint32 i = 0;
	if(lootmgr.quest_loot_go.find(entryID) != lootmgr.quest_loot_go.end())
		for(std::set<uint32>::iterator itr = lootmgr.quest_loot_go[entryID].begin(); itr != lootmgr.quest_loot_go[entryID].end(), i < 6; itr++, i++)
			data << uint32(*itr);
	for(; i < 6; i++)
		data << uint32(0);			// itemId[6], quest drop

	SendPacket( &data );
}

void BuildCorpseInfo(WorldPacket* data, Corpse* corpse)
{
	MapInfo *pMapinfo = WorldMapInfoStorage.LookupEntry(corpse->GetMapId());
	if(pMapinfo == NULL || (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_PVP))
	{
		*data << uint8(0x01); //show ?
		*data << corpse->GetMapId(); // mapid (that tombstones shown on)
		*data << corpse->GetPositionX();
		*data << corpse->GetPositionY();
		*data << corpse->GetPositionZ();
		*data << corpse->GetMapId();
	}
	else
	{
		*data << uint8(0x01); //show ?
		*data << pMapinfo->repopmapid; // mapid (that tombstones shown on)
		*data << pMapinfo->repopx;
		*data << pMapinfo->repopy;
		*data << pMapinfo->repopz;
		*data << corpse->GetMapId(); //instance mapid (needs to be same as mapid to be able to recover corpse)
	}
}

//////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleCorpseQueryOpcode(WorldPacket &recv_data)
{
	OUT_DEBUG("WORLD: Received MSG_CORPSE_QUERY");

	Corpse *pCorpse= objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID());
	if(pCorpse == NULL)
		return;

	WorldPacket data(MSG_CORPSE_QUERY, 21);
	BuildCorpseInfo(&data, pCorpse);
	SendPacket(&data);
}

void WorldSession::HandlePageTextQueryOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 4);
	uint32 pageid = 0;
	uint64 itemguid;
	recv_data >> pageid;
	recv_data >> itemguid;

	WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 300);
	while(pageid)
	{
		data.clear();
		ItemPage * page = ItemPageStorage.LookupEntry(pageid);
		if(page == NULL)
			return;

		char* text = page->text;
		if(text == NULL || *text == NULL)
			return;

		data << pageid;
		data << text;
		data << page->next_page;
		pageid = page->next_page;
		SendPacket(&data);
	}
}

//////////////////////////////////////////////////////////////
/// This function handles CMSG_ITEM_NAME_QUERY:
//////////////////////////////////////////////////////////////
void WorldSession::HandleItemNameQueryOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 4);
	uint32 itemid;
	uint64 guid;
	recv_data >> itemid;
	recv_data >> guid;

	ItemEntry* itemE = dbcItem.LookupEntry(itemid);
	WorldPacket reply(SMSG_ITEM_NAME_QUERY_RESPONSE, 1000);
	ItemPrototype *proto = ItemPrototypeStorage.LookupEntry(itemid);

	reply << itemid;
	if(proto)
	{
		reply << proto->Name1;
		reply << uint32(proto->InventoryType);
	}	// Crow: This could all be done on server load by the way...
	else if(objmgr.ItemsInSets.find(itemid) != objmgr.ItemsInSets.end() && itemE != NULL)
	{
		std::list<ItemPrototype*>* protoset = objmgr.GetListForItemSet(objmgr.ItemsInSets.at(itemid));
		for(std::list<ItemPrototype*>::iterator itr = protoset->begin(); itr != protoset->end(); itr++)
		{
			if((*itr)->ItemSetRank == 1 && (*itr)->InventoryType == itemE->InventoryType)
				proto = (*itr);
		}

		if(proto != NULL)
		{
			reply << proto->Name1;
			reply << uint32(proto->InventoryType);
		}
		else
		{
			reply << "Unknown Item";
			reply << (itemE ? itemE->InventoryType : uint32(0));
		}
	}
	else
	{
		reply << "Unknown Item";
		reply << (itemE ? itemE->InventoryType : uint32(0));
	}

	SendPacket(&reply);
}

void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket & recv_data)
{
	CHECK_INWORLD_RETURN();

	WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 1000);
	Object::InRangeSet::iterator itr;
	Creature* pCreature;
	uint32 count = 0;
	data << count;
	for( itr = _player->m_objectsInRange.begin(); itr != _player->m_objectsInRange.end(); itr++ )
	{
		pCreature = TO_CREATURE(*itr);
		if( pCreature->GetTypeId() != TYPEID_UNIT )
			continue;

		if( pCreature->isQuestGiver() )
		{
			data << pCreature->GetGUID();
			data << sQuestMgr.CalcStatus( pCreature, _player );
			++count;
		}
	}

	data.put<uint32>(0, count);
	SendPacket(&data);
}