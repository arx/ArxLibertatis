/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/effects/Field.h"

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

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Scene.h"

CCreateField::CCreateField()
	: eSrc(0.f)
	, youp(true)
	, fwrap(0.f)
	, size(0.1f)
	, ft(0.0f)
	, fglow(0.f)
	, falpha(0.f)
{
	tex_jelly = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu3");
}

void CCreateField::Create(Vec3f aeSrc) {
	
	eSrc = aeSrc;
	size = 0.1f;
	ft = 0.0f;
	fglow = 0;
	youp = true;
}

void CCreateField::RenderQuad(const Vec3f & p1, const Vec3f & p2, const Vec3f & p3, const Vec3f & p4, int rec, Vec3f norm, RenderMaterial & mat) {
	
	if(rec < 3) {
		
		rec++;
		
		Vec3f v[5] = {
			p1 + (p3 - p1) * 0.5f,
			p1 + (p4 - p1) * 0.5f,
			p2 + (p3 - p2) * 0.5f,
			p4 + (p3 - p4) * 0.5f,
			p1 + (p2 - p1) * 0.5f,
		};
		
		float patchsize = 0.005f;
		v[0].x += glm::sin(glm::radians((v[0].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[0].y += glm::sin(glm::radians((v[0].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[0].z += glm::sin(glm::radians((v[0].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[1].x += glm::sin(glm::radians((v[1].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[1].y += glm::sin(glm::radians((v[1].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[1].z += glm::sin(glm::radians((v[1].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[2].x += glm::sin(glm::radians((v[2].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[2].y += glm::sin(glm::radians((v[2].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[2].z += glm::sin(glm::radians((v[2].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[3].x += glm::sin(glm::radians((v[3].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[3].y += glm::sin(glm::radians((v[3].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[3].z += glm::sin(glm::radians((v[3].z - eSrc.z) * patchsize + fwrap)) * 5;
		v[4].x += glm::sin(glm::radians((v[4].x - eSrc.x) * patchsize + fwrap)) * 5;
		v[4].y += glm::sin(glm::radians((v[4].y - eSrc.y) * patchsize + fwrap)) * 5;
		v[4].z += glm::sin(glm::radians((v[4].z - eSrc.z) * patchsize + fwrap)) * 5;

		RenderQuad(p1, v[4], v[0], v[1], rec, norm, mat);
		RenderQuad(v[4], p2, v[2], v[0], rec, norm, mat);
		RenderQuad(v[0], v[2], p3, v[3], rec, norm, mat);
		RenderQuad(v[1], v[0], v[3], p4, rec, norm, mat);
	} else if(rec == 3) {
		float zab = glm::sin(glm::radians(ft));
		
		TexturedQuad q;
		
		q.v[0].uv.x = 0 + zab;
		q.v[0].uv.y = 0 + zab;
		q.v[1].uv.x = 1 + zab;
		q.v[1].uv.y = 0 + zab;
		q.v[2].uv.x = 1 + zab;
		q.v[2].uv.y = 1 + zab;
		q.v[3].uv.x = 0 + zab;
		q.v[3].uv.y = 1 + zab;

		q.v[1].color = q.v[2].color = Color3f(falpha * .3f + Random::getf(0.f, .025f), 0.f, falpha * .5f + Random::getf(0.f, .025f)).toRGB();
		q.v[0].color = q.v[3].color = Color3f(falpha * .3f + Random::getf(0.f, .025f), 0.f, falpha * .5f + Random::getf(0.f, .025f)).toRGB();
	
		q.v[0].p = p1;
		q.v[1].p = p2;
		q.v[2].p = p3;
		q.v[3].p = p4;
		
		drawQuadRTP(mat, q);
	}
}

void CCreateField::RenderSubDivFace(Vec3f * b, Vec3f * t, int b1, int b2, int t1, int t2, RenderMaterial & mat) {
	Vec3f norm = (b[b1] + b[b2] + t[t1] + t[t2]) * 0.25f - eSrc;
	norm = glm::normalize(norm);
	RenderQuad(b[b1], b[b2], t[t1], t[t2], 1, norm, mat);
}

void CCreateField::Update(GameDuration timeDelta)
{
	m_elapsed += timeDelta;
}

void CCreateField::Render()
{
	if(!VisibleSphere(Sphere(eSrc - Vec3f(0.f, 120.f, 0.f), 400.f)))
		return;
	
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
	
	float ysize = std::min(1.0f, m_elapsed / GameDurationMs(1000));
	
	if(ysize >= 1.0f) {
		size = std::min(1.0f, (m_elapsed - GameDurationMs(1000)) / GameDurationMs(1000));
		size = std::max(size, 0.1f);
	}

	// ondulation
	ft += 0.01f;

	if(ft > 360.0f) {
		ft = 0.0f;
	}

	falpha = glm::sin(glm::radians(fglow)) + Random::getf(0.f, 0.2f);
	falpha = glm::clamp(falpha, 0.f, 1.f);
	
	float smul = 100 * size;

	// bottom points
	Vec3f b[4] = {
		eSrc + Vec3f(-smul, 0.f, -smul),
		eSrc + Vec3f(smul, 0.f, -smul),
		eSrc + Vec3f(smul, 0.f, smul),
		eSrc + Vec3f(-smul, 0.f, smul)
	};
	
	// top points
	Vec3f t[4] = {
		b[0] + Vec3f(0.f, -250 * ysize, 0.f),
		b[1] + Vec3f(0.f, -250 * ysize, 0.f),
		b[2] + Vec3f(0.f, -250 * ysize, 0.f),
		b[3] + Vec3f(0.f, -250 * ysize, 0.f)
	};
	
	fwrap -= 5.0f; // TODO ignores the frame delay
	while(fwrap < 0) {
		fwrap += 360;
	}
	
	RenderMaterial mat;
	mat.setTexture(tex_jelly);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Additive);
	
	RenderSubDivFace(b, b, 0, 1, 2, 3, mat);
	RenderSubDivFace(t, t, 0, 3, 2, 1, mat);
	RenderSubDivFace(b, t, 1, 0, 0, 1, mat);
	RenderSubDivFace(b, t, 3, 2, 2, 3, mat);
	RenderSubDivFace(b, t, 0, 3, 3, 0, mat);
	RenderSubDivFace(b, t, 2, 1, 1, 2, mat);
	
	EERIE_LIGHT * light = lightHandleGet(lLightId);
	if(light) {
		light->intensity = 0.7f + 2.3f * falpha;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f(0.8f, 0.0f, 1.0f);
		light->pos = eSrc + Vec3f(0.f, -150.f, 0.f);
		light->duration = GameDurationMs(800);
	}
	
}
