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

//-----------------------------------------------------------------------------
// Added immediate return (return anything;)
inline float IsPolyInCylinder(const EERIEPOLY & ep, const Cylinder & cyl, long flag) {

	long flags = flag;
	POLYIN = 0;
	float minf = cyl.origin.y + cyl.height;
	float maxf = cyl.origin.y;

	if(minf > ep.max.y || maxf < ep.min.y)
		return 999999.f;
	
	long to = (ep.type & POLY_QUAD) ? 4 : 3;

	float nearest = 99999999.f;

	for(long num = 0; num < to; num++) {
		float dd = fdist(Vec2f(ep.v[num].p.x, ep.v[num].p.z), Vec2f(cyl.origin.x, cyl.origin.z));
		if(dd < nearest) {
			nearest = dd;
		}
	}
	
	if(nearest > std::max(82.f, cyl.radius))
		return 999999.f;

	if(cyl.radius < 30.f
	   || cyl.height > -80.f
	   || ep.area > 5000.f
	) {
		flags |= CFLAG_EXTRA_PRECISION;
	}

	if(!(flags & CFLAG_EXTRA_PRECISION)) {
		if(ep.area < 100.f)
			return 999999.f;
	}
	
	float anything = 999999.f;

	if(PointInCylinder(cyl, ep.center)) {
		POLYIN = 1;
		
		if(ep.norm.y < 0.5f)
			anything = std::min(anything, ep.min.y);
		else
			anything = std::min(anything, ep.center.y);

		if(!(flags & CFLAG_EXTRA_PRECISION))
			return anything;
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

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}
		}

		if(ep.area > 2000.f || (flags & CFLAG_EXTRA_PRECISION)) {
			Vec3f center = (ep.v[n].p + ep.v[r].p) * 0.5f;
			if(PointInCylinder(cyl, center)) {
				anything = std::min(anything, center.y);
				POLYIN = 1;

				if(!(flags & CFLAG_EXTRA_PRECISION))
					return anything;
			}

			if(ep.area > 4000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (ep.v[n].p + ep.center) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}

			if(ep.area > 6000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (center + ep.v[n].p) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}
		}

		if(PointInCylinder(cyl, ep.v[n].p)) {
			
			anything = std::min(anything, ep.v[n].p.y);
			POLYIN = 1;

			if(!(flags & CFLAG_EXTRA_PRECISION))
				return anything;
		}
		
		r++;
		if(r >= to) {
			r = 0;
		}
		
	}
	
	if(anything != 999999.f && ep.norm.y < 0.1f && ep.norm.y > -0.1f)
		anything = std::min(anything, ep.min.y);
	
	return anything;
}

