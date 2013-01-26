
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool HeroicFury(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL )
		pSpell->p_caster->ClearCooldownForSpell( 20252 );
	return true;
}

bool SwordAndBoard(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL)
	{
		uint32 ClearSpellId[8] =
		{
			23922,  /* Shield Slam - Rank 1 */
			23923,  /* Shield Slam - Rank 2 */
			23924,  /* Shield Slam - Rank 3 */
			23925,  /* Shield Slam - Rank 4 */
			25258,  /* Shield Slam - Rank 5 */
			30356,  /* Shield Slam - Rank 6 */
			47487,  /* Shield Slam - Rank 7 */
			47488,  /* Shield Slam - Rank 8 */
		};

		for(i = 0; i < 8; i++)
			pSpell->p_caster->ClearCooldownForSpell( ClearSpellId[i] );
	}
	return true;
}

bool Execute(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->u_caster == NULL|| !pSpell->u_caster->IsInWorld() || !unitTarget || !unitTarget->IsInWorld() || !pSpell->GetSpellProto())
		return true;

	int32 value = pSpell->GetSpellProto()->EffectBasePoints[i]+1;
	int32 rageUsed = pSpell->p_caster->GetUInt32Value(UNIT_FIELD_POWER2);

	int32 rageLeft = 0; // We use all available rage by default
	Aura* suddenDeath = NULL;
	suddenDeath = pSpell->u_caster->m_AuraInterface.FindActiveAura(52437);

	if(suddenDeath != NULL && unitTarget->GetHealthPct() > 20)
	{
		SpellEntry * sd = NULL;
		sd = dbcSpell.LookupEntry(suddenDeath->pSpellId);
		if(sd!=NULL)
			rageLeft = sd->RankNumber > 1 ? sd->RankNumber * 30 + 10 : sd->RankNumber * 30;
		unitTarget->RemoveAura(suddenDeath); // Sudden Death is removed after 1 execute
		// With Sudden Death can only use up to 30 total rage. so 30-15 = 15 goes to execute damage
		rageLeft = std::max(rageLeft, rageUsed - 150);
		rageUsed = std::min(rageUsed, 150);
	}

	if( pSpell->u_caster->HasDummyAura(SPELL_HASH_GLYPH_OF_EXECUTION) )
		rageUsed += 100; //Your Execute ability deals damage as if you had 10 additional rage.

	value += (int32) (rageUsed * pSpell->GetSpellProto()->dmg_multiplier[0]);
	pSpell->u_caster->SetPower(POWER_TYPE_RAGE, rageLeft);
	SpellEntry *spellInfo = dbcSpell.LookupEntry(20647);
	pSpell->TotalDamage += pSpell->u_caster->Strike(unitTarget,MELEE,spellInfo,0,0,value,false,false);
	return true;
}

bool DamageShield(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
		pSpell->p_caster->Damageshield_amount = pSpell->GetSpellProto()->EffectBasePoints[i]+1;
	return true;
}

bool LastStand(uint32 i, Spell* pSpell)
{
	SpellEntry *inf = dbcSpell.LookupEntry(12976);
	if(pSpell->p_caster != NULL && inf != NULL)
	{
		Spell *spe = new Spell(pSpell->u_caster, inf, true, NULLAURA);
		SpellCastTargets tgt(pSpell->GetPlayerTarget()->GetGUID());
		spe->prepare(&tgt);
	}
	return true;
}

bool Vigilance(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		Aura* aur = pSpell->p_caster->m_AuraInterface.FindAura( 50720 );
		if( aur  != NULL )
		{
			Unit* caster = aur->GetUnitCaster();
			if( caster != NULL && caster->IsPlayer() )
				TO_PLAYER(caster)->ClearCooldownForSpell(355);
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void ShieldSlam(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
		pSpell->damage += pSpell->p_caster->GetUInt32Value(PLAYER_SHIELD_BLOCK);
}

void VictoryRush(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
		if(pSpell->u_caster->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_VICTORIOUS))
			pSpell->u_caster->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_VICTORIOUS);
}

void HeroicStrike(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if(pSpell->GetUnitTarget()->IsDazed())
		{
			switch(pSpell->GetSpellProto()->Id)
			{ // This info isn't in the dbc files.....
			case 29707:
				{
					pSpell->damage += 81.9;
				}break;
			case 30324:
				{
					pSpell->damage += 110.95;
				}break;
			case 47449:
				{
					pSpell->damage += 151.2;
				}break;
			case 47450:
				{
					pSpell->damage += 173.25;
				}break;
			}
		}
	}
}

