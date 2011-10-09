
#ifndef ARX_PLATFORM_LOCK_H
#define ARX_PLATFORM_LOCK_H

#include "Configure.h"

#if defined(HAVE_PTHREADS)
#include <pthread.h>
#elif defined(HAVE_WINAPI)
#include <windows.h>
#else
#error "Locking not supported: need either HAVE_PTHREADS or HAVE_WINAPI"
#endif

class Lock {
	
private:
	
#if defined(HAVE_PTHREADS)
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	bool locked;
#elif defined(HAVE_WINAPI)
	HANDLE mutex;
#endif
	
public:
	
	Lock();
	~Lock();
	
	void lock();
	
	void unlock();
	
};

class Autolock {
	
private:
	
	Lock * lock;
	
public:
	
	inline Autolock(Lock * _lock) : lock(_lock) {
		lock->lock();
	}
	
	inline Autolock(Lock & _lock) : lock(&_lock) {
		lock->lock();
	}
	
	inline ~Autolock() {
		lock->unlock();
	}
	
};

#endif // ARX_PLATFORM_LOCK_H
