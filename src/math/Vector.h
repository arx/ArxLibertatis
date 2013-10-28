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

#include <glm/gtx/norm.hpp>

#include "math/Types.h"

// TODO replace calls with their direct glm equivalents!

template <class V>
typename V::value_type dist(const V & a, const V & b) {
	return glm::distance(a, b);
}

template <class V>
typename V::value_type distSqr(const V & a, const V & b) {
	return glm::distance2(a, b);
}

template <class V>
bool closerThan(const V & a, const V & b, typename V::value_type d) {
	return distSqr(a, b) < (d * d);
}

template <class V>
bool fartherThan(const V & a, const V & b, typename V::value_type d) {
	return distSqr(a, b) > (d * d);
}

template <class V>
typename V::value_type length(const V & v0) {
	return glm::length(v0);
}

template <class V>
typename V::value_type lengthSqr(const V & v0) {
	return glm::length2(v0);
}

template <class V>
V normalize(const V & v0) {
	return glm::normalize(v0);
}

template <class V>
V cross(const V & a, const V & b) {
	return glm::cross(a, b);
}

template <class V>
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
bool isallfinite(const V & vec) {
	
	for(size_t i = 0; i < vec.length(); i++) {
		if(!std::isfinite(vec[i])) {
			return false;
		}
	}
	
	return true;
}

#endif // ARX_MATH_VECTOR_H
