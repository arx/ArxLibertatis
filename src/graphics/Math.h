/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef ARX_GRAPHICS_MATH_H
#define ARX_GRAPHICS_MATH_H

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <boost/numeric/conversion/cast.hpp>
#include <boost/static_assert.hpp>

#include <glm/gtx/norm.hpp>

using std::min;
using std::max;

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"

// RANDOM Sequences Funcs/Defs
inline float rnd() {
	return rand() * (1.0f / RAND_MAX);
}

/*!
 * Generate a random vertor with independently unform distributed components.
 *
 * @param min minimum value for all components (default: 0.f)
 * @param max maximum value for all components (default: 1.f)
 */
inline Vec3f randomVec(float min = 0.f, float max = 1.f) {
	float range = max - min;
	return Vec3f(rnd() * range + min, rnd() * range + min, rnd() * range + min);
}

//Approximative Methods
#define EEcos(val)  (float)cos((float)val)
#define EEsin(val)  (float)sin((float)val)
#define EEfabs(val) (float)fabs(val)

inline bool In3DBBoxTolerance(const Vec3f * pos, const EERIE_3D_BBOX * bbox, const float tolerance) {
	return ((pos->x >= bbox->min.x - tolerance)
	        && (pos->x <= bbox->max.x + tolerance)
	        && (pos->y >= bbox->min.y - tolerance)
	        && (pos->y <= bbox->max.y + tolerance)
	        && (pos->z >= bbox->min.z - tolerance)
	        && (pos->z <= bbox->max.z + tolerance));
}

//! Clamp a positive value to the range [0, 255]
inline u8 clipByte255(int value) {
	
	// clamp larger values to 255
	value |= (-(int)(value > 255));
	value &= 255;
	
	return static_cast<u8>(value);
}

//! Clamp a value to the range [0, 255]
inline u8 clipByte(int value) {
	
	// clamp negative values to zero
	value &= -(int)!(value < 0);
	
	return clipByte255(value);
}

inline long F2L_RoundUp(float val) {
	return static_cast<long>(ceil(val));
}

bool CylinderInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_CYLINDER * cyl2);
bool SphereInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_SPHERE * s);

template <class T, class O>
inline T reinterpret(O v) {
	BOOST_STATIC_ASSERT(sizeof(T) == sizeof(O));
	T t;
	memcpy(&t, &v, sizeof(T));
	return t; 
}

inline float ffsqrt(float f) {
	return reinterpret<f32, u32>(((reinterpret<u32, f32>(f) - 0x3f800000) >> 1) + 0x3f800000);
}

/*!
 * Obtain the approximated inverse of the square root of a float.
 * @brief  This code compute a fast 1 / sqrtf(v) approximation.
 * @note   Originaly from Matthew Jones (Infogrames).
 * @param  pValue  a float, the number we want the square root.
 * @return The square root of \a fValue, as a float.
 */
inline float FastRSqrt(float value) {
	
	s32 intval = reinterpret<s32, f32>(value);
	
	const int MAGIC_NUMBER = 0x5f3759df;
			
	float half = value * 0.5f;
	intval = MAGIC_NUMBER - (intval >> 1);
	
	value = reinterpret<f32, s32>(intval);
	
	return value * (1.5f - half * value * value);
}

inline bool IsPowerOf2(unsigned int n) {
	return (n & (n - 1)) == 0;
}

inline unsigned int GetNextPowerOf2(unsigned int n) {
	
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	
	return ++n;
}

// Rotations

inline void ZRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->y * s, in->y * c - in->x * s, in->z);
}

inline void YRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->z * s, in->y, in->z * c - in->x * s);
}

inline void XRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x, in->y * c - in->z * s, in->y * s + in->z * c);
}

//! Normalizes a Vector approximately. Returns its approcimate length before normalization.
inline float fnormalize(Vec3f & v) {
	float len = ffsqrt(glm::length2(v));
	v *= 1 / len;
	return len;
}

// Matrix functions

void MatrixSetByVectors(EERIEMATRIX * m, const Vec3f * d, const Vec3f * u);
void MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b);
void VectorMatrixMultiply(Vec3f * vDest, const Vec3f * vSrc, const EERIEMATRIX * mat);
void GenerateMatrixUsingVector(EERIEMATRIX * matrix, const Vec3f * vect, float rollDegrees);

// Rotation Functions

inline void YXZRotatePoint(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam) {
	float tempy;
	out->z = (in->z * cam->orgTrans.ycos) - (in->x * cam->orgTrans.ysin);
	out->y = (in->x * cam->orgTrans.ycos) + (in->z * cam->orgTrans.ysin);
	tempy = (in->y * cam->orgTrans.xcos) - (out->z * cam->orgTrans.xsin);
	out->x = (out->y * cam->orgTrans.zcos) + (tempy * cam->orgTrans.zsin);
	out->y = (tempy * cam->orgTrans.zcos) - (out->y * cam->orgTrans.zsin);
	out->z = (in->y * cam->orgTrans.xsin) + (out->z * cam->orgTrans.xcos);
}

// QUATERNION Funcs/Defs

