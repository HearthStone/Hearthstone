/***
 * Demonstrike Core
 */

#pragma once

#include "../Common.h"
#include "Mutex.h"
#include "ThreadStarter.h"

struct SERVER_DECL Thread
{
	char* name;
	uint32 ThreadId;
	Thread(char* tname) { name = tname; }

	ThreadContext * ExecutionTarget;
};

typedef std::set<Thread*> ThreadSet;

class SERVER_DECL CThreadPool
{
	Mutex _mutex;

	ThreadSet m_activeThreads;

public:
	CThreadPool();

	// shutdown all threads
	void Shutdown();

	// return true - suspend ourselves, and wait for a future task.
	// return false - exit, we're shutting down or no longer needed.
	void ThreadExit(Thread * t);

	// creates a thread, returns a handle to it.
	Thread * StartThread(ThreadContext * ExecutionTarget);

	// grabs/spawns a thread, and tells it to execute a task.
	void ExecuteTask(const char* ThreadName, ThreadContext * ExecutionTarget);

	// gets active thread count
	HEARTHSTONE_INLINE uint32 GetActiveThreadCount() { return (uint32)m_activeThreads.size(); }
};

extern SERVER_DECL CThreadPool ThreadPool;
