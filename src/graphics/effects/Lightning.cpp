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

#include "graphics/effects/Lightning.h"

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "physics/Collisions.h"

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Scene.h"

struct CLightning::LIGHTNING {
	Vec3f eStart;
	Vec3f eVect;
	int anb;
	int anbrec;
	bool abFollow;
	int aParent;
	Vec3f fAngleMin;
	Vec3f fAngleMax;
	
	LIGHTNING()
		: eStart(Vec3f_ZERO)
		, eVect(Vec3f_ZERO)
		, anb(0)
		, anbrec(0)
		, abFollow(false)
		, aParent(0)
		, fAngleMin(Vec3f_ZERO)
		, fAngleMax(Vec3f_ZERO)
	{ }
};

CLightning::CLightning()
	: m_pos(Vec3f_ZERO)
	, m_beta(0.f)
	, m_alpha(0.f)
	, m_caster()
	, m_level(1.f)
	, m_fDamage(1)
	, m_isMassLightning(false)
	, m_nbtotal(0)
	, m_lNbSegments(40)
	, m_invNbSegments(1.0f / 40.0f)
	, m_fLengthMin(5.0f)
	, m_fLengthMax(40.0f)
	, m_fAngleMin(5.0f, 5.0f, 5.0f)
	, m_fAngleMax(32.0f, 32.0f, 32.0f)
	, m_iTTL(0)
{
	SetDuration(ArxDurationMs(2000));
	m_elapsed = m_duration + ArxDurationMs(1);
	
	m_tex_light = NULL;
	fTotoro = 0;
	fMySize = 2;
}

//------------------------------------------------------------------------------
// Params une méchante struct
//------------------------------------------------------------------------------

void CLightning::BuildS(LIGHTNING * pLInfo)
{
	Vec3f astart = pLInfo->eStart;
	Vec3f avect = pLInfo->eVect;

	if(pLInfo->anb > 0 && m_nbtotal < (MAX_NODES - 1)) {
		m_nbtotal++;
		int moi = m_nbtotal;

		if(pLInfo->abFollow) {
			avect = glm::normalize(m_eDest - pLInfo->eStart);
		}

		Vec3f fAngle;
		fAngle.x = Random::getf(-1.f, 1.f) * (pLInfo->fAngleMax.x - pLInfo->fAngleMin.x) + pLInfo->fAngleMin.x;
		fAngle.y = Random::getf(-1.f, 1.f) * (pLInfo->fAngleMax.y - pLInfo->fAngleMin.y) + pLInfo->fAngleMin.y;
		fAngle.z = Random::getf(-1.f, 1.f) * (pLInfo->fAngleMax.z - pLInfo->fAngleMin.z) + pLInfo->fAngleMin.z;

		Vec3f av;
		av.x = glm::cos(glm::acos(avect.x) - glm::radians(fAngle.x));
		av.y = glm::sin(glm::asin(avect.y) - glm::radians(fAngle.y));
		av.z = glm::tan(glm::atan(avect.z) - glm::radians(fAngle.z));
		av = glm::normalize(av);
		avect = av;

		float ts = Random::getf();
		av *= ts * (m_fLengthMax - m_fLengthMin) * pLInfo->anb * m_invNbSegments + m_fLengthMin;

		astart += av;
		pLInfo->eStart = astart;
		
		m_cnodetab[m_nbtotal].pos = pLInfo->eStart;
		m_cnodetab[m_nbtotal].size = m_cnodetab[0].size * pLInfo->anb * m_invNbSegments;
		m_cnodetab[m_nbtotal].parent = pLInfo->aParent;
		
		int anb = pLInfo->anb;
		int anbrec = pLInfo->anbrec;

		float p = Random::getf();

		if(p <= 0.15f && pLInfo->anbrec < 7) {
			float m = Random::getf();

			if(pLInfo->abFollow) {
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->abFollow = false;
				pLInfo->anb =  anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = m_fAngleMin;
				pLInfo->fAngleMax = m_fAngleMax;
				
				BuildS(pLInfo);

				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->abFollow = true;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = m_fAngleMin;
				pLInfo->fAngleMax = m_fAngleMax;
				
				BuildS(pLInfo);
			} else {
				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = m_fAngleMin;
				pLInfo->fAngleMax = m_fAngleMax;
				
				BuildS(pLInfo);

				pLInfo->abFollow = false;
				pLInfo->eStart = astart;
				pLInfo->eVect = avect;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleMin = m_fAngleMin;
				pLInfo->fAngleMax = m_fAngleMax;
				
				BuildS(pLInfo);
			}
		} else {
			if(Random::getf() <= 0.10f) {
				pLInfo->abFollow = true;
			}

			pLInfo->eStart = astart;
			pLInfo->eVect = avect;
			pLInfo->anb = anb - 1;
			pLInfo->anbrec = anbrec;
			pLInfo->aParent = moi;
			pLInfo->fAngleMin = m_fAngleMin;
			pLInfo->fAngleMax = m_fAngleMax;
			
			BuildS(pLInfo);
		}
	}
}

void CLightning::Create(Vec3f aeFrom, Vec3f aeTo) {
	
	SetDuration(m_duration);
	
	m_eSrc = aeFrom;
	m_eDest = aeTo;
	
	ReCreate(15);
}

