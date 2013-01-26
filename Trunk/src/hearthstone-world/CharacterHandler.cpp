/***
 * Demonstrike Core
 */

#include "StdAfx.h"

bool ChatHandler::HandleRenameAllCharacter(const char * args, WorldSession * m_session)
{
	uint32 uCount = 0;
	uint32 ts = getMSTime();
	QueryResult * result = CharacterDatabase.Query("SELECT guid, name FROM characters");
	if( result )
	{
		do
		{
			uint32 uGuid = result->Fetch()[0].GetUInt32();
			const char * pName = result->Fetch()[1].GetString();
			size_t szLen = strlen(pName);

			if( !sWorld.VerifyName(pName, szLen) )
			{
				printf("renaming character %s, %u\n", pName,uGuid);
				Player* pPlayer = objmgr.GetPlayer(uGuid);
				if( pPlayer != NULL )
				{
					pPlayer->rename_pending = true;
					pPlayer->GetSession()->SystemMessage("Your character has had a force rename set, you will be prompted to rename your character at next login in conformance with server rules.");
				}

				CharacterDatabase.WaitExecute("UPDATE characters SET forced_rename_pending = 1 WHERE guid = %u", uGuid);
				++uCount;
			}

		} while (result->NextRow());
		delete result;
	}

	SystemMessage(m_session, "Procedure completed in %u ms. %u character(s) forced to rename.", getMSTime() - ts, uCount);
	return true;
}

void CapitalizeString(string& arg)
{
	if(arg.length() == 0) return;
	arg[0] = toupper(arg[0]);
	for(uint32 x = 1; x < arg.size(); ++x)
		arg[x] = tolower(arg[x]);
}

void WorldSession::CharacterEnumProc(QueryResult * result)
{
	uint8 num = 0;
	m_asyncQuery = false;

	//Erm, reset it here in case player deleted his DK.
	m_hasDeathKnight= false;

	// should be more than enough.. 200 bytes per char..
	WorldPacket data(SMSG_CHAR_ENUM, (result ? result->GetRowCount() * 200 : 1));

	// parse m_characters and build a mighty packet of
	// characters to send to the client.
	data << num;
	if( result )
	{
		CreatureInfo *info = NULL;
		player_item items[19];
		ItemPrototype * proto;
//		player_item bags[4];
		QueryResult * res;
		Field *fields;
		int8 slot;
		int8 containerslot;
		uint8 Class;
		uint8 race;
		uint32 i;
		uint32 bytes2;
		uint32 flags;
		uint32 banned;
		uint64 guid;
		do
		{
			fields = result->Fetch();
			guid = fields[0].GetUInt32();
			bytes2 = fields[6].GetUInt32();
			Class = fields[3].GetUInt8();
			flags = fields[17].GetUInt32();
			race = fields[3].GetUInt8();

			if( _side < 0 )
			{
				// work out the side
				static uint8 sides[RACE_DRAENEI+1] = { 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0 };
				_side = sides[race];
			}

			/* build character enum, w0000t :p */
			data << fields[0].GetUInt64();		// guid
			data << fields[7].GetString();		// name
			data << fields[2].GetUInt8();		// race
			data << race;						// class
			data << fields[4].GetUInt8();		// gender
			data << fields[5].GetUInt32();		// PLAYER_BYTES
			data << uint8(bytes2 & 0xFF);		// facial hair
			data << fields[1].GetUInt8();		// Level
			data << fields[12].GetUInt32();		// zoneid
			data << fields[11].GetUInt32();		// Mapid
			data << fields[8].GetFloat();		// X
			data << fields[9].GetFloat();		// Y
			data << fields[10].GetFloat();		// Z
			data << fields[18].GetUInt32();		// GuildID

			if( fields[1].GetUInt8() > m_highestLevel )
				m_highestLevel = fields[1].GetUInt8();

			if( Class == DEATHKNIGHT )
				m_hasDeathKnight = true;

			banned = fields[13].GetUInt32();
			if(banned && (banned<10 || banned > (uint32)UNIXTIME))
				data << uint32(0x01A04040);
			else
			{
				if(fields[16].GetUInt32() != 0)
					data << uint32(0x00A04342);
				else if(fields[15].GetUInt32() != 0)
					data << (uint32)8704; // Dead (displaying as Ghost)
				else
					data << uint32(1);		// alive
			}

			data << uint32(fields[19].GetUInt8());	// Character Customization
			data << fields[14].GetUInt8();			// Rest State

			if( Class == WARLOCK || Class == HUNTER )
			{
				res = CharacterDatabase.Query("SELECT entry FROM playerpets WHERE ownerguid="I64FMTD" AND ( active MOD 10 ) =1", guid);

				if(res)
				{
					info = CreatureNameStorage.LookupEntry(res->Fetch()[0].GetUInt32());
					delete res;
				}
				else
					info=NULL;
			}
			else
				info=NULL;

			if(info)  //PET INFO uint32 displayid,	uint32 level,		 uint32 familyid
				data << uint32(info->Male_DisplayID) << uint32(10) << uint32(info->Family);
			else
				data << uint32(0) << uint32(0) << uint32(0);

			res = CharacterDatabase.Query("SELECT containerslot, slot, entry, enchantments FROM playeritems WHERE ownerguid=%u", GUID_LOPART(guid));

			uint32 enchantid;
			EnchantEntry * enc;
			memset(items, 0, sizeof(player_item) * EQUIPMENT_SLOT_END);
			if(res)
			{
				do
				{
					containerslot = res->Fetch()[0].GetInt8();
					slot = res->Fetch()[1].GetInt8();
					if( containerslot == -1 && slot < EQUIPMENT_SLOT_END && slot >= 0 )
					{
						proto = ItemPrototypeStorage.LookupEntry(res->Fetch()[2].GetUInt32());
						if(proto)
						{
							// slot0 = head, slot14 = cloak
							if(!(slot == 0 && (flags & (uint32)PLAYER_FLAG_NOHELM) != 0) && !(slot == 14 && (flags & (uint32)PLAYER_FLAG_NOCLOAK) != 0))
							{
								items[slot].displayid = proto->DisplayInfoID;
								items[slot].invtype = proto->InventoryType;
								if( slot == EQUIPMENT_SLOT_MAINHAND || slot == EQUIPMENT_SLOT_OFFHAND )
								{
									// get enchant visual ID
									const char * enchant_field = res->Fetch()[3].GetString();
									if( sscanf( enchant_field , "%u,0,0;" , (unsigned int *)&enchantid ) == 1 && enchantid > 0 )
									{
										enc = dbcEnchant.LookupEntry( enchantid );
										if( enc != NULL )
											items[slot].enchantment = enc->visual;
										else
											items[slot].enchantment = 0;;
									}
								}
							}
						}
					}
				} while(res->NextRow());
				delete res;
			}

			for( i = 0; i < EQUIPMENT_SLOT_END; i++ )
				data << items[i].displayid << items[i].invtype << uint32(items[i].enchantment);

			for( i = 0; i < 4; i++)
				data << uint32(0)/*bags[i].displayid*/ << uint8(18) << uint32(0);
			//			Displayid						  // Bag	  // Enchant

			num++;
		}
		while( result->NextRow() );
	}

	data.put<uint8>(0, num);

	//OUT_DEBUG("Character Enum", "Built in %u ms.", getMSTime() - start_time);
	SendPacket( &data );
}

