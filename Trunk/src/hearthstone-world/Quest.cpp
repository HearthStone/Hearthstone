/***
 * Demonstrike Core
 */

#include "StdAfx.h"

//Packet Building
/////////////////

WorldPacket* WorldSession::BuildQuestQueryResponse(Quest *qst)
{
	// 2048 bytes should be more than enough. The fields cost ~200 bytes.
	// better to allocate more at startup than have to realloc the buffer later on.

	uint32 i = 0, total = 0;

	WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 248);
	*data << uint32(qst->id);																// Quest ID
	*data << uint32(2);																		// Unknown, always seems to be 2
	*data << int32(qst->qst_max_level);														// Quest level
	*data << uint32(qst->qst_min_level);													// minlevel !!!

	if(qst->qst_sort > 0)
		*data << int32(-(int32)qst->qst_sort);												// Negative if pointing to a sort.
	else
		*data << uint32(qst->qst_zone_id);													// Positive if pointing to a zone.

	*data << uint32(qst->qst_type);															// Info ID / Type
	*data << uint32(qst->qst_suggested_players);											// suggested players
	*data << uint32(qst->requirements ? qst->requirements->required_rep_faction : 0);		// Faction ID
	*data << uint32(qst->requirements ? qst->requirements->required_rep_value : 0);			// Faction Amount
	*data << uint32(0);																		// 3.3.3 // Opposite Faction ID
	*data << uint32(0);																		// 3.3.3 // Opposite Faction Amount
	*data << uint32(qst->qst_next_quest_id);												// Next Quest ID
	*data << uint32(0);																		// 3.3.0

	*data << uint32(sQuestMgr.GenerateRewardMoney(_player, qst));							// Copper reward
	*data << uint32(qst->objectives ? qst->objectives->required_money : 0);					// Required Money
	*data << uint32(qst->rewards ? qst->rewards->reward_spell : 0);							// Spell added to spellbook upon completion
	*data << uint32(qst->rewards ? qst->rewards->reward_cast_on_player : 0);				// Spell casted on player upon completion
	*data << uint32(qst->rewards ? qst->rewards->reward_honor : 0);							// bonus honor
	*data << float(0);																		// Reward Honor Multiplier
	*data << uint32(qst->rewards ? qst->rewards->srcitem : 0);								// Item given at the start of a quest (srcitem)
	*data << uint32(qst->qst_flags);														// Quest Flags
	*data << uint32(qst->rewards ? qst->rewards->reward_title : 0);							// Reward Title Id - Player is givn this title upon completion
	*data << uint32(qst->objectives ? qst->objectives->required_player_kills : 0);			// Required Kill Player
	*data << uint32(qst->rewards ? qst->rewards->reward_talents : 0);						// Reward Talents
	*data << uint32(qst->rewards ? qst->rewards->reward_arenapoints : 0);					// Arena Points
	*data << uint32(0);																		// unk

	if(qst->rewards == NULL)
	{
		total = (4*2)+(6*2)+(5*3);
		for(i = 0; i < total; i++)
			*data << uint32(0);
		total = 0;
	}
	else
	{
		// (loop 4 times)
		for(i = 0; i < 4; i++)
		{
			*data << uint32(qst->rewards->reward_item[i]);									// Forced Reward Item [i]
			*data << uint32(qst->rewards->reward_itemcount[i]);								// Forced Reward Item Count [i]
		}

		// (loop 6 times)
		for(uint32 i = 0; i < 6; i++)
		{
			*data << uint32(qst->rewards->reward_choiceitem[i]);							// Choice Reward Item [i]
			*data << uint32(qst->rewards->reward_choiceitemcount[i]);						// Choice Reward Item Count [i]
		}

		// Faction Reward Stuff.
		for(i = 0; i < 5; i++)
			*data << uint32(qst->rewards->reward_repfaction[i]);

		for(i = 0; i < 5; i++)
			*data << int32(qst->rewards->reward_repvalue[i]);

		for(i = 0; i < 5; i++)
			*data << int32(qst->rewards->reward_replimit[i]);
		// end
	}

	if(qst->objectives == NULL)
		*data << uint32(0) << float(0.0f) << float(0.0f) << uint32(0);
	else
	{
		*data << uint32(qst->objectives->required_point_mapid);								// Unknown
		*data << float(qst->objectives->required_point_x);									// Unknown
		*data << float(qst->objectives->required_point_y);									// Unknown
		*data << uint32(qst->objectives->required_point_radius);							// Unknown
	}

	*data << qst->qst_title;																// Title / name of quest
	*data << qst->qst_objectivetext;														// Objectives / description
	*data << qst->qst_details;																// Details
	*data << qst->qst_endtext;																// Subdescription

	*data << uint8(0);																		// Displayed after finishing quest.

	if(qst->objectives == NULL)
	{
		for(i = 0; i < 28; i++)
			*data << uint32(0);
		total = 0;
	}
	else
	{
		// (loop 4 times)
		for(uint32 i = 0; i < 4; i++)
		{
			*data << uint32(qst->objectives->required_mob[i]);								// Kill mob entry ID [i]
			*data << uint64(qst->objectives->required_mobcount[i]);							// Kill mob count [i]
			*data << int32(0);																// 3.0.2
		}

		for(uint32 i = 0; i < 6; i++)
		{
			*data << uint32(qst->objectives->required_item[i]);								// Collect item [i]
			*data << uint32(qst->objectives->required_itemcount[i]);						// Collect item count [i]
		}
	}

	*data << qst->qst_objectivetexts[0];													// Objective 1 - Used as text if mob not set
	*data << qst->qst_objectivetexts[1];													// Objective 2 - Used as text if mob not set
	*data << qst->qst_objectivetexts[2];													// Objective 3 - Used as text if mob not set
	*data << qst->qst_objectivetexts[3];													// Objective 4 - Used as text if mob not set

	return data;
}


