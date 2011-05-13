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
///////////////////////////////////////////////////////////////////////////////
// EERIEMath
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#ifndef EERIEMATH_H
#define EERIEMATH_H

#include <algorithm>

using std::min;
using std::max;

#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"

//-----------------------------------------------------------------------------
// RANDOM Sequences Funcs/Defs
//-----------------------------------------------------------------------------
#define rnd()  (((float)rand() ) * (1.0f / RAND_MAX))

//Approximative Methods
#define EEsqrt(val) (float)ffsqrt(val)
#define EEcos(val)  (float)cos((float)val)
#define EEsin(val)  (float)sin((float)val)
#define EEfabs(val) (float)fabs(val)
#define EEatan(val) (float)atan(val)

//True Methods
// TODO just use sqrt directly
#define TRUEsqrt(val) (float)sqrt(val)

//-----------------------------------------------------------------------------
// Math constants
//-----------------------------------------------------------------------------
#define PI 3.14159265358979323846f

// TODO consider replacing these with defines from #include <cfloat>
#define EEdef_EPSILON   1.0e-5f  // Tolerance for FLOATs
#define EEdef_MAXfloat  1.0e+38f
#define EEdef_MINfloat  -1.0e+38f

//-----------------------------------------------------------------------------

inline bool In3DBBoxTolerance(const Vec3f * pos, const EERIE_3D_BBOX * bbox, const float tolerance)
{
	return ((pos->x >= bbox->min.x - tolerance)
	        &&	(pos->x <= bbox->max.x + tolerance)
	        &&	(pos->y >= bbox->min.y - tolerance)
	        &&	(pos->y <= bbox->max.y + tolerance)
	        &&	(pos->z >= bbox->min.z - tolerance)
	        &&	(pos->z <= bbox->max.z + tolerance));
}


//-----------------------------------------------------------------------------
inline unsigned __int8 clipByte(int value)
{
	value = (0 & (-(int)(value < 0))) | (value & (-(int)!(value < 0)));
	value = (255 & (-(int)(value > 255))) | (value & (-(int)!(value > 255)));
	return  static_cast<unsigned char>(value);
}
inline unsigned __int8 clipByte255(int value)
{
	value = (255 & (-(int)(value > 255))) | (value & (-(int)!(value > 255)));
	return static_cast<unsigned char>(value);
}

long F2L_RoundUp(float val);
void EERIEMathPrecalc();
void PrecalcATAN();
void PrecalcSIN();
bool PointInCylinder(const EERIE_CYLINDER * cyl, const Vec3f * pt);
bool CylinderInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_CYLINDER * cyl2);
bool SphereInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_SPHERE * s);

inline D3DCOLOR EERIERGB(float r, float g, float b)
{
	long t[3];
	t[0] = r * 255.f;
	t[1] = g * 255.f;
	t[2] = b * 255.f;
	return (0xff000000L | (t[0] << 16) | (t[1] << 8) | t[2]);
}

inline D3DCOLOR _EERIERGB(float v)
{
	long t = v * 255.f;
	return (0xff000000L | (t << 16) | (t << 8) | t);
}
inline D3DCOLOR _EERIERGBA(float v)
{
	long t = v * 255.f;
	return (0x00000000L | (t << 24) | (t << 16) | (t << 8) | t);
}

#define EERIELRGB255(r,g,b) (0xff000000L | ( r << 16) | ( g << 8) | b);

#define FLOAT2LONG(floatx,longx) \
	longx = (long)floatx


float	SSQRT(long a);

float InterpolateAngle(float a1, float a2, float pour);

//*************************************************************************************
// Simple 2D Functions
//*************************************************************************************
float FORCEANGLE(float a);

inline float ffsqrt(float f)
{
	unsigned int y = ((((unsigned int &)f) - 0x3f800000) >> 1) + 0x3f800000;
	// can repeat the following line 3 times for improved precision...
	return (float &)y;
}

/**
 *  Obtain the approximated inverse of the square root of a float.
 *  @brief  This code compute a fast 1 / sqrtf(v) approximation.
 *  @note   Originaly from Matthew Jones (Infogrames).
 *  @param  pValue  a float, the number we want the square root.
 *  @return The square root of \a fValue, as a float.
 */
inline float FastRSqrt( float pValue )    
{  
	// Avoid issues with strict aliasing - use a union!
    union FloatInt
    {
        float f;
        int   i;
    };
    FloatInt floatInt;
    floatInt.f = pValue;
    
    const int MAGIC_NUMBER = 0x5f3759df;
        
    float v_half = pValue * 0.5f;
    floatInt.i = MAGIC_NUMBER - (floatInt.i >> 1);
    pValue = floatInt.f;
    return pValue * (1.5f - v_half * pValue * pValue);
}

