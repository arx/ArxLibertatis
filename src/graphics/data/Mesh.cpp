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

#include "graphics/data/Mesh.h"

#include <cstdlib>
#include <cstdio>
#include <map>

#include <boost/scoped_array.hpp>
#include <boost/unordered_map.hpp>

#include "ai/PathFinder.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/Player.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/VertexBuffer.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/FastSceneFormat.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"
#include "io/fs/FileStream.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/Blast.h"
#include "io/Implode.h"
#include "io/IO.h"
#include "io/log/Logger.h"

#include "physics/Anchors.h"

#include "scene/Scene.h"
#include "scene/Light.h"
#include "scene/Interactive.h"

#include "util/String.h"

using std::min;
using std::max;
using std::copy;
using std::string;
using std::vector;

void ComputeFastBkgData(EERIE_BACKGROUND * eb);

static void EERIE_PORTAL_Release();

float Xratio = 1.f;
float Yratio = 1.f;

static bool RayIn3DPolyNoCull(Vec3f * orgn, Vec3f * dest, EERIEPOLY * epp);

EERIEMATRIX ProjectionMatrix;

extern TextureContainer * sphere_particle;

void DebugAddParticle(const Vec3f & position, float siz, long tim, Color color) {
	
	arxtime.update();
	
	PARTICLE_DEF * pd = createParticle();
	if(!pd) {
		return;
	}
	
	pd->ov = position;
	pd->scale = Vec3f_ZERO;
	pd->tolive = tim;
	pd->tc = sphere_particle;
	pd->siz = siz;
	pd->rgb = color.to<float>();
}

static bool IntersectLinePlane(const Vec3f & l1, const Vec3f & l2, const EERIEPOLY * ep, Vec3f * intersect) {
	
	Vec3f v = l2 - l1;
	
	float d = glm::dot(v, ep->norm);
	
	if(d != 0.0f) {
		Vec3f v1 = ep->center - l2;
		d = glm::dot(v1, ep->norm) / d;
		
		*intersect = (v * d) + l2;
		
		return true;
	}

	return false;
}

bool RayCollidingPoly(Vec3f * orgn, Vec3f * dest, EERIEPOLY * ep, Vec3f * hit)
{
	if(IntersectLinePlane(*orgn, *dest, ep, hit)) {
		if(RayIn3DPolyNoCull(orgn, dest, ep))
			return true;
	}

	return false;
}

long MakeTopObjString(Entity * io,  string & dest) {
	
	if(!io) {
		return -1;
	}
	
	EERIE_3D_BBOX box;
	box.reset();

	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		box.add(io->obj->vertexlist3[i].v);
	}
	box.min.y -= 5.f;
	box.max.y -= 5.f;
	
	dest = "";
	
	if(player.pos.x > box.min.x
			&& player.pos.x < box.max.x
			&& player.pos.z > box.min.z
			&& player.pos.z < box.max.z)
	{
		if(EEfabs(player.pos.y + 160.f - box.min.y) < 50.f)
			dest += " player";
	}

	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] && entities[i] != io) {
			if(entities[i]->show == SHOW_FLAG_IN_SCENE) {
				if((entities[i]->ioflags & IO_NPC) || (entities[i]->ioflags & IO_ITEM)) {
					if(entities[i]->pos.x > box.min.x
							&& entities[i]->pos.x < box.max.x
							&& entities[i]->pos.z > box.min.z
							&& entities[i]->pos.z < box.max.z)
					{
						if(EEfabs(entities[i]->pos.y - box.min.y) < 40.f) {
							dest += ' ' + entities[i]->long_name();
						}
					}
				}
			}
		}
	}

	if(dest.length() == 0)
		dest = "none";

	return -1;
}


EERIEPOLY * CheckInPoly(float x, float y, float z, float * needY)
{
	Vec3f poss(x, y, z);

	long px = poss.x * ACTIVEBKG->Xmul;
	long pz = poss.z * ACTIVEBKG->Zmul;

	if(pz <= 0 || pz >= ACTIVEBKG->Zsize - 1 || px <= 0 || px >= ACTIVEBKG->Xsize - 1)
		return NULL;

	float rx = poss.x - ((float)px * ACTIVEBKG->Xdiv);
	float rz = poss.z - ((float)pz * ACTIVEBKG->Zdiv);


	short pzi, pza, pxi, pxa;

	(void)checked_range_cast<short>(pz - 1);
	(void)checked_range_cast<short>(pz + 1);
	short sPz = static_cast<short>(pz);

	if (rz < -40.f) {
		pzi = sPz - 1;
		pza = sPz - 1;
	} else if (rz < 40.f) {
		pzi = sPz - 1;
		pza = sPz;
	} else if(rz > 60.f) {
		pzi = sPz;
		pza = sPz + 1;
	} else {
		pzi = sPz;
		pza = sPz;
	}

	(void)checked_range_cast<short>(px + 1);
	(void)checked_range_cast<short>(px - 1);
	short sPx = static_cast<short>(px);

	if(rx < -40.f) {
		pxi = sPx - 1;
		pxa = sPx - 1;
	} else if(rx < 40.f) {
		pxi = sPx - 1;
		pxa = sPx;
	} else if(rx > 60.f) {
		pxi = sPx;
		pxa = sPx + 1;
	} else {
		pxi = sPx;
		pxa = sPx;
	}

	EERIEPOLY * found = NULL;
	float foundY = 0.f;

	for(short j = pzi; j <= pza; j++) {
		for(short i = pxi; i <= pxa; i++) {
			FAST_BKG_DATA * feg = &ACTIVEBKG->fastdata[i][j];

			for(short k = 0; k < feg->nbpolyin; k++) {
				EERIEPOLY * ep = feg->polyin[k];

				if(poss.x >= ep->min.x
				&& poss.x <= ep->max.x
				&& poss.z >= ep->min.z
				&& poss.z <= ep->max.z
				&& !(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				&& ep->max.y >= poss.y
				&& ep != found
				&& PointIn2DPolyXZ(ep, poss.x, poss.z)
				&& GetTruePolyY(ep, &poss, &rz)
				&& rz >= poss.y
				&& (!found || (found && rz <= foundY))
				) {
					found = ep;
					foundY = rz;
				}
			}
		}
	}

	if(needY)
		*needY = foundY;

	return found;
}

EERIEPOLY * EECheckInPoly(const Vec3f * pos, float * needY) {
	return CheckInPoly(pos->x, pos->y, pos->z, needY);
}

FAST_BKG_DATA * getFastBackgroundData(float x, float z) {
	
	long px = x * ACTIVEBKG->Xmul;
	long pz = z * ACTIVEBKG->Zmul;

	if(px < 0 || px >= ACTIVEBKG->Xsize || pz < 0 || pz >= ACTIVEBKG->Zsize)
		return NULL;
	
	return &ACTIVEBKG->fastdata[px][pz];
}

EERIEPOLY * CheckTopPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if((!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
		   && (ep->min.y < y)
		   && (x >= ep->min.x) && (x <= ep->max.x)
		   && (z >= ep->min.z) && (z <= ep->max.z)
		   && (PointIn2DPolyXZ(ep, x, z))) {
			
			if((EEfabs(ep->max.y - ep->min.y) > 50.f) && (y - ep->center.y < 60.f)) {
				continue;
			}
			
			if(ep->tex != NULL) {
				if(found == NULL || ep->min.y > found->min.y) {
					found = ep;
				}
			}
		}
	}
	
	return found;
}

bool IsAnyPolyThere(float x, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return false;
	}
	
	for(long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			return true;
		}
	}
	
	return false;
}

EERIEPOLY * GetMinPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return NULL;
	}
	
	Vec3f pos(x, y, z);
	
	EERIEPOLY * found = NULL;
	float foundy = 0.0f;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];

		if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			continue;
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			float ret;
			if(GetTruePolyY(ep, &pos, &ret)) {
				if(!found || ret > foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * GetMaxPoly(float x, float y, float z) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(x, z);
	if(!feg) {
		return NULL;
	}
	
	Vec3f pos(x, y, z);
	
	EERIEPOLY * found = NULL;
	float foundy = 0.0f;
	for(long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			continue;
		
		if(PointIn2DPolyXZ(ep, x, z)) {
			float ret;
			if(GetTruePolyY(ep, &pos, &ret)) {
				if(!found || ret < foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * EEIsUnderWater(const Vec3f * pos) {
	
	FAST_BKG_DATA * feg = getFastBackgroundData(pos->x, pos->z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	for(short k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & POLY_WATER) {
			if(ep->max.y < pos->y && PointIn2DPolyXZ(ep, pos->x, pos->z)) {
				if(!found || ep->max.y < found->max.y) {
					found = ep;
				}
			}
		}
	}
	return found;
}

bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f * pos, float * ret) {
	
	
	Vec3f s21 = ep->v[1].p - ep->v[0].p;
	Vec3f s31 = ep->v[2].p - ep->v[0].p;
	
	Vec3f n;
	n.y = (s21.z * s31.x) - (s21.x * s31.z);
	if (n.y == 0.f) return false; 
	n.x = (s21.y * s31.z) - (s21.z * s31.y);
	n.z = (s21.x * s31.y) - (s21.y * s31.x);
	
	// uses s21.x instead of d
	s21.x = ep->v[0].p.x * n.x + ep->v[0].p.y * n.y + ep->v[0].p.z * n.z;
	
	s21.x = (s21.x - (n.x * pos->x) - (n.z * pos->z)) / n.y;
	
	// Perhaps we can remove the two following lines... (need to test)
	if (s21.x < ep->min.y) s21.x = ep->min.y;
	else if (s21.x > ep->max.y) s21.x = ep->max.y;
	
	*ret = s21.x;
	return true;
}

//*************************************************************************************
//*************************************************************************************
EERIE_BACKGROUND * ACTIVEBKG = NULL;

//*************************************************************************************
// Selects Active Background Structure
//*************************************************************************************
EERIE_CAMERA * ACTIVECAM = NULL;

//*************************************************************************************
// Selects Active Camera
//*************************************************************************************
void SetActiveCamera(EERIE_CAMERA * cam)
{
	if (ACTIVECAM != cam) ACTIVECAM = cam;
}

void EE_RT(const Vec3f & in, Vec3f & out) {
	out = Vec3f(ACTIVECAM->orgTrans.worldToView * Vec4f(in, 1.0f));
}

void EE_RT2(TexturedVertex * in, TexturedVertex * out) {
	EE_RT(in->p, out->p);
}

// TODO get rid of sw transform
static inline float clamp_and_invert(float z) {
	
	const float near_clamp = .000001f; // just a random small number
	
	return 1.f / std::max(z, near_clamp);
}

void EE_P(Vec3f * in, TexturedVertex * out) {
	
	float fZTemp = clamp_and_invert(in->z);
	
	out->p.z = fZTemp * ProjectionMatrix._33 + ProjectionMatrix._43; //HYPERBOLIC
	out->p.x = in->x * ProjectionMatrix._11 * fZTemp + ACTIVECAM->orgTrans.mod.x;
	out->p.y = in->y * ProjectionMatrix._22 * fZTemp + ACTIVECAM->orgTrans.mod.y;
	out->rhw = fZTemp;
}

void EE_RTP(TexturedVertex * in, TexturedVertex * out) {
	EE_RT(in->p, out->p);
	EE_P(&out->p, out);
}

static void camEE_RTP(TexturedVertex * in, TexturedVertex * out, EERIE_CAMERA * cam) {
	
	TexturedVertex tout;
	out->p = in->p - cam->orgTrans.pos;

	tout.p.x = (out->p.x * cam->orgTrans.ycos) + (out->p.z * cam->orgTrans.ysin);
	tout.p.z = (out->p.z * cam->orgTrans.ycos) - (out->p.x * cam->orgTrans.ysin);
	out->p.z = (out->p.y * cam->orgTrans.xsin) + (tout.p.z * cam->orgTrans.xcos);
	out->p.y = (out->p.y * cam->orgTrans.xcos) - (tout.p.z * cam->orgTrans.xsin);
	tout.p.y = (out->p.y * cam->orgTrans.zcos) - (tout.p.x * cam->orgTrans.zsin);
	tout.p.x = (tout.p.x * cam->orgTrans.zcos) + (out->p.y * cam->orgTrans.zsin);

	if (out->p.z <= 0.f)
	{
		out->rhw = 1.f - out->p.z;
	}
	else
	{
		out->rhw = 1.f / out->p.z;
	}

	tout.rhw = (cam->focal * Xratio) * out->rhw;
	out->p.z = out->p.z * (1.f / (cam->cdepth * 1.2f));
	out->p.x = cam->orgTrans.mod.x + (tout.p.x * tout.rhw);
	out->p.y = cam->orgTrans.mod.y + (tout.p.y * tout.rhw) ;
}

//*************************************************************************************
//*************************************************************************************
static void EERIERTPPolyCam(EERIEPOLY * ep, EERIE_CAMERA * cam) {
	
	camEE_RTP(&ep->v[0], &ep->tv[0], cam);
	camEE_RTP(&ep->v[1], &ep->tv[1], cam);
	camEE_RTP(&ep->v[2], &ep->tv[2], cam);

	if (ep->type & POLY_QUAD) camEE_RTP(&ep->v[3], &ep->tv[3], cam);
}


//*************************************************************************************
//*************************************************************************************

extern float GetIOHeight(Entity * io);
extern float GetIORadius(Entity * io);

long GetVertexPos(Entity * io, long id, Vec3f * pos)
{
	if (!io) return 0;

	if (id != -1)
	{
		*pos = io->obj->vertexlist3[id].v;
		return 1;
	}
	else
	{
		*pos = io->pos + Vec3f(0.f, GetIOHeight(io), 0.f);
		return 2;
	}
}

long EERIEDrawnPolys = 0;

// Checks if point (x,y) is in a 2D poly defines by ep
int PointIn2DPoly(EERIEPOLY * ep, float x, float y) {
	
	int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((ep->tv[i].p.y <= y) && (y < ep->tv[j].p.y)) ||
				((ep->tv[j].p.y <= y) && (y < ep->tv[i].p.y))) &&
				(x < (ep->tv[j].p.x - ep->tv[i].p.x) *(y - ep->tv[i].p.y) / (ep->tv[j].p.y - ep->tv[i].p.y) + ep->tv[i].p.x))
			c = !c;
	}

	if (c) return c;
	else if (ep->type & POLY_QUAD)
		for (i = 1, j = 3; i < 4; j = i++)
		{
			if ((((ep->tv[i].p.y <= y) && (y < ep->tv[j].p.y)) ||
					((ep->tv[j].p.y <= y) && (y < ep->tv[i].p.y))) &&
					(x < (ep->tv[j].p.x - ep->tv[i].p.x) *(y - ep->tv[i].p.y) / (ep->tv[j].p.y - ep->tv[i].p.y) + ep->tv[i].p.x))
				c = !c;
		}

	return c;
}

//*************************************************************************************
//*************************************************************************************

float PtIn2DPolyProj(EERIE_3DOBJ * obj, EERIE_FACE * ef, float x, float z) {
	
	int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((obj->vertexlist[ef->vid[i]].vert.p.y <= z) && (z < obj->vertexlist[ef->vid[j]].vert.p.y)) ||
				((obj->vertexlist[ef->vid[j]].vert.p.y <= z) && (z < obj->vertexlist[ef->vid[i]].vert.p.y))) &&
				(x < (obj->vertexlist[ef->vid[j]].vert.p.x - obj->vertexlist[ef->vid[i]].vert.p.x) *(z - obj->vertexlist[ef->vid[i]].vert.p.y) / (obj->vertexlist[ef->vid[j]].vert.p.y - obj->vertexlist[ef->vid[i]].vert.p.y) + obj->vertexlist[ef->vid[i]].vert.p.x))
			c = !c;
	}

	if (c)
		return obj->vertexlist[ef->vid[0]].vert.p.z;
	else
		return 0.f;
}

