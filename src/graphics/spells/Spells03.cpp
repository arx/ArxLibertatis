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

#include "graphics/spells/Spells03.h"

#include <climits>

#include "core/GameTime.h"

#include "game/Spells.h"
#include "game/Damage.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/spells/Spells05.h"

#include "scene/Object.h"
#include "scene/Interactive.h"

//-----------------------------------------------------------------------------
CFireBall::CFireBall() : CSpellFx()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	bExplo = false;
}

//-----------------------------------------------------------------------------
CFireBall::~CFireBall()
{

}

//-----------------------------------------------------------------------------
void CFireBall::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;

	std::list<Particle *>::iterator i;

	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff = static_cast<long>(ulCalc);


	for (i = pPSSmoke.listParticle.begin(); i != pPSSmoke.listParticle.end(); ++i)
	{
		Particle * pP = *i;

		if (pP->isAlive())
		{
			if (pP->ulTime + ff < pP->ulTTL)
			{
				pP->ulTime = pP->ulTTL - ff;
			}
		}
	}

	// Light
	if (lLightId != -1)
	{
		lLightId = -1;
	}
}

//-----------------------------------------------------------------------------
void CFireBall::Create(Vec3f aeSrc, float afBeta, float afAlpha, float _fLevel)
{
	SetDuration(ulDuration);
	SetAngle(afBeta);

	eSrc.x = aeSrc.x - fBetaRadSin * 60;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z + fBetaRadCos * 60;

	eMove.x = - fBetaRadSin * 80 * cos(radians(MAKEANGLE(afAlpha)));
	eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 80;
	eMove.z = + fBetaRadCos * 80 * cos(radians(MAKEANGLE(afAlpha)));

	fLevel = _fLevel;

	ParticleParams cp;

	//FIRE
	cp.iNbMax = 200;
	cp.fLife = 550;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 0;
	cp.p3Pos.z = 0;
	cp.p3Direction.x = -eMove.x;
	cp.p3Direction.y = -eMove.y;
	cp.p3Direction.z = -eMove.z;
	cp.fAngle = radians(3);
	cp.fSpeed = 0;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 50;

	cp.fStartSize = 1 * _fLevel; 
	cp.fStartSizeRandom = 2 * _fLevel; 
	cp.fStartColor[0] = 22;
	cp.fStartColor[1] = 30;
	cp.fStartColor[2] = 30;
	cp.fStartColor[3] = 0;
	cp.fStartColorRandom[0] = 22;
	cp.fStartColorRandom[1] = 0;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 2;

	cp.fEndSize = 0;
	cp.fEndSizeRandom = 2;
	cp.fEndColor[0] = 25;
	cp.fEndColor[1] = 25;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 50;
	cp.fEndColorRandom[0] = 50; 
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 120; 

	pPSFire.SetParams(cp);
	pPSFire.fParticleFreq = 100.0f;
	pPSFire.ulParticleSpawn = 0;
	pPSFire.SetTexture("graph/particles/fire", 0, 200);

	pPSFire.Update(0);

	//FIRE
	cp.iNbMax = 20;
	cp.fLife = 550;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 0;
	cp.p3Pos.z = 0;
	cp.p3Direction.x = -eMove.x;
	cp.p3Direction.y = -eMove.y;
	cp.p3Direction.z = -eMove.z;
	cp.fAngle = radians(3);
	cp.fSpeed = 0;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 50;

	cp.fStartSize = 1 * _fLevel; 
	cp.fStartSizeRandom = 2 * _fLevel; 
	cp.fStartColor[0] = 22;
	cp.fStartColor[1] = 30;
	cp.fStartColor[2] = 30;
	cp.fStartColor[3] = 0;
	cp.fStartColorRandom[0] = 22;
	cp.fStartColorRandom[1] = 0;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 2;

	cp.fEndSize = 3 * _fLevel; 
	cp.fEndSizeRandom = 2 * _fLevel; 
	cp.fEndColor[0] = 25;
	cp.fEndColor[1] = 25;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 50; 
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 120; 

	pPSFire2.SetParams(cp);
	pPSFire2.fParticleFreq = 20.0f;
	pPSFire2.ulParticleSpawn = 0;
	pPSFire2.SetTexture("graph/particles/fire", 0, 200);
	pPSFire2.Update(0);

	// Smoke
	cp.iNbMax = 30;
	cp.fLife = 2000;
	cp.fLifeRandom = 3000;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 10;
	cp.p3Direction.x = -eMove.x;
	cp.p3Direction.y = -eMove.y;
	cp.p3Direction.z = -eMove.z;
	cp.fAngle = radians(9);
	cp.fSpeed = 150; 
	cp.fSpeedRandom = 150; 
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = -10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 90;

	cp.fStartSize = 0;
	cp.fStartSizeRandom = 2;
	cp.fStartColor[0] = 70;
	cp.fStartColor[1] = 70;
	cp.fStartColor[2] = 51;
	cp.fStartColor[3] = 50;
	cp.fStartColorRandom[0] = 0;
	cp.fStartColorRandom[1] = 0;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 0;

	cp.fEndSize = 7 * _fLevel;
	cp.fEndSizeRandom = 2 * _fLevel; 
	cp.fEndColor[0] = 0;
	cp.fEndColor[1] = 0;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 27; 
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 20;

	pPSSmoke.SetParams(cp);
	pPSSmoke.ulParticleSpawn = 0;
	pPSSmoke.fParticleFreq = 20.0f;

	pPSSmoke.SetTexture("graph/particles/big_greypouf", 0, 0);
	pPSSmoke.Update(0);

	pPSFire.SetPos(eSrc);
	pPSFire2.SetPos(eSrc);
	pPSSmoke.SetPos(eSrc);

	// Light
	lLightId = -1; 
	eCurPos.x = eSrc.x;
	eCurPos.y = eSrc.y;
	eCurPos.z = eSrc.z;
}
#define MIN_TIME_FIREBALL 2000  //750
//-----------------------------------------------------------------------------
void CFireBall::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if (ulCurrentTime > MIN_TIME_FIREBALL)
	{
		// smoke en retard
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.Update(aulTime);

		float titi = aulTime * 0.0045f;

		eCurPos.x = eCurPos.x + eMove.x * titi;
		eCurPos.y = eCurPos.y + eMove.y * titi;
		eCurPos.z = eCurPos.z + eMove.z * titi;

		pPSFire.SetPos(eCurPos);
		pPSFire.fParticleSpeed = 100;
		pPSFire.fParticleSpeedRandom = 200;

		pPSFire.p3ParticleGravity.x = -eMove.x * 2;
		pPSFire.p3ParticleGravity.y = -eMove.y * 2;
		pPSFire.p3ParticleGravity.z = -eMove.z * 2;

		pPSFire2.p3ParticleGravity.x = -eMove.x * 2;
		pPSFire2.p3ParticleGravity.y = -eMove.y * 2;
		pPSFire2.p3ParticleGravity.z = -eMove.z * 2;

		pPSFire2.SetPos(eCurPos);
		pPSFire2.fParticleSpeed = 100;
		pPSFire2.fParticleSpeedRandom = 100;
	}
	else
	{
		float afAlpha = 0.f;
	
		if (spells[spellinstance].caster == 0)
		{
			SetAngle(player.angle.b);
			afAlpha = player.angle.a;
			long idx = GetGroupOriginByName(inter.iobj[spells[spellinstance].caster]->obj, "chest");

			if (idx)
			{
				eCurPos.x = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.x - fBetaRadSin * 60;
				eCurPos.y = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.y;
				eCurPos.z = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.z + fBetaRadCos * 60;
			}
			else
			{
				eCurPos.x = player.pos.x - fBetaRadSin * 60;
				eCurPos.y = player.pos.y;
				eCurPos.z = player.pos.z + fBetaRadCos * 60;
			}
		}
		else
		{
			SetAngle(inter.iobj[spells[spellinstance].caster]->angle.b);

			eCurPos.x = inter.iobj[spells[spellinstance].caster]->pos.x - fBetaRadSin * 60;
			eCurPos.y = inter.iobj[spells[spellinstance].caster]->pos.y;
			eCurPos.z = inter.iobj[spells[spellinstance].caster]->pos.z + fBetaRadCos * 60;

			if ((ValidIONum(spells[spellinstance].caster))
			        && (inter.iobj[spells[spellinstance].caster]->ioflags & IO_NPC))
			{
				eCurPos.x -= EEsin(radians(inter.iobj[spells[spellinstance].caster]->angle.b)) * 30.f;
				eCurPos.y -= 80.f;
				eCurPos.z += EEcos(radians(inter.iobj[spells[spellinstance].caster]->angle.b)) * 30.f;
			}
			
			INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].caster];

			if (ValidIONum(io->targetinfo))
			{
				Vec3f * p1 = &eCurPos;
				Vec3f p2 = inter.iobj[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (degrees(getAngle(p1->y, p1->z, p2.y, p2.z + dist(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}


		eMove.x = - fBetaRadSin * 100 * cos(radians(MAKEANGLE(afAlpha)));
		eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 100;
		eMove.z = + fBetaRadCos * 100 * cos(radians(MAKEANGLE(afAlpha)));

		Vec3f vMove = eMove.getNormalized();

		// smoke en retard
		pPSSmoke.p3ParticleDirection.x = -vMove.x;
		pPSSmoke.p3ParticleDirection.y = -vMove.y;
		pPSSmoke.p3ParticleDirection.z = -vMove.z;
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.RecomputeDirection();

		float titi = aulTime * 0.0045f;

		eCurPos.x = eCurPos.x + eMove.x * titi;
		eCurPos.y = eCurPos.y + eMove.y * titi;
		eCurPos.z = eCurPos.z + eMove.z * titi;

		pPSFire.p3ParticleDirection.x = -vMove.x;
		pPSFire.p3ParticleDirection.y = -vMove.y;
		pPSFire.p3ParticleDirection.z = -vMove.z;
		pPSFire.RecomputeDirection();
		pPSFire.SetPos(eCurPos);

		pPSFire.p3ParticleGravity.x = -eMove.x * 2;
		pPSFire.p3ParticleGravity.y = -eMove.y * 2;
		pPSFire.p3ParticleGravity.z = -eMove.z * 2;

		pPSFire2.p3ParticleDirection.x = -vMove.x;
		pPSFire2.p3ParticleDirection.y = -vMove.y;
		pPSFire2.p3ParticleDirection.z = -vMove.z;
		pPSFire2.p3ParticleGravity.x = -eMove.x * 2;
		pPSFire2.p3ParticleGravity.y = -eMove.y * 2;
		pPSFire2.p3ParticleGravity.z = -eMove.z * 2;
		pPSFire2.RecomputeDirection();
		pPSFire2.SetPos(eCurPos);
	}

	pPSFire.Update(aulTime);
	pPSFire2.Update(aulTime);
}

//-----------------------------------------------------------------------------
float CFireBall::Render()
{
	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	pPSFire.Render();
	pPSFire2.Render();
	pPSSmoke.Render();

	return 1 - 0.5f * rnd();
}

//-----------------------------------------------------------------------------
CIceProjectile::~CIceProjectile()
{
	stite_count--;

	if (stite && (stite_count <= 0))
	{
		stite_count = 0;
		delete stite;
		stite = NULL;
	}

	smotte_count--;

	if (smotte && (smotte_count <= 0))
	{
		smotte_count = 0;
		delete smotte;
		smotte = NULL;
	}
}
CIceProjectile::CIceProjectile()
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	iNumber = MAX_ICE;

	tex_p1 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_bluepouf");

	if (!stite)
		stite = _LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/stalagmite.teo");

	stite_count++;

	if (!smotte)
		smotte = _LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");

	smotte_count++;

	iMax = MAX_ICE;
	fStep = 20;
}

//-----------------------------------------------------------------------------
void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel)
{
	iMax = (int)(30 + fLevel * 5.2f);

	Create(aeSrc, afBeta);
}

//-----------------------------------------------------------------------------
void CIceProjectile::Create(Vec3f aeSrc, float afBeta)
{
	SetDuration(ulDuration);
	SetAngle(afBeta);

	fSize = 1;

	float xmin, ymin, zmin;

	Vec3f s, e, h;

	s.x					= aeSrc.x;
	s.y					= aeSrc.y - 100;
	s.z					= aeSrc.z;
	float fspelldist	= static_cast<float>(iMax * 15);

	fspelldist = min(fspelldist, 200.0f);
	fspelldist = max(fspelldist, 450.0f);
	e.x = aeSrc.x - fBetaRadSin * fspelldist;
	e.y = aeSrc.y - 100;
	e.z = aeSrc.z + fBetaRadCos * fspelldist;

	float fd;

	if (!Visible(&s, &e, NULL, &h))
	{
		e.x = h.x + fBetaRadSin * 20;
		e.y = h.y;
		e.z = h.z - fBetaRadCos * 20;
	}

	fd = fdist(s, e);

	float fCalc = ulDuration * (fd / fspelldist);
	SetDuration(checked_range_cast<unsigned long>(fCalc));

	float fDist = (fd / fspelldist) * iMax ;

	iNumber = checked_range_cast<int>(fDist);

	int end = iNumber / 2;
	tv1a[0].p.x = s.x;
	tv1a[0].p.y = s.y + 100;
	tv1a[0].p.z = s.z;
	tv1a[end].p.x = e.x;
	tv1a[end].p.y = e.y + 100;
	tv1a[end].p.z = e.z;

	Split(tv1a, 0, end, 80, 0.5f, 0, 1, 80, 0.5f);

	for (int i = 0; i < iNumber; i++)
	{
		float t = rnd();

		if (t < 0.5f)
			tType[i] = 0;
		else
			tType[i] = 1;

		tSize[i].x = 0;
		tSize[i].y = 0;
		tSize[i].z = 0;
		tSizeMax[i].x = rnd();
		tSizeMax[i].y = rnd() + 0.2f;
		tSizeMax[i].z = rnd();

		if (tType[i] == 0)
		{
			xmin = 1.2f;
			ymin = 1;
			zmin = 1.2f;
		}
		else
		{
			xmin = 0.4f;
			ymin = 0.3f;
			zmin = 0.4f;
		}

		if (tSizeMax[i].x < xmin)
			tSizeMax[i].x = xmin;

		if (tSizeMax[i].y < ymin)
			tSizeMax[i].y = ymin;

		if (tSizeMax[i].z < zmin)
			tSizeMax[i].z = zmin;

		int iNum = static_cast<int>(i / 2);

		if (tType[i] == 0)
		{
			tPos[i].x = tv1a[iNum].p.x + frand2() * 80;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 80;
		}
		else
		{
			tPos[i].x = tv1a[iNum].p.x + frand2() * 40;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 40;
		}

		long ttt = ARX_DAMAGES_GetFree();

		if (ttt != -1)
		{
			damages[ttt].pos.x = tPos[i].x;
			damages[ttt].pos.y = tPos[i].y;
			damages[ttt].pos.z = tPos[i].z;
			damages[ttt].radius = 60.f;
			damages[ttt].damages = 0.1f * spells[spellinstance].caster_level;
			damages[ttt].area = DAMAGE_FULL;
			damages[ttt].duration = ulDuration;
			damages[ttt].source = spells[spellinstance].caster;
			damages[ttt].flags = DAMAGE_FLAG_DONT_HURT_SOURCE;
			damages[ttt].type = DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD;
			damages[ttt].exist = true;
		}
	}

	fColor = 1;
}

//---------------------------------------------------------------------
void CIceProjectile::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if (ulDuration - ulCurrentTime < 1000)
	{
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for (int i = 0; i < iNumber; i++)
		{
			tSize[i].y *= fColor;
		}
	}
}

