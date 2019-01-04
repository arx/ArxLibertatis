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

#ifndef ARX_TESTS_GRAPHICS_LEGACYMATH_H
#define ARX_TESTS_GRAPHICS_LEGACYMATH_H

#include <cmath>

#include "graphics/Math.h"

//! Transforms a Vertex by a quaternion
inline Vec3f TransformVertexQuat(const glm::quat & quat, const Vec3f & vertexin) {
	
	float rx = vertexin.x * quat.w - vertexin.y * quat.z + vertexin.z * quat.y;
	float ry = vertexin.y * quat.w - vertexin.z * quat.x + vertexin.x * quat.z;
	float rz = vertexin.z * quat.w - vertexin.x * quat.y + vertexin.y * quat.x;
	float rw = vertexin.x * quat.x + vertexin.y * quat.y + vertexin.z * quat.z;
	
	return Vec3f(
		quat.w * rx + quat.x * rw + quat.y * rz - quat.z * ry,
		quat.w * ry + quat.y * rw + quat.z * rx - quat.x * rz,
		quat.w * rz + quat.z * rw + quat.x * ry - quat.y * rx);
}

//! Invert-Transform of vertex by a quaternion
inline void TransformInverseVertexQuat(const glm::quat & quat, const Vec3f & vertexin, Vec3f & vertexout) {
	
	glm::quat rev_quat = glm::inverse(quat);
	
	float x = vertexin.x;
	float y = vertexin.y;
	float z = vertexin.z;
	
	float qx = rev_quat.x;
	float qy = rev_quat.y;
	float qz = rev_quat.z;
	float qw = rev_quat.w;
	
	float rx = x * qw - y * qz + z * qy;
	float ry = y * qw - z * qx + x * qz;
	float rz = z * qw - x * qy + y * qx;
	float rw = x * qx + y * qy + z * qz;
	
	vertexout.x = qw * rx + qx * rw + qy * rz - qz * ry;
	vertexout.y = qw * ry + qy * rw + qz * rx - qx * rz;
	vertexout.z = qw * rz + qz * rw + qx * ry - qy * rx;
}

//! Invert Multiply of a quaternion by another quaternion
inline void Quat_Divide(glm::quat * dest, const glm::quat * q1, const glm::quat * q2) {
	dest->x = q1->w * q2->x - q1->x * q2->w - q1->y * q2->z + q1->z * q2->y;
	dest->y = q1->w * q2->y - q1->y * q2->w - q1->z * q2->x + q1->x * q2->z;
	dest->z = q1->w * q2->z - q1->z * q2->w - q1->x * q2->y + q1->y * q2->x;
	dest->w = q1->w * q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;
}

//! Inverts a Quaternion
inline void Quat_Reverse(glm::quat * q) {
	glm::quat qw = quat_identity();
	glm::quat qr;
	Quat_Divide(&qr, q, &qw);
	*q = qr;
}


inline glm::quat Quat_Multiply(const glm::quat & q1, const glm::quat & q2) {
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
inline void MatrixFromQuat(glm::mat4x4 & m, const glm::quat & quat) {
	
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
	
	m[0][0] = 1.f - (yy + zz);
	m[1][0] = xy - wz;
	m[2][0] = xz + wy;
	m[3][0] = 0.f;
	
	m[0][1] = xy + wz;
	m[1][1] = 1.f - (xx + zz);
	m[2][1] = yz - wx;
	m[3][1] = 0.f;
	
	m[0][2] = xz - wy;
	m[1][2] = yz + wx;
	m[2][2] = 1.f - (xx + yy);
	m[3][2] = 0.f;
	
}

//! Converts a rotation matrix into a unit quaternion.
inline void QuatFromMatrix(glm::quat & quat, const glm::mat4x4 & mat) {
	float m[4][4];
	m[0][0] = mat[0][0];
	m[0][1] = mat[0][1];
	m[0][2] = mat[0][2];
	m[0][3] = mat[0][3];
	m[1][0] = mat[1][0];
	m[1][1] = mat[1][1];
	m[1][2] = mat[1][2];
	m[1][3] = mat[1][3];
	m[2][0] = mat[2][0];
	m[2][1] = mat[2][1];
	m[2][2] = mat[2][2];
	m[2][3] = mat[2][3];
	m[3][0] = mat[3][0];
	m[3][1] = mat[3][1];
	m[3][2] = mat[3][2];
	m[3][3] = mat[3][3];
	float  tr, s, q[4];

	int nxt[3] = {1, 2, 0};

	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0f)
	{
		s = std::sqrt(tr + 1.0f);
		quat.w = s * ( 1.0f / 2 );
		s = 0.5f / s;
		quat.x = (m[1][2] - m[2][1]) * s;
		quat.y = (m[2][0] - m[0][2]) * s;
		quat.z = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		int i = 0;

		if (m[1][1] > m[0][0]) i = 1;

		if (m[2][2] > m[i][i]) i = 2;

		int j = nxt[i];
		int k = nxt[j];

		s = std::sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0f) s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;


		quat.x = q[0];
		quat.y = q[1];
		quat.z = q[2];
		quat.w = q[3];
	}
}


