/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/spells/Spells04.h"

#include "animation/AnimationRender.h"

#include "core/GameTime.h"

#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

CBless::CBless() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(4000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_sol = TextureContainer::Load("graph/particles/(fx)_pentagram_bless");
}

void CBless::Create(Vec3f _eSrc, float _fBeta) {
	
	SetDuration(ulDuration);
	SetAngle(_fBeta);
	
	eSrc = _eSrc;
	eTarget = eSrc;
	fSize = 1;
	fRot = 0;
	fRotPerMSec = 0.25f;
	bDone = true;
}

void CBless::Set_Angle(const Anglef & angle)
{
	fBeta = angle.getPitch();
}

void CBless::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	fRot += _ulTime * fRotPerMSec;
}

void CBless::Render()
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y - 5;
	float z = eSrc.z;

	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	TexturedVertex v[4];
	TexturedVertex v3[4];

	float ff = ((float)spells[spellinstance].caster_level + 10) * 6.f;
	float fBetaRadCos = (float) cos(radians(MAKEANGLE(player.angle.getPitch()))) * ff;
	float fBetaRadSin = (float) sin(radians(MAKEANGLE(player.angle.getPitch()))) * ff;

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
	
	v3[0].uv = Vec2f_ZERO;
	v3[1].uv = Vec2f_X_AXIS;
	v3[2].uv = Vec2f_Y_AXIS;
	v3[3].uv = Vec2f_ONE;
	
	EE_RT2(&v[0], &v3[0]);
	EE_RT2(&v[1], &v3[1]);
	EE_RT2(&v[2], &v3[2]);
	EE_RT2(&v[3], &v3[3]);
	ARX_DrawPrimitive(&v3[0], &v3[1], &v3[2]);
	ARX_DrawPrimitive(&v3[1], &v3[2], &v3[3]);
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	for(i = 0; i < 12; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = eSrc - Vec3f(0.f, 20.f, 0.f);
		pd->move = Vec3f(3.f * frand2(), rnd() * 0.5f, 3.f * frand2());
		pd->siz = 0.005f;
		pd->tolive = Random::get(1000, 2000);
		pd->tc = tex_p1;
		pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
		pd->fparam = 0.0000001f;
		pd->rgb = Color3f(0.7f, 0.6f, 0.2f);
	}
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
}

void CDispellField::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	eSrc = aeSrc;
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	eTarget = eSrc;
	fSize = 1;
	bDone = true;
}

void CDispellField::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CDispellField::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetTexture(0, tex_p2);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	Anglef stiteangle(0.f, float(ulCurrentTime) * fOneOnDuration * 180.f, 0.f);
	Vec3f stitepos = player.pos + Vec3f(0.f, 80.f, 0.f);
	Color3f stitecolor = Color3f::white;
	Vec3f stitescale = Vec3f(2.f);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, stitecolor);
	
	stitepos.y = player.pos.y + 20;
	stitecolor = Color3f::white;
	stitescale = Vec3f(1.8f);
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, stitecolor);
}

CTelekinesis::~CTelekinesis()
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

void CTelekinesis::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	eSrc = aeSrc;
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	eTarget = eSrc;
	fSize = 1;
	bDone = true;
}

void CTelekinesis::Update(unsigned long _ulTime) {
	ulCurrentTime += _ulTime;
}

void CTelekinesis::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetTexture(0, tex_p2);
	
	Anglef stiteangle(0.f, float(ulCurrentTime) * fOneOnDuration * 180.f, 0.f);
	Vec3f stitepos = player.pos + Vec3f(0.f, 80.f, 0.f);
	Color3f stitecolor = Color3f::white;
	Vec3f stitescale = Vec3f(2.f);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, stitecolor);
	
	stitepos.y = player.pos.y + 20;
	stitecolor = Color3f::white;
	stitescale = Vec3f(1.8f);
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, stitecolor);
}

CCurse::~CCurse() {
	svoodoo_count--;
	if(svoodoo && (svoodoo_count <= 0)) {
		svoodoo_count = 0;
		delete svoodoo;
		svoodoo = NULL;
	}
}

CCurse::CCurse() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(3000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	if(!svoodoo) {
		svoodoo = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_voodoodoll/fx_voodoodoll.teo");
	}
	
	svoodoo_count++;
}

void CCurse::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	
	eSrc = aeSrc;
	
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);
	
	eTarget = eSrc;
	bDone = true;
	fRot = 0;
	fRotPerMSec = 0.25f;
}

void CCurse::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	fRot += fRotPerMSec * _ulTime;
}

void CCurse::Render() {
	
	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
	if(svoodoo) {
		Anglef stiteangle = Anglef(0, fRot, 0);
		Vec3f stitepos = eTarget;
		Vec3f stitescale = Vec3f_ONE;
		Color3f stitecolor = Color3f::white;
		DrawEERIEObjEx(svoodoo , &stiteangle, &stitepos, &stitescale, stitecolor);
	}
	
	for(int i = 0; i < 4; i++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = eTarget;
		pd->move = Vec3f(2.f * frand2(), rnd() * -10.f - 10.f, 2.f * frand2());
		pd->siz = 0.015f;
		pd->tolive = Random::get(1000, 1600);
		pd->tc = tex_p1;
		pd->special = ROTATING | MODULATE_ROTATION | DISSIPATING | SUBSTRACT | GRAVITY;
		pd->fparam = 0.0000001f;
	}

	GRenderer->SetCulling(Renderer::CullNone);
}

//-----------------------------------------------------------------------------
//	FIRE PROTECTION
//-----------------------------------------------------------------------------
CFireProtection::CFireProtection()
{
}

CFireProtection::~CFireProtection()
{
	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = 0;
		io->halo.color.r = 0.8f;
		io->halo.color.g = 0.8f;
		io->halo.color.b = 0.9f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CFireProtection::Create(long _ulDuration)
{
	SetDuration(_ulDuration);

	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.3f;
		io->halo.color.b = 0.f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CFireProtection::Update(unsigned long _ulTime)
{
	if(!arxtime.is_paused())
		ulCurrentTime += _ulTime;

	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.5f;
		io->halo.color.g = 0.3f;
		io->halo.color.b = 0.f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CFireProtection::Render() {
	
}

//-----------------------------------------------------------------------------
//	COLD PROTECTION
//-----------------------------------------------------------------------------
CColdProtection::CColdProtection()
{
}

CColdProtection::~CColdProtection()
{
	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = 0;
		io->halo.color.r = 0.8f;
		io->halo.color.g = 0.8f;
		io->halo.color.b = 0.9f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CColdProtection::Create(long _ulDuration, int _iNpc)
{
	SetDuration(_ulDuration);

	iNpc = _iNpc;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CColdProtection::Update(unsigned long _ulTime)
{
	if(!arxtime.is_paused())
		ulCurrentTime += _ulTime;

	long iNpc = spells[spellinstance].target;

	if(ValidIONum(iNpc)) {
		Entity *io = entities[iNpc];
		io->halo.flags = HALO_ACTIVE;
		io->halo.color.r = 0.2f;
		io->halo.color.g = 0.2f;
		io->halo.color.b = 0.45f;
		io->halo.radius = 45.f;
		io->halo.dynlight = -1;
	}
}

void CColdProtection::Render() {

}
