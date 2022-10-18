/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "physics/Collisions.h"

#include <algorithm>
#include <limits>

#include <boost/range/adaptor/strided.hpp>

#include "ai/Anchors.h"
#include "core/GameTime.h"
#include "core/Core.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"
#include "scene/Tiles.h"


long ON_PLATFORM = 0;
size_t EXCEPTIONS_LIST_Pos = 0;
EntityHandle EXCEPTIONS_LIST[MAX_IN_SPHERE + 1];

static long POLYIN = 0;
long COLLIDED_CLIMB_POLY = 0;
long MOVING_CYLINDER = 0;

Vec3f vector2D;
bool DIRECT_PATH = true;

static float IsPolyInCylinder(const EERIEPOLY & ep, const Cylinder & cyl, CollisionFlags flags) {
	
	POLYIN = 0;
	float minf = cyl.origin.y + cyl.height;
	float maxf = cyl.origin.y;
	if(minf > ep.max.y || maxf < ep.min.y) {
		return 999999.f;
	}
	
	long to = (ep.type & POLY_QUAD) ? 4 : 3;
	float nearest = 99999999.f;
	for(long num = 0; num < to; num++) {
		float dd = fdist(getXZ(ep.v[num].p), getXZ(cyl.origin));
		if(dd < nearest) {
			nearest = dd;
		}
	}
	if(nearest > std::max(82.f, cyl.radius)) {
		return 999999.f;
	}
	
	if(cyl.radius < 30.f || cyl.height > -80.f || ep.area > 5000.f) {
		flags |= CFLAG_EXTRA_PRECISION;
	}
	
	if(!(flags & CFLAG_EXTRA_PRECISION) && ep.area < 100.f) {
		return 999999.f;
	}
	
	float anything = 999999.f;
	if(PointInCylinder(cyl, ep.center)) {
		POLYIN = 1;
		anything = std::min(anything, (ep.norm.y < 0.5f) ? ep.min.y : ep.center.y);
		if(!(flags & CFLAG_EXTRA_PRECISION)) {
			return anything;
		}
	}
	
	long r = to - 1;
	for(long n = 0; n < to; n++) {
		
		if(flags & CFLAG_EXTRA_PRECISION) {
			for(long o = 0; o < 5; o++) {
				float p = float(o) * 0.2f;
				Vec3f center = ep.v[n].p * p + ep.center * (1.f - p);
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;
				}
			}
		}
		
		if(ep.area > 2000.f || (flags & CFLAG_EXTRA_PRECISION)) {
			
			Vec3f center = (ep.v[n].p + ep.v[r].p) * 0.5f;
			if(PointInCylinder(cyl, center)) {
				anything = std::min(anything, center.y);
				POLYIN = 1;
				if(!(flags & CFLAG_EXTRA_PRECISION)) {
					return anything;
				}
			}
			
			if(ep.area > 4000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (ep.v[n].p + ep.center) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;
					if(!(flags & CFLAG_EXTRA_PRECISION)) {
						return anything;
					}
				}
			}
			
			if(ep.area > 6000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (center + ep.v[n].p) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;
					if(!(flags & CFLAG_EXTRA_PRECISION)) {
						return anything;
					}
				}
			}
			
		}
		
		if(PointInCylinder(cyl, ep.v[n].p)) {
			anything = std::min(anything, ep.v[n].p.y);
			POLYIN = 1;
			if(!(flags & CFLAG_EXTRA_PRECISION)) {
				return anything;
			}
		}
		
		r++;
		if(r >= to) {
			r = 0;
		}
		
	}
	
	if(anything != 999999.f && ep.norm.y < 0.1f && ep.norm.y > -0.1f) {
		anything = std::min(anything, ep.min.y);
	}
	
	return anything;
}

static bool IsPolyInSphere(const EERIEPOLY & ep, const Sphere & sphere) {
	
	if(ep.area < 100.f) {
		return false;
	}
	
	long to = (ep.type & POLY_QUAD) ? 4 : 3;
	long r = to - 1;
	
	for(long n = 0; n < to; n++) {
		
		if(ep.area > 2000.f) {
			Vec3f center = (ep.v[n].p + ep.v[r].p) * 0.5f;
			if(sphere.contains(center)) {
				return true;
			}
			if(ep.area > 4000.f) {
				center = (ep.v[n].p + ep.center) * 0.5f;
				if(sphere.contains(center)) {
					return true;
				}
			}
			if(ep.area > 6000.f) {
				center = (center + ep.v[n].p) * 0.5f;
				if(sphere.contains(center)) {
					return true;
				}
			}
		}
		
		if(sphere.contains(ep.v[n].p)) {
			return true;
		}
		
		r++;
		if(r >= to) {
			r = 0;
		}
		
	}
	
	return false;
}

bool IsCollidingIO(Entity * io, Entity * ioo) {
	
	arx_assert(io);
	arx_assert(ioo);
	
	if(io == ioo ||
	   (ioo->ioflags & IO_NO_COLLISIONS) ||
	   ioo->show != SHOW_FLAG_IN_SCENE ||
	   !ioo->obj ||
	   !(ioo->ioflags & IO_NPC)) {
		return false;
	}
	
	Cylinder cyl = ioo->physics.cyl;
	cyl.radius += 25.f;
	
	for(const EERIE_VERTEX & vertex : io->obj->vertexWorldPositions) {
		if(PointInCylinder(cyl, vertex.v)) {
			return true;
		}
	}
	
	return false;
}

