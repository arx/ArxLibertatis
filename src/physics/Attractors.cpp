/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <vector>

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "graphics/Math.h"
#include "scene/Interactive.h"
#include "util/Range.h"

struct Attractor {
	EntityHandle source;
	float power;
	float radius;
};

static std::vector<Attractor> g_attractors;

void ARX_SPECIAL_ATTRACTORS_Reset() {
	g_attractors.clear();
}

void ARX_SPECIAL_ATTRACTORS_Add(EntityHandle ionum, float power, float radius) {
	
	if(power == 0.f) {
		util::unordered_remove_if(g_attractors, [ionum](const auto & entry) { return entry.source == ionum; });
		return;
	}
	
	for(Attractor & attractor : g_attractors) {
		if(attractor.source == ionum) {
			attractor.power = power;
			attractor.radius = radius;
			return;
		}
	}
	
	Attractor & attractor = g_attractors.emplace_back();
	attractor.source = ionum;
	attractor.power = power;
	attractor.radius = radius;
	
}

void ARX_SPECIAL_ATTRACTORS_ComputeForIO(const Entity & ioo, Vec3f & force) {
	
	force = Vec3f(0.f);
	
	for(auto & attractor : g_attractors) {
		
		Entity * iop = entities.get(attractor.source);
		if(!iop) {
			continue;
		}
		
		const Entity & io = *iop;
		
		if(io.show != SHOW_FLAG_IN_SCENE || (io.ioflags & IO_NO_COLLISIONS)
			 || !(io.gameFlags & GFLAG_ISINTREATZONE)) {
			continue;
		}
		
		float dist = fdist(ioo.pos, io.pos);
		
		if(dist > (ioo.physics.cyl.radius + io.physics.cyl.radius + 10.f) || attractor.power < 0.f) {
			
			float max_radius = attractor.radius;
			
			if(dist < max_radius) {
				float ratio_dist = 1.f - (dist / max_radius);
				Vec3f vect = io.pos - ioo.pos;
				force += glm::normalize(vect) * (attractor.power * ratio_dist * 0.01f);
			}
		}
		
	}
	
}
