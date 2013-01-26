
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool HolyShock(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL && pSpell->GetUnitTarget() != NULL )
	{
		uint32 hostileSpell = 0;
		uint32 friendlySpell = 0;
		switch( pSpell->GetSpellProto()->Id )
		{
		case 20473: //Rank 1
			{
				hostileSpell = 25912;
				friendlySpell = 25914;
			}break;
		case 20929: //Rank 2
			{
				hostileSpell = 25911;
				friendlySpell = 25913;
			}break;
		case 20930: //Rank 3
			{
				hostileSpell = 25902;
				friendlySpell = 25903;
			}break;
		case 27174: //Rank 4
			{
				hostileSpell = 27176;
				friendlySpell = 27175;
			}break;
		case 33072: //Rank 5
			{
				hostileSpell = 33073;
				friendlySpell = 33074;
			}break;
		case 48824: //Rank 6
			{
				hostileSpell = 48822;
				friendlySpell = 48820;
			}break;
		case 48825: //Rank 7
			{
				hostileSpell = 48823;
				friendlySpell = 48821;
			}break;
		default:
			{
				hostileSpell = 48823;
				friendlySpell = 48821;
			}break;
		}

		// Do holy damage
		pSpell->p_caster->CastSpell(pSpell->GetUnitTarget(), isAttackable(pSpell->p_caster, pSpell->GetUnitTarget()) ? hostileSpell : friendlySpell, true);
	}
	return true;
}

bool RighteousDefense(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL )
	{
		//we will try to lure 3 enemies from our target
		Unit* targets[3];
		int targets_got = 0;
		for(unordered_set<Object* >::iterator itr = unitTarget->GetInRangeSetBegin(), i2; itr != unitTarget->GetInRangeSetEnd(); )
		{
			i2 = itr++;
			// don't add objects that are not units and that are dead
			if((*i2)->GetTypeId() != TYPEID_UNIT || !TO_UNIT(*i2)->isAlive())
				continue;

			Creature *cr = TO_CREATURE((*i2));
			if(cr->GetAIInterface()->GetNextTarget() == unitTarget)
				targets[targets_got++] = cr;
			if(targets_got == 3)
				break;
		}

		for(int i=0;i<targets_got;++i)
		{
			//set threat to this target so we are the msot hated
			uint32 threat_to_him = targets[i]->GetAIInterface()->getThreatByPtr( unitTarget );
			uint32 threat_to_us = targets[i]->GetAIInterface()->getThreatByPtr(pSpell->p_caster);
			int threat_dif = threat_to_him - threat_to_us;
			if(threat_dif > 0)//should nto happen
				targets[i]->GetAIInterface()->modThreatByPtr(pSpell->p_caster, threat_dif);
			targets[i]->GetAIInterface()->AttackReaction(pSpell->p_caster, 1, 0);
			targets[i]->GetAIInterface()->SetNextTarget(pSpell->p_caster);
		}
	}
	return true;
}

bool DivineStorm(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL && pSpell->TotalDamage > 0)
	{
		uint32 amt = float2int32(0.25f*pSpell->TotalDamage);
		if( pSpell->p_caster->GetGroup() )
		{
			uint32 count = 0;
			pSpell->p_caster->GetGroup()->Lock();
			for(uint32 x = 0; x < pSpell->p_caster->GetGroup()->GetSubGroupCount(); ++x)
			{
				for(GroupMembersSet::iterator itr = pSpell->p_caster->GetGroup()->GetSubGroup( x )->GetGroupMembersBegin(); itr != pSpell->p_caster->GetGroup()->GetSubGroup( x )->GetGroupMembersEnd(); itr++)
				{
					if( count == 3 )
						break;

					if( (*itr) && (*itr)->m_loggedInPlayer && (*itr)->m_loggedInPlayer->GetPowerType() == POWER_TYPE_MANA && count != 3)
					{
						if(pSpell->p_caster->isInRange((*itr)->m_loggedInPlayer, pSpell->GetRadius(i)))
						{
							SpellEntry *spellInfo = dbcSpell.LookupEntry( 54172 );
							Spell* sp(new Spell( pSpell->p_caster, spellInfo, true, NULLAURA ));
							sp->forced_basepoints[0] = amt;
							SpellCastTargets tgt;
							tgt.m_unitTarget = (*itr)->m_loggedInPlayer->GetGUID();
							sp->prepare(&tgt);
							count++;
						}
					}
				}
			}
			pSpell->p_caster->GetGroup()->Unlock();
		}
		else
		{
			SpellEntry *spellInfo = dbcSpell.LookupEntry( 54172 );
			Spell* sp(new Spell( pSpell->p_caster, spellInfo, true, NULLAURA ));
			sp->forced_basepoints[0] = amt;
			SpellCastTargets tgt;
			tgt.m_unitTarget = pSpell->p_caster->GetGUID();
			sp->prepare(&tgt);
		}
	}
	return true;
}

