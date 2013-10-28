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

CFireBall::CFireBall() : CSpellFx() {
	
	eSrc = Vec3f_ZERO;
	
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	
	bExplo = false;
	
	// TODO using memset on a class is naughty
	
	memset(&fire_1, 0, sizeof(fire_1));
	fire_1.iNbMax = 200;
	fire_1.fLife = 550;
	fire_1.fLifeRandom = 500;
	fire_1.p3Pos = Vec3f_ZERO;
	fire_1.fAngle = radians(3);
	fire_1.fSpeed = 0;
	fire_1.fSpeedRandom = 0;
	fire_1.p3Gravity = Vec3f_ZERO;
	fire_1.fFlash = 0;
	fire_1.fRotation = 50;
	fire_1.fStartColor[0] = 22;
	fire_1.fStartColor[1] = 30;
	fire_1.fStartColor[2] = 30;
	fire_1.fStartColor[3] = 0;
	fire_1.fStartColorRandom[0] = 22;
	fire_1.fStartColorRandom[1] = 0;
	fire_1.fStartColorRandom[2] = 0;
	fire_1.fStartColorRandom[3] = 2;
	fire_1.fEndSize = 0;
	fire_1.fEndSizeRandom = 2;
	fire_1.fEndColor[0] = 25;
	fire_1.fEndColor[1] = 25;
	fire_1.fEndColor[2] = 0;
	fire_1.fEndColor[3] = 50;
	fire_1.fEndColorRandom[0] = 50; 
	fire_1.fEndColorRandom[1] = 0;
	fire_1.fEndColorRandom[2] = 0;
	fire_1.fEndColorRandom[3] = 120; 

	memset(&fire_2, 0, sizeof(fire_2));
	fire_2.iNbMax = 20;
	fire_2.fLife = 550;
	fire_2.fLifeRandom = 500;
	fire_2.p3Pos = Vec3f_ZERO; 
	fire_2.fAngle = radians(3);
	fire_2.fSpeed = 0;
	fire_2.fSpeedRandom = 0;
	fire_2.p3Gravity = Vec3f_ZERO;
	fire_2.fFlash = 0;
	fire_2.fRotation = 50;
	fire_2.fStartColor[0] = 22;
	fire_2.fStartColor[1] = 30;
	fire_2.fStartColor[2] = 30;
	fire_2.fStartColor[3] = 0;
	fire_2.fStartColorRandom[0] = 22;
	fire_2.fStartColorRandom[1] = 0;
	fire_2.fStartColorRandom[2] = 0;
	fire_2.fStartColorRandom[3] = 2;
	fire_2.fEndColor[0] = 25;
	fire_2.fEndColor[1] = 25;
	fire_2.fEndColor[2] = 0;
	fire_2.fEndColor[3] = 0;
	fire_2.fEndColorRandom[0] = 50; 
	fire_2.fEndColorRandom[1] = 0;
	fire_2.fEndColorRandom[2] = 0;
	fire_2.fEndColorRandom[3] = 120; 

	memset(&smoke, 0, sizeof(smoke));
	smoke.iNbMax = 30;
	smoke.fLife = 2000;
	smoke.fLifeRandom = 3000;
	smoke.p3Pos.x = 0;
	smoke.p3Pos.y = 10;
	smoke.p3Pos.z = 10;
	smoke.fAngle = radians(9);
	smoke.fSpeed = 150; 
	smoke.fSpeedRandom = 150; 
	smoke.p3Gravity.x = 0;
	smoke.p3Gravity.y = -10;
	smoke.p3Gravity.z = 0;
	smoke.fFlash = 0;
	smoke.fRotation = 90;
	smoke.fStartSize = 0;
	smoke.fStartSizeRandom = 2;
	smoke.fStartColor[0] = 70;
	smoke.fStartColor[1] = 70;
	smoke.fStartColor[2] = 51;
	smoke.fStartColor[3] = 50;
	smoke.fStartColorRandom[0] = 0;
	smoke.fStartColorRandom[1] = 0;
	smoke.fStartColorRandom[2] = 0;
	smoke.fStartColorRandom[3] = 0;
	smoke.fEndColor[0] = 0;
	smoke.fEndColor[1] = 0;
	smoke.fEndColor[2] = 0;
	smoke.fEndColor[3] = 27; 
	smoke.fEndColorRandom[0] = 0;
	smoke.fEndColorRandom[1] = 0;
	smoke.fEndColorRandom[2] = 0;
	smoke.fEndColorRandom[3] = 20;
}