float CEDRIC_PtIn2DPolyProjV2(EERIE_3DOBJ * obj, EERIE_FACE * ef, float x, float z) {
	
	int i, j, c = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((obj->vertexlist3[ef->vid[i]].vert.p.y <= z) && (z < obj->vertexlist3[ef->vid[j]].vert.p.y)) ||
				((obj->vertexlist3[ef->vid[j]].vert.p.y <= z) && (z < obj->vertexlist3[ef->vid[i]].vert.p.y))) &&
				(x < (obj->vertexlist3[ef->vid[j]].vert.p.x - obj->vertexlist3[ef->vid[i]].vert.p.x) *(z - obj->vertexlist3[ef->vid[i]].vert.p.y) / (obj->vertexlist3[ef->vid[j]].vert.p.y - obj->vertexlist3[ef->vid[i]].vert.p.y) + obj->vertexlist3[ef->vid[i]].vert.p.x))
			c = !c;
	}

	if (c) return obj->vertexlist3[ef->vid[0]].vert.p.z;
	else return 0.f;
}

int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z) {
	
	int i, j, c = 0, d = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((ep->v[i].p.z <= z) && (z < ep->v[j].p.z)) ||
				((ep->v[j].p.z <= z) && (z < ep->v[i].p.z))) &&
				(x < (ep->v[j].p.x - ep->v[i].p.x) *(z - ep->v[i].p.z) / (ep->v[j].p.z - ep->v[i].p.z) + ep->v[i].p.x))
			c = !c;
	}

	if (ep->type & POLY_QUAD)
		for (i = 1, j = 3; i < 4; j = i++)
		{
			if ((((ep->v[i].p.z <= z) && (z < ep->v[j].p.z)) ||
					((ep->v[j].p.z <= z) && (z < ep->v[i].p.z))) &&
					(x < (ep->v[j].p.x - ep->v[i].p.x) *(z - ep->v[i].p.z) / (ep->v[j].p.z - ep->v[i].p.z) + ep->v[i].p.x))
				d = !d;
		}

	return c + d;
}

extern EERIE_CAMERA raycam;

static void SP_PrepareCamera(EERIE_CAMERA * cam) {
	cam->orgTrans.updateFromAngle(cam->angle);
	cam->orgTrans.mod = Vec2f(cam->center + cam->clip.origin.toVec2());
}

static bool RayIn3DPolyNoCull(Vec3f * orgn, Vec3f * dest, EERIEPOLY * epp) {

	EERIEPOLY ep;
	memcpy(&ep, epp, sizeof(EERIEPOLY));
	raycam.orgTrans.pos = *orgn;
	raycam.setTargetCamera(*dest);
	SP_PrepareCamera(&raycam);
	EERIERTPPolyCam(&ep, &raycam);

	if(PointIn2DPoly(&ep, 320.f, 320.f))
		return true;

	return false;
}

int EERIELaunchRay3(Vec3f * orgn, Vec3f * dest,  Vec3f * hit, EERIEPOLY * epp, long flag) {
	
	Vec3f p; //current ray pos
	Vec3f d; // ray incs
	Vec3f ad; // absolute ray incs
	Vec3f i;
	long lpx, lpz;
	long voidlast;
	long px, pz;
	float pas = 1.5f;
	
	long iii = 0;
	float maxstepp = 20000.f / pas;
	*hit = p = *orgn;
	
	voidlast = 0;
	lpx = lpz = -1;
	d.x = (dest->x - orgn->x);
	ad.x = EEfabs(d.x);
	d.y = (dest->y - orgn->y);
	ad.y = EEfabs(d.y);
	d.z = (dest->z - orgn->z);
	ad.z = EEfabs(d.z);
	
	if(ad.x >= ad.y && ad.x >= ad.z) {
		i.x = (ad.x != d.x) ? (-1.f * pas) : (1.f * pas);
		i.y = d.y / (ad.x / pas);
		i.z = d.z / (ad.x / pas);
	} else if(ad.y >= ad.x && ad.y >= ad.z) {
		i.x = d.x / (ad.y / pas);
		i.y = (ad.y != d.y) ? (-1.f * pas) : (1.f * pas);
		i.z = d.z / (ad.y / pas);
	} else {
		i.x = d.x / (ad.z / pas);
		i.y = d.y / (ad.z / pas);
		i.z = (ad.z != d.z) ? (-1.f * pas) : (1.f * pas);
	}
	
	for(;;) {
		
		p += i;
		
		if(i.x == -1.f * pas && p.x <= dest->x) {
			*hit = p;
			return 0;
		}
		
		if(i.x == 1.f * pas && p.x >= dest->x) {
			*hit = p;
			return 0;
		}
		
		if(i.y == -1.f * pas && p.y <= dest->y) {
			*hit = p;
			return 0;
		}
		
		if(i.y == 1.f * pas && p.y >= dest->y) {
			*hit = p;
			return 0;
		}
		
		if(i.z == -1.f * pas && p.z <= dest->z) {
			*hit = p;
			return 0;
		}
		
		if(i.z == 1.f * pas && p.z >= dest->z) {
			*hit = p;
			return 0;
		}
		
		iii++;
		
		if(iii > maxstepp) {
			*hit = p;
			return -1;
		}
		
		px = long(p.x * ACTIVEBKG->Xmul);
		pz = long(p.z * ACTIVEBKG->Zmul);
		
		if(px < 0 || px > ACTIVEBKG->Xsize - 1 || pz < 0 || pz > ACTIVEBKG->Zsize - 1) {
			*hit = p;
			return -1;
		}
		
		if(lpx == px && lpz == pz && voidlast) {
			continue;
		}
		
		lpx = px;
		lpz = pz;
		voidlast = !flag;
		long jx1 = clamp(px - 1l, 0l, ACTIVEBKG->Xsize - 1l);
		long jx2 = clamp(px + 1l, 0l, ACTIVEBKG->Xsize - 1l);
		long jz1 = clamp(pz - 1l, 0l, ACTIVEBKG->Zsize - 1l);
		long jz2 = clamp(pz + 1l, 0l, ACTIVEBKG->Zsize - 1l);
		
		EERIE_BKG_INFO * eg = &ACTIVEBKG->Backg[px + pz * ACTIVEBKG->Xsize];
		if(eg->nbpoly == 0) {
			*hit = p;
			return 1;
		}
		
		for(pz = jz1; pz < jz2; pz++) for (px = jx1; px < jx2; px++) {
			eg = &ACTIVEBKG->Backg[px + pz * ACTIVEBKG->Xsize];
			for(long k = 0; k < eg->nbpoly; k++) {
				EERIEPOLY * ep = &eg->polydata[k];
				if(ep->type & POLY_TRANS) {
					continue;
				}
				if(p.y < ep->min.y - 10.f || p.y > ep->max.y + 10.f
				   || p.x < ep->min.x - 10.f || p.x > ep->max.x + 10.f
				   || p.z < ep->min.z - 10.f || p.z > ep->max.z + 10.f) {
					continue;
				}
				voidlast = 0;
				if(RayIn3DPolyNoCull(orgn, dest, ep)) {
					*hit = p;
					return (ep == epp) ? 0 : 1;
				}
			}
		}
	}
}

// Computes the visibility from a point to another... (sort of...)
bool Visible(Vec3f * orgn, Vec3f * dest, EERIEPOLY * epp, Vec3f * hit)
{
	float ix,iy,iz;
	long px,pz;
	EERIEPOLY *ep;

	EERIE_BKG_INFO *eg;
	float pas = 35.f;

	Vec3f found_hit = Vec3f_ZERO;
	EERIEPOLY *found_ep = NULL;
	float iter,t;
	
	//current ray pos
	float x = orgn->x;
	float y = orgn->y;
	float z = orgn->z;

	float distance;
	float nearest = distance = fdist(*orgn, *dest);

	if(distance < pas)
		pas = distance * .5f;

	// ray incs
	float dx = (dest->x - orgn->x);
	float dy = (dest->y - orgn->y);
	float dz = (dest->z - orgn->z);

	// absolute ray incs
	float adx = EEfabs(dx);
	float ady = EEfabs(dy);
	float adz = EEfabs(dz);

	if(adx >= ady && adx >= adz) {
		if(adx != dx)
			ix = -pas;
		else
			ix = pas;

		iter = adx / pas;
		t = 1.f / (iter);
		iy = dy * t;
		iz = dz * t;
	} else if(ady >= adx && ady >= adz) {
		if(ady != dy)
			iy = -pas;
		else
			iy = pas;

		iter = ady / pas;
		t = 1.f / (iter);
		ix = dx * t;
		iz = dz * t;
	} else {
		if(adz != dz)
			iz = -pas;
		else
			iz = pas;

		iter = adz / pas;
		t = 1.f / (iter);
		ix = dx * t;
		iy = dy * t;
	}

	float dd;
	x -= ix;
	y -= iy;
	z -= iz;

	while(iter > 0.f) {
		iter -= 1.f;
		x += ix;
		y += iy;
		z += iz;

		px = (long)(x * ACTIVEBKG->Xmul);
		pz = (long)(z * ACTIVEBKG->Zmul);

		if(px < 0 || px > ACTIVEBKG->Xsize - 1 || pz < 0 || pz > ACTIVEBKG->Zsize - 1)
			break;

		eg = &ACTIVEBKG->Backg[px+pz*ACTIVEBKG->Xsize];

		for(long k = 0; k < eg->nbpolyin; k++) {
			ep = eg->polyin[k];

			if (ep)
			if ((ep->min.y - pas < y) && (ep->max.y + pas > y))
			if ((ep->min.x - pas < x) && (ep->max.x + pas > x))
			if ((ep->min.z - pas < z) && (ep->max.z + pas > z))
			if (RayCollidingPoly(orgn, dest, ep, hit)) {
				dd = fdist(*orgn, *hit);

				if(dd < nearest) {
					nearest = dd;
					found_ep = ep;
					found_hit = *hit;
				}
			}
		}
	}

	if(!found_ep)
		return true;

	if(found_ep == epp)
		return true;
	
	*hit = found_hit;
	
	return false;
}


