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
// CSpellFx_Lvl09.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 09
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEMath.h>
#include <EERIEDraw.h>
#include <EERIEObject.h>

#include "ARX_Spells.h"
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl09.h"
#include "ARX_SpellFx_Lvl06.h"
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_Particles.h"
#include "ARX_Time.h"


#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)


//-----------------------------------------------------------------------------
CSummonCreature::CSummonCreature(LPDIRECT3DDEVICE7 m_pd3dDevice)
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

	tex_light = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu4.bmp");
}

//-----------------------------------------------------------------------------

void CSummonCreature::SetDuration(const unsigned long alDuration)
{
	ulDurationIntro			=	alDuration;

	if (ulDurationIntro <= 0)
	{
		ulDurationIntro	=	100;
	}
	else if (ulDurationIntro >= 100000)
	{
		ulDurationIntro	=	100000;
	}



	fOneOnDurationIntro		=	1.f / (float)(ulDurationIntro);

	ulDurationRender		=	1000;
	fOneOnDurationRender	=	1.f / (float)(ulDurationRender);

	ulDurationOuttro		=	1000;
	fOneOnDurationOuttro	=	1.f / (float)(ulDurationOuttro);

	ulCurrentTime			=	0;
}

//-----------------------------------------------------------------------------
void CSummonCreature::SetDuration(unsigned long alDurationIntro, unsigned long alDurationRender, unsigned long alDurationOuttro)
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
void CSummonCreature::SetColorBorder(float afRed, float afGreen, float afBlue)
{
	fColorBorder[0] = afRed;
	fColorBorder[1] = afGreen;
	fColorBorder[2] = afBlue;
}

//-----------------------------------------------------------------------------
void CSummonCreature::SetColorRays1(float afRed, float afGreen, float afBlue)
{
	fColorRays1[0] = afRed;
	fColorRays1[1] = afGreen;
	fColorRays1[2] = afBlue;
}

//-----------------------------------------------------------------------------
void CSummonCreature::SetColorRays2(float afRed, float afGreen, float afBlue)
{
	fColorRays2[0] = afRed;
	fColorRays2[1] = afGreen;
	fColorRays2[2] = afBlue;
}

//-----------------------------------------------------------------------------
unsigned long CSummonCreature::GetDuration()
{
	return (ulDurationIntro + ulDurationRender + ulDurationOuttro);
}

//-----------------------------------------------------------------------------
void CSummonCreature::Create(EERIE_3D aeSrc, float afBeta)
{
	int i;
	D3DTLVERTEX target;

	SetDuration(ulDurationIntro, ulDurationRender, ulDurationOuttro);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y - 50;
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

	target.sx = eSrc.x - 100;
	target.sy = eSrc.y;
	target.sz = eSrc.z;

	v1a[0].sx = eSrc.x;
	v1a[0].sy = eSrc.y - 100;
	v1a[0].sz = eSrc.z;
	v1a[end].sx = eSrc.x;
	v1a[end].sy = eSrc.y + 100;
	v1a[end].sz = eSrc.z;

	v1b[0].sx = v1a[0].sx;
	v1b[0].sy = v1a[0].sy;
	v1b[0].sz = v1a[0].sz;
	v1b[end].sx = v1a[end].sx;
	v1b[end].sy = v1a[end].sy;
	v1b[end].sz = v1a[end].sz;

	sizeF = 200;
	Split(v1a, 0, end, 20);
	Split(v1b, 0, end, -20);

	sizeF = 200;
	Split(v1a, 0, end, 80);
	Split(v1b, 0, end, -80);

	// check de la conformité du split
	// sinon recalc de l'un de l'autre ou des deux
	// espace min
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
}

//-----------------------------------------------------------------------------
void CSummonCreature::Split(D3DTLVERTEX * v, int a, int b, float yo)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].sx = (v[a].sx + v[b].sx) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadCos;
			v[i].sy = v[0].sy + (i + 1) * 5;
			v[i].sz = (v[a].sz + v[b].sz) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

