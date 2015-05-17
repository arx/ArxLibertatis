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

#include "game/magic/spells/SpellsLvl04.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "gui/Speech.h"
#include "graphics/spells/Spells04.h"
#include "graphics/spells/Spells06.h"
#include "graphics/spells/Spells07.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool BlessSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void BlessSpell::Launch()
{
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	spells.endByCaster(m_target, SPELL_BLESS);
	
	ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
	
	// TODO this tolive value is probably never read
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.3333f * m_level;
	
	CBless * effect = new CBless();
	Vec3f target = entities[m_caster]->pos;
	effect->Create(target);
	effect->SetDuration(20000);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
	
	m_targets.push_back(m_target);
}

void BlessSpell::End()
{
	m_targets.clear();
}

void BlessSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		CBless * pBless=(CBless *)m_pSpellFx;

		if(pBless) {
			Anglef angle = Anglef::ZERO;
			
			if(ValidIONum(m_target)) {
				pBless->eSrc = entities[m_target]->pos;
				
				if(m_target == PlayerEntityHandle)
					angle.setPitch(player.angle.getPitch());
				else 
					angle.setPitch(entities[m_target]->angle.getPitch());
			}
			
			pBless->Set_Angle(angle);
			pBless->m_scale = (m_level + 10) * 6.f;
		}

		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}

Vec3f BlessSpell::getPosition() {
	return getTargetPosition();
}

void DispellFieldSpell::Launch()
{
	m_duration = 10;
	
	long valid = 0, dispelled = 0;

	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell || !spell->m_pSpellFx) {
			continue;
		}
		
		bool cancel = false;
		Vec3f pos;
		
		switch(spell->m_type) {
			
			case SPELL_CREATE_FIELD: {
				if(m_caster != PlayerEntityHandle || spell->m_caster == PlayerEntityHandle) {
					pos = static_cast<CCreateField *>(spell->m_pSpellFx)->eSrc;
					cancel = true;
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				pos = static_cast<CFireField *>(spell->m_pSpellFx)->pos;
				cancel = true;
				break;
			}
			
			case SPELL_ICE_FIELD: {
				pos = static_cast<CIceField *>(spell->m_pSpellFx)->eSrc;
				cancel = true;
				break;
			}
			
			default: break;
		}
		
		Entity * caster = entities[m_caster];
		if(cancel && closerThan(pos, caster->pos, 400.f)) {
			valid++;
			if(spell->m_level <= m_level) {
				spells.endSpell(spell);
				dispelled++;
			}
		}
	}
	
	if(valid > dispelled) {
		// Some fileds could not be dispelled
		ARX_SPEECH_AddSpeech(entities.player(), "player_not_skilled_enough",
		                     ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
	}
	
	if(dispelled > 0) {
		ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_FIELD);
	} else {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &m_caster_pos);
	}
}

void FireProtectionSpell::Launch()
{
	spells.endByTarget(m_target, SPELL_FIRE_PROTECTION);
	spells.endByCaster(m_caster, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_COLD_PROTECTION);
	
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	} else {
		m_duration = (m_caster == PlayerEntityHandle) ? 2000000 : 20000;
	}
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION, &entities[m_target]->pos);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.3f;
		io->halo.color.b = 0.f;
		io->halo.radius = 45.f;
	}
	
	m_targets.push_back(m_target);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_LOOP, 
	                                       &entities[m_target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireProtectionSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_END, &entities[m_target]->pos);
	m_targets.clear();
	
	if(ValidIONum(m_target))
		ARX_HALO_SetToNative(entities[m_target]);
}

void FireProtectionSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.3f;
		io->halo.color.b = 0.f;
		io->halo.radius = 45.f;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f FireProtectionSpell::getPosition() {
	return getTargetPosition();
}

void ColdProtectionSpell::Launch()
{
	spells.endByTarget(m_target, SPELL_COLD_PROTECTION);
	spells.endByCaster(m_caster, SPELL_ARMOR);
	spells.endByCaster(m_caster, SPELL_LOWER_ARMOR);
	spells.endByCaster(m_caster, SPELL_FIRE_PROTECTION);
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START, &entities[m_target]->pos);
	
	if(m_launchDuration > -1) {
		m_duration = m_launchDuration;
	} else {
		m_duration = (m_caster == PlayerEntityHandle) ? 2000000 : 20000;
	}
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP,
	                                       &entities[m_target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	m_targets.push_back(m_target);
}

void ColdProtectionSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END, &entities[m_target]->pos);
	m_targets.clear();
	
	if(ValidIONum(m_target))
		ARX_HALO_SetToNative(entities[m_target]);
}

void ColdProtectionSpell::Update(float timeDelta)
{
	ARX_UNUSED(timeDelta);
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

Vec3f ColdProtectionSpell::getPosition() {
	return getTargetPosition();
}

bool TelekinesisSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void TelekinesisSpell::Launch()
{
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 6000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.9f;
	
	if(m_caster == PlayerEntityHandle) {
		player.m_telekinesis = true;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &m_caster_pos);
}

void TelekinesisSpell::End()
{
	if(m_caster == PlayerEntityHandle)
		player.m_telekinesis = false;
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[m_caster]->pos);
}

void CurseSpell::Launch()
{
	spells.endByCaster(m_target, SPELL_CURSE);
	
	ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.5f * m_level;
	
	CCurse * effect = new CCurse();
	
	Vec3f target = getTargetPos(m_caster, m_target);
	if(m_target == PlayerEntityHandle) {
		target.y -= 200.f;
	} else if(m_target > 0 && entities[m_target]) {
		target.y += entities[m_target]->physics.cyl.height - 50.f;
	}
	
	effect->Create(target);
	effect->SetDuration(m_duration);
	m_pSpellFx = effect;
	m_duration = effect->GetDuration();
	
	m_targets.push_back(m_target);
}

void CurseSpell::End()
{
	m_targets.clear();
}

void CurseSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		CCurse * curse=(CCurse *)m_pSpellFx;
		Vec3f target = Vec3f_ZERO;
			
		if(m_target >= PlayerEntityHandle && entities[m_target]) {
			target = entities[m_target]->pos;

			if(m_target == PlayerEntityHandle)
				target.y -= 200.f;
			else
				target.y += entities[m_target]->physics.cyl.height - 30.f;
		}
		
		curse->Update(timeDelta);
		
		curse->eTarget = target;
		curse->Render();
	}
}

Vec3f CurseSpell::getPosition() {
	return getTargetPosition();
}
