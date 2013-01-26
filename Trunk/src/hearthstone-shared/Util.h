/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"

///////////////////////////////////////////////////////////////////////////////
// String Functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::vector<std::string> StrSplit(const std::string &src, const std::string &sep);

// This HAS to be called outside the threads __try / __except block!
void SetThreadName(const char* format);
time_t convTimePeriod ( uint32 dLength, char dType);

HEARTHSTONE_INLINE uint32 secsToTimeBitFields(time_t secs)
{
	tm* lt = localtime(&secs);
	return (lt->tm_year - 100) << 24 | lt->tm_mon  << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min;
}

#if PLATFORM == PLATFORM_WIN

	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // must be 0x1000
		LPCSTR szName; // pointer to name (in user addr space)
		DWORD dwThreadID; // thread ID (-1=caller thread)
		DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;

#endif
