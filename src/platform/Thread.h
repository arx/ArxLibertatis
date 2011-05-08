
#ifndef ARX_PLATFORM_THREAD_H
#define ARX_PLATFORM_THREAD_H

#include "platform/Platform.h"

#ifdef HAVE_PTHREADS
#include <pthread.h>
#elif defined(__WIN32__)
#include <windows.h>
#endif

class Thread {
	
private:
	
#ifdef HAVE_PTHREADS
	
	pthread_t thread;
	int priority;
	
	static void * entryPoint(void * param);
	
#elif defined(__WIN32__)
	
	HANDLE thread;
	
	static DWORD WINAPI entryPoint(LPVOID param);
	
#else
	#warning "threads not supported"
#endif
	
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

#endif // ARX_PLATFORM_THREAD_H
