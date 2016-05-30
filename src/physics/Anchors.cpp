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

#include "physics/Anchors.h"

#include <cstdio>

#include "ai/PathFinderManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "graphics/Math.h"
#include "io/log/Logger.h"
#include "physics/Collisions.h"
#include "platform/profiler/Profiler.h"

extern float MAX_ALLOWED_PER_SECOND;
extern bool DIRECT_PATH;

static EERIEPOLY * ANCHOR_CheckInPolyPrecis(const Vec3f & pos) {
	
	long px = pos.x * ACTIVEBKG->Xmul;
	long pz = pos.z * ACTIVEBKG->Zmul;

	if(px <= 0 || px >= ACTIVEBKG->Xsize - 1 || pz <= 0 || pz >= ACTIVEBKG->Zsize - 1)
		return NULL;

	EERIEPOLY * found = NULL;
	float foundY = 9999999.f;

	for(short z = pz - 1; z <= pz + 1; z++)
	for(short x = px - 1; x <= px + 1; x++) {
			EERIE_BKG_INFO *feg = &ACTIVEBKG->fastdata[x][z];

			for(long k = 0; k < feg->nbpolyin; k++) {
				EERIEPOLY *ep = feg->polyin[k];

				if(!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL)) && PointIn2DPolyXZ(ep, pos.x, pos.z)) {
					Vec3f poss = pos;
					float yy;

					if(GetTruePolyY(ep, poss, &yy) && yy >= pos.y && (!found || yy <= foundY)) {
						found = ep;
						foundY = yy;
					}
				}
			}
	}

	return found;
}

static EERIEPOLY * ANCHOR_CheckInPoly(const Vec3f & pos) {
	
	long x = pos.x * ACTIVEBKG->Xmul;
	long z = pos.z * ACTIVEBKG->Zmul;

	if(x < 0 || x >= ACTIVEBKG->Xsize || z < 0 || z >= ACTIVEBKG->Zsize)
		return NULL;

	EERIEPOLY *found = NULL;

	EERIE_BKG_INFO *feg = &ACTIVEBKG->fastdata[x][z];

	for(long k = 0; k < feg->nbpolyin; k++) {
		EERIEPOLY *ep = feg->polyin[k];

		if (!(ep->type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
		        &&	(ep->max.y >= pos.y)
		        &&	(ep != found)
		        &&	((ep->norm.y < 0.f) || ((ep->type & POLY_QUAD) && (ep->norm2.y < 0.f)))
		        &&	(PointIn2DPolyXZ(ep, pos.x, pos.z)))
		{
			if(!found || ep->min.y < found->min.y)
				found = ep;
		}
	}

	if(!found)
		return CheckInPoly(pos);

	return found;
}

extern Vec3f vector2D;

static float ANCHOR_IsPolyInCylinder(const EERIEPOLY & ep, const Cylinder & cyl,
                                     CollisionFlags flags) {
	
	if (!(flags & CFLAG_EXTRA_PRECISION))
	{
		if (ep.area < 100.f) return 999999.f;
	}

	if (PointInCylinder(cyl, ep.center))
	{
		if (ep.norm.y < 0.5f)
			return ep.min.y;

		return ep.center.y;
	}

	float minf = std::min(cyl.origin.y, cyl.origin.y + cyl.height);
	float maxf = std::max(cyl.origin.y, cyl.origin.y + cyl.height);

	if (minf > ep.max.y) return 999999.f;

	if (maxf < ep.min.y) return 999999.f;

	long to = (ep.type & POLY_QUAD) ? 4 : 3;

	long r = to - 1;
	float anything = 999999.f;
	Vec3f center;
	long n;

	for(n = 0; n < to; n++) {
		if(flags & CFLAG_EXTRA_PRECISION) {
			for(long o = 0; o < 5; o++) {
				float p = (float)o * ( 1.0f / 5 );
				center = ep.v[n].p * p + ep.center * (1.f - p);
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					return anything;
				}
			}
		}

		if(ep.area > 2000.f || (flags & CFLAG_EXTRA_PRECISION)) {
			center = (ep.v[n].p + ep.v[r].p) * 0.5f;
			if(PointInCylinder(cyl, center)) {
				anything = std::min(anything, center.y);
				return anything;
			}

			if(ep.area > 4000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (ep.v[n].p + ep.center) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					return anything;
				}
			}

			if(ep.area > 6000.f || (flags & CFLAG_EXTRA_PRECISION)) {
				center = (center + ep.v[n].p) * 0.5f;
				if(PointInCylinder(cyl, center)) {
					anything = std::min(anything, center.y);
					return anything;
				}
			}
		}

		if(PointInCylinder(cyl, ep.v[n].p)) {
			anything = std::min(anything, ep.v[n].p.y);
			return anything;
		}

		r++;

		if(r >= to) {
			r = 0;
		}
	}

	if ((anything != 999999.f) && (ep.norm.y < 0.1f) && (ep.norm.y > -0.1f))
		anything = std::min(anything, ep.min.y);

	return anything;
}