void WorldSession::HandleCharEnumOpcode( WorldPacket & recv_data )
{
	if( m_asyncQuery )		// should be enough
		return;

	AsyncQuery * q = new AsyncQuery( new SQLClassCallbackP1<World, uint32>(World::getSingletonPtr(), &World::CharacterEnumProc, GetAccountId()) );
	q->AddQuery("SELECT guid, level, race, class, gender, bytes, bytes2, name, positionX, positionY, positionZ, mapId, zoneId, banned, restState, deathstate, forced_rename_pending, player_flags, guild_data.guildid, customizable FROM characters LEFT JOIN guild_data ON characters.guid = guild_data.playerid WHERE acct=%u ORDER BY guid ASC LIMIT 10", GetAccountId());
	m_asyncQuery = true;
	CharacterDatabase.QueueAsyncQuery(q);
}

void WorldSession::LoadAccountDataProc(QueryResult * result)
{
	size_t len;
	const char * data;
	char * d;

	if(!result)
	{
		CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", _accountId);
		return;
	}

	for(uint32 i = 0; i < 7; i++)
	{
		data = result->Fetch()[1+i].GetString();
		len = data ? strlen(data) : 0;
		if(len > 1)
		{
			d = new char[len+1];
			memcpy(d, data, len+1);
			SetAccountData(i, d, true, (uint32)len);
		}
	}
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 10);
	std::string name;
	uint8 race, class_;

	recv_data >> name >> race >> class_;
	recv_data.rpos(0);

	WorldPacket data(SMSG_CHAR_CREATE, 1);
	if(!sWorld.VerifyName(name.c_str(), name.length()))
	{
		data << uint8(CHAR_CREATE_NAME_IN_USE);
		SendPacket(&data);
		return;
	}

	if(g_characterNameFilter->Parse(name, false))
	{
		data << uint8(CHAR_CREATE_NAME_IN_USE);
		SendPacket(&data);
		return;
	}

	//reserved for console whisper
	if(name == "Console" ||  name == "console")
	{
		data << uint8(CHAR_CREATE_NAME_IN_USE);
		SendPacket(&data);
		return;
	}

	if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
	{
		data << uint8(CHAR_CREATE_NAME_IN_USE);
		SendPacket(&data);
		return;
	}

	if(!sHookInterface.OnNewCharacter(race, class_, this, name.c_str()))
	{
		data << uint8(CHAR_CREATE_NAME_IN_USE);
		SendPacket(&data);
		return;
	}

	if( class_ == DEATHKNIGHT )
	{
		if(!HasFlag(ACCOUNT_FLAG_XPACK_02))
		{
			data << uint8(CHAR_CREATE_EXPANSION);
			SendPacket(&data);
			return;
		}
		else if(!CanCreateDeathKnight() && !HasGMPermissions())
		{
			if(sWorld.m_deathKnightReqLevel > m_highestLevel)
				data << uint8(CHAR_CREATE_LEVEL_REQUIREMENT);
			else
				data << uint8(CHAR_CREATE_UNIQUE_CLASS_LIMIT);
			SendPacket(&data);
			return;
		}
	}

	QueryResult * result = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
	if(result)
	{
		if(result->Fetch()[0].GetUInt32() > 0)
		{
			// That name is banned!
			data << uint8(CHAR_NAME_PROFANE);
			SendPacket(&data);
			delete result;
			return;
		}
		delete result;
	}
	// loading characters

	//checking number of chars is useless since client will not allow to create more than 10 chars
	//as the 'create' button will not appear (unless we want to decrease maximum number of characters)
	Player* pNewChar = objmgr.CreatePlayer();
	pNewChar->SetSession(this);
	if(!pNewChar->Create( recv_data ))
	{
		// failed.
		pNewChar->ok_to_remove = true;
		pNewChar->Destruct();
		pNewChar = NULLPLR;
		return;
	}

	pNewChar->UnSetBanned();
	pNewChar->addSpell(22027);	  // Remove Insignia

	if(pNewChar->getClass() == WARLOCK)
	{
		pNewChar->AddSummonSpell(416, 3110);		// imp fireball
		pNewChar->AddSummonSpell(417, 19505);
		pNewChar->AddSummonSpell(1860, 3716);
		pNewChar->AddSummonSpell(1863, 7814);
	}

	pNewChar->SaveToDB(true);

	PlayerInfo *pn = new PlayerInfo;
	memset(pn, 0, sizeof(PlayerInfo));
	pn->guid = pNewChar->GetLowGUID();
	pn->name = strdup(pNewChar->GetName());
	pn->_class = pNewChar->getClass();
	pn->race = pNewChar->getRace();
	pn->gender = pNewChar->getGender();
	pn->lastLevel = pNewChar->getLevel();
	pn->lastZone = pNewChar->GetZoneId();
	pn->lastOnline = UNIXTIME;
	pn->team = pNewChar->GetTeam();
	pn->acct = GetAccountId();
	objmgr.AddPlayerInfo(pn);

	pNewChar->ok_to_remove = true;
	pNewChar->Destruct();
	pNewChar = NULLPLR;

	// CHAR_CREATE_SUCCESS
	data << uint8(CHAR_CREATE_SUCCESS);
	SendPacket(&data);

	sLogonCommHandler.UpdateAccountCount(GetAccountId(), 1);
}