//-----------------------------------------------------------------------------
void CSummonCreature::RenderFissure(LPDIRECT3DDEVICE7 m_pd3dDevice)
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

	target.sx = eSrc.x + -fBetaRadSin * (1.5f * sizeF); 
	target.sy = eSrc.y;
	target.sz = eSrc.z + fBetaRadCos * (1.5f * sizeF); 

	EE_RTP(&vt[1], &vr[0]);
	vr[0].color = D3DRGB(fColorRays1[0], fColorRays1[1], fColorRays1[2]);
	vr[1].color = D3DRGB(fColorRays1[0], fColorRays1[1], fColorRays1[2]);
	vr[2].color = D3DRGB(fColorRays2[0], fColorRays2[1], fColorRays2[2]);
	vr[3].color = D3DRGB(fColorRays2[0], fColorRays2[1], fColorRays2[2]);

	vr[0].tu = fTexWrap;
	vr[0].tv = 1;
	vr[1].tu = 1.0f + fTexWrap;
	vr[1].tv = 1;
	vr[2].tu = fTexWrap;
	vr[2].tv = 0;
	vr[3].tu = 1.0f + fTexWrap;
	vr[3].tv = 0;

	for (i = 0; i < end - 1; i++)
	{	
		if (i < fSizeIntro)
		{
			vt[0].sx = va[i].sx;
			vt[0].sy = va[i].sy;
			vt[0].sz = va[i].sz;
			vt[1].sx = va[i+1].sx;
			vt[1].sy = va[i+1].sy;
			vt[1].sz = va[i+1].sz;
			vt[2].sx = va[i].sx + (va[i].sx - target.sx) * 2;
			vt[2].sy = va[i].sy + (va[i].sy - target.sy) * 2;
			vt[2].sz = va[i].sz + (va[i].sz - target.sz) * 2;
			vt[3].sx = va[i+1].sx + (va[i+1].sx - target.sx) * 2;
			vt[3].sy = va[i+1].sy + (va[i+1].sy - target.sy) * 2;
			vt[3].sz = va[i+1].sz + (va[i+1].sz - target.sz) * 2;

			vr[0].color = D3DRGB(tfRaysa[i] * fColorRays1[0],   tfRaysa[i] * fColorRays1[1],   tfRaysa[i] * fColorRays1[2]);
			vr[1].color = D3DRGB(tfRaysa[i+1] * fColorRays1[0], tfRaysa[i+1] * fColorRays1[1], tfRaysa[i+1] * fColorRays1[2]);
			vr[2].color = D3DRGB(tfRaysa[i] * fColorRays2[0],   tfRaysa[i] * fColorRays2[1],   tfRaysa[i] * fColorRays2[2]);
			vr[3].color = D3DRGB(tfRaysa[i+1] * fColorRays2[0], tfRaysa[i+1] * fColorRays2[1], tfRaysa[i+1] * fColorRays2[2]);
			
			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
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
			vt[2].sx = vb[i+1].sx + (vb[i+1].sx - target.sx) * 2;
			vt[2].sy = vb[i+1].sy + (vb[i+1].sy - target.sy) * 2;
			vt[2].sz = vb[i+1].sz + (vb[i+1].sz - target.sz) * 2;
			vt[3].sx = vb[i].sx + (vb[i].sx - target.sx) * 2;
			vt[3].sy = vb[i].sy + (vb[i].sy - target.sy) * 2;
			vt[3].sz = vb[i].sz + (vb[i].sz - target.sz) * 2;

			vr[0].color = D3DRGB(tfRaysb[i] * fColorRays1[0],   tfRaysb[i] * fColorRays1[1],   tfRaysb[i] * fColorRays1[2]);
			vr[1].color = D3DRGB(tfRaysb[i+1] * fColorRays1[0], tfRaysb[i+1] * fColorRays1[1], tfRaysb[i+1] * fColorRays1[2]);
			vr[2].color = D3DRGB(tfRaysb[i] * fColorRays2[0],   tfRaysb[i] * fColorRays2[1],   tfRaysb[i] * fColorRays2[2]);
			vr[3].color = D3DRGB(tfRaysb[i+1] * fColorRays2[0], tfRaysb[i+1] * fColorRays2[1], tfRaysb[i+1] * fColorRays2[2]);

			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
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
void CSummonCreature::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
// rendu de la déchirure spatio temporelle
float CSummonCreature::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
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
	fTexWrap += 0.02f;

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

	SETZWRITE(m_pd3dDevice, TRUE);
	SETALPHABLEND(m_pd3dDevice, FALSE);
	SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_WRAP);

	return (fSizeIntro / end);
}

