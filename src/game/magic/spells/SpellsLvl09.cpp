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

#include "game/magic/spells/SpellsLvl09.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"
#include "game/spell/Cheat.h"

#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells09.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool SummonCreatureSpellLaunch(long i, long duration)
{
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 1.9f;
	spells[i].longinfo_summon_creature = 0;
	spells[i].longinfo2_entity = 0;
	spells[i].tolive = (duration > -1) ? duration : 2000000;
	
	Vec3f target;
	float beta;
	bool displace = false;
	if(spells[i].caster == 0) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		target = entities[spells[i].caster]->pos;
		beta = entities[spells[i].caster]->angle.getPitch();
		displace = (entities[spells[i].caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target.x -= std::sin(radians(MAKEANGLE(beta))) * 300.f;
		target.z += std::cos(radians(MAKEANGLE(beta))) * 300.f;
	}
	
	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		spells[i].exist = false;
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	spells[i].fdata = (spells[i].caster == 0 && cur_mega == 10) ? 1.f : 0.f;
	spells[i].target_pos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &spells[i].target_pos);
	CSummonCreature * effect = new CSummonCreature();
	effect->spellinstance = i;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	spells[i].pSpellFx = effect;
	
	return true;
}

void SummonCreatureSpellEnd(size_t i)
{
	if(ValidIONum(spells[i].longinfo2_entity) && spells[i].longinfo2_entity != 0) {
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[spells[i].longinfo2_entity]->pos);
	}

	lightHandleDestroy(spells[i].pSpellFx->lLightId);
	// need to killio
}

void SummonCreatureSpellKill(long i)
{
	lightHandleDestroy(spells[i].pSpellFx->lLightId);
	
	if(ValidIONum(spells[i].longinfo2_entity) && spells[i].longinfo2_entity != 0) {
		
		if(entities[spells[i].longinfo2_entity]->scriptload
		   && (entities[spells[i].longinfo2_entity]->ioflags & IO_NOSAVE)) {
			
			AddRandomSmoke(entities[spells[i].longinfo2_entity], 100);
			Vec3f posi = entities[spells[i].longinfo2_entity]->pos;
			posi.y -= 100.f;
			MakeCoolFx(posi);
		
			LightHandle nn = GetFreeDynLight();
			if(lightHandleIsValid(nn)) {
				EERIE_LIGHT * light = lightHandleGet(nn);
				
				light->intensity = 0.7f + 2.f * rnd();
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.0f);
				light->pos = posi;
				light->duration = 600;
			}
			
			entities[spells[i].longinfo2_entity]->destroyOne();
		}
	}
	
	spells[i].longinfo2_entity = 0;
}

bool FakeSummonSpellLaunch(long i)
{
	if(spells[i].caster <= 0 || !ValidIONum(spells[i].target)) {
		return false;
	}
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 1.9f;
	spells[i].tolive = 4000;
	
	Vec3f target = entities[spells[i].target]->pos;
	if(spells[i].target != 0) {
		target.y += player.baseHeight();
	}
	spells[i].target_pos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &spells[i].target_pos);
	CSummonCreature * effect = new CSummonCreature();
	effect->spellinstance = i;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	spells[i].pSpellFx = effect;
	
	return true;
}

void FakeSummonSpellEnd(size_t i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].target_pos);
	
	lightHandleDestroy(spells[i].pSpellFx->lLightId);
}

void FakeSummonSpellKill(long i)
{
	lightHandleDestroy(spells[i].pSpellFx->lLightId);
}

void LaunchAntiMagicField(size_t ident) {
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].exist || n == ident)
			continue;
		
		if(spells[ident].caster_level < spells[n].caster_level)
			continue;
		
		Vec3f pos;
		GetSpellPosition(&pos,n);
		if(closerThan(pos, entities[spells[ident].target]->pos, 600.f)) {
			if(spells[n].type != SPELL_CREATE_FIELD) {
				spells[n].tolive = 0;
			} else if(spells[ident].target == 0 && spells[n].caster == 0) {
				spells[n].tolive = 0;
			}
		}
	}
}

void NegateMagicSpellLaunch(long duration, long i)
{
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC, &entities[spells[i].target]->pos);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 2.f;
	spells[i].tolive = (duration > -1) ? duration : 1000000;
	
	CNegateMagic * effect = new CNegateMagic();
	effect->spellinstance = i;
	effect->Create(spells[i].target_pos, MAKEANGLE(entities[spells[i].target]->angle.getPitch()));
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	if(ValidIONum(spells[i].target)) {
		LaunchAntiMagicField(i);
	}
}

bool IncinerateSpellLaunch(long i)
{
	Entity * tio = entities[spells[i].target];
	if((tio->ioflags & IO_NPC) && tio->_npcdata->life <= 0.f) {
		return false;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE, &entities[spells[i].target]->pos);
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_LOOP, 
	                                       &entities[spells[i].target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 20000;
	
	tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
	tio->sfx_time = (unsigned long)(arxtime);
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
	
	return true;
}

void IncinerateSpellEnd(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
}

void MassParalyseSpellLaunch(long i, long duration)
{
	ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
	
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 10000;
	spells[i].longinfo2_entity = 0;
	
	for(size_t ii = 0; ii < entities.size(); ii++) {
		
		Entity * tio = entities[ii];
		if(long(ii) == spells[i].caster || !tio || !(tio->ioflags & IO_NPC)) {
			continue;
		}
		
		if(tio->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(tio->ioflags & IO_FREEZESCRIPT) {
			continue;
		}
		
		if(fartherThan(tio->pos, entities[spells[i].caster]->pos, 500.f)) {
			continue;
		}
		
		tio->ioflags |= IO_FREEZESCRIPT;
		
		ARX_NPC_Kill_Spell_Launch(tio);
		ARX_SPELLS_AddSpellOn(ii, i);
		
		spells[i].longinfo2_entity ++;
		spells[i].misc = realloc(spells[i].misc,
		                         sizeof(long) * spells[i].longinfo2_entity);
		long * ptr = (long *)spells[i].misc;
		ptr[spells[i].longinfo2_entity - 1] = ii;
	}
}

void MassParalyseSpellEnd(size_t i)
{
	long *ptr = (long *) spells[i].misc;

	for(long in = 0; in < spells[i].longinfo2_entity; in++) {
		if(ValidIONum(ptr[in])) {
			ARX_SPELLS_RemoveSpellOn(ptr[in], i);
			entities[ptr[in]]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}

	if(ptr)
		free(spells[i].misc);

	spells[i].misc=NULL;
}

void MassParalyseSpellKill()
{
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

