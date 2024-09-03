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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "graphics/particle/ParticleEffects.h"

#include <algorithm>
#include <chrono>

#include <boost/format.hpp>
#include <boost/range/adaptor/strided.hpp>

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/spell/Cheat.h"

#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/GlobalFog.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Decal.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/particle/ParticleTextures.h"

#include "input/Input.h"

#include "math/GtxFunctions.h"
#include "math/Random.h"
#include "math/RandomVector.h"

#include "platform/profiler/Profiler.h"

#include "physics/Collisions.h"
#include "physics/Physics.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Tiles.h"


static const size_t MAX_PARTICLES = 2200;
static long ParticleCount = 0;
static PARTICLE_DEF g_particles[MAX_PARTICLES];

long NewSpell = 0;

long getParticleCount() {
	return ParticleCount;
}

void createFireParticles(Vec3f pos, int perPos, ShortGameDuration delay) {
	
	for(long nn = 0 ; nn < perPos; nn++) {
		if(Random::getf() >= 0.4f) {
			continue;
		}
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		pd->ov = pos;
		pd->move = Vec3f(2.f, 2.f, 2.f) - Vec3f(4.f, 22.f, 4.f) * arx::randomVec3f();
		pd->size = 7.f;
		pd->duration = Random::get(500ms, 1500ms);
		pd->m_flags = FIRE_TO_SMOKE | ROTATING;
		pd->tc = g_particleTextures.fire2;
		pd->m_rotation = Random::getf(-0.1f, 0.1f);
		pd->sizeDelta = -8.f;
		pd->rgb = Color3f(0.71f, 0.43f, 0.29f);
		pd->elapsed = delay * -nn;
	}
	
}

void createObjFireParticles(const EERIE_3DOBJ * obj, int particlePositions, int perPos,
                            ShortGameDuration delay) {
	
	for(int i = 0; i < particlePositions; i++) {
		
		long notok = 10;
		std::vector<EERIE_FACE>::const_iterator it;
		
		while(notok-- > 0) {
			it = Random::getIterator(obj->facelist);
			arx_assert(it != obj->facelist.end());
			
			if(it->facetype & POLY_HIDE)
				continue;
			
			notok = -1;
		}
		
		if(notok < 0) {
			Vec3f pos = obj->vertexWorldPositions[it->vid[0]].v;
			createFireParticles(pos, perPos, delay);
		}
		
	}
	
}


void ARX_PARTICLES_Spawn_Lava_Burn(Vec3f pos, Entity * io) {
	
	if(io && io->obj && !io->obj->facelist.empty()) {
		size_t num = 0;
		long notok = 10;
		while(notok-- > 0) {
			num = Random::getu(0, io->obj->facelist.size() - 1);
			if(io->obj->facelist[num].facetype & POLY_HIDE) {
				continue;
			}
			if(glm::abs(pos.y - io->obj->vertexWorldPositions[io->obj->facelist[num].vid[0]].v.y) > 50.f) {
				continue;
			}
			notok = -1;
		}
		pos = io->obj->vertexWorldPositions[io->obj->facelist[num].vid[0]].v;
	}
	
	PARTICLE_DEF * pd = createParticle();
	if(!pd) {
		return;
	}
	
	pd->ov = pos;
	pd->move = arx::randomVec3f() * Vec3f(2.f, -12.f, 2.f) - Vec3f(4.f, 15.f, 4.f);
	pd->duration = 800ms;
	pd->tc = g_particleTextures.smoke;
	pd->size = 15.f;
	pd->sizeDelta = Random::getf(15.f, 20.f);
	pd->m_flags = FIRE_TO_SMOKE;
	if(Random::getf() > 0.5f) {
		pd->m_flags |= SUBSTRACT;
	}
}

static void ARX_PARTICLES_Spawn_Rogue_Blood(const Vec3f & pos, float dmgs, Color col) {
	
	PARTICLE_DEF * pd = createParticle();
	if(!pd) {
		return;
	}
	
	pd->ov = pos;
	pd->size = 3.1f * (dmgs * (1.f / 60) + .9f);
	pd->sizeDelta = -pd->size * 0.25f;
	pd->m_flags = PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | SPLAT_GROUND;
	pd->duration = 1600ms;
	pd->move = arx::randomVec3f() * Vec3f(60.f, -10.f, 60.f) - Vec3f(30.f, 15.f, 30.f);
	pd->rgb = Color3f(col);
	long num = Random::get(0, 5);
	pd->tc = g_particleTextures.bloodsplat[num];
	pd->m_rotation = Random::getf(-0.05f, 0.05f);
	
}

