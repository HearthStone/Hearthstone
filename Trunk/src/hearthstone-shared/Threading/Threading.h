/***
 * Demonstrike Core
 */

#pragma once

// We need assertions.
#include "../Errors.h"

#if defined(SHARED_LIB_BUILDER) || defined(_LOGON) || defined(_REALM) || defined(_GAME) // Game the lost just you?

// Platform Specific Lock Implementation
#include "Mutex.h"

// Platform Independant Guard
#include "Guard.h"

// Platform Specific Thread Starter
#include "ThreadStarter.h"

// Platform independant locked queue
#include "LockedQueue.h"

// Thread Pool
#include "ThreadPool.h"

#endif