//---------------------------------------------------------------------
float CIceProjectile::Render()
{
	int i = 0;

	if (ulCurrentTime >= ulDuration) return 0.f;

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if (iMax > iNumber) iMax = iNumber;

	for (i = 0; i < iMax; i++)
	{
		if (tSize[i].x < tSizeMax[i].x)
			tSize[i].x += 0.1f;

		if (tSize[i].x > tSizeMax[i].x)
			tSize[i].x = tSizeMax[i].x;

		if (tSize[i].y < tSizeMax[i].y)
			tSize[i].y += 0.1f;

		if (tSize[i].y < 0)
			tSize[i].y = 0;

		if (tSize[i].y > tSizeMax[i].y)
			tSize[i].y = tSizeMax[i].y;

		if (tSize[i].z < tSizeMax[i].z)
			tSize[i].z += 0.1f;

		if (tSize[i].z > tSizeMax[i].z)
			tSize[i].z = tSizeMax[i].z;

		Anglef stiteangle;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.b = (float) cos(radians(tPos[i].x)) * 360;
		stiteangle.a = 0;
		stiteangle.g = 0;
		stitepos.x = tPos[i].x;
		stitepos.y = tPos[i].y;
		stitepos.z = tPos[i].z;

		float tt;
		tt = tSizeMax[i].y * fColor;
		stitecolor.g = stitecolor.r = tt * 0.7f;
		stitecolor.b = tt * 0.9f;

		if (stitecolor.r > 1) stitecolor.r = 1;

		if (stitecolor.g > 1) stitecolor.g = 1;

		if (stitecolor.b > 1) stitecolor.b = 1;

		stitescale.x = tSize[i].x;
		stitescale.y = tSize[i].y;
		stitescale.z = tSize[i].z;

		if (tType[i] == 0)
			DrawEERIEObjEx(smotte, &stiteangle, &stitepos, &stitescale, &stitecolor);
		else
			DrawEERIEObjEx(stite, &stiteangle, &stitepos, &stitescale, &stitecolor);
	}

	float x, y, z;

	//----------------
	for (i = 0; i < min(iNumber, iMax + 1); i++)
	{
		float t = rnd();

		if (t < 0.01f)
		{
			x = tPos[i].x;
			y = tPos[i].y;
			z = tPos[i].z;

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x = x + 5.f - rnd() * 10.f;
				particle[j].ov.y = y + 5.f - rnd() * 10.f;
				particle[j].ov.z = z + 5.f - rnd() * 10.f;
				particle[j].move.x = 2.f - 4.f * rnd();
				particle[j].move.y = 2.f - 4.f * rnd();
				particle[j].move.z = 2.f - 4.f * rnd();
				particle[j].siz = 20.f;

				float fMin = min(2000 + (rnd() * 2000.f), ulDuration - ulCurrentTime + 500.0f * rnd());
				particle[j].tolive = checked_range_cast<unsigned long>(fMin);

				particle[j].scale.x = 1.f;
				particle[j].scale.y = 1.f;
				particle[j].scale.z = 1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc = tex_p2;
				particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam = 0.0000001f;
				particle[j].rgb = Color3f(.7f, .7f, 1.f);
			}
		}
		else if (t > 0.095f)
		{
			x = tPos[i].x;
			y = tPos[i].y - 50;
			z = tPos[i].z;

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov.x = x + 5.f - rnd() * 10.f;
				particle[j].ov.y = y + 5.f - rnd() * 10.f;
				particle[j].ov.z = z + 5.f - rnd() * 10.f;
				particle[j].move.x = 0;
				particle[j].move.y = 2.f - 4.f * rnd();
				particle[j].move.z = 0;
				particle[j].siz = 0.5f;

				float fMin = min(2000 + (rnd() * 1000.f), ulDuration - ulCurrentTime + 500.0f * rnd());
				particle[j].tolive = checked_range_cast<unsigned long>(fMin);

				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc 			=	tex_p1;
				particle[j].special 	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam		=	0.0000001f;
				particle[j].rgb = Color3f(.7f, .7f, 1.f);
			}
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
// SPEED
//-----------------------------------------------------------------------------
void CSpeed::Create(int numinteractive, int duration)
{
	this->key = 1;
	this->duration = duration;
	this->currduration = 0;
	this->num = numinteractive;

	this->nbrubandef = 0;

	int nb = 2048;

	while (nb--)
	{
		this->truban[nb].actif = 0;
	}

	nb = (inter.iobj[this->num]->obj)->nbgroups;

	if (nb > 256) nb = 256;

	EERIE_GROUPLIST * grouplist = inter.iobj[this->num]->obj->grouplist;
	nb >>= 1;

	while (nb--)
	{
		float col = 0.05f + (rnd() * 0.05f);
		float size = 4.f + (4.f * rnd());
		int taille = 8 + (int)(8.f * rnd());
		this->AddRubanDef(grouplist->origin, size, taille, col, col, col, 0.f, 0.f, 0.f);
		grouplist += 2;
	}

	this->tp = TextureContainer::Load("graph/particles/fire");
}

//-----------------------------------------------------------------------------
void CSpeed::AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2)
{
	if (this->nbrubandef > 255) return;

	this->trubandef[this->nbrubandef].first = -1;
	this->trubandef[this->nbrubandef].origin = origin;
	this->trubandef[this->nbrubandef].size = size;
	this->trubandef[this->nbrubandef].dec = dec;
	this->trubandef[this->nbrubandef].r = r;
	this->trubandef[this->nbrubandef].g = g;
	this->trubandef[this->nbrubandef].b = b;
	this->trubandef[this->nbrubandef].r2 = r2;
	this->trubandef[this->nbrubandef].g2 = g2;
	this->trubandef[this->nbrubandef].b2 = b2;
	this->nbrubandef++;
}

