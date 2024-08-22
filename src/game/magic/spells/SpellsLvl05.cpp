/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/spells/SpellsLvl05.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/ParticleSystems.h"

#include "graphics/effects/Decal.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"
#include "graphics/spells/Spells05.h"

#include "math/RandomVector.h"

#include "physics/Collisions.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "util/Range.h"


RuneOfGuardingSpell::RuneOfGuardingSpell()
	: m_pos(0.f)
	, tex_p2(nullptr)
{ }

void RuneOfGuardingSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_RUNE_OF_GUARDING);
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_RUNE_OF_GUARDING);
	}
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	
	m_pos = entities[m_caster]->pos;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 0.7f + 2.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(1.0f, 0.2f, 0.2f);
		light->pos = m_pos - Vec3f(0.f, 50.f, 0.f);
		light->creationTime = g_gameTime.now();
		light->duration = 200ms;
	}
}

void RuneOfGuardingSpell::End() {
	
	endLightDelayed(m_light, 500ms);
}

void RuneOfGuardingSpell::Update() {
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		float fa = Random::getf(0.85f, 1.0f);
		light->intensity = 0.7f + 2.3f * fa;
		light->fallend = 350.f;
		light->fallstart = 150.f;
		light->rgb = Color3f(1.0f, 0.2f, 0.2f);
		light->creationTime = g_gameTime.now();
		light->duration = 200ms;
	}
	
	Vec3f pos = m_pos + Vec3f(0.f, -20.f, 0.f);
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef stiteangle;
	Color3f stitecolor;
	
	float stiteangleb = m_elapsed / 100ms;
	stiteangle.setPitch(0);
	stiteangle.setRoll(0);
	
	stiteangle.setYaw(stiteangleb * 0.1f);
	stitecolor = Color3f(0.4f, 0.4f, 0.6f);
	float scale = std::sin(m_elapsed / (GameDuration(200ms) / 3));
	Vec3f stitescale = Vec3f(1.f, -0.1f, 1.f);
	
	Draw3DObject(slight.get(), stiteangle, pos, stitescale, stitecolor, mat);
	
	stiteangle.setYaw(stiteangleb);
	stitecolor = Color3f(0.6f, 0.f, 0.f);
	stitescale = Vec3f(2.f) * (1.f + 0.01f * scale);
	
	Draw3DObject(ssol.get(), stiteangle, pos, stitescale, stitecolor, mat);
	
	stitecolor = Color3f(0.6f, 0.3f, 0.45f);
	stitescale = Vec3f(1.8f) * (1.f + 0.02f * scale);
	
	Draw3DObject(srune.get(), stiteangle, pos, stitescale, stitecolor, mat);
	
	size_t count = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.12f);
	for(size_t i = 0; i < count; i++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		pd->ov = pos + arx::randomOffsetXZ(40.f);
		pd->move = arx::linearRand(Vec3f(-0.8f, -4.f, -0.8f), Vec3f(0.8f, 0.f, 0.8f));
		pd->sizeDelta = -0.1f;
		pd->duration = Random::get(2600ms, 3200ms);
		pd->tc = tex_p2;
		pd->size = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
	
	Sphere sphere = Sphere(m_pos, std::max(m_level * 15.f, 50.f));
	Entity * caster = entities.get(m_caster);
	if(CheckAnythingInSphere(sphere, caster, CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL
	                                         | CAS_NO_FIX_COL | CAS_NO_DEAD_COL)) {
		spawnFireHitParticle(m_pos, 0);
		PolyBoomAddScorch(m_pos);
		LaunchFireballBoom(m_pos, m_level);
		doSphericDamage(Sphere(m_pos, 30.f * m_level), 4.f * m_level,
		                DAMAGE_AREA, this, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, caster);
		if(emitsSound()) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_RUNE_OF_GUARDING_END, &m_pos);
		}
		requestEnd();
	}
	
}

Vec3f RuneOfGuardingSpell::getPosition() const {
	return m_pos;
}

LevitateSpell::LevitateSpell()
	: m_pos(0.f)
	, m_baseRadius(50.f)
{ }

void LevitateSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_LEVITATE);
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_LEVITATE_START, &entities[m_target]->pos);
	}
	
	m_fManaCostPerSecond = 1.f;
	
	Vec3f target;
	if(m_target == EntityHandle_Player) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		m_duration = 0;
		m_hasDuration = false;
		player.levitate = true;
	} else {
		target = entities[m_target]->pos;
		m_hasDuration = m_launchDuration >= 0;
		m_duration = m_hasDuration ? m_launchDuration : 0;
	}
	
	m_pos = target;
	
	float rhaut = 100.f;
	float hauteur = 80.f;
	
	cone1.Init(m_baseRadius, rhaut, hauteur);
	cone2.Init(m_baseRadius, rhaut * 1.5f, hauteur * 0.5f);
	m_stones.Init(m_baseRadius);
	
	if(emitsSound()) {
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_LEVITATE_LOOP, &entities[m_target]->pos, 0.7f);
	}
	
	m_targets.push_back(m_target);
}

void LevitateSpell::End() {
	
	if(emitsSound()) {
		ARX_SOUND_Stop(m_snd_loop);
		m_snd_loop = audio::SourcedSample();
		
		Entity * target = entities.get(m_target);
		if(target) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_LEVITATE_END, &target->pos);
		}
	}
	
	m_targets.clear();
	
	if(m_target == EntityHandle_Player)
		player.levitate = false;
}

void LevitateSpell::Update() {
	
	Vec3f target = m_caster_pos;

	if(m_target == EntityHandle_Player) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		player.levitate = true;
	} else if(m_caster != EntityHandle()) {
		target = entities[m_caster]->pos;
	}
	
	m_pos = target;
	
	float coneScale = 0.f;
	size_t dustParticles = 0;
	
	if(m_elapsed < 1s) {
		coneScale = m_elapsed / 1s;
		dustParticles = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.09f);
	} else {
		coneScale = 1.f;
		dustParticles = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.3f);
	}
	
	cone1.Update(g_gameTime.lastFrameDuration(), m_pos, coneScale);
	cone2.Update(g_gameTime.lastFrameDuration(), m_pos, coneScale);
	
	m_stones.Update(g_gameTime.lastFrameDuration(), m_pos);
	
	for(size_t i = 0; i < dustParticles; i++) {
		createDustParticle();
	}
	
	cone1.Render();
	cone2.Render();
	m_stones.DrawStone();
	
	if(emitsSound()) {
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}
}

void LevitateSpell::createDustParticle() {
	
	if(Random::getf() <= 0.7f) {
		return;
	}
	PARTICLE_DEF * pd = createParticle(true);
	if(!pd) {
		return;
	}
	Vec3f p = toXZ(arx::circularRand(1.f));
	Vec3f move = arx::linearRand(Vec3f(5.f, 0.f, 5.f), Vec3f(10.f, 3.f, 10.f)) * (p + Vec3f(0.f, 1.f, 0.f));
	pd->ov = m_pos + p * m_baseRadius + move;
	pd->move = move * 0.5f;
	pd->size = Random::getf(10.f, 20.f);
	pd->sizeDelta = 2.4f;
	pd->duration = 4125ms;
	pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
	pd->m_rotation = 0.0000001f;
	pd->tc = g_particleTextures.smoke;
	pd->rgb = Color3f::gray(.45f);
	
}

CurePoisonSpell::CurePoisonSpell()
	: m_pos(0.f)
{ }

void CurePoisonSpell::Launch() {
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	float cure = m_level * 10;
	if(m_target == EntityHandle_Player) {
		player.poison -= std::min(player.poison, cure);
		if(emitsSound()) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_CURE_POISON);
		}
	} else if(Entity * io = entities.get(m_target)) {
		if(io->ioflags & IO_NPC) {
			io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
		}
		if(emitsSound()) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_CURE_POISON, &io->pos);
		}
	}
	
	m_duration = 3500ms;
	m_hasDuration = true;
	
	m_particles.SetParams(g_particleParameters[ParticleParam_CurePoison]);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 1.5f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.f, 1.f, 0.0f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->creationTime = g_gameTime.now();
		light->duration = 200ms;
		light->extras = 0;
	}
}

