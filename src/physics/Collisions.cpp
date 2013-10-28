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
#include "scene/Interactive.h"

using std::min;
using std::max;
using std::vector;

//-----------------------------------------------------------------------------
extern float framedelay;
long ON_PLATFORM=0;
//-----------------------------------------------------------------------------
size_t EXCEPTIONS_LIST_Pos = 0;
short EXCEPTIONS_LIST[MAX_IN_SPHERE + 1];

static long POLYIN = 0;
long COLLIDED_CLIMB_POLY=0;
long MOVING_CYLINDER=0;
 
Vec3f vector2D;
bool DIRECT_PATH=true;

//-----------------------------------------------------------------------------
// Added immediate return (return anything;)
inline float IsPolyInCylinder(EERIEPOLY * ep, EERIE_CYLINDER * cyl, long flag) {

	long flags = flag;
	POLYIN = 0;
	float minf = cyl->origin.y + cyl->height;
	float maxf = cyl->origin.y;

	if(minf > ep->max.y || maxf < ep->min.y)
		return 999999.f;
	
	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	float nearest = 99999999.f;

	for(long num = 0; num < to; num++) {
		float dd = fdist(Vec2f(ep->v[num].p.x, ep->v[num].p.z), Vec2f(cyl->origin.x, cyl->origin.z));

		if(dd < nearest) {
			nearest = dd;
		}		
	}

	if(nearest > max(82.f, cyl->radius))
		return 999999.f;

	if(cyl->radius < 30.f
	   || cyl->height > -80.f
	   || ep->area > 5000.f
	) {
		flags |= CFLAG_EXTRA_PRECISION;
	}

	if(!(flags & CFLAG_EXTRA_PRECISION)) {
		if(ep->area < 100.f)
			return 999999.f;
	}
	
	float anything = 999999.f;

	if(PointInCylinder(cyl, &ep->center)) {
		POLYIN = 1;
		
		if(ep->norm.y < 0.5f)
			anything = min(anything, ep->min.y);
		else
			anything = min(anything, ep->center.y);

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
				center = ep->v[n].p * p + ep->center * (1.f - p);
				if(PointInCylinder(cyl, &center)) {
					anything = min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}
		}

		if(ep->area > 2000.f || (flags & CFLAG_EXTRA_PRECISION)) {
			center = (ep->v[n].p + ep->v[r].p) * 0.5f;
			if(PointInCylinder(cyl, &center)) {
				anything = min(anything, center.y);
				POLYIN = 1;

				if(!(flags & CFLAG_EXTRA_PRECISION))
					return anything;
			}

			if(ep->area > 4000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (ep->v[n].p + ep->center) * 0.5f;
				if(PointInCylinder(cyl, &center)) {
					anything = min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}

			if(ep->area > 6000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (center + ep->v[n].p) * 0.5f;
				if(PointInCylinder(cyl, &center)) {
					anything = min(anything, center.y);
					POLYIN = 1;

					if(!(flags & CFLAG_EXTRA_PRECISION))
						return anything;
				}
			}
		}

		if(PointInCylinder(cyl, &ep->v[n].p)) {
			
			anything = min(anything, ep->v[n].p.y);
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
		float xx=-EEsin(radians((float)j))*cyl->radius;
		float yy=EEcos(radians((float)j))*cyl->radius;
		EERIE_3D pos;
		pos.x=cyl->origin.x+xx;

		pos.z=cyl->origin.z+yy;
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
	if(anything != 999999.f && ep->norm.y < 0.1f && ep->norm.y > -0.1f)
		anything = min(anything, ep->min.y);

	return anything;
}

inline bool IsPolyInSphere(EERIEPOLY *ep, EERIE_SPHERE * sph) {

	if(!ep || !sph)
		return false;

	if(ep->area < 100.f)
		return false;

	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	long r = to - 1;
	Vec3f center;

	for(long n = 0; n < to; n++) {
		if(ep->area > 2000.f) {
			center = (ep->v[n].p + ep->v[r].p) * 0.5f;
			if(sph->contains(center)) {	
				return true;
			}
			if(ep->area > 4000.f) {
				center = (ep->v[n].p + ep->center) * 0.5f;
				if(sph->contains(center)) {
					return true;
				}
			}
			if(ep->area > 6000.f) {
				center = (center + ep->v[n].p) * 0.5f;
				if(sph->contains(center)) {
					return true;
				}
			}
		}
		
		Vec3f v(ep->v[n].p.x, ep->v[n].p.y, ep->v[n].p.z);

		if(sph->contains(v)) {
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
			float old = ioo->physics.cyl.radius;
			ioo->physics.cyl.radius += 25.f;

			for(size_t j = 0; j < io->obj->vertexlist3.size(); j++) {
				if(PointInCylinder(&ioo->physics.cyl, &io->obj->vertexlist3[j].v)) {
					ioo->physics.cyl.radius = old;
					return true;
				}
			}

			ioo->physics.cyl.radius = old;
		}
	}
	
	return false;
}

// TODO include header?
extern void GetIOCyl(Entity * io,EERIE_CYLINDER * cyl);
void PushIO_ON_Top(Entity * ioo, float ydec) {
	
	if(ydec != 0.f)
	for(size_t i = 0; i < entities.size(); i++) {
		Entity * io = entities[i];

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
					miny = min(miny, ioo->obj->vertexlist3[ii].v.y);
					maxy = max(maxy, ioo->obj->vertexlist3[ii].v.y);
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
									moveto.y += ydec;
									EERIE_CYLINDER cyl = player.baseCylinder();
									cyl.origin.y += ydec;
									float vv = CheckAnythingInCylinder(&cyl, entities.player(), 0);
									if(vv < 0) {
										player.pos.y += ydec + vv;
									}
								} else {
									EERIE_CYLINDER cyl = player.baseCylinder();
									cyl.origin.y += ydec;
									if(CheckAnythingInCylinder(&cyl, entities.player(), 0) >= 0) {
										player.pos.y += ydec;
										moveto.y += ydec;
									}
								}
							} else {
								if(ydec <= 0) {
									io->pos.y += ydec;
								} else {
									EERIE_CYLINDER cyl;
									GetIOCyl(io, &cyl);
									cyl.origin.y += ydec;
									if(CheckAnythingInCylinder(&cyl, io ,0) >= 0) {
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
		Entity * io = entities[i];

		if(io
		   && io != pfrm
		   && (io->ioflags & IO_NPC)
		   && !(io->ioflags & IO_NO_COLLISIONS)
		   && io->show == SHOW_FLAG_IN_SCENE
		) {
			EERIE_CYLINDER cyl;
			GetIOCyl(io, &cyl);

			if(CylinderPlatformCollide(&cyl, pfrm) != 0.f)
				return true;
		}
	}

	return false;
}

float CylinderPlatformCollide(EERIE_CYLINDER * cyl, Entity * io) {
 
	float miny = io->bbox3D.min.y;
	float maxy = io->bbox3D.max.y;
	
	if(maxy <= cyl->origin.y + cyl->height || miny >= cyl->origin.y) {
		return 0.f;
	}
	
	if(In3DBBoxTolerance(&cyl->origin,&io->bbox3D,cyl->radius)) {
		return 1.f;
	}
	
	return 0.f;
}

static long NPC_IN_CYLINDER = 0;

extern void GetIOCyl(Entity * io,EERIE_CYLINDER * cyl);

inline void EE_RotateY(TexturedVertex *in,TexturedVertex *out,float c, float s)
{
	out->p.x = (in->p.x*c) + (in->p.z*s);
	out->p.y = in->p.y;
	out->p.z = (in->p.z*c) - (in->p.x*s);
}

bool CollidedFromBack(Entity * io,Entity * ioo)
{
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

	float ft = radians(135.f + 90.f);
	ep.v[1].p.x = EEsin(ft) * 180.f;
	ep.v[1].p.z = -EEcos(ft) * 180.f;

	ft = radians(225.f + 90.f);
	ep.v[2].p.x = EEsin(ft) * 180.f;
	ep.v[2].p.z = -EEcos(ft) * 180.f;

	ft=radians(270.f-io->angle.getPitch());
	float ec=EEcos(ft);
	float es=EEsin(ft);
	EE_RotateY(&ep.v[1], &ep.tv[1], ec, es);
	EE_RotateY(&ep.v[2], &ep.tv[2], ec, es);
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
float CheckAnythingInCylinder(EERIE_CYLINDER * cyl,Entity * ioo,long flags) {
	
	NPC_IN_CYLINDER = 0;
	
	long rad = (cyl->radius + 100) * ACTIVEBKG->Xmul;

	long px = cyl->origin.x*ACTIVEBKG->Xmul;
	long pz = cyl->origin.z*ACTIVEBKG->Zmul;

	if(px > ACTIVEBKG->Xsize-2-rad)
		return 0.f;

	if(px < 1+rad)
		return 0.f;
	
	if(pz > ACTIVEBKG->Zsize-2-rad)
		return 0.f;

	if(pz < 1+rad)
		return 0.f;

	float anything = 999999.f; 
	
	EERIEPOLY * ep;
	
	for(long j = pz - rad; j <= pz + rad; j++)
	for(long i = px - rad; i <= px + rad; i++) {
		float nearx,nearz;
		float nearest = 99999999.f;

		for(long num = 0; num < 4; num++) {

			nearx = static_cast<float>(i * 100);
			nearz = static_cast<float>(j * 100);

			if(num == 1 || num == 2)
				nearx += 100;

			if(num == 2 || num == 3)
				nearz += 100;

			float dd = fdist(Vec2f(nearx, nearz), Vec2f(cyl->origin.x, cyl->origin.z));

			if(dd < nearest) {
				nearest = dd;
			}
		}

		if(nearest > max(82.f, cyl->radius))
			continue;


		FAST_BKG_DATA * feg = &ACTIVEBKG->fastdata[i][j];
		for(long k = 0; k < feg->nbpoly; k++) {
			ep = &feg->polydata[k];

			if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				continue;

			if(ep->min.y < anything) {
				anything= min(anything, IsPolyInCylinder(ep, cyl, flags));

				if(POLYIN) {
					if(ep->type & POLY_CLIMB)
						COLLIDED_CLIMB_POLY = 1;
				}
			}
		}
	}	

	float tempo;
	
	ep = CheckInPoly(cyl->origin.x, cyl->origin.y + cyl->height, cyl->origin.z, &tempo);
	
	if(ep) {
		anything = min(anything, tempo);
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
			if(FULL_TEST) {
				io = entities[i];
			} else {
				io = treatio[i].io;
			}

			if(!io
				|| io == ioo
				|| !io->obj
				||	(	(io->show!=SHOW_FLAG_IN_SCENE)
					||	((io->ioflags & IO_NO_COLLISIONS)  && !(flags & CFLAG_COLLIDE_NOCOL))
					) 
				|| fartherThan(io->pos, cyl->origin, 1000.f)) continue;
	
			{
				EERIE_CYLINDER * io_cyl = &io->physics.cyl;
				GetIOCyl(io, io_cyl);
				float dealt = 0;

				if (	(io->gameFlags & GFLAG_PLATFORM)
					||	((flags & CFLAG_COLLIDE_NOCOL) && (io->ioflags & IO_NPC) &&  (io->ioflags & IO_NO_COLLISIONS))
					)
				{
					if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(cyl->origin.x, cyl->origin.z), 440.f + cyl->radius))
					if(In3DBBoxTolerance(&cyl->origin, &io->bbox3D, cyl->radius+80))
					{
						if(io->ioflags & IO_FIELD) {
							if(In3DBBoxTolerance(&cyl->origin, &io->bbox3D, cyl->radius + 10))
								anything = -99999.f;
						} else {
						for(size_t ii = 0; ii < io->obj->vertexlist3.size(); ii++) {
							long res = PointInUnderCylinder(cyl, &io->obj->vertexlist3[ii].v);

							if(res > 0) {
								if(res == 2)
									ON_PLATFORM = 1;

								anything = min(anything, io->obj->vertexlist3[ii].v.y - 10.f);
							}			
						}

						for(size_t ii = 0; ii < io->obj->facelist.size(); ii++) {
							Vec3f c = Vec3f_ZERO;
							float height = io->obj->vertexlist3[io->obj->facelist[ii].vid[0]].v.y;

							for(long kk = 0; kk < 3; kk++) {
								c.x += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.x;
								c.y += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y;
								c.z += io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.z;

								height = min(height, io->obj->vertexlist3[io->obj->facelist[ii].vid[kk]].v.y);
							}

							c.x *= ( 1.0f / 3 );
							c.z *= ( 1.0f / 3 );
							c.y = io->bbox3D.min.y;

							long res = PointInUnderCylinder(cyl, &c);

							if(res > 0) {
								if(res == 2)
									ON_PLATFORM = 1;

								anything = min(anything, height);
							}
						}
						}
					}
				}
				else if (	(io->ioflags & IO_NPC)
						&&	(!(flags & CFLAG_NO_NPC_COLLIDE)) // MUST be checked here only (not before...)
						&&	(!(ioo && (ioo->ioflags & IO_NO_COLLISIONS)))	
						&&	(io->_npcdata->life>0.f)
						)
				{
					
					if(CylinderInCylinder(cyl, io_cyl)) {
 						NPC_IN_CYLINDER = 1;
						anything = min(anything, io_cyl->origin.y + io_cyl->height); 

						if(!(flags & CFLAG_JUST_TEST) && ioo) {
							if(float(arxtime) > io->collide_door_time + 500) {
								EVENT_SENDER = ioo;
								io->collide_door_time = (unsigned long)(arxtime); 	

								if(CollidedFromBack(io, ioo))
									SendIOScriptEvent(io, SM_COLLIDE_NPC, "back");
								else
									SendIOScriptEvent(io, SM_COLLIDE_NPC);

								EVENT_SENDER = io;
								io->collide_door_time = (unsigned long)(arxtime); 

								if(CollidedFromBack(ioo, io))
									SendIOScriptEvent(ioo, SM_COLLIDE_NPC, "back");
								else
									SendIOScriptEvent(ioo, SM_COLLIDE_NPC);
							}

							if(!dealt && (ioo->damager_damages > 0 || io->damager_damages > 0)) {

								if(ioo->damager_damages > 0)
									ARX_DAMAGES_DealDamages(i, ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);

								if(io->damager_damages > 0)
									ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
							}
							
							if(io->targetinfo == i) {
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
					EERIE_SPHERE sp;

					float miny = io->bbox3D.min.y;
					float maxy = io->bbox3D.max.y;

					if(maxy <= cyl->origin.y + cyl->height)
						goto suivant;

					if(miny >= cyl->origin.y)
						goto suivant;

					if(In3DBBoxTolerance(&cyl->origin, &io->bbox3D, cyl->radius + 30.f)) {
						vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;
						size_t nbv = io->obj->vertexlist.size();

						if(io->obj->nbgroups > 10) {
							for(long ii = 0; ii < io->obj->nbgroups; ii++) {
								long idx = io->obj->grouplist[ii].origin;
								sp.origin = vlist[idx].v;

								if(ioo == entities.player()) {
									sp.radius = 22.f; 
								} else if(ioo->ioflags & IO_NPC) {
									sp.radius = 26.f;
								} else {
									sp.radius = 22.f;
								}

								if(SphereInCylinder(cyl, &sp)) {
									if(!(flags & CFLAG_JUST_TEST) && ioo) {
										if(io->gameFlags & GFLAG_DOOR) {
											if(float(arxtime) > io->collide_door_time + 500) {
												EVENT_SENDER = ioo;
												io->collide_door_time = (unsigned long)(arxtime); 	
												SendIOScriptEvent(io, SM_COLLIDE_DOOR);

												EVENT_SENDER = io;
												io->collide_door_time = (unsigned long)(arxtime); 	
												SendIOScriptEvent(ioo, SM_COLLIDE_DOOR);
											}
										}

										if(io->ioflags & IO_FIELD) {
											EVENT_SENDER = NULL;
											io->collide_door_time = (unsigned long)(arxtime); 	
											SendIOScriptEvent(ioo, SM_COLLIDE_FIELD);
										}

										if(!dealt && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
											dealt = 1;

											if(ioo->damager_damages > 0)
												ARX_DAMAGES_DealDamages(i, ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);

											if(io->damager_damages > 0)
												ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
										}
									}

									anything = min(anything, min(sp.origin.y - sp.radius, io->bbox3D.min.y));
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

							if(nbv < 300)
								step = 1;
							else if(nbv < 600)
								step = 2;
							else if(nbv < 1200)
								step = 4;
							else
								step = 6;

							for(size_t ii = 1; ii < nbv; ii += step) {
								if(ii != (size_t)io->obj->origin) {
									sp.origin = vlist[ii].v;
									
									if(SphereInCylinder(cyl, &sp)) {
										if(!(flags & CFLAG_JUST_TEST) && ioo) {
											if(io->gameFlags & GFLAG_DOOR) {
												if(float(arxtime) > io->collide_door_time + 500) {
													EVENT_SENDER = ioo;
													io->collide_door_time = (unsigned long)(arxtime); 	
													SendIOScriptEvent(io, SM_COLLIDE_DOOR);

													EVENT_SENDER = io;
													io->collide_door_time = (unsigned long)(arxtime); 	
													SendIOScriptEvent(ioo, SM_COLLIDE_DOOR);
												}
											}

										if(io->ioflags & IO_FIELD) {
											EVENT_SENDER = NULL;
											io->collide_door_time = (unsigned long)(arxtime);
											SendIOScriptEvent(ioo, SM_COLLIDE_FIELD);
										}
					
										if(!dealt && ioo && (ioo->damager_damages > 0 || io->damager_damages > 0)) {
											dealt = 1;
											
											if(ioo->damager_damages > 0)
												ARX_DAMAGES_DealDamages(i, ioo->damager_damages, ioo->index(), ioo->damager_type, &io->pos);
									
											if(io->damager_damages > 0)
												ARX_DAMAGES_DealDamages(ioo->index(), io->damager_damages, io->index(), io->damager_type, &ioo->pos);
										}
										}
										anything = min(anything, min(sp.origin.y - sp.radius, io->bbox3D.min.y));
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

	anything = anything - cyl->origin.y;

	return anything;	
}

static bool InExceptionList(long val) {
	
	for(size_t i = 0; i < EXCEPTIONS_LIST_Pos; i++) {
		if(val == EXCEPTIONS_LIST[i]) {
			return true;
		}
	}
	
	return false;
}

bool CheckEverythingInSphere(EERIE_SPHERE * sphere, long source, long targ, std::vector<long> & sphereContent) //except source...
{
	bool vreturn = false;
	
	Entity * io;
	long ret_idx = -1;
	
	float sr30 = sphere->radius + 20.f;
	float sr40 = sphere->radius + 30.f;
	float sr180 = sphere->radius + 500.f;

	for(long i = 0; i < TREATZONE_CUR; i++) {
		if(targ > -1) {
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
			   || treatio[i].num == source
			   || InExceptionList(treatio[i].num)
			) {
				continue;
			}

			io = treatio[i].io;
			ret_idx = treatio[i].num;
		}

		if(!(io->ioflags & IO_NPC) && (io->ioflags & IO_NO_COLLISIONS))
			continue;

		if(!io->obj)
			continue;

		if(io->gameFlags & GFLAG_PLATFORM) {
			float miny = io->bbox3D.min.y;
			float maxy = io->bbox3D.max.y;

			if(maxy <= sphere->origin.y + sphere->radius || miny >= sphere->origin.y)
			if(In3DBBoxTolerance(&sphere->origin, &io->bbox3D, sphere->radius))
			{
				if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere->origin.x, sphere->origin.z), 440.f + sphere->radius)) {

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

						if(PointIn2DPolyXZ(&ep, sphere->origin.x, sphere->origin.z)) {
							sphereContent.push_back(ret_idx);

							vreturn = true;
							goto suivant;
						}
					}
				}
			}
		}

		if(closerThan(io->pos, sphere->origin, sr180)) {

			long amount = 1;
			std::vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->obj->nbgroups > 4) {
				for(long ii = 0; ii < io->obj->nbgroups; ii++) {
					if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere->origin, sr40)) {

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

				if(closerThan(fcenter, sphere->origin, sr30)
				   || closerThan(vlist[ef->vid[0]].v, sphere->origin, sr30)
				   || closerThan(vlist[ef->vid[1]].v, sphere->origin, sr30)
				   || closerThan(vlist[ef->vid[2]].v, sphere->origin, sr30)) {

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

EERIEPOLY * CheckBackgroundInSphere(EERIE_SPHERE * sphere) //except source...
{
	long rad = sphere->radius*ACTIVEBKG->Xmul;
	rad += 2;

	long px = sphere->origin.x * ACTIVEBKG->Xmul;
	long pz = sphere->origin.z * ACTIVEBKG->Zmul;

	long spx = std::max(px - rad, 0L);
	long epx = std::min(px + rad, ACTIVEBKG->Xsize - 1L);
	long spz = std::max(pz - rad, 0L);
	long epz = std::min(pz + rad, ACTIVEBKG->Zsize - 1L);

	for(long j = spz; j <= epz; j++)
	for(long i = spx; i <= epx; i++) {
		FAST_BKG_DATA * feg = &ACTIVEBKG->fastdata[i][j];

		for(long k = 0; k < feg->nbpoly; k++) {
			EERIEPOLY * ep = &feg->polydata[k];

			if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				continue;

			if(IsPolyInSphere(ep, sphere)) {
				return ep;					
			}			
		}
	}	
	
	return NULL;	
}

bool CheckAnythingInSphere(EERIE_SPHERE * sphere, long source, CASFlags flags, long * num) //except source...
{
	if(num)
		*num = -1;

	long rad = sphere->radius * ACTIVEBKG->Xmul;
	rad += 2;

	long px = sphere->origin.x * ACTIVEBKG->Xmul;
	long pz = sphere->origin.z * ACTIVEBKG->Zmul;

	if(!(flags & CAS_NO_BACKGROUND_COL)) {
		long spx = std::max(px - rad, 0L);
		long epx = std::min(px + rad, ACTIVEBKG->Xsize - 1L);
		long spz = std::max(pz - rad, 0L);
		long epz = std::min(pz + rad, ACTIVEBKG->Zsize - 1L);

		for(long j = spz; j <= epz; j++)
		for(long i = spx; i <= epx; i++) {
			FAST_BKG_DATA *feg = &ACTIVEBKG->fastdata[i][j];

			for(long k = 0; k < feg->nbpoly; k++) {
				EERIEPOLY *ep = &feg->polydata[k];

				if(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
					continue;

				if(IsPolyInSphere(ep,sphere))
					return true;
			}
		}	
	}

	if(flags & CAS_NO_NPC_COL)
		return false;

	bool validsource = false;

	if(flags & CAS_NO_SAME_GROUP)
		validsource = ValidIONum(source);

	float sr30 = sphere->radius + 20.f;
	float sr40 = sphere->radius + 30.f;
	float sr180 = sphere->radius + 500.f;

	for(long i = 0; i < TREATZONE_CUR; i++) {
		
		if(treatio[i].show != 1 || !treatio[i].io || treatio[i].num == source)
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

		if(treatio[i].num != 0 && source != 0 && validsource && HaveCommonGroup(io,entities[source]))
			continue;

		if(io->gameFlags & GFLAG_PLATFORM) {
			float miny = io->bbox3D.min.y;
			float maxy = io->bbox3D.max.y;

			if(maxy > sphere->origin.y - sphere->radius || miny < sphere->origin.y + sphere->radius)
			if(In3DBBoxTolerance(&sphere->origin, &io->bbox3D, sphere->radius))
			{
				if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(sphere->origin.x, sphere->origin.z), 440.f + sphere->radius)) {

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

						if(PointIn2DPolyXZ(&ep, sphere->origin.x, sphere->origin.z)) {
							if(num)
								*num=treatio[i].num;

							return true;
						}
					}
				}
			}
		}

		if(closerThan(io->pos, sphere->origin, sr180)) {
			long amount = 1;
			std::vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->obj->nbgroups > 4) {
				for(long ii = 0; ii < io->obj->nbgroups; ii++) {
					if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere->origin, sr40)) {
						if(num)
							*num = treatio[i].num;

						return true;
					}
				}

				amount = 2;
			}

			for(size_t ii = 0; ii < io->obj->facelist.size(); ii += amount) {

				if(io->obj->facelist[ii].facetype & POLY_HIDE)
					continue;

				if(closerThan(vlist[io->obj->facelist[ii].vid[0]].v, sphere->origin, sr30)
				   || closerThan(vlist[io->obj->facelist[ii].vid[1]].v, sphere->origin, sr30)) {
					if(num)
						*num = treatio[i].num;

					return true;
				}
			}
		}
	}

	return false;	
}


bool CheckIOInSphere(EERIE_SPHERE * sphere, long target, bool ignoreNoCollisionFlag) {
	
	if(!ValidIONum(target))
		return false;

	Entity * io=entities[target];
	float sr30 = sphere->radius + 22.f;
	float sr40 = sphere->radius + 27.f; 
	float sr180 = sphere->radius + 500.f;

	if((ignoreNoCollisionFlag || !(io->ioflags & IO_NO_COLLISIONS))
	   && (io->show == SHOW_FLAG_IN_SCENE)
	   && (io->gameFlags & GFLAG_ISINTREATZONE)
	   && (io->obj)
	) {
		if(closerThan(io->pos, sphere->origin, sr180)) {
			vector<EERIE_VERTEX> & vlist = io->obj->vertexlist3;

			if(io->obj->nbgroups>10) {
				long count=0;
				long ii=io->obj->nbgroups-1;

				while(ii) {
					if(closerThan(vlist[io->obj->grouplist[ii].origin].v, sphere->origin, sr40)) {
						count++;

						if(count>3)
							return true;
					}

					ii--;
				}
			}

			long count=0;
			long step;
			long nbv = io->obj->vertexlist.size();

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

			for(size_t ii = 0; ii < vlist.size(); ii += step) {
				if(closerThan(vlist[ii].v, sphere->origin, sr30)) {
					count++;

					if(count > 6)
						return true;
				}

				if(io->obj->vertexlist.size() < 120) {
					for(size_t kk = 0; kk < vlist.size(); kk+=1) {
						if(kk != ii) {
							for(float nn = 0.2f; nn < 1.f; nn += 0.2f) {
								Vec3f posi = vlist[ii].v * nn + vlist[kk].v * (1.f - nn);
								if(!fartherThan(sphere->origin, posi, sr30 + 20)) {
									count++;

									if(count > 3) {
										if(io->ioflags & IO_FIX)
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

			if(count > 3 && (io->ioflags & IO_FIX))
				return true;
		}
	}
	
	return false;	
}


float MAX_ALLOWED_PER_SECOND=12.f;
//-----------------------------------------------------------------------------
// Checks if a position is valid, Modify it for height if necessary
// Returns true or false

bool AttemptValidCylinderPos(EERIE_CYLINDER * cyl, Entity * io, CollisionFlags flags) {
	
	float anything = CheckAnythingInCylinder(cyl, io, flags); 

	if((flags & CFLAG_LEVITATE) && anything == 0.f)
		return true;

	// Falling Cylinder but valid pos !
	if(anything >= 0.f) {
		if(flags & CFLAG_RETURN_HEIGHT)
			cyl->origin.y += anything;

		return true;
	}
	
	EERIE_CYLINDER tmp;

	if(!(flags & CFLAG_ANCHOR_GENERATION)) {
		
		memcpy(&tmp, cyl, sizeof(EERIE_CYLINDER));

		while(anything < 0.f) {
			tmp.origin.y += anything;
			anything = CheckAnythingInCylinder(&tmp, io, flags);
		}

		anything = tmp.origin.y - cyl->origin.y;
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
				tolerate = cyl->height * 0.5f;
			}
			
			if(anything < tolerate) {
				return false;
			}
		}
		
		if(io && (flags & CFLAG_PLAYER) && anything < 0.f && (flags & CFLAG_JUST_TEST)) {
			EERIE_CYLINDER tmpp;
			memcpy(&tmpp, cyl, sizeof(EERIE_CYLINDER));
			tmpp.radius *= 0.7f;

			float tmp = CheckAnythingInCylinder(&tmpp, io, flags | CFLAG_JUST_TEST);

			if(tmp > 50.f) {
				tmpp.radius = cyl->radius * 1.4f;
				tmpp.origin.y -= 30.f;
				float tmp = CheckAnythingInCylinder(&tmpp, io, flags | CFLAG_JUST_TEST);

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
				float pente = EEfabs(anything) / dist * ( 1.0f / 2 ); 
				io->_npcdata->climb_count += pente;

				if(io->_npcdata->climb_count > MAX_ALLOWED_PER_SECOND) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
				}

				if(anything < -55) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
					return false;
				}

				EERIE_CYLINDER tmpp;
				memcpy(&tmpp, cyl, sizeof(EERIE_CYLINDER));
				tmpp.radius *= 0.65f; 
				float tmp = CheckAnythingInCylinder(&tmpp, io, flags | CFLAG_JUST_TEST);

				if(tmp > 50.f) {
					tmpp.radius = cyl->radius * 1.45f;
					tmpp.origin.y -= 30.f;
					float tmp = CheckAnythingInCylinder(&tmpp, io, flags | CFLAG_JUST_TEST);

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
			cyl->origin.y += anything;

		return false;
	}

	memcpy(&tmp, cyl, sizeof(EERIE_CYLINDER));
	tmp.origin.y += anything;
	anything = CheckAnythingInCylinder(&tmp, io, flags);

	if(anything < 0.f) {
		if(flags & CFLAG_RETURN_HEIGHT) {
			while(anything < 0.f) {
				tmp.origin.y += anything;
				anything = CheckAnythingInCylinder(&tmp, io, flags);
			}

			cyl->origin.y = tmp.origin.y; 
		}

		return false;
	}

	cyl->origin.y = tmp.origin.y;
	return true;
}

//----------------------------------------------------------------------------------------------
//flags & 1 levitate
//flags & 2 no inter col
//flags & 4 special
//flags & 8	easier sliding.
//flags & 16 climbing !!!
//flags & 32 Just Test !!!
//flags & 64 NPC mode
//----------------------------------------------------------------------------------------------
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
		float curmovedist = min(distance, MOVE_CYLINDER_STEP);
		distance -= curmovedist;

		// Store our cylinder desc into a test struct
		memcpy(&test, ip, sizeof(IO_PHYSICS));

		// uses test struct to simulate movement.
		vector2D.x = mvector.x * curmovedist;
		vector2D.y = 0.f;
		vector2D.z = mvector.z * curmovedist;

		test.cyl.origin.x += vector2D.x; 
		test.cyl.origin.y += mvector.y * curmovedist;
		test.cyl.origin.z += vector2D.z; 
		
		if((flags & CFLAG_CHECK_VALID_POS) && CylinderAboveInvalidZone(&test.cyl))
				return false;

		if(AttemptValidCylinderPos(&test.cyl,io,flags)) {
			// Found without complication
			memcpy(ip, &test, sizeof(IO_PHYSICS));
		} else {
			//return false;
			if(mvector.x == 0.f && mvector.z == 0.f)
				return true;
			
			if(flags & CFLAG_CLIMBING) {
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				test.cyl.origin.y += mvector.y * curmovedist;

				if(AttemptValidCylinderPos(&test.cyl, io, flags)) {
					memcpy(ip,&test,sizeof(IO_PHYSICS)); 
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
				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				float t = radians(MAKEANGLE(rangle));
				YRotatePoint(&mvector, &vecatt, EEcos(t), EEsin(t));
				test.cyl.origin += vecatt * curmovedist;
				float cc = io->_npcdata->climb_count;

				if(AttemptValidCylinderPos(&test.cyl, io, flags)) {
					rpos = test.cyl.origin;
					RFOUND = 1;
				} else {
					io->_npcdata->climb_count = cc;
				}

				rangle += ANGLESTEPP;

				memcpy(&test.cyl, &ip->cyl, sizeof(EERIE_CYLINDER)); 
				t = radians(MAKEANGLE(langle));
				YRotatePoint(&mvector, &vecatt, EEcos(t), EEsin(t));
				test.cyl.origin += vecatt * curmovedist;
				cc = io->_npcdata->climb_count;

				if(AttemptValidCylinderPos(&test.cyl, io, flags)) {
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
			if(EEfabs(ip->startpos.y - ip->cyl.origin.y) > 30.f)
				return false;
		}

	oki:
		;
	}

	MOVING_CYLINDER = 0;
	return true;
}

//TODO copy-paste
bool IO_Visible(Vec3f * orgn, Vec3f * dest, EERIEPOLY * epp, Vec3f * hit)
{
	float ix,iy,iz;
	long px,pz;
	EERIEPOLY *ep;

	FAST_BKG_DATA *feg;
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

		EERIE_SPHERE sphere;
		sphere.origin.x=x;
		sphere.origin.y=y;
		sphere.origin.z=z;
		sphere.radius=65.f;

		for(size_t num = 0; num < entities.size(); num++) {
			Entity * io = entities[num];

			if(io && (io->gameFlags & GFLAG_VIEW_BLOCKER)) {
				if(CheckIOInSphere(&sphere, num)) {
					dd = fdist(*orgn, sphere.origin);

					if(dd < nearest) {
						hit->x=x;
						hit->y=y;
						hit->z=z;
						return false;
					}
				}
			}
		}

		px = (long)(x * ACTIVEBKG->Xmul);
		pz = (long)(z * ACTIVEBKG->Zmul);

		if(px < 0 || px >= ACTIVEBKG->Xsize || pz < 0 || pz >= ACTIVEBKG->Zsize)
			goto fini;

			feg = &ACTIVEBKG->fastdata[px][pz];

			for(long k = 0; k < feg->nbpolyin; k++) {
				ep = feg->polyin[k];

				if(!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)))
				if((ep->min.y - pas < y) && (ep->max.y + pas > y))
				if((ep->min.x - pas < x) && (ep->max.x + pas > x))
				if((ep->min.z - pas < z) && (ep->max.z + pas > z))
				{
					if(RayCollidingPoly(orgn, dest, ep, hit)) {
						dd = fdist(*orgn, *hit);
						if(dd < nearest) {
							nearest = dd;
							found_ep = ep;
							found_hit = *hit;
						}
					}
				}
			}
			
		}
		
fini:
	;
	
	if(!found_ep)
		return true;

	if(found_ep == epp)
		return true;
	
	*hit = found_hit;

	return false;
}

void ANCHOR_BLOCK_Clear() {

	EERIE_BACKGROUND * eb = ACTIVEBKG;

	if(!eb)
		return;

	for(long k = 0; k < eb->nbanchors; k++) {
		ANCHOR_DATA * ad = &eb->anchors[k];
		ad->flags &= ~ANCHOR_FLAG_BLOCKED;
	}
}

void ANCHOR_BLOCK_By_IO(Entity * io, long status) {

	EERIE_BACKGROUND * eb = ACTIVEBKG;

	for(long k = 0; k < eb->nbanchors; k++) {
		ANCHOR_DATA * ad = &eb->anchors[k];

		if(fartherThan(ad->pos, io->pos, 600.f))
			continue;

		if(closerThan(Vec2f(io->pos.x, io->pos.z), Vec2f(ad->pos.x, ad->pos.z), 440.f)) {
			
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

				if(PointIn2DPolyXZ(&ep, ad->pos.x, ad->pos.z)) {
					if(status)
						ad->flags |= ANCHOR_FLAG_BLOCKED;
					else
						ad->flags &= ~ANCHOR_FLAG_BLOCKED;
				}
			}
		}
	}					
}
