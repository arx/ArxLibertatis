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

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"

#include "graphics/particle/ParticleEffects.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

void InvisibilitySpellLaunch(long i, long duration)
{
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 6000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 3.f;
	
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}

	entities[spells[i].target]->gameFlags |= GFLAG_INVISIBILITY;
	entities[spells[i].target]->invisibility = 0.f;
	
	ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_START, &spells[i].caster_pos);
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void InvisibilitySpellEnd(long i)
{
	if(ValidIONum(spells[i].target)) {
		entities[spells[i].target]->gameFlags &= ~GFLAG_INVISIBILITY;
		ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &entities[spells[i].target]->pos);
		ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	}
}

void ManaDrainSpellLaunch(long i, long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,
	                                                   spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 6000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 2.f;
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &spells[i].caster_pos, 1.2f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[spells[i].longinfo_damage];
		
		damage->radius = 150.f;
		damage->damages = 8.f;
		damage->area = DAMAGE_FULL;
		damage->duration = 100000000;
		damage->source = spells[i].caster;
		damage->flags = DAMAGE_FLAG_DONT_HURT_SOURCE
		              | DAMAGE_FLAG_FOLLOW_SOURCE
		              | DAMAGE_FLAG_ADD_VISUAL_FX;
		damage->type = DAMAGE_TYPE_FAKEFIRE
		             | DAMAGE_TYPE_MAGICAL
		             | DAMAGE_TYPE_DRAIN_MANA;
		damage->exist = true;
	}
	
	spells[i].longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::blue;
		light->pos = spells[i].caster_pos;
		light->duration=900;
	}
}

void ManaDrainSpellKill(long i)
{
	if(spells[i].longinfo_damage != -1) {
		damages[spells[i].longinfo_damage].exist = false;
	}
	
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 600; 
	}
	
	ARX_SOUND_Stop(spells[i].snd_loop);
}

void ExplosionSpellLaunch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_EXPLOSION);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 2000;
	
	Vec3f target = entities[spells[i].caster]->pos;
	if(spells[i].caster == 0) {
		target.y += 60.f;
	} else {
		target.y -= 60.f;
	}
	
	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[spells[i].longinfo_damage];
		
		damage->radius = 350.f;
		damage->damages = 10.f;
		damage->area = DAMAGE_AREA; 
		damage->duration = spells[i].tolive;
		damage->source = spells[i].caster;
		damage->flags = DAMAGE_FLAG_DONT_HURT_SOURCE
		              | DAMAGE_FLAG_FOLLOW_SOURCE
		              | DAMAGE_FLAG_ADD_VISUAL_FX;
		damage->type = DAMAGE_TYPE_FAKEFIRE
		             | DAMAGE_TYPE_MAGICAL;
		damage->exist = true;
		damage->pos = target;
	}
	
	spells[i].longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb.r = 0.1f + rnd() * (1.f / 3);
		light->rgb.g = 0.1f + rnd() * (1.f / 3);
		light->rgb.b = 0.8f + rnd() * (1.f / 5);
		light->pos = target;
		light->duration = 200;
	}
	
	AddQuakeFX(300, 2000, 400, 1);
	
	for(long i_angle = 0 ; i_angle < 360 ; i_angle += 12) {
		for(long j = -100 ; j < 100 ; j += 50) {
			float rr = radians(float(i_angle));
			Vec3f pos(target.x - std::sin(rr) * 360.f, target.y,
			          target.z + std::cos(rr) * 360.f);
			Vec3f dir = glm::normalize(Vec3f(pos.x - target.x, 
                                        0.f,
                                        pos.z - target.z)) * 60.f;
			Color3f rgb(0.1f + rnd() * (1.f/3), 0.1f + rnd() * (1.f/3),
			            0.8f + rnd() * (1.f/5));
			Vec3f posi = target + Vec3f(0.f, j * 2, 0.f);
			LaunchFireballBoom(&posi, 16, &dir, &rgb);
		}
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND);
}

void EnchantWeaponSpellLaunch(bool & notifyAll, long i)
{
	spells[i].exist = true;
	spells[i].tolive = 20;
	
	notifyAll = false;
}

void LifeDrainSpellLaunch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,
	                                                   spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 6000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 12.f;
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &spells[i].caster_pos, 0.8f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		long id = spells[i].longinfo_damage;
		damages[id].radius = 150.f;
		damages[id].damages = spells[i].caster_level * 0.08f;
		damages[id].area = DAMAGE_AREA;
		damages[id].duration = 100000000;
		damages[id].source = spells[i].caster;
		damages[id].flags = DAMAGE_FLAG_DONT_HURT_SOURCE
		                    | DAMAGE_FLAG_FOLLOW_SOURCE
		                    | DAMAGE_FLAG_ADD_VISUAL_FX;
		damages[id].type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL
		                   | DAMAGE_TYPE_DRAIN_LIFE;
		damages[id].exist = true;
	}
	
	spells[i].longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = spells[i].caster_pos;
		light->duration = 900;
	}
}

void LifeDrainSpellKill(long i)
{
	if(spells[i].longinfo_damage != -1) {
		damages[spells[i].longinfo_damage].exist = false;
	}
	
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 600; 
	}
	
	ARX_SOUND_Stop(spells[i].snd_loop);
}
