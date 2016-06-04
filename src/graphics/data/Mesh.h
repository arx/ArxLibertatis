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
#include "game/Camera.h"

class Entity;

struct EERIE_BKG_INFO
{
	bool				treat;
	short				nbpoly;
	short				nbianchors;
	short				nbpolyin;
	EERIEPOLY *			polydata;
	EERIEPOLY **		polyin;
	long *				ianchors; // index on anchors list
	
	EERIE_BKG_INFO()
		: treat(false)
		, nbpoly(0)
		, nbianchors(0)
		, nbpolyin(0)
		, polydata(NULL)
		, polyin(NULL)
		, ianchors(NULL)
	{}
};

static const short MAX_BKGX = 160;
static const short MAX_BKGZ = 160;
static const short BKG_SIZX = 100;
static const short BKG_SIZZ = 100;

struct ANCHOR_DATA;

struct EERIE_BACKGROUND
{
	EERIE_BKG_INFO	fastdata[MAX_BKGX][MAX_BKGZ];
	long		exist;
	short		Xsize;
	short		Zsize;
	short		Xdiv;
	short		Zdiv;
	float		Xmul;
	float		Zmul;
	long		  nbanchors;
	ANCHOR_DATA * anchors;
	
	EERIE_BACKGROUND()
		: exist(false)
		, Xsize(0)
		, Zsize(0)
		, Xdiv(0)
		, Zdiv(0)
		, Xmul(0)
		, Zmul(0)
		, nbanchors(0)
		, anchors(NULL)
	{ }
};

extern long EERIEDrawnPolys;

extern EERIE_BACKGROUND * ACTIVEBKG;
extern EERIE_CAMERA * ACTIVECAM;

//	Entity Struct End

bool Visible(const Vec3f & orgn, const Vec3f & dest, Vec3f * hit);

EERIE_BKG_INFO * getFastBackgroundData(float x, float z);

EERIEPOLY * CheckTopPoly(const Vec3f & pos);
EERIEPOLY * CheckInPoly(const Vec3f & poss, float * needY = NULL);

/*!
 * Check if the given condition is under water.
 * 
 * \return the lowest water polygon pos is under, or NULL if pos is not under water.
 */
EERIEPOLY * EEIsUnderWater(const Vec3f & pos);

/*!
 * Get the height a polygon as at a specific position
 *
 * \param pos the (x, z) postion where to query the height. The y component is ignored.
 *
 * \return false if no height could be calculated because the the polygon's normal lies
 *         on the xz plane, true otherwise.
 */
bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f & pos, float * ret);

bool IsAnyPolyThere(float x, float z);
bool IsVertexIdxInGroup(EERIE_3DOBJ * eobj, size_t idx, size_t grs);
EERIEPOLY * GetMinPoly(const Vec3f & pos);
EERIEPOLY * GetMaxPoly(const Vec3f & pos);
 
int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z);

int EERIELaunchRay3(const Vec3f & orgn, const Vec3f & dest, Vec3f & hit, long flag);

Vec3f EE_RT(const Vec3f & in);
void EE_P(const Vec3f & in, TexturedVertex & out);
void EE_RTP(const Vec3f & in, TexturedVertex & out);


// FAST SAVE LOAD
bool FastSceneLoad(const res::path & path);

struct RenderMaterial;

void Draw3DObject(EERIE_3DOBJ * eobj, const Anglef & angle, const Vec3f & pos, const Vec3f & scale, const Color4f & coll, RenderMaterial mat);

//****************************************************************************
// BACKGROUND MANAGEMENT FUNCTIONS START
long BKG_CountPolys(const EERIE_BACKGROUND & eb);
long BKG_CountIgnoredPolys(const EERIE_BACKGROUND & eb);

#if BUILD_EDIT_LOADSAVE
void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms);
#endif

void ClearBackground(EERIE_BACKGROUND * eb);
int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv);

void EERIEAddPoly(TexturedVertex * vert, TexturedVertex * vert2, TextureContainer * tex, long render, float transval);
// BACKGROUND MANAGEMENT FUNCTIONS END
//****************************************************************************

long MakeTopObjString(Entity * io, std::string& dest);
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj);
Vec2f getWaterFxUvOffset(const Vec3f & odtv);

//*************************************************************************************
//*************************************************************************************

long EERIERTPPoly(EERIEPOLY *ep);

float PtIn2DPolyProj(const std::vector<EERIE_VERTEX> & verts, EERIE_FACE * ef, float x, float z);

long CountBkgVertex();

bool RayCollidingPoly(const Vec3f & orgn, const Vec3f & dest, const EERIEPOLY & ep, Vec3f * hit);

void EERIEPOLY_Compute_PolyIn();


#define MAX_FRUSTRUMS 32

struct Plane
{
	float	a;
	float	b;
	float	c;
	float	d; // dist to origin
};

inline float distanceToPoint(const Plane & plane, const Vec3f & point) {
	return point.x * plane.a + point.y * plane.b + point.z * plane.c + plane.d;
}

inline void normalizePlane(Plane & plane) {
	
	float n = 1.f / std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
	
	plane.a = plane.a * n;
	plane.b = plane.b * n;
	plane.c = plane.c * n;
	plane.d = plane.d * n;
}

struct EERIE_FRUSTRUM
{
	Plane plane[4];
};

struct EERIE_FRUSTRUM_DATA
{
	long nb_frustrums;
	EERIE_FRUSTRUM frustrums[MAX_FRUSTRUMS];
};

struct PORTAL_ROOM_DRAW
{
	short			count;
	EERIE_FRUSTRUM_DATA	frustrum;
};

struct ROOM_DIST_DATA
{
	float	distance; // -1 means use truedist
	Vec3f startpos;
	Vec3f endpos;
};

extern ROOM_DIST_DATA * RoomDistance;

void UpdateIORoom(Entity * io);
float SP_GetRoomDist(const Vec3f & pos, const Vec3f & c_pos, long io_room, long Cam_Room);
void EERIE_PORTAL_ReleaseOnlyVertexBuffer();
void ComputePortalVertexBuffer();
bool GetNameInfo( const std::string& name1,long& type,long& val1,long& val2);


#endif // ARX_GRAPHICS_DATA_MESH_H