void PushIO_ON_Top(const Entity & platform, float ydec) {
	
	if(ydec == 0.f) {
		return;
	}
	
	for(Entity & entity : entities.inScene()) {
		
		if(entity == platform || (entity.ioflags & IO_NO_COLLISIONS) || !entity.obj
		   || (entity.ioflags & (IO_FIX | IO_CAMERA | IO_MARKER))) {
			continue;
		}
		
		if(!closerThan(getXZ(entity.pos), getXZ(platform.pos), 450.f)) {
			continue;
		}
		
		float miny = 9999999.f;
		float maxy = -9999999.f;
		for(const EERIE_VERTEX & vertex : platform.obj->vertexWorldPositions) {
			miny = std::min(miny, vertex.v.y);
			maxy = std::max(maxy, vertex.v.y);
		}
		
		float posy = (&entity == entities.player()) ? player.basePosition().y : entity.pos.y;
		float modd = (ydec > 0) ? -20.f : 0;
		if(posy > maxy || posy < miny + modd) {
			continue;
		}
		
		for(const EERIE_FACE & face : platform.obj->facelist) {
			
			EERIEPOLY ep;
			float cx = 0;
			float cz = 0;
			for(long kk = 0; kk < 3; kk++) {
				ep.v[kk].p = platform.obj->vertexWorldPositions[face.vid[kk]].v;
				cx += ep.v[kk].p.x;
				cz += ep.v[kk].p.z;
			}
			cx *= 1.f / 3;
			cz *= 1.f / 3;
			
			float tval = 1.1f;
			
			for(int kk = 0; kk < 3; kk++) {
				ep.v[kk].p.x = (ep.v[kk].p.x - cx) * tval + cx;
				ep.v[kk].p.z = (ep.v[kk].p.z - cz) * tval + cz;
			}
			
			if(PointIn2DPolyXZ(&ep, entity.pos.x, entity.pos.z)) {
				if(&entity == entities.player()) {
					if(ydec <= 0) {
						player.pos.y += ydec;
						g_moveto.y += ydec;
						Cylinder cyl = player.baseCylinder();
						cyl.origin.y += ydec;
						float vv = CheckAnythingInCylinder(cyl, entities.player(), 0);
						if(vv < 0) {
							player.pos.y += ydec + vv;
						}
					} else {
						Cylinder cyl = player.baseCylinder();
						cyl.origin.y += ydec;
						if(CheckAnythingInCylinder(cyl, entities.player(), 0) >= 0) {
							player.pos.y += ydec;
							g_moveto.y += ydec;
						}
					}
				} else {
					if(ydec <= 0) {
						entity.pos.y += ydec;
					} else {
						Cylinder cyl = getEntityCylinder(entity);
						cyl.origin.y += ydec;
						if(CheckAnythingInCylinder(cyl, &entity, 0) >= 0) {
							entity.pos.y += ydec;
						}
					}
				}
				break;
			}
			
		}
		
	}
	
}

bool isAnyNPCOnPlatform(const Entity & platform) {
	
	for(const Entity & entity : entities.inScene(IO_NPC)) {
		if(entity != platform && !(entity.ioflags & IO_NO_COLLISIONS)) {
			if(isCylinderCollidingWithPlatform(getEntityCylinder(entity), platform)) {
				return true;
			}
		}
	}
	
	return false;
}

bool isCylinderCollidingWithPlatform(const Cylinder & cylinder, const Entity & platform) {
 
	if(platform.bbox3D.max.y <= cylinder.origin.y + cylinder.height
	   || platform.bbox3D.min.y >= cylinder.origin.y) {
		return false;
	}
	
	return In3DBBoxTolerance(cylinder.origin, platform.bbox3D, cylinder.radius);
}

static long NPC_IN_CYLINDER = 0;

static void CheckAnythingInCylinder_Platform(const Cylinder & cylinder, const Entity & target,
                                             float & anything) {
	
	if(!closerThan(getXZ(target.pos), getXZ(cylinder.origin), cylinder.radius + 440.f) ||
	   !In3DBBoxTolerance(cylinder.origin, target.bbox3D, cylinder.radius + 80.f)) {
		return;
	}
	
	if(target.ioflags & IO_FIELD) {
		if(In3DBBoxTolerance(cylinder.origin, target.bbox3D, cylinder.radius + 10)) {
			anything = -99999.f;
		}
		return;
	}
	
	for(const EERIE_VERTEX & vertex : target.obj->vertexWorldPositions) {
		long res = PointInUnderCylinder(cylinder, vertex.v);
		if(res > 0) {
			if(res == 2) {
				ON_PLATFORM = 1;
			}
			anything = std::min(anything, vertex.v.y - 10.f);
		}
	}
	
	for(const EERIE_FACE & face : target.obj->facelist) {
		
		Vec3f center(0.f);
		float height = std::numeric_limits<float>::max();
		for(VertexId vertex : face.vid) {
			center += target.obj->vertexWorldPositions[vertex].v;
			height = std::min(height, target.obj->vertexWorldPositions[vertex].v.y);
		}
		center /= std::size(face.vid);
		center.y = target.bbox3D.min.y;
		
		long res = PointInUnderCylinder(cylinder, center);
		if(res > 0) {
			if(res == 2) {
				ON_PLATFORM = 1;
			}
			anything = std::min(anything, height);
		}
		
	}
	
}

