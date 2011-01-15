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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// CSpellFx_Lvl06.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 06
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEDraw.h>
#include "EERIELight.h"
#include <EERIEMath.h>
#include <EERIEObject.h>

#include "ARX_Spells.h"
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_SpellFx_Lvl06.h"
#include "ARX_Particles.h"
#include "ARX_CParticles.h"
#include "ARX_Time.h"


#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)


extern CParticleManager * pParticleManager;

//-----------------------------------------------------------------------------
// CREATE FIELD
//-----------------------------------------------------------------------------
CCreateField::CCreateField(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	fColor1[0] = 0.4f;
	fColor1[1] = 0.0f;
	fColor1[2] = 0.4f;
	fColor2[0] = 0.0f;
	fColor2[1] = 0.0f;
	fColor2[2] = 0.6f;

	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	tex_jelly = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu3.bmp");
}

//-----------------------------------------------------------------------------
void CCreateField::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;
	fbeta = afBeta;
	ysize = 0.1f;
	size = 0.1f;
	ft = 0.0f;
	fglow = 0;
	youp = true;
}

//-----------------------------------------------------------------------------
void CCreateField::RenderQuad(LPDIRECT3DDEVICE7 m_pd3dDevice, D3DTLVERTEX p1, D3DTLVERTEX p2, D3DTLVERTEX p3, D3DTLVERTEX p4, int rec, EERIE_3D norm)
{
	D3DTLVERTEX v[5];
	D3DTLVERTEX v2[5];

 

	if (rec < 3)
	{
		rec ++;

		// milieu
		v[0].sx = p1.sx + (p3.sx - p1.sx) * 0.5f;
		v[0].sy = p1.sy + (p3.sy - p1.sy) * 0.5f;
		v[0].sz = p1.sz + (p3.sz - p1.sz) * 0.5f;
		// gauche
		v[1].sx = p1.sx + (p4.sx - p1.sx) * 0.5f;
		v[1].sy = p1.sy + (p4.sy - p1.sy) * 0.5f;
		v[1].sz = p1.sz + (p4.sz - p1.sz) * 0.5f;
		// droite
		v[2].sx = p2.sx + (p3.sx - p2.sx) * 0.5f;
		v[2].sy = p2.sy + (p3.sy - p2.sy) * 0.5f;
		v[2].sz = p2.sz + (p3.sz - p2.sz) * 0.5f;
		// haut
		v[3].sx = p4.sx + (p3.sx - p4.sx) * 0.5f;
		v[3].sy = p4.sy + (p3.sy - p4.sy) * 0.5f;
		v[3].sz = p4.sz + (p3.sz - p4.sz) * 0.5f;
		// bas
		v[4].sx = p1.sx + (p2.sx - p1.sx) * 0.5f;
		v[4].sy = p1.sy + (p2.sy - p1.sy) * 0.5f;
		v[4].sz = p1.sz + (p2.sz - p1.sz) * 0.5f;

		float patchsize = 0.005f; 

		v[0].sx += (float) sin(DEG2RAD((v[0].sx - eSrc.x) * patchsize + fwrap)) * 5;
		v[0].sy += (float) sin(DEG2RAD((v[0].sy - eSrc.y) * patchsize + fwrap)) * 5;
		v[0].sz += (float) sin(DEG2RAD((v[0].sz - eSrc.z) * patchsize + fwrap)) * 5;
		v[1].sx += (float) sin(DEG2RAD((v[1].sx - eSrc.x) * patchsize + fwrap)) * 5;
		v[1].sy += (float) sin(DEG2RAD((v[1].sy - eSrc.y) * patchsize + fwrap)) * 5;
		v[1].sz += (float) sin(DEG2RAD((v[1].sz - eSrc.z) * patchsize + fwrap)) * 5;
		v[2].sx += (float) sin(DEG2RAD((v[2].sx - eSrc.x) * patchsize + fwrap)) * 5;
		v[2].sy += (float) sin(DEG2RAD((v[2].sy - eSrc.y) * patchsize + fwrap)) * 5;
		v[2].sz += (float) sin(DEG2RAD((v[2].sz - eSrc.z) * patchsize + fwrap)) * 5;
		v[3].sx += (float) sin(DEG2RAD((v[3].sx - eSrc.x) * patchsize + fwrap)) * 5;
		v[3].sy += (float) sin(DEG2RAD((v[3].sy - eSrc.y) * patchsize + fwrap)) * 5;
		v[3].sz += (float) sin(DEG2RAD((v[3].sz - eSrc.z) * patchsize + fwrap)) * 5;
		v[4].sx += (float) sin(DEG2RAD((v[4].sx - eSrc.x) * patchsize + fwrap)) * 5;
		v[4].sy += (float) sin(DEG2RAD((v[4].sy - eSrc.y) * patchsize + fwrap)) * 5;
		v[4].sz += (float) sin(DEG2RAD((v[4].sz - eSrc.z) * patchsize + fwrap)) * 5;

		RenderQuad(m_pd3dDevice, p1, v[4], v[0], v[1], rec, norm);
		RenderQuad(m_pd3dDevice, v[4], p2, v[2], v[0], rec, norm);
		RenderQuad(m_pd3dDevice, v[0], v[2], p3, v[3], rec, norm);
		RenderQuad(m_pd3dDevice, v[1], v[0], v[3], p4, rec, norm);
	}
	else if (rec == 3)
	{
		float zab = (float) sin(DEG2RAD(ft));
		v2[0].tu = 0 + zab;
		v2[0].tv = 0 + zab;
		v2[1].tu = 1 + zab;
		v2[1].tv = 0 + zab;
		v2[2].tu = 1 + zab;
		v2[2].tv = 1 + zab;
		v2[3].tu = 0 + zab;
		v2[3].tv = 1 + zab;

		v2[1].color = v2[2].color = D3DRGB(falpha * 0.3f + rnd() * 0.025f, falpha * 0.00f, falpha * 0.5f + rnd() * 0.025f);
		v2[0].color = v2[3].color = D3DRGB(falpha * 0.3f + rnd() * 0.025f, falpha * 0.00f, falpha * 0.5f + rnd() * 0.025f);
	
		EE_RT2(&p1, &v2[0]);
		EE_RT2(&p2, &v2[1]);
		EE_RT2(&p3, &v2[2]);
		EE_RT2(&p4, &v2[3]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[1],
		                             &v2[3]);
		ARX_DrawPrimitive_SoftClippZ(&v2[1],
		                             &v2[2],
		                             &v2[3]);
	}
}

//-----------------------------------------------------------------------------
void CCreateField::RenderSubDivFace(LPDIRECT3DDEVICE7 m_pd3dDevice, D3DTLVERTEX * b, D3DTLVERTEX * t, int b1, int b2, int t1, int t2)
{
	EERIE_3D norm;
	norm.x = (b[b1].sx + b[b2].sx + t[t1].sx + t[t2].sx) * 0.25f - eSrc.x;
	norm.y = (b[b1].sy + b[b2].sy + t[t1].sy + t[t2].sy) * 0.25f - eSrc.y;
	norm.z = (b[b1].sz + b[b2].sz + t[t1].sz + t[t2].sz) * 0.25f - eSrc.z;
	Vector_Normalize(&norm);
	RenderQuad(m_pd3dDevice, b[b1], b[b2], t[t1], t[t2], 1, norm);
}

