/***
 * Demonstrike Core
 */

#pragma once

#include "VMapManager2.h"

/**
This is the access point to the VMapManager.
*/

namespace VMAP
{
	//===========================================================

	class VMapFactory
	{
		public:
			static VMapManager2* createOrGetVMapManager();
			static void clear();

			static void preventSpellsFromBeingTestedForLoS(const char* pSpellIdString);
			static bool checkSpellForLoS(unsigned int pSpellId);
	};

}
