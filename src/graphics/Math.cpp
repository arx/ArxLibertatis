/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <glm/gtc/type_ptr.hpp>

#include "graphics/GraphicsTypes.h"

using std::min;
using std::max;

/* Triangle/triangle intersection test routine,
 * int tri_tri_intersect(float V0[3],float V1[3],float V2[3],
 *                         float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 */

#define OPTIM_COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
	{ \
		if(D0D1>0.0f) \
		{ \
			/* here we know that D0D2<=0.0 */ \
			/* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
			A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
		} \
		else if(D0D2>0.0f)\
		{ \
			/* here we know that d0d1<=0.0 */ \
			A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
		} \
		else if(D1*D2>0.0f || D0!=0.0f) \
		{ \
			/* here we know that d0d1<=0.0 or that D0!=0.0 */ \
			A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
		} \
		else if(D1!=0.0f) \
		{ \
			A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
		} \
		else if(D2!=0.0f) \
		{ \
			A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
		} \
		else \
		{ \
			/* triangles are coplanar */ \
			return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
		} \
	}



/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0,U0,U1)                        \
	Bx=U0[i0]-U1[i0];                                   \
	By=U0[i1]-U1[i1];                                   \
	Cx=V0[i0]-U0[i0];                                   \
	Cy=V0[i1]-U0[i1];                                   \
	f=Ay*Bx-Ax*By;                                      \
	d=By*Cx-Bx*Cy;                                      \
	if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
	{                                                   \
		e=Ax*Cy-Ay*Cx;                                  \
		if(f>0)                                         \
		{                                               \
			if(e>=0 && e<=f) return 1;                  \
		}                                               \
		else                                            \
		{                                               \
			if(e<=0 && e>=f) return 1;                  \
		}                                               \
	}

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2)       \
	{                                                \
		float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
		Ax=V1[i0]-V0[i0];                            \
		Ay=V1[i1]-V0[i1];                            \
		/* test edge U0,U1 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U0,U1);                    \
		/* test edge U1,U2 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U1,U2);                    \
		/* test edge U2,U1 against V0,V1 */          \
		EDGE_EDGE_TEST(V0,U2,U0);                    \
	}

#define POINT_IN_TRI(V0,U0,U1,U2)                 \
	{                                             \
		float a,b,c,d0,d1,d2;                     \
		/* is T1 completly inside T2? */          \
		/* check if V0 is inside tri(U0,U1,U2) */ \
		a=U1[i1]-U0[i1];                          \
		b=-(U1[i0]-U0[i0]);                       \
		c=-a*U0[i0]-b*U0[i1];                     \
		d0=a*V0[i0]+b*V0[i1]+c;                   \
		\
		a=U2[i1]-U1[i1];                          \
		b=-(U2[i0]-U1[i0]);                       \
		c=-a*U1[i0]-b*U1[i1];                     \
		d1=a*V0[i0]+b*V0[i1]+c;                   \
		\
		a=U0[i1]-U2[i1];                          \
		b=-(U0[i0]-U2[i0]);                       \
		c=-a*U2[i0]-b*U2[i1];                     \
		d2=a*V0[i0]+b*V0[i1]+c;                   \
		if(d0*d1>0.0)                             \
		{                                         \
			if(d0*d2>0.0) return 1;               \
		}                                         \
	}

int coplanar_tri_tri(const float N[3], const float V0[3], const float V1[3], const float V2[3],
                     const float U0[3], const float U1[3], const float U2[3])
{
	float A[3];
	short i0, i1;
	/* first project onto an axis-aligned plane, that maximizes the area */
	/* of the triangles, compute indices: i0,i1. */
	A[0] = EEfabs(N[0]);
	A[1] = EEfabs(N[1]);
	A[2] = EEfabs(N[2]);

	if (A[0] > A[1])
	{
		if (A[0] > A[2])
		{
			i0 = 1;    /* A[0] is greatest */
			i1 = 2;
		}
		else
		{
			i0 = 0;    /* A[2] is greatest */
			i1 = 1;
		}
	}
	else   /* A[0]<=A[1] */
	{
		if (A[2] > A[1])
		{
			i0 = 0;    /* A[2] is greatest */
			i1 = 1;
		}
		else
		{
			i0 = 0;    /* A[1] is greatest */
			i1 = 2;
		}
	}

	/* test all edges of triangle 1 against the edges of triangle 2 */
	EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
	EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);

	return 0;
}

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2];