//-----------------------------------------------------------------------------
void CCreateField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
float CCreateField::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (ulCurrentTime >= ulDuration) return 0.f;

	falpha = 1.f - (((float)(ulCurrentTime)) * fOneOnDuration);

	if (falpha > 1.f) falpha = 1.f;

	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_CLAMP);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//-------------------------------------------------------------------------
	// rendu
	if (youp)
	{
		fglow += 0.5f;

		if (fglow >= 50)
		{
			youp = false;
		}
	}
	else
	{
		fglow -= 0.5f;

		if (fglow <= 0)
		{
			youp = true;
		}
	}

	ysize = min(1.0f, ulCurrentTime * 0.001f);

	if (ysize >= 1.0f)
	{
		size = min(1.0f, (ulCurrentTime - 1000) * 0.001f);
		size = max(size, 0.1f);
	}

	// ondulation
	ft += 0.01f;

	if (ft > 360.0f)
	{
		ft = 0.0f;
	}

	falpha = (float) sin(DEG2RAD(fglow)) + rnd() * 0.2f;

	if (falpha > 1.0f)
		falpha = 1.0f;

	if (falpha < 0.0f)
		falpha = 0.0f;

	float x = eSrc.x;
	float y = eSrc.y;
	float z = eSrc.z;
	float smul = 100 * size;

	// bottom points
	b[0].sx = x - smul;
	b[0].sy = y;
	b[0].sz = z - smul;

	b[1].sx = x + smul;
	b[1].sy = y;
	b[1].sz = z - smul;

	b[2].sx = x + smul;
	b[2].sy = y;
	b[2].sz = z + smul;

	b[3].sx = x - smul;
	b[3].sy = y;
	b[3].sz = z + smul;

	// top points
	t[0].sx = x - smul;
	t[0].sy = y - 250 * ysize;
	t[0].sz = z - smul;

	t[1].sx = x + smul;
	t[1].sy = y - 250 * ysize;
	t[1].sz = z - smul;

	t[2].sx = x + smul;
	t[2].sy = y - 250 * ysize;
	t[2].sz = z + smul;

	t[3].sx = x - smul;
	t[3].sy = y - 250 * ysize;
	t[3].sz = z + smul;

	fwrap -= 5.0f;

	if (fwrap > 360)
	{
		fwrap = 0;
	}

	if (fwrap < 0)
	{
		fwrap = 360;
	}

	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_WRAP);

	if (tex_jelly && tex_jelly->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_jelly);
	}

	RenderSubDivFace(m_pd3dDevice, b, b, 0, 1, 2, 3);
	RenderSubDivFace(m_pd3dDevice, t, t, 0, 3, 2, 1);
	RenderSubDivFace(m_pd3dDevice, b, t, 1, 0, 0, 1);
	RenderSubDivFace(m_pd3dDevice, b, t, 3, 2, 2, 3);
	RenderSubDivFace(m_pd3dDevice, b, t, 0, 3, 3, 0);
	RenderSubDivFace(m_pd3dDevice, b, t, 2, 1, 1, 2);

	SETZWRITE(m_pd3dDevice, TRUE);
	SETALPHABLEND(m_pd3dDevice, FALSE);

	if (lLightId != -1)
	{
		DynLight[lLightId].exist = 1;
		DynLight[lLightId].intensity = 0.7f + 2.3f * falpha;
		DynLight[lLightId].fallend = 500.f;
		DynLight[lLightId].fallstart = 400.f;
		DynLight[lLightId].rgb.r = 0.8f;
		DynLight[lLightId].rgb.g = 0.0f;
		DynLight[lLightId].rgb.b = 1.0f;
		DynLight[lLightId].pos.x = eSrc.x;
		DynLight[lLightId].pos.y = eSrc.y - 150;
		DynLight[lLightId].pos.z = eSrc.z;
		DynLight[lLightId].duration = 800;
	}

	return falpha;
}

//-----------------------------------------------------------------------------
// SLOW DOWN
//-----------------------------------------------------------------------------
CSlowDown::CSlowDown(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");

	if (!ssol) // Pentacle
	{
		ssol   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(ssol);
	}

	ssol_count++;

	if (!slight) // Twirl
	{
		slight = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard02.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(slight);
	}

	slight_count++; //runes

	if (!srune)
	{
		srune  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard03.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(srune);
	}

	srune_count++;
}

CSlowDown::~CSlowDown()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		ReleaseEERIE3DObj(ssol);
		ssol = NULL;
	}

	slight_count--;

	if (slight && (slight_count <= 0))
	{
		slight_count = 0;
		ReleaseEERIE3DObj(slight);
		slight = NULL;
	}

	srune_count--;

	if (srune && (srune_count <= 0))
	{
		srune_count = 0;
		ReleaseEERIE3DObj(srune);
		srune = NULL;
	}
}
//-----------------------------------------------------------------------------
void CSlowDown::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CSlowDown::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CSlowDown::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y + 100.0f;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			x = inter.iobj[i]->pos.x;
			y = inter.iobj[i]->pos.y;
			z = inter.iobj[i]->pos.z;
		}
	}

	y -= 40;

	y = eSrc.y + 140;

	y -= 40;
	
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	x = player.pos.x;
	y = player.pos.y + 80;
	z = player.pos.z;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120; 
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;

	SETALPHABLEND(m_pd3dDevice, TRUE);

	stiteangle.b = -stiteangle.b * 1.5f;
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 1;
	stitescale.y = -0.1f;
	stitescale.z = 1;

	stiteangle.b = -stiteangle.b;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;
	SETALPHABLEND(m_pd3dDevice, TRUE);

	y = player.pos.y + 20;
	stitepos.y = y;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	

	return 1;
}

//-----------------------------------------------------------------------------
// RISE DEAD
//-----------------------------------------------------------------------------
CRiseDead::~CRiseDead()
{
	stone0_count--;

	if (stone0 && (stone0_count <= 0))
	{
		stone0_count = 0;
		ReleaseEERIE3DObj(stone0);
		stone0 = NULL;
	}

	stone1_count--;

	if (stone1 && (stone1_count <= 0))
	{
		stone1_count = 0;
		ReleaseEERIE3DObj(stone1);
		stone1 = NULL;
	}
}
CRiseDead::CRiseDead(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDurationIntro + ulDurationRender + ulDurationOuttro + 1;

	iSize = 100;
	fOneOniSize = 1.0f / ((float) iSize);

	fColorBorder[0] = 1;
	fColorBorder[1] = 1;
	fColorBorder[2] = 1;

	fColorRays1[0] = 1;
	fColorRays1[1] = 1;
	fColorRays1[2] = 1;

	fColorRays2[0] = 0;
	fColorRays2[1] = 0;
	fColorRays2[2] = 0;

	if (stone0 == NULL)
	{
		stone0 = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_raise_dead\\stone01.teo", NULL);
	}

	stone0_count++;

	if (stone1 == NULL)
	{
		stone1 = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_raise_dead\\stone02.teo", NULL);
	}

	stone1_count++;
	tex_light = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu4.bmp");
}

//-----------------------------------------------------------------------------

void CRiseDead::SetDuration(const unsigned long alDuration)
{
	ulDurationIntro			= alDuration;

	if (ulDurationIntro <= 0)
	{
		ulDurationIntro = 100;
	}
	else if (ulDurationIntro >= 100000)
	{
		ulDurationIntro = 100000;
	}


	fOneOnDurationIntro		= 1.f / (float)(ulDurationIntro);

	ulDurationRender		= 1000;
	fOneOnDurationRender	= 1.f / (float)(ulDurationRender);

	ulDurationOuttro		= 1000;
	fOneOnDurationOuttro	= 1.f / (float)(ulDurationOuttro);

	ulCurrentTime = 0;
}

