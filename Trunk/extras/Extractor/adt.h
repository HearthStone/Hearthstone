#ifndef ADT_H
#define ADT_H

#define WaitForInput() { char cmd[300]; memset( cmd, 0, sizeof( cmd ) ); fgets( cmd, 300, stdin ); }

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)

#if defined(_MSC_VER) || defined(__BORLANDC__)
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
#endif

#define SMALL_MAPFILES
#define PACK_MAPS
#define MAP_RESOLUTION 256

#define TilesCount 64
#define TileSize 533.33333f

#define _minY (-TilesCount*TileSize/2)
#define _minX (-TilesCount*TileSize/2)

#define _maxY (TilesCount*TileSize/2)
#define _maxX (TilesCount*TileSize/2)

#define CellsPerTile 8
#define _cellSize (TileSize/CellsPerTile)
#define _sizeX (TilesCount*CellsPerTile)
#define _sizeY (TilesCount*CellsPerTile)

typedef struct {
float x;
float y;
float z;
}svec;

typedef struct {
double x;
double y;
double z;
}vec;

typedef struct{
	vec v[3];

}triangle;

class WMO;
class WMOManager;
void fixname(std::string &name);

class MPQFile;

class MPQFile;

typedef struct
{
	uint32 ofsInformation;
	uint32 layerCount;
	uint32 ofsData;
}MH2Oheader;

typedef struct
{
	uint32 flags;
	uint32 ix;
	uint32 iy;
	uint32 nLayers;
	uint32 nDoodadRefs;
	uint32 ofsHeight;
	uint32 ofsNormal;
	uint32 ofsLayer;
	uint32 ofsRefs;
	uint32 ofsAlpha;
	uint32 sizeAlpha;
	uint32 ofsShadow;
	uint32 sizeShadow;
	uint32 areaid;
	uint32 nMapObjRefs;
	uint32 holes;
	uint16 s1;
	uint16 s2;
	uint32 d1;
	uint32 d2;
	uint32 d3;
	uint32 predTex;
	uint32 nEffectDoodad;
	uint32 ofsSndEmitters;
	uint32 nSndEmitters;
	uint32 ofsLiquid;
	uint32 sizeLiquid;
	float  xpos;
	float  ypos;
	float  zpos;
	uint32 textureId;
	uint32 props;
	uint32 effectId;
}MapChunkHeader;

typedef struct
{
	uint16 type;
	uint16 flags;
	float levels[2];
	uint8 xOffset;
	uint8 yOffset;
	uint8 Width;
	uint8 Height;
	uint32 offsData2a;
	uint32 offsData2b;
}MH2Oinformation;

typedef struct
{
	uint16 AreaInfo[256];
	uint16 LiquidInfo[256];
	uint64 LiquidMasks[256];

	float V8[128][128];
	float V9[128+1][128+1];

	bool  liquid_show[128][128];
	float liquid_height[128+1][128+1];
}mcell;

struct M2Header
{
	char id[4];
	unsigned char version[4];
	uint32 nameLength;
	uint32 nameOfs;
	uint32 type;
	uint32 nGlobalSequences;
	uint32 ofsGlobalSequences;
	uint32 nAnimations;
	uint32 ofsAnimations;
	uint32 nAnimationLookup;
	uint32 ofsAnimationLookup;
	uint32 nBones;
	uint32 ofsBones;
	uint32 nKeyBoneLookup;
	uint32 ofsKeyBoneLookup;

	uint32 nVertices;
	uint32 ofsVertices;
	uint32 nViews;

	uint32 nColors;
	uint32 ofsColors;

	uint32 nTextures;
	uint32 ofsTextures;

	uint32 nTransparency;
	uint32 ofsTransparency;
	uint32 nUVAnimation;
	uint32 ofsUVAnimation;
	uint32 nTexReplace;
	uint32 ofsTexReplace;

	uint32 nRenderFlags;
	uint32 ofsRenderFlags;
	uint32 nBoneLookupTable;
	uint32 ofsBoneLookupTable;

