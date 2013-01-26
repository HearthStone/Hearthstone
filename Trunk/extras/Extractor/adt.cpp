#ifdef WIN32
#include <windows.h>
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <bitset>
#include <sstream>

#include "adt.h"
#include "mpq_libmpq.h"

#define FL2UINT(f) uint32(f == 0 ? f : floor(f))

bool AreCoordinatesValid(float x, float y)
{
	if(x > _maxX || x < _minX)
		return false;
	if(y > _maxY || y < _minY)
		return false;
	return true;
}

__int32 GetTile(float pos)
{
	return (__int32)(32 - (pos / 533.33333f));
}

/* Converts a global x co-ordinate into a cell x co-ordinate.
	Parameter 1: global x co-ordinate.
	Returns the cell x co-ordinate.
	*/
uint32 ConvertGlobalXCoordinate(float x)
{
	return FL2UINT((float(_maxX)-float(x))/float(_cellSize));
}

/* Converts a global y co-ordinate into a cell y co-ordinate.
	Parameter 1: global y co-ordinate.
	Returns the cell y co-ordinate.
*/
uint32 ConvertGlobalYCoordinate(float y)
{
	return FL2UINT((float(_maxY)-float(y))/float(_cellSize));
}

uint32 ConvertToChunk(uint32 Cell, uint32 Tile)
{
	if(Tile == 0)
		return Cell;
	return Cell%Tile;
}

/* Converts a global x co-ordinate into a INTERNAL cell x co-ordinate.
	Parameter 1: global x co-ordinate.
	Parameter 2: the cell x co-ordinate.
	Returns the internal x co-ordinate.
*/
float ConvertInternalXCoordinate(float x, uint32 cellx)
{
	float X = (_maxX - x);
	X -= (cellx * _cellSize);
	return X;
}

/* Converts a global y co-ordinate into a INTERNAL cell y co-ordinate.
	Parameter 1: global y co-ordinate.
	Parameter 2: the cell y co-ordinate.
	Returns the internal y co-ordinate.
*/
float ConvertInternalYCoordinate(float y, uint32 celly)
{
	float Y = (_maxY - y);
	Y -= (celly * _cellSize);
	return Y;
}

/* Converts the internal co-ordinate to an index in the
	2 dimension areaid, or liquid type arrays.
	*/
uint32 ConvertTo2dArray(float c)
{
	return FL2UINT(float(c)*float(float(16)/float(CellsPerTile)/float(_cellSize)));
}

//#pragma pack(push, 1)

typedef struct
{
	uint16	AreaID[16/CellsPerTile][16/CellsPerTile];
	uint16	LiquidType[16/CellsPerTile][16/CellsPerTile];
	float	Z[MAP_RESOLUTION/CellsPerTile][MAP_RESOLUTION/CellsPerTile];
	float	LZ[MAP_RESOLUTION/CellsPerTile][MAP_RESOLUTION/CellsPerTile];
}MapCellInfo;

typedef struct
{
	float MinX, MaxX;
	float MinY, MaxY;
}HeightStore;

//#pragma pack(pop)

uint32 wmo_count;
mcell *mcells;

mcell * mcell_cache[64][64];

void reset()
{
	for(uint32 i = 0; i < 64; ++i)
		for(uint32 j = 0; j < 64; ++j)
			mcell_cache[i][j] = 0;
}

void CleanCache()
{
	for(uint32 i = 0; i < 64; ++i)
	{
		for(uint32 j = 0; j < 64; ++j)
		{
			if(mcell_cache[i][j] != 0)
			{
				delete mcell_cache[i][j];
				mcell_cache[i][j] = 0;
			}
		}
	}
}

uint16* LiqType;
extern int expansion;

uint32 s_Tx = 0;
uint32 s_Ty = 0;

