/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

/* Triangle/triangle intersection test routine,
 * int tri_tri_intersect(float V0[3],float V1[3],float V2[3],
 *                         float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 */

#define OPTIM_COMPUTE_INTERVALS(VV0, VV1, VV2, D0, D1, D2, D0D1, D0D2, A, B, C, X0, X1) \
	{ \
		if(D0D1 > 0.0f) { \
			/* here we know that D0D2 <= 0.0 */ \
			/* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
			A = VV2; \
			B = (VV0 - VV2) * D2; \
			C = (VV1 - VV2) * D2; \
			X0 = D2 - D0; \
			X1 = D2 - D1; \
		} else if(D0D2 > 0.0f) { \
			/* here we know that d0d1 <= 0.0 */ \
			A = VV1; \
			B = (VV0 - VV1) * D1; \
			C = (VV2 - VV1) * D1; \
			X0 = D1 - D0; \
			X1 = D1 - D2; \
		} else if(D1 * D2 > 0.f || D0 != 0.f) { \
			/* here we know that d0d1 <= 0 or that D0 != 0 */ \
			A = VV0; \
			B = (VV1 - VV0) * D0; \
			C = (VV2 - VV0) * D0; \
			X0 = D0 - D1; \
			X1 = D0 - D2; \
		} else if(D1 != 0.0f) { \
			A = VV1; \
			B = (VV0 - VV1) * D1; \
			C = (VV2 - VV1) * D1; \
			X0 = D1 - D0; \
			X1 = D1 - D2; \
		} else if(D2 != 0.0f) { \
			A = VV2; \
			B = (VV0 - VV2) * D2; \
			C = (VV1 - VV2) * D2; \
			X0 = D2 - D0; \
			X1 = D2 - D1; \
		} else { \
			/* triangles are coplanar */ \
			return coplanar_tri_tri(N1, V0, V1, V2, U0, U1, U2); \
		} \
	}

/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0, U0, U1) \
	Bx = U0[i0] - U1[i0];                                   \
	By = U0[i1] - U1[i1];                                   \
	Cx = V0[i0] - U0[i0];                                   \
	Cy = V0[i1] - U0[i1];                                   \
	f = Ay * Bx - Ax * By;                                      \
	d = By * Cx - Bx * Cy;                                      \
	if((f > 0 && d >= 0 && d <= f) || (f < 0 && d <= 0 && d >= f)) { \
		e = Ax * Cy - Ay * Cx;                             \
		if(f > 0) {                                               \
			if(e >= 0 && e <= f) return 1;                  \
		} else {                                               \
			if(e <= 0 && e >= f) return 1;                  \
		}                                               \
	}

#define EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2) \
	{ \
		float Ax, Ay, Bx, By, Cx, Cy, e, d, f; \
		Ax = V1[i0] - V0[i0]; \
		Ay = V1[i1] - V0[i1]; \
		/* test edge U0,U1 against V0,V1 */ \
		EDGE_EDGE_TEST(V0, U0, U1); \
		/* test edge U1,U2 against V0,V1 */ \
		EDGE_EDGE_TEST(V0, U1, U2); \
		/* test edge U2,U1 against V0,V1 */ \
		EDGE_EDGE_TEST(V0, U2, U0); \
	}

static int coplanar_tri_tri(const float N[3], const float V0[3], const float V1[3],
                            const float V2[3], const float U0[3], const float U1[3],
                            const float U2[3]) {
	
	float A[3];
	short i0, i1;
	/* first project onto an axis-aligned plane, that maximizes the area */
	/* of the triangles, compute indices: i0,i1. */
	A[0] = glm::abs(N[0]);
	A[1] = glm::abs(N[1]);
	A[2] = glm::abs(N[2]);
	
	if(A[0] > A[1]) {
		if(A[0] > A[2]) {
			i0 = 1; /* A[0] is greatest */
			i1 = 2;
		} else {
			i0 = 0; /* A[2] is greatest */
			i1 = 1;
		}
	} else {
		/* A[0]<=A[1] */
		if(A[2] > A[1]) {
			i0 = 0; /* A[2] is greatest */
			i1 = 1;
		} else {
			i0 = 0; /* A[1] is greatest */
			i1 = 2;
		}
	}
	
	/* test all edges of triangle 1 against the edges of triangle 2 */
	EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);
	
	return 0;
}

#define CROSS(dest, v1, v2) \
	dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; \
	dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; \
	dest[2] = v1[0] * v2[1] - v1[1] * v2[0];

#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2) \
	dest[0] = v1[0] - v2[0]; \
	dest[1] = v1[1] - v2[1]; \
	dest[2] = v1[2] - v2[2];

/* sort so that a<=b */
#define SORT(a, b) \
	if(a > b) { \
		std::swap(a, b); \
	}

