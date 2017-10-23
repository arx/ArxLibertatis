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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "graphics/effects/Fog.h"

#include "animation/AnimationRender.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/particle/ParticleEffects.h"

#include "math/Random.h"
#include "math/RandomVector.h"

FOG_DEF fogs[MAX_FOG];

void ARX_FOGS_Clear()
{
	for(size_t i = 0; i < MAX_FOG; i++) {
		fogs[i] = FOG_DEF();
	}
}

long ARX_FOGS_GetFree()
{
	for(size_t i = 0; i < MAX_FOG; i++) {
		if(!fogs[i].exist)
			return i;
	}

	return -1;
}

void ARX_FOGS_Render() {
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	int iDiv = 4 - config.video.levelOfDetail;
	
	float flDiv = static_cast<float>(1 << iDiv);
	
	for(size_t i = 0; i < MAX_FOG; i++) {
		const FOG_DEF & fog = fogs[i];
		
		if(!fog.exist)
			continue;
		
		long count = std::max(1l, checked_range_cast<long>(g_framedelay / flDiv));
		while(count--) {
			
			if(Random::getf(0.f, 2000.f) >= fog.frequency) {
				continue;
			}
			
			PARTICLE_DEF * pd = createParticle(true);
			if(!pd) {
				break;
			}
			
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			if(fog.special & FOG_DIRECTIONAL) {
				pd->ov = fog.pos;
				pd->move = fog.move * (fog.speed * 0.1f);
			} else {
				pd->ov = fog.pos + arx::randomVec(-100.f, 100.f);
				pd->move = Vec3f(fog.speed) - arx::randomVec(0.f, 2.f);
				pd->move *= Vec3f(fog.speed * 0.2f,  1.f / 15, fog.speed * 0.2f);
			}
			pd->scale = Vec3f(fog.scale);
			pd->tolive = fog.tolive + Random::get(0, fog.tolive);
			pd->tc = TC_smoke;
			pd->siz = (fog.size + Random::getf(0.f, 2.f) * fog.size) * (1.0f / 3);
			pd->rgb = fog.rgb;
			pd->m_rotation = fog.rotatespeed;
		}
	}
}
