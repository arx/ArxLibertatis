/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "graphics/spells/Spells09.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Spells.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells06.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

CSummonCreature::CSummonCreature() : fColorRays1(Color3f::white), fColorBorder(Color3f::white), fColorRays2(Color3f::black) {
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDurationIntro + ulDurationRender + ulDurationOuttro + 1;

	iSize = 100;
	fOneOniSize = 1.0f / ((float) iSize);

	tex_light = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu4");
}

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

void CSummonCreature::SetColorBorder(Color3f color) {
	fColorBorder = color;
}

void CSummonCreature::SetColorRays1(Color3f color) {
	fColorRays1 = color;
}

void CSummonCreature::SetColorRays2(Color3f color) {
	fColorRays2 = color;
}

unsigned long CSummonCreature::GetDuration() {
	return (ulDurationIntro + ulDurationRender + ulDurationOuttro);
}

//-----------------------------------------------------------------------------
void CSummonCreature::Create(Vec3f aeSrc, float afBeta)
{
	int i;
	TexturedVertex target;

	SetDuration(ulDurationIntro, ulDurationRender, ulDurationOuttro);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y - 50;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
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

	target.p.x = eSrc.x - 100;
	target.p.y = eSrc.y;
	target.p.z = eSrc.z;

	v1a[0].p.x = eSrc.x;
	v1a[0].p.y = eSrc.y - 100;
	v1a[0].p.z = eSrc.z;
	v1a[end].p.x = eSrc.x;
	v1a[end].p.y = eSrc.y + 100;
	v1a[end].p.z = eSrc.z;

	v1b[0].p.x = v1a[0].p.x;
	v1b[0].p.y = v1a[0].p.y;
	v1b[0].p.z = v1a[0].p.z;
	v1b[end].p.x = v1a[end].p.x;
	v1b[end].p.y = v1a[end].p.y;
	v1b[end].p.z = v1a[end].p.z;

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
		if (v1a[i].p.x > v1b[i].p.x)
		{
			float fTemp = v1a[i].p.x;
			v1a[i].p.x = v1b[i].p.x;
			v1b[i].p.x = fTemp;
		}

		if (v1a[i].p.z > v1b[i].p.z)
		{
			float fTemp = v1a[i].p.z;
			v1a[i].p.z = v1b[i].p.z;
			v1b[i].p.z = fTemp;
		}

		if ((v1b[i].p.x - v1a[i].p.x) > 20)
		{
			v1b[i].p.x = v1a[i].p.x + rnd() * 20.0f;
		}

		if ((v1b[i].p.z - v1a[i].p.z) > 20)
		{
			v1b[i].p.z = v1a[i].p.z + rnd() * 20.0f;
		}
	}

	for (i = 0; i <= end; i++)
	{
		va[i].p.x = eSrc.x;
		va[i].p.y = eSrc.y;
		va[i].p.z = eSrc.z;
		vb[i].p.x = eSrc.x;
		vb[i].p.y = eSrc.y;
		vb[i].p.z = eSrc.z;
	}

	sizeF = 0;
}

//-----------------------------------------------------------------------------
void CSummonCreature::Split(TexturedVertex * v, int a, int b, float yo)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].p.x = (v[a].p.x + v[b].p.x) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadCos;
			v[i].p.y = v[0].p.y + (i + 1) * 5;
			v[i].p.z = (v[a].p.z + v[b].p.z) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