/* Last Checked at patch 3.0.2 Specfic SMSG_CHAR_CREATE Error Codes:

    RESPONSE_SUCCESS = 0,
    RESPONSE_FAILURE = 1,
    RESPONSE_CANCELLED = 2,
    RESPONSE_DISCONNECTED = 3,
    RESPONSE_FAILED_TO_CONNECT = 4,
    RESPONSE_VERSION_MISMATCH = 5,
    CSTATUS_CONNECTING = 6,
    CSTATUS_NEGOTIATING_SECURITY = 7,
    CSTATUS_NEGOTIATION_COMPLETE = 8,
    CSTATUS_NEGOTIATION_FAILED = 9,
    CSTATUS_AUTHENTICATING = 10,
    AUTH_OK = 11,
    AUTH_FAILED = 12,
    AUTH_REJECT = 13,
    AUTH_BAD_SERVER_PROOF = 14,
    AUTH_UNAVAILABLE = 15,
    AUTH_SYSTEM_ERROR = 16,
    AUTH_BILLING_ERROR = 17,
    AUTH_BILLING_EXPIRED = 18,
    AUTH_VERSION_MISMATCH = 19,
    AUTH_UNKNOWN_ACCOUNT = 20,
    AUTH_INCORRECT_PASSWORD = 21,
    AUTH_SESSION_EXPIRED = 22,
    AUTH_SERVER_SHUTTING_DOWN = 23,
    AUTH_ALREADY_LOGGING_IN = 24,
    AUTH_LOGIN_SERVER_NOT_FOUND = 25,
    AUTH_WAIT_QUEUE = 26,
    AUTH_BANNED = 27,
    AUTH_ALREADY_ONLINE = 28,
    AUTH_NO_TIME = 29,
    AUTH_DB_BUSY = 30,
    AUTH_SUSPENDED = 31,
    AUTH_PARENTAL_CONTROL = 32,
    AUTH_LOCKED_ENFORCED = 33,
    REALM_LIST_SUCCESS = 34,
    REALM_LIST_FAILED = 35,
    REALM_LIST_INVALID = 36,
    REALM_LIST_REALM_NOT_FOUND = 37,
    ACCOUNT_CREATE_IN_PROGRESS = 38,
    ACCOUNT_CREATE_SUCCESS = 39,
    ACCOUNT_CREATE_FAILED = 40,
    CHAR_LIST_RETRIEVED = 41,
    CHAR_LIST_FAILED = 42,
    CHAR_CREATE_IN_PROGRESS = 43,
    CHAR_CREATE_SUCCESS = 44,
    CHAR_CREATE_ERROR = 45,
    CHAR_CREATE_FAILED = 46,
    CHAR_CREATE_DISABLED = 47,
    CHAR_CREATE_PVP_TEAMS_VIOLATION = 48,
    CHAR_CREATE_SERVER_LIMIT = 49,
    CHAR_CREATE_ACCOUNT_LIMIT = 50,
    CHAR_CREATE_SERVER_QUEUE = 51,
    CHAR_CREATE_ONLY_EXISTING = 52,
    CHAR_CREATE_EXPANSION = 53,
    CHAR_CREATE_EXPANSION_CLASS = 54,
    CHAR_CREATE_LEVEL_REQUIREMENT = 55,
    CHAR_CREATE_UNIQUE_CLASS_LIMIT = 56,
    CHAR_DELETE_IN_PROGRESS = 57,
    CHAR_DELETE_SUCCESS = 58,0x3A
    CHAR_DELETE_FAILED = 59, 0x3B
    CHAR_DELETE_FAILED_LOCKED_FOR_TRANSFER = 60, 0x3C
    CHAR_DELETE_FAILED_GUILD_LEADER = 61, 0x3D
    CHAR_DELETE_FAILED_ARENA_CAPTAIN = 62, 0x3E
    CHAR_LOGIN_IN_PROGRESS = 63,
    CHAR_LOGIN_SUCCESS = 64,
    CHAR_LOGIN_NO_WORLD = 65,
    CHAR_LOGIN_DUPLICATE_CHARACTER = 66,
    CHAR_LOGIN_NO_INSTANCES = 67,
    CHAR_LOGIN_FAILED = 68,
    CHAR_LOGIN_DISABLED = 69,
    CHAR_LOGIN_NO_CHARACTER = 70,
    CHAR_LOGIN_LOCKED_FOR_TRANSFER = 71,
    CHAR_LOGIN_LOCKED_BY_BILLING = 72,
    CHAR_NAME_SUCCESS = 73,
    CHAR_NAME_FAILURE = 74,
    CHAR_NAME_NO_NAME = 75,
    CHAR_NAME_TOO_SHORT = 76,
    CHAR_NAME_TOO_LONG = 77,
    CHAR_NAME_INVALID_CHARACTER = 78,
    CHAR_NAME_MIXED_LANGUAGES = 79,
    CHAR_NAME_PROFANE = 80,
    CHAR_NAME_RESERVED = 81,
    CHAR_NAME_INVALID_APOSTROPHE = 82,
    CHAR_NAME_MULTIPLE_APOSTROPHES = 83,
    CHAR_NAME_THREE_CONSECUTIVE = 84,
    CHAR_NAME_INVALID_SPACE = 85,
    CHAR_NAME_CONSECUTIVE_SPACES = 86,
    CHAR_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS = 87,
    CHAR_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END = 88,
    CHAR_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME = 89,
*/