static void ARX_PARTICLES_Spawn_Blood3(const Vec3f & pos, float dmgs, Color col,
                                       ParticlesTypeFlags flags = 0) {
	
	PARTICLE_DEF * pd = createParticle();
	if(pd) {
		float sinW = timeWaveSin(g_gameTime.now(), 2s * glm::pi<float>());
		float cosW = timeWaveCos(g_gameTime.now(), 2s * glm::pi<float>());
		float power = dmgs * (1.f / 60) + .9f;
		pd->ov = pos + Vec3f(-sinW, sinW, cosW) * 30.f;
		pd->size = 3.5f * power + sinW;
		pd->sizeDelta = -pd->size * 0.5f;
		pd->m_flags = PARTICLE_SUB2 | SUBSTRACT | GRAVITY | ROTATING | flags;
		pd->duration = 1100ms;
		pd->rgb = Color3f(col);
		pd->tc = g_particleTextures.bloodsplat[0];
		pd->m_rotation = Random::getf(-0.05f, 0.05f);
	}
	
	if(Random::getf() > .90f) {
		ARX_PARTICLES_Spawn_Rogue_Blood(pos, dmgs, col);
	}
	
}



void ARX_PARTICLES_Spawn_Blood2(const Vec3f & pos, float dmgs, Color col, Entity * io) {
	
	bool isNpc = io && (io->ioflags & IO_NPC);
	
	if(isNpc && io->_npcdata->SPLAT_TOT_NB) {
		
		if(io->_npcdata->SPLAT_DAMAGES < 3) {
			return;
		}
		
		float power = (io->_npcdata->SPLAT_DAMAGES * (1.f / 60)) + .9f;
		
		Vec3f vect = pos - io->_npcdata->last_splat_pos;
		float dist = glm::length(vect);
		vect = glm::normalize(vect);
		long nb = long(dist / 4.f * power);
		if(nb == 0) {
			nb = 1;
		}
		
		long MAX_GROUND_SPLATS;
		switch(config.video.levelOfDetail) {
			case 2:  MAX_GROUND_SPLATS = 10; break;
			case 1:  MAX_GROUND_SPLATS = 5; break;
			default: MAX_GROUND_SPLATS = 1; break;
		}
		
		for(long k = 0; k < nb; k++) {
			Vec3f posi = io->_npcdata->last_splat_pos + vect * Vec3f(k * 4.f * power);
			io->_npcdata->SPLAT_TOT_NB++;
			if(io->_npcdata->SPLAT_TOT_NB > MAX_GROUND_SPLATS) {
				ARX_PARTICLES_Spawn_Blood3(posi, io->_npcdata->SPLAT_DAMAGES, col, SPLAT_GROUND);
				io->_npcdata->SPLAT_TOT_NB = 1;
			} else {
				ARX_PARTICLES_Spawn_Blood3(posi, io->_npcdata->SPLAT_DAMAGES, col);
			}
		}
		
	} else {
		if(isNpc) {
			io->_npcdata->SPLAT_DAMAGES = short(dmgs);
		}
		ARX_PARTICLES_Spawn_Blood3(pos, dmgs, col, SPLAT_GROUND);
		if(isNpc) {
			io->_npcdata->SPLAT_TOT_NB = 1;
		}
	}
	
	if(isNpc) {
		io->_npcdata->last_splat_pos = pos;
	}
}