void CLightning::ReCreate(float rootSize)
{
	m_nbtotal = 0;

	if(m_nbtotal == 0) {
		LIGHTNING LInfo = LIGHTNING();
		
		LInfo.eStart = m_eSrc;
		LInfo.eVect = m_eDest - m_eSrc;
		LInfo.anb = m_lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fAngleMin = m_fAngleMin;
		LInfo.fAngleMax = m_fAngleMax;
		
		m_cnodetab[0].pos = m_eSrc;
		m_cnodetab[0].size = rootSize;
		m_cnodetab[0].parent = 0;

		BuildS(&LInfo);
	}
	
	m_iTTL = Random::get(500, 1500);
}

void CLightning::Update(ArxDuration timeDelta)
{
	m_elapsed += timeDelta;
	m_iTTL -= timeDelta;
	fTotoro += 8;

	if(fMySize > 0.3f)
		fMySize -= 0.1f;
}

void CLightning::Render()
{
	if(m_elapsed >= m_duration)
		return;
	
	if(m_iTTL <= 0) {
		fTotoro = 0;
		fMySize = 2;
		ReCreate(8);
	}
	
	Vec3f ePos;
	
	float fBeta = 0.f;
	float falpha = 0.f;
	
	if(m_isMassLightning) {
		ePos = Vec3f_ZERO;
	} else {
		ePos = m_pos;
		fBeta = m_beta;
		falpha = m_alpha;
	}
	
	float f = 1.5f * fMySize;
	m_cnodetab[0].f = randomVec(-f, f);
	
	RenderMaterial mat;
	mat.setCulling(CullNone);
	mat.setDepthTest(false);
	mat.setBlendType(RenderMaterial::Additive);
	
	float fbeta = fBeta + Random::getf(0.f, 2.f) * fMySize;

	for(size_t i = 0; i < m_nbtotal && i <= fTotoro; i++) {
		CLightningNode & node = m_cnodetab[i];
		
		Vec3f astart = m_cnodetab[node.parent].pos + m_cnodetab[node.parent].f;
		float temp = 1.5f * fMySize;
		Vec3f z_z = m_cnodetab[node.parent].f + randomVec(-temp, temp);
		float zz = node.size + node.size * Random::getf(0.f, 0.3f);
		float xx = node.size * glm::cos(glm::radians(-fbeta));
		node.f = z_z;
		
		Vec3f a = node.pos + z_z;
		if(!m_isMassLightning) {
			Vec3f vv2;
			Vec3f vv1 = astart;
			vv1 = VRotateX(vv1, (falpha));  
			vv2 = VRotateY(vv1, 180 - MAKEANGLE(fBeta));
			astart = vv2;
			vv1 = a;
			vv1 = VRotateX(vv1, (falpha)); 
			vv2 = VRotateY(vv1, 180 - MAKEANGLE(fBeta));
			a = vv2;
			astart += ePos;
			a += ePos;
		}
		
		if(i % 4 == 0) {
			Sphere sphere;
			sphere.origin = a;
			sphere.radius = std::min(node.size, 50.f);

			if(CheckAnythingInSphere(sphere, m_caster, CAS_NO_SAME_GROUP)) {

				DamageParameters damage;
				damage.pos = sphere.origin;
				damage.radius = sphere.radius;
				damage.damages = m_fDamage * m_level * ( 1.0f / 3 );
				damage.area = DAMAGE_FULL;
				damage.duration = ArxDurationMs(1);
				damage.source = m_caster;
				damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damage.type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_LIGHTNING;
				DamageCreate(damage);
			}
		}
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_ZERO;
		q.v[2].uv = Vec2f_Y_AXIS;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart + Vec3f(0.f, zz, 0.f);
		q.v[2].p = a + Vec3f(0.f, zz, 0.f);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		{
		TexturedQuad q;

		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_X_AXIS;
		q.v[2].uv = Vec2f_ONE;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart - Vec3f(0.f, zz, 0.f);
		q.v[2].p = a - Vec3f(0.f, zz, 0.f);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		zz *= glm::sin(glm::radians(fbeta));
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_X_AXIS;
		q.v[2].uv = Vec2f_ONE;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart + Vec3f(xx, 0.f, zz);
		q.v[2].p = a + Vec3f(xx, 0.f, zz);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
		
		{
		TexturedQuad q;
		
		q.v[0].color = Color(255, 255, 255, 255).toRGBA();
		q.v[1].color = Color(0, 0, 90, 255).toRGBA();
		q.v[2].color = Color(0, 0, 90, 255).toRGBA();
		q.v[3].color = Color(255, 255, 255, 255).toRGBA();
		q.v[0].uv = Vec2f(0.5f, 0.f);
		q.v[1].uv = Vec2f_ZERO;
		q.v[2].uv = Vec2f_Y_AXIS;
		q.v[3].uv = Vec2f(0.5f, 1.f);
		q.v[0].p = astart;
		q.v[1].p = astart - Vec3f(xx, 0.f, zz);
		q.v[2].p = a - Vec3f(xx, 0.f, zz);
		q.v[3].p = a;
		
		drawQuadRTP(mat, q);
		}
	}
}
