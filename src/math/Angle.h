/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_ANGLE_H
#define ARX_MATH_ANGLE_H

#include <limits>
#include <cmath>

#include "math/MathFwd.h"

/*!
 * A 3-dimensional euler-angle.
 * @brief 3x1 Vector class.
 */
template <class T>
class Angle {
	
public:
	
	/*!
	 * Constructor.
	 */
	Angle() {}
	
	/*!
	 * Constructor accepting initial values.
	 */
	Angle(T _a, T _b, T _g) : a(_a), b(_b), g(_g) { }
	
	/*!
	 * Copy constructor.
	 * @param other An angle to be copied.
	 */
	Angle(const Angle & other) : a(other.a), b(other.b), g(other.g) { }
	
	/*!
	 * Set this angle to the content of another angle.
	 * @brief Assignement operator.
	 * @param other An euler angle to be copied.
	 * @return Reference to this object.
	 */
	Angle & operator=(const Angle & other) {
		a = other.a, b = other.b, g = other.g;
		return *this;
	}
	
	/*!
	 * Test if this angle is equal to another angle.
	 * @brief Equal operator.
	 * @param other An euler angle to be compared to.
	 * @return A boolean, \b true if the two angles are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Angle & other) const {
		return (a == other.a && b == other.b && g == other.g);
	}
	
	/*!
	 * Test if this angle is not equal to another angle.
	 * @brief Not equal operator.
	 * @param other An angle to be compared to.
	 * @return A boolean, \b true if the two angles are not equal(some members are not equal), or \b false otherwise.
	 */
	bool operator!=(const Angle & other) const {
		return !((*this) == other);
	}
	
	/*!
	 * Invert the sign of the angle.
	 * @brief Unary minus operator.
	 * @return A new angle, same as this one but with the signs of all the elements inverted.
	 */
	Angle operator-() const {
		return Angle(-a, -b, -g);
	}
	
	/*!
	 * Add an angle to this angle.
	 * @brief Addition operator.
	 * @param other an angle, to be added to this angle.
	 * @return A new angle, the result of the addition of the two angles.
	 */
	Angle operator+(const Angle & other) const {
		return Angle(a + other.a, b + other.b, g + other.g);
	}
	
	/*!
	 * Substract an angle to this angle.
	 * @brief Substraction operator.
	 * @param other an angle, to be substracted to this angle.
	 * @return A new angle, the result of the substraction of the two angles.
	 */
	Angle operator-(const Angle & other) const {
		return Angle(a - other.a, b - other.b, g - other.g);
	}
	
	Angle operator*(T scale) const {
		return Angle(a * scale, b * scale, g * scale);
	}
	
	const Angle & operator+=(const Angle & other) {
		a += other.a, b += other.b, g += other.g;
		return *this;
	}
	
	const Angle & operator-=(const Angle & other) {
		a -= other.a, b -= other.b, g -= other.g;
		return *this;
	}
	
	const Angle & operator/=(T scale) {
		a /= scale, b /= scale, g /= scale;
		return *this;
	}
	
	const Angle & operator *=(T scale) {
		a *= scale, b *= scale, g *= scale;
		return *this;
	}
	
	bool equalEps(const Angle & other, T pEps = std::numeric_limits<T>::epsilon()) const {
		return a > (other.a - pEps) && a < (other.a + pEps) && b > (other.b - pEps) && b < (other.b + pEps) && g > (other.g - pEps) && g < (other.g + pEps);
	}
	
	union {
		T a;
		T yaw;
	};
	union {
		T b;
		T pitch;
	};
	union {
		T g;
		T roll;
	};
	
	static const Angle ZERO; //!< A zero angle.
	
};

template<class T> const Angle<T> Angle<T>::ZERO(T(0), T(0), T(0));

float AngleDifference(float d, float e);

inline float MAKEANGLE(float a) {
	float angle = std::fmod(a, 360.f);
	return (angle >= 0) ? angle : angle + 360.f;
}

float InterpolateAngle(float a1, float a2, float p);

inline Anglef interploate(const Anglef & a1, const Anglef & a2, float p) {
	return Anglef(InterpolateAngle(a1.a, a2.a, p), InterpolateAngle(a1.b, a2.b, p), InterpolateAngle(a1.g, a2.g, p));
}

//! Get the angle of the 2D vector (0,0)--(x,y), in radians.
inline float getAngle(float x, float y) {
	float angle = PI * 1.5f + std::atan2(y, x);
	return (angle >= 0) ? angle : angle + 2 * PI;
}

//! Get the angle of the 2D vector (x0,y0)--(x1,y1), in radians.
inline float getAngle(float x0, float y0, float x1, float y1) {
	return getAngle(x1 - x0, y1 - y0);
}

//! Convert from degrees to radians.
inline float radians(float degrees) {
	return degrees * (2 * PI/360);
}

//! Convert from radians to degrees.
inline float degrees(float radians) {
	return radians * (360 / (2 * PI));
}

#endif // ARX_MATH_ANGLE_H