//-----------------------------------------------------------------------------
void CRiseDead::SetDuration(unsigned long alDurationIntro, unsigned long alDurationRender, unsigned long alDurationOuttro)
{
	if (alDurationIntro <= 0) alDurationIntro = 100;
	else if (alDurationIntro >= 100000) alDurationIntro = 100000;

	ulDurationIntro = alDurationIntro;
	fOneOnDurationIntro = 1.f / (float)(ulDurationIntro);

	if (alDurationRender <= 0) alDurationRender = 100;
	else if (alDurationRender >= 100000) alDurationRender = 100000;

	ulDurationRender = alDurationRender;
	fOneOnDurationRender = 1.f / (float)(ulDurationRender);

	if (alDurationOuttro <= 0) alDurationOuttro = 100;
	else if (alDurationOuttro >= 100000) alDurationOuttro = 100000;

	ulDurationOuttro = alDurationOuttro;
	fOneOnDurationOuttro = 1.f / (float)(ulDurationOuttro);

	ulCurrentTime = 0;
}

//-----------------------------------------------------------------------------
void CRiseDead::SetColorBorder(float afRed, float afGreen, float afBlue)
{
	fColorBorder[0] = afRed;
	fColorBorder[1] = afGreen;
	fColorBorder[2] = afBlue;
}

//-----------------------------------------------------------------------------
void CRiseDead::SetColorRays1(float afRed, float afGreen, float afBlue)
{
	fColorRays1[0] = afRed;
	fColorRays1[1] = afGreen;
	fColorRays1[2] = afBlue;
}

//-----------------------------------------------------------------------------
void CRiseDead::SetColorRays2(float afRed, float afGreen, float afBlue)
{
	fColorRays2[0] = afRed;
	fColorRays2[1] = afGreen;
	fColorRays2[2] = afBlue;
}

//-----------------------------------------------------------------------------
void CRiseDead::Create(EERIE_3D aeSrc, float afBeta)
{
	int i;

	SetDuration(ulDurationIntro, ulDurationRender, ulDurationOuttro);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y - 10.f; 
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	sizeF = 0;
	fSizeIntro = 0.0f;
	fTexWrap = 0;
	fRand = (float) rand();
	end = 40 - 1;
	bIntro = true;

	for (i = 0; i < 40; i++)
	{
		tfRaysa[i] = 0.4f * rnd();
		tfRaysb[i] = 0.4f * rnd();
	}

	v1a[0].sx = eSrc.x - fBetaRadSin * 100;
	v1a[0].sy = eSrc.y;
	v1a[0].sz = eSrc.z + fBetaRadCos * 100;
	v1a[end].sx = eSrc.x + fBetaRadSin * 100;
	v1a[end].sy = eSrc.y;
	v1a[end].sz = eSrc.z - fBetaRadCos * 100;

	v1b[0].sx = v1a[0].sx;
	v1b[0].sy = v1a[0].sy;
	v1b[0].sz = v1a[0].sz;
	v1b[end].sx = v1a[end].sx;
	v1b[end].sy = v1a[end].sy;
	v1b[end].sz = v1a[end].sz;

	sizeF = 200;
	this->Split(v1a, 0, end, 20);
	this->Split(v1b, 0, end, -20);

	sizeF = 200;
	this->Split(v1a, 0, end, 80);
	this->Split(v1b, 0, end, -80);

	// check de la conformité du split
	// sinon recalc de l'un de l'autre ou des deux
	// espace min
	if (0)
		for (i = 0; i < 40; i++)
		{
			if (v1a[i].sx > v1b[i].sx)
			{
				float fTemp = v1a[i].sx;
				v1a[i].sx = v1b[i].sx;
				v1b[i].sx = fTemp;
			}

			if (v1a[i].sz > v1b[i].sz)
			{
				float fTemp = v1a[i].sz;
				v1a[i].sz = v1b[i].sz;
				v1b[i].sz = fTemp;
			}

			if ((v1b[i].sx - v1a[i].sx) > 20)
			{
				v1b[i].sx = v1a[i].sx + rnd() * 20.0f;
			}

			if ((v1b[i].sz - v1a[i].sz) > 20)
			{
				v1b[i].sz = v1a[i].sz + rnd() * 20.0f;
			}
		}

	for (i = 0; i <= end; i++)
	{
		va[i].sx = eSrc.x;
		va[i].sy = eSrc.y;
		va[i].sz = eSrc.z;
		vb[i].sx = eSrc.x;
		vb[i].sy = eSrc.y;
		vb[i].sz = eSrc.z;
	}

	sizeF = 0;


	//cailloux					
	this->timestone = 0;
	this->nbstone = 0;
	this->stone[0] = stone0; 
	this->stone[1] = stone1; 

	int nb = 256;

	while (nb--)
	{
		this->tstone[nb].actif = 0;
	}
}

unsigned long CRiseDead::GetDuration()
{
	return (ulDurationIntro + ulDurationRender + ulDurationOuttro);
}
/*--------------------------------------------------------------------------*/
void CRiseDead::AddStone(EERIE_3D * pos)
{
	if (ARXPausedTimer) return;

	if (this->nbstone > 255) return;

	int	nb = 256;

	while (nb--)
	{
		if (!this->tstone[nb].actif)
		{
			this->nbstone++;

			this->tstone[nb].actif = 1;
			this->tstone[nb].numstone = rand() & 1;
			this->tstone[nb].pos = *pos;
			this->tstone[nb].yvel = rnd() * -5.f;
			this->tstone[nb].ang.a = rnd() * 360.f;
			this->tstone[nb].ang.b = rnd() * 360.f;
			this->tstone[nb].ang.g = rnd() * 360.f;
			this->tstone[nb].angvel.a = 5.f * rnd();
			this->tstone[nb].angvel.b = 6.f * rnd();
			this->tstone[nb].angvel.g = 3.f * rnd();
			float a = 0.2f + rnd() * 0.3f;
			this->tstone[nb].scale.x = a;
			this->tstone[nb].scale.y = a;
			this->tstone[nb].scale.z = a;
			this->tstone[nb].time = 2000 + (int)(500.f * rnd());
			this->tstone[nb].currtime = 0;
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/
void CRiseDead::DrawStone(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	int	nb = 256;
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_INVDESTCOLOR);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(_pD3DDevice, TRUE);

	while (nb--)
	{
		if (this->tstone[nb].actif)
		{
			float a = (float)this->tstone[nb].currtime / (float)this->tstone[nb].time;

			if (a > 1.f)
			{
				a = 1.f;
				this->tstone[nb].actif = 0;
			}

			int col = RGBA_MAKE(255, 255, 255, (int)(255.f * (1.f - a)));
			DrawEERIEObjExEx(_pD3DDevice, this->stone[this->tstone[nb].numstone], &this->tstone[nb].ang, &this->tstone[nb].pos, &this->tstone[nb].scale, col);

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov = this->tstone[nb].pos;
				particle[j].move.x = 0.f;
				particle[j].move.y = 3.f * rnd();
				particle[j].move.z = 0.f;
				particle[j].siz = 3.f + 3.f * rnd();
				particle[j].tolive = 1000;
				particle[j].scale.x = 1.f;
				particle[j].scale.y = 1.f;
				particle[j].scale.z = 1.f;
				particle[j].timcreation = -(long)(ARXTime + 1000); 
				particle[j].tc = NULL;
				particle[j].special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam = 0.0000001f;
				particle[j].r = 1.f;
				particle[j].g = 1.f;
				particle[j].b = 1.f;
			}


			//update mvt
			if (!ARXPausedTimer)
			{
				a = (((float)this->currframetime) * 100.f) / (float)this->tstone[nb].time;
				this->tstone[nb].pos.y += this->tstone[nb].yvel * a;
				this->tstone[nb].ang.a += this->tstone[nb].angvel.a * a;
				this->tstone[nb].ang.b += this->tstone[nb].angvel.b * a;
				this->tstone[nb].ang.g += this->tstone[nb].angvel.g * a;

				this->tstone[nb].yvel *= 1.f - (1.f / 100.f);

				this->tstone[nb].currtime += this->currframetime;
			}
		}
	}

	SETALPHABLEND(_pD3DDevice, FALSE);
}

//-----------------------------------------------------------------------------
void CRiseDead::Split(D3DTLVERTEX * v, int a, int b, float yo)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].sx = (v[a].sx + v[b].sx) * 0.5f + yo * frand2() * fBetaRadCos;
			v[i].sy = v[0].sy;// + (i+1)*5;
			v[i].sz = (v[a].sz + v[b].sz) * 0.5f + yo * frand2() * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

