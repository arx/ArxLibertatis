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

#define D3D_OVERLOADS

#include <d3d.h>
#include <math.h>

#include "EERIETypes.h"
#include "EeriePoly.h"

//-----------------------------------------------------------------------------
// RANDOM Sequences Funcs/Defs
//-----------------------------------------------------------------------------
#define ornd()  (((FLOAT)rand() ) / RAND_MAX)
#define rnd()  (((FLOAT)rand() ) * 0.00003051850947599f)

//Approximative Methods
#define EEsqrt(val)	(float)ffsqrt(val)
#define EEcos(val)	(float)cos((float)val)
#define EEsin(val)	(float)sin((float)val)
#define EEfabs(val)	(float)fabs(val)
#define EEatan(val) (float)atan(val)

//True Methods
#define TRUEsqrt(val)	(float)sqrt(val)
#define TRUEcos(val)	(float)cos(val)
#define TRUEsin(val)	(float)sin(val)
#define TRUEfabs(val)	(float)fabs(val)
#define TRUEatan(val)	(float)atan(val)

//-----------------------------------------------------------------------------
// Math constants
//-----------------------------------------------------------------------------
#define EEdef_PI			3.14159265358979323846f		// Pi
#define EEdef_2_PI			6.28318530717958623200f		// 2 * Pi
#define EEdef_PI_DIV_2		1.57079632679489655800f		// Pi / 2
#define EEdef_PI_DIV_4		0.78539816339744827900f		// Pi / 4
#define EEdef_PI_0_75		4.7123889803846896397f		//EEdef_2_PI-EEdef_PI_DIV_2
#define EEdef_DEGTORAD		0.01745329251994329547f		// Degrees to Radians
#define EEdef_RADTODEG		57.29577951308232286465f	// Radians to Degrees
#define EEdef_HUGE			1.0e+38f					// Huge number for FLOAT
#define EEdef_EPSILON		1.0e-5f						// Tolerance for FLOATs
#define EEdef_MAXfloat		1.0e+38f
#define EEdef_MINfloat		-1.0e+38f

#define RAD2DEG(x) ((x)*(float)EEdef_RADTODEG)
#define DEG2RAD(x) ((x)*(float)EEdef_DEGTORAD)

//-----------------------------------------------------------------------------
// DIVISIONS Optimization List (Mul)
//-----------------------------------------------------------------------------
#define DEUXTIERS	0.6666666666666666666667f
#define DIV2		0.5f
#define DIV3		0.3333333333333333333333f
#define DIV4		0.25f
#define DIV5		0.2f
#define DIV6		0.1666666666666666666666f
#define DIV7		0.1428571428571428571428f
#define DIV8		0.125f
#define DIV9		0.1111111111111111111111f
#define DIV10		0.1f
#define DIV11		0.0909090909090909090909f
#define DIV12		0.0833333333333333333333f
#define DIV13		0.0769230769230769230769f
#define DIV14		0.0714285714285714285714f
#define DIV15		0.0666666666666666666667f
#define DIV16		0.0625f
#define DIV17		0.0588235294117647058823f
#define DIV18		0.0555555555555555555555f
#define DIV19		0.0526315789473684210526f
#define DIV20		0.05f
#define DIV21		0.0476190476190476190476f
#define DIV22		0.0454545454545454545454f
#define DIV23		0.0434782608695652173913f
#define DIV24		0.0416666666666666666667f
#define DIV25		0.04f
#define DIV26		0.0384615384615384615384f
#define DIV27		0.0370370370370370370370f
#define DIV28		0.0357142857142857142857f
#define DIV29		0.0344827586206896551724f
#define DIV30		0.0333333333333333333333f
#define DIV32		0.03125f
#define DIV40		0.025f
#define DIV50		0.02f
#define DIV60		0.0166666666666666666666f
#define DIV64		0.015625f
#define DIV70		0.0142857142857142857142f
#define DIV80		0.0125f
#define DIV90		0.0111111111111111111111f
#define DIV100		0.01f
#define DIV110		0.0090909090909090909090f
#define DIV120		0.0083333333333333333333f
#define DIV130		0.0076923076923076923076f
#define DIV128		0.0078125f
#define DIV140		0.0071428571428571428571f
#define DIV150		0.0066666666666666666666f
#define DIV160		0.00625f
#define DIV170		0.0058823529411764705882f
#define DIV180		0.0055555555555555555555f
#define DIV190		0.0052631578947368421052f
#define DIV200		0.005f
#define DIV255		0.00392156862745098f
#define DIV256		0.00390625f
#define DIV300		0.0033333333333333333333f
#define DIV384		0.0026041666666666666666f
#define DIV400		0.0025f
#define DIV480		0.0020833333333333333333f
#define DIV500		0.002f
#define DIV512		0.001953125f
#define DIV600		0.0016666666666666666666f
#define DIV640		0.0015625f
#define DIV700		0.0014285714285714285714f
#define DIV800		0.00125f
#define DIV900		0.0011111111111111111111f
#define DIV1000		0.001f
#define DIV1024		0.0009765625f
#define DIV2000		0.0005f
#define DIV2048		0.00048828125f
#define DIV3000		0.00033333333f
#define DIV4000		0.00025f
#define DIV4096		0.000244140625f
#define DIV5000		0.0002f
#define DIV10000    0.0001f
#define DIV12000    0.0000833333333333333333f
#define DIV13000    0.0000769230769230769231f
#define DIV15000	0.0000666666666666666666f
#define DIV16000	0.0000625f
#define DIV18000	0.0000555555555555555555f
#define DIV20000	0.00005f
#define DIV32768	0.000030517578125f
#define DIV100000   0.00001f
#define DIVPI		0.318309886183790671537767526745029f