bool TeirBonusDivineStormReset(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL )
		pSpell->p_caster->ClearCooldownForSpell(53385);
	return true;
}

bool Illumination(uint32 i, Spell* pSpell)
{
	if( pSpell->p_caster != NULL && pSpell->ProcedOnSpell != NULL )
	{
		SpellEntry* sp = dbcSpell.LookupEntry(pSpell->ProcedOnSpell->Id);
		if(sp != NULL)
		{
			uint32 cost = 0;
			if(sp->ManaCostPercentage)
			{
				uint32 maxmana = pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXPOWER1);
				cost = maxmana*(float(sp->ManaCostPercentage)/100);
			}
			else if(sp->manaCostPerlevel)
			{
				cost = sp->manaCostPerlevel*pSpell->p_caster->getLevel();
			}
			else if(sp->manaCost)
			{
				cost = sp->manaCost;
			}

			if(cost > 0)
			{
				uint32 amount = ((cost*pSpell->p_caster->m_Illumination_amount)/100);
				SpellEntry* newspell = dbcSpell.LookupEntry(20272);
				Spell* nsp = new Spell(pSpell->p_caster, newspell, true, NULLAURA);
				SpellCastTargets tgts(pSpell->p_caster->GetGUID());
				nsp->forced_basepoints[0] = amount;
				nsp->prepare(&tgts);
			}
		}
	}
	return true;
}

bool JudgementsoftheWise(uint32 i, Spell* pSpell)
{
	SpellEntry* Replinishment = dbcSpell.LookupEntryForced( 57669 );
	if( pSpell->p_caster != NULL && Replinishment != NULL )
	{
		//cast Replenishment
		if( pSpell->p_caster->GetGroup() )
		{
			uint32 TargetCount = 0;
			pSpell->p_caster->GetGroup()->Lock();
			for(uint32 x = 0; x < pSpell->p_caster->GetGroup()->GetSubGroupCount(); ++x)
			{
				if( TargetCount == 3 )
					break;

				for(GroupMembersSet::iterator itr = pSpell->p_caster->GetGroup()->GetSubGroup( x )->GetGroupMembersBegin(); itr != pSpell->p_caster->GetGroup()->GetSubGroup( x )->GetGroupMembersEnd(); itr++)
				{
					if(TargetCount == 3)
						break;

					if((*itr)->m_loggedInPlayer)
					{
						Player* p_target = (*itr)->m_loggedInPlayer;
						if( p_target->GetPowerType() != POWER_TYPE_MANA )
							continue;
						if( p_target->HasAura(57669) )
							continue; // Skip our target if they already have it.

						Spell* pSpell = new Spell(pSpell->p_caster, Replinishment, true, NULLAURA);
						SpellCastTargets tgt;
						tgt.m_unitTarget = p_target->GetGUID();
						pSpell->prepare(&tgt);
						TargetCount++;
					}
				}
			}
			pSpell->p_caster->GetGroup()->Unlock();
		}

		Spell* nSpell = new Spell(pSpell->p_caster, Replinishment, true, NULLAURA);
		SpellCastTargets tgt(pSpell->p_caster->GetGUID());
		nSpell->prepare(&tgt);
	}
	return true;
}

