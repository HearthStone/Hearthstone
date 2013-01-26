/***
 * Demonstrike Core
 */

#include "StdAfx.h"
initialiseSingleton( QuestMgr );

void WorldSession::HandleQuestgiverStatusQueryOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_STATUS_QUERY." );

	CHECK_INWORLD_RETURN();

	uint64 guid;
	WorldPacket data(SMSG_QUESTGIVER_STATUS, 9);
	Object* qst_giver = NULLOBJ;

	recv_data >> guid;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);
	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = quest_giver;
		else
			return;

		if (!quest_giver->isQuestGiver())
		{
			OUT_DEBUG("WORLD: Creature is not a questgiver.");
			return;
		}
	}
	else if(guidtype==HIGHGUID_TYPE_ITEM)
	{
		Item* quest_giver = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
	}
	else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID "I64FMT".", guid);
		return;
	}

	data << guid << sQuestMgr.CalcStatus(qst_giver, GetPlayer());
	SendPacket( &data );
}

void WorldSession::HandleQuestgiverHelloOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_HELLO." );
	CHECK_INWORLD_RETURN();

	uint64 guid;
	recv_data >> guid;

	Creature* qst_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if (!qst_giver->isQuestGiver())
	{
		OUT_DEBUG("WORLD: Creature is not a questgiver.");
		return;
	}

	if(qst_giver->GetAIInterface()) // NPC Stops moving for 3 minutes
		qst_giver->GetAIInterface()->StopMovement(180000);

	//qst_giver->Emote(EMOTE_ONESHOT_TALK); // this doesnt work
	sQuestMgr.OnActivateQuestGiver(qst_giver, GetPlayer());
}

void WorldSession::HandleQuestGiverQueryQuestOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_QUERY_QUEST." );
	CHECK_INWORLD_RETURN();

	WorldPacket data;
	uint64 guid;
	uint32 quest_id;
	uint32 status = 0;

	recv_data >> guid;
	recv_data >> quest_id;

	Object* qst_giver = NULLOBJ;

	bool bValid = false;
	Quest* qst = sQuestMgr.GetQuestPointer(quest_id);
	if (!qst)
	{
		OUT_DEBUG("WORLD: Invalid quest ID.");
		return;
	}

	uint32 guidtype = GET_TYPE_FROM_GUID(guid);
	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = quest_giver;
		else
			return;
		bValid = quest_giver->isQuestGiver();
		if(bValid)
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
	}
	else if(guidtype == HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = quest_giver->isQuestGiver();
		if(bValid)
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id), false);
	}
	else if(guidtype==HIGHGUID_TYPE_ITEM)
	{
		Item* quest_giver = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = true;
		if( qst->id != quest_giver->GetProto()->QuestId )
			return;

		status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, 1, false);
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if (!bValid)
	{
		OUT_DEBUG("WORLD: object is not a questgiver.");
		return;
	}

	if(status == QMGR_QUEST_NOT_FINISHED || status == QMGR_QUEST_FINISHED)
	{
		if(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
			sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
		else
			sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status);
		SendPacket(&data);
		DEBUG_LOG( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
	}
	else if (status == QMGR_QUEST_CHAT || status == QMGR_QUEST_AVAILABLE)
	{
		sQuestMgr.BuildQuestDetails(&data, qst, qst_giver, 1, _player);
		SendPacket(&data);
		OUT_DEBUG( "WORLD: Sent SMSG_QUESTGIVER_QUEST_DETAILS." );
	}
}

