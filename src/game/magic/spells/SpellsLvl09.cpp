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

#include "game/magic/spells/SpellsLvl09.h"

#include <boost/foreach.hpp>

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
#include "graphics/RenderBatcher.h"
#include "graphics/particle/ParticleEffects.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void SummonCreatureSpell::GetTargetAndBeta(Vec3f & target, float & beta)
{
	bool displace = false;
	if(m_caster == PlayerEntityHandle) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		target = entities[m_caster]->pos;
		beta = entities[m_caster]->angle.getPitch();
		displace = (entities[m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 300.f;
	}
}

SummonCreatureSpell::SummonCreatureSpell()
	: m_targetPos(Vec3f_ZERO)
	, m_megaCheat(false)
	, m_requestSummon(false)
	, m_summonedEntity()
{
	
}


bool SummonCreatureSpell::CanLaunch()
{
	Vec3f target;
	float beta;
	GetTargetAndBeta(target, beta);
	
	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	return true;
}

void SummonCreatureSpell::Launch()
{
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.9f;
	m_requestSummon = false;
	m_summonedEntity = EntityHandle();
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(2000000);
	
	Vec3f target;
	float beta;
	GetTargetAndBeta(target, beta);
	
	m_megaCheat = (m_caster == PlayerEntityHandle && cur_mega == 10);
	m_targetPos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &m_targetPos);
	
	m_fissure.Create(target, MAKEANGLE(player.angle.getPitch()));
	m_fissure.SetDuration(ArxDurationMs(2000), ArxDurationMs(500), ArxDurationMs(1500));
	m_fissure.SetColorBorder(Color3f::red);
	m_fissure.SetColorRays1(Color3f::red);
	m_fissure.SetColorRays2(Color3f::yellow * .5f);
	
	m_light = GetFreeDynLight();
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = m_fissure.m_eSrc;
	}
}

void SummonCreatureSpell::End() {
	
	lightHandleDestroy(m_light);
	// need to killio
	
	if(ValidIONum(m_summonedEntity)) {
		Entity * io = entities[m_summonedEntity];
		
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &io->pos);
		
		if(io->scriptload && (io->ioflags & IO_NOSAVE)) {
			
			AddRandomSmoke(io, 100);
			Vec3f posi = io->pos;
			posi.y -= 100.f;
			MakeCoolFx(posi);
		
			LightHandle nn = GetFreeDynLight();
			if(lightHandleIsValid(nn)) {
				EERIE_LIGHT * light = lightHandleGet(nn);
				
				light->intensity = Random::getf(0.7f, 2.7f);
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.0f);
				light->pos = posi;
				light->duration = ArxDurationMs(600);
			}
			
			io->destroyOne();
		}
	}
	
	m_summonedEntity = EntityHandle();
}

