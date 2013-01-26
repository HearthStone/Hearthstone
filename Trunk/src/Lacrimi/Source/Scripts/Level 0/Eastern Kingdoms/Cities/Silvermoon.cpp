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

class BloodKnightStillblade : public CreatureAIScript
{
public:
	ADD_CREATURE_FACTORY_FUNCTION(BloodKnightStillblade);
	BloodKnightStillblade(Creature *pCreature) : CreatureAIScript(pCreature) {}

	void Reset()
	{
		if(active)
		{
			_unit->SetPosition(_unit->GetSpawnX(), _unit->GetSpawnY(), _unit->GetSpawnZ(), _unit->GetSpawnO(), true);
			lifetimer = 120000;
			timediff = UNIXTIME;
			_unit->SetUInt32Value(UNIT_NPC_FLAGS, 1);
			_unit->SetStandState(STANDSTATE_DEAD);
			_unit->setDeathState(CORPSE);
			_unit->GetAIInterface()->setCanMove(false);
		}
	}

	void OnLoad()
	{
		if(_unit->m_spawn)
		{
			active = true;
			Reset();
		}
	}

	void AIUpdate(MapManagerScript* MMScript, uint32 p_time)
	{
		if(active)
		{
			uint32 diff = UNIXTIME - timediff;
			if(_unit->GetStandState() == STANDSTATE_STAND)
			{
				if (lifetimer < diff)
					Reset();
			}
		}
	}
	
protected:
	Creature* BKStillblade;
	uint32 lifetimer;
	uint32 timediff;
	bool active;
};


void Lacrimi::SetupSilvermoon()
{
	RegisterCtrAIScript(17768, BloodKnightStillblade);
}
