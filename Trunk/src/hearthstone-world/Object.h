/***
 * Demonstrike Core
 */

#pragma once

class Unit;
class Spell;
class Aura;
class DynamicObject;

enum HIGHGUID_TYPE
{
	HIGHGUID_TYPE_VEHICLE			= 0xF1500000,
	HIGHGUID_TYPE_CREATURE			= 0xF1300000,
	HIGHGUID_TYPE_PET				= 0xF1400000,
	HIGHGUID_TYPE_GAMEOBJECT		= 0xF1100000,
	HIGHGUID_TYPE_ITEM				= 0x40000000,
	HIGHGUID_TYPE_CONTAINER			= 0x50000000,			// confirm this pl0x
	HIGHGUID_TYPE_PLAYER			= 0x00000000,
	HIGHGUID_TYPE_DYNAMICOBJECT		= 0x60000000,
	HIGHGUID_TYPE_TRANSPORTER		= 0x1FC00000,
	HIGHGUID_TYPE_WAYPOINT			= 0x10000000,
	HIGHGUID_TYPE_CORPSE			= 0x30000000,
//===============================================
	HIGHGUID_TYPE_MASK				= 0xFFF00000,
	LOWGUID_ENTRY_MASK				= 0x00FFFFFF,
};

// Externals
#define HIGHGUID_TYPE_UNIT HIGHGUID_TYPE_CREATURE

#define GET_TYPE_FROM_GUID(x) (GUID_HIPART((x)) & HIGHGUID_TYPE_MASK)
#define GET_LOWGUID_PART(x) (GUID_LOPART((x)) & LOWGUID_ENTRY_MASK)

// TODO: fix that type mess

enum TYPE
{
	TYPE_OBJECT			= 1,
	TYPE_ITEM			= 2,
	TYPE_CONTAINER		= 4,
	TYPE_UNIT			= 8,
	TYPE_PLAYER			= 16,
	TYPE_GAMEOBJECT		= 32,
	TYPE_DYNAMICOBJECT	= 64,
	TYPE_CORPSE			= 128,
	TYPE_AIGROUP		= 256,
	TYPE_AREATRIGGER	= 512,
};

enum TYPEID
{
	TYPEID_OBJECT			= 0,
	TYPEID_ITEM				= 1,
	TYPEID_CONTAINER		= 2,
	TYPEID_UNIT				= 3,
	TYPEID_PLAYER			= 4,
	TYPEID_GAMEOBJECT		= 5,
	TYPEID_DYNAMICOBJECT	= 6,
	TYPEID_CORPSE			= 7,
	TYPEID_AIGROUP			= 8,
	TYPEID_AREATRIGGER		= 9,
	TYPEID_UNUSED			= 10,//used to signal invalid reference (object dealocated but someone is still using it)
};

enum OBJECT_UPDATE_TYPE {
	UPDATETYPE_VALUES = 0,
	//  8 bytes - GUID
	//  Goto Update Block
	UPDATETYPE_MOVEMENT = 1,
	//  8 bytes - GUID
	//  Goto Position Update
	UPDATETYPE_CREATE_OBJECT = 2,
	//  8 bytes - GUID
	//  1 byte - Object Type (*)
	//  Goto Position Update
	//  Goto Update Block
	UPDATETYPE_CREATE_YOURSELF = 3, // looks like 3 & 4 do the same thing
	//  4 bytes - Count
	//  Loop Count Times:
	//  8 bytes - GUID
	UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4, // <- this is correct, not sure about 3
	//  4 bytes - Count
	//  Loop Count Times:
	//  8 bytes - GUID

	UPDATETYPE_NEAR_OBJECTS = 5,
	//  4 bytes - Count
	//  Loop Count Times:
	//  8 bytes - GUID
};

typedef struct
{
	uint32 school_type;
	int32 full_damage;
	uint32 resisted_damage;
} dealdamage;

class WorldPacket;
class ByteBuffer;
class WorldSession;
class Player;
class MapCell;
class MapMgr;

//====================================================================
//  Object
//  Base object for every item, unit, player, corpse, container, etc
//====================================================================
class SERVER_DECL Object : public EventableObject
{
public:
	typedef unordered_set< Object* > InRangeSet;
	typedef std::map<string, void*> ExtensionSet;

