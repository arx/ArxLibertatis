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

#include "game/magic/spells/SpellsLvl05.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/spells/Spells05.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RuneOfGuardingSpellLaunch(long i, SpellType typ, long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING);
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 99999999;
	
	CRuneOfGuarding * effect = new CRuneOfGuarding();
	effect->spellinstance = i;
	effect->Create(entities[spells[i].caster]->pos, 0);
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void LevitateSpellLaunch(long duration, long i, SpellType typ)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_START, &entities[spells[i].target]->pos);
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 2000000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 1.f;
	
	CLevitate * effect = new CLevitate();
	effect->spellinstance = i;
	
	Vec3f target;
	if(spells[i].target == 0) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		spells[i].tolive = 200000000;
		player.levitate = true;
	} else {
		target = entities[spells[i].target]->pos;
	}
	
	effect->Create(16, 50.f, 100.f, 80.f, &target, spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
	
	spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_LOOP,
	                                       &entities[spells[i].target]->pos, 0.7f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	ARX_SPELLS_AddSpellOn(spells[i].target, i);
}

void CurePoisonSpellLaunch(long i)
{
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}
	
	float cure = spells[i].caster_level * 10;
	if(spells[i].target == 0) {
		player.poison -= std::min(player.poison, cure);
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON);
	} else if (ValidIONum(spells[i].target)) {
		Entity * io = entities[spells[i].target];
		if(io->ioflags & IO_NPC) {
			io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
		}
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON, &io->pos);
	}
	
	spells[i].exist = true;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].tolive = 3500;
	
	CCurePoison * effect = new CCurePoison();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void RepelUndeadSpellLaunch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_REPEL_UNDEAD, spells[i].caster);
	if(iCancel > -1) {
		spells[iCancel].tolive = 0;
	}
	
	if(spells[i].caster == 0) {
		spells[i].target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD, &entities[spells[i].target]->pos);
	if(spells[i].target == 0) {
		spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD_LOOP,
		                                       &entities[spells[i].target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	spells[i].exist = true;
	spells[i].tolive = (duration > -1) ? duration : 20000000;
	spells[i].bDuration = true;
	spells[i].fManaCostPerSecond = 1.f;
	
	CRepelUndead * effect = new CRepelUndead();
	effect->spellinstance = i;
	effect->Create(player.pos, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(spells[i].tolive);
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void PoisonProjectileSpellLaunch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_POISON_PROJECTILE_LAUNCH,
	                  &spells[i].caster_pos);
	
	spells[i].exist = true;
	spells[i].tolive = 900000000; // TODO probably never read
	
	long level = std::max(long(spells[i].caster_level), 1l);
	CMultiPoisonProjectile * effect = new CMultiPoisonProjectile(level);
	effect->spellinstance = i;
	effect->SetDuration(8000ul);
	float ang;
	if(spells[i].caster == 0) {
		ang = player.angle.getPitch();
	} else {
		ang = entities[spells[i].caster]->angle.getPitch();
	}
	effect->Create(Vec3f_ZERO, MAKEANGLE(ang));
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}
