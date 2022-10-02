/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <chrono>
#include <limits>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/operators.hpp>

#include <glm/gtc/constants.hpp>

#include "graphics/Math.h"
#include "platform/Platform.h"


template <typename Tag, typename T>
class DurationType : boost::totally_ordered1<DurationType<Tag, T>> {
	
	T m_value;
	
	explicit constexpr DurationType(const T duration, int /* disambiguate */) noexcept
		: m_value(duration)
	{ }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	[[nodiscard]] constexpr bool operator==(const DurationType & rhs) const noexcept {
		return m_value == rhs.m_value;
	}
	
	[[nodiscard]] constexpr bool operator<(const DurationType & rhs) const noexcept {
		return m_value < rhs.m_value;
	}
	
	[[nodiscard]] constexpr float operator/(DurationType b) const noexcept {
		return float(m_value) / float(b.m_value);
	}
	
	constexpr DurationType & operator+=(const DurationType & rhs) noexcept {
		m_value += rhs.m_value;
		return *this;
	}
	
	constexpr DurationType & operator-=(const DurationType & rhs) noexcept {
		m_value -= rhs.m_value;
		return *this;
	}
	
	[[nodiscard]] constexpr DurationType operator+(DurationType rhs) const noexcept {
		return ofRaw(m_value + rhs.m_value);
	}
	
	[[nodiscard]] constexpr DurationType operator-(DurationType rhs) const noexcept {
		return ofRaw(m_value - rhs.m_value);
	}
	
	template <class Type, typename = std::enable_if_t<std::is_integral_v<Type> || std::is_same_v<Type, float>>>
	[[nodiscard]] constexpr DurationType operator*(Type rhs) const noexcept {
		return value() * rhs;
	}
	
	template <class Type, typename = std::enable_if_t<std::is_integral_v<Type> || std::is_same_v<Type, float>>>
	[[nodiscard]] constexpr DurationType operator/(Type rhs) const noexcept {
		return value() / rhs;
	}
	
	[[nodiscard]] static constexpr DurationType max() noexcept {
		return ofRaw(std::numeric_limits<T>::max());
	}
	
	[[nodiscard]] static constexpr DurationType ofRaw(T value) noexcept {
		return DurationType(value, 0);
	}
	
	[[nodiscard]] constexpr std::chrono::microseconds value() const noexcept {
		return std::chrono::microseconds(m_value);
	}
	
	template <typename Rep, typename Period>
	[[nodiscard]] explicit constexpr operator std::chrono::duration<Rep, Period>() const noexcept {
		return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(value());
	}
	
	template <typename Rep, typename Period>
	/* implicit */ constexpr DurationType(std::chrono::duration<Rep, Period> duration) noexcept
		: m_value(checked_range_cast<T>(std::chrono::duration_cast<std::chrono::microseconds>(duration).count()))
	{ }
	
	/* implicit */ constexpr DurationType(Zero /* zero */ = 0) noexcept : m_value(0) { }
	
	template <typename NullPtr, typename = std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<NullPtr>>, decltype(nullptr)>
	>>
	/* implicit */ DurationType(NullPtr /* nullptr */) = delete;
	
	// TODO C++20 use conditional explicit to allow implicit non-narrowing conversions
	template <typename T2>
	[[nodiscard]] explicit constexpr operator DurationType<Tag, T2>() const noexcept {
		return DurationType<Tag, T2>::ofRaw(checked_range_cast<T2>(m_value));
	}
	
};

template <typename Rep, typename Period, typename Tag, typename T>
[[nodiscard]] inline constexpr DurationType<Tag, T> operator-(std::chrono::duration<Rep, Period> a,
                                                              DurationType<Tag, T> b) noexcept {
	return DurationType<Tag, T>(a) - b;
}

template <typename Tag, typename T>
struct InstantType : boost::totally_ordered1<InstantType<Tag, T>> {
	
	T t;
	