//*************************************************************************************
// Counts total number of polys in a background
//*************************************************************************************
long BKG_CountPolys(EERIE_BACKGROUND * eb) {
	long count = 0;

	for(long i = 0; i < eb->Xsize * eb->Zsize; i++) {
		EERIE_BKG_INFO *eg = &eb->Backg[i];
		count += eg->nbpoly;
	}

	return count;
}

//*************************************************************************************
// Counts number of ignored polys in a background
//*************************************************************************************

long BKG_CountIgnoredPolys(EERIE_BACKGROUND * eb) {
	long count = 0;

	for(long i = 0; i < eb->Xsize * eb->Zsize; i++) {
		EERIE_BKG_INFO *eg = &eb->Backg[i];

		for(long k = 0; k < eg->nbpoly; k++){
			EERIEPOLY *pol = &eg->polydata[k];

			if(pol->type & POLY_IGNORE)
				count++;
		}
	}

	return count;
}

// Releases BKG_INFO from a tile
void ReleaseBKG_INFO(EERIE_BKG_INFO * eg) {
	free(eg->polydata), eg->polydata = NULL;
	free(eg->polyin), eg->polyin = NULL;
	eg->nbpolyin = 0;
	memset(eg, 0, sizeof(EERIE_BKG_INFO));
}

//*************************************************************************************
//*************************************************************************************

void AddAData(ANCHOR_DATA * ad, long linked)
{
	for(long i=0; i < ad->nblinked; i++)
		if(ad->linked[i] == linked)
			return;

	ad->linked = (long *)realloc(ad->linked, sizeof(long) * (ad->nblinked + 1));

	ad->linked[ad->nblinked] = linked;
	ad->nblinked++;
}

void UpdateIORoom(Entity * io)
{
	Vec3f pos = io->pos;
	pos.y -= 60.f;

	long roo = ARX_PORTALS_GetRoomNumForPosition(&pos, 2);

	if(roo >= 0)
		io->room = checked_range_cast<short>(roo);

	io->room_flags &= ~1;
}

bool GetRoomCenter(long room_num, Vec3f * center) {
	
	//TODO off by one ? (portals->nb_rooms + 1)
	if(!portals || room_num > portals->nb_rooms || portals->room[room_num].nb_polys <= 0)
		return false;
	
	EERIE_ROOM_DATA & room = portals->room[room_num];

	EERIE_3D_BBOX bbox;
	bbox.min = Vec3f(99999999.f);
	bbox.max = Vec3f(-99999999.f);

	for(long lll = 0; lll < room.nb_polys; lll++) {
		FAST_BKG_DATA * feg;
		feg = &ACTIVEBKG->fastdata[room.epdata[lll].px][room.epdata[lll].py];
		EERIEPOLY * ep = &feg->polydata[room.epdata[lll].idx];
		bbox.min = glm::min(bbox.min, ep->center);
		bbox.max = glm::max(bbox.max, ep->center);
	}
	
	*center = (bbox.max + bbox.min) * .5f;
	
	room.center = *center;
	room.radius = fdist(*center, bbox.max);
	return true;
}

ROOM_DIST_DATA * RoomDistance = NULL;
static long NbRoomDistance = 0;

static void SetRoomDistance(long i, long j, float val, const Vec3f * p1, const Vec3f * p2) {
	
	if(i < 0 || j < 0 || i >= NbRoomDistance || j >= NbRoomDistance || !RoomDistance)
		return;
	
	long offs = i + j * NbRoomDistance;
	
	if(p1)
		RoomDistance[offs].startpos = *p1;

	if(p2)
		RoomDistance[offs].endpos = *p2;

	RoomDistance[offs].distance = val;
}

static float GetRoomDistance(long i, long j, Vec3f * p1, Vec3f * p2)
{
	if(i < 0 || j < 0 || i >= NbRoomDistance || j >= NbRoomDistance)
		return -1.f;

	long offs = i + j * NbRoomDistance;

	if(p1)
		*p1 = RoomDistance[offs].startpos;

	if(p2)
		*p2 = RoomDistance[offs].endpos;

	return RoomDistance[offs].distance;
}

float SP_GetRoomDist(Vec3f * pos, Vec3f * c_pos, long io_room, long Cam_Room)
{
	float dst = fdist(*pos, *c_pos);

	if(dst < 150.f)
		return dst;

	if(!portals || !RoomDistance)
		return dst;

	long Room = io_room;

	if(Room >= 0) {
		Vec3f p1, p2;
		float v = GetRoomDistance(Cam_Room, Room, &p1, &p2);

		if(v > 0.f) {
			v += fdist(*pos, p2);
			v += fdist(*c_pos, p1);
			return v;
		}
	}

	return dst;
}

// Clears a background of its infos
void ClearBackground(EERIE_BACKGROUND * eb) {
	
	if(!eb)
		return;
	
	AnchorData_ClearAll(eb);
	
	free(eb->minmax), eb->minmax = NULL;
	
	for(long i = 0; i < eb->Xsize * eb->Zsize; i++) {
		ReleaseBKG_INFO(&eb->Backg[i]);
	}
	free(eb->Backg), eb->Backg = NULL;
	
	free(RoomDistance), RoomDistance = NULL;
	NbRoomDistance = 0;
}

int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv) {
	
	if(!eb)
		return 0;

	if(eb->exist) {
		EERIE_PORTAL_Release();
		ClearBackground(eb);
	}

	eb->exist = 1;
	eb->anchors = NULL;
	eb->nbanchors = 0;
	eb->Xsize = sx;
	eb->Zsize = sz;

	if (Xdiv < 0) Xdiv = 1;
	if (Zdiv < 0) Zdiv = 1;

	eb->Xdiv = Xdiv;
	eb->Zdiv = Zdiv;
	eb->Xmul = 1.f / (float)eb->Xdiv;
	eb->Zmul = 1.f / (float)eb->Zdiv;

	//todo free
	eb->Backg = (EERIE_BKG_INFO *)malloc(sizeof(EERIE_BKG_INFO) * sx * sz);

	memset(eb->Backg, 0, sizeof(EERIE_BKG_INFO) * sx * sz);

	for(int i = 0; i < eb->Xsize * eb->Zsize; i++) {
		EERIE_BKG_INFO *eg = &eb->Backg[i];
		eg->treat = 0;
		eg->nothing = 1;
		eg->nbianchors = 0;
		eg->ianchors = NULL;
	}

	for(long j = 0; j < eb->Zsize; j++)
		for(int i = 0; i < eb->Xsize; i++) {
			FAST_BKG_DATA *feg = &eb->fastdata[i][j];
			memset(feg, 0, sizeof(FAST_BKG_DATA));
		}

	//todo free
	eb->minmax = (EERIE_SMINMAX *)malloc(sizeof(EERIE_SMINMAX) * eb->Zsize);

	for(int i = 0; i < eb->Zsize; i++) {
		eb->minmax[i].min = 9999;
		eb->minmax[i].max = -1;
	}

	return 1;
}
//*************************************************************************************
// Checks for angular difference between normals
//*************************************************************************************
bool LittleAngularDiff(Vec3f * norm, Vec3f * norm2) {
	return closerThan(*norm, *norm2, 1.41421f);
}

//*************************************************************************************
//*************************************************************************************

void DeclareEGInfo(float x, float z)
{
	long posx = x * ACTIVEBKG->Xmul;
	long posz = z * ACTIVEBKG->Zmul;

	if(posx < 0 || posx >= ACTIVEBKG->Xsize || posz < 0 || posz >= ACTIVEBKG->Zsize)
		return;

	EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[posx+posz*ACTIVEBKG->Xsize];
	eg->nothing = 0;
}

void EERIEPOLY_Add_PolyIn(EERIE_BKG_INFO * eg, EERIEPOLY * ep)
{
	for(long i = 0; i < eg->nbpolyin; i++)
		if(eg->polyin[i] == ep)
			return;

	eg->polyin = (EERIEPOLY **)realloc(eg->polyin, sizeof(EERIEPOLY *) * (eg->nbpolyin + 1));

	eg->polyin[eg->nbpolyin] = ep;
	eg->nbpolyin++;
}

bool PointInBBox(Vec3f * point, EERIE_2D_BBOX * bb)
{
	if ((point->x > bb->max.x)
			||	(point->x < bb->min.x)
			||	(point->z > bb->max.y)
			||	(point->z < bb->min.y)
	   )
		return false;

	return true;
}

void EERIEPOLY_Compute_PolyIn()
{
	for(long j = 0; j < ACTIVEBKG->Zsize; j++)
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];
			
			free(eg->polyin), eg->polyin = NULL;
			eg->nbpolyin = 0;
			
			long ii = max(i - 2, 0L);
			long ij = max(j - 2, 0L);
			long ai = min(i + 2, ACTIVEBKG->Xsize - 1L);
			long aj = min(j + 2, ACTIVEBKG->Zsize - 1L);

			EERIE_2D_BBOX bb;
			bb.min.x = (float)i * ACTIVEBKG->Xdiv - 10;
			bb.max.x = (float)bb.min.x + ACTIVEBKG->Xdiv + 20;
			bb.min.y = (float)j * ACTIVEBKG->Zdiv - 10;
			bb.max.y = (float)bb.min.y + ACTIVEBKG->Zdiv + 20;
			Vec3f bbcenter;
			bbcenter.x = (bb.min.x + bb.max.x) * .5f;
			bbcenter.z = (bb.min.y + bb.max.y) * .5f;

			for(long cj = ij; cj < aj; cj++)
				for(long ci = ii; ci < ai; ci++) {
					EERIE_BKG_INFO *eg2 = &ACTIVEBKG->Backg[ci+cj*ACTIVEBKG->Xsize];

					for(long l = 0; l < eg2->nbpoly; l++) {
						EERIEPOLY *ep2 = &eg2->polydata[l];

						if(fartherThan(Vec2f(bbcenter.x, bbcenter.z), Vec2f(ep2->center.x, ep2->center.z), 120.f))
							continue;

						long nbvert = (ep2->type & POLY_QUAD) ? 4 : 3;

						if(PointInBBox(&ep2->center, &bb)) {
							EERIEPOLY_Add_PolyIn(eg, ep2);
						} else {
							for(long k = 0; k < nbvert; k++) {
								if(PointInBBox(&ep2->v[k].p, &bb)) {
									EERIEPOLY_Add_PolyIn(eg, ep2);
									break;
								} else {
									Vec3f pt = (ep2->v[k].p + ep2->center) * .5f;
									if(PointInBBox(&pt, &bb)) {
										EERIEPOLY_Add_PolyIn(eg, ep2);
										break;
									}
								}
							}
						}
					}
				}

			if(eg->nbpolyin)
				eg->nothing = 0;
			else
				eg->nothing = 1;
		}

	for(int j = 0; j < ACTIVEBKG->Zsize; j++)
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];
			eg->tile_miny = 999999999.f;
			eg->tile_maxy = -999999999.f;

			for(long kk = 0; kk < eg->nbpolyin; kk++) {
				EERIEPOLY *ep = eg->polyin[kk];
				eg->tile_miny = min(eg->tile_miny, ep->min.y);
				eg->tile_maxy = max(eg->tile_maxy, ep->max.y);
			}

			FAST_BKG_DATA * fbd = &ACTIVEBKG->fastdata[i][j];
			fbd->treat = eg->treat;
			fbd->nothing = eg->nothing;
			fbd->nbpoly = eg->nbpoly;
			fbd->nbianchors = eg->nbianchors;
			fbd->nbpolyin = eg->nbpolyin;
			fbd->frustrum_miny = eg->frustrum_miny;
			fbd->frustrum_maxy = eg->frustrum_maxy;
			fbd->polydata = eg->polydata;
			fbd->polyin = eg->polyin;
			fbd->ianchors = eg->ianchors;
		}
}

float GetTileMinY(long i, long j) {
	float minf = 9999999999.f;
	EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

	for (long kk = 0; kk < eg->nbpolyin; kk++) {
		EERIEPOLY * ep = eg->polyin[kk];
		minf = min(minf, ep->min.y);
	}

	return minf;
}

float GetTileMaxY(long i, long j) {
	float maxf = -9999999999.f;
	EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

	for(long kk = 0; kk < eg->nbpolyin; kk++) {
		EERIEPOLY * ep = eg->polyin[kk];
		maxf = max(maxf, ep->max.y);
	}

	return maxf;
}

