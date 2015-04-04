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

#include "game/magic/spells/SpellsLvl09.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"
#include "game/spell/Cheat.h"

#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells09.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void SummonCreatureSpell::GetTargetAndBeta(Vec3f & target, float & beta)
{
	bool displace = false;
	if(m_caster == PlayerEntityHandle) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		target = entities[m_caster]->pos;
		beta = entities[m_caster]->angle.getPitch();
		displace = (entities[m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 300.f;
	}
}

SummonCreatureSpell::SummonCreatureSpell()
	: m_targetPos(Vec3f_ZERO)
	, m_megaCheat(false)
	, m_longinfo_summon_creature(-1) //TODO is this correct ?
	, m_longinfo2_entity(EntityHandle::Invalid)
{
	
}

bool SummonCreatureSpell::CanLaunch()
{
	Vec3f target;
	float beta;
	GetTargetAndBeta(target, beta);
	
	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	return true;
}

void SummonCreatureSpell::Launch()
{
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.9f;
	m_longinfo_summon_creature = 0;
	m_longinfo2_entity = PlayerEntityHandle; // TODO is this correct ?
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000;
	
	Vec3f target;
	float beta;
	GetTargetAndBeta(target, beta);
	
	m_megaCheat = (m_caster == PlayerEntityHandle && cur_mega == 10);
	m_targetPos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &m_targetPos);
	CSummonCreature * effect = new CSummonCreature();
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	m_pSpellFx = effect;
}

void SummonCreatureSpell::End()
{
	if(ValidIONum(m_longinfo2_entity) && m_longinfo2_entity != PlayerEntityHandle) {
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[m_longinfo2_entity]->pos);
	}

	lightHandleDestroy(m_pSpellFx->lLightId);
	// need to killio
	
	if(ValidIONum(m_longinfo2_entity) && m_longinfo2_entity != PlayerEntityHandle) {
		
		if(entities[m_longinfo2_entity]->scriptload
		   && (entities[m_longinfo2_entity]->ioflags & IO_NOSAVE)) {
			
			AddRandomSmoke(entities[m_longinfo2_entity], 100);
			Vec3f posi = entities[m_longinfo2_entity]->pos;
			posi.y -= 100.f;
			MakeCoolFx(posi);
		
			LightHandle nn = GetFreeDynLight();
			if(lightHandleIsValid(nn)) {
				EERIE_LIGHT * light = lightHandleGet(nn);
				
				light->intensity = 0.7f + 2.f * rnd();
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.0f);
				light->pos = posi;
				light->duration = 600;
			}
			
			entities[m_longinfo2_entity]->destroyOne();
		}
	}
	
	// TODO is this correct ?
	m_longinfo2_entity = PlayerEntityHandle;
}

void SummonCreatureSpell::Update(float timeDelta)
{
	if(!arxtime.is_paused()) {
		if(float(arxtime) - (float)m_timcreation <= 4000) {
			if(rnd() > 0.7f) {
				CSummonCreature * pSummon = (CSummonCreature *)m_pSpellFx;
				if(pSummon) {
					Vec3f pos = pSummon->eSrc;
					MakeCoolFx(pos);
				}
			}

			CSpellFx *pCSpellFX = m_pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(timeDelta);
				pCSpellFX->Render();
			}	

			m_longinfo_summon_creature = 1;
			m_longinfo2_entity = EntityHandle::Invalid;

		} else if(m_longinfo_summon_creature) {
			lightHandleDestroy(m_pSpellFx->lLightId);

			m_longinfo_summon_creature = 0;
			ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
			CSummonCreature *pSummon;
			pSummon= (CSummonCreature *)m_pSpellFx;

			if(pSummon) {
				Cylinder phys;
				phys.height = -200;
				phys.radius = 50;
				phys.origin = m_targetPos;
				float anything = CheckAnythingInCylinder(phys, NULL, CFLAG_JUST_TEST);

				if(glm::abs(anything) < 30) {
				
				long tokeep;
				res::path cls;
				if(m_megaCheat) {
					if(rnd() > 0.5) {
						tokeep = -1;
						cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
					} else {
						tokeep = 0;
						cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
					}
				} else if(rnd() > 0.997f || (sp_max && rnd() > 0.8f)
				   || (cur_mr >= 3 && rnd() > 0.3f)) {
					tokeep = 0;
					cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
				} else if(rnd() > 0.997f || (cur_rf >= 3 && rnd() > 0.8f)
				   || (cur_mr >= 3 && rnd() > 0.3f)) {
					tokeep = -1;
					cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
				} else if(m_level >= 9) {
					tokeep = 1;
					cls = "graph/obj3d/interactive/npc/demon/demon";
				} else if(rnd() > 0.98f) {
					tokeep = -1;
					cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
				} else {
					tokeep = 0;
					cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
				}
				
				Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
				if(!io) {
					cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
					tokeep = 0;
					io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
				}
				
				if(io) {
					RestoreInitialIOStatusOfIO(io);
					
					long lSpellsCaster = m_caster ; 
					io->summoner = checked_range_cast<short>(lSpellsCaster);

					io->scriptload = 1;
					
					if(tokeep == 1) {
						io->ioflags |= IO_NOSAVE;
					}
					
					io->pos = phys.origin;
					SendInitScriptEvent(io);

					if(tokeep < 0) {
						io->scale=1.65f;
						io->physics.cyl.radius=25;
						io->physics.cyl.height=-43;
						io->speed_modif=1.f;
					}

					if(ValidIONum(m_caster)) {
						EVENT_SENDER = entities[m_caster];
					} else {
						EVENT_SENDER = NULL;
					}

					SendIOScriptEvent(io,SM_SUMMONED);
					
					for(long j = 0; j < 3; j++) {
						Vec3f pos = pSummon->eSrc;
						pos += Vec3f(rnd(), rnd(), rnd()) * 100.f;
						pos += Vec3f(-50.f, 50.f, -50.f);
						
						MakeCoolFx(pos);
					}

					if(tokeep==1)
						m_longinfo2_entity = io->index();
					else
						m_longinfo2_entity = EntityHandle::Invalid;
				}
				}
			}
		} else if(m_longinfo2_entity <= PlayerEntityHandle) {
			m_duration = 0;
		}
	}	
}

