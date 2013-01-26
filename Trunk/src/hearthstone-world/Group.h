/***
 * Demonstrike Core
 */

#pragma once

enum PartyErrors
{
	ERR_PARTY_NO_ERROR					= 0,
	ERR_PARTY_CANNOT_FIND				= 1,
	ERR_PARTY_IS_NOT_IN_YOUR_PARTY		= 2,
	ERR_PARTY_UNK						= 3,
	ERR_PARTY_IS_FULL					= 4,
	ERR_PARTY_ALREADY_IN_GROUP			= 5,
	ERR_PARTY_YOU_ARENT_IN_A_PARTY		= 6,
	ERR_PARTY_YOU_ARE_NOT_LEADER		= 7,
	ERR_PARTY_WRONG_FACTION				= 8,
	ERR_PARTY_IS_IGNORING_YOU			= 9,
};

enum GroupTypes
{
	GROUP_TYPE_PARTY	= 0x00,
	GROUP_TYPE_BG		= 0x01,
	GROUP_TYPE_RAID		= 0x02,
	GROUP_TYPE_BGRAID	= GROUP_TYPE_BG | GROUP_TYPE_RAID,
	GROUP_TYPE_LFD		= 0x08,
	// 0x10 Leaving battleground, going to normal group
};

enum MaxGroupCount
{
	MAX_GROUP_SIZE_PARTY	= 5,
	MAX_GROUP_SIZE_RAID		= 40,
};

enum QuickGroupUpdateFlags
{
	PARTY_UPDATE_FLAG_POSITION			= 1,
	PARTY_UPDATE_FLAG_ZONEID			= 2,
//	GROUP_UPDATE_FLAG_VEHICLE_SEAT
};

enum GroupFlags
{
	GROUP_FLAG_DONT_DISBAND_WITH_NO_MEMBERS			= 1,
	GROUP_FLAG_REMOVE_OFFLINE_PLAYERS				= 2,
	GROUP_FLAG_BATTLEGROUND_GROUP					= 4,
};

struct PlayerInfo;
typedef struct
{
	PlayerInfo * player_info;
	Player* player;
}GroupMember;

class Group;
class Player;

typedef std::set<PlayerInfo*> GroupMembersSet;

class SERVER_DECL SubGroup	  // Most stuff will be done through here, not through the "Group" class.
{
public:
	friend class Group;

	SubGroup(Group* parent, uint32 id):m_Parent(parent),m_Id(id)
	{
	}

	~SubGroup();

	HEARTHSTONE_INLINE GroupMembersSet::iterator GetGroupMembersBegin(void) { return m_GroupMembers.begin(); }
	HEARTHSTONE_INLINE GroupMembersSet::iterator GetGroupMembersEnd(void)   { return m_GroupMembers.end();   }

	bool AddPlayer(PlayerInfo * info);
	void RemovePlayer(PlayerInfo * info);

	HEARTHSTONE_INLINE bool IsFull(void)				{ return m_GroupMembers.size() >= MAX_GROUP_SIZE_PARTY; }
	HEARTHSTONE_INLINE size_t GetMemberCount(void)		{ return m_GroupMembers.size(); }

	HEARTHSTONE_INLINE uint32 GetID(void)			   { return m_Id; }
	HEARTHSTONE_INLINE void SetID(uint32 newid)		 { m_Id = newid; }

	HEARTHSTONE_INLINE void   SetParent(Group* parent)  { m_Parent = parent; }
	HEARTHSTONE_INLINE Group* GetParent(void)		   { return m_Parent; }

	void   Disband();
	bool HasMember(uint32 guid);

protected:

	GroupMembersSet	 m_GroupMembers;
	Group*			  m_Parent;
	uint32			  m_Id;

};

class Arena;
class SERVER_DECL Group
{
public:
	friend class SubGroup;

	std::map<uint32, uint32> m_BeaconOfLightTargets;

	static Group* Create();

	Group(bool Assign);
	~Group();

	// Adding/Removal Management
	bool AddMember(PlayerInfo * info, int32 subgroupid=-1);
	void RemovePlayer(PlayerInfo * info);
	void AddBeaconOfLightTarget(uint32 LowGUID, uint32 CasterLow);
	void RemoveBeaconOfLightTarget(uint32 LowGUID, uint32 CasterLow);

	// Leaders and Looting
	void SetLeader(Player* pPlayer,bool silent);
	void SetLooter(Player* pPlayer, uint8 method, uint16 threshold);

	// Transferring data to clients
	void Update();

	HEARTHSTONE_INLINE void SendPacketToAll(WorldPacket *packet) { SendPacketToAllButOne(packet, NULLPLR); }
	void SendPacketToAllButOne(WorldPacket *packet, Player* pSkipTarget);

	HEARTHSTONE_INLINE void OutPacketToAll(uint16 op, uint16 len, const void* data) { OutPacketToAllButOne(op, len, data, NULLPLR); }
	void OutPacketToAllButOne(uint16 op, uint16 len, const void* data, Player* pSkipTarget);