void ARX_PARTICLES_Spawn_Blood(const Vec3f & pos, float dmgs, EntityHandle source) {
	
	Entity * sourceIo = entities.get(source);
	if(!sourceIo) {
		return;
	}
	
	float nearest_dist = std::numeric_limits<float>::max();
	VertexId nearest;
	for(const VertexGroup & group : sourceIo->obj->grouplist | boost::adaptors::strided(2)) {
		float dist = arx::distance2(pos, sourceIo->obj->vertexWorldPositions[group.origin].v);
		if(dist < nearest_dist) {
			nearest_dist = dist;
			nearest = group.origin;
		}
	}
	if(!nearest) {
		return;
	}
	
	// Decides number of blood particles...
	const unsigned int spawn_nb = glm::clamp(long(dmgs * 2.f), 5l, 26l);
	
	ShortGameDuration elapsed = 0;
	
	for(unsigned int k = 0; k < spawn_nb; k++) {
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		pd->size = 0.f;
		pd->sizeDelta = float(spawn_nb);
		pd->m_flags = GRAVITY | ROTATING | DELAY_FOLLOW_SOURCE;
		pd->source = &sourceIo->obj->vertexWorldPositions[nearest].v;
		pd->sourceionum = source;
		pd->duration = 1200ms + spawn_nb * 5ms;
		elapsed -= 45ms + Random::get(0ms, 150ms - std::chrono::milliseconds(spawn_nb));
		pd->elapsed = elapsed;
		pd->rgb = Color3f(.9f, 0.f, 0.f);
		pd->tc = g_particleTextures.bloodsplat[0];
		pd->m_rotation = Random::getf(-0.05f, 0.05f);
	}
	
}


void MakeCoolFx(const Vec3f & pos) {
	spawnFireHitParticle(pos, 1);
	PolyBoomAddScorch(pos);
}

void MakePlayerAppearsFX(const Entity & io) {
	MakeCoolFx(io.pos);
	MakeCoolFx(io.pos);
	AddRandomSmoke(io, 30);
	ARX_PARTICLES_Add_Smoke(io.pos, 1 | 2, 20); // flag 1 = randomize pos
}

void AddRandomSmoke(const Entity & io, long amount) {
	
	for(size_t i = 0; i < size_t(amount); i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		
		VertexId vertex(Random::get(size_t(0), io.obj->vertexlist.size() - 1));
		pd->ov = io.obj->vertexWorldPositions[vertex].v + arx::randomVec(-5.f, 5.f);
		pd->size = Random::getf(0.f, 8.f);
		if(pd->size < 4.f) {
			pd->size = 4.f;
		}
		pd->sizeDelta = 10.f;
		pd->m_flags = ROTATING | FADE_IN_AND_OUT;
		pd->duration = Random::get(900ms, 1300ms);
		pd->move = arx::linearRand(Vec3f(-0.25f, -0.7f, -0.25f), Vec3f(0.25f, 0.3f, 0.25f));
		pd->rgb = Color3f(0.3f, 0.3f, 0.34f);
		pd->tc = g_particleTextures.smoke;
		pd->m_rotation = 0.001f;
	}
	
}

// flag 1 = randomize pos
void ARX_PARTICLES_Add_Smoke(const Vec3f & pos, long flags, long amount, const Color3f & rgb) {
	
	Vec3f mod = (flags & 1) ? arx::randomVec(-50.f, 50.f) : Vec3f(0.f);
	
	while(amount--) {
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		pd->ov = pos + mod;
		if(flags & 2) {
			pd->size = Random::getf(15.f, 35.f);
			pd->sizeDelta = Random::getf(40.f, 55.f);
		} else {
			pd->size = Random::getf(5.f, 13.f);
			pd->sizeDelta = Random::getf(10.f, 15.f);
		}
		pd->m_flags = ROTATING | FADE_IN_AND_OUT;
		pd->duration = Random::get(1100ms, 1500ms);
		pd->elapsed = -(amount * 120ms + Random::get(0ms, 100ms));
		pd->move = arx::linearRand(Vec3f(-0.25f, -0.7f, -0.25f), Vec3f(0.25f, 0.3f, 0.25f));
		pd->rgb = rgb;
		pd->tc = g_particleTextures.smoke;
		pd->m_rotation = 0.01f;
	}
	
}

