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


template <typename TAG, typename T>
struct DurationType
	: boost::totally_ordered1<DurationType<TAG, T>
	, boost::additive1<DurationType<TAG, T>
	> >
{
	T t;
	
	DurationType()
		: t()
	{ }
	explicit DurationType(const T t_)
		: t(t_)
	{ }
	DurationType(const DurationType<TAG, T> & t_)
		: t(t_.t)
	{ }
	bool operator==(const DurationType<TAG, T> & rhs) const {
		return t == rhs.t;
	}
	bool operator<(const DurationType<TAG, T> & rhs) const {
		return t < rhs.t;
	}
	DurationType<TAG, T> & operator=(const DurationType<TAG, T> & rhs) {
		t = rhs.t;
		return *this;
	}
	DurationType<TAG, T> & operator+=(const DurationType<TAG, T> & rhs) {
		this->t += rhs.t;
		return *this;
	}
	DurationType<TAG, T> & operator-=(const DurationType<TAG, T> & rhs) {
		this->t -= rhs.t;
		return *this;
	}
};

template <typename TAG, typename T>
struct InstantType
	: boost::totally_ordered1<InstantType<TAG, T>
	>
{
	T t;
	
	InstantType()
		: t()
	{ }
	explicit InstantType(const T t_)
		: t(t_)
	{ }
	InstantType(const InstantType<TAG, T> & t_)
		: t(t_.t)
	{ }
	bool operator==(const InstantType<TAG, T> & rhs) const {
		return t == rhs.t;
	}
	bool operator<(const InstantType<TAG, T> & rhs) const {
		return t < rhs.t;
	}
	InstantType<TAG, T> & operator=(const InstantType<TAG, T> & rhs) {
		t = rhs.t;
		return *this;
	}
};
template <typename TAG, typename T>
inline DurationType<TAG, T> operator -(InstantType<TAG, T> a, InstantType<TAG, T> b) {
	return DurationType<TAG, T>(a.t - b.t);
}
template <typename TAG, typename T>
inline InstantType<TAG, T> operator +(InstantType<TAG, T> a, DurationType<TAG, T> b) {
	return InstantType<TAG, T>(a.t + b.t);
}
template <typename TAG, typename T>
inline InstantType<TAG, T> operator -(InstantType<TAG, T> a, DurationType<TAG, T> b) {
	return InstantType<TAG, T>(a.t - b.t);
}

// ArxTime
// in ms
typedef StrongType<struct ArxInstant_TAG,  s64> ArxInstant;

//#define ARX_REFACTOR_TIMETYPES 1
#ifdef ARX_REFACTOR_TIMETYPES
typedef DurationType<struct ArxTime_TAG, s64> ArxDuration;
#else
typedef StrongType<struct ArxDuration_TAG, s64> ArxDuration;
#endif

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

#ifndef ARX_REFACTOR_TIMETYPES
inline ArxDuration operator +(ArxDuration a, ArxDuration b) {
	return ArxDuration(a.t + b.t);
}
inline ArxDuration operator -(ArxDuration a, ArxDuration b) {
	return ArxDuration(a.t - b.t);
}
#endif

// PlatformTime
// in microseconds
typedef InstantType <struct PlatformTime_TAG, s64> PlatformInstant;
typedef DurationType<struct PlatformTime_TAG, s64> PlatformDuration;

const PlatformInstant  PlatformInstant_ZERO  = PlatformInstant(0);
const PlatformDuration PlatformDuration_ZERO = PlatformDuration(0);

inline PlatformInstant PlatformInstantMs(s64 val) {
	return PlatformInstant(val * 1000);
}

inline PlatformDuration PlatformDurationUs(s64 val) {
	return PlatformDuration(val);
}
inline PlatformDuration PlatformDurationMs(s64 val) {
	return PlatformDuration(val * 1000);
}

inline float toMs(PlatformDuration val) {
	return val.t / (1000.f);
}
inline float toS(PlatformDuration val) {
	return val.t / (1000.f * 1000.f);
}

// AnimationTime
// in microseconds
typedef DurationType<struct AnimationTime_TAG, s64> AnimationDuration;
const AnimationDuration AnimationDuration_ZERO = AnimationDuration(0);

inline AnimationDuration operator*(AnimationDuration v, float scalar) {
	return AnimationDuration(s64(float(v.t) * scalar));
}

inline AnimationDuration AnimationDurationUs(s64 val) {
	return AnimationDuration(val);
}
inline AnimationDuration AnimationDurationMs(s64 val) {
	return AnimationDuration(val * 1000);
}

inline s64 toMsi(AnimationDuration val) {
	return val.t / (1000);
}
inline float toMsf(AnimationDuration val) {
	return val.t / (1000.f);
}
inline float toS(AnimationDuration val) {
	return val.t / (1000.f * 1000.f);
}

inline AnimationDuration toAnimationDuration(PlatformDuration val) {
	return AnimationDuration(val.t);
}
inline AnimationDuration toAnimationDuration(ArxDuration val) {
	return AnimationDuration(val.t * 1000);
}
inline ArxDuration toArxDuration(AnimationDuration val) {
	return ArxDuration(val.t / 1000);
}



namespace arx {
template <typename T>
T clamp(T const & x, T const & minVal, T const & maxVal) {
	return std::min(maxVal, std::max(minVal, x));
}
} // namespace arx

#endif // ARX_CORE_TIMETYPES_H
