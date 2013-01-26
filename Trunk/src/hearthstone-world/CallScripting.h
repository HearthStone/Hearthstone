/***
 * Demonstrike Core
 */

/*	Crow: This style of scripting can be used with C++ scripts to add temporary hooks to what the player/creature is doing.
	While it is up to the creater of the script to clean these up, the core will automatically delete it when a player changes maps. */

#pragma once

class Unit;
class Creature;

class SERVER_DECL UnitOnKillUnitScript
{
public:
	UnitOnKillUnitScript() { links = 0; };
	virtual ~UnitOnKillUnitScript() {};

	virtual void UnitOnKillUnit(Unit* unit,  Unit* uVictim) {}
	uint64 links;
};

class SERVER_DECL UnitOnDeathScript
{
public:
	UnitOnDeathScript() {};
	virtual ~UnitOnDeathScript() { links = 0; };

	virtual void UnitOnDeath(Unit* unit) {}
	uint64 links;
};

class SERVER_DECL UnitOnEnterCombatScript
{
public:
	UnitOnEnterCombatScript() { links = 0; };
	virtual ~UnitOnEnterCombatScript() {};

	virtual void UnitOnEnterCombat(Unit* unit, Unit* uTarget) {}
	uint64 links;
};

class SERVER_DECL UnitOnCastSpellScript
{
public:
	UnitOnCastSpellScript() { links = 0; };
	virtual ~UnitOnCastSpellScript() {};

	virtual void UnitOnCastSpell(Unit* unit, SpellEntry * uSpell) {}
	uint64 links;
};
