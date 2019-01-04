/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <cstring>

#include "platform/Architecture.h"

#if ARX_HAVE_XMMINTRIN
#include <xmmintrin.h>
#endif

#if ARX_HAVE_PMMINTRIN
#include <pmmintrin.h>
#endif

#if ARX_COMPILER_MSVC && (ARX_ARCH == ARX_ARCH_X86 || ARX_ARCH == ARX_ARCH_X86_64)
#include <intrin.h>
#include <immintrin.h>
#endif

#if ARX_HAVE_GET_CPUID && !defined(ARX_INCLUDED_CPUID_H)
#define ARX_INCLUDED_CPUID_H <cpuid.h>
#include ARX_INCLUDED_CPUID_H
#endif

#include <boost/static_assert.hpp>

#include "math/Random.h"

#include "platform/Alignment.h"
#include "platform/CrashHandler.h"
#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"

void Thread::setThreadName(const std::string & threadName) {
	m_threadName = threadName;
}

#if ARX_HAVE_PTHREADS

#include <sched.h>
#include <unistd.h>

#if !ARX_HAVE_PTHREAD_SETNAME_NP && !ARX_HAVE_PTHREAD_SET_NAME_NP && ARX_HAVE_PRCTL
#include <sys/prctl.h>
#endif

#if ARX_HAVE_PTHREAD_SET_NAME_NP
#include <pthread_np.h>
#endif

Thread::Thread()
	: m_thread()
	, m_priority()
	, m_started(false)
{
	setPriority(Normal);
}

void Thread::start() {
	
	if(m_started) {
		return;
	}
	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	sched_param param;
	param.sched_priority = m_priority;
	pthread_attr_setschedparam(&attr, &param);
	
	pthread_create(&m_thread, NULL, entryPoint, this);
	
	pthread_attr_destroy(&attr);
	
	m_started = true;
}

void Thread::setPriority(Priority priority) {
	
	#if ARX_HAVE_SCHED_GETSCHEDULER
	int policy = sched_getscheduler(0);
	#else
	int policy = SCHED_RR;
	#endif
	
	int min = sched_get_priority_min(policy);
	int max = sched_get_priority_max(policy);
	
	m_priority = min + ((priority - Lowest) * (max - min) / (Highest - Lowest));
	
	if(m_started && min != max) {
		sched_param param;
		param.sched_priority = m_priority;
		pthread_setschedparam(m_thread, policy, &param);
	}
}

Thread::~Thread() { }

void Thread::waitForCompletion() {
	if(m_started) {
		pthread_join(m_thread, NULL);
	}
}

void * Thread::entryPoint(void * param) {
	
	// Denormals must be disabled for each thread separately
	disableFloatDenormals();
	
	Thread & thread = *static_cast<Thread *>(param);
	
	// Set the thread name.
	#if ARX_HAVE_PTHREAD_SETNAME_NP && ARX_PLATFORM != ARX_PLATFORM_MACOS
	// Linux
	pthread_setname_np(thread.m_thread, thread.m_threadName.c_str());
	#elif ARX_HAVE_PTHREAD_SETNAME_NP && ARX_PLATFORM == ARX_PLATFORM_MACOS
	// macOS
	pthread_setname_np(thread.m_threadName.c_str());
	#elif ARX_HAVE_PTHREAD_SET_NAME_NP
	// FreeBSD & OpenBSD
	pthread_set_name_np(thread.m_thread, thread.m_threadName.c_str());
	#elif ARX_HAVE_PRCTL && defined(PR_SET_NAME)
	// Linux
	prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(thread.m_threadName.c_str()), 0, 0, 0);
	#else
	// This is non-fatal, but let's print a warning so future ports will be
	// reminded to implement it.
	#pragma message ( "No function available to set thread names!" )
	#endif
	
	Random::seed();
	CrashHandler::registerThreadCrashHandlers();
	profiler::registerThread(thread.m_threadName);
	thread.run();
	profiler::unregisterThread();
	CrashHandler::unregisterThreadCrashHandlers();
	Random::shutdown();
	
	return NULL;
}

void Thread::exit() {
	pthread_exit(NULL);
}

thread_id_type Thread::getCurrentThreadId() {
	return pthread_self();
}

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

Thread::Thread() {
	m_thread = CreateThread(NULL, 0, entryPoint, this, CREATE_SUSPENDED, NULL);
	arx_assert(m_thread);
	setPriority(Normal);
}

void Thread::start() {
	DWORD ret = ResumeThread(m_thread);
	arx_assert(ret != DWORD(-1));
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
	
	BOOL ret = SetThreadPriority(m_thread, windowsThreadPriorities[priority - Lowest]);
	arx_assert(ret);
	ARX_UNUSED(ret);
}

Thread::~Thread() {
	CloseHandle(m_thread);
}

namespace {

void SetCurrentThreadName(const std::string & threadName) {
	
	#if ARX_COMPILER_MSVC
	
	if(threadName.empty() || !IsDebuggerPresent()) {
		return;
	}
	
	typedef struct tagTHREADNAME_INFO {
		DWORD   dwType;         // must be 0x1000
		LPCSTR  szName;         // pointer to name (in user addr space)
		DWORD   dwThreadID;     // thread ID (-1=caller thread)
		DWORD   dwFlags;        // reserved for future use, must be zero
	} THREADNAME_INFO;
	
	THREADNAME_INFO info;
	info.dwType         = 0x1000;
	info.szName         = threadName.c_str();
	info.dwThreadID     = ::GetCurrentThreadId();
	info.dwFlags        = 0;
	
	const DWORD MS_VC_EXCEPTION = 0x406D1388;
	
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), reinterpret_cast<ULONG_PTR *>(&info));
	}
	__except(EXCEPTION_CONTINUE_EXECUTION) { }
	
	#else
	ARX_UNUSED(threadName);
	#endif
	
}

} // anonymous namespace

