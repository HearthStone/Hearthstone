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

#pragma once

#include "loadlib/loadlib.h"
#include "vec3d.h"
//#include "mpq.h"
#include "modelheaders.h"
#include <vector>

class Model;
class WMOInstance;
class MPQFile;

Vec3D fixCoordSystem(Vec3D v);

class LiquidModelInstance
{
private:
	LiquidHeader LiqHeader;

public:
	bool Info;

    LiquidModelInstance() {}
    LiquidModelInstance(MPQFile &f, uint32 base_pos, uint32 tileID, MH20_Header* header, uint32 mapID, uint32 tileX, uint32 tileY, FILE *pDirfile);

	bool ShouldWrite() { return Info; };
};
