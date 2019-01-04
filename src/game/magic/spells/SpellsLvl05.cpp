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
#include "graphics/effects/PolyBoom.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells05.h"
#include "math/RandomVector.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

RuneOfGuardingSpell::RuneOfGuardingSpell()
	: m_pos(0.f)
	, tex_p2(NULL)
{ }

void RuneOfGuardingSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_RUNE_OF_GUARDING);
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_RUNE_OF_GUARDING);
	
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
		light->duration = GameDurationMs(200);
	}
}

void RuneOfGuardingSpell::End() {
	
	endLightDelayed(m_light, GameDurationMs(500));
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
		light->duration = GameDurationMs(200);
	}
	
	Vec3f pos = m_pos + Vec3f(0.f, -20.f, 0.f);
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef stiteangle;
	Color3f stitecolor;
	
	float stiteangleb = m_elapsed / GameDurationMs(100);
	stiteangle.setPitch(0);
	stiteangle.setRoll(0);
	
	stiteangle.setYaw(stiteangleb * 0.1f);
	stitecolor = Color3f(0.4f, 0.4f, 0.6f);
	float scale = std::sin(m_elapsed / GameDurationMsf(66.6666666f));
	Vec3f stitescale = Vec3f(1.f, -0.1f, 1.f);
	
	Draw3DObject(slight, stiteangle, pos, stitescale, stitecolor, mat);
	
	stiteangle.setYaw(stiteangleb);
	stitecolor = Color3f(0.6f, 0.f, 0.f);
	stitescale = Vec3f(2.f) * (1.f + 0.01f * scale);
	
	Draw3DObject(ssol, stiteangle, pos, stitescale, stitecolor, mat);
	
	stitecolor = Color3f(0.6f, 0.3f, 0.45f);
	stitescale = Vec3f(1.8f) * (1.f + 0.02f * scale);
	
	Draw3DObject(srune, stiteangle, pos, stitescale, stitecolor, mat);
	
	for(int n = 0; n < 4; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = pos + arx::randomOffsetXZ(40.f);
		pd->move = arx::linearRand(Vec3f(-0.8f, -4.f, -0.8f), Vec3f(0.8f, 0.f, 0.8f));
		pd->scale = Vec3f(-0.1f);
		pd->tolive = Random::getu(2600, 3200);
		pd->tc = tex_p2;
		pd->siz = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
	
	
	
	Sphere sphere = Sphere(m_pos, std::max(m_level * 15.f, 50.f));
	if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL
	                                           | CAS_NO_FIX_COL | CAS_NO_DEAD_COL)) {
		spawnFireHitParticle(m_pos, 0);
		PolyBoomAddScorch(m_pos);
		LaunchFireballBoom(m_pos, m_level);
		DoSphericDamage(Sphere(m_pos, 30.f * m_level), 4.f * m_level,
		                DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, m_caster);
		ARX_SOUND_PlaySFX(g_snd.SPELL_RUNE_OF_GUARDING_END, &m_pos);
		requestEnd();
	}
}

Vec3f RuneOfGuardingSpell::getPosition() {
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
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_LEVITATE_START, &entities[m_target]->pos);
	
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
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_LEVITATE_LOOP, &entities[m_target]->pos, 0.7f);
	
	m_targets.push_back(m_target);
}

void LevitateSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	Entity * target = entities.get(m_target);
	if(target) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_LEVITATE_END, &target->pos);
	}
	
	m_targets.clear();
	
	if(m_target == EntityHandle_Player)
		player.levitate = false;
}

