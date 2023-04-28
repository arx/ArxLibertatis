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
#include "core/GameTime.h"

#include "scene/Object.h"

#include "graphics/Renderer.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"

FlyingEye eyeball;

float MagicSightFader = 0.f;

static TextureContainer * Flying_Eye = nullptr;
static std::unique_ptr<EERIE_3DOBJ> eyeballobj; // EyeBall 3D Object

FlyingEye::FlyingEye()
	: exist(0)
	, status(EYEBALL_INACTIVE)
	, pos(0.f)
	, size(0.f)
	, floating(0.f)
{

}

FlyingEye::~FlyingEye() {

}

void FlyingEye::init() {
	
	Flying_Eye = TextureContainer::LoadUI("graph/particles/flying_eye_fx");
	eyeballobj = loadObject("editor/obj3d/eyeball.teo");
}

void FlyingEye::release() {
	eyeballobj = { };
}

void FlyingEye::DrawMagicSightInterface() {
	if(eyeball.status == EYEBALL_LAUNCHED || !Flying_Eye)
		return;
	
	UseRenderState state(render2D().blend(BlendZero, BlendInvSrcColor));
	
	float col = 0.75f + PULSATE * (1.f / 20);

	if(col > 1.f) {
		col = 1.f;
	}

	if(eyeball.status == EYEBALL_DISAPPEAR) {
		col = -eyeball.exist * (1.f / 100);
	} else if(eyeball.status == EYEBALL_APPEAR) {
		col = 1.f - eyeball.size.x;
	}

	EERIEDrawBitmap(Rectf(g_size), 0.0001f, Flying_Eye, Color::gray(col));
	
	if(MagicSightFader > 0.f) {
		col = MagicSightFader;
		EERIEDrawBitmap(Rectf(g_size), 0.0001f, nullptr, Color::gray(col));
		MagicSightFader -= g_platformTime.lastFrameDuration() / 400ms;
		if(MagicSightFader < 0.f) {
			MagicSightFader = 0.f;
		}
	}
	
}

void FlyingEye::ARXDRAW_DrawEyeBall() {
	
	if(eyeball.status == EYEBALL_INACTIVE || !eyeballobj) {
		return;
	}
	
	float d;
	
	if(eyeball.status == EYEBALL_DISAPPEAR) {
		d = -eyeball.exist * (1.0f / 100);
		eyeball.exist++;
		if (eyeball.exist == 0) {
			eyeball.status = EYEBALL_INACTIVE;
		}
	} else if(eyeball.status == EYEBALL_APPEAR) {
		d = eyeball.exist * (1.0f / 100);
	} else {
		return;
	}
	
	Anglef angle = eyeball.angle;
	angle.setYaw(MAKEANGLE(180.f - angle.getYaw()));
	
	Vec3f pos = eyeball.pos;
	pos.y += eyeball.floating;
	
	Vec3f scale = Vec3f(d);
	Color3f rgb = Color3f::gray(d);
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Draw3DObject(eyeballobj.get(), angle, pos, scale, rgb, mat);
	
}