void WorldSession::HandleQuestgiverAcceptQuestOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_ACCEPT_QUEST" );
	CHECK_INWORLD_RETURN();

	uint64 guid;
	uint32 quest_id;
	uint32 unk;

	recv_data >> guid;
	recv_data >> quest_id;
	recv_data >> unk;

	bool bValid = false;
	bool hasquest = true;
	bool bSkipLevelCheck = false;
	Quest *qst = NULL;
	Object* qst_giver = NULLOBJ;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);

	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = quest_giver->isQuestGiver();
		hasquest = quest_giver->HasQuest(quest_id, 1);
		if(bValid)
			qst = sQuestMgr.GetQuestPointer(quest_id);
	}
	else if(guidtype == HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		//bValid = quest_giver->isQuestGiver();
		//if(bValid)
		bValid = true;
		qst = sQuestMgr.GetQuestPointer(quest_id);
	}
	else if(guidtype == HIGHGUID_TYPE_ITEM)
	{
		Item* quest_giver = GetPlayer()->GetItemInterface()->GetItemByGUID(guid);
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = true;
		bSkipLevelCheck = true;
		qst = sQuestMgr.GetQuestPointer(quest_id);
		if( qst && qst->id != quest_giver->GetProto()->QuestId )
			return;
	}
	else if(guidtype == HIGHGUID_TYPE_PLAYER)
	{
		Player* quest_giver = _player->GetMapMgr()->GetPlayer((uint32)guid);
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = true;
		qst = sQuestMgr.GetQuestPointer(quest_id);
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if( !bValid || qst == NULL )
	{
		OUT_DEBUG("WORLD: Creature is not a questgiver.");
		return;
	}

	if( _player->GetQuestLogForEntry( qst->id ) )
		return;

	if( qst_giver->GetTypeId() == TYPEID_UNIT && TO_CREATURE( qst_giver )->m_escorter != NULL )
	{
		SystemMessage("You cannot accept this quest at this time.");
		return;
	}

	// Check the player hasn't already taken this quest, or
	// it isn't available.
	uint32 status = sQuestMgr.CalcQuestStatus(_player, qst, 3, bSkipLevelCheck);
	if((!sQuestMgr.IsQuestRepeatable(qst) && _player->HasFinishedQuest(qst->id)) || ( status != QMGR_QUEST_CHAT && status != QMGR_QUEST_AVAILABLE && status != QMGR_QUEST_AVAILABLELOW_LEVEL ) || !hasquest)
	{
		// We've got a hacker. Disconnect them.
		//sWorld.LogCheater(this, "tried to accept incompatible quest %u from %u.", qst->id, qst_giver->GetEntry());
		//Disconnect();
		SystemMessage("Internal quest error 3");
		return;
	}

	int32 log_slot = _player->GetOpenQuestSlot();
	if (log_slot == -1)
	{
		sQuestMgr.SendQuestLogFull(GetPlayer());
		return;
	}

	if(qst->rewards)
	{
		if(qst->rewards->count_receiveitems || qst->rewards->srcitem)
		{
			uint32 slots_required = qst->rewards->count_receiveitems;

			if(_player->GetItemInterface()->CalculateFreeSlots(NULL) < slots_required)
			{
				_player->GetItemInterface()->BuildInventoryChangeError(NULLITEM, NULLITEM, INV_ERR_BAG_FULL);
				sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, _player);
				return;
			}
		}
	}

	QuestLogEntry *qle = new QuestLogEntry();
	qle->Init(qst, _player, log_slot);
	qle->UpdatePlayerFields();

	if(qst->rewards)
	{
		// If the quest should give any items on begin, give them the items.
		for(uint32 i = 0; i < 4; i++)
		{
			if(qst->rewards->receive_items[i])
			{
				Item* item = objmgr.CreateItem( qst->rewards->receive_items[i], GetPlayer());
				if(item)
				{
					item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->rewards->receive_itemcount[i]);
					if(!_player->GetItemInterface()->AddItemToFreeSlot(item))
					{
						item->DeleteMe();
						item = NULLITEM;
					}
					else
						SendItemPushResult(item, false, true, false, true, _player->GetItemInterface()->LastSearchItemBagSlot(), _player->GetItemInterface()->LastSearchItemSlot(), 1);
				}
			}
		}

		if(qst->rewards->srcitem && qst->rewards->srcitem != qst->rewards->receive_items[0])
		{
			Item* item = objmgr.CreateItem( qst->rewards->srcitem, _player );
			if(item)
			{
				item->SetUInt32Value(ITEM_FIELD_STACK_COUNT, qst->rewards->srcitemcount ? qst->rewards->srcitemcount : 1);
				if(!_player->GetItemInterface()->AddItemToFreeSlot(item))
				{
					item->DeleteMe();
					item = NULLITEM;
				}
			}
		}
	}

	if(qst->objectives)
		if(qst->objectives->count_required_item || qst_giver->GetTypeId() == TYPEID_GAMEOBJECT)	// gameobject quests deactivate
			_player->UpdateNearbyGameObjects();

	CALL_QUESTSCRIPT_EVENT(qst->id, OnQuestStart)(_player, qle);

	sQuestMgr.OnQuestAccepted(_player,qst,qst_giver);

	if(qst->qst_start_phase != 0 )
		_player->EnablePhase(qst->qst_start_phase, true);

	sHookInterface.OnQuestAccept(_player, qst, qst_giver);
}