void WorldSession::HandleCharDeleteOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 8);
	uint8 fail = CHAR_DELETE_SUCCESS;

	uint64 guid;
	recv_data >> guid;

	if(objmgr.GetPlayer((uint32)guid) != NULL)
		fail = CHAR_DELETE_FAILED; // "Char deletion failed"
	else
		fail = DeleteCharacter((uint32)guid);

	OutPacket(SMSG_CHAR_DELETE, 1, &fail);
	if(fail == CHAR_DELETE_SUCCESS)
		sLogonCommHandler.UpdateAccountCount(GetAccountId(), -1);
}

uint8 WorldSession::DeleteCharacter(uint32 guid)
{
	PlayerInfo * inf = objmgr.GetPlayerInfo(guid);
	if( inf != NULL && inf->m_loggedInPlayer == NULL )
	{
		QueryResult * result = CharacterDatabase.Query("SELECT name FROM characters WHERE guid = %u AND acct = %u", (uint32)guid, _accountId);
		if(!result)
			return CHAR_DELETE_FAILED;

		if(inf->GuildId)
		{
			GuildMember* gMember = guildmgr.GetGuildMember(inf->guid);
			if(gMember->pRank->Rank.iId == 0)
				return CHAR_DELETE_FAILED_GUILD_LEADER;
			else
				guildmgr.RemoveMember(NULL, inf);
		}

		string name = result->Fetch()[0].GetString();
		delete result;

		for(int i = 0; i < NUM_CHARTER_TYPES; i++)
		{
			if( inf->charterId[i] != 0 )
			{
				Charter *pCharter = guildmgr.GetCharter(inf->charterId[i], (CharterTypes)i);
				if( pCharter->LeaderGuid == inf->guid )
					pCharter->Destroy();
				else
					pCharter->RemoveSignature(inf->guid);
			}
		}

		for(int i = 0; i < NUM_ARENA_TEAM_TYPES; i++)
		{
			if( inf->arenaTeam[i] != NULL )
			{
				if( inf->arenaTeam[i]->m_leader == guid )
					return CHAR_DELETE_FAILED_ARENA_CAPTAIN;
				else
					inf->arenaTeam[i]->RemoveMember(inf);
			}
		}

		/*if( _socket != NULL )
			sPlrLog.write("Account: %s | IP: %s >> Deleted player %s", GetAccountName().c_str(), GetSocket()->GetRemoteIP().c_str(), name.c_str());*/

		sWorld.LogPlayer(this, "deleted character %s (GUID: %u)", name.c_str(), (uint32)guid);

/*		if ( sWorld.char_restore_enabled )
		{
			//insert data into "deleted" tables
			CharacterDatabase.WaitExecute("INSERT INTO characters_deleted SELECT UNIX_TIMESTAMP(NOW()),characters.* FROM characters WHERE guid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO playerglyphs_deleted SELECT playerglyphs.* FROM playerglyphs WHERE guid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO playeritems_deleted SELECT playeritems.* FROM playeritems WHERE ownerguid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO playerskills_deleted SELECT playerskills.* FROM playerskills WHERE player_guid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO playerspells_deleted SELECT playerspells.* FROM playerspells WHERE guid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO playertalents_deleted SELECT playertalents.* FROM playertalents WHERE guid = %u", (uint32)guid);
			CharacterDatabase.WaitExecute("INSERT INTO questlog_deleted SELECT questlog.* FROM questlog WHERE player_guid = %u", (uint32)guid);
		}*/

		CharacterDatabase.WaitExecute("DELETE FROM characters WHERE guid = %u", (uint32)guid);

		Corpse* c=objmgr.GetCorpseByOwner((uint32)guid);
		if(c)
			CharacterDatabase.Execute("DELETE FROM corpses WHERE guid = %u", c->GetLowGUID());

		CharacterDatabase.Execute("DELETE FROM achievements WHERE player = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM auctions WHERE owner = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM charters WHERE leaderGuid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM guild_data WHERE playerid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM instances WHERE creator_guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM mailbox WHERE player_guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playerglyphs WHERE guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playeritems WHERE ownerguid=%u",(uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playerskills WHERE player_guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playerspells WHERE guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM playertalents WHERE guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid = %u", (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u OR friend_guid = %u", (uint32)guid, (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u OR ignore_guid = %u", (uint32)guid, (uint32)guid);
		CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerId = %u", (uint32)guid);

		/* remove player info */
		objmgr.DeletePlayerInfo((uint32)guid);
		return CHAR_DELETE_SUCCESS;
	}

	return CHAR_DELETE_FAILED;
}

void WorldSession::HandleCharRenameOpcode(WorldPacket & recv_data)
{
	WorldPacket data(SMSG_CHAR_RENAME, recv_data.size() + 1);

	uint64 guid;
	string name;
	recv_data >> guid >> name;

	PlayerInfo * pi = objmgr.GetPlayerInfo((uint32)guid);
	if(pi == NULL)
		return;

	QueryResult * result = CharacterDatabase.Query("SELECT forced_rename_pending FROM characters WHERE guid = %u AND acct = %u", (uint32)guid, _accountId);
	if(result == NULL)
		return;
	delete result;

	// Check name for rule violation.
	const char * szName=name.c_str();
	for(uint32 x = 0; x < strlen(szName); x++)
	{
		if((int)szName[x] < 65 || ((int)szName[x] > 90 && (int)szName[x] < 97) || (int)szName[x] > 122)
		{
			if((int)szName[x] < 65)
			{
				data << uint8(CHAR_NAME_TOO_SHORT); // Name is too short.
			}
			else if((int)szName[x] > 122) // Name is too long.
			{
				data << uint8(CHAR_NAME_TOO_LONG);
			}
			else
			{
				data << uint8(CHAR_NAME_FAILURE); // No clue.
			}
			data << guid << name;
			SendPacket(&data);
			return;
		}
	}

	QueryResult * result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
	if(result2)
	{
		if(result2->Fetch()[0].GetUInt32() > 0)
		{
			// That name is banned!
			data << uint8(CHAR_NAME_PROFANE);
			data << guid << name;
			SendPacket(&data);
			delete result2;
			return;
		}
		delete result2;
	}

	// Check if name is in use.
	if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
	{
		data << uint8(CHAR_NAME_FAILURE);
		data << guid << name;
		SendPacket(&data);
		return;
	}

	// correct capitalization
	CapitalizeString(name);
	objmgr.RenamePlayerInfo(pi, pi->name, name.c_str());

	sWorld.LogPlayer(this, "a rename was pending. Renamed character %s (GUID: %u) to %s.", pi->name, pi->guid, name.c_str());

	// If we're here, the name is okay.
	CharacterDatabase.Query("UPDATE characters SET name = \'%s\',  forced_rename_pending  = 0 WHERE guid = %u AND acct = %u",name.c_str(), (uint32)guid, _accountId);
	free(pi->name);
	pi->name = strdup(name.c_str());

	data << uint8(0) << guid << name;
	SendPacket(&data);
}


void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 8);
	uint64 playerGuid = 0;

	DEBUG_LOG( "WorldSession"," Recvd Player Logon Message" );

	recv_data >> playerGuid; // this is the GUID selected by the player

	//Better validate this Guid before we create an invalid _player.
	uint8 response = CHAR_LOGIN_NO_CHARACTER;

	//already active?
	if(objmgr.GetPlayer((uint32)playerGuid) != NULL || m_loggingInPlayer || _player)
		response = CHAR_LOGIN_DUPLICATE_CHARACTER;
	else //Do we exist in DB yet?
	{
		PlayerInfo * plrInfo = objmgr.GetPlayerInfo(playerGuid);
		if( plrInfo )
			response = CHAR_LOGIN_SUCCESS;
	}

	if(response != CHAR_LOGIN_SUCCESS)
	{
		OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &response);
		return;
	}

	//We have a valid Guid so let's create the player and login
	Player* plr = new Player((uint32)playerGuid);
	plr->Init();
	plr->SetSession(this);
	m_bIsWLevelSet = false;

	DEBUG_LOG("WorldSession", "Async loading player %u", (uint32)playerGuid);
	m_loggingInPlayer = plr;
	plr->LoadFromDB((uint32)playerGuid);
}

