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

#ifndef ARX_GRAPHICS_COLOR_H
#define ARX_GRAPHICS_COLOR_H

#include <algorithm>
#include <limits>
#include <type_traits>

#include <glm/glm.hpp>

#include "platform/Platform.h"

template <typename TAG, typename T>
struct IntegerColorType {
	
	T t;
	
	explicit constexpr IntegerColorType(const T t_) noexcept
		: t(t_)
	{ }
	
	constexpr IntegerColorType() noexcept
		: t()
	{ }
	
	[[nodiscard]] constexpr bool operator==(const IntegerColorType<TAG, T> & rhs) const noexcept {
		return t == rhs.t;
	}
	
};

typedef IntegerColorType<struct ColorBGR_TAG,  u32> ColorBGR;
typedef IntegerColorType<struct ColorRGB_TAG,  u32> ColorRGB;
typedef IntegerColorType<struct ColorRGBA_TAG, u32> ColorRGBA;
typedef IntegerColorType<struct ColorBGRA_TAG, u32> ColorBGRA;

static_assert(sizeof(ColorBGR) == sizeof(u32));
static_assert(sizeof(ColorRGB) == sizeof(u32));
static_assert(sizeof(ColorRGBA) == sizeof(u32));
static_assert(sizeof(ColorBGRA) == sizeof(u32));

constexpr ColorRGBA ColorRGBA_ZERO = ColorRGBA(0);


template <typename T>
class Color4;

/*!
 * Integer colors use the range [0, std::numeric_limits<T>::max()] and clamp values into that range.
 */
template <typename T>
struct ColorTraits {
	[[nodiscard]] static constexpr T max() noexcept {
		return std::numeric_limits<T>::max();
	}
	template <typename O>
	[[nodiscard]] static constexpr T clamp(O value) noexcept {
		return T(glm::clamp(value, O(0), O(max())));
	}
	template <typename O>
	[[nodiscard]] static constexpr T convert(O value) noexcept {
		return clamp(value * (max() / ColorTraits<O>::max()));
	}
	[[nodiscard]] static constexpr T convert(T value) { return value; }
};

/*!
 * Float colors use the range [0, 1.f] but don't clamp values.
 */
template <>
struct ColorTraits<float> {
	[[nodiscard]] static constexpr float max() noexcept {
		return 1.f;
	}
	template <typename O>
	[[nodiscard]] static constexpr float convert(O value) noexcept {
		return value * (max() / float(ColorTraits<O>::max()));
	}
	[[nodiscard]] static constexpr float convert(float value) noexcept {
		return value;
	}
};

// TODO add clamping to more color operations

/*!
 * A color with red, blue and green components.
 */
template <typename T>
class Color3 {
	
	typedef ColorTraits<u8> ByteTraits;
	
public:
	
	typedef T type;
	typedef ColorTraits<T> Traits;
	
	T r;
	T g;
	T b;
	
	static const Color3 black;
	static const Color3 white;
	static const Color3 red;
	static const Color3 blue;
	static const Color3 green;
	static const Color3 yellow;
	static const Color3 cyan;
	static const Color3 magenta;
	
	constexpr Color3() noexcept : r(T(0)), g(T(0)), b(T(0)) { }
	constexpr Color3(T _r, T _g, T _b) noexcept : r(_r), g(_g), b(_b) { }
	
	/*!
	 * Converts a color from a different type, clamping according to the color traits
	 */
	template <typename O, typename = std::enable_if_t<!std::is_same_v<T, O>>>
	explicit constexpr Color3(const Color3<O> & o) noexcept
		: r(Traits::convert(o.r))
		, g(Traits::convert(o.g))
		, b(Traits::convert(o.b))
	{ }
	
	[[nodiscard]] constexpr bool operator==(const Color3 & o) const noexcept {
		return (r == o.r && g == o.g && b == o.b);
	}
	
	[[nodiscard]] constexpr bool operator!=(const Color3 & o) const noexcept {
		return !(*this == o);
	}
	