#define TYPE_PORTAL	1
#define TYPE_ROOM	2
bool GetNameInfo(const string &name, long &type, long &val1, long &val2)
{
	if(name[0] == 'r') {
		if(name[1] == '_') {
			type = TYPE_ROOM;
			val1 = atoi(name.c_str() + 2);
			val2 = 0;
			return true;
		}

		if(name[1] == 'o' && name[2] == 'o' && name[3] == 'm' && name[4] == '_') {
			type = TYPE_ROOM;
			val1 = atoi(name.c_str() + 5);
			val2 = 0;
			return true;
		}
	}

	if ((name[0] == 'p') && (name[1] == 'o') && (name[2] == 'r')
			&& (name[3] == 't') && (name[4] == 'a') && (name[5] == 'l')
			&& (name[6] == '_'))
	{
		type = TYPE_PORTAL;
		char temp[16];
		strcpy(temp, name.c_str() + 7);
		temp[3] = 0;
		val1 = atoi(temp);
		val2 = atoi(name.c_str() + 11);
		return true;
	}

	return false;
}

void EERIE_PORTAL_Blend_Portals_And_Rooms() {
	
	if(!portals)
		return;

	for(size_t num = 0; num < portals->portals.size(); num++) {
		CalcFaceNormal(&portals->portals[num].poly, portals->portals[num].poly.v);
		EERIEPOLY * ep = &portals->portals[num].poly;
		ep->center = ep->v[0].p;

		long to = (ep->type & POLY_QUAD) ? 4 : 3;

		float divide = ( 1.0f / to );
		
		ep->max = ep->min = ep->v[0].p;
		for(long i = 1; i < to; i++) {
			ep->center += ep->v[i].p;
			ep->min = glm::min(ep->min, ep->v[i].p);
			ep->max = glm::max(ep->max, ep->v[i].p);
		}
		
		ep->center *= divide;
		float d = 0.f;

		for(long ii = 0; ii < to; ii++) {
			d = max(d, glm::distance(ep->center, ep->v[ii].p));
		}

		ep->norm2.x = d;

		for(long nroom = 0; nroom < portals->roomsize(); nroom++) {
			if(nroom == portals->portals[num].room_1 || nroom == portals->portals[num].room_2)
			{
				portals->room[nroom].portals = (long *)realloc(portals->room[nroom].portals, sizeof(long) * (portals->room[nroom].nb_portals + 1));
				portals->room[nroom].portals[portals->room[nroom].nb_portals] = num;
				portals->room[nroom].nb_portals++;
			}
		}
	}
}

static void EERIE_PORTAL_Release() {
	
	if(!portals)
		return;
	
	if(portals->room) {
		if(portals->nb_rooms > 0) {
			for(long nn = 0; nn < portals->roomsize(); nn++) {
				free(portals->room[nn].epdata), portals->room[nn].epdata = NULL;
				free(portals->room[nn].portals), portals->room[nn].portals = NULL;
				delete portals->room[nn].pVertexBuffer, portals->room[nn].pVertexBuffer = NULL;
				free(portals->room[nn].pussIndice), portals->room[nn].pussIndice = NULL;
				free(portals->room[nn].ppTextureContainer);
				portals->room[nn].ppTextureContainer = NULL;
			}
		}
		free(portals->room), portals->room = NULL;
	}
	
	delete portals;
	portals = NULL;
}

float EERIE_TransformOldFocalToNewFocal(float _fOldFocal)
{
	if(_fOldFocal < 200)
		return (-.34f * _fOldFocal + 168.5f);
	else if(_fOldFocal < 300)
		return (-.25f * _fOldFocal + 150.5f);
	else if(_fOldFocal < 400)
		return (-.155f * _fOldFocal + 124.f);
	else if(_fOldFocal < 500)
		return (-.11f * _fOldFocal + 106.f);
	else if(_fOldFocal < 600)
		return (-.075f * _fOldFocal + 88.5f);
	else if(_fOldFocal < 700)
		return (-.055f * _fOldFocal + 76.5f);
	else if(_fOldFocal < 800)
		return (-.045f * _fOldFocal + 69.5f);
	else
		return 33.5f;
}

void EERIE_CreateMatriceProj(float _fWidth, float _fHeight, EERIE_CAMERA * cam) {

	float _fFOV = EERIE_TransformOldFocalToNewFocal(cam->focal);
	float _fZNear = 1.f;
	float _fZFar = cam->cdepth;


	float fAspect = _fHeight / _fWidth;
	float fFOV = radians(_fFOV);
	float fFarPlane = _fZFar;
	float fNearPlane = _fZNear;
	float w = fAspect * (cosf(fFOV / 2) / sinf(fFOV / 2));
	float h =   1.0f  * (cosf(fFOV / 2) / sinf(fFOV / 2));
	float Q = fFarPlane / (fFarPlane - fNearPlane);

	memset(&ProjectionMatrix, 0, sizeof(EERIEMATRIX));
	ProjectionMatrix._11 = w;
	ProjectionMatrix._22 = h;
	ProjectionMatrix._33 = Q;
	ProjectionMatrix._43 = (-Q * fNearPlane);
	ProjectionMatrix._34 = 1.f;
	GRenderer->SetProjectionMatrix(ProjectionMatrix);

	EERIEMATRIX tempViewMatrix;
	Util_SetViewMatrix(tempViewMatrix, cam->orgTrans);
	GRenderer->SetViewMatrix(tempViewMatrix);

	ProjectionMatrix._11 *= _fWidth * .5f;
	ProjectionMatrix._22 *= _fHeight * .5f;
	ProjectionMatrix._33 = -(fFarPlane * fNearPlane) / (fFarPlane - fNearPlane);	//HYPERBOLIC
	ProjectionMatrix._43 = Q;

	GRenderer->SetViewport(Rect(static_cast<s32>(_fWidth), static_cast<s32>(_fHeight)));
}

void PrepareCamera(EERIE_CAMERA * cam)
{
	SP_PrepareCamera(cam);

	EERIE_CreateMatriceProj(static_cast<float>(g_size.width()),
							static_cast<float>(g_size.height()),
							cam);

}

void SetCameraDepth(EERIE_CAMERA &cam, float depth) {
	cam.cdepth = depth;
}

long CountBkgVertex() {

	long count = 0;

	for(long j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[i + j*ACTIVEBKG->Xsize];

			for(long l = 0; l < eg->nbpoly; l++) {
				EERIEPOLY *ep = &eg->polydata[l];

				if(ep) {
					if(ep->type & POLY_QUAD)
						count += 4;
					else
						count += 3;
				}
			}
		}
	}

	return count;
}


void DrawEERIEObjEx(EERIE_3DOBJ *eobj, Anglef *angle, Vec3f *pos, Vec3f *scale, const Color3f & col) {
	if(!eobj)
		return;

	ColorBGRA coll = col.toBGR(255);
	DrawEERIEObjExEx(eobj, angle, pos, scale, coll);
}
//*************************************************************************************
//routine qui gere l'alpha au vertex SEB
//*************************************************************************************
void DrawEERIEObjExEx(EERIE_3DOBJ *eobj, Anglef *angle, Vec3f *pos, Vec3f *scale, int coll)
{
	if(!eobj)
		return;

	TexturedVertex v;
	TexturedVertex rv;
	TexturedVertex vert_list[4];

	float temp;

	temp = radians(angle->getYaw());
	float Xcos = (float)EEcos(temp);
	float Xsin = (float)EEsin(temp);

	temp = radians(angle->getPitch());
	float Ycos = (float)EEcos(temp);
	float Ysin = (float)EEsin(temp);

	temp = radians(angle->getRoll());
	float Zcos = (float)EEcos(temp);
	float Zsin = (float)EEsin(temp);

	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		v.p = eobj->vertexlist[i].v * *scale;

		YRotatePoint(&v.p, &rv.p, Ycos, Ysin);
		XRotatePoint(&rv.p, &v.p, Xcos, Xsin);
		ZRotatePoint(&v.p, &rv.p, Zcos, Zsin);

		eobj->vertexlist3[i].v = (rv.p += *pos);

		Vec3f tempWorld;
		EE_RT(rv.p, tempWorld);
		EE_P(&tempWorld, &eobj->vertexlist[i].vert);
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE & face = eobj->facelist[i];

		vert_list[0] = eobj->vertexlist[face.vid[0]].vert;
		vert_list[1] = eobj->vertexlist[face.vid[1]].vert;
		vert_list[2] = eobj->vertexlist[face.vid[2]].vert;
		
		vert_list[0].uv.x = face.u[0];
		vert_list[0].uv.y = face.v[0];
		vert_list[1].uv.x = face.u[1];
		vert_list[1].uv.y = face.v[1];
		vert_list[2].uv.x = face.u[2];
		vert_list[2].uv.y = face.v[2];
		vert_list[0].color = vert_list[1].color = vert_list[2].color = coll;

		if(face.facetype == 0 || eobj->texturecontainer[face.texid] == NULL)
			GRenderer->ResetTexture(0);
		else
			GRenderer->SetTexture(0, eobj->texturecontainer[face.texid]);

		if(face.facetype & POLY_DOUBLESIDED)
			GRenderer->SetCulling(Renderer::CullNone);
		else
			GRenderer->SetCulling(Renderer::CullCW);

		EERIEDRAWPRIM(Renderer::TriangleList, vert_list);
	}
}

bool IsVertexIdxInGroup(EERIE_3DOBJ *eobj, long idx, long grs) {

	if(!eobj)
		return false;

	for(size_t i = 0; i < eobj->grouplist[grs].indexes.size(); i++) {
		long ii = eobj->grouplist[grs].indexes[i];

		if(ii == idx)
			return true;
	}

	return false;
}

extern void LoadLevelScreen();
extern void LoadLevelScreen(long lev);

extern float PROGRESS_BAR_COUNT;


struct file_truncated_exception { };

template <typename T>
const T * fts_read(const char * & data, const char * end, size_t n = 1) {
	
	size_t toread = sizeof(T) * n;
	
	if(data + toread > end) {
		LogDebug(sizeof(T) << " * " << n << " > " << (end - data));
		throw file_truncated_exception();
	}
	
	const T * result = reinterpret_cast<const T *>(data);
	
	data += toread;
	
	return result;
}


static bool loadFastScene(const res::path & file, const char * data,
                          const char * end);

template <typename T>
class scoped_malloc {
	
	T * data;
	
public:
	
	explicit scoped_malloc(T * data) : data(data) { }
	
	~scoped_malloc() { free(data); }
	
	T * get() { return data; }
	const T * get() const { return data; }
	
};

bool FastSceneLoad(const res::path & partial_path) {
	
	res::path file = "game" / partial_path / "fast.fts";
	
	const char * data = NULL, * end = NULL;
	boost::scoped_array<char> bytes;
	
	try {
		
		// Load the whole file
		LogDebug("Loading " << file);
		size_t size;
		scoped_malloc<char> dat(resources->readAlloc(file, size));
		data = dat.get(), end = dat.get() + size;
		// TODO use new[] instead of malloc so we can use (boost::)unique_ptr
		LogDebug("FTS: read " << size << " bytes");
		if(!data) {
			LogError << "FTS: could not read " << file;
			return false;
		}
		
		
		// Read the file header
		const UNIQUE_HEADER * uh = fts_read<UNIQUE_HEADER>(data, end);
		if(uh->version != FTS_VERSION) {
			LogError << "FTS version mismatch: got " << uh->version << ", expected "
			         << FTS_VERSION << " in " << file;
			return false;
		}
		PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
		
		
		// Skip .scn file list and initialize the scene data
		(void)fts_read<UNIQUE_HEADER3>(data, end, uh->count);
		InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, BKG_SIZX, BKG_SIZZ);
		PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
		
		
		// Decompress the actual scene data
		size_t input_size = end - data;
		LogDebug("FTS: decompressing " << input_size << " -> "
		                               << uh->uncompressedsize);
		bytes.reset(new char[uh->uncompressedsize]);
		if(!bytes) {
			LogError << "FTS: can't allocate buffer for uncompressed data";
			return false;
		}
		size = blastMem(data, input_size, bytes.get(), uh->uncompressedsize);
		data = bytes.get(), end = bytes.get() + size;
		if(!size) {
			LogError << "FTS: error decompressing scene data in " << file;
			return false;
		} else if(size != size_t(uh->uncompressedsize)) {
			LogWarning << "FTS: unexpected decompressed size: " << size << " < "
			           << uh->uncompressedsize << " in " << file;
		}
		PROGRESS_BAR_COUNT += 3.f, LoadLevelScreen();
		
		
	} catch(file_truncated_exception) {
		LogError << "FTS: truncated file " << file;
		return false;
	}
	
	try {
		return loadFastScene(file, data, end);
	} catch(file_truncated_exception) {
		LogError << "FTS: truncated compressed data in " << file;
		return false;
	}
}


