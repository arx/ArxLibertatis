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

#include "game/magic/spells/SpellsLvl07.h"

#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/ParticleSystems.h"
#include "game/spell/FlyingEye.h"

#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"

#include "gui/Interface.h"
#include "math/RandomVector.h"

#include "scene/GameSound.h"
#include "scene/Object.h"
#include "scene/Scene.h"

extern PlatformInstant SLID_START;
bool bOldLookToggle;

FlyingEyeSpell::FlyingEyeSpell()
	: m_lastupdate(0)
{ }

bool FlyingEyeSpell::CanLaunch() {
	
	if(!eyeball.isInactive()) {
		return false;
	}

	if(spells.getSpellByCaster(m_caster, m_type)) {
		return false;
	}
	
	if(m_caster == EntityHandle_Player) {
		m_target = EntityHandle_Player;
	}
	
	if(m_target != EntityHandle_Player) {
		return false;
	}
	
	return true;
}

void FlyingEyeSpell::Launch() {
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_EYEBALL_IN);
	
	m_lastupdate = m_timcreation;
	m_duration = 0;
	m_hasDuration = false;
	m_fManaCostPerSecond = 3.2f;
	
	eyeball.launch();
	
	for(size_t n = 0; n < 12; n++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		pd->ov = eyeball.pos + arx::randomVec(-5.f, 5.f);
		pd->move = arx::randomVec(-2.f, 2.f);
		pd->size = 28.f;
		pd->duration = Random::get(2000ms, 6000ms);
		pd->sizeDelta = 12.f;
		pd->tc = g_particleTextures.smoke;
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->m_rotation = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.7f, 1.f);
	}
	
	TRUE_PLAYER_MOUSELOOK_ON = true;
	SLID_START = g_platformTime.frameStart();
	bOldLookToggle = config.input.mouseLookToggle;
	config.input.mouseLookToggle = true;
}

void FlyingEyeSpell::End() {

	if (eyeball.isInactive()) {
		return;
	}
	
	Entity * caster = entities.get(m_caster);
	if(caster) {
		ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE, &caster->pos);
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_EYEBALL_OUT);
	eyeball.end();

	for(long n = 0; n < 12; n++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		pd->ov = eyeball.pos + arx::randomVec(-5.f, 5.f);
		pd->move = arx::randomVec(-2.f, 2.f);
		pd->size = 28.f;
		pd->duration = Random::get(2000ms, 6000ms);
		pd->sizeDelta = 12.f;
		pd->tc = g_particleTextures.smoke;
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->m_rotation = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.7f, 1.f);
	}
	
	config.input.mouseLookToggle = bOldLookToggle;
	
	lightHandleDestroy(m_light1);
	lightHandleDestroy(m_light2);
}

static void FlyingEyeSpellUpdateHand(const Vec3f & pos, LightHandle & light, size_t particles) {
	
	EERIE_LIGHT * el = dynLightCreate(light);
	if(el) {
		el->intensity = 1.3f;
		el->fallend = 180.f;
		el->fallstart = 50.f;
		el->rgb = Color3f(0.7f, 0.3f, 1.f);
		el->pos = pos;
	}
	
	for(size_t i = 0; i < particles; i++) {
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		pd->ov = pos + arx::randomVec(-1.f, 1.f);
		pd->move = Vec3f(0.1f, 0.f, 0.1f) + Vec3f(-0.2f, -2.2f, -0.2f) * arx::randomVec3f();
		pd->size = 5.f;
		pd->duration = Random::get(1500ms, 3500ms);
		pd->sizeDelta = 0.2f;
		pd->tc = g_particleTextures.smoke;
		pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->sourceionum = EntityHandle_Player;
		pd->m_rotation = 0.0000001f;
		pd->rgb = Color3f(.7f, .3f, 1.f) + Color3f(-.1f, -.1f, -.1f) * randomColor3f();
	}
	
}

void FlyingEyeSpell::Update() {
	
	const GameInstant now = g_gameTime.now();
	
	eyeball.update();
	
	m_lastupdate = now;
	
	Entity * io = entities.player();
	
	size_t particles = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.06f);
	
	if(io->obj->fastaccess.primary_attach) {
		Vec3f pos = io->obj->vertexWorldPositions[io->obj->fastaccess.primary_attach].v;
		FlyingEyeSpellUpdateHand(pos, m_light1, particles);
	}
	
	if(io->obj->fastaccess.left_attach) {
		Vec3f pos = io->obj->vertexWorldPositions[io->obj->fastaccess.left_attach].v;
		FlyingEyeSpellUpdateHand(pos, m_light2, particles);
	}
	
}

