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

#include "graphics/spells/Spells04.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/Spells.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

//-----------------------------------------------------------------------------
CBless::CBless()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(4000);
	ulCurrentTime = ulDuration + 1;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_sol = TextureContainer::Load("graph/particles/(fx)_pentagram_bless");
}

//-----------------------------------------------------------------------------
void CBless::Create(Vec3f _eSrc, float _fBeta)
{
	SetDuration(ulDuration);
	SetAngle(_fBeta);

	eSrc.x = _eSrc.x;
	eSrc.y = _eSrc.y;
	eSrc.z = _eSrc.z;

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	fSize = 1;

	fRot = 0;
	fRotPerMSec = 0.25f;
	bDone = true;
}

void CBless::Set_Angle(const Anglef & angle)
{
	fBeta = angle.b;
}
//-----------------------------------------------------------------------------
void CBless::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	fRot += _ulTime * fRotPerMSec;
}

//---------------------------------------------------------------------
float CBless::Render()
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y - 5;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	TexturedVertex v[4];
	TexturedVertex v3[4];

	float ff = ((float)spells[spellinstance].caster_level + 10) * 6.f;
	float fBetaRadCos = (float) cos(radians(MAKEANGLE(player.angle.b))) * ff;
	float fBetaRadSin = (float) sin(radians(MAKEANGLE(player.angle.b))) * ff;

	ColorBGRA color = Color::white.toBGR();

	v[0].p.x = x - fBetaRadCos - fBetaRadSin;
	v[0].p.y = y;
	v[0].p.z = z - fBetaRadSin + fBetaRadCos;
	v[1].p.x = x + fBetaRadCos - fBetaRadSin;
	v[1].p.y = y;
	v[1].p.z = z + fBetaRadSin + fBetaRadCos;
	v[2].p.x = x - fBetaRadCos + fBetaRadSin;
	v[2].p.y = y;
	v[2].p.z = z - fBetaRadSin - fBetaRadCos;
	v[3].p.x = x + fBetaRadCos + fBetaRadSin;
	v[3].p.y = y;
	v[3].p.z = z + fBetaRadSin - fBetaRadCos;

	v3[0].color = color;
	v3[1].color = color;
	v3[2].color = color;
	v3[3].color = color;

	GRenderer->SetTexture(0, tex_sol);

	v3[0].uv.x = 0;
	v3[0].uv.y = 0;
	v3[1].uv.x = 1.f;
	v3[1].uv.y = 0;
	v3[2].uv.x = 0;
	v3[2].uv.y = 1.f;
	v3[3].uv.x = 1.f;
	v3[3].uv.y = 1.f;

	EE_RT2(&v[0], &v3[0]);
	EE_RT2(&v[1], &v3[1]);
	EE_RT2(&v[2], &v3[2]);
	EE_RT2(&v[3], &v3[3]);
	ARX_DrawPrimitive(&v3[0],
	                             &v3[1],
	                             &v3[2]);
	ARX_DrawPrimitive(&v3[1],
	                             &v3[2],
	                             &v3[3]);
	
	//----------------------------
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	for (i = 0; i < 12; i++)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!arxtime.is_paused()))
		{
			ParticleCount++;
			particle[j].exist = 1;
			particle[j].zdec = 0;

			particle[j].ov.x		=	eSrc.x;
			particle[j].ov.y		=	eSrc.y - 20;
			particle[j].ov.z		=	eSrc.z;
			particle[j].move.x		=	3.f * frand2();
			particle[j].move.y		=	rnd() * 0.5f;
			particle[j].move.z		=	3.f * frand2();
			particle[j].siz			=	0.005f;
			particle[j].tolive		=	1000 + (unsigned long)(float)(rnd() * 1000.f);
			particle[j].scale.x		=	1.f;
			particle[j].scale.y		=	1.f;
			particle[j].scale.z		=	1.f;
			particle[j].timcreation	=	(long)arxtime;
			particle[j].tc			=	tex_p1;
			particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].fparam		=	0.0000001f;
			particle[j].rgb = Color3f(.7f, .6f, .2f);
		}
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	return 1;
}

//-----------------------------------------------------------------------------
void CDispellField::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CDispellField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CDispellField::Render()
{
	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	GRenderer->SetTexture(0, tex_p2);
	
	Anglef stiteangle;
	Vec3f stitepos;
	Vec3f stitescale;
	Color3f stitecolor;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = player.pos.x;
	stitepos.y = player.pos.y + 80;
	stitepos.z = player.pos.z;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	stiteangle.b = -stiteangle.b * 1.5f;
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 1;
	stitescale.y = -0.1f;
	stitescale.z = 1;

	stiteangle.b = -stiteangle.b;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

	stitepos.y = player.pos.y + 20;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

	return 1;
}

//-----------------------------------------------------------------------------
CTelekinesis::~CTelekinesis()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		delete ssol;
		ssol = NULL;
	}

	slight_count--;

	if (slight && (slight_count <= 0))
	{
		slight_count = 0;
		delete slight;
		slight = NULL;
	}

	srune_count--;

	if (srune && (srune_count <= 0))
	{
		srune_count = 0;
		delete srune;
		srune = NULL;
	}
}
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

//-----------------------------------------------------------------------------
void CTelekinesis::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CTelekinesis::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

