/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <limits>
#include <algorithm>

#include "platform/Platform.h"

template <typename TAG, typename T>
struct IntegerColorType
{
	T t;
	
	explicit IntegerColorType(const T t_)
		: t(t_)
	{ }
	IntegerColorType()
		: t()
	{ }
	IntegerColorType(const IntegerColorType<TAG, T> & t_)
		: t(t_.t)
	{ }
	bool operator==(const IntegerColorType<TAG, T> & rhs) const {
		return t == rhs.t;
	}
	IntegerColorType<TAG, T> & operator=(const IntegerColorType<TAG, T> & rhs) {
		t = rhs.t;
		return *this;
	}
};

typedef IntegerColorType<struct ColorBGR_TAG,  u32> ColorBGR;
typedef IntegerColorType<struct ColorRGB_TAG,  u32> ColorRGB;
typedef IntegerColorType<struct ColorRGBA_TAG, u32> ColorRGBA;
typedef IntegerColorType<struct ColorBGRA_TAG, u32> ColorBGRA;

ARX_STATIC_ASSERT(sizeof(ColorBGR) == sizeof(u32), "");
ARX_STATIC_ASSERT(sizeof(ColorRGB) == sizeof(u32), "");
ARX_STATIC_ASSERT(sizeof(ColorRGBA) == sizeof(u32), "");
ARX_STATIC_ASSERT(sizeof(ColorBGRA) == sizeof(u32), "");

const ColorRGBA ColorRGBA_ZERO = ColorRGBA(0);


template <class T>
class Color4;

template <class T>
struct ColorLimits {
	static T max() { return std::numeric_limits<T>::max(); }
};
template <>
struct ColorLimits<float> {
	static float max() { return 1.f; }
};

/*!
 * A color with red, blue and green components.
 */
template <class T>
class Color3 {
	
public:
	
	typedef T type;
	typedef ColorLimits<T> Limits;
	
	T b;
	T g;
	T r;
	
	static const Color3 black;
	static const Color3 white;
	static const Color3 red;
	static const Color3 blue;
	static const Color3 green;
	static const Color3 yellow;
	static const Color3 cyan;
	static const Color3 magenta;
	
	Color3() : b(T(0)), g(T(0)), r(T(0)) { }
	Color3(T _r, T _g, T _b) : b(_b), g(_g), r(_r) { }
	Color3(const Color3 & o) : b(o.b), g(o.g), r(o.r) { }
	
	Color3 & operator=(const Color3 & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	static Color3 fromRGB(ColorRGB rgb) {
		return Color3(value(rgb.t), value(rgb.t >> 8), value(rgb.t >> 16));
	}
	
	static Color3 fromBGR(ColorBGR bgr) {
		return Color3(value(bgr.t >> 16), value(bgr.t >> 8), value(bgr.t));
	}
	
	ColorRGBA toRGB(u8 _a = ColorLimits<u8>::max()) const {
		return ColorRGBA(byteval(r) | (byteval(g) << 8) | (byteval(b) << 16) | (u32(_a) << 24));
	}
	
	ColorBGRA toBGR(u8 _a = ColorLimits<u8>::max()) const {
		return ColorBGRA(byteval(b) | (byteval(g) << 8) | (byteval(r) << 16) | (u32(_a) << 24));
	}
	
	static u32 byteval(T val) {
		return u32(val * (ColorLimits<u8>::max() / Limits::max()));
	}
	
	static T value(u32 val) {
		return T(val & 0xff) * (Limits::max() / ColorLimits<u8>::max());
	}
	
	template <class O>
	Color4<O> to(O a = ColorLimits<O>::max()) const {
		return Color4<O>(scale<O>(r), scale<O>(g), scale<O>(b), a);
	}
	
	template <class O>
	static O scale(T val) {
		return O(val * (ColorLimits<O>::max() / Limits::max()));
	}
	
	static Color3 grayb(u8 val) {
		T v = T(val * T(Limits::max() / ColorLimits<u8>::max()));
		return Color3(v, v, v);
	}
	
	static Color3 gray(float val) {
		val *= (Limits::max() / ColorLimits<float>::max());
		return Color3(T(val), T(val), T(val));
	}
	
	Color3 operator*(float factor) const {
		return Color3(r * factor, g * factor, b * factor);
	}

	Color3 & operator+=(const Color3 & right) {
		r += right.r;
		g += right.g;
		b += right.b;
		return *this;
	}

	Color3 & operator*=(const Color3 & right) {
		r *= right.r;
		g *= right.g;
		b *= right.b;
		return *this;
	}
	
};

template <class T>
const Color3<T> Color3<T>::black(T(0), T(0), T(0));
template <class T>
const Color3<T> Color3<T>::white(ColorLimits<T>::max(), ColorLimits<T>::max(), ColorLimits<T>::max());
template <class T>
const Color3<T> Color3<T>::red(ColorLimits<T>::max(), T(0), T(0));
template <class T>
const Color3<T> Color3<T>::blue(T(0), T(0), ColorLimits<T>::max());
template <class T>
const Color3<T> Color3<T>::green(T(0), ColorLimits<T>::max(), T(0));
template <class T>
const Color3<T> Color3<T>::yellow(ColorLimits<T>::max(), ColorLimits<T>::max(), T(0));
template <class T>
const Color3<T> Color3<T>::magenta(ColorLimits<T>::max(), T(0), ColorLimits<T>::max());
template <class T>
const Color3<T> Color3<T>::cyan(T(0), ColorLimits<T>::max(), ColorLimits<T>::max());

/*!
 * A color with red, blue, green and alpha components.
 */
template <class T>
class Color4 : public Color3<T> {
	