static void sendNpcCollisionEvent(Entity & target, Entity & source) {
	
	arx_assert(source.ioflags & IO_NPC);
	arx_assert(target.ioflags & IO_NPC);
	
	EERIEPOLY ep;
	ep.type = 0;
	ep.v[0].p.x = target.pos.x;
	ep.v[0].p.z = target.pos.z;
	
	float ft = glm::radians(135.f + 90.f);
	ep.v[1].p.x =  std::sin(ft) * 180.f;
	ep.v[1].p.z = -std::cos(ft) * 180.f;
	
	ft = glm::radians(225.f + 90.f);
	ep.v[2].p.x =  std::sin(ft) * 180.f;
	ep.v[2].p.z = -std::cos(ft) * 180.f;
	
	float angle = 270.f - target.angle.getYaw();
	Vec3f p1 = VRotateY(ep.v[1].p, angle);
	Vec3f p2 = VRotateY(ep.v[2].p, angle);
	
	ep.v[1].p.x = p1.x + ep.v[0].p.x;
	ep.v[1].p.z = p1.z + ep.v[0].p.z;
	ep.v[2].p.x = p2.x + ep.v[0].p.x;
	ep.v[2].p.z = p2.z + ep.v[0].p.z;
	
	if(PointIn2DPolyXZ(&ep, source.pos.x, source.pos.z)) {
		SendIOScriptEvent(&source, &target, SM_COLLIDE_NPC, "back");
	} else {
		SendIOScriptEvent(&source, &target, SM_COLLIDE_NPC);
	}
	
}

static void handleNpcCollision(Entity & source, Entity & target) {
	
	GameDuration elapsed = g_gameTime.now() - target.collide_door_time;
	if(elapsed > 500ms) {
		target.collide_door_time = g_gameTime.now();
		sendNpcCollisionEvent(target, source);
		target.collide_door_time = g_gameTime.now();
		sendNpcCollisionEvent(source, target);
	}
	
	if(source.damager_damages > 0) {
		damageCharacter(target, source.damager_damages, source, nullptr, source.damager_type, &target.pos);
	}
	if(target.damager_damages > 0) {
		damageCharacter(source, target.damager_damages, target, nullptr, target.damager_type, &source.pos);
	}
	
	// TODO should this be source.index()?
	if(target.targetinfo == target.index()) {
		if(target._npcdata->pathfind.listnb > 0) {
			target._npcdata->pathfind.listpos = 0;
			target._npcdata->pathfind.listnb = -1;
			delete[] target._npcdata->pathfind.list;
			target._npcdata->pathfind.list = nullptr;
		}
		if(!target._npcdata->reachedtarget) {
			SendIOScriptEvent(&source, &target, SM_REACHEDTARGET);
			target._npcdata->reachedtarget = 1;
		}
	}
	
}

static void handlePropCollision(Entity & source, Entity & target, bool & dealt) {
	
	if(target.gameFlags & GFLAG_DOOR) {
		GameDuration elapsed = g_gameTime.now() - target.collide_door_time;
		if(elapsed > 500ms) {
			target.collide_door_time = g_gameTime.now();
			SendIOScriptEvent(&source, &target, SM_COLLIDE_DOOR);
			target.collide_door_time = g_gameTime.now();
			SendIOScriptEvent(&target, &source, SM_COLLIDE_DOOR);
		}
	}
	
	if(target.ioflags & IO_FIELD) {
		target.collide_door_time = g_gameTime.now();
		SendIOScriptEvent(nullptr, &source, SM_COLLIDE_FIELD);
	}
	
	if(!dealt && (source.damager_damages > 0 || target.damager_damages > 0)) {
		dealt = true;
		if((target.ioflags & IO_NPC) && source.damager_damages > 0) {
			damageCharacter(target, source.damager_damages, source, nullptr, source.damager_type, &target.pos);
		}
		if((source.ioflags & IO_NPC) && target.damager_damages > 0) {
			damageCharacter(source, target.damager_damages, target, nullptr, target.damager_type, &source.pos);
		}
	}
	
}

