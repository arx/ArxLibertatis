/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#include <limits>

#include <boost/scoped_array.hpp>
#include <boost/unordered_map.hpp>

#include "ai/Anchors.h"
#include "ai/PathFinder.h"
#include "ai/PathFinderManager.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/LoadLevelScreen.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/VertexBuffer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/FastSceneFormat.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/texture/Texture.h"

#include "io/resource/ResourcePath.h"
#include "io/fs/FileStream.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/Blast.h"
#include "io/log/Logger.h"

#include "platform/profiler/Profiler.h"

#include "scene/Scene.h"
#include "scene/Light.h"
#include "scene/Interactive.h"

#include "util/String.h"

static void EERIE_PORTAL_Release();

static bool IntersectLinePlane(const Vec3f & l1, const Vec3f & l2, const EERIEPOLY & ep, Vec3f * intersect) {
	
	Vec3f v = l2 - l1;
	
	float d = glm::dot(v, ep.norm);
	
	if(d != 0.0f) {
		Vec3f v1 = ep.center - l2;
		d = glm::dot(v1, ep.norm) / d;
		
		*intersect = (v * d) + l2;
		
		return true;
	}

	return false;
}

static bool RayIn3DPolyNoCull(const Vec3f & start, const Vec3f & end, const EERIEPOLY & epp) {
	
	Vec3f dir = end - start;
	
	Vec3f hit;
	if(arx::intersectLineTriangle(start, dir, epp.v[0].p, epp.v[1].p, epp.v[2].p, hit)) {
		if(hit.x >= 0.f && hit.x <= 1.f) {
			return true;
		}
	}
	
	if((epp.type & POLY_QUAD)) {
		if(arx::intersectLineTriangle(start, dir, epp.v[1].p, epp.v[3].p, epp.v[2].p, hit)) {
			if(hit.x >= 0.f && hit.x <= 1.f) {
				return true;
			}
		}
	}
	
	return false;
}

bool RayCollidingPoly(const Vec3f & orgn, const Vec3f & dest, const EERIEPOLY & ep, Vec3f * hit)
{
	if(IntersectLinePlane(orgn, dest, ep, hit)) {
		if(RayIn3DPolyNoCull(orgn, dest, ep))
			return true;
	}

	return false;
}

