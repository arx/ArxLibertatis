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
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/spells/Spells03.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

void SpeedSpellLaunch(long i, long duration)
{
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 2.f;
	
	if(spells[i].caster == 0) {
		spells[i].target = spells[i].caster;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_START, &entities[spells[i].target]->pos);
	
	if(spells[i].target == 0) {
		spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_SPEED_LOOP,
		                                       &entities[spells[i].target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	spells[i].exist = true;
	spells[i].tolive = (spells[i].caster == 0) ? 200000000 : 20000;
	if(duration > -1) {
		spells[i].tolive = duration;
	}
	
	CSpeed * effect = new CSpeed();
	effect->spellinstance = i;
	effect->Create(spells[i].target, spells[i].tolive);
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void SpeedSpellEnd(long i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
	
	if(spells[i].caster == 0)
		ARX_SOUND_Stop(spells[i].snd_loop);
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_END, &entities[spells[i].target]->pos);
}

void DispellIllusionSpellLaunch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_ILLUSION);
	spells[i].exist = true;
	spells[i].tolive = 1000;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].exist || spells[n].target == spells[i].caster) {
			continue;
		}
		
		if(spells[n].caster_level > spells[i].caster_level) {
			continue;
		}
		
		if(spells[n].type == SPELL_INVISIBILITY) {
			if(ValidIONum(spells[n].target) && ValidIONum(spells[i].caster)) {
				if(closerThan(entities[spells[n].target]->pos,
				   entities[spells[i].caster]->pos, 1000.f)) {
					spells[n].tolive = 0;
				}
			}
		}
	}
}

void FireballSpellLaunch(long i)
{
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 20000; // TODO probably never read
	
	CFireBall * effect = new CFireBall();
	effect->spellinstance = i;
	
	if(spells[i].caster != 0) {
		spells[i].hand_group = -1;
	}
	
	Vec3f target;
	if(spells[i].hand_group >= 0) {
		target = spells[i].hand_pos;
	} else {
		target = spells[i].caster_pos;
		if(ValidIONum(spells[i].caster)) {
			Entity * c = entities[spells[i].caster];
			if(c->ioflags & IO_NPC) {
				target.x -= std::sin(radians(c->angle.getPitch())) * 30.f;
				target.y -= 80.f;
				target.z += std::cos(radians(c->angle.getPitch())) * 30.f;
			}
		}
	}
	
	effect->SetDuration(6000ul);
	
	float anglea = 0, angleb;
	if(spells[i].caster == 0) {
		anglea = player.angle.getYaw(), angleb = player.angle.getPitch();
	} else {
		
		Vec3f start = entities[spells[i].caster]->pos;
		if(ValidIONum(spells[i].caster)
		   && (entities[spells[i].caster]->ioflags & IO_NPC)) {
			start.y -= 80.f;
		}
		
		Entity * _io = entities[spells[i].caster];
		if(ValidIONum(_io->targetinfo)) {
			const Vec3f & end = entities[_io->targetinfo]->pos;
			float d = glm::distance(Vec2f(end.x, end.z), Vec2f(start.x, start.z));
			anglea = degrees(getAngle(start.y, start.z, end.y, end.z + d));
		}
		
		angleb = entities[spells[i].caster]->angle.getPitch();
	}
	
	effect->Create(target, MAKEANGLE(angleb), anglea, spells[i].caster_level);
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &spells[i].caster_pos);
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND,
	                                       &spells[i].caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireballSpellEnd(long i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
}

void FireballSpellKill(long i)
{
	if(lightHandleIsValid(spells[i].longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo_light);
		
		light->duration = 500;
		light->time_creation = (unsigned long)(arxtime);
	}
	spells[i].longinfo_light = -1;
}

void CreateFoodSpellLaunch(long duration, long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &spells[i].caster_pos);
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 3500;
	
	if(spells[i].caster == 0 || spells[i].target == 0) {
		player.hunger = 100;
	}
	
	CCreateFood * effect = new CCreateFood();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void IceProjectileSpellLaunch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_PROJECTILE_LAUNCH, &spells[i].caster_pos);
	spells[i].exist = true;
	spells[i].tolive = 4200;
	
	CIceProjectile * effect = new CIceProjectile();
	effect->spellinstance = i;
	
	Vec3f target;
	float angleb;
	if(spells[i].caster == 0) {
		target = player.pos + Vec3f(0.f, 160.f, 0.f);
		angleb = player.angle.getPitch();
	} else {
		target = entities[spells[i].caster]->pos;
		angleb = entities[spells[i].caster]->angle.getPitch();
	}
	angleb = MAKEANGLE(angleb);
	target.x -= std::sin(radians(angleb)) * 150.0f;
	target.z += std::cos(radians(angleb)) * 150.0f;
	effect->Create(target, angleb, spells[i].caster_level);
	
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}