	virtual ~Object ( );
	virtual void Init();
	virtual void Destruct();

	virtual void Update ( uint32 time ) { }
	//! True if object exists in world

	float GetCHeightForPosition(bool checkwater = false, float x = 0.0f, float y = 0.0f, float z = 0.0f);

	HEARTHSTONE_INLINE bool IsInWorld() { return m_mapMgr != NULL; }
	virtual void AddToWorld();
	virtual void AddToWorld(MapMgr* pMapMgr);
	void PushToWorld(MapMgr* );
	virtual void OnPushToWorld() { }
	virtual void OnPrePushToWorld() { }
	virtual void RemoveFromWorld(bool free_guid);

	HEARTHSTONE_INLINE void ObjLock() { m_objlock.Acquire(); }
	HEARTHSTONE_INLINE void ObjUnlock() { m_objlock.Release(); }

	// guid always comes first
	HEARTHSTONE_INLINE const uint64& GetGUID() const { return *((uint64*)m_uint32Values); }
	void SetGUID(uint64 GUID) { SetUInt64Value(OBJECT_FIELD_GUID, GUID);  }
	void SetLowGUID(uint32 val) { m_uint32Values[0] = val; }
	void SetHighGUID(uint32 val) { m_uint32Values[1] = val; }

	HEARTHSTONE_INLINE const WoWGuid& GetNewGUID() const { return m_wowGuid; }
	HEARTHSTONE_INLINE uint32 GetEntry(){return m_uint32Values[OBJECT_FIELD_ENTRY];}
	void SetEntry(uint32 value) { SetUInt32Value(OBJECT_FIELD_ENTRY, value); }

//	float GetScale() { return m_floatValues[ OBJECT_FIELD_SCALE_X ]; }
	void SetScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); };

	HEARTHSTONE_INLINE uint32 GetEntryFromGUID() const	{ return uint32( (*(uint64*)m_uint32Values >> 24) & 0xFFFFFFFF ); }
	HEARTHSTONE_INLINE uint32 GetTypeFromGUID() const { return (m_uint32Values[1] & HIGHGUID_TYPE_MASK); }
	HEARTHSTONE_INLINE uint32 GetUIdFromGUID() const { return (m_uint32Values[0] & LOWGUID_ENTRY_MASK); }
	HEARTHSTONE_INLINE uint32 GetHighGUID() const { return (m_uint32Values[1]); } /// Sooooooooooooooooooooooooooooooooooo high
	HEARTHSTONE_INLINE uint32 GetLowGUID() const { return (m_uint32Values[0]); }

	bool m_isVehicle;
	bool m_isSummon;
	bool m_isTotem;
	bool m_isPet;

	// type
	HEARTHSTONE_INLINE const uint8& GetTypeId() const { return m_objectTypeId; }
	HEARTHSTONE_INLINE bool IsUnit()	{ return ( m_objectTypeId == TYPEID_UNIT || m_objectTypeId == TYPEID_PLAYER ); }
	HEARTHSTONE_INLINE bool IsPlayer() { return m_objectTypeId == TYPEID_PLAYER; }
	HEARTHSTONE_INLINE bool IsCreature() { return m_objectTypeId == TYPEID_UNIT; }
	HEARTHSTONE_INLINE bool IsDynamicObj() { return m_objectTypeId == TYPE_DYNAMICOBJECT; }
	HEARTHSTONE_INLINE bool IsGameObject() { return m_objectTypeId == TYPEID_GAMEOBJECT; }
	HEARTHSTONE_INLINE bool IsContainer()	{ return m_objectTypeId == TYPEID_CONTAINER; }
	HEARTHSTONE_INLINE bool IsItem()	{ return m_objectTypeId == TYPEID_ITEM; }
	HEARTHSTONE_INLINE bool IsVehicle() { return m_isVehicle; }
	HEARTHSTONE_INLINE bool IsSummon() { return m_isSummon; }
	HEARTHSTONE_INLINE bool IsTotem() { return m_isTotem; }
	HEARTHSTONE_INLINE bool IsPet() { return m_isPet; }

	//! This includes any nested objects we have, inventory for example.
	virtual uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
	uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer *buf, Player* target );
	uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer * buf, UpdateMask * mask );
	uint32 __fastcall BuildOutOfRangeUpdateBlock( ByteBuffer *buf );

	WorldPacket* BuildFieldUpdatePacket(uint32 index,uint32 value);
	void BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value);
	void BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value);

	int32 DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

	virtual void DestroyForPlayer( Player* target, bool anim = false );

	WorldPacket * BuildTeleportAckMsg( const LocationVector & v);

	bool SetPosition( float newX, float newY, float newZ, float newOrientation, bool allowPorting = false );
	bool SetPosition( const LocationVector & v, bool allowPorting = false);
	void SetRotation( uint64 guid );

	void CastSpell(Object* Target, SpellEntry* Sp, bool triggered);
	void CastSpell(Object* Target, uint32 SpellID, bool triggered);
	void CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered);
	void CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered);

	HEARTHSTONE_INLINE const float& GetPositionX( ) const { return m_position.x; }
	HEARTHSTONE_INLINE const float& GetPositionY( ) const { return m_position.y; }
	HEARTHSTONE_INLINE const float& GetPositionZ( ) const { return m_position.z; }
	HEARTHSTONE_INLINE const float& GetOrientation( ) const { return m_position.o; }
	HEARTHSTONE_INLINE void SetOrientation( float &o ) { m_position.o = o; }

	HEARTHSTONE_INLINE const float& GetSpawnX( ) const { return m_spawnLocation.x; }
	HEARTHSTONE_INLINE const float& GetSpawnY( ) const { return m_spawnLocation.y; }
	HEARTHSTONE_INLINE const float& GetSpawnZ( ) const { return m_spawnLocation.z; }
	HEARTHSTONE_INLINE const float& GetSpawnO( ) const { return m_spawnLocation.o; }
	HEARTHSTONE_INLINE void SetSpawnX(float x) { m_spawnLocation.x = x; }
	HEARTHSTONE_INLINE void SetSpawnY(float y) { m_spawnLocation.y = y; }
	HEARTHSTONE_INLINE void SetSpawnZ(float z) { m_spawnLocation.z = z; }
	HEARTHSTONE_INLINE void SetSpawnO(float o) { m_spawnLocation.o = o; }

	bool canWalk();
	bool canSwim();
	bool canFly();

	HEARTHSTONE_INLINE const LocationVector & GetPosition() { return m_position; }
	HEARTHSTONE_INLINE LocationVector & GetPositionNC() { return m_position; }
	HEARTHSTONE_INLINE LocationVector * GetPositionV() { return &m_position; }
	HEARTHSTONE_INLINE void GetPosition(float &x, float &y, float &z) { x = m_position.x; y = m_position.y; z = m_position.z; }

	bool IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta);

	//Distance Calculation
	float CalcDistance(Object* Ob);
	float CalcDistance(float ObX, float ObY, float ObZ);
	float CalcDistance(Object* Oa, Object* Ob);
	float CalcDistance(Object* Oa, float ObX, float ObY, float ObZ);
	float CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ);

	//! Only for MapMgr use
	HEARTHSTONE_INLINE MapCell* GetMapCell() const { return m_mapCell; }
	//! Only for MapMgr use
	HEARTHSTONE_INLINE void SetMapCell(MapCell* cell) { m_mapCell = cell; }
	//! Only for MapMgr use
	HEARTHSTONE_INLINE MapMgr* GetMapMgr() const { return m_mapMgr; }

	HEARTHSTONE_INLINE void SetMapId(uint32 newMap) { m_mapId = newMap; }
	void SetZoneId(uint32 newZone);

	HEARTHSTONE_INLINE const uint32 GetMapId( ) const { return m_mapId; }
	HEARTHSTONE_INLINE const uint32& GetZoneId( ) const { return m_zoneId; }

	// Area id
	uint32 GetAreaID(MapMgr* mgr = NULL); // Finds the area id for the object.
	uint32 GetAreaID(float x, float y, float z = 0.0f, int32 mapid = -1, MapMgr* mgr = NULL);

	//! Get uint16 property
	HEARTHSTONE_INLINE const uint16& GetUInt16Value(uint16 index, uint8 offset) const { ASSERT( index < m_valuesCount ); ASSERT( offset < 2 ); return *(((uint16*)&m_uint32Values[index])+offset); }

	//! Get uint32 property
	HEARTHSTONE_INLINE const uint32& GetUInt32Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_uint32Values[ index ]; }

	//! Get uint64 property
	HEARTHSTONE_INLINE const uint64& GetUInt64Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return *((uint64*)&(m_uint32Values[ index ])); }

	//! Get float property
	HEARTHSTONE_INLINE const float& GetFloatValue( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_floatValues[ index ]; }

	void __fastcall ModFloatValue(const uint32 index, const float value );
	void ModSignedInt32Value(uint32 index, int32 value);
	void ModUnsigned32Value(uint32 index, int32 mod);
	uint32 GetModPUInt32Value(const uint32 index, const int32 value);

	//! Set uint32 property
	void SetByte(uint32 index, uint32 index1,uint8 value);

	HEARTHSTONE_INLINE uint8 GetByte(uint32 i,uint32 i1) { return ((uint8*)m_uint32Values)[i*4+i1]; }

	HEARTHSTONE_INLINE void SetNewGuid(uint32 Guid)
	{
		uint64 new_full_guid = Guid;
		if( m_objectTypeId == TYPEID_GAMEOBJECT )
			new_full_guid |= ((uint64)GetEntry() << 24) | ((uint64)HIGHGUID_TYPE_GAMEOBJECT << 32);

		if( m_objectTypeId == TYPEID_UNIT )
			new_full_guid |= ((uint64)GetEntry() << 24) | ((uint64)HIGHGUID_TYPE_CREATURE << 32);

		SetUInt64Value(OBJECT_FIELD_GUID, new_full_guid);
		m_wowGuid.Init(GetGUID());
	}

	void __fastcall SetUInt16Value(uint16 index, uint8 offset, uint16 value);

	void EventSetUInt32Value(uint32 index, uint32 value);
	void __fastcall SetUInt32Value( const uint32 index, const uint32 value );

	//! Set uint64 property
	void __fastcall SetUInt64Value( const uint32 index, const uint64 value );

	//! Set float property
	void __fastcall SetFloatValue( const uint32 index, const float value );

	void __fastcall SetFlag( const uint32 index, uint32 newFlag );

	void __fastcall RemoveFlag( const uint32 index, uint32 oldFlag );

	void __fastcall SetByteFlag( const uint32 index, const uint32 flag, uint8 newFlag);
	bool __fastcall HasByteFlag( const uint32 index, const uint32 flag, uint8 checkFlag);
	void __fastcall RemoveByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag);

	HEARTHSTONE_INLINE bool HasFlag( const uint32 index, uint32 flag ) const { return (m_uint32Values[ index ] & flag) != 0;	}

	////////////////////////////////////////
	void ClearUpdateMask( )
	{
		m_updateMask.Clear();
		m_objectUpdated = false;
	}

	void SetUpdateField(uint32 index);
	bool HasUpdateField(uint32 index) { return m_updateMask.GetBit(index); }

	//use it to check if a object is in range of another
	bool isInRange(Object* target, float range);

	// Use it to Check if a object is in front of another one
	bool isTargetInFront(Object* target);
	bool isTargetInBack(Object* target) { return !isTargetInFront(target); };

	bool isInFrontOfTarget(Object* target) { return target->isTargetInFront(this); };
	bool isInBackOfTarget(Object* target) { return target->isTargetInBack(this); };

	// Check to see if an object is in front of a target in a specified arc (in degrees)
	bool isInArc(Object* target , float degrees);

	/* Calculates the angle between two Positions */
	float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );
	float calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );

	/* converts to 360 > x > 0 */
	float getEasyAngle( float angle );

	HEARTHSTONE_INLINE float CalcDistance(LocationVector & comp)
	{
		return comp.Distance(m_position);
	}

	HEARTHSTONE_INLINE float GetDistanceSq(LocationVector & comp)
	{
		return comp.DistanceSq(m_position);
	}

	HEARTHSTONE_INLINE const float GetDistanceSq(Object* obj)
	{
		if(obj->GetMapId() != m_mapId)
			return 40000.0f; //enough for out of range
		return m_position.DistanceSq(obj->GetPosition());
	}

	HEARTHSTONE_INLINE const float GetDistanceSq(float x, float y, float z)
	{
		return m_position.DistanceSq(x, y, z);
	}

	HEARTHSTONE_INLINE const float GetDistance2dSq( Object* obj )
	{
		if( obj->GetMapId() != m_mapId )
			return 40000.0f; //enough for out of range
		return m_position.Distance2DSq( obj->m_position );
	}

	HEARTHSTONE_INLINE float GetDistance2dSq(float x, float y)
	{
		return m_position.Distance2DSq( x, y );
	}

	// In-range object management, not sure if we need it
	HEARTHSTONE_INLINE bool IsInRangeSet( Object* pObj )
	{
		return !( m_objectsInRange.find( pObj ) == m_objectsInRange.end() );
	}

	virtual void AddInRangeObject(Object* pObj)
	{
		if( pObj == NULL )
			return;

		m_objectsInRange.insert( pObj );

		if( pObj->IsUnit() )
			m_unitsInRange.insert( TO_UNIT(pObj) );
		if( pObj->IsPlayer() )
			m_inRangePlayers.insert( TO_PLAYER(pObj) );
	}

	HEARTHSTONE_INLINE void RemoveInRangeObject( Object* pObj )
	{
		if( pObj == NULL )
			return;

		OnRemoveInRangeObject( pObj );
		m_objectsInRange.erase( pObj );
	}

	HEARTHSTONE_INLINE bool HasInRangeObjects()
	{
		return ( m_objectsInRange.size() > 0 );
	}

	virtual void OnRemoveInRangeObject( Object* pObj )
	{
		if( pObj->IsUnit() )
			m_unitsInRange.erase( TO_UNIT(pObj) );
		if( pObj->IsPlayer() )
			m_inRangePlayers.erase( TO_PLAYER(pObj) );
	}

	virtual void ClearInRangeSet()
	{
		m_objectsInRange.clear();
		m_inRangePlayers.clear();
		m_oppFactsInRange.clear();
	}

	HEARTHSTONE_INLINE size_t GetInRangeCount() { return m_objectsInRange.size(); }
	HEARTHSTONE_INLINE size_t GetInRangeUnitsCount() { return m_unitsInRange.size();}
	HEARTHSTONE_INLINE size_t GetInRangePlayersCount() { return m_inRangePlayers.size();}
	HEARTHSTONE_INLINE unordered_set<Player*  > * GetInRangePlayerSet() { return &m_inRangePlayers; };

	InRangeSet::iterator GetInRangeSetBegin() { return m_objectsInRange.begin(); }
	InRangeSet::iterator GetInRangeSetEnd() { return m_objectsInRange.end(); }
	InRangeSet::iterator FindInRangeSet(Object * obj) { return m_objectsInRange.find(obj); }

	void RemoveInRangeObject(InRangeSet::iterator itr)
	{
		OnRemoveInRangeObject(*itr);
		m_objectsInRange.erase(itr);
	}

	HEARTHSTONE_INLINE bool RemoveIfInRange( Object* obj )
	{
		InRangeSet::iterator itr = m_objectsInRange.find(obj);
		if( obj->IsPlayer() )
			m_inRangePlayers.erase( TO_PLAYER(obj) );

		if( itr == m_objectsInRange.end() )
			return false;

		m_objectsInRange.erase( itr );
		return true;
	}

	HEARTHSTONE_INLINE void AddInRangePlayer( Object* obj )
	{
		m_inRangePlayers.insert( TO_PLAYER(obj) );
	}

	HEARTHSTONE_INLINE void RemoveInRangePlayer( Object* obj )
	{
		m_inRangePlayers.erase( TO_PLAYER(obj) );
	}

	bool IsInRangeOppFactSet(Object* pObj) { return (m_oppFactsInRange.count(pObj) > 0); }
	void UpdateOppFactionSet();

	HEARTHSTONE_INLINE unordered_set<Object* >::iterator GetInRangeOppFactsSetBegin() { return m_oppFactsInRange.begin(); }
	HEARTHSTONE_INLINE unordered_set<Object* >::iterator GetInRangeOppFactsSetEnd() { return m_oppFactsInRange.end(); }
	HEARTHSTONE_INLINE unordered_set<Player*  >::iterator GetInRangePlayerSetBegin() { return m_inRangePlayers.begin(); }
	HEARTHSTONE_INLINE unordered_set<Player*  >::iterator GetInRangePlayerSetEnd() { return m_inRangePlayers.end(); }
	HEARTHSTONE_INLINE unordered_set<Unit*  >::iterator GetInRangeUnitSetBegin() { return m_unitsInRange.begin(); }
	HEARTHSTONE_INLINE unordered_set<Unit*  >::iterator GetInRangeUnitSetEnd() { return m_unitsInRange.end(); }

	void __fastcall SendMessageToSet(WorldPacket *data, bool self,bool myteam_only=false);
	void OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self);

	//! Fill values with data from a space seperated string of uint32s.
	void LoadValues(const char* data);

	HEARTHSTONE_INLINE uint16 GetValuesCount() const { return m_valuesCount; }

	//! Blizzard seem to send those for all object types. weird.
	float m_walkSpeed;
	float m_runSpeed;
	float m_backWalkSpeed;
	float m_swimSpeed;
	float m_backSwimSpeed;
	float m_turnRate;
	float m_pitchRate;
	float m_flySpeed;
	float m_backFlySpeed;

	float m_base_runSpeed;
	float m_base_walkSpeed;
	uint16 m_movementflags;

	int32 SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage = false, bool no_remove_auras = false, uint32 AdditionalCritChance = 0);

	//*****************************************************************************************
	//* SpellLog packets just to keep the code cleaner and better to read
	//*****************************************************************************************
	void SendSpellLog(Object* Caster, Object* Target,uint32 Ability, uint8 SpellLogType);
	void SendSpellNonMeleeDamageLog( Object* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet );
	void SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate );

	//Dynamic objects
	DynamicObject* dynObj;

	//object faction
	void _setFaction();
	uint32 _getFaction() { return m_faction ? m_faction->Faction : 0; }
	uint32 GetFaction() { return _getFaction(); }

	FactionTemplateDBC *m_faction;
	FactionDBC *m_factionDBC;

	uint32 VendorMask;

	HEARTHSTONE_INLINE void SetInstanceID(int32 instance) { m_instanceId = instance; }
	HEARTHSTONE_INLINE int32 GetInstanceID() { return m_instanceId; }

	int32 event_GetInstanceID();

	bool Active;
	bool CanActivate();
	void Activate(MapMgr* mgr);
	void Deactivate(MapMgr* mgr);
	bool m_inQueue;
	HEARTHSTONE_INLINE void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

	HEARTHSTONE_INLINE size_t GetInRangeOppFactCount() { return m_oppFactsInRange.size(); }
	void PlaySoundToPlayer( Player* plr, uint32 sound_entry );
	void PlaySoundToSet(uint32 sound_entry);
	void EventSpellHit(Spell* pSpell);

	bool PhasedCanInteract(Object* pObj);
	int32 GetPhaseMask() { return m_phaseMask; }
	bool IsInPhase(int32 phaseMode) { if(AllPhases) return true; return ((m_phaseMask & phaseMode) != 0); };
	void EnablePhase(int32 phaseMode);
	void DisablePhase(int32 phaseMode);
	void SendPhaseShift(uint8 phaseMode);

	// Don't fucking use this.
	void SetPhaseMask(int32 phase);

	Aura* m_phaseAura;
	bool OwnPhase; // Players only really.

