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

#include "game/magic/spells/SpellsLvl03.h"

#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells03.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Object.h"

void SpeedSpell::Launch()
{
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.f;
	
	if(m_caster == PlayerEntityHandle) {
		m_target = m_caster;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_START, &entities[m_target]->pos);
	
	if(m_target == PlayerEntityHandle) {
		m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_SPEED_LOOP,
		                                       &entities[m_target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	m_duration = (m_caster == PlayerEntityHandle) ? 200000000 : 20000;
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	}
	
	CSpeed * effect = new CSpeed();
	effect->Create(m_target);
	
	m_pSpellFx = effect;
	
	m_targets.push_back(m_target);
}

void SpeedSpell::End()
{
	m_targets.clear();
	
	if(m_caster == PlayerEntityHandle)
		ARX_SOUND_Stop(m_snd_loop);
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_END, &entities[m_target]->pos);
}

void SpeedSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		if(m_caster == PlayerEntityHandle)
			ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
		
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}

void DispellIllusionSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_ILLUSION);
	
	m_duration = 1000;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell || spell->m_target == m_caster) {
			continue;
		}
		
		if(spell->m_level > m_level) {
			continue;
		}
		
		if(spell->m_type == SPELL_INVISIBILITY) {
			if(ValidIONum(spell->m_target) && ValidIONum(m_caster)) {
				if(closerThan(entities[spell->m_target]->pos,
				   entities[m_caster]->pos, 1000.f)) {
					spells.endSpell(spell);
				}
			}
		}
	}
}

void DispellIllusionSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}

FireballSpell::FireballSpell()
	: m_light(LightHandle::Invalid)
{
}

void FireballSpell::Launch()
{
	m_duration = 20000; // TODO probably never read
	
	CFireBall * effect = new CFireBall();
	
	if(m_caster != PlayerEntityHandle) {
		m_hand_group = -1;
	}
	
	Vec3f target;
	if(m_hand_group >= 0) {
		target = m_hand_pos;
	} else {
		target = m_caster_pos;
		if(ValidIONum(m_caster)) {
			Entity * c = entities[m_caster];
			if(c->ioflags & IO_NPC) {
				target += angleToVectorXZ(c->angle.getPitch()) * 30.f;
				target += Vec3f(0.f, -80.f, 0.f);
			}
		}
	}
	
	effect->SetDuration(6000ul);
	
	float anglea = 0, angleb;
	if(m_caster == PlayerEntityHandle) {
		anglea = player.angle.getYaw(), angleb = player.angle.getPitch();
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
		
		angleb = entities[m_caster]->angle.getPitch();
	}
	
	effect->Create(target, MAKEANGLE(angleb), anglea);
	
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &m_caster_pos);
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND,
	                                       &m_caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireballSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->duration = 500;
		light->time_creation = (unsigned long)(arxtime);
	}
	m_light = LightHandle::Invalid;
}

void FireballSpell::Update(float timeDelta)
{
	CFireBall *effect = static_cast<CFireBall *>(m_pSpellFx);
	if(!effect)
		return;
	
	effect->Update(timeDelta);
	
	if(effect->ulCurrentTime <= effect->m_createBallDuration) {
		
		float afAlpha = 0.f;
		float afBeta = 0.f;
		
		if(m_caster == PlayerEntityHandle) {
			afBeta = player.angle.getPitch();
			afAlpha = player.angle.getYaw();
			long idx = GetGroupOriginByName(entities[m_caster]->obj, "chest");

			if(idx >= 0) {
				effect->eCurPos = entities[m_caster]->obj->vertexlist3[idx].v;
			} else {
				effect->eCurPos = player.pos;
			}
			
			effect->eCurPos += angleToVectorXZ(afBeta) * 60.f;
		} else {
			afBeta = entities[m_caster]->angle.getPitch();
			
			effect->eCurPos = entities[m_caster]->pos;
			effect->eCurPos += angleToVectorXZ(afBeta) * 60.f;
			
			if(ValidIONum(m_caster) && (entities[m_caster]->ioflags & IO_NPC)) {
				
				effect->eCurPos += angleToVectorXZ(entities[m_caster]->angle.getPitch()) * 30.f;
				effect->eCurPos += Vec3f(0.f, -80.f, 0.f);
			}
			
			Entity * io = entities[m_caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &effect->eCurPos;
				Vec3f p2 = entities[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (glm::degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}
		
		effect->eMove = angleToVector(Anglef(afAlpha, afBeta, 0.f)) * 100.f;
	}
	
	effect->eCurPos += effect->eMove * (timeDelta * 0.0045f);
	
	
	if(!lightHandleIsValid(m_light))
		m_light = GetFreeDynLight();
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->pos = effect->eCurPos;
		light->intensity = 2.2f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb.r = 1.0f-rnd()*0.3f;
		light->rgb.g = 0.6f-rnd()*0.1f;
		light->rgb.b = 0.3f-rnd()*0.1f;
	}
	
	Sphere sphere;
	sphere.origin = effect->eCurPos;
	sphere.radius=std::max(m_level*2.f,12.f);
	
		if(effect->ulCurrentTime > effect->m_createBallDuration) {
			SpawnFireballTail(&effect->eCurPos,&effect->eMove,(float)m_level,0);
		} else {
			if(rnd()<0.9f) {
				Vec3f move = Vec3f_ZERO;
				float dd=(float)effect->ulCurrentTime / (float)effect->m_createBallDuration*10;
				
				dd = glm::clamp(dd, 1.f, m_level);
				
				SpawnFireballTail(&effect->eCurPos,&move,(float)dd,1);
			}
		}
	
	if(!effect->bExplo)
	if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP)) {
		ARX_BOOMS_Add(effect->eCurPos);
		LaunchFireballBoom(&effect->eCurPos,(float)m_level);
		
		effect->eMove *= 0.5f;
		effect->SetTTL(1500);
		effect->bExplo = true;
		
		DoSphericDamage(effect->eCurPos, 3.f * m_level, 30.f * m_level, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, m_caster);
		m_duration=0;
		ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
		ARX_NPC_SpawnAudibleSound(sphere.origin, entities[m_caster]);
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, effect->eCurPos);
}

void CreateFoodSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &m_caster_pos);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 3500;
	
	if(m_caster == PlayerEntityHandle || m_target == PlayerEntityHandle) {
		player.hunger = 100;
	}
	
	CCreateFood * effect = new CCreateFood();
	effect->Create();
	effect->SetDuration(m_duration);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
}

void CreateFoodSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}	
}

void IceProjectileSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_PROJECTILE_LAUNCH, &m_caster_pos);
	
	m_duration = 4200;
	
	CIceProjectile * effect = new CIceProjectile();
	
	Vec3f target;
	float angleb;
	if(m_caster == PlayerEntityHandle) {
		target = player.pos + Vec3f(0.f, 160.f, 0.f);
		angleb = player.angle.getPitch();
	} else {
		target = entities[m_caster]->pos;
		angleb = entities[m_caster]->angle.getPitch();
	}
	target += angleToVectorXZ(angleb) * 150.0f;
	
	effect->Create(target, angleb, m_level, m_caster);
	
	effect->SetDuration(m_duration);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
}

void IceProjectileSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}