static bool loadFastScene(const res::path & file, const char * data, const char * end) {
	
	// Read the scene header
	const FAST_SCENE_HEADER * fsh = fts_read<FAST_SCENE_HEADER>(data, end);
	if(fsh->version != FTS_VERSION) {
		LogError << "FTS: version mismatch: got " << fsh->version << ", expected "
		         << FTS_VERSION << " in " << file;
		return false;
	}
	if(fsh->sizex != ACTIVEBKG->Xsize || fsh->sizez != ACTIVEBKG->Zsize) {
		LogError << "FTS: size mismatch in FAST_SCENE_HEADER";
		return false;
	}
	player.pos = fsh->playerpos.toVec3();
	Mscenepos = fsh->Mscenepos.toVec3();
	
	
	// Load textures
	typedef std::map<s32, TextureContainer *> TextureContainerMap;
	TextureContainerMap textures;
	const FAST_TEXTURE_CONTAINER * ftc;
	ftc = fts_read<FAST_TEXTURE_CONTAINER>(data, end, fsh->nb_textures);
	for(long k = 0; k < fsh->nb_textures; k++) {
		res::path file = res::path::load(util::loadString(ftc[k].fic)).remove_ext();
		TextureContainer * tmpTC;
		tmpTC = TextureContainer::Load(file, TextureContainer::Level);
		if(tmpTC) {
			textures[ftc[k].tc] = tmpTC;
		}
	}
	PROGRESS_BAR_COUNT += 4.f, LoadLevelScreen();
	
	
	// Load cells with polygons and anchors
	LogDebug("FTS: loading " << fsh->sizex << " x " << fsh->sizez
	         << " cells ...");
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			
			const FAST_SCENE_INFO * fsi = fts_read<FAST_SCENE_INFO>(data, end);
			
			EERIE_BKG_INFO & bkg = ACTIVEBKG->Backg[i + (j * fsh->sizex)];
			
			bkg.nbianchors = (short)fsi->nbianchors;
			bkg.nbpoly = (short)fsi->nbpoly;
			
			if(fsi->nbpoly > 0) {
				bkg.polydata = (EERIEPOLY *)malloc(sizeof(EERIEPOLY) * fsi->nbpoly);
			} else {
				bkg.polydata = NULL;
			}
			
			bkg.treat = 0;
			bkg.nothing = fsi->nbpoly ? 0 : 1;
			
			bkg.frustrum_maxy = -99999999.f;
			bkg.frustrum_miny = 99999999.f;
			
			const FAST_EERIEPOLY * eps;
			eps = fts_read<FAST_EERIEPOLY>(data, end, fsi->nbpoly);
			for(long k = 0; k < fsi->nbpoly; k++) {
				
				const FAST_EERIEPOLY * ep = &eps[k];
				EERIEPOLY * ep2 = &bkg.polydata[k];
				
				memset(ep2, 0, sizeof(EERIEPOLY));
				
				ep2->room = ep->room;
				ep2->area = ep->area;
				ep2->norm = ep->norm.toVec3();
				ep2->norm2 = ep->norm2.toVec3();

				for(int i = 0; i < 4; i++)
					ep2->nrml[i] = ep->nrml[i].toVec3();

				if(ep->tex != 0) {
					TextureContainerMap::const_iterator cit = textures.find(ep->tex);
					ep2->tex = (cit != textures.end()) ? cit->second : NULL;
				} else {
					ep2->tex = NULL;
				}
				
				ep2->transval = ep->transval;
				ep2->type = PolyType::load(ep->type);
				
				for(size_t kk = 0; kk < 4; kk++) {
					ep2->v[kk].color = 0xFFFFFFFF;
					ep2->v[kk].rhw = 1;
					ep2->v[kk].specular = 1;
					ep2->v[kk].p.x = ep->v[kk].ssx;
					ep2->v[kk].p.y = ep->v[kk].sy;
					ep2->v[kk].p.z = ep->v[kk].ssz;
					ep2->v[kk].uv.x = ep->v[kk].stu;
					ep2->v[kk].uv.y = ep->v[kk].stv;
				}
				
				memcpy(ep2->tv, ep2->v, sizeof(TexturedVertex) * 4);
				
				for(size_t kk = 0; kk < 4; kk++) {
					ep2->tv[kk].color = 0xFF000000;
				}
				
				long to = (ep->type & POLY_QUAD) ? 4 : 3;
				float div = 1.f / to;
				
				ep2->center = Vec3f_ZERO;
				for(long h = 0; h < to; h++) {
					ep2->center += ep2->v[h].p;
					if(h != 0) {
						ep2->max = glm::max(ep2->max, ep2->v[h].p);
						ep2->min = glm::min(ep2->min, ep2->v[h].p);
					} else {
						ep2->min = ep2->max = ep2->v[0].p;
					}
				}
				ep2->center *= div;
				
				float dist = 0.f;
				for(int h = 0; h < to; h++) {
					float x = ep2->v[h].p.x - ep2->center.x;
					float y = ep2->v[h].p.y - ep2->center.y;
					float z = ep2->v[h].p.z - ep2->center.z;
					float d = sqrt((x * x) + (y * y) + (z * z));
					dist = max(dist, d);
				}
				ep2->v[0].rhw = dist;
				
				DeclareEGInfo(ep2->center.x, ep2->center.z);
				DeclareEGInfo(ep2->v[0].p.x, ep2->v[0].p.z);
				DeclareEGInfo(ep2->v[1].p.x, ep2->v[1].p.z);
				DeclareEGInfo(ep2->v[2].p.x, ep2->v[2].p.z);
				if(ep->type & POLY_QUAD) {
					DeclareEGInfo(ep2->v[3].p.x, ep2->v[3].p.z);
				}
			}
			
			if(fsi->nbianchors <= 0) {
				bkg.ianchors = NULL;
			} else {
				bkg.ianchors = (long *)malloc(sizeof(long) * fsi->nbianchors);
				const s32 * anchors = fts_read<s32>(data, end, fsi->nbianchors);
				std::copy(anchors, anchors + fsi->nbianchors, bkg.ianchors);
			}
			
		}
	}
	PROGRESS_BAR_COUNT += 4.f, LoadLevelScreen();
	
	
	// Load anchor links
	LogDebug("FTS: loading " << fsh->nb_anchors << " anchors ...");
	ACTIVEBKG->nbanchors = fsh->nb_anchors;
	if(fsh->nb_anchors > 0) {
		size_t anchorsize = sizeof(ANCHOR_DATA) * fsh->nb_anchors;
		ACTIVEBKG->anchors = (ANCHOR_DATA *)malloc(anchorsize);
		memset(ACTIVEBKG->anchors, 0, anchorsize);
	} else {
		ACTIVEBKG->anchors = NULL;
	}
	for(long i = 0; i < fsh->nb_anchors; i++) {
		
		const FAST_ANCHOR_DATA * fad = fts_read<FAST_ANCHOR_DATA>(data, end);
		
		ANCHOR_DATA & anchor = ACTIVEBKG->anchors[i];
		anchor.flags = AnchorFlags::load(fad->flags); // TODO save/load flags
		anchor.pos = fad->pos.toVec3();
		anchor.nblinked = fad->nb_linked;
		anchor.height = fad->height;
		anchor.radius = fad->radius;
		
		if(fad->nb_linked <= 0) {
			anchor.linked = NULL;
		} else {
			anchor.linked = (long *)malloc(sizeof(long) * fad->nb_linked);
			const s32 * links = fts_read<s32>(data, end, fad->nb_linked);
			std::copy(links, links + fad->nb_linked, anchor.linked);
		}
	}
	PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
	
	
	// Load rooms and portals
	if(fsh->nb_rooms <= 0) {
		USE_PORTALS = false;
	} else {
		
		EERIE_PORTAL_Release();
		
		portals = new EERIE_PORTAL_DATA;
		portals->nb_rooms = fsh->nb_rooms;
		portals->room = (EERIE_ROOM_DATA *)malloc(sizeof(EERIE_ROOM_DATA)
												  * (portals->roomsize()));

		portals->portals.resize(fsh->nb_portals);
		
		LogDebug("FTS: loading " << portals->portals.size() << " portals ...");
		const EERIE_SAVE_PORTALS * epos;
		epos = fts_read<EERIE_SAVE_PORTALS>(data, end, portals->portals.size());
		for(size_t i = 0; i < portals->portals.size(); i++) {
			
			const EERIE_SAVE_PORTALS * epo = &epos[i];
			EERIE_PORTALS & portal = portals->portals[i];
			
			memset(&portal, 0, sizeof(EERIE_PORTALS));
			
			portal.room_1 = epo->room_1;
			portal.room_2 = epo->room_2;
			portal.useportal = epo->useportal;
			portal.paddy = epo->paddy;
			portal.poly.area = epo->poly.area;
			portal.poly.type = PolyType::load(epo->poly.type);
			portal.poly.transval = epo->poly.transval;
			portal.poly.room = epo->poly.room;
			portal.poly.misc = epo->poly.misc;
			portal.poly.center = epo->poly.center.toVec3();
			portal.poly.max = epo->poly.max.toVec3();
			portal.poly.min = epo->poly.min.toVec3();
			portal.poly.norm = epo->poly.norm.toVec3();
			portal.poly.norm2 = epo->poly.norm2.toVec3();
			
			for(int i = 0; i < 4; i++)
				portal.poly.nrml[i] = epo->poly.nrml[i].toVec3();

			std::copy(epo->poly.v, epo->poly.v + 4, portal.poly.v);
			std::copy(epo->poly.tv, epo->poly.tv + 4, portal.poly.tv);
		}
		
		
		LogDebug("FTS: loading " << portals->roomsize() << " rooms ...");
		for(long i = 0; i < portals->roomsize(); i++) {
			
			const EERIE_SAVE_ROOM_DATA * erd;
			erd = fts_read<EERIE_SAVE_ROOM_DATA>(data, end);
			
			EERIE_ROOM_DATA & room = portals->room[i];
			
			memset(&room, 0, sizeof(EERIE_ROOM_DATA));
			room.nb_portals = erd->nb_portals;
			room.nb_polys = erd->nb_polys;
			
			LogDebug(" - room " << i << ": " << room.nb_portals << " portals, "
			         << room.nb_polys << " polygons");
			
			if(room.nb_portals) {
				room.portals = (long *)malloc(sizeof(long) * room.nb_portals);
				const s32 * start = fts_read<s32>(data, end, room.nb_portals);
				std::copy(start, start + room.nb_portals, room.portals);
			} else {
				room.portals = NULL;
			}
			
			if(room.nb_polys) {
				room.epdata = (EP_DATA *)malloc(sizeof(EP_DATA) * room.nb_polys);
				const FAST_EP_DATA * ed;
				ed = fts_read<FAST_EP_DATA>(data, end, room.nb_polys);
				std::copy(ed, ed + room.nb_polys, room.epdata);
			} else {
				portals->room[i].epdata = NULL;
			}
			
		}
		
		USE_PORTALS = true;
	}
	
	
	// Load distances between rooms
	free(RoomDistance), RoomDistance = NULL;
	NbRoomDistance = 0;
	if(portals) {
		NbRoomDistance = portals->roomsize();
		RoomDistance = (ROOM_DIST_DATA *)malloc(sizeof(ROOM_DIST_DATA)
		                                        * NbRoomDistance * NbRoomDistance);
		LogDebug("FTS: loading " << (NbRoomDistance * NbRoomDistance)
		         << " room distances ...");
		for(long n = 0; n < NbRoomDistance; n++) {
			for(long m = 0; m < NbRoomDistance; m++) {
				const ROOM_DIST_DATA_SAVE * rdds;
				rdds = fts_read<ROOM_DIST_DATA_SAVE>(data, end);
				Vec3f start = rdds->startpos.toVec3();
				Vec3f end = rdds->endpos.toVec3();
				SetRoomDistance(m, n, rdds->distance, &start, &end);
			}
		}
	}
	PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
	
	
	// Prepare the loaded data
	
	LogDebug("FTS: preparing scene data ...");
	
	EERIEPOLY_Compute_PolyIn();
	PROGRESS_BAR_COUNT += 3.f, LoadLevelScreen();
	
	EERIE_PATHFINDER_Create();
	EERIE_PORTAL_Blend_Portals_And_Rooms();
	PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
	
	ComputePortalVertexBuffer();
	PROGRESS_BAR_COUNT += 1.f, LoadLevelScreen();
	
	
	if(data != end) {
		LogWarning << "FTS: ignoring " << (end - data) << " bytes at the end of "
		           << file;
	}
	LogDebug("FTS: done loading");
	
	return true;
}

void EERIEPOLY_FillMissingVertex(EERIEPOLY * po, EERIEPOLY * ep)
{
	long missing = -1;

	for(long i = 0; i < 3; i++) {
		long same = 0;

		for(long j = 0; j < 3; j++) {
			if((po->v[j].p.x == ep->v[i].p.x)
				&& (po->v[j].p.y == ep->v[i].p.y)
				&& (po->v[j].p.z == ep->v[i].p.z))
				same = 1;
		}

		if(!same)
			missing = i;
	}
	
	if(missing >= 0) {
		Vec3f temp = po->v[2].p;
		po->v[2].p = ep->v[missing].p;
		po->v[3].p = temp;
		po->type |= POLY_QUAD;
	}
}

#if BUILD_EDIT_LOADSAVE

