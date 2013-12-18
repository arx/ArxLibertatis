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

#include "graphics/spells/Spells09.h"

#include <cmath>

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/DrawLine.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells06.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

CSummonCreature::CSummonCreature()
	: fColorRays1(Color3f::white),
	  fColorBorder(Color3f::white),
	  fColorRays2(Color3f::black) {
	
	eSrc = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDurationIntro + ulDurationRender + ulDurationOuttro + 1;
	
	iSize = 100;
	fOneOniSize = 1.0f / ((float) iSize);
	
	tex_light = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu4");
}

void CSummonCreature::SetDuration(const unsigned long alDuration)
{
	ulDurationIntro			=	alDuration;

	if(ulDurationIntro <= 0)
		ulDurationIntro	=	100;
	else if(ulDurationIntro >= 100000)
		ulDurationIntro	=	100000;

	fOneOnDurationIntro		=	1.f / (float)(ulDurationIntro);

	ulDurationRender		=	1000;
	fOneOnDurationRender	=	1.f / (float)(ulDurationRender);

	ulDurationOuttro		=	1000;
	fOneOnDurationOuttro	=	1.f / (float)(ulDurationOuttro);

	ulCurrentTime			=	0;
}

void CSummonCreature::SetDuration(unsigned long alDurationIntro, unsigned long alDurationRender, unsigned long alDurationOuttro)
{
	if(alDurationIntro <= 0)
		alDurationIntro = 100;
	else if(alDurationIntro >= 100000)
		alDurationIntro = 100000;

	ulDurationIntro = alDurationIntro;
	fOneOnDurationIntro = 1.f / (float)(ulDurationIntro);

	if(alDurationRender <= 0)
		alDurationRender = 100;
	else if(alDurationRender >= 100000)
		alDurationRender = 100000;

	ulDurationRender = alDurationRender;
	fOneOnDurationRender = 1.f / (float)(ulDurationRender);

	if(alDurationOuttro <= 0)
		alDurationOuttro = 100;
	else if(alDurationOuttro >= 100000)
		alDurationOuttro = 100000;

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

	for(i = 0; i < 40; i++) {
		tfRaysa[i] = 0.4f * rnd();
		tfRaysb[i] = 0.4f * rnd(); 
	}
	
	target.p = eSrc - Vec3f(100.f, 0.f, 0.f);
	v1a[0].p = eSrc - Vec3f(0.f, 100.f, 0.f);
	v1a[end].p = eSrc + Vec3f(0.f, 100.f, 0.f);	
	v1b[0].p = v1a[0].p;
	v1b[end].p = v1a[end].p;

	sizeF = 200;
	Split(v1a, 0, end, 20);
	Split(v1b, 0, end, -20);

	sizeF = 200;
	Split(v1a, 0, end, 80);
	Split(v1b, 0, end, -80);

	// check de la conformité du split
	// sinon recalc de l'un de l'autre ou des deux
	// espace min
	for(i = 0; i < 40; i++) {
		if(v1a[i].p.x > v1b[i].p.x) {
			float fTemp = v1a[i].p.x;
			v1a[i].p.x = v1b[i].p.x;
			v1b[i].p.x = fTemp;
		}

		if(v1a[i].p.z > v1b[i].p.z) {
			float fTemp = v1a[i].p.z;
			v1a[i].p.z = v1b[i].p.z;
			v1b[i].p.z = fTemp;
		}

		if((v1b[i].p.x - v1a[i].p.x) > 20) {
			v1b[i].p.x = v1a[i].p.x + rnd() * 20.0f;
		}

		if((v1b[i].p.z - v1a[i].p.z) > 20) {
			v1b[i].p.z = v1a[i].p.z + rnd() * 20.0f;
		}
	}
	
	for(i = 0; i <= end; i++) {
		vb[i].p = va[i].p = eSrc;
	}
	
	sizeF = 0;
}

