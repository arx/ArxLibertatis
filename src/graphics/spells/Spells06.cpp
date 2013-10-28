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

#include "graphics/spells/Spells06.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Interactive.h"

using std::min;
using std::max;

extern ParticleManager * pParticleManager;

CCreateField::CCreateField() {
	
	fwrap = 0.f;
	eSrc = Vec3f_ZERO;
	fColor1[0] = 0.4f;
	fColor1[1] = 0.0f;
	fColor1[2] = 0.4f;
	fColor2[0] = 0.0f;
	fColor2[1] = 0.0f;
	fColor2[2] = 0.6f;
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	tex_jelly = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu3");
}

void CCreateField::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	fbeta = afBeta;
	ysize = 0.1f;
	size = 0.1f;
	ft = 0.0f;
	fglow = 0;
	youp = true;
}

void CCreateField::RenderQuad(TexturedVertex p1, TexturedVertex p2, TexturedVertex p3, TexturedVertex p4, int rec, Vec3f norm)
{
	TexturedVertex v[5];
	TexturedVertex v2[5];

	if(rec < 3) {
		rec ++;

		// milieu
		v[0].p = p1.p + (p3.p - p1.p) * 0.5f;
		// gauche
		v[1].p = p1.p + (p4.p - p1.p) * 0.5f;
		// droite
		v[2].p = p2.p + (p3.p - p2.p) * 0.5f;
		// haut
		v[3].p = p4.p + (p3.p - p4.p) * 0.5f;
		// bas
		v[4].p = p1.p + (p2.p - p1.p) * 0.5f;

		float patchsize = 0.005f;

		v[0].p.x += (float) sin(radians((v[0].p.x - eSrc.x) * patchsize + fwrap)) * 5;
		v[0].p.y += (float) sin(radians((v[0].p.y - eSrc.y) * patchsize + fwrap)) * 5;
		v[0].p.z += (float) sin(radians((v[0].p.z - eSrc.z) * patchsize + fwrap)) * 5;
		v[1].p.x += (float) sin(radians((v[1].p.x - eSrc.x) * patchsize + fwrap)) * 5;
		v[1].p.y += (float) sin(radians((v[1].p.y - eSrc.y) * patchsize + fwrap)) * 5;
		v[1].p.z += (float) sin(radians((v[1].p.z - eSrc.z) * patchsize + fwrap)) * 5;
		v[2].p.x += (float) sin(radians((v[2].p.x - eSrc.x) * patchsize + fwrap)) * 5;
		v[2].p.y += (float) sin(radians((v[2].p.y - eSrc.y) * patchsize + fwrap)) * 5;
		v[2].p.z += (float) sin(radians((v[2].p.z - eSrc.z) * patchsize + fwrap)) * 5;
		v[3].p.x += (float) sin(radians((v[3].p.x - eSrc.x) * patchsize + fwrap)) * 5;
		v[3].p.y += (float) sin(radians((v[3].p.y - eSrc.y) * patchsize + fwrap)) * 5;
		v[3].p.z += (float) sin(radians((v[3].p.z - eSrc.z) * patchsize + fwrap)) * 5;
		v[4].p.x += (float) sin(radians((v[4].p.x - eSrc.x) * patchsize + fwrap)) * 5;
		v[4].p.y += (float) sin(radians((v[4].p.y - eSrc.y) * patchsize + fwrap)) * 5;
		v[4].p.z += (float) sin(radians((v[4].p.z - eSrc.z) * patchsize + fwrap)) * 5;

		RenderQuad(p1, v[4], v[0], v[1], rec, norm);
		RenderQuad(v[4], p2, v[2], v[0], rec, norm);
		RenderQuad(v[0], v[2], p3, v[3], rec, norm);
		RenderQuad(v[1], v[0], v[3], p4, rec, norm);
	} else if(rec == 3) {
		float zab = (float) sin(radians(ft));
		v2[0].uv.x = 0 + zab;
		v2[0].uv.y = 0 + zab;
		v2[1].uv.x = 1 + zab;
		v2[1].uv.y = 0 + zab;
		v2[2].uv.x = 1 + zab;
		v2[2].uv.y = 1 + zab;
		v2[3].uv.x = 0 + zab;
		v2[3].uv.y = 1 + zab;

		v2[1].color = v2[2].color = Color3f(falpha * .3f + rnd() * .025f, 0.f, falpha * .5f + rnd() * .025f).toBGR();
		v2[0].color = v2[3].color = Color3f(falpha * .3f + rnd() * .025f, 0.f, falpha * .5f + rnd() * .025f).toBGR();
	
		EE_RT2(&p1, &v2[0]);
		EE_RT2(&p2, &v2[1]);
		EE_RT2(&p3, &v2[2]);
		EE_RT2(&p4, &v2[3]);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[3]);
		ARX_DrawPrimitive(&v2[1], &v2[2], &v2[3]);
	}
}

