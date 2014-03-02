/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TESTS_GRAPHICS_LEGACYMATH_H
#define ARX_TESTS_GRAPHICS_LEGACYMATH_H

#include <glm/gtc/quaternion.hpp>

glm::quat Quat_Multiply(const glm::quat & q1, const glm::quat & q2) {
	/*
	Fast multiplication

	There are some schemes available that reduces the number of internal
	multiplications when doing quaternion multiplication. Here is one:

	   q = (a, b, c, d), p = (x, y, z, w)

	   tmp_00 = (d - c) * (z - w)
	   tmp_01 = (a + b) * (x + y)
	   tmp_02 = (a - b) * (z + w)
	   tmp_03 = (c + d) * (x - y)
	   tmp_04 = (d - b) * (y - z)
	   tmp_05 = (d + b) * (y + z)
	   tmp_06 = (a + c) * (x - w)
	   tmp_07 = (a - c) * (x + w)
	   tmp_08 = tmp_05 + tmp_06 + tmp_07
	   tmp_09 = 0.5 * (tmp_04 + tmp_08)

	   q * p = (tmp_00 + tmp_09 - tmp_05,
	            tmp_01 + tmp_09 - tmp_08,
	            tmp_02 + tmp_09 - tmp_07,
	            tmp_03 + tmp_09 - tmp_06)

	With this method You get 7 less multiplications, but 15 more
	additions/subtractions. Generally, this is still an improvement.
	  */

	return glm::quat(
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z,
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x
	);
}

#endif // ARX_TESTS_GRAPHICS_LEGACYMATH_H