//-----------------------------------------------------------------------------
// INCINERATE
//-----------------------------------------------------------------------------
CIncinerate::~CIncinerate()
{
	sfirewave_count--;

	if (sfirewave && (sfirewave_count <= 0))
	{
		sfirewave_count = 0;
		ReleaseEERIE3DObj(sfirewave);
		sfirewave = NULL;
	}
}

//-----------------------------------------------------------------------------
void CIncinerate::Create(EERIE_3D _eSrc, float _fBeta, float _fLevel)
{
	iMax = (int)(30 + _fLevel * 5.2f);
	Create(_eSrc, _fBeta);
}

//-----------------------------------------------------------------------------
void CIncinerate::Create(EERIE_3D _eSrc, float _fBeta)
{
	SetDuration(ulDuration);
	SetAngle(_fBeta);

	eSrc.x = _eSrc.x;
	eSrc.y = _eSrc.y - 20;
	eSrc.z = _eSrc.z;

	eTarget.x = eSrc.x - fBetaRadSin * 500;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z + fBetaRadCos * 500;

	fSize = 1;

	int i = 0;
	iMax = iNumber;
	EERIE_3D s, e, h;

	s.x = eSrc.x;
	s.y = eSrc.y - 20;
	s.z = eSrc.z;
	e.x = eSrc.x;
	e.y = eSrc.y - 20;
	e.z = eSrc.z;

	e.x = s.x - fBetaRadSin * 900;
	e.y = s.y;
	e.z = s.z + fBetaRadCos * 900;

	float fd;
	i = iMax;

	if (!Visible(&s, &e, NULL, &h))
	{
		e.x = h.x + fBetaRadSin * 20;
		e.y = h.y;
		e.z = h.z - fBetaRadCos * 20;
	}

	fd = Distance3D(s.x, s.y, s.z, e.x, e.y, e.z);


	float fDur = ulDuration * (fd / 900.0f);
	ARX_CHECK_ULONG(fDur);

	SetDuration(ARX_CLEAN_WARN_CAST_ULONG(fDur));



	float fCalc = (fd / 900.0f) * iMax ;
	ARX_CHECK_INT(fCalc);

	i = ARX_CLEAN_WARN_CAST_INT(fCalc);


	iNumber = i;

	int end = 40;
	tv1a[0].sx = s.x;
	tv1a[0].sy = s.y;
	tv1a[0].sz = s.z;
	tv1a[end].sx = e.x;
	tv1a[end].sy = e.y;
	tv1a[end].sz = e.z;

	Split(tv1a, 0, end, 10, 1, 0, 1, 10, 1);

	CParticleParams cp;
	cp.iNbMax = 250; 
	cp.fLife = 1000;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 0;
	cp.p3Direction.x = + fBetaRadSin * 4;
	cp.p3Direction.y = 0;
	cp.p3Direction.z = - fBetaRadCos * 4;
	cp.fAngle = DEG2RAD(1);
	cp.fSpeed = 0;
	cp.fSpeedRandom = 20;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;
	cp.bRotationRandomDirection = false;
	cp.bRotationRandomStart = false;

	cp.fStartSize = 5;
	cp.fStartSizeRandom = 5; 
	cp.fStartColor[0] = 80;
	cp.fStartColor[1] = 80;
	cp.fStartColor[2] = 0;
	cp.fStartColor[3] = 20; 
	cp.fStartColorRandom[0] = 81;
	cp.fStartColorRandom[1] = 81;
	cp.fStartColorRandom[2] = 51;
	cp.fStartColorRandom[3] = 51;
	cp.bStartLock = true;

	cp.fEndSize = 1;
	cp.fEndSizeRandom = 5;
	cp.fEndColor[0] = 50;
	cp.fEndColor[1] = 0;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 20;
	cp.fEndColorRandom[0] = 50;
	cp.fEndColorRandom[1] = 50;
	cp.fEndColorRandom[2] = 50;
	cp.fEndColorRandom[3] = 20;
	cp.bEndLock = true;
	cp.bTexLoop = true;

	cp.iBlendMode = 3;

	pPSStream.SetParams(cp);
	pPSStream.ulParticleSpawn = 0;

	pPSStream.SetTexture("graph\\particles\\smallfire", 4, 10);

	pPSStream.fParticleFreq = 250;
	pPSStream.bParticleFollow = false;
	pPSStream.SetPos(eSrc);
	pPSStream.Update(0);



	// Hit
	cp.iNbMax = 150; 
	cp.fLife = 2000;
	cp.fLifeRandom = 1000;
	cp.p3Pos.x = 80;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 80;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = 2;
	cp.p3Direction.z = 0;
	cp.fAngle = 0;
	cp.fSpeed = 0;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;
	cp.bRotationRandomDirection = false;
	cp.bRotationRandomStart = false;

	cp.fStartSize = 10;
	cp.fStartSizeRandom = 3;
	cp.fStartColor[0] = 25;
	cp.fStartColor[1] = 25;
	cp.fStartColor[2] = 25;
	cp.fStartColor[3] = 50; 
	cp.fStartColorRandom[0] = 51;
	cp.fStartColorRandom[1] = 51;
	cp.fStartColorRandom[2] = 51;
	cp.fStartColorRandom[3] = 101;
	cp.bStartLock = false;

	cp.fEndSize = 10;
	cp.fEndSizeRandom = 3;
	cp.fEndColor[0] = 25;
	cp.fEndColor[1] = 25;
	cp.fEndColor[2] = 25;
	cp.fEndColor[3] = 50; //0
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 100; //0
	cp.bEndLock = false;
	cp.bTexLoop = true;

	cp.iBlendMode = 0;

	pPSHit.SetParams(cp);
	pPSHit.ulParticleSpawn = 0;

	pPSHit.SetTexture("graph\\particles\\firebase", 4, 100);

	pPSHit.fParticleFreq = -1;
	pPSHit.SetPos(eSrc);
	pPSHit.Update(0);

}

