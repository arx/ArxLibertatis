/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/spells/SpellsLvl10.h"

#include "core/Application.h"
#include "core/Core.h"
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

#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleEffects.h"

#include "math/RandomVector.h"

#include "scene/GameSound.h"

#include "util/Range.h"


MassLightningStrikeSpell::MassLightningStrikeSpell()
	: m_pos(0.f)
	, m_soundEffectPlayed(false)
{ }

void MassLightningStrikeSpell::Launch() {
	spells.endByType(SPELL_MASS_LIGHTNING_STRIKE);
	
	m_hasDuration = true;
	m_duration = 5s;
	m_soundEffectPlayed = false;
	
	float beta;
	if(m_caster == EntityHandle_Player) {
		m_pos = player.pos + Vec3f(0.f, 150.f, 0.f);
		beta = player.angle.getYaw();
	} else {
		Entity * io = entities[m_caster];
		m_pos = io->pos + Vec3f(0.f, -20.f, 0.f);
		beta = io->angle.getYaw();
	}
	m_pos += angleToVectorXZ(beta) * 500.f;
	
	GameDuration minDuration = 500ms * m_level;
	GameDuration maxDuration = 0;
	
	int number = glm::clamp(int(m_level), 1, 10);
	float ft = 360.0f / number;
	
	for(int i = 0; i < number; i++) {
		
		Vec3f target = m_pos + angleToVectorXZ(i * ft) * 500.0f;
		GameDuration duration = minDuration + Random::get(0ms, 5000ms);
		maxDuration = std::max(maxDuration, duration);
		
		std::unique_ptr<CLightning> lightning = std::make_unique<CLightning>(this);
		lightning->m_isMassLightning = true;
		lightning->m_fDamage = 2 * m_level * 1.067f;
		lightning->Create(m_pos, target);
		lightning->SetDuration(duration);
		m_arcs.emplace_back(std::move(lightning));
		
	}
	
	m_duration = maxDuration + 1s;
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 1.8f;
		light->fallend = 850.f;
		light->fallstart = 500.f;
		light->rgb = Color3f::red; // Color3f(1.f, 0.75f, 0.75f);
		light->pos = m_pos;
	}
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_LIGHTNING_START);
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_LIGHTNING_LOOP, &m_pos, 1.f);
	}
	
	// Draws White Flash on Screen
	UseRenderState state(render2D().blend(BlendOne, BlendOne));
	EERIEDrawBitmap(Rectf(g_size), 0.00009f, nullptr, Color::white);
}

void MassLightningStrikeSpell::End() {
	
	endLightDelayed(m_light, 200ms);
	
	if(emitsSound()) {
		ARX_SOUND_Stop(m_snd_loop);
		m_snd_loop = audio::SourcedSample();
		
		ARX_SOUND_PlaySFX(g_snd.SPELL_LIGHTNING_END);
	}
	
	m_arcs.clear();
	
}

void MassLightningStrikeSpell::Update() {
	
	for(CLightning & arc : util::dereference(m_arcs)) {
		arc.Update(g_gameTime.lastFrameDuration());
	}
	
	for(CLightning & arc : util::dereference(m_arcs)) {
		arc.Render();
	}
	
	if(emitsSound()) {
		Vec3f position = m_pos + arx::randomVec(-250.f, 250.f);
		
		ARX_SOUND_RefreshPosition(m_snd_loop, position);
		ARX_SOUND_RefreshVolume(m_snd_loop, 1.f);
		ARX_SOUND_RefreshPitch(m_snd_loop, Random::getf(0.8f, 1.2f));
		
		if(Random::getf() > 0.62f) {
			position = m_pos + arx::randomVec(-250.f, 250.f);
			ARX_SOUND_PlaySFX(g_snd.SPELL_SPARK, &position, Random::getf(0.8f, 1.2f));
		}
		
		if(Random::getf() > 0.82f) {
			position = m_pos + arx::randomVec(-250.f, 250.f);
			ARX_SOUND_PlaySFX(g_snd.SPELL_ELECTRIC, &position, Random::getf(0.8f, 1.2f));
		}
		
		if(0 > m_duration - 1800ms && !m_soundEffectPlayed) {
			m_soundEffectPlayed = true;
			ARX_SOUND_PlaySFX(g_snd.SPELL_ELECTRIC, nullptr, Random::getf(0.8f, 1.2f));
		}
	}
	
	EERIE_LIGHT * light = lightHandleGet(m_light);
	if(light) {
		light->intensity = Random::getf(1.3f, 2.3f);
	}
	
}

ControlTargetSpell::ControlTargetSpell()
	: eSrc(0.f)
	, eTarget(0.f)
	, tex_mm(nullptr)
{ }

bool ControlTargetSpell::CanLaunch() {
	
	if(!entities.get(m_target)) {
		return false;
	}
	
	for(const Entity & npc : entities.inScene(IO_NPC)) {
		
		if(npc._npcdata->lifePool.current <= 0.f) {
			continue;
		}
		
		if(npc.groups.find("demon") == npc.groups.end()) {
			continue;
		}
		
		if(closerThan(npc.pos, m_caster_pos, 900.f)) {
			return true;
		}
		
	}
	
	return false;
}

