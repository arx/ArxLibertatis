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

#include "graphics/effects/CinematicEffects.h"

#include <cstring>

#include "animation/Cinematic.h"
#include "animation/CinematicKeyframer.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/CinematicTexture.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"

/*---------------------------------------------------------------------------------*/
#define		NBOLDPOS	10
/*---------------------------------------------------------------------------------*/
float		SpecialFadeDx;
float		FlashAlpha;
int			TotOldPos;
static Vec3f OldPos[NBOLDPOS];
static float OldAz[NBOLDPOS];

/*---------------------------------------------------------------------------------*/
int FX_FadeIN(float a, int color, int colord)
{
	int		c;
	float	r, g, b;
	float	rd, gd, bd;

	r = (float)((color >> 16) & 0xFF);
	g = (float)((color >> 8) & 0xFF);
	b = (float)(color & 0xFF);

	rd = (float)((colord >> 16) & 0xFF);
	gd = (float)((colord >> 8) & 0xFF);
	bd = (float)(colord & 0xFF);

	r = (r - rd) * a + rd;
	g = (g - gd) * a + gd;
	b = (b - bd) * a + bd;

	c = Color((int)r, (int)g, (int)b, 0).toBGR();

	return c;
}
/*---------------------------------------------------------------------------------*/
int FX_FadeOUT(float a, int color, int colord)
{
	int	c;
	float	r, g, b;
	float	rd, gd, bd;

	a = 1.f - a;

	r = (float)((color >> 16) & 0xFF);
	g = (float)((color >> 8) & 0xFF);
	b = (float)(color & 0xFF);

	rd = (float)((colord >> 16) & 0xFF);
	gd = (float)((colord >> 8) & 0xFF);
	bd = (float)(colord & 0xFF);

	r = (r - rd) * a + rd;
	g = (g - gd) * a + gd;
	b = (b - bd) * a + bd;

	c = Color((int)r, (int)g, (int)b, 0).toBGR();

	return c;
}

static float LastTime;

bool FX_Blur(Cinematic *c, CinematicBitmap *tb, EERIE_CAMERA &camera)
{
	if(c->numbitmap < 0 || !tb)
		return false;

	if(TotOldPos == NBOLDPOS) {
		TotOldPos--;
		std::copy(OldPos + 1, OldPos + 1 + TotOldPos, OldPos);
		memmove(OldAz, OldAz + 1, TotOldPos * 4);
	}

	if((GetTimeKeyFramer() - LastTime) < 0.40f) {
		LastTime = GetTimeKeyFramer();
		OldPos[TotOldPos] = c->pos;
		OldAz[TotOldPos] = c->angz;
		TotOldPos++;
	}

	float alpha = 32.f;
	float dalpha = (127.f / NBOLDPOS);
	Vec3f *pos = OldPos;
	float *az = OldAz;
	int nb = TotOldPos;

	while(nb) {
		camera.orgTrans.pos = *pos;
		camera.setTargetCamera(camera.orgTrans.pos.x, camera.orgTrans.pos.y, 0.f);
		camera.angle.setPitch(0);
		camera.angle.setRoll(*az);
		PrepareCamera(&camera);

		int col = (int)alpha;
		col = (col << 24) | 0x00FFFFFF;
		DrawGrille(&tb->grid, col, 0, NULL, &c->posgrille, c->angzgrille);

		alpha += dalpha;
		pos++;
		az++;
		nb--;
	}

	return true;
}

