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

bool RiseDeadSpell::Launch(long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_RISE_DEAD, m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	float beta;
	Vec3f target;
	bool displace = true;
	if(m_caster == 0) {
		target = player.basePosition();
		beta = MAKEANGLE(player.angle.getPitch());
	} else {
		target = entities[m_caster]->pos;
		beta = MAKEANGLE(entities[m_caster]->angle.getPitch());
		displace = (entities[m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target.x -= std::sin(radians(beta)) * 300.f;
		target.z += std::cos(radians(beta)) * 300.f;
	}
	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	m_target_pos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_RAISE_DEAD, &m_target_pos);
	m_exist = true;
	// TODO this tolive value is probably never read
	m_tolive = (duration > -1) ? duration : 2000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 1.2f;
	m_longinfo_entity = -1;
	
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
	m_tolive = effect->GetDuration();
	
	return true;
}

void RiseDeadSpell::End()
{
	if(ValidIONum(m_longinfo_entity) && m_longinfo_entity != 0) {
		
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[m_longinfo_entity]->pos);
		
		Entity *entity = entities[m_longinfo_entity];

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
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		if(m_longinfo_entity == -2) {
			pCSpellFX->lLightId=-1;
			return;
		}

		m_tolive+=200;
	
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();

		if(lightHandleIsValid(pCSpellFX->lLightId)) {
			EERIE_LIGHT * light = lightHandleGet(pCSpellFX->lLightId);
			
			light->intensity = 0.7f + 2.3f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb.r = 0.8f;
			light->rgb.g = 0.2f;
			light->rgb.b = 0.2f;
			light->duration=800;
			light->time_creation = (unsigned long)(arxtime);
		}

		unsigned long tim=pCSpellFX->getCurrentTime();

		if(tim > 3000 && m_longinfo_entity == -1) {
			ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_target_pos);
			CRiseDead *prise = (CRiseDead *)m_pSpellFx;

			if(prise) {
				EERIE_CYLINDER phys;
				phys.height=-200;
				phys.radius=50;
				phys.origin=m_target_pos;

				float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

				if(EEfabs(anything) < 30) {
					
					const char * cls = "graph/obj3d/interactive/npc/undead_base/undead_base";
					Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
					
					if(io) {
						ARX_INTERACTIVE_HideGore(io);
						RestoreInitialIOStatusOfIO(io);
						
						long lSpellsCaster = m_caster;
						io->summoner = checked_range_cast<short>(lSpellsCaster);
						
						io->ioflags|=IO_NOSAVE;
						m_longinfo_entity = io->index();
						io->scriptload=1;
						
						ARX_INTERACTIVE_Teleport(io, phys.origin);
						SendInitScriptEvent(io);

						if(ValidIONum(m_caster)) {
							EVENT_SENDER = entities[m_caster];
						} else {
							EVENT_SENDER = NULL;
						}

						SendIOScriptEvent(io,SM_SUMMONED);
							
						Vec3f pos;
						pos.x=prise->eSrc.x+rnd()*100.f-50.f;
						pos.y=prise->eSrc.y+100+rnd()*100.f-50.f;
						pos.z=prise->eSrc.z+rnd()*100.f-50.f;
						MakeCoolFx(pos);
					}

					pCSpellFX->lLightId=-1;
				} else {
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
					m_longinfo_entity = -2;
					m_tolive=0;
				}
			}
		} else if(!arxtime.is_paused() && tim < 4000) {
		  if(rnd() > 0.95f) {
				CRiseDead *pRD = (CRiseDead*)pCSpellFX;
				Vec3f pos = pRD->eSrc;
				MakeCoolFx(pos);
			}
		}

	}
}

void ParalyseSpell::Launch(long i, long duration)
{
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE, &entities[m_target]->pos);
	
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 5000;
	
	float resist_magic = 0.f;
	if(m_target == 0 && m_caster_level <= player.level) {
		resist_magic = player.resist_magic;
	} else if(entities[m_target]->ioflags & IO_NPC) {
		resist_magic = entities[m_target]->_npcdata->resist_magic;
	}
	if(rnd() * 100.f < resist_magic) {
		float mul = max(0.5f, 1.f - (resist_magic * 0.005f));
		m_tolive = long(m_tolive * mul);
	}
	
	entities[m_target]->ioflags |= IO_FREEZESCRIPT;
	
	ARX_SPELLS_AddSpellOn(m_target, i);
	ARX_NPC_Kill_Spell_Launch(entities[m_target]);
}

