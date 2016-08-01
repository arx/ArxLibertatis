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

#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/magic/spells/SpellsLvl06.h"
#include "game/magic/spells/SpellsLvl07.h"
#include "gui/Speech.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/GameSound.h"
#include "scene/Interactive.h"


BlessSpell::BlessSpell()
	: SpellBase()
	, m_pos(Vec3f_ZERO)
	, m_yaw(0)
	, m_scale(0)
	, tex_p1(NULL)
	, tex_sol(NULL)
	, fRot(0)
{ }

bool BlessSpell::CanLaunch()
{
	return !spells.ExistAnyInstanceForThisCaster(m_type, m_caster);
}

void BlessSpell::Launch()
{
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	spells.endByCaster(m_target, SPELL_BLESS);
	
	ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
	
	// TODO m_launchDuration is not used
	// m_duration = (m_launchDuration > -1) ? m_launchDuration : 2000000;
	m_duration = ArxDurationMs(20000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.3333f * m_level;
	
	m_pos = entities[m_caster]->pos;
	
	m_yaw = 0.f;
	m_scale = 0.f;
	fRot = 0.f;
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_sol = TextureContainer::Load("graph/particles/(fx)_pentagram_bless");
	
	m_targets.push_back(m_target);
}

void BlessSpell::End() {
	
	m_targets.clear();
}

void BlessSpell::Update() {
	
	fRot += g_framedelay * 0.25f;
	
	if(ValidIONum(m_target)) {
		m_pos = entities[m_target]->pos;
		
		if(m_target == EntityHandle_Player)
			m_yaw = player.angle.getYaw();
		else 
			m_yaw = entities[m_target]->angle.getYaw();
	}
	
	m_scale = (m_level + 10) * 6.f;
	
	Vec3f pos = m_pos + Vec3f(0, -5, 0);
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setDepthTest(true);
	mat.setLayer(RenderMaterial::Decal);
	mat.setTexture(tex_sol);
	
	float fBetaRadCos = glm::cos(glm::radians(MAKEANGLE(m_yaw))) * m_scale;
	float fBetaRadSin = glm::sin(glm::radians(MAKEANGLE(m_yaw))) * m_scale;

	ColorRGBA color = Color::white.toRGB();
	
	{
	TexturedQuad q;
	
	q.v[0].color = color;
	q.v[1].color = color;
	q.v[2].color = color;
	q.v[3].color = color;
	
	q.v[0].uv = Vec2f_ZERO;
	q.v[1].uv = Vec2f_X_AXIS;
	q.v[2].uv = Vec2f_ONE;
	q.v[3].uv = Vec2f_Y_AXIS;
	
	q.v[0].p.x = pos.x + fBetaRadCos - fBetaRadSin;
	q.v[0].p.y = pos.y;
	q.v[0].p.z = pos.z + fBetaRadSin + fBetaRadCos;
	q.v[1].p.x = pos.x - fBetaRadCos - fBetaRadSin;
	q.v[1].p.y = pos.y;
	q.v[1].p.z = pos.z - fBetaRadSin + fBetaRadCos;
	q.v[2].p.x = pos.x - fBetaRadCos + fBetaRadSin;
	q.v[2].p.y = pos.y;
	q.v[2].p.z = pos.z - fBetaRadSin - fBetaRadCos;
	q.v[3].p.x = pos.x + fBetaRadCos + fBetaRadSin;
	q.v[3].p.y = pos.y;
	q.v[3].p.z = pos.z + fBetaRadSin - fBetaRadCos;
	
	drawQuadRTP(mat, q);
	}
	
	for(int i = 0; i < 12; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = m_pos - Vec3f(0.f, 20.f, 0.f);
		pd->move = Vec3f(Random::getf(-3.f, 3.f), Random::getf(0.f, 0.5f), Random::getf(-3.f, 3.f));
		pd->siz = 0.005f;
		pd->tolive = Random::getu(1000, 2000);
		pd->tc = tex_p1;
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->m_rotation = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.6f, 0.2f);
	}
}

Vec3f BlessSpell::getPosition() {
	return getTargetPosition();
}