void CurePoisonSpell::Update() {
	
	m_pos = entities[m_target]->pos;
	
	if(m_target == EntityHandle_Player)
		m_pos.y += 200;
	
	GameDuration ff = m_duration - m_elapsed;
	
	if(ff < 1500ms) {
		
		m_particles.m_parameters.m_spawnFlags = PARTICLE_CIRCULAR;
		m_particles.m_parameters.m_gravity = Vec3f(0.f);
		
		for(Particle & particle : m_particles.m_particles) {
			if(particle.isAlive()) {
				particle.fColorEnd.a = 0;
				if(particle.m_age + ff < particle.m_timeToLive) {
					particle.m_age = particle.m_timeToLive - ff;
				}
			}
		}
		
	}
	
	m_particles.SetPos(m_pos);
	m_particles.Update(g_gameTime.lastFrameDuration());
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 1.f, 0.4f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = 200ms;
		light->creationTime = g_gameTime.now();
		light->extras = 0;
	}
	
	m_particles.Render();
}

RepelUndeadSpell::RepelUndeadSpell()
	: m_pos(0.f)
	, m_yaw(0.f)
	, tex_p2(nullptr)
{ }

void RepelUndeadSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_REPEL_UNDEAD);
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_REPEL_UNDEAD, &entities[m_target]->pos);
		if(m_target == EntityHandle_Player) {
			m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_REPEL_UNDEAD_LOOP, &entities[m_target]->pos, 1.f);
		}
	}
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 1.f;
	
	m_pos = player.pos;
	m_yaw = 0.f;
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
}

void RepelUndeadSpell::End() {
	
	if(emitsSound()) {
		ARX_SOUND_Stop(m_snd_loop);
		m_snd_loop = audio::SourcedSample();
	}
	
	endLightDelayed(m_light, 500ms);
}

void RepelUndeadSpell::Update() {
	
	Vec3f pos = entities[m_target]->pos;
	
	float rot;
	if(m_target == EntityHandle_Player) {
		rot = player.angle.getYaw();
	} else {
		rot = entities[m_target]->angle.getYaw();
	}
	
	m_pos = pos;
	m_yaw = rot;
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef  eObjAngle;

	eObjAngle.setYaw(m_yaw);
	eObjAngle.setPitch(0);
	eObjAngle.setRoll(0);
	
	float wave = timeWaveSin(g_gameTime.now(), 2s * glm::pi<float>());
	
	float vv = (1.f + wave) * 0.5f + 1.1f;
	
	Draw3DObject(ssol.get(), eObjAngle, m_pos + Vec3f(0.f, -5.f, 0.f), Vec3f(vv), Color3f(0.6f, 0.6f, 0.8f), mat);
	
	size_t count = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.12f);
	for(size_t i = 0; i < count; i++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		// XXX was this supposed to be sphericalRand ?
		pd->ov = m_pos + toXZ(arx::diskRand(vv * 100.f));
		pd->move = arx::linearRand(Vec3f(-0.8f, -4.f, -0.8f), Vec3f(0.8f, 0.f, 0.8f));
		pd->sizeDelta = -0.1f;
		pd->duration = Random::get(2600ms, 3200ms);
		pd->tc = tex_p2;
		pd->size = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 350.f;
		light->fallstart = 150.f;
		light->rgb = Color3f(0.8f, 0.8f, 1.f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = 200ms;
		light->creationTime = g_gameTime.now();
	}
	
	if(emitsSound() && m_target == EntityHandle_Player) {
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}
	
}


PoisonProjectileSpell::~PoisonProjectileSpell() {
	
	for(const CPoisonProjectile & projectile : util::dereference(m_projectiles)) {
		endLightDelayed(projectile.lLightId, 2s);
	}
	
}