inline bool IsPolyInSphere(const EERIEPOLY & ep, const Sphere & sph) {
	
	if(ep.area < 100.f) {
		return false;
	}
	
	long to = (ep.type & POLY_QUAD) ? 4 : 3;
	long r = to - 1;
	
	for(long n = 0; n < to; n++) {
		
		if(ep.area > 2000.f) {
			Vec3f center = (ep.v[n].p + ep.v[r].p) * 0.5f;
			if(sph.contains(center)) {
				return true;
			}
			if(ep.area > 4000.f) {
				center = (ep.v[n].p + ep.center) * 0.5f;
				if(sph.contains(center)) {
					return true;
				}
			}
			if(ep.area > 6000.f) {
				center = (center + ep.v[n].p) * 0.5f;
				if(sph.contains(center)) {
					return true;
				}
			}
		}
		
		if(sph.contains(ep.v[n].p)) {
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
	
	if(io != ioo
	   && !(ioo->ioflags & IO_NO_COLLISIONS)
	   && ioo->show == SHOW_FLAG_IN_SCENE
	   && ioo->obj
	) {
		if(ioo->ioflags & IO_NPC) {
			Cylinder cyl = ioo->physics.cyl;
			cyl.radius += 25.f;
			
			for(size_t j = 0; j < io->obj->vertexWorldPositions.size(); j++) {
				if(PointInCylinder(cyl, io->obj->vertexWorldPositions[j].v)) {
					return true;
				}
			}
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
		
		if(!closerThan(Vec2f(entity.pos.x, entity.pos.z), Vec2f(platform.pos.x, platform.pos.z), 450.f)) {
			continue;
		}
		
		float miny = 9999999.f;
		float maxy = -9999999.f;
		for(size_t ii = 0; ii < platform.obj->vertexWorldPositions.size(); ii++) {
			miny = std::min(miny, platform.obj->vertexWorldPositions[ii].v.y);
			maxy = std::max(maxy, platform.obj->vertexWorldPositions[ii].v.y);
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

static bool CollidedFromBack(Entity * io, Entity * ioo) {
	
	// io was collided from back ?
	EERIEPOLY ep;
	ep.type = 0;
	
	if(io && ioo && (io->ioflags & IO_NPC) && (ioo->ioflags & IO_NPC)) {
		
		ep.v[0].p.x = io->pos.x;
		ep.v[0].p.z = io->pos.z;
		
		float ft = glm::radians(135.f + 90.f);
		ep.v[1].p.x =  std::sin(ft) * 180.f;
		ep.v[1].p.z = -std::cos(ft) * 180.f;
		
		ft = glm::radians(225.f + 90.f);
		ep.v[2].p.x =  std::sin(ft) * 180.f;
		ep.v[2].p.z = -std::cos(ft) * 180.f;
		
		float angle = 270.f - io->angle.getYaw();
		Vec3f p1 = VRotateY(ep.v[1].p, angle);
		Vec3f p2 = VRotateY(ep.v[2].p, angle);
		
		ep.v[1].p.x = p1.x + ep.v[0].p.x;
		ep.v[1].p.z = p1.z + ep.v[0].p.z;
		ep.v[2].p.x = p2.x + ep.v[0].p.x;
		ep.v[2].p.z = p2.z + ep.v[0].p.z;
		
		// To keep if we need some visual debug
		if(PointIn2DPolyXZ(&ep, ioo->pos.x, ioo->pos.z)) {
			return true;
		}
		
	}
	
	return false;
}

static void CheckAnythingInCylinder_Inner(const Cylinder & cyl, Entity * ioo, long flags, Entity * io,
                                          float & anything) {
	
	if(!io
	   || io == ioo
	   || !io->obj
	   || io->show != SHOW_FLAG_IN_SCENE
	   || ((io->ioflags & IO_NO_COLLISIONS)  && !(flags & CFLAG_COLLIDE_NOCOL))
	   || fartherThan(io->pos, cyl.origin, 1000.f)) {
		return;
	}
	
	Cylinder & io_cyl = io->physics.cyl;
	io_cyl = getEntityCylinder(*io);
	
	if((io->gameFlags & GFLAG_PLATFORM)
	   || ((flags & CFLAG_COLLIDE_NOCOL) && (io->ioflags & IO_NPC) &&  (io->ioflags & IO_NO_COLLISIONS))) {
		
		if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(cyl.origin.x, cyl.origin.z), 440.f + cyl.radius))
		if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius + 80))
		{
			if(io->ioflags & IO_FIELD) {
				if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius + 10))
					anything = -99999.f;
			} else {
				for(size_t ii = 0; ii < io->obj->vertexWorldPositions.size(); ii++) {
					long res = PointInUnderCylinder(cyl, io->obj->vertexWorldPositions[ii].v);
						if(res > 0) {
						if(res == 2)
							ON_PLATFORM = 1;
						anything = std::min(anything, io->obj->vertexWorldPositions[ii].v.y - 10.f);
					}
				}
				
				for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
					Vec3f c(0.f);
					float height = io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[0]].v.y;
					
					for(long kk = 0; kk < 3; kk++) {
						c.x += io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[kk]].v.x;
						c.y += io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[kk]].v.y;
						c.z += io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[kk]].v.z;
						
						height = std::min(height, io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[kk]].v.y);
					}
					
					c.x *= (1.0f / 3);
					c.z *= (1.0f / 3);
					c.y = io->bbox3D.min.y;
					long res = PointInUnderCylinder(cyl, c);
					if(res > 0) {
						if(res == 2)
							ON_PLATFORM = 1;

						anything = std::min(anything, height);
					}
				}
			}
		}
		
	} else if((io->ioflags & IO_NPC)
	          && !(flags & CFLAG_NO_NPC_COLLIDE) // MUST be checked here only (not before...)
	          && !(ioo && (ioo->ioflags & IO_NO_COLLISIONS))
	          && io->_npcdata->lifePool.current > 0.f) {
		
		if(CylinderInCylinder(cyl, io_cyl)) {
			NPC_IN_CYLINDER = 1;
			anything = std::min(anything, io_cyl.origin.y + io_cyl.height);
			
			if(!(flags & CFLAG_JUST_TEST) && ioo) {
				arx_assert(ioo->ioflags & IO_NPC);
				GameDuration elapsed = g_gameTime.now() - io->collide_door_time;
				if(elapsed > 500ms) {
					
					io->collide_door_time = g_gameTime.now();
					
					if(CollidedFromBack(io, ioo)) {
						SendIOScriptEvent(ioo, io, SM_COLLIDE_NPC, "back");
					} else {
						SendIOScriptEvent(ioo, io, SM_COLLIDE_NPC);
					}
					
					io->collide_door_time = g_gameTime.now();
					
					if(CollidedFromBack(ioo, io)) {
						SendIOScriptEvent(io, ioo, SM_COLLIDE_NPC, "back");
					} else {
						SendIOScriptEvent(io, ioo, SM_COLLIDE_NPC);
					}
					
				}
				
				if(ioo->damager_damages > 0) {
					damageCharacter(*io, ioo->damager_damages, *ioo, nullptr, ioo->damager_type, &io->pos);
				}
				if(io->damager_damages > 0) {
					damageCharacter(*ioo, io->damager_damages, *io, nullptr, io->damager_type, &ioo->pos);
				}
				
				if(io->targetinfo == io->index()) {
					if(io->_npcdata->pathfind.listnb > 0) {
						io->_npcdata->pathfind.listpos = 0;
						io->_npcdata->pathfind.listnb = -1;
						delete[] io->_npcdata->pathfind.list;
						io->_npcdata->pathfind.list = nullptr;
					}
					if(!io->_npcdata->reachedtarget) {
						SendIOScriptEvent(ioo, io, SM_REACHEDTARGET);
						io->_npcdata->reachedtarget = 1;
					}
				}
			}
		}
	} else if(io->ioflags & IO_FIX) {
		
		if(io->bbox3D.max.y <= cyl.origin.y + cyl.height || io->bbox3D.min.y >= cyl.origin.y) {
			return;
		}
		
		Sphere sp;
		
		if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius + 30.f)) {
			const std::vector<EERIE_VERTEX> & vlist = io->obj->vertexWorldPositions;
			
			if(io->obj->grouplist.size() > 10) {
				bool dealt = false;
				for(size_t ii = 0; ii < io->obj->grouplist.size(); ii++) {
					long idx = io->obj->grouplist[ii].origin;
					sp.origin = vlist[idx].v;
					
					if(ioo == entities.player()) {
						sp.radius = 22.f;
					} else if(ioo && !(ioo->ioflags & IO_NPC)) {
						sp.radius = 22.f;
					} else {
						sp.radius = 26.f;
					}
					
					if(SphereInCylinder(cyl, sp)) {
						if(!(flags & CFLAG_JUST_TEST) && ioo) {
							if(io->gameFlags & GFLAG_DOOR) {
								GameDuration elapsed = g_gameTime.now() - io->collide_door_time;
								if(elapsed > 500ms) {
									io->collide_door_time = g_gameTime.now();
									SendIOScriptEvent(ioo, io, SM_COLLIDE_DOOR);
									io->collide_door_time = g_gameTime.now();
									SendIOScriptEvent(io, ioo, SM_COLLIDE_DOOR);
								}
							}

							if(io->ioflags & IO_FIELD) {
								io->collide_door_time = g_gameTime.now();
								SendIOScriptEvent(nullptr, ioo, SM_COLLIDE_FIELD);
							}

							if(!dealt && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
								dealt = true;
								if((io->ioflags & IO_NPC) && ioo->damager_damages > 0) {
									damageCharacter(*io, ioo->damager_damages, *ioo, nullptr, ioo->damager_type, &io->pos);
								}
								if((ioo->ioflags & IO_NPC) && io->damager_damages > 0) {
									damageCharacter(*ioo, io->damager_damages, *io, nullptr, io->damager_type, &ioo->pos);
								}
							}
						}
						anything = std::min(anything, std::min(sp.origin.y - sp.radius, io->bbox3D.min.y));
					}
				}
			} else {
				long step;
				
				if(ioo == entities.player())
					sp.radius = 23.f;
				else if(ioo && !(ioo->ioflags & IO_NPC))
					sp.radius = 32.f;
				else
					sp.radius = 25.f;
				
				size_t nbv = io->obj->vertexlist.size();
				
				if(nbv < 300)
					step = 1;
				else if(nbv < 600)
					step = 2;
				else if(nbv < 1200)
					step = 4;
				else
					step = 6;
				
				bool dealt = false;
				for(size_t ii = 1; ii < nbv; ii += step) {
					if(ii != io->obj->origin) {
						sp.origin = vlist[ii].v;
						
						if(SphereInCylinder(cyl, sp)) {
							if(!(flags & CFLAG_JUST_TEST) && ioo) {
								if(io->gameFlags & GFLAG_DOOR) {
									GameDuration elapsed = g_gameTime.now() - io->collide_door_time;
									if(elapsed > 500ms) {
										io->collide_door_time = g_gameTime.now();
										SendIOScriptEvent(ioo, io, SM_COLLIDE_DOOR);
										io->collide_door_time = g_gameTime.now();
										SendIOScriptEvent(io, ioo, SM_COLLIDE_DOOR);
									}
								}
								
								if(io->ioflags & IO_FIELD) {
									io->collide_door_time = g_gameTime.now();
									SendIOScriptEvent(nullptr, ioo, SM_COLLIDE_FIELD);
								}
								
								if(!dealt && ioo && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
									dealt = true;
									if((io->ioflags & IO_NPC) && ioo->damager_damages > 0) {
										damageCharacter(*io, ioo->damager_damages, *ioo, nullptr, ioo->damager_type, &io->pos);
									}
									if((ioo->ioflags & IO_NPC) && io->damager_damages > 0) {
										damageCharacter(*ioo, io->damager_damages, *io, nullptr, io->damager_type, &ioo->pos);
									}
								}
								
							}
							anything = std::min(anything, std::min(sp.origin.y - sp.radius, io->bbox3D.min.y));
						}
					}
				}
			}
		}
	}
	
}

