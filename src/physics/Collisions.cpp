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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "physics/Collisions.h"

#include "core/GameTime.h"
#include "core/Core.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "physics/Anchors.h"
#include "platform/profiler/Profiler.h"
#include "scene/Interactive.h"


//-----------------------------------------------------------------------------
long ON_PLATFORM=0;
//-----------------------------------------------------------------------------
size_t EXCEPTIONS_LIST_Pos = 0;
EntityHandle EXCEPTIONS_LIST[MAX_IN_SPHERE + 1];

static long POLYIN = 0;
long COLLIDED_CLIMB_POLY=0;
long MOVING_CYLINDER=0;
 
Vec3f vector2D;
bool DIRECT_PATH=true;

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
	
	Vec3f center;
	long n;
	
	for(n = 0; n < to; n++) {
		if(flags & CFLAG_EXTRA_PRECISION) {
			for(long o = 0; o < 5; o++) {
				float p = (float)o * (1.f/5);
				center = ep.v[n].p * p + ep.center * (1.f - p);
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}
		}

		if(ep.area > 2000.f || (flags & CFLAG_EXTRA_PRECISION)) {
			center = (ep.v[n].p + ep.v[r].p) * 0.5f;
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

		if(r >= to)
			r=0;
	}


//	To Add "more" precision

/*if (flags & CFLAG_EXTRA_PRECISION)
{

	for (long j=0;j<360;j+=90)
	{
		float xx=-std::sin(radians((float)j))*cyl.radius;
		float yy=std::cos(radians((float)j))*cyl.radius;
		EERIE_3D pos;
		pos.x=cyl.origin.x+xx;

		pos.z=cyl.origin.z+yy;
		//EERIEPOLY * epp;

		if (PointIn2DPolyXZ(ep, pos.x, pos.z)) 
		{
			if (GetTruePolyY(ep,&pos,&xx))
			{				
				anything=min(anything,xx);
				return anything;
			}
		}
	} 
//}*/
	if(anything != 999999.f && ep.norm.y < 0.1f && ep.norm.y > -0.1f)
		anything = std::min(anything, ep.min.y);

	return anything;
}