	uint32 nTexLookup;
	uint32 ofsTexLookup;

	uint32 nTexUnitLookup;
	uint32 ofsTexUnitLookup;
	uint32 nTransparencyLookup;
	uint32 ofsTransparencyLookup;
	uint32 nUVAnimLookup;
	uint32 ofsUVAnimLookup;

	float vertexbox1[3];
	float vertexbox2[3];
	float vertexradius;
	float boundingbox1[3];
	float boundingbox2[3];
	float boundingradius;

	uint32 nBoundingTriangles;
	uint32 ofsBoundingTriangles;
	uint32 nBoundingVertices;
	uint32 ofsBoundingVertices;
	uint32 nBoundingNormals;
	uint32 ofsBoundingNormals;

	uint32 nAttachments;
	uint32 ofsAttachments;
	uint32 nAttachmentLookup;
	uint32 ofsAttachmentLookup;
	uint32 nEvents;
	uint32 ofsEvents;
	uint32 nLights;
	uint32 ofsLights;
	uint32 nCameras;
	uint32 ofsCameras;
	uint32 nCameraLookup;
	uint32 ofsCameraLookup;
	uint32 nRibbonEmitters;
	uint32 ofsRibbonEmitters;
	uint32 nParticleEmitters;
	uint32 ofsParticleEmitters;
};

struct M2Header_Old
{
	char id[4];
	uint8 version[4];
	uint32 nameLength, nameOfs;
	uint32 type;
	uint32 nGlobalSequences, ofsGlobalSequences;
	uint32 nAnimations, ofsAnimations;
	uint32 nAnimationLookup, ofsAnimationLookup;
	uint32 nD, ofsD;
	uint32 nBones, ofsBones;
	uint32 nKeyBoneLookup, ofsKeyBoneLookup;
	uint32 nVertices, ofsVertices;
	uint32 nViews, ofsViews;
	uint32 nColors, ofsColors, nTextures, ofsTextures;
	uint32 nTransparency, ofsTransparency, nI, ofsI;
	uint32 nTextureanimations, ofsTextureanimations;
	uint32 nTexReplace, ofsTexReplace;
	uint32 nRenderFlags, ofsRenderFlags;
	uint32 nBoneLookupTable, ofsBoneLookupTable;
	uint32 nTexLookup, ofsTexLookup;
	uint32 nTexUnits, ofsTexUnits;
	uint32 nTransLookup, ofsTransLookup;
	uint32 nTexAnimLookup, ofsTexAnimLookup;
	float vertexbox1[3], vertexbox2[3], vertexradius;
	float boundingbox1[3], boundingbox2[3], boundingradius;
	uint32 nBoundingTriangles, ofsBoundingTriangles;
	uint32 nBoundingVertices, ofsBoundingVertices;
	uint32 nBoundingNormals, ofsBoundingNormals;
	uint32 nAttachments, ofsAttachments;
	uint32 nAttachLookup, ofsAttachLookup;
	uint32 nAttachments_2, ofsAttachments_2;
	uint32 nLights, ofsLights;
	uint32 nCameras, ofsCameras;
	uint32 nCameraLookup, ofsCameraLookup;
	uint32 nRibbonEmitters, ofsRibbonEmitters;
	uint32 nParticleEmitters, ofsParticleEmitters;
};

struct AnimationBlock
{
	uint16 interpolation;
	uint16 globalsequenceid;
	uint32 list1offset;
	uint32 timestampdataoffset;
	uint32 list2offset;
	uint32 keysoffset;
};

struct M2Attachment
{
	uint32 id;
	uint32 bone;
	float pos[3];
	AnimationBlock unk;
};

struct M2Bone
{
	int keyboneid;
	uint32 flags;
	short parentbone;
	uint16 unk[3];
	AnimationBlock translation;
	AnimationBlock rotation;
	AnimationBlock scaling;
	float pivotpoint[3];
};

#endif