//-----------------------------------------------------------------------------
int CSpeed::GetFreeRuban()
{
	int nb = 2048;

	while (nb--)
	{
		if (!this->truban[nb].actif) return nb;
	}

	return -1;
}

//-----------------------------------------------------------------------------
void CSpeed::AddRuban(int * f, int id, int dec)
{
	int	num;

	num = this->GetFreeRuban();

	if (num >= 0)
	{
		this->truban[num].actif = 1;

		this->truban[num].pos.x = inter.iobj[this->num]->obj->vertexlist3[id].v.x;
		this->truban[num].pos.y = inter.iobj[this->num]->obj->vertexlist3[id].v.y;
		this->truban[num].pos.z = inter.iobj[this->num]->obj->vertexlist3[id].v.z;

		if (*f < 0)
		{
			*f = num;
			this->truban[num].next = -1;
		}
		else
		{
			this->truban[num].next = *f;
			*f = num;
		}

		int nb = 0, oldnum = num;

		while (num != -1)
		{
			nb++;
			oldnum = num;
			num = this->truban[num].next;
		}

		if (oldnum < 0) ARX_DEAD_CODE();

		if (nb > dec)
		{

			this->truban[oldnum].actif = 0;
			num = *f;
			nb -= 2;

			while (nb--)
			{
				num = this->truban[num].next;
			}

			this->truban[num].next = -1;
		}
	}
}