/*****************
* QuestLogEntry *
*****************/
QuestLogEntry::QuestLogEntry()
{
	mInitialized = false;
	m_quest = NULL;
	mDirty = false;
	m_slot = -1;
	m_player_slain = 0;
	Quest_Status = QUEST_STATUS__INCOMPLETE;
}

QuestLogEntry::~QuestLogEntry()
{
	m_plr = NULLPLR;
	m_quest = NULL;
}

void QuestLogEntry::Init(Quest* quest, Player* plr, uint32 slot)
{
	ASSERT(quest);
	ASSERT(plr);

	m_quest = quest;
	m_plr = plr;
	m_slot = slot;
	m_time_left = 0;

	iscastquest = false;
	if(quest->objectives)
	{
		for (uint32 i = 0; i < 4; i++)
		{
			if (quest->objectives->required_spell[i] != 0)
			{
				iscastquest = true;
				if (!plr->HasQuestSpell(GetRequiredSpell(i)))
					plr->quest_spells.insert(GetRequiredSpell(i));
			}
			else if (quest->objectives->required_mob[i] != 0)
			{
				if (!plr->HasQuestMob(quest->objectives->required_mob[i]))
					plr->quest_mobs.insert(quest->objectives->required_mob[i]);
			}
		}

		m_time_left = m_quest->objectives->required_timelimit;
	}


	// update slot
	plr->SetQuestLogSlot(this, slot);

	mDirty = true;

	memset(m_mobcount, 0, 4*4);
	memset(m_explored_areas, 0, 4*4);
}

void QuestLogEntry::ClearAffectedUnits()
{
	if (m_affected_units.size()>0)
		m_affected_units.clear();
}
void QuestLogEntry::AddAffectedUnit(Unit* target)
{
	if (!target)
		return;
	if (!IsUnitAffected(target))
		m_affected_units.insert(target->GetGUID());
}
bool QuestLogEntry::IsUnitAffected(Unit* target)
{
	if (!target)
		return true;
	if (m_affected_units.find(target->GetGUID()) != m_affected_units.end())
		return true;
	return false;
}