static void CheckAnythingInCylinder_Inner(const Cylinder & cylinder, Entity * source, CollisionFlags flags,
                                          Entity * target, float & anything) {
	
	if(!target
	   || target == source
	   || !target->obj
	   || target->show != SHOW_FLAG_IN_SCENE
	   || ((target->ioflags & IO_NO_COLLISIONS)  && !(flags & CFLAG_COLLIDE_NOCOL))
	   || fartherThan(target->pos, cylinder.origin, 1000.f)) {
		return;
	}
	
	target->physics.cyl = getEntityCylinder(*target);
	
	if((target->gameFlags & GFLAG_PLATFORM) ||
	   ((flags & CFLAG_COLLIDE_NOCOL) && (target->ioflags & IO_NPC) && (target->ioflags & IO_NO_COLLISIONS))) {
		CheckAnythingInCylinder_Platform(cylinder, *target, anything);
		return;
	}
	
	if((target->ioflags & IO_NPC)
	   && !(flags & CFLAG_NO_NPC_COLLIDE) // MUST be checked here only (not before...)
	   && !(source && (source->ioflags & IO_NO_COLLISIONS))
	   && target->_npcdata->lifePool.current > 0.f) {
		if(CylinderInCylinder(cylinder, target->physics.cyl)) {
			NPC_IN_CYLINDER = 1;
			anything = std::min(anything, target->physics.cyl.origin.y + target->physics.cyl.height);
			if(!(flags & CFLAG_JUST_TEST) && source) {
				arx_assert(source->ioflags & IO_NPC);
				handleNpcCollision(*source, *target);
			}
		}
		return;
	}
	
	if(!(target->ioflags & IO_FIX) ||
	   target->bbox3D.max.y <= cylinder.origin.y + cylinder.height ||
	   target->bbox3D.min.y >= cylinder.origin.y ||
	   !In3DBBoxTolerance(cylinder.origin, target->bbox3D, cylinder.radius + 30.f)) {
		return;
	}
	
	bool dealt = false;
	
	if(target->obj->grouplist.size() > 10) {
		
		float radius = 26.f;
		if(source == entities.player()) {
			radius = 22.f;
		} else if(source && !(source->ioflags & IO_NPC)) {
			radius = 22.f;
		}
		
		for(const VertexGroup & group : target->obj->grouplist) {
			Vec3f pos = target->obj->vertexWorldPositions[group.origin].v;
			if(SphereInCylinder(cylinder, Sphere(pos, radius))) {
				if(!(flags & CFLAG_JUST_TEST) && source) {
					handlePropCollision(*source, *target, dealt);
				}
				anything = std::min(anything, std::min(pos.y - radius, target->bbox3D.min.y));
			}
		}
		
	} else {
		
		float radius = 25.f;
		if(source == entities.player()) {
			radius = 23.f;
		} else if(source && !(source->ioflags & IO_NPC)) {
			radius = 32.f;
		}
		
		size_t step = 6;
		if(target->obj->vertexlist.size() < 300) {
			step = 1;
		} else if(target->obj->vertexlist.size() < 600) {
			step = 2;
		} else if(target->obj->vertexlist.size() < 1200) {
			step = 4;
		}
		
		for(size_t i = 1; i < target->obj->vertexlist.size(); i += step) {
			if(VertexId(i) != target->obj->origin) {
				Vec3f pos = target->obj->vertexWorldPositions[VertexId(i)].v;
				if(SphereInCylinder(cylinder, Sphere(pos, radius))) {
					if(!(flags & CFLAG_JUST_TEST) && source) {
						handlePropCollision(*source, *target, dealt);
					}
					anything = std::min(anything, std::min(pos.y - radius, target->bbox3D.min.y));
				}
			}
		}
		
	}
	
}

// Returns 0 if nothing in cyl
// Else returns Y Offset to put cylinder in a proper place
float CheckAnythingInCylinder(const Cylinder & cylinder, Entity * source, CollisionFlags flags) {
	
	ARX_PROFILE_FUNC();
	
	NPC_IN_CYLINDER = 0;
	
	float anything = 999999.f;
	
	// TODO should this be tilesAround(cyl.origin, cyl.radius)
	u16 radius = u16((cylinder.radius + g_backgroundTileSize.x) * g_tiles->m_mul.x);
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(cylinder.origin), radius)) {
		
		float nearest = 99999999.f;
		for(long num = 0; num < 4; num++) {
			Vec2f pos = Vec2f(tile) * g_backgroundTileSize;
			if(num == 1 || num == 2) {
				pos.x += g_backgroundTileSize.x;
			}
			if(num == 2 || num == 3) {
				pos.y += g_backgroundTileSize.y;
			}
			float dd = fdist(pos, getXZ(cylinder.origin));
			if(dd < nearest) {
				nearest = dd;
			}
		}
		
		if(nearest > std::max(82.f, cylinder.radius)) {
			continue;
		}
		
		for(const EERIEPOLY & ep : tile.polygons()) {
			
			if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
				continue;
			}
			
			if(ep.min.y < anything) {
				anything = std::min(anything, IsPolyInCylinder(ep, cylinder, flags));
				if(POLYIN && (ep.type & POLY_CLIMB)) {
					COLLIDED_CLIMB_POLY = 1;
				}
			}
			
		}
		
	}
	
	float tempo;
	if(CheckInPoly(cylinder.origin + Vec3f(0.f, cylinder.height, 0.f), &tempo)) {
		anything = std::min(anything, tempo);
	}
	
	if(!(flags & CFLAG_NO_INTERCOL)) {
		if(source && (source->ioflags & IO_NPC) && (source->_npcdata->pathfind.flags & PATHFIND_ALWAYS)) {
			for(Entity & other : entities) {
				CheckAnythingInCylinder_Inner(cylinder, source, flags, &other, anything);
			}
		} else {
			for(const auto & entry : treatio) {
				CheckAnythingInCylinder_Inner(cylinder, source, flags, entry.io, anything);
			}
		}
	}
	
	if(anything == 999999.f) {
		return 0.f;
	}
	
	return anything - cylinder.origin.y;
}

