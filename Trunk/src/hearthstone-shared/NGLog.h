/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "Singleton.h"

class WorldPacket;
class WorldSession;

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

extern SERVER_DECL time_t UNIXTIME;		/* update this every loop to avoid the time() syscall! */
extern SERVER_DECL tm g_localTime;
#define LOG_USE_MUTEX

class SERVER_DECL CLog : public Singleton< CLog >
{
#ifdef LOG_USE_MUTEX
	Mutex mutex;
#define LOCK_LOG mutex.Acquire()
#define UNLOCK_LOG mutex.Release();
#else
#define LOCK_LOG
#define UNLOCK_LOG
#endif

public:
#if PLATFORM == PLATFORM_WIN
	HANDLE stdout_handle, stderr_handle;
#endif
	int32 log_level;

	CLog()
	{
		log_level = 3;
#if PLATFORM == PLATFORM_WIN
		stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
		stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	}

	void Color(unsigned int color)
	{
#if PLATFORM != PLATFORM_WIN
		static const char* colorstrings[TBLUE+1] = {
			"",
				"\033[22;31m",
				"\033[22;32m",
				"\033[01;33m",
				//"\033[22;37m",
				"\033[0m",
				"\033[01;37m",
				"\033[1;34m",
		};
		fputs(colorstrings[color], stdout);
#else
		SetConsoleTextAttribute(stdout_handle, (WORD)color);
#endif
	}

	HEARTHSTONE_INLINE void Time()
	{
		printf("%02u:%02u:%02u ", g_localTime.tm_hour, g_localTime.tm_min,g_localTime.tm_sec);
	}

	HEARTHSTONE_INLINE std::string GetTime()
	{
		return format("%02u:%02u:%02u ", g_localTime.tm_hour, g_localTime.tm_min,g_localTime.tm_sec);
	}

	void Notice(const char * source, const char * format, ...)
	{
		if(log_level < 0)
			return;

		/* notice is old loglevel 0/string */
		LOCK_LOG;
		va_list ap;
		va_start(ap, format);
		Time();
		fputs("N ", stdout);
		if(source != NULL && *source)
		{
			Color(TWHITE);
			fputs(source, stdout);
			putchar(':');
			putchar(' ');
			Color(TNORMAL);
		}

		Color(TNORMAL);
		vprintf(format, ap);
		putchar('\n');
		va_end(ap);
		Color(TNORMAL);
		UNLOCK_LOG;
	}

	void CNotice(int color, const char * source, const char * format, ...)
	{
		LOCK_LOG;
		va_list ap;
		va_start(ap, format);
		Time();
		fputs("N ", stdout);
		if(source != NULL && *source)
		{
			Color(TWHITE);
			fputs(source, stdout);
			putchar(':');
			putchar(' ');
			Color(TNORMAL);
		}

		Color(color);
		vprintf(format, ap);
		putchar('\n');
		va_end(ap);
		Color(TNORMAL);
		UNLOCK_LOG;
	}

	void Info(const char * source, const char * format, ...)
	{
		/* notice is old loglevel 0/string */
		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TPURPLE, source, msg0);
	}

	void Line()
	{
		LOCK_LOG;
		putchar('\n');
		UNLOCK_LOG;
	}

	void Error(const char * source, const char * format, ...)
	{
		if(log_level < 1)
			return;

		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TRED, source, msg0);
	}

	void Warning(const char * source, const char * format, ...)
	{
		if(log_level < 2)
			return;

		/* warning is old loglevel 2/detail */
		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TYELLOW, source, msg0);
	}

	void Success(const char * source, const char * format, ...)
	{
		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TGREEN, source, msg0);
	}

	void Debug(const char * source, const char * format, ...)
	{
		if(log_level != 3 && log_level != 6)
			return;

		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TBLUE, source, msg0);
	}

	void DebugSpell(const char * source, const char * format, ...)
	{
		if(log_level != 3 && log_level != 7)
			return;

		va_list ap;
		va_start(ap, format);
		char msg0[1024];
		vsnprintf(msg0, 1024, format, ap);
		va_end(ap);
		CNotice(TBLUE, source, msg0);
	}

#define LARGERRORMESSAGE_ERROR 1
#define LARGERRORMESSAGE_WARNING 2

	void LargeErrorMessage(uint32 Colour, ...)
	{
		std::vector<char*> lines;
		char * pointer;
		va_list ap;
		va_start(ap, Colour);

		size_t i,j,k;
		pointer = va_arg(ap, char*);
		while( pointer != NULL )
		{
			lines.push_back( pointer );
			pointer = va_arg(ap, char*);
		}

		LOCK_LOG;

		if( Colour == LARGERRORMESSAGE_ERROR )
			Color(TRED);
		else
			Color(TYELLOW);

		printf("*********************************************************************\n");
		printf("*                        MAJOR ERROR/WARNING                        *\n");
		printf("*                        ===================                        *\n");
		printf("*********************************************************************\n");
		printf("*                                                                   *\n");

		for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
		{
			i = strlen(*itr);
			j = (i<=65) ? 65 - i : 0;

			printf("* %s", *itr);
			for( k = 0; k < j; ++k )
			{
				printf(" ");
			}

			printf(" *\n");
		}

		printf("*********************************************************************\n");

#if PLATFORM == PLATFORM_WIN
		std::string str = "MAJOR ERROR/WARNING:\n";
		for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
		{
			str += *itr;
			str += "\n";
		}

		MessageBox(0, str.c_str(), "Error", MB_OK);
#else
		printf("Sleeping for 5 seconds.\n");
		usleep(5000*1000);
#endif

		Color(TNORMAL);
		UNLOCK_LOG;
	}
};

#define Log CLog::getSingleton()