CFireBall::~CFireBall()
{
}

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

	for(i = pPSSmoke.listParticle.begin(); i != pPSSmoke.listParticle.end(); ++i) {
		Particle * pP = *i;

		if(pP->isAlive()) {
			if(pP->ulTime + ff < pP->ulTTL) {
				pP->ulTime = pP->ulTTL - ff;
			}
		}
	}

	// Light
	if(lLightId != -1)
		lLightId = -1;
}

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

	//FIRE
	fire_1.p3Direction = -eMove;
	fire_1.fStartSize = 1 * _fLevel; 
	fire_1.fStartSizeRandom = 2 * _fLevel; 

	pPSFire.SetParams(fire_1);
	pPSFire.fParticleFreq = 100.0f;
	pPSFire.ulParticleSpawn = 0;
	pPSFire.SetTexture("graph/particles/fire", 0, 200);

	pPSFire.Update(0);

	//FIRE
	fire_2.p3Direction = -eMove;
	fire_2.fStartSize = 1 * _fLevel; 
	fire_2.fStartSizeRandom = 2 * _fLevel; 
	fire_2.fEndSize = 3 * _fLevel; 
	fire_2.fEndSizeRandom = 2 * _fLevel; 

	pPSFire2.SetParams(fire_2);
	pPSFire2.fParticleFreq = 20.0f;
	pPSFire2.ulParticleSpawn = 0;
	pPSFire2.SetTexture("graph/particles/fire", 0, 200);
	pPSFire2.Update(0);

	// Smoke
	smoke.p3Direction = -eMove;
	smoke.fEndSize = 7 * _fLevel;
	smoke.fEndSizeRandom = 2 * _fLevel; 

	pPSSmoke.SetParams(smoke);
	pPSSmoke.ulParticleSpawn = 0;
	pPSSmoke.fParticleFreq = 20.0f;

	pPSSmoke.SetTexture("graph/particles/big_greypouf", 0, 0);
	pPSSmoke.Update(0);

	pPSFire.SetPos(eSrc);
	pPSFire2.SetPos(eSrc);
	pPSSmoke.SetPos(eSrc);

	// Light
	lLightId = -1; 
	eCurPos = eSrc;
}