	typedef Color3<T> C3;
	
public:
	
	typedef ColorLimits<T> Limits;
	
	T a;
	
	//! A fully transparent, black color.
	static const Color4 none;
	
	Color4() : C3(), a(T(0)) { }
	Color4(T _r, T _g, T _b, T _a = Limits::max()) : C3(_r, _g, _b), a(_a) { }
	Color4(const Color4 & o) : C3(o), a(o.a) { }
	/* implicit */ Color4(const C3 & o, T _a = Limits::max()) : C3(o), a(_a) { }
	
	Color4 & operator=(const Color4 & o) {
		C3::operator=(o), a = o.a;
		return *this;
	}
	
	Color4 & operator=(const C3 & o) {
		C3::operator=(o), a = Limits::max();
		return *this;
	}
	
	bool operator==(const Color4 & o) const {
		return (C3::r == o.r && C3::g == o.g && C3::b == o.b && a == o.a);
	}
	
	bool operator!=(const Color4 & o) const {
		return !(*this == o);
	}
	
	ColorRGBA toRGBA() const {
		return C3::toRGB((u8)C3::byteval(a));
	}
	
	ColorBGRA toBGRA() const {
		return C3::toBGR((u8)C3::byteval(a));
	}
	
	static Color4 fromRGB(ColorRGB rgb, T a = Limits::max()) {
		return Color4(C3::fromRGB(rgb), a);
	}
	
	static Color4 fromBGR(ColorBGR bgr, T a = Limits::max()) {
		return Color4(C3::fromBGR(bgr), a);
	}
	
	static Color4 fromRGBA(ColorRGBA rgba) {
		return fromRGB(ColorRGB(rgba.t), C3::value(rgba.t >> 24));
	}
	
	static Color4 fromBGRA(ColorBGRA bgra) {
		return fromBGR(ColorBGR(bgra.t), C3::value(bgra.t >> 24));
	}
	
	template <class O>
	Color4<O> to() const {
		return C3::to(scale<O>(a));
	}
	
	template <class O>
	static O scale(T val) {
		return O(val * (ColorLimits<O>::max() / Limits::max()));
	}
	
	Color4 operator*(float factor) const {
		return Color4(C3::operator*(factor), a);
	}
	
};

template <class T>
const Color4<T> Color4<T>::none(Color3<T>::black, T(0));


typedef Color3<float> Color3f;
typedef Color4<float> Color4f;
typedef Color4<u8> Color;

template <typename T>
Color4<T> componentwise_min(Color4<T> c0, Color4<T> c1) {
	return Color4<T>(std::min(c0.r, c1.r), std::min(c0.g, c1.g), std::min(c0.b, c1.b),
	                 std::min(c0.a, c1.a));
}
template <typename T>
Color4<T> componentwise_max(Color4<T> c0, Color4<T> c1) {
	return Color4<T>(std::max(c0.r, c1.r), std::max(c0.g, c1.g), std::max(c0.b, c1.b),
	                 std::max(c0.a, c1.a));
}
template <typename T>
Color3<T> componentwise_min(Color3<T> c0, Color3<T> c1) {
	return Color3<T>(std::min(c0.r, c1.r), std::min(c0.g, c1.g), std::min(c0.b, c1.b));
}
template <typename T>
Color3<T> componentwise_max(Color3<T> c0, Color3<T> c1) {
	return Color3<T>(std::max(c0.r, c1.r), std::max(c0.g, c1.g), std::max(c0.b, c1.b));
}

template <typename T>
Color3<T> operator+(Color3<T> c0, Color3<T> c1) {
	return Color3<T>(c0.r + c1.r, c0.g + c1.g, c0.b + c1.b);
}
template <typename T>
Color3<T> operator-(Color3<T> c0, Color3<T> c1) {
	return Color3<T>(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b);
}
template <typename T>
Color3<T> operator*(Color3<T> c0, Color3<T> c1) {
	T m = ColorLimits<T>::max();
	return Color3<T>(c0.r * c1.r / m, c0.g * c1.g / m, c0.b * c1.b / m);
}
template <typename T>
Color3<T> operator*(Color3<T> c0, float scale) {
	return Color3<T>(c0.r * scale, c0.g * scale, c0.b * scale);
}

template <typename T>
Color4<T> operator+(Color4<T> c0, Color4<T> c1) {
	return Color4<T>(c0.r + c1.r, c0.g + c1.g, c0.b + c1.b, c0.a + c1.a);
}
template <typename T>
Color4<T> operator-(Color4<T> c0, Color4<T> c1) {
	return Color4<T>(c0.r - c1.r, c0.g - c1.g, c0.b - c1.b, c0.a - c1.a);
}
template <typename T>
Color4<T> operator*(Color4<T> c0, Color4<T> c1) {
	T m = ColorLimits<T>::max();
	return Color4<T>(c0.r * c1.r / m, c0.g * c1.g / m, c0.b * c1.b / m, c0.a * c1.a / m);
}
template <typename T>
Color4<T> operator*(Color4<T> c0, float scale) {
	return Color4<T>(c0.r * scale, c0.g * scale, c0.b * scale, c0.a * scale);
}

#endif // ARX_GRAPHICS_COLOR_H