static int tri_tri_intersect(const EERIE_TRI & VV, const EERIE_TRI & UU)  {
	
	float E1[3], E2[3];
	float N1[3], N2[3], d1, d2;
	float du0, du1, du2, dv0, dv1, dv2;
	float D[3];
	float isect1[2], isect2[2];
	float du0du1, du0du2, dv0dv1, dv0dv2;
	short index;
	float vp0, vp1, vp2;
	float up0, up1, up2;
	float bb, cc, max;
	float a, b, c, x0, x1;
	float d, e, f, y0, y1;

	float xx, yy, xxyy, tmp;

	const float * V0;
	const float * V1;
	const float * V2;
	const float * U0;
	const float * U1;
	const float * U2;

	V0 = glm::value_ptr(VV.v[0]);
	V1 = glm::value_ptr(VV.v[1]);
	V2 = glm::value_ptr(VV.v[2]);

	U0 = glm::value_ptr(UU.v[0]);
	U1 = glm::value_ptr(UU.v[1]);
	U2 = glm::value_ptr(UU.v[2]);

	/* compute plane equation of triangle(V0,V1,V2) */
	SUB(E1, V1, V0);
	SUB(E2, V2, V0);
	CROSS(N1, E1, E2);
	d1 = -DOT(N1, V0);
	/* plane equation 1: N1.X+d1=0 */

	/* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
	du0 = DOT(N1, U0) + d1;
	du1 = DOT(N1, U1) + d1;
	du2 = DOT(N1, U2) + d1;

	/* coplanarity robustness check */
	du0du1 = du0 * du1;
	du0du2 = du0 * du2;

	if (du0du1 > 0.0f && du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
		return 0;                    /* no intersection occurs */

	/* compute plane of triangle (U0,U1,U2) */
	SUB(E1, U1, U0);
	SUB(E2, U2, U0);
	CROSS(N2, E1, E2);
	d2 = -DOT(N2, U0);
	/* plane equation 2: N2.X+d2=0 */

	/* put V0,V1,V2 into plane equation 2 */
	dv0 = DOT(N2, V0) + d2;
	dv1 = DOT(N2, V1) + d2;
	dv2 = DOT(N2, V2) + d2;

	dv0dv1 = dv0 * dv1;
	dv0dv2 = dv0 * dv2;

	if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) // same sign on all of them + not equal 0 ?
		return 0;                    // no intersection occurs

	// compute direction of intersection line
	CROSS(D, N1, N2);

	// compute and index to the largest component of D
	max = glm::abs(D[0]);
	index = 0;
	bb = glm::abs(D[1]);
	cc = glm::abs(D[2]);

	if (bb > max) max = bb, index = 1;

	if (cc > max) index = 2;

	// this is the simplified projection onto L
	vp0 = V0[index];
	vp1 = V1[index];
	vp2 = V2[index];

	up0 = U0[index];
	up1 = U1[index];
	up2 = U2[index];

	// compute interval for triangle 1
	OPTIM_COMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

	// compute interval for triangle 2
	OPTIM_COMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

	xx = x0 * x1;
	yy = y0 * y1;
	xxyy = xx * yy;

	tmp = a * xxyy;
	isect1[0] = tmp + b * x1 * yy;
	isect1[1] = tmp + c * x0 * yy;

	tmp = d * xxyy;
	isect2[0] = tmp + e * xx * y1;
	isect2[1] = tmp + f * xx * y0;

	SORT(isect1[0], isect1[1]);
	SORT(isect2[0], isect2[1]);

	if (isect1[1] < isect2[0] || isect2[1] < isect1[0]) return 0;

	return 1;
}

#undef CROSS
#undef DOT
#undef SUB

// Computes Bounding Box for a triangle
static EERIE_3D_BBOX Triangle_ComputeBoundingBox(const EERIE_TRI & v) {
	EERIE_3D_BBOX bb;
	bb.min = glm::min(glm::min(v.v[0], v.v[1]), v.v[2]);
	bb.max = glm::max(glm::max(v.v[0], v.v[1]), v.v[2]);
	return bb;
}

bool Triangles_Intersect(const EERIE_TRI & v, const EERIE_TRI & u)
{
	EERIE_3D_BBOX bb1 = Triangle_ComputeBoundingBox(v);
	EERIE_3D_BBOX bb2 = Triangle_ComputeBoundingBox(u);

	if (bb1.max.y < bb2.min.y) return false;

	if (bb1.min.y > bb2.max.y) return false;

	if (bb1.max.x < bb2.min.x) return false;

	if (bb1.min.x > bb2.max.x) return false;

	if (bb1.max.z < bb2.min.z) return false;

	if (bb1.min.z > bb2.max.z) return false;
	
	return tri_tri_intersect(v, u) != 0;
}

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
	
	return !fartherThan(Vec2f(cyl1.origin.x, cyl1.origin.z), Vec2f(cyl2.origin.x, cyl2.origin.z), m1);
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
	
	return !fartherThan(Vec2f(cyl1.origin.x, cyl1.origin.z), Vec2f(s.origin.x, s.origin.z), cyl1.radius + s.radius);
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

glm::quat angleToQuatForArrow(const Anglef & angle) {
	float aa = angle.getPitch();
	float ab = 90.f - angle.getYaw();
	
	Vec3f front(0.f, 0.f, 1.f);
	Vec3f up(0.f, -1.f, 0.f);
	
	front = VRotateZ(front, aa);
	front = VRotateY(front, ab);
	up = VRotateZ(up, aa);
	up = VRotateY(up, ab);
	
	glm::mat4x4 tmat;
	MatrixSetByVectors(tmat, front, up);
	return glm::quat_cast(tmat);
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

Vec3f CalcFaceNormal(const Vec3f * v) {
	Vec3f A = v[1] - v[0];
	Vec3f B = v[2] - v[0];
	return glm::normalize(Vec3f(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x));
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
