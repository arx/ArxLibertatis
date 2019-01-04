/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/Fissure.h"

#include <cmath>

#include "animation/AnimationRender.h"
#include "core/GameTime.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"
#include "scene/Interactive.h"


FissureFx::FissureFx()
	: m_elapsed(0)
	, m_duration(0)
	, m_durationIntro(GameDurationMs(1000))
	, m_durationRender(GameDurationMs(1000))
	, m_durationOuttro(GameDurationMs(1000))
	, m_colorBorder(Color3f::white)
	, m_colorRays1(Color3f::white)
	, m_colorRays2(Color3f::black)
{
	m_duration = m_durationIntro + m_durationRender + m_durationOuttro;
}

void FissureFx::SetDuration(GameDuration alDurationIntro, GameDuration alDurationRender, GameDuration alDurationOuttro)
{
	m_durationIntro  = arx::clamp(alDurationIntro,  GameDurationMs(100), GameDurationMs(100000));
	m_durationRender = arx::clamp(alDurationRender, GameDurationMs(100), GameDurationMs(100000));
	m_durationOuttro = arx::clamp(alDurationOuttro, GameDurationMs(100), GameDurationMs(100000));
	
	m_elapsed = 0;
	m_duration = m_durationIntro + m_durationRender + m_durationOuttro;
}

void FissureFx::SetColorBorder(Color3f color)
{
	m_colorBorder = color;
}

void FissureFx::SetColorRays1(Color3f color)
{
	m_colorRays1 = color;
}

void FissureFx::SetColorRays2(Color3f color)
{
	m_colorRays2 = color;
}

CRiseDead::~CRiseDead() { }

CRiseDead::CRiseDead()
	: m_eSrc(0.f)
	, fBetaRadCos(0.f)
	, fBetaRadSin(0.f)
	, tex_light(TextureContainer::Load("graph/obj3d/textures/(fx)_tsu4"))
	, end(40 - 1)
	, iSize(100)
	, bIntro(true)
	, sizeF(0)
	, m_visibleNotches(0)
{
	m_elapsed = m_duration + GameDurationMs(1);
}

void CRiseDead::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(m_durationIntro, m_durationRender, m_durationOuttro);

	m_eSrc = aeSrc + Vec3f(0.f, -10.f, 0.f);
	
	float fBetaRad = glm::radians(afBeta);
	fBetaRadCos = glm::cos(fBetaRad);
	fBetaRadSin = glm::sin(fBetaRad);
	
	sizeF = 0;
	m_visibleNotches = 0;
	end = 40 - 1;
	bIntro = true;

	for(int i = 0; i < 40; i++) {
		tfRaysa[i] = Random::getf(0.f, 0.4f);
		tfRaysb[i] = Random::getf(0.f, 0.4f);
	}
	
	v1a[0].x = m_eSrc.x - fBetaRadSin * 100;
	v1a[0].y = m_eSrc.y;
	v1a[0].z = m_eSrc.z + fBetaRadCos * 100;
	v1a[end].x = m_eSrc.x + fBetaRadSin * 100;
	v1a[end].y = m_eSrc.y;
	v1a[end].z = m_eSrc.z - fBetaRadCos * 100;

	v1b[0] = v1a[0];
	v1b[end] = v1a[end];

	sizeF = 200;
	Split(v1a, 0, end, 20);
	Split(v1b, 0, end, -20);

	sizeF = 200;
	Split(v1a, 0, end, 80);
	Split(v1b, 0, end, -80);
	
	for(int i = 0; i <= end; i++) {
		vb[i] = va[i] = m_eSrc;
	}
	
	sizeF = 0;
	
	m_stones.Init(80.f);
}

GameDuration CRiseDead::GetDuration() {
	return m_duration;
}

