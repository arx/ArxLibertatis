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

#ifndef ARX_GRAPHICS_DATA_MESH_H
#define ARX_GRAPHICS_DATA_MESH_H

#include <set>

#include "graphics/GraphicsTypes.h"
#include "math/Rectangle.h"

class Entity;

void specialEE_RTP(TexturedVertex*,TexturedVertex*);
void EERIE_CreateMatriceProj(float _fWidth,float _fHeight,float _fFOV,float _fZNear,float _fZFar);


struct ANIM_HANDLE {
	
	ANIM_HANDLE();
	
	res::path path; // empty path means an unallocated slot
	
	EERIE_ANIM ** anims;
	short alt_nb;
	
	long locks;
	
};

struct EERIE_TRANSFORM {
	Vec3f pos;
	float ycos;
	float ysin;
	float xsin;
	float xcos;
	float use_focal;
	float xmod;
	float ymod;
	float zmod;
};

struct EERIE_CAMERA {
	
	EERIE_TRANSFORM transform;
	Vec3f pos; // 0 4 8
	float	Ycos; // 12
	float	Ysin; // 16
	float	Xcos; // 20
	float	Xsin; // 24
	float	Zcos; // 28
	float	Zsin; // 32
	float	focal;// 36
	float	use_focal;
	float	Zmul; // 40
	float posleft;// 44
	float postop; // 48 do not move/insert before this point !!!
	
	float xmod;
	float ymod;
	EERIEMATRIX matrix;
	Anglef angle;
	
	Vec3f d_pos;
	Anglef d_angle;
	Vec3f lasttarget;
	Vec3f lastpos;
	Vec3f translatetarget;
	bool lastinfovalid;
	Vec3f norm;
	Color3f fadecolor;
	Rect clip;
	float clipz0;
	float clipz1;
	Vec2i center;
	
	float smoothing;
	float AddX;
	float AddY;
	long Xsnap;
	long Zsnap;
	float Zdiv;
	
	long clip3D;
	long type;
	Color bkgcolor; // TODO was BGR!
	long nbdrawn;
	float cdepth;
	
	Anglef size;
	
};

struct EERIE_BKG_INFO
{
	char				treat;
	char				nothing;
	short				nbpoly;
	short				nbianchors;
	short				nbpolyin;
	float				frustrum_miny;
	float				frustrum_maxy;
	EERIEPOLY *			polydata;
	EERIEPOLY **		polyin;
	long *				ianchors; // index on anchors list
	long				flags;
	float				tile_miny;
	float				tile_maxy;
};

struct EERIE_SMINMAX
{
	short min;
	short max;
};

#define FBD_TREAT		1
#define FBD_NOTHING		2

struct FAST_BKG_DATA
{
	char				treat;
	char				nothing;
	short				nbpoly;
	short				nbianchors;
	short				nbpolyin;
	long				flags;
	float				frustrum_miny;
	float				frustrum_maxy;
	EERIEPOLY *			polydata;
	EERIEPOLY **		polyin;
	long *				ianchors; // index on anchors list
};
#define MAX_BKGX	160
#define MAX_BKGZ	160
#define BKG_SIZX	100
#define BKG_SIZZ	100

struct ANCHOR_DATA;

struct EERIE_BACKGROUND
{
	FAST_BKG_DATA	fastdata[MAX_BKGX][MAX_BKGZ];
	long		exist;
	short		Xsize;
	short		Zsize;
	short		Xdiv;
	short		Zdiv;
	float		Xmul;
	float		Zmul;
	EERIE_BKG_INFO * Backg;
	Color3f ambient;
	Color3f ambient255;
	EERIE_SMINMAX *	minmax;
	long		  nbanchors;
	ANCHOR_DATA * anchors;
	char		name[256];
};

struct ANIM_USE;

#define MAX_TRANSPOL 512

#define CAM_SUBJVIEW 0
#define CAM_TOPVIEW 1

extern long EERIEDrawnPolys;
extern Vec3f BBOXMIN,BBOXMAX;
extern EERIE_BACKGROUND * ACTIVEBKG;
extern EERIE_CAMERA * ACTIVECAM;

extern float Xratio;
extern float Yratio;

float FirstPolyPosY(float x,float z);
void SetActiveCamera(EERIE_CAMERA* cam);
//	Entity Struct End

