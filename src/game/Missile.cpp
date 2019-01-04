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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/Missile.h"

#include <stddef.h>

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

#include "math/Random.h"
#include "math/RandomVector.h"
#include "math/Vector.h"

#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

class TextureContainer;

struct ARX_MISSILE {
	
	ARX_SPELLS_MISSILE_TYPE type;
	Vec3f startpos;
	Vec3f velocity;
	Vec3f lastpos;
	GameInstant timecreation;
	GameInstant lastupdate;
	GameDuration tolive;
	LightHandle m_light;
	EntityHandle owner;
	
	ARX_MISSILE()
		: type(MISSILE_NONE)
		, startpos(0.f)
		, velocity(0.f)
		, lastpos(0.f)
		, timecreation(0)
		, lastupdate(0)
		, tolive(0)
	{ }
	
};

static const size_t MAX_MISSILES = 100;
static ARX_MISSILE missiles[MAX_MISSILES];

// Gets a Free Projectile Slot
static long ARX_MISSILES_GetFree() {
	
	for(size_t i = 0; i < MAX_MISSILES; i++) {
		if(missiles[i].type == MISSILE_NONE) {
			return i;
		}
	}
	
	return -1;
}

// Kills a missile
static void ARX_MISSILES_Kill(long i) {
	
	switch (missiles[i].type)
	{
		case MISSILE_FIREBALL : {
			
			EERIE_LIGHT * light = lightHandleGet(missiles[i].m_light);
			if(light) {
				light->duration = GameDurationMs(150);
			}

			break;
		}
		case MISSILE_NONE: break;
	}

	missiles[i].type = MISSILE_NONE;
}

//-----------------------------------------------------------------------------
// Clear all missiles
void ARX_MISSILES_ClearAll() {
	for(size_t i = 0; i < MAX_MISSILES; i++) {
		ARX_MISSILES_Kill(i);
	}
}

//-----------------------------------------------------------------------------
// Spawns a Projectile using type, starting position/TargetPosition
void ARX_MISSILES_Spawn(Entity * io, ARX_SPELLS_MISSILE_TYPE type, const Vec3f & startpos, const Vec3f & targetpos) {
	
	long i(ARX_MISSILES_GetFree());

	if (i == -1) return;

	missiles[i].owner = (io == NULL) ? EntityHandle() : io->index();
	missiles[i].type = type;
	missiles[i].lastpos = missiles[i].startpos = startpos;

	float dist;

	dist = 1.f / fdist(startpos, targetpos);
	missiles[i].velocity = (targetpos - startpos) * dist;
	missiles[i].lastupdate = missiles[i].timecreation = g_gameTime.now();

	switch (type)
	{
		case MISSILE_NONE: break;
		case MISSILE_FIREBALL:
		{
			missiles[i].tolive = GameDurationMs(6000);
			missiles[i].velocity *= 0.8f;
			
			EERIE_LIGHT * light = dynLightCreate(missiles[i].m_light);
			if(light) {
				light->intensity = 1.3f;
				light->fallend = 420.f;
				light->fallstart = 250.f;
				light->rgb = Color3f(1.f, .8f, .6f);
				light->pos = startpos;
			}

			ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_WIND_LOOP, &missiles[i].startpos, 2.f);
			ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_LAUNCH, &missiles[i].startpos, 2.f);
		}
	}
}

extern TextureContainer * TC_fire;

//-----------------------------------------------------------------------------
// Updates all currently launched projectiles
void ARX_MISSILES_Update() {
	
	ARX_PROFILE_FUNC();
	
	TextureContainer * tc = TC_fire;
	
	GameInstant now = g_gameTime.now();

	for(unsigned long i(0); i < MAX_MISSILES; i++) {
		if(missiles[i].type == MISSILE_NONE)
			continue;
		
		GameDuration framediff3 = now - missiles[i].timecreation;
		if(framediff3 > missiles[i].tolive) {
			ARX_MISSILES_Kill(i);
			continue;
		}
		
		switch(missiles[i].type) {
			case MISSILE_NONE:
			break;
			case MISSILE_FIREBALL: {
				
				Vec3f pos = missiles[i].startpos + missiles[i].velocity * Vec3f(toMsf(framediff3));
				
				EERIE_LIGHT * light = lightHandleGet(missiles[i].m_light);
				if(light) {
					light->pos = pos;
				}

				Vec3f orgn = missiles[i].lastpos;
				Vec3f dest = pos;
				
				EERIEPOLY * ep = GetMinPoly(dest);
				EERIEPOLY * epp = GetMaxPoly(dest);
				
				bool hit = false;
				
				if(closerThan(player.pos, dest, 200.f) || (ep && ep->center.y < dest.y) || (epp && epp->center.y > dest.y)) {
					hit = true;
				} else {
					RaycastResult ray = RaycastLine(orgn, dest);
					if(ray.hit) {
						dest = ray.pos;
						hit = true;
					} else if(!CheckInPoly(dest) || EEIsUnderWater(dest)) {
						hit = true;
					} else {
						Vec3f tro = Vec3f(70.f);
						EntityHandle ici = IsCollidingAnyInter(dest, tro);
						
						if(ici != EntityHandle() && ici != missiles[i].owner) {
							hit = true;
						}
					}
				}
				
				if(hit) {
					ARX_MISSILES_Kill(i);
					spawnFireHitParticle(dest, 0);
					PolyBoomAddScorch(dest);
					Add3DBoom(dest);
					DoSphericDamage(Sphere(dest, 200.f), 180.f, DAMAGE_AREAHALF, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, EntityHandle());
					break;
				}
				
				PARTICLE_DEF * pd = createParticle();
				if(pd) {
					pd->ov = pos;
					pd->move = missiles[i].velocity;
					pd->move += Vec3f(3.f, 4.f, 3.f) + Vec3f(-6.f, -12.f, -6.f) * arx::randomVec3f();
					pd->tolive = Random::getu(500, 1000);
					pd->tc = tc;
					pd->siz = 12.f * ((missiles[i].tolive - framediff3) / GameDurationMs(4000));
					pd->scale = arx::randomVec(15.f, 20.f);
					pd->m_flags = FIRE_TO_SMOKE;
				}
				
				missiles[i].lastpos = pos;
				
				break;
			}
		}

		missiles[i].lastupdate = now;
	}
}