void CCreateField::RenderSubDivFace(TexturedVertex * b, TexturedVertex * t, int b1, int b2, int t1, int t2) {
	Vec3f norm = (b[b1].p + b[b2].p + t[t1].p + t[t2].p) * 0.25f - eSrc;
	fnormalize(norm);
	RenderQuad(b[b1], b[b2], t[t1], t[t2], 1, norm);
}

void CCreateField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

extern bool VisibleSphere(float x, float y, float z, float radius);

void CCreateField::Render()
{
	if(!VisibleSphere(eSrc.x, eSrc.y - 120.f, eSrc.z, 400.f))
		return;

	if(ulCurrentTime >= ulDuration)
		return;

	falpha = 1.f - (((float)(ulCurrentTime)) * fOneOnDuration);

	if (falpha > 1.f) falpha = 1.f;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	//-------------------------------------------------------------------------
	// rendu
	if(youp) {
		fglow += 0.5f;

		if(fglow >= 50) {
			youp = false;
		}
	} else {
		fglow -= 0.5f;

		if(fglow <= 0) {
			youp = true;
		}
	}

	ysize = min(1.0f, ulCurrentTime * 0.001f);

	if(ysize >= 1.0f) {
		size = min(1.0f, (ulCurrentTime - 1000) * 0.001f);
		size = max(size, 0.1f);
	}

	// ondulation
	ft += 0.01f;

	if(ft > 360.0f) {
		ft = 0.0f;
	}

	falpha = (float) sin(radians(fglow)) + rnd() * 0.2f;

	if(falpha > 1.0f)
		falpha = 1.0f;

	if(falpha < 0.0f)
		falpha = 0.0f;

	float x = eSrc.x;
	float y = eSrc.y;
	float z = eSrc.z;
	float smul = 100 * size;

	// bottom points
	b[0].p.x = x - smul;
	b[0].p.y = y;
	b[0].p.z = z - smul;

	b[1].p.x = x + smul;
	b[1].p.y = y;
	b[1].p.z = z - smul;

	b[2].p.x = x + smul;
	b[2].p.y = y;
	b[2].p.z = z + smul;

	b[3].p.x = x - smul;
	b[3].p.y = y;
	b[3].p.z = z + smul;

	// top points
	t[0].p.x = x - smul;
	t[0].p.y = y - 250 * ysize;
	t[0].p.z = z - smul;

	t[1].p.x = x + smul;
	t[1].p.y = y - 250 * ysize;
	t[1].p.z = z - smul;

	t[2].p.x = x + smul;
	t[2].p.y = y - 250 * ysize;
	t[2].p.z = z + smul;

	t[3].p.x = x - smul;
	t[3].p.y = y - 250 * ysize;
	t[3].p.z = z + smul;

	fwrap -= 5.0f; // TODO ignores the frame delay
	while(fwrap < 0) {
		fwrap += 360;
	}

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	GRenderer->SetTexture(0, tex_jelly);

	RenderSubDivFace(b, b, 0, 1, 2, 3);
	RenderSubDivFace(t, t, 0, 3, 2, 1);
	RenderSubDivFace(b, t, 1, 0, 0, 1);
	RenderSubDivFace(b, t, 3, 2, 2, 3);
	RenderSubDivFace(b, t, 0, 3, 3, 0);
	RenderSubDivFace(b, t, 2, 1, 1, 2);

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	if(lLightId != -1) {
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

	//return falpha;
}

CSlowDown::CSlowDown() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	if(!ssol) { // Pentacle
		ssol = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	}
	ssol_count++;
	
	if(!slight) { // Twirl
		slight = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard02.teo");
	}
	slight_count++; //runes
	
	if(!srune) {
		srune  = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard03.teo");
	}
	srune_count++;
	
}

CSlowDown::~CSlowDown()
{
	ssol_count--;

	if(ssol && ssol_count <= 0) {
		ssol_count = 0;
		delete ssol;
		ssol = NULL;
	}

	slight_count--;

	if(slight && slight_count <= 0) {
		slight_count = 0;
		delete slight;
		slight = NULL;
	}

	srune_count--;

	if(srune && srune_count <= 0) {
		srune_count = 0;
		delete srune;
		srune = NULL;
	}
}

void CSlowDown::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	eSrc = aeSrc;
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	eTarget = eSrc;
	fSize = 1;
	bDone = true;
}

void CSlowDown::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CSlowDown::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
}

