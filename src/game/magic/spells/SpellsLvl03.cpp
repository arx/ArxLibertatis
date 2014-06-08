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

void SpeedSpell::Launch()
{
	m_bDuration = true;
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
	
	m_exist = true;
	m_tolive = (m_caster == PlayerEntityHandle) ? 200000000 : 20000;
	if(m_launchDuration > -1) {
		m_tolive = m_launchDuration;
	}
	
	CSpeed * effect = new CSpeed();
	effect->Create(m_target);
	
	m_pSpellFx = effect;
	
	ARX_SPELLS_AddSpellOn(m_target, m_thisHandle);
}

void SpeedSpell::End(SpellHandle i)
{
	ARX_SPELLS_RemoveSpellOn(m_target,i);
	
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
	m_exist = true;
	m_tolive = 1000;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		const SpellHandle handle = SpellHandle(n);
		SpellBase & spell = spells[handle];
		
		if(!spell.m_exist || spell.m_target == m_caster) {
			continue;
		}
		
		if(spell.m_caster_level > m_caster_level) {
			continue;
		}
		
		if(spell.m_type == SPELL_INVISIBILITY) {
			if(ValidIONum(spell.m_target) && ValidIONum(m_caster)) {
				if(closerThan(entities[spell.m_target]->pos,
				   entities[m_caster]->pos, 1000.f)) {
					spell.m_tolive = 0;
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

void FireballSpell::Launch()
{
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	m_tolive = 20000; // TODO probably never read
	
	CFireBall * effect = new CFireBall();
	effect->spellinstance = m_thisHandle;
	
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
				target.x -= std::sin(radians(c->angle.getPitch())) * 30.f;
				target.y -= 80.f;
				target.z += std::cos(radians(c->angle.getPitch())) * 30.f;
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
			anglea = degrees(getAngle(start.y, start.z, end.y, end.z + d));
		}
		
		angleb = entities[m_caster]->angle.getPitch();
	}
	
	effect->Create(target, MAKEANGLE(angleb), anglea, m_caster_level);
	
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &m_caster_pos);
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND,
	                                       &m_caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireballSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	
	if(lightHandleIsValid(m_longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo_light);
		
		light->duration = 500;
		light->time_creation = (unsigned long)(arxtime);
	}
	m_longinfo_light = -1;
}

void FireballSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		CFireBall *pCF = (CFireBall*) pCSpellFX;
			
		if(!lightHandleIsValid(m_longinfo_light))
			m_longinfo_light = GetFreeDynLight();

		if(lightHandleIsValid(m_longinfo_light)) {
			EERIE_LIGHT * light = lightHandleGet(m_longinfo_light);
			
			light->pos = pCF->eCurPos;
			light->intensity = 2.2f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb.r = 1.0f-rnd()*0.3f;
			light->rgb.g = 0.6f-rnd()*0.1f;
			light->rgb.b = 0.3f-rnd()*0.1f;
		}

		Sphere sphere;
		sphere.origin = pCF->eCurPos;
		sphere.radius=std::max(m_caster_level*2.f,12.f);
		#define MIN_TIME_FIREBALL 2000 

		if(pCF->pPSFire.m_parameters.m_nbMax) {
			if(pCF->ulCurrentTime > MIN_TIME_FIREBALL) {
				SpawnFireballTail(&pCF->eCurPos,&pCF->eMove,(float)m_caster_level,0);
			} else {
				if(rnd()<0.9f) {
					Vec3f move = Vec3f_ZERO;
					float dd=(float)pCF->ulCurrentTime / (float)MIN_TIME_FIREBALL*10;

					if(dd > m_caster_level)
						dd = m_caster_level;

					if(dd < 1)
						dd = 1;

					SpawnFireballTail(&pCF->eCurPos,&move,(float)dd,1);
				}
			}
		}

		if(!pCF->bExplo)
		if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP)) {
			ARX_BOOMS_Add(pCF->eCurPos);
			LaunchFireballBoom(&pCF->eCurPos,(float)m_caster_level);
			
			pCF->pPSFire.StopEmission();
			pCF->pPSFire2.StopEmission();
			pCF->pPSSmoke.StopEmission();
			
			pCF->eMove *= 0.5f;
			pCF->SetTTL(1500);
			pCF->bExplo = true;
			
			DoSphericDamage(pCF->eCurPos, 3.f * m_caster_level, 30.f * m_caster_level, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, m_caster);
			m_tolive=0;
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
			ARX_NPC_SpawnAudibleSound(sphere.origin, entities[m_caster]);
		}

		pCSpellFX->Update(timeDelta);
		ARX_SOUND_RefreshPosition(m_snd_loop, pCF->eCurPos);
	}
}

void CreateFoodSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &m_caster_pos);
	m_exist = true;
	m_tolive = (m_launchDuration > -1) ? m_launchDuration : 3500;
	
	if(m_caster == PlayerEntityHandle || m_target == PlayerEntityHandle) {
		player.hunger = 100;
	}
	
	CCreateFood * effect = new CCreateFood();
	effect->Create();
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
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
	m_exist = true;
	m_tolive = 4200;
	
	CIceProjectile * effect = new CIceProjectile();
	effect->spellinstance = m_thisHandle;
	
	Vec3f target;
	float angleb;
	if(m_caster == PlayerEntityHandle) {
		target = player.pos + Vec3f(0.f, 160.f, 0.f);
		angleb = player.angle.getPitch();
	} else {
		target = entities[m_caster]->pos;
		angleb = entities[m_caster]->angle.getPitch();
	}
	angleb = MAKEANGLE(angleb);
	target.x -= std::sin(radians(angleb)) * 150.0f;
	target.z += std::cos(radians(angleb)) * 150.0f;
	effect->Create(target, angleb, m_caster_level);
	
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void IceProjectileSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}
