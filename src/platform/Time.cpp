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

#include "platform/Time.h"

#include "Configure.h"

#include "io/log/Logger.h"

namespace platform {

#if ARX_HAVE_CLOCK_GETTIME

#include <time.h>

static clockid_t g_clockId = CLOCK_REALTIME;

void initializeTime() {
	
#ifdef CLOCK_MONOTONIC
	struct timespec ts;
	if(!clock_gettime(CLOCK_MONOTONIC, &ts)) {
		LogDebug("using CLOCK_MONOTONIC");
		g_clockId = CLOCK_MONOTONIC;
		return;
	}
#endif
	
	LogWarning << "Falling back to CLOCK_REALTIME, time will jump if adjusted by other processes";
}

PlatformInstant getTime() {
	struct timespec ts;
	clock_gettime(g_clockId, &ts);
	return PlatformInstantUs(ts.tv_sec * 1000000ll + ts.tv_nsec / 1000ll);
}

#elif ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <windows.h>

// Avoid costly calls to QueryPerformanceFrequency... cache its result
static s64 g_clockFrequency;

void initializeTime() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	g_clockFrequency = frequency.QuadPart;
}

PlatformInstant getTime() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	// Ugly trick to avoid losing precision...
	return PlatformInstantUs(counter.QuadPart * 1000ll / (g_clockFrequency / 1000ll));
}

#elif ARX_HAVE_MACH_CLOCK

#include <mach/clock.h>
#include <mach/clock_types.h>
#include <mach/mach_host.h>

static clock_serv_t g_clockRef;

void initializeTime() {
	if(host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &g_clockRef) != KERN_SUCCESS) {
		LogWarning << "Error getting system clock";
	}
}

PlatformInstant getTime() {
	mach_timespec_t ts;
	clock_get_time(g_clockRef, &ts);
	return PlatformInstantUs(ts.tv_sec * 1000000ll + ts.tv_nsec / 1000ll);
}

#else
#error "Time not supported: need ARX_HAVE_CLOCK_GETTIME or ARX_HAVE_MACH_CLOCK on non-Windows systems"
#endif

} // namespace platform
