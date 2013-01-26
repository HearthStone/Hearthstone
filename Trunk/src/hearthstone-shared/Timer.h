/***
 * Demonstrike Core
 */

#pragma once

#if PLATFORM == PLATFORM_WIN
HEARTHSTONE_INLINE uint32 getMSTime() { return GetTickCount(); }
#else
HEARTHSTONE_INLINE uint32 getMSTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif
