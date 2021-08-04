/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <cmath>
#include <algorithm>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/operators.hpp>

#include <glm/gtc/constants.hpp>

#include "platform/Platform.h"


template <typename Tag, typename T>
struct DurationType
	: boost::totally_ordered1<DurationType<Tag, T>, boost::additive1<DurationType<Tag, T>>>
{
	
	T t;
	
	[[nodiscard]] constexpr bool operator==(const DurationType & rhs) const noexcept {
		return t == rhs.t;
	}
	
	[[nodiscard]] constexpr bool operator<(const DurationType & rhs) const noexcept {
		return t < rhs.t;
	}
	
	constexpr DurationType & operator+=(const DurationType & rhs) noexcept {
		t += rhs.t;
		return *this;
	}
	
	constexpr DurationType & operator-=(const DurationType & rhs) noexcept {
		t -= rhs.t;
		return *this;
	}
	
	[[nodiscard]] static constexpr DurationType ofRaw(T value) noexcept {
		return DurationType(value, 0);
	}
	
private:
	
	[[nodiscard]] explicit constexpr DurationType(const T t_, int /* disambiguate */) noexcept : t(t_) { }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	[[nodiscard]] /* implicit */ constexpr DurationType(Zero /* zero */ = 0) noexcept : t(0) { }
	
	template <typename NullPtr, typename = std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<NullPtr>>, decltype(nullptr)>>
	>
	/* implicit */ DurationType(NullPtr /* nullptr */) = delete;
	
};

template <typename Tag, typename T>
struct InstantType
	: boost::totally_ordered1<InstantType<Tag, T>, boost::additive2<InstantType<Tag, T>, DurationType<Tag, T>>>
{
	
	T t;
	
	[[nodiscard]] constexpr bool operator==(const InstantType & rhs) const noexcept {
		return t == rhs.t;
	}
	[[nodiscard]] constexpr bool operator<(const InstantType & rhs) const noexcept {
		return t < rhs.t;
	}
	
	constexpr InstantType & operator+=(DurationType<Tag, T> rhs) noexcept {
		t += rhs.t;
		return *this;
	}
	
	constexpr InstantType & operator-=(DurationType<Tag, T> rhs) noexcept {
		t -= rhs.t;
		return *this;
	}
	
	[[nodiscard]] static constexpr InstantType ofRaw(T value) noexcept {
		return InstantType(value, 0);
	}
	
private:
	
	[[nodiscard]] explicit constexpr InstantType(const T t_, int /* disambiguate */) noexcept : t(t_) { }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	/* implicit */ InstantType(Zero /* zero */ = 0) : t(0) { }
	
	template <typename NullPtr, typename = std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<NullPtr>>, decltype(nullptr)>>
	>
	/* implicit */ InstantType(NullPtr /* nullptr */) = delete;
	
};

template <typename Tag, typename T>
[[nodiscard]] inline constexpr DurationType<Tag, T> operator-(InstantType<Tag, T> a, InstantType<Tag, T> b) noexcept {
	return DurationType<Tag, T>::ofRaw(a.t - b.t);
}

template <typename Tag, typename T, class IntType>
[[nodiscard]] inline constexpr DurationType<Tag, T> operator*(DurationType<Tag, T> a, IntType b) noexcept {
	static_assert(std::is_integral<IntType>::value, "factor must be int type");
	return DurationType<Tag, T>::ofRaw(a.t * T(b));
}

template <typename Tag, typename T>
[[nodiscard]] inline constexpr float operator/(DurationType<Tag, T> a, DurationType<Tag, T> b) noexcept {
	return float(a.t) / float(b.t);
}


