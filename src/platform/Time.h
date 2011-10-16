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

#ifndef ARX_PLATFORM_TIME_H
#define ARX_PLATFORM_TIME_H

#include "platform/Platform.h"

namespace Time {

/**
 * Get the number of milliseconds elapsed since some unspecified starting point.
 * @return The number of milliseconds elapsed.
 **/
u32 getMs();

/**
 * Get the number of microseconds elapsed since some unspecified starting point.
 * @return The number of microseconds elapsed.
 **/
u64 getUs();

/**
 * Get the number of milliseconds elapsed between now and the specified time, handling wrap around correctly.
 * @param startMs Start time in milliseconds.
 * @return The number of milliseconds elapsed between now and startMs.
 **/
inline u32 getElapsedMs(u32 startMs);

/**
 * Get the number of milliseconds elapsed between two point in time, handling wrap around correctly.
 * @param startMs Start time in milliseconds.
 * @param endMs End time in milliseconds.
 * @return The number of milliseconds elapsed between the specified time range.
 **/
inline u32 getElapsedMs(u32 startMs, u32 endMs);

/**
 * Get the number of microseconds elapsed between now and the specified time, handling wrap around correctly.
 * @param startUs Start time in microseconds.
 * @return The number of microseconds elapsed between now and startUs.
 **/
inline u64 getElapsedUs(u64 startUs);

/**
 * Get the number of microseconds elapsed between two point in time, handling wrap around correctly.
 * @param startUs Start time in microseconds.
 * @param endUs End time in microseconds.
 * @return The number of microseconds elapsed between the specified time range.
 **/
inline u64 getElapsedUs(u64 startUs, u64 endUs);

inline u32 getElapsedMs(u32 startMs) {
	return getElapsedMs(startMs, getMs());
}

inline u32 getElapsedMs(u32 startMs, u32 endMs) {
	return (u32)(((u64)endMs - (u64)startMs) & 0xffffffff);
}

inline u64 getElapsedUs(u64 startUs) {
	return getElapsedUs(startUs, getUs());
}

inline u64 getElapsedUs(u64 startUs, u64 endUs) {
	if(endUs >= startUs) {
		return endUs - startUs;
	} else {
		return (((u64)-1) - startUs) + endUs + 1;
	}
}

}

#endif // ARX_PLATFORM_TIME_H
