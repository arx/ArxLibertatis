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

#include "platform/Time.h"

#include "Configure.h"

namespace Time {

#if defined(HAVE_CLOCK_GETTIME)

#include <time.h>

u32 getMs() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

u64 getUs() {
	
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	
	u64 timeUs = ts.tv_sec * 1000000ull; // Convert seconds to microseconds (in 64 bit to avoid overflow)
	timeUs += ts.tv_nsec / 1000; // Convert nanoseconds to microseconds...
	
	return timeUs;
}

#elif defined(HAVE_WINAPI)

#include <windows.h>

// Avoid costly calls to QueryPerformanceFrequency... cache its result
class FrequencyInit {
public:
	FrequencyInit() {
		QueryPerformanceFrequency(&Frequency);
	}
	LARGE_INTEGER Frequency;
} ;

FrequencyInit gFrequencyInit;
const u64 FREQUENCY_HZ  = gFrequencyInit.Frequency.QuadPart;

u32 getMs() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	// Ugly trick to avoid losing precision...
	u32 valMs = (counter.QuadPart * 10) / (FREQUENCY_HZ / 100);
	return valMs;
}

u64 getUs() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	// Ugly trick to avoid losing precision...
	u64 valUs = (counter.QuadPart * 1000) / (FREQUENCY_HZ / 1000);
	return valUs;
}

#else
#error "Time not supported: need either HAVE_CLOCK_GETTIME or HAVE_WINAPI"
#endif

}