#define ARXROTCONVERT	0.087890625f 

//-----------------------------------------------------------------------------

__inline bool In3DBBoxTolerance(const EERIE_3D * pos, const EERIE_3D_BBOX * bbox, const float tolerance)
{
	return ((pos->x >= bbox->min.x - tolerance)
	        &&	(pos->x <= bbox->max.x + tolerance)
	        &&	(pos->y >= bbox->min.y - tolerance)
	        &&	(pos->y <= bbox->max.y + tolerance)
	        &&	(pos->z >= bbox->min.z - tolerance)
	        &&	(pos->z <= bbox->max.z + tolerance));
}

extern EERIE_QUAT LocalUseQuat;
extern float Eatan[];
extern float Esin[];


//-----------------------------------------------------------------------------
__inline unsigned __int8 clipByte(int value)
{
	value = (0 & (-(int)(value < 0))) | (value & (-(int)!(value < 0)));
	value = (255 & (-(int)(value > 255))) | (value & (-(int)!(value > 255)));
	return  ARX_CLEAN_WARN_CAST_UCHAR(value);
}
__inline unsigned __int8 clipByte255(int value)
{
	value = (255 & (-(int)(value > 255))) | (value & (-(int)!(value > 255)));
	return ARX_CLEAN_WARN_CAST_UCHAR(value);
}

long F2L_RoundUp(float val);
void EERIEMathPrecalc();
void PrecalcATAN();
void PrecalcSIN();
BOOL PointInCylinder(const EERIE_CYLINDER * cyl, const EERIE_3D * pt);
BOOL CylinderInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_CYLINDER * cyl2);
BOOL SphereInCylinder(const EERIE_CYLINDER * cyl1, const EERIE_SPHERE * s);

// Optimized Float 2 Long Conversion
__forceinline void F2L(const float f, long * l)
{
	_asm
	{
		_asm	fld   f
		_asm	mov eax, DWORD PTR [l]
		_asm	fistp DWORD PTR [eax]
	}
}
__forceinline D3DCOLOR EERIERGB(float r, float g, float b)
{
	long t[3];
	F2L(r * 255.f, &t[0]);
	F2L(g * 255.f, &t[1]);
	F2L(b * 255.f, &t[2]);
	return (0xff000000L | (t[0] << 16) | (t[1] << 8) | t[2]);
}

__forceinline D3DCOLOR _EERIERGB(float v)
{
	long t;
	F2L(v * 255.f, &t);
	return (0xff000000L | (t << 16) | (t << 8) | t);
}
__forceinline D3DCOLOR _EERIERGBA(float v)
{
	long t;
	F2L(v * 255.f, &t);
	return (0x00000000L | (t << 24) | (t << 16) | (t << 8) | t);
}

