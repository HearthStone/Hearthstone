#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <vector>
#include <deque>
#include <set>
#include "dbcfile.h"
#include "adt.h"
#include "mpq_libmpq.h"
#include <io.h>
#include <fcntl.h>
#include <string>
#include <map>
#include <Windows.h>
#include <mmsystem.h>
#include <cstdlib>

using namespace std;

#ifdef WIN32
#include "direct.h"
#else
#include <sys/stat.h>
#endif

#if defined( __GNUC__ )
	#define _open   open
	#define _close close
	#ifndef O_BINARY
		#define O_BINARY 0
	#endif
#else
	#include <io.h>
#endif

#ifdef O_LARGEFILE
	#define OPEN_FLAGS  (O_RDONLY | O_BINARY | O_LARGEFILE)
#else
	#define OPEN_FLAGS (O_RDONLY | O_BINARY)
#endif

extern unsigned int iRes;
bool StoreADTData(uint32 tile_x, uint32 tile_y, char* name);
bool ConvertADT(uint32 x, uint32 y, FILE * out_file, char* name);
void reset();
void CleanCache();

typedef struct{
	char name[64];
	unsigned int id;
}map_id;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
map_id * map_ids;

uint32 MapCount;
extern uint16* LiqType;

void ReadLiquidTypeTableDBC(int expansion)
{
	switch(expansion)
	{
	case 0:
		{
			printf("Setting liquid types...");
			LiqType = new uint16[21];
			memset(LiqType, 0xff, (21)*sizeof(uint16));
			LiqType[1] = 0;
			LiqType[2] = 1;
			LiqType[3] = 2;
			LiqType[4] = 3;
			LiqType[21] = 3;
			printf("Done! (5 LiqTypes set)\n");
		}break;
	case 1:
		{
			printf("Setting liquid types...");
			LiqType = new uint16[62];
			memset(LiqType, 0xff, (62)*sizeof(uint16));
			LiqType[1] = 0;
			LiqType[2] = 1;
			LiqType[3] = 2;
			LiqType[4] = 3;
			LiqType[21] = 3;
			LiqType[41] = 0;
			LiqType[61] = 0;
			printf("Done! (7 LiqTypes set)\n");
		}break;
	default:
		{
			printf("Read LiquidType.dbc file...");
			DBCFile* dbc = new DBCFile("DBFilesClient\\LiquidType.dbc");
			dbc->open();

			size_t LiqType_count = dbc->getRecordCount();
			size_t LiqType_maxid = dbc->getMaxId();
			LiqType = new uint16[LiqType_maxid + 1];
			memset(LiqType, 0xff, (LiqType_maxid + 1) * sizeof(uint16));

			for(uint32 x = 0; x < LiqType_count; ++x)
				LiqType[dbc->getRecord(x).getUInt(0)] = dbc->getRecord(x).getUInt(3);

			printf("Done! (%u LiqTypes loaded)\n", LiqType_count);
		}break;
	}
}

