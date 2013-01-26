/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "Singleton.h"

class WorldPacket;
class WorldSession;
class Database;

#if PLATFORM == PLATFORM_WIN

#define TRED FOREGROUND_RED | FOREGROUND_INTENSITY
#define TGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#define TWHITE TNORMAL | FOREGROUND_INTENSITY
#define TBLUE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TPURPLE FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY

#else

#define TRED 1
#define TGREEN 2
#define TYELLOW 3
#define TNORMAL 4
#define TWHITE 5
#define TBLUE 6
#define TPURPLE 7

#endif
std::string FormatOutputString(const char * Prefix, const char * Description, bool useTimeStamp);

// log define
#define OUT_DEBUG sLog.outDebug
#define DEBUG_LOG Log.Debug
#define DETAIL_LOG sLog.outDetail

class SERVER_DECL oLog : public Singleton< oLog > {
public:
	void outString( const char * str, ... );
	void outError( const char * err, ... );
	void outDetail( const char * str, ... );
	void outDebug( const char * str, ... );
	void outDebugInLine( const char * str, ... );
	void outSpellDebug( const char * str, ... );

	void fLogText(const char *text);
	void SetLogging(bool enabled);

	void Init(int32 screenLogLevel);
	void SetScreenLoggingLevel(int32 level);
	void outColor(uint32 colorcode, const char * str, ...);
	bool IsOutDevelopement() const { return m_screenLogLevel == 4 || m_screenLogLevel == 6; }
	bool IsOutProccess() const { return m_screenLogLevel == 5 || m_screenLogLevel == 6; }

#if PLATFORM == PLATFORM_WIN
	HANDLE stdout_handle, stderr_handle;
#endif

	int32 m_screenLogLevel;
};

#define sLog oLog::getSingleton()