#define MIN_TIME_FIREBALL 2000  //750
void CFireBall::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulCurrentTime > MIN_TIME_FIREBALL) {
		// smoke en retard
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.Update(aulTime);
		eCurPos += eMove * (aulTime * 0.0045f);
		pPSFire.SetPos(eCurPos);
		pPSFire.fParticleSpeed = 100;
		pPSFire.fParticleSpeedRandom = 200;
		pPSFire.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.SetPos(eCurPos);
		pPSFire2.fParticleSpeed = 100;
		pPSFire2.fParticleSpeedRandom = 100;
	} else {
		float afAlpha = 0.f;
	
		if(spells[spellinstance].caster == 0) {
			SetAngle(player.angle.getPitch());
			afAlpha = player.angle.getYaw();
			long idx = GetGroupOriginByName(entities[spells[spellinstance].caster]->obj, "chest");

			if(idx) {
				eCurPos.x = entities[spells[spellinstance].caster]->obj->vertexlist3[idx].v.x - fBetaRadSin * 60;
				eCurPos.y = entities[spells[spellinstance].caster]->obj->vertexlist3[idx].v.y;
				eCurPos.z = entities[spells[spellinstance].caster]->obj->vertexlist3[idx].v.z + fBetaRadCos * 60;
			} else {
				eCurPos.x = player.pos.x - fBetaRadSin * 60;
				eCurPos.y = player.pos.y;
				eCurPos.z = player.pos.z + fBetaRadCos * 60;
			}
		} else {
			SetAngle(entities[spells[spellinstance].caster]->angle.getPitch());

			eCurPos.x = entities[spells[spellinstance].caster]->pos.x - fBetaRadSin * 60;
			eCurPos.y = entities[spells[spellinstance].caster]->pos.y;
			eCurPos.z = entities[spells[spellinstance].caster]->pos.z + fBetaRadCos * 60;

			if ((ValidIONum(spells[spellinstance].caster))
			        && (entities[spells[spellinstance].caster]->ioflags & IO_NPC))
			{
				eCurPos.x -= EEsin(radians(entities[spells[spellinstance].caster]->angle.getPitch())) * 30.f;
				eCurPos.y -= 80.f;
				eCurPos.z += EEcos(radians(entities[spells[spellinstance].caster]->angle.getPitch())) * 30.f;
			}
			
			Entity * io = entities[spells[spellinstance].caster];

			if(ValidIONum(io->targetinfo)) {
				Vec3f * p1 = &eCurPos;
				Vec3f p2 = entities[io->targetinfo]->pos;
				p2.y -= 60.f;
				afAlpha = 360.f - (degrees(getAngle(p1->y, p1->z, p2.y, p2.z + glm::distance(Vec2f(p2.x, p2.z), Vec2f(p1->x, p1->z))))); //alpha entre orgn et dest;
			}
		}

		eMove.x = - fBetaRadSin * 100 * cos(radians(MAKEANGLE(afAlpha)));
		eMove.y = sin(radians(MAKEANGLE(afAlpha))) * 100;
		eMove.z = + fBetaRadCos * 100 * cos(radians(MAKEANGLE(afAlpha)));

		Vec3f vMove = glm::normalize(eMove);

		// smoke en retard
		pPSSmoke.p3ParticleDirection = -vMove;
		pPSSmoke.SetPos(eCurPos);
		pPSSmoke.RecomputeDirection();
		eCurPos = eCurPos + eMove * (aulTime * 0.0045f);
		pPSFire.p3ParticleDirection = -vMove;
		pPSFire.RecomputeDirection();
		pPSFire.SetPos(eCurPos);
		pPSFire.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.p3ParticleDirection = -vMove;
		pPSFire2.p3ParticleGravity = -eMove * 2.f;
		pPSFire2.RecomputeDirection();
		pPSFire2.SetPos(eCurPos);
	}
	
	pPSFire.Update(aulTime);
	pPSFire2.Update(aulTime);
}

void CFireBall::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	pPSFire.Render();
	pPSFire2.Render();
	pPSSmoke.Render();
}

