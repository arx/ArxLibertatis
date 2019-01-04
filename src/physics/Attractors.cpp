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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "physics/Attractors.h"

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "graphics/Math.h"
#include "scene/Interactive.h"

struct ARX_SPECIAL_ATTRACTOR {
	EntityHandle ionum;
	float power;
	float radius;
};

static const size_t MAX_ATTRACTORS = 16;
static ARX_SPECIAL_ATTRACTOR attractors[MAX_ATTRACTORS];

void ARX_SPECIAL_ATTRACTORS_Reset() {
	for(size_t i = 0; i < MAX_ATTRACTORS; i++) {
		attractors[i].ionum = EntityHandle();
	}
}

static void ARX_SPECIAL_ATTRACTORS_Remove(EntityHandle ionum) {
	for(size_t i = 0; i < MAX_ATTRACTORS; i++) {
		if(attractors[i].ionum == ionum) {
			attractors[i].ionum = EntityHandle();
		}
	}
}

static long ARX_SPECIAL_ATTRACTORS_Exist(EntityHandle ionum) {
	for(size_t i = 0; i < MAX_ATTRACTORS; i++) {
		if(attractors[i].ionum == ionum) {
			return i;
		}
	}
	return -1;
}

bool ARX_SPECIAL_ATTRACTORS_Add(EntityHandle ionum, float power, float radius) {
	
	if(power == 0.f) {
		ARX_SPECIAL_ATTRACTORS_Remove(ionum);
	}
	
	long tst;
	if((tst = ARX_SPECIAL_ATTRACTORS_Exist(ionum)) != -1) {
		attractors[tst].power = power;
		attractors[tst].radius = radius;
		return false;
	}
	
	for(size_t i = 0; i < MAX_ATTRACTORS; i++) {
		if(attractors[i].ionum == EntityHandle()) {
			attractors[i].ionum = ionum;
			attractors[i].power = power;
			attractors[i].radius = radius;
			return true;
		}
	}
	
	return false;
}

void ARX_SPECIAL_ATTRACTORS_ComputeForIO(const Entity & ioo, Vec3f & force) {
	
	force = Vec3f(0.f);
	
	for(size_t i = 0; i < MAX_ATTRACTORS; i++) {
		
		Entity * iop = entities.get(attractors[i].ionum);
		if(!iop) {
			continue;
		}
		
		const Entity & io = *iop;
		
		if(io.show != SHOW_FLAG_IN_SCENE || (io.ioflags & IO_NO_COLLISIONS)
			 || !(io.gameFlags & GFLAG_ISINTREATZONE)) {
			continue;
		}
		
		float power = attractors[i].power;
		float dist = fdist(ioo.pos, io.pos);
		
		if(dist > (ioo.physics.cyl.radius + io.physics.cyl.radius + 10.f) || power < 0.f) {
			
			float max_radius = attractors[i].radius;
			
			if(dist < max_radius) {
				float ratio_dist = 1.f - (dist / max_radius);
				Vec3f vect = io.pos - ioo.pos;
				vect = glm::normalize(vect);
				power *= ratio_dist * 0.01f;
				force = vect * power;
			}
		}
		
	}
}
