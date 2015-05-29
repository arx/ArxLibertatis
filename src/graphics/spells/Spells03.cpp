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

#include "graphics/spells/Spells03.h"

#include <climits>

#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "scene/Object.h"
#include "scene/Interactive.h"


CIceProjectile::~CIceProjectile() {
}

CIceProjectile::CIceProjectile()
	: fColor(1.f)
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	iNumber = MAX_ICE;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");
	
	iMax = MAX_ICE;
}

void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel, EntityHandle caster)
{
	iMax = (int)(30 + fLevel * 5.2f);
	
	SetDuration(ulDuration);
	
	float fspelldist	= static_cast<float>(iMax * 15);

	fspelldist = std::min(fspelldist, 200.0f);
	fspelldist = std::max(fspelldist, 450.0f);
	
	Vec3f s = aeSrc + Vec3f(0.f, -100.f, 0.f);
	Vec3f e = s + angleToVectorXZ(afBeta) * fspelldist;
	
	Vec3f h;
	if(!Visible(s, e, NULL, &h)) {
		e = h + angleToVectorXZ(afBeta) * 20.f;
	}

	float fd = fdist(s, e);

	float fCalc = ulDuration * (fd / fspelldist);
	SetDuration(checked_range_cast<unsigned long>(fCalc));

	float fDist = (fd / fspelldist) * iMax ;

	iNumber = checked_range_cast<int>(fDist);

	int end = iNumber / 2;
	
	Vec3f tv1a[MAX_ICE];
	tv1a[0] = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end] = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, Vec3f(80, 0, 80), Vec3f(0.5f, 1, 0.5f));

	for(int i = 0; i < iNumber; i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = rnd();
		
		Vec3f minSize;
		int randomRange;
		
		if(t < 0.5f) {
			icicle.type = 0;
			minSize = Vec3f(1.2f, 1.f, 1.2f);
			randomRange = 80;
		} else {
			icicle.type = 1;
			minSize = Vec3f(0.4f, 0.3f, 0.4f);
			randomRange = 40;
		}

		icicle.size = Vec3f_ZERO;
		icicle.sizeMax = randomVec() + Vec3f(0.f, 0.2f, 0.f);
		icicle.sizeMax = glm::max(icicle.sizeMax, minSize);
		
		int iNum = static_cast<int>(i / 2);
		icicle.pos = tv1a[iNum] + randomOffsetXZ(randomRange);
		
		DamageParameters damage;
		damage.pos = icicle.pos;
		damage.radius = 60.f;
		damage.damages = 0.1f * fLevel;
		damage.area = DAMAGE_FULL;
		damage.duration = ulDuration;
		damage.source = caster;
		damage.flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
		damage.type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
		DamageCreate(damage);
	}

	fColor = 1;
}

void CIceProjectile::Update(float timeDelta)
{
	ulCurrentTime += timeDelta;

	if(ulDuration - ulCurrentTime < 1000) {
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for(int i = 0; i < iNumber; i++) {
			m_icicles[i].size.y *= fColor;
		}
	}
}

void CIceProjectile::Render()
{
	if(ulCurrentTime >= ulDuration)
		return;
	
	RenderMaterial mat;
	mat.setCulling(Renderer::CullCW);
	mat.setDepthTest(true);
	mat.setBlendType(RenderMaterial::Screen);
	
	float fOneOnDuration = 1.f / (float)(ulDuration);
	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if(iMax > iNumber)
		iMax = iNumber;

	for(int i = 0; i < iMax; i++) {
		Icicle & icicle = m_icicles[i];
		
		icicle.size += Vec3f(0.1f);
		icicle.size = glm::clamp(icicle.size, Vec3f(0.f), icicle.sizeMax);
		
		Anglef stiteangle;
		stiteangle.setPitch(glm::cos(glm::radians(icicle.pos.x)) * 360);
		stiteangle.setYaw(0);
		stiteangle.setRoll(0);
		
		float tt = icicle.sizeMax.y * fColor;
		Color3f stitecolor = Color3f(0.7f, 0.7f, 0.9f) * tt;
		stitecolor = componentwise_min(stitecolor, Color3f(1.f, 1.f, 1.f));
		
		EERIE_3DOBJ * eobj = (icicle.type == 0) ? smotte : stite;
		
		Draw3DObject(eobj, stiteangle, icicle.pos, icicle.size, stitecolor, mat);
	}
	
	for(int i = 0; i < std::min(iNumber, iMax + 1); i++) {
		Icicle & icicle = m_icicles[i];
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = icicle.pos + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = std::min(2000.f + rnd() * 2000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p2;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		} else if(t > 0.095f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = icicle.pos + randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = std::min(2000.f + rnd() * 1000.f,
				              ulDuration - ulCurrentTime + 500.0f * rnd());
				pd->tolive = checked_range_cast<unsigned long>(t);
				pd->tc = tex_p1;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
		}
	}
}
