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

#include "graphics/spells/Spells08.h"

#include "core/GameTime.h"

#include "game/Damage.h"

#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/Light.h"

//-----------------------------------------------------------------------------
CExplosion::~CExplosion()
{
	this->Kill();
}

void CExplosion::Kill(void) {
	
	if(disqued3d) {
		free(disqued3d), disqued3d = NULL;
	}
	
	if(disquevertex) {
		free(disquevertex), disquevertex = NULL;
	}

	if(disqueind) {
		free(disqueind), disqueind = NULL;
	}
	
	if (tactif)
	{
		int nb = (disquenbvertex >> 1);

		while (nb--)
		{
			if (tactif[nb] >= 0)
			{
				DynLight[tactif[nb]].exist = 0;
			}
		}

		free(tactif), tactif = NULL;
	}
}

//-----------------------------------------------------------------------------
void CExplosion::Update(unsigned long _ulTime)
{
	float	a;


	switch (key)
	{
		case 0:
			//elargissement du disque
			a = (float)ulCurrentTime / 1000.f;

			if (a > 1.f)
			{
				a = 0.f;
				key++;
			}

			if (!ARXPausedTimer) ulCurrentTime += _ulTime;

			scale = a;
			break;
		case 1:
			//avancé du disque
			scale = (float)ulCurrentTime / (float)ulDuration;

			if (!ARXPausedTimer) ulCurrentTime += _ulTime;

			if (ulCurrentTime >= ulDuration)
			{
				scale = 1.f;
				key++;
			}

			break;
		case 2:
			Kill();
			key++;
			break;
	}
}

//-----------------------------------------------------------------------------
void CExplosion::ExplosionAddParticule(int num, TexturedVertex * v, TextureContainer * tp)
{
	if (DoSphericDamage(&v->p, 4.f, 30.f, DAMAGE_AREA, DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE, 0)) // 0=player source
	{
		Vec3f hit;
		hit.x = v->p.x;
		hit.y = v->p.y;
		hit.z = v->p.z;
		DynLight[tactif[num]].exist = 0;
		tactif[num] = -1;
		ARX_BOOMS_Add(&hit);
		Add3DBoom(&hit); 
	}

	if (rnd() > .25f)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist = 1;
			particle[j].zdec = 0;

			particle[j].ov.x		=	v->p.x;
			particle[j].ov.y		=	v->p.y;
			particle[j].ov.z		=	v->p.z;
			particle[j].move.x		=	0.f;
			particle[j].move.y		=	rnd();
			particle[j].move.z		=	0.f;
			particle[j].siz			=	10.f + 20.f * rnd();
			particle[j].tolive		=	500 + (unsigned long)(float)(rnd() * 500.f);
			particle[j].scale.x		=	1.f;
			particle[j].scale.y		=	1.f;
			particle[j].scale.z		=	1.f;
			particle[j].timcreation	=	lARXTime;
			particle[j].tc			=	tp;
			particle[j].special		=	FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION;
			particle[j].fparam		=	0.0000001f;
			particle[j].rgb = Color3f::white;
		}
	}
}

//-----------------------------------------------------------------------------
void CExplosion::Collision(int num, Vec3f * v, Vec3f * dir)
{
	Vec3f	hit;
	EERIEPOLY	* tp = NULL;

	if (EERIELaunchRay3(v, dir, &hit, tp, 1) != 0)	//@seb: ray cast more accurate
	{
		DynLight[tactif[num]].exist = 0;
		tactif[num] = -1;
		ARX_BOOMS_Add(&hit);
		Add3DBoom(&hit); 
	}
}