	void SendNullUpdate(Player* pPlayer);

	// Group Combat
	void SendPartyKillLog(Object* player, Object* Unit);

	// Destroying/Converting
	void Disband();
	Player* FindFirstPlayer();
	bool HasAcceptableDisenchanters(int32 requiredskill);

	// Accessing functions
	HEARTHSTONE_INLINE SubGroup* GetSubGroup(uint32 Id)
	{
		if(Id >= 8)
			return 0;

		return m_SubGroups[Id];
	}

	HEARTHSTONE_INLINE uint32 GetSubGroupCount(void) { return m_SubGroupCount; }

	HEARTHSTONE_INLINE uint8 GetMethod(void) { return m_LootMethod; }
	HEARTHSTONE_INLINE uint16 GetThreshold(void) { return m_LootThreshold; }
	HEARTHSTONE_INLINE PlayerInfo* GetLeader(void) { return m_Leader; }
	HEARTHSTONE_INLINE PlayerInfo* GetLooter(void) { return m_Looter; }

	void MovePlayer(PlayerInfo* info, uint8 subgroup);

	bool HasMember(Player* pPlayer);
	bool HasMember(PlayerInfo * info);
	HEARTHSTONE_INLINE uint32 MemberCount(void) { return m_MemberCount; }
	HEARTHSTONE_INLINE bool IsFull() { return ((m_GroupType == GROUP_TYPE_PARTY && m_MemberCount >= MAX_GROUP_SIZE_PARTY) || (m_GroupType == GROUP_TYPE_RAID && m_MemberCount >= MAX_GROUP_SIZE_RAID)); }
	//HEARTHSTONE_INLINE uint32 GetOnlineMemberCount();
	uint32 GetOnlineMemberCount();

	SubGroup* FindFreeSubGroup();

	void ExpandToRaid();

	void SaveToDB();
	void LoadFromDB(Field *fields);

	HEARTHSTONE_INLINE uint8 GetGroupType() { return m_GroupType; }
	HEARTHSTONE_INLINE uint32 GetID() { return m_Id; }

	void UpdateOutOfRangePlayer(Player* pPlayer, uint32 Flags, bool Distribute, WorldPacket * Packet);
	void UpdateAllOutOfRangePlayersFor(Player* pPlayer);
	void HandleUpdateFieldChange(uint32 Index, Player* pPlayer);
	void HandlePartialChange(uint32 Type, Player* pPlayer);

	uint64 m_targetIcons[8];
	HEARTHSTONE_INLINE Mutex& getLock() { return m_groupLock; }
	HEARTHSTONE_INLINE void Lock() { m_groupLock.Acquire(); }
	HEARTHSTONE_INLINE void Unlock() { return m_groupLock.Release(); }
	bool m_isqueued;

	void SetAssistantLeader(PlayerInfo * pMember);
	void SetMainTank(PlayerInfo * pMember);
	void SetMainAssist(PlayerInfo * pMember);

	HEARTHSTONE_INLINE PlayerInfo * GetAssistantLeader() { return m_assistantLeader; }
	HEARTHSTONE_INLINE PlayerInfo * GetMainTank() { return m_mainTank; }
	HEARTHSTONE_INLINE PlayerInfo * GetMainAssist() { return m_mainAssist; }

public:
	HEARTHSTONE_INLINE void SetFlag(uint8 groupflag) { m_groupFlags |= groupflag; }
	HEARTHSTONE_INLINE void RemoveFlag(uint8 groupflag) { m_groupFlags &= ~groupflag; }
	HEARTHSTONE_INLINE bool HasFlag(uint8 groupflag) { return (m_groupFlags & groupflag) > 0 ? true : false; }
	HEARTHSTONE_INLINE int8 GetDifficulty() { return m_difficulty; }
	HEARTHSTONE_INLINE int8 GetRaidDifficulty() { return m_raiddifficulty; }
	HEARTHSTONE_INLINE void SetDifficulty(uint8 diff) { m_difficulty = diff; }
	HEARTHSTONE_INLINE void SetRaidDifficulty(uint8 diff) { m_raiddifficulty = diff; }
	uint8 m_difficulty;
	uint8 m_raiddifficulty;

protected:
	PlayerInfo * m_Leader;
	PlayerInfo * m_Looter;
	PlayerInfo * m_assistantLeader;
	PlayerInfo * m_mainTank;
	PlayerInfo * m_mainAssist;

	uint8 m_LootMethod;
	uint16 m_LootThreshold;
	uint8 m_GroupType;
	uint32 m_Id;

	SubGroup* m_SubGroups[8];
	uint8 m_SubGroupCount;
	uint32 m_MemberCount;
	Mutex m_groupLock;
	bool m_dirty;
	bool m_updateblock;
	uint8 m_groupFlags;
};