long MakeTopObjString(Entity * io, std::string & dest) {
	
	if(!io) {
		return -1;
	}
	
	EERIE_3D_BBOX box;
	box.reset();

	for(size_t i = 0; i < io->obj->vertexlist.size(); i++) {
		box.add(io->obj->vertexWorldPositions[i].v);
	}
	box.min.y -= 5.f;
	box.max.y -= 5.f;
	
	dest = "";
	
	if(player.pos.x > box.min.x
			&& player.pos.x < box.max.x
			&& player.pos.z > box.min.z
			&& player.pos.z < box.max.z)
	{
		if(glm::abs(player.pos.y + 160.f - box.min.y) < 50.f)
			dest += " player";
	}

	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e != io) {
			if(e->show == SHOW_FLAG_IN_SCENE) {
				if((e->ioflags & IO_NPC) || (e->ioflags & IO_ITEM)) {
					if(e->pos.x > box.min.x
							&& e->pos.x < box.max.x
							&& e->pos.z > box.min.z
							&& e->pos.z < box.max.z)
					{
						if(glm::abs(e->pos.y - box.min.y) < 40.f) {
							dest += ' ' + e->idString();
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


EERIEPOLY * CheckInPoly(const Vec3f & poss, float * needY)
{
	long px = long(poss.x * ACTIVEBKG->m_mul.x);
	long pz = long(poss.z * ACTIVEBKG->m_mul.y);
	
	if(pz <= 0 || pz >= ACTIVEBKG->m_size.y - 1 || px <= 0 || px >= ACTIVEBKG->m_size.x - 1)
		return NULL;
	
	float rx = poss.x - ((float)px * ACTIVEBKG->m_tileSize.x);
	float rz = poss.z - ((float)pz * ACTIVEBKG->m_tileSize.y);
	
	
	short minx;
	short minz;
	short maxx;
	short maxz;
	
	(void)checked_range_cast<short>(pz - 1);
	(void)checked_range_cast<short>(pz + 1);
	short sPz = static_cast<short>(pz);
	
	if (rz < -40.f) {
		minz = sPz - 1;
		maxz = sPz - 1;
	} else if (rz < 40.f) {
		minz = sPz - 1;
		maxz = sPz;
	} else if(rz > 60.f) {
		minz = sPz;
		maxz = sPz + 1;
	} else {
		minz = sPz;
		maxz = sPz;
	}
	
	(void)checked_range_cast<short>(px + 1);
	(void)checked_range_cast<short>(px - 1);
	short sPx = static_cast<short>(px);
	
	if(rx < -40.f) {
		minx = sPx - 1;
		maxx = sPx - 1;
	} else if(rx < 40.f) {
		minx = sPx - 1;
		maxx = sPx;
	} else if(rx > 60.f) {
		minx = sPx;
		maxx = sPx + 1;
	} else {
		minx = sPx;
		maxx = sPx;
	}
	
	EERIEPOLY * found = NULL;
	float foundY = 0.f;
	
	for(short z = minz; z <= maxz; z++)
	for(short x = minx; x <= maxx; x++) {
		const BackgroundTileData & feg = ACTIVEBKG->m_tileData[x][z];
		
		for(short k = 0; k < feg.nbpolyin; k++) {
			EERIEPOLY * ep = feg.polyin[k];
			
			if(poss.x >= ep->min.x
			&& poss.x <= ep->max.x
			&& poss.z >= ep->min.z
			&& poss.z <= ep->max.z
			&& !(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			&& ep->max.y >= poss.y
			&& ep != found
			&& PointIn2DPolyXZ(ep, poss.x, poss.z)
			&& GetTruePolyY(ep, poss, &rz)
			&& rz >= poss.y
			&& (!found || rz <= foundY)
			) {
				found = ep;
				foundY = rz;
			}
		}
	}
	
	if(needY)
		*needY = foundY;
	
	return found;
}

BackgroundTileData * getFastBackgroundData(float x, float z) {
	
	long px = long(x * ACTIVEBKG->m_mul.x);
	long pz = long(z * ACTIVEBKG->m_mul.y);

	if(px < 0 || px >= ACTIVEBKG->m_size.x || pz < 0 || pz >= ACTIVEBKG->m_size.y)
		return NULL;
	
	return &ACTIVEBKG->m_tileData[px][pz];
}

EERIEPOLY * CheckTopPoly(const Vec3f & pos) {
	
	BackgroundTileData * feg = getFastBackgroundData(pos.x, pos.z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if((!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
		   && (ep->min.y < pos.y)
		   && (pos.x >= ep->min.x) && (pos.x <= ep->max.x)
		   && (pos.z >= ep->min.z) && (pos.z <= ep->max.z)
		   && (PointIn2DPolyXZ(ep, pos.x, pos.z))) {
			
			if((glm::abs(ep->max.y - ep->min.y) > 50.f) && (pos.y - ep->center.y < 60.f)) {
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
	
	BackgroundTileData * feg = getFastBackgroundData(x, z);
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

EERIEPOLY * GetMinPoly(const Vec3f & pos) {
	
	BackgroundTileData * feg = getFastBackgroundData(pos.x, pos.z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	float foundy = 0.0f;
	for (long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];

		if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			continue;
		
		if(PointIn2DPolyXZ(ep, pos.x, pos.z)) {
			float ret;
			if(GetTruePolyY(ep, pos, &ret)) {
				if(!found || ret > foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * GetMaxPoly(const Vec3f & pos) {
	
	BackgroundTileData * feg = getFastBackgroundData(pos.x, pos.z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	float foundy = 0.0f;
	for(long k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
			continue;
		
		if(PointIn2DPolyXZ(ep, pos.x, pos.z)) {
			float ret;
			if(GetTruePolyY(ep, pos, &ret)) {
				if(!found || ret < foundy) {
					found = ep;
					foundy = ret;
				}
			}
		}
	}
	
	return found;
}

EERIEPOLY * EEIsUnderWater(const Vec3f & pos) {
	
	BackgroundTileData * feg = getFastBackgroundData(pos.x, pos.z);
	if(!feg) {
		return NULL;
	}
	
	EERIEPOLY * found = NULL;
	for(short k = 0; k < feg->nbpolyin; k++) {
		
		EERIEPOLY * ep = feg->polyin[k];
		
		if(ep->type & POLY_WATER) {
			if(ep->max.y < pos.y && PointIn2DPolyXZ(ep, pos.x, pos.z)) {
				if(!found || ep->max.y < found->max.y) {
					found = ep;
				}
			}
		}
	}
	return found;
}

bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f & pos, float * ret) {
	
	Vec3f n = glm::cross(ep->v[1].p - ep->v[0].p, ep->v[2].p - ep->v[0].p);
	if(n.y == 0.f) {
		return false;
	}
	
	float y = glm::dot(ep->v[0].p - Vec3f(pos.x, 0.f, pos.z), n) / n.y;
	
	// Perhaps we can remove the clamp... (need to test)
	*ret = glm::clamp(y, ep->min.y, ep->max.y);
	
	return true;
}

// TODO copy-paste PortalPoly
bool GetTruePolyY(const PortalPoly * ep, const Vec3f & pos, float * ret) {
	
	Vec3f n = glm::cross(ep->p[1] - ep->p[0], ep->p[2] - ep->p[0]);
	if(n.y == 0.f) {
		return false;
	}
	
	float y = glm::dot(ep->p[0] - Vec3f(pos.x, 0.f, pos.z), n) / n.y;
	
	// Perhaps we can remove the clamp... (need to test)
	*ret = glm::clamp(y, ep->min.y, ep->max.y);
	
	return true;
}


//*************************************************************************************
//*************************************************************************************
BackgroundData * ACTIVEBKG = NULL;

Vec3f EE_RT(const Vec3f & in) {
	return Vec3f(ACTIVECAM->orgTrans.worldToView * Vec4f(in, 1.0f));
}

static Vec4f viewToClipSpace(const Vec3f & in) {
	
	float z = in.z;
	
	Vec4f out;
	out.x = in.x * ACTIVECAM->ProjectionMatrix[0][0] + z * ACTIVECAM->orgTrans.mod.x;
	out.y = in.y * ACTIVECAM->ProjectionMatrix[1][1] + z * ACTIVECAM->orgTrans.mod.y;
	out.z = in.z * ACTIVECAM->ProjectionMatrix[2][2] + ACTIVECAM->ProjectionMatrix[3][2];
	out.w = z;
	return out;
}

Vec4f worldToClipSpace(const Vec3f & in) {
	return viewToClipSpace(EE_RT(in));
}

void worldToClipSpace(const Vec3f & in, TexturedVertex & out) {
	Vec4f p = worldToClipSpace(in);
	out.p = Vec3f(p);
	out.w = p.w;
}

//*************************************************************************************
//*************************************************************************************

long EERIEDrawnPolys = 0;

//*************************************************************************************
//*************************************************************************************

float PtIn2DPolyProj(const std::vector<Vec4f> & verts, EERIE_FACE * ef, float x, float z) {
	
	Vec3f p[3];
	for(size_t i = 0; i < 3; i++) {
		if(verts[ef->vid[i]].w <= 0.f) {
			return 0.f;
		}
		p[i] = Vec3f(verts[ef->vid[i]]) / verts[ef->vid[i]].w;
	}
	
	bool c = false;
	
	for(size_t i = 0, j = 2; i < 3; j = i++) {
		if(((p[i].y <= z && z < p[j].y) || (p[j].y <= z && z < p[i].y))
		   && x < (p[j].x - p[i].x) *(z - p[i].y) / (p[j].y - p[i].y) + p[i].x) {
			c = !c;
		}
	}
	
	return c ? p[0].z : 0.f;
}

static int PointIn2DPolyXZ(const Vec3f (&verts)[4], bool isQuad, float x, float z) {
	
	int i, j, c = 0, d = 0;

	for (i = 0, j = 2; i < 3; j = i++)
	{
		if ((((verts[i].z <= z) && (z < verts[j].z)) ||
				((verts[j].z <= z) && (z < verts[i].z))) &&
				(x < (verts[j].x - verts[i].x) *(z - verts[i].z) / (verts[j].z - verts[i].z) + verts[i].x))
			c = !c;
	}

	if(isQuad)
		for (i = 1, j = 3; i < 4; j = i++)
		{
			if ((((verts[i].z <= z) && (z < verts[j].z)) ||
					((verts[j].z <= z) && (z < verts[i].z))) &&
					(x < (verts[j].x - verts[i].x) *(z - verts[i].z) / (verts[j].z - verts[i].z) + verts[i].x))
				d = !d;
		}

	return c + d;
}

int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z) {
	Vec3f p[4] = { ep->v[0].p, ep->v[1].p, ep->v[2].p, ep->v[3].p };
	return PointIn2DPolyXZ(p, (ep->type & POLY_QUAD) != 0, x, z);
}

int PointIn2DPolyXZ(const PortalPoly * ep, float x, float z) {
	return PointIn2DPolyXZ(ep->p, true, x, z);
}

//*************************************************************************************
// Counts number of ignored polys in a background
//*************************************************************************************

// Releases BKG_INFO from a tile
static void ReleaseBKG_INFO(BackgroundTileData * eg) {
	free(eg->polydata);
	eg->polydata = NULL;
	free(eg->polyin);
	eg->polyin = NULL;
	eg->nbpolyin = 0;
	*eg = BackgroundTileData();
}

//*************************************************************************************
//*************************************************************************************

void UpdateIORoom(Entity * io)
{
	Vec3f pos = io->pos;
	pos.y -= 60.f;

	long roo = ARX_PORTALS_GetRoomNumForPosition(pos, 2);

	if(roo >= 0)
		io->room = checked_range_cast<short>(roo);

	io->requestRoomUpdate = false;
}

ROOM_DIST_DATA * RoomDistance = NULL;
static size_t NbRoomDistance = 0;

static void SetRoomDistance(size_t i, size_t j, float val, const Vec3f & p1, const Vec3f & p2) {
	
	if(i >= NbRoomDistance || j >= NbRoomDistance || !RoomDistance)
		return;
	
	size_t offs = i + j * NbRoomDistance;
	
	RoomDistance[offs].startpos = p1;
	RoomDistance[offs].endpos = p2;
	RoomDistance[offs].distance = val;
}

static float GetRoomDistance(size_t i, size_t j, Vec3f & p1, Vec3f & p2)
{
	if(i >= NbRoomDistance || j >= NbRoomDistance)
		return -1.f;

	size_t offs = i + j * NbRoomDistance;

	p1 = RoomDistance[offs].startpos;
	p2 = RoomDistance[offs].endpos;

	return RoomDistance[offs].distance;
}

float SP_GetRoomDist(const Vec3f & pos, const Vec3f & c_pos, long io_room, long Cam_Room)
{
	float dst = fdist(pos, c_pos);

	if(dst < 150.f)
		return dst;

	if(!portals || !RoomDistance)
		return dst;

	long Room = io_room;

	if(Room >= 0) {
		Vec3f p1, p2;
		
		float v;
		if(Cam_Room < 0 || Room < 0) {
			v = -1.f;
		} else {
			v = GetRoomDistance(size_t(Cam_Room), size_t(Room), p1, p2);
		}
		
		if(v > 0.f) {
			v += fdist(pos, p2);
			v += fdist(c_pos, p1);
			return v;
		}
	}

	return dst;
}

// Clears a background of its infos
void ClearBackground(BackgroundData * eb) {
	
	if(!eb)
		return;
	
	AnchorData_ClearAll(eb);
	
	for(long z = 0; z < eb->m_size.y; z++)
	for(long x = 0; x < eb->m_size.x; x++) {
		ReleaseBKG_INFO(&eb->m_tileData[x][z]);
	}
	
	free(RoomDistance);
	RoomDistance = NULL;
	NbRoomDistance = 0;
}

void InitBkg(BackgroundData * eb, short sx, short sz, Vec2s tileSize) {
	
	arx_assert(eb);
	
	if(eb->exist) {
		EERIE_PORTAL_Release();
		ClearBackground(eb);
	}

	eb->exist = 1;
	eb->anchors = NULL;
	eb->nbanchors = 0;
	eb->m_size.x = sx;
	eb->m_size.y = sz;

	if (tileSize.x < 0) tileSize.x = 1;
	if (tileSize.y < 0) tileSize.y = 1;

	eb->m_tileSize = tileSize;
	eb->m_mul.x = 1.f / (float)eb->m_tileSize.x;
	eb->m_mul.y = 1.f / (float)eb->m_tileSize.y;
	
	for(short z = 0; z < eb->m_size.y; z++)
	for(short x = 0; x < eb->m_size.x; x++) {
		eb->m_tileData[x][z] = BackgroundTileData();
	}
}

//*************************************************************************************
//*************************************************************************************

static void EERIEPOLY_Add_PolyIn(BackgroundTileData * eg, EERIEPOLY * ep) {
	
	for(long i = 0; i < eg->nbpolyin; i++)
		if(eg->polyin[i] == ep)
			return;

	eg->polyin = (EERIEPOLY **)realloc(eg->polyin, sizeof(EERIEPOLY *) * (eg->nbpolyin + 1));

	eg->polyin[eg->nbpolyin] = ep;
	eg->nbpolyin++;
}

static bool PointInBBox(const Vec3f & point, const Rectf & bb) {
	
	if(   point.x > bb.right
	   || point.x < bb.left
	   || point.z > bb.bottom
	   || point.z < bb.top
	)
		return false;

	return true;
}

void EERIEPOLY_Compute_PolyIn() {
	
	for(long z = 0; z < ACTIVEBKG->m_size.y; z++)
	for(long x = 0; x < ACTIVEBKG->m_size.x; x++) {
		BackgroundTileData *eg = &ACTIVEBKG->m_tileData[x][z];
		
		free(eg->polyin);
		eg->polyin = NULL;
		eg->nbpolyin = 0;
		
		long minx = std::max(x - 2, 0L);
		long minz = std::max(z - 2, 0L);
		long maxx = std::min(x + 2, ACTIVEBKG->m_size.x - 1L);
		long maxz = std::min(z + 2, ACTIVEBKG->m_size.y - 1L);
		
		Vec2f bbmin = Vec2f(x * ACTIVEBKG->m_tileSize.x - 10, z * ACTIVEBKG->m_tileSize.y - 10);
		Vec2f bbmax = Vec2f(bbmin.x + ACTIVEBKG->m_tileSize.x + 20, bbmin.y + ACTIVEBKG->m_tileSize.y + 20);
		
		Rectf bb = Rectf(bbmin, bbmax);
		
		Vec2f bbcenter = bb.center();
		
		for(long z2 = minz; z2 < maxz; z2++)
		for(long x2 = minx; x2 < maxx; x2++) {
			BackgroundTileData *eg2 = &ACTIVEBKG->m_tileData[x2][z2];
			
			for(long l = 0; l < eg2->nbpoly; l++) {
				EERIEPOLY *ep2 = &eg2->polydata[l];
				
				if(fartherThan(bbcenter, Vec2f(ep2->center.x, ep2->center.z), 120.f))
					continue;
				
				long nbvert = (ep2->type & POLY_QUAD) ? 4 : 3;
				
				if(PointInBBox(ep2->center, bb)) {
					EERIEPOLY_Add_PolyIn(eg, ep2);
				} else {
					for(long k = 0; k < nbvert; k++) {
						if(PointInBBox(ep2->v[k].p, bb)) {
							EERIEPOLY_Add_PolyIn(eg, ep2);
							break;
						} else {
							Vec3f pt = (ep2->v[k].p + ep2->center) * .5f;
							if(PointInBBox(pt, bb)) {
								EERIEPOLY_Add_PolyIn(eg, ep2);
								break;
							}
						}
					}
				}
			}
		}
		
		eg->maxy = -std::numeric_limits<float>::infinity();
		for(long i = 0; i < eg->nbpolyin; i++) {
			EERIEPOLY * ep = eg->polyin[i];
			eg->maxy = std::max(eg->maxy, ep->max.y);
		}
		
	}
}

static void EERIE_PORTAL_Blend_Portals_And_Rooms() {
	
	if(!portals)
		return;

	for(size_t num = 0; num < portals->portals.size(); num++) {
		EERIE_PORTALS & portal = portals->portals[num];
		PortalPoly * ep = &portal.poly;
		
		portal.poly.norm = CalcFaceNormal(portal.poly.p);
		
		ep->center = ep->p[0];

		long to = 4;

		float divide = ( 1.0f / to );
		
		ep->max = ep->min = ep->p[0];
		for(long i = 1; i < to; i++) {
			ep->center += ep->p[i];
			ep->min = glm::min(ep->min, ep->p[i]);
			ep->max = glm::max(ep->max, ep->p[i]);
		}
		
		ep->center *= divide;
		
		for(size_t nroom = 0; nroom < portals->rooms.size(); nroom++) {
			if(nroom == portal.room_1 || nroom == portal.room_2)
			{
				EERIE_ROOM_DATA & room = portals->rooms[nroom];
				
				room.portals = (long *)realloc(room.portals, sizeof(long) * (room.nb_portals + 1));
				room.portals[room.nb_portals] = num;
				room.nb_portals++;
			}
		}
	}
}

static void EERIE_PORTAL_Release() {
	
	if(!portals)
		return;
	
	for(size_t nn = 0; nn < portals->rooms.size(); nn++) {
		free(portals->rooms[nn].epdata);
		portals->rooms[nn].epdata = NULL;
		free(portals->rooms[nn].portals);
		portals->rooms[nn].portals = NULL;
		delete portals->rooms[nn].pVertexBuffer;
		portals->rooms[nn].pVertexBuffer = NULL;
		free(portals->rooms[nn].indexBuffer);
		portals->rooms[nn].indexBuffer = NULL;
				
		portals->rooms[nn].ppTextureContainer.clear();
	}
		
	delete portals;
	portals = NULL;
}


long CountBkgVertex() {

	long count = 0;

	for(long z = 0; z < ACTIVEBKG->m_size.y; z++) {
		for(long x = 0; x < ACTIVEBKG->m_size.x; x++) {
			const BackgroundTileData & eg = ACTIVEBKG->m_tileData[x][z];

			for(long l = 0; l < eg.nbpoly; l++) {
				const EERIEPOLY & ep = eg.polydata[l];
				
				if(ep.type & POLY_QUAD)
					count += 4;
				else
					count += 3;
			}
		}
	}

	return count;
}

void Draw3DObject(EERIE_3DOBJ *eobj, const Anglef & angle, const Vec3f & pos, const Vec3f & scale, const Color4f & coll, RenderMaterial mat)
{
	if(!eobj)
		return;
	
	TexturedVertex vert_list[3];
	
	glm::mat4 rotation = toRotationMatrix(angle);
	
	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		Vec3f scaled = eobj->vertexlist[i].v * scale;
		
		Vec3f rotated = Vec3f(rotation * Vec4f(scaled, 1.f));
		
		eobj->vertexWorldPositions[i].v = (rotated += pos);

		eobj->vertexClipPositions[i] = worldToClipSpace(rotated);
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE & face = eobj->facelist[i];
		
		for(size_t j = 0; j < 3; j++) {
			vert_list[j].p = Vec3f(eobj->vertexClipPositions[face.vid[j]]);
			vert_list[j].w = eobj->vertexClipPositions[face.vid[j]].w;
		}
		
		vert_list[0].uv.x = face.u[0];
		vert_list[0].uv.y = face.v[0];
		vert_list[1].uv.x = face.u[1];
		vert_list[1].uv.y = face.v[1];
		vert_list[2].uv.x = face.u[2];
		vert_list[2].uv.y = face.v[2];
		vert_list[0].color = vert_list[1].color = vert_list[2].color = coll.toRGBA();

		if(face.facetype == 0 || eobj->texturecontainer[face.texid] == NULL)
			mat.resetTexture();
		else
			mat.setTexture(eobj->texturecontainer[face.texid]);

		if(face.facetype & POLY_DOUBLESIDED)
			mat.setCulling(CullNone);
		else
			mat.setCulling(CullCW);
		
		g_renderBatcher.add(mat, vert_list);
	}
}

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
		scoped_malloc<char> dat(g_resources->readAlloc(file, size));
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
		progressBarAdvance();
		LoadLevelScreen();
		
		
		// Skip .scn file list and initialize the scene data
		(void)fts_read<UNIQUE_HEADER3>(data, end, uh->count);
		InitBkg(ACTIVEBKG, MAX_BKGX, MAX_BKGZ, Vec2s(BKG_SIZX, BKG_SIZZ));
		progressBarAdvance();
		LoadLevelScreen();
		
		
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
		progressBarAdvance(3.f);
		LoadLevelScreen();
		
		
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
	if(fsh->sizex != ACTIVEBKG->m_size.x || fsh->sizez != ACTIVEBKG->m_size.y) {
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
	progressBarAdvance(4.f);
	LoadLevelScreen();
	
	
	// Load cells with polygons and anchors
	LogDebug("FTS: loading " << fsh->sizex << " x " << fsh->sizez
	         << " cells ...");
	for(long j = 0; j < fsh->sizez; j++) {
		for(long i = 0; i < fsh->sizex; i++) {
			
			const FAST_SCENE_INFO * fsi = fts_read<FAST_SCENE_INFO>(data, end);
			
			BackgroundTileData & bkg = ACTIVEBKG->m_tileData[i][j];
			
			bkg.nbpoly = (short)fsi->nbpoly;
			
			if(fsi->nbpoly > 0) {
				bkg.polydata = (EERIEPOLY *)malloc(sizeof(EERIEPOLY) * fsi->nbpoly);
			} else {
				bkg.polydata = NULL;
			}
			
			bkg.treat = false;
			
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
					ep2->v[kk].color = Color(255, 255, 255, 255).toRGBA();
					ep2->v[kk].w = 1;
					ep2->v[kk].p = Vec3f(ep->v[kk].ssx, ep->v[kk].sy, ep->v[kk].ssz);
					ep2->v[kk].uv = Vec2f(ep->v[kk].stu, ep->v[kk].stv);
					ep2->color[kk] = Color(0, 0, 0, 255).toRGBA();
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
					float d = glm::distance(ep2->v[h].p, ep2->center);
					dist = std::max(dist, d);
				}
				ep2->v[0].w = dist;
			}
			
			if(fsi->nbianchors > 0) {
				fts_read<s32>(data, end, fsi->nbianchors);
			}
			
		}
	}
	progressBarAdvance(4.f);
	LoadLevelScreen();
	
	
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
	progressBarAdvance();
	LoadLevelScreen();
	
	arx_assert(fsh->nb_rooms > 0);
	
	// Load rooms and portals
	EERIE_PORTAL_Release();
	
	portals = new EERIE_PORTAL_DATA;
	portals->rooms.resize(fsh->nb_rooms + 1);
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
		portal.poly.center = epo->poly.center.toVec3();
		portal.poly.max = epo->poly.max.toVec3();
		portal.poly.min = epo->poly.min.toVec3();
		portal.poly.norm = epo->poly.norm.toVec3();
		
		for(size_t i = 0; i < 4; i++) {
			portal.poly.p[i] = epo->poly.v[i].pos.toVec3();
		}
		portal.poly.rhw = epo->poly.v[0].rhw;
		
		if(epo->poly.type == 0) {
			// Make sure all portal polys have 4 vertices
			// This is required to fix two polys in the original gamedata
			LogDebug("Adding position for non quad portal poly");
			portal.poly.p[3] = glm::mix(portal.poly.p[1], portal.poly.p[2], 0.5f);
		} else if(epo->poly.type != 64) {
			LogWarning << "Invalid poly type found in portal " << epo->poly.type;
		}
	}
	
	
	LogDebug("FTS: loading " << portals->rooms.size() << " rooms ...");
	for(size_t i = 0; i < portals->rooms.size(); i++) {
		
		const EERIE_SAVE_ROOM_DATA * erd;
		erd = fts_read<EERIE_SAVE_ROOM_DATA>(data, end);
		
		EERIE_ROOM_DATA & room = portals->rooms[i];
		
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
			portals->rooms[i].epdata = NULL;
		}
		
	}
	
	
	// Load distances between rooms
	free(RoomDistance);
	RoomDistance = NULL;
	NbRoomDistance = 0;
	if(portals) {
		NbRoomDistance = portals->rooms.size();
		RoomDistance = (ROOM_DIST_DATA *)malloc(sizeof(ROOM_DIST_DATA)
		                                        * NbRoomDistance * NbRoomDistance);
		LogDebug("FTS: loading " << (NbRoomDistance * NbRoomDistance)
		         << " room distances ...");
		
		for(size_t n = 0; n < NbRoomDistance; n++) {
			for(size_t m = 0; m < NbRoomDistance; m++) {
				const ROOM_DIST_DATA_SAVE * rdds;
				rdds = fts_read<ROOM_DIST_DATA_SAVE>(data, end);
				Vec3f start = rdds->startpos.toVec3();
				Vec3f end = rdds->endpos.toVec3();
				SetRoomDistance(m, n, rdds->distance, start, end);
			}
		}
	}
	progressBarAdvance();
	LoadLevelScreen();
	
	
	// Prepare the loaded data
	
	LogDebug("FTS: preparing scene data ...");
	
	EERIEPOLY_Compute_PolyIn();
	progressBarAdvance(3.f);
	LoadLevelScreen();
	
	EERIE_PATHFINDER_Create();
	EERIE_PORTAL_Blend_Portals_And_Rooms();
	progressBarAdvance();
	LoadLevelScreen();
	
	ComputePortalVertexBuffer();
	progressBarAdvance();
	LoadLevelScreen();
	
	
	if(data != end) {
		LogWarning << "FTS: ignoring " << (end - data) << " bytes at the end of "
		           << file;
	}
	LogDebug("FTS: done loading");
	
	return true;
}

void EERIE_PORTAL_ReleaseOnlyVertexBuffer() {
	
	if(!portals) {
		return;
	}
	
	if(portals->rooms.empty()) {
		return;
	}
	
	LogDebug("Destroying scene VBOs");
	
	for(size_t i = 0; i < portals->rooms.size(); i++) {
		delete portals->rooms[i].pVertexBuffer;
		portals->rooms[i].pVertexBuffer = NULL;
		free(portals->rooms[i].indexBuffer);
		portals->rooms[i].indexBuffer = NULL;
		portals->rooms[i].ppTextureContainer.clear();
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

struct HasAlphaChannel {
	
	bool operator()(TextureContainer * texture) {
		return !texture || !texture->m_pTexture || !texture->m_pTexture->hasAlpha();
	}
	
};

void ComputePortalVertexBuffer() {
	
	if(!portals) {
		return;
	}
	
	EERIE_PORTAL_ReleaseOnlyVertexBuffer();
	
	LogDebug("Creating scene VBOs");
	
	if(portals->rooms.size() > 255) {
		LogError << "Too many rooms: " << portals->rooms.size();
		return;
	}
	
	typedef boost::unordered_map<TextureContainer *,  SINFO_TEXTURE_VERTEX>
		TextureMap;
	TextureMap infos;
	
	for(size_t i = 0; i < portals->rooms.size(); i++) {
		
		EERIE_ROOM_DATA * room = &portals->rooms[i];
		
		// Skip empty rooms
		if(!room->nb_polys) {
			continue;
		}
		
		infos.clear();
		
		// Count vertices / indices for each texture and blend types
		int vertexCount = 0, indexCount = 0, ignored = 0, hidden = 0, notex = 0;
		for(int j = 0; j < room->nb_polys; j++) {
			int x = room->epdata[j].tile.x;
			int y = room->epdata[j].tile.y;
			BackgroundTileData & cell = ACTIVEBKG->m_tileData[x][y];
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
			
			RoomBatches & roomBatches = poly.tex->m_roomBatches;
			if(roomBatches.tMatRoomSize != portals->rooms.size()) {
				roomBatches.tMatRoomSize = portals->rooms.size();
				roomBatches.tMatRoom = (SMY_ARXMAT *)realloc(roomBatches.tMatRoom, sizeof(SMY_ARXMAT) * (portals->rooms.size()));
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
					= Color::gray(trans).toRGB();
				
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
		room->indexBuffer = (unsigned short *)malloc(sizeof(unsigned short)
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
		room->ppTextureContainer.reserve(room->ppTextureContainer.size() + ntextures);
		
		TextureMap::const_iterator it;
		for(it = infos.begin(); it != infos.end(); ++it) {
			
			TextureContainer * texture = it->first;
			const SINFO_TEXTURE_VERTEX & info = it->second;
			
			unsigned short index = 0;
			
			// Upload all vertices for this texture and remember the indices
			for(int j = 0; j < room->nb_polys; j++) {
				Vec2s tilePos = room->epdata[j].tile;
				BackgroundTileData & tile = ACTIVEBKG->m_tileData[tilePos.x][tilePos.y];
				EERIEPOLY & poly = tile.polydata[room->epdata[j].idx];
				
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
			room->ppTextureContainer.push_back(texture);
			
			// Save the
			
			SMY_ARXMAT & m = texture->m_roomBatches.tMatRoom[i];
			
			m.uslStartVertex = startIndex;
			m.uslNbVertex = index;
			
			m.offset[BatchBucket_Opaque]         =  startIndexCull;
			m.offset[BatchBucket_Blended]        = (startIndexCull += info.opaque);
			m.offset[BatchBucket_Multiplicative] = (startIndexCull += info.blended);
			m.offset[BatchBucket_Additive]       = (startIndexCull += info.multiplicative);
			m.offset[BatchBucket_Subtractive]    = (startIndexCull += info.additive);
												   (startIndexCull += info.subtractive);
			
			m.count[BatchBucket_Opaque] = 0;
			m.count[BatchBucket_Blended] = 0;
			m.count[BatchBucket_Multiplicative] = 0;
			m.count[BatchBucket_Additive] = 0;
			m.count[BatchBucket_Subtractive] = 0;
			
			if(   info.opaque > 65535
			   || info.blended > 65535
			   || info.multiplicative > 65535
			   || info.additive > 65535
			   || info.subtractive > 65535
			) {
				LogWarning << "Too many indices for texture " << texture->m_texName
				           << " in room " << i;
			}
			
			startIndex += index;
		}
		
		room->pVertexBuffer->unlock();
		
		std::partition(room->ppTextureContainer.begin(), room->ppTextureContainer.end(), HasAlphaChannel());
		
	}
}