void WorldSession::FullLogin(Player* plr)
{
	DEBUG_LOG("WorldSession", "Fully loading player %u", plr->GetLowGUID());
	SetPlayer(plr);
	m_MoverWoWGuid.Init(plr->GetGUID());

	// copy to movement array
	plr->movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
	memcpy(&plr->movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());

	/* world preload */
	packetSMSG_LOGIN_VERIFY_WORLD vwpck;
	vwpck.MapId = plr->GetMapId();
	vwpck.O = plr->GetOrientation();
	vwpck.X = plr->GetPositionX();
	vwpck.Y = plr->GetPositionY();
	vwpck.Z = plr->GetPositionZ();
	OutPacket( SMSG_LOGIN_VERIFY_WORLD, sizeof(packetSMSG_LOGIN_VERIFY_WORLD), &vwpck );

	// send voicechat state - active/inactive
	/*
	{SERVER} Packet: (0x03C7) UNKNOWN PacketSize = 2
	|------------------------------------------------|----------------|
	|00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|
	|------------------------------------------------|----------------|
	|02 01							                 |..              |
	-------------------------------------------------------------------
	*/

	WorldPacket datab(SMSG_FEATURE_SYSTEM_STATUS, 2);
	datab << uint8(2) << uint8(0);
	SendPacket(&datab);

	datab.Initialize(SMSG_LEARNED_DANCE_MOVES, 8);
	datab << uint32(0);
	datab << uint32(0);
	SendPacket(&datab);

	plr->UpdateStats();

	// Anti max level hack.
	if(sWorld.LevelCap_Custom_All && (plr->getLevel() > sWorld.LevelCap_Custom_All))
		plr->SetUInt32Value(UNIT_FIELD_LEVEL, sWorld.LevelCap_Custom_All);

	// Enable certain GM abilities on login.
	if(HasGMPermissions())
	{
		plr->bGMTagOn = true;
		plr->m_isGmInvisible = true;
		plr->m_invisible = true;
		plr->bInvincible = true;
		if(CanUseCommand('z'))
		{
			plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_DEVELOPER);
			plr->triggerpass_cheat = true; // Enable for admins automatically.
		}
		else
			plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
	}

	// Make sure our name exists (for premade system)
	PlayerInfo * info = objmgr.GetPlayerInfo(plr->GetLowGUID());
	if(info == NULL)
	{
		info = new PlayerInfo;
		memset(info, 0, sizeof(PlayerInfo));
		info->_class = plr->getClass();
		info->gender = plr->getGender();
		info->guid = plr->GetLowGUID();
		info->name = strdup(plr->GetName());
		info->lastLevel = plr->getLevel();
		info->lastOnline = UNIXTIME;
		info->lastZone = plr->GetZoneId();
		info->race = plr->getRace();
		info->team = plr->GetTeam();
		objmgr.AddPlayerInfo(info);
	}

	plr->m_playerInfo = info;
	if(plr->m_playerInfo->GuildId)
	{
		plr->m_uint32Values[PLAYER_GUILDID] = plr->m_playerInfo->GuildId;
		plr->m_uint32Values[PLAYER_GUILDRANK] = plr->m_playerInfo->GuildRank;
	}

	for(uint32 z = 0; z < NUM_ARENA_TEAM_TYPES; ++z)
	{
		if(plr->m_playerInfo->arenaTeam[z] != NULL)
		{
			plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6), plr->m_playerInfo->arenaTeam[z]->m_id);
			if(plr->m_playerInfo->arenaTeam[z]->m_leader == plr->GetLowGUID())
				plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6) + 1, 0);
			else
				plr->SetUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (z*6) + 1, 1);
		}
	}

	info->m_loggedInPlayer = plr;

	// account data == UI config
	if(sWorld.m_useAccountData)
	{
		WorldPacket data(SMSG_ACCOUNT_DATA_TIMES, 4+1+4+8*4);
		MD5Hash md5hash;
		data << uint32(UNIXTIME) << uint8(1) << uint32(0xEA);
		for (int i = 0; i < 8; i++)
		{
			AccountDataEntry* acct_data = GetAccountData(i);
			if(0xEA & (1 << i))
				data << uint32(acct_data->Time);
			md5hash.Initialize();
			md5hash.UpdateData((const uint8*)acct_data->data, acct_data->sz);
			md5hash.Finalize();
		}
		SendPacket(&data);
	}
	else
	{
		WorldPacket data(SMSG_ACCOUNT_DATA_TIMES, 4+1+4+8*4);
		MD5Hash md5hash;
		data << uint32(UNIXTIME) << uint8(1) << uint32(0xEA);
		for (int i = 0; i < 8; i++)
		{
			if(0xEA & (1 << i))
				data << uint32(0);
			AccountDataEntry* acct_data = GetAccountData(i);
			if(acct_data)
			{
				md5hash.Initialize();
				md5hash.UpdateData((const uint8*)acct_data->data, acct_data->sz);
				md5hash.Finalize();
			}
		}
		SendPacket(&data);
	}

	// Set TIME OF LOGIN
	CharacterDatabase.Execute("UPDATE characters SET online = 1 WHERE guid = %u" , plr->GetLowGUID());

	bool enter_world = true;

	// Find our transporter and add us if we're on one.
	if(plr->m_TransporterGUID != 0)
	{
		Transporter* pTrans = objmgr.GetTransporter(GUID_LOPART(plr->m_TransporterGUID));
		if(pTrans)
		{
			if(plr->isDead())
			{
				plr->RemoteRevive();
			}

			float c_tposx = pTrans->GetPositionX() + plr->m_transportPosition->x;
			float c_tposy = pTrans->GetPositionY() + plr->m_transportPosition->y;
			float c_tposz = pTrans->GetPositionZ() + plr->m_transportPosition->z;
			if(plr->GetMapId() != pTrans->GetMapId())	// loaded wrong map
			{
				plr->SetMapId(pTrans->GetMapId());

				WorldPacket dataw(SMSG_NEW_WORLD, 20);
				dataw << pTrans->GetMapId() << c_tposx << c_tposy << c_tposz << plr->GetOrientation();
				SendPacket(&dataw);

				// shit is sent in worldport ack.
				enter_world = false;
			}

			plr->SetPosition(c_tposx, c_tposy, c_tposz, plr->GetOrientation(), false);
			plr->m_CurrentTransporter = pTrans;
			pTrans->AddPlayer(plr);
		}
	}

	if(plr->GetVehicle())
		plr->GetVehicle()->RemovePassenger(plr);

	DEBUG_LOG( "WorldSession","Player %s logged in.", plr->GetName());

	if(plr->GetTeam() == 1)
		sWorld.HordePlayers++;
	else
		sWorld.AlliancePlayers++;

	if(sWorld.SendMovieOnJoin && plr->m_FirstLogin && !HasGMPermissions())
		plr->SendCinematic(plr->myRace->cinematic_id);

	DEBUG_LOG( "WorldSession","Created new player for existing players (%s)", plr->GetName() );

	// Login time, will be used for played time calc
	plr->m_playedtime[2] = (uint32)UNIXTIME;

	// Send online status to people having this char in friendlist
	plr->Social_TellOnlineStatus();

	// send friend list (for ignores)
	plr->Social_SendFriendList(7);

	// send to gms
	if(HasGMPermissions())
		sWorld.SendMessageToGMs(this, "%s%s %s (%s) is now online.|r", MSG_COLOR_GOLD, CanUseCommand('z') ? "Admin" : "GameMaster", plr->GetName(), GetAccountNameS(), GetPermissions());

	//Set current RestState
	if( plr->m_isResting) 		// We are in a resting zone, turn on Zzz
		plr->ApplyPlayerRestState(true);

	//Check if there is a time difference between lastlogoff and now
	if( plr->m_timeLogoff > 0 && plr->GetUInt32Value(UNIT_FIELD_LEVEL) < plr->GetUInt32Value(PLAYER_FIELD_MAX_LEVEL))	// if timelogoff = 0 then it's the first login
	{
		uint32 currenttime = (uint32)UNIXTIME;
		uint32 timediff = currenttime - plr->m_timeLogoff;

		//Calculate rest bonus
		if( timediff > 0 )
			plr->AddCalculatedRestXP(timediff);
	}

	sHookInterface.OnFullLogin(plr);

	if(info->m_Group)
		info->m_Group->Update();

	if(!sWorld.m_blockgmachievements || !HasGMPermissions())
	{
		// Retroactive: Level achievement
		plr->GetAchievementInterface()->HandleAchievementCriteriaLevelUp( plr->getLevel() );

		// Send achievement data!
		if( plr->GetAchievementInterface()->HasAchievements() )
		{
			WorldPacket * data = plr->GetAchievementInterface()->BuildAchievementData();
			plr->CopyAndSendDelayedPacket(data);
			delete data;
		}
	}

	if(enter_world && !plr->GetMapMgr())
		plr->AddToWorld(true);

	sTracker.CheckPlayerForTracker(plr, true);

	// If we have the talent, it returns anyway, so just call the function.
	plr->ResetTitansGrip();

	if(plr->GetItemInterface())
		plr->GetItemInterface()->CheckAreaItems();
	objmgr.AddPlayer(plr);
}

