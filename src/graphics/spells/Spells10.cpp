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

#include "graphics/spells/Spells10.h"

#include "core/GameTime.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells07.h"
#include "graphics/spells/Spells09.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/Interactive.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMassLightning::CMassLightning(long nbmissiles)
{
	SetDuration(2000);
	pTab = new CLightning*[10];
	number = std::min(10L, nbmissiles);

	for (int i = 0; i < number; i++)
	{
		pTab[i] = new CLightning();
		pTab[i]->fDamage = 2;
	}
}

//-----------------------------------------------------------------------------
void CMassLightning::Create(Vec3f aePos, float afBeta = 0) {
	
	(void)afBeta;
	
	long lMax = 0;
	Vec3f eTarget;
	float ft = 360.0f / (float)number;

	for (int i = 0; i < number; i++)
	{
		eTarget.x = aePos.x - EEsin(radians(i * ft)) * 500.0f;
		eTarget.y = aePos.y;
		eTarget.z = aePos.z + EEcos(radians(i * ft)) * 500.0f;
		pTab[i]->Create(aePos, eTarget, 0);
		long lTime = (long)(ulDuration + rnd() * 5000.0f);
		pTab[i]->SetDuration(lTime);
		lMax = std::max(lMax, lTime);
		pTab[i]->spellinstance = this->spellinstance;
		pTab[i]->SetColor1(1.f, 0.75f, 0.75f); // middle
		pTab[i]->SetColor2(0.3f, 0.f, 0.f); // extremities
	}

	SetDuration(lMax + 1000);
}

//-----------------------------------------------------------------------------
void CMassLightning::Update(unsigned long _ulTime)
{
	for (int i = 0; i < number; i++)
	{
		pTab[i]->Update(_ulTime);
	}
}