void CRiseDead::Split(Vec3f * v, int a, int b, float yo) {
	
	if(a != b) {
		int i = int((a + b) * 0.5f);
		if(i != a && i != b) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * Random::getf(-1.f, 1.f) * fBetaRadCos;
			v[i].y = v[0].y;// + (i+1)*5;
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * Random::getf(-1.f, 1.f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
	
}

// TODO copy-paste spell effect Fissure
void CRiseDead::RenderFissure() {
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(false);
	mat.setWrapMode(TextureStage::WrapClamp);
	mat.setBlendType(RenderMaterial::Opaque);
	
	mat.setLayer(RenderMaterial::EffectForeground);

	//-------------------------------------------------------------------------
	// computation des sommets

	for(int i = 0; i <= std::min(end, m_visibleNotches); i++) {
		
		float ff;
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

		va[i].x += Random::getf(0.f, 0.5f) * fTempCos;
		va[i].z += Random::getf(0.f, 0.5f) * fTempSin;
		vb[i].x -= Random::getf(0.f, 0.5f) * fTempCos;
		vb[i].z -= Random::getf(0.f, 0.5f) * fTempSin;
	}

	//-------------------------------------------------------------------------
	// rendu de la fissure
	mat.setBlendType(RenderMaterial::Opaque);
	
	TexturedVertexUntransformed vr[4];
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toRGB();

	if(bIntro) {
		for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
			vr[0].p = v1a[i];
			vr[1].p = v1b[i];
			vr[2].p = v1a[i + 1];
			vr[3].p = v1b[i + 1];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	} else {
		for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
			vr[0].p = va[i];
			vr[1].p = vb[i];
			vr[2].p = va[i + 1];
			vr[3].p = vb[i + 1];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	mat.setBlendType(RenderMaterial::Additive);
	vr[0].color = vr[1].color = Color::black.toRGB();
	vr[2].color = vr[3].color = m_colorBorder.toRGB();
	
	Vec3f vt[4];
	for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
		
		vt[2] = va[i] - (va[i] - m_eSrc) * 0.2f;
		vt[3] = va[i + 1] - (va[i + 1] - m_eSrc) * 0.2f;
		vr[0].p = vt[3];
		vr[1].p = vt[2];
		vr[2].p = va[i + 1];
		vr[3].p = va[i];
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
		
		vt[2] = vb[i] - (vb[i] - m_eSrc) * 0.2f;
		vt[3] = vb[i + 1] - (vb[i + 1] - m_eSrc) * 0.2f;
		vr[3].p = vb[i];
		vr[2].p = vb[i + 1];
		vr[1].p = vt[2];
		vr[0].p = vt[3];
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
	
	Vec3f target = m_eSrc + Vec3f(0.f, 1.5f * sizeF, 0.f);
	
	vr[0].color = vr[1].color = m_colorRays1.toRGB();
	vr[2].color = vr[3].color = m_colorRays2.toRGB();
	vr[0].uv = Vec2f(0, 1);
	vr[1].uv = Vec2f(1, 1);
	vr[2].uv = Vec2f(0, 0);
	vr[3].uv = Vec2f(1, 0);
	
	for(int i = 0; i < end - 1; i++) {
		
		float t = Random::getf();
		if(t <= 0.15f) {
			if(tfRaysa[i] < 1.f) {
				tfRaysa[i] += 0.02f;
			}
			if(tfRaysa[i + 1] < 1.f) {
				tfRaysa[i + 1] += 0.01f;
			}
			if(tfRaysa[i] > 1.f) {
				tfRaysa[i] = 1.f;
			}
			if(tfRaysa[i + 1] > 1.f) {
				tfRaysa[i + 1] = 1.f;
			}
		}
		if(t >= 0.9f) {
			if(tfRaysa[i] > 0.f) {
				tfRaysa[i] -= 0.02f;
			}
			if(tfRaysa[i + 1] > 0.f) {
				tfRaysa[i + 1] -= 0.01f;
			}
			if(tfRaysa[i] < 0.f) {
				tfRaysa[i] = 0.0f;
			}
			if(tfRaysa[i + 1] < 0.f) {
				tfRaysa[i + 1] = 0.0f;
			}
		}
		
		float t2 = Random::getf();
		if(t2 <= 0.15f) {
			if(tfRaysb[i] < 1.f) {
				tfRaysb[i] += 0.02f;
			}
			if(tfRaysb[i + 1] < 1.f) {
				tfRaysb[i + 1] += 0.01f;
			}
			if(tfRaysb[i] > 1.f) {
				tfRaysb[i] = 1.f;
			}
			if(tfRaysb[i + 1] > 1.f) {
				tfRaysb[i + 1] = 1.f;
			}
		}
		if(t2 >= 0.9f) {
			if(tfRaysb[i] > 0.f) {
				tfRaysb[i] -= 0.02f;
			}
			if(tfRaysb[i + 1] > 0.f) {
				tfRaysb[i + 1] -= 0.01f;
			}
			if(tfRaysb[i] < 0.f) {
				tfRaysb[i] = 0.f;
			}
			if(tfRaysb[i + 1] < 0.f) {
				tfRaysb[i + 1] = 0.f;
			}
		}
		
		if(i < m_visibleNotches) {
			vt[0] = va[i];
			vt[1] = va[i + 1];
			vt[2].x = va[i].x;
			vt[2].y = va[i].y + (va[i].y - target.y) * 2;
			vt[2].z = va[i].z;
			vt[3].x = va[i + 1].x;
			vt[3].y = va[i + 1].y + (va[i + 1].y - target.y) * 2;
			vt[3].z = va[i + 1].z;
			vr[0].color = (m_colorRays1 * tfRaysa[i]).toRGB();
			vr[1].color = (m_colorRays1 * tfRaysa[i + 1]).toRGB();
			vr[2].color = (m_colorRays2 * tfRaysa[i]).toRGB();
			vr[3].color = (m_colorRays2 * tfRaysa[i + 1]).toRGB();
			vr[0].p = vt[0];
			vr[1].p = vt[1];
			vr[2].p = vt[2];
			vr[3].p = vt[3];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
		if(i < m_visibleNotches) {
			vt[0] = vb[i + 1];
			vt[1] = vb[i];
			vt[2].x = vb[i + 1].x;
			vt[2].y = vb[i + 1].y + (vb[i + 1].y - target.y) * 2;
			vt[2].z = vb[i + 1].z;
			vt[3].x = vb[i].x;
			vt[3].y = vb[i].y + (vb[i].y - target.y) * 2;
			vt[3].z = vb[i].z;
			vr[0].color = (m_colorRays1 * tfRaysb[i]).toRGB();
			vr[1].color = (m_colorRays1 * tfRaysb[i + 1]).toRGB();
			vr[2].color = (m_colorRays2 * tfRaysb[i]).toRGB();
			vr[3].color = (m_colorRays2 * tfRaysb[i + 1]).toRGB();
			vr[0].p = vt[0];
			vr[1].p = vt[1];
			vr[2].p = vt[2];
			vr[3].p = vt[3];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
	}
}

void CRiseDead::Update(GameDuration timeDelta)
{
	m_elapsed += timeDelta;
	
	m_stones.Update(timeDelta, m_eSrc);
}

// render the space time tearing
void CRiseDead::Render() {
	
	if(m_elapsed >= m_duration) {
		return;
	}
	
	if(m_elapsed < m_durationIntro) {
		// Render intro (opening + rays)
		float fOneOnDurationIntro = 1.f / toMsf(m_durationIntro);
		if(m_elapsed < GameDurationMsf(toMsf(m_durationIntro) * 0.666f)) {
			m_visibleNotches = int((end + 2) * fOneOnDurationIntro * (1.5f) * toMsf(m_elapsed));
			sizeF = 1;
		} else {
			bIntro = false;
			sizeF = (iSize) * (fOneOnDurationIntro * 3) * (toMsf(m_elapsed) - toMsf(m_durationIntro) * 0.666f);
		}
	} else if(m_elapsed < m_durationIntro + m_durationRender) {
		// Do nothing, just render
	} else if(m_elapsed < m_duration) {
		// Close it all
		sizeF = iSize - (iSize) * ((m_elapsed - (m_durationIntro + m_durationRender)) / m_durationOuttro);
	}
	
	RenderFissure();
	
	m_stones.DrawStone();
	
}

CSummonCreature::CSummonCreature()
	: m_eSrc(0.f)
	, fBetaRadCos(0.f)
	, fBetaRadSin(0.f)
	, tex_light(TextureContainer::Load("graph/obj3d/textures/(fx)_tsu4"))
	, end(0)
	, iSize(100)
	, bIntro(true)
	, sizeF(0.f)
	, m_visibleNotches(0)
{
	m_elapsed = m_duration + GameDurationMs(1);
}

void CSummonCreature::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(m_durationIntro, m_durationRender, m_durationOuttro);

	m_eSrc = aeSrc + Vec3f(0.f, -50.f, 0.f);
	
	float fBetaRad = glm::radians(afBeta);
	fBetaRadCos = glm::cos(fBetaRad);
	fBetaRadSin = glm::sin(fBetaRad);
	
	sizeF = 0;
	m_visibleNotches = 0;
	end = 40 - 1;
	bIntro = true;

	for(int i = 0; i < 40; i++) {
		tfRaysa[i] = Random::getf(0.f, 0.4f);
		tfRaysb[i] = Random::getf(0.f, 0.4f);
	}
	
	v1a[0] = m_eSrc - Vec3f(0.f, 100.f, 0.f);
	v1a[end] = m_eSrc + Vec3f(0.f, 100.f, 0.f);
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
			v1b[i].x = v1a[i].x + Random::getf(0.f, 20.0f);
		}

		if((v1b[i].z - v1a[i].z) > 20) {
			v1b[i].z = v1a[i].z + Random::getf(0.f, 20.0f);
		}
	}
	
	for(int i = 0; i <= end; i++) {
		vb[i] = va[i] = m_eSrc;
	}
	
	sizeF = 0;
}

void CSummonCreature::Split(Vec3f * v, int a, int b, float yo) {
	
	if(a != b) {
		int i = int((a + b) * 0.5f);
		if((i != a) && (i != b)) {
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * Random::getf(-1.f, 1.f) * (sizeF * 0.005f) * fBetaRadCos;
			v[i].y = v[0].y + (i + 1) * 5;
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * Random::getf(-1.f, 1.f) * (sizeF * 0.005f) * fBetaRadSin;
			Split(v, a, i, yo * 0.8f);
			Split(v, i, b, yo * 0.8f);
		}
	}
	
}

// TODO copy-paste spell effect Fissure
void CSummonCreature::RenderFissure() {
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(false);
	mat.setWrapMode(TextureStage::WrapClamp);
	mat.setBlendType(RenderMaterial::Opaque);
	
	
	mat.setLayer(RenderMaterial::EffectForeground);

	//-------------------------------------------------------------------------
	// computation des sommets

	for(int i = 0; i <= std::min(end, m_visibleNotches); i++) {
		
		float ff;
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

		va[i].x += Random::getf(0.f, 0.5f) * fTempCos;
		va[i].z += Random::getf(0.f, 0.5f) * fTempSin;
		vb[i].x -= Random::getf(0.f, 0.5f) * fTempCos;
		vb[i].z -= Random::getf(0.f, 0.5f) * fTempSin;
	}

	//-------------------------------------------------------------------------
	// rendu de la fissure
	mat.setBlendType(RenderMaterial::Opaque);
	
	TexturedVertexUntransformed vr[4];
	vr[0].color = vr[1].color = vr[2].color = vr[3].color = Color::black.toRGB();
	
	if(bIntro) {
		for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
			vr[0].p = v1a[i];
			vr[1].p = v1b[i];
			vr[2].p = v1a[i + 1];
			vr[3].p = v1b[i + 1];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	} else {
		for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
			vr[0].p = va[i];
			vr[1].p = vb[i];
			vr[2].p = va[i + 1];
			vr[3].p = vb[i + 1];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}

	//-------------------------------------------------------------------------
	// rendu de la bordure
	mat.setBlendType(RenderMaterial::Additive);
	vr[0].color = vr[1].color = Color::black.toRGB();
	vr[2].color = vr[3].color = m_colorBorder.toRGB();
	
	Vec3f vt[4];
	for(int i = 0; i < std::min(end, m_visibleNotches); i++) {
		
		vt[2] = va[i] - (va[i] - m_eSrc) * 0.2f;
		vt[3] = va[i + 1] - (va[i + 1] - m_eSrc) * 0.2f;
		vr[0].p = vt[3];
		vr[1].p = vt[2];
		vr[2].p = va[i + 1];
		vr[3].p = va[i];
		drawTriangle(mat, &vr[0]);
		drawTriangle(mat, &vr[1]);
		
		vt[2] = vb[i] - (vb[i] - m_eSrc) * 0.2f;
		vt[3] = vb[i + 1] - (vb[i + 1] - m_eSrc) * 0.2f;
		vr[3].p = vb[i];
		vr[2].p = vb[i + 1];
		vr[1].p = vt[2];
		vr[0].p = vt[3];
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
	
	Vec3f target = m_eSrc.x + Vec3f(-fBetaRadSin, 0.f, fBetaRadCos) * (1.5f * sizeF);
	
	vr[0].color = vr[1].color = m_colorRays1.toRGB();
	vr[2].color = vr[3].color = m_colorRays2.toRGB();

	vr[0].uv.x = 0.f;
	vr[0].uv.y = 1;
	vr[1].uv.x = 1.0f;
	vr[1].uv.y = 1;
	vr[2].uv.x = 0.f;
	vr[2].uv.y = 0;
	vr[3].uv.x = 1.0f;
	vr[3].uv.y = 0;

	for(int i = 0; i < end - 1; i++) {
		
		if(i < m_visibleNotches) {
			vt[0] = va[i];
			vt[1] = va[i + 1];
			vt[2] = va[i] + (va[i] - target) * 2.f;
			vt[3] = va[i + 1] + (va[i + 1] - target) * 2.f;
			
			vr[0].color = (m_colorRays1 * tfRaysa[i]).toRGB();
			vr[1].color = (m_colorRays1 * tfRaysa[i + 1]).toRGB();
			vr[2].color = (m_colorRays2 * tfRaysa[i]).toRGB();
			vr[3].color = (m_colorRays2 * tfRaysa[i + 1]).toRGB();
			
			vr[3].p = vt[0];
			vr[2].p = vt[1];
			vr[1].p = vt[2];
			vr[0].p = vt[3];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
		
		if(i < m_visibleNotches) {
			vt[0] = vb[i + 1];
			vt[1] = vb[i];
			vt[2] = vb[i + 1] + (vb[i + 1] - target) * 2.f;
			vt[3] = vb[i] + (vb[i] - target) * 2.f;
			
			vr[0].color = (m_colorRays1 * tfRaysb[i]).toRGB();
			vr[1].color = (m_colorRays1 * tfRaysb[i + 1]).toRGB();
			vr[2].color = (m_colorRays2 * tfRaysb[i]).toRGB();
			vr[3].color = (m_colorRays2 * tfRaysb[i + 1]).toRGB();
			
			vr[3].p = vt[0];
			vr[2].p = vt[1];
			vr[1].p = vt[2];
			vr[0].p = vt[3];
			drawTriangle(mat, &vr[0]);
			drawTriangle(mat, &vr[1]);
		}
	}
}

void CSummonCreature::Update(GameDuration timeDelta)
{
	m_elapsed += timeDelta;
}

// rendu de la déchirure spatio temporelle
void CSummonCreature::Render() {
	
	if(m_elapsed >= m_duration) {
		return;
	}
	
	if(m_elapsed < m_durationIntro) {
		// Render intro (opening + rays)
		float fOneOnDurationIntro = 1.f / toMsf(m_durationIntro);
		if(m_elapsed < GameDurationMsf(toMsf(m_durationIntro) * 0.666f)) {
			m_visibleNotches = int((end + 2) * fOneOnDurationIntro * (1.5f) * toMsf(m_elapsed));
			sizeF = 1;
		} else {
			bIntro = false;
			sizeF = (iSize) * (fOneOnDurationIntro * 3) * (toMsf(m_elapsed) - toMsf(m_durationIntro) * 0.666f);
		}
	} else if(m_elapsed < m_durationIntro + m_durationRender) {
		// Do nothing, just render
	} else if(m_elapsed < m_duration) {
		// Close it all
		sizeF = iSize - (iSize) * ((m_elapsed - (m_durationIntro + m_durationRender)) / m_durationOuttro);
	}
	
	RenderFissure();
	
}
