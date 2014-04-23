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

#include "game/magic/spells/SpellsLvl02.h"

#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/spells/Spells02.h"
#include "graphics/Renderer.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"


void HealSpellLaunch(long i, long duration)
{
	if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &spells[i].caster_pos);
	}
	
	spells[i].exist = true;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 0.4f * spells[i].caster_level;
	spells[i].tolive = (duration > -1) ? duration : 3500;
	
	CHeal * effect = new CHeal();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(spells[i].tolive);
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void HealSpellUpdate(size_t i, float framedelay)
{
	CSpellFx *pCSpellFX = spells[i].pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(framedelay);
		pCSpellFX->Render();
	}

	CHeal * ch=(CHeal *)pCSpellFX;

	if (ch)
	for(size_t ii = 0; ii < entities.size(); ii++) {
		if ((entities[ii])
			&& (entities[ii]->show==SHOW_FLAG_IN_SCENE) 
			&& (entities[ii]->gameFlags & GFLAG_ISINTREATZONE)
			&& (entities[ii]->ioflags & IO_NPC)
			&& (entities[ii]->_npcdata->life>0.f)
			)
		{
			float dist;

			if(long(ii) == spells[i].caster)
				dist=0;
			else
				dist=fdist(ch->eSrc, entities[ii]->pos);

			if(dist<300.f) {
				float gain=((rnd()*1.6f+0.8f)*spells[i].caster_level)*(300.f-dist)*( 1.0f / 300 )*framedelay*( 1.0f / 1000 );

				if(ii==0) {
					if (!BLOCK_PLAYER_CONTROLS)
						player.life=std::min(player.life+gain,player.Full_maxlife);									
				}
				else
					entities[ii]->_npcdata->life = std::min(entities[ii]->_npcdata->life+gain, entities[ii]->_npcdata->maxlife);
			}
		}
	}	
}

