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

static EERIEPOLY * ANCHOR_CheckInPoly(const Vec3f & pos) {
	
	long x = pos.x * ACTIVEBKG->m_mul.x;
	long z = pos.z * ACTIVEBKG->m_mul.y;

	if(x < 0 || x >= ACTIVEBKG->m_size.x || z < 0 || z >= ACTIVEBKG->m_size.y)
		return NULL;

	EERIEPOLY *found = NULL;

	BackgroundTileData *feg = &ACTIVEBKG->m_tileData[x][z];

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

void AnchorData_ClearAll(BackgroundData * eb) {
	
	//	EERIE_PATHFINDER_Release();
	EERIE_PATHFINDER_Clear();

	for(long j = 0; j < eb->m_size.y; j++) {
		for(long i = 0; i < eb->m_size.x; i++) {
			BackgroundTileData * eg = &eb->m_tileData[i][j];

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
			if(GetTruePolyY(ep, pos, &vy) && glm::abs(vy - cyl.origin.y) > 160.f)
				failcount++;
		}
	}

	float failratio = failcount / count;

	if(failratio > 0.75f)
		return true;

	return false;
}