//---------------------------------------------------------------------
void CIncinerate::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	EERIE_3D et;

	pPSStream.Update(_ulTime);
	pPSHit.Update(_ulTime);

	iMax = (int)((40) * fOneOnDuration * ulCurrentTime); //*2

	if (iMax > 40) iMax = 40;

	et.x = tv1a[iMax].sx;
	et.y = tv1a[iMax].sy;
	et.z = tv1a[iMax].sz;
	pPSStream.SetPos(et);
}

//---------------------------------------------------------------------
float CIncinerate::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	int i = 0;

	SETCULL(_pD3DDevice, D3DCULL_NONE);
	SETZWRITE(_pD3DDevice, FALSE);
	SETALPHABLEND(_pD3DDevice, TRUE);
	
	iMax ++;
	float x = eSrc.x + (eTarget.x - eSrc.x) * (ulCurrentTime * fOneOnDuration);
	float y = eSrc.y + (eTarget.y - eSrc.y) * (ulCurrentTime * fOneOnDuration);
	float z = eSrc.z + (eTarget.z - eSrc.z) * (ulCurrentTime * fOneOnDuration);

	for (i = 0; i < 150 - 1; i++)
	{
		EERIE_3D s, d;
		s.x = tv1a[i].sx;
		s.y = tv1a[i].sy;
		s.z = tv1a[i].sz;
		d.x = tv1a[i+1].sx;
		d.y = tv1a[i+1].sy;
		d.z = tv1a[i+1].sz;
		EERIEDraw3DLine(_pD3DDevice, &s, &d, 0xFFFF0000);
	}

	SETALPHABLEND(_pD3DDevice, TRUE);

	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	stiteangle.b = 90 - fBeta;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;
	stitescale.x = 0.5f;
	stitescale.y = 0.5f;
	stitescale.z = 0.5f;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;

	SETCULL(_pD3DDevice, D3DCULL_NONE);
	SETZWRITE(_pD3DDevice, FALSE);
	SETALPHABLEND(_pD3DDevice, TRUE);

	pPSStream.Render(_pD3DDevice, D3DBLEND_ONE, D3DBLEND_ONE);

	return 1;
}