//POST FX
bool FX_FlashBlanc(float w, float h, float speed, int color, float fps, float currfps)
{
	TexturedVertex	v[4];
	int			col;
	
	if (FlashAlpha < 0.f)
	{
		return false;
	}
	
	if (FlashAlpha == 0.f) FlashAlpha = 1.f;
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->setAlphaOp(TextureStage::ArgDiffuse);
	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendOne);
	
	col = (int)(255.f * FlashAlpha);
	col <<= 24;
	col |= color;
	
	v[0].p = Vec3f(0.f, 0.f, 0.01f);
	v[0].rhw = 1.f;
	v[0].color = col;
	v[1].p = Vec3f(w - 1.f, 0.f, 0.01f);
	v[1].rhw = 1.f;
	v[1].color = col;
	v[2].p = Vec3f(0.f, h - 1.f, 0.01f);
	v[2].rhw = 1.f;
	v[2].color = col;
	v[3].p = Vec3f(w - 1.f, h - 1.f, 0.01f);
	v[3].rhw = 1.f;
	v[3].color = col;
	
	FlashAlpha -= speed * fps / currfps;
	
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	
	return true;
}

bool SpecialFade(TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr)
{
	TexturedVertex	v[4];
	float		w, dv;

	w = (float)(mask->m_dwWidth) * 5.f;

	if (SpecialFadeDx < 0.f) return false;

	dv = (float)0.99999f * (h - 1) / mask->m_dwHeight;

	//tracer du mask
	
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->disableAlpha();
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendZero);

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetTexture(0, mask);

	v[0].p = Vec3f(SpecialFadeDx - w, 0.f, 0.01f);
	v[0].rhw = 1.f;
	v[0].color = 0xFFFFFFFF;
	v[0].uv = Vec2f(0.01f, 0.f);
	v[1].p = Vec3f(SpecialFadeDx - w + w - 1.f, 0.f, 0.01f);
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[1].uv = Vec2f(0.9999999f, 0.f);
	v[2].p = Vec3f(SpecialFadeDx - w, h, 0.01f);
	v[2].rhw = 1.f;
	v[2].color = 0xFFFFFFFF;
	v[2].uv = Vec2f(0.01f, dv);
	v[3].p = Vec3f(SpecialFadeDx - w + w - 1.f, h, 0.01f);
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;
	v[3].uv = Vec2f(0.999999f, dv);

	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);

	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->disableAlpha();
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);

	GRenderer->ResetTexture(0);

	v[0].p.x = w - 1.f + SpecialFadeDx - w;
	v[0].p.y = 0;
	v[0].p.z = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFF000000;
	v[1].p.x = ws;
	v[1].p.y = 0;
	v[1].p.z = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[2].p.x = w - 1.f + SpecialFadeDx - w;
	v[2].p.y = h;
	v[2].p.z = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFF000000;
	v[3].p.x = ws - 1;
	v[3].p.y = h;
	v[3].p.z = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;

	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);

	if (fpscurr > 1.f)
	{
		SpecialFadeDx += speed * fps / fpscurr;
	}

	if ((SpecialFadeDx - w) > ws)
	{
		SpecialFadeDx = -1.f;
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	return true;
}
/*---------------------------------------------------------------------------------*/
bool SpecialFadeR(TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr)
{
	TexturedVertex	v[4];
	float		w, dv;

	w = (float)(mask->m_dwWidth) * 5.f;

	if (SpecialFadeDx < 0.f) return false;

	dv = (float)0.99999f * (h - 1) / mask->m_dwHeight;

	//tracer du mask
	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->disableAlpha();
	
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendZero);

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	GRenderer->SetTexture(0, mask);

	v[0].p.x = ws + 1.f - SpecialFadeDx + w;
	v[0].p.y = 0;
	v[0].p.z = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFFFFFFFF;
	v[0].uv.x = 0.01f;
	v[0].uv.y = 0.f;
	v[1].p.x = ws + 1.f - SpecialFadeDx;
	v[1].p.y = 0;
	v[1].p.z = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[1].uv.x = 0.999999f;
	v[1].uv.y = 0.f;
	v[2].p.x = ws + 1.f - SpecialFadeDx + w;
	v[2].p.y = h;
	v[2].p.z = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFFFFFFFF;
	v[2].uv.x = 0.01f;
	v[2].uv.y = dv;
	v[3].p.x = ws + 1.f - SpecialFadeDx;
	v[3].p.y = h;
	v[3].p.z = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;
	v[3].uv.x = 0.9999999f;
	v[3].uv.y = dv;

	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);

	GRenderer->GetTextureStage(0)->setColorOp(TextureStage::ArgDiffuse);
	GRenderer->GetTextureStage(0)->disableAlpha();
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);

	GRenderer->ResetTexture(0);

	v[0].p.x = 0.f;
	v[0].p.y = 0;
	v[0].p.z = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFF000000;
	v[1].p.x = ws + 1.f - SpecialFadeDx;
	v[1].p.y = 0;
	v[1].p.z = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[2].p.x = 0.f;
	v[2].p.y = h;
	v[2].p.z = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFF000000;
	v[3].p.x = ws + 1.f - SpecialFadeDx;
	v[3].p.y = h;
	v[3].p.z = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;

	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);

	if (fpscurr > 1.f)
	{
		SpecialFadeDx += speed * fps / fpscurr;
	}

	if ((SpecialFadeDx - w) > ws)
	{
		SpecialFadeDx = -1.f;
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	return true;
}
/*---------------------------------------------------------------------------------*/
float	DreamAng, DreamAng2;
float	DreamTable[64*64*2];
/*---------------------------------------------------------------------------------*/
void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps)
{
	int		nx, ny;
	float	* t;
	float	s1, s2, a, a2, ox, oy;
	float	nnx, nny;

	a = DreamAng;
	a2 = DreamAng2;

	///////////////////////
	s1 = bi->nbx * EEcos(radians(0));
	s2 = bi->nby * EEcos(radians(0));
	nx = (bi->nbx + 1) << 1;
	ny = (bi->nby + 1) << 1;
	nnx = ((float)nx) + s1;
	nny = ((float)ny) + s2;

	ox = amp * ((2 * ((float)sin(nnx / 20) + (float)sin(nnx * nny / 2000)
	                  + (float)sin((nnx + nny) / 100) + (float)sin((nny - nnx) / 70) + (float)sin((nnx + 4 * nny) / 70)
	                  + 2 * (float)sin(hypot(256 - nnx, (150 - nny / 8)) / 40))));
	oy = amp * ((((float)cos(nnx / 31) + (float)cos(nnx * nny / 1783) +
	              + 2 * (float)cos((nnx + nny) / 137) + (float)cos((nny - nnx) / 55) + 2 * (float)cos((nnx + 8 * nny) / 57)
	              + (float)cos(hypot(384 - nnx, (274 - nny / 9)) / 51))));

	///////////////////////
	t = DreamTable;
	ny = ((bi->nby * bi->grid.echelle) + 1); 

	while (ny)
	{
		nx = ((bi->nbx * bi->grid.echelle) + 1); 

		while (nx)
		{

			s1 = bi->nbx * EEcos(radians(a));
			s2 = bi->nby * EEcos(radians(a2));
			a -= 15.f;
			a2 += 8.f;

			nnx = ((float)nx) + s1;
			nny = ((float)ny) + s2;

			*t++ = (float)(-ox + amp * ((2 * (sin(nnx / 20) + sin(nnx * nny / 2000)
			                                  + sin((nnx + nny) / 100) + sin((nny - nnx) / 70) + sin((nnx + 4 * nny) / 70)
			                                  + 2 * sin(hypot(256 - nnx, (150 - nny / 8)) / 40)))));
			*t++ = (float)(-oy + amp * (((cos(nnx / 31) + cos(nnx * nny / 1783) +
			                              + 2 * cos((nnx + nny) / 137) + cos((nny - nnx) / 55) + 2 * cos((nnx + 8 * nny) / 57)
			                              + cos(hypot(384 - nnx, (274 - nny / 9)) / 51)))));

			nx--;
		}

		ny--;
	}

	DreamAng += 4.f * fps;
	DreamAng2 -= 2.f * fps;
}