	[[nodiscard]] constexpr bool operator==(const InstantType & rhs) const noexcept {
		return t == rhs.t;
	}
	[[nodiscard]] constexpr bool operator<(const InstantType & rhs) const noexcept {
		return t < rhs.t;
	}
	
	constexpr InstantType & operator+=(DurationType<Tag, T> rhs) noexcept {
		t += std::chrono::microseconds(rhs).count();
		return *this;
	}
	
	constexpr InstantType & operator-=(DurationType<Tag, T> rhs) noexcept {
		t -= std::chrono::microseconds(rhs).count();
		return *this;
	}
	
	[[nodiscard]] constexpr InstantType operator+(DurationType<Tag, T> rhs) const noexcept {
		return ofRaw(t + std::chrono::microseconds(rhs).count());
	}
	
	[[nodiscard]] constexpr InstantType operator-(DurationType<Tag, T> rhs) const noexcept {
		return ofRaw(t - std::chrono::microseconds(rhs).count());
	}
	
	[[nodiscard]] inline constexpr DurationType<Tag, T> operator-(InstantType rhs) const noexcept {
		return std::chrono::microseconds(t - rhs.t);
	}
	
	[[nodiscard]] static constexpr InstantType ofRaw(T value) noexcept {
		return InstantType(value, 0);
	}
	
private:
	
	explicit constexpr InstantType(const T t_, int /* disambiguate */) noexcept : t(t_) { }
	
	class ZeroType;
	typedef ZeroType ** Zero;
	
public:
	
	/* implicit */ constexpr InstantType(Zero /* zero */ = 0) noexcept : t(0) { }
	
	template <typename NullPtr, typename = std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<NullPtr>>, decltype(nullptr)>
	>>
	/* implicit */ InstantType(NullPtr /* nullptr */) = delete;
	
};

