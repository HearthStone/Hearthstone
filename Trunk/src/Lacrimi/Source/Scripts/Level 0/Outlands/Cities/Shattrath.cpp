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

// Exarch Nasuun
#define GOSSIP_EXARCH_NASUUN_1 "Nasuun, do you know how long until we have an alchemy lab at the Sun's Reach Harbor?"
#define GOSSIP_EXARCH_NASUUN_2 "What do you know about the magical gates at the Sunwell Plateau being brought down?"
#define GOSSIP_EXARCH_NASUUN_3 "I have something else to ask you about."

//#define USE_THE_STATUS 1	// Decomment this is for the status

class ExarchNasuun_Gossip : public GossipScript
{
public:
	void GossipHello(Object *pObject, Player *plr, bool AutoSend)
	{
		GossipMenu *Menu;
		objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12227, plr);

#ifdef USE_THE_STATUS
		Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_1, 1); // Status of the lab
		Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_2, 2);
#else
		Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_2, 3);
#endif
		if(AutoSend)
			Menu->SendTo(plr);
	}

	void GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char * Code)
	{
		if(pObject->GetTypeId() != TYPEID_UNIT)
			return;

		GossipMenu * Menu;
		switch(IntId)
		{
		case 0:
			GossipHello(pObject, plr, true);
			break;
		case 1:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12303, plr);
				Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
				Menu->SendTo(plr);
			}break;
		case 2:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12305, plr);
				Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
				Menu->SendTo(plr);
			}break;
		case 3:
			{
				objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 12623, plr);
				Menu->AddItem( 0, GOSSIP_EXARCH_NASUUN_3, 0);
				Menu->SendTo(plr);
			}break;
		}
	}

	void Destroy()
	{
		delete this;
	}
};

class ZephyrGossipScript : public GossipScript
{
public:
	void GossipHello(Object *pObject, Player *Plr, bool AutoSend)
	{
		GossipMenu *Menu;
		objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 1, Plr);
		if(Plr->GetStanding(989) >= 21000)
			Menu->AddItem(0, "Bring me to Caverns of Time!", 1); 
		Menu->SendTo(Plr);
	}

	void GossipSelectOption(Object *pObject, Player *plr, uint32 Id, uint32 IntId, const char * EnteredCode)
	{
		Creature *Zephyr = pObject->IsCreature() ? TO_CREATURE(pObject) : NULL;
		if (Zephyr == NULLCREATURE)
			return;

		switch (IntId)
		{
		case 0:
			GossipHello(pObject, plr, true);
			break;
		case 1:
			plr->Gossip_Complete();
			Zephyr->CastSpell(plr, dbcSpell.LookupEntry(37778), true);
			break;
		}
	}

	void Destroy()
	{
		delete this;
	}
};

void Lacrimi::SetupShattrath()
{
	RegisterCtrGossipScript(25967, ZephyrGossipScript);
	RegisterCtrGossipScript(24932, ExarchNasuun_Gossip);
}
