/***
 * Demonstrike Core
 */

#include "StdAfx.h"

initialiseSingleton( WintergraspInternal );

WintergraspInternal::WintergraspInternal() : ThreadContext()
{
	m_timer = 0;
	m_wintergrasp = 0;
	WGMgr = sInstanceMgr.GetMapMgr(571);
	WG = NULL;
	WG_started = false;
	defendingteam = 2;
	winnerteam = 2;
	forcestart_WG = false;
	WGCounter = 0;
}

WintergraspInternal::~WintergraspInternal()
{

}

void WintergraspInternal::dupe_tm_pointer(tm * returnvalue, tm * mypointer)
{
	memcpy(mypointer, returnvalue, sizeof(tm));
}

bool WintergraspInternal::has_timeout_expired(tm * now_time, tm * last_time)
{
	if(now_time->tm_hour != last_time->tm_hour)
		return true;

	if(now_time->tm_mday != last_time->tm_mday)
		return true;

	return false;
}

bool WintergraspInternal::run()
{
	currenttime = UNIXTIME;
	dupe_tm_pointer(localtime(&currenttime), &local_currenttime);
	last_countertime = UNIXTIME;
	dupe_tm_pointer(localtime(&last_countertime), &local_last_countertime);
	uint32 interval = 5*60*1000;
	uint32 counter = 0;
	m_timer = 180000; // 3 hours. TODO: sWorld.WintergraspHourInterval*60*1000

	Log.Notice("WintergraspInternal", "Wintergrasp Handler Initiated.");

	while(GetThreadState() != THREADSTATE_TERMINATE)
	{
		if(!SetThreadState(THREADSTATE_BUSY))
			break;

		if(has_timeout_expired(&local_currenttime, &local_last_countertime) || forcestart_WG == true)
		{
			++counter;

			if(counter >= 3/*3 hours*/ || forcestart_WG == true) // TODO: sWorld.WintergraspHourInterval
			{
				Log.Notice("WintergraspInternal", "Wintergrasp function called.");
				if(m_wintergrasp == 0)
				{
					StartWintergrasp();
					++WGCounter;
					Log.Notice("WintergraspInternal", "Starting Wintergrasp.");
					WG = Wintergrasp::Create(this, WGMgr);
					interval = interval/10;
				}
				counter = 0; // Reset our timer.
				forcestart_WG = false;
			}
			else
				Log.Notice("WintergraspInternal", "Wintergrasp counter at %u/3", counter);

			last_countertime = UNIXTIME;
			dupe_tm_pointer(localtime(&last_countertime), &local_last_countertime);
		}

		if(!SetThreadState(THREADSTATE_SLEEPING))
			break;
		Delay(120000);

		if(!SetThreadState(THREADSTATE_BUSY))
			break;

		if(WG && WG_started)
		{
			MatchTimer = (MatchTimer - 30000);
			float TimeLeft = ((MatchTimer/1000)/60);
			UpdateClock();
			printf("%u %f Minutes left till Wintergrasp ends\n",getMSTime(),TimeLeft);
			SendWSUpdateToAll(A_NUMVEH_WORLDSTATE, WG->GetNumVehicles(ALLIANCE));
			SendWSUpdateToAll(A_MAXVEH_WORLDSTATE, WG->GetNumWorkshops(ALLIANCE)*4);
			SendWSUpdateToAll(H_NUMVEH_WORLDSTATE, WG->GetNumVehicles(HORDE));
			SendWSUpdateToAll(H_NUMVEH_WORLDSTATE, WG->GetNumWorkshops(HORDE)*4);
			if(MatchTimer <= 0)
			{
				interval = interval*10;
				GetWintergrasp()->ForceEnd();
			}
		}

		if(!SetThreadState(THREADSTATE_SLEEPING))
			break;
		Delay(60000);
	}
	return false;
}

