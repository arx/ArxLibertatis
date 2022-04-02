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
#include "graphics/particle/ParticleTextures.h"

#include "math/Random.h"
#include "math/RandomVector.h"

#include "scene/Tiles.h"

std::vector<FOG_DEF> g_fogs;

void ARX_FOGS_Clear() {
	g_fogs.clear();
}

long ARX_FOGS_GetFree() {
	
	for(size_t i = 0; i < g_fogs.size(); i++) {
		if(!g_fogs[i].exist) {
			g_fogs[i] = FOG_DEF();
			return i;
		}
	}
	
	g_fogs.emplace_back();
	
	return long(g_fogs.size() - 1);
}

void ARX_FOGS_Render() {
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	while(!g_fogs.empty() && !g_fogs.back().exist) {
		g_fogs.pop_back();
	}
	
	float period = float(1 << (4 - config.video.levelOfDetail));
	
	for(FOG_DEF & fog : g_fogs) {
		
		if(!fog.exist) {
			continue;
		}
		
		if(!g_tiles->isInActiveTile(fog.pos)) {
			fog.elapsed.reset();
			fog.visible = false;
			continue;
		}
		
		float maxCount = 2.f * float(fog.tolive) / period;
		
		if(fog.visible) {
			fog.elapsed.add(g_framedelay / period);
		} else {
			fog.elapsed.add(std::min(toMsf(g_gameTime.now() - fog.creationTime), 1.f + maxCount));
			fog.visible = true;
		}
		
		int count = std::min(fog.elapsed.consume(), 1 + int(maxCount));
		while(count--) {
			
			if(Random::getf(0.f, 2000.f) >= fog.frequency) {
				continue;
			}
			
			u32 tolive = fog.tolive + Random::get(0, fog.tolive);
			if(tolive <= count * s64(period)) {
				continue;
			}
			
			PARTICLE_DEF * pd = createParticle(true);
			if(!pd) {
				break;
			}
			
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			if(fog.directional) {
				pd->ov = fog.pos;
				pd->move = fog.move * (fog.speed * 0.1f);
			} else {
				pd->ov = fog.pos + arx::randomVec(-100.f, 100.f);
				pd->move = Vec3f(fog.speed) - arx::randomVec3f() * fog.speedRandom;
				pd->move *= Vec3f(fog.speed * 0.2f,  1.f / 15, fog.speed * 0.2f);
			}
			pd->scale = Vec3f(fog.scale);
			pd->tolive = tolive;
			pd->timcreation = toMsi(g_gameTime.now()) - count * s64(period);
			pd->tc = g_particleTextures.smoke;
			pd->siz = (fog.size + Random::getf(0.f, 2.f) * fog.size) * (1.0f / 3);
			pd->rgb = fog.rgb;
			if(fog.rgbRandom != Color3f::black) {
				pd->rgb += randomColor3f() * fog.rgbRandom;
			}
			pd->m_rotation = fog.rotatespeed;
		}
		
	}
	
}