template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveSaw(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return float(t.t % std::chrono::microseconds(period).count()) / float(std::chrono::microseconds(period).count());
}
template <typename Tag, typename T, typename Rep, typename Period>
[[nodiscard]] inline constexpr float timeWaveSaw(InstantType<Tag, T> t, std::chrono::duration<Rep, Period> period) noexcept {
	return timeWaveSaw(t, DurationType<Tag, T>(period));
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr bool timeWaveSquare(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return t.t % std::chrono::microseconds(period).count() >= std::chrono::microseconds(period).count() / 2;
}
template <typename Tag, typename T, typename Rep, typename Period>
[[nodiscard]] inline constexpr bool timeWaveSquare(InstantType<Tag, T> t, std::chrono::duration<Rep, Period> period) noexcept {
	return timeWaveSquare(t, DurationType<Tag, T>(period));
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveSin(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return std::sin(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}
template <typename Tag, typename T, typename Rep, typename Period>
[[nodiscard]] inline constexpr float timeWaveSin(InstantType<Tag, T> t, std::chrono::duration<Rep, Period> period) noexcept {
	return timeWaveSin(t, DurationType<Tag, T>(period));
}
template <typename Tag, typename T>
[[nodiscard]] inline constexpr float timeWaveCos(InstantType<Tag, T> t, DurationType<Tag, T> period) noexcept {
	return std::cos(timeWaveSaw(t, period) * 2.f * glm::pi<float>());
}
template <typename Tag, typename T, typename Rep, typename Period>
[[nodiscard]] inline constexpr float timeWaveCos(InstantType<Tag, T> t, std::chrono::duration<Rep, Period> period) noexcept {
	return timeWaveCos(t, DurationType<Tag, T>(period));
}

template <typename Tag, typename T>
[[nodiscard]] float toMsf(DurationType<Tag, T> duration) noexcept {
	return std::chrono::duration<float, std::milli>(duration).count();
}

// GameTime
// in microseconds
typedef InstantType <struct GameTime_Tag, s64> GameInstant;
typedef DurationType<struct GameTime_Tag, s64> GameDuration;
typedef DurationType<struct GameTime_Tag, s32> ShortGameDuration;

[[nodiscard]] inline constexpr s64 toUs(GameDuration val) noexcept {
	return std::chrono::microseconds(val).count();
}
[[nodiscard]] inline constexpr s64 toMsi(GameDuration val) noexcept {
	return std::chrono::milliseconds(val).count();
}

[[nodiscard]] inline constexpr s64 toUs(GameInstant val) noexcept {
	return toUs(val - GameInstant(0));
}
[[nodiscard]] inline constexpr s64 toMsi(GameInstant val) noexcept {
	return toMsi(val - GameInstant(0));
}

// PlatformTime
// in microseconds
typedef InstantType <struct PlatformTimeTag, s64> PlatformInstant;
typedef DurationType<struct PlatformTimeTag, s64> PlatformDuration;

[[nodiscard]] inline constexpr s64 toUs(PlatformDuration val) noexcept {
	return std::chrono::microseconds(val).count();
}
[[nodiscard]] inline constexpr s64 toMsi(PlatformDuration val) noexcept {
	return std::chrono::milliseconds(val).count();
}
[[nodiscard]] inline constexpr float toMs(PlatformDuration val) noexcept {
	return std::chrono::duration<float, std::milli>(val).count();
}
[[nodiscard]] inline constexpr float toS(PlatformDuration val) noexcept {
	return std::chrono::duration<float>(val).count();
}

[[nodiscard]] inline constexpr s64 toUs(PlatformInstant val) noexcept {
	return toUs(val - PlatformInstant(0));
}
[[nodiscard]] inline constexpr s64 toMsi(PlatformInstant val) noexcept {
	return toMsi(val - PlatformInstant(0));
}

// AnimationTime
// in microseconds
typedef DurationType<struct AnimationTimeTag, s64> AnimationDuration;

[[nodiscard]] inline constexpr s64 toMsi(AnimationDuration val) noexcept {
	return std::chrono::milliseconds(val).count();
}
[[nodiscard]] inline constexpr float toS(AnimationDuration val) noexcept {
	return std::chrono::duration<float>(val).count();
}

[[nodiscard]] inline constexpr AnimationDuration toAnimationDuration(PlatformDuration val) noexcept {
	return val.value();
}
[[nodiscard]] inline constexpr AnimationDuration toAnimationDuration(GameDuration val) noexcept {
	return val.value();
}
[[nodiscard]] inline constexpr GameDuration toGameDuration(AnimationDuration val) noexcept {
	return val.value();
}
[[nodiscard]] inline constexpr PlatformDuration toPlatformDuration(AnimationDuration val) noexcept {
	return val.value();
}

namespace arx {

template <typename Tag, typename T>
[[nodiscard]] constexpr DurationType<Tag, T> clamp(DurationType<Tag, T> x,
                                                   DurationType<Tag, T> minVal,
                                                   DurationType<Tag, T> maxVal) noexcept {
	return std::min(maxVal, std::max(minVal, x));
}

template <typename Tag, typename T, typename Rep0, typename Period0, typename Rep1, typename Period1>
[[nodiscard]] constexpr DurationType<Tag, T> clamp(DurationType<Tag, T> x,
                                                   std::chrono::duration<Rep0, Period0> minVal,
                                                   std::chrono::duration<Rep1, Period1> maxVal) noexcept {
	return std::min(DurationType<Tag, T>(maxVal), std::max(DurationType<Tag, T>(minVal), x));
}

} // namespace arx

#if defined(__GNUC__) && __GNUC__ == 7
#pragma GCC diagnostic ignored "-Wliteral-suffix"
#endif
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4455)
#endif
using std::chrono_literals::operator""h;
using std::chrono_literals::operator""min;
using std::chrono_literals::operator""s;
using std::chrono_literals::operator""ms;
using std::chrono_literals::operator""us;
using std::chrono_literals::operator""ns;
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#if defined(__GNUC__) && __GNUC__ == 7
#pragma GCC diagnostic pop
#endif

#endif // ARX_CORE_TIMETYPES_H