void WintergraspInternal::SendInitWorldStates(Player* plr)
{
	WorldPacket data(SMSG_INIT_WORLD_STATES, (4+4+4+2));
	data << uint32(571);
	data << uint32(WINTERGRASP);
	data << uint32(0);
	data << uint16(4+5+(WG ? 4 : 0));

	data << uint32(3803) << uint32(defendingteam == ALLIANCE ? 1 : 0);
	data << uint32(3802) << uint32(defendingteam == HORDE ? 1 : 0);
	data << uint32(3801) << uint32(WG_started ? 0 : 1);
	data << uint32(3710) << uint32(WG_started ? 1 : 0);

	for (uint32 i = 0; i < 5; i++)
		data << ClockWorldState[i] << m_clock[i];

	if(WG_started && WG != NULL)
	{
		data << uint32(A_NUMVEH_WORLDSTATE) << uint32(WG->GetNumVehicles(ALLIANCE));
		data << uint32(A_MAXVEH_WORLDSTATE) << uint32(WG->GetNumWorkshops(ALLIANCE)*4);
		data << uint32(H_NUMVEH_WORLDSTATE) << uint32(WG->GetNumVehicles(HORDE));
		data << uint32(H_MAXVEH_WORLDSTATE) << uint32(WG->GetNumWorkshops(HORDE)*4);
	}

	if(plr != NULL)
		plr->GetSession()->SendPacket(&data);
	else
	{
		if(WG != NULL)
		{
			if(WG->WGPlayers.size())
			{
				for(WintergraspPlayerSet::iterator itr = WG->WGPlayers.begin(); itr != WG->WGPlayers.end(); itr++)
				{
					WorldPacket* data2 = &data;
					(*itr)->GetSession()->SendPacket(data2);
				}
			}
		}
		else
		{
			if(WGMgr->m_PlayerStorage.size())
			{
				for(PlayerStorageMap::iterator itr =  WGMgr->m_PlayerStorage.begin(); itr != WGMgr->m_PlayerStorage.end(); itr++)
				{
					plr = itr->second;
					if((plr->GetPAreaID() == WINTERGRASP) || (plr->GetZoneId() == WINTERGRASP))
					{
						WorldPacket* data2 = &data;
						plr->GetSession()->SendPacket(data2);
					}
				}
				plr = NULL;
			}
		}
	}
}

void WintergraspInternal::UpdateClockDigit(uint32 timer, uint32 digit, uint32 mod)
{
	uint32 value = timer%mod;
	if (m_clock[digit] != value)
	{
		m_clock[digit] = value;
		SendWSUpdateToAll(ClockWorldState[digit], timer+time(NULL));
	}
	WGMgr->GetStateManager().UpdateWorldState(m_clock[digit], timer+time(NULL));
}

void WintergraspInternal::SendWSUpdateToAll(uint32 WorldState, uint32 Value)
{
	WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
	data << WorldState;
	data << Value;
	SendPacketToWG(&data);
}

void WintergraspInternal::SendPacketToWG(WorldPacket* data)
{
	Player* plr = NULL;
	if(WG != NULL)
	{
		for(WintergraspPlayerSet::iterator itr = WG->WGPlayers.begin(); itr != WG->WGPlayers.end(); itr++)
			(*itr)->GetSession()->SendPacket(data);
	}
	else
	{
		for(PlayerStorageMap::iterator itr =  WGMgr->m_PlayerStorage.begin(); itr != WGMgr->m_PlayerStorage.end(); itr++)
		{
			plr = itr->second;
			if((plr->GetPAreaID() == WINTERGRASP) || (plr->GetZoneId() == WINTERGRASP))
				plr->GetSession()->SendPacket(data);
		}
	}
}

void WintergraspInternal::UpdateClock()
{
	uint32 timer = m_timer / 1000;
	UpdateClockDigit(timer, 0, 10);
	if(!WG_started)
		UpdateClockDigit(timer, 4, 10);
}

void WintergraspInternal::StartWintergrasp()
{
	WG_started = true;
	m_wintergrasp = 1;
	SendInitWorldStates();
	MatchTimer = 1800000;
}

void WintergraspInternal::EndWintergrasp()
{
	WG_started = false;
	WG = NULL;
	m_wintergrasp = 0;
	SendInitWorldStates();
	MatchTimer = 0;
}
