/***
 * Demonstrike Core
 */

#include "vmap/VMapManager2.h"
#include "g3dlite/G3DAll.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "LocationVector.h"

VMAP::VMapManager2* vmgr;

void * collision_init()
{
	vmgr = new VMAP::VMapManager2();
	return vmgr;
}

void collision_shutdown()
{
	vmgr->unloadMap(0);
	vmgr->unloadMap(1);
	vmgr->unloadMap(530);
	vmgr->unloadMap(571);
}