inline glm::quat toNonNpcRotation(const Anglef & src) {
	Anglef ang = src;
	ang.setPitch(360 - ang.getPitch());
	
	glm::mat4x4 mat(1.f);
	Vec3f vect(0, 0, 1);
	Vec3f up(0, 1, 0);
	vect = VRotateY(vect, ang.getYaw());
	vect = VRotateX(vect, ang.getPitch());
	vect = VRotateZ(vect, ang.getRoll());
	up = VRotateY(up, ang.getYaw());
	up = VRotateX(up, ang.getPitch());
	up = VRotateZ(up, ang.getRoll());
	MatrixSetByVectors(mat, vect, up);
	return glm::quat_cast(mat);
}

inline void VectorRotateY(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = std::cos(_fAngle);
	float s = std::sin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.z * s);
	_eOut.y =  _eIn.y;
	_eOut.z = (_eIn.z * c) - (_eIn.x * s);
}

inline void VectorRotateZ(Vec3f & _eIn, Vec3f & _eOut, float _fAngle) {
	float c = std::cos(_fAngle);
	float s = std::sin(_fAngle);
	_eOut.x = (_eIn.x * c) + (_eIn.y * s);
	_eOut.y = (_eIn.y * c) - (_eIn.x * s);
	_eOut.z =  _eIn.z;
}

inline Vec2s inventorySizeFromTextureSize_2(u32 m_dwWidth, u32 m_dwHeight) {
	unsigned long w = m_dwWidth >> 5;
	unsigned long h = m_dwHeight >> 5;
	return Vec2s(char(glm::clamp(((w << 5) != m_dwWidth) ? (w + 1) : w, 1ul, 3ul)),
	             char(glm::clamp(((h << 5) != m_dwHeight) ? (h + 1) : h, 1ul, 3ul)));
}

inline float focalToFovLegacy(float focal) {
	if(focal < 200)
		return (-.34f * focal + 168.5f);
	else if(focal < 300)
		return (-.25f * focal + 150.5f);
	else if(focal < 400)
		return (-.155f * focal + 124.f);
	else if(focal < 500)
		return (-.11f * focal + 106.f);
	else if(focal < 600)
		return (-.075f * focal + 88.5f);
	else if(focal < 700)
		return (-.055f * focal + 76.5f);
	else if(focal < 800)
		return (-.045f * focal + 69.5f);
	else
		return 33.5f;
}

inline Vec3f interpolatePos(float t, const Vec3f prevPos, const Vec3f currentPos, const Vec3f nextPos, const Vec3f next2Pos)
{
	const float t1 = t;
	const float t2 = t1 * t1;
	const float t3 = t2 * t1;
	const float f0 = 2.f * t3 - 3.f * t2 + 1.f;
	const float f1 = -2.f * t3 + 3.f * t2;
	const float f2 = t3 - 2.f * t2 + t1;
	const float f3 = t3 - t2;
	
	const Vec3f p0 = 0.5f * (nextPos - prevPos);
	const Vec3f p1 = 0.5f * (next2Pos - currentPos);
	
	return f0 * currentPos + f1 * nextPos + f2 * p0 + f3 * p1;
}

#endif // ARX_TESTS_GRAPHICS_LEGACYMATH_H
