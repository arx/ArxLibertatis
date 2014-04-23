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


void HealSpell::Launch(long i, long duration)
{
	if(!(flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &caster_pos);
	}
	
	exist = true;
	bDuration = true;
	fManaCostPerSecond = 0.4f * caster_level;
	tolive = (duration > -1) ? duration : 3500;
	
	CHeal * effect = new CHeal();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(tolive);
	
	pSpellFx = effect;
	tolive = effect->GetDuration();
}

void HealSpell::Update(float framedelay)
{
	CSpellFx *pCSpellFX = pSpellFx;

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

			if(long(ii) == caster)
				dist=0;
			else
				dist=fdist(ch->eSrc, entities[ii]->pos);

			if(dist<300.f) {
				float gain=((rnd()*1.6f+0.8f)*caster_level)*(300.f-dist)*( 1.0f / 300 )*framedelay*( 1.0f / 1000 );

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

void DetectTrapSpell::Launch(long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_DETECT_TRAP, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(caster == 0) {
		target = caster;
		if(!(flags & SPELLCAST_FLAG_NOSOUND)) {
			ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);
		}
	}
	
	snd_loop = SND_SPELL_DETECT_TRAP_LOOP;
	if(caster == 0 && !(flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(snd_loop, &caster_pos, 1.f,
		                  ARX_SOUND_PLAY_LOOPED);
	}
	
	exist = true;
	lastupdate = timcreation = (unsigned long)(arxtime);
	tolive = 60000;
	fManaCostPerSecond = 0.4f;
	bDuration = true;
	
	ARX_SPELLS_AddSpellOn(target, i);
}

void DetectTrapSpell::End(size_t i)
{
	if(caster == 0) {
		ARX_SOUND_Stop(snd_loop);
	}
	ARX_SPELLS_RemoveSpellOn(target, i);
}

void DetectTrapSpell::Update(float timeDelta)
{
	if(caster == 0) {
		Vec3f pos;
		ARX_PLAYER_FrontPos(&pos);
		ARX_SOUND_RefreshPosition(snd_loop, pos);
	}

	CSpellFx *pCSpellFX = pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void ArmorSpell::Launch(long duration, long i)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[target], SPELL_ARMOR);
	if(idx >= 0) {
		spells[idx].tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(caster == 0) {
		target = caster;
	}
	
	if(!(flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &entities[target]->pos);
	}
	
	snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP,
	                                       &entities[target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	exist = true;
	if(duration > -1) {
		tolive = duration;
	} else {
		tolive = (caster == 0) ? 20000000 : 20000;
	}
	
	bDuration = true;
	fManaCostPerSecond = 0.2f * caster_level;
		
	CArmor * effect = new CArmor();
	effect->spellinstance = i;
	effect->Create(tolive);
	
	pSpellFx = effect;
	tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(target, i);
}

void ArmorSpell::End(size_t i)
{
	ARX_SOUND_Stop(snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &entities[target]->pos);
	
	if(ValidIONum(target)) {
		ARX_HALO_SetToNative(entities[target]);
	}
	
	ARX_SPELLS_RemoveSpellOn(target, i);
}

void ArmorSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	
	ARX_SOUND_RefreshPosition(snd_loop, entities[target]->pos);
}

void LowerArmorSpell::Launch(long duration, long i)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[target], SPELL_LOWER_ARMOR);
	if(idx >= 0) {
		spells[idx].tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(!(flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR, &entities[target]->pos);
	}
	
	exist = true;
	if(duration > -1) {
		tolive = duration;
	} else {
		tolive = (caster == 0) ? 20000000 : 20000;
	}
	
	bDuration = true;
	fManaCostPerSecond = 0.2f * caster_level;
	
	CLowerArmor * effect = new CLowerArmor();
	effect->spellinstance = i;
	effect->Create(tolive);
	
	pSpellFx = effect;
	tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(target, i);
}

void LowerArmorSpell::End(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR_END);
	Entity *io = entities[target];
	
	if(longinfo_lower_armor) {
		io->halo.flags &= ~HALO_ACTIVE;
		ARX_HALO_SetToNative(io);
	}
	
	ARX_SPELLS_RemoveSpellOn(target, i);
}

void LowerArmorSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	
	ARX_SOUND_RefreshPosition(snd_loop, entities[target]->pos);
}

void HarmSpell::Launch(long duration)
{
	if(!(flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HARM, &caster_pos);
	}
	
	snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN, caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	exist = true;
	tolive = (duration >-1) ? duration : 6000000;
	bDuration = true;
	fManaCostPerSecond = 0.4f;

	longinfo_damage = ARX_DAMAGES_GetFree();
	if(longinfo_damage != -1) {
		DAMAGE_INFO * damage = &damages[longinfo_damage];
		
		damage->radius = 150.f;
		damage->damages = 4.f;
		damage->area = DAMAGE_FULL;
		damage->duration = 100000000;
		damage->source = caster;
		damage->flags = DAMAGE_FLAG_DONT_HURT_SOURCE
		              | DAMAGE_FLAG_FOLLOW_SOURCE
		              | DAMAGE_FLAG_ADD_VISUAL_FX;
		damage->type = DAMAGE_TYPE_FAKEFIRE
		             | DAMAGE_TYPE_MAGICAL;
		damage->exist = true;
	}
	
	longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = caster_pos;
	}
}

void HarmSpell::Kill()
{
	if(longinfo_damage != -1) {
		damages[longinfo_damage].exist = false;
	}
	
	if(lightHandleIsValid(longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(longinfo2_light);
		
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 600; 
	}
	
	ARX_SOUND_Stop(snd_loop);
}

extern EERIE_3DOBJ * cabal;

void HarmSpell::Update(float timeDelta)
{
	if(cabal) {
		float refpos;
		float scaley;

		if(caster==0)
			scaley=90.f;
		else
			scaley = EEfabs(entities[caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;


		float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

		Vec3f cabalpos;
		if(caster==0) {
			cabalpos.x = player.pos.x;
			cabalpos.y = player.pos.y + 60.f - mov;
			cabalpos.z = player.pos.z;
			refpos=player.pos.y+60.f;
		} else {
			cabalpos.x = entities[caster]->pos.x;
			cabalpos.y = entities[caster]->pos.y - scaley - mov;
			cabalpos.z = entities[caster]->pos.z;
			refpos=entities[caster]->pos.y-scaley;							
		}

		float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

		if(lightHandleIsValid(longinfo2_light)) {
			EERIE_LIGHT * light = lightHandleGet(longinfo2_light);
			
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
		cabalangle.setPitch(fdata+(float)timeDelta*0.1f);
		fdata = cabalangle.getPitch();

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
		
		ARX_SOUND_RefreshPosition(snd_loop, cabalpos);
	}
}
