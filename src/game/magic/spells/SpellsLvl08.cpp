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
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

void InvisibilitySpell::Launch(long i, long duration)
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

void InvisibilitySpell::End(long i)
{
	if(ValidIONum(spells[i].target)) {
		entities[spells[i].target]->gameFlags &= ~GFLAG_INVISIBILITY;
		ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &entities[spells[i].target]->pos);
		ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	}
}

extern void ARX_SPELLS_Fizzle(long num);

void InvisibilitySpell::Update(size_t i)
{
	if(spells[i].target != 0) {
		if(!(entities[spells[i].target]->gameFlags & GFLAG_INVISIBILITY)) {
			ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
			ARX_SPELLS_Fizzle(i);
		}
	}	
}

void ManaDrainSpell::Launch(long i, long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM, spells[i].caster);
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

void ManaDrainSpell::Kill(long i)
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

void ManaDrainSpell::Update(size_t i, float timeDelta)
{
	if(cabal) {
		float refpos;
		float scaley;

		if(spells[i].caster==0)
			scaley=90.f;
		else
			scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

		float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

		Vec3f cabalpos;
		if(spells[i].caster == 0) {
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
			light->rgb.b=rnd()*0.2f+0.8f;
			light->fallstart=Es*1.5f;
		}

		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		Anglef cabalangle(0.f, 0.f, 0.f);
		cabalangle.setPitch(spells[i].fdata + (float)timeDelta*0.1f);
		spells[i].fdata = cabalangle.getPitch();
								
		Vec3f cabalscale = Vec3f(Es);
		Color3f cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y = refpos - mov;
		cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.f, 0.f, 0.15f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		cabalangle.setPitch(-cabalangle.getPitch());
		cabalpos.y=refpos-mov;
		cabalscale = Vec3f(Es);
		cabalcolor = Color3f(0.f, 0.f, 0.15f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		cabalangle.setPitch(-cabalangle.getPitch());
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);	

		ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
	}	
}

void ExplosionSpell::Launch(long i)
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

void ExplosionSpell::Update(size_t i)
{
	if(!lightHandleIsValid(spells[i].longinfo2_light))
		spells[i].longinfo2_light = GetFreeDynLight();

	if(lightHandleIsValid(spells[i].longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
		
		light->rgb.r = 0.1f+rnd()*( 1.0f / 3 );;
		light->rgb.g = 0.1f+rnd()*( 1.0f / 3 );;
		light->rgb.b = 0.8f+rnd()*( 1.0f / 5 );;
		light->duration=200;
	
		float rr,r2;
		Vec3f pos;
		
		float choice = rnd();
		if(choice > .8f) {
			long lvl = Random::get(9, 13);
			rr=radians(rnd()*360.f);
			r2=radians(rnd()*360.f);
			pos.x=light->pos.x-std::sin(rr)*260;
			pos.y=light->pos.y-std::sin(r2)*260;
			pos.z=light->pos.z+std::cos(rr)*260;
			Color3f rgb(0.1f + rnd()*(1.f/3), 0.1f + rnd()*(1.0f/3), 0.8f + rnd()*(1.0f/5));
			LaunchFireballBoom(&pos, static_cast<float>(lvl), NULL, &rgb);
		} else if(choice > .6f) {
			rr=radians(rnd()*360.f);
			r2=radians(rnd()*360.f);
			pos.x=light->pos.x-std::sin(rr)*260;
			pos.y=light->pos.y-std::sin(r2)*260;
			pos.z=light->pos.z+std::cos(rr)*260;
			MakeCoolFx(pos);
		} else if(choice > 0.4f) {
			rr=radians(rnd()*360.f);
			r2=radians(rnd()*360.f);
			pos.x=light->pos.x-std::sin(rr)*160;
			pos.y=light->pos.y-std::sin(r2)*160;
			pos.z=light->pos.z+std::cos(rr)*160;
			ARX_PARTICLES_Add_Smoke(&pos, 2, 20); // flag 1 = randomize pos
		}
	}	
}

void EnchantWeaponSpell::Launch(bool & notifyAll, long i)
{
	spells[i].exist = true;
	spells[i].tolive = 20;
	
	notifyAll = false;
}

void EnchantWeaponSpell::Update(size_t i, float timeDelta)
{
	CSpellFx *pCSpellFX = spells[i].pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void LifeDrainSpell::Launch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN, spells[i].caster);
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

void LifeDrainSpell::Kill(long i)
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

void LifeDrainSpell::Update(size_t i, float timeDelta)
{
	if(cabal) {
		float refpos;
		float scaley;

		if(spells[i].caster==0)
			scaley=90.f;
		else
			scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

		float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

		Vec3f cabalpos;
		if(spells[i].caster == 0) {
			cabalpos.x = player.pos.x;
			cabalpos.y = player.pos.y + 60.f - mov;
			cabalpos.z = player.pos.z;
			refpos=player.pos.y+60.f;							
		} else {
			cabalpos.x = entities[spells[i].caster]->pos.x;
			cabalpos.y = entities[spells[i].caster]->pos.y - scaley-mov;
			cabalpos.z = entities[spells[i].caster]->pos.z;
			refpos=entities[spells[i].caster]->pos.y-scaley;							
		}

		float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

		if(lightHandleIsValid(spells[i].longinfo2_light)) {
			EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
			
			light->pos.x = cabalpos.x;
			light->pos.y = refpos;
			light->pos.z = cabalpos.z;
			light->rgb.r = rnd() * 0.2f + 0.8f;
			light->fallstart = Es * 1.5f;
		}

		GRenderer->SetCulling(Renderer::CullNone);
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetRenderState(Renderer::DepthWrite, false);

		Anglef cabalangle(0.f, 0.f, 0.f);
		cabalangle.setPitch(spells[i].fdata+(float)timeDelta*0.1f);
		spells[i].fdata=cabalangle.getPitch();

		Vec3f cabalscale = Vec3f(Es);
		Color3f cabalcolor = Color3f(0.8f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.5f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.25f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos-mov;
		cabalcolor = Color3f(0.15f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		cabalangle.setPitch(-cabalangle.getPitch());
		cabalpos.y=refpos-mov;
		cabalscale = Vec3f(Es);
		cabalcolor = Color3f(0.15f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.25f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.5f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		mov=std::sin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
		cabalpos.y=refpos+mov;
		cabalcolor = Color3f(0.8f, 0.f, 0.f);
		DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

		cabalangle.setPitch(-cabalangle.getPitch());
		GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
		GRenderer->SetRenderState(Renderer::DepthWrite, true);	

		ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
	}	
}