void SummonCreatureSpell::Update() {
	
	if(arxtime.is_paused())
		return;
	
	ArxDuration elapsed = arxtime.now() - m_timcreation;
	
	if(elapsed <= ArxDurationMs(4000)) {
		if(Random::getf() > 0.7f) {
			Vec3f pos = m_fissure.m_eSrc;
			MakeCoolFx(pos);
		}
		
		m_fissure.Update(g_framedelay);
		m_fissure.Render();
		
		m_requestSummon = true;
		m_summonedEntity = EntityHandle();

	} else if(m_requestSummon) {
		lightHandleDestroy(m_light);
		
		m_requestSummon = false;
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
		
		Cylinder phys = Cylinder(m_targetPos, 50, -200);
		
		float anything = CheckAnythingInCylinder(phys, NULL, CFLAG_JUST_TEST);
		
		if(glm::abs(anything) < 30) {
			
			long tokeep;
			res::path cls;
			if(m_megaCheat) {
				if(Random::getf() > 0.5f) {
					tokeep = -1;
					cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
				} else {
					tokeep = 0;
					cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
				}
			} else if(Random::getf() > 0.997f || (sp_max && Random::getf() > 0.8f)
			   || (cur_mr >= 3 && Random::getf() > 0.3f)) {
				tokeep = 0;
				cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
			} else if(Random::getf() > 0.997f || (cur_rf >= 3 && Random::getf() > 0.8f)
			   || (cur_mr >= 3 && Random::getf() > 0.3f)) {
				tokeep = -1;
				cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
			} else if(m_level >= 9) {
				tokeep = 1;
				cls = "graph/obj3d/interactive/npc/demon/demon";
			} else if(Random::getf() > 0.98f) {
				tokeep = -1;
				cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
			} else {
				tokeep = 0;
				cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
			}
			
			Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
			if(!io) {
				cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
				tokeep = 0;
				io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
			}
			
			if(io) {
				RestoreInitialIOStatusOfIO(io);
				
				io->summoner = m_caster;
				
				io->scriptload = 1;
				
				if(tokeep == 1) {
					io->ioflags |= IO_NOSAVE;
				}
				
				io->pos = phys.origin;
				SendInitScriptEvent(io);
				
				if(tokeep < 0) {
					io->scale=1.65f;
					io->physics.cyl.radius=25;
					io->physics.cyl.height=-43;
					io->speed_modif=1.f;
				}
				
				if(ValidIONum(m_caster)) {
					EVENT_SENDER = entities[m_caster];
				} else {
					EVENT_SENDER = NULL;
				}
				
				SendIOScriptEvent(io,SM_SUMMONED);
				
				for(long j = 0; j < 3; j++) {
					Vec3f pos = m_fissure.m_eSrc;
					pos += randomVec3f() * 100.f;
					pos += Vec3f(-50.f, 50.f, -50.f);
					
					MakeCoolFx(pos);
				}
				
				if(tokeep==1)
					m_summonedEntity = io->index();
				else
					m_summonedEntity = EntityHandle();
			}
		}
	} else if(m_summonedEntity == EntityHandle()) {
		m_duration = ArxDuration_ZERO;
	}
}

bool FakeSummonSpell::CanLaunch()
{
	if(m_caster.handleData() <= PlayerEntityHandle.handleData() || !ValidIONum(m_target)) {
		return false;
	}
	
	return true;
}

void FakeSummonSpell::Launch()
{
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.9f;
	m_duration = ArxDurationMs(4000);
	
	Vec3f target = entities[m_target]->pos;
	if(m_target != PlayerEntityHandle) {
		target.y += player.baseHeight();
	}
	m_targetPos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &m_targetPos);
	
	m_fissure.Create(target, MAKEANGLE(player.angle.getPitch()));
	m_fissure.SetDuration(ArxDurationMs(2000), ArxDurationMs(500), ArxDurationMs(1500));
	m_fissure.SetColorBorder(Color3f::red);
	m_fissure.SetColorRays1(Color3f::red);
	m_fissure.SetColorRays2(Color3f::yellow * .5f);
	
	m_light = GetFreeDynLight();
	
	if(lightHandleIsValid(m_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_light);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = m_fissure.m_eSrc;
	}
}

void FakeSummonSpell::End()
{
	ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &m_targetPos);
	
	lightHandleDestroy(m_light);
}

void FakeSummonSpell::Update() {
	
	if(!arxtime.is_paused()) {
		if(Random::getf() > 0.7f) {
			Vec3f pos = m_fissure.m_eSrc;
			MakeCoolFx(pos);
		}
	}
	
	m_fissure.Update(g_framedelay);
	m_fissure.Render();
}


NegateMagicSpell::NegateMagicSpell()
	: tex_p2(NULL)
	, tex_sol(NULL)
{ }

void NegateMagicSpell::Launch()
{
	if(m_caster == PlayerEntityHandle) {
		m_target = PlayerEntityHandle;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC, &entities[m_target]->pos);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.f;
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(1000000);
	
	m_pos = getTargetPos(m_caster, m_target);
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	tex_sol = TextureContainer::Load("graph/obj3d/textures/(fx)_negate_magic");
	
	LaunchAntiMagicField();
}

void NegateMagicSpell::End() {
	
}

