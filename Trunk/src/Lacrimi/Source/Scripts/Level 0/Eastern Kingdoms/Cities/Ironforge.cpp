/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

#define RHA_GOSSIP_ITEM_1 "I am ready to listen"
#define RHA_GOSSIP_ITEM_2 "That is tragic. How did this happen?"
#define RHA_GOSSIP_ITEM_3 "Interesting, continue please."
#define RHA_GOSSIP_ITEM_4 "Unbelievable! How dare they??"
#define RHA_GOSSIP_ITEM_5 "Of course I will help!"

class Royal_Historian_Archesonus : public GossipScript
{
public:
	void GossipHello(Object * pObject, Player* Plr, bool AutoSend)
	{
		if(Plr->GetQuestStatusForQuest(3702) == QMGR_QUEST_NOT_FINISHED)
		{
			GossipMenu * Menu;
			objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2235, Plr);
			Menu->AddItem(2, RHA_GOSSIP_ITEM_1, 1);
			Menu->SendTo(Plr);
		}
	}

	void GossipSelectOption(Object* pObject, Player* Plr, uint32 Id, uint32 IntId, const char * EnteredCode)
	{
		GossipMenu* Menu;
		switch(IntId)
		{
		case 1:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2236, Plr);
				Menu->AddItem(2, RHA_GOSSIP_ITEM_2, 2);
				Menu->SendTo(Plr);
			}break;
		case 2:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2237, Plr);
				Menu->AddItem(2, RHA_GOSSIP_ITEM_3, 3);
				Menu->SendTo(Plr);
			}break;
		case 3:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2238, Plr);
				Menu->AddItem(2, RHA_GOSSIP_ITEM_4, 4);
				Menu->SendTo(Plr);
			}break;
		case 4:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 2239, Plr);
				Menu->AddItem(2, RHA_GOSSIP_ITEM_5, 5);
				Menu->SendTo(Plr);
			}break;
		case 5:
			{
				Plr->Gossip_Complete();
				sQuestMgr.OnPlayerExploreArea(Plr, 3702);
			}break;
		}
	}
};

void Lacrimi::SetupIronforge()
{
	RegisterCtrGossipScript(8879, Royal_Historian_Archesonus);
}