/*!
 * \brief Check if anything is in a cylinder
 * \param cyl the cylinder to check
 * \param flags collision flags
 * \return 0 if nothing in cyl else returns Y Offset to put cylinder in a proper place
 */
static float ANCHOR_CheckAnythingInCylinder(const Cylinder & cyl, CollisionFlags flags) {
	
	ARX_PROFILE_FUNC();
	
	long rad = (cyl.radius + 230) * ACTIVEBKG->Xmul;
	
	long px = cyl.origin.x * ACTIVEBKG->Xmul;
	long pz = cyl.origin.z * ACTIVEBKG->Zmul;
	
	if(px > ACTIVEBKG->Xsize - 2 - rad)
		return 0.f;
	if(px < 1 + rad)
		return 0.f;
	if(pz > ACTIVEBKG->Zsize - 2 - rad)
		return 0.f;
	if(pz < 1 + rad)
		return 0.f;
	
	float anything = 999999.f; 
	
	for(short z = pz - rad; z <= pz + rad; z++)
	for(short x = px - rad; x <= px + rad; x++) {
		const EERIE_BKG_INFO & feg = ACTIVEBKG->fastdata[x][z];
		
		for(long k = 0; k < feg.nbpoly; k++) {
			const EERIEPOLY & ep = feg.polydata[k];
			
			if(ep.type & (POLY_WATER | POLY_TRANS | POLY_NOCOL))
				continue;
			
			if(ep.min.y < anything) {
				float minanything = std::min(anything, ANCHOR_IsPolyInCylinder(ep, cyl, flags));
				
				if(anything != minanything)
					anything = minanything;
			}
		}
	}
	
	EERIEPOLY *ep = ANCHOR_CheckInPolyPrecis(cyl.origin + Vec3f(0.f, cyl.height, 0.f));
	
	if(ep)
		anything = std::min(anything, ep->min.y);
	
	float tempo;
	
	if(ep && GetTruePolyY(ep, cyl.origin, &tempo))
		anything = std::min(anything, tempo);
	
	anything = anything - cyl.origin.y;
	return anything;
}

extern long MOVING_CYLINDER;