void ComputeRoomDistance() {
	
	free(RoomDistance), RoomDistance = NULL;
	NbRoomDistance = 0;
	
	if(!portals)
		return;
	
	NbRoomDistance = portals->roomsize();
	RoomDistance =
		(ROOM_DIST_DATA *)malloc(sizeof(ROOM_DIST_DATA) * (NbRoomDistance) * (NbRoomDistance));

	for (long n = 0; n < NbRoomDistance; n++)
		for (long m = 0; m < NbRoomDistance; m++)
			SetRoomDistance(m, n, -1.f, NULL, NULL);

	long nb_anchors = NbRoomDistance + (portals->portals.size() * 9);
	ANCHOR_DATA * ad = (ANCHOR_DATA *)malloc(sizeof(ANCHOR_DATA) * nb_anchors);

	memset(ad, 0, sizeof(ANCHOR_DATA)*nb_anchors);

	void ** ptr = NULL;
	ptr = (void **)malloc(sizeof(*ptr) * nb_anchors);
	memset(ptr, 0, sizeof(*ptr)*nb_anchors);


	for(long i = 0; i < NbRoomDistance; i++) {
		GetRoomCenter(i, &ad[i].pos);
		ptr[i] = (void *)&portals->room[i];
	}

	long curpos = NbRoomDistance;

	for(size_t i = 0; i < portals->portals.size(); i++) {
		// Add 4 portal vertices
		for(int nn = 0; nn < 4; nn++) {
			ad[curpos].pos = portals->portals[i].poly.v[nn].p;
			ptr[curpos] = (void *)&portals->portals[i];
			curpos++;
		}

		// Add center;
		ad[curpos].pos = portals->portals[i].poly.center;
		ptr[curpos] = (void *)&portals->portals[i];
		curpos++;

		// Add V centers;
		for(int nn = 0, nk = 3; nn < 4; nk = nn++) {
			ad[curpos].pos = (portals->portals[i].poly.v[nn].p
			                + portals->portals[i].poly.v[nk].p) * 0.5f;
			ptr[curpos] = (void *)&portals->portals[i];
			curpos++;
		}
	}

	// Link Room Centers to all its Room portals...
	for(int i = 0; i < portals->roomsize(); i++) {
		for(size_t j = 0; j < portals->portals.size(); j++) {
			if(portals->portals[j].room_1 == i || portals->portals[j].room_2 == i) {
				for(long tt = 0; tt < nb_anchors; tt++) {

					if(ptr[tt] == (void *)(&portals->portals[j])) {
						AddAData(&ad[tt], i);
						AddAData(&ad[i], tt);
					}
				}
			}
		}
	}

	// Link All portals of a room to all other portals of that room
	for(int i = 0; i < portals->roomsize(); i++) {
		for(size_t j = 0; j < portals->portals.size(); j++) {
			if(portals->portals[j].room_1 == i || portals->portals[j].room_2 == i) {
				for(size_t jj = 0; jj < portals->portals.size(); jj++) {
					if(jj != j && (portals->portals[jj].room_1 == i || portals->portals[jj].room_2 == i))
					{
						long p1 = -1;
						long p2 = -1;

						for(long tt = 0; tt < nb_anchors; tt++) {
							if(ptr[tt] == (void *)(&portals->portals[jj]))
								p1 = tt;

							if(ptr[tt] == (void *)(&portals->portals[j]))
								p2 = tt;
						}

						if(p1 >= 0 && p2 >= 0) {
							AddAData(&ad[p1], p2);
							AddAData(&ad[p2], p1);
						}
					}
				}
			}
		}
	}

	PathFinder pathfinder(NbRoomDistance, ad, 0, NULL);

	for(int i = 0; i < NbRoomDistance; i++) {
		for(long j = 0; j < NbRoomDistance; j++) {
			if(i == j) {
				SetRoomDistance(i, j, -1, NULL, NULL);
				continue;
			}
			
			PathFinder::Result rl;

			bool found = pathfinder.move(i, j, rl);

			if(found) {
				float d = 0.f;

				for(size_t id = 1; id < rl.size() - 1; id++) {
					d += glm::distance(ad[rl[id - 1]].pos, ad[rl[id]].pos);
				}

				if(d < 0.f)
					d = 0.f;

				float old = GetRoomDistance(i, j, NULL, NULL);

				if((d < old || old < 0.f) && rl.size() >= 2)
					SetRoomDistance(i, j, d, &ad[rl[1]].pos, &ad[rl[rl.size()-2]].pos);
			}
		}
	}

	// Don't use this for contiguous rooms !
	for(size_t i = 0; i < portals->portals.size(); i++) {
		SetRoomDistance(portals->portals[i].room_1, portals->portals[i].room_2, -1, NULL, NULL);
		SetRoomDistance(portals->portals[i].room_2, portals->portals[i].room_1, -1, NULL, NULL);
	}

	// Release our temporary Pathfinder data
	for(int ii = 0; ii < nb_anchors; ii++) {
		if(ad[ii].nblinked) {
			free(ad[ii].linked);
		}
	}

	free(ad);
	free(ptr);
}

static void EERIE_PORTAL_Room_Poly_Add(EERIEPOLY * ep, long nr, long px, long py, long idx) {
	
	portals->room[nr].epdata = (EP_DATA *)realloc(portals->room[nr].epdata, sizeof(EP_DATA) * (portals->room[nr].nb_polys + 1));
	portals->room[nr].epdata[portals->room[nr].nb_polys].idx = checked_range_cast<short>(idx);
	portals->room[nr].epdata[portals->room[nr].nb_polys].px = checked_range_cast<short>(px);
	portals->room[nr].epdata[portals->room[nr].nb_polys].py = checked_range_cast<short>(py);
	ep->room = checked_range_cast<short>(nr);
	portals->room[nr].nb_polys++;
}

static void EERIE_PORTAL_Poly_Add(EERIEPOLY * ep, const std::string& name, long px, long py, long idx) {
	
	long type, val1, val2;

	if(!GetNameInfo(name, type, val1, val2))
		return;

	if(portals == NULL) {
		portals = new EERIE_PORTAL_DATA;

		if(!portals)
			return;

		portals->nb_rooms = 0;
		portals->room = NULL;
		USE_PORTALS = true;
	}

	if(type == TYPE_PORTAL) {
		EERIE_PORTALS portal;

		portal.room_1 = val1;
		portal.room_2 = val2;
		memcpy(&portal.poly, ep, sizeof(EERIEPOLY));

		float fDistMin = std::numeric_limits<float>::max();
		float fDistMax = std::numeric_limits<float>::min();
		int nbvert = (ep->type & POLY_QUAD) ? 4 : 3;
		
		ep->center = ep->v[0].p;
		for(long ii = 1; ii < nbvert; ii++) {
			ep->center += ep->v[ii].p;
		}
		
		ep->center /= nbvert;

		for(int ii = 0; ii < nbvert; ii++) {
			float fDist = glm::distance(ep->center, ep->v[ii].p);
			fDistMin = min(fDistMin, fDist);
			fDistMax = max(fDistMax, fDist);
		}

		fDistMin = (fDistMax + fDistMin) * .5f;
		portal.poly.v[0].rhw = fDistMin;

		portals->portals.push_back(portal);
	} else if(type == TYPE_ROOM) {
		if(val1 > portals->nb_rooms) {
			portals->room = (EERIE_ROOM_DATA *)realloc(portals->room, sizeof(EERIE_ROOM_DATA) * (val1 + 1));

			if(portals->nb_rooms == 0) {
				memset(portals->room, 0, sizeof(EERIE_ROOM_DATA)*(val1 + 1));
			} else {
				for(long i = portals->nb_rooms + 1; i <= val1; i++) {
					memset(&portals->room[i], 0, sizeof(EERIE_ROOM_DATA));
				}
			}

			portals->nb_rooms = val1;
		}

		EERIE_PORTAL_Room_Poly_Add(ep, val1, px, py, idx);
	}
}

static int BkgAddPoly(EERIEPOLY * ep, EERIE_3DOBJ * eobj) {
	
	long type = -1;
	long val1 = -1;

	if(TryToQuadify(ep, eobj))
		return 0;

	float cx = (ep->v[0].p.x + ep->v[1].p.x + ep->v[2].p.x);
	float cy = (ep->v[0].p.y + ep->v[1].p.y + ep->v[2].p.y);
	float cz = (ep->v[0].p.z + ep->v[1].p.z + ep->v[2].p.z);
	long posx = (long)(float)(cx * ( 1.0f / 3 ) * ACTIVEBKG->Xmul);
	long posz = (long)(float)(cz * ( 1.0f / 3 ) * ACTIVEBKG->Zmul);
	long posy = (long)(float)(cy * ( 1.0f / 3 ) * ACTIVEBKG->Xmul + ACTIVEBKG->Xsize * .5f);

	if (posy < 0) return 0;
	else if (posy >= ACTIVEBKG->Xsize) return 0;

	if (posx < 0) return 0;
	else if (posx >= ACTIVEBKG->Xsize) return 0;

	if (posz < 0) return 0;
	else if (posz >= ACTIVEBKG->Zsize) return 0;

	EERIE_BKG_INFO *eg = &ACTIVEBKG->Backg[posx+posz*ACTIVEBKG->Xsize];

	DeclareEGInfo(cx * ( 1.0f / 3 ), cz * ( 1.0f / 3 ));
	DeclareEGInfo(ep->v[0].p.x, ep->v[0].p.z);
	DeclareEGInfo(ep->v[1].p.x, ep->v[1].p.z);
	DeclareEGInfo(ep->v[2].p.x, ep->v[2].p.z);

	cx *= ( 1.0f / 3 );
	cy *= ( 1.0f / 3 );
	cz *= ( 1.0f / 3 );
	long t = (((eg->nbpoly) >> 1) << 1) + 2; 
	long tt = (((eg->nbpoly - 1) >> 1) << 1) + 2; 

	if(!eg->polydata) {
		eg->polydata = (EERIEPOLY *)malloc(sizeof(EERIEPOLY) * t);
	} else if(tt != t) {
		eg->polydata = (EERIEPOLY *)realloc(eg->polydata, sizeof(EERIEPOLY) * t);
	}

	memcpy(&eg->polydata[eg->nbpoly], ep, sizeof(EERIEPOLY));
	
	EERIEPOLY * epp = &eg->polydata[eg->nbpoly];
	
	for(long j = 0; j < 3; j++) {
		epp->tv[j].uv = epp->v[j].uv;
		epp->tv[j].color = epp->v[j].color;
		epp->tv[j].rhw = 1.f;
	}
	
	epp->center.x = cx; 
	epp->center.y = cy; 
	epp->center.z = cz; 
	epp->max.x = max(epp->v[0].p.x, epp->v[1].p.x);
	epp->max.x = max(epp->max.x, epp->v[2].p.x);
	epp->min.x = min(epp->v[0].p.x, epp->v[1].p.x);
	epp->min.x = min(epp->min.x, epp->v[2].p.x);

	epp->max.y = max(epp->v[0].p.y, epp->v[1].p.y);
	epp->max.y = max(epp->max.y, epp->v[2].p.y);
	epp->min.y = min(epp->v[0].p.y, epp->v[1].p.y);
	epp->min.y = min(epp->min.y, epp->v[2].p.y);

	epp->max.z = max(epp->v[0].p.z, epp->v[1].p.z);
	epp->max.z = max(epp->max.z, epp->v[2].p.z);
	epp->min.z = min(epp->v[0].p.z, epp->v[1].p.z);
	epp->min.z = min(epp->min.z, epp->v[2].p.z);
	epp->type = ep->type;
	epp->type &= ~POLY_QUAD;

	CalcFaceNormal(epp, epp->v);
	epp->area = fdist((epp->v[0].p + epp->v[1].p) * .5f, epp->v[2].p)
	            * fdist(epp->v[0].p, epp->v[1].p) * .5f;
	
	if (type == TYPE_ROOM)
		epp->room = checked_range_cast<short>(val1);
	else
		epp->room = -1;

	eg->nbpoly++;

	eg->treat = 0;

	if(ep && ep->tex && !ep->tex->m_texName.empty()) {
				if ( ep->tex->m_texName.string().find("stone") != std::string::npos )         ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.string().find("pierre") != std::string::npos )   ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.string().find("wood") != std::string::npos )     ep->type |= POLY_WOOD;
				else if ( ep->tex->m_texName.string().find("bois") != std::string::npos )     ep->type |= POLY_STONE;
				else if ( ep->tex->m_texName.string().find("gavier") != std::string::npos )   ep->type |= POLY_GRAVEL;
				else if ( ep->tex->m_texName.string().find("earth") != std::string::npos )    ep->type |= POLY_EARTH;
	}

	EERIE_PORTAL_Poly_Add(epp, eobj->name, posx, posz, eg->nbpoly - 1);
	return 1;
}