bool ChatHandler::HandleRenameCommand(const char * args, WorldSession * m_session)
{
	// prevent buffer overflow
	if(strlen(args) > 100)
		return false;

	char name1[100];
	char name2[100];

	if(sscanf(args, "%s %s", name1, name2) != 2)
		return false;

	if(!sWorld.VerifyName(name2, strlen(name2)))
	{
		RedSystemMessage(m_session, "That name is invalid or contains invalid characters.");
		return true;
	}

	string new_name = name2;
	PlayerInfo * pi = objmgr.GetPlayerInfoByName(name1);
	if(pi == 0)
	{
		RedSystemMessage(m_session, "Player not found with this name.");
		return true;
	}

	if( objmgr.GetPlayerInfoByName(new_name.c_str()) != NULL )
	{
		RedSystemMessage(m_session, "Player found with this name in use already.");
		return true;
	}

	objmgr.RenamePlayerInfo(pi, pi->name, new_name.c_str());

	free(pi->name);
	pi->name = strdup(new_name.c_str());

	// look in world for him
	Player* plr = objmgr.GetPlayer(pi->guid);
	if(plr != 0)
	{
		plr->SetName(new_name);
		BlueSystemMessageToPlr(plr, "%s changed your name to '%s'.", m_session->GetPlayer()->GetName(), new_name.c_str());
		plr->SaveToDB(false);
	}
	else
	{
		CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s' WHERE guid = %u", CharacterDatabase.EscapeString(new_name).c_str(), (uint32)pi->guid);
	}

	GreenSystemMessage(m_session, "Changed name of '%s' to '%s'.", (char*)name1, (char*)name2);
	sWorld.LogGM(m_session, "renamed character %s (GUID: %u) to %s", (char*)name1, pi->guid, (char*)name2);
	sWorld.LogPlayer(m_session, "GM renamed character %s (GUID: %u) to %s", (char*)name1, pi->guid, ((char*)name2));
	return true;
}

