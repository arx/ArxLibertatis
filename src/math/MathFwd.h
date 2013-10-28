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

#include <boost/static_assert.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

template <class T>
class Angle;
typedef Angle<s32> Anglei;
typedef Angle<float> Anglef;
typedef Angle<double> Angled;

template<class T>
class Rectangle_;
typedef Rectangle_<s32> Rect;
typedef Rectangle_<float> Rectf;


#if GLM_VERSION >= 95
	template <class T, template <class, glm::precision> class V, int N>
	struct vec_traits_base {
		enum { num_components = N };
		typedef T component_type;
		typedef V<T, glm::highp> type;

		BOOST_STATIC_ASSERT(sizeof(type) == sizeof(component_type) * N);
	};
#else
	template <class T, template <class> class V, int N>
	struct vec_traits_base {
		enum { num_components = N };
		typedef T component_type;
		typedef V<T> type;

		BOOST_STATIC_ASSERT(sizeof(type) == sizeof(component_type) * N);
	};
#endif


template <class T> struct vec2_traits : public vec_traits_base<T, glm::detail::tvec2, 2>{};
template <class T> struct vec3_traits : public vec_traits_base<T, glm::detail::tvec3, 3>{};
template <class T> struct vec4_traits : public vec_traits_base<T, glm::detail::tvec4, 4>{};

template <class T, int N> struct vec_traits {};
template <class T> struct vec_traits<T, 2> : public vec2_traits<T>{};
template <class T> struct vec_traits<T, 3> : public vec3_traits<T>{};
template <class T> struct vec_traits<T, 4> : public vec4_traits<T>{};


typedef vec_traits<s32, 2>::type Vec2i;
typedef vec_traits<s16, 2>::type Vec2s;
typedef vec_traits<f32, 2>::type Vec2f;
typedef vec_traits<f64, 2>::type Vec2d;

typedef vec_traits<s32, 3>::type Vec3i;
typedef vec_traits<f32, 3>::type Vec3f;
typedef vec_traits<f64, 3>::type Vec3d;

typedef vec_traits<s32, 4>::type Vec4i;
typedef vec_traits<f32, 4>::type Vec4f;
typedef vec_traits<f64, 4>::type Vec4d;


template<class V>
typename V::value_type dist(const V & a, const V & b) {
	return glm::distance(a, b);
}

template<class V>
typename V::value_type distSqr(const V & a, const V & b) {
	return glm::distance2(a, b);
}

template<class V>
bool closerThan(const V & a, const V & b, typename V::value_type d) {
	return distSqr(a, b) < (d * d);
}

template<class V>
bool fartherThan(const V & a, const V & b, typename V::value_type d) {
	return distSqr(a, b) > (d * d);
}

template<class V>
V normalize(const V & v0) {
	return glm::normalize(v0);
}

template<class V>
typename V::value_type length(const V & v0) {
	return glm::length(v0);
}

template<class V>
typename V::value_type lengthSqr(const V & v0) {
	return glm::length2(v0);
}

template<class V>
V cross(const V & a, const V & b) {
	return glm::cross(a, b);
}

template<class V>
typename V::value_type dot(const V & a, const V & b) {
	return glm::dot(a, b);
}

template <class V>
V componentwise_min(const V & v0, const V & v1) {
	return glm::min(v0, v1);
}
template <class V>
V componentwise_max(const V & v0, const V & v1) {
	return glm::max(v0, v1);
}

template <class V>
bool isallfinite(const V& vec) {
	for(size_t i = 0; i < vec.length(); i++) {
		if(!std::isfinite(vec[i]))
			return false;
	}

	return true;
}

// Math constants
#define PI 3.14159265358979323846f

#endif // ARX_MATH_MATHFWD_H
