
#include "LacrimiStdAfx.h"

////////////////////////
//// Spell Scripts	////
////////////////////////
bool SoulLink(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->u_caster != NULL && unitTarget != NULL && unitTarget->isAlive())
	{
		uint32 pet_dmg = pSpell->forced_basepoints[0]*20/100;
		unitTarget->ModUnsigned32Value(UNIT_FIELD_HEALTH, pet_dmg);
		pSpell->TotalDamage += unitTarget->DealDamage(pSpell->u_caster, pet_dmg, 0, 0, 25228);
	}
	return true;
}

bool DemonicCircle(uint32 i, Spell* pSpell)
{
	if( pSpell->u_caster != NULL )
	{
		GameObject* DemonicCircle = pSpell->u_caster->GetMapMgr()->GetGameObject( pSpell->u_caster->m_ObjectSlots[0] );
		if( DemonicCircle != NULL )
			pSpell->u_caster->Teleport( DemonicCircle->GetPositionX(), DemonicCircle->GetPositionY(), DemonicCircle->GetPositionZ(), DemonicCircle->GetOrientation());
	}
	return true;
}

bool LifeTap(uint32 i, Spell* pSpell)
{
	Player* playerTarget = pSpell->GetPlayerTarget();
	if( pSpell->u_caster != NULL && playerTarget != NULL )
	{
		uint32 energy = (pSpell->GetSpellProto()->EffectBasePoints[i] + 1)+(pSpell->p_caster->GetDamageDoneMod(SCHOOL_SHADOW)/2);
		if(playerTarget->m_lifetapbonus)
			energy *= (100 + playerTarget->m_lifetapbonus) / 100;	// Apply improved life tap
		pSpell->p_caster->Energize(playerTarget, pSpell->pSpellId ? pSpell->pSpellId : pSpell->GetSpellProto()->Id, energy, POWER_TYPE_MANA);
	}
	return true;
}

bool DemonicEmpowerment(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->u_caster != NULL && unitTarget != NULL )
	{
		unitTarget->m_AuraInterface.AttemptDispel(pSpell->u_caster, MECHANIC_ROOTED, true);
		unitTarget->m_AuraInterface.AttemptDispel(pSpell->u_caster, MECHANIC_STUNNED, true);
		unitTarget->m_AuraInterface.AttemptDispel(pSpell->u_caster, MECHANIC_ENSNARED, true);
	}
	return true;
}

bool EverlastingAffliction(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL )
	{
		Aura* pAura = unitTarget->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_CORRUPTION);
		if(pAura != NULL)
		{
			pAura->SetDuration(pAura->GetBaseDuration());
			pAura->SetTimeLeft(pAura->GetBaseDuration());
		}
	}
	return true;
}

bool PainAndSuffering(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( unitTarget != NULL )
	{
		Aura* aur = unitTarget->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_SHADOW_WORD__PAIN);
		if( aur != NULL) // Crow: Hmmm....
			aur->SetTimeLeft(aur->GetBaseDuration());
	}
	return true;
}

bool Healthstones(uint32 i, Spell* pSpell)
{
	switch(pSpell->GetSpellProto()->Id)
	{
		// Warlock Healthstones, just how much health does a lock need?
	case 6201:		// Minor Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))
			{
				pSpell->CreateItem(19004);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))
			{
				pSpell->CreateItem(19005);
				break;
			}
			pSpell->CreateItem(5512);
		}break;
	case 6202:		// Lesser Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(19007);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(19006);
				break;
			}
			pSpell->CreateItem(5511);
		}break;
	case 5699:		// Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(19009);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(19008);
				break;
			}
			pSpell->CreateItem(5509);
		}break;
	case 11729:		// Greater Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(19011);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(19010);
				break;
			}
			pSpell->CreateItem(5510);
		}break;
	case 11730:		// Major Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(19013);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(19012);
				break;
			}
			pSpell->CreateItem(9421);
		}break;
	case 27230:		// Master Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(22105);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(22104);
				break;
			}
			pSpell->CreateItem(22103);
		}break;
	case 47871:		// Demonic Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(36891);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(36890);
				break;
			}
			pSpell->CreateItem(36889);
		}break;
	case 47878:		// Fel Healthstone
		{
			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18693))	// Improved Healthstone (2)
			{
				pSpell->CreateItem(36894);
				break;
			}

			if (pSpell->p_caster != NULL && pSpell->p_caster->HasSpell(18692))	// Improved Healthstone (1)
			{
				pSpell->CreateItem(36893);
				break;
			}
			pSpell->CreateItem(36892);
		}break;
	}
	return true;
}

