/***
 * Demonstrike Core
 */

#pragma once

#include <cstring>

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
	const char VMAP_MAGIC[] = "VMAP_3.0";

	// defined in TileAssembler.cpp currently...
	bool readChunk(FILE *rf, char *dest, const char *compare, uint32 len);
}

#ifndef NO_CORE_FUNCS
	#include "Errors.h"
	#include "NGLog.h"
	#define ERROR_LOG(...) sLog.outError(__VA_ARGS__);
#elif defined MMAP_GENERATOR
	#include <assert.h>
	#define MANGOS_ASSERT(x) assert(x)
	#define DEBUG_LOG(...) 0
	#define DETAIL_LOG(...) 0
	#define ERROR_LOG(...) do{ printf("ERROR:"); printf(__VA_ARGS__); printf("\n"); } while(0)
#else
//	#include <assert.h>
//	#define ASSERT(x) assert(x)
	#define DEBUG_LOG(info, ...) do{ printf(info); printf(__VA_ARGS__); printf("\n"); } while(0)
	#define DETAIL_LOG(...) do{ printf(__VA_ARGS__); printf("\n"); } while(0)
	#define ERROR_LOG(...) do{ printf("ERROR:"); printf(__VA_ARGS__); printf("\n"); } while(0)
#endif
