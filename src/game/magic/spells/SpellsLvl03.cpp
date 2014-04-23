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

void SpeedSpell::Launch(long i, long duration)
{
	bDuration = true;
	fManaCostPerSecond = 2.f;
	
	if(caster == 0) {
		target = caster;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_START, &entities[target]->pos);
	
	if(target == 0) {
		snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_SPEED_LOOP,
		                                       &entities[target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	exist = true;
	tolive = (caster == 0) ? 200000000 : 20000;
	if(duration > -1) {
		tolive = duration;
	}
	
	CSpeed * effect = new CSpeed();
	effect->spellinstance = i;
	effect->Create(target, tolive);
	
	pSpellFx = effect;
	tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(target, i);
}

void SpeedSpell::End(long i)
{
	ARX_SPELLS_RemoveSpellOn(target,i);
	
	if(caster == 0)
		ARX_SOUND_Stop(snd_loop);
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_END, &entities[target]->pos);
}

void SpeedSpell::Update(float timeDelta)
{
	if(pSpellFx) {
		if(caster == 0)
			ARX_SOUND_RefreshPosition(snd_loop, entities[target]->pos);
		
		pSpellFx->Update(timeDelta);
		pSpellFx->Render();
	}
}

void DispellIllusionSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_ILLUSION);
	exist = true;
	tolive = 1000;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].exist || spells[n].target == caster) {
			continue;
		}
		
		if(spells[n].caster_level > caster_level) {
			continue;
		}
		
		if(spells[n].type == SPELL_INVISIBILITY) {
			if(ValidIONum(spells[n].target) && ValidIONum(caster)) {
				if(closerThan(entities[spells[n].target]->pos,
				   entities[caster]->pos, 1000.f)) {
					spells[n].tolive = 0;
				}
			}
		}
	}
}

void DispellIllusionSpell::Update(float timeDelta)
{
	if(pSpellFx) {
		pSpellFx->Update(timeDelta);
		pSpellFx->Render();
	}
}

void FireballSpell::Launch(long i)
{
	exist = true;
	lastupdate = timcreation = (unsigned long)(arxtime);
	tolive = 20000; // TODO probably never read
	
	CFireBall * effect = new CFireBall();
	effect->spellinstance = i;
	
	if(caster != 0) {
		hand_group = -1;
	}
	
	Vec3f target;
	if(hand_group >= 0) {
		target = hand_pos;
	} else {
		target = caster_pos;
		if(ValidIONum(caster)) {
			Entity * c = entities[caster];
			if(c->ioflags & IO_NPC) {
				target.x -= std::sin(radians(c->angle.getPitch())) * 30.f;
				target.y -= 80.f;
				target.z += std::cos(radians(c->angle.getPitch())) * 30.f;
			}
		}
	}
	
	effect->SetDuration(6000ul);
	
	float anglea = 0, angleb;
	if(caster == 0) {
		anglea = player.angle.getYaw(), angleb = player.angle.getPitch();
	} else {
		
		Vec3f start = entities[caster]->pos;
		if(ValidIONum(caster)
		   && (entities[caster]->ioflags & IO_NPC)) {
			start.y -= 80.f;
		}
		
		Entity * _io = entities[caster];
		if(ValidIONum(_io->targetinfo)) {
			const Vec3f & end = entities[_io->targetinfo]->pos;
			float d = glm::distance(Vec2f(end.x, end.z), Vec2f(start.x, start.z));
			anglea = degrees(getAngle(start.y, start.z, end.y, end.z + d));
		}
		
		angleb = entities[caster]->angle.getPitch();
	}
	
	effect->Create(target, MAKEANGLE(angleb), anglea, caster_level);
	
	pSpellFx = effect;
	tolive = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &caster_pos);
	snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND,
	                                       &caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireballSpell::End()
{
	ARX_SOUND_Stop(snd_loop);
}