bool Judgement(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if(unitTarget != NULL || pSpell->p_caster != NULL)
	{
		uint32 spellid;
		if( pSpell->GetSpellProto()->Id == 20271 )
			spellid = 20185;
		else if( pSpell->GetSpellProto()->Id == 53407 )
			spellid = 20184;
		else
			spellid = 20186;

		if(pSpell->p_caster->JudgementSpell)
			pSpell->p_caster->CastSpell(unitTarget, pSpell->p_caster->JudgementSpell, true);

		SpellEntry *en = dbcSpell.LookupEntry(spellid);
		if(en != NULL)
		{
			Spell* sp(new Spell(pSpell->p_caster, en, true, NULLAURA));
			SpellCastTargets tgt;
			tgt.m_unitTarget = unitTarget->GetGUID();
			tgt.m_targetMask = TARGET_FLAG_UNIT;
			sp->prepare(&tgt);
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void Judgements(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		pSpell->damage = 1;
		if(pSpell->p_caster != NULL)
		{
			pSpell->static_damage = true;
			if( pSpell->p_caster->HasAura(34258) )
				pSpell->p_caster->CastSpell(pSpell->p_caster, 34260, true);

			if(pSpell->p_caster->HasAura(53696) || pSpell->p_caster->HasAura(53695))
				pSpell->p_caster->CastSpell(pSpell->GetUnitTarget(), 68055, true);

			// Damage Calculations:
			switch(pSpell->GetSpellProto()->Id)
			{
			case 20187: // Righteousness
				pSpell->damage += (1+uint32(0.2f*pSpell->p_caster->GetAP())+uint32(0.32f*pSpell->p_caster->GetDamageDoneMod(SCHOOL_HOLY)));
				break;
			case 20425: // Command
				{
					pSpell->damage += uint32(CalculateDamage(pSpell->u_caster, pSpell->GetUnitTarget(), MELEE, NULL) * 0.19f)+uint32(0.08f*pSpell->p_caster->GetAP())+uint32(0.13f*pSpell->p_caster->GetDamageDoneMod(SCHOOL_HOLY));
				}break;
			case 31804: // Seal of Vengeance
			case 53733: // Seal of Corruption
				{
					uint32 auraid = ((pSpell->GetSpellProto()->Id == 53733) ? 53742 : 31803);
					pSpell->p_caster->CastSpell(pSpell->GetUnitTarget(), auraid, true);
					pSpell->damage = uint32(uint32(0.22f*pSpell->p_caster->GetDamageDoneMod(SCHOOL_HOLY))+uint32(0.14f*pSpell->p_caster->GetAP()));
					if(pSpell->GetUnitTarget()->HasAura(auraid))
					{
						uint8 stacksize = 1;
						Aura* curse = pSpell->GetUnitTarget()->m_AuraInterface.FindActiveAura(auraid);
						if(curse != NULL)
							stacksize = curse->stackSize;
						pSpell->damage += (pSpell->damage*(0.1f * stacksize));
					}
				}break;
			case 54158: // Light/Wisdom/Justice
				pSpell->damage = uint32(0.16f*pSpell->p_caster->GetAP())+uint32(0.25f*pSpell->p_caster->GetDamageDoneMod(SCHOOL_HOLY));
				break;
			}

			if( pSpell->p_caster->HasAura(37186) )
				pSpell->damage += 33;
		}
	}
}

void ShieldOfRighteousness(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL )
		{
			Item* it = TO_ITEM(pSpell->p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND));
			if( it && it->GetProto() && it->GetProto()->InventoryType == INVTYPE_SHIELD )
			{
				pSpell->damage += float2int32(1.3f * it->GetProto()->Block);
			}
		}
	}
}

void EyeForAnEye(uint32 i, Spell* pSpell, uint32 effect)
{
	if(effect == SPELL_EFFECT_SCHOOL_DAMAGE)
	{
		if( pSpell->p_caster != NULL)
		{
			if(pSpell->damage > (pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2))
				pSpell->damage = (pSpell->p_caster->GetUInt32Value(UNIT_FIELD_MAXHEALTH)/2);
		}
	}
}

//void Judgements(uint32 i, Spell* pSpell, uint32 effect)
void Lacrimi::SetupPaladinSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(20473, HolyShock);
	RegisterDummySpell(20929, HolyShock);
	RegisterDummySpell(20930, HolyShock);
	RegisterDummySpell(27174, HolyShock);
	RegisterDummySpell(33072, HolyShock);
	RegisterDummySpell(48824, HolyShock);
	RegisterDummySpell(48825, HolyShock);

	RegisterDummySpell(31789, RighteousDefense);

	RegisterDummySpell(53385, DivineStorm);

	RegisterDummySpell(70769, TeirBonusDivineStormReset);

	RegisterDummySpell(18350, Illumination);

	RegisterDummySpell(54180, JudgementsoftheWise);

	RegisterSpellScriptEffect(20271, Judgement);
	RegisterSpellScriptEffect(53407, Judgement);
	RegisterSpellScriptEffect(53408, Judgement);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(20187, Judgements);
	RegisterSpellEffectModifier(20425, Judgements);
	RegisterSpellEffectModifier(31804, Judgements);
	RegisterSpellEffectModifier(53733, Judgements);
	RegisterSpellEffectModifier(54158, Judgements);

	RegisterSpellEffectModifier(53600, ShieldOfRighteousness);
	RegisterSpellEffectModifier(61411, ShieldOfRighteousness);

	RegisterSpellEffectModifier(9799, EyeForAnEye);
	RegisterSpellEffectModifier(25988, EyeForAnEye);
}