// Returns 0 if nothing in cyl
// Else returns Y Offset to put cylinder in a proper place
float CheckAnythingInCylinder(const Cylinder & cyl, Entity * ioo, long flags) {
	
	ARX_PROFILE_FUNC();
	
	NPC_IN_CYLINDER = 0;
	
	float anything = 999999.f;
	
	// TODO should this be tilesAround(cyl.origin, cyl.radius)
	u16 radius = u16((cyl.radius + g_backgroundTileSize.x) * g_tiles->m_mul.x);
	for(auto tile : g_tiles->tilesAround(g_tiles->getTile(cyl.origin), radius)) {
		
		float nearest = 99999999.f;
		for(long num = 0; num < 4; num++) {
			float nearx = float(tile.x) * g_backgroundTileSize.x;
			float nearz = float(tile.y) * g_backgroundTileSize.y;
			if(num == 1 || num == 2) {
				nearx += g_backgroundTileSize.x;
			}
			if(num == 2 || num == 3) {
				nearz += g_backgroundTileSize.y;
			}
			float dd = fdist(Vec2f(nearx, nearz), Vec2f(cyl.origin.x, cyl.origin.z));
			if(dd < nearest) {
				nearest = dd;
			}
		}
		
		if(nearest > std::max(82.f, cyl.radius)) {
			continue;
		}
		
		for(const EERIEPOLY & ep : tile.polygons()) {
			
			if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) {
				continue;
			}
			
			if(ep.min.y < anything) {
				anything = std::min(anything, IsPolyInCylinder(ep, cyl, flags));
				if(POLYIN && (ep.type & POLY_CLIMB)) {
					COLLIDED_CLIMB_POLY = 1;
				}
			}
			
		}
		
	}
	
	float tempo;
	EERIEPOLY * ep = CheckInPoly(cyl.origin + Vec3f(0.f, cyl.height, 0.f), &tempo);
	
	if(ep) {
		anything = std::min(anything, tempo);
	}
	
	if(!(flags & CFLAG_NO_INTERCOL)) {
		if(ioo && (ioo->ioflags & IO_NPC) && (ioo->_npcdata->pathfind.flags & PATHFIND_ALWAYS)) {
			for(Entity & other : entities) {
				CheckAnythingInCylinder_Inner(cyl, ioo, flags, &other, anything);
			}
		} else {
			for(size_t i = 0; i < treatio.size(); i++) {
				CheckAnythingInCylinder_Inner(cyl, ioo, flags, treatio[i].io, anything);
			}
		}
	}
	
	if(anything == 999999.f)
		return 0.f;

	anything = anything - cyl.origin.y;
	
	return anything;
}

