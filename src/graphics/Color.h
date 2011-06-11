
#ifndef ARX_GRAPHICS_COLOR_H
#define ARX_GRAPHICS_COLOR_H

#include <limits>

#include "platform/Platform.h"

template<class T>
class Color4;

/*!
 * A color with red, blue and green components.
 */
template<class T>
class Color3 {
	
public:
	
	typedef T type;
	
	T r;
	T g;
	T b;
	
	const static T max;
	const static Color3 black;
	const static Color3 white;
	const static Color3 red;
	const static Color3 blue;
	const static Color3 green;
	const static Color3 yellow;
	const static Color3 cyan;
	const static Color3 magenta;
	
	inline Color3() { }
	inline Color3(T _r, T _g, T _b) : r(_r), g(_g), b(_b) { }
	inline Color3(const Color3 & o) : r(o.r), g(o.g), b(o.b) { }
	
	inline Color3 & operator=(const Color3 & o) {
		r = o.r, g = o.g, b = o.b;
		return *this;
	}
	
	inline static Color3 fromRGB(u32 rgb) {
		return Color3(value(rgb), value(rgb >> 8), value(rgb >> 16));
	}
	
	inline static Color3 fromBGR(u32 bgr) {
		return Color3(value(bgr >> 16), value(bgr >> 8), value(bgr));
	}
	
	inline u32 toRGB(u8 _a = max) const {
		return byteval(r) | (byteval(g) << 8) | (byteval(b) << 16) | (u32(_a) << 24);
	}
	
	inline u32 toBGR(u8 _a = max) const {
		return byteval(b) | (byteval(g) << 8) | (byteval(r) << 16) | (u32(_a) << 24);
	}
	
	inline static u32 byteval(T val) {
		return u32(val * (Color3<u8>::max / max));
	}
	
	inline static T value(u32 val) {
		return T(val & 0xff) * (max / Color3<u8>::max);
	}
	
	template<class O>
	inline Color4<O> to(O a = Color3<O>::max) const {
		return Color4<O>(scale<O>(r), scale<O>(g), scale<O>(b), a);
	}
	
	template<class O>
	inline static O scale(T val) {
		return O(val * (Color3<O>::max / max));
	}
	
};

template class Color3<float>;

template<class T>
const T Color3<T>::max = std::numeric_limits<T>::max();
template<>
const float Color3<float>::max = 1.f;

template<class T>
const Color3<T> Color3<T>::black(T(0), T(0), T(0));
template<class T>
const Color3<T> Color3<T>::white(Color3<T>::max, Color3<T>::max, Color3<T>::max);
template<class T>
const Color3<T> Color3<T>::red(Color3<T>::max, T(0), T(0));
template<class T>
const Color3<T> Color3<T>::blue(T(0), T(0), Color3<T>::max);
template<class T>
const Color3<T> Color3<T>::green(T(0), Color3<T>::max, T(0));
template<class T>
const Color3<T> Color3<T>::yellow(Color3<T>::max, Color3<T>::max, T(0));
template<class T>
const Color3<T> Color3<T>::cyan(Color3<T>::max, T(0), Color3<T>::max);
template<class T>
const Color3<T> Color3<T>::magenta(T(0), Color3<T>::max, Color3<T>::max);

/*!
 * A color with red, blue, green and alpha components.
 */
template<class T>
class Color4 : public Color3<T> {
	
	typedef Color3<T> C3;
	
public:
	
	T a;
	
	//! A fully transparent, black color.
	const static Color4 none;
	
	inline Color4() : C3() { }
	inline Color4(T _r, T _g, T _b, T _a = C3::max) : C3(_r, _g, _b), a(_a) { }
	inline Color4(const Color4 & o) : C3(o), a(o.a) { }
	inline Color4(const C3 & o, T _a = C3::max) : C3(o), a(_a) { }
	
	inline Color4 & operator=(const Color4 & o) {
		C3::operator=(o), a = o.a;
		return *this;
	}
	
	inline Color4 & operator=(const C3 & o) {
		C3::operator=(o), a = C3::max;
		return *this;
	}
	
	inline bool operator==(const Color4 & o) const {
		return (C3::r == o.r && C3::g == o.g && C3::b == o.b && a == o.a);
	}
	
	inline bool operator!=(const Color4 & o) const {
		return !(*this == o);
	}
	
	inline u32 toRGBA() const {
		return C3::toRGB((u8)C3::byteval(a));
	}
	
	inline u32 toBGRA() const {
		return C3::toBGR((u8)C3::byteval(a));
	}
	
	inline static Color4 fromRGB(u32 rgb, u8 a = C3::max) {
		return Color4(C3::fromRGB(rgb), a);
	}
	
	inline static Color4 fromBGR(u32 bgr, u8 a = C3::max) {
		return Color4(C3::fromBGR(bgr), a);
	}
	
	inline static Color4 fromRGBA(u32 rgb) {
		return fromRGB(rgb, C3::value(rgb >> 24));
	}
	
	inline static Color4 fromBGRA(u32 bgr) {
		return fromBGR(bgr, C3::value(bgr >> 24));
	}
	
	template<class O>
	inline Color4<O> to() const {
		return C3::to(C3::scale(a));
	}
	
};

template<class T>
const Color4<T> Color4<T>::none(Color3<T>::black, T(0));


typedef Color3<float> Color3f;
typedef Color4<float> Color4f;
typedef Color4<u8> Color;

#endif // ARX_GRAPHICS_COLOR_H