//-----------------------------------------------------------------------------
void CSummonCreature::RenderFissure()
{
	int i;
	float ff;
	TexturedVertex vt[4];
	TexturedVertex vr[4];
	TexturedVertex target;

	Vec3f etarget;
	etarget.x = fBetaRadCos;
	etarget.y = 0;
	etarget.z = fBetaRadSin;

	//-------------------------------------------------------------------------
	// computation des sommets
	float fTempCos, fTempSin;

	for (i = 0; i <= std::min(end, int(fSizeIntro)); i++)
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

		va[i].p.x   = v1a[i].p.x   + sizeF * fTempCos;
		va[i].p.y   = v1a[i].p.y;
		va[i].p.z   = v1a[i].p.z   + sizeF * fTempSin;

		vb[i].p.x   = v1b[i].p.x   - sizeF * fTempCos;
		vb[i].p.y   = v1b[i].p.y;
		vb[i].p.z   = v1b[i].p.z   - sizeF * fTempSin;

		va[i].p.x += rnd() * 0.5f * fTempCos;
		va[i].p.z += rnd() * 0.5f * fTempSin;
		vb[i].p.x -= rnd() * 0.5f * fTempCos;
		vb[i].p.z -= rnd() * 0.5f * fTempSin;
	}

	//-------------------------------------------------------------------------
	// rendu de la fissure
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toBGR();

	if (bIntro)
	{
		for (i = 0; i < std::min(end, (int)fSizeIntro); i++)
		{
			EE_RT2(&v1a[i], &vr[0]);
			EE_RT2(&v1b[i], &vr[1]);
			EE_RT2(&v1a[i+1], &vr[2]);
			EE_RT2(&v1b[i+1], &vr[3]);
			ARX_DrawPrimitive(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
	}
	else
	{
		for (i = 0; i < std::min(end, (int)fSizeIntro); i++)
		{
			EE_RT2(&va[i], &vr[0]);
			EE_RT2(&vb[i], &vr[1]);
			EE_RT2(&va[i+1], &vr[2]);
			EE_RT2(&vb[i+1], &vr[3]);
			ARX_DrawPrimitive(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	vr[0].color = vr[1].color = Color::black.toBGR();
	vr[2].color = vr[3].color = fColorBorder.toBGR();

	for (i = 0; i < std::min(end, (int)fSizeIntro); i++)
	{
		vt[2].p.x = va[i].p.x   - (va[i].p.x - eSrc.x) * 0.2f;
		vt[2].p.y = va[i].p.y   - (va[i].p.y - eSrc.y) * 0.2f;
		vt[2].p.z = va[i].p.z   - (va[i].p.z - eSrc.z) * 0.2f;
		vt[3].p.x = va[i+1].p.x - (va[i+1].p.x - eSrc.x) * 0.2f;
		vt[3].p.y = va[i+1].p.y - (va[i+1].p.y - eSrc.y) * 0.2f;
		vt[3].p.z = va[i+1].p.z - (va[i+1].p.z - eSrc.z) * 0.2f;

		EE_RT2(&vt[3], &vr[0]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&va[i+1], &vr[2]);
		EE_RT2(&va[i], &vr[3]);
		ARX_DrawPrimitive(&vr[0],
		                             &vr[1],
		                             &vr[2]);
		ARX_DrawPrimitive(&vr[1],
		                             &vr[2],
		                             &vr[3]);

		vt[2].p.x = vb[i].p.x   - (vb[i].p.x - eSrc.x) * 0.2f;
		vt[2].p.y = vb[i].p.y   - (vb[i].p.y - eSrc.y) * 0.2f;
		vt[2].p.z = vb[i].p.z   - (vb[i].p.z - eSrc.z) * 0.2f;
		vt[3].p.x = vb[i+1].p.x - (vb[i+1].p.x - eSrc.x) * 0.2f;
		vt[3].p.y = vb[i+1].p.y - (vb[i+1].p.y - eSrc.y) * 0.2f;
		vt[3].p.z = vb[i+1].p.z - (vb[i+1].p.z - eSrc.z) * 0.2f;

		EE_RT2(&vb[i], &vr[3]);
		EE_RT2(&vb[i+1], &vr[2]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&vt[3], &vr[0]);
		ARX_DrawPrimitive(&vr[0],
		                             &vr[1],
		                             &vr[2]);
		ARX_DrawPrimitive(&vr[1],
		                             &vr[2],
		                             &vr[3]);

	}

	//-------------------------------------------------------------------------
	// rendu des faisceaux
	// blend additif ou mul
	// smooth sur les cotés ou pas ..
	// texture sympa avec glow au milieu ou uv wrap
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapMirror);
	GRenderer->SetTexture(0, tex_light);

	target.p.x = eSrc.x + -fBetaRadSin * (1.5f * sizeF); 
	target.p.y = eSrc.y;
	target.p.z = eSrc.z + fBetaRadCos * (1.5f * sizeF); 

	EE_RTP(&vt[1], &vr[0]);
	vr[0].color = vr[1].color = fColorRays1.toBGR();
	vr[2].color = vr[3].color = fColorRays2.toBGR();

	vr[0].uv.x = fTexWrap;
	vr[0].uv.y = 1;
	vr[1].uv.x = 1.0f + fTexWrap;
	vr[1].uv.y = 1;
	vr[2].uv.x = fTexWrap;
	vr[2].uv.y = 0;
	vr[3].uv.x = 1.0f + fTexWrap;
	vr[3].uv.y = 0;

	for (i = 0; i < end - 1; i++)
	{	
		if (i < fSizeIntro)
		{
			vt[0].p.x = va[i].p.x;
			vt[0].p.y = va[i].p.y;
			vt[0].p.z = va[i].p.z;
			vt[1].p.x = va[i+1].p.x;
			vt[1].p.y = va[i+1].p.y;
			vt[1].p.z = va[i+1].p.z;
			vt[2].p.x = va[i].p.x + (va[i].p.x - target.p.x) * 2;
			vt[2].p.y = va[i].p.y + (va[i].p.y - target.p.y) * 2;
			vt[2].p.z = va[i].p.z + (va[i].p.z - target.p.z) * 2;
			vt[3].p.x = va[i+1].p.x + (va[i+1].p.x - target.p.x) * 2;
			vt[3].p.y = va[i+1].p.y + (va[i+1].p.y - target.p.y) * 2;
			vt[3].p.z = va[i+1].p.z + (va[i+1].p.z - target.p.z) * 2;

			vr[0].color = (fColorRays1 * tfRaysa[i]).toBGR();
			vr[1].color = (fColorRays1 * tfRaysa[i + 1]).toBGR();
			vr[2].color = (fColorRays2 * tfRaysa[i]).toBGR();
			vr[3].color = (fColorRays2 * tfRaysa[i + 1]).toBGR();
			
			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
			ARX_DrawPrimitive(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive(&vr[1],
			                             &vr[2],
			                             &vr[3]);
		}
		
		if (i < fSizeIntro)
		{
			vt[0].p.x = vb[i+1].p.x;
			vt[0].p.y = vb[i+1].p.y;
			vt[0].p.z = vb[i+1].p.z;
			vt[1].p.x = vb[i].p.x;
			vt[1].p.y = vb[i].p.y;
			vt[1].p.z = vb[i].p.z;
			vt[2].p.x = vb[i+1].p.x + (vb[i+1].p.x - target.p.x) * 2;
			vt[2].p.y = vb[i+1].p.y + (vb[i+1].p.y - target.p.y) * 2;
			vt[2].p.z = vb[i+1].p.z + (vb[i+1].p.z - target.p.z) * 2;
			vt[3].p.x = vb[i].p.x + (vb[i].p.x - target.p.x) * 2;
			vt[3].p.y = vb[i].p.y + (vb[i].p.y - target.p.y) * 2;
			vt[3].p.z = vb[i].p.z + (vb[i].p.z - target.p.z) * 2;

			vr[0].color = (fColorRays1 * tfRaysb[i]).toBGR();
			vr[1].color = (fColorRays1 * tfRaysb[i + 1]).toBGR();
			vr[2].color = (fColorRays2 * tfRaysb[i]).toBGR();
			vr[3].color = (fColorRays2 * tfRaysb[i + 1]).toBGR();

			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
			ARX_DrawPrimitive(&vr[0],
			                             &vr[1],
			                             &vr[2]);
			ARX_DrawPrimitive(&vr[1],
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
float CSummonCreature::Render()
{
	if (ulCurrentTime >= (ulDurationIntro + ulDurationRender + ulDurationOuttro)) return 0.f;

	GRenderer->ResetTexture(0);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

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

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	RenderFissure();

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);

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
		delete sfirewave;
		sfirewave = NULL;
	}
}

//-----------------------------------------------------------------------------
void CIncinerate::Create(Vec3f _eSrc, float _fBeta, float _fLevel)
{
	iMax = (int)(30 + _fLevel * 5.2f);
	Create(_eSrc, _fBeta);
}

//-----------------------------------------------------------------------------
void CIncinerate::Create(Vec3f _eSrc, float _fBeta)
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

	iMax = iNumber;
	Vec3f s, e, h;

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

	if (!Visible(&s, &e, NULL, &h))
	{
		e.x = h.x + fBetaRadSin * 20;
		e.y = h.y;
		e.z = h.z - fBetaRadCos * 20;
	}

	fd = fdist(s, e);


	float fDur = ulDuration * (fd / 900.0f);
	SetDuration(checked_range_cast<unsigned long>(fDur));

	float fCalc = (fd / 900.0f) * iMax;

	iNumber = checked_range_cast<int>(fCalc);

	int end = 40;
	tv1a[0].p.x = s.x;
	tv1a[0].p.y = s.y;
	tv1a[0].p.z = s.z;
	tv1a[end].p.x = e.x;
	tv1a[end].p.y = e.y;
	tv1a[end].p.z = e.z;

	Split(tv1a, 0, end, 10, 1, 0, 1, 10, 1);

	ParticleParams cp;
	cp.iNbMax = 250; 
	cp.fLife = 1000;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 0;
	cp.p3Direction.x = + fBetaRadSin * 4;
	cp.p3Direction.y = 0;
	cp.p3Direction.z = - fBetaRadCos * 4;
	cp.fAngle = radians(1);
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

	pPSStream.SetTexture("graph/particles/smallfire", 4, 10);

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

	pPSHit.SetTexture("graph/particles/firebase", 4, 100);

	pPSHit.fParticleFreq = -1;
	pPSHit.SetPos(eSrc);
	pPSHit.Update(0);

}

//---------------------------------------------------------------------
void CIncinerate::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	Vec3f et;

	pPSStream.Update(_ulTime);
	pPSHit.Update(_ulTime);

	iMax = (int)((40) * fOneOnDuration * ulCurrentTime); //*2

	if (iMax > 40) iMax = 40;

	et.x = tv1a[iMax].p.x;
	et.y = tv1a[iMax].p.y;
	et.z = tv1a[iMax].p.z;
	pPSStream.SetPos(et);
}

//---------------------------------------------------------------------
float CIncinerate::Render()
{
	int i = 0;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	iMax ++;
	float x = eSrc.x + (eTarget.x - eSrc.x) * (ulCurrentTime * fOneOnDuration);
	float y = eSrc.y + (eTarget.y - eSrc.y) * (ulCurrentTime * fOneOnDuration);
	float z = eSrc.z + (eTarget.z - eSrc.z) * (ulCurrentTime * fOneOnDuration);

	for (i = 0; i < 150 - 1; i++)
	{
		Vec3f s, d;
		s.x = tv1a[i].p.x;
		s.y = tv1a[i].p.y;
		s.z = tv1a[i].p.z;
		d.x = tv1a[i+1].p.x;
		d.y = tv1a[i+1].p.y;
		d.z = tv1a[i+1].p.z;
		EERIEDraw3DLine(s, d, Color::red);
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	Anglef stiteangle;
	Vec3f stitepos;
	Vec3f stitescale;

	stiteangle.b = 90 - fBeta;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;
	stitescale.x = 0.5f;
	stitescale.y = 0.5f;
	stitescale.z = 0.5f;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	pPSStream.Render();

	return 1;
}

//-----------------------------------------------------------------------------
CNegateMagic::~CNegateMagic()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		delete ssol;
		ssol = NULL;
	}
}
CNegateMagic::CNegateMagic()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	tex_sol = TextureContainer::Load("graph/obj3d/textures/(fx)_negate_magic");

	if (!ssol)
		ssol = _LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
		
	ssol_count++;
	
}

//-----------------------------------------------------------------------------
void CNegateMagic::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
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
float CNegateMagic::Render()
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

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetTexture(0, tex_sol);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	fSize = ulCurrentTime * fOneOnDuration * 200;
 
	for (i = 0; i < 360; i++)
	{

		float t = rnd();

		if (t < 0.04f)
		{

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
				particle[j].rgb = Color3f::white;
			}
		}
	}
	
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	Anglef stiteangle(0.f, -(float) ulCurrentTime * fOneOnDuration * 120, 0.f);
	Vec3f stitepos(x, y, z);
	
	Color3f stitecolor(.4f, .4f, .4f);
	Vec3f stitescale(3.f, 3.f, 3.f);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	stitecolor = Color3f(.5f, 0.f, .5f);
	stitescale = Vec3f(3.1f, 3.1f, 3.1f);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

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
				if(!fartherThan(ePos, inter.iobj[nb]->pos, fRayon)) {
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
float CMassParalyse::Render()
{
	if (ulCurrentTime > ulDuration) return 0;

	int	nb = iNbParalyse;

	while (nb--)
	{
		(tabparalyse[nb].paralyse)->Render();
	}

	return 0;
}