static bool InExceptionList(EntityHandle val) {
	
	for(size_t i = 0; i < EXCEPTIONS_LIST_Pos; i++) {
		if(val == EXCEPTIONS_LIST[i]) {
			return true;
		}
	}
	
	return false;
}

// TODO almost duplicated from CheckAnythingInSphere
static bool CheckEverythingInSphere_Inner(const Sphere & sphere, Entity & entity) {
	
	if(entity.show != SHOW_FLAG_IN_SCENE || InExceptionList(entity.index())) {
		return false;
	}
	
	if(!(entity.ioflags & IO_NPC) && (entity.ioflags & IO_NO_COLLISIONS)) {
		return false;
	}
	
	if(!entity.obj) {
		return false;
	}
	
	if((entity.gameFlags & GFLAG_PLATFORM) &&
	   (entity.bbox3D.max.y <= sphere.origin.y + sphere.radius || entity.bbox3D.min.y >= sphere.origin.y) &&
	   platformCollides(entity, sphere)) {
		return true;
	}
	
	if(!closerThan(entity.pos, sphere.origin, sphere.radius + 500.f)) {
		return false;
	}
	
	size_t amount = 1;
	const auto & vlist = entity.obj->vertexWorldPositions;
	if(entity.obj->grouplist.size() > 4) {
		for(const VertexGroup & group : entity.obj->grouplist) {
			if(closerThan(vlist[group.origin].v, sphere.origin, sphere.radius + 30.f)) {
				return true;
			}
		}
		amount = 2;
	}
	
	for(size_t i = 0; i < entity.obj->facelist.size(); i += amount) {
		const EERIE_FACE & face = entity.obj->facelist[i];
		if(face.facetype & POLY_HIDE) {
			continue;
		}
		Vec3f center = (vlist[face.vid[0]].v + vlist[face.vid[1]].v + vlist[face.vid[2]].v) / 3.f;
		if(closerThan(center, sphere.origin, sphere.radius + 20.f) ||
		   closerThan(vlist[face.vid[0]].v, sphere.origin, sphere.radius + 20.f) ||
		   closerThan(vlist[face.vid[1]].v, sphere.origin, sphere.radius + 20.f) ||
		   closerThan(vlist[face.vid[2]].v, sphere.origin, sphere.radius + 20.f)) {
			return true;
		}
	}
	
	return false;
}

bool CheckEverythingInSphere(const Sphere & sphere, const Entity * source, Entity * target,
                             std::vector<Entity *> & sphereContent) {
	
	if(target) {
		if(target == source || !(target->gameFlags & GFLAG_ISINTREATZONE)) {
			return false;
		}
		if(CheckEverythingInSphere_Inner(sphere, *target)) {
			sphereContent.push_back(target);
			return true;
		}
		return false;
	}
	
	bool found = false;
	for(const auto & entry : treatio) {
		if(!entry.io || entry.io == source) {
			continue;
		}
		if(CheckEverythingInSphere_Inner(sphere, *entry.io)) {
			sphereContent.push_back(entry.io);
			found = true;
		}
	}
	
	return found;
}

const EERIEPOLY * CheckBackgroundInSphere(const Sphere & sphere) {
	
	ARX_PROFILE_FUNC();
	
	u16 radius = u16(sphere.radius * g_tiles->m_mul.x) + 2;
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(sphere.origin), radius)) {
		for(const EERIEPOLY & polygon : tile.polygons()) {
			
			if(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
				continue;
			}
			
			if(IsPolyInSphere(polygon, sphere)) {
				return &polygon;
			}
			
		}
	}
	
	return nullptr;
}

bool platformCollides(const Entity & platform, const Sphere & sphere) {
	
	if(!In3DBBoxTolerance(sphere.origin, platform.bbox3D, sphere.radius) ||
	   !closerThan(getXZ(platform.pos), getXZ(sphere.origin), sphere.radius + 440.f)) {
		return false;
	}
	
	for(const EERIE_FACE & face : platform.obj->facelist) {
		
		EERIEPOLY ep;
		ep.type = 0;
		Vec2f center(0.f);
		for(size_t i = 0; i < std::size(face.vid); i++) {
			ep.v[i].p = platform.obj->vertexWorldPositions[face.vid[i]].v;
			center += getXZ(ep.v[i].p);
		}
		center /= float(std::size(face.vid));
		
		for(size_t i = 0; i < std::size(face.vid); i++) {
			ep.v[i].p.x = (ep.v[i].p.x - center.x) * 3.5f + center.x;
			ep.v[i].p.z = (ep.v[i].p.z - center.y) * 3.5f + center.y;
		}
		
		if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z)) {
			return true;
		}
		
	}
	
	return false;
}