inline bool IsPolyInSphere(const EERIEPOLY & ep, const Sphere & sph) {
	
	if(ep.area < 100.f)
		return false;

	long to = (ep.type & POLY_QUAD) ? 4 : 3;

	long r = to - 1;
	Vec3f center;

	for(long n = 0; n < to; n++) {
		if(ep.area > 2000.f) {
			center = (ep.v[n].p + ep.v[r].p) * 0.5f;
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
		
		Vec3f v(ep.v[n].p.x, ep.v[n].p.y, ep.v[n].p.z);

		if(sph.contains(v)) {
			return true;
		}

		r++;

		if(r >= to)
			r = 0;
	}

	return false;
}

bool IsCollidingIO(Entity * io,Entity * ioo) {

	if(ioo != NULL
	   && io != ioo
	   && !(ioo->ioflags & IO_NO_COLLISIONS)
	   && ioo->show == SHOW_FLAG_IN_SCENE
	   && ioo->obj
	) {
		if(ioo->ioflags & IO_NPC) {
			Cylinder cyl = ioo->physics.cyl;
			cyl.radius += 25.f;
			
			for(size_t j = 0; j < io->obj->vertexlist3.size(); j++) {
				if(PointInCylinder(cyl, io->obj->vertexlist3[j].v)) {
					return true;
				}
			}
		}
	}
	
	return false;
}

void PushIO_ON_Top(Entity * ioo, float ydec) {
	
	if(ydec != 0.f)
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(io
		   && io != ioo
		   && !(io->ioflags & IO_NO_COLLISIONS)
		   && io->show == SHOW_FLAG_IN_SCENE
		   && io->obj
		   && !(io->ioflags & (IO_FIX | IO_CAMERA | IO_MARKER))
		) {
			if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(ioo->pos.x, ioo->pos.z), 450.f)) {
				EERIEPOLY ep;
				ep.type = 0;
				float miny = 9999999.f;
				float maxy = -9999999.f;

				for(size_t ii = 0; ii < ioo->obj->vertexlist3.size(); ii++) {
					miny = std::min(miny, ioo->obj->vertexlist3[ii].v.y);
					maxy = std::max(maxy, ioo->obj->vertexlist3[ii].v.y);
				}

				float posy = (io == entities.player()) ? player.basePosition().y : io->pos.y;
				float modd = (ydec > 0) ? -20.f : 0;

				if(posy <= maxy && posy >= miny + modd) {
					for(size_t ii = 0; ii < ioo->obj->facelist.size(); ii++) {
						float cx = 0;
						float cz = 0;

						for(long kk = 0; kk < 3; kk++) {
							ep.v[kk].p = ioo->obj->vertexlist3[ioo->obj->facelist[ii].vid[kk]].v;

							cx += ep.v[kk].p.x;
							cz += ep.v[kk].p.z;
						}

						cx *= (1.f/3);
						cz *= (1.f/3);
							
						float tval=1.1f;

						for(int kk = 0; kk < 3; kk++) {
								ep.v[kk].p.x = (ep.v[kk].p.x - cx) * tval + cx; 
								ep.v[kk].p.z = (ep.v[kk].p.z - cz) * tval + cz; 
						}
						
						if(PointIn2DPolyXZ(&ep, io->pos.x, io->pos.z)) {
							if(io == entities.player()) {
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
									io->pos.y += ydec;
								} else {
									Cylinder cyl = GetIOCyl(io);
									cyl.origin.y += ydec;
									if(CheckAnythingInCylinder(cyl, io ,0) >= 0) {
										io->pos.y += ydec;
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}
}

bool IsAnyNPCInPlatform(Entity * pfrm) {
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(io
		   && io != pfrm
		   && (io->ioflags & IO_NPC)
		   && !(io->ioflags & IO_NO_COLLISIONS)
		   && io->show == SHOW_FLAG_IN_SCENE
		) {
			Cylinder cyl = GetIOCyl(io);

			if(CylinderPlatformCollide(cyl, pfrm) != 0.f)
				return true;
		}
	}

	return false;
}

float CylinderPlatformCollide(const Cylinder & cyl, Entity * io) {
 
	float miny = io->bbox3D.min.y;
	float maxy = io->bbox3D.max.y;
	
	if(maxy <= cyl.origin.y + cyl.height || miny >= cyl.origin.y) {
		return 0.f;
	}
	
	if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius)) {
		return 1.f;
	}
	
	return 0.f;
}

static long NPC_IN_CYLINDER = 0;

static bool CollidedFromBack(Entity * io,Entity * ioo) {
	
	// io was collided from back ?
	EERIEPOLY ep;
	ep.type=0;

	if(io
	   && ioo
	   && (io->ioflags & IO_NPC)
	   && (ioo->ioflags & IO_NPC)
	) {

	ep.v[0].p.x=io->pos.x;
	ep.v[0].p.z=io->pos.z;

	float ft = glm::radians(135.f + 90.f);
	ep.v[1].p.x =  std::sin(ft) * 180.f;
	ep.v[1].p.z = -std::cos(ft) * 180.f;

	ft = glm::radians(225.f + 90.f);
	ep.v[2].p.x =  std::sin(ft) * 180.f;
	ep.v[2].p.z = -std::cos(ft) * 180.f;

	{
	float angle = 270.f - io->angle.getPitch();
	ep.tv[1].p = VRotateY(ep.v[1].p, angle);
	ep.tv[2].p = VRotateY(ep.v[2].p, angle);
	}
	
	ep.v[1].p.x=ep.tv[1].p.x+ep.v[0].p.x;
	ep.v[1].p.z=ep.tv[1].p.z+ep.v[0].p.z;
	ep.v[2].p.x=ep.tv[2].p.x+ep.v[0].p.x;
	ep.v[2].p.z=ep.tv[2].p.z+ep.v[0].p.z;

	// To keep if we need some visual debug
	if(PointIn2DPolyXZ(&ep, ioo->pos.x, ioo->pos.z))
		return true;

	}

	return false;
}

// Returns 0 if nothing in cyl
// Else returns Y Offset to put cylinder in a proper place
float CheckAnythingInCylinder(const Cylinder & cyl, Entity * ioo, long flags) {
	
	ARX_PROFILE_FUNC();
	
	NPC_IN_CYLINDER = 0;
	
	float anything = 999999.f;
	
	// TODO copy-paste background tiles
	int tilex = cyl.origin.x * ACTIVEBKG->Xmul;
	int tilez = cyl.origin.z * ACTIVEBKG->Zmul;
	
	int radius = (cyl.radius + 100) * ACTIVEBKG->Xmul;
	
	short minx = std::max(tilex - radius, 0);
	short maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
	short minz = std::max(tilez - radius, 0);
	short maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);
	
	for(short z = minz; z <= maxz; z++)
	for(short x = minx; x <= maxx; x++) {
		float nearest = 99999999.f;

		for(long num = 0; num < 4; num++) {

			float nearx = static_cast<float>(x * 100);
			float nearz = static_cast<float>(z * 100);

			if(num == 1 || num == 2)
				nearx += 100;

			if(num == 2 || num == 3)
				nearz += 100;

			float dd = fdist(Vec2f(nearx, nearz), Vec2f(cyl.origin.x, cyl.origin.z));

			if(dd < nearest) {
				nearest = dd;
			}
		}

		if(nearest > std::max(82.f, cyl.radius))
			continue;
		
		const EERIE_BKG_INFO & feg = ACTIVEBKG->fastdata[x][z];
		for(long k = 0; k < feg.nbpoly; k++) {
			const EERIEPOLY & ep = feg.polydata[k];

			if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				continue;

			if(ep.min.y < anything) {
				anything = std::min(anything, IsPolyInCylinder(ep, cyl, flags));

				if(POLYIN) {
					if(ep.type & POLY_CLIMB)
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
		Entity * io;
		long FULL_TEST = 0;
		long AMOUNT = TREATZONE_CUR;

		if(ioo
			&& (ioo->ioflags & IO_NPC)
			&& (ioo->_npcdata->pathfind.flags & PATHFIND_ALWAYS))
		{
			FULL_TEST = 1;
			AMOUNT = entities.size();
		}

		for(long i = 0; i < AMOUNT; i++) {
			const EntityHandle handle = EntityHandle(i);
			
			if(FULL_TEST) {
				io = entities[handle];
			} else {
				io = treatio[i].io;
			}

			if(!io
				|| io == ioo
				|| !io->obj
				||	(	(io->show!=SHOW_FLAG_IN_SCENE)
					||	((io->ioflags & IO_NO_COLLISIONS)  && !(flags & CFLAG_COLLIDE_NOCOL))
					) 
				|| fartherThan(io->pos, cyl.origin, 1000.f)) continue;
	
			{
				Cylinder & io_cyl = io->physics.cyl;
				io_cyl = GetIOCyl(io);
				float dealt = 0;

				if (	(io->gameFlags & GFLAG_PLATFORM)
					||	((flags & CFLAG_COLLIDE_NOCOL) && (io->ioflags & IO_NPC) &&  (io->ioflags & IO_NO_COLLISIONS))
					)
				{
					if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(cyl.origin.x, cyl.origin.z), 440.f + cyl.radius))
					if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius+80))
					{
						if(io->ioflags & IO_FIELD) {
							if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius + 10))
								anything = -99999.f;
						} else {
						for(size_t ii = 0; ii < io->obj->vertexlist3.size(); ii++) {
							long res = PointInUnderCylinder(cyl, io->obj->vertexlist3[ii].v);

							if(res > 0) {
								if(res == 2)
									ON_PLATFORM = 1;

								anything = std::min(anything, io->obj->vertexlist3[ii].v.y - 10.f);
							}			
						}

						for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
							Vec3f c = Vec3f_ZERO;
							float height = io->obj->vertexlist3[io->obj->facelist[ii].vid[0]].v.y;

							for(long kk = 0; kk < 3; kk++) {
								c.x += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.x;
								c.y += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y;
								c.z += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.z;

								height = std::min(height, io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y);
							}

							c.x *= ( 1.0f / 3 );
							c.z *= ( 1.0f / 3 );
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
				}
				else if (	(io->ioflags & IO_NPC)
						&&	(!(flags & CFLAG_NO_NPC_COLLIDE)) // MUST be checked here only (not before...)
						&&	(!(ioo && (ioo->ioflags & IO_NO_COLLISIONS)))	
						&&	(io->_npcdata->lifePool.current>0.f)
						)
				{
					
					if(CylinderInCylinder(cyl, io_cyl)) {
 						NPC_IN_CYLINDER = 1;
						anything = std::min(anything, io_cyl.origin.y + io_cyl.height);

						if(!(flags & CFLAG_JUST_TEST) && ioo) {
							float elapsed = arxtime.now_f() - io->collide_door_time;
							if(elapsed > 500) {
								EVENT_SENDER = ioo;
								io->collide_door_time = arxtime.now_ul(); 	

								if(CollidedFromBack(io, ioo))
									SendIOScriptEvent(io, SM_COLLIDE_NPC, "back");
								else
									SendIOScriptEvent(io, SM_COLLIDE_NPC);

								EVENT_SENDER = io;
								io->collide_door_time = arxtime.now_ul(); 

								if(CollidedFromBack(ioo, io))
									SendIOScriptEvent(ioo, SM_COLLIDE_NPC, "back");
								else
									SendIOScriptEvent(ioo, SM_COLLIDE_NPC);
							}

							if(!dealt && (ioo->damager_damages > 0 || io->damager_damages > 0)) {

								if(ioo->damager_damages > 0)
									ARX_DAMAGES_DealDamages(EntityHandle(i), ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);

								if(io->damager_damages > 0)
									ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
							}
							
							if(io->targetinfo == handle) {
								if(io->_npcdata->pathfind.listnb > 0) {
									io->_npcdata->pathfind.listpos = 0;
									io->_npcdata->pathfind.listnb = -1;

									free(io->_npcdata->pathfind.list);
									io->_npcdata->pathfind.list = NULL;

									SendIOScriptEvent(io, SM_NULL, "", "pathfinder_end");
								}
								if(!io->_npcdata->reachedtarget) {
									EVENT_SENDER = ioo;
									SendIOScriptEvent(io, SM_REACHEDTARGET);
									io->_npcdata->reachedtarget = 1;
								}
							}
						}
					}
				} else if(io->ioflags & IO_FIX) {
					Sphere sp;

					float miny = io->bbox3D.min.y;
					float maxy = io->bbox3D.max.y;

					if(maxy <= cyl.origin.y + cyl.height)
						goto suivant;

					if(miny >= cyl.origin.y)
						goto suivant;

					if(In3DBBoxTolerance(cyl.origin, io->bbox3D, cyl.radius + 30.f)) {
						std::vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;
						
						if(io->obj->grouplist.size() > 10) {
							for(size_t ii = 0; ii < io->obj->grouplist.size(); ii++) {
								long idx = io->obj->grouplist[ii].origin;
								sp.origin = vlist[idx].v;

								if(ioo == entities.player()) {
									sp.radius = 22.f; 
								} else if(ioo->ioflags & IO_NPC) {
									sp.radius = 26.f;
								} else {
									sp.radius = 22.f;
								}

								if(SphereInCylinder(cyl, sp)) {
									if(!(flags & CFLAG_JUST_TEST) && ioo) {
										if(io->gameFlags & GFLAG_DOOR) {
											float elapsed = arxtime.now_f() - io->collide_door_time;
											if(elapsed > 500) {
												EVENT_SENDER = ioo;
												io->collide_door_time = arxtime.now_ul(); 	
												SendIOScriptEvent(io, SM_COLLIDE_DOOR);

												EVENT_SENDER = io;
												io->collide_door_time = arxtime.now_ul(); 	
												SendIOScriptEvent(ioo, SM_COLLIDE_DOOR);
											}
										}

										if(io->ioflags & IO_FIELD) {
											EVENT_SENDER = NULL;
											io->collide_door_time = arxtime.now_ul(); 	
											SendIOScriptEvent(ioo, SM_COLLIDE_FIELD);
										}

										if(!dealt && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
											dealt = 1;

											if(ioo->damager_damages > 0)
												ARX_DAMAGES_DealDamages(EntityHandle(i), ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);

											if(io->damager_damages > 0)
												ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
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

							for(size_t ii = 1; ii < nbv; ii += step) {
								if(ii != io->obj->origin) {
									sp.origin = vlist[ii].v;
									
									if(SphereInCylinder(cyl, sp)) {
										if(!(flags & CFLAG_JUST_TEST) && ioo) {
											if(io->gameFlags & GFLAG_DOOR) {
												float elapsed = arxtime.now_f() - io->collide_door_time;
												if(elapsed > 500) {
													EVENT_SENDER = ioo;
													io->collide_door_time = arxtime.now_ul(); 	
													SendIOScriptEvent(io, SM_COLLIDE_DOOR);

													EVENT_SENDER = io;
													io->collide_door_time = arxtime.now_ul(); 	
													SendIOScriptEvent(ioo, SM_COLLIDE_DOOR);
												}
											}

										if(io->ioflags & IO_FIELD) {
											EVENT_SENDER = NULL;
											io->collide_door_time = arxtime.now_ul();
											SendIOScriptEvent(ioo, SM_COLLIDE_FIELD);
										}
					
										if(!dealt && ioo && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
											dealt = 1;
											
											if(ioo->damager_damages > 0)
												ARX_DAMAGES_DealDamages(EntityHandle(i), ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);
									
											if(io->damager_damages > 0)
												ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
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
		suivant:
			;
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

bool CheckEverythingInSphere(const Sphere & sphere, EntityHandle source, EntityHandle targ, std::vector<EntityHandle> & sphereContent) //except source...
{
	bool vreturn = false;
	
	Entity * io;
	EntityHandle ret_idx = EntityHandle();
	
	float sr30 = sphere.radius + 20.f;
	float sr40 = sphere.radius + 30.f;
	float sr180 = sphere.radius + 500.f;

	for(long i = 0; i < TREATZONE_CUR; i++) {
		if(ValidIONum(targ)) {
			i = TREATZONE_CUR;
			io = entities[targ];

			if(!io
			   || InExceptionList(targ)
			   || targ == source
			   || io->show != SHOW_FLAG_IN_SCENE
			   || !(io->gameFlags & GFLAG_ISINTREATZONE)
			   || !(io->obj)
			) {
				return false;
			}

			ret_idx = targ;
		} else {
			if(treatio[i].show != 1
			   || treatio[i].io == NULL
			   || treatio[i].io->index() == source
			   || InExceptionList(treatio[i].io->index())
			) {
				continue;
			}

			io = treatio[i].io;
			ret_idx = treatio[i].io->index();
		}

		if(!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS))
			continue;

		if(!io->obj)
			continue;

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
							ep.v[kk].p = io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v;

							cx += ep.v[kk].p.x;
							cz += ep.v[kk].p.z;
						}

						cx *= (1.f/3);
						cz *= (1.f/3);

						for(int kk = 0; kk < 3; kk++) {
							ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
							ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
						}

						if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z)) {
							sphereContent.push_back(ret_idx);

							vreturn = true;
							goto suivant;
						}
					}
				}
			}
		}

		if(closerThan(io->pos, sphere.origin, sr180)) {

			long amount = 1;
			std::vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->obj->grouplist.size() > 4) {
				for(size_t ii = 0; ii < io->obj->grouplist.size(); ii++) {
					if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere.origin, sr40)) {

						sphereContent.push_back(ret_idx);

						vreturn = true;
						goto suivant;
					}
				}

				amount=2;
			}

			for(size_t ii = 0; ii < io->obj->facelist.size(); ii += amount) {
				EERIE_FACE * ef = &io->obj->facelist[ii];

				if(ef->facetype & POLY_HIDE)
					continue;

				Vec3f fcenter = (vlist[ef->vid[0]].v + vlist[ef->vid[1]].v
								 + vlist[ef->vid[2]].v) * (1.0f / 3);

				if(closerThan(fcenter, sphere.origin, sr30)
				   || closerThan(vlist[ef->vid[0]].v, sphere.origin, sr30)
				   || closerThan(vlist[ef->vid[1]].v, sphere.origin, sr30)
				   || closerThan(vlist[ef->vid[2]].v, sphere.origin, sr30)) {

					sphereContent.push_back(ret_idx);

					vreturn = true;
					goto suivant;
				}
			}
		}

	suivant:
	  ;
		
	}	

	return vreturn;	
}

//except source...
const EERIEPOLY * CheckBackgroundInSphere(const Sphere & sphere) {
	
	// TODO copy-paste background tiles
	short tilex = sphere.origin.x * ACTIVEBKG->Xmul;
	short tilez = sphere.origin.z * ACTIVEBKG->Zmul;
	short radius = (sphere.radius * ACTIVEBKG->Xmul) + 2;

	short minx = std::max(tilex - radius, 0);
	short maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
	short minz = std::max(tilez - radius, 0);
	short maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);

	for(short z = minz; z <= maxz; z++)
	for(short x = minx; x <= maxx; x++) {
		const EERIE_BKG_INFO & feg = ACTIVEBKG->fastdata[x][z];

		for(long k = 0; k < feg.nbpoly; k++) {
			const EERIEPOLY & ep = feg.polydata[k];

			if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				continue;

			if(IsPolyInSphere(ep, sphere)) {
				return &ep;
			}			
		}
	}	
	
	return NULL;	
}

bool CheckAnythingInSphere(const Sphere & sphere, EntityHandle source, CASFlags flags, EntityHandle * num) //except source...
{
	ARX_PROFILE_FUNC();
	
	if(num)
		*num = EntityHandle();
	
	if(!(flags & CAS_NO_BACKGROUND_COL)) {
		ARX_PROFILE("Background Collision");
		
		// TODO copy-paste background tiles
		short tilex = sphere.origin.x * ACTIVEBKG->Xmul;
		short tilez = sphere.origin.z * ACTIVEBKG->Zmul;
		short radius = (sphere.radius * ACTIVEBKG->Xmul) + 2;
		
		short minx = std::max(tilex - radius, 0);
		short maxx = std::min(tilex + radius, ACTIVEBKG->Xsize - 1);
		short minz = std::max(tilez - radius, 0);
		short maxz = std::min(tilez + radius, ACTIVEBKG->Zsize - 1);

		for(short z = minz; z <= maxz; z++)
		for(short x = minx; x <= maxx; x++) {
			const EERIE_BKG_INFO & feg = ACTIVEBKG->fastdata[x][z];
			for(long k = 0; k < feg.nbpoly; k++) {
				const EERIEPOLY & ep = feg.polydata[k];

				if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
					continue;

				if(IsPolyInSphere(ep, sphere))
					return true;
			}
		}	
	}

	if(flags & CAS_NO_NPC_COL)
		return false;

	bool validsource = false;

	if(flags & CAS_NO_SAME_GROUP)
		validsource = ValidIONum(source);

	float sr30 = sphere.radius + 20.f;
	float sr40 = sphere.radius + 30.f;
	float sr180 = sphere.radius + 500.f;

	for(long i = 0; i < TREATZONE_CUR; i++) {
		
		if(treatio[i].show != 1 || !treatio[i].io || treatio[i].io->index() == source)
			continue;

		Entity * io = treatio[i].io;

		if(!io->obj)
			continue;

		if(!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS))
			continue;

		if((flags & CAS_NO_DEAD_COL) && (io->ioflags & IO_NPC) && IsDeadNPC(io))
			continue;

		if((io->ioflags & IO_FIX) && (flags & CAS_NO_FIX_COL))
			continue;

		if((io->ioflags & IO_ITEM) && (flags & CAS_NO_ITEM_COL))
			continue;

		if(treatio[i].io->index() != PlayerEntityHandle && source != PlayerEntityHandle && validsource && HaveCommonGroup(io,entities[source]))
			continue;

		if(io->gameFlags & GFLAG_PLATFORM) {
			float miny = io->bbox3D.min.y;
			float maxy = io->bbox3D.max.y;

			if(maxy > sphere.origin.y - sphere.radius || miny < sphere.origin.y + sphere.radius)
			if(In3DBBoxTolerance(sphere.origin, io->bbox3D, sphere.radius))
			{
				if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere.origin.x, sphere.origin.z), 440.f + sphere.radius)) {

					EERIEPOLY ep;
					ep.type = 0;

					for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
						float cx = 0;
						float cz = 0;

						for(long kk = 0; kk < 3; kk++) {
							ep.v[kk].p = io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v;

							cx += ep.v[kk].p.x;
							cz += ep.v[kk].p.z;
						}

						cx *= (1.f/3);
						cz *= (1.f/3);

						for(int kk = 0; kk < 3; kk++) {
							ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
							ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
						}

						if(PointIn2DPolyXZ(&ep, sphere.origin.x, sphere.origin.z)) {
							if(num)
								*num=treatio[i].io->index();

							return true;
						}
					}
				}
			}
		}

		if(closerThan(io->pos, sphere.origin, sr180)) {
			long amount = 1;
			std::vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->obj->grouplist.size() > 4) {
				for(size_t ii = 0; ii < io->obj->grouplist.size(); ii++) {
					if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere.origin, sr40)) {
						if(num)
							*num = treatio[i].io->index();

						return true;
					}
				}

				amount = 2;
			}

			for(size_t ii = 0; ii < io->obj->facelist.size(); ii += amount) {

				if(io->obj->facelist[ii].facetype & POLY_HIDE)
					continue;

				if(closerThan(vlist[io->obj->facelist[ii].vid[0]].v, sphere.origin, sr30)
				   || closerThan(vlist[io->obj->facelist[ii].vid[1]].v, sphere.origin, sr30)) {
					if(num)
						*num = treatio[i].io->index();

					return true;
				}
			}
		}
	}

	return false;	
}


