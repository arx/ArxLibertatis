/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIEMath
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define D3D_OVERLOADS
#define STRICT

#include <stdio.h>

#include "EERIEApp.h"
#include "EERIEPoly.h"
#include "EERIETypes.h"
#include "EerieMath.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


 

#ifdef ASSEMBLER_OPTIMS	 //////////////////////
float __mov;
#endif



/* Triangle/triangle intersection test routine,
 * int tri_tri_intersect(float V0[3],float V1[3],float V2[3],
 *                         float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 */
#define USE_EPSILON_TEST FALSE
#define USE_POINT_IN_TRI FALSE

#define COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1) \
	if(D0D1>0.0f)                                         \
	{                                                     \
		/* here we know that D0D2<=0.0 */                   \
		/* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
		ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
	}                                                     \
	else if(D0D2>0.0f)                                    \
	{                                                     \
		/* here we know that d0d1<=0.0 */                   \
		ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
	}                                                     \
	else if(D1*D2>0.0f || D0!=0.0f)                       \
	{                                                     \
		/* here we know that d0d1<=0.0 or that D0!=0.0 */   \
		ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1);          \
	}                                                     \
	else if(D1!=0.0f)                                     \
	{                                                     \
		ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
	}                                                     \
	else if(D2!=0.0f)                                     \
	{                                                     \
		ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
	}                                                     \
	else                                                  \
	{                                                     \
		/* triangles are coplanar */                        \
		return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
	}

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
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
	Bx=U0[i0]-U1[i0];                                   \
	By=U0[i1]-U1[i1];                                   \
	Cx=V0[i0]-U0[i0];                                   \
	Cy=V0[i1]-U0[i1];                                   \
	f=Ay*Bx-Ax*By;                                      \
	d=By*Cx-Bx*Cy;                                      \
	if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
	{                                                   \
		e=Ax*Cy-Ay*Cx;                                    \
		if(f>0)                                           \
		{                                                 \
			if(e>=0 && e<=f) return 1;                      \
		}                                                 \
		else                                              \
		{                                                 \
			if(e<=0 && e>=f) return 1;                      \
		}                                                 \
	}

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
	{                                              \
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

#define POINT_IN_TRI(V0,U0,U1,U2)           \
	{                                           \
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
			if(d0*d2>0.0) return 1;                 \
		}                                         \
	}

int coplanar_tri_tri(float N[3], float V0[3], float V1[3], float V2[3],
                     float U0[3], float U1[3], float U2[3])
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

	// Considered Impossible
#if USE_EPSILON_TEST==TRUE
	// finally, test if tri1 is totally contained in tri2 or vice versa
	POINT_IN_TRI(V0, U0, U1, U2);
	POINT_IN_TRI(U0, V0, V1, V2);
#endif
	return 0;
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

	float * V0;
	float * V1;
	float * V2;
	float * U0;
	float * U1;
	float * U2;
	V0 = (float *)&VV->v[0];
	V1 = (float *)&VV->v[1];
	V2 = (float *)&VV->v[2];

	U0 = (float *)&UU->v[0];
	U1 = (float *)&UU->v[1];
	U2 = (float *)&UU->v[2];

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
#if USE_EPSILON_TEST==TRUE

	if (EEfabs(du0) < EPSILON) du0 = 0.0;

	if (EEfabs(du1) < EPSILON) du1 = 0.0;

	if (EEfabs(du2) < EPSILON) du2 = 0.0;

#endif
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

#if USE_EPSILON_TEST==TRUE

	if (EEfabs(dv0) < EPSILON) dv0 = 0.0;

	if (EEfabs(dv1) < EPSILON) dv1 = 0.0;

	if (EEfabs(dv2) < EPSILON) dv2 = 0.0;

#endif

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

	if (cc > max) max = cc, index = 2;

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

// Computes Bounding Box for a triangle
inline void Triangle_ComputeBoundingBox(EERIE_3D_BBOX * bb, const EERIE_TRI * v)
{
	bb->min.x = __min(v->v[0].x, v->v[1].x);
	bb->min.x = __min(bb->min.x, v->v[2].x);

	bb->max.x = __max(v->v[0].x, v->v[1].x);
	bb->max.x = __max(bb->max.x, v->v[2].x);

	bb->min.y = __min(v->v[0].y, v->v[1].y);
	bb->min.y = __min(bb->min.y, v->v[2].y);

	bb->max.y = __max(v->v[0].y, v->v[1].y);
	bb->max.y = __max(bb->max.y, v->v[2].y);

	bb->min.z = __min(v->v[0].z, v->v[1].z);
	bb->min.z = __min(bb->min.z, v->v[2].z);

	bb->max.z = __max(v->v[0].z, v->v[1].z);
	bb->max.z = __max(bb->max.z, v->v[2].z);
}

