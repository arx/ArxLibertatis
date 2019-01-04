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

#include "game/magic/spells/SpellsLvl08.h"

#include "core/Application.h"
#include "core/Core.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"
#include "graphics/RenderBatcher.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleEffects.h"
#include "math/RandomVector.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool InvisibilitySpell::CanLaunch() {
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void InvisibilitySpell::Launch() {
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 3.f;
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}

	entities[m_target]->gameFlags |= GFLAG_INVISIBILITY;
	entities[m_target]->invisibility = 0.f;
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_INVISIBILITY_START, &m_caster_pos);
	
	m_targets.push_back(m_target);
}

void InvisibilitySpell::End() {
	Entity * target = entities.get(m_target);
	if(target) {
		target->gameFlags &= ~GFLAG_INVISIBILITY;
		ARX_SOUND_PlaySFX(g_snd.SPELL_INVISIBILITY_END, &target->pos);
		m_targets.clear();
	}
}

void InvisibilitySpell::Update() {
	
	if(m_target != EntityHandle_Player) {
		Entity * target = entities.get(m_target);
		if(target) {
			if(!(target->gameFlags & GFLAG_INVISIBILITY)) {
				m_targets.clear();
				ARX_SPELLS_Fizzle(this);
				spells.endSpell(this);
			}
		}
	}
}

Vec3f InvisibilitySpell::getPosition() {
	return getTargetPosition();
}


ManaDrainSpell::ManaDrainSpell() { }

bool ManaDrainSpell::CanLaunch() {
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void ManaDrainSpell::Launch() {
	spells.endByCaster(m_caster, SPELL_LIFE_DRAIN);
	spells.endByCaster(m_caster, SPELL_HARM);
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 2.f;
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_MAGICAL_SHIELD_LOOP, &m_caster_pos, 1.2f);
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 8.f;
	damage.area = DAMAGE_FULL;
	damage.duration = GameDurationMs(100000000);
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_MANA;
	m_damage = DamageCreate(damage);
	
	m_cabal.addRing(Color3f(0.4f, 0.4f, 0.8f));
	m_cabal.addRing(Color3f(0.2f, 0.2f, 0.5f));
	m_cabal.addRing(Color3f(0.1f, 0.1f, 0.25f));
	m_cabal.addRing(Color3f(0.f, 0.f, 0.15f));
	
	m_cabal.setLowerColorRndRange(Color3f(0.0f, 0.0f, 0.8f));
	m_cabal.setUpperColorRndRange(Color3f(0.0f, 0.0f, 1.0f));
	m_cabal.setStartLightColor(Color3f::blue);
	m_cabal.create(m_caster_pos);
}

void ManaDrainSpell::End() {
	DamageRequestEnd(m_damage);
	
	m_cabal.end();
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
}

void ManaDrainSpell::Update() {
	
	float scaley;
	float offset;
	Vec3f casterPos;
	
	if(m_caster == EntityHandle_Player) {
		scaley = 90.f;
		offset = 60.0f;
		casterPos = player.pos;
	} else {
		scaley = glm::abs(entities[m_caster]->physics.cyl.height * (1.0f / 2)) + 30.f;
		offset = -scaley;
		casterPos = entities[m_caster]->pos;
	}
	
	m_cabal.setYScale(scaley);
	m_cabal.setOffset(offset);
	
	Vec3f cabalPos = m_cabal.update(casterPos);
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalPos);
}

Vec3f ManaDrainSpell::getPosition() {
	return getTargetPosition();
}


ExplosionSpell::ExplosionSpell() { }