DWORD WINAPI Thread::entryPoint(LPVOID param) {
	
	// Denormals must be disabled for each thread separately
	disableFloatDenormals();
	
	SetCurrentThreadName(static_cast<Thread *>(param)->m_threadName);
	
	Random::seed();
	CrashHandler::registerThreadCrashHandlers();
	profiler::registerThread(static_cast<Thread *>(param)->m_threadName);
	static_cast<Thread *>(param)->run();
	profiler::unregisterThread();
	CrashHandler::unregisterThreadCrashHandlers();
	Random::shutdown();
	
	return 0;
}

void Thread::exit() {
	ExitThread(0);
}

void Thread::waitForCompletion() {
	DWORD ret = WaitForSingleObject(m_thread, INFINITE);
	arx_assert(ret == WAIT_OBJECT_0);
	ARX_UNUSED(ret);
}

thread_id_type Thread::getCurrentThreadId() {
	return GetCurrentThreadId();
}

#endif

#ifndef _MM_DENORMALS_ZERO_MASK
#define _MM_DENORMALS_ZERO_MASK  0x0040
#endif
#ifndef _MM_DENORMALS_ZERO_ON
#define _MM_DENORMALS_ZERO_ON    0x0040
#endif
#ifndef _MM_SET_DENORMALS_ZERO_MODE
#define _MM_SET_DENORMALS_ZERO_MODE(mode) \
  _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
#endif

void Thread::disableFloatDenormals() {
	
	#if ARX_ARCH == ARX_ARCH_X86 && !ARX_HAVE_SSE
	
	// Denormals can only be disabled for SSE instructions
	// We would need to drop support for x86 CPUs without SSE(2) and
	// compile with -msse(2) -mfpmath=sse for this to have an effect
	
	#elif ARX_ARCH == ARX_ARCH_X86 || ARX_ARCH == ARX_ARCH_X86_64
	
	BOOST_STATIC_ASSERT(ARX_HAVE_SSE);
	
	#if ARX_HAVE_XMMINTRIN

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // SSE
	
	#if ARX_HAVE_SSE3 || ARX_COMPILER_MSVC || ARX_HAVE_GET_CPUID
	
	#if ARX_HAVE_SSE3
	
	const bool have_daz = true;
	
	#else // !ARX_HAVE_SSE3
	
	bool have_daz = false;
	#if ARX_COMPILER_MSVC
	int cpuinfo[4] = { 0 };
	__cpuid(cpuinfo, 1);
	#else
	unsigned cpuinfo[4] = { 0 };
	__get_cpuid(1, &cpuinfo[0], &cpuinfo[1], &cpuinfo[2], &cpuinfo[3]);
	#endif
	
	#define ARX_CPUID_ECX_SSE3 (1 << 0)
	if(cpuinfo[2] & ARX_CPUID_ECX_SSE3) {
		have_daz = true;
	}
	
	#if ARX_COMPILER_MSVC || ARX_HAVE_BUILTIN_IA32_FXSAVE
	#define ARX_CPUID_EDX_FXSR (1 << 24)
	else if(cpuinfo[3] & ARX_CPUID_EDX_FXSR) {
		ARX_ALIGNAS(16) char buffer[512];
		#if ARX_COMPILER_MSVC
		_fxsave(buffer);
		#else
		__builtin_ia32_fxsave(buffer);
		#endif
		unsigned mxcsr_mask;
		std::memcpy(&mxcsr_mask, buffer + 28, sizeof(mxcsr_mask));
		have_daz = (mxcsr_mask & _MM_DENORMALS_ZERO_ON) != 0;
	}
	#endif
	
	#endif // !ARX_HAVE_SSE3
	
	if(have_daz) {
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON); // SSE3 (and most SSE2 CPUs)
	}
	
	#else
	#pragma message ( "Disabling SSE2 float denormals is not supported for this compiler!" )
	#endif
	
	#else
	#pragma message ( "Disabling SSE float denormals is not supported for this compiler!" )
	#endif
	
	#elif ARX_ARCH == ARX_ARCH_ARM && ARX_HAVE_VFP
	
	// Denormals are always disabled for NEON, disable them for VFP instructions as well
	// Set bit 24 (flush-to-zero) in the floating-point status and control register
	asm volatile(
		"vmrs r0, FPSCR \n"
		"orr r0, r0, #0x1000000 \n"
		"vmsr FPSCR, r0 \n"
	);
	
	#else
	
	#pragma message ( "Disabling float denormals is not supported for this architecture!" )
	
	#endif
	
}

#if ARX_HAVE_NANOSLEEP

#include <time.h>

void Thread::sleep(PlatformDuration time) {
	
	timespec t;
	t.tv_sec = time_t(toUs(time) / 1000000);
	t.tv_nsec = long(toUs(time) % 1000000) * 1000l;
	
	nanosleep(&t, NULL);
}

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

void Thread::sleep(PlatformDuration time) {
	Sleep(DWORD(toMsi(time)));
}

#else
#error "Sleep not supported: need ARX_HAVE_NANOSLEEP in non-Windows systems"
#endif
