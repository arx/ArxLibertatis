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
#include "core/Core.h"
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
#include "scene/Object.h"
#include "scene/Scene.h"

extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern float SLID_START;
bool bOldLookToggle;

static LightHandle special[3];

bool FlyingEyeSpell::CanLaunch()
{
	if(eyeball.exist)
		return false;

	if(spells.ExistAnyInstanceForThisCaster(m_type, m_caster))
		return false;
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	if(m_target != PlayerEntityHandle) {
		return false;
	}
	
	return true;
}

void FlyingEyeSpell::Launch()
{
	static TextureContainer * tc4 = TextureContainer::Load("graph/particles/smoke");
	
	ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_IN);
	
	m_lastupdate = m_timcreation;
	m_tolive = 1000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 3.2f;
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
}

void FlyingEyeSpell::End()
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &entities[m_caster]->pos);
	
	static TextureContainer * tc4=TextureContainer::Load("graph/particles/smoke");
	
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
		pd->timcreation = m_lastupdate;
		pd->tc = tc4;
		pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
		pd->fparam = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.7f, 1.f);
	}
	
	config.input.mouseLookToggle = bOldLookToggle;
	
	lightHandleDestroy(special[2]);
	lightHandleDestroy(special[1]);
}

void FlyingEyeSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	const unsigned long tim = (unsigned long)(arxtime);
	
	const long framediff3 = tim - m_lastupdate;
	
	eyeball.floating = std::sin(m_lastupdate-m_timcreation * 0.001f);
	eyeball.floating *= 10.f;
	
	if(m_lastupdate-m_timcreation <= 3000) {
		eyeball.exist = m_lastupdate - m_timcreation * (1.0f / 30);
		eyeball.size = Vec3f(1.f - float(eyeball.exist) * 0.01f);
		eyeball.angle.setPitch(eyeball.angle.getPitch() + framediff3 * 0.6f);
	} else {
		eyeball.exist = 2;
	}
	
	m_lastupdate=tim;
	
	Entity * io = entities.player();
	EERIE_3DOBJ * eobj = io->obj;
	long pouet = 2;

	while(pouet) {
		long id;

		if(pouet == 2)
			id = io->obj->fastaccess.primary_attach;
		else
			id = GetActionPointIdx(io->obj, "left_attach");

		pouet--;

		if(id != -1) {
			if(!lightHandleIsValid(special[pouet])) {
				special[pouet] = GetFreeDynLight();
			}
			if(lightHandleIsValid(special[pouet])) {
				EERIE_LIGHT * el = lightHandleGet(special[pouet]);
				el->intensity = 1.3f;
				el->fallend = 180.f;
				el->fallstart = 50.f;
				el->rgb = Color3f(0.7f, 0.3f, 1.f);
				el->pos = eobj->vertexlist3[id].v;
			}
			
			for(long kk = 0; kk < 2; kk++) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					break;
				}
				
				pd->ov = eobj->vertexlist3[id].v + randomVec(-1.f, 1.f);
				pd->move = Vec3f(0.1f - 0.2f * rnd(), -2.2f * rnd(), 0.1f - 0.2f * rnd());
				pd->siz = 5.f;
				pd->tolive = Random::get(1500, 3500);
				pd->scale = Vec3f(0.2f);
				pd->tc = TC_smoke;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->sourceionum = PlayerEntityHandle;
				pd->source = &eobj->vertexlist3[id].v;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(.7f - rnd() * .1f, .3f - rnd() * .1f, 1.f - rnd() * .1f);
			}
		}
	}
}

void FireFieldSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_FIRE_FIELD);
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_START);
	
	m_tolive = (m_launchDuration > -1) ? m_launchDuration : 100000;
	m_bDuration = true;
	m_fManaCostPerSecond = 2.8f;
	m_longinfo2_light = InvalidLightHandle;
	
	CFireField * effect = new CFireField();
	
	Vec3f target;
	float beta;
	float displace = false;
	if(m_caster == PlayerEntityHandle) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		if(ValidIONum(m_caster)) {
			Entity * io = entities[m_caster];
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
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 10.f;
	damage.area = DAMAGE_FULL;
	damage.duration = 100000000;
	damage.source = m_caster;
	damage.flags = 0;
	damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_FIELD;
	damage.pos = target;
	m_longinfo_damage = DamageCreate(damage);
	
	effect->Create(200.f, target + Vec3f(0, -10, 0), m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_LOOP,
	                                       &target, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireFieldSpell::End()
{
	DamageRequestEnd(m_longinfo_damage);
	
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_END);
}

void FireFieldSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	
	if(pCSpellFX) {
		CFireField *pf = (CFireField *) pCSpellFX;
		pCSpellFX->Update(timeDelta);
		
		if(!lightHandleIsValid(m_longinfo2_light))
			m_longinfo2_light = GetFreeDynLight();

		if(lightHandleIsValid(m_longinfo2_light)) {
			EERIE_LIGHT * el = lightHandleGet(m_longinfo2_light);
			
			el->pos.x = pf->pos.x;
			el->pos.y = pf->pos.y-120.f;
			el->pos.z = pf->pos.z;
			el->intensity = 4.6f;
			el->fallstart = 150.f+rnd()*30.f;
			el->fallend   = 290.f+rnd()*30.f;
			el->rgb.r = 1.f-rnd()*( 1.0f / 10 );
			el->rgb.g = 0.8f;
			el->rgb.b = 0.6f;
			el->duration = 600;
			el->extras=0;
		}
		
		if(VisibleSphere(pf->pos - Vec3f(0.f, 120.f, 0.f), 350.f)) {
			
			pCSpellFX->Render();
			float fDiff = timeDelta / 8.f;
			int nTime = checked_range_cast<int>(fDiff);
			
			for(long nn=0;nn<=nTime+1;nn++) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					break;
				}
				
				float t = rnd() * (PI * 2.f) - PI;
				float ts = std::sin(t);
				float tc = std::cos(t);
				pd->ov = pf->pos + Vec3f(120.f * ts, 15.f * ts, 120.f * tc) * randomVec();
				pd->move = Vec3f(2.f - 4.f * rnd(), 1.f - 8.f * rnd(), 2.f - 4.f * rnd());
				pd->siz = 7.f;
				pd->tolive = Random::get(500, 1500);
				pd->tc = fire2;
				pd->special = ROTATING | MODULATE_ROTATION | FIRE_TO_SMOKE;
				pd->fparam = 0.1f - rnd() * 0.2f;
				pd->scale = Vec3f(-8.f);
				
				PARTICLE_DEF * pd2 = createParticle();
				if(!pd2) {
					break;
				}
				
				*pd2 = *pd;
				pd2->delay = Random::get(60, 210);
			}
			
		}
	}
}

void IceFieldSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_ICE_FIELD);
	
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD);
	
	m_tolive = (m_launchDuration > -1) ? m_launchDuration : 100000;
	m_bDuration = true;
	m_fManaCostPerSecond = 2.8f;
	m_longinfo2_light = InvalidLightHandle;
	
	CIceField * effect = new CIceField();
	
	Vec3f target;
	float beta;
	float displace = false;
	if(m_caster == PlayerEntityHandle) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		if(ValidIONum(m_caster)) {
			Entity * io = entities[m_caster];
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
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 10.f;
	damage.area = DAMAGE_FULL;
	damage.duration = 100000000;
	damage.source = m_caster;
	damage.flags = 0;
	damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD | DAMAGE_TYPE_FIELD;
	damage.pos = target;
	m_longinfo_damage = DamageCreate(damage);
	
	effect->Create(target);
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	m_snd_loop = ARX_SOUND_PlaySFX( SND_SPELL_ICE_FIELD_LOOP, 
	                                       &target, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED );
}

void IceFieldSpell::End()
{
	DamageRequestEnd(m_longinfo_damage);
	
	ARX_SOUND_Stop(m_snd_loop); 
	ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD_END);
}

void IceFieldSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		
		CIceField *pf = (CIceField *) pCSpellFX;

		if(!lightHandleIsValid(m_longinfo2_light))
			m_longinfo2_light = GetFreeDynLight();

		if(lightHandleIsValid(m_longinfo2_light)) {
			EERIE_LIGHT * el = lightHandleGet(m_longinfo2_light);
			
			el->pos.x = pf->eSrc.x;
			el->pos.y = pf->eSrc.y-120.f;
			el->pos.z = pf->eSrc.z;
			el->intensity = 4.6f;
			el->fallstart = 150.f+rnd()*30.f;
			el->fallend   = 290.f+rnd()*30.f;
			el->rgb.r = 0.76f;
			el->rgb.g = 0.76f;
			el->rgb.b = 1.0f-rnd()*( 1.0f / 10 );
			el->duration = 600;
			el->extras=0;
		}

		pCSpellFX->Render();
	}
}

void LightningStrikeSpell::Launch()
{
	CLightning * effect = new CLightning();
	effect->spellinstance = m_thisHandle;
	Vec3f target(0.f, 0.f, -500.f);
	effect->Create(Vec3f_ZERO, target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(long(500 * m_level));
	effect->m_isMassLightning = false;
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &m_caster_pos);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP,
	                                       &m_caster_pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
}

void LightningStrikeSpell::End()
{
	ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[m_caster]->pos);
	
	if(lightHandleIsValid(m_longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo_light);
		
		light->duration = 200;
		light->time_creation = (unsigned long)(arxtime);
	}
	m_longinfo_light = InvalidLightHandle;
	
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_END, &entities[m_caster]->pos);
}

void LightningStrikeSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_caster]->pos);
}

void ConfuseSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_CONFUSE, &entities[m_target]->pos);
	
	m_bDuration = true;
	m_fManaCostPerSecond = 1.5f;
	m_tolive = (m_launchDuration > -1) ? m_launchDuration : 5000;
	
	CConfuse * effect = new CConfuse();
	effect->Create();
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	m_targets.push_back(m_target);
}

void ConfuseSpell::End()
{
	m_targets.clear();
}

void ConfuseSpell::Update(float timeDelta)
{
	CConfuse * effect = static_cast<CConfuse *>(m_pSpellFx);
	
	if(effect) {
		Vec3f pos = entities[m_target]->pos;
		if(m_target != PlayerEntityHandle) {
			pos.y += entities[m_target]->physics.cyl.height - 30.f;
		}
		
		long idx = entities[m_target]->obj->fastaccess.head_group_origin;
		if(idx >= 0) {
			pos = entities[m_target]->obj->vertexlist3[idx].v;
			pos.y -= 50.f;
		}
		
		effect->SetPos(pos);
		effect->Update(timeDelta);
		effect->Render();
	}
}