void WorldSession::HandleQuestgiverCancelOpcode(WorldPacket& recvPacket)
{
	WorldPacket data(SMSG_GOSSIP_COMPLETE, 0);
	SendPacket(&data);
	//OutPacket(SMSG_GOSSIP_COMPLETE, 0, NULL);

	OUT_DEBUG("WORLD: Sent SMSG_GOSSIP_COMPLETE");
}

void WorldSession::HandleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket)
{
	CHECK_INWORLD_RETURN();

	DEBUG_LOG( "QuestHandler","Received CMSG_QUESTLOG_REMOVE_QUEST" );

	uint8 quest_slot;
	recvPacket >> quest_slot;
	if(quest_slot >= 25)
		return;

	QuestLogEntry *qEntry = _player->GetQuestLogInSlot(quest_slot);
	if (!qEntry)
	{
		DEBUG_LOG("QuestHandler","No quest in slot %d.", quest_slot);
		return;
	}
	Quest *qPtr = qEntry->GetQuest();

	if (!qPtr)
	{
		DEBUG_LOG("QuestHandler","Quest %u does not exist in database", qPtr->id);
		return;
	}

	CALL_QUESTSCRIPT_EVENT(qPtr->id, OnQuestCancel)(_player);

	qEntry->Finish();

	uint32 srcItem = 0;

	// Remove all items given by the questgiver at the beginning
	if(qPtr->rewards)
	{
		srcItem = qPtr->rewards->srcitem;

		for(uint32 i = 0; i < 4; i++)
			if(qPtr->rewards->receive_items[i] && qPtr->rewards->receive_items[i] != srcItem)
				_player->GetItemInterface()->RemoveItemAmt( qPtr->rewards->receive_items[i], qPtr->rewards->receive_itemcount[i] );

		// Remove source item
		if(qPtr->rewards->srcitem)
			_player->GetItemInterface()->RemoveItemAmt( qPtr->rewards->srcitem, 1 );
	}

	// Reset timed quests, remove timed event
	if(qPtr->objectives)
	{
		// always remove collected items (need to be recollectable again in case of repeatable).
		for( uint32 y = 0; y < 6; y++)
			if( qPtr->objectives->required_item[y] && qPtr->objectives->required_item[y] != srcItem )
				_player->GetItemInterface()->RemoveItemAmt(qPtr->objectives->required_item[y], qPtr->objectives->required_itemcount[y]);

		if(qPtr->objectives->required_timelimit > 0)
			if(sEventMgr.HasEvent(_player,EVENT_TIMED_QUEST_EXPIRE))
				sEventMgr.RemoveEvents(_player, EVENT_TIMED_QUEST_EXPIRE); 
	}

	if(qPtr->qst_start_phase != 0)
		_player->DisablePhase(qPtr->qst_start_phase, true);

	_player->UpdateNearbyQuestGivers();
	_player->UpdateNearbyGameObjects();

	sHookInterface.OnQuestCancelled(_player, qPtr);

	_player->SaveToDB(false);
}

void WorldSession::HandleQuestQueryOpcode( WorldPacket & recv_data )
{
	CHECK_INWORLD_RETURN();
	DEBUG_LOG( "WORLD"," Received CMSG_QUEST_QUERY" );

	uint32 quest_id;

	recv_data >> quest_id;

	Quest *qst = sQuestMgr.GetQuestPointer(quest_id);

	if (!qst)
	{
		OUT_DEBUG("WORLD: Invalid quest ID.");
		return;
	}

	WorldPacket *pkt = BuildQuestQueryResponse(qst);
	SendPacket(pkt);
	delete pkt;

	DEBUG_LOG( "WORLD"," Sent SMSG_QUEST_QUERY_RESPONSE." );
}

