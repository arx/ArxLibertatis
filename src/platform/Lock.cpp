
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

void Lock::lock() {
	
	pthread_mutex_lock(&mutex);
	
	while(locked) {
		int rc = pthread_cond_wait(&cond, &mutex);
		arx_assert(rc == 0);
	}
	
	locked = true;
	pthread_mutex_unlock(&mutex);
}

void Lock::unlock() {
	pthread_mutex_lock(&mutex);
	locked = false;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

#elif defined(__WIN32__)

Lock::Lock() {
	mutex = CreateMutex(NULL, false, NULL);
}

Lock::~Lock() {
	unlock();
	CloseHandle(mutex);
}

void Lock::lock() {
	DWORD rc = WaitForSingleObject(mutex, INFINITE);
	arx_assert(rc == WAIT_OBJECT_0);
}

void Lock::unlock() {
	ReleaseMutex(mutex);
}

#endif
