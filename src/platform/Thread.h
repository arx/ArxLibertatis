/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
typedef DWORD thread_id_type;
#else
#error "Threads not supported: need ARX_HAVE_PTHREADS on non-Windows systems"
#endif

#include "core/TimeTypes.h"

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
	 * \brief Start the thread
	 */
	void start();
	
	/*!
	 * \brief Set the thread name (for easier debugging)
	 *
	 * \note This should be called BEFORE starting the thread.
	 *
	 * \param threadName The thread name.
	 */
	void setThreadName(const std::string & threadName);

	void setPriority(Priority priority);
	
	/*!
	 * \brief Suspend the current thread for a specific amount of time
	 */
	static void sleep(PlatformDuration time);
	
	/*!
	 * \brief Exit the current thread
	 */
	static void exit();
	
	/*!
	 * \brief Wait until the thread exists
	 */
	void waitForCompletion();
	
	static thread_id_type getCurrentThreadId();
	
	/*!
	 * \brief Disable denormals for the current thread for faster floating point operations
	 */
	static void disableFloatDenormals();
	
protected:
	
	/*!
	 * \brief The threads main entry point, to be implemented by subclasses
	 */
	virtual void run() = 0;
};

class StoppableThread : public Thread {
	
private:
	
	volatile bool m_stopRequested;
	
public:
	
	StoppableThread() : m_stopRequested(false) { }
	
	void stop(Priority priority = Highest) {
		m_stopRequested = true;
		setPriority(priority);
		waitForCompletion();
	}
	
	bool isStopRequested() {
		return m_stopRequested;
	}
	
};

#endif // ARX_PLATFORM_THREAD_H
