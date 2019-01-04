/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "cinematic/CinematicEffects.h"

#include <cstring>

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicKeyframer.h"
#include "cinematic/CinematicTexture.h"

#include "core/Core.h"

#include "game/Camera.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"

float FlashAlpha;
int TotOldPos;

static const int NBOLDPOS = 10;
static Vec3f OldPos[NBOLDPOS];
static float OldAz[NBOLDPOS];

static float LastTime;

bool FX_Blur(Cinematic * c, CinematicBitmap * tb, Camera & camera) {
	
	if(c->numbitmap < 0 || !tb)
		return false;

	if(TotOldPos == NBOLDPOS) {
		TotOldPos--;
		std::copy(OldPos + 1, OldPos + 1 + TotOldPos, OldPos);
		memmove(OldAz, OldAz + 1, TotOldPos * 4);
	}

	if((GetTimeKeyFramer() - LastTime) < 0.40f) {
		LastTime = GetTimeKeyFramer();
		OldPos[TotOldPos] = c->m_pos;
		OldAz[TotOldPos] = c->angz;
		TotOldPos++;
	}

	float alpha = 32.f;
	float dalpha = (127.f / NBOLDPOS);
	Vec3f * pos = OldPos;
	float * az = OldAz;
	int nb = TotOldPos;

	while(nb) {
		camera.m_pos = *pos;
		camera.angle.setPitch(0);
		camera.angle.setYaw(0);
		camera.angle.setRoll(*az);
		PrepareCamera(&camera, g_size);
		
		Color col = Color::white;
		col.a = u8(alpha);
		DrawGrille(tb, col, 0, NULL, c->posgrille, c->angzgrille, c->fadegrille);

		alpha += dalpha;
		pos++;
		az++;
		nb--;
	}

	return true;
}

bool FX_FlashBlanc(Vec2f size, float speed, Color color, float fps, float currfps) {
	
	if(FlashAlpha < 0.f)
		return false;
	
	if(FlashAlpha == 0.f)
		FlashAlpha = 1.f;
	
	UseRenderState state(render2D().blendAdditive());
	GRenderer->ResetTexture(0);
	
	ColorRGBA col = (color * FlashAlpha).toRGB();
	
	TexturedVertex v[4];
	v[0].p = Vec3f(0.f, 0.f, 0.01f);
	v[0].w = 1.f;
	v[0].color = col;
	v[1].p = Vec3f(size.x - 1.f, 0.f, 0.01f);
	v[1].w = 1.f;
	v[1].color = col;
	v[2].p = Vec3f(0.f, size.y - 1.f, 0.01f);
	v[2].w = 1.f;
	v[2].color = col;
	v[3].p = Vec3f(size.x - 1.f, size.y - 1.f, 0.01f);
	v[3].w = 1.f;
	v[3].color = col;
	
	FlashAlpha -= speed * fps / currfps;
	
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	
	return true;
}

float DreamAng, DreamAng2;
float DreamTable[64 * 64 * 2];

void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps) {
	
	float a = DreamAng;
	float a2 = DreamAng2;
	
	Vec2i n = bi->m_count * s32(2) + Vec2i(2);
	
	Vec2f nn = Vec2f(n) + Vec2f(bi->m_count);
	
	Vec2f o;
	o.x = amp * ((2 * (std::sin(nn.x / 20) + std::sin(nn.x * nn.y / 2000)
	                  + std::sin((nn.x + nn.y) / 100) + std::sin((nn.y - nn.x) / 70) + std::sin((nn.x + 4 * nn.y) / 70)
	                  + 2 * std::sin(hypotf(256 - nn.x, (150 - nn.y / 8)) / 40))));
	o.y = amp * (((std::cos(nn.x / 31) + std::cos(nn.x * nn.y / 1783) +
	              + 2 * std::cos((nn.x + nn.y) / 137) + std::cos((nn.y - nn.x) / 55) + 2 * std::cos((nn.x + 8 * nn.y) / 57)
	              + std::cos(hypotf(384 - nn.x, (274 - nn.y / 9)) / 51))));
	
	float * t = DreamTable;
	n.y = ((bi->m_count.y * bi->grid.m_scale) + 1);
	
	while(n.y) {
		n.x = ((bi->m_count.x * bi->grid.m_scale) + 1);
		while(n.x) {
			a -= 15.f;
			a2 += 8.f;
			
			nn = Vec2f(n) + Vec2f(bi->m_count) * Vec2f(std::cos(glm::radians(a)), std::cos(glm::radians(a2)));
			
			*t++ = (-o.x + amp * ((2 * (std::sin(nn.x / 20) + std::sin(nn.x * nn.y / 2000)
			                                  + std::sin((nn.x + nn.y) / 100) + std::sin((nn.y - nn.x) / 70) + std::sin((nn.x + 4 * nn.y) / 70)
			                                  + 2 * std::sin(hypotf(256 - nn.x, (150 - nn.y / 8)) / 40)))));
			*t++ = (-o.y + amp * (((std::cos(nn.x / 31) + std::cos(nn.x * nn.y / 1783) +
			                              + 2 * std::cos((nn.x + nn.y) / 137) + std::cos((nn.y - nn.x) / 55) + 2 * std::cos((nn.x + 8 * nn.y) / 57)
			                              + std::cos(hypotf(384 - nn.x, (274 - nn.y / 9)) / 51)))));
			
			n.x--;
		}
		n.y--;
	}
	
	DreamAng += 4.f * fps;
	DreamAng2 -= 2.f * fps;
}