void ManageTorch() {
	
	arx_assert(entities.player());
	
	EERIE_LIGHT * el = lightHandleGet(torchLightHandle);
	
	if(player.torch) {
		
		float rr = Random::getf();
		el->pos = player.pos;
		el->intensity = 1.6f;
		el->fallstart = 280.f + rr * 20.f;
		el->fallend = el->fallstart + 280.f;
		el->m_exists = true;
		el->rgb = player.m_torchColor - Color3f(rr, rr, rr) * Color3f(0.1f, 0.1f, 0.1f);
		el->duration = 0;
		el->extras = 0;
		
	} else if(cur_mr == CHEAT_ENABLED) {
		
		el->pos = player.pos;
		el->intensity = 1.8f;
		el->fallstart = 480.f;
		el->fallend = el->fallstart + 480.f;
		el->m_exists = true;
		el->rgb = Color3f(1.f, .5f, .8f);
		el->duration = 0;
		el->extras = 0;
		
	} else {
		
		long count = MagicFlareCountNonFlagged();
		
		if(count) {
			float rr = Random::getf();
			el->pos = player.pos;
			el->fallstart = 140.f + float(count) * 0.333333f + rr * 5.f;
			el->fallend = 220.f + float(count) * 0.5f + rr * 5.f;
			el->intensity = 1.6f;
			el->m_exists = true;
			el->rgb = Color3f(0.01f * count, 0.009f * count, 0.008f * count);
		} else {
			el->m_exists = false;
		}
	}
	
	if(entities.player()->obj && entities.player()->obj->fastaccess.head_group_origin) {
		VertexId vertex = entities.player()->obj->fastaccess.head_group_origin;
		el->pos.y = entities.player()->obj->vertexWorldPositions[vertex].v.y;
	}
	
}

void Add3DBoom(const Vec3f & position) {
	
	Vec3f poss = position;
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &poss);
	
	{
		float dist = fdist(player.pos - Vec3f(0, 160.f, 0.f), position);
		if(dist < 300) {
			Vec3f vect = (player.pos - position - Vec3f(0.f, 160.f, 0.f)) / dist;
			player.physics.forces += vect * ((300.f - dist) * 0.0125f);
		}
	}
	
	for(Entity & item : entities.inScene(IO_ITEM)) {
		
		if(!item.obj || !item.obj->pbox) {
			continue;
		}
		
		for(size_t k = 0; k < item.obj->pbox->vert.size(); k++) {
			float dist = fdist(item.obj->pbox->vert[k].pos, position);
			if(dist < 300.f) {
				item.obj->pbox->active = 1;
				item.obj->pbox->stopcount = 0;
				Vec3f vect = (item.obj->pbox->vert[k].pos - position) / dist;
				item.obj->pbox->vert[k].velocity += vect * ((300.f - dist) * 10.f);
			}
		}
		
	}
}

void ARX_PARTICLES_ClearAll() {
	
	std::fill(g_particles, g_particles + MAX_PARTICLES, PARTICLE_DEF());
	ParticleCount = 0;
}

PARTICLE_DEF * createParticle(bool allocateWhilePaused) {
	
	if(!allocateWhilePaused && g_gameTime.isPaused()) {
		return nullptr;
	}
	
	for(size_t i = 0; i < MAX_PARTICLES; i++) {
		
		PARTICLE_DEF * pd = &g_particles[i];
		
		if(pd->exist) {
			continue;
		}
		
		ParticleCount++;
		pd->exist = true;
		pd->elapsed = 0;
		
		pd->rgb = Color3f::white;
		pd->tc = nullptr;
		pd->m_flags = 0;
		pd->source = nullptr;
		pd->move = Vec3f(0.f);
		pd->sizeDelta = 1.f;
		
		return pd;
	}
	
	return nullptr;
}

void MagFX(const Vec3f & pos, float size) {
	
	PARTICLE_DEF * pd = createParticle(true);
	if(!pd) {
		return;
	}
	
	pd->m_flags = PARTICLE_2D;
	pd->ov = pos + Vec3f(Random::getf(0.f, 6.f) - Random::getf(0.f, 12.f), Random::getf(0.f, 6.f) - Random::getf(0.f, 12.f), 0.f);
	pd->move = Vec3f(Random::getf(-6.f, 6.f), Random::getf(-8.f, 8.f), 0.f);
	pd->sizeDelta = 4.4f;
	pd->duration = Random::get(1500ms, 2400ms);
	pd->tc = g_particleTextures.healing;
	pd->rgb = Color3f::magenta;
	pd->size = 56.f * size;
	
}

void ARX_PARTICLES_Spawn_Splat(const Vec3f & pos, float dmgs, Color col) {
	
	for(long kk = 0; kk < 20; kk++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			return;
		}
		pd->m_flags = PARTICLE_SUB2 | SUBSTRACT | GRAVITY | PARTICLE_ZDEC;
		pd->ov = pos;
		pd->move = arx::randomVec(-11.5f, 11.5f);
		pd->duration = 1s + u32(dmgs) * 3ms;
		pd->tc = g_particleTextures.blood_splat;
		float power = (dmgs * (1.f / 60)) + .9f;
		pd->size = 0.3f + 0.01f * power;
		pd->sizeDelta = 0.2f + 0.3f * power;
		pd->rgb = Color3f(col);
	}
	
}