//-----------------------------------------------------------------------------
CNegateMagic::~CNegateMagic()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		ReleaseEERIE3DObj(ssol);
		ssol = NULL;
	}
}
CNegateMagic::CNegateMagic(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_bluepouf.bmp");
	tex_sol = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_negate_magic.bmp");

	if (!ssol)
	{
		ssol   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(ssol);
	}

	ssol_count++;
}

//-----------------------------------------------------------------------------
void CNegateMagic::Create(EERIE_3D aeSrc, float afBeta)
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

//-----------------------------------------------------------------------------
void CNegateMagic::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CNegateMagic::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	if (spells[spellinstance].caster == 0)
	{
		eSrc.x = player.pos.x;
		eSrc.y = player.pos.y + 170.f;
		eSrc.z = player.pos.z;
	}
	else
	{
		eSrc.x = inter.iobj[spells[spellinstance].caster]->pos.x;
		eSrc.y = inter.iobj[spells[spellinstance].caster]->pos.y;
		eSrc.z = inter.iobj[spells[spellinstance].caster]->pos.z;
	}

	float x = eSrc.x;
	float y = eSrc.y - 10.f;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	if (tex_sol && tex_sol->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_sol);
	}

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	fSize = ulCurrentTime * fOneOnDuration * 200;
 
	float size = 50;

	for (i = 0; i < 360; i++)
	{

		float t = rnd();

		if (t < 0.04f)
		{

			t = rnd();

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x		=	x + frand2() * 150.f;
				particle[j].ov.y		=	y;
				particle[j].ov.z		=	z + frand2() * 150.f;
				particle[j].move.x		=	0;
				particle[j].move.y		=	- 3.0f * rnd();
				particle[j].move.z		=	0;
				particle[j].siz			=	 0.3f;
				particle[j].tolive		=	2000 + (unsigned long)(float)(rnd() * 2000.f);
				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc			=	tex_p2;
				particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING | SUBSTRACT;
				particle[j].fparam		=	0.0000001f;
				particle[j].r			=	1.0f;
				particle[j].g			=	1.0f;
				particle[j].b			=	1.0f;
			}
		}
	}

	//----------------------------
	size = 100;

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

	SETALPHABLEND(m_pd3dDevice, TRUE);

	stiteangle.b = -stiteangle.b;
	stitecolor.r = 0.4f;
	stitecolor.g = 0.4f;
	stitecolor.b = 0.4f;
	stitescale.x = 3.f;
	stitescale.y = 3.f;
	stitescale.z = 3.f;
	DrawEERIEObjEx(m_pd3dDevice, ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	stitecolor.r = 0.5f;
	stitecolor.g = 0.f;
	stitecolor.b = 0.5f;
	stitescale.x = 3.1f;
	stitescale.y = 3.1f;
	stitescale.z = 3.1f;
	DrawEERIEObjEx(m_pd3dDevice, ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

	return 1;
}

//----------------------------------------------------------------------
// MASS PARALYSE
//----------------------------------------------------------------------

void CMassParalyse::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	if (ulCurrentTime < ulDuration)
	{
		int	nb = inter.nbmax, nb2;

		while (nb--)
		{
			if (inter.iobj[nb])
			{
				float d = Distance3D(ePos.x, ePos.y, ePos.z,
				                     inter.iobj[nb]->pos.x, inter.iobj[nb]->pos.y, inter.iobj[nb]->pos.z);

				if (d <= fRayon)
				{
					nb2 = iNbParalyse;

					while (nb2--)
					{
						if (tabparalyse[nb2].id == nb) break;
					}

					if (nb2 < 0)
					{
						tabparalyse[iNbParalyse].id = nb;
						tabparalyse[iNbParalyse].paralyse = new CParalyse();
						(tabparalyse[iNbParalyse].paralyse)->Create(4, 50, 200.f, 150.f, &inter.iobj[nb]->pos, 10000);
						iNbParalyse++;
					}
				}
			}
		}

		nb = iNbParalyse;

		while (nb--)
		{
			tabparalyse[nb].paralyse->Update(_ulTime);
		}
	}
}

//----------------------------------------------------------------------
float CMassParalyse::Render(LPDIRECT3DDEVICE7 device)
{
	if (ulCurrentTime > ulDuration) return 0;

	int	nb = iNbParalyse;

	while (nb--)
	{
		(tabparalyse[nb].paralyse)->Render(device);
	}

	return 0;
}