BOOL Triangles_Intersect(const EERIE_TRI * v, const EERIE_TRI * u)
{
	EERIE_3D_BBOX bb1, bb2;
	Triangle_ComputeBoundingBox(&bb1, v);
	Triangle_ComputeBoundingBox(&bb2, u);

	if (bb1.max.y < bb2.min.y) return FALSE;

	if (bb1.min.y > bb2.max.y) return FALSE;

	if (bb1.max.x < bb2.min.x) return FALSE;

	if (bb1.min.x > bb2.max.x) return FALSE;

	if (bb1.max.z < bb2.min.z) return FALSE;

	if (bb1.min.z > bb2.max.z) return FALSE;

	if (tri_tri_intersect(v, u)) 
		return TRUE;

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////

#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
	min = max = x0;   \
	if(x1<min) min=x1;\
	else if(x1>max) max=x1;\
	if(x2<min) min=x2;\
	else if(x2>max) max=x2;

/*======================== X-tests ========================*/
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

/*======================== Y-tests ========================*/
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

/*======================== Z-tests ========================*/

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

//*************************************************************************************
//*************************************************************************************
float InterpolateAngle(float a1, float a2, const float pour)
{
	a1 = MAKEANGLE(a1);
	a2 = MAKEANGLE(a2);
	float t1 = a1 - a2;
	float t2 = a2 - a1;
	float t3 = 360.f + a1 - a2;
	float t4 = 360.f + a2 - a1;
	float ft1 = EEfabs(t1);
	float ft2 = EEfabs(t2);
	float ft3 = EEfabs(t3);
	float ft4 = EEfabs(t4);
	float t = __min(ft1, ft2);
	t = __min(t, ft3);
	t = __min(t, ft4);

	float i;

	if (t == ft1)
	{
		i = ft1 * pour;

		if (a1 < a2)
			a1 = a2 - i;
		else a1 = a2 + i;

		return MAKEANGLE(a2);
	}

	if (t == ft2)
	{
		i = ft2 * pour;

		if (a2 < a1)
			a2 = a1 - i;
		else a2 = a1 + i;

		return MAKEANGLE(a2);
	}

	if (t == ft3)
	{
		i = ft3 * pour;

		if (a1 < a2)
			a1 = a2 - i;
		else a1 = a2 + i;

		return MAKEANGLE(a2);
	}
	else
	{
		i = ft4 * pour;

		if (a2 < a1)
			a2 = a1 + i;
		else a2 = a1 - i;

		return MAKEANGLE(a2);
	}
}

//*******************************************************************************************
//*******************************************************************************************

// Cylinder y origin must be min Y of cylinder
// Cylinder height MUST be negative FROM origin (inverted Theo XYZ system Legacy)
BOOL CylinderInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_CYLINDER * cyl2)
{
	if (cyl1 == cyl2) return FALSE;

	float m1 = cyl1->origin.y;					//tokeep: __max(cyl1->origin.y,cyl1->origin.y+cyl1->height);
	float m2 = cyl2->origin.y + cyl2->height;	//tokeep: __min(cyl2->origin.y,cyl2->origin.y+cyl2->height);

	if (m2 > m1) return FALSE;

	m1 = cyl1->origin.y + cyl1->height;			//tokeep: __min(cyl1->origin.y,cyl1->origin.y+cyl1->height);
	m2 = cyl2->origin.y;						//tokeep: __max(cyl2->origin.y,cyl2->origin.y+cyl2->height);

	if (m1 > m2) return FALSE;

	m1 = cyl1->radius + cyl2->radius;

	if (SquaredDistance2D(cyl1->origin.x, cyl1->origin.z, cyl2->origin.x, cyl2->origin.z)
	        <= m1 * m1)
		return TRUE;

	return FALSE;
}
// Sort of...
BOOL SphereInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_SPHERE * s)
{
	float m1 = __max(cyl1->origin.y, cyl1->origin.y + cyl1->height);
	float m2 = s->origin.y - s->radius;

	if (m2 > m1) return FALSE;

	m1 = __min(cyl1->origin.y, cyl1->origin.y + cyl1->height);
	m2 = s->origin.y + s->radius;

	if (m1 > m2) return FALSE;

	// Using Squared dists
	if (SquaredDistance2D(cyl1->origin.x,
	                      cyl1->origin.z,
	                      s->origin.x,
	                      s->origin.z)
	        <= (cyl1->radius + s->radius)*(cyl1->radius + s->radius)) return TRUE;

	return FALSE;
}

