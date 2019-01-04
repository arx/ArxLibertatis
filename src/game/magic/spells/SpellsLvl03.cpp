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

#include "game/magic/spells/SpellsLvl03.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/ParticleSystems.h"
#include "graphics/Raycast.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "math/RandomVector.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Object.h"


SpeedSpell::~SpeedSpell() {
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		delete m_trails[i].trail;
	}
	m_trails.clear();
}

void SpeedSpell::Launch() {
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.f;
	
	if(m_caster == EntityHandle_Player) {
		m_target = m_caster;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_SPEED_START, &entities[m_target]->pos);
	
	if(m_target == EntityHandle_Player) {
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_SPEED_LOOP, &entities[m_target]->pos, 1.f);
	}
	
	if(m_caster == EntityHandle_Player) {
		m_duration = 0;
		m_hasDuration = false;
	} else {
		m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(20000);
		m_hasDuration = true;
	}
	
	std::vector<VertexGroup> & grouplist = entities[m_target]->obj->grouplist;
	
	bool skip = true;
	std::vector<VertexGroup>::const_iterator itr;
	for(itr = grouplist.begin(); itr != grouplist.end(); ++itr) {
		skip = !skip;
		
		if(skip) {
			continue;
		}
		
		float col = Random::getf(0.05f, 0.1f);
		float size = Random::getf(1.f, 1.5f);
		int taille = Random::get(130, 260);
		
		SpeedTrail trail;
		trail.vertexIndex = itr->origin;
		trail.trail = new Trail(taille, Color4f::gray(col), Color4f::black, size, 0.f);
		
		m_trails.push_back(trail);
	}
	
	m_targets.push_back(m_target);
}

void SpeedSpell::End() {
	
	m_targets.clear();
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	Entity * target = entities.get(m_target);
	if(target) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_SPEED_END, &target->pos);
	}
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		delete m_trails[i].trail;
	}
	m_trails.clear();
}

void SpeedSpell::Update() {
	
	if(m_caster == EntityHandle_Player)
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		Vec3f pos = entities[m_target]->obj->vertexWorldPositions[m_trails[i].vertexIndex].v;
		
		m_trails[i].trail->SetNextPosition(pos);
		m_trails[i].trail->Update(g_gameTime.lastFrameDuration());
	}
	
	for(size_t i = 0; i < m_trails.size(); i++) {
		m_trails[i].trail->Render();
	}
}

Vec3f SpeedSpell::getPosition() {
	return getTargetPosition();
}


void DispellIllusionSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_DISPELL_ILLUSION);
	
	m_duration = GameDurationMs(1000);
	m_hasDuration = true;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell || spell->m_target == m_caster) {
			continue;
		}
		
		if(spell->m_level > m_level) {
			continue;
		}
		
		if(spell->m_type == SPELL_INVISIBILITY) {
			Entity * target = entities.get(spell->m_target);
			Entity * caster = entities.get(m_caster);
			if(target && caster) {
				if(closerThan(target->pos, caster->pos, 1000.f)) {
					spells.endSpell(spell);
				}
			}
		}
	}
}


void DispellIllusionSpell::Update() { }


FireballSpell::FireballSpell()
	: eCurPos(0.f)
	, eMove(0.f)
	, bExplo(false)
	, m_createBallDuration(GameDurationMs(2000))
{ }

FireballSpell::~FireballSpell() { }

void FireballSpell::Launch() {
	
	m_duration = GameDurationMs(6000);
	m_hasDuration = true;
	
	if(m_caster != EntityHandle_Player) {
		m_hand_group = ActionPoint();
	}
	
	Vec3f target = m_hand_pos;
	if(m_hand_group == ActionPoint()) {
		target = m_caster_pos;
		Entity * c = entities.get(m_caster);
		if(c) {
			if(c->ioflags & IO_NPC) {
				target += angleToVectorXZ(c->angle.getYaw()) * 30.f;
				target += Vec3f(0.f, -80.f, 0.f);
			}
		}
	}
	
	float anglea = 0, angleb;
	if(m_caster == EntityHandle_Player) {
		anglea = player.angle.getPitch(), angleb = player.angle.getYaw();
	} else {
		
		Vec3f start = entities[m_caster]->pos;
		if(ValidIONum(m_caster)
		   && (entities[m_caster]->ioflags & IO_NPC)) {
			start.y -= 80.f;
		}
		
		Entity * _io = entities[m_caster];
		if(ValidIONum(_io->targetinfo)) {
			const Vec3f & end = entities[_io->targetinfo]->pos;
			float d = glm::distance(Vec2f(end.x, end.z), Vec2f(start.x, start.z));
			anglea = glm::degrees(getAngle(start.y, start.z, end.y, end.z + d));
		}
		
		angleb = entities[m_caster]->angle.getYaw();
	}
	
	Vec3f eSrc = target;
	eSrc += angleToVectorXZ(angleb) * 60.f;
	eCurPos = eSrc;
	
	eMove = angleToVector(Anglef(anglea, angleb, 0.f)) * 80.f;
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_LAUNCH, &m_caster_pos);
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_FIRE_WIND_LOOP, &m_caster_pos, 1.f);
}

void FireballSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	endLightDelayed(m_light, GameDurationMs(500));
}

void FireballSpell::Update() {
	
	if(m_elapsed <= m_createBallDuration) {
		
		float afAlpha = 0.f;
		float afBeta = 0.f;
		
		if(m_caster == EntityHandle_Player) {
			afBeta = player.angle.getYaw();
			afAlpha = player.angle.getPitch();
			ObjVertHandle idx = GetGroupOriginByName(entities[m_caster]->obj, "chest");

			if(idx != ObjVertHandle()) {
				eCurPos = entities[m_caster]->obj->vertexWorldPositions[idx.handleData()].v;
			} else {
				eCurPos = player.pos;
			}
			
			eCurPos += angleToVectorXZ(afBeta) * 60.f;
		} else {
			afBeta = entities[m_caster]->angle.getYaw();
			
			eCurPos = entities[m_caster]->pos;
			eCurPos += angleToVectorXZ(afBeta) * 60.f;
			
			if(ValidIONum(m_caster) && (entities[m_caster]->ioflags & IO_NPC)) {
				
				eCurPos += angleToVectorXZ(entities[m_caster]->angle.getYaw()) * 30.f;
				eCurPos += Vec3f(0.f, -80.f, 0.f);
			}
			
			Entity * io = entities[m_caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &eCurPos;
				Vec3f p2 = entities[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (glm::degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z)))));
			}
		}
		
		eMove = angleToVector(Anglef(afAlpha, afBeta, 0.f)) * 100.f;
	}
	
	eCurPos += eMove * (g_framedelay * 0.0045f);
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->pos = eCurPos;
		light->intensity = 2.2f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(1.0f, 0.6f, 0.3f) - Color3f(0.3f, 0.1f, 0.1f) * randomColor3f();
	}
	
	Sphere sphere = Sphere(eCurPos, std::max(m_level * 2.f, 12.f));
	
	if(m_elapsed > m_createBallDuration) {
		SpawnFireballTail(eCurPos, eMove, m_level, 0);
	} else {
		if(Random::getf() < 0.9f) {
			float dd = glm::clamp(m_elapsed / m_createBallDuration * 10, 1.f, m_level);
			SpawnFireballTail(eCurPos, Vec3f(0.f), dd, 1);
		}
	}
	
	if(!bExplo)
	if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP)) {
		spawnFireHitParticle(eCurPos, 0);
		PolyBoomAddScorch(eCurPos);
		LaunchFireballBoom(eCurPos, m_level);
		
		eMove *= 0.5f;
		bExplo = true;
		
		DoSphericDamage(Sphere(eCurPos, 30.f * m_level), 3.f * m_level, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, m_caster);
		ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &sphere.origin);
		ARX_NPC_SpawnAudibleSound(sphere.origin, entities[m_caster]);
		requestEnd();
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, eCurPos);
}

Vec3f FireballSpell::getPosition() {
	return eCurPos;
}

CreateFoodSpell::CreateFoodSpell()
	: m_pos(0.f)
{ }

void CreateFoodSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_CREATE_FOOD, &m_caster_pos);
	
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(3500);
	m_hasDuration = true;
	m_elapsed = 0;
	
	if(m_caster == EntityHandle_Player || m_target == EntityHandle_Player) {
		player.hunger = 100;
	}
	
	m_pos = player.pos;
	
	m_particles.SetPos(m_pos);
	
	m_particles.SetParams(g_particleParameters[ParticleParam_CreateFood]);
}

void CreateFoodSpell::End() { }

void CreateFoodSpell::Update() {
	
	m_pos = entities.player()->pos;
	
	GameDuration timeRemaining = m_duration - m_elapsed;
	
	if(timeRemaining < GameDurationMs(1500)) {
		m_particles.m_parameters.m_spawnFlags = PARTICLE_CIRCULAR;
		m_particles.m_parameters.m_gravity = Vec3f(0.f);
		
		std::list<Particle *>::iterator i;
		
		for(i = m_particles.listParticle.begin(); i != m_particles.listParticle.end(); ++i) {
			Particle * pP = *i;
			
			if(pP->isAlive()) {
				pP->fColorEnd.a = 0;
					
				if(pP->m_age + timeRemaining < pP->m_timeToLive) {
					pP->m_age = pP->m_timeToLive - timeRemaining;
				}
			}
		}
	}

	m_particles.SetPos(m_pos);
	m_particles.Update(g_gameTime.lastFrameDuration());
	
	m_particles.Render();
}


IceProjectileSpell::IceProjectileSpell()
	: iNumber(0)
	, tex_p1(NULL)
	, tex_p2(NULL)
{ }

void IceProjectileSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_ICE_PROJECTILE_LAUNCH, &m_caster_pos);
	
	m_duration = GameDurationMs(4200);
	m_hasDuration = true;
	
	Vec3f target;
	float angleb;
	if(m_caster == EntityHandle_Player) {
		target = player.pos + Vec3f(0.f, 160.f, 0.f);
		angleb = player.angle.getYaw();
	} else {
		target = entities[m_caster]->pos;
		angleb = entities[m_caster]->angle.getYaw();
	}
	target += angleToVectorXZ(angleb) * 150.0f;
	
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	int iMax = int(30 + m_level * 5.2f);
	
	float fspelldist = glm::clamp(float(iMax * 15), 200.0f, 450.0f);
	
	Vec3f s = target + Vec3f(0.f, -100.f, 0.f);
	Vec3f e = s + angleToVectorXZ(angleb) * fspelldist;
	
	RaycastResult ray = RaycastLine(s, e);
	if(ray.hit) {
		e = ray.pos + angleToVectorXZ(angleb) * 20.f;
	}

	float fd = fdist(s, e);

	float fCalc = toMsf(m_duration) * (fd / fspelldist);
	m_duration = GameDurationMsf(fCalc);

	float fDist = (fd / fspelldist) * iMax;

	iNumber = checked_range_cast<int>(fDist);

	int end = iNumber / 2;
	
	Vec3f tv1a[MAX_ICE];
	tv1a[0] = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end] = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, Vec3f(80, 0, 80));

	for(int i = 0; i < iNumber; i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = Random::getf();
		
		Vec3f minSize;
		int randomRange;
		
		if(t < 0.5f) {
			icicle.type = 0;
			minSize = Vec3f(1.2f, 1.f, 1.2f);
			randomRange = 80;
		} else {
			icicle.type = 1;
			minSize = Vec3f(0.4f, 0.3f, 0.4f);
			randomRange = 40;
		}

		icicle.size = Vec3f(0.f);
		icicle.sizeMax = arx::randomVec() + Vec3f(0.f, 0.2f, 0.f);
		icicle.sizeMax = glm::max(icicle.sizeMax, minSize);
		
		icicle.pos = tv1a[i / 2] + arx::randomOffsetXZ(randomRange);
		
		DamageParameters damage;
		damage.pos = icicle.pos;
		damage.radius = 60.f;
		damage.damages = 0.1f * m_level;
		damage.area = DAMAGE_FULL;
		damage.duration = m_duration;
		damage.source = m_caster;
		damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
		damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
		DamageCreate(damage);
	}
}

void IceProjectileSpell::End() {
	
}

void IceProjectileSpell::Update() {
	
	float fColor = 1.f;
	if(m_duration - m_elapsed < GameDurationMs(1000)) {
		fColor = (m_duration - m_elapsed) / GameDurationMs(1000);
		
		for(int i = 0; i < iNumber; i++) {
			m_icicles[i].size.y *= fColor;
		}
	}
	
	RenderMaterial mat;
	mat.setCulling(CullCW);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	int iMax = int(float(iNumber) * 2.f * (m_elapsed / m_duration));

	if(iMax > iNumber) {
		iMax = iNumber;
	}

	for(int i = 0; i < iMax; i++) {
		Icicle & icicle = m_icicles[i];
		
		icicle.size += Vec3f(0.1f);
		icicle.size = glm::clamp(icicle.size, Vec3f(0.f), icicle.sizeMax);
		
		Anglef stiteangle;
		stiteangle.setYaw(glm::cos(glm::radians(icicle.pos.x)) * 360);
		stiteangle.setPitch(0);
		stiteangle.setRoll(0);
		
		float tt = icicle.sizeMax.y * fColor;
		Color3f stitecolor = Color3f(0.7f, 0.7f, 0.9f) * tt;
		stitecolor = componentwise_min(stitecolor, Color3f(1.f, 1.f, 1.f));
		
		EERIE_3DOBJ * eobj = (icicle.type == 0) ? smotte : stite;
		
		Draw3DObject(eobj, stiteangle, icicle.pos, icicle.size, stitecolor, mat);
	}
	
	for(int i = 0; i < std::min(iNumber, iMax + 1); i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = Random::getf();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = icicle.pos + arx::randomVec(-5.f, 5.f);
				pd->move = arx::randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float tolive = std::min(Random::getf(2000.f, 4000.f),
				                        toMsf(m_duration - m_elapsed) + Random::getf(0.f, 500.0f));
				pd->tolive = checked_range_cast<u32>(tolive);
				pd->tc = tex_p2;
				pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = icicle.pos + arx::randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, Random::getf(-2.f, 2.f), 0.f);
				pd->siz = 0.5f;
				float tolive = std::min(Random::getf(2000.f, 3000.f),
				                        toMsf(m_duration - m_elapsed) + Random::getf(0.f, 500.0f));
				pd->tolive = checked_range_cast<u32>(tolive);
				pd->tc = tex_p1;
				pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
		}
	}
}