void FireballSpell::Kill()
{
	if(lightHandleIsValid(longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(longinfo_light);
		
		light->duration = 500;
		light->time_creation = (unsigned long)(arxtime);
	}
	longinfo_light = -1;
}

void FireballSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = pSpellFx;

	if(pCSpellFX) {
		CFireBall *pCF = (CFireBall*) pCSpellFX;
			
		if(!lightHandleIsValid(longinfo_light))
			longinfo_light = GetFreeDynLight();

		if(lightHandleIsValid(longinfo_light)) {
			EERIE_LIGHT * light = lightHandleGet(longinfo_light);
			
			light->pos = pCF->eCurPos;
			light->intensity = 2.2f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb.r = 1.0f-rnd()*0.3f;
			light->rgb.g = 0.6f-rnd()*0.1f;
			light->rgb.b = 0.3f-rnd()*0.1f;
		}

		EERIE_SPHERE sphere;
		sphere.origin = pCF->eCurPos;
		sphere.radius=std::max(caster_level*2.f,12.f);
		#define MIN_TIME_FIREBALL 2000 

		if(pCF->pPSFire.iParticleNbMax) {
			if(pCF->ulCurrentTime > MIN_TIME_FIREBALL) {
				SpawnFireballTail(&pCF->eCurPos,&pCF->eMove,(float)caster_level,0);
			} else {
				if(rnd()<0.9f) {
					Vec3f move = Vec3f_ZERO;
					float dd=(float)pCF->ulCurrentTime / (float)MIN_TIME_FIREBALL*10;

					if(dd > caster_level)
						dd = caster_level;

					if(dd < 1)
						dd = 1;

					SpawnFireballTail(&pCF->eCurPos,&move,(float)dd,1);
				}
			}
		}

		if(!pCF->bExplo)
		if(CheckAnythingInSphere(&sphere, caster, CAS_NO_SAME_GROUP)) {
			ARX_BOOMS_Add(pCF->eCurPos);
			LaunchFireballBoom(&pCF->eCurPos,(float)caster_level);
			pCF->pPSFire.iParticleNbMax = 0;
			pCF->pPSFire2.iParticleNbMax = 0;
			pCF->eMove *= 0.5f;
			pCF->pPSSmoke.iParticleNbMax = 0;
			pCF->SetTTL(1500);
			pCF->bExplo = true;
			
			DoSphericDamage(&pCF->eCurPos,3.f*caster_level,30.f*caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,caster);
			tolive=0;
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
			ARX_NPC_SpawnAudibleSound(sphere.origin, entities[caster]);
		}

		pCSpellFX->Update(timeDelta);
		ARX_SOUND_RefreshPosition(snd_loop, pCF->eCurPos);
	}
}

void CreateFoodSpell::Launch(long duration, long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &caster_pos);
	exist = true;
	tolive = (duration > -1) ? duration : 3500;
	
	if(caster == 0 || target == 0) {
		player.hunger = 100;
	}
	
	CCreateFood * effect = new CCreateFood();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(tolive);
	pSpellFx = effect;
	tolive = effect->GetDuration();
}

void CreateFoodSpell::Update(float timeDelta)
{
	if(pSpellFx) {
		pSpellFx->Update(timeDelta);
		pSpellFx->Render();
	}	
}

void IceProjectileSpell::Launch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_PROJECTILE_LAUNCH, &caster_pos);
	exist = true;
	tolive = 4200;
	
	CIceProjectile * effect = new CIceProjectile();
	effect->spellinstance = i;
	
	Vec3f target;
	float angleb;
	if(caster == 0) {
		target = player.pos + Vec3f(0.f, 160.f, 0.f);
		angleb = player.angle.getPitch();
	} else {
		target = entities[caster]->pos;
		angleb = entities[caster]->angle.getPitch();
	}
	angleb = MAKEANGLE(angleb);
	target.x -= std::sin(radians(angleb)) * 150.0f;
	target.z += std::cos(radians(angleb)) * 150.0f;
	effect->Create(target, angleb, caster_level);
	
	effect->SetDuration(tolive);
	pSpellFx = effect;
	tolive = effect->GetDuration();
}

void IceProjectileSpell::Update(float timeDelta)
{
	if(pSpellFx) {
		pSpellFx->Update(timeDelta);
		pSpellFx->Render();
	}
}