static void EERIEAddPolyToBackground(TexturedVertex * vert2, TextureContainer * tex, PolyType render, float transval, EERIE_3DOBJ * eobj) {
	
	EERIEPOLY ep;
	
	memset(ep.tv, 0, sizeof(TexturedVertex) * 3);
	
	if(vert2) {
		memcpy(ep.v, vert2, sizeof(TexturedVertex) * 3);
	} else {
		memset(ep.tv, 0, sizeof(TexturedVertex) * 3);
	}
	
	ep.type = render;
	ep.tex = tex;
	ep.transval = transval;
	BkgAddPoly(&ep, eobj);
}

static void SceneAddObjToBackground(EERIE_3DOBJ * eobj) {
	
	float       Xcos, Ycos, Zcos, Xsin, Ysin, Zsin;

	TexturedVertex vlist[3];
	float tempAngle;
	tempAngle = radians(eobj->angle.getYaw());
	Xcos = (float)EEcos(tempAngle);
	Xsin = (float)EEsin(tempAngle);
	tempAngle = radians(eobj->angle.getPitch());
	Ycos = (float)EEcos(tempAngle);
	Ysin = (float)EEsin(tempAngle);
	tempAngle = radians(eobj->angle.getRoll());
	Zcos = (float)EEcos(tempAngle);
	Zsin = (float)EEsin(tempAngle);

	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		//Local Transform
		Vec3f p = eobj->vertexlist[i].v - eobj->point0;
		Vec3f rp;
		YRotatePoint(&p, &rp, Ycos, Ysin);
		XRotatePoint(&rp, &p, Xcos, Xsin);
		ZRotatePoint(&p, &rp, Zcos, Zsin);
		eobj->vertexlist[i].vert.p = rp + eobj->pos + eobj->point0;
	}

	long type, val1, val2;

	if(GetNameInfo(eobj->name, type, val1, val2)) {
		if(type == TYPE_PORTAL) {
			EERIEPOLY ep;
			EERIEPOLY epp;

			for(size_t i = 0; i < eobj->facelist.size(); i++) {
				for(long kk = 0; kk < 3; kk++) {
					memcpy(&ep.v[kk], &eobj->vertexlist[eobj->facelist[i].vid[kk]].vert, sizeof(TexturedVertex));
				}

				if(i == 0) {
					memcpy(&epp, &ep, sizeof(EERIEPOLY));
					epp.type = 0;
				}
				else if(i == 1)
					EERIEPOLY_FillMissingVertex(&epp, &ep);
				else
					break;
			}

			if(!eobj->facelist.empty())
				EERIE_PORTAL_Poly_Add(&epp, eobj->name, -1, -1, -1);

			return;
		}
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		vlist[0] = eobj->vertexlist[eobj->facelist[i].vid[0]].vert;
		vlist[1] = eobj->vertexlist[eobj->facelist[i].vid[1]].vert;
		vlist[2] = eobj->vertexlist[eobj->facelist[i].vid[2]].vert;

		vlist[0].color = vlist[1].color = vlist[2].color = Color::white.toBGR();

		TextureContainer *tex = NULL;
		bool addToBackground = true;
		if(eobj->facelist[i].facetype & POLY_NO_SHADOW) {
			vlist[0].uv.x = eobj->facelist[i].u[0];
			vlist[0].uv.y = eobj->facelist[i].v[0];
			vlist[1].uv.x = eobj->facelist[i].u[1];
			vlist[1].uv.y = eobj->facelist[i].v[1];
			vlist[2].uv.x = eobj->facelist[i].u[2];
			vlist[2].uv.y = eobj->facelist[i].v[2];

			if(eobj->facelist[i].texid >= 0)
				tex = eobj->texturecontainer[eobj->facelist[i].texid];
			else
				addToBackground = false;
		}

		if(addToBackground)
			EERIEAddPolyToBackground(vlist, tex, eobj->facelist[i].facetype, eobj->facelist[i].transval, eobj);
	}
}

/*!
 * Save the currently loaded scene.
 * @param partal_path Where to save the scene to.
 */
static bool FastSceneSave(const fs::path & partial_path) {
	
	fs::path path = "game" / partial_path;
	
	LogDebug("FastSceneSave" << path);
	
	if(!fs::create_directories(path)) {
		return false;
	}
	
	size_t allocsize = (256) * 60 + 1000000 + sizeof(FAST_SCENE_HEADER)
	                   + sizeof(FAST_TEXTURE_CONTAINER) * 1000
	                   + sizeof(FAST_ANCHOR_DATA) * ACTIVEBKG->nbanchors * 2;
	
	if(portals) {
		
		for(size_t i = 0; i < portals->portals.size() + 1; i++) {
			allocsize += sizeof(EERIE_SAVE_PORTALS);
		}
		
		for(long i = 0; i < portals->roomsize(); i++) {
			allocsize += sizeof(EERIE_SAVE_ROOM_DATA);
			allocsize += sizeof(s32) * portals->room[i].nb_portals;
			allocsize += sizeof(FAST_EP_DATA) * portals->room[i].nb_polys;
		}
		
		allocsize += sizeof(ROOM_DIST_DATA_SAVE) * (portals->roomsize()) * (portals->roomsize());
	}
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		allocsize += ACTIVEBKG->anchors[i].nblinked * sizeof(s32);
	}
	
	for(long j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			allocsize += sizeof(FAST_SCENE_INFO);
			allocsize += ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize].nbpoly * (sizeof(EERIEPOLY) + 1);
		}
	}
	
	
	size_t pos = 0;
	char * dat = new char[allocsize];
	
	memset(dat, 0, allocsize);
	UNIQUE_HEADER * uh = reinterpret_cast<UNIQUE_HEADER *>(dat);
	pos += sizeof(UNIQUE_HEADER);
	strncpy(uh->path, path.string().c_str(), sizeof(uh->path));
	uh->version = FTS_VERSION;
	
	long count = 0;
	
	for(fs::directory_iterator it(partial_path); !it.end(); ++it) {
		
		fs::path path = partial_path / it.name();
		
		if(!path.has_ext("scn") || !it.is_regular_file()) {
			continue;
		}
		
		UNIQUE_HEADER2 * uh2 = reinterpret_cast<UNIQUE_HEADER2 *>(dat + pos);
		pos += sizeof(UNIQUE_HEADER2);
		strncpy(uh2->path, path.filename().c_str(), sizeof(uh2->path));
		
		char check[512];
		HERMES_CreateFileCheck(path, check, 512, FTS_VERSION);
		
		memcpy(dat + pos, check, 512);
		pos += 512;
		
		count++;
		
		if(count > 60) {
			delete[] dat;
			return false;
		}
	}
	
	uh->count = count;
	fs::path file = path / "fast.fts";
	long compressedstart = pos;
	
	FAST_SCENE_HEADER * fsh = reinterpret_cast<FAST_SCENE_HEADER *>(dat + pos);
	pos += sizeof(FAST_SCENE_HEADER);
	
	fsh->version = FTS_VERSION;
	fsh->sizex = ACTIVEBKG->Xsize;
	fsh->sizez = ACTIVEBKG->Zsize;
	fsh->nb_textures = 0;
	fsh->playerpos = player.pos;
	fsh->Mscenepos = Mscenepos;
	fsh->nb_anchors = ACTIVEBKG->nbanchors;
	fsh->nb_portals = 0;
	fsh->nb_rooms = 0;
	
	if(portals) {
		fsh->nb_portals = portals->portals.size();
		fsh->nb_rooms = portals->nb_rooms;
	}
	
	fsh->nb_polys = 0;
	
	//Count textures...
	typedef std::map<TextureContainer *, s32> TextureContainerMap;
	TextureContainerMap textures;
	s32 texid = 0;
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			for(long k = 0; k < ACTIVEBKG->Backg[i+j*fsh->sizex].nbpoly; k++) {
				
				EERIEPOLY * ep = &ACTIVEBKG->Backg[i+j*fsh->sizex].polydata[k];
				
				if(ep && ep->tex) {
					
					if(textures.find(ep->tex) == textures.end()) {
						textures[ep->tex] = ++texid;
						
						FAST_TEXTURE_CONTAINER * ftc = reinterpret_cast<FAST_TEXTURE_CONTAINER *>(dat + pos);
						pos += sizeof(FAST_TEXTURE_CONTAINER);
						ftc->tc = texid;
						strncpy(ftc->fic, ep->tex->m_texName.string().c_str(), sizeof(ftc->fic));
						ftc->temp = 0;
						fsh->nb_textures++;
						
						if(pos >= allocsize - 100000) {
							delete[] dat;
							return false;
						}
					}
				}
			}
		}
	}
	
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			FAST_SCENE_INFO * fsi = reinterpret_cast<FAST_SCENE_INFO *>(dat + pos);
			pos += sizeof(FAST_SCENE_INFO);
			
			if(pos >= allocsize - 100000) {
				delete[] dat;
				return false;
			}
			
			fsi->nbianchors = ACTIVEBKG->Backg[i+j*fsh->sizex].nbianchors;
			fsi->nbpoly = ACTIVEBKG->Backg[i+j*fsh->sizex].nbpoly;
			
			for(long k = 0; k < fsi->nbpoly; k++) {
				fsh->nb_polys++;
				
				FAST_EERIEPOLY * ep = reinterpret_cast<FAST_EERIEPOLY *>(dat + pos);
				pos += sizeof(FAST_EERIEPOLY);
				EERIEPOLY * ep2 = &ACTIVEBKG->Backg[i+j*fsh->sizex].polydata[k];
				
				if(pos >= allocsize - 100000) {
					delete[] dat;
					return false;
				}
				
				ep->room = ep2->room;
				ep->paddy = 0;
				ep->area = ep2->area;
				ep->norm = ep2->norm;
				ep->norm2 = ep2->norm2;
				std::copy(ep2->nrml, ep2->nrml + 4, ep->nrml);
				ep->tex = textures[ep2->tex];
				ep->transval = ep2->transval;
				ep->type = ep2->type;
				
				for(long kk = 0; kk < 4; kk++) {
					ep->v[kk].ssx = ep2->v[kk].p.x;
					ep->v[kk].sy = ep2->v[kk].p.y;
					ep->v[kk].ssz = ep2->v[kk].p.z;
					ep->v[kk].stu = ep2->v[kk].uv.x;
					ep->v[kk].stv = ep2->v[kk].uv.y;
				}
			}
			
			for(long k = 0; k < fsi->nbianchors; k++) {
				s32 * ianch = (s32 *)(dat + pos);
				pos += sizeof(s32);
				if(pos >= allocsize - 100000) {
					delete[] dat;
					return false;
				}
				*ianch = ACTIVEBKG->Backg[i+j*fsh->sizex].ianchors[k];
			}
		}
	}
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		
		FAST_ANCHOR_DATA * fad = reinterpret_cast<FAST_ANCHOR_DATA *>(dat + pos);
		pos += sizeof(FAST_ANCHOR_DATA);
		
		if(pos >= allocsize - 100000) {
			delete[] dat;
			return false;
		}
		
		fad->flags = ACTIVEBKG->anchors[i].flags;
		fad->pos = ACTIVEBKG->anchors[i].pos;
		fad->nb_linked = ACTIVEBKG->anchors[i].nblinked;
		fad->radius = ACTIVEBKG->anchors[i].radius;
		fad->height = ACTIVEBKG->anchors[i].height;
		
		for(long kk = 0; kk < fad->nb_linked; kk++) {
			s32 * lng = reinterpret_cast<s32 *>(dat + pos);
			pos += sizeof(s32);
			if(pos >= allocsize - 100000) {
				delete[] dat;
				return false;
			}
			*lng = ACTIVEBKG->anchors[i].linked[kk];
		}
	}
	
	if(portals) {
		
		for(size_t i = 0; i < portals->portals.size(); i++) {
			
			EERIE_SAVE_PORTALS * epo = reinterpret_cast<EERIE_SAVE_PORTALS *>(dat + pos);
			pos += sizeof(EERIE_SAVE_PORTALS);
			
			const EERIE_PORTALS & portal = portals->portals[i];
			
			epo->room_1 = portal.room_1;
			epo->room_2 = portal.room_2;
			epo->useportal = portal.useportal;
			epo->paddy = portal.paddy;
			epo->poly.area = portal.poly.area;
			epo->poly.type = portal.poly.type;
			epo->poly.transval = portal.poly.transval;
			epo->poly.room = portal.poly.room;
			epo->poly.misc = portal.poly.misc;
			epo->poly.center = portal.poly.center;
			epo->poly.max = portal.poly.max;
			epo->poly.min = portal.poly.min;
			epo->poly.norm = portal.poly.norm;
			epo->poly.norm2 = portal.poly.norm2;
			
			copy(portal.poly.nrml, portal.poly.nrml + 4, epo->poly.nrml);
			copy(portal.poly.v, portal.poly.v + 4, epo->poly.v);
			copy(portal.poly.tv, portal.poly.tv + 4, epo->poly.tv);
		}
		
		for(long i = 0; i < portals->roomsize(); i++) {
			
			EERIE_SAVE_ROOM_DATA * erd = reinterpret_cast<EERIE_SAVE_ROOM_DATA *>(dat + pos);
			pos += sizeof(EERIE_SAVE_ROOM_DATA);
			
			memset(erd, 0, sizeof(EERIE_SAVE_ROOM_DATA));
			erd->nb_polys = portals->room[i].nb_polys;
			erd->nb_portals = portals->room[i].nb_portals;
			
			for(long jj = 0; jj < portals->room[i].nb_portals; jj++) {
				s32 * lng = reinterpret_cast<s32 *>(dat + pos);
				pos += sizeof(s32);
				*lng = portals->room[i].portals[jj];
			}
			
			if(portals->room[i].nb_polys) {
				EP_DATA * ed = reinterpret_cast<EP_DATA *>(dat + pos);
				pos += sizeof(EP_DATA) * portals->room[i].nb_polys;
				memcpy(ed, portals->room[i].epdata, sizeof(EP_DATA)*portals->room[i].nb_polys);
			}
		}
	}
	
	if(!RoomDistance) {
		ComputeRoomDistance();
	}
	
	if(portals && RoomDistance && NbRoomDistance > 0) {
		for(long n = 0; n < NbRoomDistance; n++) {
			for(long m = 0; m < NbRoomDistance; m++) {
				ROOM_DIST_DATA_SAVE * rdds = reinterpret_cast<ROOM_DIST_DATA_SAVE *>(dat + pos);
				pos += sizeof(ROOM_DIST_DATA_SAVE);
				Vec3f start;
				Vec3f end;
				rdds->distance = GetRoomDistance(m, n, &start, &end);
				rdds->startpos = start;
				rdds->endpos = end;
			}
		}
	}
	
	// Now Saving Whole Buffer
	uh->uncompressedsize = pos - compressedstart;
	
	fs::ofstream ofs(file, fs::fstream::out | fs::fstream::binary | fs::fstream::trunc);
	if(!ofs.is_open()) {
		delete[] dat;
		return false;
	}
	
	if(ofs.write(dat, compressedstart).fail()) {
		delete[] dat;
		return false;
	}
	
	size_t compressedSize;
	char * compressed = implodeAlloc(dat + compressedstart, pos - compressedstart, compressedSize);
	delete[] dat;
	if(!compressed) {
		LogError << "Error compressing scene";
		return false;
	}
	
	ofs.write(compressed, compressedSize);
	
	delete[] compressed;
	
	return !ofs.fail();
}