void ParalyseSpell::End(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(m_target,i);
	entities[m_target]->ioflags &= ~IO_FREEZESCRIPT;
	
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

void CreateFieldSpell::Launch(SpellcastFlags flags, long duration)
{
	m_exist = true;
	
	unsigned long start = (unsigned long)(arxtime);
	if(flags & SPELLCAST_FLAG_RESTORE) {
		start -= std::min(start, 4000ul);
	}
	m_timcreation = start;
	
	m_tolive = (duration > -1) ? duration : 800000;
	m_bDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	Vec3f target;
	float beta;
	bool displace = false;
	if(m_caster == 0) {
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
		target.x -= std::sin(radians(MAKEANGLE(beta))) * 250.f;
		target.z += std::cos(radians(MAKEANGLE(beta))) * 250.f;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD, &target);
	
	CCreateField * effect  = new CCreateField();
	
	res::path cls = "graph/obj3d/interactive/fix_inter/blue_cube/blue_cube";
	Entity * io = AddFix(cls, -1, IO_IMMEDIATELOAD);
	if(io) {
		
		ARX_INTERACTIVE_HideGore(io);
		RestoreInitialIOStatusOfIO(io);
		m_longinfo_entity = io->index();
		io->scriptload = 1;
		io->ioflags |= IO_NOSAVE | IO_FIELD;
		io->initpos = io->pos = target;
		SendInitScriptEvent(io);
		
		effect->Create(target);
		effect->SetDuration(m_tolive);
		effect->lLightId = GetFreeDynLight();
		
		if(lightHandleIsValid(effect->lLightId)) {
			EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
			
			light->intensity = 0.7f + 2.3f;
			light->fallend = 500.f;
			light->fallstart = 400.f;
			light->rgb = Color3f(0.8f, 0.0f, 1.0f);
			light->pos = effect->eSrc - Vec3f(0.f, 150.f, 0.f);
		}
		
		m_pSpellFx = effect;
		m_tolive = effect->GetDuration();
		
		if(flags & SPELLCAST_FLAG_RESTORE) {
			effect->Update(4000);
		}
		
	} else {
		m_tolive = 0;
	}
}

void CreateFieldSpell::End()
{
	CCreateField *pCreateField = (CCreateField *) m_pSpellFx;

	if(pCreateField && lightHandleIsValid(pCreateField->lLightId)) {
		lightHandleGet(pCreateField->lLightId)->duration = 800;
	}

	if(ValidIONum(m_longinfo_entity)) {
		delete entities[m_longinfo_entity];
	}
}

void CreateFieldSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	
	if(pCSpellFX) {
		if(ValidIONum(m_longinfo_entity)) {
			Entity * io = entities[m_longinfo_entity];
			
			CCreateField * ccf=(CCreateField *)pCSpellFX;
			io->pos = ccf->eSrc;

			if (IsAnyNPCInPlatform(io))
			{
				m_tolive=0;
			}
		
			pCSpellFX->Update(timeDelta);			
			pCSpellFX->Render();
		}
	}
}

void DisarmTrapSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_DISARM_TRAP);
	
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	m_tolive = 1;
	
	EERIE_SPHERE sphere;
	sphere.origin = player.pos;
	sphere.radius = 400.f;
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].m_exist || spells[n].m_type != SPELL_RUNE_OF_GUARDING) {
			continue;
		}
		
		if(!spells[n].m_pSpellFx) {
			continue;
		}
		
		CSpellFx * effect = spells[n].m_pSpellFx;
		if(sphere.contains(static_cast<CRuneOfGuarding *>(effect)->eSrc)) {
			spells[n].m_caster_level -= m_caster_level;
			if(spells[n].m_caster_level <= 0) {
				spells[n].m_tolive = 0;
			}
		}
	}
}

bool SlowDownSpell::Launch(long duration, long i)
{
	long target = m_target;
	
	Entity * io = entities[target];
	
	boost::container::flat_set<long>::const_iterator it;
	for(it = io->spellsOn.begin(); it != io->spellsOn.end(); ++it) {
		long spellHandle = *it;
		if(spellHandleIsValid(spellHandle)) {
			SpellBase * spell = &spells[spellHandle];
			
			if(spell->m_type == SPELL_SLOW_DOWN) {
				spell->m_exist = false;
				return false;
			}
		}
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN, &entities[m_target]->pos);
	
	m_exist = true;
	m_tolive = (m_caster == 0) ? 10000000 : 10000;
	if(duration > -1) {
		m_tolive=duration;
	}
	m_pSpellFx = NULL;
	m_bDuration = true;
	m_fManaCostPerSecond = 1.2f;
	
	CSlowDown * effect = new CSlowDown();
	effect->Create(m_target_pos, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(target, i);
	
	return true;
}

void SlowDownSpell::End(size_t i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN_END);
	ARX_SPELLS_RemoveSpellOn(m_target, i);
}

void SlowDownSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
}