Vec3f FlyingEyeSpell::getPosition() const {
	return eyeball.pos;
}

FireFieldSpell::FireFieldSpell()
	: m_pos(0.f)
{ }

void FireFieldSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_FIRE_FIELD);
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_FIELD_START);
	
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : 100s;
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.8f;
	m_light = { };
	
	Vec3f target;
	float beta;
	bool displace;
	if(m_caster == EntityHandle_Player) {
		target = player.basePosition();
		beta = player.angle.getYaw();
		displace = true;
	} else {
		Entity * io = entities.get(m_caster);
		arx_assert(io);
		target = io->pos;
		beta = io->angle.getYaw();
		displace = (io->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 250.f;
	}
	
	m_pos = target + Vec3f(0, -10, 0);
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 10.f;
	damage.area = DAMAGE_FULL;
	damage.duration = GameDuration::max();
	damage.source = m_caster;
	damage.flags = 0;
	damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_FIELD;
	damage.pos = target;
	m_damage = DamageCreate(this, damage);
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_FIRE_FIELD_LOOP, &target, 1.f);
	
	pPSStream.SetParams(g_particleParameters[ParticleParam_FireFieldBase]);
	pPSStream.SetPos(m_pos);
	
	pPSStream1.SetParams(g_particleParameters[ParticleParam_FireFieldFlame]);
	pPSStream1.SetPos(m_pos + Vec3f(0, 10, 0));
	pPSStream1.Update(0);
}

void FireFieldSpell::End() {
	
	DamageRequestEnd(m_damage);
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_FIELD_END);
}

void FireFieldSpell::Update() {
	
	pPSStream.Update(g_gameTime.lastFrameDuration());
	pPSStream1.Update(g_gameTime.lastFrameDuration());
	
	EERIE_LIGHT * el = dynLightCreate(m_light);
	if(el) {
		el->pos = m_pos + Vec3f(0.f, -120.f, 0.f);
		el->intensity = 4.6f;
		el->fallstart = Random::getf(150.f, 180.f);
		el->fallend   = Random::getf(290.f, 320.f);
		el->rgb = Color3f(1.f, 0.8f, 0.6f) + Color3f(Random::getf(-0.1f, 0.f), 0.f, 0.f);
		el->duration = 600ms;
		el->extras = 0;
	}
	
	if(!VisibleSphere(Sphere(m_pos - Vec3f(0.f, 120.f, 0.f), 350.f))) {
		return;
	}
	
	pPSStream.Render();
	pPSStream1.Render();
	
	size_t count = size_t(m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.125f));
	for(size_t i = 0; i < count; i++) {
		
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		float t = Random::getf() * (glm::pi<float>() * 2.f) - glm::pi<float>();
		float ts = std::sin(t);
		float tc = std::cos(t);
		pd->ov = m_pos + Vec3f(120.f * ts, 15.f * ts, 120.f * tc) * arx::randomVec();
		pd->move = Vec3f(2.f, 1.f, 2.f) + Vec3f(-4.f, -8.f, -4.f) * arx::randomVec3f();
		pd->size = 7.f;
		pd->duration = Random::get(500ms, 1500ms);
		pd->tc = g_particleTextures.fire2;
		pd->m_flags = ROTATING | FIRE_TO_SMOKE;
		pd->m_rotation = Random::getf(-0.1f, 0.1f);
		pd->sizeDelta = -8.f;
		
		PARTICLE_DEF copy = *pd;
		
		PARTICLE_DEF * pd2 = createParticle(true);
		if(!pd2) {
			break;
		}
		*pd2 = copy;
		pd2->elapsed = -Random::get(60ms, 210ms);
		
	}
	
}

Vec3f FireFieldSpell::getPosition() const {
	return m_pos;
}

IceFieldSpell::IceFieldSpell()
	: m_pos(0.f)
	, tex_p1(nullptr)
	, tex_p2(nullptr)
{ }