void ARX_PARTICLES_SpawnWaterSplash(const Vec3f & _ePos) {
	
	long nbParticles = Random::get(15, 35);
	for(long kk = 0; kk < nbParticles; kk++) {
		
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			return;
		}
		
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING | GRAVITY | SPLAT_WATER | PARTICLE_ZDEC;
		pd->m_rotation = 0.f; // TODO maybe remove ROTATING
		pd->ov = _ePos + Vec3f(30.f, -20.f, 30.f) * arx::randomVec3f();
		pd->move = arx::linearRand(Vec3f(-6.5f, -11.5f, -6.5f), Vec3f(6.5f, 0.f, 6.5f));
		pd->duration = Random::get(1000ms, 1300ms);
		pd->tc = g_particleTextures.water_drop[Random::get(0, 2)];
		pd->size = 0.4f;
		pd->rgb = Color3f::gray(Random::getf());
		
	}
	
}

void SpawnFireballTail(const Vec3f & poss, const Vec3f & vecto, float level, long flags) {
	
	if(!g_particleTextures.explo[0]) {
		return;
	}
	
	for(long nn = 0; nn < 2; nn++) {
		
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			return;
		}
		
		pd->m_flags = FIRE_TO_SMOKE | FADE_IN_AND_OUT | PARTICLE_ANIMATED | ROTATING;
		pd->m_rotation = Random::getf(0.f, 0.02f);
		pd->move = Vec3f(0.f, Random::getf(-3.f, 0.f), 0.f);
		pd->tc = g_particleTextures.explo[0];
		pd->rgb = Color3f::gray(.7f);
		pd->size = (level + Random::getf()) * 2.f;
		
		if(flags & 1) {
			pd->duration = Random::get(400ms, 500ms);
			pd->size *= 0.7f;
			pd->sizeDelta = level * 1.4f;
		} else {
			pd->sizeDelta = level * 2.f;
			pd->duration = Random::get(800ms, 900ms);
		}
		
		if(nn == 1) {
			unsigned delay = Random::getu(150, 250);
			pd->elapsed = -std::chrono::milliseconds(delay);
			pd->ov = poss + vecto * Vec3f(float(delay));
		} else {
			pd->ov = poss;
		}
		
	}
	
}

void LaunchFireballBoom(const Vec3f & poss, float level, Vec3f * direction, const Color3f * rgb) {
	
	level *= 1.6f;
	
	if(g_particleTextures.explo[0] == nullptr) {
		return;
	}
	
	PARTICLE_DEF * pd = createParticle(true);
	if(!pd) {
		return;
	}
	
	pd->m_flags = FIRE_TO_SMOKE | FADE_IN_AND_OUT | PARTICLE_ANIMATED | PARTICLE_ZDEC;
	pd->ov = poss;
	pd->move = (direction) ? *direction : Vec3f(0.f, Random::getf(-5.f, 0.f), 0.f);
	pd->duration = Random::get(1600ms, 2200ms);
	pd->tc = g_particleTextures.explo[0];
	pd->size = level * 3.f + Random::getf(0.f, 2.f);
	pd->sizeDelta = level * 3.f;
	if(rgb) {
		pd->rgb = *rgb;
	}
	
}

void spawnFireHitParticle(const Vec3f & poss, long type) {
	
	PARTICLE_DEF * pd = createParticle(true);
	if(pd) {
		pd->m_flags = PARTICLE_ZDEC;
		pd->ov = poss;
		pd->move = Vec3f(3.f, 4.f, 3.f) - Vec3f(6.f, 12.f, 6.f) * arx::randomVec3f();
		pd->duration = Random::get(600ms, 700ms);
		pd->tc = g_particleTextures.fire_hit;
		pd->size = Random::getf(100.f, 110.f) * ((type == 1) ? 2.f : 1.f);
		if(type == 1) {
			pd->rgb = Color3f(.4f, .4f, 1.f);
		}
		
		pd = createParticle(true);
		if(pd) {
			pd->m_flags = PARTICLE_ZDEC;
			pd->ov = poss;
			pd->move = Vec3f(3.f , 4.f, 3.f) - Vec3f(6.f, 12.f, 6.f) * arx::randomVec3f();
			pd->duration = Random::get(600ms, 700ms);
			pd->tc = g_particleTextures.fire_hit;
			pd->size = Random::getf(40.f, 70.f) * ((type == 1) ? 2.f : 1.f);
			if(type == 1) {
				pd->rgb = Color3f(.4f, .4f, 1.f);
			}
		}
		
	}
}

