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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/Missile.h"

#include <stddef.h>
#include <vector>

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/Player.h"

#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Raycast.h"
#include "graphics/data/Mesh.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"

#include "math/Random.h"
#include "math/RandomVector.h"
#include "math/Vector.h"
#include "math/Quantizer.h"

#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "util/Range.h"


class TextureContainer;

struct Missile {
	
	Vec3f startpos;
	Vec3f velocity;
	Vec3f lastpos;
	GameInstant timecreation;
	GameDuration tolive;
	LightHandle m_light;
	EntityHandle owner;
	math::Quantizer m_quantizer;
	
	Missile()
		: startpos(0.f)
		, velocity(0.f)
		, lastpos(0.f)
		, timecreation(0)
		, tolive(0)
	{ }
	
};

static std::vector<Missile> g_missiles;

static void ARX_MISSILES_Kill(Missile & missile) {
	
	EERIE_LIGHT * light = lightHandleGet(missile.m_light);
	if(light) {
		light->duration = 150ms;
	}
	
	missile.tolive = 0;
	
}

void ARX_MISSILES_ClearAll() {
	
	for(Missile & missile : g_missiles) {
		ARX_MISSILES_Kill(missile);
	}
	
	g_missiles.clear();
	
}

void ARX_MISSILES_Spawn(Entity * io, const Vec3f & startpos, const Vec3f & targetpos) {
	
	Missile & missile = g_missiles.emplace_back();
	
	missile.owner = (io == nullptr) ? EntityHandle() : io->index();
	missile.lastpos = missile.startpos = startpos;
	missile.velocity = glm::normalize(targetpos - startpos) * 0.8f;
	missile.timecreation = g_gameTime.now();
	
	missile.tolive = 6s;
	
	EERIE_LIGHT * light = dynLightCreate(missile.m_light);
	if(light) {
		light->intensity = 1.3f;
		light->fallend = 420.f;
		light->fallstart = 250.f;
		light->rgb = Color3f(1.f, .8f, .6f);
		light->pos = startpos;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_WIND_LOOP, &missile.startpos, 2.f);
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_LAUNCH, &missile.startpos, 2.f);
	
}

void ARX_MISSILES_Update() {
	
	ARX_PROFILE_FUNC();
	
	GameInstant now = g_gameTime.now();
	
	for(Missile & missile : g_missiles) {
		
		arx_assert(missile.tolive != 0);
		
		GameDuration framediff3 = now - missile.timecreation;
		if(framediff3 > missile.tolive) {
			ARX_MISSILES_Kill(missile);
			continue;
		}
		
		Vec3f pos = missile.startpos + missile.velocity * Vec3f(toMsf(framediff3));
		
		EERIE_LIGHT * light = lightHandleGet(missile.m_light);
		if(light) {
			light->pos = pos;
		}
		
		Vec3f orgn = missile.lastpos;
		Vec3f dest = pos;
		
		EERIEPOLY * ep = GetMinPoly(dest);
		EERIEPOLY * epp = GetMaxPoly(dest);
		
		bool hit = false;
		
		if(closerThan(player.pos, dest, 200.f) || (ep && ep->center.y < dest.y) || (epp && epp->center.y > dest.y)) {
			hit = true;
		} else {
			RaycastResult ray = raycastScene(orgn, dest);
			if(ray.hit) {
				dest = ray.pos;
				hit = true;
			} else if(!CheckInPoly(dest) || EEIsUnderWater(dest)) {
				hit = true;
			} else {
				Vec3f tro = Vec3f(70.f);
				Entity * ici = getCollidingEntityAt(dest, tro);
				if(ici && ici->index() != missile.owner) {
					hit = true;
				}
			}
		}
		
		if(hit) {
			ARX_MISSILES_Kill(missile);
			spawnFireHitParticle(dest, 0);
			PolyBoomAddScorch(dest);
			Add3DBoom(dest);
			doSphericDamage(Sphere(dest, 200.f), 180.f, DAMAGE_AREAHALF, nullptr,
			                DAMAGE_TYPE_FAKESPELL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL);
			continue;
		}
		
		size_t count = missile.m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.03f);
		for(size_t i = 0; i < count; i++) {
			PARTICLE_DEF * pd = createParticle(true);
			if(pd) {
				pd->ov = pos;
				pd->move = missile.velocity;
				pd->move += Vec3f(3.f, 4.f, 3.f) + Vec3f(-6.f, -12.f, -6.f) * arx::randomVec3f();
				pd->tolive = Random::getu(500, 1000);
				pd->tc = g_particleTextures.fire;
				pd->size = 12.f * ((missile.tolive - framediff3) / 4s);
				pd->sizeDelta = Random::getf(15.f, 20.f);
				pd->m_flags = FIRE_TO_SMOKE;
			}
		}
		
		missile.lastpos = pos;
		
	}
	
	util::unordered_remove_if(g_missiles, [](const Missile & missile) { return missile.tolive == 0; });
	
}
