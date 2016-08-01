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
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/magic/spells/SpellsLvl05.h"
#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells05.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RiseDeadSpell::GetTargetAndBeta(Vec3f & target, float & beta)
{
	bool displace = true;
	
	if(m_caster == EntityHandle_Player) {
		target = player.basePosition();
		beta = player.angle.getYaw();
	} else {
		target = entities[m_caster]->pos;
		beta = entities[m_caster]->angle.getYaw();
		displace = (entities[m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 300.f;
	}
}

RiseDeadSpell::RiseDeadSpell()
	: m_creationFailed(false)
	, m_entity()
{ }

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
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(2000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	m_creationFailed = false;
	m_entity = EntityHandle();
	
	m_fissure.Create(target, beta);
	m_fissure.SetDuration(ArxDurationMs(2000), ArxDurationMs(500), ArxDurationMs(1800));
	m_fissure.SetColorBorder(Color3f(0.5, 0.5, 0.5));
	m_fissure.SetColorRays1(Color3f(0.5, 0.5, 0.5));
	m_fissure.SetColorRays2(Color3f(1.f, 0.f, 0.f));
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 1.3f;
		light->fallend = 450.f;
		light->fallstart = 380.f;
		light->rgb = Color3f::black;
		light->pos = target - Vec3f(0.f, 100.f, 0.f);
		light->duration = ArxDurationMs(200);
		light->creationTime = arxtime.now();
	}
	
	m_duration = m_fissure.GetDuration();
}

void RiseDeadSpell::End()
{
	if(ValidIONum(m_entity)) {
		Entity *entity = entities[m_entity];
		
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entity->pos);
		
		if(entity->scriptload && (entity->ioflags & IO_NOSAVE)) {
			AddRandomSmoke(entity,100);
			Vec3f posi = entity->pos;
			posi.y-=100.f;
			MakeCoolFx(posi);
			
			EERIE_LIGHT * light = dynLightCreate();
			if(light) {
				light->intensity = Random::getf(0.7f, 2.7f);
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.f);
				light->pos = posi;
				light->duration = ArxDurationMs(600);
			}

			entity->destroyOne();
		}
	}
	
	endLightDelayed(m_light, ArxDurationMs(500));
}

void RiseDeadSpell::Update() {
	
	if(m_creationFailed) {
		m_light = LightHandle();
		return;
	}
	
	m_duration+=200;
	
	m_fissure.Update(g_framedelay);
	m_fissure.Render();
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		light->intensity = 0.7f + 2.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(0.8f, 0.2f, 0.2f);
		light->duration = ArxDurationMs(800);
		light->creationTime = arxtime.now();
	}
	
	unsigned long tim = m_fissure.m_elapsed;
	
	if(tim > 3000 && m_entity == EntityHandle() && !m_creationFailed) {
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
		
		Cylinder phys = Cylinder(m_targetPos, 50, -200);
		
		float anything = CheckAnythingInCylinder(phys, NULL, CFLAG_JUST_TEST);
		
		if(glm::abs(anything) < 30) {
			
			const char * cls = "graph/obj3d/interactive/npc/undead_base/undead_base";
			Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
			
			if(io) {
				ARX_INTERACTIVE_HideGore(io);
				RestoreInitialIOStatusOfIO(io);
				
				io->summoner = m_caster;
				
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
					
				Vec3f pos = m_fissure.m_eSrc;
				pos += randomVec3f() * 100.f;
				pos += Vec3f(-50.f, 50.f, -50.f);
				
				MakeCoolFx(pos);
			}
			
			m_light = LightHandle();
		} else {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
			m_creationFailed = true;
			m_duration = ArxDuration_ZERO;
		}
	} else if(!arxtime.is_paused() && tim < 4000) {
	  if(Random::getf() > 0.95f) {
			MakeCoolFx(m_fissure.m_eSrc);
		}
	}
}

void ParalyseSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(5000);
	
	float resist_magic = 0.f;
	if(m_target == EntityHandle_Player && m_level <= player.level) {
		resist_magic = player.m_misc.resistMagic;
	} else if(entities[m_target]->ioflags & IO_NPC) {
		resist_magic = entities[m_target]->_npcdata->resist_magic;
	}
	if(Random::getf(0.f, 100.f) < resist_magic) {
		float mul = std::max(0.5f, 1.f - (resist_magic * 0.005f));
		m_duration = ArxDurationMs(m_duration * mul);
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
	: m_entity()
{
}


void CreateFieldSpell::Launch()
{
	ArxInstant start = arxtime.now();
	if(m_flags & SPELLCAST_FLAG_RESTORE) {
		// FIXME what is going on here ?
		start -= std::min(start, ArxInstantMs(4000l));
	}
	m_timcreation = start;
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(800000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	Vec3f target;
	float beta = 0.f;
	bool displace = false;
	if(m_caster == EntityHandle_Player) {
		target = entities.player()->pos;
		beta = player.angle.getYaw();
		displace = true;
	} else {
		if(ValidIONum(m_caster)) {
			Entity * io = entities[m_caster];
			target = io->pos;
			beta = io->angle.getYaw();
			displace = (io->ioflags & IO_NPC) == IO_NPC;
		} else {
			ARX_DEAD_CODE();
		}
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 250.f;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD, &target);
	
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
		
		m_field.Create(target);
		m_field.SetDuration(m_duration);
		
		EERIE_LIGHT * light = dynLightCreate(m_field.lLightId);
		if(light) {
			light->intensity = 0.7f + 2.3f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb = Color3f(0.8f, 0.0f, 1.0f);
			light->pos = m_field.eSrc - Vec3f(0.f, 150.f, 0.f);
		}
		
		m_duration = m_field.m_duration;
		
		if(m_flags & SPELLCAST_FLAG_RESTORE) {
			m_field.Update(ArxDurationMs(4000));
		}
		
	} else {
		m_duration = ArxDuration_ZERO;
	}
}

void CreateFieldSpell::End() {
	
	endLightDelayed(m_field.lLightId, ArxDurationMs(800));
	
	if(ValidIONum(m_entity)) {
		delete entities[m_entity];
	}
}

void CreateFieldSpell::Update() {
	
	if(ValidIONum(m_entity)) {
		Entity * io = entities[m_entity];
		
		io->pos = m_field.eSrc;
		
		if (IsAnyNPCInPlatform(io))
		{
			m_duration=ArxDuration_ZERO;
		}
		
		m_field.Update(ArxDurationMs(g_framedelay));
		m_field.Render();
	}
}

Vec3f CreateFieldSpell::getPosition() {
	
	return m_field.eSrc;
}

void DisarmTrapSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISARM_TRAP);
	
	m_duration = ArxDurationMs(1);
	
	Sphere sphere;
	sphere.origin = player.pos;
	sphere.radius = 400.f;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell || spell->m_type != SPELL_RUNE_OF_GUARDING) {
			continue;
		}
		
		Vec3f pos = static_cast<RuneOfGuardingSpell *>(spell)->getPosition();
		
		if(sphere.contains(pos)) {
			spell->m_level -= m_level;
			if(spell->m_level <= 0) {
				spells.endSpell(spell);
			}
		}
	}
}


bool SlowDownSpell::CanLaunch() {
	
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
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(10000);
	
	if(m_caster == EntityHandle_Player)
		m_duration = ArxDurationMs(10000000);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	m_targets.push_back(m_target);
}

void SlowDownSpell::End() {
	
	ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN_END);
	m_targets.clear();
}

void SlowDownSpell::Update() {
	
}

Vec3f SlowDownSpell::getPosition() {
	
	return getTargetPosition();
}