template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveSaw(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return float(t.t % period.t) / float(period.t);
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr bool timeWaveSquare(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return t.t % period.t >= period.t / 2;
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveSin(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return std::sin(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveCos(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return std::cos(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}

// GameTime
// in microseconds
typedef InstantType <struct GameTime_Tag, s64> GameInstant;
typedef DurationType<struct GameTime_Tag, s64> GameDuration;

[[nodiscard]] inline constexpr GameInstant GameInstantUs(s64 val) noexcept {
	return GameInstant::ofRaw(val);
}
[[nodiscard]] inline constexpr GameInstant GameInstantMs(s64 val) noexcept {
	return GameInstant::ofRaw(val * 1000);
}

[[nodiscard]] inline constexpr GameDuration GameDurationUs(s64 val) noexcept {
	return GameDuration::ofRaw(val);
}
[[nodiscard]] inline constexpr GameDuration GameDurationMs(s64 val) noexcept {
	return GameDuration::ofRaw(val * 1000);
}
[[nodiscard]] inline constexpr GameDuration GameDurationMsf(float val) noexcept {
	return GameDuration::ofRaw(s64(val * 1000.f));
}

[[nodiscard]] inline constexpr s64 toUs(GameInstant val) noexcept {
	return val.t;
}
[[nodiscard]] inline constexpr s64 toMsi(GameInstant val) noexcept {
	return val.t / 1000;
}
[[nodiscard]] inline constexpr float toMsf(GameInstant val) noexcept {
	return float(val.t) / 1000.f;
}

[[nodiscard]] inline constexpr s64 toUs(GameDuration val) noexcept {
	return val.t;
}
[[nodiscard]] inline constexpr s64 toMsi(GameDuration val) noexcept {
	return val.t / 1000;
}
[[nodiscard]] inline constexpr float toMsf(GameDuration val) noexcept {
	return float(val.t) / 1000.f;
}

// PlatformTime
// in microseconds
typedef InstantType <struct PlatformTimeTag, s64> PlatformInstant;
typedef DurationType<struct PlatformTimeTag, s64> PlatformDuration;

[[nodiscard]] inline constexpr PlatformInstant PlatformInstantUs(s64 val) noexcept {
	return PlatformInstant::ofRaw(val);
}
[[nodiscard]] inline constexpr PlatformInstant PlatformInstantMs(s64 val) noexcept {
	return PlatformInstant::ofRaw(val * 1000);
}

[[nodiscard]] inline constexpr PlatformDuration PlatformDurationUs(s64 val) noexcept {
	return PlatformDuration::ofRaw(val);
}
[[nodiscard]] inline constexpr PlatformDuration PlatformDurationMs(s64 val) noexcept {
	return PlatformDuration::ofRaw(val * 1000);
}
[[nodiscard]] inline constexpr PlatformDuration PlatformDurationMsf(float val) noexcept {
	return PlatformDuration::ofRaw(s64(val * 1000.f));
}

[[nodiscard]] inline constexpr s64 toUs(PlatformInstant val) noexcept {
	return val.t;
}
[[nodiscard]] inline constexpr s64 toMsi(PlatformInstant val) noexcept {
	return val.t / 1000;
}

[[nodiscard]] inline constexpr s64 toUs(PlatformDuration val) noexcept {
	return val.t;
}
[[nodiscard]] inline constexpr s64 toMsi(PlatformDuration val) noexcept {
	return val.t / 1000;
}
[[nodiscard]] inline constexpr float toMs(PlatformDuration val) noexcept {
	return float(val.t) / (1000.f);
}
[[nodiscard]] inline constexpr float toS(PlatformDuration val) noexcept {
	return float(val.t) / (1000.f * 1000.f);
}

// AnimationTime
// in microseconds
typedef DurationType<struct AnimationTimeTag, s64> AnimationDuration;

[[nodiscard]] inline constexpr AnimationDuration operator*(AnimationDuration v, float scalar) noexcept {
	return AnimationDuration::ofRaw(s64(float(v.t) * scalar));
}

[[nodiscard]] inline constexpr AnimationDuration AnimationDurationUs(s64 val) noexcept {
	return AnimationDuration::ofRaw(val);
}
[[nodiscard]] inline constexpr AnimationDuration AnimationDurationMs(s64 val) noexcept {
	return AnimationDuration::ofRaw(val * 1000);
}
[[nodiscard]] inline constexpr AnimationDuration AnimationDurationMsf(float val) noexcept {
	return AnimationDuration::ofRaw(s64(val * 1000.f));
}

[[nodiscard]] inline constexpr s64 toMsi(AnimationDuration val) noexcept {
	return val.t / (1000);
}
[[nodiscard]] inline constexpr float toMsf(AnimationDuration val) noexcept {
	return float(val.t) / (1000.f);
}
[[nodiscard]] inline constexpr float toS(AnimationDuration val) noexcept {
	return float(val.t) / (1000.f * 1000.f);
}

[[nodiscard]] inline constexpr AnimationDuration toAnimationDuration(PlatformDuration val) noexcept {
	return AnimationDuration::ofRaw(val.t);
}
[[nodiscard]] inline constexpr AnimationDuration toAnimationDuration(GameDuration val) noexcept {
	return AnimationDuration::ofRaw(val.t);
}
[[nodiscard]] inline constexpr GameDuration toGameDuration(AnimationDuration val) noexcept {
	return GameDuration::ofRaw(val.t);
}
[[nodiscard]] inline constexpr PlatformDuration toPlatformDuration(AnimationDuration val) noexcept {
	return PlatformDuration::ofRaw(val.t);
}

namespace arx {

template <typename T>
[[nodiscard]] constexpr T clamp(T const & x, T const & minVal, T const & maxVal) noexcept {
	return std::min(maxVal, std::max(minVal, x));
}

} // namespace arx

#endif // ARX_CORE_TIMETYPES_H