bool CheckIOInSphere(const Sphere & sphere, const Entity & entity, bool ignoreNoCollisionFlag) {
	
	ARX_PROFILE_FUNC();
	
	float sr30 = sphere.radius + 22.f;
	float sr40 = sphere.radius + 27.f;
	float sr180 = sphere.radius + 500.f;

	if((ignoreNoCollisionFlag || !(entity.ioflags & IO_NO_COLLISIONS))
	   && (entity.show == SHOW_FLAG_IN_SCENE)
	   && (entity.gameFlags & GFLAG_ISINTREATZONE)
	   && (entity.obj)
	) {
		if(closerThan(entity.pos, sphere.origin, sr180)) {
			std::vector<EERIE_VERTEX> & vlist = entity.obj->vertexlist3;

			if(entity.obj->grouplist.size()>10) {
				long count=0;
				long ii=entity.obj->grouplist.size()-1;

				while(ii) {
					if(closerThan(vlist[entity.obj->grouplist[ii].origin].v, sphere.origin, sr40)) {
						count++;

						if(count>3)
							return true;
					}

					ii--;
				}
			}
			
			long step;
			long nbv = entity.obj->vertexlist.size();

			if(nbv < 150)
				step = 1;
			else if(nbv < 300)
				step = 2;
			else if(nbv < 600)
				step = 4;
			else if(nbv < 1200)
				step = 6;
			else
				step = 7;
			
			long count=0;
			
			for(size_t ii = 0; ii < vlist.size(); ii += step) {
				if(closerThan(vlist[ii].v, sphere.origin, sr30)) {
					count++;

					if(count > 6)
						return true;
				}

				if(entity.obj->vertexlist.size() < 120) {
					for(size_t kk = 0; kk < vlist.size(); kk+=1) {
						if(kk != ii) {
							for(float nn = 0.2f; nn < 1.f; nn += 0.2f) {
								Vec3f posi = vlist[ii].v * nn + vlist[kk].v * (1.f - nn);
								if(!fartherThan(sphere.origin, posi, sr30 + 20)) {
									count++;

									if(count > 3) {
										if(entity.ioflags & IO_FIX)
											return true;

										if(count > 6)
											return true;
									}
								}
							}
						}
					}
				}
			}

			if(count > 3 && (entity.ioflags & IO_FIX))
				return true;
		}
	}
	
	return false;	
}


