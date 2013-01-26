/***
 * Demonstrike Core
 */

#pragma once

#ifdef __DragonFly__
#include <pthread.h>
#endif
#ifdef _WIN32_WINNT
#include <Windows.h>
#endif

class SERVER_DECL Mutex
{
public:
	friend class Condition;

	/** Initializes a mutex class, with InitializeCriticalSection / pthread_mutex_init
	 */
	Mutex();

	/** Deletes the associated critical section / mutex
	 */
	~Mutex();

	/** Acquires this mutex. If it cannot be acquired immediately, it will block.
	 */
	HEARTHSTONE_INLINE void Acquire()
	{
#if PLATFORM != PLATFORM_WIN
		pthread_mutex_lock(&mutex);
#else
		EnterCriticalSection(&cs);
#endif
	}

	/** Releases this mutex. No error checking performed
	 */
	HEARTHSTONE_INLINE void Release()
	{
#if PLATFORM != PLATFORM_WIN
		pthread_mutex_unlock(&mutex);
#else
		LeaveCriticalSection(&cs);
#endif
	}

	/** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
	 * it will return false.
	 * @return false if cannot be acquired, true if it was acquired.
	 */
	HEARTHSTONE_INLINE bool AttemptAcquire()
	{
#if PLATFORM != PLATFORM_WIN
		return (pthread_mutex_trylock(&mutex) == 0);
#else
		return (TryEnterCriticalSection(&cs) == TRUE ? true : false);
#endif
	}

protected:
#if PLATFORM == PLATFORM_WIN
	/** Critical section used for system calls
	 */
	CRITICAL_SECTION cs;

#else
	/** Static mutex attribute
	 */
	static bool attr_initalized;
	static pthread_mutexattr_t attr;

	/** pthread struct used in system calls
	 */
	pthread_mutex_t mutex;
#endif
};

#if PLATFORM == PLATFORM_WIN

class SERVER_DECL FastMutex
{
#pragma pack(push,8)
	volatile long m_lock;
#pragma pack(pop)
	DWORD m_recursiveCount;

public:
	HEARTHSTONE_INLINE FastMutex() : m_lock(0),m_recursiveCount(0) {}
	HEARTHSTONE_INLINE ~FastMutex() {}

	HEARTHSTONE_INLINE void Acquire()
	{
		DWORD thread_id = GetCurrentThreadId(), owner;
		if(thread_id == (DWORD)m_lock)
		{
			++m_recursiveCount;
			return;
		}

		for(;;)
		{
			owner = InterlockedCompareExchange(&m_lock, thread_id, 0);
			if(owner == 0)
				break;

			if(!SwitchToThread())
				printf("REPORT TO DEVS: Thread %u entered wait state at(no next thread found), possible lockup! m_recursiveCount = %u \n", thread_id, m_recursiveCount);
		}

		++m_recursiveCount;
	}

	HEARTHSTONE_INLINE bool AttemptAcquire()
	{
		DWORD thread_id = GetCurrentThreadId();
		if(thread_id == (DWORD)m_lock)
		{
			++m_recursiveCount;
			return true;
		}

		DWORD owner = InterlockedCompareExchange(&m_lock, thread_id, 0);
		if(owner == 0)
		{
			++m_recursiveCount;
			return true;
		}

		return false;
	}

	HEARTHSTONE_INLINE void Release()
	{
		if((--m_recursiveCount) == 0)
			InterlockedExchange(&m_lock, 0);
	}
};

#else

#define FastMutex Mutex

#endif