//-----------------------------------------------------------------------------
float CMassLightning::Render()
{
	for (int i = 0; i < number; i++)
	{
		pTab[i]->Render();
	}

	return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CControlTarget::CControlTarget()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

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
//-----------------------------------------------------------------------------
void CControlTarget::Create(Vec3f aeSrc, float afBeta)
{
	int i;

	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	fSize = 1;
	bDone = true;

	eTarget.x = eSrc.x - fBetaRadSin * 1000;
	eTarget.y = eSrc.y + 100;
	eTarget.z = eSrc.z + fBetaRadCos * 1000;

	for (i = 1; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			eTarget.x = inter.iobj[i]->pos.x;
			eTarget.y = inter.iobj[i]->pos.y;
			eTarget.z = inter.iobj[i]->pos.z;
		}
	}

	end = 20 - 1;
	v1a[0].p.x = eSrc.x;
	v1a[0].p.y = eSrc.y + 100;
	v1a[0].p.z = eSrc.z;
	v1a[end].p.x = eTarget.x;
	v1a[end].p.y = eTarget.y;
	v1a[end].p.z = eTarget.z;

	Vec3f s, e, h;
	s.x = v1a[0].p.x;
	s.y = v1a[0].p.y;
	s.z = v1a[0].p.z;
	e.x = v1a[end].p.x;
	e.y = v1a[end].p.y;
	e.z = v1a[end].p.z;
	//	GenereArcElectrique(&s,&e,v1a,end,0);

	s.x = eSrc.x;
	s.y = eSrc.y;
	s.z = eSrc.z;
	e.x = eSrc.x;
	e.y = eSrc.y;
	e.z = eSrc.z;

	i = 0;

	while (Visible(&s, &e, NULL, &h) && i < 20)
	{
		e.x -= fBetaRadSin * 50;
		e.z += fBetaRadCos * 50;

		i++;
	}

	pathways[0].p.x = eSrc.x;
	pathways[0].p.y = eSrc.y + 100;
	pathways[0].p.z = eSrc.z;
	pathways[9].p.x = eTarget.x;
	pathways[9].p.y = eTarget.y;
	pathways[9].p.z = eTarget.z;
	Split(pathways, 0, 9, 150);

	for (i = 0; i < 9; i++)
	{
		if (pathways[i].p.y >= eSrc.y + 150)
		{
			pathways[i].p.y = eSrc.y + 150;
		}
	}

	fTrail = 0;
}
//-----------------------------------------------------------------------------
void CControlTarget::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CControlTarget::Render()
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

	Vec3f lastpos, newpos;
	Vec3f v;

	int arx_check_init = -1;
	newpos.x = 0;
	newpos.y = 0;
	newpos.z = 0;


	lastpos.x = pathways[0].p.x;
	lastpos.y = pathways[0].p.y;
	lastpos.z = pathways[0].p.z;

	for (i = 0; i < 9; i++)
	{
		int kp		= i;
		int kpprec	= (i > 0) ? kp - 1 : kp ;
		int kpsuiv	= kp + 1 ;
		int kpsuivsuiv = (i < (9 - 2)) ? kpsuiv + 1 : kpsuiv;

		for (int toto = 1; toto < n; toto++)
		{
			if (fTrail < i * n + toto) break;

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

			newpos.x = v.x;
			newpos.y = v.y;
			newpos.z = v.z;

			if (!((fTrail - (i * n + toto)) > 70))
			{
				float c = 1.0f - ((fTrail - (i * n + toto)) / 70.0f);

				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!arxtime.is_paused()))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					particle[j].ov.x		= lastpos.x;
					particle[j].ov.y		= lastpos.y;
					particle[j].ov.z		= lastpos.z;
					particle[j].move.x		= 0;
					particle[j].move.y		= 0;
					particle[j].move.z		= 0;
					particle[j].siz			= 5 * c;
					particle[j].tolive		= 10 + (unsigned long)(float)(rnd() * 100.f);
					particle[j].scale.x 	= 1;
					particle[j].scale.y 	= 1;
					particle[j].scale.z 	= 1;
					particle[j].timcreation = (long)arxtime;
					particle[j].tc			= tex_mm;
					particle[j].special 	= FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam		= 0.0000001f;
					particle[j].rgb = Color3f::gray(c);
				}
			}

			float nx = lastpos.x;
			float ny = lastpos.y;
			float nz = lastpos.z;

			lastpos.x = newpos.x;
			lastpos.y = newpos.y;
			lastpos.z = newpos.z;

			newpos.x = nx;
			newpos.y = ny;
			newpos.z = nz;
			++arx_check_init;

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!arxtime.is_paused()))
			{
				ParticleCount++;
				particle[j].exist	= 1;
				particle[j].zdec	= 0;

				particle[j].ov.x	= lastpos.x;
				particle[j].ov.y	= lastpos.y;
				particle[j].ov.z	= lastpos.z;
				particle[j].move.x	= 0;
				particle[j].move.y	= 0;
				particle[j].move.z	= 0;
				particle[j].siz		= 5;
				particle[j].tolive	= 10 + (unsigned long)(float)(rnd() * 100.f);
				particle[j].scale.x = 1;
				particle[j].scale.y = 1;
				particle[j].scale.z = 1;
				particle[j].timcreation = (long)arxtime;
				particle[j].tc		= tex_mm;
				particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam	= 0.0000001f;
				particle[j].rgb = Color3f::gray(0.1f);
			}
		}
	}
	
	arx_assert(arx_check_init >= 0);
	
	eCurPos = lastpos;
	
	return 1;
}

//---------------------------------------------------------------------
CMassIncinerate::~CMassIncinerate()
{
}

//---------------------------------------------------------------------
void CMassIncinerate::Create(Vec3f aePos, float afBeta = 0) {
	
	(void)afBeta;
	
	aePos.y += 150.0f;

	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Create(aePos, i * 36.f);
	}
}

//---------------------------------------------------------------------
void CMassIncinerate::Update(unsigned long _ulTime)
{
	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Update(_ulTime);
	}

}

//---------------------------------------------------------------------
float CMassIncinerate::Render()
{
	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Render();
	}

	return 0;
}

