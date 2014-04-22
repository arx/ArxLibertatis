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

#include "game/magic/spells/SpellsLvl07.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/FlyingEye.h"

#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells07.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern float SLID_START;
extern bool bOldLookToggle;

bool FlyingEyeSpellLaunch(long i, TextureContainer * tc4)
{
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}
	
	if(spells[i].target != 0) {
		return false;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_IN);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 1000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 3.2f;
	eyeball.exist = 1;
	float angleb = MAKEANGLE(player.angle.getPitch());
	eyeball.pos.x = player.pos.x - std::sin(radians(angleb)) * 200.f;
	eyeball.pos.y = player.pos.y + 50.f;
	eyeball.pos.z = player.pos.z + std::cos(radians(angleb)) * 200.f;
	eyeball.angle = player.angle;
	
	for(long n = 0; n < 12; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			continue;
		}
		
		pd->ov = eyeball.pos + randomVec(-5.f, 5.f);
		pd->move = randomVec(-2.f, 2.f);
		pd->siz = 28.f;
		pd->tolive = Random::get(2000, 6000);
		pd->scale = Vec3f(12.f);
		pd->tc = tc4;
		pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
		              | DISSIPATING;
		pd->fparam = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.7f, 1.f);
	}
	
	TRUE_PLAYER_MOUSELOOK_ON = true;
	SLID_START = float(arxtime);
	bOldLookToggle = config.input.mouseLookToggle;
	config.input.mouseLookToggle = true;
	
	return true;
}

void FlyingEyeSpellKill(TextureContainer* tc4, long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_OUT);
	eyeball.exist = -100;
	
	for(long n = 0; n < 12; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = eyeball.pos + randomVec(-5.f, 5.f);
		pd->move = randomVec(-2.f, 2.f);
		pd->siz = 28.f;
		pd->tolive = Random::get(2000, 6000);
		pd->scale = Vec3f(12.f);
		pd->timcreation = spells[i].lastupdate;
		pd->tc = tc4;
		pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
		pd->fparam = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.7f, 1.f);
	}
	
	config.input.mouseLookToggle = bOldLookToggle;
}

void FireFieldSpellLaunch(long i, SpellType typ, long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_START);
	
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 100000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 2.8f;
	spells[i].longinfo2_light = -1;
	
	CFireField * effect = new CFireField();
	effect->spellinstance = i;
	
	Vec3f target;
	float beta;
	float displace = false;
	if(spells[i].caster == 0) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		if(ValidIONum(spells[i].caster)) {
			Entity * io = entities[spells[i].caster];
			target = io->pos;
			beta = io->angle.getPitch();
			displace = (io->ioflags & IO_NPC);
		} else {
			ARX_DEAD_CODE();
		}
	}
	if(displace) {
		target.x -= std::sin(radians(MAKEANGLE(beta))) * 250.f;
		target.z += std::cos(radians(MAKEANGLE(beta))) * 250.f;
	}
	
	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[spells[i].longinfo_damage];
		
		damage->radius = 150.f;
		damage->damages = 10.f;
		damage->area = DAMAGE_FULL;
		damage->duration = 100000000;
		damage->source = spells[i].caster;
		damage->flags = 0;
		damage->type = DAMAGE_TYPE_MAGICAL
		             | DAMAGE_TYPE_FIRE
		             | DAMAGE_TYPE_FIELD;
		damage->exist = true;
		damage->pos = target;
	}
	
	effect->Create(200.f, &target, spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_LOOP,
	                                       &target, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireFieldSpellKill(long i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_END);
}

void IceFieldSpellLaunch(long i, long duration, SpellType typ)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
		
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = (duration > -1) ? duration : 100000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 2.8f;
	spells[i].longinfo2_light = -1;
	
	CIceField * effect = new CIceField();
	effect->spellinstance = i;
	
	Vec3f target;
	float beta;
	float displace = false;
	if(spells[i].caster == 0) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		if(ValidIONum(spells[i].caster)) {
			Entity * io = entities[spells[i].caster];
			target = io->pos;
			beta = io->angle.getPitch();
			displace = (io->ioflags & IO_NPC);
		} else {
			ARX_DEAD_CODE();
		}
	}
	if(displace) {
		target.x -= std::sin(radians(MAKEANGLE(beta))) * 250.f;
		target.z += std::cos(radians(MAKEANGLE(beta))) * 250.f;
	}
	
	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[spells[i].longinfo_damage];
		
		damage->radius = 150.f;
		damage->damages = 10.f;
		damage->area = DAMAGE_FULL;
		damage->duration = 100000000;
		damage->source = spells[i].caster;
		damage->flags = 0;
		damage->type = DAMAGE_TYPE_MAGICAL
		             | DAMAGE_TYPE_COLD
		             | DAMAGE_TYPE_FIELD;
		damage->exist = true;
		damage->pos = target;
	}
	
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX( SND_SPELL_ICE_FIELD_LOOP, 
	                                       &target, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED );
}

void IceFieldSpellKill(long i)
{
	ARX_SOUND_Stop(spells[i].snd_loop); 
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD_END);
}

void LightningStrikeSpellLaunch(long i)
{
	spells[i].exist = true;
	
	CLightning * effect = new CLightning();
	effect->spellinstance = i;
	Vec3f target(0.f, 0.f, -500.f);
	effect->Create(Vec3f_ZERO, target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(long(500 * spells[i].caster_level));
	effect->lSrc = 0;
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &spells[i].caster_pos);
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP,
	                                       &spells[i].caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void LightningStrikeKill(long i)
{
	if(lightHandleIsValid(spells[i].longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo_light);
		
		light->duration = 200;
		light->time_creation = (unsigned long)(arxtime);
	}
	spells[i].longinfo_light = -1;
	
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_END, &entities[spells[i].caster]->pos);
}

void ConfuseSpellLaunch(long i, bool & notifyAll, long duration)
{
	ARX_SOUND_PlaySFX(SND_SPELL_CONFUSE, &entities[spells[i].target]->pos);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 1.5f;
	if(duration > -1) {
		spells[i].tolive = duration;
	} else {
		// TODO what then?
	}
	
	CConfuse * effect = new CConfuse();
	effect->spellinstance = i;
	effect->Create(player.pos, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
	
	notifyAll = false;
}