//-----------------------------------------------------------------------------
// RISE DEAD
//-----------------------------------------------------------------------------
CRiseDead::~CRiseDead()
{
	stone0_count--;

	if(stone0 && stone0_count <= 0) {
		stone0_count = 0;
		delete stone0;
		stone0 = NULL;
	}

	stone1_count--;

	if(stone1 && stone1_count <= 0) {
		stone1_count = 0;
		delete stone1;
		stone1 = NULL;
	}
}

CRiseDead::CRiseDead() {
	
	eSrc = Vec3f_ZERO;
	
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
	
	if(!stone0) {
		stone0 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone01.teo");
	}
	stone0_count++;
	
	if(!stone1) {
		stone1 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone02.teo");
	}
	stone1_count++;
	
	tex_light = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu4");
}

void CRiseDead::SetDuration(const unsigned long alDuration)
{
	ulDurationIntro			= alDuration;

	if(ulDurationIntro <= 0)
		ulDurationIntro = 100;
	else if(ulDurationIntro >= 100000)
		ulDurationIntro = 100000;

	fOneOnDurationIntro		= 1.f / (float)(ulDurationIntro);

	ulDurationRender		= 1000;
	fOneOnDurationRender	= 1.f / (float)(ulDurationRender);

	ulDurationOuttro		= 1000;
	fOneOnDurationOuttro	= 1.f / (float)(ulDurationOuttro);

	ulCurrentTime = 0;
}

void CRiseDead::SetDuration(unsigned long alDurationIntro, unsigned long alDurationRender, unsigned long alDurationOuttro)
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

void CRiseDead::SetColorBorder(float afRed, float afGreen, float afBlue)
{
	fColorBorder[0] = afRed;
	fColorBorder[1] = afGreen;
	fColorBorder[2] = afBlue;
}

void CRiseDead::SetColorRays1(float afRed, float afGreen, float afBlue)
{
	fColorRays1[0] = afRed;
	fColorRays1[1] = afGreen;
	fColorRays1[2] = afBlue;
}

void CRiseDead::SetColorRays2(float afRed, float afGreen, float afBlue)
{
	fColorRays2[0] = afRed;
	fColorRays2[1] = afGreen;
	fColorRays2[2] = afBlue;
}