	[[nodiscard]] static constexpr Color3 fromRGB(ColorRGB rgb) noexcept {
		return Color3(Traits::convert(u8(rgb.t)),
		              Traits::convert(u8(rgb.t >> 8)),
		              Traits::convert(u8(rgb.t >> 16)));
	}
	
	[[nodiscard]] static constexpr Color3 fromBGR(ColorBGR bgr) noexcept {
		return Color3(Traits::convert(u8(bgr.t >> 16)),
		              Traits::convert(u8(bgr.t >> 8)),
		              Traits::convert(u8(bgr.t)));
	}
	
	[[nodiscard]] constexpr ColorRGBA toRGB(u8 _a = ByteTraits::max()) const noexcept {
		return ColorRGBA(u32(ByteTraits::convert(r))
		                 | (u32(ByteTraits::convert(g)) << 8)
		                 | (u32(ByteTraits::convert(b)) << 16)
		                 | (u32(_a) << 24));
	}
	
	[[nodiscard]] constexpr ColorBGRA toBGR(u8 _a = ByteTraits::max()) const noexcept {
		return ColorBGRA(u32(ByteTraits::convert(b))
		                 | (u32(ByteTraits::convert(g)) << 8)
		                 | (u32(ByteTraits::convert(r)) << 16)
		                 | (u32(_a) << 24));
	}
	
	[[nodiscard]] constexpr static Color3 gray(float val) noexcept {
		T value = Traits::convert(val);
		return Color3(value, value, value);
	}
	
	[[nodiscard]] constexpr static Color3 rgb(float r, float g, float b) noexcept {
		return Color3(Traits::convert(r), Traits::convert(g), Traits::convert(b));
	}
	
	[[nodiscard]] constexpr Color3 operator*(float factor) const noexcept {
		return Color3(T(r * factor), T(g * factor), T(b * factor));
	}
	
	constexpr Color3 & operator+=(const Color3 & right) noexcept {
		r += right.r;
		g += right.g;
		b += right.b;
		return *this;
	}
	
	constexpr Color3 & operator*=(const Color3 & right) noexcept {
		r *= right.r;
		g *= right.g;
		b *= right.b;
		return *this;
	}
	
};

template <typename T>
const Color3<T> Color3<T>::black(T(0), T(0), T(0));
template <typename T>
const Color3<T> Color3<T>::white(ColorTraits<T>::max(), ColorTraits<T>::max(), ColorTraits<T>::max());
template <typename T>
const Color3<T> Color3<T>::red(ColorTraits<T>::max(), T(0), T(0));
template <typename T>
const Color3<T> Color3<T>::blue(T(0), T(0), ColorTraits<T>::max());
template <typename T>
const Color3<T> Color3<T>::green(T(0), ColorTraits<T>::max(), T(0));
template <typename T>
const Color3<T> Color3<T>::yellow(ColorTraits<T>::max(), ColorTraits<T>::max(), T(0));
template <typename T>
const Color3<T> Color3<T>::magenta(ColorTraits<T>::max(), T(0), ColorTraits<T>::max());
template <typename T>
const Color3<T> Color3<T>::cyan(T(0), ColorTraits<T>::max(), ColorTraits<T>::max());

/*!
 * A color with red, blue, green and alpha components.
 */
template <typename T>
class Color4 : public Color3<T> {
	
	typedef Color3<T> C3;
	typedef ColorTraits<u8> ByteTraits;
	
public:
	
	typedef ColorTraits<T> Traits;
	
	T a;
	
	static const Color4 black;
	static const Color4 white;
	static const Color4 red;
	static const Color4 blue;
	static const Color4 green;
	static const Color4 yellow;
	static const Color4 cyan;
	static const Color4 magenta;
	
	//! A fully transparent, black color.
	static const Color4 none;
	
	constexpr Color4() noexcept : C3(), a(T(0)) { }
	constexpr Color4(T _r, T _g, T _b, T _a = Traits::max()) noexcept : C3(_r, _g, _b), a(_a) { }
	/* implicit */ constexpr Color4(const C3 & o, T _a = Traits::max()) noexcept : C3(o), a(_a) { }
	
