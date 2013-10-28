/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_PLATFORM_THREAD_H
#define ARX_PLATFORM_THREAD_H

#include "Configure.h"
#include "platform/Platform.h"

#include <string>

#if ARX_HAVE_PTHREADS
#include <pthread.h>
#include <sys/types.h>
typedef pthread_t thread_id_type;
typedef pid_t process_id_type;
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
typedef DWORD thread_id_type;
typedef DWORD process_id_type;
#else
#error "Threads not supported: need ARX_HAVE_PTHREADS on non-Windows systems"
#endif

class Thread {
	
private:
	
#if ARX_HAVE_PTHREADS
	
	pthread_t thread;
	int priority;
	bool started;
	
	static void * entryPoint(void * param);
	
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	
	HANDLE thread;

	static DWORD WINAPI entryPoint(LPVOID param);
	
#endif
	
	std::string threadName;
	
public:
	
	Thread();
	
	virtual ~Thread();
	
	enum Priority {
		Lowest   = -2,
		Low      = -1,
		Normal   = 0,
		High     = 1,
		Highest  = 2
	};
	
	/*!
	 * Start the thread.
	 */
	void start();
	
	/*!
	 * Set the thread name (for easier debugging)
	 * @note This should be called BEFORE starting the thread.
	 * @param threadName The thread name.
	 */
	void setThreadName(const std::string & threadName);

	void setPriority(Priority priority);
	
	/*!
	 * Suspend the current thread for a specific amount of time.
	 */
	static void sleep(unsigned milliseconds);
	
	/*!
	 * Exit the current thread.
	 */
	static void exit();
	
	/*!
	 * Wait until the thread exists.
	 */
	void waitForCompletion();
	
	static thread_id_type getCurrentThreadId();
	
protected:
	
	/*!
	 * The threads main entry point, to be implemented by subclasses.
	 */
	virtual void run() = 0;
};

class StoppableThread : public Thread {
	
private:
	
	bool stopRequested;
	
public:
	
	StoppableThread() : stopRequested(false) { }
	
	inline void stop(Priority priority = Highest) {
		stopRequested = true;
		setPriority(priority);
		waitForCompletion();
	}
	
	inline bool isStopRequested() {
		return stopRequested;
	}
	
};

process_id_type getProcessId();

#endif // ARX_PLATFORM_THREAD_H