// TODO copy-paste AttemptValidCylinderPos
static bool ANCHOR_AttemptValidCylinderPos(Cylinder & cyl, Entity * io,
	                                         CollisionFlags flags) {
	
	float anything = ANCHOR_CheckAnythingInCylinder(cyl, flags);

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
			anything = ANCHOR_CheckAnythingInCylinder(tmp, flags);
		}

		anything = tmp.origin.y - cyl.origin.y;
	}

	if(MOVING_CYLINDER) {
		if(flags & CFLAG_NPC) {
			float tolerate;

			if(io
			   && (io->ioflags & IO_NPC)
			   && (io->_npcdata->pathfind.listnb > 0)
			   && (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb)
			) {
				tolerate = -80;
			} else {
				tolerate = -45;
			}

			if(anything < tolerate)
				return false;
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
					return false;
				}

				if(anything < -55) {
					io->_npcdata->climb_count = MAX_ALLOWED_PER_SECOND;
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
	anything = ANCHOR_CheckAnythingInCylinder(tmp, flags);

	if(anything < 0.f) {
		if(flags & CFLAG_RETURN_HEIGHT) {
			while(anything < 0.f) {
				tmp.origin.y += anything;
				anything = ANCHOR_CheckAnythingInCylinder(tmp, flags);
			}

			cyl.origin.y = tmp.origin.y;
		}

		return false;
	}

	cyl.origin.y = tmp.origin.y;
	return true;
}

// TODO copy-paste Move_Cylinder
static bool ANCHOR_ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip, Entity * io,
                                               float MOVE_CYLINDER_STEP,
                                               CollisionFlags flags) {
	
	MOVING_CYLINDER = 1;
	DIRECT_PATH = true;
	IO_PHYSICS test;

	if(ip == NULL) {
		MOVING_CYLINDER = 0;
		return false;
	}

	float distance = glm::distance(ip->startpos, ip->targetpos);

	if(distance <= 0.f) {
		MOVING_CYLINDER = 0;
		return true; 
	}

	Vec3f mvector = (ip->targetpos - ip->startpos) / distance;

	while(distance > 0.f) {
		// First We compute current increment
		float curmovedist = std::min(distance, MOVE_CYLINDER_STEP);

		distance -= curmovedist;
		//CUR_FRAME_SLICE=curmovedist*onedist;
		// Store our cylinder desc into a test struct
		test = *ip;

		// uses test struct to simulate movement.
		test.cyl.origin += mvector * curmovedist;
		
		vector2D.x = mvector.x * curmovedist;
		vector2D.y = 0.f;
		vector2D.z = mvector.z * curmovedist;

		if ((flags & CFLAG_CHECK_VALID_POS)
		        && (CylinderAboveInvalidZone(test.cyl)))
			return false;

		if(ANCHOR_AttemptValidCylinderPos(test.cyl, io, flags)) {
			*ip = test;

		} else {
			if(flags & CFLAG_CLIMBING) {
				test.cyl = ip->cyl;
				test.cyl.origin.y += mvector.y * curmovedist;

				if(ANCHOR_AttemptValidCylinderPos(test.cyl, io, flags)) {
					*ip = test;
					goto oki;
				}
			}

			DIRECT_PATH = false;
			// Must Attempt To Slide along collisions
			Vec3f vecatt;
			Vec3f rpos = Vec3f_ZERO;
			Vec3f lpos = Vec3f_ZERO;
			long				RFOUND		=	0;
			long				LFOUND		=	0;
			long				maxRANGLE	=	90;
			float				ANGLESTEPP;

			// player sliding in fact...
			if(flags & CFLAG_EASY_SLIDING) {
				ANGLESTEPP = 10.f;
				maxRANGLE = 70;
			} else {
				ANGLESTEPP = 30.f;
			}

			float rangle = ANGLESTEPP;
			float langle = 360.f - ANGLESTEPP;

			//tries on the Right and Left sides
			while(rangle <= maxRANGLE) {
				test.cyl = ip->cyl;
				float t = MAKEANGLE(rangle);
				vecatt = VRotateY(mvector, t);
				test.cyl.origin += vecatt * curmovedist;

				if(ANCHOR_AttemptValidCylinderPos(test.cyl, io, flags)) {
					rpos = test.cyl.origin;
					RFOUND = 1;
				}

				rangle += ANGLESTEPP;

				test.cyl = ip->cyl;
				t = MAKEANGLE(langle);
				vecatt = VRotateY(mvector, t);
				test.cyl.origin += vecatt * curmovedist;

				if(ANCHOR_AttemptValidCylinderPos(test.cyl, io, flags)) {
					lpos = test.cyl.origin;
					LFOUND = 1;
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
			} else { //stopped
				ip->velocity = Vec3f_ZERO;
				MOVING_CYLINDER = 0;
				return false;
			}
		}

	oki:
		;
	}

	MOVING_CYLINDER = 0;
	return true;
}

void AnchorData_ClearAll(EERIE_BACKGROUND * eb) {
	
	//	EERIE_PATHFINDER_Release();
	EERIE_PATHFINDER_Clear();

	for(long j = 0; j < eb->Zsize; j++) {
		for(long i = 0; i < eb->Xsize; i++) {
			EERIE_BKG_INFO * eg = &eb->fastdata[i][j];

			if(eg->nbianchors && eg->ianchors)
				free(eg->ianchors);

			eg->nbianchors = 0;
			eg->ianchors = NULL;
		}
	}

	if(eb->anchors && eb->nbanchors) {
		for(int j = 0; j < eb->nbanchors; j++) {
			if(eb->anchors[j].nblinked && eb->anchors[j].linked) {
				free(eb->anchors[j].linked);
				eb->anchors[j].linked = NULL;
			}
		}

		free(eb->anchors);
	}

	eb->anchors = NULL;
	eb->nbanchors = 0;
}

static const int INC_RADIUS = 10;

bool CylinderAboveInvalidZone(const Cylinder & cyl) {
	
	float count = 0;
	float failcount = 0;

	for(float rad = 0; rad < cyl.radius; rad += 10.f) {
		for(float ang = 0; ang < 360; ang += 45) {
			if(rad == 0)
				ang = 360;

			Vec3f pos = cyl.origin;
			pos += angleToVectorXZ(ang) * rad;
			pos += Vec3f(0.f, -20.f, 0.f);
			
			EERIEPOLY * ep = ANCHOR_CheckInPoly(pos);

			if(!ep)
				continue;

			if(ep->type & POLY_NOPATH)
				return true;

			count += 1.f;
			float vy;
			GetTruePolyY(ep, pos, &vy);

			if(glm::abs(vy - cyl.origin.y) > 160.f)
				failcount++;
		}
	}

	float failratio = failcount / count;

	if(failratio > 0.75f)
		return true;

	return false;
}
//*************************************************************************************
// Adds an Anchor... and tries to generate the best possible cylinder for it
//*************************************************************************************

static bool DirectAddAnchor_Original_Method(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, Vec3f * pos) {
	
	long found = 0;
	long stop_radius = 0;

	Cylinder testcyl;
	Cylinder currcyl;
	Cylinder bestcyl;

	bestcyl.height = 0;
	bestcyl.radius = 0;
	currcyl.radius = 40;
	currcyl.height = -165.f;
	currcyl.origin = *pos;

	stop_radius = 0;
	found = 0;
	long climb = 0;

	while (stop_radius != 1)
	{
		testcyl = currcyl;
		testcyl.radius += INC_RADIUS;

		if (ANCHOR_AttemptValidCylinderPos(testcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_ANCHOR_GENERATION))
		{
			currcyl = testcyl;
			found = 1;
		}
		else
		{
			if ((testcyl.origin.y != currcyl.origin.y)
			        && (glm::abs(testcyl.origin.y - pos->y) < 50))
			{
				testcyl.radius -= INC_RADIUS;
				currcyl = testcyl;
				climb++;
			}
			else
				stop_radius = 1;
		}

		if (climb > 4) stop_radius = 1;

		if (currcyl.radius >= 50.f) stop_radius = 1;

	}

	if(found && currcyl.radius >= bestcyl.radius) {
		bestcyl = currcyl;
	} else {
		return false;
	}

	if(CylinderAboveInvalidZone(bestcyl))
		return false;

	for (long k = 0; k < eb->nbanchors; k++)
	{
		const ANCHOR_DATA & ad = eb->anchors[k];

		if(closerThan(ad.pos, bestcyl.origin, 50.f)) {
			return false;
		}

		if(closerThan(Vec2f(ad.pos.x, ad.pos.z), Vec2f(bestcyl.origin.x, bestcyl.origin.z), 45.f)) {
			
			if (glm::abs(ad.pos.y - bestcyl.origin.y) < 90.f) return false;

			EERIEPOLY * ep = ANCHOR_CheckInPolyPrecis(ad.pos);
			EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(Vec3f(ad.pos.x, bestcyl.origin.y, ad.pos.z));

			if (ep2 == ep) return false;
		}

	}

	eg->ianchors = (long *)realloc(eg->ianchors, sizeof(long) * (eg->nbianchors + 1));

	eg->ianchors[eg->nbianchors] = eb->nbanchors;
	eg->nbianchors++;

	eb->anchors = (ANCHOR_DATA *)realloc(eb->anchors, sizeof(ANCHOR_DATA) * (eb->nbanchors + 1));

	ANCHOR_DATA * ad = &eb->anchors[eb->nbanchors];
	ad->pos = bestcyl.origin;
	ad->height = bestcyl.height;
	ad->radius = bestcyl.radius;
	ad->linked = NULL;
	ad->nblinked = 0;
	ad->flags = 0;
	eb->nbanchors++;
	return true;
}

static bool AddAnchor_Original_Method(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, Vec3f * pos) {
	
	long found = 0;
	long best = 0;
	long stop_radius = 0;
	float best_dist = 99999999999.f;

	Cylinder testcyl;
	Cylinder currcyl;
	Cylinder bestcyl;

	bestcyl.height = 0;
	bestcyl.radius = 0;

	
	for (long rad = 0; rad < 20; rad += 10) 
		for (long ang = 0; ang < 360; ang += 45) // 45
		{
			// We set our current position depending on given position, radius & angle.
			currcyl.radius = 40; 
			currcyl.height = -165;
			
			currcyl.origin = *pos;
			currcyl.origin += angleToVectorXZ(ang) * float(rad);
			
			stop_radius = 0;
			found = 0;
			long climb = 0;

			while ((stop_radius != 1))
			{
				testcyl = currcyl;
				testcyl.radius += INC_RADIUS;

				if (ANCHOR_AttemptValidCylinderPos(testcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_ANCHOR_GENERATION))
				{
					currcyl = testcyl;
					found = 1;
				}
				else
				{
					if ((testcyl.origin.y != currcyl.origin.y)
					        && (glm::abs(testcyl.origin.y - pos->y) < 50))
					{
						testcyl.radius -= INC_RADIUS;
						currcyl = testcyl;
						climb++;
					}
					else
						stop_radius = 1;
				}

				if (climb > 4) stop_radius = 1;

				if (currcyl.radius >= 50.f) stop_radius = 1;
			}

			if (found)
			{
				float d = glm::distance(*pos, currcyl.origin);

				if (currcyl.radius >= bestcyl.radius)	
				{
					if (((best_dist > d) && (currcyl.radius == bestcyl.radius))
					        || (currcyl.radius > bestcyl.radius))
					{
						bestcyl = currcyl;
						best_dist = d;
						best = 1;
					}
				}
			}
		}

	if (!best) return false;

	if(CylinderAboveInvalidZone(bestcyl))
		return false;

	// avoid to recreate same anchor twice...
	if (0)
		for (long k = 0; k < eb->nbanchors; k++)
		{
			ANCHOR_DATA * ad = &eb->anchors[k];

			if ((ad->pos.x == bestcyl.origin.x)
			        &&	(ad->pos.y == bestcyl.origin.y)
			        &&	(ad->pos.z == bestcyl.origin.z))
			{
				if (ad->radius >= bestcyl.radius)
					return false;

				if (ad->radius <= bestcyl.radius)
				{
					ad->height = bestcyl.height;
					ad->radius = bestcyl.radius;
					return false;
				}
			}
		}

	eg->ianchors = (long *)realloc(eg->ianchors, sizeof(long) * (eg->nbianchors + 1));

	eg->ianchors[eg->nbianchors] = eb->nbanchors;
	eg->nbianchors++;

	eb->anchors = (ANCHOR_DATA *)realloc(eb->anchors, sizeof(ANCHOR_DATA) * (eb->nbanchors + 1));

	ANCHOR_DATA * ad = &eb->anchors[eb->nbanchors];
	ad->pos = bestcyl.origin;
	ad->height = bestcyl.height;
	ad->radius = bestcyl.radius;
	ad->linked = NULL;
	ad->nblinked = 0;
	ad->flags = 0;
	eb->nbanchors++;
	return true;
}

//**********************************************************************************************
// Adds an Anchor Link to an Anchor
//**********************************************************************************************
static void AddAnchorLink(EERIE_BACKGROUND * eb, long anchor, long linked) {
	
	ANCHOR_DATA & a = eb->anchors[anchor];
	
	// Avoid to store Already existing Links
	for(long i = 0; i < a.nblinked; i++)
		if(a.linked[i] == linked)
			return;

	// Realloc & fill data
	a.linked = (long *)realloc(a.linked, sizeof(long) * (a.nblinked + 1));

	a.linked[a.nblinked] = linked;
	a.nblinked++;
}

//**********************************************************************************************
// Generates Links between Anchors for a background
//**********************************************************************************************

static void AnchorData_Create_Links_Original_Method(EERIE_BACKGROUND * eb) {
	
	Vec3f p1, p2; 
	long count = 0;
	long per;
	long lastper = -1;
	long total = eb->Zsize * eb->Xsize;

	for(long j = 0; j < eb->Zsize; j++)
	for(long i = 0; i < eb->Xsize; i++) {
		per = count / total * 100.f;
		
		if(per != lastper) {
			LogInfo << "Anchor Links Generation: %" << per;
			lastper = per;
		}
		
		count++;
		const EERIE_BKG_INFO & eg = eb->fastdata[i][j];
		long precise = 0;
		
		for(long kkk = 0; kkk < eg.nbpolyin; kkk++) {
			EERIEPOLY * ep = eg.polyin[kkk];
			
			if(ep->type & POLY_PRECISE_PATH) {
				precise = 1;
				break;
			}
		}
		
		
		for(long k = 0; k < eg.nbianchors; k++) {
			long ii = glm::clamp(i - 2, 0l, eb->Xsize - 1l);
			long ia = glm::clamp(i + 2, 0l, eb->Xsize - 1l);
			long ji = glm::clamp(j - 2, 0l, eb->Zsize - 1l);
			long ja = glm::clamp(j + 2, 0l, eb->Zsize - 1l);
			
			for(long j2 = ji; j2 <= ja; j2++)
			for(long i2 = ii; i2 <= ia; i2++) {
				const EERIE_BKG_INFO & eg2 = eb->fastdata[i2][j2];
				long precise2 = 0;
				
				for(long kkk = 0; kkk < eg2.nbpolyin; kkk++) {
					EERIEPOLY * ep2 = eg2.polyin[kkk];
					
					if(ep2->type & POLY_PRECISE_PATH) {
						precise2 = 1;
						break;
					}
				}
				
				for(long k2 = 0; k2 < eg2.nbianchors; k2++) {
					// don't treat currently treated anchor
					if(eg.ianchors[k] == eg2.ianchors[k2])
						continue;
					
					p1 = eb->anchors[eg.ianchors[k]].pos;
					p2 = eb->anchors[eg2.ianchors[k2]].pos;
					p1.y += 10.f;
					p2.y += 10.f;
					long _onetwo = 0;
					bool treat = true;
					float _dist = glm::distance(p1, p2);
					float dd = glm::distance(Vec2f(p1.x, p1.z), Vec2f(p2.x, p2.z));
					
					if(dd < 5.f)
						continue;
					
					if(dd > 200.f)
						continue; 
					
					if(precise || precise2) {
						if(_dist > 120.f)
							continue;
					} else if(_dist > 200.f) {
						continue;
					}
					
					if(glm::abs(p1.y - p2.y) > dd * 0.9f)
						continue;
					
					IO_PHYSICS ip;
					ip.startpos = ip.cyl.origin = p1;
					ip.targetpos = p2;
					
					ip.cyl.height = eb->anchors[eg.ianchors[k]].height;
					ip.cyl.radius = eb->anchors[eg.ianchors[k]].radius;
					
					long t = 2;
					
					//TODO check for dead code CFLAG_SPECIAL
					if(ANCHOR_ARX_COLLISION_Move_Cylinder(&ip, NULL, 20, CFLAG_CHECK_VALID_POS | CFLAG_NO_INTERCOL | CFLAG_EASY_SLIDING | CFLAG_NPC | CFLAG_JUST_TEST | CFLAG_EXTRA_PRECISION)) {
						if(fartherThan(Vec2f(ip.cyl.origin.x, ip.cyl.origin.z), Vec2f(ip.targetpos.x, ip.targetpos.z), 25.f)) { 
							t--;
						} else {
							_onetwo = 1;
						}
					} else {
						t--;
					}
					
					if(t == 1) {
						ip.startpos = ip.cyl.origin = p2;
						ip.targetpos = p1;
						
						ip.cyl.height = eb->anchors[eg2.ianchors[k2]].height;
						ip.cyl.radius = eb->anchors[eg2.ianchors[k2]].radius;
						
						//CFLAG_SPECIAL
						if(ANCHOR_ARX_COLLISION_Move_Cylinder(&ip, NULL, 20, CFLAG_CHECK_VALID_POS | CFLAG_NO_INTERCOL | CFLAG_EASY_SLIDING | CFLAG_NPC | CFLAG_JUST_TEST | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT)) {
							if(fartherThan(Vec2f(ip.cyl.origin.x, ip.cyl.origin.z), Vec2f(ip.targetpos.x, ip.targetpos.z), 25.f)) {
								t--;
							} else {
								_onetwo |= 2;
							}
						} else {
							t--;
						}
					} else {
						t--;
					}
					
					if(t <= 0)
						treat = false;
					else
						treat = true;
					
					if(treat) {
						if(_onetwo) {
							AddAnchorLink(eb, eg.ianchors[k], eg2.ianchors[k2]);
							AddAnchorLink(eb, eg2.ianchors[k2], eg.ianchors[k]);
						}
					}
				}
			}
		}
	}
	
	EERIE_PATHFINDER_Create();
}

static float GetTileMinY(long i, long j) {
	float minf = 9999999999.f;
	EERIE_BKG_INFO *eg = &ACTIVEBKG->fastdata[i][j];

	for (long kk = 0; kk < eg->nbpolyin; kk++) {
		EERIEPOLY * ep = eg->polyin[kk];
		minf = std::min(minf, ep->min.y);
	}

	return minf;
}

static float GetTileMaxY(long i, long j) {
	float maxf = -9999999999.f;
	EERIE_BKG_INFO *eg = &ACTIVEBKG->fastdata[i][j];

	for(long kk = 0; kk < eg->nbpolyin; kk++) {
		EERIEPOLY * ep = eg->polyin[kk];
		maxf = std::max(maxf, ep->max.y);
	}

	return maxf;
}

static void AnchorData_Create_Phase_II_Original_Method(EERIE_BACKGROUND * eb) {
	
	Vec3f pos;
	float count = 0;
	
	long lastper	=	-1;
	long per;
	float total		=	static_cast<float>(eb->Zsize * eb->Xsize);
	
	for(long z = 0; z < eb->Zsize; z++)
	for(long x = 0; x < eb->Xsize; x++) {
		per = count / total * 100.f;
		
		if(per != lastper) {
			LogInfo << "Anchor Generation: %" << per << " (Pass II)";
			lastper = per;
		}
		
		count += 1.f;
		
		EERIE_BKG_INFO * eg = &eb->fastdata[x][z];
		pos.x = float(x) * float(eb->Xdiv);
		pos.y = 0.f;
		pos.z = float(z) * float(eb->Zdiv);
		Cylinder currcyl = Cylinder(pos, 30, -150.f);
		
		if(eg->nbpolyin) {
			long ok = 0;
			
			for(long kkk = 0; kkk < eg->nbpolyin; kkk++) {
				EERIEPOLY * ep = eg->polyin[kkk];

				if (ep->type & POLY_PRECISE_PATH) {
					ok = 1;
					break;
				}
			}
			
			if(!ok)
				continue;
			
			float roof = GetTileMinY(x, z); 
			float current_y = GetTileMaxY(x, z); 
			
			while(current_y > roof) {
				long added = 0;
				
				for(float pposz = 0.f; pposz < 1.f; pposz += 0.1f)
				for(float pposx = 0.f; pposx < 1.f; pposx += 0.1f) {
					currcyl.origin.x = pos.x + pposx * eb->Xdiv;
					currcyl.origin.z = pos.z + pposz * eb->Zdiv;
					currcyl.origin.y = current_y;
					
					EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin + Vec3f(0.f, -10.f, 0.f));
					
					if(!ep2)
						continue;
					
					if(!(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
						continue;
					
					if(ep2->type & POLY_NOPATH)
						continue;
					
					if(ANCHOR_AttemptValidCylinderPos(currcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT | CFLAG_ANCHOR_GENERATION)) {
						EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin + Vec3f(0.f, -10.f, 0.f));
						
						if(!ep2)
							continue;
						
						if(!(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f))
							continue;
						
						if(ep2->type & POLY_NOPATH)
							continue;
						
						if(DirectAddAnchor_Original_Method(eb, eg, &currcyl.origin)) {
							added = 1;
						}
					}
				}
				
				
				if(added)
					current_y -= 160.f; 
				
				current_y -= 50.f; 
			}
		}
	}

}

void AnchorData_Create(EERIE_BACKGROUND * eb) {
	
	AnchorData_ClearAll(eb);
	Vec3f pos;

	float count = 0;
	long lastper	=	-1;
	long per;
	float total		=	static_cast<float>(eb->Zsize * eb->Xsize * 9);

	for(long z = 0; z < eb->Zsize; z++)
	for(long x = 0; x < eb->Xsize; x++) {
		long LASTFOUND = 0;
		
		for(long divv = 0; divv < 9; divv++) {
			long divvx, divvy;
			
			switch (divv) {
			case 0:
				divvx = 0;
				divvy = 0;
				break;
			case 1:
				divvx = 1;
				divvy = 0;
				break;
			case 2:
				divvx = 2;
				divvy = 0;
				break;
			case 3:
				divvx = 0;
				divvy = 1;
				break;
			case 4:
				divvx = 1;
				divvy = 1;
				break;
			case 5:
				divvx = 2;
				divvy = 1;
				break;
			case 6:
				divvx = 0;
				divvy = 2;
				break;
			case 7:
				divvx = 1;
				divvy = 2;
				break;
			case 8:
				divvx = 2;
				divvy = 2;
				break;
			}
			
			per = count / total * 100.f;
			
			if(per != lastper) {
				LogInfo << "Anchor Generation: %" << per;
				lastper = per;
			}
			
			count += 1.f;
			
			if(LASTFOUND)
				break;
			
			EERIE_BKG_INFO * eg = &eb->fastdata[x][z];
			pos.x = (float)((float)((float)x + 0.33f * (float)divvx) * (float)eb->Xdiv);
			pos.y = 0.f;
			pos.z = (float)((float)((float)z + 0.33f * (float)divvy) * (float)eb->Zdiv);
			EERIEPOLY * ep = GetMinPoly(pos);
			Cylinder currcyl = Cylinder(pos, 20 - (4.f * divv), -120.f);
			
			if(ep) {
				EERIEPOLY * epmax;
				epmax = GetMaxPoly(pos);
				float roof = (epmax ? epmax->min.y : ep->min.y) - 300;
				
				float current_y = ep->max.y;
				
				while(current_y > roof) {
					currcyl.origin.y = current_y;
					EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin + Vec3f(0.f, -30.f, 0.f));
					
					if(   ep2
					   && !(ep2->type & POLY_DOUBLESIDED)
					   && (ep2->norm.y > 0.f)
					) {
						ep2 = NULL;
					}
					
					if(ep2 && !(ep2->type & POLY_NOPATH)) {
						bool bval = ANCHOR_AttemptValidCylinderPos(currcyl, NULL, CFLAG_NO_INTERCOL | CFLAG_EXTRA_PRECISION | CFLAG_RETURN_HEIGHT | CFLAG_ANCHOR_GENERATION);
						
						if(   bval
						   && currcyl.origin.y - 10.f <= current_y
						) {
							EERIEPOLY * ep2 = ANCHOR_CheckInPolyPrecis(currcyl.origin + Vec3f(0.f, -38.f, 0.f));
							
							if(ep2 && !(ep2->type & POLY_DOUBLESIDED) && (ep2->norm.y > 0.f)) {
								current_y -= 10.f;
							} else if ((ep2) && (ep2->type & POLY_NOPATH)) {
								current_y -= 10.f;
							} else if (AddAnchor_Original_Method(eb, eg, &currcyl.origin)) {
								LASTFOUND++;
								current_y = currcyl.origin.y + currcyl.height;
							} else {
								current_y -= 10.f;
							}
						} else {
							current_y -= 10.f;
						}
					} else {
						current_y -= 10.f;
					}
				}
			}
		}
	}

	AnchorData_Create_Phase_II_Original_Method(eb);
	AnchorData_Create_Links_Original_Method(eb);
}
