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
	: fBetaRadCos(0.f)
	, fBetaRadSin(0.f)
	, fColorRays1(Color3f::white)
	, end(0)
	, bIntro(true)
	, sizeF(0.f)
	, fSizeIntro(0.f)
	, fTexWrap(0.f)
	, fColorBorder(Color3f::white)
	, fColorRays2(Color3f::black)
{
	
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
	SetDuration(ulDurationIntro, ulDurationRender, ulDurationOuttro);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y - 50;
	eSrc.z = aeSrc.z;
	SetAngle(afBeta);
	sizeF = 0;
	fSizeIntro = 0.0f;
	fTexWrap = 0;
	end = 40 - 1;
	bIntro = true;

	for(int i = 0; i < 40; i++) {
		tfRaysa[i] = 0.4f * rnd();
		tfRaysb[i] = 0.4f * rnd(); 
	}
	
	v1a[0] = eSrc - Vec3f(0.f, 100.f, 0.f);
	v1a[end] = eSrc + Vec3f(0.f, 100.f, 0.f);	
	v1b[0] = v1a[0];
	v1b[end] = v1a[end];

	sizeF = 200;
	Split(v1a, 0, end, 20);
	Split(v1b, 0, end, -20);

	sizeF = 200;
	Split(v1a, 0, end, 80);
	Split(v1b, 0, end, -80);

	// check de la conformité du split
	// sinon recalc de l'un de l'autre ou des deux
	// espace min
	for(int i = 0; i < 40; i++) {
		if(v1a[i].x > v1b[i].x) {
			float fTemp = v1a[i].x;
			v1a[i].x = v1b[i].x;
			v1b[i].x = fTemp;
		}

		if(v1a[i].z > v1b[i].z) {
			float fTemp = v1a[i].z;
			v1a[i].z = v1b[i].z;
			v1b[i].z = fTemp;
		}

		if((v1b[i].x - v1a[i].x) > 20) {
			v1b[i].x = v1a[i].x + rnd() * 20.0f;
		}

		if((v1b[i].z - v1a[i].z) > 20) {
			v1b[i].z = v1a[i].z + rnd() * 20.0f;
		}
	}
	
	for(int i = 0; i <= end; i++) {
		vb[i] = va[i] = eSrc;
	}
	
	sizeF = 0;
}