void ExplosionSpell::Launch() {
	ARX_SOUND_PlaySFX(g_snd.SPELL_EXPLOSION);
	
	m_duration = GameDurationMs(2000);
	m_hasDuration = true;
	
	Vec3f target = entities[m_caster]->pos;
	if(m_caster == EntityHandle_Player) {
		target.y += 60.f;
	} else {
		target.y -= 60.f;
	}
	
	DamageParameters damage;
	damage.radius = 350.f;
	damage.damages = 10.f;
	damage.area = DAMAGE_AREA;
	damage.duration = m_duration;
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
	damage.pos = target;
	m_damage = DamageCreate(damage);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f(0.1f, 0.1f, 0.8f) + Color3f(1.f / 3, 1.f / 3, 1.f / 5) * randomColor3f();
		light->pos = target;
		light->duration = GameDurationMs(200);
	}
	
	AddQuakeFX(300, GameDurationMs(2000), 400, true);
	
	for(long i_angle = 0 ; i_angle < 360 ; i_angle += 12) {
		for(long j = -100 ; j < 100 ; j += 50) {
			Vec3f dir = angleToVectorXZ(i_angle) * 60.f;
			
			Color3f color = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f / 3, 1.f / 3, 1.f / 5);
			
			Vec3f posi = target + Vec3f(0.f, j * 2, 0.f);
			LaunchFireballBoom(posi, 16, &dir, &color);
		}
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_WIND_LOOP);
}

void ExplosionSpell::Update() {
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		
		light->rgb = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f / 3, 1.f / 3, 0.2f);
		light->duration = GameDurationMs(200);
		
		float choice = Random::getf();
		if(choice > .8f) {
			long lvl = Random::get(9, 13);
			Vec3f pos = light->pos + arx::sphericalRand(260.f);
			Color3f color = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f / 3, 1.f / 3, 0.2f);
			LaunchFireballBoom(pos, static_cast<float>(lvl), NULL, &color);
		} else if(choice > .6f) {
			Vec3f pos = light->pos + arx::sphericalRand(260.f);
			MakeCoolFx(pos);
		} else if(choice > 0.4f) {
			Vec3f pos = light->pos + arx::sphericalRand(160.f);
			ARX_PARTICLES_Add_Smoke(pos, 2, 20); // flag 1 = randomize pos
		}
		
	}
	
}

void EnchantWeaponSpell::Launch() {
	m_duration = GameDurationMs(20);
	m_hasDuration = true;
}

void EnchantWeaponSpell::End() { }

void EnchantWeaponSpell::Update() { }


LifeDrainSpell::LifeDrainSpell() { }

bool LifeDrainSpell::CanLaunch() {
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void LifeDrainSpell::Launch() {
	spells.endByCaster(m_caster, SPELL_HARM);
	spells.endByCaster(m_caster, SPELL_MANA_DRAIN);
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 12.f;
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_MAGICAL_SHIELD_LOOP, &m_caster_pos, 0.8f);
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = m_level * 0.08f;
	damage.area = DAMAGE_AREA;
	damage.duration = GameDurationMs(100000000);
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_LIFE;
	m_damage = DamageCreate(damage);
	
	m_cabal.addRing(Color3f(0.8f, 0.0f, 0.0f));
	m_cabal.addRing(Color3f(0.5f, 0.0f, 0.0f));
	m_cabal.addRing(Color3f(0.25f, 0.0f, 0.0f));
	m_cabal.addRing(Color3f(0.15f, 0.0f, 0.0f));
	
	m_cabal.setLowerColorRndRange(Color3f(0.8f, 0.0f, 0.0f));
	m_cabal.setUpperColorRndRange(Color3f(1.0f, 0.0f, 0.0f));
	m_cabal.setStartLightColor(Color3f::red);
	m_cabal.create(m_caster_pos);
}

void LifeDrainSpell::End() {
	DamageRequestEnd(m_damage);
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	m_cabal.end();
}

void LifeDrainSpell::Update() {
	
	float scaley;
	float offset;
	Vec3f casterPos;
	
	if(m_caster == EntityHandle_Player) {
		scaley = 90.f;
		offset = 60.0f;
		casterPos = player.pos;
	} else {
		scaley = glm::abs(entities[m_caster]->physics.cyl.height * (1.0f / 2)) + 30.f;
		offset = -scaley;
		casterPos = entities[m_caster]->pos;
	}
	
	m_cabal.setYScale(scaley);
	m_cabal.setOffset(offset);
	
	Vec3f cabalPos = m_cabal.update(casterPos);
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalPos);
}

Vec3f LifeDrainSpell::getPosition() {
	return getTargetPosition();
}
