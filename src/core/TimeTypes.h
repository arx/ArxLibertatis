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

#ifndef ARX_CORE_TIMETYPES_H
#define ARX_CORE_TIMETYPES_H

#include <algorithm>

#include "platform/Platform.h"
#include "util/StrongType.h"

ARX_STRONG_TYPEDEF(s64, ArxInstant)
ARX_STRONG_TYPEDEF(s64, ArxDuration)

const ArxInstant  ArxInstant_ZERO  = ArxInstant(0);
const ArxDuration ArxDuration_ZERO = ArxDuration(0);

inline ArxInstant ArxInstantMs(s64 val) {
	return ArxInstant(val);
}
inline ArxDuration ArxDurationMs(s64 val) {
	return ArxDuration(val);
}

inline s64 toMs(ArxInstant val) {
	return val.t;
}
inline s64 toMs(ArxDuration val) {
	return val.t;
}

inline ArxDuration operator -(ArxInstant a, ArxInstant b) {
	return ArxDuration(a.t - b.t);
}
inline ArxInstant operator +(ArxInstant a, ArxDuration b) {
	return ArxInstant(a.t + b.t);
}
inline ArxInstant operator -(ArxInstant a, ArxDuration b) {
	return ArxInstant(a.t - b.t);
}

inline ArxDuration operator +(ArxDuration a, ArxDuration b) {
	return ArxDuration(a.t + b.t);
}
inline ArxDuration operator -(ArxDuration a, ArxDuration b) {
	return ArxDuration(a.t - b.t);
}

// FIXME copy-paste

// in microseconds
typedef StrongType<struct PlatformInstant_TAG,  s64> PlatformInstant;
typedef StrongType<struct PlatformDuration_TAG, s64> PlatformDuration;

const PlatformInstant  PlatformInstant_ZERO  = PlatformInstant(0);
const PlatformDuration PlatformDuration_ZERO = PlatformDuration(0);

inline PlatformInstant PlatformInstantMs(s64 val) {
	return PlatformInstant(val * 1000);
}
inline PlatformDuration PlatformDurationMs(s64 val) {
	return PlatformDuration(val * 1000);
}

inline double toMs(PlatformDuration val) {
	return val.t / (1000.0);
}
inline double toS(PlatformDuration val) {
	return val.t / (1000.0 * 1000.0);
}

inline PlatformDuration operator -(PlatformInstant a, PlatformInstant b) {
	return PlatformDuration(a.t - b.t);
}
inline PlatformInstant operator +(PlatformInstant a, PlatformDuration b) {
	return PlatformInstant(a.t + b.t);
}
inline PlatformInstant operator -(PlatformInstant a, PlatformDuration b) {
	return PlatformInstant(a.t - b.t);
}

inline PlatformDuration operator +(PlatformDuration a, PlatformDuration b) {
	return PlatformDuration(a.t + b.t);
}
inline PlatformDuration operator -(PlatformDuration a, PlatformDuration b) {
	return PlatformDuration(a.t - b.t);
}

// copy-paste end


namespace arx {
template <typename T>
T clamp(T const & x, T const & minVal, T const & maxVal) {
	return std::min(maxVal, std::max(minVal, x));
}
} // namespace arx

#endif // ARX_CORE_TIMETYPES_H
