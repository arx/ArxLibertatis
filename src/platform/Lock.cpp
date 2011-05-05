
#include "platform/Lock.h"

#ifdef HAVE_PTHREADS

#include <time.h>
#include <errno.h>

#include "io/Logger.h"

Lock::Lock() : locked(false) {
	const pthread_mutex_t mutex_init = PTHREAD_MUTEX_INITIALIZER;
	mutex = mutex_init;
	const pthread_cond_t cond_init = PTHREAD_COND_INITIALIZER;
	cond = cond_init;
}

Lock::~Lock() {
	
}

bool Lock::lock(long timeout) {
	
	pthread_mutex_lock(&mutex);
	
	if(locked) {
		struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
		time.tv_sec += timeout;
		int rc;
		do {
			rc = pthread_cond_timedwait(&cond, &mutex, &time);
		} while(rc == 0 && locked);
		if(rc == ETIMEDOUT) {
			pthread_mutex_unlock(&mutex);
			LogWarning << "lock timeout " << timeout << " " << time.tv_nsec;
			return false;
		}
	}
	
	locked = true;
	pthread_mutex_unlock(&mutex);
	return true;
}

void Lock::unlock() {
	pthread_mutex_lock(&mutex);
	locked = false;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

Lock::Lock() {
	mutex = CreateMutex(NULL, false, NULL);
}

Lock::~Lock() {
	unlock();
	CloseHandle(mutex);
}

bool Lock::lock(long timeout) {
	return WaitForSingleObject(mutex, timeout) == WAIT_TIMEOUT ? false : true;
}

void Lock::unlock() {
	ReleaseMutex(mutex);
}

#endif