//-----------------------------------------------------------------------------
void CSpeed::Update(unsigned long _ulTime)
{
	int	nb, num;

	switch (this->key)
	{
		case 0:
			break;
		case 1:

			if (ARXPausedTimer) break;

			if (this->currduration > this->duration)
			{
				this->key++;
			}

			num = 0;
			nb = this->nbrubandef;

			while (nb--)
			{
				this->AddRuban(&this->trubandef[num].first, this->trubandef[num].origin, this->trubandef[num].dec);
				num++;
			}

			break;
	}

	if (!ARXPausedTimer) this->currduration += _ulTime;
}

//-----------------------------------------------------------------------------
void CSpeed::DrawRuban(int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2)
{
	int numsuiv;

	float	dsize = 0.f;
	int		r1 = ((int)(r * 255.f)) << 16;
	int		g1 = ((int)(g * 255.f)) << 16;
	int		b1 = ((int)(b * 255.f)) << 16;
	int		rr2 = ((int)(r2 * 255.f)) << 16;
	int		gg2 = ((int)(g2 * 255.f)) << 16;
	int		bb2 = ((int)(b2 * 255.f)) << 16;
	int		dr = (rr2 - r1) / dec;
	int		dg = (gg2 - g1) / dec;
	int		db = (bb2 - b1) / dec;

	for (;;)
	{
		numsuiv = this->truban[num].next;

		if ((num >= 0) && (numsuiv >= 0))
		{
			Draw3DLineTex2(this->truban[num].pos, this->truban[numsuiv].pos, size, Color(r1 >> 16, g1 >> 16, b1 >> 16, 0), Color((r1 + dr) >> 16, (g1 + dg) >> 16, (b1 + db) >> 16, 0));
			r1 += dr;
			g1 += dg;
			b1 += db;
			size -= dsize;
		}
		else
		{
			break;
		}

		num = numsuiv;
	}
}

