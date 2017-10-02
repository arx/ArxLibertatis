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

#ifndef ARX_CORE_TIMETYPES_H
#define ARX_CORE_TIMETYPES_H

#include <algorithm>

#include <boost/config.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits.hpp>

#include "platform/Platform.h"


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
	
	static inline DurationType ofRaw(T value) {
		return DurationType(value);
	}
	
private:
	explicit DurationType(const T t_)
		: t(t_)
	{ }
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
	
	static inline InstantType ofRaw(T value) {
		return InstantType(value);
	}
	
private:
	explicit InstantType(const T t_)
		: t(t_)
	{ }
};

template <typename TAG, typename T>
inline DurationType<TAG, T> operator -(InstantType<TAG, T> a, InstantType<TAG, T> b) {
	return DurationType<TAG, T>::ofRaw(a.t - b.t);
}
template <typename TAG, typename T, class IntType>
inline DurationType<TAG, T> operator *(DurationType<TAG, T> a, IntType b) {
	ARX_STATIC_ASSERT(boost::is_integral<IntType>::value, "factor must be int type");
	return DurationType<TAG, T>::ofRaw(a.t * T(b));
}

template <typename TAG, typename T>
inline InstantType<TAG, T> operator +(InstantType<TAG, T> a, DurationType<TAG, T> b) {
	return InstantType<TAG, T>::ofRaw(a.t + b.t);
}
template <typename TAG, typename T>
inline InstantType<TAG, T> operator -(InstantType<TAG, T> a, DurationType<TAG, T> b) {
	return InstantType<TAG, T>::ofRaw(a.t - b.t);
}

template <typename TAG, typename T>
inline InstantType<TAG, T> & operator +=(InstantType<TAG, T> & a, DurationType<TAG, T> b) {
	a.t += b.t;
	return a;
}

template <typename TAG, typename T>
inline float operator /(DurationType<TAG, T> a, DurationType<TAG, T> b) {
	return float(a.t) / float(b.t);
}


// ArxTime
// in ms
typedef InstantType <struct ArxTime_TAG, s64> ArxInstant;
typedef DurationType<struct ArxTime_TAG, s64> ArxDuration;

const ArxInstant  ArxInstant_ZERO  = ArxInstant::ofRaw(0);
const ArxDuration ArxDuration_ZERO = ArxDuration::ofRaw(0);

inline ArxInstant ArxInstantUs(s64 val) {
	return ArxInstant::ofRaw(val / 1000);
}
inline ArxInstant ArxInstantMs(s64 val) {
	return ArxInstant::ofRaw(val);
}
inline ArxDuration ArxDurationMs(s64 val) {
	return ArxDuration::ofRaw(val);
}
inline ArxDuration ArxDurationMsf(float val) {
	return ArxDuration::ofRaw(s64(val)); // TODO loss of precision
}

inline s64 toUs(ArxInstant val) {
	return val.t * 1000;
}
inline s64 toMsi(ArxInstant val) {
	return val.t;
}
inline float toMsf(ArxInstant val) {
	return float(val.t);
}

inline s64 toMsi(ArxDuration val) {
	return val.t;
}
inline float toMsf(ArxDuration val) {
	return float(val.t);
}

// PlatformTime
// in microseconds
typedef InstantType <struct PlatformTime_TAG, s64> PlatformInstant;
typedef DurationType<struct PlatformTime_TAG, s64> PlatformDuration;

const PlatformInstant  PlatformInstant_ZERO  = PlatformInstant::ofRaw(0);
const PlatformDuration PlatformDuration_ZERO = PlatformDuration::ofRaw(0);

inline PlatformInstant PlatformInstantUs(s64 val) {
	return PlatformInstant::ofRaw(val);
}
inline PlatformInstant PlatformInstantMs(s64 val) {
	return PlatformInstant::ofRaw(val * 1000);
}

inline PlatformDuration PlatformDurationUs(s64 val) {
	return PlatformDuration::ofRaw(val);
}
inline PlatformDuration PlatformDurationMs(s64 val) {
	return PlatformDuration::ofRaw(val * 1000);
}
inline PlatformDuration PlatformDurationMsf(float val) {
	return PlatformDuration::ofRaw(s64(val * 1000.f));
}

inline float toMs(PlatformDuration val) {
	return float(val.t) / (1000.f);
}
inline float toS(PlatformDuration val) {
	return float(val.t) / (1000.f * 1000.f);
}

// AnimationTime
// in microseconds
typedef DurationType<struct AnimationTime_TAG, s64> AnimationDuration;
const AnimationDuration AnimationDuration_ZERO = AnimationDuration::ofRaw(0);

inline AnimationDuration operator*(AnimationDuration v, float scalar) {
	return AnimationDuration::ofRaw(s64(float(v.t) * scalar));
}

inline AnimationDuration AnimationDurationUs(s64 val) {
	return AnimationDuration::ofRaw(val);
}
inline AnimationDuration AnimationDurationMs(s64 val) {
	return AnimationDuration::ofRaw(val * 1000);
}
inline AnimationDuration AnimationDurationMsf(float val) {
	return AnimationDuration::ofRaw(s64(val * 1000.f));
}

inline s64 toMsi(AnimationDuration val) {
	return val.t / (1000);
}
inline float toMsf(AnimationDuration val) {
	return float(val.t) / (1000.f);
}
inline float toS(AnimationDuration val) {
	return float(val.t) / (1000.f * 1000.f);
}

inline AnimationDuration toAnimationDuration(PlatformDuration val) {
	return AnimationDuration::ofRaw(val.t);
}
inline AnimationDuration toAnimationDuration(ArxDuration val) {
	return AnimationDuration::ofRaw(val.t * 1000);
}
inline ArxDuration toArxDuration(AnimationDuration val) {
	return ArxDuration::ofRaw(val.t / 1000);
}
inline PlatformDuration toPlatformDuration(AnimationDuration val) {
	return PlatformDuration::ofRaw(val.t);
}


namespace arx {
template <typename T>
T clamp(T const & x, T const & minVal, T const & maxVal) {
	return std::min(maxVal, std::max(minVal, x));
}
} // namespace arx

#endif // ARX_CORE_TIMETYPES_H
