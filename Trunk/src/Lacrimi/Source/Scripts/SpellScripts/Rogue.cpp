
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool HungerForBlood(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		if( pSpell->p_caster->m_AuraInterface.RemoveAllAurasByMechanic( MECHANIC_BLEEDING , 1 , true ) )
		{	// Give 15 energy.
			pSpell->p_caster->Energize(pSpell->p_caster, 51662, 15, POWER_TYPE_ENERGY);
		}
	}
	return true;
}

bool DisarmTrap(uint32 i, Spell* pSpell)
{
	GameObject* gameObjTarget = pSpell->GetGameObjectTarget();
	if(pSpell->p_caster != NULL && gameObjTarget != NULL )
	{
		if( gameObjTarget->GetType() == GAMEOBJECT_TYPE_TRAP )
			gameObjTarget->_Expire();
	}
	return true;
}

bool Preparation(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		uint32 ClearSpellId[10] =
		{
			5277,  /* Evasion - Rank 1 */
			26669, /* Evasion - Rank 2 */
			2983,  /* Sprint  - Rank 1 */
			8696,  /* Sprint  - Rank 2 */
			11305, /* Sprint  - Rank 3 */
			1856,  /* Vanish  - Rank 1 */
			1857,  /* Vanish  - Rank 2 */
			26889, /* Vanish  - Rank 3 */
			14177, /* Cold Blood	   */
			36554  /* Shadowstep	   */
		};

		for(i = 0; i < 10; i++)
			pSpell->p_caster->ClearCooldownForSpell( ClearSpellId[i] );
	}
	return true;
}

bool CloakOfShadows(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->p_caster != NULL && unitTarget != NULL && unitTarget->isAlive())
		unitTarget->m_AuraInterface.RemoveAllAurasWithAttributes(ATTRIBUTES_IGNORE_INVULNERABILITY);
	return true;
}

bool Shiv(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->p_caster != NULL && unitTarget != NULL && unitTarget->isAlive())
	{
		Item* Offhand = pSpell->p_caster->GetItemInterface()->GetInventoryItem( EQUIPMENT_SLOT_OFFHAND);
		if( Offhand != NULL && Offhand->GetEnchantment(1) != NULL )
		{
			SpellEntry* spellInfo = dbcSpell.LookupEntry(Offhand->GetEnchantment(1)->Enchantment->spell[0]);
			if(spellInfo && spellInfo->c_is_flags & SPELL_FLAG_IS_POISON )
			{
				Spell* spell(new Spell( pSpell->p_caster, spellInfo, true, NULLAURA));
				SpellCastTargets targets;
				targets.m_unitTarget = unitTarget->GetGUID();
				spell->prepare( &targets );
			}
		}
	}
	return true;
}

bool Vanish(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		pSpell->p_caster->m_AuraInterface.RemoveAllAurasByMechanic(7);
		pSpell->p_caster->m_AuraInterface.RemoveAllAurasByMechanic(11);
		pSpell->p_caster->m_AuraInterface.RemoveAuraNegByNameHash(SPELL_HASH_HUNTER_S_MARK);
		pSpell->p_caster->CastSpell(pSpell->p_caster->GetGUID(), dbcSpell.LookupEntryForced(1784), true);
	}
	return true;
}

bool ImprovedSprint(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		pSpell->p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_ROOT);
		pSpell->p_caster->m_AuraInterface.RemoveAllAurasWithAuraName(SPELL_AURA_MOD_DECREASE_SPEED);
	}
	return true;
}

bool NullComboPoints(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
		pSpell->p_caster->NullComboPoints();
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void Gouge(uint32 i, Spell* pSpell, uint32 effect)
{
	if(i == 0)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->p_caster->EventAttackStop();
			pSpell->p_caster->smsg_AttackStop( pSpell->GetUnitTarget() );
		}
	}
}

void Blind(uint32 i, Spell* pSpell, uint32 effect)
{
	if(i == 0)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->p_caster->EventAttackStop();
			pSpell->p_caster->smsg_AttackStop( pSpell->GetUnitTarget() );
		}
	}
}

void Eviserate(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->static_damage = true;

			uint32 perc = pSpell->p_caster->m_comboPoints * 3;
			perc += (RandomUInt(5) * pSpell->p_caster->m_comboPoints);
			pSpell->damage += float2int32(pSpell->p_caster->GetAP() * ( perc * 0.01f ));//UINT =+ INT + FLOAT = 0 LOL
			if( pSpell->p_caster->HasDummyAura(SPELL_HASH_CUT_TO_THE_CHASE) )
			{
				Aura* aur = pSpell->p_caster->m_AuraInterface.FindPositiveAuraByNameHash(SPELL_HASH_SLICE_AND_DICE);
				if( aur )
				{
					aur->SetDuration(21000);
					aur->SetTimeLeft(21000);
				}
			}
		}
	}
}