void DispellFieldSpell::Launch()
{
	m_duration = ArxDurationMs(10);
	
	long valid = 0, dispelled = 0;

	for(size_t n = 0; n < MAX_SPELLS; n++) {
		SpellBase * spell = spells[SpellHandle(n)];
		
		if(!spell) {
			continue;
		}
		
		bool cancel = false;
		Vec3f pos;
		
		switch(spell->m_type) {
			
			case SPELL_CREATE_FIELD: {
				if(m_caster != EntityHandle_Player || spell->m_caster == EntityHandle_Player) {
					pos = static_cast<CreateFieldSpell *>(spell)->getPosition();
					cancel = true;
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				pos = static_cast<FireFieldSpell *>(spell)->getPosition();
				cancel = true;
				break;
			}
			
			case SPELL_ICE_FIELD: {
				pos = static_cast<IceFieldSpell *>(spell)->getPosition();
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
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(20000);
	
	if(m_caster == EntityHandle_Player)
		m_duration = ArxDurationMs(2000000);
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION, &entities[m_target]->pos);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.5f, 0.3f, 0.f);
		io->halo.radius = 45.f;
	}
	
	m_targets.push_back(m_target);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_LOOP, &entities[m_target]->pos, 1.f, ARX_SOUND_PLAY_LOOPED);
}

void FireProtectionSpell::End()
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_END, &entities[m_target]->pos);
	m_targets.clear();
	
	if(ValidIONum(m_target))
		ARX_HALO_SetToNative(entities[m_target]);
}

void FireProtectionSpell::Update() {
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.5f, 0.3f, 0.f);
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
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(20000);
	
	if(m_caster == EntityHandle_Player)
		m_duration = ArxDurationMs(2000000);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.2f, 0.2f, 0.45f);
		io->halo.radius = 45.f;
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP, &entities[m_target]->pos, 1.f, ARX_SOUND_PLAY_LOOPED);
	
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

void ColdProtectionSpell::Update() {
	
	if(ValidIONum(m_target)) {
		Entity *io = entities[m_target];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color = Color3f(0.2f, 0.2f, 0.45f);
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
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(6000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.9f;
	
	if(m_caster == EntityHandle_Player) {
		player.m_telekinesis = true;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &m_caster_pos);
}

void TelekinesisSpell::End()
{
	if(m_caster == EntityHandle_Player)
		player.m_telekinesis = false;
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[m_caster]->pos);
}



CurseSpell::CurseSpell()
	: SpellBase()
	, tex_p1(NULL)
	, fRot(0.f)
{}

void CurseSpell::Launch()
{
	spells.endByCaster(m_target, SPELL_CURSE);
	
	ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &entities[m_target]->pos);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(2000000);
	m_hasDuration = true;
	m_fManaCostPerSecond = 0.5f * m_level;
	
	Vec3f target = getTargetPos(m_caster, m_target);
	if(m_target == EntityHandle_Player) {
		target.y -= 200.f;
	} else if(ValidIONum(m_target)) {
		target.y += entities[m_target]->physics.cyl.height - 50.f;
	}
	
	m_pos = target;
	fRot = 0.f;
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	m_targets.push_back(m_target);
}

void CurseSpell::End() {
	
	m_targets.clear();
}

void CurseSpell::Update() {
	
	fRot += g_framedelay * 0.25f;
	
	Vec3f target = Vec3f_ZERO;
	if(ValidIONum(m_target)) {
		target = entities[m_target]->pos;

		if(m_target == EntityHandle_Player)
			target.y -= 200.f;
		else
			target.y += entities[m_target]->physics.cyl.height - 30.f;
	}
	m_pos = target;
	
	RenderMaterial mat;
	mat.setCulling(CullCW);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Opaque);
	
	Draw3DObject(svoodoo, Anglef(0, fRot, 0), m_pos, Vec3f_ONE, Color3f::white, mat);
	
	for(int i = 0; i < 4; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = m_pos;
		pd->move = Vec3f(Random::getf(-2.f, 2.f), Random::getf(-20.f, -10.f), Random::getf(-2.f, 2.f));
		pd->siz = 0.015f;
		pd->tolive = Random::getu(1000, 1600);
		pd->tc = tex_p1;
		pd->m_flags = ROTATING | DISSIPATING | SUBSTRACT | GRAVITY;
		pd->m_rotation = 0.0000001f;
	}
}

Vec3f CurseSpell::getPosition() {
	return getTargetPosition();
}
