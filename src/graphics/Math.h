/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/static_assert.hpp>

using std::min;
using std::max;

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"

// RANDOM Sequences Funcs/Defs
inline float rnd() {
	return rand() * (1.0f / (RAND_MAX - 1));
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

inline void _ZRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	out->x = (in->x * c) + (in->y * s);
	out->y = (in->y * c) - (in->x * s);
	out->z = in->z;
}

inline void _YRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	out->x = (in->x * c) + (in->z * s);
	out->y = in->y;
	out->z = (in->z * c) - (in->x * s);
}

inline void _XRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	out->x = in->x;
	out->y = (in->y * c) - (in->z * s);
	out->z = (in->y * s) + (in->z * c);
}

//! Normalizes a Vector approximately. Returns its approcimate length before normalization.
inline float fnormalize(Vec3f & v) {
	float len = ffsqrt(v.lengthSqr());
	v *= 1 / len;
	return len;
}

// Matrix functions

void MatrixSetByVectors(EERIEMATRIX * m, const Vec3f * d, const Vec3f * u);
void MatrixReset(EERIEMATRIX * mat);
void MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b);
void VectorMatrixMultiply(Vec3f * vDest, const Vec3f * vSrc, const EERIEMATRIX * mat);
void GenerateMatrixUsingVector(EERIEMATRIX * matrix, const Vec3f * vect, float rollDegrees);

// Rotation Functions

inline void _YXZRotatePoint(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam) {
	register float tempy;
	out->z = (in->z * cam->Ycos) - (in->x * cam->Ysin);
	out->y = (in->x * cam->Ycos) + (in->z * cam->Ysin);
	tempy = (in->y * cam->Xcos) - (out->z * cam->Xsin);
	out->x = (out->y * cam->Zcos) + (tempy * cam->Zsin);
	out->y = (tempy * cam->Zcos) - (out->y * cam->Zsin);
	out->z = (in->y * cam->Xsin) + (out->z * cam->Xcos);
}

//! Fast normal rotation :p
inline void _YXZRotateNorm(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam) {
	out->z = (in->y * cam->Xsin) + (((in->z * cam->Ycos) - (in->x * cam->Ysin)) * cam->Xcos);
}

// QUATERNION Funcs/Defs

//! Copy a quaternion into another
inline void Quat_Copy(EERIE_QUAT * dest, const EERIE_QUAT * src) {
	dest->x = src->x;
	dest->y = src->y;
	dest->z = src->z;
	dest->w = src->w;
}

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
inline void TransformVertexQuat(EERIE_QUAT * quat, Vec3f * vertexin, Vec3f * vertexout) {
	
	register float rx = vertexin->x * quat->w - vertexin->y * quat->z + vertexin->z * quat->y;
	register float ry = vertexin->y * quat->w - vertexin->z * quat->x + vertexin->x * quat->z;
	register float rz = vertexin->z * quat->w - vertexin->x * quat->y + vertexin->y * quat->x;
	register float rw = vertexin->x * quat->x + vertexin->y * quat->y + vertexin->z * quat->z;
	
	vertexout->x = quat->w * rx + quat->x * rw + quat->y * rz - quat->z * ry;
	vertexout->y = quat->w * ry + quat->y * rw + quat->z * rx - quat->x * rz;
	vertexout->z = quat->w * rz + quat->z * rw + quat->x * ry - quat->y * rx;
}

void TransformInverseVertexQuat(const EERIE_QUAT * quat, const Vec3f * vertexin, Vec3f * vertexout);
void Quat_Divide(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2);
void Quat_Multiply(EERIE_QUAT * dest , const EERIE_QUAT * q1, const EERIE_QUAT * q2);

void Quat_Slerp(EERIE_QUAT * result, const EERIE_QUAT * from, EERIE_QUAT * to, float t);
void Quat_Reverse(EERIE_QUAT * quat);

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
	return ffsqrt(distSqr(from, to));
}

/*!
 * Compute (approximate) Distance between two 2D points
 * may use an approximative way of computing sqrt !
 */
inline float fdist(const Vec2f & from, const Vec2f & to) {
	return ffsqrt(distSqr(from, to));
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

void specialEE_RT(TexturedVertex * in, Vec3f * out);
void specialEE_P(Vec3f * in, TexturedVertex * out);

template <class T1, class T2, class T3>
inline T1 clamp(T1 value, T2 min, T3 max) {
	return (value <= min) ? min : ((value >= max) ? max : value);
}

struct _checked_range_cast {
	
	const char * file;
	size_t line;
	
	inline _checked_range_cast(const char * _file, size_t _line) : file(_file), line(_line) { }
	
	template <class T, class V>
	inline T cast(V value) {
		arx_assert_impl(value >= min(T(0)) && value <= std::numeric_limits<T>::max(), file, line, format(value), cast(value));
		return static_cast<T>(value);
	}
	
private:
	
	template <class T>
	static inline T min(T t) { ARX_UNUSED(t); return std::numeric_limits<T>::min(); }
	static inline float min(float t) { ARX_UNUSED(t); return -std::numeric_limits<float>::max(); }
	static inline double min(double t) { ARX_UNUSED(t); return -std::numeric_limits<double>::max(); }
	
	static inline const char * format(unsigned long v) { ARX_UNUSED(v); return "value is %ul"; }
	static inline const char * format(long v) { ARX_UNUSED(v); return "value is %l"; }
	static inline const char * format(unsigned int v) { ARX_UNUSED(v); return "value is %ud"; }
	static inline const char * format(int v) { ARX_UNUSED(v); return "value is %d"; }
	static inline const char * format(double v) { ARX_UNUSED(v); return "value is %f"; }
	
	static inline unsigned long cast(unsigned long v) { return v; }
	static inline long cast(long v) { return v; }
	static inline unsigned int cast(unsigned int v) { return v; }
	static inline int cast(int v) { return v; }
	static inline double cast(double v) { return v; }
	
};

#ifdef _DEBUG
#define checked_range_cast _checked_range_cast(__FILE__, __LINE__).cast
#else
#define checked_range_cast static_cast
#endif

#endif // ARX_GRAPHICS_MATH_H
