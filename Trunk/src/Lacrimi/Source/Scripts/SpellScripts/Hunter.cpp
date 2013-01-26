
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool SteadyShot(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL && pSpell->GetUnitTarget() != NULL )
		if ( pSpell->GetUnitTarget()->IsDazed() )
			pSpell->TotalDamage += pSpell->p_caster->Strike( pSpell->GetUnitTarget(), RANGED, NULL, 0, 0, pSpell->damage, false, false );
	return true;
}

bool ChimeraShotEffect(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL )
	{
		SpellEntry* sting_proto = NULL;
		uint32 actdur = 0;
		Aura* pAura = unitTarget->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_SERPENT_STING);
		if(pAura == NULL)
			pAura = unitTarget->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_VIPER_STING);
		if(pAura == NULL)
			pAura = unitTarget->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_SCORPID_STING);
		if(pAura != NULL)
		{
			actdur = pAura->GetBaseDuration();
			sting_proto = pAura->m_spellProto;
		}

		if( sting_proto != NULL )
		{
			pSpell->p_caster->CastSpell(unitTarget, sting_proto->Id, true);

			//trigger effect
			switch( sting_proto->NameHash )
			{
			case SPELL_HASH_SERPENT_STING:
				{
					uint32 ticks = float2int32(actdur * 0.003f);

					SpellEntry* spellInfo = dbcSpell.LookupEntryForced( 53353 );
					Spell* sp = new Spell( pSpell->p_caster, spellInfo, true, NULLAURA );
					sp->forced_basepoints[0] = float2int32(((sting_proto->EffectBasePoints[0] / 5) * ticks) * 0.4f);
					SpellCastTargets tgt;
					tgt.m_unitTarget = unitTarget->GetGUID();
					sp->prepare(&tgt);
				}break;
			case SPELL_HASH_VIPER_STING:
				{
					uint32 ticks = float2int32(actdur * 0.002f);

					SpellEntry* spellInfo = dbcSpell.LookupEntryForced( 53358 );
					Spell* sp = new Spell( pSpell->p_caster, spellInfo, true, NULLAURA );
					sp->forced_basepoints[0] = float2int32(((unitTarget->GetUInt32Value(UNIT_FIELD_MAXPOWER1) * 0.04f) * ((16 / 4) * ticks)) * 0.6f);
					SpellCastTargets tgt;
					tgt.m_unitTarget = pSpell->p_caster->GetGUID();
					sp->prepare(&tgt);
				}break;
			case SPELL_HASH_SCORPID_STING:
				{
					if( pSpell->p_caster->m_CustomTimers[CUSTOM_TIMER_CHIMERA_SCORPID] <= getMSTime() )
					{
						pSpell->p_caster->m_CustomTimers[CUSTOM_TIMER_CHIMERA_SCORPID] = getMSTime() + MSTIME_MINUTE;
						pSpell->p_caster->CastSpell( unitTarget, 53359, true);
					}
				}break;
			}
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void ArcaneShot(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		uint32 dmg = pSpell->damage;
		if( pSpell->p_caster != NULL)
		{
			if(dmg > (pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2))
				dmg = (pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2);
		}
		pSpell->damage = dmg;
	}
}

void AimedShot(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->damage = CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), RANGED, NULL) + pSpell->GetSpellProto()->EffectBasePoints[i];
			if(pSpell->p_caster->HasAurasOfNameHashWithCaster(SPELL_HASH_RAPID_KILLING, pSpell->u_caster)) //Aimed/Auto shot deals Y% extra damage with rapid killing buff
				pSpell->damage += float2int32(pSpell->damage * (pSpell->GetSpellProto()->RankNumber * 0.1f));
		}
	}
}

void ChimeraShot(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->damage = CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), RANGED, NULL) * 1.25;
			if(pSpell->p_caster->HasAurasOfNameHashWithCaster(SPELL_HASH_RAPID_KILLING, pSpell->u_caster))
				pSpell->damage += float2int32(pSpell->damage * (pSpell->GetSpellProto()->RankNumber * 0.1f));
		}
	}
}

void SteadyShotDMG(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->static_damage = true;

			// Stun Damage
			uint32 stundmg;
			if(pSpell->GetUnitTarget()->IsDazed())
				stundmg = pSpell->p_caster->GetRAP()/10 + pSpell->GetSpellProto()->EffectBasePoints[i] + pSpell->GetSpellProto()->EffectBasePoints[i + 1];
			else
				stundmg = pSpell->p_caster->GetRAP()/10 + pSpell->GetSpellProto()->EffectBasePoints[i];

			// Ammo Damage
			float ammodmg, bowdmg = CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), RANGED, NULL);
			if(ItemPrototype* ip = ItemPrototypeStorage.LookupEntry(pSpell->p_caster->GetUInt32Value(PLAYER_AMMO_ID)))
				ammodmg = (ip->Damage[0].Min + ip->Damage[0].Max) * 0.2f; //+unmodified ammo damage
			else
				ammodmg = 0;
			// Actual damage :D
			pSpell->damage = float2int32(ammodmg + bowdmg) + stundmg;
		}
	}
}

void Lacrimi::SetupHunterSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(34120, SteadyShot);
	RegisterDummySpell(49051, SteadyShot);
	RegisterDummySpell(49052, SteadyShot);
	RegisterDummySpell(56641, SteadyShot);
	RegisterDummySpell(65867, SteadyShot);

	RegisterDummySpell(53209, ChimeraShotEffect);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(3044, ArcaneShot);
	RegisterSpellEffectModifier(14281, ArcaneShot);
	RegisterSpellEffectModifier(14282, ArcaneShot);
	RegisterSpellEffectModifier(14283, ArcaneShot);
	RegisterSpellEffectModifier(14284, ArcaneShot);
	RegisterSpellEffectModifier(14285, ArcaneShot);
	RegisterSpellEffectModifier(14286, ArcaneShot);
	RegisterSpellEffectModifier(14287, ArcaneShot);
	RegisterSpellEffectModifier(27019, ArcaneShot);
	RegisterSpellEffectModifier(49044, ArcaneShot);
	RegisterSpellEffectModifier(49045, ArcaneShot);

	RegisterSpellEffectModifier(19434, AimedShot);
	RegisterSpellEffectModifier(20900, AimedShot);
	RegisterSpellEffectModifier(20901, AimedShot);
	RegisterSpellEffectModifier(20902, AimedShot);
	RegisterSpellEffectModifier(20903, AimedShot);
	RegisterSpellEffectModifier(20904, AimedShot);
	RegisterSpellEffectModifier(27065, AimedShot);
	RegisterSpellEffectModifier(49049, AimedShot);
	RegisterSpellEffectModifier(49050, AimedShot);

	RegisterSpellEffectModifier(53209, ChimeraShot);

	RegisterSpellEffectModifier(34120, SteadyShotDMG);
	RegisterSpellEffectModifier(49051, SteadyShotDMG);
	RegisterSpellEffectModifier(49052, SteadyShotDMG);
	RegisterSpellEffectModifier(56641, SteadyShotDMG);
}
