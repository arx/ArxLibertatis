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

RuneOfGuardingSpell::~RuneOfGuardingSpell() {
	delete m_pSpellFx;
}

void RuneOfGuardingSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_RUNE_OF_GUARDING);
	
	ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 99999999;
	
	m_pos = entities[m_caster]->pos;
	
	m_pSpellFx = new CRuneOfGuarding();
	m_pSpellFx->Create(m_pos);
	m_pSpellFx->SetDuration(m_duration);
	m_duration = m_pSpellFx->GetDuration();
	
	
	m_light = GetFreeDynLight();
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->intensity = 0.7f + 2.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(1.0f, 0.2f, 0.2f);
		light->pos = m_pos - Vec3f(0.f, 50.f, 0.f);
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 200;
	}
}

void RuneOfGuardingSpell::End() {
	
	endLightDelayed(m_light, 500);
	
	delete m_pSpellFx;
	m_pSpellFx = NULL;
}

void RuneOfGuardingSpell::Update(float timeDelta) {
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		float fa = 1.0f - rnd() * 0.15f;
		light->intensity = 0.7f + 2.3f * fa;
		light->fallend = 350.f;
		light->fallstart = 150.f;
		light->rgb = Color3f(1.0f, 0.2f, 0.2f);
		light->time_creation = (unsigned long)(arxtime);
		light->duration = 200;
	}
	
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
	
	Sphere sphere = Sphere(m_pos, std::max(m_level * 15.f, 50.f));
	if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL| CAS_NO_FIX_COL | CAS_NO_DEAD_COL)) {
		ARX_BOOMS_Add(m_pos);
		LaunchFireballBoom(m_pos, (float)m_level);
		DoSphericDamage(m_pos, 4.f * m_level, 30.f * m_level, DAMAGE_AREA, DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL, m_caster);
		ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING_END, &m_pos);
		m_duration = 0;
	}
}

Vec3f RuneOfGuardingSpell::getPosition() {
	
	return m_pos;
}


LevitateSpell::~LevitateSpell() {
	
	delete m_pSpellFx;
}

void LevitateSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_LEVITATE);
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_START, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	Vec3f target;
	if(m_target == PlayerEntityHandle) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		m_duration = 200000000;
		player.levitate = true;
	} else {
		target = entities[m_target]->pos;
	}
	
	m_pSpellFx = new CLevitate();
	m_pSpellFx->Create(16, 50.f, 100.f, 80.f, &target, m_duration);
	m_duration = m_pSpellFx->GetDuration();
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_LOOP,
	                                       &entities[m_target]->pos, 0.7f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	m_targets.push_back(m_target);
}

void LevitateSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_END, &entities[m_target]->pos);
	m_targets.clear();
	
	if(m_target == PlayerEntityHandle)
		player.levitate = false;
	
	delete m_pSpellFx;
	m_pSpellFx = NULL;
}

void LevitateSpell::Update(float timeDelta)
{
	CLevitate *pLevitate=(CLevitate *)m_pSpellFx;
	Vec3f target;

	if(m_target == PlayerEntityHandle) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		player.levitate = true;
	} else {
		target = entities[m_caster]->pos;
	}

	pLevitate->ChangePos(&target);
		
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

CurePoisonSpell::~CurePoisonSpell() {
	
	delete m_pSpellFx;
}

void CurePoisonSpell::Launch()
{
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	float cure = m_level * 10;
	if(m_target == PlayerEntityHandle) {
		player.poison -= std::min(player.poison, cure);
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON);
	} else if (ValidIONum(m_target)) {
		Entity * io = entities[m_target];
		if(io->ioflags & IO_NPC) {
			io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
		}
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON, &io->pos);
	}
	
	m_duration = 3500;
	
	m_pSpellFx = new CCurePoison();
	m_pSpellFx->Create();
	m_pSpellFx->SetDuration(m_duration);
	m_duration = m_pSpellFx->GetDuration();
}

