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
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells05.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RuneOfGuardingSpell::Launch(long i, SpellType typ, long duration)
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

void RuneOfGuardingSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].pSpellFx) {
		spells[i].pSpellFx->Update(timeDelta);
		spells[i].pSpellFx->Render();
		CRuneOfGuarding * pCRG=(CRuneOfGuarding *)spells[i].pSpellFx;

		if (pCRG)
		{
			EERIE_SPHERE sphere;
			sphere.origin = pCRG->eSrc;
			sphere.radius=std::max(spells[i].caster_level*15.f,50.f);

			if (CheckAnythingInSphere(&sphere,spells[i].caster,CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL| CAS_NO_FIX_COL | CAS_NO_DEAD_COL))
			{
				ARX_BOOMS_Add(pCRG->eSrc);
				LaunchFireballBoom(&pCRG->eSrc,(float)spells[i].caster_level);
				DoSphericDamage(&pCRG->eSrc,4.f*spells[i].caster_level,30.f*spells[i].caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,spells[i].caster);
				spells[i].tolive=0;
				ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING_END, &sphere.origin);
			}
		}
	}
}

void LevitateSpell::Launch(long duration, long i, SpellType typ)
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

void LevitateSpell::End(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_END, &entities[spells[i].target]->pos);
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	
	if(spells[i].target == 0)
		player.levitate = false;
}

void LevitateSpell::Update(size_t i, float timeDelta)
{
	CLevitate *pLevitate=(CLevitate *)spells[i].pSpellFx;
	Vec3f target;

	if(spells[i].target == 0) {
		target.x=player.pos.x;
		target.y=player.pos.y+150.f;
		target.z=player.pos.z;
		player.levitate = true;
	} else {
		target.x = entities[spells[i].caster]->pos.x;
		target.y = entities[spells[i].caster]->pos.y;
		target.z = entities[spells[i].caster]->pos.z;
	}

	pLevitate->ChangePos(&target);
		
	CSpellFx *pCSpellFX = spells[i].pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
}

void CurePoisonSpell::Launch(long i)
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

void CurePoisonSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].pSpellFx) {
		spells[i].pSpellFx->Update(timeDelta);
		spells[i].pSpellFx->Render();
	}
}

void RepelUndeadSpell::Launch(long duration, long i)
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

void RepelUndeadSpell::Kill(long i)
{
	if(lightHandleIsValid(spells[i].longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo_light);
		
		light->duration = 200;
		light->time_creation = (unsigned long)(arxtime);
	}
	spells[i].longinfo_light = -1;
	
	ARX_SOUND_Stop(spells[i].snd_loop);
}

void RepelUndeadSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].pSpellFx) {
		spells[i].pSpellFx->Update(timeDelta);
		spells[i].pSpellFx->Render();

		if (spells[i].target == 0)
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
	}
}

void PoisonProjectileSpell::Launch(long i)
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

void PoisonProjectileSpell::Update(size_t i, float timeDelta)
{
	if(spells[i].pSpellFx) {
		spells[i].pSpellFx->Update(timeDelta);
		spells[i].pSpellFx->Render();
	}
}
