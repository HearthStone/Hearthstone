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

#include "vmapexport.h"
#include "liquidmodel.h"
#include "wmo.h"
#include "mpq_libmpq04.h"
#include <cassert>
#include <algorithm>
#include <cstdio>
#include <sstream>

extern uint16 *LiqType;

LiquidModelInstance::LiquidModelInstance(MPQFile &f, uint32 base_pos, uint32 tileID, MH20_Header* header, uint32 mapID, uint32 tileX, uint32 tileY, FILE *pDirfile) : Info(false)
{
	memset(&LiqHeader, 0, sizeof(LiquidHeader));
	LiqHeader.layerCount = header->layerCount;
	if(header->ofsInformation == 0)
	{
		Info = false;
		return;
	}

#ifdef _DEBUG
	printf("Information offset: %i\n", header->ofsInformation);
#endif
	f.seek(header->ofsInformation+base_pos);
	f.read(&LiqHeader.info, sizeof(MH20_Information));

#ifdef _DEBUG
	if(LiqHeader.info.LiquidType != 2) // Skip oceans
	{
		printf("Liquid Data Dump:\n");
		printf("Flags: %08i |\n", LiqHeader.info.flags);
//		printf("Height %08i |\n", LiqHeader.info.height);
//		printf("Level1 %08f |\n", LiqHeader.info.heightLevel1);
//		printf("Level2 %08f |\n", LiqHeader.info.heightLevel2);
		printf("LType: %08i |\n", LiqHeader.info.LiquidType);
		printf("Width: %08i |\n", LiqHeader.info.width);
		printf("xOfset %08i |\n", LiqHeader.info.xOffset);
		printf("yOfset %08i |\n", LiqHeader.info.yOffset);
		WaitForInput();
	}
#endif

	uint16* buff = new uint16[3];
	buff[0] = buff[1] = buff[2] = 0;
	if(LiqHeader.info.ofsHeightmap > 0)
	{
#ifdef _DEBUG
		printf("Height Map Offset: %i\n", LiqHeader.info.ofsHeightmap);
#endif
		f.seek(LiqHeader.info.ofsHeightmap+base_pos);
		f.read(&LiqHeader.heightMap.heightMap, sizeof(float)*2);
		f.read(&LiqHeader.heightMap.transparency, sizeof(char)*2);
		buff[0] = sizeof(LiqHeader.heightMap);
	}

	if(LiqHeader.info.ofsMask2 > 0)
	{
#ifdef _DEBUG
		printf("Mask Offset: %i\n", LiqHeader.info.ofsMask2);
#endif
		f.seek(LiqHeader.info.ofsMask2+base_pos);
		f.read(&LiqHeader.Mask2, sizeof(uint8));
		buff[1] = sizeof(uint8);
	}

	if(header->ofsRender > 0)
	{
#ifdef _DEBUG
		printf("Render offset: %i\n", header->ofsRender);
#endif
		f.seek(header->ofsRender+base_pos);
		f.read(&LiqHeader.Render, sizeof(uint64));
		buff[2] = sizeof(uint64);
	}

	fwrite(&LiqHeader.layerCount, sizeof(uint32), 1, pDirfile);
	fwrite(&LiqHeader.info, sizeof(MH20_Information), 1, pDirfile);

	fwrite(&buff[0], sizeof(uint16), 1, pDirfile);
	if(buff[0])
		fwrite(&LiqHeader.heightMap, sizeof(MH2O_HeightMapData), 1, pDirfile);
	fwrite(&buff[1], sizeof(uint16), 1, pDirfile);
	if(buff[1])
		fwrite(&LiqHeader.Mask2, sizeof(uint8), 1, pDirfile);
	fwrite(&buff[2], sizeof(uint16), 1, pDirfile);
	if(buff[2])
		fwrite(&LiqHeader.Render, sizeof(uint64), 1, pDirfile);
}
