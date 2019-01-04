/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/spells/SpellsLvl02.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/ParticleSystems.h"
#include "graphics/RenderBatcher.h"
#include "graphics/Renderer.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "io/log/Logger.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

HealSpell::HealSpell()
	: m_pos(0.f)
{ }

bool HealSpell::CanLaunch() {
	
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void HealSpell::Launch() {
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_HEALING, &m_caster_pos);
	}
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.4f * m_level;
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(3500);
	
	if(m_caster == EntityHandle_Player) {
		m_pos = player.pos;
	} else {
		m_pos = entities[m_caster]->pos;
	}
	
	m_particles.SetPos(m_pos);
	
	m_particles.SetParams(g_particleParameters[ParticleParam_Heal]);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 0.4f, 1.0f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = GameDurationMs(200);
		light->extras = 0;
	}
}

void HealSpell::End() {
	
}

void HealSpell::Update() {
	
	if(m_caster == EntityHandle_Player) {
		m_pos = player.pos;
	} else if(Entity * target = entities.get(m_target)) {
		m_pos = target->pos;
	}
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 0.4f, 1.0f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = GameDurationMs(200);
		light->extras = 0;
	}

	GameDuration ff = m_duration - m_elapsed;
	
	if(ff < GameDurationMs(1500)) {
		m_particles.m_parameters.m_spawnFlags = PARTICLE_CIRCULAR;
		m_particles.m_parameters.m_gravity = Vec3f(0.f);

		std::list<Particle *>::iterator i;

		for(i = m_particles.listParticle.begin(); i != m_particles.listParticle.end(); ++i) {
			Particle * pP = *i;

			if(pP->isAlive()) {
				pP->fColorEnd.a = 0;

				if(pP->m_age + ff < pP->m_timeToLive) {
					pP->m_age = pP->m_timeToLive - ff;
				}
			}
		}
	}

	m_particles.SetPos(m_pos);
	m_particles.Update(g_gameTime.lastFrameDuration());
	m_particles.Render();
	
	for(size_t ii = 0; ii < entities.size(); ii++) {
		const EntityHandle handle = EntityHandle(ii);
		Entity * e = entities[handle];
		
		if(e && e->show == SHOW_FLAG_IN_SCENE && (e->gameFlags & GFLAG_ISINTREATZONE) && (e->ioflags & IO_NPC)
		   && e->_npcdata->lifePool.current > 0.f) {
			
			float dist;
			if(handle == m_caster) {
				dist = 0;
			} else {
				dist = fdist(m_pos, e->pos);
			}
			
			if(dist < 300.f) {
				float gain = Random::getf(0.8f, 2.4f) * m_level * (300.f - dist) * (1.f / 300) * g_framedelay * 0.001f;
				if(handle == EntityHandle_Player) {
					if(!BLOCK_PLAYER_CONTROLS) {
						player.lifePool.current = std::min(player.lifePool.current + gain, player.Full_maxlife);
					}
				} else {
					e->_npcdata->lifePool.current = std::min(e->_npcdata->lifePool.current + gain, e->_npcdata->lifePool.max);
				}
			}
			
		}
		
	}
	
}

void DetectTrapSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_DETECT_TRAP);
	
	if(m_caster == EntityHandle_Player) {
		m_target = m_caster;
		if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
			ARX_SOUND_PlayInterface(g_snd.SPELL_DETECT_TRAP);
			m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_DETECT_TRAP_LOOP, &m_caster_pos, 1.f);
		}
	}
	
	m_duration = GameDurationMs(60000);
	m_fManaCostPerSecond = 0.4f;
	m_hasDuration = true;
	
	m_targets.push_back(m_target);
}

void DetectTrapSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	m_targets.clear();
}

void DetectTrapSpell::Update() {
	
	if(m_caster == EntityHandle_Player) {
		Vec3f pos = ARX_PLAYER_FrontPos();
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
	}
}


void ArmorSpell::Launch()
{
	spells.endByTarget(m_target, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_FIRE_PROTECTION);
	spells.endByCaster(m_caster, SPELL_COLD_PROTECTION);
	
	if(m_caster == EntityHandle_Player) {
		m_target = m_caster;
	}
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_ARMOR_START, &entities[m_target]->pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_ARMOR_LOOP, &entities[m_target]->pos, 1.f);
	
	if(m_caster == EntityHandle_Player) {
		m_duration = 0;
		m_hasDuration = false;
	} else {
		m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(20000);
		m_hasDuration = true;
	}
	
	m_fManaCostPerSecond = 0.2f * m_level;
	
	Entity * io = entities.get(m_target);
	if(io) {
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.5f, 0.5f, 0.25f);
		io->halo.radius = 45.f;
	}
	
	m_targets.push_back(m_target);
}

void ArmorSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	Entity * target = entities.get(m_target);
	if(target) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_ARMOR_END, &target->pos);
		ARX_HALO_SetToNative(target);
	}
	
	m_targets.clear();
}

void ArmorSpell::Update() {
	
	Entity * io = entities.get(m_target);
	if(io) {
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.5f, 0.5f, 0.25f);
		io->halo.radius = 45.f;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f ArmorSpell::getPosition() {
	return getTargetPosition();
}

LowerArmorSpell::LowerArmorSpell()
	: m_haloCreated(false)
{
	
}

void LowerArmorSpell::Launch() {
	
	spells.endByTarget(m_target, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_FIRE_PROTECTION);
	spells.endByCaster(m_caster, SPELL_COLD_PROTECTION);
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_LOWER_ARMOR, &entities[m_target]->pos);
	}
	
	if(m_caster == EntityHandle_Player) {
		m_duration = 0;
		m_hasDuration = false;
	} else {
		m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(20000);
		m_hasDuration = true;
	}
	
	m_fManaCostPerSecond = 0.2f * m_level;
	
	Entity * io = entities.get(m_target);
	if(io) {
		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color = Color3f(1.f, 0.05f, 0.0f);
			io->halo.radius = 45.f;
			
			m_haloCreated = true;
		} else {
			m_haloCreated = false;
		}
	}
	
	m_targets.push_back(m_target);
}

void LowerArmorSpell::End() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_LOWER_ARMOR_END);
	
	if(m_haloCreated) {
		Entity * io = entities.get(m_target);
		if(io) {
			io->halo.flags &= ~HALO_ACTIVE;
			ARX_HALO_SetToNative(io);
		}
	}
	
	m_targets.clear();
}

void LowerArmorSpell::Update() {
	
	Entity * io = entities.get(m_target);
	if(io) {
		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color = Color3f(1.f, 0.05f, 0.0f);
			io->halo.radius = 45.f;
			
			m_haloCreated = true;
		}
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f LowerArmorSpell::getPosition() {
	return getTargetPosition();
}

HarmSpell::HarmSpell() { }

void HarmSpell::Launch() {
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_HARM, &m_caster_pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_MAGICAL_SHIELD_LOOP, &m_caster_pos, 1.f);
	
	spells.endByCaster(m_caster, SPELL_LIFE_DRAIN);
	spells.endByCaster(m_caster, SPELL_MANA_DRAIN);
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 0.4f;

	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 4.f;
	damage.area = DAMAGE_FULL;
	damage.duration = GameDurationMs(100000000);
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
	m_damage = DamageCreate(damage);
	
	m_cabal.addRing(Color3f(0.8f, 0.4f, 0.f));
	m_cabal.addRing(Color3f(0.5f, 3.f, 0.f));
	m_cabal.addRing(Color3f(0.25f, 0.1f, 0.f));
	m_cabal.addRing(Color3f(0.15f, 0.1f, 0.f));
	m_cabal.disableSecondRingSet();
	
	m_cabal.setLowerColorRndRange(Color3f(0.8f, 0.6f, 0.0f));
	m_cabal.setUpperColorRndRange(Color3f(1.0f, 0.8f, 0.0f));
	m_cabal.setStartLightColor(Color3f::red);
	m_cabal.create(m_caster_pos);
}

void HarmSpell::End() {
	
	DamageRequestEnd(m_damage);
	
	m_cabal.end();
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
}

void HarmSpell::Update() {
	
	float scaley = 90.f;
	float offset = 60.f;
	if(m_caster != EntityHandle_Player) {
		scaley = glm::abs(entities[m_caster]->physics.cyl.height * (1.0f / 2)) + 30.f;
		offset = -scaley;
	}
	m_cabal.setYScale(scaley);
	m_cabal.setOffset(offset);
	
	Vec3f casterPos = (m_caster == EntityHandle_Player) ? player.pos : entities[m_caster]->pos;
	Vec3f cabalPos = m_cabal.update(casterPos);
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalPos);
}