void CSummonCreature::Split(Vec3f * v, int a, int b, float yo)
{
	if(a != b) {
		int i = (int)((a + b) * 0.5f);

		if((i != a) && (i != b)) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadCos;
			v[i].y = v[0].y + (i + 1) * 5;
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * frand2() * (sizeF * 0.005f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

// TODO copy-paste spell effect Fissure
void CSummonCreature::RenderFissure()
{
	int i;
	float ff;
	Vec3f vt[4];
	TexturedVertex vr[4];
	Vec3f target;

	Vec3f etarget;
	etarget.x = fBetaRadCos;
	etarget.y = 0;
	etarget.z = fBetaRadSin;
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullNone);
	mat.setDepthTest(false);
	mat.setWrapMode(TextureStage::WrapClamp);
	mat.setBlendType(RenderMaterial::Opaque);
	
	
	mat.setLayer(RenderMaterial::EffectForeground);

	//-------------------------------------------------------------------------
	// computation des sommets

	for(i = 0; i <= std::min(end, int(fSizeIntro)); i++) {
		if(i <= end * 0.5f)
			ff = i / (end * 0.5f);
		else
			ff = 1.0f - ((i - (end + 1) * 0.5f) / (end * 0.5f));

		float fTempCos = ff * fBetaRadCos;
		float fTempSin = ff * fBetaRadSin;

		va[i].x   = v1a[i].x   + sizeF * fTempCos;
		va[i].y   = v1a[i].y;
		va[i].z   = v1a[i].z   + sizeF * fTempSin;

		vb[i].x   = v1b[i].x   - sizeF * fTempCos;
		vb[i].y   = v1b[i].y;
		vb[i].z   = v1b[i].z   - sizeF * fTempSin;

		va[i].x += rnd() * 0.5f * fTempCos;
		va[i].z += rnd() * 0.5f * fTempSin;
		vb[i].x -= rnd() * 0.5f * fTempCos;
		vb[i].z -= rnd() * 0.5f * fTempSin;
	}

	//-------------------------------------------------------------------------
	// rendu de la fissure
	mat.setBlendType(RenderMaterial::Opaque);
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toRGB();

	if(bIntro) {
		for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
			vr[0].p = EE_RT(v1a[i]);
			vr[1].p = EE_RT(v1b[i]);
			vr[2].p = EE_RT(v1a[i+1]);
			vr[3].p = EE_RT(v1b[i+1]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	} else {
		for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
			vr[0].p = EE_RT(va[i]);
			vr[1].p = EE_RT(vb[i]);
			vr[2].p = EE_RT(va[i+1]);
			vr[3].p = EE_RT(vb[i+1]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	mat.setBlendType(RenderMaterial::Additive);
	vr[0].color = vr[1].color = Color::black.toRGB();
	vr[2].color = vr[3].color = fColorBorder.toRGB();

	for(i = 0; i < std::min(end, (int)fSizeIntro); i++) {
		vt[2] = va[i] - (va[i] - eSrc) * 0.2f;
		vt[3] = va[i + 1] - (va[i + 1] - eSrc) * 0.2f;
		
		vr[0].p = EE_RT(vt[3]);
		vr[1].p = EE_RT(vt[2]);
		vr[2].p = EE_RT(va[i+1]);
		vr[3].p = EE_RT(va[i]);
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
		
		vt[2] = vb[i] - (vb[i] - eSrc) * 0.2f;
		vt[3] = vb[i + 1] - (vb[i + 1] - eSrc) * 0.2f;
		
		vr[3].p = EE_RT(vb[i]);
		vr[2].p = EE_RT(vb[i+1]);
		vr[1].p = EE_RT(vt[2]);
		vr[0].p = EE_RT(vt[3]);
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

	target.x = eSrc.x + -fBetaRadSin * (1.5f * sizeF); 
	target.y = eSrc.y;
	target.z = eSrc.z + fBetaRadCos * (1.5f * sizeF); 

	EE_RTP(vt[1], &vr[0]);
	vr[0].color = vr[1].color = fColorRays1.toRGB();
	vr[2].color = vr[3].color = fColorRays2.toRGB();

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
			vt[0] = va[i];
			vt[1] = va[i + 1];
			vt[2] = va[i] + (va[i] - target) * 2.f;
			vt[3] = va[i + 1] + (va[i + 1] - target) * 2.f;
			
			vr[0].color = (fColorRays1 * tfRaysa[i]).toRGB();
			vr[1].color = (fColorRays1 * tfRaysa[i + 1]).toRGB();
			vr[2].color = (fColorRays2 * tfRaysa[i]).toRGB();
			vr[3].color = (fColorRays2 * tfRaysa[i + 1]).toRGB();
			
			vr[3].p = EE_RT(vt[0]);
			vr[2].p = EE_RT(vt[1]);
			vr[1].p = EE_RT(vt[2]);
			vr[0].p = EE_RT(vt[3]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
		if(i < fSizeIntro) {
			vt[0] = vb[i + 1];
			vt[1] = vb[i];
			vt[2] = vb[i + 1] + (vb[i + 1] - target) * 2.f;
			vt[3] = vb[i] + (vb[i] - target) * 2.f;
			
			vr[0].color = (fColorRays1 * tfRaysb[i]).toRGB();
			vr[1].color = (fColorRays1 * tfRaysb[i + 1]).toRGB();
			vr[2].color = (fColorRays2 * tfRaysb[i]).toRGB();
			vr[3].color = (fColorRays2 * tfRaysb[i + 1]).toRGB();
			
			vr[3].p = EE_RT(vt[0]);
			vr[2].p = EE_RT(vt[1]);
			vr[1].p = EE_RT(vt[2]);
			vr[0].p = EE_RT(vt[3]);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}
}

void CSummonCreature::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
}

//-----------------------------------------------------------------------------
// rendu de la déchirure spatio temporelle
void CSummonCreature::Render()
{
	if(ulCurrentTime >= (ulDurationIntro + ulDurationRender + ulDurationOuttro))
		return;
	
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
	
	RenderFissure();
	
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
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	tex_sol = TextureContainer::Load("graph/obj3d/textures/(fx)_negate_magic");
	
	if(!ssol) {
		ssol = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	}
	ssol_count++;
	
}

void CNegateMagic::Create(Vec3f aeSrc) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
}

void CNegateMagic::Update(float timeDelta) {
	ulCurrentTime += timeDelta;
}

void CNegateMagic::Render() {
	
	Vec3f stitepos = eSrc - Vec3f(0.f, 10.f, 0.f);

	if(ulCurrentTime >= ulDuration)
		return;
	
	RenderMaterial mat;
	mat.setLayer(RenderMaterial::Decal);
	mat.setDepthTest(true);
	mat.setTexture(tex_sol);
	mat.setBlendType(RenderMaterial::Additive);
	
	for(int i = 0; i < 360; i++) {
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
	
	Anglef stiteangle(0.f, -(float) ulCurrentTime * 0.02f, 0.f);
	Color3f stitecolor = Color3f::gray(.4f);
	float scalediff = std::sin(ulCurrentTime * 0.004f);
	Vec3f stitescale = Vec3f(3.f + 0.5f * scalediff);
	Draw3DObject(ssol, stiteangle, stitepos, stitescale, stitecolor, mat);
	
	stitecolor = Color3f(.5f, 0.f, .5f);
	stitescale = Vec3f(3.1f + 0.2f * scalediff);
	Draw3DObject(ssol, stiteangle, stitepos, stitescale, stitecolor, mat);
}

void CNegateMagic::SetPos(Vec3f pos)
{
	eSrc = pos;
}