void WorldSession::HandleAlterAppearance(WorldPacket & recv_data)
{
	OUT_DEBUG("WORLD: CMSG_ALTER_APPEARANCE");
	CHECK_PACKET_SIZE(recv_data, 12);

	uint32 hair, colour, facialhair;
	recv_data >> hair >> colour >> facialhair;

	BarberShopStyleEntry * Hair = dbcBarberShopStyle.LookupEntry(hair);
	BarberShopStyleEntry * facialHair = dbcBarberShopStyle.LookupEntry(facialhair);
	if(!facialHair || !Hair)
		return;

	uint8 newHair = Hair->hair_id;
	uint8 newFacialHair = facialHair->hair_id;
	uint32 level = _player->getLevel();
	float cost = 0;
	uint8 oldHair = _player->GetByte(PLAYER_BYTES, 2);
	uint8 oldColour = _player->GetByte(PLAYER_BYTES, 3);
	uint8 oldFacialHair = _player->GetByte(PLAYER_BYTES_2, 0);

	if(oldHair == newHair && oldColour == (uint8)colour && oldFacialHair == newFacialHair)
		return;

	if(level >= 100)
		level = 100;

	gtFloat *cutcosts = dbcBarberShopPrices.LookupEntry(level - 1);
	if(!cutcosts)
		return;

	WorldPacket data(SMSG_BARBER_SHOP_RESULT, 4);

	if(oldHair != newHair)
		cost += cutcosts->val;

    if((oldColour != colour) && (oldHair == newHair))
        cost += cutcosts->val * 0.5f;

    if(oldFacialHair != newFacialHair)
        cost += cutcosts->val * 0.75f;

	if(_player->GetUInt32Value(PLAYER_FIELD_COINAGE) < cost)
	{
		data << uint32(1); // not enough money
		SendPacket(&data);
		return;
	}

	data << uint32(0); // ok
	SendPacket(&data);
	_player->ModUnsigned32Value(PLAYER_FIELD_COINAGE, -(int32)cost);
	_player->SetByte(PLAYER_BYTES, 2, newHair);
	_player->SetByte(PLAYER_BYTES, 3, (uint8)colour);
	_player->SetByte(PLAYER_BYTES_2, 0, newFacialHair);
	_player->SetStandState(0);
}