void QuestLogEntry::SaveToDB(QueryBuffer * buf)
{
	ASSERT(m_slot != -1);
	if(!mDirty)
		return;

	//Made this into a replace not an insert
	//CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid=%u AND quest_id=%u", m_plr->GetGUIDLow(), m_quest->id);
	std::stringstream ss;
	ss << "REPLACE INTO questlog VALUES(";
	ss << m_plr->GetLowGUID() << "," << m_quest->id << "," << m_slot << "," << m_time_left;
	for(int i = 0; i < 4; i++)
		ss << "," << m_explored_areas[i];

	for(int i = 0; i < 4; i++)
		ss << "," << m_mobcount[i];

	ss << "," << m_player_slain;

	ss << ")";

	if( buf == NULL )
		CharacterDatabase.Execute( ss.str().c_str() );
	else
		buf->AddQueryStr(ss.str());
}

bool QuestLogEntry::LoadFromDB(Field *fields)
{
	// playerguid,questid,timeleft,area0,area1,area2,area3,kill0,kill1,kill2,kill3
	int f = 3;
	ASSERT(m_plr && m_quest);
	m_time_left = fields[f].GetUInt32();
	f++;
	for(int i = 0; i < 4; i++)
	{
		m_explored_areas[i] = fields[f++].GetUInt32();
		CALL_QUESTSCRIPT_EVENT(m_quest->id, OnExploreArea)(m_explored_areas[i], m_plr, this);
	}

	if(GetQuest()->objectives == NULL)
		f += 4;
	else
	{
		for(int i = 0; i < 4; i++)
		{
			m_mobcount[i] = fields[f++].GetUInt32();
			if(GetQuest()->objectives->required_mobtype[i] == QUEST_MOB_TYPE_CREATURE)
				CALL_QUESTSCRIPT_EVENT(m_quest->id, OnCreatureKill)(GetQuest()->objectives->required_mob[i], m_plr, this);
			else
				CALL_QUESTSCRIPT_EVENT(m_quest->id, OnGameObjectActivate)(GetQuest()->objectives->required_mob[i], m_plr, this);
		}
	}

	m_player_slain = fields[f].GetUInt32();
	mDirty = false;
	return true;
}

bool QuestLogEntry::CanBeFinished()
{
	if(m_quest->qst_is_repeatable == 2 && m_plr->GetFinishedDailiesCount() >= 25)
		return false;

	uint32 i = 0;
	if(m_quest->objectives)
	{
		for(i = 0; i < 4; i++)
		{
			bool required = false;
			if(iscastquest)
			{
				if(m_quest->objectives->required_spell[i] || m_quest->objectives->required_mob[i])
					required = true;
			}
			else
			{
				if(m_quest->objectives->required_mob[i])
					required = true;
			}

			if(required)
				if(m_quest->objectives->required_mobcount[i] > m_mobcount[i])
					return false;
		}

		for(i = 0; i < 6; i++)
			if(m_quest->objectives->required_item[i])
				if(m_plr->GetItemInterface()->GetItemCount(m_quest->objectives->required_item[i]) < m_quest->objectives->required_itemcount[i])
					return false;

		//Check for Gold & AreaTrigger Requirement s
		for(i = 0; i < 4; i++)
		{
			if(m_quest->objectives->required_money && (m_plr->GetUInt32Value(PLAYER_FIELD_COINAGE) < m_quest->objectives->required_money))
				return false;

			if(m_quest->objectives->required_areatriggers[i])
			{
				if(m_explored_areas[i] == 0)
					return false;
			}
		}

		if(m_quest->objectives->required_player_kills != m_player_slain)
			return false;

		if(m_quest->objectives->required_timelimit && !m_time_left)
			return false;
	}

	return true;
}

void QuestLogEntry::SubtractTime(uint32 value)
{
	if(m_time_left <= value)
		m_time_left = 0;
	else
		m_time_left-=value;
}

void QuestLogEntry::SetPlayerSlainCount(uint32 count)
{
	m_player_slain = count;
	mDirty = true;
}

void QuestLogEntry::SetMobCount(uint32 i, uint32 count)
{
	ASSERT(i<4);
	m_mobcount[i] = count;
	mDirty = true;
}

