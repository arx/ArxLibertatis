/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Math.h"

#include <algorithm>
#include <limits>

#include <glm/gtc/type_ptr.hpp>

#include "graphics/GraphicsTypes.h"


// Cylinder y origin must be min Y of cylinder
// Cylinder height MUST be negative FROM origin (inverted Theo XYZ system Legacy)
bool CylinderInCylinder(const Cylinder & cyl1, const Cylinder & cyl2) {
	
	float m1 = cyl1.origin.y;
	float m2 = cyl2.origin.y + cyl2.height;
	
	if (m2 > m1) return false;
	
	m1 = cyl1.origin.y + cyl1.height;
	m2 = cyl2.origin.y;
	
	if (m1 > m2) return false;
	
	m1 = cyl1.radius + cyl2.radius;
	
	return !fartherThan(getXZ(cyl1.origin), getXZ(cyl2.origin), m1);
}

// Sort of...
bool SphereInCylinder(const Cylinder & cyl1, const Sphere & s)
{
	float m1 = std::max(cyl1.origin.y, cyl1.origin.y + cyl1.height);
	float m2 = s.origin.y - s.radius;

	if (m2 > m1) return false;

	m1 = std::min(cyl1.origin.y, cyl1.origin.y + cyl1.height);
	m2 = s.origin.y + s.radius;

	if (m1 > m2) return false;
	
	return !fartherThan(getXZ(cyl1.origin), getXZ(s.origin), cyl1.radius + s.radius);
}

glm::quat Quat_Slerp(const glm::quat & from, glm::quat to, float ratio)
{
	float fCosTheta = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	if (fCosTheta < 0.0f)
	{
		fCosTheta = -fCosTheta;
		to.x = -to.x;
		to.y = -to.y;
		to.z = -to.z;
		to.w = -to.w;
	}

	float fBeta = 1.f - ratio;

	if (1.0f - fCosTheta > 0.001f)
	{
		float fTheta = glm::acos(fCosTheta);
		float t = 1 / std::sin(fTheta);
		fBeta = std::sin(fTheta * fBeta) * t;
		ratio = std::sin(fTheta * ratio) * t;
	}

	return glm::quat(
		fBeta * from.w + ratio * to.w,
		fBeta * from.x + ratio * to.x,
		fBeta * from.y + ratio * to.y,
		fBeta * from.z + ratio * to.z
	);
}

glm::quat QuatFromAngles(const Anglef & angle) {
	
	float A, B;
	A = glm::radians(angle.getPitch()) * 0.5f;
	B = glm::radians(angle.getYaw()) * 0.5f;
	
	float fSinYaw   = glm::sin(A);
	float fCosYaw   = glm::cos(A);
	float fSinPitch = glm::sin(B);
	float fCosPitch = glm::cos(B);
	A = glm::radians(angle.getRoll()) * 0.5f;
	float fSinRoll  = glm::sin(A);
	float fCosRoll  = glm::cos(A);
	A = fCosRoll * fCosPitch;
	B = fSinRoll * fSinPitch;
	
	glm::quat q;
	q.x = fSinRoll * fCosPitch * fCosYaw - fCosRoll * fSinPitch * fSinYaw;
	q.y = fCosRoll * fSinPitch * fCosYaw + fSinRoll * fCosPitch * fSinYaw;
	q.z = A * fSinYaw - B * fCosYaw;
	q.w = A * fCosYaw + B * fSinYaw;
	return q;
}

glm::mat4 toRotationMatrix(const Anglef & angle) {
	float pitch = glm::radians(angle.getPitch());
	float yaw = glm::radians(angle.getYaw());
	float roll = glm::radians(angle.getRoll());
	glm::mat4 rotateX = arx::eulerAngleX(pitch);
	glm::mat4 rotateY = arx::eulerAngleY(yaw);
	glm::mat4 rotateZ = arx::eulerAngleZ(-roll);
	return rotateZ * rotateX * rotateY;
}

glm::quat toQuaternion(const Anglef & angle) {
	glm::quat rotation = glm::quat(glm::vec3(0.f, glm::radians(angle.getYaw()), 0.f));
	rotation = glm::quat(glm::vec3(glm::radians(angle.getPitch()), 0.f, 0.f)) * rotation;
	return glm::quat(glm::vec3(0.f, 0.f, glm::radians(-angle.getRoll()))) * rotation;
}

Anglef toAngle(const glm::quat & quat) {
	glm::quat q = quat * glm::quat(glm::vec3(0.f, 0.f, glm::radians(90.f)));
	return Anglef(-glm::degrees(arx::yaw(q)), glm::degrees(arx::pitch(q)), 90.f - glm::degrees(arx::roll(q)));
}

glm::quat angleToQuatForExtraRotation(const Anglef & angle) {
	Anglef vt1;
	vt1.setPitch(angle.getRoll());
	vt1.setYaw(angle.getYaw());
	vt1.setRoll(angle.getPitch());
	
	return QuatFromAngles(vt1);
}

std::pair<Vec3f, Vec3f> angleToFrontUpVec(const Anglef & angle) {
	
	Vec3f front = angleToVector(angle);
	Vec3f up = angleToVector(angle + Anglef(90.f, 0, 0));
	
	arx_assert_msg(glm::abs(glm::dot(front, up)) < 10.f * std::numeric_limits<float>::epsilon(),
	               "front=(%f,%f,%f) and up=(%f,%f,%f) should be orthogonal; dot=%1f*epsilon",
	               double(front.x), double(front.y), double(front.z),
	               double(up.x), double(up.y), double(up.z),
	               double(glm::dot(front, up) / std::numeric_limits<float>::epsilon()));
	
	return std::make_pair(front, up);
}

