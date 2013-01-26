/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( LfgMgr );

LfgMgr::LfgMgr()
{
	MaxDungeonID = 0;
	uint32 levelgroup[2];
	levelgroup[0] = LFG_LEVELGROUP_NONE;
	levelgroup[1] = LFG_LEVELGROUP_NONE;
	DBCStorage<LookingForGroup>::iterator itr;
	for(itr = dbcLookingForGroup.begin(); itr != dbcLookingForGroup.end(); ++itr)
	{
		if((*itr)->ID > MaxDungeonID)
			MaxDungeonID = (*itr)->ID;

		levelgroup[0] = GetPlayerLevelGroup((*itr)->minlevel);
		if(levelgroup[0] != LFG_LEVELGROUP_NONE)
			DungeonsByLevel[levelgroup[0]].insert((*itr)->ID);

		levelgroup[1] = GetPlayerLevelGroup((*itr)->maxlevel);
		if(levelgroup[1] != LFG_LEVELGROUP_NONE)
			if(levelgroup[0] != levelgroup[1])
				DungeonsByLevel[levelgroup[1]].insert((*itr)->ID);

		levelgroup[0] = LFG_LEVELGROUP_NONE;
		levelgroup[1] = LFG_LEVELGROUP_NONE;
	}
}

LfgMgr::~LfgMgr()
{

}

void LfgMgr::LoadRandomDungeonRewards()
{
	uint32 count = 0;
	QueryResult* result = WorldDatabase.Query("SELECT * FROM lfd_rewards ORDER BY dungeonid");
	if(result != NULL)
	{
		do
		{
			Field* fields = result->Fetch();
			uint32 dungeonid = fields[0].GetUInt32();
			if(GetLFGReward(dungeonid))
				continue;

			uint32 questid[2], moneyreward[2], XPreward[2];
			questid[0] = questid[1] = moneyreward[0] = moneyreward[1] = XPreward[0] = XPreward[1] = 0;

			// First Reward
			questid[0] = fields[2].GetUInt32();
			moneyreward[0] = fields[3].GetUInt32();
			XPreward[0] = fields[4].GetUInt32();

			// Second reward
			questid[1] = fields[5].GetUInt32();
			moneyreward[1] = fields[6].GetUInt32();
			XPreward[1] = fields[7].GetUInt32();

			LfgReward* reward = new LfgReward(questid[0], moneyreward[0], XPreward[0], questid[1], moneyreward[1], XPreward[1]);
			DungeonRewards[dungeonid] = reward;
			count++;
		}while(result->NextRow());
	}

	Log.Notice("LfgMgr", "%u LFD rewards loaded.", count);
}

bool LfgMgr::AttemptLfgJoin(Player* pl, uint32 LfgDungeonId)
{
	return false;
}

uint32 LfgMgr::GetPlayerLevelGroup(uint32 level)
{
	if(level > 80)
		return LFG_LEVELGROUP_80_UP;
	else if(level == 80)
		return LFG_LEVELGROUP_80;
	else if(level >= 70)
		return LFG_LEVELGROUP_70_UP;
	else if(level >= 60)
		return LFG_LEVELGROUP_60_UP;
	else if(level >= 50)
		return LFG_LEVELGROUP_50_UP;
	else if(level >= 40)
		return LFG_LEVELGROUP_40_UP;
	else if(level >= 30)
		return LFG_LEVELGROUP_30_UP;
	else if(level >= 20)
		return LFG_LEVELGROUP_20_UP;
	else if(level >= 10)
		return LFG_LEVELGROUP_10_UP;

	return LFG_LEVELGROUP_NONE;
}

void LfgMgr::SetPlayerInLFGqueue(Player* pl,uint32 LfgDungeonId)
{

}

void LfgMgr::RemovePlayerFromLfgQueues(Player* pl)
{

}

void LfgMgr::RemovePlayerFromLfgQueue( Player* plr, uint32 LfgDungeonId )
{

}

void LfgMgr::UpdateLfgQueue(uint32 LfgDungeonId)
{

}

void LfgMgr::SendLfgList( Player* plr, uint32 Dungeon )
{

}

void LfgMgr::SetPlayerInLfmList(Player* pl, uint32 LfgDungeonId)
{

}

void LfgMgr::RemovePlayerFromLfmList(Player* pl, uint32 LfmDungeonId)
{

}
