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

CCreateField::CCreateField()
	: eSrc(Vec3f_ZERO)
	, youp(true)
	, fwrap(0.f)
	, ysize(0.1f)
	, size(0.1f)
	, ft(0.0f)
	, fglow(0.f)
	, falpha(0.f)
{
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

void CCreateField::Create(Vec3f aeSrc) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	ysize = 0.1f;
	size = 0.1f;
	ft = 0.0f;
	fglow = 0;
	youp = true;
}

void CCreateField::RenderQuad(const Vec3f & p1, const Vec3f & p2, const Vec3f & p3, const Vec3f & p4, int rec, Vec3f norm)
{
	if(rec < 3) {
		rec ++;
		
		Vec3f v[5];
		// milieu
		v[0] = p1 + (p3 - p1) * 0.5f;
		// gauche
		v[1] = p1 + (p4 - p1) * 0.5f;
		// droite
		v[2] = p2 + (p3 - p2) * 0.5f;
		// haut
		v[3] = p4 + (p3 - p4) * 0.5f;
		// bas
		v[4] = p1 + (p2 - p1) * 0.5f;

		float patchsize = 0.005f;

		v[0].x += (float) sin(radians((v[0].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[0].y += (float) sin(radians((v[0].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[0].z += (float) sin(radians((v[0].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[1].x += (float) sin(radians((v[1].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[1].y += (float) sin(radians((v[1].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[1].z += (float) sin(radians((v[1].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[2].x += (float) sin(radians((v[2].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[2].y += (float) sin(radians((v[2].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[2].z += (float) sin(radians((v[2].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[3].x += (float) sin(radians((v[3].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[3].y += (float) sin(radians((v[3].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[3].z += (float) sin(radians((v[3].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[4].x += (float) sin(radians((v[4].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[4].y += (float) sin(radians((v[4].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[4].z += (float) sin(radians((v[4].z - eSrc.z) * patchsize + fwrap)) * 5;

		RenderQuad(p1, v[4], v[0], v[1], rec, norm);
		RenderQuad(v[4], p2, v[2], v[0], rec, norm);
		RenderQuad(v[0], v[2], p3, v[3], rec, norm);
		RenderQuad(v[1], v[0], v[3], p4, rec, norm);
	} else if(rec == 3) {
		TexturedVertex v2[5];
		
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
	
		EE_RT(p1, v2[0].p);
		EE_RT(p2, v2[1].p);
		EE_RT(p3, v2[2].p);
		EE_RT(p4, v2[3].p);
		ARX_DrawPrimitive(&v2[0], &v2[1], &v2[3]);
		ARX_DrawPrimitive(&v2[1], &v2[2], &v2[3]);
	}
}

void CCreateField::RenderSubDivFace(Vec3f * b, Vec3f * t, int b1, int b2, int t1, int t2) {
	Vec3f norm = (b[b1] + b[b2] + t[t1] + t[t2]) * 0.25f - eSrc;
	fnormalize(norm);
	RenderQuad(b[b1], b[b2], t[t1], t[t2], 1, norm);
}

void CCreateField::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
}

extern bool VisibleSphere(const Vec3f & pos, float radius);

void CCreateField::Render()
{
	if(!VisibleSphere(eSrc - Vec3f(0.f, 120.f, 0.f), 400.f))
		return;

	if(ulCurrentTime >= ulDuration)
		return;

	float fOneOnDuration = 1.f / (float)(ulDuration);
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
	b[0].x = x - smul;
	b[0].y = y;
	b[0].z = z - smul;

	b[1].x = x + smul;
	b[1].y = y;
	b[1].z = z - smul;

	b[2].x = x + smul;
	b[2].y = y;
	b[2].z = z + smul;

	b[3].x = x - smul;
	b[3].y = y;
	b[3].z = z + smul;

	// top points
	t[0].x = x - smul;
	t[0].y = y - 250 * ysize;
	t[0].z = z - smul;

	t[1].x = x + smul;
	t[1].y = y - 250 * ysize;
	t[1].z = z - smul;

	t[2].x = x + smul;
	t[2].y = y - 250 * ysize;
	t[2].z = z + smul;

	t[3].x = x - smul;
	t[3].y = y - 250 * ysize;
	t[3].z = z + smul;

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

	if(lightHandleIsValid(lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(lLightId);
		
		light->intensity = 0.7f + 2.3f * falpha;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb.r = 0.8f;
		light->rgb.g = 0.0f;
		light->rgb.b = 1.0f;
		light->pos.x = eSrc.x;
		light->pos.y = eSrc.y - 150;
		light->pos.z = eSrc.z;
		light->duration = 800;
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

void CSlowDown::Create(Vec3f aeSrc) {
	
	SetDuration(ulDuration);
	eSrc = aeSrc;
	eTarget = eSrc;
}

void CSlowDown::Update(float timeDelta) {
	ulCurrentTime += timeDelta;
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

CRiseDead::CRiseDead()
	: eSrc(Vec3f_ZERO)
	, tex_light(NULL)
	, end(40 - 1)
	, iSize(100)
	, bIntro(true)
	, sizeF(0)
	, fSizeIntro(0.f)
	, fRand(0.f)
	, fTexWrap(0)
	, ulDurationIntro(1000)
	, ulDurationRender(1000)
	, ulDurationOuttro(1000)
	, currframetime(0)
	, timestone(0)
	, nbstone(0)
{
	SetDuration(1000);
	ulCurrentTime = ulDurationIntro + ulDurationRender + ulDurationOuttro + 1;
	
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
	
	stone[0] = NULL;
	stone[1] = NULL;
	
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
	SetAngle(afBeta);
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

	v1a[0].x = eSrc.x - fBetaRadSin * 100;
	v1a[0].y = eSrc.y;
	v1a[0].z = eSrc.z + fBetaRadCos * 100;
	v1a[end].x = eSrc.x + fBetaRadSin * 100;
	v1a[end].y = eSrc.y;
	v1a[end].z = eSrc.z - fBetaRadCos * 100;

	v1b[0] = v1a[0];
	v1b[end] = v1a[end];

	sizeF = 200;
	Split(v1a, 0, end, 20);
	Split(v1b, 0, end, -20);

	sizeF = 200;
	Split(v1a, 0, end, 80);
	Split(v1b, 0, end, -80);
	
	for(i = 0; i <= end; i++) {
		vb[i] = va[i] = eSrc;
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

			Color4f col = Color4f(Color3f::white, 1.f - a);
			Draw3DObject(stone[tstone[nb].numstone], tstone[nb].ang, tstone[nb].pos, tstone[nb].scale, col);
			
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

void CRiseDead::Split(Vec3f * v, int a, int b, float yo)
{
	if(a != b) {
		int i = (int)((a + b) * 0.5f);

		if(i != a && i != b) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * frand2() * fBetaRadCos;
			v[i].y = v[0].y;// + (i+1)*5;
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * frand2() * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
}

void CRiseDead::RenderFissure()
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

	RenderMaterial mat = RenderMaterial::getCurrent();
	mat.setLayer(RenderMaterial::EffectForeground);

	//-------------------------------------------------------------------------
	// computation des sommets

	for(i = 0; i <= min(end, (int)fSizeIntro); i++) {
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
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toBGR();

	if(bIntro) {
		for(i = 0; i < min(end, (int)fSizeIntro); i++) {
			EE_RT(v1a[i], vr[0].p);
			EE_RT(v1b[i], vr[1].p);
			EE_RT(v1a[i+1], vr[2].p);
			EE_RT(v1b[i+1], vr[3].p);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	} else {
		for(i = 0; i < min(end, (int)fSizeIntro); i++) {
			EE_RT(va[i], vr[0].p);
			EE_RT(vb[i], vr[1].p);
			EE_RT(va[i+1], vr[2].p);
			EE_RT(vb[i+1], vr[3].p);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	mat.setBlendType(RenderMaterial::Additive);	
	vr[0].color = vr[1].color = Color::black.toBGR();
	vr[2].color = vr[3].color = Color3f(fColorBorder[0], fColorBorder[1], fColorBorder[2]).toBGR();

	for(i = 0; i < min(end, (int)fSizeIntro); i++) {
		vt[2] = va[i].p - (va[i].p - eSrc) * 0.2f;
		vt[3] = va[i + 1].p - (va[i + 1].p - eSrc) * 0.2f;
		
		EE_RT(vt[3], vr[0].p);
		EE_RT(vt[2], vr[1].p);
		EE_RT(va[i+1], vr[2].p);
		EE_RT(va[i], vr[3].p);
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
		
		vt[2] = vb[i].p - (vb[i].p - eSrc) * 0.2f;
		vt[3] = vb[i + 1].p - (vb[i + 1].p - eSrc) * 0.2f;
		
		EE_RT(vb[i], vr[3].p);
		EE_RT(vb[i+1], vr[2].p);
		EE_RT(vt[2], vr[1].p);
		EE_RT(vt[3], vr[0].p);
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

	target.x = eSrc.x ;
	target.y = eSrc.y + 1.5f * sizeF; 
	target.z = eSrc.z ;

	EE_RTP(vt[1], &vr[0]);
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
			vt[0] = va[i];
			vt[1] = va[i + 1];
			vt[2].x = va[i].x ;
			vt[2].y = va[i].y + (va[i].y - target.y) * 2;
			vt[2].z = va[i].z ;
			vt[3].x = va[i+1].x ;
			vt[3].y = va[i+1].y + (va[i+1].y - target.y) * 2;
			vt[3].z = va[i+1].z ;

			vr[0].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysa[i]).toBGR();
			vr[1].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysa[i + 1]).toBGR();
			vr[2].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysa[i]).toBGR();
			vr[3].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysa[i + 1]).toBGR();
			
			EE_RT(vt[0], vr[0].p);
			EE_RT(vt[1], vr[1].p);
			EE_RT(vt[2], vr[2].p);
			EE_RT(vt[3], vr[3].p);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
		if(i < fSizeIntro) {
			vt[0] = vb[i + 1];
			vt[1] = vb[i];
			vt[2].x = vb[i+1].x ;
			vt[2].y = vb[i+1].y + (vb[i+1].y - target.y) * 2;
			vt[2].z = vb[i+1].z ;
			vt[3].x = vb[i].x ;
			vt[3].y = vb[i].y + (vb[i].y - target.y) * 2;
			vt[3].z = vb[i].z ;

			vr[0].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysb[i]).toBGR();
			vr[1].color = (Color3f(fColorRays1[0], fColorRays1[1], fColorRays1[2]) * tfRaysb[i + 1]).toBGR();
			vr[2].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysb[i]).toBGR();
			vr[3].color = (Color3f(fColorRays2[0], fColorRays2[1], fColorRays2[2]) * tfRaysb[i + 1]).toBGR();

			EE_RT(vt[0], vr[0].p);
			EE_RT(vt[1], vr[1].p);
			EE_RT(vt[2], vr[2].p);
			EE_RT(vt[3], vr[3].p);
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}
}

void CRiseDead::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;
	currframetime = timeDelta;

	if(!arxtime.is_paused())
		this->timestone -= timeDelta;
}

//-----------------------------------------------------------------------------
// render the space time tearing
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