inline uint32 LoadMapChunk(MPQFile & mf, uint32 ch_x, uint32 ch_y, mcell *Cell, bool ReadMLQ)
{
	uint32 size;
	uint32 result = 0;
	mf.seekRelative(4);
	mf.read(&size, 4);
	size_t lastpos = mf.getPos() + size;
	MapChunkHeader header;
	mf.read(&header, 0x80);
	Cell->AreaInfo[ch_x*16+ch_y] = header.areaid;

	float Zbase = header.zpos;
	int chunkflags = header.flags;
	for (int y = 0; y <= 8; y++)
	{
		int cy = ch_x*8 + y;
		for (int x = 0; x <= 8; x++)
		{
			int cx = ch_y*8 + x;
			Cell->V9[cy][cx] = Zbase;
			if(y == 8 || x == 8)
				continue;
			Cell->V8[cy][cx]= Zbase;
		}
	}

	int nTextures = 0;
	while (mf.getPos() < lastpos)
	{
		char fourcc[5];
		uint32 fourcc2;
		mf.read(&fourcc2, 4);
		mf.read(&size, 4);
		if(size == 0 && fourcc2 == 0)
			continue;

		memcpy(&fourcc, &fourcc2, 4);
		flipcc(fourcc);
		fourcc[4] = 0;

		size_t nextpos = mf.getPos()+size;
		if (!strcmp(fourcc, "MCVT"))
		{
			result |= 0x01;
			float *height_map = new float[145];
			int pos = 0;
			while(pos < 145)
				mf.read(&height_map[pos++], sizeof(float));

			// get V9 height map
			for (int y=0; y <= 8; y++)
			{
				int cy = ch_x*8 + y;
				for (int x=0; x <= 8; x++)
				{
					int cx = ch_y*8 + x;
					Cell->V9[cy][cx] += height_map[y*(8*2+1)+x];
				}
			}

			// get V8 height map
			for (int y=0; y < 8; y++)
			{
				int cy = ch_x*8 + y;
				for (int x=0; x < 8; x++)
				{
					int cx = ch_y*8 + x;
					Cell->V8[cy][cx] += height_map[y*(8*2+1)+8+1+x];
				}
			}
			delete height_map;
			height_map = NULL;
		}
		else if (!strcmp(fourcc, "MCLV"))
		{

		}
		else if (!strcmp(fourcc, "MCCV"))
		{

		}
		else if(!strcmp(fourcc, "MCNR"))
		{
			nextpos = mf.getPos() + 0x1C0; // size fix
		}
		else if (!strcmp(fourcc, "MCLY"))
		{
			nTextures = (int)size;
		}
		else if (!strcmp(fourcc, "MCRF"))
		{

		}
		else if (!strcmp(fourcc, "MCSH"))
		{

		}
		else if (!strcmp(fourcc, "MCAL"))
		{
			if(expansion == 0 || expansion == 1)
			{
				if(nTextures <= 0)
					continue;
			}
			else if(expansion == 2)
			{
				if(size)
				{
					if(size == 4096)
					{

					}
					else if(size == 2048)
					{

					}
					else if(size == 4) // Seems legit.
					{
						continue;
					}
				}
			}
		}
		else if(!strcmp(fourcc, "MCLQ")) //MCLQ
		{
			if(header.sizeLiquid)
			{
				size = header.sizeLiquid-8; // MCLQ+size
				if(size)
				{
					nextpos = mf.getPos()+size;
					if(ReadMLQ)
					{
						result |= 0x02;
						float height1;
						float height2;
						struct liquid_data
						{
							uint32 light;
							float  height;
						} liquid[9][9];
						uint8 liquidflags[8][8];

						mf.read(&height1, sizeof(float));
						mf.read(&height2, sizeof(float));
						mf.read(&liquid, sizeof(liquid_data)*9*9);
						mf.read(&liquidflags, sizeof(uint8)*8*8);

						int count = 0;
						for (int y=0; y < 8; y++)
						{
							int cy = ch_y*8 + y;
							for (int x=0; x < 8; x++)
							{
								int cx = ch_x*8 + x;
								if (liquidflags[y][x] != 0x0F)
								{
									Cell->liquid_show[cy][cx] = true;
									if (liquidflags[y][x]&(1<<7))
										Cell->LiquidInfo[ch_x*16+ch_y] |= 0x10;
									++count;
								}
							}
						}

						if(chunkflags & 0x04)
							Cell->LiquidInfo[ch_x*16+ch_y]	|= 0x01;		// water
						if(chunkflags & 0x08)
							Cell->LiquidInfo[ch_x*16+ch_y]	|= 0x02;		// ocean
						if(chunkflags & 0x10)
							Cell->LiquidInfo[ch_x*16+ch_y]	|= 0x04;		// magma/slime

						if (!count && Cell->LiquidInfo[ch_x*16+ch_y])
							printf("Wrong liquid detect in MCLQ chunk");

						for (int y = 0; y <= 8; y++)
						{
							int cy = ch_y*8 + y;
							for (int x = 0; x<= 8; x++)
							{
								int cx = ch_x*8 + x;
								Cell->liquid_height[cy][cx] = liquid[y][x].height;
							}
						}
					}
				}
				else if(ReadMLQ)
				{
					result |= 0x02;
				}
			}
		}
		else if(!strcmp(fourcc, "MCSE")) //MCSE
		{

		}
		else
		{
			if(expansion == 2)
			{
				if(fourcc2 == 0xefb88b70) // Fix for ICC map
					nextpos = mf.getPos()+0x1199;
			}
		}

		mf.seek(nextpos);
	}

	return result;
}

