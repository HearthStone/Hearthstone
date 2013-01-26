/***
 * Demonstrike Core
 */

#pragma once

#define WINTERGRASP 4197
#define A_NUMVEH_WORLDSTATE 3680
#define A_MAXVEH_WORLDSTATE 3681
#define H_NUMVEH_WORLDSTATE 3490
#define H_MAXVEH_WORLDSTATE 3491
#define HORDE_CONTROLLED 3802
#define ALLIANCE_CONTROLLED 3803
const uint32 ClockWorldState[5] = { 3785, 3784, 3782, 3976, 3975};

#define WGSPELL_ESSENCE_OF_WINTERGRASP 57940
#define WGSPELL_TENACITY 59911

#define DEFENDERS_PORTAL 190763 // 2d: The portal thing which you select to teleport inside if a defender

typedef std::set<Creature*> CreatureSwapSet;
typedef std::set<Player*> WintergraspPlayerSet;

class Wintergrasp
{
public:
	Wintergrasp(WintergraspInternal* WGI, MapMgr* mgr);
	~Wintergrasp();
	static Wintergrasp* Create( WintergraspInternal* i, MapMgr* m) { return new Wintergrasp(i, m); }
	void Init();
	void End(Player*plr);
	void ForceEnd();
	void OnAddPlayer(Player* plr);
	void OnRemovePlayer(Player* plr);
	WintergraspPlayerSet WGPlayers;
	CreatureSwapSet WGCreatures[2];
	uint32 GetNumVehicles(uint32 team) { if(team > 1) return 0; return numvehicles[team]; };
	uint32 GetNumWorkshops(uint32 team) { if(team > 1) return 0; return numworkshop[team]; };
	void GoDestroyEvent(uint32 Entry, Player* Plr);
	void GoDamageEvent(uint32 Entry, Player* Plr);
	void _SendMessage(const char* text);
	void ShortenBattle(uint32 Time);
	HEARTHSTONE_INLINE uint64 GetID() { return WGID; };
	// Workshops
	uint32 numworkshop[2];

private:
	WintergraspInternal Internal;
	// Counts
	uint32 playercount[3];
	// Vehicles
	uint32 numvehicles[2];
	// Same as BattleGround id
	uint64 WGID;
	bool FlameWatchDestroyed;
	bool ShadowsightDestroyed;
	bool WintersEdgeDestroyed;
};

enum GameObjectEntries
{
    TITAN_RELIC           = 192829,
    FORTRESS_GATE         = 190375,
    KEEP_DOOR01_COLLISION = 194162,
    KEEP_COLLISION_WALL   = 194323,
    WORKSHOP_NW           = 192028,
    WORKSHOP_W            = 192030,
    WORKSHOP_SW           = 192032,
    WORKSHOP_NE           = 192029,
    WORKSHOP_E            = 192031,
    WORKSHOP_SE           = 192033,
    WORKSHOP_BANNER_NO    = 190475,
    WORKSHOP_BANNER_NW    = 190487,
    TOWER_SHADOWSIGHT     = 190356,
    TOWER_WINTERS_EDGE    = 190357,
    TOWER_FLAMEWATCH      = 190358,
    KEEP_TOWER_NW         = 190221,
    KEEP_TOWER_NE         = 190378,
    KEEP_TOWER_SW         = 190373,
    KEEP_TOWER_SE         = 190377,
    WALL_1                = 191797,
    WALL_2                = 191798,
    WALL_3                = 191805,
    FORTRESS_WALL_1       = 191799,
    FORTRESS_WALL_2       = 191809,
    FORTRESS_DOOR         = 191810,
    DEFENDER_PORTAL_1     = 190763,
    DEFENDER_PORTAL_2     = 191575,
    DEFENDER_PORTAL_3     = 192819,
    VEHICLE_TELEPORTER    = 192951,
    BANNER_A              = 192487,
    BANNER_H              = 192488
};
