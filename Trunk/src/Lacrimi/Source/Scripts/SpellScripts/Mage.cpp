
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool FrostWard(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(unitTarget == NULL)
		return true;
	for(std::list<struct ReflectSpellSchool*>::iterator i = unitTarget->m_reflectSpellSchool.begin(), i2;i != unitTarget->m_reflectSpellSchool.end();)
	{
		if(pSpell->GetSpellProto()->Id == (*i)->spellId)
		{
			i2 = i;
			++i;

			unitTarget->m_reflectSpellSchool.erase(i2);
		}
		else
			++i;
	}

	ReflectSpellSchool *rss = new ReflectSpellSchool;
	rss->chance = pSpell->GetSpellProto()->procChance;
	rss->spellId = pSpell->GetSpellProto()->Id;
	rss->require_aura_hash = 2161224959UL;
	rss->school = pSpell->GetSpellProto()->School;
	unitTarget->m_reflectSpellSchool.push_back(rss);
	return true;
}

bool ColdSnap(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL) // Clears our cooldowns on frost spells.
	{
		for(uint8 index = COOLDOWN_TYPE_SPELL; index < NUM_COOLDOWN_TYPES; index++)
		{
			PlayerCooldownMap cm = pSpell->p_caster->GetCooldownMap(index);
			for(PlayerCooldownMap::const_iterator itr = cm.begin(); itr != cm.end(); itr++)
			{
				uint32 spellid = itr->second.SpellId;
				SpellEntry *spellInfo = dbcSpell.LookupEntry(spellid);
				if(spellInfo)
				{
					uint32 cooldown = spellInfo->RecoveryTime ? spellInfo->RecoveryTime : spellInfo->CategoryRecoveryTime;
					if(spellInfo->SpellFamilyName == SPELLFAMILY_MAGE && (spellInfo->School == SCHOOL_FROST) && spellInfo->Id != 11958 && cooldown > 0)
						pSpell->p_caster->ClearCooldownForSpell(spellid);
				}
			}
		}
	}
	return true;
}

bool SummonWaterElemental(uint32 i, Spell* pSpell)
{
	if(pSpell->u_caster != NULL)
	{
		CreatureProto* cp = CreatureProtoStorage.LookupEntry(510);
		if(cp != NULL)
		{
			LocationVector loc = pSpell->u_caster->GetPosition();
			pSpell->SummonGuardian(i, NULL, cp, loc);
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void IceLance(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE && pSpell->GetUnitTarget()->HasFlag( UNIT_FIELD_AURASTATE, AURASTATE_FLAG_FROZEN)
		|| pSpell->GetUnitTarget()->m_frozenTargetCharges > 0 )
		pSpell->damage *= 3;
}

void FireShield(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
		pSpell->damage = pSpell->GetSpellProto()->EffectBasePoints[i] + 1;
}

void Lacrimi::SetupMageSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(11189, FrostWard);
	RegisterDummySpell(28332, FrostWard);

	RegisterDummySpell(11958, ColdSnap);
	RegisterDummySpell(31687, SummonWaterElemental);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(30455, IceLance);
	RegisterSpellEffectModifier(42913, IceLance);
	RegisterSpellEffectModifier(42914, IceLance);

	RegisterSpellEffectModifier(34913, FireShield);
	RegisterSpellEffectModifier(43043, FireShield);
	RegisterSpellEffectModifier(43044, FireShield);
}