void IceFieldSpell::Launch() {
	
	spells.endByCaster(m_caster, SPELL_ICE_FIELD);
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_ICE_FIELD);
	
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : 100s;
	m_hasDuration = true;
	m_fManaCostPerSecond = 2.8f;
	m_light = { };
	
	Vec3f target;
	float beta;
	bool displace;
	if(m_caster == EntityHandle_Player) {
		target = player.basePosition();
		beta = player.angle.getYaw();
		displace = true;
	} else {
		Entity * io = entities.get(m_caster);
		arx_assert(io);
		target = io->pos;
		beta = io->angle.getYaw();
		displace = (io->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target += angleToVectorXZ(beta) * 250.f;
	}
	
	m_pos = target;
	
	DamageParameters damage;
	damage.radius = 150.f;
	damage.damages = 10.f;
	damage.area = DAMAGE_FULL;
	damage.duration = GameDuration::max();
	damage.source = m_caster;
	damage.flags = 0;
	damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD | DAMAGE_TYPE_FIELD;
	damage.pos = target;
	m_damage = DamageCreate(this, damage);
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	
	for(int i = 0; i < iMax; i++) {
		float t = Random::getf();

		if(t < 0.5f) {
			tType[i] = 0;
		} else {
			tType[i] = 1;
		}
		
		tSize[i] = Vec3f(0.f);
		tSizeMax[i] = arx::randomVec3f() + Vec3f(0.f, 0.2f, 0.f);
		
		Vec3f minPos = (tType[i] == 0) ? Vec3f(1.2f, 1.f, 1.2f) : Vec3f(0.4f, 0.3f, 0.4f);
		
		tSizeMax[i] = glm::max(tSizeMax[i], minPos);
		
		if(tType[i] == 0) {
			tPos[i].x = m_pos.x + Random::getf(-80.f, 80.f);
			tPos[i].y = m_pos.y;
			tPos[i].z = m_pos.z + Random::getf(-80.f, 80.f);
		} else {
			tPos[i].x = m_pos.x + Random::getf(-120.f, 120.f);
			tPos[i].y = m_pos.y;
			tPos[i].z = m_pos.z + Random::getf(-120.f, 120.f);
		}
	}
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_ICE_FIELD_LOOP, &target, 1.f);
}

void IceFieldSpell::End() {
	
	DamageRequestEnd(m_damage);
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_ICE_FIELD_END);
}

void IceFieldSpell::Update() {
	
	EERIE_LIGHT * el = dynLightCreate(m_light);
	if(el) {
		el->pos = m_pos + Vec3f(0.f, -120.f, 0.f);
		el->intensity = 4.6f;
		el->fallstart = Random::getf(150.f, 180.f);
		el->fallend   = Random::getf(290.f, 320.f);
		el->rgb = Color3f(0.76f, 0.76f, 1.0f) + Color3f(0.f, 0.f, Random::getf(-0.1f, 0.f));
		el->duration = 600ms;
		el->extras = 0;
	}

	if(!VisibleSphere(Sphere(m_pos - Vec3f(0.f, 120.f, 0.f), 350.f))) {
		return;
	}
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	for(int i = 0; i < iMax; i++) {
		
		tSize[i] += Vec3f(0.1f);
		tSize[i] = glm::min(tSize[i], tSizeMax[i]);
		
		Anglef stiteangle(0.f, glm::cos(glm::radians(tPos[i].x)) * 360, 0.f);
		Vec3f stitepos(tPos[i].x, m_pos.y, tPos[i].z);
		Vec3f stitescale(tSize[i].z, tSize[i].y, tSize[i].x);
		Color3f stitecolor = Color3f(0.7f, 0.7f, 0.9f) * tSizeMax[i].y;
		
		if(stitecolor.r > 1) {
			stitecolor.r = 1;
		}
		
		if(stitecolor.g > 1) {
			stitecolor.g = 1;
		}
		
		if(stitecolor.b > 1) {
			stitecolor.b = 1;
		}
		
		EERIE_3DOBJ * obj = (tType[i] == 0) ? smotte.get() : stite.get();
		
		Draw3DObject(obj, stiteangle, stitepos, stitescale, stitecolor, mat);
	}
	
	if(m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.03f) < 1) {
		return;
	}
	
	for(int i = 0; i < iMax / 2; i++) {
		
		float t = Random::getf();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle(true);
			if(pd) {
				pd->ov = tPos[i] + arx::randomVec(-5.f, 5.f);
				pd->move = arx::randomVec(-2.f, 2.f);
				pd->size = 20.f;
				pd->duration = Random::get(2000ms, 6000ms);
				pd->tc = tex_p2;
				pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle(true);
			if(pd) {
				pd->ov = tPos[i] + arx::randomVec(-5.f, 5.f) + Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, Random::getf(-2.f, 2.f), 0.f);
				pd->size = 0.5f;
				pd->duration = Random::get(2000ms, 6000ms);
				pd->tc = tex_p1;
				pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
				pd->m_rotation = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
		
	}
	
}

