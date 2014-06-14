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
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/Cheat.h"

#include "graphics/spells/Spells01.h"
#include "graphics/spells/Spells03.h"

#include "scene/GameSound.h"

bool MagicSightSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void MagicSightSpell::Launch()
{
	m_fManaCostPerSecond = 0.36f;
	m_bDuration = true;
	m_tolive = (m_launchDuration > -1) ? m_launchDuration : 6000000l;
	
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &m_caster_pos);
	
	if(m_caster == PlayerEntityHandle) {
		Project.improve = 1;
		m_snd_loop = SND_SPELL_VISION_LOOP;
		ARX_SOUND_PlaySFX(m_snd_loop, &m_caster_pos, 1.f, ARX_SOUND_PLAY_LOOPED);
	}
}

void MagicSightSpell::End()
{
	if(m_caster == PlayerEntityHandle) {
		Project.improve = 0;
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
		Vec3f pos;
		ARX_PLAYER_FrontPos(&pos);
		ARX_SOUND_RefreshPosition(m_snd_loop, pos);
		
		if(subj.focal > IMPROVED_FOCAL)
			subj.focal -= DEC_FOCAL;
	}	
}

void MagicMissileSpell::Launch()
{
	m_tolive = 20000; // TODO probably never read
	
	long number;
	if(sp_max || cur_rf == 3) {
		number = long(m_level);
	} else {
		number = clamp(long(m_level + 1) / 2, 1l, 5l);
	}
	
	CMultiMagicMissile * effect = new CMultiMagicMissile(number, m_thisHandle);
	effect->SetDuration(6000ul);
	effect->Create();
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void MagicMissileSpell::End()
{
	lightHandleDestroy(m_longinfo_light);
}

void MagicMissileSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		CMultiMagicMissile *pMMM = (CMultiMagicMissile *) pCSpellFX;
		pMMM->CheckCollision();

		// Update
		pCSpellFX->Update(timeDelta);

		if(pMMM->CheckAllDestroyed())
			m_tolive = 0;

		pCSpellFX->Render();
	}
}

void IgnitSpell::Launch()
{
	m_tolive = 500;
	
	CIgnit * effect = new CIgnit();
	effect->spellinstance = m_thisHandle;
	
	Vec3f target;
	if(m_hand_group != -1) {
		target = m_hand_pos;
	} else {
		target = m_caster_pos - Vec3f(0.f, 50.f, 0.f);
	}
	
	LightHandle id = GetFreeDynLight();
	if(lightHandleIsValid(id)) {
		EERIE_LIGHT * light = lightHandleGet(id);
		
		light->intensity = 1.8f;
		light->fallend   = 450.f;
		light->fallstart = 380.f;
		light->rgb       = Color3f(1.f, 0.75f, 0.5f);
		light->pos       = target;
		light->duration  = 300;
	}
	
	float fPerimeter = 400.f + m_level * 30.f;
	
	effect->Create(&target, m_tolive);
	CheckForIgnition(target, fPerimeter, 1, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		
		if(!GLight[ii] || !(GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(m_caster == PlayerEntityHandle && (GLight[ii]->extras & EXTRAS_NO_IGNIT)) {
			continue;
		}
		
		if(!(GLight[ii]->extras & EXTRAS_SEMIDYNAMIC)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNFIRE)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(GLight[ii]->status) {
			continue;
		}
		
		if(!fartherThan(target, GLight[ii]->pos, fPerimeter)) {
			effect->AddLight(ii);
		}
	}
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell->m_exist) {
			continue;
		}
		if(spell->m_type == SPELL_FIREBALL) {
			CSpellFx * pCSpellFX = spell->m_pSpellFx;
			if(pCSpellFX) {
				CFireBall * pCF = (CFireBall *)pCSpellFX;
				float radius = std::max(m_level * 2.f, 12.f);
				if(closerThan(target, pCF->eCurPos,
				              fPerimeter + radius)) {
					spell->m_level += 1;
				}
			}
		}
	}
	
	m_pSpellFx = effect;
}

void IgnitSpell::End()
{
	CIgnit *pIgnit = (CIgnit *)m_pSpellFx;
	pIgnit->Action(true);
}

void IgnitSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
}

void DouseSpell::Launch()
{
	m_tolive = 500;
	
	CDoze * effect = new CDoze();
	effect->spellinstance = m_thisHandle;
	
	Vec3f target;
	if(m_hand_group >= 0) {
		target = m_hand_pos;
	} else {
		target = m_caster_pos;
		target.y -= 50.f;
	}
	
	float fPerimeter = 400.f + m_level * 30.f;
	effect->CreateDoze(&target, m_tolive);
	CheckForIgnition(target, fPerimeter, 0, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		
		if(!GLight[ii] || !(GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(!(GLight[ii]->extras & EXTRAS_SEMIDYNAMIC)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNFIRE)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(!GLight[ii]->status) {
			continue;
		}
		
		if(!fartherThan(target, GLight[ii]->pos, fPerimeter)) {
			effect->AddLightDoze(ii);	
		}
	}
	
	if(player.torch && closerThan(target, player.pos, fPerimeter)) {
		ARX_PLAYER_ClickedOnTorch(player.torch);
	}
	
	for(size_t k = 0; k < MAX_SPELLS; k++) {
		SpellBase * spell = spells[SpellHandle(k)];
		
		if(!spell->m_exist) {
			continue;
		}
		
		switch(spell->m_type) {
			
			case SPELL_FIREBALL: {
				CSpellFx * pCSpellFX = spell->m_pSpellFx;
				if(pCSpellFX) {
					CFireBall * pCF = (CFireBall *)pCSpellFX;
					float radius = std::max(m_level * 2.f, 12.f);
					if(closerThan(target, pCF->eCurPos,
					              fPerimeter + radius)) {
						spell->m_level -= m_level;
						if(spell->m_level < 1) {
							spell->m_tolive = 0;
						}
					}
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				Vec3f pos;
				if(GetSpellPosition(&pos, spell)) {
					if(closerThan(target, pos, fPerimeter + 200)) {
						spell->m_level -= m_level;
						if(spell->m_level < 1) {
							spell->m_tolive=0;
						}
					}
				}
				break;
			}
			
			default: break;
		}
	}
	
	m_pSpellFx = effect;
}

void DouseSpell::End()
{
	CDoze *pDoze = (CDoze *)m_pSpellFx;
	pDoze->Action(false);
}

void DouseSpell::Update(float timeDelta)
{
	CSpellFx *pCSpellFX = m_pSpellFx;
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
}

void ActivatePortalSpell::Launch()
{
	ARX_SOUND_PlayInterface(SND_SPELL_ACTIVATE_PORTAL);
	
	m_tolive = 20;
}
