/***
 * Demonstrike Core
 */

#include "Common.h"
#include "Config/ConfigEnv.h"
#include "Log.h"
#include "NGLog.h"
#include <stdarg.h>

string FormatOutputString(const char * Prefix, const char * Description, bool useTimeStamp)
{

	char p[MAX_PATH];
	p[0] = 0;
	time_t t = time(NULL);
	tm * a = gmtime(&t);
	strcat(p, Prefix);
	strcat(p, "/");
	strcat(p, Description);
	if(useTimeStamp)
	{
		char ftime[100];
		snprintf(ftime, 100, "-%-4d-%02d-%02d %02d-%02d-%02d", a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour, a->tm_min, a->tm_sec);
		strcat(p, ftime);
	}

	strcat(p, ".log");
	return string(p);
}

createFileSingleton( oLog );
createFileSingleton(CLog);

SERVER_DECL time_t UNIXTIME;
SERVER_DECL tm g_localTime;

#if PLATFORM != PLATFORM_WIN
static const char* colorstrings[TBLUE+1] = {
"",
"\033[22;31m",
"\033[22;32m",
"\033[01;33m",
//"\033[22;37m",
"\033[0m",
"\033[01;37m",
"\033[22;34m",
};
#endif

void oLog::outString( const char * str, ... )
{
	va_list ap;
	char buf[32768];

	if(m_screenLogLevel < 0)
		return;

	va_start(ap, str);
	vsnprintf(buf, 32768, str, ap);
	va_end(ap);

	if(m_screenLogLevel >= 0)
	{
		printf(buf);
		putc('\n', stdout);
	}
}

void oLog::outError( const char * err, ... )
{
	va_list ap;
	char buf[32768];

	if(m_screenLogLevel < 1)
		return;

	va_start(ap, err);
	vsnprintf(buf, 32768, err, ap);
	va_end(ap);

	if(m_screenLogLevel >= 1)
	{
#if PLATFORM == PLATFORM_WIN
		SetConsoleTextAttribute(stderr_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
		puts(colorstrings[TRED]);
#endif
		fprintf(stderr, buf);
		putc('\n', stderr);
#if PLATFORM == PLATFORM_WIN
		SetConsoleTextAttribute(stderr_handle, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#else
		puts(colorstrings[TNORMAL]);
#endif
	}
}

void oLog::outDetail( const char * str, ... )
{
	va_list ap;
	char buf[32768];

	if(m_screenLogLevel < 2)
		return;

	va_start(ap, str);
	vsnprintf(buf, 32768, str, ap);
	va_end(ap);

	if(m_screenLogLevel >= 2)
	{
		printf(buf);
		putc('\n', stdout);
	}
}

void oLog::outDebug( const char * str, ... )
{
	va_list ap;
	char buf[32768];

	if(m_screenLogLevel < 3)
		return;

	va_start(ap, str);
	vsnprintf(buf, 32768, str, ap);
	va_end(ap);

	if(m_screenLogLevel == 3 || m_screenLogLevel == 6)
	{
		printf(buf);
		putc('\n', stdout);
	}
}

void oLog::outDebugInLine(const char * str, ...)
{
	if(!str)
		return;

	va_list ap;
	char buf[32768];

	va_start(ap, str);
	vsnprintf(buf, 32768, str, ap);
	va_end(ap);

	if(m_screenLogLevel == 5 || m_screenLogLevel == 6)
	{
		printf(buf);
	}
}

void oLog::outSpellDebug( const char * str, ... )
{
	va_list ap;
	char buf[32768];

	if(m_screenLogLevel != 3 && m_screenLogLevel != 7)
		return;

	va_start(ap, str);
	vsnprintf(buf, 32768, str, ap);
	va_end(ap);

	if(m_screenLogLevel == 3 || m_screenLogLevel == 7)
	{
		printf(buf);
		putc('\n', stdout);
	}
}

void oLog::Init(int32 screenLogLevel)
{
	m_screenLogLevel = screenLogLevel;
	Log.log_level = screenLogLevel;

	// get error handle
#if PLATFORM == PLATFORM_WIN
	stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void oLog::SetScreenLoggingLevel(int32 level)
{
	m_screenLogLevel = level;
	Log.log_level = level;
}

void oLog::outColor(uint32 colorcode, const char * str, ...)
{
	if( !str ) return;
	va_list ap;
	va_start(ap, str);
#if PLATFORM == PLATFORM_WIN
	SetConsoleTextAttribute(stdout_handle, colorcode);
#else
	printf(colorstrings[colorcode]);
#endif
	vprintf( str, ap );
	fflush(stdout);
	va_end(ap);
}