/* sort so that a<=b */
#define SORT(a,b)       \
	if(a>b)    \
	{          \
		float c; \
		c=a;     \
		a=b;     \
		b=c;     \
	}

//***********************************************************************************************
// Computes Intersection of 2 triangles
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/19)
// OPTIMIZED (Cyril 2001/10/19) removed divisions, need some more optims perhaps...
//***********************************************************************************************
int tri_tri_intersect(const EERIE_TRI * VV, const EERIE_TRI * UU) 
{
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

	V0 = glm::value_ptr(VV->v[0]);
	V1 = glm::value_ptr(VV->v[1]);
	V2 = glm::value_ptr(VV->v[2]);

	U0 = glm::value_ptr(UU->v[0]);
	U1 = glm::value_ptr(UU->v[1]);
	U2 = glm::value_ptr(UU->v[2]);

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
	max = (float)EEfabs(D[0]);
	index = 0;
	bb = (float)EEfabs(D[1]);
	cc = (float)EEfabs(D[2]);

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
static inline void Triangle_ComputeBoundingBox(EERIE_3D_BBOX * bb, const EERIE_TRI * v) {
	bb->min.x = min(v->v[0].x, v->v[1].x);
	bb->min.x = min(bb->min.x, v->v[2].x);

	bb->max.x = max(v->v[0].x, v->v[1].x);
	bb->max.x = max(bb->max.x, v->v[2].x);

	bb->min.y = min(v->v[0].y, v->v[1].y);
	bb->min.y = min(bb->min.y, v->v[2].y);

	bb->max.y = max(v->v[0].y, v->v[1].y);
	bb->max.y = max(bb->max.y, v->v[2].y);

	bb->min.z = min(v->v[0].z, v->v[1].z);
	bb->min.z = min(bb->min.z, v->v[2].z);

	bb->max.z = max(v->v[0].z, v->v[1].z);
	bb->max.z = max(bb->max.z, v->v[2].z);
}

bool Triangles_Intersect(const EERIE_TRI * v, const EERIE_TRI * u)
{
	EERIE_3D_BBOX bb1, bb2;
	Triangle_ComputeBoundingBox(&bb1, v);
	Triangle_ComputeBoundingBox(&bb2, u);

	if (bb1.max.y < bb2.min.y) return false;

	if (bb1.min.y > bb2.max.y) return false;

	if (bb1.max.x < bb2.min.x) return false;

	if (bb1.min.x > bb2.max.x) return false;

	if (bb1.max.z < bb2.min.z) return false;

	if (bb1.min.z > bb2.max.z) return false;

	if (tri_tri_intersect(v, u)) 
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
/*
#define X 0
#define Y 1
#define Z 2

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	else if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	else if(x2>max) max=x2;

//======================== X-tests ========================
#define AXISTEST_X01(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			       	   \
	p2 = a*v2[Y] - b*v2[Z];			       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)			   \
	p0 = a*v0[Y] - b*v0[Z];			           \
	p1 = a*v1[Y] - b*v1[Z];			       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

//======================== Y-tests ========================
#define AXISTEST_Y02(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p2 = -a*v2[X] + b*v2[Z];	       	       	   \
	if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)			   \
	p0 = -a*v0[X] + b*v0[Z];		      	   \
	p1 = -a*v1[X] + b*v1[Z];	     	       	   \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
	if(min>rad || max<-rad) return 0;

//======================== Z-tests ========================

#define AXISTEST_Z12(a, b, fa, fb)			   \
	p1 = a*v1[X] - b*v1[Y];			           \
	p2 = a*v2[X] - b*v2[Y];			       	   \
	if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)			   \
	p0 = a*v0[X] - b*v0[Y];				   \
	p1 = a*v1[X] - b*v1[Y];			           \
	if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
	rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
	if(min>rad || max<-rad) return 0;


#undef X
#undef Y
#undef Z
*/

//*******************************************************************************************
//*******************************************************************************************

// Cylinder y origin must be min Y of cylinder
// Cylinder height MUST be negative FROM origin (inverted Theo XYZ system Legacy)
bool CylinderInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_CYLINDER * cyl2)
{
	if (cyl1 == cyl2) return false;

	float m1 = cyl1->origin.y;					//tokeep: max(cyl1->origin.y,cyl1->origin.y+cyl1->height);
	float m2 = cyl2->origin.y + cyl2->height;	//tokeep: min(cyl2->origin.y,cyl2->origin.y+cyl2->height);

	if (m2 > m1) return false;

	m1 = cyl1->origin.y + cyl1->height;			//tokeep: min(cyl1->origin.y,cyl1->origin.y+cyl1->height);
	m2 = cyl2->origin.y;						//tokeep: max(cyl2->origin.y,cyl2->origin.y+cyl2->height);

	if (m1 > m2) return false;

	m1 = cyl1->radius + cyl2->radius;

	if(!fartherThan(Vec2f(cyl1->origin.x, cyl1->origin.z), Vec2f(cyl2->origin.x, cyl2->origin.z), m1)) {
		return true;
	}

	return false;
}

// Sort of...
bool SphereInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_SPHERE * s)
{
	float m1 = max(cyl1->origin.y, cyl1->origin.y + cyl1->height);
	float m2 = s->origin.y - s->radius;

	if (m2 > m1) return false;

	m1 = min(cyl1->origin.y, cyl1->origin.y + cyl1->height);
	m2 = s->origin.y + s->radius;

	if (m1 > m2) return false;

	if(!fartherThan(Vec2f(cyl1->origin.x, cyl1->origin.z), Vec2f(s->origin.x, s->origin.z), cyl1->radius + s->radius)) {
		return true;
	}
	
	return false;
}

