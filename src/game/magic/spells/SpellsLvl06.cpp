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

#include "game/magic/spells/SpellsLvl06.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells05.h"
#include "graphics/spells/Spells06.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RiseDeadSpell::GetTargetAndBeta(Vec3f & target, float & beta)
{
	bool displace = true;
	
	if(m_caster == PlayerEntityHandle) {
		target = player.basePosition();
		beta = player.angle.getPitch();
	} else {
		target = entities[m_caster]->pos;
		beta = entities[m_caster]->angle.getPitch();
		displace = (entities[m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 300.f;
	}
}

RiseDeadSpell::RiseDeadSpell()
	: m_entity(EntityHandle::Invalid)
{
	
}

bool RiseDeadSpell::CanLaunch()
{
	//TODO always cancel spell even if new one can't be launched ?
	spells.endByCaster(m_caster, SPELL_RISE_DEAD);
	
	float beta;
	Vec3f target;
	
	GetTargetAndBeta(target, beta);

	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}

	return true;
}

void RiseDeadSpell::Launch()
{
	float beta;
	Vec3f target;
	
	GetTargetAndBeta(target, beta);
	
	m_targetPos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_RAISE_DEAD, &m_targetPos);
	
	// TODO this tolive value is probably never read
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	m_entity = EntityHandle::Invalid;
	
	CRiseDead * effect = new CRiseDead();
	effect->Create(target, beta);
	effect->SetDuration(2000, 500, 1800);
	effect->SetColorBorder(0.5, 0.5, 0.5);
	effect->SetColorRays1(0.5, 0.5, 0.5);
	effect->SetColorRays2(1, 0, 0);
	
	if(!lightHandleIsValid(effect->lLightId)) {
		effect->lLightId = GetFreeDynLight();
	}
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 1.3f;
		light->fallend = 450.f;
		light->fallstart = 380.f;
		light->rgb = Color3f::black;
		light->pos = target - Vec3f(0.f, 100.f, 0.f);
		light->duration = 200;
		light->time_creation = (unsigned long)(arxtime);
	}
	
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
}

void RiseDeadSpell::End()
{
	if(ValidIONum(m_entity) && m_entity != PlayerEntityHandle) {
		
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[m_entity]->pos);
		
		Entity *entity = entities[m_entity];

		if(entity->scriptload && (entity->ioflags & IO_NOSAVE)) {
			AddRandomSmoke(entity,100);
			Vec3f posi = entity->pos;
			posi.y-=100.f;
			MakeCoolFx(posi);
			
			LightHandle nn = GetFreeDynLight();

			if(lightHandleIsValid(nn)) {
				EERIE_LIGHT * light = lightHandleGet(nn);
				
				light->intensity = 0.7f + 2.f*rnd();
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.f);
				light->pos = posi;
				light->duration = 600;
			}

			entity->destroyOne();
		}
	}
}

void RiseDeadSpell::Update(float timeDelta)
{
	CRiseDead * effect = static_cast<CRiseDead *>(m_pSpellFx);
	if(!effect)
		return;
	
	if(m_entity == -2) {
		effect->lLightId = LightHandle::Invalid;
		return;
	}
	
	m_duration+=200;
	
	effect->Update(timeDelta);
	effect->Render();
	
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.7f + 2.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb.r = 0.8f;
		light->rgb.g = 0.2f;
		light->rgb.b = 0.2f;
		light->duration=800;
		light->time_creation = (unsigned long)(arxtime);
	}
	
	unsigned long tim = effect->ulCurrentTime;
	
	if(tim > 3000 && m_entity == EntityHandle::Invalid) {
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
		
		Cylinder phys;
		phys.height = -200;
		phys.radius = 50;
		phys.origin = m_targetPos;
		
		float anything = CheckAnythingInCylinder(phys, NULL, CFLAG_JUST_TEST);
		
		if(glm::abs(anything) < 30) {
			
			const char * cls = "graph/obj3d/interactive/npc/undead_base/undead_base";
			Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
			
			if(io) {
				ARX_INTERACTIVE_HideGore(io);
				RestoreInitialIOStatusOfIO(io);
				
				long lSpellsCaster = m_caster;
				io->summoner = checked_range_cast<short>(lSpellsCaster);
				
				io->ioflags|=IO_NOSAVE;
				m_entity = io->index();
				io->scriptload=1;
				
				ARX_INTERACTIVE_Teleport(io, phys.origin);
				SendInitScriptEvent(io);
				
				if(ValidIONum(m_caster)) {
					EVENT_SENDER = entities[m_caster];
				} else {
					EVENT_SENDER = NULL;
				}
				
				SendIOScriptEvent(io,SM_SUMMONED);
					
				Vec3f pos = effect->eSrc;
				pos += Vec3f(rnd(), rnd(), rnd()) * 100.f;
				pos += Vec3f(-50.f, 50.f, -50.f);
				
				MakeCoolFx(pos);
			}
			
			effect->lLightId = LightHandle::Invalid;
		} else {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
			m_entity = EntityHandle(-2); // FIXME inband signaling
			m_duration=0;
		}
	} else if(!arxtime.is_paused() && tim < 4000) {
	  if(rnd() > 0.95f) {
			MakeCoolFx(effect->eSrc);
		}
	}
}

void ParalyseSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 5000;
	
	float resist_magic = 0.f;
	if(m_target == PlayerEntityHandle && m_level <= player.level) {
		resist_magic = player.m_misc.resistMagic;
	} else if(entities[m_target]->ioflags & IO_NPC) {
		resist_magic = entities[m_target]->_npcdata->resist_magic;
	}
	if(rnd() * 100.f < resist_magic) {
		float mul = std::max(0.5f, 1.f - (resist_magic * 0.005f));
		m_duration = long(m_duration * mul);
	}
	
	entities[m_target]->ioflags |= IO_FREEZESCRIPT;
	
	m_targets.push_back(m_target);
	ARX_NPC_Kill_Spell_Launch(entities[m_target]);
}

void ParalyseSpell::End()
{
	m_targets.clear();
	entities[m_target]->ioflags &= ~IO_FREEZESCRIPT;
	
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

Vec3f ParalyseSpell::getPosition() {
	return getTargetPosition();
}

CreateFieldSpell::CreateFieldSpell()
	: m_entity(EntityHandle::Invalid)
{
}

void CreateFieldSpell::Launch()
{
	unsigned long start = (unsigned long)(arxtime);
	if(m_flags & SPELLCAST_FLAG_RESTORE) {
		start -= std::min(start, 4000ul);
	}
	m_timcreation = start;
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 800000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	Vec3f target;
	float beta = 0.f;
	bool displace = false;
	if(m_caster == PlayerEntityHandle) {
		target = entities.player()->pos;
		beta = player.angle.getPitch();
		displace = true;
	} else {
		if(ValidIONum(m_caster)) {
			Entity * io = entities[m_caster];
			target = io->pos;
			beta = io->angle.getPitch();
			displace = (io->ioflags & IO_NPC) == IO_NPC;
		} else {
			ARX_DEAD_CODE();
		}
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 250.f;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD, &target);
	
	CCreateField * effect  = new CCreateField();
	m_pSpellFx = effect;
	
	res::path cls = "graph/obj3d/interactive/fix_inter/blue_cube/blue_cube";
	Entity * io = AddFix(cls, -1, IO_IMMEDIATELOAD);
	if(io) {
		
		ARX_INTERACTIVE_HideGore(io);
		RestoreInitialIOStatusOfIO(io);
		m_entity = io->index();
		io->scriptload = 1;
		io->ioflags |= IO_NOSAVE | IO_FIELD;
		io->initpos = io->pos = target;
		SendInitScriptEvent(io);
		
		effect->Create(target);
		effect->SetDuration(m_duration);
		effect->lLightId = GetFreeDynLight();
		
		if(lightHandleIsValid(effect->lLightId)) {
			EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
			
			light->intensity = 0.7f + 2.3f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb = Color3f(0.8f, 0.0f, 1.0f);
			light->pos = effect->eSrc - Vec3f(0.f, 150.f, 0.f);
		}
		
		m_duration = effect->GetDuration();
		
		if(m_flags & SPELLCAST_FLAG_RESTORE) {
			effect->Update(4000);
		}
		
	} else {
		m_duration = 0;
	}
}

void CreateFieldSpell::End()
{
	CCreateField *pCreateField = (CCreateField *) m_pSpellFx;

	if(pCreateField && lightHandleIsValid(pCreateField->lLightId)) {
		lightHandleGet(pCreateField->lLightId)->duration = 800;
	}

	if(ValidIONum(m_entity)) {
		delete entities[m_entity];
	}
}

void CreateFieldSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	
	if(pCSpellFX) {
		if(ValidIONum(m_entity)) {
			Entity * io = entities[m_entity];
			
			CCreateField * ccf=(CCreateField *)pCSpellFX;
			io->pos = ccf->eSrc;

			if (IsAnyNPCInPlatform(io))
			{
				m_duration=0;
			}
		
			pCSpellFX->Update(timeDelta);			
			pCSpellFX->Render();
		}
	}
}

Vec3f CreateFieldSpell::getPosition() {
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		CCreateField *pCreateField = (CCreateField *) pCSpellFX;
			
		return pCreateField->eSrc;
	} else {
		return Vec3f_ZERO;
	}
}

void DisarmTrapSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISARM_TRAP);
	
	m_duration = 1;
	
	Sphere sphere;
	sphere.origin = player.pos;
	sphere.radius = 400.f;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell || spell->m_type != SPELL_RUNE_OF_GUARDING) {
			continue;
		}
		
		if(!spell->m_pSpellFx) {
			continue;
		}
		
		CSpellFx * effect = spell->m_pSpellFx;
		if(sphere.contains(static_cast<CRuneOfGuarding *>(effect)->eSrc)) {
			spell->m_level -= m_level;
			if(spell->m_level <= 0) {
				spells.endSpell(spell);
			}
		}
	}
}

bool SlowDownSpell::CanLaunch()
{
	// TODO this seems to be the only spell that ends itself when cast twice
	SpellBase * spell = spells.getSpellOnTarget(m_target, SPELL_SLOW_DOWN);
	if(spell) {
		spells.endSpell(spell);
		return false;
	}
	
	return true;
}

void SlowDownSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN, &entities[m_target]->pos);
	
	m_duration = (m_caster == PlayerEntityHandle) ? 10000000 : 10000;
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	}
	m_pSpellFx = NULL;
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	Vec3f targetPos = getTargetPos(m_caster, m_target);
	
	CSlowDown * effect = new CSlowDown();
	effect->Create(targetPos);
	effect->SetDuration(m_duration);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
	
	m_targets.push_back(m_target);
}

void SlowDownSpell::End()
{
	ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN_END);
	m_targets.clear();
}

void SlowDownSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
}

Vec3f SlowDownSpell::getPosition() {
	
	return getTargetPosition();
}
