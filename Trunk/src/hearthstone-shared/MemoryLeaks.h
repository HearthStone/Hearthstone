/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "Singleton.h"

struct MemoryManager : public Singleton < MemoryManager > {
	MemoryManager();
};
