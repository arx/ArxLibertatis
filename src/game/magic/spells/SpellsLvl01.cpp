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

#include "game/magic/spells/SpellsLvl01.h"

#include "core/Application.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/Cheat.h"

#include "graphics/spells/Spells01.h"
#include "graphics/spells/Spells03.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Object.h"

bool MagicSightSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void MagicSightSpell::Launch()
{
	m_fManaCostPerSecond = 0.36f;
	m_hasDuration = true;
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 6000000l;
	
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &m_caster_pos);
	
	if(m_caster == PlayerEntityHandle) {
		player.m_improve = true;
		m_snd_loop = SND_SPELL_VISION_LOOP;
		ARX_SOUND_PlaySFX(m_snd_loop, &m_caster_pos, 1.f, ARX_SOUND_PLAY_LOOPED);
	}
}

void MagicSightSpell::End()
{
	if(m_caster == PlayerEntityHandle) {
		player.m_improve = false;
		ARX_SOUND_Stop(m_snd_loop);
	}
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &entities[m_caster]->pos);
}

static const float DEC_FOCAL = 50.0f;
static const float IMPROVED_FOCAL = 320.0f;

extern EERIE_CAMERA subj;

void MagicSightSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(m_caster == PlayerEntityHandle) {
		Vec3f pos = ARX_PLAYER_FrontPos();
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
		
		if(subj.focal > IMPROVED_FOCAL)
			subj.focal -= DEC_FOCAL;
	}	
}

void MagicMissileSpell::Launch()
{
	m_duration = 20000; // TODO probably never read
	
	long number;
	if(sp_max || cur_rf == 3) {
		number = long(m_level);
	} else {
		number = glm::clamp(long(m_level + 1) / 2, 1l, 5l);
	}
	
	bool mrCheat = (m_caster == PlayerEntityHandle && cur_mr == 3);
	
	CMultiMagicMissile * effect = new CMultiMagicMissile(number, mrCheat);
	effect->SetDuration(6000ul);
	
	{
	m_hand_group = GetActionPointIdx(entities[m_caster]->obj, "primary_attach");
	
	if(m_hand_group != -1) {
		Entity * caster = entities[m_caster];
		long group = m_hand_group;
		m_hand_pos = caster->obj->vertexlist3[group].v;
	}
	
	Vec3f aePos;
	float afAlpha, afBeta;
	if(m_caster == PlayerEntityHandle) {
		afBeta = player.angle.getPitch();
		afAlpha = player.angle.getYaw();
		
		Vec3f vector = angleToVector(Anglef(afAlpha, afBeta, 0.f)) * 60.f;
		
		if(m_hand_group != -1) {
			aePos = m_hand_pos;
		} else {
			aePos = player.pos;
			aePos += angleToVectorXZ(afBeta);
		}
		
		aePos += vector;
		
	} else {
		afAlpha = 0;
		afBeta = entities[m_caster]->angle.getPitch();
		
		Vec3f vector = angleToVector(Anglef(afAlpha, afBeta, 0.f)) * 60.f;
		
		if(m_hand_group != -1) {
			aePos = m_hand_pos;
		} else {
			aePos = entities[m_caster]->pos;
		}
		
		aePos += vector;
		
		Entity * io = entities[m_caster];
		
		if(ValidIONum(io->targetinfo)) {
			const Vec3f & p1 = m_caster_pos;
			const Vec3f & p2 = entities[io->targetinfo]->pos;
			afAlpha = -(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1.x, p1.z))))); //alpha entre orgn et dest;
		} else if (ValidIONum(m_target)) {
			const Vec3f & p1 = m_caster_pos;
			const Vec3f & p2 = entities[m_target]->pos;
			afAlpha = -(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1.x, p1.z))))); //alpha entre orgn et dest;
		}
	}
	
	effect->Create(aePos, afAlpha, afBeta);
	}
	
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
}

void MagicMissileSpell::End()
{
}

void MagicMissileSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		CMultiMagicMissile *pMMM = (CMultiMagicMissile *) pCSpellFX;
		pMMM->CheckCollision(m_level, m_caster);

		// Update
		pCSpellFX->Update(timeDelta);

		if(pMMM->CheckAllDestroyed())
			m_duration = 0;

		pCSpellFX->Render();
	}
}

IgnitSpell::IgnitSpell()
	: m_srcPos(Vec3f_ZERO)
	, m_elapsed(0)
{
	
}

