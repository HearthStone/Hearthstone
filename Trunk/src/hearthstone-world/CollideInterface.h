/***
 * Demonstrike Core
 */

#pragma once

/* imports */
#define NO_WMO_HEIGHT -100000.0f
#define WMO_MAX_HEIGHT 100000.0f

extern VMAP::VMapManager2* CollisionMgr;

class SERVER_DECL CCollideInterface
{
public:
	void Init();
	void DeInit();

	bool ActivateTile(uint32 mapId, uint32 tileX, uint32 tileY);
	void DeactivateTile(uint32 mapId, uint32 tileX, uint32 tileY);
	bool IsActiveTile(uint32 mapId, uint32 tileX, uint32 tileY);
	void ActivateMap(uint32 mapId);
	void DeactivateMap(uint32 mapId);

	bool CheckLOS(uint32 mapId, float x1, float y1, float z1, float x2, float y2, float z2);
	bool GetFirstPoint(uint32 mapId, float x1, float y1, float z1, float x2, float y2, float z2, float & outx, float & outy, float & outz, float distmod);
	bool IsIndoor(uint32 mapId, float x, float y, float z);
	bool IsIncity(uint32 mapid, float x, float y, float z);
	uint32 GetVmapAreaFlags(uint32 mapId, float x, float y, float z);
	float GetHeight(uint32 mapId, float x, float y, float z);
};

extern SERVER_DECL CCollideInterface CollideInterface;