//! Quaternion Initialization
//!  quat -> quaternion to init
inline void Quat_Init(EERIE_QUAT * quat, float x = 0, float y = 0, float z = 0, float w = 1) {
	quat->x = x;
	quat->y = y;
	quat->z = z;
	quat->w = w;
}

// Transforms a Vertex by a matrix
inline void TransformVertexMatrix(EERIEMATRIX * mat, Vec3f * vertexin, Vec3f * vertexout) {
	vertexout->x = vertexin->x * mat->_11 + vertexin->y * mat->_21 + vertexin->z * mat->_31;
	vertexout->y = vertexin->x * mat->_12 + vertexin->y * mat->_22 + vertexin->z * mat->_32;
	vertexout->z = vertexin->x * mat->_13 + vertexin->y * mat->_23 + vertexin->z * mat->_33;
}

// Transforms a Vertex by a quaternion
inline Vec3f TransformVertexQuat(const EERIE_QUAT & quat, const Vec3f & vertexin) {
	
	float rx = vertexin.x * quat.w - vertexin.y * quat.z + vertexin.z * quat.y;
	float ry = vertexin.y * quat.w - vertexin.z * quat.x + vertexin.x * quat.z;
	float rz = vertexin.z * quat.w - vertexin.x * quat.y + vertexin.y * quat.x;
	float rw = vertexin.x * quat.x + vertexin.y * quat.y + vertexin.z * quat.z;
	
	return Vec3f(
		quat.w * rx + quat.x * rw + quat.y * rz - quat.z * ry,
		quat.w * ry + quat.y * rw + quat.z * rx - quat.x * rz,
		quat.w * rz + quat.z * rw + quat.x * ry - quat.y * rx);
}

void TransformInverseVertexQuat(const EERIE_QUAT * quat, const Vec3f * vertexin, Vec3f * vertexout);
void Quat_Divide(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2);
EERIE_QUAT Quat_Multiply(const EERIE_QUAT & q1, const EERIE_QUAT & q2);

EERIE_QUAT Quat_Slerp(const EERIE_QUAT & from, EERIE_QUAT to, float t);
void Quat_Reverse(EERIE_QUAT * quat);

void worldAngleToQuat(EERIE_QUAT *dest, const Anglef & src, bool isNpc = false);

// VECTORS Functions

void Vector_RotateY(Vec3f * dest, const Vec3f * src, const float angle);
void Vector_RotateZ(Vec3f * dest, const Vec3f * src, const float angle);
void VRotateX(Vec3f * v1, const float angle);
void VRotateY(Vec3f * v1, const float angle);
void VRotateZ(Vec3f * v1, const float angle);
void QuatFromMatrix(EERIE_QUAT & quat, EERIEMATRIX & mat);

void CalcFaceNormal(EERIEPOLY * ep, const TexturedVertex * v);
void CalcObjFaceNormal(const Vec3f * v0, const Vec3f * v1, const Vec3f * v2, EERIE_FACE * ef);
bool Triangles_Intersect(const EERIE_TRI * v, const EERIE_TRI * u);
void MatrixFromQuat(EERIEMATRIX * mat, const EERIE_QUAT * q);

inline float square(float x) {
	return x * x;
}

/*!
 * Compute (approximate) Distance between two 3D points
 * may use an approximative way of computing sqrt !
 */
inline float fdist(const Vec3f & from, const Vec3f & to) {
	return ffsqrt(glm::distance2(from, to));
}

/*!
 * Compute (approximate) Distance between two 2D points
 * may use an approximative way of computing sqrt !
 */
inline float fdist(const Vec2f & from, const Vec2f & to) {
	return ffsqrt(glm::distance2(from, to));
}

inline bool PointInCylinder(const EERIE_CYLINDER * cyl, const Vec3f * pt) {
	
	float pos1 = cyl->origin.y + cyl->height;
	
	if(pt->y < min(cyl->origin.y, pos1)) {
		return false;
	}
	
	if(pt->y > max(cyl->origin.y, pos1)) {
		return false;
	}
	
	if(!fartherThan(Vec2f(cyl->origin.x, cyl->origin.z), Vec2f(pt->x, pt->z), cyl->radius)) {
		return true;
	}
	
	return false;
}

inline long PointInUnderCylinder(const EERIE_CYLINDER * cyl, const Vec3f * pt) {
	
	float pos1 = cyl->origin.y + cyl->height;
	
	if(pt->y < min(cyl->origin.y, pos1)) {
		return 0;
	}
	
	if(!fartherThan(Vec2f(cyl->origin.x, cyl->origin.z), Vec2f(pt->x, pt->z), cyl->radius)) {
		return (pt->y > max(cyl->origin.y, pos1)) ? 1 : 2;
	}
	
	return 0;
}

// ANGLES Functions

void QuatFromAngles(EERIE_QUAT * q, const Anglef * angle);

template <class T1, class T2, class T3>
inline T1 clamp(T1 value, T2 min, T3 max) {
	return (value <= min) ? min : ((value >= max) ? max : value);
}

#ifdef ARX_DEBUG
#define checked_range_cast boost::numeric_cast
#else
#define checked_range_cast static_cast
#endif

#endif // ARX_GRAPHICS_MATH_H