inline void LoadH2OChunk(MPQFile & mf, uint32 ch_x, uint32 ch_y, mcell *Cell, uint32 base)
{
	MH2Oinformation header;
	mf.read(&header, sizeof(MH2Oinformation));

	uint64 mask;
	if(header.offsData2a)
	{
		mf.seek(base+header.offsData2a);
		mf.read(&mask, sizeof(uint64));
	}
	else mask = 0xFFFFFFFFFFFFFFFFLL;

	int count = 0;
	for (int y = 0; y < header.Height;y++)
	{
		int cy = ch_x*8 + y + header.yOffset;
		for (int x = 0; x < header.Width; x++)
		{
			int cx = ch_y*8 + x + header.xOffset;
			if (mask& 1)
			{
				Cell->liquid_show[cy][cx] = true;
				++count;
			}
			mask>>=1;
		}
	}

	bool UseHeight = false;
	if(!(header.flags & 0x02) && header.offsData2b)
	{
		mf.seek(base+header.offsData2b);
		UseHeight = true;
	}

	int pos = 0;
	for (int y = 0; y <= header.Height; y++)
	{
		int cy = ch_x*8 + y + header.yOffset;
		for (int x = 0; x <= header.Width; x++)
		{
			int cx = ch_y*8 + x + header.xOffset;
			if (UseHeight)
				mf.read(&Cell->liquid_height[cy][cx], sizeof(float));
			else
				Cell->liquid_height[cy][cx] = header.levels[0];
			pos++;
		}
	}

	uint32 type = LiqType[header.type];
	switch(type)
	{
	case 0:
		{
			Cell->LiquidInfo[ch_x*16+ch_y] |= 0x01;
		}break;
	case 1:
		{
			Cell->LiquidInfo[ch_x*16+ch_y] |= 0x02;
			if(header.flags & 0x01 || !header.offsData2b)
				Cell->LiquidInfo[ch_x*16+ch_y] |= 0x10;
		}break;
	case 2:
		{
			Cell->LiquidInfo[ch_x*16+ch_y] |= 0x04;
		}break;
	case 3:
		{
			Cell->LiquidInfo[ch_x*16+ch_y] |= 0x08;
		}break;
	default:
		{
			printf("Unknown H20 Type: %u\n", header.type);
		}break;
	}
	if(header.flags & 0x02)
		Cell->LiquidInfo[ch_x*16+ch_y] |= 0x20;

	if (!count && Cell->LiquidInfo[ch_x*16+ch_y])
		printf("Wrong liquid detect in MH2O chunk\n");
	return;
}