void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms) {
	
	res::path ftemp = LastLoadedScene.parent();
	
	// First Release Any Portal Data
	EERIE_PORTAL_Release();
	
	// Try to Load Fast Scene
	if(!FastSceneLoad(ftemp)) {
		
		//failure: Not-Fast Load
		
		for(long j = 0; j < ms->nb_scenes; j++) {
			EERIE_3DSCENE * escn = ms->scenes[j];
			for(long i = 0; i < escn->nbobj; i++) {
				escn->objs[i]->pos += ms->pos;
				SceneAddObjToBackground(escn->objs[i]);
			}
		}
		
		EERIE_LIGHT_MoveAll(&ms->pos);
		ARX_PrepareBackgroundNRMLs();
		EERIEPOLY_Compute_PolyIn();
		EERIE_PORTAL_Blend_Portals_And_Rooms();
		
		AnchorData_Create(ACTIVEBKG);
		
		FastSceneSave(ftemp.string());
		ComputePortalVertexBuffer();
		ComputeRoomDistance();
	}
	
}

#endif // BUILD_EDIT_LOADSAVE

void EERIE_PORTAL_ReleaseOnlyVertexBuffer() {
	
	if(!portals) {
		return;
	}
	
	if(!portals->room || portals->nb_rooms <= 0) {
		return;
	}
	
	LogDebug("Destroying scene VBOs");
	
	for(long i = 0; i < portals->roomsize(); i++) {
		portals->room[i].usNbTextures = 0;
		delete portals->room[i].pVertexBuffer, portals->room[i].pVertexBuffer = NULL;
		free(portals->room[i].pussIndice), portals->room[i].pussIndice = NULL;
		free(portals->room[i].ppTextureContainer), portals->room[i].ppTextureContainer = NULL;
	}
}

namespace {

struct SINFO_TEXTURE_VERTEX {
	
	int opaque;
	int multiplicative;
	int additive;
	int blended;
	int subtractive;
	
	SINFO_TEXTURE_VERTEX()
		: opaque(0)
		, multiplicative(0)
		, additive(0)
		, blended(0)
		, subtractive(0)
	{}
};

} // anonymous namespace

void ComputePortalVertexBuffer() {
	
	if(!portals) {
		return;
	}
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	LogDebug("Creating scene VBOs");
	
	if(portals->nb_rooms > 255) {
		LogError << "Too many rooms: " << portals->roomsize();
		return;
	}
	
	typedef boost::unordered_map<TextureContainer *,  SINFO_TEXTURE_VERTEX>
		TextureMap;
	TextureMap infos;
	
	for(int i = 0; i < portals->roomsize(); i++) {
		
		EERIE_ROOM_DATA * room = &portals->room[i];
		
		// Skip empty rooms
		if(!room->nb_polys) {
			continue;
		}
		
		infos.clear();
		
		// Count vertices / indices for each texture and blend types
		int vertexCount = 0, indexCount = 0, ignored = 0, hidden = 0, notex = 0;
		for(int j = 0; j < room->nb_polys; j++) {
			int x = room->epdata[j].px, y = room->epdata[j].py;
			EERIE_BKG_INFO & cell = ACTIVEBKG->Backg[x + y * ACTIVEBKG->Xsize];
			EERIEPOLY & poly = cell.polydata[room->epdata[j].idx];
			
			if(poly.type & POLY_IGNORE) {
				ignored++;
				continue;
			}
			
			if(poly.type & POLY_HIDE) {
				hidden++;
				continue;
			}
			
			if(!poly.tex) {
				notex++;
				continue;
			}
			
			if(!poly.tex->tMatRoom) {
				poly.tex->tMatRoom = (SMY_ARXMAT *)malloc(sizeof(SMY_ARXMAT)
														   * (portals->roomsize()));
			}
			
			SINFO_TEXTURE_VERTEX & info = infos[poly.tex];
			
			int nvertices = (poly.type & POLY_QUAD) ? 4 : 3;
			int nindices  = (poly.type & POLY_QUAD) ? 6 : 3;
			
			if(poly.type & POLY_TRANS) {
				
				float trans = poly.transval;
				
				if(poly.transval >= 2.f) { // multiplicative
					info.multiplicative += nindices;
					trans = trans * 0.5f + 0.5f;
				} else if(poly.transval >= 1.f) { // additive
					info.additive += nindices;
					trans -= 1.f;
				} else if(poly.transval > 0.f) { // normal trans
					info.blended += nindices;
					trans = 1.f - trans;
				} else { // subtractive
					info.subtractive += nindices;
					trans = 1.f - trans;
				}
				
				poly.v[3].color = poly.v[2].color = poly.v[1].color = poly.v[0].color
					= Color::gray(trans).toBGR();
				
			} else {
				info.opaque += nindices;
			}
			
			vertexCount += nvertices;
			indexCount += nindices;
		}
		
		
		if(!vertexCount) {
			LogWarning << "No visible vertices in room " << i << ": "
			           << ignored << " ignored, " << hidden << " hidden, "
			           << notex << " untextured";
			continue;
		}
		
		
		// Allocate the index buffer for this room
		room->pussIndice = (unsigned short *)malloc(sizeof(unsigned short)
		                                            * indexCount);
		
		// Allocate the vertex buffer for this room
		// TODO should be static, but is updated for dynamic lighting
		room->pVertexBuffer = GRenderer->createVertexBuffer(vertexCount,
		                                                    Renderer::Dynamic);
		
		
		// Now fill the buffers
		
		SMY_VERTEX * vertex = room->pVertexBuffer->lock(NoOverwrite);
		
		int startIndex = 0;
		int startIndexCull = 0;
		
		size_t ntextures = infos.size();
		
		LogDebug(" - room " << i << ": " << ntextures << " textures, "
		         << vertexCount << " vertices, " << indexCount << " indices");
		
		// Allocate space to list all textures for this room
		// TODO use std::vector
		room->ppTextureContainer = (TextureContainer **)realloc(
			room->ppTextureContainer,
			sizeof(*room->ppTextureContainer) * (room->usNbTextures + ntextures)
		);
		
		TextureMap::const_iterator it;
		for(it = infos.begin(); it != infos.end(); ++it) {
			
			TextureContainer * texture = it->first;
			const SINFO_TEXTURE_VERTEX & info = it->second;
			
			unsigned short index = 0;
			
			// Upload all vertices for this texture and remember the indices
			for(int j = 0; j < room->nb_polys; j++) {
				int x = room->epdata[j].px, y = room->epdata[j].py;
				EERIE_BKG_INFO & cell = ACTIVEBKG->Backg[x + y * ACTIVEBKG->Xsize];
				EERIEPOLY & poly = cell.polydata[room->epdata[j].idx];
				
				if((poly.type & POLY_IGNORE) || (poly.type & POLY_HIDE) || !poly.tex) {
					continue;
				}
				
				if(poly.tex != texture) {
					continue;
				}
				
				vertex->p.x = poly.v[0].p.x;
				vertex->p.y = -(poly.v[0].p.y);
				vertex->p.z = poly.v[0].p.z;
				vertex->color = poly.v[0].color;
				vertex->uv = poly.v[0].uv + texture->hd;
				vertex++;
				poly.uslInd[0] = index++;
				
				vertex->p.x = poly.v[1].p.x;
				vertex->p.y = -(poly.v[1].p.y);
				vertex->p.z = poly.v[1].p.z;
				vertex->color = poly.v[1].color;
				vertex->uv = poly.v[1].uv + texture->hd;
				vertex++;
				poly.uslInd[1] = index++;
				
				vertex->p.x = poly.v[2].p.x;
				vertex->p.y = -(poly.v[2].p.y);
				vertex->p.z = poly.v[2].p.z;
				vertex->color = poly.v[2].color;
				vertex->uv = poly.v[2].uv + texture->hd;
				vertex++;
				poly.uslInd[2] = index++;
				
				if(poly.type & POLY_QUAD) {
					vertex->p.x = poly.v[3].p.x;
					vertex->p.y = -(poly.v[3].p.y);
					vertex->p.z = poly.v[3].p.z;
					vertex->color = poly.v[3].color;
					vertex->uv = poly.v[3].uv + texture->hd;
					vertex++;
					poly.uslInd[3] = index++;
				}
			}
			
			// Record that the texture is used for this room
			room->ppTextureContainer[room->usNbTextures++] = texture;
			
			// Save the 
			
			SMY_ARXMAT & m = texture->tMatRoom[i];
			
			m.uslStartVertex = startIndex;
			m.uslNbVertex = index;
			
			m.offset[SMY_ARXMAT::Opaque]         =  startIndexCull;
			m.offset[SMY_ARXMAT::Blended]        = (startIndexCull += info.opaque);
			m.offset[SMY_ARXMAT::Multiplicative] = (startIndexCull += info.blended);
			m.offset[SMY_ARXMAT::Additive]       = (startIndexCull += info.multiplicative);
			m.offset[SMY_ARXMAT::Subtractive]    = (startIndexCull += info.additive);
												   (startIndexCull += info.subtractive);
			
			m.count[SMY_ARXMAT::Opaque] = 0;
			m.count[SMY_ARXMAT::Blended] = 0;
			m.count[SMY_ARXMAT::Multiplicative] = 0;
			m.count[SMY_ARXMAT::Additive] = 0;
			m.count[SMY_ARXMAT::Subtractive] = 0;
			
			if(info.opaque > 65535 || info.multiplicative > 65535
			   || info.additive > 65535 || info.blended > 65535
			   || info.subtractive > 65535) {
				LogWarning << "Too many indices for texture " << texture->m_texName
				           << " in room " << i;
			}
			
			startIndex += index;
		}
		
		room->pVertexBuffer->unlock();
	}
}

long EERIERTPPoly(EERIEPOLY *ep)
{
	EE_RTP(&ep->v[0],&ep->tv[0]);
	EE_RTP(&ep->v[1],&ep->tv[1]);
	EE_RTP(&ep->v[2],&ep->tv[2]);

	if (ep->type & POLY_QUAD) 
	{
		EE_RTP(&ep->v[3],&ep->tv[3]);

		if ((ep->tv[0].p.z<=0.f) &&
			(ep->tv[1].p.z<=0.f) &&
			(ep->tv[2].p.z<=0.f) &&
			(ep->tv[3].p.z<=0.f) ) 
		{
			return 0;
		}
	}
	else
	{
		if ((ep->tv[0].p.z<=0.f) &&
			(ep->tv[1].p.z<=0.f) &&
			(ep->tv[2].p.z<=0.f)  ) 
		{
			return 0;
		}
	}

	return 1;
}