// Returned in Radians ! use RAD2DEG() to convert it to degrees
float	GetAngle(const float x0, const float y0, const float x1, const float y1)
{
	register float x, y;
	x = x1 - x0;
	y = y1 - y0;

	if (x > 0.f)
	{
		if (y >= 0.f)	return (EEdef_PI_0_75 + (EEatan(y / x)));
		else return (EEdef_PI_0_75 - (EEatan(EEfabs(y) / x)));   
	}
	else if (x < 0.f)
	{
		if (y > 0.f)	return (EEdef_PI_DIV_2 - (EEatan(y / EEfabs(x))));
		else return (EEdef_PI_DIV_2 + (EEatan(y / x)));
	}
	else if (y < 0) return EEdef_PI;
	else return 0.f;
}

//--------------------------------------------------------------------------------------
// Quaternions Funcs
//--------------------------------------------------------------------------------------

//*************************************************************************************
// Multiply Quaternion 'q1' by Quaternion 'q2', returns result in Quaternion 'dest'
//*************************************************************************************
void Quat_Multiply(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2)
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

	dest->x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
	dest->y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
	dest->z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
	dest->w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
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

//*************************************************************************************
// Invert-Transform of vertex by a quaternion
//*************************************************************************************
void TransformInverseVertexQuat(const EERIE_QUAT * quat, const EERIE_3D * vertexin, EERIE_3D * vertexout)
{
	register EERIE_QUAT rev_quat;

	Quat_Copy(&rev_quat, (EERIE_QUAT *) quat);
	Quat_Reverse(&rev_quat);

	register float x = vertexin->x;
	register float y = vertexin->y;
	register float z = vertexin->z;

	register float qx = rev_quat.x;
	register float qy = rev_quat.y;
	register float qz = rev_quat.z;
	register float qw = rev_quat.w;

	register float rx = x * qw - y * qz + z * qy;
	register float ry = y * qw - z * qx + x * qz;
	register float rz = z * qw - x * qy + y * qx;
	register float rw = x * qx + y * qy + z * qz;

	vertexout->x = qw * rx + qx * rw + qy * rz - qz * ry;
	vertexout->y = qw * ry + qy * rw + qz * rx - qx * rz;
	vertexout->z = qw * rz + qz * rw + qx * ry - qy * rx;
}


void Quat_Slerp(EERIE_QUAT * result, const EERIE_QUAT * from, EERIE_QUAT * to, float ratio)
{
	FLOAT fCosTheta = from->x * to->x + from->y * to->y + from->z * to->z + from->w * to->w;

	if (fCosTheta < 0.0f)
	{
		fCosTheta = -fCosTheta;
		to->x = -to->x;
		to->y = -to->y;
		to->z = -to->z;
		to->w = -to->w;
	}

	FLOAT fBeta = 1.f - ratio;

	if (1.0f - fCosTheta > 0.001f)
	{
		FLOAT fTheta = acosf(fCosTheta);
		float t = 1 / EEsin(fTheta);
		fBeta  = EEsin(fTheta * fBeta) * t ;
		ratio = EEsin(fTheta * ratio) * t ;
	}

	result->x = fBeta * from->x + ratio * to->x;
	result->y = fBeta * from->y + ratio * to->y;
	result->z = fBeta * from->z + ratio * to->z;
	result->w = fBeta * from->w + ratio * to->w;
}



//*************************************************************************************
// Inverts a Quaternion
//*************************************************************************************
void Quat_Reverse(EERIE_QUAT * q)
{
	EERIE_QUAT qw, qr;
	Quat_Init(&qw);
	Quat_Divide(&qr, q, &qw);
	Quat_Copy(q, &qr);

}


//*************************************************************************************
// Converts euler angles to a unit quaternion.
//*************************************************************************************
void QuatFromAngles(EERIE_QUAT * q, const EERIE_3D * angle)