//--------------------------------------------------------------------------------------
// Quaternions Funcs
//--------------------------------------------------------------------------------------

//*************************************************************************************
// Multiply Quaternion 'q1' by Quaternion 'q2', returns result in Quaternion 'dest'
//*************************************************************************************
EERIE_QUAT Quat_Multiply(const EERIE_QUAT & q1, const EERIE_QUAT & q2)
{
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

	return EERIE_QUAT(
		q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
		q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z,
		q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x,
		q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z);
}

//*************************************************************************************
// Invert Multiply of a quaternion by another quaternion
//*************************************************************************************
void Quat_Divide(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2)
{
	dest->x = q1->w * q2->x - q1->x * q2->w - q1->y * q2->z + q1->z * q2->y;
	dest->y = q1->w * q2->y - q1->y * q2->w - q1->z * q2->x + q1->x * q2->z;
	dest->z = q1->w * q2->z - q1->z * q2->w - q1->x * q2->y + q1->y * q2->x;
	dest->w = q1->w * q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z;
}

// Invert-Transform of vertex by a quaternion
void TransformInverseVertexQuat(const EERIE_QUAT * quat, const Vec3f * vertexin,
                                Vec3f * vertexout) {
	
	EERIE_QUAT rev_quat = *quat;
	Quat_Reverse(&rev_quat);
	
	float x = vertexin->x;
	float y = vertexin->y;
	float z = vertexin->z;
	
	float qx = rev_quat.x;
	float qy = rev_quat.y;
	float qz = rev_quat.z;
	float qw = rev_quat.w;
	
	float rx = x * qw - y * qz + z * qy;
	float ry = y * qw - z * qx + x * qz;
	float rz = z * qw - x * qy + y * qx;
	float rw = x * qx + y * qy + z * qz;
	
	vertexout->x = qw * rx + qx * rw + qy * rz - qz * ry;
	vertexout->y = qw * ry + qy * rw + qz * rx - qx * rz;
	vertexout->z = qw * rz + qz * rw + qx * ry - qy * rx;
}