void WorldSession::HandleQuestgiverRequestRewardOpcode( WorldPacket & recv_data )
{
	CHECK_INWORLD_RETURN();
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_REQUESTREWARD_QUEST." );

	uint64 guid;
	uint32 quest_id;

	recv_data >> guid;
	recv_data >> quest_id;

	bool bValid = false;
	Quest *qst = NULL;
	Object* qst_giver = NULLOBJ;
	uint32 status = 0;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);

	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = quest_giver->isQuestGiver();
		if(bValid)
		{
			qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
			if(!qst)
				qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_START);

			/*if(!qst)
				qst = sQuestMgr.GetQuestPointer(quest_id);*/
			if(!qst)
			{
				OUT_DEBUG("WARNING: Cannot complete quest, as it doesnt exist.");
				return;
			}
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
		}
	}
	else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return; // oops..
		bValid = quest_giver->isQuestGiver();
		if(bValid)
		{
			qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
			/*if(!qst) sQuestMgr.FindQuest(quest_id);*/
			if(!qst)
			{
				OUT_DEBUG("WARNING: Cannot complete quest, as it doesnt exist.");
				return;
			}
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
		}
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if (!bValid || qst == NULL)
	{
		OUT_DEBUG("WORLD: Creature is not a questgiver.");
		return;
	}

	if (status == QMGR_QUEST_FINISHED)
	{
        WorldPacket data;
		sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
		SendPacket(&data);
		DEBUG_LOG( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
	}

	// if we got here it means we're cheating
}

void WorldSession::HandleQuestgiverCompleteQuestOpcode( WorldPacket & recvPacket )
{
	CHECK_INWORLD_RETURN();
	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_COMPLETE_QUEST." );

	uint64 guid;
	uint32 quest_id;

	recvPacket >> guid;
	recvPacket >> quest_id;

	bool bValid = false;
	Quest *qst = NULL;
	Object* qst_giver = NULLOBJ;
	uint32 status = 0;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);

	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		bValid = quest_giver->isQuestGiver();
		if(bValid)
		{
			qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
			/*if(!qst)
				sQuestMgr.FindQuest(quest_id);*/
			if(!qst)
			{
				OUT_DEBUG("WARNING: Cannot complete quest, as it doesnt exist.");
				return;
			}
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
		}
	}
	else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return; // oops..
		bValid = quest_giver->isQuestGiver();
		if(bValid)
		{
			qst = quest_giver->FindQuest(quest_id, QUESTGIVER_QUEST_END);
			/*if(!qst) sQuestMgr.FindQuest(quest_id);*/
			if(!qst)
			{
				OUT_DEBUG("WARNING: Cannot complete quest, as it doesnt exist.");
				return;
			}
			status = sQuestMgr.CalcQuestStatus(GetPlayer(), qst, (uint8)quest_giver->GetQuestRelation(qst->id),false);
		}
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if (!bValid || qst == NULL)
	{
		OUT_DEBUG("WORLD: Creature is not a questgiver.");
		return;
	}

	if (status == QMGR_QUEST_FINISHED || qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE)
	{
		WorldPacket data;
		sQuestMgr.BuildOfferReward(&data, qst, qst_giver, 1, _player);
		SendPacket(&data);
		DEBUG_LOG( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
	}
	else if (status == QMGR_QUEST_NOT_FINISHED || qst->qst_is_repeatable)
	{
		WorldPacket data;
		sQuestMgr.BuildRequestItems(&data, qst, qst_giver, status);
		SendPacket(&data);
		DEBUG_LOG( "WORLD"," Sent SMSG_QUESTGIVER_REQUEST_ITEMS." );
	}

	sHookInterface.OnQuestFinished(_player, qst, qst_giver);
}