void spawn2DFireParticle(const Vec2f & pos, float scale) {
	
	PARTICLE_DEF * pd = createParticle(true);
	if(!pd) {
		return;
	}
	
	pd->m_flags = FIRE_TO_SMOKE | PARTICLE_2D;
	pd->ov = Vec3f(pos, 0.0000001f);
	pd->move = Vec3f(Random::getf(-1.5f, 1.5f), Random::getf(-6.f, -5.f), 0.f) * scale;
	pd->sizeDelta = 1.8f;
	pd->duration = Random::get(500ms, 900ms);
	pd->tc = g_particleTextures.fire2;
	pd->rgb = Color3f(1.f, .6f, .5f);
	pd->size = 14.f * scale;
	
}



void ARX_PARTICLES_Update()  {
	
	ARX_PROFILE_FUNC();
	
	if(!g_tiles) {
		return;
	}
	
	if(ParticleCount == 0) {
		return;
	}
	
	const GameInstant now = g_gameTime.now();
	
	ShortGameDuration delta = g_gameTime.lastFrameDuration();
	
	long pcc = ParticleCount;
	
	for(size_t i = 0; i < MAX_PARTICLES && pcc > 0; i++) {
		
		PARTICLE_DEF * part = &g_particles[i];
		if(!part->exist) {
			continue;
		}
		
		arx_assume(part->duration > 0 && part->duration <= ShortGameDuration::max() / 2);
		
		ShortGameDuration elapsed = part->elapsed;
		part->elapsed += delta;
		if(elapsed < 0) {
			if(part->elapsed >= 0) {
				Entity * target = entities.get(part->sourceionum);
				if((part->m_flags & DELAY_FOLLOW_SOURCE) && target) {
					part->ov = *part->source;
					Vec3f vector = (part->ov - target->pos) * Vec3f(1.f, 0.5f, 1.f);
					vector = glm::normalize(vector);
					part->move = vector * Vec3f(18.f, 5.f, 18.f) + arx::randomVec(-0.5f, 0.5f);
				}
			}
			continue;
		}
		
		if(!(part->m_flags & PARTICLE_2D) && !g_tiles->isInActiveTile(part->ov)) {
			part->exist = false;
			ParticleCount--;
			continue;
		}
		
		if(elapsed >= part->duration) {
			if((part->m_flags & FIRE_TO_SMOKE) && Random::getf() > 0.7f) {
				part->ov += part->move;
				part->duration = part->duration * 1.375f;
				part->m_flags &= ~FIRE_TO_SMOKE;
				part->tc = g_particleTextures.smoke;
				part->sizeDelta = glm::abs(part->sizeDelta * 2.4f);
				part->rgb = Color3f::gray(.45f);
				part->move *= 0.5f;
				part->size *= 1.f / 3;
				part->elapsed = delta;
				elapsed = 0;
			} else {
				part->exist = false;
				ParticleCount--;
				continue;
			}
		}
		
		float val = elapsed / 100ms;
		
		Vec3f in = part->ov + part->move * val;
		
		if(part->m_flags & GRAVITY) {
			in.y += 1.47f * val * val;
		}
		
		float fd = elapsed / part->duration;
		
		float r = 1.f - fd;
		if(part->m_flags & FADE_IN_AND_OUT) {
			r = (fd <= 0.5f) ? 2.f * fd : 2.f - 2.f * fd;
		}
		
		float size = part->size + part->sizeDelta * fd;
		
		if(!(part->m_flags & PARTICLE_2D)) {
			
			Sphere sp;
			sp.origin = in;
			
			Vec4f p = worldToClipSpace(in);
			float z = p.z / p.w;
			if(p.w <= 0.f || z > g_camera->cdepth * fZFogEnd) {
				continue;
			}
			
			if(part->m_flags & SPLAT_GROUND) {
				sp.radius = size * 10.f;
				if(CheckAnythingInSphere(sp, entities.player(), CAS_NO_NPC_COL)) {
					if(Random::getf() < 0.9f) {
						Color3f rgb = part->rgb;
						PolyBoomAddSplat(sp, rgb, 0);
					}
					part->exist = false;
					ParticleCount--;
					continue;
				}
			}
			
			if(part->m_flags & SPLAT_WATER) {
				sp.radius = size * Random::getf(10.f, 30.f);
				if(CheckAnythingInSphere(sp, entities.player(), CAS_NO_NPC_COL)) {
					if(Random::getf() < 0.9f) {
						Color3f rgb = part->rgb * 0.5f;
						PolyBoomAddSplat(sp, rgb, 2);
					}
					part->exist = false;
					ParticleCount--;
					continue;
				}
			}
			
			if((part->m_flags & DISSIPATING) && z < 0.05f) {
				r *= z * 20.f;
			}
		}
		
		if(r <= 0.f) {
			pcc--;
			continue;
		}
		
		if(part->m_flags & PARTICLE_GOLDRAIN) {
			float v = Random::getf(-0.1f, 0.1f);
			if(part->rgb.r + v <= 1.f && part->rgb.r + v > 0.f
				&& part->rgb.g + v <= 1.f && part->rgb.g + v > 0.f
				&& part->rgb.b + v <= 1.f && part->rgb.b + v > 0.f) {
				part->rgb = Color3f(part->rgb.r + v, part->rgb.g + v, part->rgb.b + v);
			}
		}
		
		Color color(part->rgb * r);
		if(player.m_improve) {
			color.g = 0;
		}
		
		TextureContainer * tc = part->tc;
		if(tc == g_particleTextures.explo[0] && (part->m_flags & PARTICLE_ANIMATED)) {
			long animrange = g_particleTextures.MAX_EXPLO - 1;
			tc = g_particleTextures.explo[glm::clamp(long(fd * float(animrange)), 0l, animrange)];
		}
		
		RenderMaterial mat;
		mat.setTexture(tc);
		mat.setDepthTest(!(part->m_flags & PARTICLE_NOZBUFFER));
		
		if(part->m_flags & PARTICLE_SUB2) {
			mat.setBlendType(RenderMaterial::Subtractive2);
			color.a = Color::Traits::convert(r * 1.5f);
		} else if(part->m_flags & SUBSTRACT) {
			mat.setBlendType(RenderMaterial::Subtractive);
		} else {
			mat.setBlendType(RenderMaterial::Additive);
		}
		
		float zpos = (part->m_flags & PARTICLE_ZDEC) ? 0.0001f : 2.f;
		
		if(part->m_flags & PARTICLE_2D) {
			EERIEAddBitmap(mat, in, size, size, tc, color);
		} else if(part->m_flags & ROTATING) {
			float rott = MAKEANGLE(float(toMsi(now + GameDuration(elapsed))) * part->m_rotation); // TODO wat
			EERIEAddSprite(mat, in, std::max(size, 0.f), color, zpos, rott);
		} else {
			EERIEAddSprite(mat, in, size, color, zpos);
		}
		
		pcc--;
	}
}