void DetectTrapSpellLaunch(long i, SpellType typ)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(spells[i].caster == 0) {
		spells[i].target = spells[i].caster;
		if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
			ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);
		}
	}
	
	spells[i].snd_loop = SND_SPELL_DETECT_TRAP_LOOP;
	if(spells[i].caster == 0 && !(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(spells[i].snd_loop, &spells[i].caster_pos, 1.f,
		                  ARX_SOUND_PLAY_LOOPED);
	}
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 60000;
	spells[i].fManaCostPerSecond = 0.4f;
	spells[i].bDuration = true;
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void DetectTrapSpellEnd(size_t i)
{
	if(spells[i].caster == 0) {
		ARX_SOUND_Stop(spells[i].snd_loop);
	}
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void DetectTrapSpellUpdate(size_t i, float timeDelta)
{
	if(spells[i].caster == 0) {
		Vec3f pos;
		ARX_PLAYER_FrontPos(&pos);
		ARX_SOUND_RefreshPosition(spells[i].snd_loop, pos);
	}

	CSpellFx *pCSpellFX = spells[i].pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void ArmorSpellLaunch(SpellType typ, long duration, long i)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
	if(idx >= 0) {
		spells[idx].tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,
	                                                   spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(spells[i].caster == 0) {
		spells[i].target = spells[i].caster;
	}
	
	if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &entities[spells[i].target]->pos);
	}
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP,
	                                       &entities[spells[i].target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells[i].exist = true;
	if(duration > -1) {
		spells[i].tolive = duration;
	} else {
		spells[i].tolive = (spells[i].caster == 0) ? 20000000 : 20000;
	}
	
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 0.2f * spells[i].caster_level;
		
	CArmor * effect = new CArmor();
	effect->spellinstance = i;
	effect->Create(spells[i].tolive);
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void ArmorSpellEnd(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &entities[spells[i].target]->pos);
	
	if(ValidIONum(spells[i].target)) {
		ARX_HALO_SetToNative(entities[spells[i].target]);
	}
	
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void ArmorSpellUpdate(size_t i, float timeDelta)
{
	CSpellFx *pCSpellFX = spells[i].pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	
	ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
}

void LowerArmorSpellLaunch(SpellType typ, long duration, long i)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
	if(idx >= 0) {
		spells[idx].tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,
	                                                   spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,
	                                              spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR, &entities[spells[i].target]->pos);
	}
	
	spells[i].exist = true;
	if(duration > -1) {
		spells[i].tolive = duration;
	} else {
		spells[i].tolive = (spells[i].caster == 0) ? 20000000 : 20000;
	}
	
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 0.2f * spells[i].caster_level;
	
	CLowerArmor * effect = new CLowerArmor();
	effect->spellinstance = i;
	effect->Create(spells[i].tolive);
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void LowerArmorSpellEnd(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR_END);
	Entity *io = entities[spells[i].target];
	
	if(spells[i].longinfo_lower_armor) {
		io->halo.flags &= ~HALO_ACTIVE;
		ARX_HALO_SetToNative(io);
	}
	
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void LowerArmorSpellUpdate(size_t i, float timeDelta)
{
	CSpellFx *pCSpellFX = spells[i].pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	
	ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
}

void HarmSpellLaunch(long duration, long i)
{
	if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HARM, &spells[i].caster_pos);
	}
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &spells[i].caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,
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
	spells[i].tolive = (duration >-1) ? duration : 6000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 0.4f;

	spells[i].longinfo_damage = ARX_DAMAGES_GetFree();
	if(spells[i].longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[spells[i].longinfo_damage];
		
		damage->radius = 150.f;
		damage->damages = 4.f;
		damage->area = DAMAGE_FULL;
		damage->duration = 100000000;
		damage->source = spells[i].caster;
		damage->flags = DAMAGE_FLAG_DONT_HURT_SOURCE
		              | DAMAGE_FLAG_FOLLOW_SOURCE
		              | DAMAGE_FLAG_ADD_VISUAL_FX;
		damage->type = DAMAGE_TYPE_FAKEFIRE
		             | DAMAGE_TYPE_MAGICAL;
		damage->exist = true;
	}
	
	spells[i].longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = spells[i].caster_pos;
	}
}

void HarmSpellKill(long i)
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

extern EERIE_3DOBJ * cabal;

void HarmSpellUpdate(size_t i, float timeDelta)
{
	if(cabal) {
		float refpos;
		float scaley;

		if(spells[i].caster==0)
			scaley=90.f;
		else
			scaley = EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;


		float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

		Vec3f cabalpos;
		if(spells[i].caster==0) {
			cabalpos.x = player.pos.x;
			cabalpos.y = player.pos.y + 60.f - mov;
			cabalpos.z = player.pos.z;
			refpos=player.pos.y+60.f;
		} else {
			cabalpos.x = entities[spells[i].caster]->pos.x;
			cabalpos.y = entities[spells[i].caster]->pos.y - scaley - mov;
			cabalpos.z = entities[spells[i].caster]->pos.z;
			refpos=entities[spells[i].caster]->pos.y-scaley;							
		}

		float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

		if(lightHandleIsValid(spells[i].longinfo2_light)) {
			EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
			
			light->pos.x = cabalpos.x;
			light->pos.y = refpos;
			light->pos.z = cabalpos.z; 
			light->rgb.r=rnd()*0.2f+0.8f;
			light->rgb.g=rnd()*0.2f+0.6f;
			light->fallstart=Es*1.5f;
		}

		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		Anglef cabalangle(0.f, 0.f, 0.f);
		cabalangle.setPitch(spells[i].fdata+(float)timeDelta*0.1f);
		spells[i].fdata = cabalangle.getPitch();

		Vec3f cabalscale = Vec3f(Es);
		Color3f cabalcolor = Color3f(0.8f, 0.4f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y = refpos - mov;
		cabalcolor = Color3f(0.5f, 3.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.25f, 0.1f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.15f, 0.1f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);	
		
		ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
	}
}