protected:
	Object (  );

	//void _Create (uint32 guidlow, uint32 guidhigh);
	void _Create( uint32 mapid, float x, float y, float z, float ang);

	//! Mark values that need updating for specified player.
	virtual void _SetUpdateBits(UpdateMask *updateMask, Player* target) const;
	//! Mark values that player should get when he/she/it sees object for first time.
	virtual void _SetCreateBits(UpdateMask *updateMask, Player* target) const;

	void _BuildMovementUpdate( ByteBuffer *data, uint32 flags, uint32 moveflags, Player* target );
	void _BuildValuesUpdate( ByteBuffer *data, UpdateMask *updateMask, Player* target );

	/* Main Function called by isInFront(); */
	bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y );

	bool AllPhases;
	int32 m_phaseMask;
	LocationVector m_phaseLocation;
	uint32 m_phaseDistanceLimit;

	Mutex m_objlock;

	//! WoWGuid class
	WoWGuid m_wowGuid;

	 //! Type id.
	uint8 m_objectTypeId;

	//! Zone id.
	uint32 m_zoneId;
	//! Continent/map id.
	int32 m_mapId;
	//! Map manager
	MapMgr* m_mapMgr;
	//! Current map cell
	MapCell *m_mapCell;

	LocationVector m_position;
	LocationVector m_lastMapUpdatePosition;
	LocationVector m_spawnLocation;

	// Semaphores - needed to forbid two operations on the same object at the same very time (may cause crashing\lack of data)
	bool mSemaphoreTeleport;

	//! Object properties.
	union {
		uint32* m_uint32Values;
		float* m_floatValues;
	};

	//! Number of properties
	uint32 m_valuesCount;

	//! List of object properties that need updating.
	UpdateMask m_updateMask;

	//! True if object was updated
	bool m_objectUpdated;

	//! Set of Objects in range.
	//! TODO: that functionality should be moved into WorldServer.
	unordered_set<Object* > m_objectsInRange;
	unordered_set<Player* > m_inRangePlayers;
	unordered_set<Object* > m_oppFactsInRange;
	unordered_set<Unit* > m_unitsInRange;

	int32 m_instanceId;

	ExtensionSet * m_extensions;
	void _SetExtension(const string& name, void* ptr);		// so we can set from scripts. :)