void Envenom(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->static_damage = true;

			// Lets find all deadly poisons...
			uint32 dosestoeat = 0;
			uint32 doses = pSpell->GetUnitTarget()->GetPoisonDosesCount( POISON_TYPE_DEADLY );
			if( doses )
			{
				if (doses <= uint32(pSpell->p_caster->m_comboPoints))
					dosestoeat = doses;
				else
					dosestoeat = pSpell->p_caster->m_comboPoints;

				uint32 bpdamage = pSpell->GetSpellProto()->EffectBasePoints[i] + 1;
				pSpell->damage = ( bpdamage * dosestoeat) + float2int32(pSpell->p_caster->GetAP() * (0.07f * dosestoeat));

				// Remove deadly poisons
				pSpell->GetUnitTarget()->m_AuraInterface.UpdateDeadlyPoisons(dosestoeat);
			}
		}
	}
}

void InstantPoison(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		pSpell->static_damage = true;
		switch(pSpell->GetSpellProto()->Id)
		{
		case 8680: // Instant Poison 1
			{
				pSpell->damage = (13+RandomUInt(4));
			}break;
		case 8685: // Instant Poison 2
			{
				pSpell->damage = (21+RandomUInt(4));
			}break;
		case 8689: // Instant Poison 3
			{
				pSpell->damage = (30+RandomUInt(8));
			}break;
		case 11335: // Instant Poison 4
			{
				pSpell->damage = (45+RandomUInt(12));
			}break;
		case 11336: // Instant Poison 5
			{
				pSpell->damage = (62+RandomUInt(18));
			}break;
		case 11337: // Instant Poison 6
			{
				pSpell->damage = (76+RandomUInt(24));
			}break;
		case 26890: // Instant Poison 7
			{
				pSpell->damage = (161+RandomUInt(54));
			}break;
		case 57964: // Instant Poison 8
			{
				pSpell->damage = (245+RandomUInt(82));
			}break;
		case 57965: // Instant Poison 9
			{
				pSpell->damage = (300+RandomUInt(100));
			}break;
		}
	}
}

void Lacrimi::SetupRogueSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(51662, HungerForBlood);

	RegisterDummySpell(1842, DisarmTrap);

	RegisterDummySpell(14185, Preparation);

	RegisterDummySpell(35729, CloakOfShadows);

	RegisterDummySpell(5938, Shiv);

	RegisterDummySpell(18461, Vanish);

	RegisterDummySpell(30918, ImprovedSprint);

	// Expose Armor
	RegisterDummySpell(8647, NullComboPoints);
	RegisterDummySpell(8649, NullComboPoints);
	RegisterDummySpell(8650, NullComboPoints);
	RegisterDummySpell(11197, NullComboPoints);
	RegisterDummySpell(11198, NullComboPoints);
	RegisterDummySpell(26866, NullComboPoints);
	RegisterDummySpell(48669, NullComboPoints);

	// Kidney Shot
	RegisterDummySpell(408, NullComboPoints);
	RegisterDummySpell(8643, NullComboPoints);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(1776, Gouge);

	RegisterSpellEffectModifier(2094, Blind);

	RegisterSpellEffectModifier(2098, Eviserate);
	RegisterSpellEffectModifier(6760, Eviserate);
	RegisterSpellEffectModifier(6761, Eviserate);
	RegisterSpellEffectModifier(6762, Eviserate);
	RegisterSpellEffectModifier(8623, Eviserate);
	RegisterSpellEffectModifier(8624, Eviserate);
	RegisterSpellEffectModifier(11299, Eviserate);
	RegisterSpellEffectModifier(11300, Eviserate);
	RegisterSpellEffectModifier(26865, Eviserate);
	RegisterSpellEffectModifier(31016, Eviserate);
	RegisterSpellEffectModifier(48667, Eviserate);
	RegisterSpellEffectModifier(48668, Eviserate);

	RegisterSpellEffectModifier(32645, Envenom);
	RegisterSpellEffectModifier(32684, Envenom);
	RegisterSpellEffectModifier(57992, Envenom);
	RegisterSpellEffectModifier(57993, Envenom);

	RegisterSpellEffectModifier(8680, InstantPoison);
	RegisterSpellEffectModifier(8685, InstantPoison);
	RegisterSpellEffectModifier(8689, InstantPoison);
	RegisterSpellEffectModifier(11335, InstantPoison);
	RegisterSpellEffectModifier(11336, InstantPoison);
	RegisterSpellEffectModifier(11337, InstantPoison);
	RegisterSpellEffectModifier(26890, InstantPoison);
	RegisterSpellEffectModifier(57964, InstantPoison);
	RegisterSpellEffectModifier(57965, InstantPoison);
}