Vec3f IceFieldSpell::getPosition() const {
	return m_pos;
}


void LightningStrikeSpell::Launch() {
	
	Vec3f target(0.f, 0.f, -500.f);
	m_lightning.Create(Vec3f(0.f), target);
	m_lightning.SetDuration(500ms * m_level);
	m_lightning.m_isMassLightning = false;
	m_duration = m_lightning.m_duration;
	m_hasDuration = true;
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_LIGHTNING_START, &m_caster_pos);
	
	m_snd_loop = ARX_SOUND_PlaySFX_loop(g_snd.SPELL_LIGHTNING_LOOP, &m_caster_pos, 1.f);
}

void LightningStrikeSpell::End() {
	
	ARX_SOUND_Stop(m_snd_loop);
	m_snd_loop = audio::SourcedSample();
	
	Entity * caster = entities.get(m_caster);
	if(caster) {
		ARX_SOUND_PlaySFX(g_snd.SPELL_ELECTRIC, &caster->pos);
		ARX_SOUND_PlaySFX(g_snd.SPELL_LIGHTNING_END, &caster->pos);
	}
}

static Vec3f GetChestPos(EntityHandle num) {
	
	if(num == EntityHandle_Player) {
		return player.pos + Vec3f(0.f, 70.f, 0.f);
	}
	
	Entity * io = entities.get(num);
	if(!io) {
		// should not happen
		return Vec3f(0.f);
	}
	
	if(VertexGroupId chest = EERIE_OBJECT_GetGroup(io->obj, "chest")) {
		return io->obj->vertexWorldPositions[io->obj->grouplist[chest].origin].v;
	}
	
	return io->pos + Vec3f(0.f, -120.f, 0.f);
}

void LightningStrikeSpell::Update() {
	
	float fBeta = 0.f;
	float falpha = 0.f;
	
	Entity * caster = entities.get(m_caster);
	if(caster) {
		if(VertexGroupId chest = EERIE_OBJECT_GetGroup(caster->obj, "chest")) {
			m_caster_pos = caster->obj->vertexWorldPositions[caster->obj->grouplist[chest].origin].v;
		} else {
			m_caster_pos = caster->pos;
		}
	}
	
	if(m_caster == EntityHandle_Player) {
		falpha = -player.angle.getPitch();
		fBeta = player.angle.getYaw();
	} else {
		fBeta = caster ? caster->angle.getYaw() : 0.f;
		if(caster && entities.get(caster->targetinfo) && caster->targetinfo != m_caster) {
			const Vec3f & p1 = m_caster_pos;
			Vec3f p2 = GetChestPos(caster->targetinfo);
			float d = glm::distance(getXZ(p2), getXZ(p1));
			falpha = MAKEANGLE(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + d)));
		} else if(entities.get(m_target)) {
			const Vec3f & p1 = m_caster_pos;
			Vec3f p2 = GetChestPos(m_target);
			float d = glm::distance(getXZ(p2), getXZ(p1));
			falpha = MAKEANGLE(glm::degrees(getAngle(p1.y, p1.z, p2.y, p2.z + d)));
		}
	}
	
	m_lightning.m_pos = m_caster_pos;
	m_lightning.m_beta = fBeta;
	m_lightning.m_alpha = falpha;
	
	m_lightning.m_fDamage = m_level * 1.067f;
	
	m_lightning.Update(g_gameTime.lastFrameDuration());
	m_lightning.Render();
	
	if(caster) {
		ARX_SOUND_RefreshPosition(m_snd_loop, caster->pos);
	}
}

ConfuseSpell::ConfuseSpell()
	: tex_p1(nullptr)
	, tex_trail(nullptr)
	, eCurPos(0.f)
{ }

