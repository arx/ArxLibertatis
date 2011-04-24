/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Time.H
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Time Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_CORE_TIME_H
#define ARX_CORE_TIME_H

#include "platform/Platform.h"

extern float ARXPausedTime;
extern float ARXTotalPausedTime;
extern float ARXTime;
extern bool ARXPausedTimer;

#define lARXTime (static_cast<long>( ARXTime ))
#define dwARX_TIME_Get() (static_cast<DWORD>(ARX_TIME_Get()))

void ARX_TIME_Pause();
void ARX_TIME_UnPause();
void ARX_TIME_Init();
void ARX_TIME_Force_Time_Restore(float time);

float _ARX_TIME_GetTime();

inline float ARX_TIME_Get(bool _bUsePause = true) {
	
	float tim = _ARX_TIME_GetTime();
	
	if(ARXPausedTimer && _bUsePause) {
		ARXTime = tim - ARXTotalPausedTime - (tim - ARXPausedTime);
	} else {
		ARXTime = tim - ARXTotalPausedTime;
	}
	
	return ARXTime;
}

inline unsigned long ARX_TIME_GetUL(bool _bUsePause = true) {
	float time = ARX_TIME_Get(_bUsePause);
	ARX_CHECK_ULONG(time);
	return static_cast<unsigned long>(time);
}

inline unsigned long ARXTimeUL() {
	ARX_CHECK_ULONG(ARXTime);
	return static_cast<unsigned long>(ARXTime);
}


namespace Time
{
	/**
	 * Suspends the execution of the current thread.
	 * @param sleepMs The time interval for which execution is to be suspended, in milliseconds.
	 **/
	inline void SleepMs(u32 sleepMs);

	/**
	 * Get the number of milliseconds elapsed since some unspecified starting point.
	 * @return The number of milliseconds elapsed.
	 **/
	inline u32 GetMs();

	/**
	 * Get the number of microseconds elapsed since some unspecified starting point.
	 * @return The number of microseconds elapsed.
	 **/
	inline u64 GetUs();

	/**
	 * Get the number of milliseconds elapsed between now and the specified time, handling wrap around correctly.
	 * @param startMs Start time in milliseconds.
	 * @return The number of milliseconds elapsed between now and startMs.
	 **/
	inline u32 GetElapsedMs(u32 startMs);

	/**
	 * Get the number of milliseconds elapsed between two point in time, handling wrap around correctly.
	 * @param startMs Start time in milliseconds.
	 * @param endMs End time in milliseconds.
	 * @return The number of milliseconds elapsed between the specified time range.
	 **/
	inline u32 GetElapsedMs(u32 startMs, u32 endMs);

	/**
	 * Get the number of microseconds elapsed between now and the specified time, handling wrap around correctly.
	 * @param startUs Start time in microseconds.
	 * @return The number of microseconds elapsed between now and startUs.
	 **/
	inline u64 GetElapsedUs(u64 startUs);

	/**
	 * Get the number of microseconds elapsed between two point in time, handling wrap around correctly.
	 * @param startUs Start time in microseconds.
	 * @param endUs End time in microseconds.
	 * @return The number of microseconds elapsed between the specified time range.
	 **/
	inline u64 GetElapsedUs(u64 startUs, u64 endUs);

	//-------------------------------------------------------------------------

#if ARX_PLATFORM == ARX_PLATFORM_LINUX
	inline void SleepMs(u32 sleepMs) {
		int ret = usleep(sleepMs*1000);
		arx_assert_msg(ret == 0, "usleep failed");
		ARX_UNUSED(ret);
	}

	inline u32 GetMs() {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	}
	
	inline u64 GetUs() {
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	}
#elif ARX_PLATFORM == ARX_PLATFORM_WIN32
	inline void SleepMs(u32 sleepMs) {
		Sleep(sleepMs);
	}

    extern const u64 FREQUENCY_HZ;
	inline u32 GetMs() {
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		u32 valMs = (counter.QuadPart * 10) / (FREQUENCY_HZ / 100);		// Ugly trick to avoid losing precision...
		return valMs;
	}
	
	inline u64 GetUs() {
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		u32 valUs = (counter.QuadPart * 1000) / (FREQUENCY_HZ / 1000);	// Ugly trick to avoid losing precision...
		return valUs;
	}
#endif

	inline u32 GetElapsedMs(u32 startMs) {
		return GetElapsedMs(startMs, GetMs());
	}
	
	inline u32 GetElapsedMs(u32 startMs, u32 endMs) {
		return (u32)(((u64)endMs - (u64)startMs) & ULONG_MAX);
	}

	inline u64 GetElapsedUs(u64 startUs) {
		return GetElapsedUs(startUs, GetUs());
	}
	
	inline u64 GetElapsedUs(u64 startUs, u64 endUs) {
		if (endUs >= startUs)
            return endUs - startUs;
        else
            return (ULLONG_MAX - startUs) + endUs + 1;
	}
}


#endif // ARX_CORE_TIME_H