float MAX_ALLOWED_PER_SECOND=12.f;
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
		if(flags & CFLAG_RETURN_HEIGHT)
			cyl.origin.y += anything;

		return true;
	}
	
	Cylinder tmp;

	if(!(flags & CFLAG_ANCHOR_GENERATION)) {
		
		tmp = cyl;

		while(anything < 0.f) {
			tmp.origin.y += anything;
			anything = CheckAnythingInCylinder(tmp, io, flags);
		}

		anything = tmp.origin.y - cyl.origin.y;
	}

	if(MOVING_CYLINDER) {
		if(flags & CFLAG_NPC) {
			float tolerate;
			
			if((flags & CFLAG_PLAYER) && player.jumpphase != NotJumping) {
				tolerate = 0;
			} else if(io
					&& (io->ioflags & IO_NPC)
					&& io->_npcdata->pathfind.listnb > 0
					&& io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb
			) {
				tolerate = -65 - io->_npcdata->moveproblem;
			} else {
				if(io && io->_npcdata) {
					tolerate = -55 - io->_npcdata->moveproblem; 
				} else {
					tolerate = 0.f;
				}
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

			float tmp = CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST);

			if(tmp > 50.f) {
				tmpp.radius = cyl.radius * 1.4f;
				tmpp.origin.y -= 30.f;
				float tmp = CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST);

				if(tmp < 0)
					return false;
			}
		}

		if(io && !(flags & CFLAG_JUST_TEST)) {
			if((flags & CFLAG_PLAYER) && anything < 0.f) {
				
				if(player.jumpphase != NotJumping) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return false;
				}

				float dist = std::max(glm::length(vector2D), 1.f);
				float pente = glm::abs(anything) / dist * ( 1.0f / 2 );
				io->_npcdata->climb_count += pente;

				if(io->_npcdata->climb_count > MAX_ALLOWED_PER_SECOND) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
				}

				if(anything < -55) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return false;
				}

				Cylinder tmpp = cyl;
				tmpp.radius *= 0.65f; 
				float tmp = CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST);

				if(tmp > 50.f) {
					tmpp.radius = cyl.radius * 1.45f;
					tmpp.origin.y -= 30.f;
					float tmp = CheckAnythingInCylinder(tmpp, io, flags | CFLAG_JUST_TEST);

					if(tmp < 0)
						return false;
				}	
			}			
		}
	} else if(anything < -45) {
		return false;
	}

	if((flags & CFLAG_SPECIAL) && anything < -40) {
		if(flags & CFLAG_RETURN_HEIGHT)
			cyl.origin.y += anything;

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
bool ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip, Entity * io, float MOVE_CYLINDER_STEP, CollisionFlags flags)
{
//	HERMESPerf script(HPERF_PHYSICS);
//	+5 on 15
//	
//	memcpy(&ip->cyl.origin,&ip->targetpos,sizeof(EERIE_3D));
//	return true;

	ON_PLATFORM = 0;
	MOVING_CYLINDER = 1;
	COLLIDED_CLIMB_POLY = 0;
	DIRECT_PATH = true;
	IO_PHYSICS test;

	if(ip == NULL) {
		MOVING_CYLINDER = 0;
		return false;
	}

	float distance = glm::distance(ip->startpos, ip->targetpos);

	if(distance < 0.1f) {
		MOVING_CYLINDER = 0;

		if(distance == 0.f)
			return true;

		return false;
	}

	Vec3f mvector = (ip->targetpos - ip->startpos) / distance;
	long count=100;

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
		
		if((flags & CFLAG_CHECK_VALID_POS) && CylinderAboveInvalidZone(test.cyl))
				return false;

		if(AttemptValidCylinderPos(test.cyl,io,flags)) {
			// Found without complication
			*ip = test;
		} else {
			//return false;
			if(mvector.x == 0.f && mvector.z == 0.f)
				return true;
			
			if(flags & CFLAG_CLIMBING) {
				test.cyl = ip->cyl;
				test.cyl.origin.y += mvector.y * curmovedist;

				if(AttemptValidCylinderPos(test.cyl, io, flags)) {
					*ip = test;
					goto oki;
				}
			}

			DIRECT_PATH = false;
			// Must Attempt To Slide along collisions
			Vec3f vecatt;
			Vec3f rpos;
			Vec3f lpos;
			long RFOUND = 0;
			long LFOUND = 0;
			long maxRANGLE = 90;
			float ANGLESTEPP;

			if(flags & CFLAG_EASY_SLIDING) { // player sliding in fact...
				ANGLESTEPP = 10.f;
				maxRANGLE = 70;
			} else {
				ANGLESTEPP = 30.f;
			}

			float rangle = ANGLESTEPP;
			float langle = 360.f - ANGLESTEPP;


			while(rangle <= maxRANGLE) { //tries on the Right and Left sides
				test.cyl = ip->cyl;
				float t = MAKEANGLE(rangle);
				vecatt = VRotateY(mvector, t);
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

				if(RFOUND || LFOUND)
					break;
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
				//stopped
				ip->velocity = Vec3f_ZERO;
				MOVING_CYLINDER = 0;
				return false;
			}
		}

		if(flags & CFLAG_NO_HEIGHT_MOD) {
			if(glm::abs(ip->startpos.y - ip->cyl.origin.y) > 30.f)
				return false;
		}

	oki:
		;
	}

	MOVING_CYLINDER = 0;
	return true;
}

