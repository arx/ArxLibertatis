/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Time.h"

#include "Configure.h"

#include "io/log/Logger.h"

namespace Time {

#if ARX_HAVE_CLOCK_GETTIME

#include <time.h>

static clock_t clock_id = CLOCK_REALTIME;

void init() {
	
#ifdef CLOCK_MONOTONIC
	struct timespec ts;
	if(!clock_gettime(CLOCK_MONOTONIC, &ts)) {
		LogDebug("using CLOCK_MONOTONIC");
		clock_id = CLOCK_MONOTONIC;
		return;
	}
#endif
	
	LogWarning << "Falling back to CLOCK_REALTIME, time will jump if adjusted by other processes";
}

u32 getMs() {
	struct timespec ts;
	clock_gettime(clock_id, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

u64 getUs() {
	struct timespec ts;
	clock_gettime(clock_id, &ts);
	return (ts.tv_sec * 1000000ull) + (ts.tv_nsec / 1000);
}

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <windows.h>

// Avoid costly calls to QueryPerformanceFrequency... cache its result
static u64 frequency_hz;

void init() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	frequency_hz = frequency.QuadPart;
}

u32 getMs() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	// Ugly trick to avoid losing precision...
	u32 valMs = (counter.QuadPart * 10) / (frequency_hz / 100);
	return valMs;
}

u64 getUs() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	// Ugly trick to avoid losing precision...
	u64 valUs = (counter.QuadPart * 1000) / (frequency_hz / 1000);
	return valUs;
}

#elif ARX_HAVE_MACH_CLOCK

#include <mach/clock.h>
#include <mach/clock_types.h>
#include <mach/mach_host.h>

static clock_serv_t clock_ref;

void init() {
	if(host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock_ref) != KERN_SUCCESS) {
		LogWarning << "Error getting system clock";
	}
}

u32 getMs() {
	mach_timespec_t ts;
	clock_get_time(clock_ref, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

u64 getUs() {
	mach_timespec_t ts;
	clock_get_time(clock_ref, &ts);
	return (ts.tv_sec * 1000000ull) + (ts.tv_nsec / 1000);
}

#else
#error "Time not supported: need ARX_HAVE_CLOCK_GETTIME or ARX_HAVE_MACH_CLOCK on non-Windows systems"
#endif

} // namespace Time