void IgnitSpell::Launch()
{
	m_duration = 500;
	
	if(m_hand_group != -1) {
		m_srcPos = m_hand_pos;
	} else {
		m_srcPos = m_caster_pos - Vec3f(0.f, 50.f, 0.f);
	}
	
	LightHandle id = GetFreeDynLight();
	if(lightHandleIsValid(id)) {
		EERIE_LIGHT * light = lightHandleGet(id);
		
		light->intensity = 1.8f;
		light->fallend   = 450.f;
		light->fallstart = 380.f;
		light->rgb       = Color3f(1.f, 0.75f, 0.5f);
		light->pos       = m_srcPos;
		light->duration  = 300;
	}
	
	float fPerimeter = 400.f + m_level * 30.f;
	
	m_lights.clear();
	m_elapsed = 0;
	
	CheckForIgnition(m_srcPos, fPerimeter, 1, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		EERIE_LIGHT * light = GLight[ii];
		
		if(!light || !(light->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(m_caster == PlayerEntityHandle && (light->extras & EXTRAS_NO_IGNIT)) {
			continue;
		}
		
		if(!(light->extras & EXTRAS_SEMIDYNAMIC)
		  && !(light->extras & EXTRAS_SPAWNFIRE)
		  && !(light->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(light->m_ignitionStatus) {
			continue;
		}
		
		if(!fartherThan(m_srcPos, light->pos, fPerimeter)) {
			
			T_LINKLIGHTTOFX entry;
			
			entry.iLightNum = ii;
			entry.poslight = light->pos;
		
			entry.idl = GetFreeDynLight();
		
			if(lightHandleIsValid(entry.idl)) {
				EERIE_LIGHT * light = lightHandleGet(entry.idl);
				
				light->intensity = 0.7f + 2.f * rnd();
				light->fallend = 400.f;
				light->fallstart = 300.f;
				light->rgb = Color3f(1.f, 1.f, 1.f);
				light->pos = entry.poslight;
			}
		
			m_lights.push_back(entry);
		}
	}
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell) {
			continue;
		}
		if(spell->m_type == SPELL_FIREBALL) {
			CSpellFx * pCSpellFX = spell->m_pSpellFx;
			if(pCSpellFX) {
				CFireBall * pCF = (CFireBall *)pCSpellFX;
				float radius = std::max(m_level * 2.f, 12.f);
				if(closerThan(m_srcPos, pCF->eCurPos,
				              fPerimeter + radius)) {
					spell->m_level += 1;
				}
			}
		}
	}
}

void IgnitSpell::End() {
	
	std::vector<T_LINKLIGHTTOFX>::iterator itr;
	for(itr = m_lights.begin(); itr != m_lights.end(); ++itr) {
		GLight[itr->iLightNum]->m_ignitionStatus = true;
		ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &itr->poslight);
		lightHandleDestroy(itr->idl);
	}
	
	m_lights.clear();
}

void IgnitSpell::Update(float timeDelta)
{
	if(m_elapsed < m_duration) {
		float a = (((float)m_elapsed)) / ((float)m_duration);
		
		if(a >= 1.f)
			a = 1.f;
		
		std::vector<T_LINKLIGHTTOFX>::iterator itr;
		for(itr = m_lights.begin(); itr != m_lights.end(); ++itr) {
			Vec3f pos = glm::mix(m_srcPos, itr->poslight, a);
			
				LightHandle id = itr->idl;
				
				if(lightHandleIsValid(id)) {
					EERIE_LIGHT * light = lightHandleGet(id);
					
					light->intensity = 0.7f + 2.f * rnd();
					light->pos = pos;
				}
		}
	}
	
	if(!arxtime.is_paused())
		m_elapsed += timeDelta;
}

void DouseSpell::Launch()
{
	m_duration = 500;
	
	Vec3f target;
	if(m_hand_group >= 0) {
		target = m_hand_pos;
	} else {
		target = m_caster_pos;
		target.y -= 50.f;
	}
	
	float fPerimeter = 400.f + m_level * 30.f;
	
	CheckForIgnition(target, fPerimeter, 0, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		EERIE_LIGHT * light = GLight[ii];
		
		if(!light || !(light->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(!(light->extras & EXTRAS_SEMIDYNAMIC)
		  && !(light->extras & EXTRAS_SPAWNFIRE)
		  && !(light->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(!light->m_ignitionStatus) {
			continue;
		}
		
		if(!fartherThan(target, light->pos, fPerimeter)) {
			T_LINKLIGHTTOFX entry;
			
			entry.iLightNum = ii;
			entry.poslight = light->pos;
			m_lights.push_back(entry);
		}
	}
	
	if(player.torch && closerThan(target, player.pos, fPerimeter)) {
		ARX_PLAYER_ClickedOnTorch(player.torch);
	}
	
	for(size_t k = 0; k < MAX_SPELLS; k++) {
		SpellBase * spell = spells[SpellHandle(k)];
		
		if(!spell) {
			continue;
		}
		
		switch(spell->m_type) {
			
			case SPELL_FIREBALL: {
				Vec3f pos = spell->getPosition();
				float radius = std::max(m_level * 2.f, 12.f);
				if(closerThan(target, pos, fPerimeter + radius)) {
					spell->m_level -= m_level;
					if(spell->m_level < 1) {
						spells.endSpell(spell);
					}
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				Vec3f pos = spell->getPosition();
				if(closerThan(target, pos, fPerimeter + 200)) {
					spell->m_level -= m_level;
					if(spell->m_level < 1) {
						spells.endSpell(spell);
					}
				}
				break;
			}
			
			default: break;
		}
	}
}

void DouseSpell::End() {
	
	std::vector<T_LINKLIGHTTOFX>::const_iterator itr;
	for(itr = m_lights.begin(); itr != m_lights.end(); ++itr) {
		GLight[itr->iLightNum]->m_ignitionStatus = false;
		ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &itr->poslight);
	}
}

void DouseSpell::Update(float timeDelta) {
	ARX_UNUSED(timeDelta);
}

void ActivatePortalSpell::Launch()
{
	ARX_SOUND_PlayInterface(SND_SPELL_ACTIVATE_PORTAL);
	
	m_duration = 20;
}