// TODO visible copy-paste
bool IO_Visible(const Vec3f & orgn, const Vec3f & dest, Vec3f * hit)
{
	ARX_PROFILE_FUNC();
	
	Vec3f i;
	float pas = 35.f;

	Vec3f found_hit = Vec3f_ZERO;
	EERIEPOLY *found_ep = NULL;
	float iter;
	
	//current ray pos
	Vec3f tmpPos = orgn;
	
	float distance;
	float nearest = distance = fdist(orgn, dest);

	if(distance < pas)
		pas = distance * .5f;

	// ray incs
	Vec3f d = dest - orgn;
	
	// absolute ray incs
	Vec3f ad = glm::abs(d);
	
	if(ad.x >= ad.y && ad.x >= ad.z) {
		if(ad.x != d.x)
			i.x = -pas;
		else
			i.x = pas;

		iter = ad.x / pas;
		float t = 1.f / (iter);
		i.y = d.y * t;
		i.z = d.z * t;
	} else if(ad.y >= ad.x && ad.y >= ad.z) {
		if(ad.y != d.y)
			i.y = -pas;
		else
			i.y = pas;

		iter = ad.y / pas;
		float t = 1.f / (iter);
		i.x = d.x * t;
		i.z = d.z * t;
	} else {
		if(ad.z != d.z)
			i.z = -pas;
		else
			i.z = pas;

		iter = ad.z / pas;
		float t = 1.f / (iter);
		i.x = d.x * t;
		i.y = d.y * t;
	}
	
	tmpPos.x -= i.x;
	tmpPos.y -= i.y;
	tmpPos.z -= i.z;

	while(iter > 0.f) {
		iter -= 1.f;
		tmpPos.x += i.x;
		tmpPos.y += i.y;
		tmpPos.z += i.z;

		Sphere sphere = Sphere(tmpPos, 65.f);
		
		for(size_t num = 0; num < entities.size(); num++) {
			const EntityHandle handle = EntityHandle(num);
			Entity * io = entities[handle];

			if(io && (io->gameFlags & GFLAG_VIEW_BLOCKER)) {
				if(CheckIOInSphere(sphere, *io)) {
					float dd = fdist(orgn, sphere.origin);

					if(dd < nearest) {
						hit->x=tmpPos.x;
						hit->y=tmpPos.y;
						hit->z=tmpPos.z;
						return false;
					}
				}
			}
		}

		long px = (long)(tmpPos.x * ACTIVEBKG->Xmul);
		long pz = (long)(tmpPos.z * ACTIVEBKG->Zmul);

		if(px < 0 || px >= ACTIVEBKG->Xsize || pz < 0 || pz >= ACTIVEBKG->Zsize)
			break;

		EERIE_BKG_INFO * eg = &ACTIVEBKG->fastdata[px][pz];

		for(long k = 0; k < eg->nbpolyin; k++) {
			EERIEPOLY * ep = eg->polyin[k];

			if(!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
			if((ep->min.y - pas < tmpPos.y) && (ep->max.y + pas > tmpPos.y))
			if((ep->min.x - pas < tmpPos.x) && (ep->max.x + pas > tmpPos.x))
			if((ep->min.z - pas < tmpPos.z) && (ep->max.z + pas > tmpPos.z))
			if(RayCollidingPoly(orgn, dest, *ep, hit)) {
				float dd = fdist(orgn, *hit);
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
	
	*hit = found_hit;

	return false;
}

void ANCHOR_BLOCK_Clear() {

	EERIE_BACKGROUND * eb = ACTIVEBKG;

	if(!eb)
		return;

	for(long k = 0; k < eb->nbanchors; k++) {
		ANCHOR_DATA & ad = eb->anchors[k];
		ad.flags &= ~ANCHOR_FLAG_BLOCKED;
	}
}

void ANCHOR_BLOCK_By_IO(Entity * io, long status) {

	EERIE_BACKGROUND * eb = ACTIVEBKG;

	for(long k = 0; k < eb->nbanchors; k++) {
		ANCHOR_DATA & ad = eb->anchors[k];

		if(fartherThan(ad.pos, io->pos, 600.f))
			continue;

		if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(ad.pos.x, ad.pos.z), 440.f)) {
			
			EERIEPOLY ep;
			ep.type = 0;

			for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
				float cx = 0;
				float cz = 0;

				for(long kk = 0; kk < 3; kk++) {
					ep.v[kk].p = io->obj->vertexlist[io->obj->facelist[ii].vid[kk]].v + io->pos;

					cx += ep.v[kk].p.x;
					cz += ep.v[kk].p.z;
				}

				cx *= (1.f/3);
				cz *= (1.f/3);

				for(int kk = 0; kk < 3; kk++) {
					ep.v[kk].p.x = (ep.v[kk].p.x - cx) * 3.5f + cx;
					ep.v[kk].p.z = (ep.v[kk].p.z - cz) * 3.5f + cz;
				}

				if(PointIn2DPolyXZ(&ep, ad.pos.x, ad.pos.z)) {
					if(status)
						ad.flags |= ANCHOR_FLAG_BLOCKED;
					else
						ad.flags &= ~ANCHOR_FLAG_BLOCKED;
				}
			}
		}
	}					
}