void CRiseDead::Create(Vec3f aeSrc, float afBeta)
{
	int i;

	SetDuration(ulDurationIntro, ulDurationRender, ulDurationOuttro);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y - 10.f; 
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

	v1a[0].p.x = eSrc.x - fBetaRadSin * 100;
	v1a[0].p.y = eSrc.y;
	v1a[0].p.z = eSrc.z + fBetaRadCos * 100;
	v1a[end].p.x = eSrc.x + fBetaRadSin * 100;
	v1a[end].p.y = eSrc.y;
	v1a[end].p.z = eSrc.z - fBetaRadCos * 100;

	v1b[0].p = v1a[0].p;
	v1b[end].p = v1a[end].p;

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
	
	for(i = 0; i <= end; i++) {
		vb[i].p = va[i].p = eSrc;
	}
	
	sizeF = 0;
	
	// cailloux
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

void CRiseDead::AddStone(Vec3f * pos) {
	
	if(arxtime.is_paused() || nbstone > 255) {
		return;
	}
	
	int nb = 256;
	while(nb--) {
		if(!tstone[nb].actif) {
			nbstone++;
			tstone[nb].actif = 1;
			tstone[nb].numstone = rand() & 1;
			tstone[nb].pos = *pos;
			tstone[nb].yvel = rnd() * -5.f;
			tstone[nb].ang = Anglef(rnd() * 360.f, rnd() * 360.f, rnd() * 360.f);
			tstone[nb].angvel = Anglef(5.f * rnd(), 6.f * rnd(), 3.f * rnd());
			tstone[nb].scale = Vec3f(0.2f + rnd() * 0.3f);
			tstone[nb].time = Random::get(2000, 2500);
			tstone[nb].currtime = 0;
			break;
		}
	}
}

void CRiseDead::DrawStone()
{
	int	nb = 256;
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	while(nb--) {
		if(this->tstone[nb].actif) {
			float a = (float)this->tstone[nb].currtime / (float)this->tstone[nb].time;

			if(a > 1.f) {
				a = 1.f;
				this->tstone[nb].actif = 0;
			}

			int col = Color::white.toBGR((int)(255.f * (1.f - a)));
			DrawEERIEObjExEx(this->stone[this->tstone[nb].numstone], &this->tstone[nb].ang, &this->tstone[nb].pos, &this->tstone[nb].scale, col);
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tstone[nb].pos;
				pd->move = Vec3f(0.f, 3.f * rnd(), 0.f);
				pd->siz = 3.f + 3.f * rnd();
				pd->tolive = 1000;
				pd->timcreation = -(long(arxtime) + 1000l); // TODO WTF
				pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			
			//update mvt
			if(!arxtime.is_paused()) {
				a = (((float)this->currframetime) * 100.f) / (float)this->tstone[nb].time;
				tstone[nb].pos.y += tstone[nb].yvel * a;
				tstone[nb].ang += tstone[nb].angvel * a;

				this->tstone[nb].yvel *= 1.f - (1.f / 100.f);

				this->tstone[nb].currtime += this->currframetime;
			}
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void CRiseDead::Split(TexturedVertex * v, int a, int b, float yo)
{
	if(a != b) {
		int i = (int)((a + b) * 0.5f);

		if(i != a && i != b) {
			v[i].p.x = (v[a].p.x + v[b].p.x) * 0.5f + yo * frand2() * fBetaRadCos;
			v[i].p.y = v[0].p.y;// + (i+1)*5;
			v[i].p.z = (v[a].p.z + v[b].p.z) * 0.5f + yo * frand2() * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

void CRiseDead::RenderFissure()
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

	for(i = 0; i <= min(end, (int)fSizeIntro); i++) {
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
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toBGR();

	if(bIntro) {
		for(i = 0; i < min(end, (int)fSizeIntro); i++) {
			EE_RT2(&v1a[i], &vr[0]);
			EE_RT2(&v1b[i], &vr[1]);
			EE_RT2(&v1a[i+1], &vr[2]);
			EE_RT2(&v1b[i+1], &vr[3]);
			ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
			ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
		}
	} else {
		for(i = 0; i < min(end, (int)fSizeIntro); i++) {
			EE_RT2(&va[i], &vr[0]);
			EE_RT2(&vb[i], &vr[1]);
			EE_RT2(&va[i+1], &vr[2]);
			EE_RT2(&vb[i+1], &vr[3]);
			ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
			ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	vr[0].color = vr[1].color = Color::black.toBGR();
	vr[2].color = vr[3].color = Color3f(fColorBorder[0], fColorBorder[1], fColorBorder[2]).toBGR();

	for(i = 0; i < min(end, (int)fSizeIntro); i++) {
		vt[2].p = va[i].p - (va[i].p - eSrc) * 0.2f;
		vt[3].p = va[i + 1].p - (va[i + 1].p - eSrc) * 0.2f;
		
		EE_RT2(&vt[3], &vr[0]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&va[i+1], &vr[2]);
		EE_RT2(&va[i], &vr[3]);
		ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
		ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
		
		vt[2].p = vb[i].p - (vb[i].p - eSrc) * 0.2f;
		vt[3].p = vb[i + 1].p - (vb[i + 1].p - eSrc) * 0.2f;
		
		EE_RT2(&vb[i], &vr[3]);
		EE_RT2(&vb[i+1], &vr[2]);
		EE_RT2(&vt[2], &vr[1]);
		EE_RT2(&vt[3], &vr[0]);
		ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
		ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
	}

	//-------------------------------------------------------------------------
	// rendu des faisceaux
	// blend additif ou mul
	// smooth sur les cotés ou pas ..
	// texture sympa avec glow au milieu ou uv wrap
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapMirror);
	GRenderer->SetTexture(0, tex_light);

	target.p.x = eSrc.x ;
	target.p.y = eSrc.y + 1.5f * sizeF; 
	target.p.z = eSrc.z ;

	EE_RTP(&vt[1], &vr[0]);
	vr[0].color = vr[1].color = Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]).toBGR();
	vr[2].color = vr[3].color = Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]).toBGR();

	vr[0].uv.x = fTexWrap;
	vr[0].uv.y = 1;
	vr[1].uv.x = 1 + fTexWrap;
	vr[1].uv.y = 1;
	vr[2].uv.x = fTexWrap;
	vr[2].uv.y = 0;
	vr[3].uv.x = 1 + fTexWrap;
	vr[3].uv.y = 0;

	for(i = 0; i < end - 1; i++) {
		float t = rnd();

		if(t <= 0.15f) {
			if(tfRaysa[i] < 1.0f)
				tfRaysa[i] += 0.02f;

			if(tfRaysa[i+1] < 1.0f)
				tfRaysa[i+1] += 0.01f;

			if(tfRaysa[i] > 1.0f)
				tfRaysa[i] = 1.0f;

			if(tfRaysa[i+1] > 1.0f)
				tfRaysa[i+1] = 1.0f;
		}

		if(t >= 0.9f) {
			if(tfRaysa[i] > 0.0f)
				tfRaysa[i] -= 0.02f;

			if(tfRaysa[i+1] > 0.0f)
				tfRaysa[i+1] -= 0.01f;

			if(tfRaysa[i] < 0.0f)
				tfRaysa[i] = 0.0f;

			if(tfRaysa[i+1] < 0.0f)
				tfRaysa[i+1] = 0.0f;
		}

		float t2 = rnd();

		if(t2 <= 0.15f) {
			if(tfRaysb[i] < 1.0f)
				tfRaysb[i] += 0.02f;

			if(tfRaysb[i+1] < 1.0f)
				tfRaysb[i+1] += 0.01f;

			if(tfRaysb[i] > 1.0f)
				tfRaysb[i] = 1.0f;

			if(tfRaysb[i+1] > 1.0f)
				tfRaysb[i+1] = 1.0f;
		}

		if(t2 >= 0.9f) {
			if(tfRaysb[i] > 0.0f)
				tfRaysb[i] -= 0.02f;

			if(tfRaysb[i+1] > 0.0f)
				tfRaysb[i+1] -= 0.01f;

			if(tfRaysb[i] < 0.0f)
				tfRaysb[i] = 0.0f;

			if(tfRaysb[i+1] < 0.0f)
				tfRaysb[i+1] = 0.0f;
		}
		
		if(i < fSizeIntro) {
			vt[0].p = va[i].p;
			vt[1].p = va[i + 1].p;
			vt[2].p.x = va[i].p.x ;
			vt[2].p.y = va[i].p.y + (va[i].p.y - target.p.y) * 2;
			vt[2].p.z = va[i].p.z ;
			vt[3].p.x = va[i+1].p.x ;
			vt[3].p.y = va[i+1].p.y + (va[i+1].p.y - target.p.y) * 2;
			vt[3].p.z = va[i+1].p.z ;

			vr[0].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysa[i]).toBGR();
			vr[1].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysa[i + 1]).toBGR();
			vr[2].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysa[i]).toBGR();
			vr[3].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysa[i + 1]).toBGR();
			
			EE_RT2(&vt[0], &vr[0]);
			EE_RT2(&vt[1], &vr[1]);
			EE_RT2(&vt[2], &vr[2]);
			EE_RT2(&vt[3], &vr[3]);
			ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
			ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
		}
		
		if(i < fSizeIntro) {
			vt[0].p = vb[i + 1].p;
			vt[1].p = vb[i].p;
			vt[2].p.x = vb[i+1].p.x ;
			vt[2].p.y = vb[i+1].p.y + (vb[i+1].p.y - target.p.y) * 2;
			vt[2].p.z = vb[i+1].p.z ;
			vt[3].p.x = vb[i].p.x ;
			vt[3].p.y = vb[i].p.y + (vb[i].p.y - target.p.y) * 2;
			vt[3].p.z = vb[i].p.z ;

			vr[0].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysb[i]).toBGR();
			vr[1].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysb[i + 1]).toBGR();
			vr[2].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysb[i]).toBGR();
			vr[3].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysb[i + 1]).toBGR();

			EE_RT2(&vt[0], &vr[0]);
			EE_RT2(&vt[1], &vr[1]);
			EE_RT2(&vt[2], &vr[2]);
			EE_RT2(&vt[3], &vr[3]);
			ARX_DrawPrimitive(&vr[0], &vr[1], &vr[2]);
			ARX_DrawPrimitive(&vr[1], &vr[2], &vr[3]);
		}
	}
}

