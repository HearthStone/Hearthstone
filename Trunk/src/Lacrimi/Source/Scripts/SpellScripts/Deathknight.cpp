
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool RuneQuestHandle(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster == NULL)
		return true;

	QuestLogEntry* qle = pSpell->p_caster->GetQuestLogForEntry(12842);
	if(qle != NULL && !qle->CanBeFinished())
	{
		qle->SetMobCount(0, 1);
		qle->SendUpdateAddKill(1);
		qle->SendQuestComplete();
		qle->UpdatePlayerFields();
	}
	return true;
}

bool DeathCoil(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->u_caster != NULL && unitTarget != NULL)
	{
		if(pSpell->GetSpellProto()->Id == 52375)
			pSpell->damage = pSpell->damage * 2 / 5;	// 40% for rank 1
		if(isAttackable(pSpell->u_caster, unitTarget, false))
			pSpell->TotalDamage += pSpell->u_caster->SpellNonMeleeDamageLog(unitTarget, pSpell->GetSpellProto()->Id, pSpell->damage, true);
		if(unitTarget->GetCreatureType() == UNDEAD)
		{
			pSpell->u_caster->Heal(unitTarget, pSpell->GetSpellProto()->Id, float2int32(pSpell->damage * 1.5f));
			pSpell->u_caster->Energize(pSpell->u_caster, 47541, 20, POWER_TYPE_RUNIC);
		}
	}
	return true;
}

bool DeathStrike(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->u_caster != NULL && unitTarget != NULL)
	{
		uint32 diseasecount = 0;
		uint32 diseases[2] = { 55078, 55095 };
		for(int8 i = 0; i < 2; i++)
			if(unitTarget->HasAura(diseases[i]))
				diseasecount++;

		if(diseasecount)
		{
			uint32 healamount = (pSpell->u_caster->GetMaxHealth()/20)*diseasecount;
			pSpell->u_caster->Heal(pSpell->u_caster, pSpell->GetSpellProto()->Id, healamount);
		}
	}
	return true;
}

bool Obliterate(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(unitTarget != NULL)
	{
		uint32 diseasecount = 0;
		uint32 diseases[2] = { 55078, 55095 };
		for(int8 u = 0; u < 2; u++)
		{
			if(unitTarget->HasAura(diseases[u]))
			{
				diseasecount++;
				unitTarget->RemoveAura(diseases[u]);
			}
		}
	}
	return true;
}

bool DeathGrip(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->u_caster != NULL && unitTarget != NULL)
	{
		if(!(unitTarget->IsCreature() && isTargetDummy(unitTarget->GetEntry())))
		{
			// Move Effect
			unitTarget->CastSpellAoF( pSpell->u_caster->GetPositionX(), pSpell->u_caster->GetPositionY(), pSpell->u_caster->GetPositionZ(), dbcSpell.LookupEntryForced(49575), true);
			pSpell->u_caster->CastSpell( unitTarget, 51399, true ); // Taunt Effect
		}
	}
	return true;
}

bool HungeringCold(uint32 i, Spell* pSpell)
{
	if(pSpell->u_caster != NULL)
	{
		Spell* newSpell = new Spell(pSpell->u_caster, dbcSpell.LookupEntry(61058), true, NULLAURA);
		SpellCastTargets targets;
		targets.m_destX = pSpell->u_caster->GetPositionX();
		targets.m_destY = pSpell->u_caster->GetPositionY();
		targets.m_destZ = pSpell->u_caster->GetPositionZ();
		targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
		newSpell->prepare(&targets);
	}
	return true;
}

