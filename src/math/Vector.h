/*
 * Copyright 2013-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <cmath>

#include "math/GtxFunctions.h"
#include "math/Types.h"


template <class V>
[[nodiscard]] bool closerThan(const V & a, const V & b, typename V::value_type d) noexcept {
	return arx::distance2(a, b) < (d * d);
}

template <class V>
[[nodiscard]] bool fartherThan(const V & a, const V & b, typename V::value_type d) noexcept {
	return arx::distance2(a, b) > (d * d);
}

template <class V>
[[nodiscard]] bool isallfinite(const V & vec) noexcept {
	
	for(size_t i = 0; i < size_t(vec.length()); i++) {
		if(!(std::isfinite)(vec[i])) {
			return false;
		}
	}
	
	return true;
}

[[nodiscard]] inline constexpr Vec2f getXZ(Vec3f pos) noexcept {
	return { pos.x, pos.z };
}

[[nodiscard]] inline constexpr Vec3f toXZ(float pos) noexcept {
	return { pos, 0.f, pos };
}

[[nodiscard]] inline constexpr Vec3f toXZ(Vec2f pos) noexcept {
	return { pos.x, 0.f, pos.y };
}

[[nodiscard]] inline constexpr Vec3f toXZ(Vec3f pos) noexcept {
	return { pos.x, 0.f, pos.z };
}

#endif // ARX_MATH_VECTOR_H