EERIE_QUAT Quat_Slerp(const EERIE_QUAT & from, EERIE_QUAT to, float ratio)
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
		float fTheta = acosf(fCosTheta);
		float t = 1 / EEsin(fTheta);
		fBeta  = EEsin(fTheta * fBeta) * t ;
		ratio = EEsin(fTheta * ratio) * t ;
	}

	return EERIE_QUAT(
		fBeta * from.x + ratio * to.x,
		fBeta * from.y + ratio * to.y,
		fBeta * from.z + ratio * to.z,
		fBeta * from.w + ratio * to.w);
}



//*************************************************************************************
// Inverts a Quaternion
//*************************************************************************************
void Quat_Reverse(EERIE_QUAT * q)
{
	EERIE_QUAT qw, qr;
	Quat_Init(&qw);
	Quat_Divide(&qr, q, &qw);
	*q = qr;
}


//*************************************************************************************
// Converts euler angles to a unit quaternion.
//*************************************************************************************
void QuatFromAngles(EERIE_QUAT * q, const Anglef * angle)

{
	float A, B;
	A = angle->getYaw() * ( 1.0f / 2 );
	B = angle->getPitch() * ( 1.0f / 2 );

	float fSinYaw   = sinf(A);
	float fCosYaw   = cosf(A);
	float fSinPitch = sinf(B);
	float fCosPitch = cosf(B);
	A = angle->getRoll() * ( 1.0f / 2 );
	float fSinRoll  = sinf(A);
	float fCosRoll  = cosf(A);
	A = fCosRoll * fCosPitch;
	B = fSinRoll * fSinPitch;
	q->x = fSinRoll * fCosPitch * fCosYaw - fCosRoll * fSinPitch * fSinYaw;
	q->y = fCosRoll * fSinPitch * fCosYaw + fSinRoll * fCosPitch * fSinYaw;
	q->z = A * fSinYaw - B * fCosYaw;
	q->w = A * fCosYaw + B * fSinYaw;

}

void worldAngleToQuat(EERIE_QUAT *dest, const Anglef & src, bool isNpc) {

	if(!isNpc) {
		// To correct invalid angle in Animated FIX/ITEMS
		Anglef ang = src;
		ang.setYaw(360 - ang.getYaw());
		
		EERIEMATRIX mat;
		Vec3f vect(0, 0, 1);
		Vec3f up(0, 1, 0);
		VRotateY(&vect, ang.getPitch());
		VRotateX(&vect, ang.getYaw());
		VRotateZ(&vect, ang.getRoll());
		VRotateY(&up, ang.getPitch());
		VRotateX(&up, ang.getYaw());
		VRotateZ(&up, ang.getRoll());
		MatrixSetByVectors(&mat, &vect, &up);
		QuatFromMatrix(*dest, mat);
	} else {
		Anglef vt1 = Anglef(radians(src.getYaw()), radians(src.getPitch()), radians(src.getRoll()));
		QuatFromAngles(dest, &vt1);
	}
}


//*************************************************************************************
// Converts a unit quaternion into a rotation matrix.
//*************************************************************************************

void MatrixFromQuat(EERIEMATRIX * m, const EERIE_QUAT * quat)
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = quat->x + quat->x;
	y2 = quat->y + quat->y;
	z2 = quat->z + quat->z;
	xx = quat->x * x2;
	xy = quat->x * y2;
	xz = quat->x * z2;
	yy = quat->y * y2;
	yz = quat->y * z2;
	zz = quat->z * z2;
	wx = quat->w * x2;
	wy = quat->w * y2;
	wz = quat->w * z2;

	m->_11 = 1.0F - (yy + zz);
	m->_21 = xy - wz;
	m->_31 = xz + wy;
	m->_41 = 0.0F;

	m->_12 = xy + wz;
	m->_22 = 1.0F - (xx + zz);
	m->_32 = yz - wx;
	m->_42 = 0.0F;

	m->_13 = xz - wy;
	m->_23 = yz + wx;
	m->_33 = 1.0F - (xx + yy);
	m->_43 = 0.0F;
}