#define FORCERANGE(a,b,c)	if (a<b) a=b; \
	if (a>c) a=c;


//*************************************************************************************
// Rotations
//*************************************************************************************
inline void _ZRotatePoint(Vec3f * in, Vec3f * out, float c, float s)
{
	out->x = (in->x * c) + (in->y * s);
	out->y = (in->y * c) - (in->x * s);
	out->z = in->z;
}
inline void _YRotatePoint(Vec3f * in, Vec3f * out, float c, float s)
{
	out->x = (in->x * c) + (in->z * s);
	out->y = in->y;
	out->z = (in->z * c) - (in->x * s);
}
inline void _XRotatePoint(Vec3f * in, Vec3f * out, float c, float s)
{
	out->x = in->x;
	out->y = (in->y * c) - (in->z * s);
	out->z = (in->y * s) + (in->z * c);
}

//*************************************************************************************
// Fuzzy compares (within tolerance)
//*************************************************************************************
#define EPSILON 0.000001f
 
//*************************************************************************************
// Computes Length of a vector
// WARNING: EEsqrt may use a approximative way of computing sqrt
//*************************************************************************************
inline float TRUEVector_Magnitude(const Vec3f * v)
{
	return (float)TRUEsqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

inline float Vector_Magnitude(const Vec3f * v)
{
	return (float)EEsqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}
//*************************************************************************************
// Normalizes a Vector. Returns its length before normalization
//*************************************************************************************
inline float Vector_Normalize(Vec3f * v)
{
	register float len = Vector_Magnitude(v);
	register float l2 = 1.f / len;
	v->x *= l2;
	v->y *= l2;
	v->z *= l2;
	return len;
}
inline float TRUEVector_Normalize(Vec3f * v)
{
	register float len = TRUEVector_Magnitude(v);
	register float l2 = 1.f / len;
	v->x *= l2;
	v->y *= l2;
	v->z *= l2;
	return len;
}

//*******************************************************************************
// Matrix functions
//*******************************************************************************

void	MatrixSetByVectors(EERIEMATRIX * m, const Vec3f * d, const Vec3f * u);
void	MatrixReset(EERIEMATRIX * mat);
VOID    MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b);
void	VectorMatrixMultiply(Vec3f * vDest, const Vec3f * vSrc, const EERIEMATRIX * mat);
#define VertexMatrixMultiply(a,b,c) VectorMatrixMultiply(a,b,c)
void	GenerateMatrixUsingVector(EERIEMATRIX * matrix, const Vec3f * vect, const float rollDegrees);

float	ffsqrt(float value);
long	isqrt(long value);


//*******************************************************************************
// Rotation Functions
//*******************************************************************************
inline void _YXZRotatePoint(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam)
{
	register float tempy;
	out->z = (in->z * cam->Ycos) - (in->x * cam->Ysin);
	out->y = (in->x * cam->Ycos) + (in->z * cam->Ysin);
	tempy = (in->y * cam->Xcos) - (out->z * cam->Xsin);
	out->x = (out->y * cam->Zcos) + (tempy * cam->Zsin);
	out->y = (tempy * cam->Zcos) - (out->y * cam->Zsin);
	out->z = (in->y * cam->Xsin) + (out->z * cam->Xcos);
}
//*************************************************************************************
// Fast normal rotation :p
//*************************************************************************************
inline void _YXZRotateNorm(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam)
{
	out->z = (in->y * cam->Xsin) + (((in->z * cam->Ycos) - (in->x * cam->Ysin)) * cam->Xcos);
}

// QUATERNION Funcs/Defs
//*************************************************************************************
// Copy a quaternion into another
//*************************************************************************************
inline void Quat_Copy(EERIE_QUAT * dest, const EERIE_QUAT * src)
{
	dest->x = src->x;
	dest->y = src->y;
	dest->z = src->z;
	dest->w = src->w;
}

//*************************************************************************************
// Quaternion Initialization
//		quat -> quaternion to init
//*************************************************************************************
inline void Quat_Init(EERIE_QUAT * quat, const float x, const float y, const float z, const float w)
{
	quat->x = x;
	quat->y = y;
	quat->z = z;
	quat->w = w;
}

// Transforms a Vertex by a matrix
inline void TransformVertexMatrix(EERIEMATRIX * mat, Vec3f * vertexin, Vec3f * vertexout)
{

	vertexout->x = vertexin->x * mat->_11 + vertexin->y * mat->_21 + vertexin->z * mat->_31;
	vertexout->y = vertexin->x * mat->_12 + vertexin->y * mat->_22 + vertexin->z * mat->_32;
	vertexout->z = vertexin->x * mat->_13 + vertexin->y * mat->_23 + vertexin->z * mat->_33;

}

