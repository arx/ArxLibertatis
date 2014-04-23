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

void BlessSpell::Launch(long i, long duration)
{
	if(spells[i].m_caster == 0) {
		spells[i].m_target = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_BLESS, spells[i].m_target);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
	spells[i].m_exist = true;
	// TODO this tolive value is probably never read
	spells[i].m_tolive = (duration > -1) ? duration : 2000000;
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 0.5f * spells[i].m_caster_level * 0.6666f;
	
	CBless * effect = new CBless();
	effect->spellinstance = i;
	Vec3f target = entities[spells[i].m_caster]->pos;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(20000);
	spells[i].m_pSpellFx = effect;
	spells[i].m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].m_target, i);
}

void BlessSpell::End(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].m_target,i);
}

void BlessSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].m_pSpellFx) {
		CBless * pBless=(CBless *)spells[i].m_pSpellFx;

		if(pBless) {
			if(ValidIONum(spells[i].m_target)) {
				pBless->eSrc = entities[spells[i].m_target]->pos;
				Anglef angle = Anglef::ZERO;

				if(spells[i].m_target == 0)
					angle.setPitch(player.angle.getPitch());
				else 
					angle.setPitch(entities[spells[i].m_target]->angle.getPitch());

				pBless->Set_Angle(angle);
			}
		}

		spells[i].m_pSpellFx->Update(timeDelta);
		spells[i].m_pSpellFx->Render();
	}
}

void DispellFieldSpell::Launch(long i)
{
	spells[i].m_tolive = 10;
	
	long valid = 0, dispelled = 0;

	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].m_exist || !spells[n].m_pSpellFx) {
			continue;
		}
		
		bool cancel = false;
		Vec3f pos;
		
		switch(spells[n].m_type) {
			
			case SPELL_CREATE_FIELD: {
				if(spells[i].m_caster != 0 || spells[n].m_caster == 0) {
					pos = static_cast<CCreateField *>(spells[n].m_pSpellFx)->eSrc;
					cancel = true;
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				pos = static_cast<CFireField *>(spells[n].m_pSpellFx)->pos;
				cancel = true;
				break;
			}
			
			case SPELL_ICE_FIELD: {
				pos = static_cast<CIceField *>(spells[n].m_pSpellFx)->eSrc;
				cancel = true;
				break;
			}
			
			default: break;
		}
		
		Entity * caster = entities[spells[i].m_caster];
		if(cancel && closerThan(pos, caster->pos, 400.f)) {
			valid++;
			if(spells[n].m_caster_level <= spells[i].m_caster_level) {
				spells[n].m_tolive = 0;
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
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[i].m_caster_pos);
	}
}

void FireProtectionSpell::Launch(long i, long duration)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].m_target], SPELL_FIRE_PROTECTION);
	if(idx >= 0) {
		spells[idx].m_tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	if(duration > -1) {
		spells[i].m_tolive = duration;
	} else {
		spells[i].m_tolive = (spells[i].m_caster == 0) ? 2000000 : 20000;
	}
	
	if(spells[i].m_caster == 0) {
		spells[i].m_target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION, &entities[spells[i].m_target]->pos);
	
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 1.f;
	
	CFireProtection * effect = new CFireProtection();
	effect->spellinstance = i;
	effect->Create(spells[i].m_tolive);
	spells[i].m_pSpellFx = effect;
	spells[i].m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].m_target, i);
	
	spells[i].m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_LOOP, 
	                                       &entities[spells[i].m_target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
}

void FireProtectionSpell::End(size_t i)
{
	ARX_SOUND_Stop(spells[i].m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_END, &entities[spells[i].m_target]->pos);
	ARX_SPELLS_RemoveSpellOn(spells[i].m_target, i);
	
	if(ValidIONum(spells[i].m_target))
		ARX_HALO_SetToNative(entities[spells[i].m_target]);
}

