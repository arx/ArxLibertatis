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

#include "graphics/Math.h"

//! Transforms a Vertex by a quaternion
Vec3f TransformVertexQuat(const glm::quat & quat, const Vec3f & vertexin) {
	
	float rx = vertexin.x * quat.w - vertexin.y * quat.z + vertexin.z * quat.y;
	float ry = vertexin.y * quat.w - vertexin.z * quat.x + vertexin.x * quat.z;
	float rz = vertexin.z * quat.w - vertexin.x * quat.y + vertexin.y * quat.x;
	float rw = vertexin.x * quat.x + vertexin.y * quat.y + vertexin.z * quat.z;
	
	return Vec3f(
		quat.w * rx + quat.x * rw + quat.y * rz - quat.z * ry,
		quat.w * ry + quat.y * rw + quat.z * rx - quat.x * rz,
		quat.w * rz + quat.z * rw + quat.x * ry - quat.y * rx);
}

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

//! Converts a unit quaternion into a rotation matrix.
void MatrixFromQuat(glm::mat4x4 & m, const glm::quat & quat) {
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat.x + quat.x;
	y2 = quat.y + quat.y;
	z2 = quat.z + quat.z;
	xx = quat.x * x2;
	xy = quat.x * y2;
	xz = quat.x * z2;
	yy = quat.y * y2;
	yz = quat.y * z2;
	zz = quat.z * z2;
	wx = quat.w * x2;
	wy = quat.w * y2;
	wz = quat.w * z2;

	m[0][0] = 1.0F - (yy + zz);
	m[1][0] = xy - wz;
	m[2][0] = xz + wy;
	m[3][0] = 0.0F;

	m[0][1] = xy + wz;
	m[1][1] = 1.0F - (xx + zz);
	m[2][1] = yz - wx;
	m[3][1] = 0.0F;

	m[0][2] = xz - wy;
	m[1][2] = yz + wx;
	m[2][2] = 1.0F - (xx + yy);
	m[3][2] = 0.0F;
}

#endif // ARX_TESTS_GRAPHICS_LEGACYMATH_H
