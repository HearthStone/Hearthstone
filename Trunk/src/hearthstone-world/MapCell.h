/***
 * Demonstrike Core
 */

//
// MapCell.h
//

#pragma once

class Map;

#define MAKE_CELL_EVENT(x,y) ( ((x) * 1000) + 200 + y )
#define DECODE_CELL_EVENT(dest_x, dest_y, ev) (dest_x) = ((ev-200)/1000); (dest_y) = ((ev-200)%1000);

class SERVER_DECL MapCell
{
	friend class MapMgr;
public:
	MapCell();
	~MapCell();

	typedef unordered_set<Object* > ObjectSet;

	//Init
	void Init(uint32 x, uint32 y, uint32 mapid, MapMgr* mapmgr);

	//Object Managing
	void AddObject(Object* obj);
	void RemoveObject(Object* obj);
	bool HasObject(Object* obj) { return (_objects.find(obj) != _objects.end()); }
	bool HasPlayers() { return ((_playerCount > 0) ? true : false); }
	HEARTHSTONE_INLINE size_t GetObjectCount() { return _objects.size(); }
	void RemoveObjects();
	HEARTHSTONE_INLINE ObjectSet::iterator Begin() { return _objects.begin(); }
	HEARTHSTONE_INLINE ObjectSet::iterator End() { return _objects.end(); }

	//State Related
	void SetActivity(bool state);

	HEARTHSTONE_INLINE bool IsActive() { return _active; }
	HEARTHSTONE_INLINE bool IsLoaded() { return _loaded; }

	//Object Loading Managing
	void LoadObjects(CellSpawns * sp);
	HEARTHSTONE_INLINE uint32 GetPlayerCount() { return _playerCount; }

	//ING Events
	void RemoveEventIdObjects(uint8 eventToRemove);
	void ModifyEventIdSetting(bool active, uint8 eventId);
	void LoadEventIdObjects(CellSpawns * sp, uint8 eventId);

	HEARTHSTONE_INLINE bool IsUnloadPending() { return _unloadpending; }
	HEARTHSTONE_INLINE void SetUnloadPending(bool up) { _unloadpending = up; }
	void QueueUnloadPending();
	void CancelPendingUnload();
	void Unload();

	void SetPermanentActivity(bool val) { _forcedActive = val; }
	bool IsForcedActive() { return _forcedActive; }

	uint16 GetPositionX() { return _x; }
	uint16 GetPositionY() { return _y; }

	ObjectSet _respawnObjects;

private:
	bool _forcedActive;
	uint16 _x,_y;
	ObjectSet _objects;
	bool _active, _loaded;
	bool _unloadpending;

	uint16 _playerCount;
	MapMgr* _mapmgr;
};
