/***
 * Demonstrike Core
 */

#pragma once

#include "../../Common.h"
#include "BIH.h"

/*Flag	Meaning
0x1		Always set
0x4		Has vertex colors (MOCV chunk)
0x8		Outdoor
0x200	Has lights (MOLR chunk)
0x800	Has doodads (MODR chunk)
0x1000	Has water (MLIQ chunk)
0x2000	Indoor
0x40000	Show skybox
**********************
0x8000 seems to be set in the areas in citys (while it has the indoor flag, its not
an indoor area*/

enum VMapAreaFlags
{
	VA_FLAG_EXISTS		= 0x1,
	VA_FLAG_UNK			= 0x2,
	VA_FLAG_VERTEX		= 0x4,
	VA_FLAG_OUTSIDE		= 0x8,
	VA_FLAG_UNK1		= 0x10,
	VA_FLAG_UNK2		= 0x20,
	VA_FLAG_IN_CITY		= 0x40, // Main Cities
	VA_FLAG_UNK3		= 0x80,
	VA_FLAG_IN_CITY2	= 0x100, // Areas inside cities, Cleft of Shadows ect..
	VA_FLAG_HAS_LIGHTS	= 0x200,
	VA_FLAG_UNK5		= 0x400,
	VA_FLAG_HAS_DOODAD	= 0x800,
	VA_FLAG_HAS_WATER	= 0x1000,
	VA_FLAG_INDOORS		= 0x2000,
	VA_FLAG_UNK6		= 0x4000,
	VA_FLAG_IN_CITY3	= 0x8000, // Other cities, like IF, Darnassus, Silvermoon, Exodar, 
	VA_FLAG_HAS_SKYBOX	= 0x40000,
};

namespace VMAP
{
	class ModelInstance;
	class GroupModel;
	class VMapManager2;

	struct LocationInfo
	{
		LocationInfo(): hitInstance(0), hitModel(0), ground_Z(-G3D::inf()) {};
		const ModelInstance *hitInstance;
		const GroupModel *hitModel;
		float ground_Z;
	};

	class StaticMapTree
	{
		typedef UNORDERED_MAP<uint32, bool> loadedTileMap;
		typedef UNORDERED_MAP<uint32, uint32> loadedSpawnMap;
		private:
			uint32 iMapID;
			bool iIsTiled;
			BIH iTree;
			ModelInstance *iTreeValues; // the tree entries
			uint32 iNTreeValues;

			// Store all the map tile idents that are loaded for that map
			// some maps are not splitted into tiles and we have to make sure, not removing the map before all tiles are removed
			// empty tiles have no tile file, hence map with bool instead of just a set (consistency check)
			loadedTileMap iLoadedTiles;
			// stores <tree_index, reference_count> to invalidate tree values, unload map, and to be able to report errors
			loadedSpawnMap iLoadedSpawns;
			std::string iBasePath;

		private:
			bool getIntersectionTime(const G3D::Ray& pRay, float &pMaxDist, bool pStopAtFirstHit) const;
			//bool containsLoadedMapTile(unsigned int pTileIdent) const { return(iLoadedMapTiles.containsKey(pTileIdent)); }
		public:
			static std::string getTileFileName(uint32 mapID, uint32 tileX, uint32 tileY);
			static uint32 packTileID(uint32 tileX, uint32 tileY) { return tileX<<16 | tileY; }
			static void unpackTileID(uint32 ID, uint32 &tileX, uint32 &tileY) { tileX = ID>>16; tileY = ID&0xFF; }
			static bool CanLoadMap(const std::string &basePath, uint32 mapID, uint32 tileX, uint32 tileY);

			StaticMapTree(uint32 mapID, const std::string &basePath);
			~StaticMapTree();

			bool isInLineOfSight(const G3D::Vector3& pos1, const G3D::Vector3& pos2) const;
			bool getObjectHitPos(const G3D::Vector3& pos1, const G3D::Vector3& pos2, G3D::Vector3& pResultHitPos, float pModifyDist) const;
			float getHeight(const G3D::Vector3& pPos, float maxSearchDist) const;
			uint32 GetVmapFlags(Vector3& pos);
			bool getAreaInfo(G3D::Vector3 &pos, uint32 &flags, int32 &adtId, int32 &rootId, int32 &groupId) const;
			bool GetLocationInfo(const Vector3 &pos, LocationInfo &info) const;

			bool InitMap(const std::string &fname, VMapManager2 *vm);
			void UnloadMap(VMapManager2 *vm);
			bool LoadMapTile(uint32 tileX, uint32 tileY, VMapManager2 *vm);
			void UnloadMapTile(uint32 tileX, uint32 tileY, VMapManager2 *vm);
			bool isTiled() const { return iIsTiled; }
			uint32 numLoadedTiles() const { return uint32(iLoadedTiles.size()); }

#ifdef MMAP_GENERATOR
		public:
			void getModelInstances(ModelInstance* &models, uint32 &count);
#endif
	};

	struct AreaInfo
	{
		AreaInfo(): result(false), ground_Z(-G3D::inf()) {};
		bool result;
		float ground_Z;
		uint32 flags;
		int32 adtId;
		int32 rootId;
		int32 groupId;
	};
}															// VMAP