void WorldSession::HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket)
{
	CHECK_INWORLD_RETURN();

	DEBUG_LOG( "WORLD"," Received CMSG_QUESTGIVER_CHOOSE_REWARD." );

	uint64 guid;
	uint32 quest_id;
	uint32 reward_slot;

	recvPacket >> guid;
	recvPacket >> quest_id;
	recvPacket >> reward_slot;

	if( reward_slot >= 6 )
		return;

	bool bValid = false;
	Quest *qst = NULL;
	Object* qst_giver = NULLOBJ;
	uint32 guidtype = GET_TYPE_FROM_GUID(guid);

	if(guidtype == HIGHGUID_TYPE_CREATURE)
	{
		Creature* quest_giver = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = quest_giver;
		else
			return;
		bValid = quest_giver->isQuestGiver();
		if(bValid)
			qst = sQuestMgr.GetQuestPointer(quest_id);
	}
	else if(guidtype==HIGHGUID_TYPE_GAMEOBJECT)
	{
		GameObject* quest_giver = _player->GetMapMgr()->GetGameObject(GET_LOWGUID_PART(guid));
		if(quest_giver)
			qst_giver = TO_OBJECT(quest_giver);
		else
			return;
		//bValid = quest_giver->isQuestGiver();
		//if(bValid)
		bValid = true;
		qst = sQuestMgr.GetQuestPointer(quest_id);
	}

	if (!qst_giver)
	{
		OUT_DEBUG("WORLD: Invalid questgiver GUID.");
		return;
	}

	if (!bValid || qst == NULL)
	{
		OUT_DEBUG("WORLD: Creature is not a questgiver.");
		return;
	}

	//FIXME: Some Quest givers talk in the end of the quest.
	//   qst_giver->SendChatMessage(CHAT_MSG_MONSTER_SAY,LANG_UNIVERSAL,qst->GetQuestEndMessage().c_str());
	QuestLogEntry *qle = _player->GetQuestLogForEntry(quest_id);
	if (!qle && !qst->qst_is_repeatable && !(qst->qst_flags & QUEST_FLAG_AUTOCOMPLETE))
	{
		OUT_DEBUG("WORLD: QuestLogEntry not found.");
		return;
	}

	if (qle && !qle->CanBeFinished())
	{
		OUT_DEBUG("WORLD: Quest not finished.");
		return;
	}

	//check for room in inventory for all items
	if(!sQuestMgr.CanStoreReward(_player,qst,reward_slot))
	{
		sQuestMgr.SendQuestFailed(FAILED_REASON_INV_FULL, qst, _player);
		return;
	}

	sQuestMgr.OnQuestFinished(_player, qst, qst_giver, reward_slot);
	//if(qst_giver->GetTypeId() == TYPEID_UNIT) qst->LUA_SendEvent(TO_CREATURE( qst_giver ),GetPlayer(),ON_QUEST_COMPLETEQUEST);

	if(qst->qst_next_quest_id)
	{
		WorldPacket data(12);
		data.Initialize(CMSG_QUESTGIVER_QUERY_QUEST);
		data << guid;
		data << qst->qst_next_quest_id;
		HandleQuestGiverQueryQuestOpcode(data);
	}

	_player->SaveToDB(false);
}

void WorldSession::HandlePushQuestToPartyOpcode(WorldPacket &recv_data)
{
	CHECK_INWORLD_RETURN();
	uint32 questid, status;
	recv_data >> questid;

	DEBUG_LOG( "WORLD"," Received CMSG_PUSHQUESTTOPARTY quest = %u", questid );

	Quest *pQuest = sQuestMgr.GetQuestPointer(questid);
	if(pQuest)
	{
		Group *pGroup = _player->GetGroup();
		if(pGroup)
		{
			uint32 pguid = _player->GetLowGUID();
			SubGroup * sgr = _player->GetGroup() ? 	_player->GetGroup()->GetSubGroup(_player->GetSubGroup()) : 0;

			if(sgr)
			{
				_player->GetGroup()->Lock();
				GroupMembersSet::iterator itr;
				for(itr = sgr->GetGroupMembersBegin(); itr != sgr->GetGroupMembersEnd(); itr++)
				{
					Player* pPlayer = (*itr)->m_loggedInPlayer;
					if(pPlayer && pPlayer->GetGUID() !=  pguid)
					{
						WorldPacket data( MSG_QUEST_PUSH_RESULT, 13 );
						data << pPlayer->GetGUID();
						data << uint32(QUEST_SHARE_MSG_SHARING_QUEST);
						data << uint8(0);
						_player->GetSession()->SendPacket(&data);

						uint32 response = 0;
						//CHECKS IF CAN TAKE THE QUEST
						status = sQuestMgr.PlayerMeetsReqs(pPlayer, pQuest, false);
						if(status != QMGR_QUEST_CHAT && status != QMGR_QUEST_AVAILABLE)
							response = QUEST_SHARE_MSG_CANT_TAKE_QUEST;

						//CHECKS IF QUESTLOG ISN'T FULL
						if(pPlayer->GetOpenQuestSlot() == -1)
							response = QUEST_SHARE_MSG_LOG_FULL;

						//CHEQUEA SI TIENE LA QUEST EN EL QUESTLOG | CHEKS IF HAVE ALREADY THE QUEST IN QUESTLOG
						QuestLogEntry *qst = pPlayer->GetQuestLogForEntry(questid);
						if(qst)
							response = QUEST_SHARE_MSG_HAVE_QUEST;

						//CHECKS IF ALREADY HAVE COMPLETED THE QUEST
						if(pPlayer->HasFinishedQuest(questid))
							response = QUEST_SHARE_MSG_FINISH_QUEST;

						//CHECKS IF  ALREADY HAVE COMPLETED THE DAILY QUEST
						if(pPlayer->HasFinishedDailyQuest(questid))
							response = QUEST_SHARE_MSG_CANT_SHARE_TODAY;

						//CHECKS IF IS IN THE PARTY
						if(!pGroup)
							response = QUEST_SHARE_MSG_NOT_IN_PARTY;

						//CHECKS IF IS BUSY
						if(pPlayer->DuelingWith != NULL)
							response = QUEST_SHARE_MSG_BUSY;

						if(response > 0)
						{
							sQuestMgr.SendPushToPartyResponse(_player, pPlayer, response);
							continue;
						}

						data.clear();
						sQuestMgr.BuildQuestDetails(&data, pQuest, pPlayer, 1, _player);
						pPlayer->GetSession()->SendPacket(&data);
						pPlayer->SetQuestSharer(pguid);
					}
				}
				_player->GetGroup()->Unlock();
			}
		}
	}
}