void NegateMagicSpell::Update() {
	
	LaunchAntiMagicField();
	
	if(m_target == PlayerEntityHandle) {
		m_pos = player.basePosition();
	} else {
		m_pos = entities[m_target]->pos;
	}
	
	Vec3f stitepos = m_pos - Vec3f(0.f, 10.f, 0.f);
	
	RenderMaterial mat;
	mat.setLayer(RenderMaterial::Decal);
	mat.setDepthTest(true);
	mat.setTexture(tex_sol);
	mat.setBlendType(RenderMaterial::Additive);
	
	for(int i = 0; i < 360; i++) {
		float t = Random::getf();
		if(t < 0.04f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(!pd) {
				break;
			}
			
			pd->ov = stitepos + Vec3f(Random::getf(-150.f, 150.f), 0.f, Random::getf(-150.f, 150.f));
			pd->move = Vec3f(0.f, Random::getf(-3.f, 0.f), 0.f);
			pd->siz = 0.3f;
			pd->tolive = Random::getu(2000, 4000);
			pd->tc = tex_p2;
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING | SUBSTRACT;
			pd->m_rotation = 0.0000001f;
		}
	}
	
	float now = arxtime.now_f();
	
	Anglef stiteangle(0.f, -now * 0.02f, 0.f);
	float scalediff = std::sin(now * 0.004f);
	
	{
	Color3f stitecolor = Color3f::gray(.4f);
	Vec3f stitescale = Vec3f(3.f + 0.5f * scalediff);
	Draw3DObject(ssol, stiteangle, stitepos, stitescale, stitecolor, mat);
	}
	
	{
	Color3f stitecolor = Color3f(.5f, 0.f, .5f);
	Vec3f stitescale = Vec3f(3.1f + 0.2f * scalediff);
	Draw3DObject(ssol, stiteangle, stitepos, stitescale, stitecolor, mat);
	}
}

void NegateMagicSpell::LaunchAntiMagicField() {
	
	if(!ValidIONum(m_target))
		return;
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = spells[SpellHandle(i)];
		
		if(!spell)
			continue;
		
		if(this == spell)
			continue;
		
		if(m_level < spell->m_level)
			continue;
		
		Vec3f pos = spell->getPosition();
		if(closerThan(pos, entities[m_target]->pos, 600.f)) {
			if(spell->m_type != SPELL_CREATE_FIELD) {
				spells.endSpell(spell);
			} else if(m_target == PlayerEntityHandle && spell->m_caster == PlayerEntityHandle) {
				spells.endSpell(spell);
			}
		}
	}
}

bool IncinerateSpell::CanLaunch()
{
	Entity * tio = entities[m_target];
	if((tio->ioflags & IO_NPC) && tio->_npcdata->lifePool.current <= 0.f) {
		return false;
	}
	
	return true;
}

void IncinerateSpell::Launch()
{
	Entity * tio = entities[m_target];
	
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE, &entities[m_target]->pos);
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_LOOP, &entities[m_target]->pos, 1.f, ARX_SOUND_PLAY_LOOPED);
	
	m_duration = ArxDurationMs(20000);
	
	tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
	tio->sfx_time = arxtime.now();
	
	m_targets.push_back(m_target);
}

void IncinerateSpell::End()
{
	m_targets.clear();
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
}

void IncinerateSpell::Update() {
	
	if(ValidIONum(m_target)) {
		ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}	
}

Vec3f IncinerateSpell::getPosition() {
	return getTargetPosition();
}


void MassParalyseSpell::Launch()
{
	ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
	
	m_duration = (m_launchDuration > ArxDuration(-1)) ? m_launchDuration : ArxDurationMs(10000);
	
	for(size_t ii = 0; ii < entities.size(); ii++) {
		const EntityHandle handle = EntityHandle(ii);
		Entity * tio = entities[handle];
		
		if(handle == m_caster || !tio || !(tio->ioflags & IO_NPC)) {
			continue;
		}
		
		if(tio->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(tio->ioflags & IO_FREEZESCRIPT) {
			continue;
		}
		
		if(fartherThan(tio->pos, entities[m_caster]->pos, 500.f)) {
			continue;
		}
		
		tio->ioflags |= IO_FREEZESCRIPT;
		
		ARX_NPC_Kill_Spell_Launch(tio);
		m_targets.push_back(tio->index());
	}
}

void MassParalyseSpell::End()
{
	BOOST_FOREACH(EntityHandle handle, m_targets) {
		if(ValidIONum(handle)) {
			entities[handle]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}
	
	m_targets.clear();
	
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

