/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#if GLM_VERSION >= 990

template <class T, int N> struct vec_traits {
	typedef glm::vec<N, T, glm::highp> type;
	ARX_STATIC_ASSERT(sizeof(type) == sizeof(T) * N, "vector has padding");
};

#else

template <class T, template <class, glm::precision> class V, int N>
struct vec_traits_base {
	typedef V<T, glm::highp> type;
	ARX_STATIC_ASSERT(sizeof(type) == sizeof(T) * N, "vector has padding");
};

// In GLM version 0.9.6 the template vector and matrix types is exposed in 'glm' namespace
#if GLM_VERSION >= 96
template <class T> struct vec2_traits : public vec_traits_base<T, glm::tvec2, 2>{};
template <class T> struct vec3_traits : public vec_traits_base<T, glm::tvec3, 3>{};
template <class T> struct vec4_traits : public vec_traits_base<T, glm::tvec4, 4>{};
#else
template <class T> struct vec2_traits : public vec_traits_base<T, glm::detail::tvec2, 2>{};
template <class T> struct vec3_traits : public vec_traits_base<T, glm::detail::tvec3, 3>{};
template <class T> struct vec4_traits : public vec_traits_base<T, glm::detail::tvec4, 4>{};
#endif

template <class T, int N> struct vec_traits {};
template <class T> struct vec_traits<T, 2> : public vec2_traits<T>{};
template <class T> struct vec_traits<T, 3> : public vec3_traits<T>{};
template <class T> struct vec_traits<T, 4> : public vec4_traits<T>{};

#endif

typedef vec_traits<s32, 2>::type Vec2i;
typedef vec_traits<s16, 2>::type Vec2s;
typedef vec_traits<f32, 2>::type Vec2f;
typedef vec_traits<f32, 3>::type Vec3f;
typedef vec_traits<f32, 4>::type Vec4f;

ARX_USE_ALIGNED_ALLOCATOR(Vec4f)

#endif // ARX_MATH_TYPES_H
