/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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
#include "math/RandomVector.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RiseDeadSpell::GetTargetAndBeta(Vec3f & target, float & beta) {
	
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
	: m_targetPos(0.f)
	, m_creationFailed(false)
{ }

bool RiseDeadSpell::CanLaunch() {
	
	// TODO Always cancel spell even if new one can't be launched ?
	spells.endByCaster(m_caster, SPELL_RISE_DEAD);
	
	float beta;
	Vec3f target;
	
	GetTargetAndBeta(target, beta);

	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE);
		return false;
	}

	return true;
}

void RiseDeadSpell::Launch() {
	
	float beta;
	Vec3f target;
	
	GetTargetAndBeta(target, beta);
	
	m_targetPos = target;
	ARX_SOUND_PlaySFX(g_snd.SPELL_RAISE_DEAD, &m_targetPos);
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 1.2f;
	
	m_creationFailed = false;
	m_entity = EntityHandle();
	
	m_fissure.Create(target, beta);
	m_fissure.SetDuration(GameDurationMs(2000), GameDurationMs(500), GameDurationMs(1800));
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
		light->duration = GameDurationMs(200);
		light->creationTime = g_gameTime.now();
	}
	
	m_duration = m_fissure.GetDuration();
}

void RiseDeadSpell::End() {
	
	Entity * entity = entities.get(m_entity);
	if(entity) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_ELECTRIC, &entity->pos);
		
		if(entity->scriptload && (entity->ioflags & IO_NOSAVE)) {
			AddRandomSmoke(*entity, 100);
			Vec3f posi = entity->pos;
			posi.y -= 100.f;
			MakeCoolFx(posi);
			
			EERIE_LIGHT * light = dynLightCreate();
			if(light) {
				light->intensity = Random::getf(0.7f, 2.7f);
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.f);
				light->pos = posi;
				light->duration = GameDurationMs(600);
			}

			entity->destroyOne();
		}
	}
	
	endLightDelayed(m_light, GameDurationMs(500));
}

void RiseDeadSpell::Update() {
	
	if(m_creationFailed) {
		m_light = LightHandle();
		return;
	}
	
	// TODO why is the duration extended here ?
	m_duration += GameDurationMs(200);
	
	m_fissure.Update(g_gameTime.lastFrameDuration());
	m_fissure.Render();
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		light->intensity = 0.7f + 2.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(0.8f, 0.2f, 0.2f);
		light->duration = GameDurationMs(800);
		light->creationTime = g_gameTime.now();
	}
	
	GameDuration tim = m_fissure.m_elapsed;
	
	if(tim > GameDurationMs(3000) && m_entity == EntityHandle() && !m_creationFailed) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_ELECTRIC, &m_targetPos);
		
		Cylinder phys = Cylinder(m_targetPos, 50, -200);
		
		float anything = CheckAnythingInCylinder(phys, NULL, CFLAG_JUST_TEST);
		
		if(glm::abs(anything) < 30) {
			
			const char * cls = "graph/obj3d/interactive/npc/undead_base/undead_base";
			Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
			
			if(io) {
				ARX_INTERACTIVE_HideGore(io);
				RestoreInitialIOStatusOfIO(io);
				
				io->summoner = m_caster;
				
				io->ioflags |= IO_NOSAVE;
				m_entity = io->index();
				io->scriptload = 1;
				
				ARX_INTERACTIVE_Teleport(io, phys.origin);
				SendInitScriptEvent(io);
				
				SendIOScriptEvent(entities.get(m_caster), io, SM_SUMMONED);
				
				Vec3f pos = m_fissure.m_eSrc;
				pos += arx::randomVec3f() * 100.f;
				pos += Vec3f(-50.f, 50.f, -50.f);
				
				MakeCoolFx(pos);
			}
			
			m_light = LightHandle();
		} else {
			ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE);
			m_creationFailed = true;
			requestEnd();
		}
	} else if(!g_gameTime.isPaused() && tim < GameDurationMs(4000)) {
		if(Random::getf() > 0.95f) {
			MakeCoolFx(m_fissure.m_eSrc);
		}
	}
	
}

void ParalyseSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_PARALYSE, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(5000);
	m_hasDuration = true;
	
	float resist_magic = 0.f;
	if(m_target == EntityHandle_Player && m_level <= player.level) {
		resist_magic = player.m_miscFull.resistMagic;
	} else if(entities[m_target]->ioflags & IO_NPC) {
		resist_magic = entities[m_target]->_npcdata->resist_magic;
	}
	if(Random::getf(0.f, 100.f) < resist_magic) {
		float mul = std::max(0.5f, 1.f - (resist_magic * 0.005f));
		m_duration = GameDurationMsf(toMsf(m_duration) * mul);
	}
	
	entities[m_target]->ioflags |= IO_FREEZESCRIPT;
	
	m_targets.push_back(m_target);
	ARX_NPC_Kill_Spell_Launch(entities[m_target]);
}

void ParalyseSpell::End() {
	
	m_targets.clear();
	
	Entity * target = entities.get(m_target);
	if(target) {
		target->ioflags &= ~IO_FREEZESCRIPT;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_PARALYSE_END);
}

Vec3f ParalyseSpell::getPosition() {
	return getTargetPosition();
}


CreateFieldSpell::CreateFieldSpell() { }

void CreateFieldSpell::Launch() {
	
	GameInstant start = g_gameTime.now();
	if(m_flags & SPELLCAST_FLAG_RESTORE) {
		// Move time of creation back by 4 seconds or whatever elapsed after game time 0 (if it is smaller)
		// Prevents difference between creation time and elapsed time of m_field (or as small as possible)
		// related to m_field.Update() with comment below
		start -= std::min(start - GameInstant(0), GameDurationMs(4000));
	}
	m_timcreation = start;
	
	m_hasDuration = m_launchDuration >= 0;
	m_duration = m_hasDuration ? m_launchDuration : 0;
	m_fManaCostPerSecond = 1.2f;
	
	Vec3f target;
	float beta;
	bool displace;
	if(m_caster == EntityHandle_Player) {
		target = entities.player()->pos;
		beta = player.angle.getYaw();
		displace = true;
	} else {
		Entity * io = entities.get(m_caster);
		arx_assert(io);
		target = io->pos;
		beta = io->angle.getYaw();
		displace = (io->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 250.f;
	}
	
	// Don't play sound for persistent fields
	if(!(m_flags & SPELLCAST_FLAG_RESTORE)) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_CREATE_FIELD, &target);
	}
	
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
		
		EERIE_LIGHT * light = dynLightCreate(m_field.lLightId);
		if(light) {
			light->intensity = 0.7f + 2.3f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb = Color3f(0.8f, 0.0f, 1.0f);
			light->pos = m_field.eSrc - Vec3f(0.f, 150.f, 0.f);
		}
		
		if(m_flags & SPELLCAST_FLAG_RESTORE) {
			// Fast forward the field's animation so that players don't see it
			// being casted in front of them on game load
			m_field.Update(GameDurationMs(4000));
		}
		
	} else {
		requestEnd();
	}
	
}

void CreateFieldSpell::End() {
	
	endLightDelayed(m_field.lLightId, GameDurationMs(800));
	
	delete entities.get(m_entity);
}

void CreateFieldSpell::Update() {
	
	Entity * io = entities.get(m_entity);
	if(io) {
		io->pos = m_field.eSrc;
		
		if(IsAnyNPCInPlatform(io)) {
			requestEnd();
		}
		
		m_field.Update(g_gameTime.lastFrameDuration());
		m_field.Render();
	}
}

Vec3f CreateFieldSpell::getPosition() {
	
	return m_field.eSrc;
}

void DisarmTrapSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_DISARM_TRAP);
	
	m_duration = GameDurationMs(1);
	m_hasDuration = true;
	
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

void SlowDownSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_SLOW_DOWN, &entities[m_target]->pos);
	
	if(m_caster == EntityHandle_Player) {
		m_duration = 0;
		m_hasDuration = false;
	} else {
		m_duration = (m_launchDuration >= 0) ? m_launchDuration : GameDurationMs(10000);
		m_hasDuration = true;
	}
	
	m_fManaCostPerSecond = 1.2f;
	
	m_targets.push_back(m_target);
}

void SlowDownSpell::End() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_SLOW_DOWN_END);
	m_targets.clear();
}

void SlowDownSpell::Update() {
	
}

Vec3f SlowDownSpell::getPosition() {
	
	return getTargetPosition();
}
