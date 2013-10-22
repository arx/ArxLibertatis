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

#ifndef ARX_MATH_MATHFWD_H
#define ARX_MATH_MATHFWD_H

#include "platform/Platform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

// Temp macro to compare results of GLM vs old arx maths
#define USE_GLM_VECTORS
#define GLM_VALIDATE(v1, v2) static bool BOOST_PP_CAT(bValidate,__LINE__) = true; if(BOOST_PP_CAT(bValidate,__LINE__)) arx_assert(v1 == v2)

template <class T>
class Angle;
typedef Angle<s32> Anglei;
typedef Angle<float> Anglef;
typedef Angle<double> Angled;

template<class T>
class Rectangle_;
typedef Rectangle_<s32> Rect;
typedef Rectangle_<float> Rectf;

template <class T>
class Vector2;

#ifdef USE_GLM_VECTORS
	typedef glm::detail::tvec2<s32> Vec2i;
	typedef glm::detail::tvec2<short> Vec2s;
	typedef glm::detail::tvec2<float> Vec2f;
	typedef glm::detail::tvec2<double> Vec2d;
#else
	typedef Vector2<s32> Vec2i;
	typedef Vector2<short> Vec2s;
	typedef Vector2<float> Vec2f;
	typedef Vector2<double> Vec2d;
#endif

template <class T>
class Vector3;

#ifdef USE_GLM_VECTORS
	typedef glm::detail::tvec3<s32> Vec3i;
	typedef glm::detail::tvec3<float> Vec3f;
	typedef glm::detail::tvec3<double> Vec3d;
#else
	typedef Vector3<s32> Vec3i;
	typedef Vector3<float> Vec3f;
	typedef Vector3<double> Vec3d;
#endif

// Math constants
#define PI 3.14159265358979323846f

#endif // ARX_MATH_MATHFWD_H
