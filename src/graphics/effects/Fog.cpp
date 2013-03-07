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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "graphics/effects/Fog.h"

#include "animation/Animation.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/particle/ParticleEffects.h"

#include "math/Random.h"

EERIE_3DOBJ * fogobj = NULL;

FOG_DEF fogs[MAX_FOG];

//*************************************************************************************
// Used to Set 3D Object Visual for Fogs
//*************************************************************************************
void ARX_FOGS_Set_Object(EERIE_3DOBJ * _fogobj)
{
	fogobj = _fogobj;
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_FirstInit()
{
	ARX_FOGS_Clear();
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_Clear()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		memset(&fogs[i], 0, sizeof(FOG_DEF));
	}
}

void ARX_FOGS_TranslateSelected(Vec3f * trans) {
	for(long i = 0; i < MAX_FOG; i++) {
		if(fogs[i].selected) {
			fogs[i].pos += *trans;
		}
	}
}

void ARX_FOGS_UnselectAll()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		fogs[i].selected = 0;
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_Select(long n)
{
	if (fogs[n].selected) fogs[n].selected = 0;
	else fogs[n].selected = 1;
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_KillByIndex(long num)
{
	if ((num >= 0) && (num < MAX_FOG))
	{
		memset(&fogs[num], 0, sizeof(FOG_DEF));
	}
}

void ARX_FOGS_KillSelected()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].selected)
		{
			ARX_FOGS_KillByIndex(i);
		}
	}
}
//*************************************************************************************
//*************************************************************************************
long ARX_FOGS_GetFree()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		if (!fogs[i].exist)  return i;
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
long ARX_FOGS_Count()
{
	long count = 0;

	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)  count++;
	}

	return count;
}
void ARX_FOGS_TimeReset()
{
}

void AddPoisonFog(Vec3f * pos, float power) {
	
	int iDiv = 4 - config.video.levelOfDetail;
	
	float flDiv = static_cast<float>(1 << iDiv);
	
	arxtime.update();
	
	long count = std::max(1l, checked_range_cast<long>(framedelay / flDiv));
	while(count--) {
		
		if(rnd() * 2000.f >= power) {
			continue;
		}
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			return;
		}
		
		float speed = 1.f;
		float fval = speed * 0.2f;
		pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
		pd->ov = *pos + randomVec(-100.f, 100.f);
		pd->scale = Vec3f(8.f, 8.f, 10.f);
		pd->move = Vec3f((speed - rnd()) * fval, (speed - speed * rnd()) * (1.f / 15),
		                 (speed - rnd()) * fval);
		pd->tolive = Random::get(4500, 9000);
		pd->tc = TC_smoke;
		pd->siz = (80.f + rnd() * 160.f) * (1.f / 3);
		pd->rgb = Color3f(rnd() * (1.f / 3), 1.f, rnd() * 0.1f);
		pd->fparam = 0.001f;
	}
}

void ARX_FOGS_Render() {
	
	if(arxtime.is_paused()) {
		return;
	}
	
	int iDiv = 4 - config.video.levelOfDetail;
	
	float flDiv = static_cast<float>(1 << iDiv);
	
	for(long i = 0; i < MAX_FOG; i++) {
		
		if(!fogs[i].exist) {
			continue;
		}
		
		long count = std::max(1l, checked_range_cast<long>(framedelay / flDiv));
		while(count--) {
			
			if(rnd() * 2000.f >= fogs[i].frequency) {
				continue;
			}
			
			PARTICLE_DEF * pd = createParticle(true);
			if(!pd) {
				break;
			}
			
			pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			if(fogs[i].special & FOG_DIRECTIONAL) {
				pd->ov = fogs[i].pos;
				pd->move = fogs[i].move * (fogs[i].speed * 0.1f);
			} else {
				pd->ov = fogs[i].pos + randomVec(-100.f, 100.f);
				pd->move = Vec3f::repeat(fogs[i].speed) - randomVec(0.f, 2.f);
				pd->move *= Vec3f(fogs[i].speed * 0.2f,  1.f / 15, fogs[i].speed * 0.2f);
			}
			pd->scale = Vec3f::repeat(fogs[i].scale);
			pd->tolive = fogs[i].tolive + Random::get(0, fogs[i].tolive);
			pd->tc = TC_smoke;
			pd->siz = (fogs[i].size + rnd() * fogs[i].size * 2.f) * (1.0f / 3);
			pd->rgb = fogs[i].rgb;
			pd->fparam = fogs[i].rotatespeed;
		}
		
		fogs[i].lastupdate = (unsigned long)(arxtime);
	}
}

void ARX_FOGS_RenderAll() {
	
	Anglef angle = Anglef::ZERO;

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)
		{
			if (fogobj)
				DrawEERIEInter(fogobj, &angle, &fogs[i].pos, NULL);

			fogs[i].bboxmin = BBOXMIN;
			fogs[i].bboxmax = BBOXMAX;

			if(fogs[i].special & FOG_DIRECTIONAL) {
				EERIEDraw3DLine(fogs[i].pos, fogs[i].pos + fogs[i].move * 50.f, Color::white); 
			}

			if(fogs[i].selected) {
				EERIEDraw2DLine(fogs[i].bboxmin.x, fogs[i].bboxmin.y, fogs[i].bboxmax.x, fogs[i].bboxmin.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(fogs[i].bboxmax.x, fogs[i].bboxmin.y, fogs[i].bboxmax.x, fogs[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(fogs[i].bboxmax.x, fogs[i].bboxmax.y, fogs[i].bboxmin.x, fogs[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(fogs[i].bboxmin.x, fogs[i].bboxmax.y, fogs[i].bboxmin.x, fogs[i].bboxmin.y, 0.01f, Color::yellow);
			}
		}
	}
}