void Cleave(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if(pSpell->p_caster != NULL)
		{
			Item* itm = pSpell->p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
			if(pSpell->p_caster->HasAura(12329))
				pSpell->damage += (CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL) + pSpell->GetSpellProto()->EffectBasePoints[i]) * 1.4;
			else if(pSpell->p_caster->HasAura(12950))
				pSpell->damage += (CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL) + pSpell->GetSpellProto()->EffectBasePoints[i]) * 1.8;
			else if(pSpell->p_caster->HasAura(20496))
				pSpell->damage += (CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL) + pSpell->GetSpellProto()->EffectBasePoints[i]) * 2.2;
			else
				pSpell->damage += CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL) + pSpell->GetSpellProto()->EffectBasePoints[i];
		}
	}
}

void DamageShieldDMG(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if(pSpell->p_caster != NULL)
		{
			Item* shield = pSpell->p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
			if(shield != NULL && shield->GetProto() && shield->GetProto()->InventoryType == INVTYPE_SHIELD)
			{
				if(pSpell->p_caster->Damageshield_amount)
				{
					pSpell->damage = ((pSpell->p_caster->GetUInt32Value(PLAYER_SHIELD_BLOCK)*pSpell->p_caster->Damageshield_amount)/100);
				}
			}
		}
	}
}

void SlamDMG(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if(pSpell->u_caster != NULL)
		{
			pSpell->damage = pSpell->GetSpellProto()->EffectBasePoints[i]+CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL);
		}
	}
}

void Lacrimi::SetupWarriorSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(60970, HeroicFury);

	RegisterDummySpell(50227, SwordAndBoard);

	RegisterDummySpell(5308, Execute);
	RegisterDummySpell(20658, Execute);
	RegisterDummySpell(20660, Execute);
	RegisterDummySpell(20661, Execute);
	RegisterDummySpell(20662, Execute);
	RegisterDummySpell(25234, Execute);
	RegisterDummySpell(25236, Execute);
	RegisterDummySpell(47470, Execute);
	RegisterDummySpell(47471, Execute);

	RegisterDummySpell(58872, DamageShield);
	RegisterDummySpell(58874, DamageShield);

	RegisterDummySpell(12975, LastStand);

	RegisterDummySpell(50725, Vigilance);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(23922, ShieldSlam);
	RegisterSpellEffectModifier(23923, ShieldSlam);
	RegisterSpellEffectModifier(23924, ShieldSlam);
	RegisterSpellEffectModifier(23925, ShieldSlam);
	RegisterSpellEffectModifier(25258, ShieldSlam);
	RegisterSpellEffectModifier(30356, ShieldSlam);
	RegisterSpellEffectModifier(47487, ShieldSlam);
	RegisterSpellEffectModifier(47488, ShieldSlam);

	RegisterSpellEffectModifier(34428, VictoryRush);

	RegisterSpellEffectModifier(29707, HeroicStrike);
	RegisterSpellEffectModifier(30324, HeroicStrike);
	RegisterSpellEffectModifier(47449, HeroicStrike);
	RegisterSpellEffectModifier(47450, HeroicStrike);

	RegisterSpellEffectModifier(845, Cleave);
	RegisterSpellEffectModifier(7369, Cleave);
	RegisterSpellEffectModifier(11608, Cleave);
	RegisterSpellEffectModifier(11609, Cleave);
	RegisterSpellEffectModifier(20569, Cleave);
	RegisterSpellEffectModifier(25231, Cleave);
	RegisterSpellEffectModifier(47519, Cleave);
	RegisterSpellEffectModifier(47520, Cleave);

	RegisterSpellEffectModifier(1464, SlamDMG);
	RegisterSpellEffectModifier(8820, SlamDMG);
	RegisterSpellEffectModifier(11604, SlamDMG);
	RegisterSpellEffectModifier(11605, SlamDMG);
	RegisterSpellEffectModifier(25241, SlamDMG);
	RegisterSpellEffectModifier(25242, SlamDMG);
	RegisterSpellEffectModifier(47474, SlamDMG);
	RegisterSpellEffectModifier(47475, SlamDMG);

	RegisterSpellEffectModifier(59653, DamageShieldDMG);
}
