/*
 * Copyright 2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_RANDOMVECTOR_H
#define ARX_MATH_RANDOMVECTOR_H

#include <glm/gtc/constants.hpp>

#include "math/Random.h"
#include "math/Types.h"

namespace arx {
	
	inline Vec3f randomOffsetXZ(float range) {
		return Vec3f(Random::getf(-range, range), 0.f, Random::getf(-range, range));
	}
	
	/*!
	 * Generate a random vertor with independently unform distributed components.
	 *
	 * \param min minimum value for all components (default: 0.f)
	 * \param max maximum value for all components (default: 1.f)
	 */
	inline Vec3f randomVec(float min = 0.f, float max = 1.f) {
		float range = max - min;
		return Vec3f(Random::getf() * range + min, Random::getf() * range + min, Random::getf() * range + min);
	}
	
	inline Vec3f randomVec3f() {
		return Vec3f(Random::getf(), Random::getf(), Random::getf());
	}
	
	inline Vec2f linearRand(Vec2f const & min, Vec2f const & max) {
		return Vec2f(Random::getf(min.x, max.x), Random::getf(min.y, max.y));
	}
	
	inline Vec3f linearRand(Vec3f const & min, Vec3f const & max) {
		return Vec3f(Random::getf(min.x, max.x), Random::getf(min.y, max.y), Random::getf(min.z, max.z));
	}
	
	//! Generate a random 2D vector which coordinates are regulary distributed within the area of a disk of a given radius
	inline Vec2f diskRand(float radius) {
		Vec2f result(0);
		float lenRadius(0);
		
		do {
			result = linearRand(Vec2f(-radius), Vec2f(radius));
			lenRadius = glm::length(result);
		} while(lenRadius > radius);
		
		return result;
	}
	
	//! Generate a random 2D vector which coordinates areregulary distributed on a circle of a given radius
	inline Vec2f circularRand(float radius) {
		float a = Random::getf(float(0), glm::pi<float>() * 2);
		return Vec2f(std::cos(a), std::sin(a)) * radius;
	}
	
	//! Generate a random 3D vector which coordinates are regulary distributed on a sphere of a given radius
	inline Vec3f sphericalRand(float radius) {
		float z = Random::getf(float(-1), float(1));
		float a = Random::getf(float(0), glm::pi<float>() * 2);
		
		float r = std::sqrt(float(1) - z * z);
		
		float x = r * std::cos(a);
		float y = r * std::sin(a);
		
		return Vec3f(x, y, z) * radius;
	}
	
} // namespace arx

#endif // ARX_MATH_RANDOMVECTOR_H