void AcquireLastAnim(Entity * io);
void FinishAnim(Entity * io,ANIM_HANDLE * eanim);
bool Visible(Vec3f * orgn, Vec3f * dest,EERIEPOLY * epp,Vec3f * hit);
void FaceTarget(Entity * io);

void DebugSphere(float x, float y, float z, float siz, long tim, Color color);

EERIEPOLY * CheckTopPoly(float x,float y,float z);
EERIEPOLY * CheckPolyOnTop(float x,float y,float z);
EERIEPOLY * CheckInPoly(float x,float y,float z,float * needY = NULL);
EERIEPOLY * EECheckInPoly(const Vec3f * pos,float * needY = NULL);
EERIEPOLY * CheckInPolyIn(float x,float y,float z);
EERIEPOLY * CheckInPolyPrecis(float x,float y,float z,float * needY = NULL);

/*!
 * Check if the given condition is under water.
 * 
 * @return the lowest water polygon pos is under, or NULL if pos is not under water.
 */
EERIEPOLY * EEIsUnderWater(const Vec3f * pos);

/*!
 * Check if the given condition is under water.
 * 
 * @return any water polygon pos is under, or NULL if pos is not under water.
 */
EERIEPOLY * EEIsUnderWaterFast(const Vec3f * pos);

bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f * pos,float * ret);
bool IsAnyPolyThere(float x, float z);
bool IsVertexIdxInGroup(EERIE_3DOBJ * eobj,long idx,long grs);
EERIEPOLY * GetMinPoly(float x, float y, float z);
EERIEPOLY * GetMaxPoly(float x, float y, float z);
 
float GetColorz(float x, float y, float z);
int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z);

int EERIELaunchRay2(Vec3f * orgn, Vec3f * dest,  Vec3f * hit, EERIEPOLY * tp, long flag);
int EERIELaunchRay3(Vec3f * orgn, Vec3f * dest,  Vec3f * hit, EERIEPOLY * tp, long flag);
float GetGroundY(Vec3f * pos);
void EE_IRTP(TexturedVertex *in,TexturedVertex *out);
void EE_RTT(TexturedVertex *in,TexturedVertex *out);

void extEE_RTP(TexturedVertex *in,TexturedVertex *out);
void MakeColorz(Entity * io);

void EE_RotateX(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RotateY(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RotateZ(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RTP(TexturedVertex *in,TexturedVertex *out);

void GetAnimTotalTranslate( ANIM_HANDLE * eanim,long alt_idx,Vec3f * pos);

long PhysicalDrawBkgVLine(Vec3f * orgn,Vec3f * dest);

// FAST SAVE LOAD
bool FastSceneLoad(const res::path & path);

//****************************************************************************
// DRAWING FUNCTIONS START

void DrawEERIEObjEx(EERIE_3DOBJ * eobj, Anglef * angle, Vec3f * pos, Vec3f * scale, Color3f * col);
void DrawEERIEObjExEx(EERIE_3DOBJ * eobj, Anglef * angle, Vec3f * pos, Vec3f * scale, int coll);
// DRAWING FUNCTIONS END
//****************************************************************************


//****************************************************************************
// BACKGROUND MANAGEMENT FUNCTIONS START
long BKG_CountPolys(EERIE_BACKGROUND * eb);
long BKG_CountChildPolys(EERIE_BACKGROUND * eb);
long BKG_CountIgnoredPolys(EERIE_BACKGROUND * eb);

#ifdef BUILD_EDIT_LOADSAVE
void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms);
#endif

void ClearBackground(EERIE_BACKGROUND * eb);
int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv);

void EERIEAddPoly(TexturedVertex * vert, TexturedVertex * vert2, TextureContainer * tex, long render, float transval);
// BACKGROUND MANAGEMENT FUNCTIONS END
//****************************************************************************


//****************************************************************************
// LIGHT FUNCTIONS START
void EERIEPrecalcLights(long minx=0,long minz=0,long maxx=99999,long maxz=99999);
void EERIERemovePrecalcLights();
void PrecalcDynamicLighting(long x0,long x1,long z0,long z1);
void ApplyDynLight(EERIEPOLY *ep);
long GetFreeDynLight();
// LIGHT FUNCTIONS END
//****************************************************************************


//****************************************************************************
// CAMERA FUNCTIONS START
void SetTargetCamera(EERIE_CAMERA * cam,float x,float y, float z);
void PrepareCamera(EERIE_CAMERA *cam);
void PrepareActiveCamera();
// CAMERA FUNCTIONS END
//****************************************************************************

//****************************************************************************
// BBOX FUNCTIONS START
void ResetBBox3D(Entity * io);
void AddToBBox3D(Entity * io,Vec3f * pos);
// BBOX FUNCTIONS END
//****************************************************************************

void ApplyLight(EERIEPOLY *ep);
long MakeTopObjString(Entity * io, std::string& dest);
void DeclareEGInfo(float x, float z);
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj);
void ApplyWaterFXToVertex(Vec3f * odtv,TexturedVertex * dtv,float power);
int BackFaceCull2D(TexturedVertex * tv);
void ResetAnim(ANIM_USE * eanim);

