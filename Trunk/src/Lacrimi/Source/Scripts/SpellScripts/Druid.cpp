
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool Starfall(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL)
	{
		SpellEntry* spellInfo= dbcSpell.LookupEntryForced( pSpell->damage );
		Spell* sp (new Spell(pSpell->p_caster, spellInfo, true, NULLAURA));
		SpellCastTargets targets;
		sp->FillAllTargetsInArea(i, pSpell->m_caster->GetPositionX(), pSpell->m_caster->GetPositionY(), pSpell->m_caster->GetPositionZ(), pSpell->GetRadius(i));
		sp->prepare(&targets);
	}
	return true;
}

bool ImprovedLeaderofthePack(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL)
		Aura::CreateProcTriggerSpell(pSpell->p_caster, pSpell->p_caster->GetGUID(), 34299, 34299, 100, PROC_ON_CRIT_ATTACK, PROC_TARGET_SELF);
	return true;
}

bool SavageRoar(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL)
	{	// We add an event so we can have the duration from combo points.
		sEventMgr.AddEvent(pSpell->p_caster, &Player::NullComboPoints, EVENT_COMBO_POINT_CLEAR_FOR_TARGET, 20, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
	}
	return true;
}

bool LivingRootoftheWildheart(uint32 i, Spell* pSpell)
{
	Player* playerTarget = pSpell->GetPlayerTarget();
	if( pSpell->p_caster != NULL && playerTarget != NULL )
	{
		uint32 NewSpell = 0;
		uint8 shape = playerTarget->GetShapeShift();
		switch( shape )
		{
		case FORM_NORMAL:
			NewSpell = 37344; break;
		case FORM_BEAR:
			NewSpell = 37340; break;
		case FORM_DIREBEAR:
			NewSpell = 37340; break;
		case FORM_CAT:
			NewSpell = 37341; break;
		case FORM_TREE:
			NewSpell = 37342; break;
		case FORM_MOONKIN:
			NewSpell = 37343; break;
		}
		if( NewSpell != 0 )
			pSpell->p_caster->CastSpell(playerTarget, NewSpell, true);
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void Maim(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->p_caster->EventAttackStop();
			pSpell->p_caster->smsg_AttackStop( pSpell->GetUnitTarget() );
		}
	}
}

void FerociousBite(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if( pSpell->p_caster != NULL )
		{
			pSpell->static_damage = true;
			int32 usedpower = std::max(pSpell->p_caster->GetUInt32Value(UNIT_FIELD_POWER4), (uint32)30);
			pSpell->damage += float2int32( ( pSpell->p_caster->m_comboPoints * pSpell->p_caster->GetAP() * 0.07f ) + ( (usedpower * pSpell->GetSpellProto()->dmg_multiplier[1] + pSpell->p_caster->GetAP()) / 410.0f ) );

			pSpell->p_caster->ModUnsigned32Value(UNIT_FIELD_POWER4, -usedpower);
		}
	}
}

void CatSwipe(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if( pSpell->p_caster != NULL )
		{
			// Get weapon damage.
			Item* it = pSpell->p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
			float wpdmg;
			if(it)
				wpdmg = (it->GetProto()->Damage[0].Min + it->GetProto()->Damage[0].Max) * 0.2f;
			else
				wpdmg = (pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MINDAMAGE) + pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXDAMAGE) * 0.2f);

			// Set damage. "Swipe nearby enemies, inflicting 250% weapon damage." This should work :|
			pSpell->damage = wpdmg * 2.5;
		}
	}
}

void Lacrimi::SetupDruidSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(2912, Starfall);
	RegisterDummySpell(8949, Starfall);
	RegisterDummySpell(8950, Starfall);
	RegisterDummySpell(8951, Starfall);
	RegisterDummySpell(9875, Starfall);
	RegisterDummySpell(9876, Starfall);
	RegisterDummySpell(25298, Starfall);
	RegisterDummySpell(26986, Starfall);
	RegisterDummySpell(48464, Starfall);
	RegisterDummySpell(48465, Starfall);

	RegisterDummySpell(34297, ImprovedLeaderofthePack);
	RegisterDummySpell(34300, ImprovedLeaderofthePack);

	RegisterDummySpell(52610, SavageRoar);

	RegisterDummySpell(37336, LivingRootoftheWildheart);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(22570, Maim);
	RegisterSpellEffectModifier(49802, Maim);

	RegisterSpellEffectModifier(22568, FerociousBite);
	RegisterSpellEffectModifier(22827, FerociousBite);
	RegisterSpellEffectModifier(22828, FerociousBite);
	RegisterSpellEffectModifier(22829, FerociousBite);
	RegisterSpellEffectModifier(31018, FerociousBite);
	RegisterSpellEffectModifier(24248, FerociousBite);
	RegisterSpellEffectModifier(48576, FerociousBite);
	RegisterSpellEffectModifier(48577, FerociousBite);

	RegisterSpellEffectModifier(62078, CatSwipe);
}