bool CheckAnythingInSphere(const Sphere & sphere, Entity * source, CASFlags flags, Entity ** result) {
	
	ARX_PROFILE_FUNC();
	
	if(result) {
		*result = nullptr;
	}
	
	if(!(flags & CAS_NO_BACKGROUND_COL) && CheckBackgroundInSphere(sphere)) {
		return true;
	}
	
	if(flags & CAS_NO_NPC_COL) {
		return false;
	}
	
	for(const auto & entry : treatio) {
		
		if(entry.show != SHOW_FLAG_IN_SCENE || !entry.io) {
			continue;
		}
		
		Entity & entity = *entry.io;
		if(!entity.obj || &entity == source) {
			continue;
		}
		
		if(!(entity.ioflags & IO_NPC) && (entity.ioflags & IO_NO_COLLISIONS)) {
			continue;
		}
		
		if((flags & CAS_NO_DEAD_COL) && (entity.ioflags & IO_NPC) && IsDeadNPC(entity)) {
			continue;
		}
		
		if((entity.ioflags & IO_FIX) && (flags & CAS_NO_FIX_COL)) {
			continue;
		}
		
		if((entity.ioflags & IO_ITEM) && (flags & CAS_NO_ITEM_COL)) {
			continue;
		}
		
		if(&entity != entities.player()
		   && source != entities.player()
		   && (flags & CAS_NO_SAME_GROUP)
		   && HaveCommonGroup(&entity, source)) {
			continue;
		}
		
		if((entity.gameFlags & GFLAG_PLATFORM) &&
		   (entity.bbox3D.min.y > sphere.origin.y - sphere.radius ||
		    entity.bbox3D.max.y < sphere.origin.y + sphere.radius) &&
		   platformCollides(entity, sphere)) {
			if(result) {
				*result = &entity;
			}
			return true;
		}
		
		if(!closerThan(entity.pos, sphere.origin, sphere.radius + 500.f)) {
			continue;
		}
		
		long amount = 1;
		const auto & vlist = entity.obj->vertexWorldPositions;
		if(entity.obj->grouplist.size() > 4) {
			for(const VertexGroup & group : entity.obj->grouplist) {
				if(closerThan(vlist[group.origin].v, sphere.origin, sphere.radius + 30.f)) {
					if(result) {
						*result = &entity;
					}
					return true;
				}
			}
			amount = 2;
		}
		
		for(size_t i = 0; i < entity.obj->facelist.size(); i += amount) {
			const EERIE_FACE & face = entity.obj->facelist[i];
			if(face.facetype & POLY_HIDE) {
				continue;
			}
			if(closerThan(vlist[face.vid[0]].v, sphere.origin, sphere.radius + 20.f) ||
			   closerThan(vlist[face.vid[1]].v, sphere.origin, sphere.radius + 20.f)) {
				if(result) {
					*result = &entity;
				}
				return true;
			}
		}
		
	}
	
	return false;
}


bool CheckIOInSphere(const Sphere & sphere, const Entity & entity, bool ignoreNoCollisionFlag) {
	
	ARX_PROFILE_FUNC();
	
	if((!ignoreNoCollisionFlag && (entity.ioflags & IO_NO_COLLISIONS)) ||
	   entity.show != SHOW_FLAG_IN_SCENE ||
	   !(entity.gameFlags & GFLAG_ISINTREATZONE) ||
	   !entity.obj) {
		return false;
	}
	
	if(!closerThan(entity.pos, sphere.origin, sphere.radius + 500.f)) {
		return false;
	}
	
	const auto & vlist = entity.obj->vertexWorldPositions;
	
	if(entity.obj->grouplist.size() > 10) {
		long count = 0;
		for(const VertexGroup & group : entity.obj->grouplist) {
			if(closerThan(vlist[group.origin].v, sphere.origin, sphere.radius + 27.f)) {
				count++;
				if(count > 3) {
					return true;
				}
			}
		}
	}
	
	float sr30 = sphere.radius + 22.f;
	long step = 7;
	long nbv = entity.obj->vertexlist.size();
	if(nbv < 150) {
		step = 1;
	} else if(nbv < 300) {
		step = 2;
	} else if(nbv < 600) {
		step = 4;
	} else if(nbv < 1200) {
		step = 6;
	}
	
	long count = 0;
	for(const EERIE_VERTEX & vertex : vlist | boost::adaptors::strided(step)) {
		
		if(closerThan(vertex.v, sphere.origin, sr30)) {
			count++;
			if(count > 6) {
				return true;
			}
		}
		
		if(entity.obj->vertexlist.size() >= 120) {
			continue;
		}
		
		for(const EERIE_VERTEX & other : vlist) {
			if(&other == &vertex) {
				continue;
			}
			for(size_t n = 1; n < 5; n++) {
				if(!fartherThan(sphere.origin, glm::mix(other.v, vertex.v, float(n) * 0.2f), sr30 + 20)) {
					count++;
					if(count > ((entity.ioflags & IO_FIX) ? 3 : 6)) {
						return true;
					}
				}
			}
		}
		
	}
	
	return (count > 3 && (entity.ioflags & IO_FIX));
}

//-----------------------------------------------------------------------------
// Checks if a position is valid, Modify it for height if necessary
// Returns true or false

