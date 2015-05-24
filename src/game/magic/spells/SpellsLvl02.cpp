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
#include "graphics/RenderBatcher.h"
#include "graphics/Renderer.h"
#include "io/log/Logger.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"


bool HealSpell::CanLaunch() {
	
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void HealSpell::Launch()
{
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &m_caster_pos);
	}
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.4f * m_level;
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 3500;
	
	CHeal * effect = new CHeal();
	
	if(m_caster == PlayerEntityHandle) {
		effect->setPos(player.pos);
	} else {
		effect->setPos(entities[m_caster]->pos);
	}
	
	effect->Create();
	effect->SetDuration(m_duration);
	
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
}

void HealSpell::Update(float timeDelta)
{
	CHeal * effect = static_cast<CHeal *>(m_pSpellFx);
	if(!effect)
		return;

	Vec3f pos;
	if(m_caster == PlayerEntityHandle) {
		pos = player.pos;
	} else if(ValidIONum(m_target)) {
		pos = entities[m_target]->pos;
	}
	effect->setPos(pos);
	
	effect->Update(timeDelta);
	effect->Render();
	
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
				dist=fdist(pos, e->pos);

			if(dist<300.f) {
				float gain=((rnd()*1.6f+0.8f)*m_level)*(300.f-dist)*( 1.0f / 300 )*timeDelta*( 1.0f / 1000 );

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
	spells.endByCaster(m_caster, SPELL_DETECT_TRAP);
	
	m_snd_loop = SND_SPELL_DETECT_TRAP_LOOP;
	
	if(m_caster == PlayerEntityHandle) {
		m_target = m_caster;
		if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
			ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);
			ARX_SOUND_PlaySFX(m_snd_loop, &m_caster_pos, 1.f, ARX_SOUND_PLAY_LOOPED);
		}
	}
	
	m_duration = 60000;
	m_fManaCostPerSecond = 0.4f;
	m_hasDuration = true;
	
	m_targets.push_back(m_target);
}

void DetectTrapSpell::End()
{
	if(m_caster == PlayerEntityHandle) {
		ARX_SOUND_Stop(m_snd_loop);
	}
	m_targets.clear();
}

void DetectTrapSpell::Update(float timeDelta)
{
	if(m_caster == PlayerEntityHandle) {
		Vec3f pos = ARX_PLAYER_FrontPos();
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
	}

	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void ArmorSpell::Launch()
{
	spells.endByTarget(m_target, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_FIRE_PROTECTION);
	spells.endByCaster(m_caster, SPELL_COLD_PROTECTION);
	
	if(m_caster == PlayerEntityHandle) {
		m_target = m_caster;
	}
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &entities[m_target]->pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP,
	                                       &entities[m_target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	} else {
		m_duration = (m_caster == PlayerEntityHandle) ? 20000000 : 20000;
	}
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.2f * m_level;
		
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.5f;
		io->halo.color.b = 0.25f;
		io->halo.radius = 45.f;
	}
	
	m_targets.push_back(m_target);
}

void ArmorSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &entities[m_target]->pos);
	
	if(ValidIONum(m_target)) {
		ARX_HALO_SetToNative(entities[m_target]);
	}
	
	m_targets.clear();
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
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f ArmorSpell::getPosition() {
	return getTargetPosition();
}

LowerArmorSpell::LowerArmorSpell()
	: m_longinfo_lower_armor(-1) //TODO is this correct ?
{
	
}

void LowerArmorSpell::Launch()
{
	spells.endByTarget(m_target, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_FIRE_PROTECTION);
	spells.endByCaster(m_caster, SPELL_COLD_PROTECTION);
	
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR, &entities[m_target]->pos);
	}
	
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	} else {
		m_duration = (m_caster == PlayerEntityHandle) ? 20000000 : 20000;
	}
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.2f * m_level;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		
		if(io && !(io->halo.flags & HALO_ACTIVE)) {
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			
			m_longinfo_lower_armor = 1;
		} else {
			m_longinfo_lower_armor = 0;
		}
	}
	
	m_targets.push_back(m_target);
}

void LowerArmorSpell::End()
{
	ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR_END);
	Entity *io = entities[m_target];
	
	if(m_longinfo_lower_armor) {
		io->halo.flags &= ~HALO_ACTIVE;
		ARX_HALO_SetToNative(io);
	}
	
	m_targets.clear();
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
			
			m_longinfo_lower_armor = 1;
		}
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f LowerArmorSpell::getPosition() {
	return getTargetPosition();
}

HarmSpell::HarmSpell()
	: m_light(LightHandle::Invalid)
	, m_damage(DamageHandle::Invalid)
	, m_pitch(0.f)
{
	
}

void HarmSpell::Launch()
{
	if(!(m_flags & SPELLCAST_FLAG_NOSOUND)) {
		ARX_SOUND_PlaySFX(SND_SPELL_HARM, &m_caster_pos);
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
	                                       &m_caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells.endByCaster(m_caster, SPELL_LIFE_DRAIN);
	spells.endByCaster(m_caster, SPELL_MANA_DRAIN);
	
	m_duration = (m_launchDuration >-1) ? m_launchDuration : 6000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.4f;

	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 4.f;
	damage.area = DAMAGE_FULL;
	damage.duration = 100000000;
	damage.source = m_caster;
	damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
	damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
	m_damage = DamageCreate(damage);
	
	m_light = GetFreeDynLight();
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->intensity = 2.3f;
		light->fallend = 700.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red;
		light->pos = m_caster_pos;
	}
}

void HarmSpell::End()
{
	DamageRequestEnd(m_damage);
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 600; 
	}
	
	ARX_SOUND_Stop(m_snd_loop);
}

extern EERIE_3DOBJ * cabal;

// TODO copy-paste cabal
void HarmSpell::Update(float timeDelta)
{
	float refpos;
	float scaley;
	
	if(m_caster == PlayerEntityHandle)
		scaley = 90.f;
	else
		scaley = glm::abs(entities[m_caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;
	
	
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
	
	float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + glm::radians(scaley));
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->pos.x = cabalpos.x;
		light->pos.y = refpos;
		light->pos.z = cabalpos.z;
		light->rgb.r = rnd() * 0.2f + 0.8f;
		light->rgb.g = rnd() * 0.2f + 0.6f;
		light->fallstart = Es * 1.5f;
	}
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullNone);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Anglef cabalangle(0.f, 0.f, 0.f);
	cabalangle.setPitch(m_pitch + (float)timeDelta*0.1f);
	m_pitch = cabalangle.getPitch();
	
	Vec3f cabalscale = Vec3f(Es);
	Color3f cabalcolor = Color3f(0.8f, 0.4f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y = refpos - mov;
	cabalcolor = Color3f(0.5f, 3.f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y=refpos-mov;
	cabalcolor = Color3f(0.25f, 0.1f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
	cabalpos.y=refpos-mov;
	cabalcolor = Color3f(0.15f, 0.1f, 0.f);
	Draw3DObject(cabal, cabalangle, cabalpos, cabalscale, cabalcolor, mat);
	
	ARX_SOUND_RefreshPosition(m_snd_loop, cabalpos);
}