	/*!
	 * Converts a color from a different type, clamping according to the color traits
	 */
	template <typename O>
	explicit constexpr Color4(const Color3<O> & o, T alpha = Traits::max()) noexcept
		: C3(o)
		, a(alpha)
	{ }
	
	/*!
	 * Converts a color from a different type, clamping according to the color traits
	 */
	template <typename O, typename = std::enable_if_t<!std::is_same_v<T, O>>>
	explicit constexpr Color4(const Color4<O> & o) noexcept
		: C3(o)
		, a(Traits::convert(o.a))
	{ }
	
	constexpr Color4 & operator=(const C3 & o) noexcept {
		C3::operator=(o), a = Traits::max();
		return *this;
	}
	
	[[nodiscard]] constexpr bool operator==(const Color4 & o) const noexcept {
		return (C3::r == o.r && C3::g == o.g && C3::b == o.b && a == o.a);
	}
	
	[[nodiscard]] constexpr bool operator!=(const Color4 & o) const noexcept {
		return !(*this == o);
	}
	
	[[nodiscard]] constexpr ColorRGBA toRGBA() const noexcept {
		return C3::toRGB(ByteTraits::convert(a));
	}
	
	[[nodiscard]] constexpr ColorBGRA toBGRA() const noexcept {
		return C3::toBGR(ByteTraits::convert(a));
	}
	
	[[nodiscard]] static constexpr Color4 fromRGB(ColorRGB rgb, T a = Traits::max()) noexcept {
		return Color4(C3::fromRGB(rgb), a);
	}
	
	[[nodiscard]] static constexpr Color4 fromBGR(ColorBGR bgr, T a = Traits::max()) noexcept {
		return Color4(C3::fromBGR(bgr), a);
	}
	
	[[nodiscard]] static constexpr Color4 fromRGBA(ColorRGBA rgba) noexcept {
		return fromRGB(ColorRGB(rgba.t), Traits::convert(u8(rgba.t >> 24)));
	}
	
	[[nodiscard]] static constexpr Color4 fromBGRA(ColorBGRA bgra) noexcept {
		return fromBGR(ColorBGR(bgra.t), Traits::convert(u8(bgra.t >> 24)));
	}
	
	[[nodiscard]] static constexpr Color4 gray(float val, float a = ColorTraits<float>::max()) noexcept {
		return Color4(C3::gray(val), Traits::convert(a));
	}
	
	[[nodiscard]] static constexpr Color4 rgb(float r, float g, float b) noexcept {
		return Color4(C3::rgb(r, g, b));
	}
	
	[[nodiscard]] static constexpr Color4 rgba(float r, float g, float b, float a) noexcept {
		return Color4(C3::rgb(r, g, b), Traits::convert(a));
	}
	
	[[nodiscard]] constexpr Color4 operator*(float factor) const noexcept {
		return Color4(C3::operator*(factor), a);
	}
	
};

template <typename T>
const Color4<T> Color4<T>::black(T(0), T(0), T(0));
template <typename T>
const Color4<T> Color4<T>::white(ColorTraits<T>::max(), ColorTraits<T>::max(), ColorTraits<T>::max());
template <typename T>
const Color4<T> Color4<T>::red(ColorTraits<T>::max(), T(0), T(0));
template <typename T>
const Color4<T> Color4<T>::blue(T(0), T(0), ColorTraits<T>::max());
template <typename T>
const Color4<T> Color4<T>::green(T(0), ColorTraits<T>::max(), T(0));
template <typename T>
const Color4<T> Color4<T>::yellow(ColorTraits<T>::max(), ColorTraits<T>::max(), T(0));
template <typename T>
const Color4<T> Color4<T>::magenta(ColorTraits<T>::max(), T(0), ColorTraits<T>::max());
template <typename T>
const Color4<T> Color4<T>::cyan(T(0), ColorTraits<T>::max(), ColorTraits<T>::max());