void QuestLogEntry::SetTrigger(uint32 i)
{
	ASSERT(i<4);
	m_explored_areas[i] = 1;
	mDirty = true;
}

void QuestLogEntry::SetSlot(int32 i)
{
	ASSERT(i!=-1);
	m_slot = i;
}

void QuestLogEntry::Finish()
{
	uint32 base = GetBaseField(m_slot);
	m_plr->SetUInt32Value(base + 0, 0);
	m_plr->SetUInt32Value(base + 1, 0);
	m_plr->SetUInt32Value(base + 2, 0);

	// clear from player log
	m_plr->SetQuestLogSlot(NULL, m_slot);
	m_plr->PushToRemovedQuests(m_quest->id);

	// delete ourselves
	delete this;
}

void QuestLogEntry::UpdatePlayerFields()
{
	if(!m_plr)
		return;

	m_plr->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + m_slot*5 + 0, m_quest->id);

	uint32 i = 0, field0 = 0x04;
	uint64 field1 = 0;

	// explored areas
	if(m_quest->objectives)
	{
		if(m_quest->objectives->count_requiredareatriggers)
		{
			uint32 count = 0;
			for(i = 0; i < 4; i++)
			{
				if(m_quest->objectives->required_areatriggers[i])
				{
					if(m_explored_areas[i] == 1)
					{
						count++;
					}
				}
			}

			if(count == m_quest->objectives->count_requiredareatriggers)
			{
				field1 |= 0x01000000;
			}
		}

		// mob hunting
		if(m_quest->objectives->count_required_mob)
		{
			// optimized this - burlex
			uint8 * p = (uint8*)&field1;
			for(i = 0; i < 4; i++)
			{
				if( m_quest->objectives->required_mob[i] && m_mobcount[i] > 0 )
					p[2*i] |= (uint8)m_mobcount[i];
			}
		}
	}

	if( Quest_Status == QUEST_STATUS__FAILED )
		field0 |= 2;
	else if(CanBeFinished() || Quest_Status == QUEST_STATUS__COMPLETE )
		field0 |= 0x01000001;

	m_plr->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + m_slot*5 + 1, field0);
	m_plr->SetUInt64Value(PLAYER_QUEST_LOG_1_1 + m_slot*5 + 2, field1);
	m_plr->SetUInt32Value(PLAYER_QUEST_LOG_1_1 + m_slot*5 + 4, ( m_time_left ? static_cast<uint32>(time(NULL) + (m_time_left/1000)) : 0 ));

	// Timed quest handler.
	if(m_time_left && !sEventMgr.HasEvent( m_plr,EVENT_TIMED_QUEST_EXPIRE ))
		m_plr->EventTimedQuestExpire(m_quest, this , m_slot, 1000);
}

void QuestLogEntry::SendQuestComplete()
{
	WorldPacket data(SMSG_QUESTUPDATE_COMPLETE, 4);
	data << m_quest->id;
	m_plr->GetSession()->SendPacket(&data);
	CALL_QUESTSCRIPT_EVENT(m_quest->id, OnQuestComplete)(m_plr, this);
}

void QuestLogEntry::SendUpdateAddKill(uint32 i)
{
	if(m_quest->objectives)
		sQuestMgr.SendQuestUpdateAddKill(m_plr, m_quest->id, m_quest->objectives->required_mob[i], m_mobcount[i], m_quest->objectives->required_mobcount[i], 0);
}

uint32 QuestLogEntry::GetRequiredSpell(uint32 i)
{
	if(m_quest->objectives == NULL)
		return 0;

	if(!m_plr->HasSpell(m_quest->objectives->required_spell[i]))
	{
		SpellEntry* reqspell = dbcSpell.LookupEntry(m_quest->objectives->required_spell[i]);
		 // Spell has a power type, so we check if player has spells with the same namehash, and replace it with that.
		if(reqspell && (reqspell->powerType != m_plr->GetPowerType()))
		{
			uint32 newspell = m_plr->FindSpellWithNamehash(reqspell->NameHash);
			if(newspell != 0)
				return newspell;
		}
	}

	return m_quest->objectives->required_spell[i];
}
