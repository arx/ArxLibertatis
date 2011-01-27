/*
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
#include <stdlib.h>
#include "arx_c_cinematique.h"
#include "Resource.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/*---------------------------------------------------------------------------------*/
extern HWND HwndPere;
extern char AllTxt[];
extern EERIE_CAMERA	Camera;
/*---------------------------------------------------------------------------------*/
#define		NBOLDPOS	10
/*---------------------------------------------------------------------------------*/
float		SpecialFadeDx;
float		FlashAlpha;
int			TotOldPos;
EERIE_3D	OldPos[NBOLDPOS];
float		OldAz[NBOLDPOS];
 
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

	c = RGB_MAKE((int)r, (int)g, (int)b);

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

	c = RGB_MAKE((int)r, (int)g, (int)b);

	return c;
}

float LastTime;
/*---------------------------------------------------------------------------------*/
BOOL FX_Blur(CINEMATIQUE * c, LPDIRECT3DDEVICE7 device, C_BITMAP * tb)
{
	int			nb;
	EERIE_3D	* pos;
	float	*	az;
	float		alpha, dalpha;
	int			col;

	if (c->numbitmap < 0 || !tb->actif) return FALSE;

	if (TotOldPos == NBOLDPOS)
	{
		TotOldPos--;
		memmove(OldPos, OldPos + 1, TotOldPos * sizeof(EERIE_3D));
		memmove(OldAz, OldAz + 1, TotOldPos * 4);
	}

	if ((GetTimeKeyFramer(c) - LastTime) < 0.40f)
	{
		LastTime = GetTimeKeyFramer(c);
		OldPos[TotOldPos] = c->pos;
		OldAz[TotOldPos] = c->angz;
		TotOldPos++;
	}

	alpha = 32.f;
	dalpha = (127.f / NBOLDPOS);
	pos = OldPos;
	az = OldAz;
	nb = TotOldPos;

	while (nb)
	{
		Camera.pos = *pos;
		SetTargetCamera(&Camera, Camera.pos.x, Camera.pos.y, 0.f);
		Camera.angle.b = 0;
		Camera.angle.g = *az;
		PrepareCamera(&Camera);

		col = (int)alpha;
		col = (col << 24) | 0x00FFFFFF;
		DrawGrille(device, &tb->grille, col, 0, NULL, &c->posgrille, c->angzgrille);
		alpha += dalpha;
		pos++;
		az++;
		nb--;
	}

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
//POST FX
/*---------------------------------------------------------------------------------*/
BOOL FX_FlashBlanc(LPDIRECT3DDEVICE7 device, float w, float h, float speed, int color, float fps, float currfps)
{
	D3DTLVERTEX	v[4];
	int			col;

	if (FlashAlpha < 0.f)
	{
		return FALSE;
	}

	if (FlashAlpha == 0.f) FlashAlpha = 1.f;

	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	col = (int)(255.f * FlashAlpha);
	col <<= 24;
	col |= color;

	v[0].sx = 0;
	v[0].sy = 0;
	v[0].sz = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = col;
	v[1].sx = w - 1;
	v[1].sy = 0;
	v[1].sz = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = col;
	v[2].sx = 0;
	v[2].sy = h - 1;
	v[2].sz = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = col;
	v[3].sx = w - 1;
	v[3].sy = h - 1;
	v[3].sz = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = col;

	FlashAlpha -= speed * fps / currfps;

	EERIEDRAWPRIM(device, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
BOOL SpecialFade(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr)
{
	D3DTLVERTEX	v[4];
	float		w, dv;

	w = (float)(mask->m_dwWidth) * 5.f;

	if (SpecialFadeDx < 0.f) return FALSE;

	dv = (float)0.99999f * (h - 1) / mask->m_dwHeight;

	//tracer du mask
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	device->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_WRAP);

	//SETTC(device,mask->m_pddsSurface);
	SETTC(device, mask);

	v[0].sx = SpecialFadeDx - w;
	v[0].sy = 0;
	v[0].sz = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFFFFFFFF;
	v[0].tu = 0.01f;
	v[0].tv = 0.f;
	v[1].sx = SpecialFadeDx - w + w - 1;
	v[1].sy = 0;
	v[1].sz = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[1].tu = 0.9999999f;
	v[1].tv = 0.f;
	v[2].sx = SpecialFadeDx - w;
	v[2].sy = h;
	v[2].sz = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFFFFFFFF;
	v[2].tu = 0.01f;
	v[2].tv = dv;
	v[3].sx = SpecialFadeDx - w + w - 1;
	v[3].sy = h;
	v[3].sz = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;
	v[3].tu = 0.999999f;
	v[3].tv = dv;

	EERIEDRAWPRIM(device, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

	//empty
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	SETTC(device, NULL);

	v[0].sx = w - 1.f + SpecialFadeDx - w;
	v[0].sy = 0;
	v[0].sz = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFF000000;
	v[1].sx = ws;
	v[1].sy = 0;
	v[1].sz = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[2].sx = w - 1.f + SpecialFadeDx - w;
	v[2].sy = h;
	v[2].sz = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFF000000;
	v[3].sx = ws - 1;
	v[3].sy = h;
	v[3].sz = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;

	EERIEDRAWPRIM(device, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

	if (fpscurr > 1.f)
	{
		SpecialFadeDx += speed * fps / fpscurr;
	}

	if ((SpecialFadeDx - w) > ws)
	{
		SpecialFadeDx = -1.f;
	}

	device->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_CLAMP);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
BOOL SpecialFadeR(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr)
{
	D3DTLVERTEX	v[4];
	float		w, dv;

	w = (float)(mask->m_dwWidth) * 5.f;

	if (SpecialFadeDx < 0.f) return FALSE;

	dv = (float)0.99999f * (h - 1) / mask->m_dwHeight;

	//tracer du mask
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	device->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_WRAP);

	SETTC(device, mask);

	v[0].sx = ws + 1.f - SpecialFadeDx + w;
	v[0].sy = 0;
	v[0].sz = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFFFFFFFF;
	v[0].tu = 0.01f;
	v[0].tv = 0.f;
	v[1].sx = ws + 1.f - SpecialFadeDx;
	v[1].sy = 0;
	v[1].sz = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[1].tu = 0.999999f;
	v[1].tv = 0.f;
	v[2].sx = ws + 1.f - SpecialFadeDx + w;
	v[2].sy = h;
	v[2].sz = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFFFFFFFF;
	v[2].tu = 0.01f;
	v[2].tv = dv;
	v[3].sx = ws + 1.f - SpecialFadeDx;
	v[3].sy = h;
	v[3].sz = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;
	v[3].tu = 0.9999999f;
	v[3].tv = dv;

	EERIEDRAWPRIM(device, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	SETTC(device, NULL);

	v[0].sx = 0.f;
	v[0].sy = 0;
	v[0].sz = 0.01f;
	v[0].rhw = 1.f;
	v[0].color = 0xFF000000;
	v[1].sx = ws + 1.f - SpecialFadeDx;
	v[1].sy = 0;
	v[1].sz = 0.01f;
	v[1].rhw = 1.f;
	v[1].color = 0xFF000000;
	v[2].sx = 0.f;
	v[2].sy = h;
	v[2].sz = 0.01f;
	v[2].rhw = 1.f;
	v[2].color = 0xFF000000;
	v[3].sx = ws + 1.f - SpecialFadeDx;
	v[3].sy = h;
	v[3].sz = 0.01f;
	v[3].rhw = 1.f;
	v[3].color = 0xFF000000;

	EERIEDRAWPRIM(device, D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, v, 4, 0);

	if (fpscurr > 1.f)
	{
		SpecialFadeDx += speed * fps / fpscurr;
	}

	if ((SpecialFadeDx - w) > ws)
	{
		SpecialFadeDx = -1.f;
	}

	device->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_CLAMP);

	return TRUE;
}
/*---------------------------------------------------------------------------------*/
float	DreamAng, DreamAng2;
float	DreamTable[64*64*2];
/*---------------------------------------------------------------------------------*/
void FX_DreamPrecalc(C_BITMAP * bi, float amp, float fps)
{
	int		nx, ny;
	float	* t;
	float	s1, s2, a, a2, ox, oy;
	float	nnx, nny;

	a = DreamAng;
	a2 = DreamAng2;

	///////////////////////
	s1 = bi->nbx * EEcos(DEG2RAD(0));
	s2 = bi->nby * EEcos(DEG2RAD(0));
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
	ny = ((bi->nby * bi->grille.echelle) + 1); 

	while (ny)
	{
		nx = ((bi->nbx * bi->grille.echelle) + 1); 

		while (nx)
		{

			s1 = bi->nbx * EEcos(DEG2RAD(a));
			s2 = bi->nby * EEcos(DEG2RAD(a2));
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