//*************************************************************************************
//*************************************************************************************

long EERIERTPPoly(EERIEPOLY *ep);

void EE_RTP3(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam);

void ReleaseAnimFromIO(Entity * io,long num);

void ShadowPolys_ClearZone(EERIE_BACKGROUND * eb,long x0, long y0, long x1, long y1);
short ANIM_GetAltIdx(ANIM_HANDLE * ah,long old);
void ANIM_Set(ANIM_USE * au,ANIM_HANDLE * anim);

bool LittleAngularDiff(Vec3f * norm,Vec3f * norm2);
void RecalcLight(EERIE_LIGHT * el);
void CreatePWorld(long x0,long x1,long z0,long z1);
void ComputeSworld();
float PtIn2DPolyProj(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
float PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);

void ResetWorlds();
float GetSWorld(float x,float y,float z);

void EERIE_ANIMMANAGER_PurgeUnused();
void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load(const res::path & path);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load_NoWarning(const res::path & path);
void BkgAddShadowPoly(EERIEPOLY * ep,EERIEPOLY * father);

EERIEPOLY * GetMinNextPoly(long i,long j,EERIEPOLY * ep);

long GetVertexPos(Entity * io,long id,Vec3f * pos);
void ARX_PrepareBackgroundNRMLs();
void DrawInWorld();
long CountBkgVertex();
void CreateInWorld();
void EERIE_LIGHT_ChangeLighting();
void SetCameraDepth(float depth);

extern void EERIETreatPoint(TexturedVertex *in,TexturedVertex *out);
extern void EERIETreatPoint2(TexturedVertex *in,TexturedVertex *out);
bool RayCollidingPoly(Vec3f * orgn,Vec3f * dest,EERIEPOLY * ep,Vec3f * hit);

void EERIEPOLY_Compute_PolyIn();
void F_PrepareCamera(EERIE_CAMERA * cam);

float GetTileMinY(long i,long j);
float GetTileMaxY(long i,long j);

#define MAX_FRUSTRUMS 32

struct EERIE_FRUSTRUM_PLANE
{
	float	a;
	float	b;
	float	c;
	float	d; // dist to origin
};

struct EERIE_FRUSTRUM
{
	EERIE_FRUSTRUM_PLANE plane[4];
	long nb;
};

struct EERIE_FRUSTRUM_DATA
{
	long nb_frustrums;
	EERIE_FRUSTRUM frustrums[MAX_FRUSTRUMS];
};

struct PORTAL_ROOM_DRAW
{
	short			count;
	short			flags;
	EERIE_2D_BBOX	bbox;
	EERIE_FRUSTRUM_DATA	frustrum;
};

// Default Mode for Portals when found
#define NPC_ITEMS_AMBIENT_VALUE_255 35

struct ROOM_DIST_DATA
{
	float	distance; // -1 means use truedist
	Vec3f startpos;
	Vec3f endpos;
};

extern ROOM_DIST_DATA * RoomDistance;

void UpdateIORoom(Entity * io);
float SP_GetRoomDist(Vec3f * pos,Vec3f * c_pos,long io_room,long Cam_Room);
float CEDRIC_PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
void EERIE_PORTAL_ReleaseOnlyVertexBuffer();
void ComputePortalVertexBuffer();
bool GetNameInfo( const std::string& name1,long& type,long& val1,long& val2);

struct TILE_LIGHTS
{
	short			num;
	short			max;
	EERIE_LIGHT **	el;
};

#endif // ARX_GRAPHICS_DATA_MESH_H
