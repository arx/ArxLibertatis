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

template<class T>
class Color4;

template<class T>
struct ColorLimits {
	inline static T max() { return std::numeric_limits<T>::max(); }
};
template<>
struct ColorLimits<float> {
	inline static float max() { return 1.f; }
};

typedef u32 ColorBGR;
typedef u32 ColorRGB;
typedef u32 ColorRGBA;
typedef u32 ColorBGRA;

/*!
 * A color with red, blue and green components.
 */
template<class T>
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
	
	inline Color3() { }
	inline Color3(T _r, T _g, T _b) : b(_b), g(_g), r(_r) { }
	inline Color3(const Color3 & o) : b(o.b), g(o.g), r(o.r) { }
	
	inline Color3 & operator=(const Color3 & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	inline static Color3 fromRGB(ColorRGB rgb) {
		return Color3(value(rgb), value(rgb >> 8), value(rgb >> 16));
	}
	
	inline static Color3 fromBGR(ColorBGR bgr) {
		return Color3(value(bgr >> 16), value(bgr >> 8), value(bgr));
	}
	
	inline ColorRGBA toRGB(u8 _a = Limits::max()) const {
		return byteval(r) | (byteval(g) << 8) | (byteval(b) << 16) | (u32(_a) << 24);
	}
	
	inline ColorBGRA toBGR(u8 _a = Limits::max()) const {
		return byteval(b) | (byteval(g) << 8) | (byteval(r) << 16) | (u32(_a) << 24);
	}
	
	inline static u32 byteval(T val) {
		return u32(val * (ColorLimits<u8>::max() / Limits::max()));
	}
	
	inline static T value(u32 val) {
		return T(val & 0xff) * (Limits::max() / ColorLimits<u8>::max());
	}
	
	template<class O>
	inline Color4<O> to(O a = ColorLimits<O>::max()) const {
		return Color4<O>(scale<O>(r), scale<O>(g), scale<O>(b), a);
	}
	
	template<class O>
	inline static O scale(T val) {
		return O(val * (ColorLimits<O>::max() / Limits::max()));
	}
	
	inline static Color3 grayb(u8 val) {
		T v = T(val * T(Limits::max() / ColorLimits<u8>::max()));
		return Color3(v, v, v);
	}
	
	inline static Color3 gray(float val) {
		val *= (Limits::max() / ColorLimits<float>::max());
		return Color3(T(val), T(val), T(val));
	}
	
	inline Color3 operator*(float factor) const {
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

template<class T>
const Color3<T> Color3<T>::black(T(0), T(0), T(0));
template<class T>
const Color3<T> Color3<T>::white(ColorLimits<T>::max(), ColorLimits<T>::max(), ColorLimits<T>::max());
template<class T>
const Color3<T> Color3<T>::red(ColorLimits<T>::max(), T(0), T(0));
template<class T>
const Color3<T> Color3<T>::blue(T(0), T(0), ColorLimits<T>::max());
template<class T>
const Color3<T> Color3<T>::green(T(0), ColorLimits<T>::max(), T(0));
template<class T>
const Color3<T> Color3<T>::yellow(ColorLimits<T>::max(), ColorLimits<T>::max(), T(0));
template<class T>
const Color3<T> Color3<T>::magenta(ColorLimits<T>::max(), T(0), ColorLimits<T>::max());
template<class T>
const Color3<T> Color3<T>::cyan(T(0), ColorLimits<T>::max(), ColorLimits<T>::max());

/*!
 * A color with red, blue, green and alpha components.
 */
template<class T>
class Color4 : public Color3<T> {
	
	typedef Color3<T> C3;
	
public:
	
	typedef ColorLimits<T> Limits;
	
	T a;
	
	//! A fully transparent, black color.
	static const Color4 none;
	
	inline Color4() : C3() { }
	inline Color4(T _r, T _g, T _b, T _a = Limits::max()) : C3(_r, _g, _b), a(_a) { }
	inline Color4(const Color4 & o) : C3(o), a(o.a) { }
	inline Color4(const C3 & o, T _a = Limits::max()) : C3(o), a(_a) { }
	
	inline Color4 & operator=(const Color4 & o) {
		C3::operator=(o), a = o.a;
		return *this;
	}
	
	inline Color4 & operator=(const C3 & o) {
		C3::operator=(o), a = Limits::max();
		return *this;
	}
	
	inline bool operator==(const Color4 & o) const {
		return (C3::r == o.r && C3::g == o.g && C3::b == o.b && a == o.a);
	}
	
	inline bool operator!=(const Color4 & o) const {
		return !(*this == o);
	}
	
	inline ColorRGBA toRGBA() const {
		return C3::toRGB((u8)C3::byteval(a));
	}
	
	inline ColorBGRA toBGRA() const {
		return C3::toBGR((u8)C3::byteval(a));
	}
	
	inline static Color4 fromRGB(ColorRGB rgb, u8 a = Limits::max()) {
		return Color4(C3::fromRGB(rgb), a);
	}
	
	inline static Color4 fromBGR(ColorBGR bgr, u8 a = Limits::max()) {
		return Color4(C3::fromBGR(bgr), a);
	}
	
	inline static Color4 fromRGBA(ColorRGBA rgba) {
		return fromRGB(rgba, C3::value(rgba >> 24));
	}
	
	inline static Color4 fromBGRA(ColorBGRA bgra) {
		return fromBGR(bgra, C3::value(bgra >> 24));
	}
	
	template<class O>
	inline Color4<O> to() const {
		return C3::to(scale<O>(a));
	}
	
	template<class O>
	inline static O scale(T val) {
		return O(val * (ColorLimits<O>::max() / Limits::max()));
	}
	
	inline Color4 operator*(float factor) const {
		return Color4(C3::operator*(factor), a);
	}
	
};

template<class T>
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
Color3<T> operator*(Color3<T> c0, Color3<T> c1) {
	return Color3<T>(c0.r * c1.r, c0.g * c1.g, c0.b * c1.b);
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
	return Color4<T>(c0.r * c1.r, c0.g * c1.g, c0.b * c1.b, c0.a * c1.a);
}
template <typename T>
Color4<T> operator*(Color4<T> c0, float scale) {
	return Color4<T>(c0.r * scale, c0.g * scale, c0.b * scale, c0.a * scale);
}

#endif // ARX_GRAPHICS_COLOR_H