//*************************************************************************************
// Converts a rotation matrix into a unit quaternion.
//*************************************************************************************
void QuatFromMatrix(EERIE_QUAT & quat, EERIEMATRIX & mat)
{
	float m[4][4];
	m[0][0] = mat._11;
	m[0][1] = mat._12;
	m[0][2] = mat._13;
	m[0][3] = mat._14;
	m[1][0] = mat._21;
	m[1][1] = mat._22;
	m[1][2] = mat._23;
	m[1][3] = mat._24;
	m[2][0] = mat._31;
	m[2][1] = mat._32;
	m[2][2] = mat._33;
	m[2][3] = mat._34;
	m[3][0] = mat._41;
	m[3][1] = mat._42;
	m[3][2] = mat._43;
	m[3][3] = mat._44;
	float  tr, s, q[4];

	int nxt[3] = {1, 2, 0};

	tr = m[0][0] + m[1][1] + m[2][2];

	// check the diagonal
	if (tr > 0.0f)
	{
		s = sqrt(tr + 1.0f);
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

		s = sqrt((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

		q[i] = s * 0.5f;

		if (s != 0.0) s = 0.5f / s;

		q[3] = (m[j][k] - m[k][j]) * s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;


		quat.x = q[0];
		quat.y = q[1];
		quat.z = q[2];
		quat.w = q[3];
	}
}

//--------------------------------------------------------------------------------------
// VECTORS Functions
//--------------------------------------------------------------------------------------

// Rotates a Vector around X. angle is given in degrees
void VRotateX(Vec3f * out, const float angle) {
	Vec3f in = *out;
	float s = radians(angle);
	float c = EEcos(s);
	s = EEsin(s);
	*out = Vec3f(in.x, (in.y * c) + (in.z * s), (in.z * c) - (in.y * s));
}

// Rotates a Vector around Y. angle is given in degrees
void VRotateY(Vec3f * out, const float angle) {
	Vec3f in = *out;
	float s = radians(angle);
	float c = EEcos(s);
	s = EEsin(s);
	*out = Vec3f((in.x * c) + (in.z * s), in.y, (in.z * c) - (in.x * s));
}

// Rotates a Vector around Z. angle is given in degrees
void VRotateZ(Vec3f * out, const float angle) {
	Vec3f in = *out;
	float s = radians(angle);
	float c = EEcos(s);
	s = EEsin(s);
	*out = Vec3f((in.x * c) + (in.y * s), (in.y * c) - (in.x * s), in.z);
}

// Rotates a Vector around Y. angle is given in degrees
void Vector_RotateY(Vec3f * dest, const Vec3f * src, const float angle) {
	float s = radians(angle);
	float c = EEcos(s);
	s = EEsin(s);
	*dest = Vec3f((src->x * c) + (src->z * s), src->y, (src->z * c) - (src->x * s));
}

// Rotates a Vector around Z. angle is given in degrees
void Vector_RotateZ(Vec3f * dest, const Vec3f * src, const float angle) {
	float s = radians(angle);
	float c = EEcos(s);
	s = EEsin(s);
	*dest = Vec3f((src->x * c) + (src->y * s), (src->y * c) - (src->x * s), src->z);
}

//A x B = <Ay*Bz - Az*By, Az*Bx - Ax*Bz, Ax*By - Ay*Bx>
void CalcFaceNormal(EERIEPOLY * ep, const TexturedVertex * v) {
	
	float Ax, Ay, Az, Bx, By, Bz;
	Ax = v[1].p.x - v[0].p.x;
	Ay = v[1].p.y - v[0].p.y;
	Az = v[1].p.z - v[0].p.z;
	
	Bx = v[2].p.x - v[0].p.x;
	By = v[2].p.y - v[0].p.y;
	Bz = v[2].p.z - v[0].p.z;
	
	ep->norm = Vec3f(Ay * Bz - Az * By, Az * Bx - Ax * Bz, Ax * By - Ay * Bx);
	fnormalize(ep->norm);
}

void CalcObjFaceNormal(const Vec3f * v0, const Vec3f * v1, const Vec3f * v2,
                       EERIE_FACE * ef) {
	
	float Ax, Ay, Az, Bx, By, Bz;
	Ax = v1->x - v0->x;
	Ay = v1->y - v0->y;
	Az = v1->z - v0->z;
	Bx = v2->x - v0->x;
	By = v2->y - v0->y;
	Bz = v2->z - v0->z;
	
	ef->norm = Vec3f(Ay * Bz - Az * By, Az * Bx - Ax * Bz, Ax * By - Ay * Bx);
	ef->norm = glm::normalize(ef->norm);
}

void MatrixSetByVectors(EERIEMATRIX * m, const Vec3f * d, const Vec3f * u)
{
	float t;
	Vec3f D, U, R;
	D = glm::normalize(*d);
	U = *u;
	t = U.x * D.x + U.y * D.y + U.z * D.z;
	U.x -= D.x * t;
	U.y -= D.y * t;
	U.z -= D.y * t; // TODO is this really supposed to be D.y?
	U = glm::normalize(U);
	R = glm::cross(U, D);
	m->_11 = R.x;
	m->_12 = R.y;
	m->_21 = U.x;
	m->_22 = U.y;
	m->_31 = D.x;
	m->_32 = D.y;
	m->_33 = D.z;
	m->_13 = R.z;
	m->_23 = U.z;
}

void GenerateMatrixUsingVector(EERIEMATRIX * matrix, const Vec3f * vect, float rollDegrees)
{
	// Get our direction vector (the Z vector component of the matrix)
	// and make sure it's normalized into a unit vector
	Vec3f zAxis = glm::normalize(*vect);

	// Build the Y vector of the matrix (handle the degenerate case
	// in the way that 3DS does) -- This is not the true vector, only
	// a reference vector.
	Vec3f yAxis;

	if (!zAxis.x && !zAxis.z)
		yAxis = Vec3f(-zAxis.y, 0.f, 0.f);
	else
		yAxis = Vec3f(0.f, 1.f, 0.f);

	// Build the X axis vector based on the two existing vectors
	Vec3f xAxis = glm::normalize(glm::cross(yAxis, zAxis));

	// Correct the Y reference vector
	yAxis = glm::normalize(glm::cross(xAxis, zAxis));
	yAxis = -yAxis;

	// Generate rotation matrix without roll included
	EERIEMATRIX rot;
	EERIEMATRIX roll;
	rot._11 = yAxis.x;
	rot._12 = yAxis.y;
	rot._13 = yAxis.z;
	rot._21 = zAxis.x;
	rot._22 = zAxis.y;
	rot._23 = zAxis.z;
	rot._31 = xAxis.x;
	rot._32 = xAxis.y;
	rot._33 = xAxis.z;

	// Generate the Z rotation matrix for roll
	roll._33 = 1.f;
	roll._44 = 1.f;
	roll._11 = EEcos(radians(rollDegrees));
	roll._12 = -EEsin(radians(rollDegrees));
	roll._21 = EEsin(radians(rollDegrees));
	roll._22 = EEcos(radians(rollDegrees));

	// Concatinate them for a complete rotation matrix that includes
	// all rotations
	MatrixMultiply(matrix, &rot, &roll);
}


//-----------------------------------------------------------------------------
// MatrixMultiply()
// Does the matrix operation: [Q] = [A] * [B]. Note that the order of
// this operation was changed from the previous version of the DXSDK.
//-----------------------------------------------------------------------------
void MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b)
{
	const float * pA = &a->_11;
	const float * pB = &b->_11;
	float pM[16];

	memset(pM, 0, sizeof(EERIEMATRIX));

	for (size_t i = 0; i < 4; i++)
		for (size_t j = 0; j < 4; j++)
			for (size_t k = 0; k < 4; k++)
				pM[4*i+j] +=  pA[4*i+k] * pB[4*k+j];

	memcpy(q, pM, sizeof(EERIEMATRIX));
}

// Desc: Multiplies a vector by a matrix
void VectorMatrixMultiply(Vec3f * vDest, const Vec3f * vSrc, const EERIEMATRIX * mat) {
	float x = vSrc->x * mat->_11 + vSrc->y * mat->_21 + vSrc->z * mat->_31 + mat->_41;
	float y = vSrc->x * mat->_12 + vSrc->y * mat->_22 + vSrc->z * mat->_32 + mat->_42;
	float z = vSrc->x * mat->_13 + vSrc->y * mat->_23 + vSrc->z * mat->_33 + mat->_43;
	*vDest = Vec3f(x, y, z);
}
