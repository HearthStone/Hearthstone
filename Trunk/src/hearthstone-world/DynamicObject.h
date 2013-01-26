/***
 * Demonstrike Core
 */

#pragma once

struct SpellEntry;

typedef std::set<uint64>  DynamicObjectList;

class SERVER_DECL DynamicObject : public Object
{
public:
	DynamicObject( uint32 high, uint32 low );
	~DynamicObject( );
	virtual void Init();
	virtual void Destruct();

	void Create(Object* caster, Spell* pSpell, float x, float y, float z, uint32 duration, float radius);
	void UpdateTargets(uint32 p_time);

	void AddInRangeObject(Object* pObj);
	void OnRemoveInRangeObject(Object* pObj);
	void Remove();

protected:
	SpellEntry * m_spellProto;
	Unit* u_caster;
	Player* p_caster;
	Spell* m_parentSpell;
	DynamicObjectList targets;

	int32 m_aliveDuration;
	uint32 _fields[DYNAMICOBJECT_END];
};
