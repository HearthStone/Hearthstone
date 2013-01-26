/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MODELHEADERS_H
#define MODELHEADERS_H

/* typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32; */

#pragma pack(push,1)

struct ModelHeader0
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

struct ModelHeader
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

struct MH20_Header { uint32 ofsInformation, layerCount, ofsRender; };
struct MH2O_HeightMapData { float *heightMap; char *transparency; };
struct MH20_Information
{
/*0x00*/int16	LiquidType;		//	Points to LiquidType.dbc
/*0x02*/int16	flags;			//
/*0x04*/float	heightLevel1;	//	The global liquid-height of this chunk. Which is always in there twice. Blizzard knows why.
/*0x08*/float	heightLevel2;	//	(Actually these values are not always identical, I think they define the highest and lowest points in the heightmap)
/*0x0C*/int8	xOffset;		//	The X offset of the liquid square (0-7)
/*0x0D*/int8	yOffset;		//	The Y offset of the liquid square (0-7)
/*0x0E*/int8	width;			//	The width of the liquid square (1-8)
/*0x0F*/int8	height;			//	The height of the liquid square (1-8)
/*0x10*/int32	ofsMask2;		//	Offset to some data.
/*0x14*/int32	ofsHeightmap;	//	Offset to MH2O_HeightmapData structure for this chunk.
};

struct LiquidHeader
{
	MH2O_HeightMapData heightMap;
	MH20_Information info;
	uint32 layerCount;
	uint64 Render;
	uint8 Mask2;
};

#pragma pack(pop)
#endif