{
	float A, B;
	A = angle->yaw * DIV2;
	B = angle->pitch * DIV2;

	float fSinYaw   = sinf(A);
	float fCosYaw   = cosf(A);
	float fSinPitch = sinf(B);
	float fCosPitch = cosf(B);
	A = angle->roll * DIV2;
	float fSinRoll  = sinf(A);
	float fCosRoll  = cosf(A);
	A = fCosRoll * fCosPitch;
	B = fSinRoll * fSinPitch;
	q->x = fSinRoll * fCosPitch * fCosYaw - fCosRoll * fSinPitch * fSinYaw;
	q->y = fCosRoll * fSinPitch * fCosYaw + fSinRoll * fCosPitch * fSinYaw;
	q->z = A * fSinYaw - B * fCosYaw;
	q->w = A * fCosYaw + B * fSinYaw;

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
	int    i, j, k;


	int nxt[3] = {1, 2, 0};


	tr = m[0][0] + m[1][1] + m[2][2];


	// check the diagonal
	if (tr > 0.0f)
	{
		s = sqrt(tr + 1.0f);
		quat.w = s * DIV2;
		s = 0.5f / s;
		quat.x = (m[1][2] - m[2][1]) * s;
		quat.y = (m[2][0] - m[0][2]) * s;
		quat.z = (m[0][1] - m[1][0]) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;

		if (m[1][1] > m[0][0]) i = 1;

		if (m[2][2] > m[i][i]) i = 2;

		j = nxt[i];
		k = nxt[j];

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
// ANGLES Functions
//--------------------------------------------------------------------------------------

/*----------------------------------------------------------------------------*/
/*
	angle de 0 a 360(par exemple)
*/
float AngleDifference(const float d, const float e)
{
	register float	da;
	da = e - d;

	if (EEfabs(da) > 180.f)
	{
		if (da > 0) da -= 360.f;
		else da += 360.f;
	}

	return da;
}

//--------------------------------------------------------------------------------------
// VECTORS Functions
//--------------------------------------------------------------------------------------

//*************************************************************************************
// Rotates a Vector around X. angle is given in degrees
//*************************************************************************************
void VRotateX(EERIE_3D * out, const float angle)
{
	EERIE_3D in;
	in.x = out->x;
	in.y = out->y;
	in.z = out->z;
	register float s = DEG2RAD(angle);
	register float c = EEcos(s);
	s = EEsin(s);
	out->x = in.x;
	out->y = (in.y * c) + (in.z * s);
	out->z = (in.z * c) - (in.y * s);
}

//*************************************************************************************
// Rotates a Vector around Y. angle is given in degrees
//*************************************************************************************
void VRotateY(EERIE_3D * out, const float angle)
{
	EERIE_3D in;
	in.x = out->x;
	in.y = out->y;
	in.z = out->z;
	register float s = DEG2RAD(angle);
	register float c = EEcos(s);
	s = EEsin(s);
	out->x = (in.x * c) + (in.z * s);
	out->y = in.y;
	out->z = (in.z * c) - (in.x * s);
}
//*************************************************************************************
// Rotates a Vector around Z. angle is given in degrees
//*************************************************************************************
void VRotateZ(EERIE_3D * out, const float angle)
{
	EERIE_3D in;
	in.x = out->x;
	in.y = out->y;
	in.z = out->z;
	register float s = DEG2RAD(angle);
	register float c = EEcos(s);
	s = EEsin(s);
	out->x = (in.x * c) + (in.y * s);
	out->y = (in.y * c) - (in.x * s);
	out->z = in.z;
}

//*************************************************************************************
// Rotates a Vector around Y. angle is given in degrees
//*************************************************************************************
void Vector_RotateY(EERIE_3D * dest, const EERIE_3D * src, const float angle)
{
	register float s = DEG2RAD(angle);
	register float c = EEcos(s);
	s = EEsin(s);
	dest->x = (src->x * c) + (src->z * s);
	dest->y = src->y;
	dest->z = (src->z * c) - (src->x * s);
}
//*************************************************************************************
// Rotates a Vector around Z. angle is given in degrees
//*************************************************************************************
void Vector_RotateZ(EERIE_3D * dest, const EERIE_3D * src, const float angle)
{
	register float s = DEG2RAD(angle);
	register float c = EEcos(s);
	s = EEsin(s);
	dest->x = (src->x * c) + (src->y * s);
	dest->y = (src->y * c) - (src->x * s);
	dest->z = src->z;
}

//*************************************************************************************
// Subtract v2 from v1 giving "dest"
//*************************************************************************************
void Vector_Sub(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2)
{
	dest->x = v1->x - v2->x;
	dest->y = v1->y - v2->y;
	dest->z = v1->z - v2->z;
}
//*************************************************************************************
// Adds 2 vectors together giving "dest"
//*************************************************************************************
void Vector_Add(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2)
{
	dest->x = v1->x + v2->x;
	dest->y = v1->y + v2->y;
	dest->z = v1->z + v2->z;
}
//*************************************************************************************
// Returns TRUE if vector v1 equals vector v2
//*************************************************************************************
bool Vector_Compare(const EERIE_3D * v1, const EERIE_3D * v2)
{
	return ((v1->x == v2->x)
	        &&	(v1->y == v2->y)
	        &&	(v1->z == v2->z));
}
//*************************************************************************************
// Copy Vector "src" in vector "dest"
//*************************************************************************************
void Vector_Copy(EERIE_3D * dest, const EERIE_3D * src)
{
	dest->x = src->x;
	dest->y = src->y;
	dest->z = src->z;
}
//*************************************************************************************
// Computes Cross Product of vectors "v1" & "v2" giving vector "dest"
//*************************************************************************************
void Vector_CrossProduct(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2)
{
	dest->x = v1->y * v2->z - v1->z * v2->y;
	dest->y = v1->z * v2->x - v1->x * v2->z;
	dest->z = v1->x * v2->y - v1->y * v2->x;
}
//*************************************************************************************
// Returns Dot Product of 2 vectors
//*************************************************************************************
float Vector_DotProduct(const EERIE_3D * v1, const EERIE_3D * v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

///////////////////////////////////////////////////////////////////////////////

void EERIEMathPrecalc()
{
	
}

long F2L_RoundUp(float val)
{
	long l;
	F2L(val, &l);
	val = val - (float)l;

	if (val > 0.f) return l + 1;

	return l;
}

//A x B = <Ay*Bz - Az*By, Az*Bx - Ax*Bz, Ax*By - Ay*Bx>
void CalcFaceNormal(EERIEPOLY * ep, const D3DTLVERTEX * v)
{
	register float Ax, Ay, Az, Bx, By, Bz, epnlen;
	Ax = v[1].sx - v[0].sx;
	Ay = v[1].sy - v[0].sy;
	Az = v[1].sz - v[0].sz;

	Bx = v[2].sx - v[0].sx;
	By = v[2].sy - v[0].sy;
	Bz = v[2].sz - v[0].sz;

	ep->norm.x = Ay * Bz - Az * By;
	ep->norm.y = Az * Bx - Ax * Bz;
	ep->norm.z = Ax * By - Ay * Bx;

	epnlen = (float)EEsqrt(ep->norm.x * ep->norm.x + ep->norm.y * ep->norm.y + ep->norm.z * ep->norm.z);
	epnlen = 1.f / epnlen;
	ep->norm.x *= epnlen;
	ep->norm.y *= epnlen;
	ep->norm.z *= epnlen;
}

void CalcObjFaceNormal(const EERIE_3D * v0, const EERIE_3D * v1, const EERIE_3D * v2, EERIE_FACE * ef)
{
	register float Ax, Ay, Az, Bx, By, Bz, epnlen;
	Ax = v1->x - v0->x;
	Ay = v1->y - v0->y;
	Az = v1->z - v0->z;

	Bx = v2->x - v0->x;
	By = v2->y - v0->y;
	Bz = v2->z - v0->z;

	ef->norm.x = Ay * Bz - Az * By;
	ef->norm.y = Az * Bx - Ax * Bz;
	ef->norm.z = Ax * By - Ay * Bx;

	epnlen = (float)sqrt(ef->norm.x * ef->norm.x + ef->norm.y * ef->norm.y + ef->norm.z * ef->norm.z);
	epnlen = 1.f / epnlen;
	ef->norm.x *= epnlen;
	ef->norm.y *= epnlen;
	ef->norm.z *= epnlen;
}
void MatrixReset(EERIEMATRIX * mat)
{
	memset(mat, 0, sizeof(EERIEMATRIX));
}

void MatrixSetByVectors(EERIEMATRIX * m, const EERIE_3D * d, const EERIE_3D * u)
{
	float t;
	EERIE_3D D, U, R;
	D.x = d->x;
	D.y = d->y;
	D.z = d->z;
	TRUEVector_Normalize(&D);
	U.x = u->x;
	U.y = u->y;
	U.z = u->z;
	t = U.x * D.x + U.y * D.y + U.z * D.z;
	U.x -= D.x * t;
	U.y -= D.y * t;
	U.z -= D.y * t;
	TRUEVector_Normalize(&U);
	Vcross(&R, &U, &D);
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

void GenerateMatrixUsingVector(EERIEMATRIX * matrix, const EERIE_3D * vect, const float rollDegrees)
{
	// Get our direction vector (the Z vector component of the matrix)
	// and make sure it's normalized into a unit vector
	EERIE_3D zAxis;
	memcpy(&zAxis, vect, sizeof(EERIE_3D));
	TRUEVector_Normalize(&zAxis);

	// Build the Y vector of the matrix (handle the degenerate case
	// in the way that 3DS does) -- This is not the TRUE vector, only
	// a reference vector.
	EERIE_3D yAxis;

	if (!zAxis.x && !zAxis.z)
		Vinit(&yAxis, -zAxis.y, 0.f, 0.f);
	else
		Vinit(&yAxis, 0.f, 1.f, 0.f);

	// Build the X axis vector based on the two existing vectors
	EERIE_3D xAxis;
	Vcross(&xAxis, &yAxis, &zAxis);
	TRUEVector_Normalize(&xAxis);

	// Correct the Y reference vector
	Vcross(&yAxis, &xAxis, &zAxis);
	TRUEVector_Normalize(&yAxis);
	yAxis.x = -yAxis.x;
	yAxis.y = -yAxis.y;
	yAxis.z = -yAxis.z;

	// Generate rotation matrix without roll included
	EERIEMATRIX rot, roll;
	MatrixReset(&rot);
	MatrixReset(&roll);
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
	roll._11 = EEcos(DEG2RAD(rollDegrees));
	roll._12 = -EEsin(DEG2RAD(rollDegrees));
	roll._21 = EEsin(DEG2RAD(rollDegrees));
	roll._22 = EEcos(DEG2RAD(rollDegrees));

	// Concatinate them for a complete rotation matrix that includes
	// all rotations
	MatrixMultiply(matrix, &rot, &roll);
}


//-----------------------------------------------------------------------------
// MatrixMultiply()
// Does the matrix operation: [Q] = [A] * [B]. Note that the order of
// this operation was changed from the previous version of the DXSDK.
//-----------------------------------------------------------------------------
VOID MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b)
{
	FLOAT * pA = (float *)a;
	FLOAT * pB = (float *)b;
	FLOAT  pM[16];

	ZeroMemory(pM, sizeof(EERIEMATRIX));

	for (WORD i = 0; i < 4; i++)
		for (WORD j = 0; j < 4; j++)
			for (WORD k = 0; k < 4; k++)
				pM[4*i+j] +=  pA[4*i+k] * pB[4*k+j];

	memcpy(q, pM, sizeof(EERIEMATRIX));
}

//-----------------------------------------------------------------------------
// Name: D3DMath_MatrixInvert()
// Desc: Does the matrix operation: [Q] = inv[A]. Note: this function only
//       works for matrices with [0 0 0 1] for the 4th column.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Name: D3DMath_VectorMatrixMultiply()
// Desc: Multiplies a vector by a matrix
//-----------------------------------------------------------------------------
void VectorMatrixMultiply(EERIE_3D * vDest, const EERIE_3D * vSrc,
                          const EERIEMATRIX * mat)
{
	FLOAT x = vSrc->x * mat->_11 + vSrc->y * mat->_21 + vSrc->z * mat->_31 + mat->_41;
	FLOAT y = vSrc->x * mat->_12 + vSrc->y * mat->_22 + vSrc->z * mat->_32 + mat->_42;
	FLOAT z = vSrc->x * mat->_13 + vSrc->y * mat->_23 + vSrc->z * mat->_33 + mat->_43;

	vDest->x = x;
	vDest->y = y;
	vDest->z = z;
}

//-----------------------------------------------------------------------------
float GetNearestSnappedAngle(float angle)
{
	angle = MAKEANGLE(angle);

	if (angle < 22.5f) return 0.f;

	if (angle < 67.5f) return 45.f;

	if (angle < 112.5f) return 90.f;

	if (angle < 157.5f) return 135.f;

	if (angle < 202.5f) return 180.f;

	if (angle < 247.5f) return 225.f;

	if (angle < 292.5f) return 270.f;

	if (angle < 337.5f) return 315.f;

	return 0.f;
}