static bool InExceptionList(EntityHandle val) {
	
	for(size_t i = 0; i < EXCEPTIONS_LIST_Pos; i++) {
		if(val == EXCEPTIONS_LIST[i]) {
			return true;
		}
	}
	
	return false;
}

static bool CheckEverythingInSphere_Inner(const Sphere & sphere, Entity * io,
                                          std::vector<Entity *> & sphereContent) {
	
	float sr30 = sphere.radius + 20.f;
	float sr40 = sphere.radius + 30.f;
	float sr180 = sphere.radius + 500.f;
	
	if(io->show != SHOW_FLAG_IN_SCENE || InExceptionList(io->index())) {
		return false;
	}
	
	if(!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS)) {
		return false;
	}
	
	if(!io->obj) {
		return false;
	}
	
	if(io->gameFlags & GFLAG_PLATFORM) {
		float miny = io->bbox3D.min.y;
		float maxy = io->bbox3D.max.y;

		if(maxy <= sphere.origin.y + sphere.radius || miny >= sphere.origin.y)
		if(In3DBBoxTolerance(sphere.origin, io->bbox3D, sphere.radius))
		{
			if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere.origin.x, sphere.origin.z), 440.f + sphere.radius)) {
				
				EERIEPOLY ep;
				ep.type = 0;
				
				for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
					
					float cx = 0;
					float cz = 0;
					for(long kk = 0; kk < 3; kk++) {
						ep.v[kk].p = io->obj->vertexWorldPositions[io->obj->facelist[ii].vid[kk]].v;
						cx += ep.v[kk].p.x;
						cz += ep.v[kk].p.z;
					}
					cx *= 1.f / 3;
					cz *= 1.f / 3;
					
					for(int kk = 0; kk < 3; kk++) {
						ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
						ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
					}
					
					if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z)) {
						sphereContent.push_back(io);
						return true;
					}
					
				}
			}
		}
	}
	
	if(closerThan(io->pos, sphere.origin, sr180)) {
		long amount = 1;
		const std::vector<EERIE_VERTEX> & vlist = io->obj->vertexWorldPositions;
		
		if(io->obj->grouplist.size() > 4) {
			for(size_t ii = 0; ii < io->obj->grouplist.size(); ii++) {
				if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere.origin, sr40)) {
					sphereContent.push_back(io);
					return true;
				}
			}
			amount = 2;
		}
		
		for(size_t ii = 0; ii < io->obj->facelist.size(); ii += amount) {
			EERIE_FACE * ef = &io->obj->facelist[ii];
			
			if(ef->facetype & POLY_HIDE) {
				continue;
			}
			
			Vec3f fcenter = (vlist[ef->vid[0]].v + vlist[ef->vid[1]].v + vlist[ef->vid[2]].v) * (1.0f / 3);
			
			if(closerThan(fcenter, sphere.origin, sr30)
			   || closerThan(vlist[ef->vid[0]].v, sphere.origin, sr30)
			   || closerThan(vlist[ef->vid[1]].v, sphere.origin, sr30)
			   || closerThan(vlist[ef->vid[2]].v, sphere.origin, sr30)) {
				sphereContent.push_back(io);
				return true;
			}
			
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
		return CheckEverythingInSphere_Inner(sphere, target, sphereContent);
	}
	
	bool vreturn = false;
	
	for(const auto & entry : treatio) {
		
		if(!entry.io || entry.io == source) {
			continue;
		}
		
		if(CheckEverythingInSphere_Inner(sphere, entry.io, sphereContent)) {
			vreturn = true;
		}
		
	}
	
	return vreturn;
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
	   !closerThan(Vec2f(platform.pos.x, platform.pos.z), Vec2f(sphere.origin.x, sphere.origin.z),
	               sphere.radius + 440.f)) {
		return false;
	}
	
	for(const EERIE_FACE & face : platform.obj->facelist) {
		
		EERIEPOLY ep;
		ep.type = 0;
		Vec2f center(0.f);
		for(size_t i = 0; i < std::size(face.vid); i++) {
			ep.v[i].p = platform.obj->vertexWorldPositions[face.vid[i]].v;
			center += Vec2f(ep.v[i].p.x, ep.v[i].p.z);
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

bool CheckAnythingInSphere(const Sphere & sphere, Entity * source, CASFlags flags, Entity  ** result) {
	
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
		const std::vector<EERIE_VERTEX> & vlist = entity.obj->vertexWorldPositions;
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
			if(entity.obj->facelist[i].facetype & POLY_HIDE) {
				continue;
			}
			if(closerThan(vlist[entity.obj->facelist[i].vid[0]].v, sphere.origin, sphere.radius + 20.f) ||
			   closerThan(vlist[entity.obj->facelist[i].vid[1]].v, sphere.origin, sphere.radius + 20.f)) {
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
	
	const std::vector<EERIE_VERTEX> & vlist = entity.obj->vertexWorldPositions;
	
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
	for(size_t ii = 0; ii < vlist.size(); ii += step) {
		
		if(closerThan(vlist[ii].v, sphere.origin, sr30)) {
			count++;
			if(count > 6) {
				return true;
			}
		}
		
		if(entity.obj->vertexlist.size() >= 120) {
			continue;
		}
		
		for(const EERIE_VERTEX & other : vlist) {
			if(&other == &vlist[ii]) {
				continue;
			}
			for(size_t n = 1; n < 5; n++) {
				if(!fartherThan(sphere.origin, glm::mix(other.v, vlist[ii].v, float(n) * 0.2f), sr30 + 20)) {
					count++;
					if(count > ((entity.ioflags & IO_FIX) ? 3 : 6)) {
						return true;
					}
				}
			}
		}
		
	}
	
	if(count > 3 && (entity.ioflags & IO_FIX)) {
		return true;
	}
	
	return false;
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