//-----------------------------------------------------------------------------
float CSpeed::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	GRenderer->ResetTexture(0);
	
	for (int i = 0; i < nbrubandef; i++)
	{
		this->DrawRuban(trubandef[i].first,
		                trubandef[i].size,
		                trubandef[i].dec,
		                trubandef[i].r, trubandef[i].g, trubandef[i].b,
		                trubandef[i].r2, trubandef[i].g2, trubandef[i].b2) ;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);

	return 0;
}

//-----------------------------------------------------------------------------
CCreateFood::CCreateFood()
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	pPS = new ParticleSystem();
}

//-----------------------------------------------------------------------------
CCreateFood::~CCreateFood()
{
}

//-----------------------------------------------------------------------------
void CCreateFood::Create()
{
	eSrc.x = player.pos.x;
	eSrc.y = player.pos.y;
	eSrc.z = player.pos.z;

	pPS->SetPos(eSrc);
	ParticleParams cp;
	cp.iNbMax = 350;
	cp.fLife = 800;
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 200;
	cp.p3Pos.z = 100;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(5);
	cp.fSpeed = 120; 
	cp.fSpeedRandom = 84; 
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = -10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 80;

	cp.fStartSize = 8;
	cp.fStartSizeRandom = 8;
	cp.fStartColor[0] = 105; 
	cp.fStartColor[1] = 105; 
	cp.fStartColor[2] = 20;
	cp.fStartColor[3] = 145; 
	cp.fStartColorRandom[0] = 50;
	cp.fStartColorRandom[1] = 50;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 10;

	cp.fEndSize = 6;
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 20;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 5;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 40;
	cp.fEndColorRandom[1] = 40;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 0;

	cp.iBlendMode = 0;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph/particles/create_food", 0, 100); //5

	fSize = 1;
}

