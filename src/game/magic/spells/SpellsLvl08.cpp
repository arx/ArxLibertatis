/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include <glm/gtc/random.hpp>

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

#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool InvisibilitySpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void InvisibilitySpell::Launch()
{
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(6000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 3.f;
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}

	entities[m_target]->gameFlags |= GFLAG_INVISIBILITY;
	entities[m_target]->invisibility = 0.f;
	
	ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_START, &m_caster_pos);
	
	m_targets.push_back(m_target);
}

void InvisibilitySpell::End()
{
	if(ValidIONum(m_target)) {
		entities[m_target]->gameFlags &= ~GFLAG_INVISIBILITY;
		ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &entities[m_target]->pos);
		m_targets.clear();
	}
}

void InvisibilitySpell::Update() {
	
	if(m_target != EntityHandle_Player) {
		if(!(entities[m_target]->gameFlags & GFLAG_INVISIBILITY)) {
			m_targets.clear();
			ARX_SPELLS_Fizzle(this);
		}
	}	
}

Vec3f InvisibilitySpell::getPosition() {
	return getTargetPosition();
}


ManaDrainSpell::ManaDrainSpell()
	: m_light()
	, m_damage()
	, m_yaw(0.f)
{
	
}

bool ManaDrainSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void ManaDrainSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_LIFE_DRAIN);
	spells.endByCaster(m_caster, SPELL_HARM);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(6000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.f;
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &m_caster_pos, 1.2f, ARX_SOUND_PLAY_LOOPED);
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 8.f;
	damage.area = DAMAGE_FULL;
	damage.duration = ArxDurationMs(100000000);
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_MANA;
	m_damage = DamageCreate(damage);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::blue;
		light->pos = m_caster_pos;
		light->duration = ArxDurationMs(900);
	}
}

void ManaDrainSpell::End()
{
	DamageRequestEnd(m_damage);
	
	endLightDelayed(m_light, ArxDurationMs(600));
	
	ARX_SOUND_Stop(m_snd_loop);
}

// TODO copy-paste cabal
void ManaDrainSpell::Update() {
	
	float refpos;
	float scaley;
	
	if(m_caster == EntityHandle_Player)
		scaley = 90.f;
	else
		scaley = glm::abs(entities[m_caster]->physics.cyl.height * (1.0f/2)) + 30.f;
	
	const float frametime = float(arxtime.get_frame_time());
	
	float mov = std::sin(frametime * (1.0f/800)) * scaley;
	
	Vec3f cabalpos;
	if(m_caster == EntityHandle_Player) {
		cabalpos.x = player.pos.x;
		cabalpos.y = player.pos.y + 60.f - mov;
		cabalpos.z = player.pos.z;
		refpos = player.pos.y + 60.f;
	} else {
		cabalpos.x = entities[m_caster]->pos.x;
		cabalpos.y = entities[m_caster]->pos.y - scaley - mov;
		cabalpos.z = entities[m_caster]->pos.z;
		refpos = entities[m_caster]->pos.y - scaley;
	}
	
	float Es = std::sin(frametime * (1.0f/800) + glm::radians(scaley));
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		light->pos.x = cabalpos.x;
		light->pos.y = refpos;
		light->pos.z = cabalpos.z;
		light->rgb.b = Random::getf(0.8f, 1.f);
		light->fallstart = Es * 1.5f;
	}
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef cabalangle(0.f, 0.f, 0.f);
	cabalangle.setYaw(m_yaw + g_framedelay * 0.1f);
	m_yaw = cabalangle.getYaw();
	
	Vec3f cabalscale = Vec3f(Es);
	Color3f cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 30.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 60.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 120.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.f, 0.f, 0.15f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	cabalangle.setYaw(-cabalangle.getYaw());
	cabalpos.y = refpos - mov;
	cabalscale = Vec3f(Es);
	cabalcolor = Color3f(0.f, 0.f, 0.15f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 30.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 60.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 120.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	cabalangle.setYaw(-cabalangle.getYaw());
	
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalpos);
}

Vec3f ManaDrainSpell::getPosition() {
	return getTargetPosition();
}


ExplosionSpell::ExplosionSpell()
	: m_light()
	, m_damage()
{
}

void ExplosionSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_EXPLOSION);
	
	m_duration = ArxDurationMs(2000);
	
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
		light->rgb = Color3f(0.1f, 0.1f, 0.8f) + Color3f(1.f/3, 1.f/3, 1.f/5) * randomColor3f();
		light->pos = target;
		light->duration = ArxDurationMs(200);
	}
	
	AddQuakeFX(300, 2000, 400, true);
	
	for(long i_angle = 0 ; i_angle < 360 ; i_angle += 12) {
		for(long j = -100 ; j < 100 ; j += 50) {
			Vec3f dir = angleToVectorXZ(i_angle) * 60.f;
			
			Color3f color = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f/3, 1.f/3, 1.f/5);
			
			Vec3f posi = target + Vec3f(0.f, j * 2, 0.f);
			LaunchFireballBoom(posi, 16, &dir, &color);
		}
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND);
}

