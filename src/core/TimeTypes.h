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

#ifndef ARX_CORE_TIMETYPES_H
#define ARX_CORE_TIMETYPES_H

#include <algorithm>

#include <boost/config.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits.hpp>

#include <glm/gtc/constants.hpp>

#include "platform/Platform.h"


template <typename TAG, typename T>
struct DurationType
	: boost::totally_ordered1<DurationType<TAG, T>
	, boost::additive1<DurationType<TAG, T>
	> >
{
	
	T t;
	
	bool operator==(const DurationType & rhs) const {
		return t == rhs.t;
	}
	
	bool operator<(const DurationType & rhs) const {
		return t < rhs.t;
	}
	
	DurationType & operator+=(const DurationType & rhs) {
		t += rhs.t;
		return *this;
	}
	
	DurationType & operator-=(const DurationType & rhs) {
		t -= rhs.t;
		return *this;
	}
	
	static DurationType ofRaw(T value) {
		return DurationType(value, 0);
	}
	
private:
	
	explicit DurationType(const T t_, int /* disambiguate */) : t(t_) { }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	/* implicit */ DurationType(Zero /* zero */ = 0) : t(0) { }
	
};

template <typename TAG, typename T>
struct InstantType
	: boost::totally_ordered1<InstantType<TAG, T>
	, boost::additive2<InstantType<TAG, T>, DurationType<TAG, T>
	> >
{
	
	T t;
	
	bool operator==(const InstantType & rhs) const {
		return t == rhs.t;
	}
	bool operator<(const InstantType & rhs) const {
		return t < rhs.t;
	}
	
	InstantType & operator+=(DurationType<TAG, T> rhs) {
		t += rhs.t;
		return *this;
	}
	
	InstantType & operator-=(DurationType<TAG, T> rhs) {
		t -= rhs.t;
		return *this;
	}
	
	static InstantType ofRaw(T value) {
		return InstantType(value, 0);
	}
	
private:
	
	explicit InstantType(const T t_, int /* disambiguate */) : t(t_) { }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	/* implicit */ InstantType(Zero /* zero */ = 0) : t(0) { }
	
};

template <typename TAG, typename T>
inline DurationType<TAG, T> operator-(InstantType<TAG, T> a, InstantType<TAG, T> b) {
	return DurationType<TAG, T>::ofRaw(a.t - b.t);
}

template <typename TAG, typename T, class IntType>
inline DurationType<TAG, T> operator*(DurationType<TAG, T> a, IntType b) {
	ARX_STATIC_ASSERT(boost::is_integral<IntType>::value, "factor must be int type");
	return DurationType<TAG, T>::ofRaw(a.t * T(b));
}

template <typename TAG, typename T>
inline float operator/(DurationType<TAG, T> a, DurationType<TAG, T> b) {
	return float(a.t) / float(b.t);
}


template <typename TAG, typename T>
inline float timeWaveSaw(InstantType<TAG, T> t, DurationType<TAG, T> period) {
	return float(t.t % period.t) / float(period.t);
}
template <typename TAG, typename T>
inline bool timeWaveSquare(InstantType<TAG, T> t, DurationType<TAG, T> period) {
	return t.t % period.t >= period.t / 2;
}
template <typename TAG, typename T>
inline float timeWaveSin(InstantType<TAG, T> t, DurationType<TAG, T> period) {
	return std::sin(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}
template <typename TAG, typename T>
inline float timeWaveCos(InstantType<TAG, T> t, DurationType<TAG, T> period) {
	return std::cos(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}

// GameTime
// in microseconds
typedef InstantType <struct GameTime_TAG, s64> GameInstant;
typedef DurationType<struct GameTime_TAG, s64> GameDuration;

inline GameInstant GameInstantUs(s64 val) {
	return GameInstant::ofRaw(val);
}
inline GameInstant GameInstantMs(s64 val) {
	return GameInstant::ofRaw(val * 1000);
}

inline GameDuration GameDurationUs(s64 val) {
	return GameDuration::ofRaw(val);
}
inline GameDuration GameDurationMs(s64 val) {
	return GameDuration::ofRaw(val * 1000);
}
inline GameDuration GameDurationMsf(float val) {
	return GameDuration::ofRaw(s64(val * 1000.f));
}

inline s64 toUs(GameInstant val) {
	return val.t;
}
inline s64 toMsi(GameInstant val) {
	return val.t / 1000;
}
inline float toMsf(GameInstant val) {
	return float(val.t) / 1000.f;
}

inline s64 toUs(GameDuration val) {
	return val.t;
}
inline s64 toMsi(GameDuration val) {
	return val.t / 1000;
}
inline float toMsf(GameDuration val) {
	return float(val.t) / 1000.f;
}

// PlatformTime
// in microseconds
typedef InstantType <struct PlatformTime_TAG, s64> PlatformInstant;
typedef DurationType<struct PlatformTime_TAG, s64> PlatformDuration;

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

inline s64 toUs(PlatformInstant val) {
	return val.t;
}
inline s64 toMsi(PlatformInstant val) {
	return val.t / 1000;
}

inline s64 toUs(PlatformDuration val) {
	return val.t;
}
inline s64 toMsi(PlatformDuration val) {
	return val.t / 1000;
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
inline AnimationDuration toAnimationDuration(GameDuration val) {
	return AnimationDuration::ofRaw(val.t);
}
inline GameDuration toGameDuration(AnimationDuration val) {
	return GameDuration::ofRaw(val.t);
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