public:

	template<typename T>
		void SetExtension(const string& name, T ptr)
	{
		_SetExtension(name, ((void*)ptr));
	}

	template<typename T>
		T GetExtension(const string& name)
	{
		if( m_extensions == NULL )
			return ((T)NULL);
		else
		{
			ExtensionSet::iterator itr = m_extensions->find( name );
			if( itr == m_extensions->end() )
				return ((T)NULL);
			else
				return ((T)itr->second);
		}
	}

	bool m_loadedFromDB;

	/************************************************************************/
	/* ACCESSOR FUNCTIONS                                                   */
	/************************************************************************/
	// Stats
	HEARTHSTONE_INLINE uint32 GetStrength() { return m_uint32Values[UNIT_FIELD_STRENGTH]; }
	HEARTHSTONE_INLINE uint32 GetAgility() { return m_uint32Values[UNIT_FIELD_AGILITY]; }
	HEARTHSTONE_INLINE uint32 GetStamina() { return m_uint32Values[UNIT_FIELD_STAMINA]; }
	HEARTHSTONE_INLINE uint32 GetIntellect() { return m_uint32Values[UNIT_FIELD_INTELLECT]; }
	HEARTHSTONE_INLINE uint32 GetSpirit() { return m_uint32Values[UNIT_FIELD_SPIRIT]; }

	// Health
	HEARTHSTONE_INLINE uint32 GetHealth() { return m_uint32Values[UNIT_FIELD_HEALTH]; }
	HEARTHSTONE_INLINE uint32 GetMaxHealth() { return m_uint32Values[UNIT_FIELD_MAXHEALTH]; }
	HEARTHSTONE_INLINE uint32 GetMana() { return m_uint32Values[UNIT_FIELD_POWER1]; }
	HEARTHSTONE_INLINE uint32 GetMaxMana() { return m_uint32Values[UNIT_FIELD_MAXPOWER1]; }

	bool IsInLineOfSight(Object* pObj);
	bool IsInLineOfSight(float x, float y, float z);
	int32 GetSpellBaseCost(SpellEntry *sp);

	/************************************************************************/
	/* END ACCESSOR FUNCTIONS                                               */
	/************************************************************************/

	// declaration to fix scripting
	HEARTHSTONE_INLINE Loot* GetLoot() { return &m_loot; }

public:
	// loooooot
	Loot m_loot;
	bool m_looted;

	// empties loot vector
	void ClearLoot();

public:
	uint16 GetMovementFlags() { return m_movementflags; }
	void AddMovementFlags(uint32 flags) { m_movementflags &= (uint16)flags;}
	void SetMovementFlags(uint32 flags){ m_movementflags = (uint16)flags;}
	void RemoveMovementFlags(uint32 flags){ m_movementflags &= ~(uint16)flags;}
};
