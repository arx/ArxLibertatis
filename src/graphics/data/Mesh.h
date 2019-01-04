/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include <vector>

#include "ai/Anchors.h"
#include "graphics/GraphicsTypes.h"
#include "math/Rectangle.h"
#include "game/Camera.h"
#include "scene/Background.h"

class Entity;

extern long EERIEDrawnPolys;

extern BackgroundData * ACTIVEBKG;

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
bool GetTruePolyY(const PortalPoly * ep, const Vec3f & pos, float * ret);

bool IsAnyPolyThere(float x, float z);
EERIEPOLY * GetMinPoly(const Vec3f & pos);
EERIEPOLY * GetMaxPoly(const Vec3f & pos);
 
int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z);
int PointIn2DPolyXZ(const PortalPoly * ep, float x, float z);

Vec3f EE_RT(const Vec3f & in);
Vec4f worldToClipSpace(const Vec3f & in);
void worldToClipSpace(const Vec3f & in, TexturedVertex & out);


// FAST SAVE LOAD
bool FastSceneLoad(const res::path & path, Vec3f & trans);

struct RenderMaterial;

void Draw3DObject(EERIE_3DOBJ * eobj, const Anglef & angle, const Vec3f & pos, const Vec3f & scale, const Color4f & coll, RenderMaterial mat);

long MakeTopObjString(Entity * io, std::string & dest);

Vec2f getWaterFxUvOffset(float watereffect, const Vec3f & odtv);

float PtIn2DPolyProj(const std::vector<Vec4f> & verts, EERIE_FACE * ef, float x, float z);

void EERIE_PORTAL_Release();

bool RayCollidingPoly(const Vec3f & orgn, const Vec3f & dest, const EERIEPOLY & ep, Vec3f * hit);


#define MAX_FRUSTRUMS 32

struct Plane {
	float a;
	float b;
	float c;
	float d; // dist to origin
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

struct EERIE_FRUSTRUM {
	Plane plane[4];
};

struct EERIE_FRUSTRUM_DATA {
	long nb_frustrums;
	EERIE_FRUSTRUM frustrums[MAX_FRUSTRUMS];
};

struct PORTAL_ROOM_DRAW {
	short count;
	EERIE_FRUSTRUM_DATA frustrum;
};

struct ROOM_DIST_DATA {
	
	float distance; // -1 means use truedist
	Vec3f startpos;
	Vec3f endpos;
	
	ROOM_DIST_DATA()
		: distance(0.f)
		, startpos(0.f)
		, endpos(0.f)
	{ }
	
};

extern std::vector<ROOM_DIST_DATA> g_roomDistance;

void UpdateIORoom(Entity * io);

void FreeRoomDistance();
float SP_GetRoomDist(const Vec3f & pos, const Vec3f & c_pos, long io_room, long Cam_Room);

void EERIE_PORTAL_ReleaseOnlyVertexBuffer();
void ComputePortalVertexBuffer();

#endif // ARX_GRAPHICS_DATA_MESH_H