void WorldSession::HandleCharCustomizeOpcode(WorldPacket & recv_data)
{
	WorldPacket data(SMSG_CHAR_CUSTOMIZE, recv_data.size() + 1);
	uint64 guid;
	string name;
	recv_data >> guid >> name;

	uint8 gender, skin, hairColor, hairStyle, facialHair, face;
	recv_data >> gender >> skin >> hairColor >> hairStyle >> facialHair >> face;

	uint32 playerGuid = uint32(guid);
	PlayerInfo* pi = objmgr.GetPlayerInfo(playerGuid);
	if( pi == NULL )
		return;

	QueryResult* result = CharacterDatabase.Query("SELECT bytes2 FROM characters WHERE guid = '%u'", playerGuid);
	if(!result)
		return;

	if(name != pi->name)
	{
		// Check name for rule violation.
		const char * szName = name.c_str();
		for(uint32 x = 0; x < strlen(szName); ++x)
		{
			if(int(szName[x]) || (int(szName[x]) > 90 && int(szName[x]) < 97) || int(szName[x]) > 122)
			{
				data << uint8(0x32);
				data << guid << name;
				SendPacket(&data);
				return;
			}
		}

		QueryResult * result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
		if(result2)
		{
			if(result2->Fetch()[0].GetUInt32() > 0)
			{
				// That name is banned!
				data << uint8(0x31);
				data << guid << name;
				SendPacket(&data);
				return;
			}
			delete result2;
		}

		// Check if name is in use.
		if(objmgr.GetPlayerInfoByName(name.c_str()) != 0)
		{
			data << uint8(0x32);
			data << guid << name;
			SendPacket(&data);
			return;
		}

		// correct capitalization
		CapitalizeString(name);
		objmgr.RenamePlayerInfo(pi, pi->name, name.c_str());
		// If we're here, the name is okay.
		free(pi->name);
		pi->name = strdup(name.c_str());

		CharacterDatabase.Execute("UPDATE characters SET name = '%s' WHERE guid = '%u'", CharacterDatabase.EscapeString(name).c_str(), playerGuid);
	}
	Field* fields = result->Fetch();
	uint32 player_bytes2 = fields[0].GetUInt32();
	player_bytes2 &= ~0xFF;
	player_bytes2 |= facialHair;
	CharacterDatabase.Execute("UPDATE characters SET gender = '%u', bytes = '%u', bytes2 = '%u', customizable = '0' WHERE guid = '%u'", gender, skin | (face << 8) | (hairStyle << 16) | (hairColor << 24), player_bytes2, playerGuid);
	delete result;

	//WorldPacket data(SMSG_CHAR_CUSTOMIZE, recv_data.size() + 1);
	data << uint8(0);
	data << guid;
	data << name;
	data << uint8(gender);
	data << uint8(skin);
	data << uint8(face);
	data << uint8(hairStyle);
	data << uint8(hairColor);
	data << uint8(facialHair);
	SendPacket(&data);
}
//#define EQ_MGR_TESTING
#ifdef EQ_MGR_TESTING

void WorldSession::HandleEquipmentSetSave(WorldPacket &recv_data)
{
	sLog.outDebug("CMSG_EQUIPMENT_SET_SAVE");
/*	WoWGuid guid;
	recv_data >> guid;

	uint32 index;
	recv_data >> index;
	if(index >= MAX_EQUIPMENT_SET_INDEX)	// client set slots amount
		return;

	std::string name, iconName;
	recv_data >> name >> iconName;

	EquipmentSet eqSet;

	eqSet.Guid		= guid.GetOldGuid();
	eqSet.Name		= name;
	eqSet.IconName	= iconName;
	eqSet.state		= EQUIPMENT_SET_NEW;

	for(uint32 i = 0; i < EQUIPMENT_SLOT_END; i++)
	{
		WoWGuid itemGuid;
		recv_data >> itemGuid;

		Item *item = _player->GetItemInterface()->GetInventoryItem(i);

		if(!item && itemGuid)	// cheating check 1
		{
			eqSet.Items[i] = 0; // Give em hell.
			continue;
		}

		if(item)
		{
			if(item->GetGUID() != itemGuid.GetOldGuid())	// cheating check 2
			{
				eqSet.Items[i] = 0; // Give em hell.
				continue;
			}
		}

		// Dirty business.
		eqSet.Items[i] = itemGuid;
	}

	_player->SetEquipmentSet(index, eqSet);*/
}

void WorldSession::HandleEquipmentSetDelete(WorldPacket &recv_data)
{
	sLog.outDebug("CMSG_EQUIPMENT_SET_DELETE");

/*	WoWGuid setGuid;
	recv_data >> setGuid;
	_player->DeleteEquipmentSet(setGuid.GetOldGuid());*/
}

void WorldSession::HandleEquipmentSetUse(WorldPacket &recv_data)
{
	sLog.outDebug("CMSG_EQUIPMENT_SET_USE");
/*	bool failed = false;
	for(uint32 i = 0; i < EQUIPMENT_SLOT_END; i++)
	{
		WoWGuid itemGuid;
		int8 SrcInvSlot;
		uint8 SrcSlot;
		recv_data >> itemGuid >> SrcInvSlot >> SrcSlot;
		uint64 guid = itemGuid.GetOldGuid();

		if(guid == NULL) // No need to change these, we just get feefee slot.
			continue;

		ItemInterface *ii = _player->GetItemInterface();
		Item* item = ii->GetItemByGUID(guid);
		if(item == NULL) // Item does not exist.
			continue;

		if(SrcSlot == (i))
			continue;

		uint16 slot = ii->GetBagSlotByGuid(guid);
		uint16 DstSlot = (i);
		uint16 DstInvSlot = INVENTORY_SLOT_NOT_SET; // Character :D

		ii->SwapItems(SrcInvSlot, DstInvSlot, slot, DstSlot);
	}

	WorldPacket data(SMSG_EQUIPMENT_SET_USE_RESULT, 1);
	data << uint8(0);	// 4 = failed(inventory is full)
	SendPacket(&data);*/
}

#else

void WorldSession::HandleEquipmentSetSave(WorldPacket &recv_data)
{

}

void WorldSession::HandleEquipmentSetDelete(WorldPacket &recv_data)
{

}

void WorldSession::HandleEquipmentSetUse(WorldPacket &recv_data)
{

}

#endif // EQ_MGR_TESTING