bool RaiseAlly(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(pSpell->p_caster != NULL && unitTarget->isDead() && unitTarget->IsPlayer())
	{
		unitTarget->setDeathState(DEAD);
 		CreatureInfo *ci = CreatureNameStorage.LookupEntry(30230);
		CreatureProto* cp = CreatureProtoStorage.LookupEntry(30230);
		Pet *summon = objmgr.CreatePet();
		summon->CreateAsSummon(cp, ci, NULL, pSpell->p_caster, NULL, pSpell->GetSpellProto(), 1, (uint32)320000); //Give it an extra seconds for being unpossesed.
		TO_PLAYER(unitTarget)->SetSummon(summon);

		summon->SetUInt32Value(UNIT_FIELD_LEVEL, unitTarget->getLevel());
		summon->SetUInt32Value(UNIT_FIELD_MAXHEALTH, unitTarget->GetMaxHealth());
		summon->SetUInt32Value(UNIT_FIELD_HEALTH, summon->GetMaxHealth());			
		summon->SetUInt32Value(UNIT_FIELD_RESISTANCES, pSpell->p_caster->GetUInt32Value(UNIT_FIELD_RESISTANCES));
		summon->AddSpell(dbcSpell.LookupEntry(62225), true); // Claw
		summon->AddSpell(dbcSpell.LookupEntry(47480), true); // Thrash
		summon->AddSpell(dbcSpell.LookupEntry(47481), true); // Gnaw
		summon->AddSpell(dbcSpell.LookupEntry(47482), true); // Leap
		summon->AddSpell(dbcSpell.LookupEntry(47484), true); // Huddle
		summon->AddSpell(dbcSpell.LookupEntry(51874), true); // Explode
		summon->SetPowerType(POWER_TYPE_ENERGY);
		summon->SetPower(POWER_TYPE_ENERGY,100);
		summon->SetMaxPower(POWER_TYPE_ENERGY,100);
		unitTarget->CastSpell(unitTarget, 46619, true);
	}
	return true;
}

bool DeathPact(uint32 i, Spell* pSpell)
{
	return true;
}

bool RaiseDead(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
		pSpell->p_caster->CastSpell(pSpell->p_caster, 46585, true );
	return true;
}

bool Pestilence(uint32 i, Spell* pSpell)
{
	if(pSpell->p_caster != NULL)
	{
		if(pSpell->p_caster->HasAura(63334)) // Glyph of Disease
		{
			uint num = 2;
			uint32 diseases[2] = { 55078, 55095 };
			Unit* unitTarget = pSpell->GetUnitTarget();
			for(uint i = 0; i < num; i++)
				if(unitTarget->HasAura(diseases[i]))
					pSpell->p_caster->CastSpell(unitTarget, diseases[i], true);
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////

void Lacrimi::SetupDeathKnightSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterSpellScriptEffect(53323, RuneQuestHandle);
	RegisterSpellScriptEffect(53331, RuneQuestHandle);
	RegisterSpellScriptEffect(53341, RuneQuestHandle);
	RegisterSpellScriptEffect(53342, RuneQuestHandle);
	RegisterSpellScriptEffect(53344, RuneQuestHandle);
	RegisterSpellScriptEffect(53362, RuneQuestHandle);
	RegisterSpellScriptEffect(54446, RuneQuestHandle);
	RegisterSpellScriptEffect(54447, RuneQuestHandle);
	RegisterSpellScriptEffect(54449, RuneQuestHandle);
	RegisterSpellScriptEffect(62158, RuneQuestHandle);
	RegisterSpellScriptEffect(70164, RuneQuestHandle);

	RegisterSpellScriptEffect(52375, DeathCoil);
	RegisterSpellScriptEffect(47541, DeathCoil);
	RegisterSpellScriptEffect(49892, DeathCoil);
	RegisterSpellScriptEffect(49893, DeathCoil);
	RegisterSpellScriptEffect(49894, DeathCoil);
	RegisterSpellScriptEffect(49895, DeathCoil);

	RegisterSpellScriptEffect(45463, DeathStrike);
	RegisterSpellScriptEffect(49923, DeathStrike);
	RegisterSpellScriptEffect(49924, DeathStrike);
	RegisterSpellScriptEffect(49998, DeathStrike);
	RegisterSpellScriptEffect(49999, DeathStrike);

	RegisterSpellScriptEffect(48743, Obliterate);

	RegisterSpellScriptEffect(49576, DeathGrip);

	RegisterSpellScriptEffect(61999, RaiseAlly);

	RegisterSpellScriptEffect(48743, DeathPact);

	RegisterSpellScriptEffect(46584, RaiseDead);

	RegisterSpellScriptEffect(50842, Pestilence);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
}
