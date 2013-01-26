/***
 * Demonstrike Core
 */

#include "StdAfx.h"

void WorldSession::HandleLFDPlrLockOpcode( WorldPacket& recv_data )
{
	CHECK_INWORLD_RETURN();

	DungeonSet::iterator itr;
	DungeonSet randomDungeonSet;
	LookingForGroup* dungeon = NULL;
	uint32 level = _player->getLevel();
	DungeonSet LevelDungeonSet = sLfgMgr.GetLevelSet(level);
	for (itr = LevelDungeonSet.begin(); itr != LevelDungeonSet.end(); itr++)
	{
		dungeon = dbcLookingForGroup.LookupEntry(*itr);
		if (dungeon != NULL && dungeon->type == LFG_RANDOM
			&& dungeon->expansion <= GetHighestExpansion()
			&& dungeon->minlevel <= level && level <= dungeon->maxlevel)
			randomDungeonSet.insert(dungeon->ID);
		dungeon = NULL;
	}

	// Crow: Confirmed structure below
	WorldPacket data(SMSG_LFD_PLAYER_LOCK_INFO_RESPONSE, 400);
	uint8 randomsize = (uint8)randomDungeonSet.size();
	data << randomsize;
	for(itr = randomDungeonSet.begin(); itr != randomDungeonSet.end(); itr++)
	{
		dungeon = dbcLookingForGroup.LookupEntry(*itr);
		data << uint32(dungeon->GetEntry());

		uint8 done = 0;
		Quest* QuestReward = NULL;
		LfgReward* reward = sLfgMgr.GetLFGReward(*itr);
		if(reward)
		{
			QuestReward = sQuestMgr.GetQuestPointer(reward->reward[0].QuestId);
			if(QuestReward)
			{
				done = _player->HasFinishedQuest(reward->reward[0].QuestId);
				if(!done)
					done = _player->HasFinishedDailyQuest(reward->reward[0].QuestId);
				if (done)
					QuestReward = sQuestMgr.GetQuestPointer(reward->reward[1].QuestId);
			}
		}

		if (QuestReward)
		{
			data << uint8(done);
			data << uint32(sQuestMgr.GenerateRewardMoney(_player, QuestReward));
			data << uint32(sQuestMgr.GenerateQuestXP(_player, QuestReward)*sWorld.getRate(RATE_QUESTXP));
			data << uint32(reward->reward[done].MoneyReward);
			data << uint32(reward->reward[done].XPReward);
			if(QuestReward->rewards == NULL)
			{
				data << uint8(0);
			}
			else
			{
				data << uint8(QuestReward->rewards->count_reward_item);
				if (QuestReward->rewards->count_reward_item)
				{
					ItemPrototype* proto = NULL;
					for (uint8 i = 0; i < 4; i++)
					{
						if (!QuestReward->rewards->reward_item[i])
							continue;

						proto = ItemPrototypeStorage.LookupEntry(QuestReward->rewards->reward_item[i]);
						data << uint32(QuestReward->rewards->reward_item[i]);
						data << uint32(proto ? proto->DisplayInfoID : 0);
						data << uint32(QuestReward->rewards->reward_itemcount[i]);
					}
				}
			}
		}
		else
		{
			data << uint8(0);
			data << uint32(0);
			data << uint32(0);
			data << uint32(0);
			data << uint32(0);
			data << uint8(0);
		}
	}
	SendPacket(&data);
}

void WorldSession::HandleLFDPartyLockOpcode( WorldPacket& recv_data )
{
	// Crow: Confirmed structure below
	WorldPacket data(SMSG_LFD_PARTY_LOCK_INFO_UPDATE, 400);
	uint8 cnt = 0;
	data << uint8(cnt);
	for(uint8 i = 0; i < cnt; i++)
	{
		data << uint64(0);
		uint32 count = 0;
		data << count;
		for(uint32 i = 0; i < count; i++)
		{
			data << uint32(0);
			data << uint32(0);
		}
	}
	SendPacket(&data);
}
