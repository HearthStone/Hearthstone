/***
 * Demonstrike Core
 */

#include "StdAfx.h"

/*// Fatty array time.
const uint32 CreatureEntryPair[][2] =
{
	{32307, 32308}, // Guards
	{30739, 30740}, // Champions
	{31101, 31051}, // Hoodoo Master & Sorceress
	{31102, 31052}, // Vieron Blazefeather & Bowyer
	{32296, 32294}, // Quartermaster
	{31107, 31109}, // Lieutenant & Senior Demolitionist
	{31151, 31153}, // Tactical Officer
	{31106, 31108}, // Siegesmith & Siege Master
	{31053, 31054}, // Primalist & Anchorite
	{31091, 31036}, // Commander
	{32615, 32626}, // Warbringer & Brigadier General
	{0,0}
};*/

Wintergrasp::Wintergrasp(WintergraspInternal* WGI, MapMgr* mgr) : Internal(*WGI)
{
	for(int i = 0; i < 3; i++)
		playercount[i] = 0;

	Player* plr = NULL;
	WGID = Internal.WGCounter;

	for(PlayerStorageMap::iterator itr =  mgr->m_PlayerStorage.begin(); itr != mgr->m_PlayerStorage.end(); itr++)
	{
		plr = itr->second;
		if((plr->GetPAreaID() == WINTERGRASP) || (plr->GetZoneId() == WINTERGRASP))
		{
			WGPlayers.insert(plr);
			plr->WinterGrasp = this;
			++playercount[2]; // Total
			++playercount[plr->GetTeam()];
		}
	}
	if(WGPlayers.size() != playercount[2])
		printf("Player disfunction occured!\n");
	FlameWatchDestroyed = false;
	ShadowsightDestroyed = false;
	WintersEdgeDestroyed = false;
	Init();
	numworkshop[0] = 0;
	numworkshop[1] = 0;
	numvehicles[0] = 0;
	numvehicles[1] = 0;
}

Wintergrasp::~Wintergrasp()
{
	Internal.m_wintergrasp = 2; // We are ending Wintergrasp.
	// Handle deletion and removal.
	Internal.EndWintergrasp();
}

void Wintergrasp::Init()
{
	Internal.StartWintergrasp();
	Internal.SendInitWorldStates();
	Internal.UpdateClock();
}

void Wintergrasp::End(Player*plr)
{
	plr->GetMapMgr()->CastSpellOnPlayers(plr->GetTeam(),57940);
	char Text[1024];
	snprintf(Text, 1024, "%sThe %s won the Battle of Wintergrasp!", MSG_COLOR_YELLOW, plr->GetTeam() ? "Horde" : "Alliance");
	_SendMessage(Text);
	sWintergraspI.EndWintergrasp();
}

void Wintergrasp::ForceEnd()
{
	if(sWintergraspI.GetTimeRemaining() <= 0)
	{
		char Text[1024];
		snprintf(Text, 1024, "%sThe Battle of Wintergrasp has past the match time limit and is now ended!", MSG_COLOR_YELLOW);
		_SendMessage(Text);
		sWintergraspI.EndWintergrasp();
	}
}

void Wintergrasp::OnAddPlayer(Player* plr)
{
	printf("Pie flavor'd bastard!\n");
	++playercount[2];
	++playercount[plr->GetTeam()];
	WGPlayers.insert(plr);
	plr->WinterGrasp = this;
	Internal.SendInitWorldStates(plr);
}

void Wintergrasp::OnRemovePlayer(Player* plr)
{
	printf("Pie flavor'd bastard left!\n");
	--playercount[2];
	--playercount[plr->GetTeam()];
	WGPlayers.erase(plr);
	plr->WinterGrasp = NULL;
}

void Wintergrasp::GoDestroyEvent(uint32 Entry, Player* Plr)
{
	switch(Entry)
	{
	case TOWER_FLAMEWATCH:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s has destroyed Flamewatch Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
			Plr->GetMapMgr()->RemoveAuraFromPlayers(Plr->GetTeam() ? 1 : 0,62064);
			FlameWatchDestroyed = true;
			ShortenBattle(600000);
		}break;
	case TOWER_WINTERS_EDGE:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s have destroyed Winter's Edge Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
			Plr->GetMapMgr()->RemoveAuraFromPlayers(Plr->GetTeam() ? 1 : 0,62064);
			WintersEdgeDestroyed = true;
			ShortenBattle(600000);
		}break;
	case TOWER_SHADOWSIGHT:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s have destroyed Shadowsight Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
			Plr->GetMapMgr()->RemoveAuraFromPlayers(Plr->GetTeam() ? 1 : 0,62064);
			ShadowsightDestroyed = true;
			ShortenBattle(600000);
		}break;
	case FORTRESS_GATE:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sWintergrasp Fortress south gate has been destroyed!", MSG_COLOR_YELLOW);
			_SendMessage(Text);
		}break;
	case FORTRESS_DOOR:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sWintergrasp Keep has been breached!", MSG_COLOR_YELLOW);
			_SendMessage(Text);
		}break;
	default:
		break;
	}

}

void Wintergrasp::GoDamageEvent(uint32 Entry, Player* Plr)
{
	switch(Entry)
	{
	case TOWER_FLAMEWATCH:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s has assaulted Flamewatch Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
		}break;
	case TOWER_WINTERS_EDGE:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s has assaulted Winter's Edge Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
		}break;
	case TOWER_SHADOWSIGHT:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sThe %s has assaulted Shadowsight Tower!", MSG_COLOR_YELLOW, Plr->GetTeam() ? "Horde" : "Alliance");
			_SendMessage(Text);
		}break;
	case FORTRESS_GATE:
		{
			char Text[1024];
			snprintf(Text, 1024, "%sWintergrasp south gate has been assaulted!", MSG_COLOR_YELLOW);
			_SendMessage(Text);
		}break;
	default:
		break;
	}

}

void Wintergrasp::_SendMessage(const char* text)
{
	uint32 textLen = (uint32)strlen((char*)text) + 1;
	WorldPacket data(textLen + 40);
	data.Initialize(SMSG_MESSAGECHAT);
	data << uint8(CHAT_MSG_SYSTEM);
	data << uint32(LANG_UNIVERSAL);
	data << (uint64)0;
	data << (uint32)0;
	data << (uint64)0;
	data << textLen;
	data << text;
	data << uint8(0);
	sWintergraspI.SendPacketToWG(&data);
}

void Wintergrasp::ShortenBattle(uint32 Time)
{
	if(FlameWatchDestroyed && WintersEdgeDestroyed && ShadowsightDestroyed)
		sWintergraspI.SetTimeRemaining(Time); //No need to check for time ending it will be checked next thread loop.
}