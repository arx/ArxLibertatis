
#include "platform/Thread.h"

#include "platform/Platform.h"

#if defined(HAVE_PTHREADS)

#include <sched.h>

Thread::Thread() : thread((pthread_t)0) {
	setPriority(Normal);
}

void Thread::start() {
	
	if(thread) {
		return;
	}
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	sched_param param;
	param.sched_priority = priority;
	pthread_attr_setschedparam(&attr, &param);
	
	pthread_create(&thread, NULL, entryPoint, this);
	
	pthread_attr_destroy(&attr);
	
}

void Thread::setPriority(Priority _priority) {
	
	int policy = sched_getscheduler(0);
	
	int min = sched_get_priority_min(policy);
	int max = sched_get_priority_max(policy);
	
	priority = min + ((_priority - Lowest) * (max - min) / (Highest - Lowest));
	
	if(thread) {
		pthread_setschedprio(thread, priority);
	}
}

Thread::~Thread() { }

void Thread::waitForCompletion() {
	if(thread) {
		pthread_join(thread, NULL);
	}
}

void * Thread::entryPoint(void * param) {
	((Thread*)param)->run();
	return NULL;
}

void Thread::exit() {
	pthread_exit(NULL);
}

#elif defined(HAVE_WINAPI)

Thread::Thread() {
	thread = CreateThread(NULL, 0, entryPoint, this, CREATE_SUSPENDED, NULL);
	arx_assert(thread);
	setPriority(Normal);
}

void Thread::start() {
	DWORD ret = ResumeThread(thread);
	arx_assert(ret != (DWORD)-1);
}

static const int windowsThreadPriorities[] = {
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST
};

void Thread::setPriority(Priority priority) {
	
	arx_assert(priority >= Lowest && priority <= Highest);
	
	BOOL ret = SetThreadPriority(thread, windowsThreadPriorities[priority - Lowest]);
	arx_assert(ret);
}

Thread::~Thread() {
	CloseHandle(thread);
}

DWORD WINAPI Thread::entryPoint(LPVOID param) {
	((Thread*)param)->run();
	return 0;
}

void Thread::exit() {
	ExitThread(0);
}

void Thread::waitForCompletion() {
	DWORD ret = WaitForSingleObject(thread, INFINITE);
	arx_assert(ret == WAIT_OBJECT_0);
}

#endif

#if defined(HAVE_NANOSLEEP)

#include <time.h>

void Thread::sleep(unsigned milliseconds) {
	
	timespec t;
	t.tv_sec = milliseconds / 1000;
	t.tv_nsec = (milliseconds % 1000) * 1000000;
	
	nanosleep(&t, NULL);
}

#elif defined(HAVE_WINAPI)

void Thread::sleep(unsigned milliseconds) {
	Sleep(milliseconds);
}

#else
#error "Sleep not supported: need either HAVE_NANOSLEEP or HAVE_WINAPI"
#endif
