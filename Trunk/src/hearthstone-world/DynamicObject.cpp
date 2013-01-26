/***
 * Demonstrike Core
 */

#include "StdAfx.h"

DynamicObject::DynamicObject(uint32 high, uint32 low)
{
	m_objectTypeId = TYPEID_DYNAMICOBJECT;
	m_valuesCount = DYNAMICOBJECT_END;
	m_uint32Values = _fields;
	memset(m_uint32Values, 0,(DYNAMICOBJECT_END)*sizeof(uint32));
	m_updateMask.SetCount(DYNAMICOBJECT_END);
	m_uint32Values[OBJECT_FIELD_TYPE] = TYPE_DYNAMICOBJECT|TYPE_OBJECT;
	m_uint32Values[OBJECT_FIELD_GUID] = low;
	m_uint32Values[OBJECT_FIELD_GUID+1] = high;
	m_wowGuid.Init(GetGUID());
	m_floatValues[OBJECT_FIELD_SCALE_X] = 1;
	m_parentSpell = NULLSPELL;
	m_aliveDuration = 0;
	u_caster = NULL;
	m_spellProto = NULL;
	p_caster = NULL;
}

DynamicObject::~DynamicObject()
{

}

void DynamicObject::Init()
{
	Object::Init();
}

void DynamicObject::Destruct()
{
	if(u_caster != NULL && u_caster->dynObj == this)
		u_caster->dynObj = NULL;

	m_parentSpell = NULLSPELL;
	m_aliveDuration = 0;
	u_caster = NULLUNIT;
	m_spellProto = 0;
	p_caster = NULLPLR;
	Object::Destruct();
}

void DynamicObject::Create(Object* caster, Spell* pSpell, float x, float y, float z, uint32 duration, float radius)
{
	Object::_Create(caster->GetMapId(), x, y, z, 0);
	if(pSpell->g_caster)
		m_parentSpell = pSpell;

	if(pSpell->p_caster == NULL)
	{
		// try to find player caster here
		if(caster->IsPlayer())
			p_caster = TO_PLAYER(caster);
	}
	else
		p_caster = pSpell->p_caster;

	m_spellProto = pSpell->m_spellInfo;
	SetUInt64Value(DYNAMICOBJECT_CASTER, caster->GetGUID());

	m_uint32Values[OBJECT_FIELD_ENTRY] = m_spellProto->Id;
	m_uint32Values[DYNAMICOBJECT_BYTES] = 0x01eeeeee;
	m_uint32Values[DYNAMICOBJECT_SPELLID] = m_spellProto->Id;

	m_floatValues[DYNAMICOBJECT_RADIUS] = radius;
	m_position.x = x; //m_floatValues[DYNAMICOBJECT_POS_X]  = x;
	m_position.y = y; //m_floatValues[DYNAMICOBJECT_POS_Y]  = y;
	m_position.z = z; //m_floatValues[DYNAMICOBJECT_POS_Z]  = z;
	m_uint32Values[DYNAMICOBJECT_CASTTIME] = (uint32)UNIXTIME;

	m_aliveDuration = duration;
	u_caster = caster->IsUnit() ? TO_UNIT(caster) : NULL;
	m_faction = caster->m_faction;
	m_factionDBC = caster->m_factionDBC;
	SetPhaseMask(caster->GetPhaseMask());

	if(pSpell->g_caster)
		PushToWorld(pSpell->g_caster->GetMapMgr());
	else
		PushToWorld(caster->GetMapMgr());

	if(caster->dynObj != NULL && caster->dynObj != this)
		caster->dynObj->Remove();
	caster->dynObj = this;

	UpdateTargets(0);
}

void DynamicObject::AddInRangeObject( Object* pObj )
{
	Object::AddInRangeObject(pObj);
}

void DynamicObject::OnRemoveInRangeObject( Object* pObj )
{
	if( pObj->IsUnit() )
		targets.erase( pObj->GetGUID() );

	Object::OnRemoveInRangeObject( pObj );
}

void DynamicObject::UpdateTargets(uint32 p_time)
{
	if(m_aliveDuration == 0)
		return;

	if((uint32)m_aliveDuration < p_time)
		m_aliveDuration = 0;
	else
	{
		Aura* pAura;
		Unit* target;

		float radius = m_floatValues[DYNAMICOBJECT_RADIUS] * m_floatValues[DYNAMICOBJECT_RADIUS];

		// Looking for targets in the Object set
		for(std::unordered_set< Object* >::iterator itr = m_objectsInRange.begin(); itr != m_objectsInRange.end(); ++itr)
		{
			Object* o = *itr;

			if(!o->IsUnit() || !TO_UNIT(o)->isAlive())
				continue;

			target = TO_UNIT(o);

			if(!isAttackable(u_caster, target, !(m_spellProto->c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)))
				continue;

			// skip units already hit, their range will be tested later
			if(targets.find(target->GetGUID()) != targets.end())
				continue;

			if(GetDistanceSq(target) <= radius)
			{
				pAura = new Aura(m_spellProto, m_aliveDuration, u_caster, target);
				for(uint32 i = 0; i < 3; ++i)
				{
					if(m_spellProto->Effect[i] == SPELL_EFFECT_PERSISTENT_AREA_AURA)
					{
						pAura->AddMod(m_spellProto->EffectApplyAuraName[i],
							m_spellProto->EffectBasePoints[i]+1, m_spellProto->EffectMiscValue[i], i);
					}
				}

				target->AddAura(pAura);
				if(p_caster)
				{
					p_caster->HandleProc(PROC_ON_CAST_SPECIFIC_SPELL | PROC_ON_CAST_SPELL, NULL, target, m_spellProto);
					p_caster->m_procCounter = 0;
				}

				// add to target list
				targets.insert(target->GetGUID());
			}
		}

		// loop the targets, check the range of all of them
		DynamicObjectList::iterator jtr  = targets.begin();
		DynamicObjectList::iterator jtr2;
		DynamicObjectList::iterator jend = targets.end();

		while(jtr != jend)
		{
			target = GetMapMgr() ? GetMapMgr()->GetUnit(*jtr) : NULL;
			jtr2 = jtr;
			++jtr;

			if((target != NULL) && (GetDistanceSq(target) > radius))
			{
				target->RemoveAura(m_spellProto->Id);
				targets.erase(jtr2);
			}
		}

		m_aliveDuration -= p_time;
	}

	if(m_aliveDuration == 0)
		Remove();
}

void DynamicObject::Remove()
{
	if(!IsInWorld())
	{
		Destruct();
		return;
	}

	// remove aura from all targets
	Unit* target;
	for(std::set< uint64 >::iterator itr = targets.begin(); itr != targets.end(); ++itr)
	{

		uint64 TargetGUID = *itr;

		target = m_mapMgr->GetUnit(TargetGUID);

		if(target != NULL)
			target->RemoveAura(m_spellProto->Id);
	}

	WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
	data << GetGUID();
	SendMessageToSet(&data, false);

	if(IsInWorld())
		RemoveFromWorld(true);

	if(u_caster != NULL && m_spellProto->IsChannelSpell())
	{
		u_caster->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, 0);
		u_caster->SetUInt32Value(UNIT_CHANNEL_SPELL, 0);
	}

	Destruct();
}