float CTelekinesis::Render() {
	
	if(ulCurrentTime >= ulDuration) {
		return 0.f;
	}
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	GRenderer->SetTexture(0, tex_p2);
	
	Anglef stiteangle((float) ulCurrentTime * fOneOnDuration * 120, 0.f, 0.f);
	
	Vec3f stitepos = player.pos + Vec3f(0.f, 80.f, 0.f);
	
	Color3f stitecolor;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	Vec3f stitescale(2.f, 2.f, 2.f);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);
	
	stitepos.y = player.pos.y + 20;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale = Vec3f(1.8f, 1.8f, 1.8f);
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, &stitecolor);
	
	return 1;
}

//-----------------------------------------------------------------------------
CCurse::~CCurse()
{
	svoodoo_count--;

	if (svoodoo && (svoodoo_count <= 0))
	{
		svoodoo_count = 0;
		delete svoodoo;
		svoodoo = NULL;
	}
}
CCurse::CCurse()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(3000);
	ulCurrentTime = ulDuration + 1;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");

	if (!svoodoo)
		svoodoo = _LoadTheObj("graph/obj3d/interactive/fix_inter/fx_voodoodoll/fx_voodoodoll.teo");

	svoodoo_count++;
}

//-----------------------------------------------------------------------------
void CCurse::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	bDone = true;
	fRot = 0;
	fRotPerMSec = 0.25f;
}

//---------------------------------------------------------------------
void CCurse::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	fRot += fRotPerMSec * _ulTime;
}

float CCurse::Render() {
	
	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	Anglef stiteangle = Anglef(fRot, 0, 0);
	Vec3f stitepos = eTarget;
	Vec3f stitescale = Vec3f(1.f, 1.f, 1.f);
	Color3f stitecolor = Color3f::white;
	
	if(svoodoo) {
		DrawEERIEObjEx(svoodoo , &stiteangle, &stitepos, &stitescale, &stitecolor);
	}
	
	for(int i = 0; i < 4; i++) {
		
		int j = ARX_PARTICLES_GetFree();
		
		if((j != -1) && (!arxtime.is_paused())) {
			ParticleCount++;
			PARTICLE_DEF * pd = &particle[j];
			pd->exist = 1;
			pd->zdec = 0;
			pd->ov = eTarget;
			pd->move = Vec3f(2.f * frand2(), rnd() * -10.f - 10.f, 2.f * frand2());
			pd->siz = 0.015f;
			pd->tolive = 1000 + (unsigned long)(float)(rnd() * 600.f);
			pd->scale = Vec3f(1.f, 1.f, 1.f);
			pd->timcreation = (long)arxtime;
			pd->tc = tex_p1;
			pd->special = ROTATING | MODULATE_ROTATION | DISSIPATING | SUBSTRACT | GRAVITY;
			pd->fparam = 0.0000001f;
			pd->rgb = Color3f::white;
		}
	}
	
	return 1;
}



//-----------------------------------------------------------------------------
//	FIRE PROTECTION
//-----------------------------------------------------------------------------
CFireProtection::CFireProtection()
{
}

//-----------------------------------------------------------------------------
CFireProtection::~CFireProtection()
{
	INTERACTIVE_OBJ * io;
	long iNpc = spells[spellinstance].target;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = 0;
		io->halo.color.r = 0.8f;
		io->halo.color.g = 0.8f;
		io->halo.color.b = 0.9f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

//-----------------------------------------------------------------------------
void CFireProtection::Create(long _ulDuration)
{
	SetDuration(_ulDuration);

	long iNpc = spells[spellinstance].target;
	INTERACTIVE_OBJ * io;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.4f;
		io->halo.color.g = 0.4f;
		io->halo.color.b = 0.125f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

//-----------------------------------------------------------------------------
void CFireProtection::Update(unsigned long _ulTime)
{
	if (!arxtime.is_paused()) ulCurrentTime += _ulTime;

	long iNpc = spells[spellinstance].target;
	INTERACTIVE_OBJ * io;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.4f;
		io->halo.color.g = 0.4f;
		io->halo.color.b = 0.125f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

//-----------------------------------------------------------------------------
float CFireProtection::Render() {
	
	return 0;
}

//-----------------------------------------------------------------------------
//	COLD PROTECTION
//-----------------------------------------------------------------------------
CColdProtection::CColdProtection()
{
}

//-----------------------------------------------------------------------------
CColdProtection::~CColdProtection()
{
	INTERACTIVE_OBJ * io;
	long iNpc = spells[spellinstance].target;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = 0;
		io->halo.color.r = 0.8f;
		io->halo.color.g = 0.8f;
		io->halo.color.b = 0.9f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

//-----------------------------------------------------------------------------
void CColdProtection::Create(long _ulDuration, int _iNpc)
{
	SetDuration(_ulDuration);

	iNpc = _iNpc;
	INTERACTIVE_OBJ * io;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

//-----------------------------------------------------------------------------
void CColdProtection::Update(unsigned long _ulTime)
{
	if (!arxtime.is_paused()) ulCurrentTime += _ulTime;

	long iNpc = spells[spellinstance].target;
	INTERACTIVE_OBJ * io;

	if (ValidIONum(iNpc))
	{
		io = inter.iobj[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

float CColdProtection::Render() {
	
	return 0;
}
