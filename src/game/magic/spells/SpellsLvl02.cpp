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


bool HealSpell::CanLaunch() {
	
	if(ARX_SPELLS_ExistAnyInstanceForThisCaster(m_type, m_caster))
		return false;
	
	return true;
}

void HealSpell::Launch(long duration)
{
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &m_caster_pos);
	}
	
	m_exist = true;
	m_bDuration = true;
	m_fManaCostPerSecond = 0.4f * m_caster_level;
	m_tolive = (duration > -1) ? duration : 3500;
	
	CHeal * effect = new CHeal();
	effect->spellinstance = m_thisHandle;
	effect->Create();
	effect->SetDuration(m_tolive);
	
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void HealSpell::Update(float framedelay)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(framedelay);
		pCSpellFX->Render();
	}

	CHeal * ch=(CHeal *)pCSpellFX;

	if (ch)
	for(size_t ii = 0; ii < entities.size(); ii++) {
		const EntityHandle handle = EntityHandle(ii);
		Entity * e = entities[handle];
		
		if ((e)
			&& (e->show==SHOW_FLAG_IN_SCENE) 
			&& (e->gameFlags & GFLAG_ISINTREATZONE)
			&& (e->ioflags & IO_NPC)
			&& (e->_npcdata->lifePool.current>0.f)
			)
		{
			float dist;

			if(long(ii) == m_caster)
				dist=0;
			else
				dist=fdist(ch->eSrc, e->pos);

			if(dist<300.f) {
				float gain=((rnd()*1.6f+0.8f)*m_caster_level)*(300.f-dist)*( 1.0f / 300 )*framedelay*( 1.0f / 1000 );

				if(ii==0) {
					if (!BLOCK_PLAYER_CONTROLS)
						player.lifePool.current=std::min(player.lifePool.current+gain,player.Full_maxlife);									
				}
				else
					e->_npcdata->lifePool.current = std::min(e->_npcdata->lifePool.current+gain, e->_npcdata->lifePool.max);
			}
		}
	}	
}

void DetectTrapSpell::Launch()
{
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_DETECT_TRAP, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(m_caster == PlayerEntityHandle) {
		m_target = m_caster;
		if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
			ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);
		}
	}
	
	m_snd_loop = SND_SPELL_DETECT_TRAP_LOOP;
	if(m_caster == PlayerEntityHandle && !(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(m_snd_loop, &m_caster_pos, 1.f,
		                  ARX_SOUND_PLAY_LOOPED);
	}
	
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	m_tolive = 60000;
	m_fManaCostPerSecond = 0.4f;
	m_bDuration = true;
	
	ARX_SPELLS_AddSpellOn(m_target, m_thisHandle);
}

void DetectTrapSpell::End(SpellHandle i)
{
	if(m_caster == PlayerEntityHandle) {
		ARX_SOUND_Stop(m_snd_loop);
	}
	ARX_SPELLS_RemoveSpellOn(m_target, i);
}

void DetectTrapSpell::Update(float timeDelta)
{
	if(m_caster == PlayerEntityHandle) {
		Vec3f pos;
		ARX_PLAYER_FrontPos(&pos);
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
	}

	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void ArmorSpell::Launch(long duration)
{
	SpellHandle idx = ARX_SPELLS_GetSpellOn(entities[m_target], SPELL_ARMOR);
	if(idx != InvalidSpellHandle) {
		spells[idx].m_tolive = 0;
	}
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(m_caster == PlayerEntityHandle) {
		m_target = m_caster;
	}
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &entities[m_target]->pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP,
	                                       &entities[m_target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	m_exist = true;
	if(duration > -1) {
		m_tolive = duration;
	} else {
		m_tolive = (m_caster == PlayerEntityHandle) ? 20000000 : 20000;
	}
	
	m_bDuration = true;
	m_fManaCostPerSecond = 0.2f * m_caster_level;
		
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.5f;
		io->halo.color.b = 0.25f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
	
	ARX_SPELLS_AddSpellOn(m_target, m_thisHandle);
}

void ArmorSpell::End(SpellHandle i)
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &entities[m_target]->pos);
	
	if(ValidIONum(m_target)) {
		ARX_HALO_SetToNative(entities[m_target]);
	}
	
	ARX_SPELLS_RemoveSpellOn(m_target, i);
}

void ArmorSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.5f;
		io->halo.color.b = 0.25f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void LowerArmorSpell::Launch(long duration)
{
	SpellHandle idx = ARX_SPELLS_GetSpellOn(entities[m_target], SPELL_LOWER_ARMOR);
	if(idx != InvalidSpellHandle) {
		spells[idx].m_tolive = 0;
	}
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR, &entities[m_target]->pos);
	}
	
	m_exist = true;
	if(duration > -1) {
		m_tolive = duration;
	} else {
		m_tolive = (m_caster == PlayerEntityHandle) ? 20000000 : 20000;
	}
	
	m_bDuration = true;
	m_fManaCostPerSecond = 0.2f * m_caster_level;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		
		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			
			m_longinfo_lower_armor = 1;
		} else {
			m_longinfo_lower_armor = 0;
		}
	}
	
	ARX_SPELLS_AddSpellOn(m_target, m_thisHandle);
}

void LowerArmorSpell::End(SpellHandle i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR_END);
	Entity *io = entities[m_target];
	
	if(m_longinfo_lower_armor) {
		io->halo.flags &= ~HALO_ACTIVE;
		ARX_HALO_SetToNative(io);
	}
	
	ARX_SPELLS_RemoveSpellOn(m_target, i);
}

void LowerArmorSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		
		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			
			m_longinfo_lower_armor = 1;
		}
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void HarmSpell::Launch(long duration)
{
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HARM, &m_caster_pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &m_caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	m_exist = true;
	m_tolive = (duration >-1) ? duration : 6000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 0.4f;

	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 4.f;
	damage.area = DAMAGE_FULL;
	damage.duration = 100000000;
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
	m_longinfo_damage = DamageCreate(damage);
	
	m_longinfo2_light = GetFreeDynLight();
	if(lightHandleIsValid(m_longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo2_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = m_caster_pos;
	}
}

void HarmSpell::End()
{
	DamageRequestEnd(m_longinfo_damage);
	
	if(lightHandleIsValid(m_longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo2_light);
		
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 600; 
	}
	
	ARX_SOUND_Stop(m_snd_loop);
}

extern EERIE_3DOBJ * cabal;

void HarmSpell::Update(float timeDelta)
{
	float refpos;
	float scaley;
	
	if(m_caster == PlayerEntityHandle)
		scaley=90.f;
	else
		scaley = EEfabs(entities[m_caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;
	
	
	float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;
	
	Vec3f cabalpos;
	if(m_caster == PlayerEntityHandle) {
		cabalpos.x = player.pos.x;
		cabalpos.y = player.pos.y + 60.f - mov;
		cabalpos.z = player.pos.z;
		refpos=player.pos.y+60.f;
	} else {
		cabalpos.x = entities[m_caster]->pos.x;
		cabalpos.y = entities[m_caster]->pos.y - scaley - mov;
		cabalpos.z = entities[m_caster]->pos.z;
		refpos=entities[m_caster]->pos.y-scaley;
	}
	
	float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));
	
	if(lightHandleIsValid(m_longinfo2_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo2_light);
		
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
	cabalangle.setPitch(m_fdata+(float)timeDelta*0.1f);
	m_fdata = cabalangle.getPitch();
	
	Vec3f cabalscale = Vec3f(Es);
	Color3f cabalcolor = Color3f(0.8f, 0.4f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);
	
	mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.5f, 3.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);
	
	mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y=refpos-mov;
	cabalcolor = Color3f(0.25f, 0.1f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);
	
	mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y=refpos-mov;
	cabalcolor = Color3f(0.15f, 0.1f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalpos);
}
