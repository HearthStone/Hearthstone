
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool Penance(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL )
	{
		uint32 hostileSpell[] = {0, 0};
		uint32 friendlySpell[] = {0, 0};
		switch( pSpell->GetSpellProto()->Id )
		{
		case 47540: //Rank 1
			hostileSpell[0] = 47666;
			hostileSpell[1] = 47758;

			friendlySpell[0] = 47750;
			friendlySpell[1] = 47757;
			break;
		case 53005:
			hostileSpell[0] = 52998;
			hostileSpell[1] = 53001;

			friendlySpell[0] = 52983;
			friendlySpell[1] = 52986;
			break;
		case 53006:
			hostileSpell[0] = 52999;
			hostileSpell[1] = 53002;

			friendlySpell[0] = 52984;
			friendlySpell[1] = 52987;
			break;
		case 53007:
			hostileSpell[0] = 53000;
			hostileSpell[1] = 53003;

			friendlySpell[0] = 52985;
			friendlySpell[1] = 52988;
			break;
		}

		if( isAttackable(pSpell->p_caster, unitTarget) ) // Do holy damage
		{
			// First tick is instant.
			pSpell->p_caster->CastSpell(unitTarget, hostileSpell[0], true);
			pSpell->p_caster->CastSpell(unitTarget, hostileSpell[1], false);
		}
		else // Heal
		{
			pSpell->p_caster->CastSpell(unitTarget, friendlySpell[0], true);
			pSpell->p_caster->CastSpell(unitTarget, friendlySpell[1], false);
		}
	}
	return true;
}

bool HolyNova(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL )
	{
		uint32 hostileSpell = pSpell->GetSpellProto()->EffectTriggerSpell[0];
		uint32 friendlySpell = pSpell->GetSpellProto()->EffectTriggerSpell[1];

		// Do holy damage
		if(isAttackable(pSpell->m_caster, unitTarget))
			pSpell->m_caster->CastSpell(unitTarget, hostileSpell, true);
		else
			pSpell->m_caster->CastSpell(unitTarget, friendlySpell, true);
	}
	return true;
}

bool LightwellSpellClick(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->u_caster != NULL && unitTarget != NULL )
	{
		switch(pSpell->u_caster->GetEntry())
		{
		case 31883:
			{
				pSpell->u_caster->CastSpell(unitTarget, 48085, true);
			}break;
		case 31893:
			{
				pSpell->u_caster->CastSpell(unitTarget, 48084, true);
			}break;
		case 31894:
			{
				pSpell->u_caster->CastSpell(unitTarget, 28276, true);
			}break;
		case 31895:
			{
				pSpell->u_caster->CastSpell(unitTarget, 27874, true);
			}break;
		case 31896:
			{
				pSpell->u_caster->CastSpell(unitTarget, 27873, true);
			}break;
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void ShadowWordDeath(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE && pSpell->p_caster != NULL )
		pSpell->p_caster->CastSpell(pSpell->u_caster, 32409, true);
}

void Lacrimi::SetupPriestSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(47540, Penance);
	RegisterDummySpell(53005, Penance);
	RegisterDummySpell(53006, Penance);
	RegisterDummySpell(53007, Penance);

	RegisterDummySpell(15237, HolyNova);
	RegisterDummySpell(15430, HolyNova);
	RegisterDummySpell(15431, HolyNova);
	RegisterDummySpell(27799, HolyNova);
	RegisterDummySpell(27800, HolyNova);
	RegisterDummySpell(27801, HolyNova);
	RegisterDummySpell(25331, HolyNova);
	RegisterDummySpell(48077, HolyNova);
	RegisterDummySpell(48078, HolyNova);

	RegisterSpellScriptEffect(60123, LightwellSpellClick);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(32379, ShadowWordDeath);
	RegisterSpellEffectModifier(32996, ShadowWordDeath);
	RegisterSpellEffectModifier(48157, ShadowWordDeath);
	RegisterSpellEffectModifier(48158, ShadowWordDeath);
}