#define EERIELRGB255(r,g,b) (0xff000000L | ( r << 16) | ( g << 8) | b);


#ifdef ASSEMBLER_OPTIMS	 //////////////////////
extern float __mov;
//__mov=x;
#define FLOAT2LONG( x, l) __asm					\
	{						\
		__asm fld x \
		__asm fistp l		\
	}
#else					 //////////////////////
#define FLOAT2LONG(floatx,longx) \
	longx = (long)floatx
#endif

float	SSQRT(long a);

float InterpolateAngle(float a1, float a2, float pour);

//*************************************************************************************
// Simple 2D Functions
//*************************************************************************************
float FORCEANGLE(float a);

__inline float ffsqrt(float f)
{
	unsigned int y = ((((unsigned int &)f) - 0x3f800000) >> 1) + 0x3f800000;
	// can repeat the following line 3 times for improved precision...
	return (float &)y;
}

#define FORCERANGE(a,b,c)	if (a<b) a=b; \
	if (a>c) a=c;


//*************************************************************************************
// Rotations
//*************************************************************************************
__inline void _ZRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s)
{
	out->x = (in->x * c) + (in->y * s);
	out->y = (in->y * c) - (in->x * s);
	out->z = in->z;
}
__inline void _YRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s)
{
	out->x = (in->x * c) + (in->z * s);
	out->y = in->y;
	out->z = (in->z * c) - (in->x * s);
}
__inline void _XRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s)
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
// Init a vector if v1, v2 & v3 aren't given, init vector with 0s
//*************************************************************************************
inline void Vector_Init(EERIE_3D * dest, const float x, const float y, const float z)
{
	dest->x = x;
	dest->y = y;
	dest->z = z;
}
//*************************************************************************************
// Computes Length of a vector
// WARNING: EEsqrt may use a approximative way of computing sqrt
//*************************************************************************************
inline float TRUEVector_Magnitude(const EERIE_3D * v)
{
	return (float)TRUEsqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

inline float Vector_Magnitude(const EERIE_3D * v)
{
	return (float)EEsqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}
//*************************************************************************************
// Normalizes a Vector. Returns its length before normalization
//*************************************************************************************
inline float Vector_Normalize(EERIE_3D * v)
{
	register float len = Vector_Magnitude(v);
	register float l2 = 1.f / len;
	v->x *= l2;
	v->y *= l2;
	v->z *= l2;
	return len;
}
inline float TRUEVector_Normalize(EERIE_3D * v)
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

void	MatrixSetByVectors(EERIEMATRIX * m, const EERIE_3D * d, const EERIE_3D * u);
void	MatrixReset(EERIEMATRIX * mat);
VOID    MatrixMultiply(EERIEMATRIX * q, const EERIEMATRIX * a, const EERIEMATRIX * b);
void	VectorMatrixMultiply(EERIE_3D * vDest, const EERIE_3D * vSrc, const EERIEMATRIX * mat);
#define VertexMatrixMultiply(a,b,c) VectorMatrixMultiply(a,b,c)
void	GenerateMatrixUsingVector(EERIEMATRIX * matrix, const EERIE_3D * vect, const float rollDegrees);

float	ffsqrt(float value);
long	isqrt(long value);


//*******************************************************************************
// Rotation Functions
//*******************************************************************************
__inline void _YXZRotatePoint(EERIE_3D * in, EERIE_3D * out, EERIE_CAMERA * cam)
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
__forceinline void _YXZRotateNorm(EERIE_3D * in, EERIE_3D * out, EERIE_CAMERA * cam)
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
__inline void TransformVertexMatrix(EERIEMATRIX * mat, EERIE_3D * vertexin, EERIE_3D * vertexout)
{

	vertexout->x = vertexin->x * mat->_11 + vertexin->y * mat->_21 + vertexin->z * mat->_31;
	vertexout->y = vertexin->x * mat->_12 + vertexin->y * mat->_22 + vertexin->z * mat->_32;
	vertexout->z = vertexin->x * mat->_13 + vertexin->y * mat->_23 + vertexin->z * mat->_33;

}

// Transforms a Vertex by a quaternion
__inline void TransformVertexQuat(EERIE_QUAT * quat, EERIE_3D * vertexin, EERIE_3D * vertexout)
{

	register float rx = vertexin->x * quat->w - vertexin->y * quat->z + vertexin->z * quat->y;
	register float ry = vertexin->y * quat->w - vertexin->z * quat->x + vertexin->x * quat->z;
	register float rz = vertexin->z * quat->w - vertexin->x * quat->y + vertexin->y * quat->x;
	register float rw = vertexin->x * quat->x + vertexin->y * quat->y + vertexin->z * quat->z;

	vertexout->x = quat->w * rx + quat->x * rw + quat->y * rz - quat->z * ry;
	vertexout->y = quat->w * ry + quat->y * rw + quat->z * rx - quat->x * rz;
	vertexout->z = quat->w * rz + quat->z * rw + quat->x * ry - quat->y * rx;
}

void TransformVertexQuat(const EERIE_QUAT * quat, const EERIE_3D * vertexin, EERIE_3D * vertexout);
void TransformInverseVertexQuat(const EERIE_QUAT * quat, const EERIE_3D * vertexin, EERIE_3D * vertexout);
void RotationFromQuat(EERIE_3D * v, FLOAT fTheta, const EERIE_QUAT * q);
void QuatFromRotation(EERIE_QUAT * q, const EERIE_3D * v, const FLOAT fTheta);
void Quat_Init(EERIE_QUAT * quat, const float x = 0, const float y = 0, const float z = 0, const float w = 1);
void Quat_Divide(EERIE_QUAT * dest, const EERIE_QUAT * q1, const EERIE_QUAT * q2);
void Quat_Multiply(EERIE_QUAT * dest , const EERIE_QUAT * q1, const EERIE_QUAT * q2);
float Quat_Magnitude(EERIE_QUAT * q);

void Quat_AxisRotate(EERIE_QUAT * quat, EERIE_3D * v, float ang);
void Quat_Rotate(EERIE_QUAT * quat, EERIE_3D * angle);
void Quat_Copy(EERIE_QUAT * q1, const EERIE_QUAT * q2);
void Quat_Slerp(EERIE_QUAT * result, const EERIE_QUAT * from, EERIE_QUAT * to, float t);
void Quat_Reverse(EERIE_QUAT * quat);
void Quat_GetShortestArc(EERIE_QUAT * q1 , EERIE_QUAT * q2);

//*******************************************************************************
// VECTORS Functions
//*******************************************************************************
void	Vector_Copy(EERIE_3D * dest, const EERIE_3D * src);
 
void	Vector_Init(EERIE_3D * dest, float v1 = 0.f, float v2 = 0.f, float v3 = 0.f);
void	Vector_Sub(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2);
void	Vector_Add(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2);
 
void	Vector_ScaleTo(EERIE_3D * dest, EERIE_3D * v, float scale);
bool	Vector_Compare(const EERIE_3D * v1, const EERIE_3D * v2);
float	Vector_Magnitude(const EERIE_3D * v);
float	Vector_Normalize(EERIE_3D * v);
float	TRUEVector_Magnitude(const EERIE_3D * v);
float	TRUEVector_Normalize(EERIE_3D * v);
void	Vector_CrossProduct(EERIE_3D * dest, const EERIE_3D * v1, const EERIE_3D * v2);
float	Vector_DotProduct(const EERIE_3D * v1, const EERIE_3D * v2);
 
void	Vector_RotateY(EERIE_3D * dest, const EERIE_3D * src, const float angle);
void	Vector_RotateZ(EERIE_3D * dest, const EERIE_3D * src, const float angle);
void	VRotateX(EERIE_3D * v1, const float angle);
void	VRotateY(EERIE_3D * v1, const float angle);
void	VRotateZ(EERIE_3D * v1, const float angle);
void	QuatFromMatrix(EERIE_QUAT & quat, EERIEMATRIX & mat);

#define Vinit Vector_Init
#define VrotY Vector_RotateY
#define Vsub Vector_Sub
#define Vadd Vector_Add
#define Vcmp Vector_Compare
#define Vcopy Vector_Copy
#define Vinv Vector_Invert
#define Vscale Vector_Scale
#define Vscaleto Vector_ScaleTo
#define Vcross Vector_CrossProduct
#define Vdot Vector_DotProduct
#define Vmag Vector_Magnitude
#define Vnorm Vector_Normalize

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

inline float ScalarProduct(EERIE_3D * v0, EERIE_3D * v1)
{
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
void CalcObjFaceNormal(const EERIE_3D * v0, const EERIE_3D * v1, const EERIE_3D * v2, EERIE_FACE * ef);
void Triangle_ComputeBoundingBox(EERIE_3D_BBOX * bb, EERIE_3D * v0, EERIE_3D * v1, EERIE_3D * v2);
BOOL Triangles_Intersect(const EERIE_TRI * v, const EERIE_TRI * u);
void MatrixFromQuat(EERIEMATRIX * mat, const EERIE_QUAT * q);

//*******************************************************************************
// DISTANCES Functions
//*******************************************************************************
#define SquaredDistance2D(x0,y0,x1,y1) (float)( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define SquaredDistance3D(x0,y0,z0,x1,y1,z1) (float)( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
__inline float EESquaredDistance3D(const EERIE_3D * from, const EERIE_3D * to)
{
	return (float)(((to->x - from->x) * (to->x - from->x)) + ((to->y - from->y) * (to->y - from->y)) + ((to->z - from->z) * (to->z - from->z)));
}

#define Distance2D(x0,y0,x1,y1) (float)EEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define Distance3D(x0,y0,z0,x1,y1,z1) (float)EEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
#define TRUEDistance2D(x0,y0,x1,y1) (float)TRUEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) )
#define TRUEDistance3D(x0,y0,z0,x1,y1,z1) (float)TRUEsqrt( ((x1-x0)*(x1-x0)) +((y1-y0)*(y1-y0)) +((z1-z0)*(z1-z0)) )
__inline float TRUEEEDistance3D(const EERIE_3D * from, const EERIE_3D * to)
{
	return (float)TRUEsqrt(((to->x - from->x) * (to->x - from->x)) + ((to->y - from->y) * (to->y - from->y)) + ((to->z - from->z) * (to->z - from->z)));
}

//*************************************************************************************
// Compute Distance between two 3D points
// WARNING: EEsqrt may use an approximative way of computing sqrt !
//*************************************************************************************
__inline float EEDistance3D(const EERIE_3D * from, const EERIE_3D * to)
{
	return (float)EEsqrt(((to->x - from->x) * (to->x - from->x)) + ((to->y - from->y) * (to->y - from->y)) + ((to->z - from->z) * (to->z - from->z)));
}

__inline BOOL PointInCylinder(const EERIE_CYLINDER * cyl, const EERIE_3D * pt)
{
	register float pos1 = cyl->origin.y + cyl->height;

	if (pt->y < __min(cyl->origin.y, pos1)) return FALSE;

	if (pt->y > __max(cyl->origin.y, pos1)) return FALSE;

	if (Distance2D(cyl->origin.x, cyl->origin.z, pt->x, pt->z) <= cyl->radius)
		return TRUE;

	return FALSE;
}

__inline long PointInUnderCylinder(const EERIE_CYLINDER * cyl, const EERIE_3D * pt)
{
	register float pos1 = cyl->origin.y + cyl->height;
	long ret = 2;

	if (pt->y < __min(cyl->origin.y, pos1)) return 0;

	if (pt->y > __max(cyl->origin.y, pos1)) ret = 1;

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

__inline float MAKEANGLE(float a)
{
	if (a >= 0)
		return a - 360 * (int)(a * 0.0027777777f);
	else
		return a + 360 * (1 + (int)(-a * 0.0027777777f));
}
 

float GetNearestSnappedAngle(float angle);
void QuatFromAngles(EERIE_QUAT * q, const EERIE_3D * angle);

extern D3DMATRIX ProjectionMatrix;

__forceinline void specialEE_RT(D3DTLVERTEX * in, EERIE_3D * out)
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

__forceinline void specialEE_P(EERIE_3D * in, D3DTLVERTEX * out)
{
	register EERIE_TRANSFORM * et = (EERIE_TRANSFORM *)&ACTIVECAM->transform;

	float fZTemp = 1.f / in->z;
	out->sz = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43;
	out->sx = in->x * ProjectionMatrix._11 * fZTemp + et->xmod;
	out->sy = in->y * ProjectionMatrix._22 * fZTemp + et->ymod;
	out->rhw = fZTemp; 
}

#endif

