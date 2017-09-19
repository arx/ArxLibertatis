/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
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

namespace platform {

/*!
 * \brief Initalize the time subsystem
 *
 * Must be called before any other time functions.
 */
void initializeTime();

/*!
 * \brief Get the number of milliseconds elapsed since some unspecified starting point
 *
 * \return The number of milliseconds elapsed.
 */
u32 getTimeMs();

/*!
 * \brief Get the number of microseconds elapsed since some unspecified starting point
 *
 * \return The number of microseconds elapsed.
 */
u64 getTimeUs();

/*!
 * \brief Get the number of milliseconds elapsed between now and the specified time
 *
 * Handles wrap around correctly.
 *
 * \param startMs Start time in milliseconds.
 *
 * \return The number of milliseconds elapsed between now and startMs.
 */
inline u32 getElapsedMs(u32 startMs);

/*!
 * \brief Get the number of milliseconds elapsed between two points in time
 *
 * Handles wrap around correctly.
 *
 * \param startMs Start time in milliseconds.
 * \param endMs End time in milliseconds.
 *
 * \return The number of milliseconds elapsed between the specified time range.
 */
inline u32 getElapsedMs(u32 startMs, u32 endMs);

/*!
 * \brief Get the number of microseconds elapsed between now and the specified time
 *
 * Handles wrap around correctly.
 *
 * \param startUs Start time in microseconds.
 *
 * \return The number of microseconds elapsed between now and startUs.
 */
inline u64 getElapsedUs(u64 startUs);

/*!
 * \brief Get the number of microseconds elapsed between two point in time
 *
 * Handles wrap around correctly.
 *
 * \param startUs Start time in microseconds.
 * \param endUs End time in microseconds.
 *
 * \return The number of microseconds elapsed between the specified time range.
 */
inline u64 getElapsedUs(u64 startUs, u64 endUs);

// Implementation:

inline u32 getElapsedMs(u32 startMs) {
	return getElapsedMs(startMs, getTimeMs());
}

inline u32 getElapsedMs(u32 startMs, u32 endMs) {
	return endMs - startMs;
}

inline u64 getElapsedUs(u64 startUs) {
	return getElapsedUs(startUs, getTimeUs());
}

inline u64 getElapsedUs(u64 startUs, u64 endUs) {
	return endUs - startUs;
}

} // namespace platform

#endif // ARX_PLATFORM_TIME_H