void ConfuseSpell::Launch() {
	
	Entity * target = entities.get(m_target);
	if(!target) {
		return;
	}
	
	ARX_SOUND_PlaySFX(g_snd.SPELL_CONFUSE, &target->pos);
	
	m_hasDuration = true;
	m_fManaCostPerSecond = 1.5f;
	m_duration = (m_launchDuration >= 0) ? m_launchDuration : 5s;
	
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_trail = TextureContainer::Load("graph/obj3d/textures/(fx)_bandelette_blue");
	
	const char tex[] = "graph/obj3d/interactive/fix_inter/fx_papivolle/fx_papivolle.tea";
	ANIM_HANDLE * anim_papii = EERIE_ANIMMANAGER_Load(tex);
	
	AnimLayer & au = animlayer[0];
	au.cur_anim = anim_papii;
	au.ctime = 0;
	au.flags = EA_LOOP;
	au.lastframe = 0;
	au.currentInterpolation = 0;
	au.currentFrame = 0;
	au.altidx_cur = 0;
	
	m_targets.push_back(m_target);
}

void ConfuseSpell::End() {
	
	m_targets.clear();
	endLightDelayed(m_light, 500ms);
}

void ConfuseSpell::Update() {
	
	Entity * target = entities.get(m_target);
	if(!target) {
		return;
	}
	
	Vec3f pos = target->pos;
	if(m_target != EntityHandle_Player) {
		pos.y += target->physics.cyl.height - 30.f;
	}
	
	if(VertexId targetHead = target->obj->fastaccess.head_group_origin) {
		pos = target->obj->vertexWorldPositions[targetHead].v - Vec3f(0.f, 50.f, 0.f);
	}
	
	eCurPos = pos;
	
	RenderMaterial mat;
	mat.setDepthTest(false);
	mat.setBlendType(RenderMaterial::Additive);
	mat.setTexture(tex_trail);
	
	float rot = timeWaveSaw(g_gameTime.now(), 3142ms) * 360.f;
	
	Anglef stiteangle = Anglef(0.f, -rot, 0.f);
	
	{
		AnimationDuration delta = toAnimationDuration(g_gameTime.lastFrameDuration());
		EERIEDrawAnimQuatUpdate(spapi.get(), animlayer, stiteangle, eCurPos, delta, nullptr, false);
		EERIEDrawAnimQuatRender(spapi.get(), eCurPos, nullptr, 0.f);
	}
	
	size_t count = m_quantizer.update(toMsf(g_gameTime.lastFrameDuration()) * 0.18f);
	for(size_t i = 0; i < count; i++) {
		
		PARTICLE_DEF * pd = createParticle(true);
		if(!pd) {
			break;
		}
		
		pd->ov = eCurPos + toXZ(arx::diskRand(15.f));
		pd->move = Vec3f(0.f, Random::getf(1.f, 4.f), 0.f);
		pd->size = 0.25f;
		pd->duration = Random::get(2300ms, 3300ms);
		pd->tc = tex_p1;
		pd->m_flags = PARTICLE_GOLDRAIN | FADE_IN_AND_OUT | ROTATING | DISSIPATING;
		pd->m_rotation = 0.0000001f;
		
		Color3f baseColor = Color3f(0.4f, 0.2f, 0.4f);
		Color3f randomFactor = Color3f(0.4f, 0.6f, 0.4f);
		Color3f c = baseColor + randomColor3f() * randomFactor;
		while(glm::abs(c.r - c.g) > 0.3f && glm::abs(c.g - c.b) > 0.3f) {
			c = baseColor + randomColor3f() * randomFactor;
		}
		pd->rgb = c * Color3f(0.8f, 0.8f, 0.8f);
		
	}
	
	EERIE_LIGHT * light = dynLightCreate(m_light);
	if(light) {
		light->intensity = 1.3f;
		light->fallstart = 180.f;
		light->fallend   = 420.f;
		light->rgb = Color3f(0.3f, 0.3f, 0.5f) + Color3f(0.2f, 0.f, 0.2f) * randomColor3f();
		light->pos = eCurPos;
		light->duration = 200ms;
		light->extras = 0;
	}
	
}

Vec3f ConfuseSpell::getPosition() const {
	return getTargetPosition();
}
