/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_VECTOR_H
#define ARX_MATH_VECTOR_H

#include <boost/math/special_functions/fpclassify.hpp>

#include <glm/gtx/norm.hpp>

#include "math/Types.h"

// TODO replace calls with their direct glm equivalents!

template <class V>
bool closerThan(const V & a, const V & b, typename V::value_type d) {
	return glm::distance2(a, b) < (d * d);
}

template <class V>
bool fartherThan(const V & a, const V & b, typename V::value_type d) {
	return glm::distance2(a, b) > (d * d);
}

template <class V>
bool isallfinite(const V & vec) {
	
	for(size_t i = 0; i < vec.length(); i++) {
		if(!(boost::math::isfinite)(vec[i])) {
			return false;
		}
	}
	
	return true;
}

// Constants

const Vec2i Vec2i_ZERO(0, 0);
const Vec2i Vec2i_ONE(1, 1);
const Vec2i Vec2i_X_AXIS(1, 0);
const Vec2i Vec2i_Y_AXIS(0, 1);

const Vec2s Vec2s_ZERO(0, 0);
const Vec2s Vec2s_ONE(1, 1);
const Vec2s Vec2s_X_AXIS(1, 0);
const Vec2s Vec2s_Y_AXIS(0, 1);

const Vec2f Vec2f_ZERO(0, 0);
const Vec2f Vec2f_ONE(1, 1);
const Vec2f Vec2f_X_AXIS(1, 0);
const Vec2f Vec2f_Y_AXIS(0, 1);

const Vec2d Vec2d_ZERO(0, 0);
const Vec2d Vec2d_ONE(1, 1);
const Vec2d Vec2d_X_AXIS(1, 0);
const Vec2d Vec2d_Y_AXIS(0, 1);

const Vec3i Vec3i_ZERO(0,0,0);
const Vec3i Vec3i_ONE(1,1,1);
const Vec3i Vec3i_X_AXIS(1,0,0);
const Vec3i Vec3i_Y_AXIS(0,1,0);
const Vec3i Vec3i_Z_AXIS(0,0,1);

const Vec3f Vec3f_ZERO(0,0,0);
const Vec3f Vec3f_ONE(1,1,1);
const Vec3f Vec3f_X_AXIS(1,0,0);
const Vec3f Vec3f_Y_AXIS(0,1,0);
const Vec3f Vec3f_Z_AXIS(0,0,1);

const Vec3d Vec3d_ZERO(0,0,0);
const Vec3d Vec3d_ONE(1,1,1);
const Vec3d Vec3d_X_AXIS(1,0,0);
const Vec3d Vec3d_Y_AXIS(0,1,0);
const Vec3d Vec3d_Z_AXIS(0,0,1);

#endif // ARX_MATH_VECTOR_H