void RestoreAllLightsInitialStatus() {
	
	for(EERIE_LIGHT & light : g_staticLights) {
		light.m_ignitionStatus = !(light.extras & EXTRAS_STARTEXTINGUISHED);
		if(!light.m_ignitionStatus) {
			lightHandleDestroy(light.m_ignitionLightHandle);
		}
	}
	
}

// Draws Flame Particles
void TreatBackgroundActions() {
	
	ARX_PROFILE_FUNC();
	
	float fZFar = square(g_camera->cdepth * fZFogEnd * 1.3f);
	
	for(EERIE_LIGHT & light : g_staticLights) {
		
		float dist = arx::distance2(light.pos, g_camera->m_pos);
		if(dist > fZFar) {
			// Out of treat range
			ARX_SOUND_Stop(light.sample);
			light.sample = audio::SourcedSample();
			DamageRequestEnd(light.m_damage);
			light.m_damage = { };
			continue;
		}
		
		if((light.extras & EXTRAS_SPAWNFIRE) && light.m_ignitionStatus) {
			Spell * spell = nullptr; // TODO create a real spell for this?
			DamageParameters & damage = damageGet(spell, light.m_damage);
			damage.radius = light.ex_radius;
			damage.damages = light.ex_radius * 0.4f;
			damage.area = DAMAGE_FULL;
			damage.duration = GameDuration::max();
			damage.source = EntityHandle();
			damage.flags = 0;
			damage.type = DAMAGE_TYPE_FAKESPELL | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_NO_FIX;
			damage.pos = light.pos;
		} else if(light.m_damage) {
			DamageRequestEnd(light.m_damage);
			light.m_damage = { };
		}
		
		if(!(light.extras & (EXTRAS_SPAWNFIRE | EXTRAS_SPAWNSMOKE)) || !light.m_ignitionStatus) {
			if(!light.m_ignitionStatus) {
				ARX_SOUND_Stop(light.sample);
				light.sample = audio::SourcedSample();
			}
			continue;
		}
		
		if(light.sample == audio::SourcedSample()) {
			light.sample = ARX_SOUND_PlaySFX_loop(g_snd.FIREPLACE_LOOP, &light.pos, Random::getf(0.95f, 1.05f));
		} else {
			ARX_SOUND_RefreshPosition(light.sample, light.pos);
		}
		
		float amount = 0.122f;
		if(dist < square(g_camera->cdepth * (1.f / 6))) {
			amount = 0.183f;
		} else if(dist < square(g_camera->cdepth * (1.f / 3))) {
			amount = 0.1525f;
		}
		
		size_t count = light.m_storedFlameTime.update(toMsf(g_gameTime.lastFrameDuration()) * amount);
		
		for(size_t n = 0; n < count; n++) {
			
			if(Random::getf() < light.ex_frequency) {
				PARTICLE_DEF * pd = createParticle(true);
				if(pd) {
					float tX = Random::getf() * glm::pi<float>();
					float tY = Random::getf() * glm::pi<float>();
					float tZ = Random::getf() * glm::pi<float>();
					Vec3f s = Vec3f(std::cos(tX), std::cos(tY), std::cos(tZ)) * arx::randomVec();
					pd->ov = light.pos + s * light.ex_radius;
					pd->move = Vec3f(2.f, 2.f, 2.f) - Vec3f(4.f, 22.f, 4.f) * arx::randomVec3f();
					pd->move *= light.ex_speed;
					pd->size = 7.f * light.ex_size;
					auto max = 1000ms * light.ex_speed;
					pd->duration = 500ms + Random::get(0ms, std::chrono::duration_cast<std::chrono::milliseconds>(max));
					if((light.extras & EXTRAS_SPAWNFIRE) && (light.extras & EXTRAS_SPAWNSMOKE)) {
						pd->m_flags = FIRE_TO_SMOKE;
					}
					pd->tc = (light.extras & EXTRAS_SPAWNFIRE) ? g_particleTextures.fire2 : g_particleTextures.smoke;
					pd->m_flags |= ROTATING;
					pd->m_rotation = 0.1f - Random::getf(0.f, 0.2f) * light.ex_speed;
					pd->sizeDelta = -8.f;
					pd->rgb = (light.extras & EXTRAS_COLORLEGACY) ? light.rgb : Color3f::white;
				}
			}
			
			if(!(light.extras & EXTRAS_SPAWNFIRE)) {
				continue;
			}
			
			if(Random::getf() < light.ex_frequency * 0.05f) {
				PARTICLE_DEF * pd = createParticle(true);
				if(pd) {
					float t = Random::getf() * (glm::pi<float>() * 2.f) - glm::pi<float>();
					Vec3f s = Vec3f(std::sin(t), std::sin(t), std::cos(t)) * arx::randomVec();
					pd->ov = light.pos + s * light.ex_radius;
					Vec3f vect = glm::normalize(pd->ov - light.pos);
					float d = (light.extras & EXTRAS_FIREPLACE) ? 6.f : 4.f;
					pd->move = Vec3f(vect.x * d, Random::getf(-18.f, -10.f), vect.z * d) * light.ex_speed;
					pd->size = 4.f * light.ex_size * 0.3f;
					auto max = 500ms * light.ex_speed;
					pd->duration = 1200ms + Random::get(0ms, std::chrono::duration_cast<std::chrono::milliseconds>(max));
					pd->tc = g_particleTextures.fire2;
					pd->m_flags |= ROTATING | GRAVITY;
					pd->m_rotation = 0.1f - Random::getf(0.f, 0.2f) * light.ex_speed;
					pd->sizeDelta = -3.f;
					pd->rgb = (light.extras & EXTRAS_COLORLEGACY) ? light.rgb : Color3f::white;
				}
			}
			
		}
		
	}
	
}