// Transforms a Vertex by a quaternion
inline void TransformVertexQuat(EERIE_QUAT * quat, Vec3f * vertexin, Vec3f * vertexout)
{

	register float rx = vertexin->x * quat->w - vertexin->y * quat->z + vertexin->z * quat->y;
	register float ry = vertexin->y * quat->w - vertexin->z * quat->x + vertexin->x * quat->z;
	register float rz = vertexin->z * quat->w - vertexin->x * quat->y + vertexin->y * quat->x;
	register float rw = vertexin->x * quat->x + vertexin->y * quat->y + vertexin->z * quat->z;

	vertexout->x = quat->w * rx + quat->x * rw + quat->y * rz - quat->z * ry;
	vertexout->y = quat->w * ry + quat->y * rw + quat->z * rx - quat->x * rz;
	vertexout->z = quat->w * rz + quat->z * rw + quat->x * ry - quat->y * rx;
}

void TransformVertexQuat(const EERIE_QUAT * quat, const Vec3f * vertexin, Vec3f * vertexout);
void TransformInverseVertexQuat(const EERIE_QUAT * quat, const Vec3f * vertexin, Vec3f * vertexout);
void RotationFromQuat(Vec3f * v, float fTheta, const EERIE_QUAT * q);
void QuatFromRotation(EERIE_QUAT * q, const Vec3f * v, const float fTheta);
void Quat_Init(EERIE_QUAT * quat, const float x = 0, const float y = 0, const float z = 0, const float w = 1);
void Quat_Divide(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2);
void Quat_Multiply(EERIE_QUAT * dest , const EERIE_QUAT * q1, const EERIE_QUAT * q2);
float Quat_Magnitude(EERIE_QUAT * q);

void Quat_AxisRotate(EERIE_QUAT * quat, Vec3f * v, float ang);
void Quat_Rotate(EERIE_QUAT * quat, Vec3f * angle);
void Quat_Copy(EERIE_QUAT * q1, const EERIE_QUAT * q2);
void Quat_Slerp(EERIE_QUAT * result, const EERIE_QUAT * from, EERIE_QUAT * to, float t);
void Quat_Reverse(EERIE_QUAT * quat);
void Quat_GetShortestArc(EERIE_QUAT * q1 , EERIE_QUAT * q2);

//*******************************************************************************
// VECTORS Functions
//*******************************************************************************
float	Vector_Magnitude(const Vec3f * v);
float	Vector_Normalize(Vec3f * v);
float	TRUEVector_Magnitude(const Vec3f * v);
float	TRUEVector_Normalize(Vec3f * v);
 
void	Vector_RotateY(Vec3f * dest, const Vec3f * src, const float angle);
void	Vector_RotateZ(Vec3f * dest, const Vec3f * src, const float angle);
void	VRotateX(Vec3f * v1, const float angle);
void	VRotateY(Vec3f * v1, const float angle);
void	VRotateZ(Vec3f * v1, const float angle);
void	QuatFromMatrix(EERIE_QUAT & quat, EERIEMATRIX & mat);

#define CROSS(dest,v1,v2) \
	dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
	dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
	dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOTPRODUCT(v1,v2) (v1.x*v2.x+v1.y*v2.y+v1.z*v2.z)
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
	dest[0]=v1[0]-v2[0]; \
	dest[1]=v1[1]-v2[1]; \
	dest[2]=v1[2]-v2[2];

inline float ScalarProduct(const Vec3f * v0, const Vec3f * v1) {
	return	((v0->x * v1->x) +	(v0->y * v1->y) +	(v0->z * v1->z));
}

/* sort so that a<=b */
#define SORT(a,b)       \
	if(a>b)    \
	{          \
		float c; \
		c=a;     \
		a=b;     \
		b=c;     \
	}

#define ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1) \
	isect0=VV0+(VV1-VV0)*D0/(D0-D1);    \
	isect1=VV0+(VV2-VV0)*D0/(D0-D2);
 
void CalcFaceNormal(EERIEPOLY * ep, const D3DTLVERTEX * v);
void CalcObjFaceNormal(const Vec3f * v0, const Vec3f * v1, const Vec3f * v2, EERIE_FACE * ef);
void Triangle_ComputeBoundingBox(EERIE_3D_BBOX * bb, Vec3f * v0, Vec3f * v1, Vec3f * v2);
bool Triangles_Intersect(const EERIE_TRI * v, const EERIE_TRI * u);
void MatrixFromQuat(EERIEMATRIX * mat, const EERIE_QUAT * q);

//*******************************************************************************
// DISTANCES Functions
//*******************************************************************************
#define SquaredDistance2D(x0,y0,x1,y1) (float)( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define SquaredDistance3D(x0,y0,z0,x1,y1,z1) (float)( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
inline float EESquaredDistance3D(const Vec3f * from, const Vec3f * to)
{
	return (float)(((to->x - from->x) * (to->x - from->x)) + ((to->y - from->y) * (to->y - from->y)) + ((to->z - from->z) * (to->z - from->z)));
}