// Rotates a Vector around X. angle is given in degrees
Vec3f VRotateX(const Vec3f in, const float angle) {
	float s = glm::radians(angle);
	float c = std::cos(s);
	s = std::sin(s);
	return Vec3f(in.x, (in.y * c) + (in.z * s), (in.z * c) - (in.y * s));
}

// Rotates a Vector around Y. angle is given in degrees
Vec3f VRotateY(const Vec3f in, const float angle) {
	float s = glm::radians(angle);
	float c = std::cos(s);
	s = std::sin(s);
	return Vec3f((in.x * c) + (in.z * s), in.y, (in.z * c) - (in.x * s));
}

// Rotates a Vector around Z. angle is given in degrees
Vec3f VRotateZ(const Vec3f in, const float angle) {
	float s = glm::radians(angle);
	float c = std::cos(s);
	s = std::sin(s);
	return Vec3f((in.x * c) + (in.y * s), (in.y * c) - (in.x * s), in.z);
}

Vec3f angleToVectorXZ(float angleDegrees) {
	float t = glm::radians(angleDegrees);
	return Vec3f(-std::sin(t), 0.f, std::cos(t));
}

Vec3f angleToVectorXZ_180offset(float angleDegrees) {
	float t = glm::radians(angleDegrees);
	return Vec3f(std::sin(t), 0.f, -std::cos(t));
}

Vec3f angleToVector(const Anglef & angle) {
	Vec3f cam_vector = angleToVectorXZ(angle.getYaw());
	
	float pitch = glm::radians(angle.getPitch());
	cam_vector.x *= std::cos(pitch);
	cam_vector.y = std::sin(pitch);
	cam_vector.z *= std::cos(pitch);
	
	return cam_vector;
}

Anglef unitVectorToAngle(const Vec3f & vector) {
	
	// y = sin(pitch) * length(vector)
	// → pitch = sin⁻¹(y / length(vector))
	float pitch = std::asin(vector.y); // assumes length(vector) == 1
	
	// x = -sin(yaw) * cos(pitch) * length(vector)
	// z = cos(yaw) * cos(pitch) * length(vector)
	// → -x / z = sin(yaw) / cos(yaw) = tan(yaw)
	// → yaw = tan⁻¹(-x / z)
	float yaw = std::atan2(-vector.x, vector.z);
	
	return Anglef(glm::degrees(pitch), glm::degrees(yaw), 0.f);
}

Anglef vectorToAngle(const Vec3f & vector) {
	return unitVectorToAngle(glm::normalize(vector));
}

Plane createNormalizedPlane(const Vec3f & origin, const Vec3f & pt1, const Vec3f & pt2) {
	
	Plane plane;
	
	plane.normal = glm::normalize(glm::cross(pt1 - origin, pt2 - origin));
	plane.offset = -glm::dot(plane.normal, origin);
	
	return plane;
}

void MatrixSetByVectors(glm::mat4x4 & m, const Vec3f & d, const Vec3f & u) {
	
	Vec3f D = glm::normalize(d);
	float t = u.x * D.x + u.y * D.y + u.z * D.z;
	Vec3f U = u - Vec3f(D.x, D.y, D.y) * t; // TODO is this really supposed to be D.y?
	U = glm::normalize(U);
	Vec3f R = glm::cross(U, D);
	
	m[0][0] = R.x;
	m[0][1] = R.y;
	m[0][2] = R.z;
	m[1][0] = U.x;
	m[1][1] = U.y;
	m[1][2] = U.z;
	m[2][0] = D.x;
	m[2][1] = D.y;
	m[2][2] = D.z;
	
}

void GenerateMatrixUsingVector(glm::mat4x4 & matrix, const Vec3f & vect, float rollDegrees) {
	
	// Get our direction vector (the Z vector component of the matrix)
	// and make sure it's normalized into a unit vector
	Vec3f zAxis = glm::normalize(vect);
	
	// Build the Y vector of the matrix (handle the degenerate case
	// in the way that 3DS does) -- This is not the true vector, only
	// a reference vector.
	Vec3f yAxis(0.f, 1.f, 0.f);
	if(zAxis.x == 0.f && zAxis.z == 0.f) {
		yAxis = Vec3f(-zAxis.y, 0.f, 0.f);
	}
	
	// Build the X axis vector based on the two existing vectors
	Vec3f xAxis = glm::normalize(glm::cross(yAxis, zAxis));
	
	// Correct the Y reference vector
	yAxis = glm::normalize(glm::cross(xAxis, zAxis));
	yAxis = -yAxis;
	
	// Generate rotation matrix without roll included
	glm::mat4x4 rot(1.f);
	rot[0][0] = yAxis.x;
	rot[0][1] = yAxis.y;
	rot[0][2] = yAxis.z;
	rot[1][0] = zAxis.x;
	rot[1][1] = zAxis.y;
	rot[1][2] = zAxis.z;
	rot[2][0] = xAxis.x;
	rot[2][1] = xAxis.y;
	rot[2][2] = xAxis.z;
	
	// Generate the Z rotation matrix for roll
	glm::mat4x4 roll(1.f);
	roll[2][2] = 1.f;
	roll[3][3] = 1.f;
	roll[0][0] =  std::cos(glm::radians(rollDegrees));
	roll[0][1] = -std::sin(glm::radians(rollDegrees));
	roll[1][0] =  std::sin(glm::radians(rollDegrees));
	roll[1][1] =  std::cos(glm::radians(rollDegrees));
	
	// Concatinate them for a complete rotation matrix that includes
	// all rotations
	matrix = roll * rot;
}