void CurePoisonSpell::End() {
	
	delete m_pSpellFx;
	m_pSpellFx = NULL;
}

void CurePoisonSpell::Update(float timeDelta)
{
	CCurePoison * effect = static_cast<CCurePoison *>(m_pSpellFx);
	
	if(effect) {
		Vec3f pos = entities[m_target]->pos;
		
		if(m_target == PlayerEntityHandle)
			pos.y += 200;
		
		effect->SetPosition(pos);
		effect->Update(timeDelta);
		effect->Render();
	}
}

RepelUndeadSpell::~RepelUndeadSpell() {
	
	delete m_pSpellFx;
}

void RepelUndeadSpell::Launch()
{
	spells.endByCaster(m_caster, SPELL_REPEL_UNDEAD);
	
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD, &entities[m_target]->pos);
	if(m_target == PlayerEntityHandle) {
		m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD_LOOP,
		                                       &entities[m_target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	m_duration = (m_launchDuration > -1) ? m_launchDuration : 20000000;
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	m_pSpellFx = new CRepelUndead();
	m_pSpellFx->Create(player.pos);
	m_pSpellFx->SetDuration(m_duration);
	m_duration = m_pSpellFx->GetDuration();
}

void RepelUndeadSpell::End() {
	ARX_SOUND_Stop(m_snd_loop);
	
	// All Levels - Kill Light
	if(m_pSpellFx) {
		endLightDelayed(m_pSpellFx->lLightId, 500);
	}
	
	delete m_pSpellFx;
	m_pSpellFx = NULL;
}

void RepelUndeadSpell::Update(float timeDelta)
{
	CRepelUndead * effect = static_cast<CRepelUndead *>(m_pSpellFx);
	
	if(effect) {
		Vec3f pos = entities[m_target]->pos;
		
		float rot;
		if(m_target == PlayerEntityHandle) {
			rot = player.angle.getPitch();
		} else {
			rot = entities[m_target]->angle.getPitch();
		}
		
		effect->SetPos(pos);
		effect->SetRotation(rot);
		effect->Update(timeDelta);
		effect->Render();

		if (m_target == PlayerEntityHandle)
			ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}
}

PoisonProjectileSpell::~PoisonProjectileSpell() {
	
	delete m_pSpellFx;
}

void PoisonProjectileSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_POISON_PROJECTILE_LAUNCH,
	                  &m_caster_pos);
	
	m_duration = 900000000; // TODO probably never read
	
	Vec3f srcPos = Vec3f_ZERO;
	float afBeta = 0.f;
	
	Entity * caster = entities[m_caster];
	m_hand_group = caster->obj->fastaccess.primary_attach;

	if(m_hand_group != -1) {
		long group = m_hand_group;
		m_hand_pos = caster->obj->vertexlist3[group].v;
	}
	
	if(m_caster == PlayerEntityHandle) {

		afBeta = player.angle.getPitch();

		if(m_hand_group != -1) {
			srcPos = m_hand_pos;
		} else {
			srcPos = player.pos;
		}
	} else {
		afBeta = entities[m_caster]->angle.getPitch();

		if(m_hand_group != -1) {
			srcPos = m_hand_pos;
		} else {
			srcPos = entities[m_caster]->pos;
		}
	}
	
	srcPos += angleToVectorXZ(afBeta) * 90.f;
	
	long level = std::max(long(m_level), 1l);
	m_pSpellFx = new CMultiPoisonProjectile(level);
	m_pSpellFx->SetDuration(8000ul);
	m_pSpellFx->Create(srcPos, afBeta);
	m_duration = m_pSpellFx->GetDuration();
}

void PoisonProjectileSpell::Update(float timeDelta)
{
	CMultiPoisonProjectile * effect = static_cast<CMultiPoisonProjectile *>(m_pSpellFx);
	
	if(effect) {
		effect->m_caster = m_caster;
		effect->m_level = m_level;
		effect->m_timcreation = m_timcreation;
		
		effect->Update(timeDelta);
		effect->Render();
	}
}