bool FakeSummonSpell::CanLaunch()
{
	if(m_caster <= PlayerEntityHandle || !ValidIONum(m_target)) {
		return false;
	}
	
	return true;
}

void FakeSummonSpell::Launch()
{
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.9f;
	m_duration = 4000;
	
	Vec3f target = entities[m_target]->pos;
	if(m_target != PlayerEntityHandle) {
		target.y += player.baseHeight();
	}
	m_targetPos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &m_targetPos);
	CSummonCreature * effect = new CSummonCreature();
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	m_pSpellFx = effect;
}

void FakeSummonSpell::End()
{
	ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
	
	lightHandleDestroy(m_pSpellFx->lLightId);
	
	lightHandleDestroy(m_pSpellFx->lLightId);
}

void FakeSummonSpell::Update(float timeDelta)
{
	if(!arxtime.is_paused()) {
		if(rnd() > 0.7f) {
			CSummonCreature * pSummon = (CSummonCreature *)m_pSpellFx;
			if(pSummon) {
				Vec3f pos = pSummon->eSrc;
				MakeCoolFx(pos);
			}
		}
	}
	CSpellFx *pCSpellFX = m_pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

void NegateMagicSpell::Launch()
{
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC, &entities[m_target]->pos);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.f;
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 1000000;
	
	Vec3f targetPos = getTargetPos(m_caster, m_target);
	
	CNegateMagic * effect = new CNegateMagic();
	effect->Create(targetPos);
	effect->SetDuration(m_duration);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
	
	LaunchAntiMagicField();
}

void NegateMagicSpell::Update(float timeDelta)
{
	LaunchAntiMagicField();

	CNegateMagic * effect = static_cast<CNegateMagic *>(m_pSpellFx);

	if(effect) {
		if(m_target == PlayerEntityHandle) {
			effect->SetPos(player.basePosition());
		} else {
			effect->SetPos(entities[m_target]->pos);
		}
		
		effect->Update(timeDelta);
		effect->Render();
	}	
}

void NegateMagicSpell::LaunchAntiMagicField() {
	
	if(!ValidIONum(m_target))
		return;
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = spells[SpellHandle(i)];
		
		if(!spell)
			continue;
		
		if(this == spell)
			continue;
		
		if(m_level < spell->m_level)
			continue;
		
		Vec3f pos;
		GetSpellPosition(&pos, spell);
		if(closerThan(pos, entities[m_target]->pos, 600.f)) {
			if(spell->m_type != SPELL_CREATE_FIELD) {
				spells.endSpell(spell);
			} else if(m_target == PlayerEntityHandle && spell->m_caster == PlayerEntityHandle) {
				spells.endSpell(spell);
			}
		}
	}
}

bool IncinerateSpell::CanLaunch()
{
	Entity * tio = entities[m_target];
	if((tio->ioflags & IO_NPC) && tio->_npcdata->lifePool.current <= 0.f) {
		return false;
	}
	
	return true;
}

void IncinerateSpell::Launch()
{
	Entity * tio = entities[m_target];
	
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE, &entities[m_target]->pos);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_LOOP, 
	                                       &entities[m_target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
	
	m_duration = 20000;
	
	tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
	tio->sfx_time = (unsigned long)(arxtime);
	
	m_targets.push_back(m_target);
}

void IncinerateSpell::End()
{
	m_targets.clear();
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
}

void IncinerateSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(ValidIONum(m_target)) {
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}	
}

void MassParalyseSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 10000;
	
	for(size_t ii = 0; ii < entities.size(); ii++) {
		const EntityHandle handle = EntityHandle(ii);
		Entity * tio = entities[handle];
		
		if(long(ii) == m_caster || !tio || !(tio->ioflags & IO_NPC)) {
			continue;
		}
		
		if(tio->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(tio->ioflags & IO_FREEZESCRIPT) {
			continue;
		}
		
		if(fartherThan(tio->pos, entities[m_caster]->pos, 500.f)) {
			continue;
		}
		
		tio->ioflags |= IO_FREEZESCRIPT;
		
		ARX_NPC_Kill_Spell_Launch(tio);
		m_targets.push_back(tio->index());
	}
}

void MassParalyseSpell::End()
{
	
	std::vector<EntityHandle>::const_iterator itr;
	for(itr = m_targets.begin(); itr != m_targets.end(); ++itr) {
		EntityHandle handle = *itr;
		
		if(ValidIONum(handle)) {
			entities[handle]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}
	
	m_targets.clear();
	
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

