/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Thread.h"

#if defined(HAVE_PTHREADS)

#include <sched.h>

Thread::Thread() : started(false) {
	setPriority(Normal);
}

void Thread::start() {
	
	if(started) {
		return;
	}
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	sched_param param;
	param.sched_priority = priority;
	pthread_attr_setschedparam(&attr, &param);
	
	pthread_create(&thread, NULL, entryPoint, this);
	
	pthread_attr_destroy(&attr);
	
	started = true;
}

void Thread::setPriority(Priority _priority) {
	
	int policy = sched_getscheduler(0);
	
	int min = sched_get_priority_min(policy);
	int max = sched_get_priority_max(policy);
	
	priority = min + ((_priority - Lowest) * (max - min) / (Highest - Lowest));
	
	if(started && min != max) {
		sched_param param;
		param.sched_priority = priority;
		pthread_setschedparam(thread, policy, &param);
	}
}

Thread::~Thread() { }

void Thread::waitForCompletion() {
	if(started) {
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

#include "platform/Platform.h"

Thread::Thread() {
	thread = CreateThread(NULL, 0, entryPoint, this, CREATE_SUSPENDED, NULL);
	arx_assert(thread);
	setPriority(Normal);
}

void Thread::start() {
	DWORD ret = ResumeThread(thread);
	arx_assert(ret != (DWORD)-1);
	ARX_UNUSED(ret);
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
	ARX_UNUSED(ret);
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
	ARX_UNUSED(ret);
}

#endif

#if defined(HAVE_NANOSLEEP_FUNC)

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
#error "Sleep not supported: need either HAVE_NANOSLEEP_FUNC or HAVE_WINAPI"
#endif