template <typename T>
const Color4<T> Color4<T>::none(Color3<T>::black, T(0));

typedef Color3<float> Color3f;
typedef Color4<float> Color4f;
typedef Color4<u8> Color;

template <typename T>
[[nodiscard]] constexpr Color4<T> componentwise_min(Color4<T> c0, Color4<T> c1) noexcept {
	return Color4<T>(std::min(c0.r, c1.r), std::min(c0.g, c1.g), std::min(c0.b, c1.b),
	                 std::min(c0.a, c1.a));
}
template <typename T>
[[nodiscard]] constexpr Color4<T> componentwise_max(Color4<T> c0, Color4<T> c1) noexcept {
	return Color4<T>(std::max(c0.r, c1.r), std::max(c0.g, c1.g), std::max(c0.b, c1.b),
	                 std::max(c0.a, c1.a));
}
template <typename T>
[[nodiscard]] constexpr Color3<T> componentwise_min(Color3<T> c0, Color3<T> c1) noexcept {
	return Color3<T>(std::min(c0.r, c1.r), std::min(c0.g, c1.g), std::min(c0.b, c1.b));
}
template <typename T>
[[nodiscard]] constexpr Color3<T> componentwise_max(Color3<T> c0, Color3<T> c1) noexcept {
	return Color3<T>(std::max(c0.r, c1.r), std::max(c0.g, c1.g), std::max(c0.b, c1.b));
}

template <typename T>
[[nodiscard]] constexpr Color3<T> operator+(Color3<T> c0, Color3<T> c1) noexcept {
	return Color3<T>(c0.r + c1.r, c0.g + c1.g, c0.b + c1.b);
}
template <typename T>
[[nodiscard]] constexpr Color3<T> operator-(Color3<T> c0, Color3<T> c1) noexcept {
	return Color3<T>(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b);
}
template <typename T>
[[nodiscard]] constexpr Color3<T> operator*(Color3<T> c0, Color3<T> c1) noexcept {
	T m = ColorTraits<T>::max();
	return Color3<T>(c0.r * c1.r / m, c0.g * c1.g / m, c0.b * c1.b / m);
}
template <typename T>
[[nodiscard]] constexpr Color3<T> operator*(Color3<T> c0, float scale) noexcept {
	return Color3<T>(c0.r * scale, c0.g * scale, c0.b * scale);
}

template <typename T>
[[nodiscard]] constexpr Color4<T> operator+(Color4<T> c0, Color4<T> c1) noexcept {
	return Color4<T>(c0.r + c1.r, c0.g + c1.g, c0.b + c1.b, c0.a + c1.a);
}
template <typename T>
[[nodiscard]] constexpr Color4<T> operator-(Color4<T> c0, Color4<T> c1) noexcept {
	return Color4<T>(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b, c0.a - c1.a);
}
template <typename T>
[[nodiscard]] constexpr Color4<T> operator*(Color4<T> c0, Color4<T> c1) noexcept {
	T m = ColorTraits<T>::max();
	return Color4<T>(c0.r * c1.r / m, c0.g * c1.g / m, c0.b * c1.b / m, c0.a * c1.a / m);
}
template <typename T>
[[nodiscard]] constexpr Color4<T> operator*(Color4<T> c0, float scale) noexcept {
	return Color4<T>(c0.r * scale, c0.g * scale, c0.b * scale, c0.a * scale);
}

template <typename T>
[[nodiscard]] constexpr Color3<T> clamp(Color3<T> color, T min = 0, T max = ColorTraits<T>::max()) noexcept {
	using std::clamp;
	return Color3<T>(clamp(color.r, min, max), clamp(color.g, min, max), clamp(color.b, min, max));
}
template <typename T>
[[nodiscard]] constexpr Color4<T> clamp(Color4<T> color, T min = 0, T max = ColorTraits<T>::max()) noexcept {
	using std::clamp;
	return Color4<T>(clamp(Color3<T>(color.r, color.g, color.b), min, max), clamp(color.a, min, max));
}

#endif // ARX_GRAPHICS_COLOR_H