// TODO copy-paste AttemptValidCylinderPos
bool AttemptValidCylinderPos(Cylinder & cyl, Entity * io, CollisionFlags flags) {
	
	float anything = CheckAnythingInCylinder(cyl, io, flags);
	
	if((flags & CFLAG_LEVITATE) && anything == 0.f) {
		return true;
	}
	
	// Falling Cylinder but valid pos !
	if(anything >= 0.f) {
		if(flags & CFLAG_RETURN_HEIGHT) {
			cyl.origin.y += anything;
		}
		return true;
	}
	
	Cylinder tmp = cyl;
	while(anything < 0.f) {
		tmp.origin.y += anything;
		anything = CheckAnythingInCylinder(tmp, io, flags);
	}
	anything = tmp.origin.y - cyl.origin.y;
	
	if(MOVING_CYLINDER) {
		
		if(flags & CFLAG_NPC) {
			
			float tolerate;
			if((flags & CFLAG_PLAYER) && player.jumpphase != NotJumping) {
				tolerate = 0.f;
			} else if(io && (io->ioflags & IO_NPC) && io->_npcdata->pathfind.listnb > 0
			          && io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb) {
				tolerate = -65.f - io->_npcdata->moveproblem;
			} else if(io && io->_npcdata) {
				tolerate = -55.f - io->_npcdata->moveproblem;
			} else {
				tolerate = 0.f;
			}
			
			if(NPC_IN_CYLINDER) {
				tolerate = cyl.height * 0.5f;
			}
			
			if(anything < tolerate) {
				return false;
			}
			
		}
		
		if(io && (flags & CFLAG_PLAYER) && anything < 0.f && (flags & CFLAG_JUST_TEST)) {
			Cylinder tmpp = cyl;
			tmpp.radius *= 0.7f;
			if(CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST) > 50.f) {
				tmpp.radius = cyl.radius * 1.4f;
				tmpp.origin.y -= 30.f;
				if(CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST) < 0.f) {
					return false;
				}
			}
		}
		
		if(io && !(flags & CFLAG_JUST_TEST) && (flags & CFLAG_PLAYER) && anything < 0.f) {
			
			if(player.jumpphase != NotJumping) {
				io->_npcdata->climb_count = MAX_ALLOWED_CLIMBS_PER_SECOND;
				return false;
			}
			
			float dist = std::max(glm::length(vector2D), 1.f);
			float pente = glm::abs(anything) / dist * 0.5f;
			io->_npcdata->climb_count += pente;
			
			if(io->_npcdata->climb_count > MAX_ALLOWED_CLIMBS_PER_SECOND) {
				io->_npcdata->climb_count = MAX_ALLOWED_CLIMBS_PER_SECOND;
			}
			
			if(anything < -55.f) {
				io->_npcdata->climb_count = MAX_ALLOWED_CLIMBS_PER_SECOND;
				return false;
			}
			
			Cylinder tmpp = cyl;
			tmpp.radius *= 0.65f;
			if(CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST) > 50.f) {
				tmpp.radius = cyl.radius * 1.45f;
				tmpp.origin.y -= 30.f;
				if(CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST) < 0.f) {
					return false;
				}
			}
			
		}
		
	} else if(anything < -45.f) {
		
		return false;
		
	}
	
	tmp = cyl;
	tmp.origin.y += anything;
	anything = CheckAnythingInCylinder(tmp, io, flags);
	
	if(anything < 0.f) {
		if(flags & CFLAG_RETURN_HEIGHT) {
			while(anything < 0.f) {
				tmp.origin.y += anything;
				anything = CheckAnythingInCylinder(tmp, io, flags);
			}
			cyl.origin.y = tmp.origin.y;
		}
		return false;
	}
	
	cyl.origin.y = tmp.origin.y;
	
	return true;
}