void ExtractMapsFromMpq()
{
	printf("\nProcessing %u maps...\n\n", MapCount);
	for(uint32 i = 0; i < MapCount; ++i)
	{
		map_id *map = &map_ids[i];
		printf("Converting maps for mapid %u [%s]...\n", map->id, map->name);
		// Create the container file
		char output_filename[50];
		sprintf(output_filename, "maps\\Map_%u.bin", map->id);
		printf("  Creating output file %s.\n", output_filename);
		FILE * out_file = fopen(output_filename, "wb");
		if(!out_file)
		{
			printf("  Could not create output file!\n");
			return;
		}
		printf("  Checking which tiles are extractable...\n");

		char mpq_filename[128];
		bool Available_Maps[64][64];
		uint32 Offsets[64][64], TotalTiles = 0, AvailableTiles = 0;
		memset(Offsets, 0, sizeof(Offsets));
		// First, check the number of present tiles.
		for(uint32 x = 0; x < 64; x++)
		{
			for(uint32 y = 0; y < 64; y++)
			{
				// set up the mpq filename
				sprintf(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", map->name, map->name, y, x);

				// check if the file exists
				if(!mpq_file_exists(mpq_filename))
				{
					// file does not exist
					Available_Maps[x][y] = false;
				}
				else
				{
					// file does exist
					Available_Maps[x][y] = true;
					++AvailableTiles;
				}
				++TotalTiles;
			}
		}


		// Calculate the estimated size.
		float Estimated_Size = 16384.0f;
		uint32 TilesToExtract = 0;
		for(uint32 x = 0; x < 64; ++x)
		{
			for(uint32 y = 0; y < 64; ++y)
			{
				Offsets[x][y] = 0;
				if(Available_Maps[x][y] == true)
					++TilesToExtract;
			}
		}

		// Write the offsets to file
		printf("  Writing empty index to the beginning of the file...\n");
		fwrite(Offsets, sizeof(Offsets), 1, out_file);

		Estimated_Size += 218128.0f * TilesToExtract;
		Estimated_Size /= 1024.0f;
		Estimated_Size /= 1024.0f;
		if(Estimated_Size == 0.015625f)
		{
			CleanCache();
			fclose(out_file);
			printf("  Skipping extraction for map %s no useable information available only the header would exist in the file.\n\n", map->name);
			continue;
		}

		printf("  %u of %u tiles are available. Estimated file size will be %.4fMB.\n", AvailableTiles, TotalTiles, Estimated_Size);
		printf("  %u passes in total have to be performed, it may take a while.\n", TilesToExtract);
		printf("  Extracting data(This takes 10 blocks)... ");
		uint32 start_time = timeGetTime();
		reset();

		uint32 AvailableCells = 0;
 		for(uint32 x = 0; x < 64; x++)
 			for(uint32 y = 0; y < 64; y++)
				if(Available_Maps[x][y])
					AvailableCells++;

		float ID = 0;
		uint32 count = 0;
 		for(uint32 x = 0; x < 64; x++)
 		{
 			for(uint32 y = 0; y < 64; y++)
 			{
 				if(Available_Maps[x][y])
 				{
					StoreADTData(x, y, map->name);
					if(count == AvailableCells/10*2 || count == AvailableCells/10*4 || count == AvailableCells/10*6 || count == AvailableCells/10*8 || count == AvailableCells/10*10)
						printf("##");
					count++;
 				}
 			}
 		}
		printf("\n");

		fseek(out_file, 0, SEEK_END);

		// call the extraction functions.
		for(uint32 x = 0; x < 64; x++)
		{
			for(uint32 y = 0; y < 64; y++)
			{
				uint32 Offset = ftell(out_file);
				if(ConvertADT(x, y, out_file, map->name))
					Offsets[x][y] = Offset;
			}
		}
		CleanCache();

		printf("  Data Extraction Finished in %ums. Appending header to start of file...\n", timeGetTime() - start_time);
		fseek(out_file, 0, SEEK_SET);
		fwrite(Offsets, sizeof(Offsets), 1, out_file);
		printf("  Closing output file.\n");
		fclose(out_file);

		printf("  Conversion of map %u completed\n\n", map->id);
	}

}

void CreateDir( const std::string& Path )
{
	#ifdef WIN32
	_mkdir( Path.c_str());
	#else
	mkdir( Path.c_str(), 0777 );
	#endif
}

char output_path[128] = ".";
static char* const langs[] = {"enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU" };
extern ArchiveSet gOpenArchives;

bool ExtractFile( char const* mpq_name, std::string const& filename )
{
	FILE *output = fopen(filename.c_str(), "wb");
	if(!output)
	{
		printf("Can't create the output file '%s'\n", filename.c_str());
		return false;
	}

	MPQFile m(mpq_name);
	if(!m.isEof())
		fwrite(m.getPointer(), 1, m.getSize(), output);

	fclose(output);
	return true;
}

void ExtractDBCFiles(int locale)
{
	printf("Extracting dbc files...\n");

	set<string> dbcfiles;

	// get DBC file list
	for(ArchiveSet::iterator i = gOpenArchives.begin(); i != gOpenArchives.end();++i)
	{
		vector<string> files;
		(*i)->GetFileListTo(files);
		for (vector<string>::iterator iter = files.begin(); iter != files.end(); ++iter)
			if (iter->rfind(".dbc") == iter->length() - strlen(".dbc"))
					dbcfiles.insert(*iter);
	}

	string path = output_path;
	path += "/dbc/";
	CreateDir(path);

	// extract DBCs
	int count = 0;
	for (set<string>::iterator iter = dbcfiles.begin(); iter != dbcfiles.end(); ++iter)
	{
		string filename = path;
		filename += (iter->c_str() + strlen("DBFilesClient\\"));

		if(ExtractFile(iter->c_str(), filename))
			++count;
	}
	printf("Extracted %u DBC files\n\n", count);
}

struct ModelCache
{
	M2Header* header;
	M2Header_Old* old_Header;
	M2Attachment* attachments;
	M2Bone* bones;
	uint16* bonelookups;
};

void replace(std::string &str, const char* find, const char* rep, uint32 limit)
{
	uint32 i=0;
	std::string::size_type pos=0;
	while((pos = str.find(find, pos)) != std::string::npos)
	{
		str.erase(pos, strlen(find));
		str.insert(pos, rep);
		pos += strlen(rep);

		++i;
		if (limit != 0 && i == limit)
			break;
	}
}

struct DisplayBounding
{
	uint32 Entry; // Display ID
	float Low[3];
	float High[3];
	float BoundRadius;
};

void CreateCustomDBCFiles(int expansion)
{
	printf("Creating required custom dbc files...\n");

	map<uint32, DisplayBounding*> m_DisplayMap;
	std::map<std::string, ModelCache> modelCache;
	std::map<uint32, DBCFile::Record> modelInfoEntries;

	//map.dbc
	DBCFile displayInfo("DBFilesClient\\CreatureDisplayInfo.dbc");
	DBCFile modelInfo("DBFilesClient\\CreatureModelData.dbc");
	displayInfo.open();
	modelInfo.open();

	string path = output_path;
	path += "/dbc/";
	string filename = path + string("CreatureBoundInformation.dbc");
	FILE* newDBC = fopen(filename.c_str(), "wb");
	if(newDBC != NULL)
	{
		char header[4];
		unsigned int na = 0, nb = 8, es = sizeof(DisplayBounding), ss = 0;
		header[0] = 'W'; header[1] = 'D';
		header[2] = 'B'; header[3] = 'C';
		fwrite(header, 4, 1, newDBC);

		for (DBCFile::Iterator itr = modelInfo.begin(); itr != modelInfo.end(); ++itr)
		{
			unsigned int entry = itr->getInt(0);
			modelInfoEntries.insert(std::make_pair(entry, *itr));
		}

		for (DBCFile::Iterator itr = displayInfo.begin(); itr != displayInfo.end(); ++itr)
		{
			unsigned int displayid = itr->getInt(0);
			unsigned int modelentry = itr->getInt(1);
			float modelscale = itr->getFloat(4);

			std::map<uint32, DBCFile::Record>::iterator  modelitr = modelInfoEntries.find(modelentry);
			if (modelitr == modelInfoEntries.end())
			{
#ifdef _DEBUG
				printf("Cannot find model entry for display %u (entry %u)\n", displayid, modelentry);
#endif
				continue;
			}

			DisplayBounding* BoundingInfo = new DisplayBounding();
			DBCFile::Record modelrec = modelitr->second;

			const char* modelname = modelrec.getString(2);

			std::string strmodelname(modelname);

			replace(strmodelname, ".mdx", ".m2", 0);
			replace(strmodelname, ".MDX", ".m2", 0);

			if(expansion == 0 || expansion == 1)
			{
				M2Header_Old* header;
				M2Attachment* attachments;
				M2Bone* bones;
				uint16* bonelookups;

				std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);
				if (cacheitr == modelCache.end())
				{
					MPQFile modelf(strmodelname.c_str());
					if (modelf.isEof())
					{
						delete BoundingInfo;
#ifdef _DEBUG
						printf("Error: cannot open %s\n", strmodelname.c_str());
#endif
						continue;
					}

					header = (M2Header_Old*)malloc(sizeof(M2Header_Old));
					modelf.read(header, sizeof(M2Header_Old));

					attachments = (M2Attachment*)malloc(header->nAttachments * sizeof(M2Attachment));
					modelf.seek(header->ofsAttachments);
					modelf.read(attachments, header->nAttachments * sizeof(M2Attachment));

					bonelookups = (uint16*)malloc(header->nBoneLookupTable * sizeof(uint16));
					modelf.seek(header->ofsBoneLookupTable);
					modelf.read(bonelookups, header->nBoneLookupTable * sizeof(uint16));

					bones = (M2Bone*)malloc(header->nBones * sizeof(M2Bone));
					modelf.seek(header->ofsBones);
					modelf.read(bones, header->nBones * sizeof(M2Bone));

					ModelCache cacheentry;
					cacheentry.attachments = attachments;
					cacheentry.bones = bones;
					cacheentry.bonelookups = bonelookups;
					cacheentry.old_Header = header;
					modelCache.insert(std::make_pair(modelname, cacheentry));
				}
				else
				{
					header = cacheitr->second.old_Header;
					bones = cacheitr->second.bones;
					bonelookups = cacheitr->second.bonelookups;
					attachments = cacheitr->second.attachments;
				}

#ifdef GET_BONE_DATA
				// try and get the bone
				for (uint32 i = 0; i < header->nAttachments; ++i)
				{
					if (attachments[i].bone > header->nBoneLookupTable)
					{
#ifdef _DEBUG
						printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header->nBoneLookupTable);
#endif
						continue;
					}

					uint16 boneindex = bonelookups[attachments[i].bone];
					if (boneindex > header->nBones)
					{
#ifdef _DEBUG
						printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header->nBones);
#endif
						continue;
					}
					M2Bone & bone = bones[boneindex];
					//printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

					float realpos[3];
					realpos[0] = (/*bone.pivotpoint[0] +*/ attachments[i].pos[0]) * modelscale;
					realpos[1] = (/*bone.pivotpoint[1] +*/ attachments[i].pos[1]) * modelscale;
					realpos[2] = (/*bone.pivotpoint[2] +*/ attachments[i].pos[2]) * modelscale;

					//fix coord system
// 					float tmp = realpos[2];
// 					realpos[2] = realpos[1];
// 					realpos[1] = -tmp;
					//fprintf(fo, "insert into `display_attachment_points` VALUES (%u, %u, %f, %f, %f);\n", displayid, attachments[i].id, attachments[i].pos[0], attachments[i].pos[1], attachments[i].pos[2]);
					//printf("Attachmnent %u point %f %f %f pivot %f %f %f\n", attachments[i].id, realpos[0], realpos[1], realpos[2], bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2]);
				}
#endif

				BoundingInfo->Entry = displayid;
				BoundingInfo->Low[0] = header->boundingbox1[0] * modelscale;
				BoundingInfo->Low[1] = header->boundingbox1[1] * modelscale;
				BoundingInfo->Low[2] = header->boundingbox1[2] * modelscale;
				BoundingInfo->High[0] = header->boundingbox2[0] * modelscale;
				BoundingInfo->High[1] = header->boundingbox2[1] * modelscale;
				BoundingInfo->High[2] = header->boundingbox2[2] * modelscale;
				BoundingInfo->BoundRadius = header->boundingradius * modelscale;
				m_DisplayMap.insert(make_pair(displayid, BoundingInfo));
				na++;
			}
			else if(expansion == 2)
			{
				M2Header* header;
				M2Attachment* attachments;
				M2Bone* bones;
				uint16* bonelookups;

				std::map<std::string, ModelCache>::iterator cacheitr = modelCache.find(modelname);
				if (cacheitr == modelCache.end())
				{
					MPQFile modelf(strmodelname.c_str());
					if (modelf.isEof())
					{
						delete BoundingInfo;
#ifdef _DEBUG
						printf("Error: cannot open %s\n", strmodelname.c_str());
#endif
						continue;
					}

					header = (M2Header*)malloc(sizeof(M2Header));
					modelf.read(header, sizeof(M2Header));

					attachments = (M2Attachment*)malloc(header->nAttachments * sizeof(M2Attachment));
					modelf.seek(header->ofsAttachments);
					modelf.read(attachments, header->nAttachments * sizeof(M2Attachment));

					bonelookups = (uint16*)malloc(header->nBoneLookupTable * sizeof(uint16));
					modelf.seek(header->ofsBoneLookupTable);
					modelf.read(bonelookups, header->nBoneLookupTable * sizeof(uint16));

					bones = (M2Bone*)malloc(header->nBones * sizeof(M2Bone));
					modelf.seek(header->ofsBones);
					modelf.read(bones, header->nBones * sizeof(M2Bone));

					ModelCache cacheentry;
					cacheentry.attachments = attachments;
					cacheentry.bones = bones;
					cacheentry.bonelookups = bonelookups;
					cacheentry.header = header;
					modelCache.insert(std::make_pair(modelname, cacheentry));
				}
				else
				{
					header = cacheitr->second.header;
					bones = cacheitr->second.bones;
					bonelookups = cacheitr->second.bonelookups;
					attachments = cacheitr->second.attachments;
				}

#ifdef GET_BONE_DATA
				// try and get the bone
				for (uint32 i = 0; i < header->nAttachments; ++i)
				{
					if (attachments[i].bone > header->nBoneLookupTable)
					{
#ifdef _DEBUG
						printf("Attachment %u requests bonelookup %u (too large, bonelookup table is only %u entries)\n", i, attachments[i].bone, header->nBoneLookupTable);
#endif
						continue;
					}

					uint16 boneindex = bonelookups[attachments[i].bone];
					if (boneindex > header->nBones)
					{
#ifdef _DEBUG
						printf("Attachment %u requests bone %u (too large, bone table is only %u entries)\n", i, boneindex, header->nBones);
#endif
						continue;
					}
					M2Bone & bone = bones[boneindex];
					//printf("Attachment %u (bone pivot %f %f %f offset %f %f %f)\n", attachments[i].id, bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2], attachments[i].pos[0],  attachments[i].pos[1],  attachments[i].pos[2]);

					float realpos[3];
					realpos[0] = (/*bone.pivotpoint[0] +*/ attachments[i].pos[0]) * modelscale;
					realpos[1] = (/*bone.pivotpoint[1] +*/ attachments[i].pos[1]) * modelscale;
					realpos[2] = (/*bone.pivotpoint[2] +*/ attachments[i].pos[2]) * modelscale;

					//fix coord system
// 					float tmp = realpos[2];
// 					realpos[2] = realpos[1];
// 					realpos[1] = -tmp;
					//fprintf(fo, "insert into `display_attachment_points` VALUES (%u, %u, %f, %f, %f);\n", displayid, attachments[i].id, attachments[i].pos[0], attachments[i].pos[1], attachments[i].pos[2]);
					//printf("Attachmnent %u point %f %f %f pivot %f %f %f\n", attachments[i].id, realpos[0], realpos[1], realpos[2], bone.pivotpoint[0], bone.pivotpoint[1], bone.pivotpoint[2]);
				}
#endif

				BoundingInfo->Entry = displayid;
				BoundingInfo->Low[0] = ((floor(1000000*(header->boundingbox1[0] * modelscale)))/1000000);
				BoundingInfo->Low[1] = ((floor(1000000*(header->boundingbox1[1] * modelscale)))/1000000);
				BoundingInfo->Low[2] = ((floor(1000000*(header->boundingbox1[2] * modelscale)))/1000000);
				BoundingInfo->High[0] = ((floor(1000000*(header->boundingbox2[0] * modelscale)))/1000000);
				BoundingInfo->High[1] = ((floor(1000000*(header->boundingbox2[1] * modelscale)))/1000000);
				BoundingInfo->High[2] = ((floor(1000000*(header->boundingbox2[2] * modelscale)))/1000000);
				BoundingInfo->BoundRadius = ((floor(1000000*(header->boundingradius * modelscale)))/1000000);
				m_DisplayMap.insert(make_pair(displayid, BoundingInfo));
				na++;
			}
		}

		printf("%u Creature Bound Information entries created.\n", na);
		fwrite(&na, 4, 1, newDBC);
		fwrite(&nb, 4, 1, newDBC);
		fwrite(&es, 4, 1, newDBC);
		fwrite(&ss, 4, 1, newDBC);
		for(map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(); itr != m_DisplayMap.end(); itr++)
			fwrite(((uint8*)(itr->second)), es, 1, newDBC);

		fclose(newDBC);
	}

	DisplayBounding* buff = NULL;
	for(map<uint32, DisplayBounding*>::iterator itr = m_DisplayMap.begin(), itr2; itr != m_DisplayMap.end();)
	{
		itr2 = itr++;
		buff = itr2->second;
		m_DisplayMap.erase(itr2);
		delete buff;
		buff = NULL;
	}

	newDBC = NULL;
}