void CRiseDead::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	currframetime = _ulTime;

	if(!arxtime.is_paused())
		this->timestone -= _ulTime;
}

//-----------------------------------------------------------------------------
// rendu de la déchirure spatio temporelle
void CRiseDead::Render()
{
	if(ulCurrentTime >= (ulDurationIntro + ulDurationRender + ulDurationOuttro))
		return;

	GRenderer->ResetTexture(0);
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

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

	//cailloux
	if(this->timestone <= 0) {
		this->timestone = Random::get(50, 150);
		Vec3f	pos;
		float r = 80.f * frand2();
		pos.x = this->eSrc.x + r;
		pos.y = this->eSrc.y;
		pos.z = this->eSrc.z + r;
		this->AddStone(&pos);
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);
	this->DrawStone();
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetCulling(Renderer::CullNone);

	//return (fSizeIntro / end);
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

CParalyse::~CParalyse()
{
	Kill();
}

void CParalyse::Kill()
{
	if(prismd3d) {
		delete [] prismd3d;
		prismd3d = NULL;
	}

	if(prismvertex) {
		delete [] prismvertex;
		prismvertex = NULL;
	}

	if(prismind) {
		delete [] prismind;
		prismind = NULL;
	}

	for(int i = 0; i < 100; i++) {
		if(tabprism[i].vertex) {
			delete [] tabprism[i].vertex;
			tabprism[i].vertex = NULL;
		}
	}

	if(lLightId >= 0) {
		DynLight[lLightId].exist = 0;
		lLightId = -1;
	}
}

void CParalyse::CreatePrismTriangleList(float arayon, float ahcapuchon, float ahauteur, int adef)
{
	float		a, da;
	int			nb;
	Vec3f	* v;

	a = 0.f;
	da = 360.f / (float)adef;
	v = prismvertex;
	nb = adef;

	v->x = 0.f;
	v->y = -ahcapuchon;
	v->z = 0.f;
	v++;

	while(nb) {
		v->x = arayon * EEcos(radians(a));
		v->y = -ahauteur + rnd() * ahauteur * 0.2f;
		v->z = arayon * EEsin(radians(a));
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

	while(nb) {
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

	while(nb) {
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

void CParalyse::CreateLittlePrismTriangleList()
{
	float		sc;
	Vec3f	* v, *vd;
	Vec3f	vt;

	for(int i = 0; i < 50; i++) {
		v = prismvertex;
		vd = tabprism[i].vertex;

		float randd = rnd() * 360.f;
		float fd = i * 3.0f;

		if(fd < 40 || fd > 120) {
			fd = 40 + rnd() * 80.0f;
		}

		fd = max(fd, 40.0f);
		fd = min(fd, 120.0f);
		tabprism[i].pos.x = pos.x + EEsin(randd) * fd;
		tabprism[i].pos.y = pos.y;
		tabprism[i].pos.z = pos.z + EEcos(randd) * fd;

		for(int j = 0; j < prismnbpt; j++) {
			sc = 0.2f + rnd() * 0.8f;

			vt.x = v->x * sc * .85f;
			vt.y = v->y * sc;
			vt.z = v->z * sc * .85f;

			vd->x = vt.x ;
			float fy = (1 - i * ( 1.0f / 20 ));
			fy = max(fy, 0.2f);
			fy = min(fy, 0.7f);
			vd->y = vt.y * fy;
			vd->z = vt.z ;

			v++;
			vd++;
		}

		v = prismvertex;
		vt = glm::normalize(*v);
		tabprism[i].offset.x = r * vt.x;
		tabprism[i].offset.y = 0;
		tabprism[i].offset.z = r * vt.z;
	}
}

//-----------------------------------------------------------------------------
//!!!!!!! def non impair
void CParalyse::Create(int adef, float arayon, float ahcapuchon, float ahauteur, Vec3f * aePos, int aduration)
{
	if(adef < 3)
		return;

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
	prismd3d = new TexturedVertex[prismnbpt];
	prismvertex = new Vec3f[prismnbpt];
	prismind = new unsigned short [prismnbface*3];

	for(int i = 0; i < 100; i++) {
		tabprism[i].vertex = new Vec3f[prismnbpt];
	}

	tex_prism = TextureContainer::Load("graph/obj3d/textures/(fx)_paralyze");
	tex_p	  = TextureContainer::Load("graph/particles/missile");
	tex_p1	  = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2	  = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");

	CreatePrismTriangleList(arayon, ahcapuchon, ahauteur, adef);
	CreateLittlePrismTriangleList();

	if(lLightId >= 0) {
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
	ParticleSystem * pPS = new ParticleSystem();
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
	cp.iNbMax = 200;
	cp.fLife = 500; //2000
	cp.fLifeRandom = 900;
	cp.p3Pos.x = 20;
	cp.p3Pos.y = 80;
	cp.p3Pos.z = 20;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(360);
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
	pPS->SetTexture("graph/particles/lil_greypouf", 0, 200);

	Vec3f ep;
	ep.x = aePos->x;
	ep.y = aePos->y - 80;
	ep.z = aePos->z;
	pPS->SetPos(ep);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	} else {
		// TODO memory leak (pPS)?
	}

	// système de partoches pour la poussière au sol
	pPS = new ParticleSystem();
	
	cp.iNbMax = 20;
	cp.fLife = 1000; //2000
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 20;
	cp.p3Pos.y = 5;
	cp.p3Pos.z = 20;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(144);
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
	pPS->SetTexture("graph/particles/lil_greypouf", 0, 200);

	ep.x = aePos->x;
	ep.y = aePos->y - 10;
	ep.z = aePos->z;
	pPS->SetPos(ep);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	if(pParticleManager) {
		pParticleManager->AddSystem(pPS);
	} else {
		// TODO memory leak (pPS)?
	}
}

void CParalyse::Update(unsigned long aulTime)
{
	float a;

	switch(key) {
		case -1:
			a = (float)currduration / 200.f;

			if(a > 1.f) {
				a = 0.f;
				key++;
				currduration = 0;
			}
			scale = a;
			break;
		case 0:
			a = (float)currduration / 300.f;

			if (a > 1.f) {
				a = 1.f;
				key++;
				currduration = 0;
			}
			scale = a;
			break;
		case 1:
			a = (float)currduration / (float)duration;
			scale = a;

			if(a > 1.f) {
				key++;
			}

			prisminterpcol = (float)colduration / 1000.f;

			if(prisminterpcol > 1.f) {
				prisminterpcol = 0.f;
				colduration = 0;
				InversePrismCol();
			}
			break;
	}

	currduration += aulTime;
}

void CParalyse::Render()
{
	if(key > 1)
		return;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	int			nb;
	Vec3f	* vertex;
	TexturedVertex	* vd3d, d3ds;

	//uv
	vd3d = this->prismd3d;
	vd3d->uv.x = .5f + .25f;
	vd3d->uv.y = 0.999999f;
	vd3d++;
	nb = (this->prismnbpt - 1) >> 2;

	while(nb) {
		vd3d->uv.x = 0.5f;
		vd3d->uv.y = 0.f;
		vd3d++;
		vd3d->uv.x = (vd3d - 1)->uv.x;
		vd3d->uv.y = 0.99999f;
		vd3d++;
		vd3d->uv.x = 0.9999f;
		vd3d->uv.y = 0.f;
		vd3d++;
		vd3d->uv.x = (vd3d - 1)->uv.x;
		vd3d->uv.y = 0.99999f;
		vd3d++;

		nb--;
	}

	GRenderer->SetTexture(0, tex_prism);

	int	nb2 = 50;

	switch(this->key) {
		
		case -1: {
			
			//calcul des chtis prism
			while(nb2--) {
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while(nb) {
					d3ds.p = tabprism[nb2].pos + *vertex * scale + tabprism[nb2].offset;
		
					EE_RTP(&d3ds, vd3d);
					vd3d->color = Color(50, 50, 64).toBGRA();
					vertex++;
					vd3d++;
					nb--;
				}

				GRenderer->SetCulling(Renderer::CullCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
				GRenderer->SetCulling(Renderer::CullCCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);

				vertex = this->tabprism[nb2].vertex;
				PARTICLE_DEF * pd = createParticle();
				if(pd) {
					pd->ov = pos + *vertex * scale;
					pd->move = Vec3f(0.f, 4.f * rnd(), 0.f);
					pd->siz = 10.f + 10.f * rnd();
					pd->tolive = 500;
					pd->tc = tex_p;
					pd->special  = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
					               | DISSIPATING;
					pd->fparam = 0.0000001f;
				}
			}
			
			break;
		}
		
		case 0: {
			
			while(nb2--) {
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while(nb) {
					float px = tabprism[nb2].pos.x ;
					float py = tabprism[nb2].pos.y ;
					float pz = tabprism[nb2].pos.z ;

					d3ds.p.x = px +  vertex->x + this->tabprism[nb2].offset.x;
					d3ds.p.y = py +  vertex->y + this->tabprism[nb2].offset.y;
					d3ds.p.z = pz +  vertex->z + this->tabprism[nb2].offset.z;
					EE_RTP(&d3ds, vd3d);
					vd3d->color = Color(50, 50, 64).toBGRA();
					vertex++;
					vd3d++;
					nb--;
				}

				GRenderer->SetCulling(Renderer::CullCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
				GRenderer->SetCulling(Renderer::CullCCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
			}
			
			vertex = prismvertex;
			vd3d = prismd3d;
			nb = prismnbpt;
			while(nb) {
				d3ds.p = pos + *vertex * scale;
				EE_RTP(&d3ds, vd3d);
				vd3d->color = Color::white.toBGR();
				vertex++;
				vd3d++;
				nb--;
			}
			
			vertex = prismvertex;
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = pos + *vertex * scale;
				pd->move = Vec3f(0.f, 4.f * rnd(), 0.f);
				pd->siz = 20.f + 20.f * rnd();
				pd->tolive = 2000;
				pd->tc = tex_p;
				pd->special  = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				               | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			
			vertex++;
			nb = (prismnbpt - 1) / 2;
			while(nb) {
				
				PARTICLE_DEF * pd = createParticle();
				if(pd) {
					pd->ov = pos + *vertex * scale;
					pd->move = Vec3f(0.f, 8.f * rnd(), 0.f);
					pd->siz = 10.f + 10.f * rnd();
					pd->tolive = 1000;
					pd->tc = tex_p;
					pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
					              | DISSIPATING;
					pd->fparam = 0.0000001f;
				}
				
				vertex += 2;
				nb--;
			}
			
			break;
		}
		
		case 1: {
			ColorBGRA col = Color(((int)(this->prismrd + (this->prismre - this->prismrd) * this->prisminterpcol)) >> 1,
			                    ((int)(this->prismgd + (this->prismge - this->prismgd) * this->prisminterpcol)) >> 1,
			                    ((int)(this->prismbd + (this->prismbe - this->prismbd) * this->prisminterpcol)) >> 1).toBGRA();

			if(this->lLightId >= 0) {
				DynLight[this->lLightId].rgb = Color3f::fromBGR(col) * 1.9f;
			}


			while(nb2--) {
				vertex = this->tabprism[nb2].vertex;
				vd3d = this->prismd3d;
				nb = this->prismnbpt;

				while(nb) {
					float px = tabprism[nb2].pos.x ;
					float py = tabprism[nb2].pos.y ;
					float pz = tabprism[nb2].pos.z ;

					d3ds.p.x = px +  vertex->x + this->tabprism[nb2].offset.x;
					d3ds.p.y = py +  vertex->y + this->tabprism[nb2].offset.y;
					d3ds.p.z = pz +  vertex->z + this->tabprism[nb2].offset.z;

					EE_RTP(&d3ds, vd3d);
					vd3d->color = col;
					vertex++;
					vd3d++;
					nb--;
				}

				GRenderer->SetCulling(Renderer::CullCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
				GRenderer->SetCulling(Renderer::CullCCW);
				GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
			}

			col = Color((int)(this->prismrd + (this->prismre - this->prismrd) * this->prisminterpcol),
			                (int)(this->prismgd + (this->prismge - this->prismgd) * this->prisminterpcol),
			                (int)(this->prismbd + (this->prismbe - this->prismbd) * this->prisminterpcol)
			                ).toBGRA();

			vertex = this->prismvertex;
			vd3d = this->prismd3d;
			nb = this->prismnbpt;

			while(nb) {
				d3ds.p = this->pos + *vertex;
				EE_RTP(&d3ds, vd3d);
				vd3d->color = col;
				vertex++;
				vd3d++;
				nb--;
			}

			break;
		}
	}

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->drawIndexed(Renderer::TriangleList, prismd3d, prismnbpt, prismind, prismnbface * 3);
	
	for(int i = 0; i < 20; i++) {
		
		float d = 2.f * r;
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = pos + Vec3f(d * frand2(), 5.f - rnd() * 10.f, d * frand2());
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = min(2000 + (rnd() * 2000.f), duration - currduration + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p2;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = pos + Vec3f(d * frand2(), 55.f - rnd() * 10.f, d * frand2());
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = min(2000 + (rnd() * 2000.f), duration - currduration + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p1;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
	}

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

CDisarmTrap::CDisarmTrap() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	if(!smotte) {
		smotte = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");
	}
	smotte_count++;
	
	if(!slight) {
		slight = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard02.teo");
	}
	slight_count++; 
	
	if(!srune) {
		srune = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard03.teo");
	}
	srune_count++;
}

CDisarmTrap::~CDisarmTrap()
{
	smotte_count--;

	if(smotte && smotte_count <= 0) {
		smotte_count = 0;
		delete smotte;
		smotte = NULL;
	}

	slight_count--;

	if(slight && slight_count <= 0) {
		slight_count = 0;
		delete slight;
		slight = NULL;
	}

	srune_count--;

	if(srune && srune_count <= 0) {
		srune_count = 0;
		delete srune;
		srune = NULL;
	}
}

void CDisarmTrap::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	eSrc = aeSrc;
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	eTarget = eSrc;
	fSize = 1;
	bDone = true;
}

void CDisarmTrap::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CDisarmTrap::Render() {
	
	float x = eSrc.x;
	float y = eSrc.y;
	float z = eSrc.z;
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	// TODO why not just entities.player()->pos ?
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			x = entities[i]->pos.x;
			y = entities[i]->pos.y;
			z = entities[i]->pos.z;
		}
	}

	GRenderer->SetTexture(0, tex_p2);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	Anglef stiteangle = Anglef::ZERO;
	Vec3f stitepos;
	Vec3f stitescale;
	Color3f stitecolor;

	stiteangle.setPitch((float) ulCurrentTime * fOneOnDuration * 120);
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;

	stitecolor.r = 0.8f;
	stitecolor.g = 0.1f;
	stitecolor.b = 0.1f;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, stitecolor);
}
