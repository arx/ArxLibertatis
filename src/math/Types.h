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

#ifndef ARX_MATH_TYPES_H
#define ARX_MATH_TYPES_H

#include "platform/Alignment.h"
#include "platform/Platform.h"

#include <glm/glm.hpp>

template <class T>
class Angle;
typedef Angle<float> Anglef;

template <class T>
class Rectangle_;
typedef Rectangle_<s32> Rect;
typedef Rectangle_<float> Rectf;

template <class T, int N> struct vec_traits {
	typedef glm::vec<N, T, glm::highp> type;
	static_assert(sizeof(type) == sizeof(T) * N, "vector has padding");
};

typedef vec_traits<s32, 2>::type Vec2i;
typedef vec_traits<s16, 2>::type Vec2s;
typedef vec_traits<f32, 2>::type Vec2f;
typedef vec_traits<f32, 3>::type Vec3f;
typedef vec_traits<f32, 4>::type Vec4f;

ARX_USE_ALIGNED_ALLOCATOR(Vec4f)

#endif // ARX_MATH_TYPES_H