// TODO copy-paste Move_Cylinder
bool ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip, Entity * io, float MOVE_CYLINDER_STEP, CollisionFlags flags) {
	
	ON_PLATFORM = 0;
	MOVING_CYLINDER = 1;
	COLLIDED_CLIMB_POLY = 0;
	DIRECT_PATH = true;
	IO_PHYSICS test;
	
	if(ip == nullptr) {
		MOVING_CYLINDER = 0;
		return false;
	}
	
	float distance = glm::distance(ip->startpos, ip->targetpos);
	if(distance < 0.1f) {
		MOVING_CYLINDER = 0;
		return (distance == 0.f);
	}
	
	Vec3f mvector = (ip->targetpos - ip->startpos) / distance;
	
	long count = 100;
	while(distance > 0.f && count--) {
		
		// First We compute current increment
		float curmovedist = std::min(distance, MOVE_CYLINDER_STEP);
		distance -= curmovedist;
		
		// Store our cylinder desc into a test struct
		test = *ip;
		
		// uses test struct to simulate movement.
		vector2D.x = mvector.x * curmovedist;
		vector2D.y = 0.f;
		vector2D.z = mvector.z * curmovedist;
		
		test.cyl.origin.x += vector2D.x;
		test.cyl.origin.y += mvector.y * curmovedist;
		test.cyl.origin.z += vector2D.z;
		
		if(AttemptValidCylinderPos(test.cyl, io, flags)) {
			
			// Found without complication
			*ip = test;
			
		} else {
			
			if(mvector.x == 0.f && mvector.z == 0.f) {
				return true;
			}
			
			if(flags & CFLAG_CLIMBING) {
				test.cyl = ip->cyl;
				test.cyl.origin.y += mvector.y * curmovedist;
				if(AttemptValidCylinderPos(test.cyl, io, flags)) {
					*ip = test;
					continue;
				}
			}
			
			DIRECT_PATH = false;
			// Must Attempt To Slide along collisions
			Vec3f rpos;
			Vec3f lpos;
			long RFOUND = 0;
			long LFOUND = 0;
			long maxRANGLE = 90;
			float ANGLESTEPP;
			
			if(flags & CFLAG_EASY_SLIDING) { // Player sliding in fact...
				ANGLESTEPP = 10.f;
				maxRANGLE = 70;
			} else {
				ANGLESTEPP = 30.f;
			}
			
			float rangle = ANGLESTEPP;
			float langle = 360.f - ANGLESTEPP;
			while(rangle <= maxRANGLE) { // Tries on the right and left sides
				
				test.cyl = ip->cyl;
				float t = MAKEANGLE(rangle);
				Vec3f vecatt = VRotateY(mvector, t);
				test.cyl.origin += vecatt * curmovedist;
				float cc = io->_npcdata->climb_count;
				
				if(AttemptValidCylinderPos(test.cyl, io, flags)) {
					rpos = test.cyl.origin;
					RFOUND = 1;
				} else {
					io->_npcdata->climb_count = cc;
				}
				
				rangle += ANGLESTEPP;
				
				test.cyl = ip->cyl;
				t = MAKEANGLE(langle);
				vecatt = VRotateY(mvector, t);
				test.cyl.origin += vecatt * curmovedist;
				cc = io->_npcdata->climb_count;
				
				if(AttemptValidCylinderPos(test.cyl, io, flags)) {
					lpos = test.cyl.origin;
					LFOUND = 1;
				} else {
					io->_npcdata->climb_count = cc;
				}
				
				langle -= ANGLESTEPP;
				
				if(RFOUND || LFOUND) {
					break;
				}
				
			}
			
			if(LFOUND && RFOUND) {
				langle = 360.f - langle;
				if(langle < rangle) {
					ip->cyl.origin = lpos;
					distance -= curmovedist;
				} else {
					ip->cyl.origin = rpos;
					distance -= curmovedist;
				}
			} else if(LFOUND) {
				ip->cyl.origin = lpos;
				distance -= curmovedist;
			} else if(RFOUND) {
				ip->cyl.origin = rpos;
				distance -= curmovedist;
			} else {
				// Stopped
				ip->velocity = Vec3f(0.f);
				MOVING_CYLINDER = 0;
				return false;
			}
			
		}
		
		if((flags & CFLAG_NO_HEIGHT_MOD) && glm::abs(ip->startpos.y - ip->cyl.origin.y) > 30.f) {
			return false;
		}
		
	}
	
	MOVING_CYLINDER = 0;
	return true;
}

bool IO_Visible(const Vec3f & orgn, const Vec3f & dest, Vec3f * hit) {
	
	ARX_PROFILE_FUNC();
	
	float pas = 35.f;
	
	Vec3f found_hit(0.f);
	EERIEPOLY * found_ep = nullptr;
	float iter;
	
	// Current ray position
	Vec3f tmpPos = orgn;
	
	float nearest =  fdist(orgn, dest);
	if(nearest < pas) {
		pas = nearest * .5f;
	}
	
	// ray incs
	Vec3f d = dest - orgn;
	
	// absolute ray incs
	Vec3f ad = glm::abs(d);
	
	Vec3f i;
	if(ad.x >= ad.y && ad.x >= ad.z) {
		i.x = (ad.x != d.x) ? -pas : pas;
		iter = ad.x / pas;
		float t = 1.f / (iter);
		i.y = d.y * t;
		i.z = d.z * t;
	} else if(ad.y >= ad.x && ad.y >= ad.z) {
		i.y = (ad.y != d.y) ? -pas : pas;
		iter = ad.y / pas;
		float t = 1.f / (iter);
		i.x = d.x * t;
		i.z = d.z * t;
	} else {
		i.z = (ad.z != d.z) ? -pas : pas;
		iter = ad.z / pas;
		float t = 1.f / (iter);
		i.x = d.x * t;
		i.y = d.y * t;
	}
	
	tmpPos -= i;
	
	while(iter > 0.f) {
		
		iter -= 1.f;
		tmpPos += i;
		
		Sphere sphere = Sphere(tmpPos, 65.f);
		
		for(Entity & entity : entities) {
			if(entity.gameFlags & GFLAG_VIEW_BLOCKER) {
				if(CheckIOInSphere(sphere, entity)) {
					float dd = fdist(orgn, sphere.origin);
					if(dd < nearest) {
						*hit = tmpPos;
						return false;
					}
				}
			}
		}
		
		auto tile = g_tiles->getTile(tmpPos);
		if(!tile.valid()) {
			break;
		}
		
		for(EERIEPOLY & polygon : tile.intersectingPolygons()) {
			if(!(polygon.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
			if((polygon.min.y - pas < tmpPos.y) && (polygon.max.y + pas > tmpPos.y))
			if((polygon.min.x - pas < tmpPos.x) && (polygon.max.x + pas > tmpPos.x))
			if((polygon.min.z - pas < tmpPos.z) && (polygon.max.z + pas > tmpPos.z))
			if(RayCollidingPoly(orgn, dest, polygon, hit)) {
				float dd = fdist(orgn, *hit);
				if(dd < nearest) {
					nearest = dd;
					found_ep = &polygon;
					found_hit = *hit;
				}
			}
		}
		
	}
	
	if(!found_ep) {
		return true;
	}
	
	*hit = found_hit;
	
	return false;
}