void FireProtectionSpell::Update(size_t i, float timeDelta)
{
	spells[i].m_pSpellFx->Update(timeDelta);
	spells[i].m_pSpellFx->Render();
	ARX_SOUND_RefreshPosition(spells[i].m_snd_loop, entities[spells[i].m_target]->pos);
}

void ColdProtectionSpell::Launch(long i, long duration)
{
	long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].m_target], SPELL_COLD_PROTECTION);
	if(idx >= 0) {
		spells[idx].m_tolive = 0;
	}
	
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION, spells[i].m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(spells[i].m_caster == 0) {
		spells[i].m_target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START, &entities[spells[i].m_target]->pos);
	
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	if(duration > -1) {
		spells[i].m_tolive = duration;
	} else {
		spells[i].m_tolive = (spells[i].m_caster == 0) ? 2000000 : 20000;
	}
	
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 1.f;
	
	CColdProtection * effect = new CColdProtection();
	effect->spellinstance=i;
	effect->Create(spells[i].m_tolive, spells[i].m_target);
	spells[i].m_pSpellFx = effect;
	spells[i].m_tolive = effect->GetDuration();
	
	spells[i].m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP,
	                                       &entities[spells[i].m_target]->pos, 1.f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	ARX_SPELLS_AddSpellOn(spells[i].m_target, i);
}

void ColdProtectionSpell::End(size_t i)
{
	ARX_SOUND_Stop(spells[i].m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END, &entities[spells[i].m_target]->pos);
	ARX_SPELLS_RemoveSpellOn(spells[i].m_target, i);
	
	if(ValidIONum(spells[i].m_target))
		ARX_HALO_SetToNative(entities[spells[i].m_target]);
}

void ColdProtectionSpell::Update(size_t i, float timeDelta)
{
	spells[i].m_pSpellFx->Update(timeDelta);
	spells[i].m_pSpellFx->Render();
	ARX_SOUND_RefreshPosition(spells[i].m_snd_loop, entities[spells[i].m_target]->pos);
}

void TelekinesisSpell::Launch(long i, long duration)
{
	spells[i].m_exist = true;
	spells[i].m_tolive = (duration > -1) ? duration : 6000000;
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 0.9f;
	
	if(spells[i].m_caster == 0) {
		Project.telekinesis = 1;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &spells[i].m_caster_pos);
}

void TelekinesisSpell::End(size_t i)
{
	if(spells[i].m_caster == 0)
		Project.telekinesis = 0;
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[spells[i].m_caster]->pos);
}

void CurseSpell::Launch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_CURSE, spells[i].m_target);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &entities[spells[i].m_target]->pos);
	
	spells[i].m_exist = true;
	spells[i].m_tolive = (duration > -1) ? duration : 2000000;
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 0.5f * spells[i].m_caster_level;
	
	CCurse * effect = new CCurse();
	effect->spellinstance = i;
	
	Vec3f target = spells[i].m_target_pos;
	if(spells[i].m_target == 0) {
		target.y -= 200.f;
	} else if(spells[i].m_target > 0 && entities[spells[i].m_target]) {
		target.y += entities[spells[i].m_target]->physics.cyl.height - 50.f;
	}
	
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(spells[i].m_tolive);
	spells[i].m_pSpellFx = effect;
	spells[i].m_tolive = effect->GetDuration();
	
	ARX_SPELLS_AddSpellOn(spells[i].m_target, i);
}

void CurseSpell::End(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].m_target,i);
}

void CurseSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].m_pSpellFx) {
		CCurse * curse=(CCurse *)spells[i].m_pSpellFx;
		Vec3f target = Vec3f_ZERO;
			
		if(spells[i].m_target >= 0 && entities[spells[i].m_target]) {
			target = entities[spells[i].m_target]->pos;

			if(spells[i].m_target == 0)
				target.y -= 200.f;
			else
				target.y += entities[spells[i].m_target]->physics.cyl.height - 30.f;
		}
		
		curse->Update(checked_range_cast<unsigned long>(timeDelta));
		
		curse->eTarget = target;
		curse->Render();
	}
}
