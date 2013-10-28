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

#ifndef ARX_MATH_ANGLE_H
#define ARX_MATH_ANGLE_H

#include <limits>
#include <cmath>

#include <glm/gtx/quaternion.hpp>

#include "math/Types.h"

inline float MAKEANGLE(float a) {
	float angle = std::fmod(a, 360.f);
	return (angle >= 0 ) ? angle : angle + 360.f;
}

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
	Angle(T yaw, T pitch, T roll) : m_yaw(yaw), m_pitch(pitch), m_roll(roll) {}
	
	/*!
	 * Copy constructor.
	 * @param other An angle to be copied.
	 */
	Angle(const Angle & other) : m_yaw(other.m_yaw), m_pitch(other.m_pitch), m_roll(other.m_roll) {}

	explicit Angle(const glm::quat & quat) {
		typename vec3_traits<T>::type v = glm::eulerAngles(quat);
		m_yaw = degrees(v.x);
		m_pitch = degrees(v.y);
		m_roll = degrees(v.z);
	}

	T getYaw() const {
		return m_yaw;
	}

	T getPitch() const {
		return m_pitch;
	}

	T getRoll() const {
		return m_roll;
	}
	
	void setYaw(T yaw) {
		m_yaw = yaw;
	}

	void setPitch(T pitch) {
		m_pitch = pitch;
	}

	void setRoll(T roll) {
		m_roll = roll;
	}

	/*!
	 * Set this angle to the content of another angle.
	 * @brief Assignment operator.
	 * @param other An euler angle to be copied.
	 * @return Reference to this object.
	 */
	Angle & operator=(const Angle & other) {
		m_yaw = other.m_yaw, m_pitch = other.m_pitch, m_roll = other.m_roll;
		return *this;
	}
	
	/*!
	 * Test if this angle is equal to another angle.
	 * @brief Equal operator.
	 * @param other An euler angle to be compared to.
	 * @return A boolean, \b true if the two angles are equal(all members are equals), or \b false otherwise.
	 */
	bool operator==(const Angle & other) const {
		return m_yaw == other.m_yaw && m_pitch == other.m_pitch && m_roll == other.m_roll;
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
		return Angle(-m_yaw, -m_pitch, -m_roll);
	}
	
	/*!
	 * Add an angle to this angle.
	 * @brief Addition operator.
	 * @param other an angle, to be added to this angle.
	 * @return A new angle, the result of the addition of the two angles.
	 */
	Angle operator+(const Angle & other) const {
		return Angle(m_yaw + other.m_yaw, m_pitch + other.m_pitch, m_roll + other.m_roll);
	}
	
	/*!
	 * Subtract an angle to this angle.
	 * @brief Subtraction operator.
	 * @param other an angle, to be subtracted to this angle.
	 * @return A new angle, the result of the subtraction of the two angles.
	 */
	Angle operator-(const Angle & other) const {
		return Angle(m_yaw - other.m_yaw, m_pitch - other.m_pitch, m_roll - other.m_roll);
	}
	

	Angle operator*(T scale) const {
		return Angle(m_yaw * scale, m_pitch * scale, m_roll * scale);
	}
	
	const Angle & operator+=(const Angle & other) {
		m_yaw += other.m_yaw, m_pitch += other.m_pitch, m_roll += other.m_roll;
		return *this;
	}
	
	const Angle & operator-=(const Angle & other) {
		m_yaw -= other.m_yaw, m_pitch -= other.m_pitch, m_roll -= other.m_roll;
		return *this;
	}
	
	const Angle & operator/=(T scale) {
		m_yaw /= scale, m_pitch /= scale, m_roll /= scale;
		return *this;
	}
	
	const Angle & operator *=(T scale) {
		m_yaw *= scale, m_pitch *= scale, m_roll *= scale;
		return *this;
	}
	
	bool equalEps(const Angle & other, T pEps = std::numeric_limits<T>::epsilon()) const {
		return m_yaw > (other.m_yaw - pEps) && m_yaw < (other.m_yaw + pEps) && m_pitch >(other.m_pitch - pEps) && m_pitch < (other.m_pitch + pEps) && m_roll >(other.m_roll - pEps) && m_roll < (other.m_roll + pEps);
	}
	
	void normalize() {
		m_yaw = MAKEANGLE(m_yaw);
		m_pitch = MAKEANGLE(m_pitch);
		m_roll = MAKEANGLE(m_roll);
	}

	static const Angle ZERO; //!< A zero angle.

private:
	T m_yaw;
	T m_pitch;
	T m_roll;
};

template<class T> const Angle<T> Angle<T>::ZERO(T(0), T(0), T(0));

float AngleDifference(float d, float e);

float InterpolateAngle(float a1, float a2, float p);

inline Anglef interpolate(const Anglef & a1, const Anglef & a2, float p) {
	return Anglef(InterpolateAngle(a1.getYaw(), a2.getYaw(), p), InterpolateAngle(a1.getPitch(), a2.getPitch(), p), InterpolateAngle(a1.getRoll(), a2.getRoll(), p));
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
