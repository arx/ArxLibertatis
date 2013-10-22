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

#include "graphics/spells/Spells10.h"

#include <algorithm>

#include "core/GameTime.h"

#include "game/EntityManager.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells07.h"
#include "graphics/spells/Spells09.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/Interactive.h"

CMassLightning::CMassLightning(long nbmissiles)
{
	SetDuration(2000);
	pTab = new CLightning*[10];
	number = std::min(10L, nbmissiles);

	for(int i = 0; i < number; i++) {
		pTab[i] = new CLightning();
		pTab[i]->fDamage = 2;
	}
}

void CMassLightning::Create(Vec3f aePos, float afBeta = 0) {
	
	(void)afBeta;
	
	long lMax = 0;
	Vec3f eTarget;
	float ft = 360.0f / (float)number;

	for(int i = 0; i < number; i++) {
		eTarget.x = aePos.x - EEsin(radians(i * ft)) * 500.0f;
		eTarget.y = aePos.y;
		eTarget.z = aePos.z + EEcos(radians(i * ft)) * 500.0f;
		pTab[i]->Create(aePos, eTarget, 0);
		long lTime = ulDuration + Random::get(0, 5000);
		pTab[i]->SetDuration(lTime);
		lMax = std::max(lMax, lTime);
		pTab[i]->spellinstance = this->spellinstance;
		pTab[i]->SetColor1(1.f, 0.75f, 0.75f); // middle
		pTab[i]->SetColor2(0.3f, 0.f, 0.f); // extremities
	}

	SetDuration(lMax + 1000);
}

void CMassLightning::Update(unsigned long _ulTime)
{
	for(int i = 0; i < number; i++) {
		pTab[i]->Update(_ulTime);
	}
}

void CMassLightning::Render()
{
	for(int i = 0; i < number; i++) {
		pTab[i]->Render();
	}
}

CControlTarget::CControlTarget() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(8000);
	ulCurrentTime = ulDuration + 1;
	
	tex_mm = TextureContainer::Load("graph/obj3d/textures/(fx)_ctrl_target");
	
	fColor[0] = 1;
	fColor[1] = 1;
	fColor[2] = 0;
	
	fColor1[0] = 0.8f;
	fColor1[1] = 0.6f;
	fColor1[2] = 0.2f;
}

void CControlTarget::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	fSize = 1;
	bDone = true;
	eTarget = eSrc + Vec3f(-fBetaRadSin * 1000.f, 100.f, fBetaRadCos * 1000.f);
	
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i]) {
			eTarget = entities[i]->pos;
		}
	}
	
	end = 20 - 1;
	v1a[0].p = eSrc + Vec3f(0.f, 100.f, 0.f);
	v1a[end].p = eTarget;
	
	Vec3f h;
	Vec3f s = eSrc;
	Vec3f e = eSrc;
	int i = 0;
	while(Visible(&s, &e, NULL, &h) && i < 20) {
		e.x -= fBetaRadSin * 50;
		e.z += fBetaRadCos * 50;
		i++;
	}
	
	pathways[0].p = eSrc + Vec3f(0.f, 100.f, 0.f);
	pathways[9].p = eTarget;
	Split(pathways, 0, 9, 150);
	
	for(int i = 0; i < 9; i++) {
		if(pathways[i].p.y >= eSrc.y + 150) {
			pathways[i].p.y = eSrc.y + 150;
		}
	}
	
	fTrail = 0;
}

void CControlTarget::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CControlTarget::Render()
{
	int i = 0;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetTexture(0, tex_mm);

	// -------------------
	fTrail += 1;

	if (fTrail >= 300) fTrail = 0;

	int n = BEZIERPrecision;
	float delta = 1.0f / n;

	fTrail = (ulCurrentTime * fOneOnDuration) * 9 * (n + 2);

	Vec3f v;
	
	int arx_check_init = -1;
	Vec3f newpos = Vec3f_ZERO;
	Vec3f lastpos = pathways[0].p;
	
	for(i = 0; i < 9; i++) {
		int kp		= i;
		int kpprec	= (i > 0) ? kp - 1 : kp ;
		int kpsuiv	= kp + 1 ;
		int kpsuivsuiv = (i < (9 - 2)) ? kpsuiv + 1 : kpsuiv;

		for(int toto = 1; toto < n; toto++) {
			if(fTrail < i * n + toto)
				break;

			float t = toto * delta;

			float t1 = t;
			float t2 = t1 * t1 ;
			float t3 = t2 * t1 ;
			float f0 = 2.f * t3 - 3.f * t2 + 1.f ;
			float f1 = -2.f * t3 + 3.f * t2 ;
			float f2 = t3 - 2.f * t2 + t1 ;
			float f3 = t3 - t2 ;

			float val = pathways[kpsuiv].p.x;
			float p0 = 0.5f * (val - pathways[kpprec].p.x) ;
			float p1 = 0.5f * (pathways[kpsuivsuiv].p.x - pathways[kp].p.x) ;
			v.x = f0 * pathways[kp].p.x + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].p.y ;
			p0 = 0.5f * (val - pathways[kpprec].p.y) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].p.y - pathways[kp].p.y) ;
			v.y = f0 * pathways[kp].p.y + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].p.z ;
			p0 = 0.5f * (val - pathways[kpprec].p.z) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].p.z - pathways[kp].p.z) ;
			v.z = f0 * pathways[kp].p.z + f1 * val + f2 * p0 + f3 * p1 ;
			
			newpos = v;
			
			if(fTrail - (i * n + toto) <= 70) {
				float c = 1.0f - (fTrail - (i * n + toto)) / 70.0f;
				PARTICLE_DEF * pd = createParticle();
				if(pd) {
					pd->ov = lastpos;
					pd->siz = 5 * c;
					pd->tolive = Random::get(10, 110);
					pd->tc = tex_mm;
					pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					pd->fparam = 0.0000001f;
					pd->rgb = Color3f::gray(c);
				}
			}
			
			std::swap(lastpos, newpos);
			++arx_check_init;
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = lastpos;
				pd->siz = 5;
				pd->tolive = Random::get(10, 110);
				pd->tc = tex_mm;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f::gray(0.1f);
			}
		}
	}
	
	arx_assert(arx_check_init >= 0);
	
	eCurPos = lastpos;
}

CMassIncinerate::~CMassIncinerate()
{
}

void CMassIncinerate::Create(Vec3f aePos, float afBeta = 0) {
	
	(void)afBeta;
	
	aePos.y += 150.0f;

	for(int i = 0; i < 10; i++) {
		pTabIncinerate[i]->Create(aePos, i * 36.f);
	}
}

void CMassIncinerate::Update(unsigned long _ulTime)
{
	for(int i = 0; i < 10; i++) {
		pTabIncinerate[i]->Update(_ulTime);
	}
}

void CMassIncinerate::Render()
{
	for(int i = 0; i < 10; i++) {
		pTabIncinerate[i]->Render();
	}
}

