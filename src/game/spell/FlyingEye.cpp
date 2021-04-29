/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Core.h"
#include "core/GameTime.h"

#include "scene/Object.h"

#include "graphics/Renderer.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"

EYEBALL_DEF eyeball;

float MagicSightFader=0.f;

static TextureContainer * Flying_Eye = NULL;
static EERIE_3DOBJ * eyeballobj = NULL;			// EyeBall 3D Object	// NEEDTO: Load dynamically

extern float PULSATE;

void FlyingEye_Init() {
	Flying_Eye = TextureContainer::LoadUI("graph/particles/flying_eye_fx");
	eyeballobj = LoadTheObj("editor/obj3d/eyeball.teo", "eyeball_teo maps");
}

void FlyingEye_Release() {
	delete eyeballobj;
	eyeballobj = NULL;
}

void DrawMagicSightInterface()
{
	if(eyeball.exist == 1 || !Flying_Eye)
		return;


	GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);

	float col = 0.75f + PULSATE * (1.f/20);

	if(col > 1.f)
		col = 1.f;

	if(eyeball.exist < 0) {
		col = -eyeball.exist * (1.f/100);
	} else if(eyeball.exist > 2) {
		col = 1.f - eyeball.size.x;
	}

	EERIEDrawBitmap(Rectf(g_size), 0.0001f, Flying_Eye, Color3f::gray(col).to<u8>());

	if(MagicSightFader > 0.f) {
		col = MagicSightFader;

		EERIEDrawBitmap(Rectf(g_size), 0.0001f, NULL, Color3f::gray(col).to<u8>());

		MagicSightFader -= toMs(g_platformTime.lastFrameDuration()) * (1.f/400);

		if(MagicSightFader < 0.f)
			MagicSightFader = 0.f;
	}

	GRenderer->SetBlendFunc(BlendOne, BlendOne);
}


void ARXDRAW_DrawEyeBall() {
	if(eyeball.exist == 0 || !eyeballobj)
		return;

	float d;

	if(eyeball.exist < 0) {
		d = -eyeball.exist * (1.0f/100);
		eyeball.exist++;
	} else if(eyeball.exist > 2) {
		d = eyeball.exist * (1.0f/100);
	}
	else
		return;

	Anglef angle = eyeball.angle;
	angle.setYaw(MAKEANGLE(180.f - angle.getYaw()));

	Vec3f pos = eyeball.pos;
	pos.y += eyeball.floating;

	Vec3f scale = Vec3f(d);
	Color3f rgb = Color3f::gray(d);
	
	RenderMaterial mat;
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	Draw3DObject(eyeballobj, angle, pos, scale, rgb, mat);
}
