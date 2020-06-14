/*
 * Copyright 2016-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Raycast.h"

#include <limits>

#include <boost/foreach.hpp>

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "graphics/data/Mesh.h"
#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"

void dbg_addRay(Vec3f start, Vec3f end);
void dbg_addTile(Vec2i tile);
void dbg_addPoly(EERIEPOLY * poly, Vec3f hit, Color c);

// Based on:
// Real-Time Collision Detection by Christer Ericson,
// published by Morgan Kaufmann Publishers, © 2005 Elsevier Inc
// page 326ff
template <class F>
static bool WalkTiles(const Vec3f & start, const Vec3f & end, F func) {
	
	Vec2f p1 = Vec2f(start.x, start.z);
	Vec2f p2 = Vec2f(end.x, end.z);
	Vec2f dir = p2 - p1;
	
	// Side dimensions of the square cell
	const Vec2f cellSide = g_backgroundTileSize;
	
	// Determine in which primary direction to step
	Vec2i d((p1.x <= p2.x) - (p1.x >= p2.x), (p1.y <= p2.y) - (p1.y >= p2.y));
	
	// Determine start grid cell coordinates
	Vec2i tile = Vec2i(glm::floor(p1 / cellSide));
	
	// Determine end grid cell coordinates
	Vec2i endTile = Vec2i(glm::floor(p2 / cellSide));
	
	// Handle invalid start tiles
	if(tile.x < 0 || tile.x >= ACTIVEBKG->m_size.x) {
		if(d.x == 0 || d.x == (tile.x < 0 ? -1 : 1)) {
			// Going away from valid tiles
			return false;
		}
		if(tile.x < 0 ? endTile.x < 0 : endTile.x >= ACTIVEBKG->m_size.x) {
			// Not going far enough to reach valid tiles
			return false;
		}
		s32 tilesToSkip = (tile.x < 0 ? -tile.x : tile.x + 1 - ACTIVEBKG->m_size.x);
		p1 += dir * (cellSide.x / dir.x * tilesToSkip);
		dir = p2 - p1;
		tile = Vec2i(glm::floor(p1 / cellSide));
	}
	if(tile.y < 0 || tile.y >= ACTIVEBKG->m_size.y) {
		if(d.y == 0 || d.y == (tile.y < 0 ? -1 : 1)) {
			// Going away from valid tiles
			return false;
		}
		if(tile.y < 0 ? endTile.y < 0 : endTile.y >= ACTIVEBKG->m_size.x) {
			// Not going far enough to reach valid tiles
			return false;
		}
		s32 tilesToSkip = (tile.y < 0 ? -tile.y : tile.y + 1 - ACTIVEBKG->m_size.y);
		p1 += dir * (cellSide.y / dir.y * tilesToSkip);
		dir = p2 - p1;
		tile = Vec2i(glm::floor(p1 / cellSide));
	}
	arx_assert(tile.x >= 0 && tile.x < ACTIVEBKG->m_size.x);
	arx_assert(tile.y >= 0 && tile.y < ACTIVEBKG->m_size.y);
	
	// Handle invalid end tiles
	endTile.x = glm::clamp(endTile.x, s32(0), s32(ACTIVEBKG->m_size.x - 1));
	endTile.y = glm::clamp(endTile.y, s32(0), s32(ACTIVEBKG->m_size.y - 1));
	
	
	// Determine tx and ty, the values of t at which the directed segment
	// (x1,y1)-(x2,y2) crosses the first horizontal and vertical cell
	// boundaries, respectively. Min(tx, ty) indicates how far one can
	// travel along the segment and still remain in the current cell
	Vec2f min = cellSide * glm::floor(p1 / cellSide);
	Vec2f max = min + cellSide;
	
	float tx = ((p1.x > p2.x) ? (p1.x - min.x) : (max.x - p1.x)) / std::abs(p2.x - p1.x);
	float ty = ((p1.y > p2.y) ? (p1.y - min.y) : (max.y - p1.y)) / std::abs(p2.y - p1.y);
	
	// Determine deltax/deltay, how far (in units of t) one must step
	// along the directed line segment for the horizontal/vertical
	// movement (respectively) to equal the width/height of a cell
	Vec2f delta = cellSide / glm::abs(dir);
	
	// Main loop. Visits cells until last cell reached
	for(;;) {
		
		dbg_addTile(tile);
		bool abort = func(start, end, tile);
		if(abort) {
			return true;
		}
		
		if(tx <= ty) {
			// tx smallest, step in x
			if(tile.x == endTile.x)
				break;
			
			tx += delta.x;
			tile.x += d.x;
		} else {
			// ty smallest, step in y
			if(tile.y == endTile.y)
				break;
			
			ty += delta.y;
			tile.y += d.y;
		}
	}
	
	return false;
}

static float linePolyIntersection(const Vec3f & start, const Vec3f & dir, const EERIEPOLY & epp) {
	
	Vec3f hit;
	if(arx::intersectLineTriangle(start, dir, epp.v[0].p, epp.v[1].p, epp.v[2].p, hit)) {
		if(hit.x >= 0.f && hit.x <= 1.f) {
			return hit.x;
		}
	}
	
	if((epp.type & POLY_QUAD)) {
		if(arx::intersectLineTriangle(start, dir, epp.v[1].p, epp.v[3].p, epp.v[2].p, hit)) {
			if(hit.x >= 0.f && hit.x <= 1.f) {
				return hit.x;
			}
		}
	}
	
	return std::numeric_limits<float>::max();
}

namespace {

struct ClosestHitRaycast {
	
	float closestHit;
	EERIEPOLY * hitPoly;
	const PolyType ignoredTypes;
	const bool anyHit;
	
	explicit ClosestHitRaycast(PolyType ignored, RaycastFlags flags)
		: closestHit(std::numeric_limits<float>::max())
		, hitPoly(NULL)
		, ignoredTypes(ignored)
		, anyHit(flags.has(RaycastAnyHit))
	{ }
	
	bool operator()(const Vec3f & start, const Vec3f & end, const Vec2i & tile) {
		
		Vec3f dir = end - start;
		
		bool previouslyHadHit = (closestHit <= 1.f);
		
		const BackgroundTileData & eg = ACTIVEBKG->m_tileData[tile.x][tile.y];
		BOOST_FOREACH(EERIEPOLY * ep, eg.polyin) {
			
			if(ep->type & ignoredTypes) {
				continue;
			}
			
			float relDist = linePolyIntersection(start, dir, *ep);
			if(relDist < closestHit) {
				closestHit = relDist;
				hitPoly = ep;
			}
			
		}
		
		if(previouslyHadHit) {
			// Assume that the previous hit was in this tile
			return true;
		} else if(closestHit > 1.f) {
			// No hit, search the next tile
			return false;
		}
		
		if(anyHit) {
			return true;
		}
		
		// Determine hit grid cell coordinates
		const Vec2f cellSide = g_backgroundTileSize;
		Vec3f hitPos = start + closestHit * dir;
		Vec2i hitTile = Vec2i(glm::floor(Vec2f(hitPos.x, hitPos.z) / cellSide));
		
		// Abort the search if hit in this tile
		// Otherwise hit is in the next tile, need to check other polygons in that tile
		// first, which may not all be in this tile.
		return (hitTile == tile);
	}
	
};

} // anonymous namespace

RaycastResult raycastScene(const Vec3f & start, const Vec3f & end, PolyType ignored, RaycastFlags flags) {
	dbg_addRay(start, end);
	ClosestHitRaycast raycast(ignored, flags);
	// TODO With C++11 we can change argument to F && instead of
	// explicitly specifying the reference type
	WalkTiles<ClosestHitRaycast &>(start, end, raycast);
	if(raycast.closestHit <= 1.f) {
		Vec3f hitPos = start + raycast.closestHit * (end - start);
		#ifdef RAYCAST_DEBUG
		dbg_addPoly(raycast.hitPoly, hitPos, Color::green);
		#endif
		return RaycastResult(raycast.hitPoly, hitPos);
	}
	return RaycastResult();
}

EntityRaycastResult raycastEntities(const Vec3f & start, const Vec3f & end,
                                    PolyType ignored, RaycastFlags flags) {
	
	Vec3f dir = end - start;
	Vec3f invdir = 1.f / dir;
	
	Entity * hitEntity = NULL;
	EERIE_FACE * hitFace = NULL;
	float t = std::numeric_limits<float>::max();
	
	for(size_t i = (flags & RaycastIgnorePlayer) ? 1 : 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * entity = entities[handle];
		
		if(!entity
		   || !entity->obj
		   || !(entity->gameFlags & GFLAG_ISINTREATZONE)
		   || (entity->gameFlags & (GFLAG_INVISIBILITY | GFLAG_MEGAHIDE))
		   || (entity->ioflags & (IO_CAMERA | IO_MARKER))) {
			continue;
		}
		
		switch(entity->show) {
			case SHOW_FLAG_LINKED:       break;
			case SHOW_FLAG_IN_SCENE:     break;
			case SHOW_FLAG_TELEPORTING:  break;
			case SHOW_FLAG_ON_PLAYER: {
				if(!(flags & RaycastIgnorePlayer) && IsEquipedByPlayer(entity)) {
					break;
				}
				continue;
			}
			default: continue;
		}
		
		const EERIE_3D_BBOX & box = entity->bbox3D;
		
		Vec3f min = (box.min - start) * invdir;
		Vec3f max = (box.max - start) * invdir;
		
		Vec3f zmax = glm::max(min, max);
		float tmax = glm::min(glm::min(zmax.x, zmax.y), zmax.z);
		if(tmax < 0) {
			continue;
		}
		
		Vec3f zmin = glm::min(min, max);
		float tmin = glm::max(glm::max(zmin.x, zmin.y), zmin.z);
		if(tmin > tmax || tmin > t) {
			continue;
		}
		
		BOOST_FOREACH(EERIE_FACE & face, entity->obj->facelist) {
			
			if(face.facetype & ignored) {
				continue;
			}
			
			Vec3f v0 = entity->obj->vertexWorldPositions[face.vid[0]].v;
			Vec3f v1 = entity->obj->vertexWorldPositions[face.vid[1]].v;
			Vec3f v2 = entity->obj->vertexWorldPositions[face.vid[2]].v;
			
			Vec3f hit;
			if(arx::intersectLineTriangle(start, dir, v0, v1, v2, hit)) {
				if(hit.x >= 0.f && hit.x <= 1.f && hit.x < t) {
					hitEntity  = entity;
					hitFace = &face;
					t = hit.x;
					if(flags & RaycastAnyHit) {
						Vec3f hitPos = start + t * dir;
						return EntityRaycastResult(hitEntity, hitFace, hitPos);
					}
				}
			}
			
		}
		
	}
	
	if(t <= 1.f) {
		Vec3f hitPos = start + t * dir;
		return EntityRaycastResult(hitEntity, hitFace, hitPos);
	}
	
	return EntityRaycastResult();
}

#ifndef RAYCAST_DEBUG

void dbg_addRay(Vec3f start, Vec3f end) { ARX_UNUSED(start), ARX_UNUSED(end); }
void dbg_addTile(Vec2i tile){ ARX_UNUSED(tile); }
void dbg_addPoly(EERIEPOLY * poly, Vec3f hit, Color c){ ARX_UNUSED(poly), ARX_UNUSED(hit), ARX_UNUSED(c); }
void RaycastDebugClear() {}
void RaycastDebugDraw() {}

#else

#include "graphics/DrawLine.h"

struct DebugPoly {
	EERIEPOLY * poly;
	Vec3f pos;
	Color c;
	
	DebugPoly(EERIEPOLY * poly, Vec3f pos, Color c)
		: poly(poly)
		, pos(pos)
		, c(c)
	{ }
};

static std::vector<std::pair<Vec3f, Vec3f> > dbg_rays;
static std::vector<Vec2i> dbg_tiles;
static std::vector<DebugPoly> dbg_hits;

void dbg_addRay(Vec3f start, Vec3f end) {
	dbg_rays.push_back(std::make_pair(start, end));
}

void dbg_addTile(Vec2i tile) {
	dbg_tiles.push_back(tile);
}

void dbg_addPoly(EERIEPOLY * poly, Vec3f pos, Color c) {
	dbg_hits.push_back(DebugPoly(poly, pos, c));
}

void RaycastDebugClear() {
	dbg_rays.clear();
	dbg_tiles.clear();
	dbg_hits.clear();
}

void RaycastDebugDraw() {
	
	RenderState nullState;
	UseRenderState state(nullState);
	
	for(auto & ray: dbg_rays) {
		drawLine(ray.first, ray.second, Color::magenta);
	}
	
	for(auto & tile: dbg_tiles) {
		Vec3f foo = Vec3f(tile.x * 100.f, g_camera->pos.y + 80.f, tile.y * 100.f);
		drawLine(foo, foo + Vec3f(100.f, 0.f, 0.f), Color::white);
		drawLine(foo + Vec3f(100.f, 0.f, 0.f), foo + Vec3f(100.f, 0.f, 100.f), Color::white);
		drawLine(foo + Vec3f(100.f, 0.f, 100.f), foo + Vec3f(0.f, 0.f, 100.f), Color::white);
		drawLine(foo, foo + Vec3f(0.f, 0.f, 100.f), Color::white);
	}
	
	for(auto hit : dbg_hits) {
		drawLineCross(hit.pos, hit.c, 2);
		
		Vec3f pp[4];
		size_t count = ((hit.poly->type & POLY_QUAD) ? 4u : 3u);
		for(size_t i = 0; i < count; i++) {
			pp[i] = hit.poly->v[i].p;
		}
		
		drawLine(pp[0], pp[1], hit.c);
		drawLine(pp[2], pp[0], hit.c);
		if(count == 4) {
			drawLine(pp[2], pp[3], hit.c);
			drawLine(pp[3], pp[1], hit.c);
		} else {
			drawLine(pp[1], pp[2], hit.c);
		}
	}
}

#endif