bool MasterDemonologist(uint32 i, Spell* pSpell)
{
	Unit* unitTarget = pSpell->GetUnitTarget();
	if( pSpell->p_caster != NULL && unitTarget != NULL)
	{
		switch(pSpell->GetSpellProto()->Id)
		{
		//warlock - Master Demonologist
		case 23784:
			{
				uint32 casted_spell_id = 0 ;
				uint32 inc_resist_by_level = 0 ;
				uint32 inc_resist_by_level_spell = 0 ;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //in case it is imp
					casted_spell_id = 23759 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					casted_spell_id = 23760 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					casted_spell_id = 23761 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
				{
					casted_spell_id = 0 ;
					inc_resist_by_level_spell = 23762 ;
					inc_resist_by_level = 20 ;
				}
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
				{
					casted_spell_id = 35702 ;
					inc_resist_by_level_spell = 23762 ;
					inc_resist_by_level = 10 ;
				}
				if( casted_spell_id )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
				if( inc_resist_by_level_spell )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = pSpell->p_caster->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = unitTarget->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
			}break;
		case 23830:
			{
				uint32 casted_spell_id = 0 ;
				uint32 inc_resist_by_level = 0 ;
				uint32 inc_resist_by_level_spell = 0 ;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //in case it is imp
					casted_spell_id = 23826 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					casted_spell_id = 23841 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					casted_spell_id = 23833 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
				{
					casted_spell_id = 1 ;
					inc_resist_by_level_spell = 23837 ;
					inc_resist_by_level = 40 ;
				}
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
				{
					casted_spell_id = 35703 ;
					inc_resist_by_level_spell = 23837 ;
					inc_resist_by_level = 20 ;
				}
				if( casted_spell_id )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = new Spell(  unitTarget, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
				if( inc_resist_by_level_spell )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = pSpell->p_caster->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = unitTarget->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
			}break;
		case 23831:
			{
				uint32 casted_spell_id = 0 ;
				uint32 inc_resist_by_level = 0 ;
				uint32 inc_resist_by_level_spell = 0 ;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //in case it is imp
					casted_spell_id = 23827 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					casted_spell_id = 23842 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					casted_spell_id = 23834 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
				{
					casted_spell_id = 0 ;
					inc_resist_by_level_spell = 23838 ;
					inc_resist_by_level = 60 ;
				}
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
				{
					casted_spell_id = 35704 ;
					inc_resist_by_level_spell = 23838 ;
					inc_resist_by_level = 30 ;
				}
				if( casted_spell_id )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
				if( inc_resist_by_level_spell )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = pSpell->p_caster->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = NULL;
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = unitTarget->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
			}break;
		case 23832:
			{
				uint32 casted_spell_id = 0 ;
				uint32 inc_resist_by_level = 0 ;
				uint32 inc_resist_by_level_spell = 0 ;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //in case it is imp
					casted_spell_id = 23828 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					casted_spell_id = 23843 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					casted_spell_id = 23835 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
				{
					casted_spell_id = 0 ;
					inc_resist_by_level_spell = 23839 ;
					inc_resist_by_level = 80 ;
				}
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
				{
					casted_spell_id = 35705 ;
					inc_resist_by_level_spell = 23839 ;
					inc_resist_by_level = 40 ;
				}
				if( casted_spell_id )
				{
					//for self
					Spell* sp = new Spell( pSpell->p_caster, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = NULL;
					sp = new Spell(  unitTarget, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
				if( inc_resist_by_level_spell )
				{
					//for self
					Spell* sp=new Spell( pSpell->p_caster, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = pSpell->p_caster->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					sp = NULL;
					sp = new Spell( unitTarget, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = unitTarget->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					sp->prepare( &tgt1 );
				}
			}break;
		case 35708:
			{
				uint32 casted_spell_id = 0 ;
				uint32 inc_resist_by_level = 0 ;
				uint32 inc_resist_by_level_spell = 0 ;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //in case it is imp
					casted_spell_id = 23829 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					casted_spell_id = 23844 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					casted_spell_id = 23836 ;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
				{
					casted_spell_id = 0 ;
					inc_resist_by_level_spell = 23840 ;
					inc_resist_by_level = 100 ;
				}
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
				{
					casted_spell_id = 35706 ;
				}
				if( casted_spell_id )
				{
					//for self
					Spell* sp=new Spell( pSpell->p_caster, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					Spell* Petsp=new Spell(  unitTarget, dbcSpell.LookupEntry( casted_spell_id ), true, NULLAURA );
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					Petsp->prepare( &tgt1 );
				}
				if( inc_resist_by_level_spell )
				{
					//for self
					Spell* sp=new Spell( pSpell->p_caster, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					sp->forced_basepoints[0] = pSpell->p_caster->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt( pSpell->p_caster->GetGUID() );
					sp->prepare( &tgt );
					//for pet
					Spell* Petsp=new Spell( unitTarget, dbcSpell.LookupEntry( inc_resist_by_level_spell ), true, NULLAURA );
					Petsp->forced_basepoints[0] = unitTarget->GetUInt32Value( UNIT_FIELD_LEVEL ) * inc_resist_by_level / 100;
					SpellCastTargets tgt1( unitTarget->GetGUID() );
					Petsp->prepare( &tgt1 );
				}
			}break;
		case 47193: // [Warlock] Demonic Empowerment
			{
				uint32 nspellid = 0;
				if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 416 ) //Imp
					nspellid = 54444;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1860 ) //VoidWalker
					nspellid = 54443;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 1863 ) //Succubus
					nspellid = 54435;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 417 ) //Felhunter
					nspellid = 54509;
				else if ( unitTarget->GetUInt32Value( OBJECT_FIELD_ENTRY ) == 17252 ) //Felguard
					nspellid = 54508;

				unitTarget->CastSpell(unitTarget, nspellid, true);
			}break;
		}
	}
	return true;
}

////////////////////////
//// Damage Scripts	////
////////////////////////
void Incinerate(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		if( pSpell->GetUnitTarget()->m_AuraInterface.GetAuraSpellIDWithNameHash( SPELL_HASH_IMMOLATE ) )
		{
			// Random extra damage
			uint32 extra_dmg = 89 + (pSpell->GetSpellProto()->RankNumber * 11) + RandomUInt(pSpell->GetSpellProto()->RankNumber * 11);
			pSpell->damage += extra_dmg;
		}
	}
}

void Shadowflame(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		uint32 spellid = ( pSpell->GetSpellProto()->RankNumber == 1 ) ? 47960 : 61291;
		SpellEntry* SpellInfo = dbcSpell.LookupEntry( spellid );
		Spell* sp (new Spell( pSpell->u_caster, SpellInfo, true, NULLAURA ));
		SpellCastTargets tgt;
		tgt.m_unitTarget = pSpell->GetUnitTarget()->GetGUID();
		sp->prepare( &tgt );
	}
}

void Conflagrate(uint32 i, Spell* pSpell, uint32 effect)
{
	if( effect == SPELL_EFFECT_SCHOOL_DAMAGE )
	{
		Aura* pAura = pSpell->GetUnitTarget()->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_IMMOLATE);
		if(pAura != NULL)
		{
			if( pSpell->u_caster->GetDummyAura(SPELL_HASH_FIRE_AND_BRIMSTONE) && pAura->GetTimeLeft() <= 5 )
				pSpell->AdditionalCritChance += pSpell->u_caster->GetDummyAura(SPELL_HASH_FIRE_AND_BRIMSTONE)->RankNumber * 5;

			pSpell->GetUnitTarget()->m_AuraInterface.RemoveAura(pAura);
		}
		else
		{
			pAura = pSpell->GetUnitTarget()->m_AuraInterface.FindNegativeAuraByNameHash(SPELL_HASH_SHADOWFLAME);
			if(pAura != NULL)
			{
				if( pSpell->u_caster->GetDummyAura(SPELL_HASH_FIRE_AND_BRIMSTONE) && pAura->GetTimeLeft() <= 5 )
					pSpell->AdditionalCritChance += pSpell->u_caster->GetDummyAura(SPELL_HASH_FIRE_AND_BRIMSTONE)->RankNumber * 5;

				pSpell->GetUnitTarget()->m_AuraInterface.RemoveAura(pAura);
			}
		}
	}
}

void Lacrimi::SetupWarlockSpells()
{
	////////////////////////
	//// Spell Scripts	////
	////////////////////////
	RegisterDummySpell(25228, SoulLink);

	RegisterDummySpell(48020, DemonicCircle);

	RegisterDummySpell(1454, LifeTap);
	RegisterDummySpell(1455, LifeTap);
	RegisterDummySpell(1456, LifeTap);
	RegisterDummySpell(11687, LifeTap);
	RegisterDummySpell(11688, LifeTap);
	RegisterDummySpell(11689, LifeTap);
	RegisterDummySpell(27222, LifeTap);
	RegisterDummySpell(57946, LifeTap);

	RegisterSpellScriptEffect(54436, DemonicEmpowerment);

	RegisterSpellScriptEffect(47422, EverlastingAffliction);

	RegisterSpellScriptEffect(6201, Healthstones);
	RegisterSpellScriptEffect(6202, Healthstones);
	RegisterSpellScriptEffect(5699, Healthstones);
	RegisterSpellScriptEffect(11730, Healthstones);
	RegisterSpellScriptEffect(11730, Healthstones);
	RegisterSpellScriptEffect(27230, Healthstones);
	RegisterSpellScriptEffect(47871, Healthstones);
	RegisterSpellScriptEffect(47878, Healthstones);

	RegisterSpellScriptEffect(23784, MasterDemonologist);
	RegisterSpellScriptEffect(23830, MasterDemonologist);
	RegisterSpellScriptEffect(23831, MasterDemonologist);
	RegisterSpellScriptEffect(23832, MasterDemonologist);
	RegisterSpellScriptEffect(35708, MasterDemonologist);
	RegisterSpellScriptEffect(47193, MasterDemonologist);

	////////////////////////
	//// Damage Scripts	////
	////////////////////////
	RegisterSpellEffectModifier(29722, Incinerate);
	RegisterSpellEffectModifier(32231, Incinerate);
	RegisterSpellEffectModifier(47837, Incinerate);
	RegisterSpellEffectModifier(47838, Incinerate);

	RegisterSpellEffectModifier(47897, Shadowflame);
	RegisterSpellEffectModifier(61290, Shadowflame);

	RegisterSpellEffectModifier(17962, Conflagrate);
}
