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

#include <glm/gtx/norm.hpp>
#include <glm/gtx/spline.hpp>

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"
#include "math/Random.h"
#include "platform/Platform.h"

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

inline Color3f randomColor3f() {
	return Color3f(Random::getf(), Random::getf(), Random::getf());
}

inline bool In3DBBoxTolerance(const Vec3f & pos, const EERIE_3D_BBOX & bbox, const float tolerance) {
	return ((pos.x >= bbox.min.x - tolerance)
	        && (pos.x <= bbox.max.x + tolerance)
	        && (pos.y >= bbox.min.y - tolerance)
	        && (pos.y <= bbox.max.y + tolerance)
	        && (pos.z >= bbox.min.z - tolerance)
	        && (pos.z <= bbox.max.z + tolerance));
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
	return static_cast<long>(std::ceil(val));
}

bool CylinderInCylinder(const Cylinder & cyl1, const Cylinder & cyl2);
bool SphereInCylinder(const Cylinder & cyl1, const Sphere & s);

template <class T, class O>
inline T reinterpret(O v) {
	ARX_STATIC_ASSERT(sizeof(T) == sizeof(O), "can only reinterpret to type of same size");
	T t;
	memcpy(&t, &v, sizeof(T));
	return t; 
}

inline float ffsqrt(float f) {
	return reinterpret<f32, u32>(((reinterpret<u32, f32>(f) - 0x3f800000) >> 1) + 0x3f800000);
}

/*!
 * Obtain the approximated inverse of the square root of a float.
 * \brief  This code compute a fast 1 / sqrtf(v) approximation.
 * \note   Originaly from Matthew Jones (Infogrames).
 * \param  pValue  a float, the number we want the square root.
 * \return The square root of \a fValue, as a float.
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

// Matrix functions

void MatrixSetByVectors(glm::mat4x4 & m, const Vec3f & d, const Vec3f & u);
void GenerateMatrixUsingVector(glm::mat4x4 & matrix, const Vec3f & vect, float rollDegrees);

// QUATERNION Funcs/Defs

glm::quat Quat_Slerp(const glm::quat & from, glm::quat to, float t);

glm::quat QuatFromAngles(const Anglef & angle);
glm::mat4 toRotationMatrix(const Anglef & angle);

glm::quat angleToQuatForArrow(const Anglef & angle);

glm::quat angleToQuatForExtraRotation(const Anglef & angle);

std::pair<Vec3f, Vec3f> angleToFrontUpVec(const Anglef & angle);

// VECTORS Functions

inline void ZRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->y * s, in->y * c - in->x * s, in->z);
}

inline void YRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x * c + in->z * s, in->y, in->z * c - in->x * s);
}

inline void XRotatePoint(Vec3f * in, Vec3f * out, float c, float s) {
	*out = Vec3f(in->x, in->y * c - in->z * s, in->y * s + in->z * c);
}

Vec3f VRotateX(const Vec3f in, const float angle);
Vec3f VRotateY(const Vec3f in, const float angle);
Vec3f VRotateZ(const Vec3f in, const float angle);

// Rotates counterclockwise zero at (0, 0, 1);
Vec3f angleToVectorXZ(float angleDegrees);

// Rotates counterclockwise zero at (0, 0, -1);
Vec3f angleToVectorXZ_180offset(float angleDegrees);

Vec3f angleToVector(const Anglef & angle);

Vec3f CalcFaceNormal(const TexturedVertex * v);
Vec3f CalcObjFaceNormal(const Vec3f & v0, const Vec3f & v1, const Vec3f & v2);
bool Triangles_Intersect(const EERIE_TRI & v, const EERIE_TRI & u);

inline float square(float x) {
	return x * x;
}

//! \return vertical angle in radians
inline float focalToFov(float focal) {
	static const float imagePlaneHeight = 480;
	return 2 * glm::atan(imagePlaneHeight / (2 * focal));
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

inline bool PointInCylinder(const Cylinder & cyl, const Vec3f & pt) {
	
	float pos1 = cyl.origin.y + cyl.height;
	
	if(pt.y < std::min(cyl.origin.y, pos1)) {
		return false;
	}
	
	if(pt.y > std::max(cyl.origin.y, pos1)) {
		return false;
	}
	
	if(!fartherThan(Vec2f(cyl.origin.x, cyl.origin.z), Vec2f(pt.x, pt.z), cyl.radius)) {
		return true;
	}
	
	return false;
}

inline long PointInUnderCylinder(const Cylinder & cyl, const Vec3f & pt) {
	
	float pos1 = cyl.origin.y + cyl.height;
	
	if(pt.y < std::min(cyl.origin.y, pos1)) {
		return 0;
	}
	
	if(!fartherThan(Vec2f(cyl.origin.x, cyl.origin.z), Vec2f(pt.x, pt.z), cyl.radius)) {
		return (pt.y > std::max(cyl.origin.y, pos1)) ? 1 : 2;
	}
	
	return 0;
}




#ifdef ARX_DEBUG
#define checked_range_cast boost::numeric_cast
#else
#define checked_range_cast static_cast
#endif

#endif // ARX_GRAPHICS_MATH_H