CIceProjectile::~CIceProjectile()
{
	stite_count--;

	if(stite && (stite_count <= 0)) {
		stite_count = 0;
		delete stite;
		stite = NULL;
	}

	smotte_count--;

	if(smotte && (smotte_count <= 0)) {
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
		stite = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/stalagmite.teo");

	stite_count++;

	if (!smotte)
		smotte = LoadTheObj("graph/obj3d/interactive/fix_inter/stalagmite/motte.teo");

	smotte_count++;

	iMax = MAX_ICE;
	fStep = 20;
}

void CIceProjectile::Create(Vec3f aeSrc, float afBeta, float fLevel)
{
	iMax = (int)(30 + fLevel * 5.2f);

	Create(aeSrc, afBeta);
}

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

	if(!Visible(&s, &e, NULL, &h)) {
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
	tv1a[0].p = s + Vec3f(0.f, 100.f, 0.f);
	tv1a[end].p = e + Vec3f(0.f, 100.f, 0.f);

	Split(tv1a, 0, end, 80, 0.5f, 0, 1, 80, 0.5f);

	for(int i = 0; i < iNumber; i++) {
		float t = rnd();

		if (t < 0.5f)
			tType[i] = 0;
		else
			tType[i] = 1;

		tSize[i] = Vec3f_ZERO;
		tSizeMax[i] = randomVec() + Vec3f(0.f, 0.2f, 0.f);

		if(tType[i] == 0) {
			xmin = 1.2f;
			ymin = 1;
			zmin = 1.2f;
		} else {
			xmin = 0.4f;
			ymin = 0.3f;
			zmin = 0.4f;
		}

		if(tSizeMax[i].x < xmin)
			tSizeMax[i].x = xmin;

		if(tSizeMax[i].y < ymin)
			tSizeMax[i].y = ymin;

		if(tSizeMax[i].z < zmin)
			tSizeMax[i].z = zmin;

		int iNum = static_cast<int>(i / 2);

		if(tType[i] == 0) {
			tPos[i].x = tv1a[iNum].p.x + frand2() * 80;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 80;
		} else {
			tPos[i].x = tv1a[iNum].p.x + frand2() * 40;
			tPos[i].y = tv1a[iNum].p.y;
			tPos[i].z = tv1a[iNum].p.z + frand2() * 40;
		}

		long ttt = ARX_DAMAGES_GetFree();
		if(ttt != -1) {
			damages[ttt].pos = tPos[i];
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

void CIceProjectile::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulDuration - ulCurrentTime < 1000) {
		fColor = (ulDuration - ulCurrentTime) * ( 1.0f / 1000 );

		for(int i = 0; i < iNumber; i++) {
			tSize[i].y *= fColor;
		}
	}
}

void CIceProjectile::Render()
{
	int i = 0;

	if(ulCurrentTime >= ulDuration)
		return;

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	iMax = (int)((iNumber * 2) * fOneOnDuration * ulCurrentTime);

	if(iMax > iNumber)
		iMax = iNumber;

	for(i = 0; i < iMax; i++) {
		if(tSize[i].x < tSizeMax[i].x)
			tSize[i].x += 0.1f;

		if(tSize[i].x > tSizeMax[i].x)
			tSize[i].x = tSizeMax[i].x;

		if(tSize[i].y < tSizeMax[i].y)
			tSize[i].y += 0.1f;

		if(tSize[i].y < 0)
			tSize[i].y = 0;

		if(tSize[i].y > tSizeMax[i].y)
			tSize[i].y = tSizeMax[i].y;

		if(tSize[i].z < tSizeMax[i].z)
			tSize[i].z += 0.1f;

		if(tSize[i].z > tSizeMax[i].z)
			tSize[i].z = tSizeMax[i].z;

		Anglef stiteangle;
		Vec3f stitepos;
		Vec3f stitescale;
		Color3f stitecolor;

		stiteangle.setPitch((float) cos(radians(tPos[i].x)) * 360);
		stiteangle.setYaw(0);
		stiteangle.setRoll(0);
		stitepos = tPos[i];

		float tt;
		tt = tSizeMax[i].y * fColor;
		stitecolor.g = stitecolor.r = tt * 0.7f;
		stitecolor.b = tt * 0.9f;

		if(stitecolor.r > 1)
			stitecolor.r = 1;

		if(stitecolor.g > 1)
			stitecolor.g = 1;

		if(stitecolor.b > 1)
			stitecolor.b = 1;

		stitescale = tSize[i];

		if(tType[i] == 0)
			DrawEERIEObjEx(smotte, &stiteangle, &stitepos, &stitescale, stitecolor);
		else
			DrawEERIEObjEx(stite, &stiteangle, &stitepos, &stitescale, stitecolor);
	}
	
	for(i = 0; i < min(iNumber, iMax + 1); i++) {
		
		float t = rnd();
		if(t < 0.01f) {
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tPos[i] + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 20.f;
				float t = min(2000.f + rnd() * 2000.f,
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
				pd->ov = tPos[i] + randomVec(-5.f, 5.f) - Vec3f(0.f, 50.f, 0.f);
				pd->move = Vec3f(0.f, 2.f - 4.f * rnd(), 0.f);
				pd->siz = 0.5f;
				float t = min(2000.f + rnd() * 1000.f,
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

	while(nb--) {
		this->truban[nb].actif = 0;
	}

	nb = (entities[this->num]->obj)->nbgroups;

	if(nb > 256)
		nb = 256;

	EERIE_GROUPLIST * grouplist = entities[this->num]->obj->grouplist;
	nb >>= 1;

	while(nb--) {
		float col = 0.05f + (rnd() * 0.05f);
		float size = 4.f + (4.f * rnd());
		int taille = Random::get(8, 16);
		this->AddRubanDef(grouplist->origin, size, taille, col, col, col, 0.f, 0.f, 0.f);
		grouplist += 2;
	}

	this->tp = TextureContainer::Load("graph/particles/fire");
}

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

int CSpeed::GetFreeRuban()
{
	int nb = 2048;

	while(nb--) {
		if(!this->truban[nb].actif)
			return nb;
	}

	return -1;
}

void CSpeed::AddRuban(int * f, int id, int dec)
{
	int	num;

	num = this->GetFreeRuban();

	if(num >= 0) {
		truban[num].actif = 1;
		truban[num].pos = entities[this->num]->obj->vertexlist3[id].v;

		if(*f < 0) {
			*f = num;
			this->truban[num].next = -1;
		} else {
			this->truban[num].next = *f;
			*f = num;
		}

		int nb = 0, oldnum = num;

		while(num != -1) {
			nb++;
			oldnum = num;
			num = this->truban[num].next;
		}

		if(oldnum < 0)
			ARX_DEAD_CODE();

		if(nb > dec) {
			this->truban[oldnum].actif = 0;
			num = *f;
			nb -= 2;

			while(nb--) {
				num = this->truban[num].next;
			}

			this->truban[num].next = -1;
		}
	}
}

void CSpeed::Update(unsigned long _ulTime)
{
	int	nb, num;

	switch(this->key) {
		case 0:
			break;
		case 1:
			if(arxtime.is_paused())
				break;

			if(this->currduration > this->duration) {
				this->key++;
			}

			num = 0;
			nb = this->nbrubandef;

			while(nb--) {
				this->AddRuban(&this->trubandef[num].first, this->trubandef[num].origin, this->trubandef[num].dec);
				num++;
			}

			break;
	}

	if(!arxtime.is_paused())
		this->currduration += _ulTime;
}

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

	for(;;) {
		numsuiv = this->truban[num].next;

		if(num >= 0 && numsuiv >= 0) {
			Draw3DLineTex2(this->truban[num].pos, this->truban[numsuiv].pos, size, Color(r1 >> 16, g1 >> 16, b1 >> 16, 0), Color((r1 + dr) >> 16, (g1 + dg) >> 16, (b1 + db) >> 16, 0));
			r1 += dr;
			g1 += dg;
			b1 += db;
			size -= dsize;
		} else {
			break;
		}

		num = numsuiv;
	}
}

void CSpeed::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

	GRenderer->ResetTexture(0);
	
	for(int i = 0; i < nbrubandef; i++) {
		this->DrawRuban(trubandef[i].first,
		                trubandef[i].size,
		                trubandef[i].dec,
		                trubandef[i].r, trubandef[i].g, trubandef[i].b,
		                trubandef[i].r2, trubandef[i].g2, trubandef[i].b2) ;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
}

CCreateFood::CCreateFood() {
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	pPS = new ParticleSystem();
}

CCreateFood::~CCreateFood() { }

void CCreateFood::Create() {
	
	eSrc = player.pos;
	
	pPS->SetPos(eSrc);
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
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

void CCreateFood::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

//ARX_BEGIN: jycorbel (2010-07-20) - @TBE : dead code
/*
if (ulCurrentTime >= ulDuration)
*/
//ARX_END: jycorbel (2010-07-20)

	eSrc = entities.player()->pos;


//ARX_BEGIN: jycorbel (2010-07-20) - Correct bug when this spell is cast, the function update particule after-life
	if( ulCurrentTime <= ulDuration ) //ulCalc (time before end spell) must be positive !
//ARX_END: jycorbel (2010-07-20)
	{
	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff =  static_cast<long>(ulCalc);

		if(ff < 1500) {
			pPS->uMaxParticles = 0;
			pPS->ulParticleSpawn = PARTICLE_CIRCULAR;
			pPS->p3ParticleGravity = Vec3f_ZERO;

		std::list<Particle *>::iterator i;

		for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
			Particle * pP = *i;

			if(pP->isAlive()) {
				pP->fColorEnd[3] = 0;

					if(pP->ulTime + ff < pP->ulTTL) {
						pP->ulTime = pP->ulTTL - ff;
					}
				}
			}
		}
	}

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);
}

void CCreateFood::Render()
{
	pPS->Render();
}