void WorldSession::HandleQuestPushResult(WorldPacket& recvPacket)
{
	CHECK_INWORLD_RETURN();
	uint64 guid;
	uint8 msg;
	recvPacket >> guid >> msg;

	DEBUG_LOG( "WORLD"," Received MSG_QUEST_PUSH_RESULT " );

	if(GetPlayer()->GetQuestSharer())
	{
		Player* pPlayer = objmgr.GetPlayer(GetPlayer()->GetQuestSharer());
		if(pPlayer)
		{
			WorldPacket data(MSG_QUEST_PUSH_RESULT, 13);
			data << guid;
			data << uint32(msg);
			data << uint8(0);
			pPlayer->GetSession()->SendPacket(&data);
			GetPlayer()->SetQuestSharer(0);
		}
	}
}

void WorldSession::HandleQuestPOI(WorldPacket& recvPacket)
{
	CHECK_INWORLD_RETURN();
	uint32 count;
	recvPacket >> count;

	if (count >= 25)
		return;

	DEBUG_LOG( "WORLD"," Received MSG_QUEST_PUSH_RESULT " );

	WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4+(4+4)*count);
	data << uint32(count);

	for (uint i = 0; i < count; ++i)
	{
		uint32 questId;
		recvPacket >> questId;

		bool questOk = false;

		uint16 questSlot = _player->FindQuestSlot(questId);

		if (questSlot != 25)
			questOk =_player->GetQuestSlotQuestId(questSlot) == questId;

		if (questOk)
		{
			QuestPOIVector const *POI = objmgr.GetQuestPOIVector(questId);

			if (POI)
			{
				data << uint32(questId);
				data << uint32(POI->size());

				for (QuestPOIVector::const_iterator itr = POI->begin(); itr != POI->end(); ++itr)
				{
					data << uint32(itr->Id);
					data << int32(itr->ObjectiveIndex);
					data << uint32(itr->MapId);
					data << uint32(itr->AreaId);
					data << uint32(itr->MapFloorId);
					data << uint32(itr->Unk3);
					data << uint32(itr->Unk4);
					data << uint32(itr->points.size());

					for (std::vector<QuestPOIPoint>::const_iterator itr2 = itr->points.begin(); itr2 != itr->points.end(); ++itr2)
					{
						data << int32(itr2->x);
						data << int32(itr2->y);
					}
				}
			}
			else
			{
				data << uint32(questId);
				data << uint32(0);
			}
		}
		else
		{
			data << uint32(questId);
			data << uint32(0);
		}
	}
	SendPacket(&data);
}