void PoisonProjectileSpell::Launch() {
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_POISON_PROJECTILE_LAUNCH, &m_caster_pos);
	}
	
	Vec3f srcPos(0.f);
	float afBeta = 0.f;
	
	Entity * caster = entities[m_caster];
	
	m_hand_group = caster->obj->fastaccess.primary_attach;
	if(m_hand_group) {
		m_hand_pos = caster->obj->vertexWorldPositions[m_hand_group].v;
	}
	
	if(m_caster == EntityHandle_Player) {
		afBeta = player.angle.getYaw();
		srcPos = m_hand_group ? m_hand_pos : player.pos;
	} else {
		afBeta = entities[m_caster]->angle.getYaw();
		srcPos = m_hand_group ? m_hand_pos : entities[m_caster]->pos;
	}
	
	srcPos += angleToVectorXZ(afBeta) * 90.f;
	
	size_t uiNumber = glm::clamp(static_cast<unsigned int>(m_level), 1u, 5u);
	m_projectiles.reserve(uiNumber);
	for(size_t i = 0; i < uiNumber; i++) {
		m_projectiles.emplace_back(std::make_unique<CPoisonProjectile>());
	}
	
	m_duration = 8s;
	m_hasDuration = true;
	
	GameDuration lMax = 0;
	
	for(CPoisonProjectile & projectile : util::dereference(m_projectiles)) {
		
		projectile.Create(srcPos, afBeta + Random::getf(-10.f, 10.f));
		GameDuration lTime = m_duration + Random::get(0ms, 5000ms);
		projectile.SetDuration(lTime);
		lMax = std::max(lMax, lTime);
		
		EERIE_LIGHT * light = dynLightCreate(projectile.lLightId);
		if(light) {
			light->intensity = 2.3f;
			light->fallend = 250.f;
			light->fallstart = 150.f;
			light->rgb = Color3f::green;
			light->pos = projectile.eSrc;
			light->creationTime = g_gameTime.now();
			light->duration = 200ms;
		}
		
		projectile.fogId = ARX_FOGS_GetFree();
		FOG_DEF & fog = g_fogs[projectile.fogId];
		fog.frequency = m_level + 7;
		fog.speedRandom = 1.f;
		fog.rotatespeed = 0.001f;
		fog.sizeDelta = 8.f;
		fog.duration = 4500ms;
		fog.rgb = Color3f(0, 1.f, 0);
		fog.rgbRandom = Color3f(1.f / 3, 0.f, 0.1f);
		fog.size = 80.f;
		fog.creationTime = m_timcreation;
		
	}
	
	m_duration = lMax + 1s;
}

void PoisonProjectileSpell::End() {
	
	for(const CPoisonProjectile & projectile : util::dereference(m_projectiles)) {
		
		endLightDelayed(projectile.lLightId, 2s);
		
		FOG_DEF & fog = g_fogs[projectile.fogId];
		fog.exist = false;
		
	}
	
}

void PoisonProjectileSpell::Update() {
	
	for(CPoisonProjectile & projectile : util::dereference(m_projectiles)) {
		projectile.Update(g_gameTime.lastFrameDuration());
	}
	
	for(CPoisonProjectile & projectile : util::dereference(m_projectiles)) {
		
		projectile.Render();
		
		EERIE_LIGHT * light = lightHandleGet(projectile.lLightId);
		if(light) {
			light->intensity = 2.3f * projectile.lightIntensityFactor;
			light->fallend = 250.f;
			light->fallstart = 150.f;
			light->rgb = Color3f::green;
			light->pos = projectile.eCurPos;
			light->creationTime = g_gameTime.now();
			light->duration = 200ms;
		}
		
		FOG_DEF & fog = g_fogs[projectile.fogId];
		fog.pos = projectile.eCurPos;
		
		if(m_elapsed > 1600ms) {
			DamageParameters damage;
			damage.pos = projectile.eCurPos;
			damage.radius = 120.f;
			damage.damages = (4.f + m_level * 0.6f) * 0.001f * g_framedelay;
			damage.area = DAMAGE_FULL;
			damage.duration = g_gameTime.lastFrameDuration();
			damage.source = m_caster;
			damage.flags = 0;
			damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_POISON;
			DamageCreate(this, damage);
		}
		
	}
	
}