//-----------------------------------------------------------------------------
float CExplosion::Render()
{
	if (this->key > 1) return 0;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	//calcul du disque
	TexturedVertex d3dvs, *d3dv;
	Vec3f	* vertex;
	int			nb, col, col2;
	float		rin;

	switch (key)
	{
		case 0:
			rin = 255.f * scale;
			vertex = disquevertex;
			d3dv = disqued3d;
			nb = disquenbvertex >> 1;

			while (nb)
			{
				d3dvs.p.x = pos.x + (vertex + 1)->x + ((vertex->x - (vertex + 1)->x) * scale);
				d3dvs.p.y = pos.y;
				d3dvs.p.z = pos.z + (vertex + 1)->z + ((vertex->z - (vertex + 1)->z) * scale);
				EE_RTP(&d3dvs, d3dv);
				d3dv->color = Color(255, 200, 0).toBGRA();
				vertex++;
				d3dv++;

				d3dvs.p.x = pos.x + vertex->x;
				d3dvs.p.y = pos.y;
				d3dvs.p.z = pos.z + vertex->z;
				EE_RTP(&d3dvs, d3dv);

				if (!ARXPausedTimer) d3dv->color = Color((int)(rin * rnd()), 0, 0).toBGRA();

				vertex++;
				d3dv++;
				nb--;
			}

			if (rnd() > .25f)
			{
				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					float a = radians(360.f * scale);
					float b = rin; 

					particle[j].ov.x		=	pos.x + b * EEcos(a);
					particle[j].ov.y		=	pos.y;
					particle[j].ov.z		=	pos.z + b * EEsin(a);
					particle[j].move.x		=	0.f;
					particle[j].move.y		=	rnd();
					particle[j].move.z		=	0.f;
					particle[j].siz			=	10.f + 10.f * rnd();
					particle[j].tolive		=	500 + (unsigned long)(float)(rnd() * 500.f);
					particle[j].scale.x		=	1.f;
					particle[j].scale.y		=	1.f;
					particle[j].scale.z		=	1.f;
					particle[j].timcreation	=	lARXTime;
					particle[j].tc			=	tp;
					particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam		=	0.0000001f;
					particle[j].rgb = Color3f::white;
				}

				j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					float a = radians(-360.f * scale);
					float b = this->rin;

					particle[j].ov.x	=	pos.x + b * EEcos(a);
					particle[j].ov.y	=	pos.y;
					particle[j].ov.z	=	pos.z + b * EEsin(a);
					particle[j].move.x	=	0.f;
					particle[j].move.y	=	rnd();
					particle[j].move.z	=	0.f;
					particle[j].siz		=	10.f + 10.f * rnd();
					particle[j].tolive	=	500 + (unsigned long)(float)(rnd() * 500.f);
					particle[j].scale.x	=	1.f;
					particle[j].scale.y	=	1.f;
					particle[j].scale.z	=	1.f;
					particle[j].timcreation	=	lARXTime;
					particle[j].tc		=	tp;
					particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam	=	0.0000001f;
					particle[j].rgb = Color3f::white;
				}
			}

			if (rnd() > .1f)
			{
				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					float a = rnd() * 360.f; 
					float b = rin * rnd();

					particle[j].ov.x	=	pos.x + b * EEcos(a);
					particle[j].ov.y	=	pos.y + 70.f;
					particle[j].ov.z	=	pos.z + b * EEsin(a);
					particle[j].move.x	=	0.f;
					particle[j].move.y	=	-(5.f + 10.f * rnd());
					particle[j].move.z	=	0.f;
					particle[j].siz		=	10.f + 20.f * rnd();
					particle[j].tolive	=	1000 + (unsigned long)(float)(rnd() * 1000.f);
					particle[j].scale.x	=	1.f;
					particle[j].scale.y	=	1.f;
					particle[j].scale.z	=	1.f;
					particle[j].timcreation	=	lARXTime;
					particle[j].tc		=	tp2;
					particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam	=	0.0000001f;
					particle[j].rgb = Color3f::white;
				}
			}

			break;
		case 1:
			TexturedVertex d3dvs2;
			rin = 1.f + (puissance * scale);
			vertex = disquevertex;
			d3dv = disqued3d;
			nb = disquenbvertex >> 1;
			float a = 1.f - scale;
			col = Color((int)(255.f * a), (int)(200.f * a), 0).toBGRA();
			col2 = Color4f(a * rnd(), 0, 0, 0).toBGRA();

			while (nb--)
			{
				d3dvs.p.x = pos.x + vertex->x * rin;
				d3dvs.p.y = pos.y;
				d3dvs.p.z = pos.z + vertex->z * rin;
				vertex++;
				d3dvs2.p.x = pos.x + vertex->x * rin;
				d3dvs2.p.y = pos.y;
				d3dvs2.p.z = pos.z + vertex->z * rin;
				vertex++;

				if (tactif[nb] >= 0)
				{
					Vec3f pos, dir;
					pos.x = d3dvs2.p.x;
					pos.y = d3dvs2.p.y;
					pos.z = d3dvs2.p.z;
					dir.x = d3dvs.p.x;
					dir.y = d3dvs.p.y;
					dir.z = d3dvs.p.z;

					DynLight[tactif[nb]].pos.x = dir.x;
					DynLight[tactif[nb]].pos.y = dir.y;
					DynLight[tactif[nb]].pos.z = dir.z;
					DynLight[tactif[nb]].intensity = .7f + 2.f * rnd();

					Collision(nb, &pos, &dir);
					ExplosionAddParticule(nb, &d3dvs, tp);
				}

				EE_RTP(&d3dvs, d3dv);

				if (!ARXPausedTimer) d3dv->color = col;

				d3dv++;

				EE_RTP(&d3dvs2, d3dv);

				if (!ARXPausedTimer) d3dv->color = col2;

				d3dv++;
			}

			break;
	}

	//tracé du disque
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->ResetTexture(0);
	GRenderer->drawIndexed(Renderer::TriangleStrip, disqued3d, disquenbvertex, disqueind, disquenbvertex + 2);

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	return 0;
}