void LevitateSpell::Update() {
	
	Vec3f target;

	if(m_target == EntityHandle_Player) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		player.levitate = true;
	} else {
		target = entities[m_caster]->pos;
	}
	
	m_pos = target;
	
	float coneScale = 0.f;
	int dustParticles = 0;
	
	if(m_elapsed < GameDurationMs(1000)) {
		coneScale = m_elapsed / GameDurationMs(1000);
		dustParticles = 3;
	} else {
		coneScale = 1.f;
		dustParticles = 10;
	}
	
	cone1.Update(g_gameTime.lastFrameDuration(), m_pos, coneScale);
	cone2.Update(g_gameTime.lastFrameDuration(), m_pos, coneScale);
	
	m_stones.Update(g_gameTime.lastFrameDuration(), m_pos);
	
	for(int i = 0; i < dustParticles; i++) {
		createDustParticle();
	}
	
	cone1.Render();
	cone2.Render();
	m_stones.DrawStone();
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void LevitateSpell::createDustParticle() {
	
	PARTICLE_DEF * pd = createParticle();
	if(!pd) {
		return;
	}
	
	Vec2f pos = arx::circularRand(m_baseRadius);
	
	pd->ov = m_pos + Vec3f(pos.x, 0.f, pos.y);
	float t = fdist(pd->ov, m_pos);
	Vec3f moveFactor = arx::linearRand(Vec3f(5.f, 0.f, 5.f), Vec3f(10.f, 3.f, 10.f));
	pd->move = moveFactor * Vec3f((pd->ov.x - m_pos.x) / t, 1.f, (pd->ov.z - m_pos.z) / t);
	pd->siz = Random::getf(30.f, 60.f);
	pd->tolive = 3000;
	pd->timcreation = -(toMsi(g_gameTime.now()) + 3000l); // TODO WTF
	pd->m_flags = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | DISSIPATING;
	pd->m_rotation = 0.0000001f;
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
		ARX_SOUND_PlaySFX(g_snd.SPELL_CURE_POISON);
	} else if(Entity * io = entities.get(m_target)) {
		if(io->ioflags & IO_NPC) {
			io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
		}
		ARX_SOUND_PlaySFX(g_snd.SPELL_CURE_POISON, &io->pos);
	}
	
	m_duration = GameDurationMs(3500);
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
		light->duration = GameDurationMs(200);
		light->extras = 0;
	}
}

void CurePoisonSpell::End() {
	
}

void CurePoisonSpell::Update() {
	
	m_pos = entities[m_target]->pos;
	
	if(m_target == EntityHandle_Player)
		m_pos.y += 200;
	
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
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallstart = 200.f;
		light->fallend   = 350.f;
		light->rgb = Color3f(0.4f, 1.f, 0.4f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = GameDurationMs(200);
		light->creationTime = g_gameTime.now();
		light->extras = 0;
	}
	
	m_particles.Render();
}

RepelUndeadSpell::RepelUndeadSpell()
	: m_pos(0.f)
	, m_yaw(0.f)
	, tex_p2(NULL)
{ }

void RepelUndeadSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_REPEL_UNDEAD);
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_REPEL_UNDEAD, &entities[m_target]->pos);
	if(m_target == EntityHandle_Player) {
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_REPEL_UNDEAD_LOOP, &entities[m_target]->pos, 1.f);
	}
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 1.f;
	
	m_pos = player.pos;
	m_yaw = 0.f;
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
}

void RepelUndeadSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	endLightDelayed(m_light, GameDurationMs(500));
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
	
	float wave = timeWaveSin(g_gameTime.now(), GameDurationMsf(6283.185307f));
	
	float vv = (1.f + wave) * 0.5f + 1.1f;
	
	Draw3DObject(ssol, eObjAngle, m_pos + Vec3f(0.f, -5.f, 0.f), Vec3f(vv), Color3f(0.6f, 0.6f, 0.8f), mat);
	
	vv *= 100.f;
	
	for(int n = 0; n < 4; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		// XXX was this supposed to be sphericalRand ?
		Vec2f d = arx::diskRand(vv);
		
		pd->ov = m_pos + Vec3f(d.x, 0.f, d.y);
		pd->move = arx::linearRand(Vec3f(-0.8f, -4.f, -0.8f), Vec3f(0.8f, 0.f, 0.8f));
		pd->scale = Vec3f(-0.1f);
		pd->tolive = Random::getu(2600, 3200);
		pd->tc = tex_p2;
		pd->siz = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 2.3f;
		light->fallend = 350.f;
		light->fallstart = 150.f;
		light->rgb = Color3f(0.8f, 0.8f, 1.f);
		light->pos = m_pos + Vec3f(0.f, -50.f, 0.f);
		light->duration = GameDurationMs(200);
		light->creationTime = g_gameTime.now();
	}
	
	if (m_target == EntityHandle_Player)
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}


PoisonProjectileSpell::~PoisonProjectileSpell() {
	
	for(size_t i = 0; i < m_projectiles.size(); i++) {
		CPoisonProjectile * projectile = m_projectiles[i];
		
		endLightDelayed(projectile->lLightId, GameDurationMs(2000));
		
		delete projectile;
	}
	m_projectiles.clear();
}

void PoisonProjectileSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_POISON_PROJECTILE_LAUNCH,
	                  &m_caster_pos);
	
	Vec3f srcPos(0.f);
	float afBeta = 0.f;
	
	Entity * caster = entities[m_caster];
	m_hand_group = caster->obj->fastaccess.primary_attach;

	if(m_hand_group != ActionPoint()) {
		ActionPoint group = m_hand_group;
		m_hand_pos = actionPointPosition(caster->obj, group);
	}
	
	if(m_caster == EntityHandle_Player) {

		afBeta = player.angle.getYaw();

		if(m_hand_group != ActionPoint()) {
			srcPos = m_hand_pos;
		} else {
			srcPos = player.pos;
		}
	} else {
		afBeta = entities[m_caster]->angle.getYaw();

		if(m_hand_group != ActionPoint()) {
			srcPos = m_hand_pos;
		} else {
			srcPos = entities[m_caster]->pos;
		}
	}
	
	srcPos += angleToVectorXZ(afBeta) * 90.f;
	
	size_t uiNumber = glm::clamp(static_cast<unsigned int>(m_level), 1u, 5u);
	
	for(size_t i = 0; i < uiNumber; i++) {
		CPoisonProjectile * projectile = new CPoisonProjectile();
		
		m_projectiles.push_back(projectile);
	}
	
	m_duration = GameDurationMs(8000);
	m_hasDuration = true;
	
	GameDuration lMax = 0;
	
	for(size_t i = 0; i < m_projectiles.size(); i++) {
		CPoisonProjectile * projectile = m_projectiles[i];
		
		projectile->Create(srcPos, afBeta + Random::getf(-10.f, 10.f));
		GameDuration lTime = m_duration + GameDurationMs(Random::getu(0, 5000));
		projectile->SetDuration(lTime);
		lMax = std::max(lMax, lTime);
		
		EERIE_LIGHT * light = dynLightCreate(projectile->lLightId);
		if(light) {
			light->intensity = 2.3f;
			light->fallend = 250.f;
			light->fallstart = 150.f;
			light->rgb = Color3f::green;
			light->pos = projectile->eSrc;
			light->creationTime = g_gameTime.now();
			light->duration = GameDurationMs(200);
		}
	}
	
	m_duration = lMax + GameDurationMs(1000);
}

void PoisonProjectileSpell::End() {
	
	for(size_t i = 0; i < m_projectiles.size(); i++) {
		CPoisonProjectile * projectile = m_projectiles[i];
		
		endLightDelayed(projectile->lLightId, GameDurationMs(2000));
		
		delete projectile;
	}
	m_projectiles.clear();
}

void PoisonProjectileSpell::Update() {
	
	for(size_t i = 0; i < m_projectiles.size(); i++) {
		CPoisonProjectile * projectile = m_projectiles[i];
		
		projectile->Update(g_gameTime.lastFrameDuration());
	}
	
	for(size_t i = 0; i < m_projectiles.size(); i++) {
		CPoisonProjectile * projectile = m_projectiles[i];
		
		projectile->Render();
		
		EERIE_LIGHT * light = lightHandleGet(projectile->lLightId);
		if(light) {
			light->intensity = 2.3f * projectile->lightIntensityFactor;
			light->fallend = 250.f;
			light->fallstart = 150.f;
			light->rgb = Color3f::green;
			light->pos = projectile->eCurPos;
			light->creationTime = g_gameTime.now();
			light->duration = GameDurationMs(200);
		}

		AddPoisonFog(projectile->eCurPos, m_level + 7);

		if(m_elapsed > GameDurationMs(1600)) {
			DamageParameters damage;
			damage.pos = projectile->eCurPos;
			damage.radius = 120.f;
			damage.damages = (4.f + m_level * 0.6f) * 0.001f * g_framedelay;
			damage.area = DAMAGE_FULL;
			damage.duration = g_gameTime.lastFrameDuration();
			damage.source = m_caster;
			damage.flags = 0;
			damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_POISON;
			DamageCreate(damage);
		}
		
	}
	
}

void PoisonProjectileSpell::AddPoisonFog(const Vec3f & pos, float power) {
	
	int iDiv = 4 - config.video.levelOfDetail;
	
	float flDiv = static_cast<float>(1 << iDiv);
	
	long count = std::max(1l, checked_range_cast<long>(g_framedelay / flDiv));
	while(count--) {
		
		if(Random::getf(0.f, 2000.f) >= power) {
			continue;
		}
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		
		float speed = 1.f;
		float fval = speed * 0.2f;
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->ov = pos + arx::randomVec(-100.f, 100.f);
		pd->scale = Vec3f(8.f, 8.f, 10.f);
		pd->move = Vec3f((speed - Random::getf()) * fval, (speed - speed * Random::getf()) * (1.f / 15),
		                 (speed - Random::getf()) * fval);
		pd->tolive = Random::getu(4500, 9000);
		pd->tc = TC_smoke;
		pd->siz = (80.f + Random::getf(0.f, 160.f)) * (1.f / 3);
		pd->rgb = Color3f(Random::getf(0.f, 1.f / 3), 1.f, Random::getf(0.f, 0.1f));
		pd->m_rotation = 0.001f;
		
	}
	
}