int expansion = 0;

int main(int argc, char * arg[])
{
	printf("Sandshroud map extractor for versions 3.x.x\n");
	printf("============================================================\n\n");

	FILE * tf;
	const char* localeNames[] = { "enUS", "enGB", "deDE", "frFR", "koKR", "zhCN", "zhTW", "esES", "ruRU", 0 };
	int maxPatches = 3;
	int locale = -1;
	char tmp[100];

	tf = fopen("Data/base.MPQ", "r");
	if(tf)
	{
		expansion = 0;
		new MPQArchive("Data/base.MPQ");

		// DBC
		tf = fopen("Data/dbc.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/dbc.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/dbc.MPQ");

		// models
		tf = fopen("Data/model.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/model.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/model.MPQ");

		// textures
		tf = fopen("Data/texture.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/texture.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/texture.MPQ");

		// Terrain
		tf = fopen("Data/terrain.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/terrain.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/terrain.MPQ");

		// patch
		tf = fopen("Data/patch.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/patch.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/patch.MPQ");

		// patch-2
		tf = fopen("Data/patch-2.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/patch-2.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/patch-2.MPQ");

		// wmo
		tf = fopen("Data/wmo.MPQ", "r");
		if (!tf)
		{
			printf("Could not find Data/wmo.MPQ\n");
			return 1;
		}
		fclose(tf);
		new MPQArchive("Data/wmo.MPQ");
	}
	else
	{
		// 2.4.3 or 3.3.5
		tf = fopen("Data/common-2.MPQ", "r");
		if (!tf)
		{
			expansion = 1;
			// 2.4.3 MPQs
			for( size_t i = 0; localeNames[i] != 0; i++ )
			{
				sprintf(tmp, "Data/%s/locale-%s.MPQ", localeNames[i], localeNames[i]);
				tf = fopen(tmp, "r");
				if (tf == NULL)
					continue;
				fclose(tf);
				locale = i;
				new MPQArchive(tmp);
			}

			if(locale == -1)
			{
				printf("Could not find a locale\n");
				return 1;
			}

			tf = fopen("Data/common.MPQ", "r");
			if (!tf)
			{
				printf("Could not find Data/common.MPQ\n");
				return 1;
			}
			fclose(tf);
			new MPQArchive("Data/common.MPQ");

			tf = fopen("Data/expansion.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/expansion.MPQ");
			}

			tf = fopen("Data/patch.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/patch.MPQ");
			}

			tf = fopen("Data/patch-2.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/patch-2.MPQ");
			}

			sprintf(tmp, "Data/%s/patch-%s.MPQ", localeNames[locale], localeNames[locale]);
			tf = fopen(tmp, "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive(tmp);
			}

			sprintf(tmp, "Data/%s/patch-%s-2.MPQ", localeNames[locale], localeNames[locale]);
			tf = fopen(tmp, "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive(tmp);
			}
		}
		else
		{
			expansion = 2;
			// 3.3.5 MPQs
			fclose(tf);
			new MPQArchive("Data/common-2.MPQ");

			tf = fopen("Data/common.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/common.MPQ");
			}

			for( size_t i = 0; localeNames[i] != 0; i++ )
			{
				sprintf(tmp, "Data/%s/locale-%s.MPQ", localeNames[i], localeNames[i]);
				tf = fopen(tmp, "r");
				if (!tf)
					continue;
				fclose(tf);
				locale = i;
				new MPQArchive(tmp);
			}

			if(locale == -1)
			{
				printf("Could not find a locale\n");
				return 1;
			}

			tf = fopen("Data/expansion.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/expansion.MPQ");
				if ( -1 != locale )
				{
					sprintf(tmp, "Data/%s/expansion-locale-%s.MPQ", localeNames[locale], localeNames[locale]);
					new MPQArchive(tmp);
				}
			}

			tf = fopen("Data/lichking.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/lichking.MPQ");
				if ( -1 != locale )
				{
					sprintf(tmp, "Data/%s/lichking-locale-%s.MPQ", localeNames[locale], localeNames[locale]);
					new MPQArchive(tmp);
				}
			}

			tf = fopen("Data/patch.MPQ", "r");
			if (tf)
			{
				fclose(tf);
				new MPQArchive("Data/patch.MPQ");
				for(int i = 2; i <= maxPatches; i++)
				{
					sprintf(tmp, "Data/patch-%d.MPQ", i);
					tf = fopen(tmp, "r");
					if (!tf)
						continue;
					fclose(tf);
					new MPQArchive(tmp);
				}

				sprintf(tmp, "Data/%s/patch-%s.MPQ", localeNames[locale], localeNames[locale]);
				tf = fopen(tmp, "r");
				if (tf)
				{
					fclose(tf);
					new MPQArchive(tmp);
					for(int i = 2; i <= maxPatches; i++)
					{
						sprintf(tmp, "Data/%s/patch-%s-%d.MPQ", localeNames[locale], localeNames[locale], i);
						tf = fopen(tmp, "r");
						if (!tf)
							continue;
						fclose(tf);
						new MPQArchive(tmp);
					}
				}
			}
		}
	}

	ExtractDBCFiles(locale);
	ReadLiquidTypeTableDBC(expansion);
	CreateCustomDBCFiles(expansion);

	//map.dbc
	DBCFile* dbc = new DBCFile("DBFilesClient\\Map.dbc");
	dbc->open();

	MapCount = dbc->getRecordCount();
	map_ids = new map_id[MapCount];
	for(unsigned int x = 0; x < MapCount; x++)
	{
		map_ids[x].id = dbc->getRecord(x).getUInt(0);
		strcpy(map_ids[x].name, dbc->getRecord(x).getString(1));
	}
	delete dbc;

	CreateDirectory("maps", NULL);
	ExtractMapsFromMpq();
	delete [] map_ids;
	printf("Map conversion complete, press enter to exit:");
	WaitForInput();
	return 0; // Exit The Program
}