//---------------------------------------------------------------------
void CCreateFood::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

//ARX_BEGIN: jycorbel (2010-07-20) - @TBE : dead code
/*
if (ulCurrentTime >= ulDuration)
*/
//ARX_END: jycorbel (2010-07-20)

	eSrc.x = inter.iobj[0]->pos.x;
	eSrc.y = inter.iobj[0]->pos.y;
	eSrc.z = inter.iobj[0]->pos.z;


//ARX_BEGIN: jycorbel (2010-07-20) - Correct bug when this spell is cast, the function update particule after-life
	if( ulCurrentTime <= ulDuration ) //ulCalc (time before end spell) must be positive !
//ARX_END: jycorbel (2010-07-20)
	{
	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff =  static_cast<long>(ulCalc);



		if (ff < 1500)
		{
			pPS->uMaxParticles = 0;
			pPS->ulParticleSpawn = PARTICLE_CIRCULAR;
			pPS->p3ParticleGravity.x = 0;
			pPS->p3ParticleGravity.y = 0;
			pPS->p3ParticleGravity.z = 0;

		std::list<Particle *>::iterator i;

		for (i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i)
		{
			Particle * pP = *i;

			if (pP->isAlive())
			{
				pP->fColorEnd[3] = 0;

					if (pP->ulTime + ff < pP->ulTTL)
					{
						pP->ulTime = pP->ulTTL - ff;
					}
				}
			}
		}
	}

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);
}

//---------------------------------------------------------------------
float CCreateFood::Render()
{
	pPS->Render();

	return 1;
}