void ExplosionSpell::Update() {
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->rgb = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f/3, 1.f/3, 1.f/5);
		light->duration = ArxDurationMs(200);
		
		float choice = Random::getf();
		if(choice > .8f) {
			long lvl = Random::get(9, 13);
			
			Vec3f pos = light->pos + glm::sphericalRand(260.f);
			
			Color3f color = Color3f(0.1f, 0.1f, 0.8f) + randomColor3f() * Color3f(1.f/3, 1.f/3, 1.f/5);
			
			LaunchFireballBoom(pos, static_cast<float>(lvl), NULL, &color);
		} else if(choice > .6f) {
			Vec3f pos = light->pos + glm::sphericalRand(260.f);
			
			MakeCoolFx(pos);
		} else if(choice > 0.4f) {
			Vec3f pos = light->pos + glm::sphericalRand(160.f);

			ARX_PARTICLES_Add_Smoke(pos, 2, 20); // flag 1 = randomize pos
		}
	}	
}


void EnchantWeaponSpell::Launch()
{
	m_duration = ArxDurationMs(20);
}

void EnchantWeaponSpell::End() {

}

void EnchantWeaponSpell::Update() {
}


LifeDrainSpell::LifeDrainSpell()
	: m_light()
	, m_damage()
	, m_yaw(0.f)
{
}

bool LifeDrainSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void LifeDrainSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_HARM);
	spells.endByCaster(m_caster, SPELL_MANA_DRAIN);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(6000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 12.f;
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &m_caster_pos, 0.8f, ARX_SOUND_PLAY_LOOPED);
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = m_level * 0.08f;
	damage.area = DAMAGE_AREA;
	damage.duration = ArxDurationMs(100000000);
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_LIFE;
	m_damage = DamageCreate(damage);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = m_caster_pos;
		light->duration = ArxDurationMs(900);
	}
}

void LifeDrainSpell::End()
{
	DamageRequestEnd(m_damage);
	
	endLightDelayed(m_light, ArxDurationMs(600));
	
	ARX_SOUND_Stop(m_snd_loop);
}

// TODO copy-paste cabal
void LifeDrainSpell::Update() {
	
	float refpos;
	float scaley;
	
	if(m_caster == EntityHandle_Player)
		scaley = 90.f;
	else
		scaley = glm::abs(entities[m_caster]->physics.cyl.height * (1.0f/2)) + 30.f;
	
	const float frametime = float(arxtime.get_frame_time());
	
	float mov = std::sin(frametime * (1.0f/800)) * scaley;
	
	Vec3f cabalpos;
	if(m_caster == EntityHandle_Player) {
		cabalpos.x = player.pos.x;
		cabalpos.y = player.pos.y + 60.f - mov;
		cabalpos.z = player.pos.z;
		refpos = player.pos.y + 60.f;
	} else {
		cabalpos.x = entities[m_caster]->pos.x;
		cabalpos.y = entities[m_caster]->pos.y - scaley - mov;
		cabalpos.z = entities[m_caster]->pos.z;
		refpos = entities[m_caster]->pos.y - scaley;
	}
	
	float Es = std::sin(frametime * (1.0f/800) + glm::radians(scaley));
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		light->pos.x = cabalpos.x;
		light->pos.y = refpos;
		light->pos.z = cabalpos.z;
		light->rgb.r = Random::getf(0.8f, 1.f);
		light->fallstart = Es * 1.5f;
	}
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef cabalangle(0.f, 0.f, 0.f);
	cabalangle.setYaw(m_yaw + g_framedelay * 0.1f);
	m_yaw = cabalangle.getYaw();
	
	Vec3f cabalscale = Vec3f(Es);
	Color3f cabalcolor = Color3f(0.8f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 30.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.5f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 60.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.25f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime - 120.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.15f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	cabalangle.setYaw(-cabalangle.getYaw());
	cabalpos.y = refpos - mov;
	cabalscale = Vec3f(Es);
	cabalcolor = Color3f(0.15f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 30.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.25f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 60.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.5f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov = std::sin((frametime + 120.f) * (1.0f/800)) * scaley;
	cabalpos.y = refpos + mov;
	cabalcolor = Color3f(0.8f, 0.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	cabalangle.setYaw(-cabalangle.getYaw());
	
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalpos);
}

Vec3f LifeDrainSpell::getPosition() {
	return getTargetPosition();
}