void ControlTargetSpell::Launch() {
	
	eSrc = player.pos;
	
	float fBetaRad = glm::radians(player.angle.getYaw());
	eTarget = eSrc + Vec3f(-glm::sin(fBetaRad) * 1000.f, 100.f, glm::cos(fBetaRad) * 1000.f);
	
	for(Entity & npc : entities.inScene(IO_NPC)) {
		
		if(npc._npcdata->lifePool.current <= 0.f) {
			continue;
		}
		
		if(npc.groups.find("demon") == npc.groups.end()) {
			continue;
		}
		
		if(closerThan(npc.pos, m_caster_pos, 900.f)) {
			ScriptParameters parameters;
			parameters.emplace_back(::idString(entities.get(m_target)));
			parameters.push_back(long(m_level));
			SendIOScriptEvent(entities.get(m_caster), &npc, "npc_control", parameters);
			eTarget = npc.pos;
		}
		
	}
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_CONTROL_TARGET);
	}
	
	m_duration = 1s;
	m_hasDuration = true;
	
	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_ctrl_target");
	
	pathways[0] = eSrc + Vec3f(0.f, 100.f, 0.f);
	pathways[9] = eTarget;
	Split(pathways.data(), 0, 9, 150);
	
	for(size_t i = 0; i < 9; i++) {
		if(pathways[i].y >= eSrc.y + 150) {
			pathways[i].y = eSrc.y + 150;
		}
	}
	
}

void ControlTargetSpell::Update() {
	
	if(m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.03f) < 1) {
		return;
	}
	
	constexpr int n = BEZIERPrecision;
	
	float fTrail = (m_elapsed / m_duration) * 9.f * float(n + 2);
	
	Vec3f lastpos = pathways[0];
	
	for(int i = 0; i < 9; i++) {
		
		const Vec3f v1 = pathways[std::max(0, i - 1)];
		const Vec3f v2 = pathways[i];
		const Vec3f v3 = pathways[i + 1];
		const Vec3f v4 = pathways[std::min(9, i + 2)];
		
		for(int toto = 1; toto < n; toto++) {
			
			if(fTrail < float(i * n + toto)) {
				break;
			}
			
			Vec3f newpos = arx::catmullRom(v1, v2, v3, v4, float(toto) * (1.f / float(n)));
			
			if(fTrail - float(i * n + toto) <= 70.f) {
				float c = 1.f - (fTrail - float(i * n + toto)) / 70.f;
				PARTICLE_DEF * pd = createParticle(true);
				if(pd) {
					pd->ov = lastpos;
					pd->size = 5.f * c;
					pd->duration = Random::get(10ms, 110ms);
					pd->tc = tex_mm;
					pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
					pd->m_rotation = 0.0000001f;
					pd->rgb = Color3f::gray(c);
				}
			}
			
			lastpos = newpos;
			
			PARTICLE_DEF * pd = createParticle(true);
			if(pd) {
				pd->ov = lastpos;
				pd->size = 5.f;
				pd->duration = Random::get(10ms, 110ms);
				pd->tc = tex_mm;
				pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
				pd->rgb = Color3f::gray(0.1f);
			}
			
		}
		
	}
	
}


FreezeTimeSpell::FreezeTimeSpell()
	: m_slowdown(0.f)
{
	
}

bool FreezeTimeSpell::CanLaunch() {
	return spells.getSpellByCaster(m_caster, m_type) == nullptr;
}

void FreezeTimeSpell::Launch() {
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_FREEZETIME);
	}
	
	float max_slowdown = std::max(0.f, g_gameTime.speed() - 0.01f);
	m_slowdown = glm::clamp(m_level * 0.08f, 0.f, max_slowdown);
	g_gameTime.setSpeed(g_gameTime.speed() - m_slowdown);
	
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : 200s;
	m_hasDuration = true;
	m_fManaCostPerSecond = 30.f * m_slowdown;
}

void FreezeTimeSpell::End() {
	g_gameTime.setSpeed(g_gameTime.speed() + m_slowdown);
	
	if(emitsSound()) {
		Entity * caster = entities.get(m_caster);
		if(caster) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_TELEKINESIS_END, &caster->pos);
		}
	}
}

void MassIncinerateSpell::Launch() {
	
	if(emitsSound()) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_MASS_INCINERATE);
	}
	
	m_duration = 20s;
	m_hasDuration = true;
	
	for(Entity & npc : entities.inScene(IO_NPC)) {
		
		if(npc.index() == m_caster || npc._npcdata->lifePool.current <= 0.f) {
			continue;
		}
		
		if(fartherThan(npc.pos, entities[m_caster]->pos, 500.f)) {
			continue;
		}
		
		npc.sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
		npc.sfx_time = g_gameTime.now();
		
		m_targets.push_back(npc.index());
		
	}
	
	if(!m_targets.empty() && emitsSound()) {
		m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_INCINERATE_LOOP, &m_caster_pos, 1.f);
	}
}

void MassIncinerateSpell::End() {
	m_targets.clear();
	
	if(emitsSound()) {
		ARX_SOUND_Stop(m_snd_loop);
		m_snd_loop = audio::SourcedSample();
		
		ARX_SOUND_PlaySFX(g_snd.SPELL_INCINERATE_END);
	}
}

void MassIncinerateSpell::Update() {
	
	if(emitsSound()) {
		Entity * caster = entities.get(m_caster);
		if(caster) {
			ARX_SOUND_RefreshPosition(m_snd_loop, caster->pos);
		}
	}
}