//-----------------------------------------------------------------------------
void CRiseDead::RenderFissure(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i;
	float ff;
	D3DTLVERTEX vt[4];
	D3DTLVERTEX vr[4];
	D3DTLVERTEX target;

	EERIE_3D etarget;
	etarget.x = fBetaRadCos;
	etarget.y = 0;
	etarget.z = fBetaRadSin;

	//-------------------------------------------------------------------------
	// computation des sommets
	float fTempCos, fTempSin;

	for (i = 0; i <= min(end, fSizeIntro); i++)
	{
		if (i <= end * 0.5f)
		{
			ff = i / (end * 0.5f);
		}
		else
		{
			ff = 1.0f - ((i - (end + 1) * 0.5f) / (end * 0.5f));
		}

		fTempCos = ff * fBetaRadCos;
		fTempSin = ff * fBetaRadSin;

		va[i].sx   = v1a[i].sx   + sizeF * fTempCos;
		va[i].sy   = v1a[i].sy;
		va[i].sz   = v1a[i].sz   + sizeF * fTempSin;

		vb[i].sx   = v1b[i].sx   - sizeF * fTempCos;
		vb[i].sy   = v1b[i].sy;
		vb[i].sz   = v1b[i].sz   - sizeF * fTempSin;

		va[i].sx += rnd() * 0.5f * fTempCos;
		va[i].sz += rnd() * 0.5f * fTempSin;
		vb[i].sx -= rnd() * 0.5f * fTempCos;
		vb[i].sz -= rnd() * 0.5f * fTempSin;
	}

	//-------------------------------------------------------------------------
	// rendu de la fissure
	SETALPHABLEND(m_pd3dDevice, FALSE);
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = D3DRGB(0, 0, 0);

	if (bIntro)
	{
		for (i = 0; i < min(end, fSizeIntro); i++)
		{
			EE_RT2(&v1a[i], &vr[0]);
			EE_RT2(&v1b[i], &vr[1]);
			EE_RT2(&v1a[i+1], &vr[2]);
			EE_RT2(&v1b[i+1], &vr[3]);
			ARX_DrawPrimitive_SoftClippZ(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive_SoftClippZ(&vr[1],
			                             &vr[2],
			                             &vr[3]);

		}
	}
	else
	{
		for (i = 0; i < min(end, fSizeIntro); i++)
		{
			EE_RT2(&va[i], &vr[0]);
			EE_RT2(&vb[i], &vr[1]);
			EE_RT2(&va[i+1], &vr[2]);
			EE_RT2(&vb[i+1], &vr[3]);
			ARX_DrawPrimitive_SoftClippZ(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive_SoftClippZ(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	SETALPHABLEND(m_pd3dDevice, TRUE);
	vr[0].color = vr[1].color = D3DRGB(0, 0, 0);
	vr[2].color = vr[3].color = D3DRGB(fColorBorder[0], fColorBorder[1], fColorBorder[2]);

	for (i = 0; i < min(end, fSizeIntro); i++)
	{
		vt[2].sx = va[i].sx   - (va[i].sx - eSrc.x) * 0.2f;
		vt[2].sy = va[i].sy   - (va[i].sy - eSrc.y) * 0.2f;
		vt[2].sz = va[i].sz   - (va[i].sz - eSrc.z) * 0.2f;
		vt[3].sx = va[i+1].sx - (va[i+1].sx - eSrc.x) * 0.2f;
		vt[3].sy = va[i+1].sy - (va[i+1].sy - eSrc.y) * 0.2f;
		vt[3].sz = va[i+1].sz - (va[i+1].sz - eSrc.z) * 0.2f;
		
		EE_RT2(&vt[3], &vr[0]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&va[i+1], &vr[2]);
		EE_RT2(&va[i], &vr[3]);
		ARX_DrawPrimitive_SoftClippZ(&vr[0],
		                             &vr[1],
		                             &vr[2]);
		ARX_DrawPrimitive_SoftClippZ(&vr[1],
		                             &vr[2],
		                             &vr[3]);

		vt[2].sx = vb[i].sx   - (vb[i].sx - eSrc.x) * 0.2f;
		vt[2].sy = vb[i].sy   - (vb[i].sy - eSrc.y) * 0.2f;
		vt[2].sz = vb[i].sz   - (vb[i].sz - eSrc.z) * 0.2f;
		vt[3].sx = vb[i+1].sx - (vb[i+1].sx - eSrc.x) * 0.2f;
		vt[3].sy = vb[i+1].sy - (vb[i+1].sy - eSrc.y) * 0.2f;
		vt[3].sz = vb[i+1].sz - (vb[i+1].sz - eSrc.z) * 0.2f;
		
		EE_RT2(&vb[i], &vr[3]);
		EE_RT2(&vb[i+1], &vr[2]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&vt[3], &vr[0]);
		ARX_DrawPrimitive_SoftClippZ(&vr[0],
		                             &vr[1],
		                             &vr[2]);
		ARX_DrawPrimitive_SoftClippZ(&vr[1],
		                             &vr[2],
		                             &vr[3]);
	}

	//-------------------------------------------------------------------------
	// rendu des faisceaux
	// blend additif ou mul
	// smooth sur les cotés ou pas ..
	// texture sympa avec glow au milieu ou uv wrap
	SETALPHABLEND(m_pd3dDevice, TRUE);

	if (tex_light && tex_light->m_pddsSurface)
	{
		SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_MIRROR);
		SETTC(m_pd3dDevice, tex_light);
	}

	target.sx = eSrc.x ;
	target.sy = eSrc.y + 1.5f * sizeF; 
	target.sz = eSrc.z ;

	EE_RTP(&vt[1], &vr[0]);
	vr[0].color = D3DRGB(fColorRays1[0], fColorRays1[1], fColorRays1[2]);
	vr[1].color = D3DRGB(fColorRays1[0], fColorRays1[1], fColorRays1[2]);
	vr[2].color = D3DRGB(fColorRays2[0], fColorRays2[1], fColorRays2[2]);
	vr[3].color = D3DRGB(fColorRays2[0], fColorRays2[1], fColorRays2[2]);

	vr[0].tu = fTexWrap;
	vr[0].tv = 1;
	vr[1].tu = 1 + fTexWrap;
	vr[1].tv = 1;
	vr[2].tu = fTexWrap;
	vr[2].tv = 0;
	vr[3].tu = 1 + fTexWrap;
	vr[3].tv = 0;

	for (i = 0; i < end - 1; i++)
	{
		float t = rnd();

		if (t <= 0.15f)
		{
			if (tfRaysa[i] < 1.0f)
				tfRaysa[i] += 0.02f;

			if (tfRaysa[i+1] < 1.0f)
				tfRaysa[i+1] += 0.01f;

			if (tfRaysa[i] > 1.0f)
				tfRaysa[i] = 1.0f;

			if (tfRaysa[i+1] > 1.0f)
				tfRaysa[i+1] = 1.0f;
		}

		if (t >= 0.9f)
		{
			if (tfRaysa[i] > 0.0f)
				tfRaysa[i] -= 0.02f;

			if (tfRaysa[i+1] > 0.0f)
				tfRaysa[i+1] -= 0.01f;

			if (tfRaysa[i] < 0.0f)
				tfRaysa[i] = 0.0f;

			if (tfRaysa[i+1] < 0.0f)
				tfRaysa[i+1] = 0.0f;
		}

		float t2 = rnd();

		if (t2 <= 0.15f)
		{
			if (tfRaysb[i] < 1.0f)
				tfRaysb[i] += 0.02f;

			if (tfRaysb[i+1] < 1.0f)
				tfRaysb[i+1] += 0.01f;

			if (tfRaysb[i] > 1.0f)
				tfRaysb[i] = 1.0f;

			if (tfRaysb[i+1] > 1.0f)
				tfRaysb[i+1] = 1.0f;
		}

		if (t2 >= 0.9f)
		{
			if (tfRaysb[i] > 0.0f)
				tfRaysb[i] -= 0.02f;

			if (tfRaysb[i+1] > 0.0f)
				tfRaysb[i+1] -= 0.01f;

			if (tfRaysb[i] < 0.0f)
				tfRaysb[i] = 0.0f;

			if (tfRaysb[i+1] < 0.0f)
				tfRaysb[i+1] = 0.0f;
		}
		
		if (i < fSizeIntro)
		{
			vt[0].sx = va[i].sx;
			vt[0].sy = va[i].sy;
			vt[0].sz = va[i].sz;
			vt[1].sx = va[i+1].sx;
			vt[1].sy = va[i+1].sy;
			vt[1].sz = va[i+1].sz;
			vt[2].sx = va[i].sx ;
			vt[2].sy = va[i].sy + (va[i].sy - target.sy) * 2;
			vt[2].sz = va[i].sz ;
			vt[3].sx = va[i+1].sx ;
			vt[3].sy = va[i+1].sy + (va[i+1].sy - target.sy) * 2;
			vt[3].sz = va[i+1].sz ;

			vr[0].color = D3DRGB(tfRaysa[i] * fColorRays1[0],   tfRaysa[i] * fColorRays1[1],   tfRaysa[i] * fColorRays1[2]);
			vr[1].color = D3DRGB(tfRaysa[i+1] * fColorRays1[0], tfRaysa[i+1] * fColorRays1[1], tfRaysa[i+1] * fColorRays1[2]);
			vr[2].color = D3DRGB(tfRaysa[i] * fColorRays2[0],   tfRaysa[i] * fColorRays2[1],   tfRaysa[i] * fColorRays2[2]);
			vr[3].color = D3DRGB(tfRaysa[i+1] * fColorRays2[0], tfRaysa[i+1] * fColorRays2[1], tfRaysa[i+1] * fColorRays2[2]);
			
			EE_RT2(&vt[0], &vr[0]);
			EE_RT2(&vt[1], &vr[1]);
			EE_RT2(&vt[2], &vr[2]);
			EE_RT2(&vt[3], &vr[3]);
			ARX_DrawPrimitive_SoftClippZ(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive_SoftClippZ(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
		
		if (i < fSizeIntro)
		{
			vt[0].sx = vb[i+1].sx;
			vt[0].sy = vb[i+1].sy;
			vt[0].sz = vb[i+1].sz;
			vt[1].sx = vb[i].sx;
			vt[1].sy = vb[i].sy;
			vt[1].sz = vb[i].sz;
			vt[2].sx = vb[i+1].sx ;
			vt[2].sy = vb[i+1].sy + (vb[i+1].sy - target.sy) * 2;
			vt[2].sz = vb[i+1].sz ;
			vt[3].sx = vb[i].sx ;
			vt[3].sy = vb[i].sy + (vb[i].sy - target.sy) * 2;
			vt[3].sz = vb[i].sz ;

			vr[0].color = D3DRGB(tfRaysb[i] * fColorRays1[0],   tfRaysb[i] * fColorRays1[1],   tfRaysb[i] * fColorRays1[2]);
			vr[1].color = D3DRGB(tfRaysb[i+1] * fColorRays1[0], tfRaysb[i+1] * fColorRays1[1], tfRaysb[i+1] * fColorRays1[2]);
			vr[2].color = D3DRGB(tfRaysb[i] * fColorRays2[0],   tfRaysb[i] * fColorRays2[1],   tfRaysb[i] * fColorRays2[2]);
			vr[3].color = D3DRGB(tfRaysb[i+1] * fColorRays2[0], tfRaysb[i+1] * fColorRays2[1], tfRaysb[i+1] * fColorRays2[2]);

			EE_RT2(&vt[0], &vr[0]);
			EE_RT2(&vt[1], &vr[1]);
			EE_RT2(&vt[2], &vr[2]);
			EE_RT2(&vt[3], &vr[3]);
			ARX_DrawPrimitive_SoftClippZ(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive_SoftClippZ(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
		
	}
}

//-----------------------------------------------------------------------------
void CRiseDead::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	currframetime = _ulTime;

	if (!ARXPausedTimer) this->timestone -= _ulTime;
}

//-----------------------------------------------------------------------------
// rendu de la déchirure spatio temporelle
float CRiseDead::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (ulCurrentTime >= (ulDurationIntro + ulDurationRender + ulDurationOuttro)) return 0.f;

	SETTC(m_pd3dDevice, NULL);
	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);

	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_CLAMP);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//-------------------------------------------------------------------------

	if (fTexWrap >= 1.0f)
	{
		fTexWrap -= 1.0f;
	}

	//-------------------------------------------------------------------------
	// render intro (opening + rays)
	if (ulCurrentTime < ulDurationIntro)
	{
		if (ulCurrentTime < ulDurationIntro * 0.666f)
		{
			fSizeIntro = (end + 2) * fOneOnDurationIntro * (1.5f) * ulCurrentTime;
			sizeF = 1;
		}
		else
		{
			if (bIntro != false)
				bIntro = false;

			sizeF = (iSize) * (fOneOnDurationIntro * 3) * (ulCurrentTime - ulDurationIntro * 0.666f);
		}
	}
	// do nothing just render
	else if (ulCurrentTime < (ulDurationIntro + ulDurationRender))
	{
	}
	// close it all
	else if (ulCurrentTime < (ulDurationIntro + ulDurationRender + ulDurationOuttro))
	{
		//if (sizeF > 0)
		{
			sizeF = iSize - (iSize) * fOneOnDurationOuttro * (ulCurrentTime - (ulDurationIntro + ulDurationRender));
		}
	}

	SETALPHABLEND(m_pd3dDevice, FALSE);
	RenderFissure(m_pd3dDevice);

	//cailloux
	if (this->timestone <= 0)
	{
		this->timestone = 50 + (int)(rnd() * 100.f);
		EERIE_3D	pos;
		float r = 80.f * frand2();
		pos.x = this->eSrc.x + r;
		pos.y = this->eSrc.y;
		pos.z = this->eSrc.z + r;
		this->AddStone(&pos);
	}

	SETZWRITE(m_pd3dDevice, TRUE);
	SETALPHABLEND(m_pd3dDevice, TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->DrawStone(m_pd3dDevice);
	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_WRAP);
	SETALPHABLEND(m_pd3dDevice, FALSE);
	SETZWRITE(m_pd3dDevice, TRUE);
	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	return (fSizeIntro / end);
}

//-----------------------------------------------------------------------------
// PARALYSE
//-----------------------------------------------------------------------------
CParalyse::CParalyse()
{
	prismd3d = NULL;
	prismvertex = NULL;
	prismind = NULL;
}

//-----------------------------------------------------------------------------
CParalyse::~CParalyse()
{
	Kill();
}

//-----------------------------------------------------------------------------
void CParalyse::Kill()
{
	if (prismd3d)
	{
		delete [] prismd3d;
		prismd3d = NULL;
	}

	if (prismvertex)
	{
		delete [] prismvertex;
		prismvertex = NULL;
	}

	if (prismind)
	{
		delete [] prismind;
		prismind = NULL;
	}

	for (int i = 0; i < 100; i++)
	{
		if (tabprism[i].vertex)
		{
			delete [] tabprism[i].vertex;
			tabprism[i].vertex = NULL;
		}
	}

	if (lLightId >= 0)
	{
		DynLight[lLightId].exist = 0;
		lLightId = -1;
	}
}

//-----------------------------------------------------------------------------
void CParalyse::CreatePrismTriangleList(float arayon, float ahcapuchon, float ahauteur, int adef)
{
	float		a, da;
	int			nb;
	EERIE_3D	* v;

	a = 0.f;
	da = 360.f / (float)adef;
	v = prismvertex;
	nb = adef;

	v->x = 0.f;
	v->y = -ahcapuchon;
	v->z = 0.f;
	v++;

	while (nb)
	{
		v->x = arayon * EEcos(DEG2RAD(a));
		v->y = -ahauteur + rnd() * ahauteur * 0.2f;
		v->z = arayon * EEsin(DEG2RAD(a));
		v++;
		v->x = (v - 1)->x * 0.90f;
		v->y = 0.f;
		v->z = (v - 1)->z * 0.90f;
		v++;
		a += da;
		nb--;
	}

	unsigned short * pind, ind = 1;
	pind = this->prismind;
	nb = adef - 1;

	while (nb)
	{
		*pind++ = 0;
		*pind++ = ind;
		*pind++ = ind + 2;
		ind += 2;
		nb--;
	}

	*pind++ = 0;
	*pind++ = ind;
	*pind++ = 1;

	ind = 1;
	nb = adef - 1;

	while (nb)
	{
		*pind++ = ind++;
		*pind++ = ind++;
		*pind++ = ind;

		*pind++ = ind;
		*pind++ = ind - 1;
		*pind++ = ind + 1;
		nb--;
	}

	*pind++ = ind++;
	*pind++ = ind;
	*pind++ = 1;

	*pind++ = 1;
	*pind++ = ind;
	*pind++ = 2;
}

//-----------------------------------------------------------------------------
void CParalyse::CreateLittlePrismTriangleList()
{
	float		sc;
	float		beta, beta2, gamma;
	EERIE_3D	* v, *vd;
	EERIE_3D	vt;

	for (int i = 0; i < 50; i++)
	{
		
		beta = 0;
		float g = frand2();
		g = 20 + rnd() * 45.f; 
		gamma = DEG2RAD(g);
		//xz
		beta2 = DEG2RAD(rnd() * 360); 

		v = prismvertex;
		vd = tabprism[i].vertex;

		float randd = rnd() * 360.f;
		float fd = i * 3.0f;

		if (fd < 40 || fd > 120)
		{
			fd = 40 + rnd() * 80.0f;
		}

		fd = max(fd, 40);
		fd = min(fd, 120);
		tabprism[i].pos.x = pos.x + EEsin(randd) * fd;
		tabprism[i].pos.y = pos.y;
		tabprism[i].pos.z = pos.z + EEcos(randd) * fd;

		for (int j = 0; j < prismnbpt; j++)
		{
			sc = 0.2f + rnd() * 0.8f;

			vt.x = v->x * sc * .85f;
			vt.y = v->y * sc;
			vt.z = v->z * sc * .85f;

			vd->x = vt.x ;
			float fy = (1 - i * DIV20);
			fy = max(fy, 0.2f);
			fy = min(fy, 0.7f);
			vd->y = vt.y * fy;
			vd->z = vt.z ;

			v++;
			vd++;
		}

		v = prismvertex;
		float t = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
		t = 1 / t;
		vt.x = v->x * t;
		vt.y = v->y * t;
		vt.z = v->z * t;
		tabprism[i].offset.x = r * vt.x;
		tabprism[i].offset.y = 0;
		tabprism[i].offset.z = r * vt.z;
	}
}

//-----------------------------------------------------------------------------
//!!!!!!! def non impair
void CParalyse::Create(int adef, float arayon, float ahcapuchon, float ahauteur, EERIE_3D * aePos, int aduration)
{
	if (adef < 3) return;

	key = -1;
	pos = *aePos;
	currduration = colduration = 0;
	duration = aduration;
	prisminterpcol = 0.f;
	r = arayon;

	SetColPrismDep(128.f, 128.f, 128.f);
	SetColPrismEnd(100.f, 100.f, 128.f);

	prismnbpt = 1 + (adef << 1);
	prismnbface = adef + (adef << 1);
	prismd3d = new D3DTLVERTEX[prismnbpt];
	prismvertex = new EERIE_3D[prismnbpt];
	prismind = new unsigned short [prismnbface*3];

	for (int i = 0; i < 100; i++)
	{
		tabprism[i].vertex = new EERIE_3D[prismnbpt];
	}

	tex_prism = MakeTCFromFile("Graph\\Obj3D\\Textures\\(FX)_paralyze.bmp");
	tex_p	  = MakeTCFromFile("Graph\\Particles\\missile.bmp");
	tex_p1	  = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");
	tex_p2	  = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_bluepouf.bmp");

	CreatePrismTriangleList(arayon, ahcapuchon, ahauteur, adef);
	CreateLittlePrismTriangleList();

	if (lLightId >= 0)
	{
		int id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 1.4f + 4.f * rnd();
		DynLight[id].fallend = r * 3.f;
		DynLight[id].fallstart = r * 3.f * .75f;
		DynLight[id].rgb.r = (prismrd * 2.f) / 255.f;
		DynLight[id].rgb.g = (prismgd * 2.f) / 255.f;
		DynLight[id].rgb.b = (prismbd * 2.f) / 255.f;
		DynLight[id].pos.x = pos.x;
		DynLight[id].pos.y = pos.y - ahcapuchon * .5f;
		DynLight[id].pos.z = pos.z;
	}


	// système de partoches pour la poussière
	CParticleSystem * pPS = new CParticleSystem();
	CParticleParams cp;
	cp.iNbMax = 200;
	cp.fLife = 500; //2000
	cp.fLifeRandom = 900;
	cp.p3Pos.x = 20;
	cp.p3Pos.y = 80;
	cp.p3Pos.z = 20;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(360);
	cp.fSpeed = 10;
	cp.fSpeedRandom = 10;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;

	cp.fStartSize = 0;
	cp.fStartSizeRandom = 1; 
	cp.fStartColor[0] = 20;
	cp.fStartColor[1] = 20;
	cp.fStartColor[2] = 20;
	cp.fStartColor[3] = 50;
	cp.fStartColorRandom[0] = 0;
	cp.fStartColorRandom[1] = 0;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 50;

	cp.fEndSize = 1; 
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 20;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 20;
	cp.fEndColor[3] = 10;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 0;

	cp.iBlendMode = 5;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	pPS->SetTexture("graph\\particles\\lil_greypouf.bmp", 0, 200);

	EERIE_3D ep;
	ep.x = aePos->x;
	ep.y = aePos->y - 80;
	ep.z = aePos->z;
	pPS->SetPos(ep);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	}

	// système de partoches pour la poussière au sol
	pPS = new CParticleSystem();
	cp;
	cp.iNbMax = 20;
	cp.fLife = 1000; //2000
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 20;
	cp.p3Pos.y = 5;
	cp.p3Pos.z = 20;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(144);
	cp.fSpeed = 10;
	cp.fSpeedRandom = 10;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = -4;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;

	cp.fStartSize = 2;
	cp.fStartSizeRandom = 2; 
	cp.fStartColor[0] = 25;
	cp.fStartColor[1] = 25;
	cp.fStartColor[2] = 25;
	cp.fStartColor[3] = 0;
	cp.fStartColorRandom[0] = 25;
	cp.fStartColorRandom[1] = 25;
	cp.fStartColorRandom[2] = 25;
	cp.fStartColorRandom[3] = 25;
	cp.bStartLock = true;

	cp.fEndSize = 5;
	cp.fEndSizeRandom = 10;
	cp.fEndColor[0] = 7;
	cp.fEndColor[1] = 7;
	cp.fEndColor[2] = 7;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 27;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 0;
	cp.bEndLock = true;
	cp.iBlendMode = 5;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	pPS->SetTexture("graph\\particles\\lil_greypouf.bmp", 0, 200);

	ep.x = aePos->x;
	ep.y = aePos->y - 10;
	ep.z = aePos->z;
	pPS->SetPos(ep);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	}
}

//-----------------------------------------------------------------------------
void CParalyse::Update(unsigned long aulTime)
{
	float a;

	switch (key)
	{
		case -1:
			a = (float)currduration / 200.f;

			if (a > 1.f)
			{
				a = 0.f;
				key++;
				currduration = 0;
			}

			scale = a;
			break;
		case 0:
			a = (float)currduration / 300.f;

			if (a > 1.f)
			{
				a = 1.f;
				key++;
				currduration = 0;
			}

			scale = a;
			break;
		case 1:
			a = (float)currduration / (float)duration;
			scale = a;

			if (a > 1.f)
			{
				key++;
			}

			prisminterpcol = (float)colduration / 1000.f;

			if (prisminterpcol > 1.f)
			{
				prisminterpcol = 0.f;
				colduration = 0;
				InversePrismCol();
			}

			break;
	}

	currduration += aulTime;
}

//-----------------------------------------------------------------------------
float CParalyse::Render(LPDIRECT3DDEVICE7 pD3DDevice)
{
	if (key > 1) return 0;

	SETALPHABLEND(pD3DDevice, TRUE);
	SETZWRITE(pD3DDevice, FALSE);

	pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	int			nb;
	EERIE_3D	* vertex;
	D3DTLVERTEX	* vd3d, d3ds;

	//uv
	vd3d = this->prismd3d;
	vd3d->tu = .5f + .25f;
	vd3d->tv = 0.999999f;
	vd3d++;
	nb = (this->prismnbpt - 1) >> 2;

	while (nb)
	{
		vd3d->tu = 0.5f;
		vd3d->tv = 0.f;
		vd3d++;
		vd3d->tu = (vd3d - 1)->tu;
		vd3d->tv = 0.99999f;
		vd3d++;
		vd3d->tu = 0.9999f;
		vd3d->tv = 0.f;
		vd3d++;
		vd3d->tu = (vd3d - 1)->tu;
		vd3d->tv = 0.99999f;
		vd3d++;

		nb--;
	}

	if (tex_prism) pD3DDevice->SetTexture(0, tex_prism->m_pddsSurface);
	else pD3DDevice->SetTexture(0, NULL);

	int	nb2 = 50;

	switch (this->key)
	{
		case -1:

			//calcul des chtis prism
			while (nb2--)
			{
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while (nb)
				{
					d3ds.sx = tabprism[nb2].pos.x + vertex->x * this->scale + this->tabprism[nb2].offset.x;
					d3ds.sy = tabprism[nb2].pos.y + vertex->y * this->scale + this->tabprism[nb2].offset.y;
					d3ds.sz = tabprism[nb2].pos.z + vertex->z * this->scale + this->tabprism[nb2].offset.z;
		
					EE_RTP(&d3ds, vd3d);
					vd3d->color = RGBA_MAKE(50, 50, 64, 255);
					vertex++;
					vd3d++;
					nb--;
				}

				SETCULL(pD3DDevice, D3DCULL_CW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
				SETCULL(pD3DDevice, D3DCULL_CCW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);

				vertex = this->tabprism[nb2].vertex;
				int j;
				j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					particle[j].ov.x 		=	this->pos.x + vertex->x * this->scale ;
					particle[j].ov.y 		=	this->pos.y + vertex->y * this->scale ;
					particle[j].ov.z 		=	this->pos.z + vertex->z * this->scale ;
					particle[j].move.x 		=	0.f;
					particle[j].move.y 		=	4.f * rnd();
					particle[j].move.z 		=	0.f;
					particle[j].siz 		=	10.f + 10.f * rnd();
					particle[j].tolive		=	500;
					particle[j].scale.x		=	1.f;
					particle[j].scale.y		=	1.f;
					particle[j].scale.z		=	1.f;
					particle[j].timcreation	=	lARXTime;
					particle[j].tc			=	tex_p;
					particle[j].special 	=	FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam		=	0.0000001f;
					particle[j].r			=	1.f;
					particle[j].g			=	1.f;
					particle[j].b			=	1.f;
				}
			}

			break;
		case 0:

			while (nb2--)
			{
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while (nb)
				{

					float px = tabprism[nb2].pos.x ;
					float py = tabprism[nb2].pos.y ;
					float pz = tabprism[nb2].pos.z ;

					d3ds.sx = px +  vertex->x + this->tabprism[nb2].offset.x;
					d3ds.sy = py +  vertex->y + this->tabprism[nb2].offset.y;
					d3ds.sz = pz +  vertex->z + this->tabprism[nb2].offset.z;
					EE_RTP(&d3ds, vd3d);
					vd3d->color = RGBA_MAKE(50, 50, 64, 255);
					vertex++;
					vd3d++;
					nb--;
				}

				SETCULL(pD3DDevice, D3DCULL_CW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
				SETCULL(pD3DDevice, D3DCULL_CCW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
			}

			vertex = this->prismvertex;
			vd3d = this->prismd3d;
			nb = this->prismnbpt;

			while (nb)
			{
				d3ds.sx = this->pos.x + vertex->x * this->scale;
				d3ds.sy = this->pos.y + vertex->y * this->scale;
				d3ds.sz = this->pos.z + vertex->z * this->scale;
				EE_RTP(&d3ds, vd3d);
				vd3d->color = ARX_OPAQUE_WHITE;
				vertex++;
				vd3d++;
				nb--;
			}

			vertex = this->prismvertex;
			int j;
			j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x 		=	this->pos.x + vertex->x * this->scale;
				particle[j].ov.y 		=	this->pos.y + vertex->y * this->scale;
				particle[j].ov.z 		=	this->pos.z + vertex->z * this->scale;
				particle[j].move.x 		=	0.f;
				particle[j].move.y 		=	4.f * rnd();
				particle[j].move.z 		=	0.f;
				particle[j].siz 		=	20.f + 20.f * rnd();
				particle[j].tolive		=	2000;
				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc			=	tex_p;
				particle[j].special 	=	FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam		=	0.0000001f;
				particle[j].r			=	1.f;
				particle[j].g			=	1.f;
				particle[j].b			=	1.f;
			}

			vertex++;
			nb = (this->prismnbpt - 1) >> 1;

			while (nb)
			{
				j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					particle[j].ov.x = this->pos.x + vertex->x * this->scale;
					particle[j].ov.y = this->pos.y + vertex->y * this->scale;
					particle[j].ov.z = this->pos.z + vertex->z * this->scale;
					particle[j].move.x = 0.f;
					particle[j].move.y = 8.f * rnd();
					particle[j].move.z = 0.f;
					particle[j].siz = 10.f + 10.f * rnd();
					particle[j].tolive = 1000;
					particle[j].scale.x = 1.f;
					particle[j].scale.y = 1.f;
					particle[j].scale.z = 1.f;
					particle[j].timcreation	=	lARXTime;
					particle[j].tc = tex_p;
					particle[j].special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam = 0.0000001f;
					particle[j].r = 1.f;
					particle[j].g = 1.f;
					particle[j].b = 1.f;
				}

				vertex += 2;
				nb--;
			}

			break;
		case 1:
			int col = RGBA_MAKE(((int)(this->prismrd + (this->prismre - this->prismrd) * this->prisminterpcol)) >> 1,
			                    ((int)(this->prismgd + (this->prismge - this->prismgd) * this->prisminterpcol)) >> 1,
			                    ((int)(this->prismbd + (this->prismbe - this->prismbd) * this->prisminterpcol)) >> 1,
			                    255);

			if (this->lLightId >= 0)
			{
				DynLight[this->lLightId].rgb.r = (((float)((col >> 16) & 0xFF)) * 1.9f) * DIV255;
				DynLight[this->lLightId].rgb.g = (((float)((col >> 8) & 0xFF)) * 1.9f) * DIV255;
				DynLight[this->lLightId].rgb.b = (((float)(col & 0xFF)) * 1.9f) * DIV255;
			}


			while (nb2--)
			{
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while (nb)
				{
					float px = tabprism[nb2].pos.x ;
					float py = tabprism[nb2].pos.y ;
					float pz = tabprism[nb2].pos.z ;

					d3ds.sx = px +  vertex->x + this->tabprism[nb2].offset.x;
					d3ds.sy = py +  vertex->y + this->tabprism[nb2].offset.y;
					d3ds.sz = pz +  vertex->z + this->tabprism[nb2].offset.z;

					EE_RTP(&d3ds, vd3d);
					vd3d->color = col;
					vertex++;
					vd3d++;
					nb--;
				}

				SETCULL(pD3DDevice, D3DCULL_CW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
				SETCULL(pD3DDevice, D3DCULL_CCW);
				pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
			}

			col = RGBA_MAKE((int)(this->prismrd + (this->prismre - this->prismrd) * this->prisminterpcol),
			                (int)(this->prismgd + (this->prismge - this->prismgd) * this->prisminterpcol),
			                (int)(this->prismbd + (this->prismbe - this->prismbd) * this->prisminterpcol),
			                255);

			vertex = this->prismvertex;
			vd3d = this->prismd3d;
			nb = this->prismnbpt;

			while (nb)
			{
				d3ds.sx = this->pos.x + vertex->x;
				d3ds.sy = this->pos.y + vertex->y;
				d3ds.sz = this->pos.z + vertex->z;
				EE_RTP(&d3ds, vd3d);
				vd3d->color = col;
				vertex++;
				vd3d++;
				nb--;
			}

			break;
	}

	SETCULL(pD3DDevice, D3DCULL_CW);
	pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);
	SETCULL(pD3DDevice, D3DCULL_CCW);
	pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->prismd3d, this->prismnbpt, this->prismind, this->prismnbface * 3, 0);


	for (int i = 0; i < 20; i++)
	{
		float t = rnd();

		if (t < 0.01f)
		{
			float x = this->pos.x;
			float y = this->pos.y;
			float z = this->pos.z;

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x = x + this->r * 2.0f * frand2();
				particle[j].ov.y = y + 5.f - rnd() * 10.f;
				particle[j].ov.z = z + this->r * 2.0f * frand2();
				particle[j].move.x = 2.f - 4.f * rnd();
				particle[j].move.y = 2.f - 4.f * rnd();
				particle[j].move.z = 2.f - 4.f * rnd();
				particle[j].siz = 20.f;


				float fMin = min(2000 + (rnd() * 2000.f), duration - currduration + 500.0f * rnd());
				ARX_CHECK_ULONG(fMin);

				particle[j].tolive = ARX_CLEAN_WARN_CAST_ULONG(fMin);


				particle[j].scale.x = 1.f;
				particle[j].scale.y = 1.f;
				particle[j].scale.z = 1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc = tex_p2;
				particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam = 0.0000001f;
				particle[j].r = 0.7f;
				particle[j].g = 0.7f;
				particle[j].b = 1.f;
			}
		}
		else if (t > 0.095f)
		{
			float x = this->pos.x;
			float y = this->pos.y - 50;
			float z = this->pos.z;

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x = x + this->r * 2.0f * frand2();
				particle[j].ov.y = y + 5.f - rnd() * 10.f;
				particle[j].ov.z = z + this->r * 2.0f * frand2();
				particle[j].move.x = 0;
				particle[j].move.y = 2.f - 4.f * rnd();
				particle[j].move.z = 0;
				particle[j].siz = 0.5f;


				float fMin = min(2000 + (rnd() * 2000.f), duration - currduration + 500.0f * rnd());
				ARX_CHECK_ULONG(fMin);

				particle[j].tolive = ARX_CLEAN_WARN_CAST_ULONG(fMin);


				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc			=	tex_p1;
				particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam		=	0.0000001f;
				particle[j].r			=	0.7f;
				particle[j].g			=	0.7f;
				particle[j].b			=	1.f;
			}
		}
	}

	pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	SETALPHABLEND(pD3DDevice, FALSE);
	SETZWRITE(pD3DDevice, TRUE);

	return 0;
}

//-----------------------------------------------------------------------------
// DISARM TRAP
//-----------------------------------------------------------------------------
CDisarmTrap::CDisarmTrap(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");
	
	if (!smotte)
	{
		smotte   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\Stalagmite\\motte.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(smotte);
	}

	smotte_count++;

	if (!slight)
	{
		slight = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard02.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(slight);
	}

	slight_count++; 

	if (!srune)
	{
		srune  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard03.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(srune);
	}

	srune_count++;
}
CDisarmTrap::~CDisarmTrap()
{
	smotte_count--;

	if (smotte && (smotte_count <= 0))
	{
		smotte_count = 0;
		ReleaseEERIE3DObj(smotte);
		smotte = NULL;
	}

	slight_count--;

	if (slight && (slight_count <= 0))
	{
		slight_count = 0;
		ReleaseEERIE3DObj(slight);
		slight = NULL;
	}

	srune_count--;

	if (srune && (srune_count <= 0))
	{
		srune_count = 0;
		ReleaseEERIE3DObj(srune);
		srune = NULL;
	}
}
//-----------------------------------------------------------------------------
void CDisarmTrap::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CDisarmTrap::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CDisarmTrap::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y;// + 100.0f;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}


	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);


	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			x = inter.iobj[i]->pos.x;
			y = inter.iobj[i]->pos.y;
			z = inter.iobj[i]->pos.z;
		}
	}

	//-------------------------------------------------------------------------
	if (tex_p2 && tex_p2->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_p2);
	}

	SETALPHABLEND(m_pd3dDevice, TRUE);

	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;

	stitecolor.r = 0.8f;
	stitecolor.g = 0.1f;
	stitecolor.b = 0.1f;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(m_pd3dDevice, srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

	return 1;
}

//-----------------------------------------------------------------------------