#define Distance2D(x0,y0,x1,y1) (float)EEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define Distance3D(x0,y0,z0,x1,y1,z1) (float)EEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
#define TRUEDistance2D(x0,y0,x1,y1) (float)TRUEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define TRUEDistance3D(x0,y0,z0,x1,y1,z1) (float)TRUEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
inline float TRUEEEDistance3D(const Vec3f * from, const Vec3f * to)
{
	return (float)TRUEsqrt(((to->x - from->x) * (to->x - from->x)) + ((to->y - from->y) * (to->y - from->y)) + ((to->z - from->z) * (to->z - from->z)));
}

//*************************************************************************************
// Compute Distance between two 3D points
// WARNING: EEsqrt may use an approximative way of computing sqrt !
//*************************************************************************************
inline float EEDistance3D(const Vec3f & from, const Vec3f & to) {
	return (float)EEsqrt(((to.x - from.x) * (to.x - from.x)) + ((to.y - from.y) * (to.y - from.y)) + ((to.z - from.z) * (to.z - from.z)));
}

// TODO remove once callers are migrated to the reference version
inline float EEDistance3D(const Vec3f * from, const Vec3f * to) {
	return EEDistance3D(*from, *to);
}

inline bool PointInCylinder(const EERIE_CYLINDER * cyl, const Vec3f * pt)
{
    using std::min;
    using std::max;
	float pos1 = cyl->origin.y + cyl->height;

	if (pt->y < min(cyl->origin.y, pos1)) return false;

	if (pt->y > max(cyl->origin.y, pos1)) return false;

	if (Distance2D(cyl->origin.x, cyl->origin.z, pt->x, pt->z) <= cyl->radius)
		return true;

	return false;
}

inline long PointInUnderCylinder(const EERIE_CYLINDER * cyl, const Vec3f * pt)
{
    using std::min;
    using std::max;
	float pos1 = cyl->origin.y + cyl->height;
	long ret = 2;

	if (pt->y < min(cyl->origin.y, pos1)) return 0;

	if (pt->y > max(cyl->origin.y, pos1)) ret = 1;

	if (Distance2D(cyl->origin.x, cyl->origin.z, pt->x, pt->z) <= cyl->radius)
	{
		return ret;
	}

	return 0;
}

//*******************************************************************************
// ANGLES Functions
//*******************************************************************************
float	GetAngle(const float x0, const float y0, const float x1, const float y1);
float	AngleDifference(float d, float e);

inline float MAKEANGLE(float a)
{
	if (a >= 0)
		return a - 360 * (int)(a * 0.0027777777f);
	else
		return a + 360 * (1 + (int)(-a * 0.0027777777f));
}
 

float GetNearestSnappedAngle(float angle);
void QuatFromAngles(EERIE_QUAT * q, const Anglef * angle);

extern D3DMATRIX ProjectionMatrix;

inline void specialEE_RT(D3DTLVERTEX * in, Vec3f * out)
{
	register EERIE_TRANSFORM * et = (EERIE_TRANSFORM *)&ACTIVECAM->transform;
	out->x = in->sx - et->posx;
	out->y = in->sy - et->posy;
	out->z = in->sz - et->posz;

	register float temp = (out->z * et->ycos) - (out->x * et->ysin);
	out->x = (out->z * et->ysin) + (out->x * et->ycos);
	out->z = (out->y * et->xsin) + (temp * et->xcos);
	out->y = (out->y * et->xcos) - (temp * et->xsin);
}

inline void specialEE_P(Vec3f * in, D3DTLVERTEX * out)
{
	register EERIE_TRANSFORM * et = (EERIE_TRANSFORM *)&ACTIVECAM->transform;

	float fZTemp = 1.f / in->z;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43;
	out->sx = in->x * ProjectionMatrix._11 * fZTemp + et->xmod;
	out->sy = in->y * ProjectionMatrix._22 * fZTemp + et->ymod;
	out->rhw = fZTemp; 
}

inline float radians(float degrees){
	return degrees*2*PI/360;
}

inline float degrees(float radians){
	return radians*360/(2*PI);
}

/**
 *  Compute a random T value between fMin and fMax.
 *  @brief  Random for T, given a range.
 *  @param  fMin    a T, minimum value wanted.
 *  @param  fMax    a T, maximum value wanted.
 *  @return A random real value between fMin and fMax.
 */
template <class T>
inline T Rand( const T& pMin, const T& pMax )
{
    return pMin + (pMax - pMin) * rand() / (T)RAND_MAX;
}

#endif
