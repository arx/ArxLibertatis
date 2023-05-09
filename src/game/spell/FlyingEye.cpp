/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/spell/FlyingEye.h"

#include <memory>

#include "core/Core.h"

#include "game/Player.h"

#include "scene/Object.h"

#include "graphics/Renderer.h"
#include "graphics/Draw.h"
#include "graphics/data/Mesh.h"

FlyingEye eyeball;

float MagicSightFader = 0.f;

FlyingEye::FlyingEye()
	: m_eyeTex(nullptr)
{
	reset();
}

FlyingEye::~FlyingEye() {

}

void FlyingEye::reset() {
	m_progress = 0;
	m_state = EYEBALL_INACTIVE;
	m_pos = Vec3f(0.f);
	m_floatY = 0.f;
}

void FlyingEye::init() {
	
	m_eyeTex = TextureContainer::LoadUI("graph/particles/flying_eye_fx");
	m_eyeballobj = loadObject("editor/obj3d/eyeball.teo");
}

void FlyingEye::release() {
	m_eyeballobj = { };
}

bool FlyingEye::isActive() {
	return m_state == EYEBALL_ACTIVE;
}

bool FlyingEye::isInactive() {
	return m_state == EYEBALL_INACTIVE;
}

void FlyingEye::launch() {

	m_timeCreation = g_gameTime.now();

	m_progress = 1;
	m_state = FlyingEye::EYEBALL_LAUNCHED;

	m_pos = player.pos;
	m_pos += angleToVectorXZ(player.angle.getYaw()) * 200.f;
	m_pos += Vec3f(0.f, 50.f, 0.f);

	m_angle = player.angle;
}

void FlyingEye::update() {
	
	GameDuration frameDiff = g_gameTime.lastFrameDuration();
	GameDuration elapsed = g_gameTime.now() - frameDiff - m_timeCreation;

	m_floatY = std::sin((elapsed) / 1s);
	m_floatY *= 10.f;

	if(elapsed <= 3s) {
		m_progress = long((elapsed) / 30ms);
		if(m_state == FlyingEye::EYEBALL_LAUNCHED && m_progress > 1) {
			m_state = FlyingEye::EYEBALL_APPEAR;
		}
		m_angle.setYaw(m_angle.getYaw() + toMsf(frameDiff) * 0.6f);
	} else {
		m_state = FlyingEye::EYEBALL_ACTIVE;
	}
}

void FlyingEye::drawMagicSightInterface() {
	if(m_state == EYEBALL_LAUNCHED || !m_eyeTex || m_state == EYEBALL_INACTIVE)
		return;
	
	UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
	
	float col = 0.75f + PULSATE * (1.f / 20);

	if(col > 1.f) {
		col = 1.f;
	}

	if(m_state == EYEBALL_DISAPPEAR || m_state == EYEBALL_APPEAR)
		col = m_progress * (1.f / 100);

	EERIEDrawBitmap(Rectf(g_size), 0.0001f, m_eyeTex, Color::gray(col));
	
	if(MagicSightFader > 0.f) {
		col = MagicSightFader;
		EERIEDrawBitmap(Rectf(g_size), 0.0001f, nullptr, Color::gray(col));
		MagicSightFader -= g_platformTime.lastFrameDuration() / 400ms;
		if(MagicSightFader < 0.f) {
			MagicSightFader = 0.f;
		}
	}
	
}

void FlyingEye::end() {
	m_state = FlyingEye::EYEBALL_DISAPPEAR;
}

void FlyingEye::render() {
	
	if((m_state != EYEBALL_APPEAR && m_state != EYEBALL_DISAPPEAR) || !m_eyeballobj) {
		return;
	}
	
	if(m_state == EYEBALL_DISAPPEAR) {
		m_progress--;
		if(m_progress == 0) {
			m_state = EYEBALL_INACTIVE;
		}
	}
	
	float d = m_progress * (1.0f / 100);
	
	Anglef angle = m_angle;
	angle.setYaw(MAKEANGLE(180.f - angle.getYaw()));
	
	Vec3f pos = m_pos;
	pos.y += m_floatY;
	
	Vec3f scale = Vec3f(d);
	Color3f rgb = Color3f::gray(d);
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Draw3DObject(m_eyeballobj.get(), angle, pos, scale, rgb, mat);
	
}