void CSummonCreature::Split(TexturedVertex * v, int a, int b, float yo)
{
	if(a != b) {
		int i = (int)((a + b) * 0.5f);

		if((i != a) && (i != b)) {
			v[i].p.x = (v[a].p.x + v[b].p.x) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadCos;
			v[i].p.y = v[0].p.y + (i + 1) * 5;
			v[i].p.z = (v[a].p.z + v[b].p.z) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

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

	RenderMaterial mat = RenderMaterial::getCurrent();
	mat.setLayer(RenderMaterial::EffectForeground);

	//-------------------------------------------------------------------------
	// computation des sommets
	float fTempCos, fTempSin;

	for(i = 0; i <= std::min(end, int(fSizeIntro)); i++) {
		if(i <= end * 0.5f)
			ff = i / (end * 0.5f);
		else
			ff = 1.0f - ((i - (end + 1) * 0.5f) / (end * 0.5f));

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
	mat.setBlendType(RenderMaterial::Opaque);
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toBGR();

	if(bIntro) {
		for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
			EE_RT2(&v1a[i], &vr[0]);
			EE_RT2(&v1b[i], &vr[1]);
			EE_RT2(&v1a[i+1], &vr[2]);
			EE_RT2(&v1b[i+1], &vr[3]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	} else {
		for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
			EE_RT2(&va[i], &vr[0]);
			EE_RT2(&vb[i], &vr[1]);
			EE_RT2(&va[i+1], &vr[2]);
			EE_RT2(&vb[i+1], &vr[3]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	mat.setBlendType(RenderMaterial::Additive);
	vr[0].color = vr[1].color = Color::black.toBGR();
	vr[2].color = vr[3].color = fColorBorder.toBGR();

	for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
		vt[2].p = va[i].p - (va[i].p - eSrc) * 0.2f;
		vt[3].p = va[i + 1].p - (va[i + 1].p - eSrc) * 0.2f;
		
		EE_RT2(&vt[3], &vr[0]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&va[i+1], &vr[2]);
		EE_RT2(&va[i], &vr[3]);
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
		
		vt[2].p = vb[i].p - (vb[i].p - eSrc) * 0.2f;
		vt[3].p = vb[i + 1].p - (vb[i + 1].p - eSrc) * 0.2f;
		
		EE_RT2(&vb[i], &vr[3]);
		EE_RT2(&vb[i+1], &vr[2]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&vt[3], &vr[0]);
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
	}

	//-------------------------------------------------------------------------
	// rendu des faisceaux
	// blend additif ou mul
	// smooth sur les cotés ou pas ..
	// texture sympa avec glow au milieu ou uv wrap
	mat.setWrapMode(TextureStage::WrapMirror);
	mat.setTexture(tex_light);

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

	for(i = 0; i < end - 1; i++) {
		
		if(i < fSizeIntro) {
			vt[0].p = va[i].p;
			vt[1].p = va[i + 1].p;
			vt[2].p = va[i].p + (va[i].p - target.p) * 2.f;
			vt[3].p = va[i + 1].p + (va[i + 1].p - target.p) * 2.f;
			vr[0].color = (fColorRays1 * tfRaysa[i]).toBGR();
			vr[1].color = (fColorRays1 * tfRaysa[i + 1]).toBGR();
			vr[2].color = (fColorRays2 * tfRaysa[i]).toBGR();
			vr[3].color = (fColorRays2 * tfRaysa[i + 1]).toBGR();
			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
		if(i < fSizeIntro) {
			vt[0].p = vb[i + 1].p;
			vt[1].p = vb[i].p;
			vt[2].p = vb[i + 1].p + (vb[i + 1].p - target.p) * 2.f;
			vt[3].p = vb[i].p + (vb[i].p - target.p) * 2.f;
			vr[0].color = (fColorRays1 * tfRaysb[i]).toBGR();
			vr[1].color = (fColorRays1 * tfRaysb[i + 1]).toBGR();
			vr[2].color = (fColorRays2 * tfRaysb[i]).toBGR();
			vr[3].color = (fColorRays2 * tfRaysb[i + 1]).toBGR();
			EE_RT2(&vt[0], &vr[3]);
			EE_RT2(&vt[1], &vr[2]);
			EE_RT2(&vt[2], &vr[1]);
			EE_RT2(&vt[3], &vr[0]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
	}
}

void CSummonCreature::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
// rendu de la déchirure spatio temporelle
void CSummonCreature::Render()
{
	if(ulCurrentTime >= (ulDurationIntro + ulDurationRender + ulDurationOuttro))
		return;

	GRenderer->ResetTexture(0);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	//-------------------------------------------------------------------------
	fTexWrap += 0.02f;

	if(fTexWrap >= 1.0f)
		fTexWrap -= 1.0f;

	//-------------------------------------------------------------------------
	// render intro (opening + rays)
	if(ulCurrentTime < ulDurationIntro) {
		if(ulCurrentTime < ulDurationIntro * 0.666f) {
			fSizeIntro = (end + 2) * fOneOnDurationIntro * (1.5f) * ulCurrentTime;
			sizeF = 1;
		} else {
			if(bIntro != false)
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
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);

	//return (fSizeIntro / end);
}

CNegateMagic::~CNegateMagic()
{
	ssol_count--;

	if(ssol && ssol_count <= 0) {
		ssol_count = 0;
		delete ssol;
		ssol = NULL;
	}
}

CNegateMagic::CNegateMagic() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	tex_sol = TextureContainer::Load("graph/obj3d/textures/(fx)_negate_magic");
	
	if(!ssol) {
		ssol = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	}
	ssol_count++;
	
}

void CNegateMagic::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	
	eTarget = eSrc;
	bDone = true;
}

void CNegateMagic::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CNegateMagic::Render() {
	
	int i = 0;
	
	if(spells[spellinstance].target == 0) {
		eSrc = player.basePosition();
	} else {
		eSrc = entities[spells[spellinstance].target]->pos;
	}
	
	Vec3f stitepos = eSrc - Vec3f(0.f, 10.f, 0.f);

	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetTexture(0, tex_sol);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	for(i = 0; i < 360; i++) {
		float t = rnd();
		if(t < 0.04f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(!pd) {
				break;
			}
			
			pd->ov = stitepos + Vec3f(frand2() * 150.f, 0.f, frand2() * 150.f);
			pd->move = Vec3f(0.f, -3.0f * rnd(), 0.f);
			pd->siz = 0.3f;
			pd->tolive = Random::get(2000, 4000);
			pd->tc = tex_p2;
			pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING
			              | SUBSTRACT;
			pd->fparam = 0.0000001f;
		}
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	Anglef stiteangle(0.f, -(float) ulCurrentTime * 0.02f, 0.f);
	Color3f stitecolor = Color3f::gray(.4f);
	float scalediff = std::sin(ulCurrentTime * 0.004f);
	Vec3f stitescale = Vec3f(3.f + 0.5f * scalediff);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, stitecolor);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	stitecolor = Color3f(.5f, 0.f, .5f);
	stitescale = Vec3f(3.1f + 0.2f * scalediff);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, stitecolor);
}
