
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool TidalForce(uint32 i, Spell* pSpell)
{
	pSpell->SpellEffectTriggerSpell(i);
	pSpell->SpellEffectTriggerSpell(i);
	pSpell->SpellEffectTriggerSpell(i);	// want stack of 3 so cast it 3 times. hacky :/
	return true;
}

bool ManaTide(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(unitTarget != NULL && unitTarget->isAlive())
	{
		uint32 gain = (uint32) (unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1)*0.06);
		if(gain)
			unitTarget->Energize(unitTarget, 16191, gain, POWER_TYPE_MANA);
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////

void Lacrimi::SetupShamanSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(55198, TidalForce);

	RegisterDummySpell(39610, ManaTide);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
//	RegisterSpellEffectModifier();
}
