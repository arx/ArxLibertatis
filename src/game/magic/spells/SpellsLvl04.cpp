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

void BlessSpell::Launch(SpellHandle i, long duration)
{
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_BLESS, m_target);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
	m_exist = true;
	// TODO this tolive value is probably never read
	m_tolive = (duration > -1) ? duration : 2000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 0.5f * m_caster_level * 0.6666f;
	
	CBless * effect = new CBless();
	effect->spellinstance = i;
	Vec3f target = entities[m_caster]->pos;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(20000);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(m_target, i);
}

void BlessSpell::End(SpellHandle i)
{
	ARX_SPELLS_RemoveSpellOn(m_target,i);
}

void BlessSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		CBless * pBless=(CBless *)m_pSpellFx;

		if(pBless) {
			if(ValidIONum(m_target)) {
				pBless->eSrc = entities[m_target]->pos;
				Anglef angle = Anglef::ZERO;

				if(m_target == PlayerEntityHandle)
					angle.setPitch(player.angle.getPitch());
				else 
					angle.setPitch(entities[m_target]->angle.getPitch());

				pBless->Set_Angle(angle);
			}
		}

		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}

void DispellFieldSpell::Launch()
{
	m_tolive = 10;
	
	long valid = 0, dispelled = 0;

	for(size_t n = 0; n < MAX_SPELLS; n++) {
		const SpellHandle handle = SpellHandle(n);
		SpellBase & spell = spells[handle];
		
		if(!spell.m_exist || !spell.m_pSpellFx) {
			continue;
		}
		
		bool cancel = false;
		Vec3f pos;
		
		switch(spell.m_type) {
			
			case SPELL_CREATE_FIELD: {
				if(m_caster != PlayerEntityHandle || spell.m_caster == PlayerEntityHandle) {
					pos = static_cast<CCreateField *>(spell.m_pSpellFx)->eSrc;
					cancel = true;
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				pos = static_cast<CFireField *>(spell.m_pSpellFx)->pos;
				cancel = true;
				break;
			}
			
			case SPELL_ICE_FIELD: {
				pos = static_cast<CIceField *>(spell.m_pSpellFx)->eSrc;
				cancel = true;
				break;
			}
			
			default: break;
		}
		
		Entity * caster = entities[m_caster];
		if(cancel && closerThan(pos, caster->pos, 400.f)) {
			valid++;
			if(spell.m_caster_level <= m_caster_level) {
				spell.m_tolive = 0;
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

void FireProtectionSpell::Launch(SpellHandle i, long duration)
{
	SpellHandle idx = ARX_SPELLS_GetSpellOn(entities[m_target], SPELL_FIRE_PROTECTION);
	if(idx != InvalidSpellHandle) {
		spells[idx].m_tolive = 0;
	}
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	if(duration > -1) {
		m_tolive = duration;
	} else {
		m_tolive = (m_caster == PlayerEntityHandle) ? 2000000 : 20000;
	}
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION, &entities[m_target]->pos);
	
	m_bDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.3f;
		io->halo.color.b = 0.f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
	
	ARX_SPELLS_AddSpellOn(m_target, i);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_LOOP, 
	                                       &entities[m_target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireProtectionSpell::End(SpellHandle i)
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_END, &entities[m_target]->pos);
	ARX_SPELLS_RemoveSpellOn(m_target, i);
	
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
		io->halo.dynlight = -1;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void ColdProtectionSpell::Launch(SpellHandle i, long duration)
{
	SpellHandle idx = ARX_SPELLS_GetSpellOn(entities[m_target], SPELL_COLD_PROTECTION);
	if(idx != InvalidSpellHandle) {
		spells[idx].m_tolive = 0;
	}
	
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, m_caster);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START, &entities[m_target]->pos);
	
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	if(duration > -1) {
		m_tolive = duration;
	} else {
		m_tolive = (m_caster == PlayerEntityHandle) ? 2000000 : 20000;
	}
	
	m_bDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP,
	                                       &entities[m_target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	ARX_SPELLS_AddSpellOn(m_target, i);
}

void ColdProtectionSpell::End(SpellHandle i)
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END, &entities[m_target]->pos);
	ARX_SPELLS_RemoveSpellOn(m_target, i);
	
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
		io->halo.dynlight = -1;
	}
	
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void TelekinesisSpell::Launch(long duration)
{
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 6000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 0.9f;
	
	if(m_caster == PlayerEntityHandle) {
		Project.telekinesis = 1;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &m_caster_pos);
}

void TelekinesisSpell::End()
{
	if(m_caster == PlayerEntityHandle)
		Project.telekinesis = 0;
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[m_caster]->pos);
}

void CurseSpell::Launch(long duration, SpellHandle i)
{
	SpellHandle iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_CURSE, m_target);
	if(iCancel != InvalidSpellHandle) {
		spells[iCancel].m_tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &entities[m_target]->pos);
	
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 2000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 0.5f * m_caster_level;
	
	CCurse * effect = new CCurse();
	
	Vec3f target = m_target_pos;
	if(m_target == PlayerEntityHandle) {
		target.y -= 200.f;
	} else if(m_target > 0 && entities[m_target]) {
		target.y += entities[m_target]->physics.cyl.height - 50.f;
	}
	
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(m_target, i);
}

void CurseSpell::End(SpellHandle i)
{
	ARX_SPELLS_RemoveSpellOn(m_target,i);
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
		
		curse->Update(checked_range_cast<unsigned long>(timeDelta));
		
		curse->eTarget = target;
		curse->Render();
	}
}