bool LoadADT(char* filename)
{
	size_t size = 0;
	MPQFile mf(filename);
	if(mf.isEof())
		return false;

	mcells = new mcell;
	memset(&mcells->AreaInfo, 0, sizeof(uint16)*256);
	memset(&mcells->LiquidInfo, 0, sizeof(uint16)*256);
	memset(&mcells->LiquidMasks, 0, sizeof(uint64)*256);

	memset(&mcells->V8, 0, sizeof(float)*128*128);
	memset(&mcells->V9, 0, sizeof(float)*129*129);

	memset(&mcells->liquid_show, 0, sizeof(bool)*128*128);
	memset(&mcells->liquid_height, 0, sizeof(float)*129*129);

	size_t mcnk_offsets[256], mcnk_sizes[256];
	MH2Oheader mh2oheader[256];
	memset(&mh2oheader,0,sizeof(MH2Oheader)*256);
	uint32 mh2o_base = 0;

	bool found = false;
	while (!mf.isEof())
	{
		char fourcc[5];
		mf.read(&fourcc, 4);
		mf.read(&size, 4);
		flipcc(fourcc);
		fourcc[4] = 0;

		size_t nextpos = mf.getPos() + size;
		if (!strcmp(fourcc,"MCIN"))
		{
			for (int i = 0; i < 256; i++)
			{
				mf.read(&mcnk_offsets[i], 4);
				mf.read(&mcnk_sizes[i], 4);
				mf.seekRelative(8);
			}
		}
		else if (!strcmp(fourcc,"MH2O"))
		{
			if (size && !mh2o_base)
			{
				mh2o_base = mf.getPos();
				for (int i = 0; i < 256; i++)
					mf.read(&mh2oheader[i], sizeof(MH2Oheader));
			}
		}
		else if (!strcmp(fourcc,"MVER"))
		{ }
		else if (!strcmp(fourcc,"MHDR"))
		{ }
		else if (!strcmp(fourcc,"MTEX"))
		{ }
		else if (!strcmp(fourcc,"MWID"))
		{ }
		else if (!strcmp(fourcc,"MMID"))
		{ }
		else if (!strcmp(fourcc,"MTEX"))
		{ }
		else if (!strcmp(fourcc,"MTXF"))
		{ }
		else if (!strcmp(fourcc,"MMDX"))
		{ }
		else if (!strcmp(fourcc,"MFBO"))
		{ }
		else if (!strcmp(fourcc,"MDDF"))
		{ }
		else if (!strcmp(fourcc,"MODF"))
		{ }
		else if (!strcmp(fourcc,"MWMO"))
		{ }

		mf.seek(nextpos);
	}

	uint32 pos = 0;
	for (int j=0; j<16; j++)
	{
		for (int i=0; i<16; i++)
		{
			mf.seek((int)mcnk_offsets[j*16+i]);
			LoadMapChunk(mf, j, i, mcells, !mh2o_base);

			if(mh2o_base)
			{
				if (mh2oheader[j*16+i].layerCount > 0)
				{
					mf.seek((int)mh2oheader[j*16+i].ofsInformation+mh2o_base);
					LoadH2OChunk(mf, j, i, mcells, mh2o_base);
				}
			}
		}
	}

	mf.close();
	return true;
}

bool StoreADTData(uint32 tile_x, uint32 tile_y, char* name)
{
	// For some odd reason, this stuff is reversed.. who knows why..
	char mpq_filename[256];
	sprintf_s(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", name, name, tile_y, tile_x);
	if(!LoadADT(mpq_filename))
		return false;

	mcell_cache[tile_x][tile_y] = mcells;
	return true;
}

//#define DEBUG_OUTPUT
bool ConvertADT(uint32 tile_x, uint32 tile_y, FILE * out_file, char* name)
{
	// See if we have it cached first.
	if(mcell_cache[tile_x][tile_y] == 0)
		return false;

	fwrite(&mcell_cache[tile_x][tile_y]->AreaInfo, sizeof(uint16), 256, out_file);
	fwrite(&mcell_cache[tile_x][tile_y]->LiquidInfo, sizeof(uint16), 256, out_file);
	fwrite(&mcell_cache[tile_x][tile_y]->V8, sizeof(float), 128*128, out_file);
	fwrite(&mcell_cache[tile_x][tile_y]->V9, sizeof(float), 129*129, out_file);
	fwrite(&mcell_cache[tile_x][tile_y]->liquid_height, sizeof(float), 129*129, out_file);
	return true;
}
