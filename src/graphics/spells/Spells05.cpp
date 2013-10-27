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

#include "graphics/spells/Spells05.h"

#include <climits>

#include "animation/AnimationRender.h"

#include "core/Core.h"
#include "core/GameTime.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Particle.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleParams.h"
#include "graphics/texture/TextureStage.h"

#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include <list>

extern ParticleManager * pParticleManager;


EERIE_3DOBJ * ssol = NULL;
long ssol_count = 0;
EERIE_3DOBJ * slight = NULL;
long slight_count = 0;
EERIE_3DOBJ * srune = NULL;
long srune_count = 0;
EERIE_3DOBJ * smotte = NULL;
long smotte_count = 0;
EERIE_3DOBJ * stite = NULL;
long stite_count = 0;
EERIE_3DOBJ * smissile = NULL;
long smissile_count = 0;
EERIE_3DOBJ * spapi = NULL;
long spapi_count = 0;
EERIE_3DOBJ * svoodoo = NULL;
long svoodoo_count = 0;

CCurePoison::CCurePoison()
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	pPS = new ParticleSystem();
}

CCurePoison::~CCurePoison()
{
}

void CCurePoison::Create()
{
	SetAngle(0);

	eSrc.x = entities[spells[spellinstance].target]->pos.x;
	eSrc.y = entities[spells[spellinstance].target]->pos.y;

	if(spells[spellinstance].target == 0)
		eSrc.y += 200;

	eSrc.z = entities[spells[spellinstance].target]->pos.z;

	pPS->SetPos(eSrc);
	ParticleParams cp;
	memset(&cp, 0, sizeof(cp));
	cp.iNbMax = 350;
	cp.fLife = 800;
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 0;
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

	cp.fStartSize = 8;//6;
	cp.fStartSizeRandom = 8;
	cp.fStartColor[0] = 20;
	cp.fStartColor[1] = 205;
	cp.fStartColor[2] = 20;
	cp.fStartColor[3] = 245;
	cp.fStartColorRandom[0] = 50;
	cp.fStartColorRandom[1] = 50;
	cp.fStartColorRandom[2] = 50;
	cp.fStartColorRandom[3] = 10;

	cp.fEndSize = 6;
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 5;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 5;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 40;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 0;
	cp.bTexInfo = false;
	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph/particles/cure_poison", 0, 100); //5

	pPS->lLightId = GetFreeDynLight();

	if(pPS->lLightId != -1) {
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 1.5f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.f;
		DynLight[id].rgb.g = 1.f;
		DynLight[id].rgb.b = 0.0f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].time_creation = (unsigned long)(arxtime);
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	fSize = 1;
}

void CCurePoison::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if(ulCurrentTime >= ulDuration)
		return;

	eSrc.x = entities[spells[spellinstance].target]->pos.x;
	eSrc.y = entities[spells[spellinstance].target]->pos.y;

	if(spells[spellinstance].target == 0)
		eSrc.y += 200;

	eSrc.z = entities[spells[spellinstance].target]->pos.z;

	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	arx_assert(ulCalc <= LONG_MAX);
	long ff = 	static_cast<long>(ulCalc);

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

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);

	if(pPS->lLightId == -1)
		pPS->lLightId = GetFreeDynLight();

	if(pPS->lLightId != -1) {
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.4f;
		DynLight[id].rgb.g = 1.f;
		DynLight[id].rgb.b = 0.4f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].time_creation = (unsigned long)(arxtime);
		DynLight[id].extras = 0;
	}
}

void CCurePoison::Render()
{
	if(ulCurrentTime >= ulDuration)
		return;

	pPS->Render();
}

CRuneOfGuarding::CRuneOfGuarding() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	if(!ssol) {
		ssol = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	}
	ssol_count++;

	if(!slight) {
		slight = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard02.teo");
	}
	slight_count++;
	
	if(!srune) {
		srune = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard03.teo");
	}
	srune_count++;
}

CRuneOfGuarding::~CRuneOfGuarding()
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

void CRuneOfGuarding::Create(Vec3f _eSrc, float _fBeta) {
	
	SetDuration(ulDuration);
	SetAngle(_fBeta);
	eSrc = _eSrc;
	eTarget = eSrc;
	fSize = 1;
	bDone = true;
	
	lLightId = GetFreeDynLight();
	if(lLightId != -1) {
		long id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 0.7f + 2.3f;
		DynLight[id].fallend = 500.f;
		DynLight[id].fallstart = 400.f;
		DynLight[id].rgb.r = 1.0f;
		DynLight[id].rgb.g = 0.2f;
		DynLight[id].rgb.b = 0.2f;
		DynLight[id].pos = eSrc - Vec3f(0.f, 50.f, 0.f);
		DynLight[id].time_creation = (unsigned long)(arxtime);
		DynLight[id].duration = 200;
	}
}

void CRuneOfGuarding::Update(unsigned long _ulTime) {
	
	ulCurrentTime += _ulTime;
	
	float fa = 1.0f - rnd() * 0.15f;
	
	if(lLightId != -1) {
		long id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 0.7f + 2.3f * fa;
		DynLight[id].fallend = 350.f;
		DynLight[id].fallstart = 150.f;
		DynLight[id].rgb.r = 1.0f;
		DynLight[id].rgb.g = 0.2f;
		DynLight[id].rgb.b = 0.2f;
		DynLight[id].time_creation = (unsigned long)(arxtime);
		DynLight[id].duration = 200;
	}
}

void CRuneOfGuarding::Render()
{
	float x = eSrc.x;
	float y = eSrc.y - 20;
	float z = eSrc.z;

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	Anglef stiteangle;
	Color3f stitecolor;
	
	float stiteangleb = (float) ulCurrentTime * fOneOnDuration * 120;
	stiteangle.setYaw(0);
	stiteangle.setRoll(0);
	Vec3f stitepos = Vec3f(x, y, z);

	float gtc = arxtime.get_updated();
	float v = EEsin(gtc * ( 1.0f / 1000 )) * ( 1.0f / 10 );
	stiteangle.setPitch(MAKEANGLE(gtc * ( 1.0f / 1000 )));
	stitecolor.r = 0.4f - v;
	stitecolor.g = 0.4f - v;
	stitecolor.b = 0.6f - v;
	Vec3f stitescale = Vec3f(1.f, -0.1f, 1.f);
	
	if(slight) {
		DrawEERIEObjEx(slight, &stiteangle, &stitepos, &stitescale, stitecolor);
	}
	
	stiteangle.setPitch(stiteangleb);
	stitecolor.r = 0.6f;
	stitecolor.g = 0.f;
	stitecolor.b = 0.f;
	stitescale = Vec3f(2.f);
	
	if(ssol) {
		DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, stitecolor);
	}
	
	stitecolor.r = 0.6f;
	stitecolor.g = 0.3f;
	stitecolor.b = 0.45f;
	stitescale = Vec3f(1.8f);
	
	if(srune) {
		DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, stitecolor);
	}
	
	for(int n = 0; n < 4; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		pd->ov = Vec3f(x + frand2() * 40.f, y, z + frand2() * 40.f);
		pd->move = Vec3f(0.8f * frand2(), -4.f * rnd(), 0.8f * frand2());
		pd->scale = Vec3f(-0.1f);
		pd->tolive = Random::get(2600, 3200);
		pd->tc = tex_p2;
		pd->siz = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
}

void LaunchPoisonExplosion(Vec3f * aePos) {
	
	// système de partoches pour l'explosion
	ParticleSystem * pPS = new ParticleSystem();
	ParticleParams cp;
	cp.iNbMax = 80; 
	cp.fLife = 1500;
	cp.fLifeRandom = 500;
	cp.p3Pos = Vec3f(5);
	cp.p3Direction.x = 0;
	cp.p3Direction.y = 4;
	cp.p3Direction.z = 0;
	cp.fAngle = radians(360);
	cp.fSpeed = 200;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 17;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 80;
	cp.bRotationRandomDirection = true;
	cp.bRotationRandomStart = true;

	cp.fStartSize = 5;
	cp.fStartSizeRandom = 3;
	cp.fStartColor[0] = 0;
	cp.fStartColor[1] = 76;
	cp.fStartColor[2] = 0;
	cp.fStartColor[3] = 0;
	cp.fStartColorRandom[0] = 0;
	cp.fStartColorRandom[1] = 0;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 150; 
	cp.bStartLock = false;

	cp.fEndSize = 30;
	cp.fEndSizeRandom = 5;
	cp.fEndColor[0] = 0;
	cp.fEndColor[1] = 0;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 25; 
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 20; 
	cp.bEndLock = false;

	cp.iBlendMode = 3;
	cp.iFreq = -1;
	cp.bTexInfo = false;
	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	pPS->SetTexture("graph/particles/big_greypouf", 0, 200);

	pPS->SetPos(*aePos);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	std::list<Particle *>::iterator i;

	for(i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i) {
		Particle * pP = *i;

		if(pP->isAlive()) {
			if(pP->p3Velocity.y >= 0.5f * 200)
				pP->p3Velocity.y = 0.5f * 200;

			if(pP->p3Velocity.y <= -0.5f * 200)
				pP->p3Velocity.y = -0.5f * 200;
		}
	}

	if(pParticleManager) {
		pParticleManager->AddSystem(pPS);
	}
}


CPoisonProjectile::CPoisonProjectile() : eSrc(Vec3f_ZERO) {
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
}

void CPoisonProjectile::Create(Vec3f _eSrc, float _fBeta)
{
	int i;

	SetDuration(ulDuration);

	SetAngle(_fBeta);

	eSrc = _eSrc;

	bOk = false;

	eMove = Vec3f(-fBetaRadSin * 2, 0.f, fBetaRadCos * 2); 

	Vec3f s, e, h;
	s = eSrc;
	e = eSrc;

	i = 0;

	while(Visible(&s, &e, NULL, &h) && i < 20) {
		e.x -= fBetaRadSin * 50;
		e.z += fBetaRadCos * 50;

		i++;
	}

	e.y += 0.f;

	pathways[0].p = eSrc;
	pathways[9].p = e;
	Split(pathways, 0, 9, 10 * fBetaRadCos, 10, 10, 10, 10 * fBetaRadSin, 10);

	if (0)
		for (i = 0; i < 10; i++)
		{
			if (pathways[i].p.y >= eSrc.y + 150)
			{
				pathways[i].p.y = eSrc.y + 150;
			}

			if (pathways[i].p.y <= eSrc.y + 50)
			{
				pathways[i].p.y = eSrc.y + 50;
			}
		}

	fTrail = -1;

	//-------------------------------------------------------------------------
	// système de partoches
	ParticleParams cp;
	cp.iNbMax = 5;
	cp.fLife = 2000;
	cp.fLifeRandom = 1000;
	cp.p3Pos = Vec3f_ZERO;
	cp.p3Direction = -eMove;
	cp.fAngle = 0;
	cp.fSpeed = 10;
	cp.fSpeedRandom = 10;
	cp.p3Gravity = Vec3f_ZERO;
	cp.fFlash = 21;
	cp.fRotation = 80;
	cp.bRotationRandomDirection = true;
	cp.bRotationRandomStart = true;

	cp.fStartSize = 5; 
	cp.fStartSizeRandom = 3;
	cp.fStartColor[0] = 0;
	cp.fStartColor[1] = 50;
	cp.fStartColor[2] = 0;
	cp.fStartColor[3] = 40; 
	cp.fStartColorRandom[0] = 0;
	cp.fStartColorRandom[1] = 100;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 50;
	cp.bStartLock = false;

	cp.fEndSize = 8; 
	cp.fEndSizeRandom = 13;
	cp.fEndColor[0] = 0;
	cp.fEndColor[1] = 60;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 40;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 100;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 50;
	cp.bEndLock = false;

	cp.iBlendMode = 5;

	pPS.SetParams(cp);
	pPS.ulParticleSpawn = 0;

	pPS.SetTexture("graph/particles/big_greypouf", 0, 200);

	pPS.fParticleFreq = -1;

	pPS.bParticleFollow = true;

	pPS.SetPos(eSrc);
	pPS.Update(0);
}

void CPoisonProjectile::Update(unsigned long _ulTime)
{
	if(ulCurrentTime <= 2000) {
		ulCurrentTime += _ulTime;
	}

	// on passe de 5 à 100 partoches en 1.5secs
	if(ulCurrentTime < 750) {
		pPS.iParticleNbMax = 2;
		pPS.Update(_ulTime);
	} else {
		if(!bOk) {
			bOk = true;

			// go
			ParticleParams cp;
			cp.iNbMax = 100;
			cp.fLife = 500;
			cp.fLifeRandom = 300;
			cp.p3Pos.x = fBetaRadSin * 20;
			cp.p3Pos.y = 0.f;
			cp.p3Pos.z = fBetaRadCos * 20;

			cp.p3Direction = -eMove;

			cp.fAngle = radians(4);
			cp.fSpeed = 150;
			cp.fSpeedRandom = 50;//15;
			cp.p3Gravity.x = 0;
			cp.p3Gravity.y = 10;
			cp.p3Gravity.z = 0;
			cp.fFlash = 0;
			cp.fRotation = 80;
			cp.bRotationRandomDirection = true;
			cp.bRotationRandomStart = true;

			cp.fStartSize = 2;
			cp.fStartSizeRandom = 2;
			cp.fStartColor[0] = 0;
			cp.fStartColor[1] = 39;
			cp.fStartColor[2] = 0;
			cp.fStartColor[3] = 100;
			cp.fStartColorRandom[0] = 50;
			cp.fStartColorRandom[1] = 21;
			cp.fStartColorRandom[2] = 0;
			cp.fStartColorRandom[3] = 0;
			cp.bStartLock = false;

			cp.fEndSize = 7;
			cp.fEndSizeRandom = 5;
			cp.fEndColor[0] = 0;
			cp.fEndColor[1] = 25; 
			cp.fEndColor[2] = 0;
			cp.fEndColor[3] = 100;
			cp.fEndColorRandom[0] = 50; 
			cp.fEndColorRandom[1] = 20;
			cp.fEndColorRandom[2] = 0;
			cp.fEndColorRandom[3] = 0; 
			cp.bEndLock = false;

			cp.iBlendMode = 5;
			cp.bTexInfo = false;
			pPSStream.SetParams(cp);
			pPSStream.ulParticleSpawn = 0;

			pPSStream.SetTexture("graph/particles/big_greypouf", 0, 200);

			pPSStream.fParticleFreq = 80;
			pPSStream.bParticleFollow = true;
		}

		pPSStream.Update(_ulTime);
		pPSStream.SetPos(eCurPos);

		pPS.Update(_ulTime);
		pPS.SetPos(eCurPos);

		fTrail = ((ulCurrentTime - 750) * (1.0f / (ulDuration - 750.0f))) * 9 * (BEZIERPrecision + 2);
	}

	if(ulCurrentTime >= ulDuration)
		lightIntensityFactor = 0.f;
	else
		lightIntensityFactor = 1.f;
}

void CPoisonProjectile::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	int n = BEZIERPrecision;
	float delta = 1.0f / n;
	
	Vec3f lastpos = pathways[0].p;
	
	int arx_check_init = -1;
	
	int i = 0;
	for(i = 0; i < 9; i++) {
		
		int kpprec = std::max(i - 1, 0);
		int kpsuiv = i + 1 ;
		int kpsuivsuiv = (i < (9 - 2)) ? kpsuiv + 1 : kpsuiv;
		
		for(int toto = 1; toto < n; toto++) {
			
			if(fTrail < i * n + toto) {
				break;
			}
			
			float t = toto * delta;
			
			float t2 = t * t ;
			float t3 = t2 * t ;
			float f0 = 2.f * t3 - 3.f * t2 + 1.f ;
			float f1 = -2.f * t3 + 3.f * t2 ;
			float f2 = t3 - 2.f * t2 + t ;
			float f3 = t3 - t2 ;
			
			float val = pathways[kpsuiv].p.x;
			float p0 = 0.5f * (val - pathways[kpprec].p.x);
			float p1 = 0.5f * (pathways[kpsuivsuiv].p.x - pathways[i].p.x);
			lastpos.x = f0 * pathways[i].p.x + f1 * val + f2 * p0 + f3 * p1;
			
			val = pathways[kpsuiv].p.y;
			p0 = 0.5f * (val - pathways[kpprec].p.y);
			p1 = 0.5f * (pathways[kpsuivsuiv].p.y - pathways[i].p.y);
			lastpos.y = f0 * pathways[i].p.y + f1 * val + f2 * p0 + f3 * p1;
			
			val = pathways[kpsuiv].p.z;
			p0 = 0.5f * (val - pathways[kpprec].p.z);
			p1 = 0.5f * (pathways[kpsuivsuiv].p.z - pathways[i].p.z);
			lastpos.z = f0 * pathways[i].p.z + f1 * val + f2 * p0 + f3 * p1;
			
			++arx_check_init;
		}
	}
	
	// arx_assert(arx_check_init >= 0); TODO why should this hold?
	
	eCurPos = lastpos;
	
	if(fTrail >= (i * n)) {
		LaunchPoisonExplosion(&lastpos);
	}
	
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
}

CMultiPoisonProjectile::CMultiPoisonProjectile(long nbmissiles)
{
	SetDuration(2000);
	uiNumber = min(5L, nbmissiles);
	pTab	 = NULL;
	pTab	 = new CPoisonProjectile*[uiNumber]();

	for(unsigned int i = 0 ; i < uiNumber ; i++) {
		pTab[i] = new CPoisonProjectile();
		pTab[i]->spellinstance = this->spellinstance;
	}
}

CMultiPoisonProjectile::~CMultiPoisonProjectile()
{
	for(unsigned int i = 0 ; i < uiNumber ; i++) {
		if(pTab[i]->lLightId != -1) {
			DynLight[pTab[i]->lLightId].duration	  = 2000;
			DynLight[pTab[i]->lLightId].time_creation = (unsigned long)(arxtime);
			pTab[i]->lLightId						  = -1;
		}

		delete pTab[i];
	}

	delete [] pTab;
}

void CMultiPoisonProjectile::Create(Vec3f _eSrc, float _afBeta = 0) {
	
	(void)_afBeta;

	float afBeta = 0.f;
	
	Entity * caster = entities[spells[spellinstance].caster];
	spells[spellinstance].hand_group = caster->obj->fastaccess.primary_attach;

	if(spells[spellinstance].hand_group != -1) {
		long group = spells[spellinstance].hand_group;
		spells[spellinstance].hand_pos = caster->obj->vertexlist3[group].v;
	}
	
	if(spells[spellinstance].caster == 0) { // player

		afBeta = player.angle.getPitch();

		if(spells[spellinstance].hand_group != -1) {
			_eSrc.x = spells[spellinstance].hand_pos.x - EEsin(radians(afBeta)) * 90;
			_eSrc.y = spells[spellinstance].hand_pos.y;
			_eSrc.z = spells[spellinstance].hand_pos.z + EEcos(radians(afBeta)) * 90;
		} else {
			_eSrc.x = player.pos.x - EEsin(radians(afBeta)) * 90;
			_eSrc.y = player.pos.y;
			_eSrc.z = player.pos.z + EEcos(radians(afBeta)) * 90;
		}
	} else {
		afBeta = entities[spells[spellinstance].caster]->angle.getPitch();

		if(spells[spellinstance].hand_group != -1) {
			_eSrc.x = spells[spellinstance].hand_pos.x - EEsin(radians(afBeta)) * 90;
			_eSrc.y = spells[spellinstance].hand_pos.y;
			_eSrc.z = spells[spellinstance].hand_pos.z + EEcos(radians(afBeta)) * 90;
		} else {
			_eSrc.x = entities[spells[spellinstance].caster]->pos.x - EEsin(radians(afBeta)) * 90;
			_eSrc.y = entities[spells[spellinstance].caster]->pos.y;
			_eSrc.z = entities[spells[spellinstance].caster]->pos.z + EEcos(radians(afBeta)) * 90;
		}
	}

	long lMax = 0;

	for(unsigned int i = 0; i < uiNumber; i++) {
		pTab[i]->Create(_eSrc, afBeta + frand2() * 10.0f);
		long lTime = ulDuration + Random::get(0, 5000);
		pTab[i]->SetDuration(lTime);
		lMax = max(lMax, lTime);

		CPoisonProjectile * pPP = (CPoisonProjectile *) pTab[i];

		pPP->lLightId = GetFreeDynLight();

		if(pPP->lLightId != -1) {
			long id						= pPP->lLightId;
			DynLight[id].exist			= 1;
			DynLight[id].intensity		= 2.3f;
			DynLight[id].fallend		= 250.f;
			DynLight[id].fallstart		= 150.f;
			DynLight[id].rgb = Color3f::green;
			DynLight[id].pos = pPP->eSrc;
			DynLight[id].time_creation	= (unsigned long)(arxtime);
			DynLight[id].duration		= 200;
		}

		pTab[i]->spellinstance = this->spellinstance;

	}

	SetDuration(lMax + 1000);
}

void CMultiPoisonProjectile::Update(unsigned long _ulTime)
{
	for(unsigned int i = 0; i < uiNumber; i++) {
		pTab[i]->Update(_ulTime);
	}
}

void CMultiPoisonProjectile::Render()
{
	for(unsigned int i = 0; i < uiNumber; i++) {
		pTab[i]->Render();

		CPoisonProjectile * pPoisonProjectile = (CPoisonProjectile *) pTab[i];

		if(pPoisonProjectile->lLightId != -1) {
			long id					= pPoisonProjectile->lLightId;
			DynLight[id].exist		= 1;
			DynLight[id].intensity	= 2.3f * pPoisonProjectile->lightIntensityFactor;
			DynLight[id].fallend	= 250.f;
			DynLight[id].fallstart	= 150.f;
			DynLight[id].rgb = Color3f::green;
			DynLight[id].pos = pPoisonProjectile->eCurPos;
			DynLight[id].time_creation = (unsigned long)(arxtime);
			DynLight[id].duration	= 200;
		}

		long t = ARX_DAMAGES_GetFree();
		AddPoisonFog(&pPoisonProjectile->eCurPos, spells[spellinstance].caster_level + 7);

		if((t != -1) && (spells[pTab[i]->spellinstance].timcreation + 1600 < (unsigned long)(arxtime)))
		{
			damages[t].pos = pPoisonProjectile->eCurPos;
			damages[t].radius = 120.f;
			float v = spells[spellinstance].caster_level;
			v = 4.f + v * ( 1.0f / 10 ) * 6.f ;
			damages[t].damages	= v * ( 1.0f / 1000 ) * framedelay;
			damages[t].area		= DAMAGE_FULL;
			damages[t].duration	= static_cast<long>(framedelay);
			damages[t].source	= spells[spellinstance].caster;
			damages[t].flags	= 0;
			damages[t].type		= DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_POISON;
			damages[t].exist	= true;
		}
	}
}

CRepelUndead::CRepelUndead() {
	
	eSrc = Vec3f_ZERO;
	eTarget = Vec3f_ZERO;
	
	ulCurrentTime = ulDuration + 1;
	
	tex_p2 = TextureContainer::Load("graph/obj3d/textures/(fx)_tsu_blueting");
	
	if(!ssol) { // Pentacle
		ssol = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard.teo");
	}
	ssol_count++;
	
	if(!slight) { // Twirl
		slight = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard02.teo");
	}
	slight_count++; //runes
	
	if(!srune) {
		srune = LoadTheObj("graph/obj3d/interactive/fix_inter/fx_rune_guard/fx_rune_guard03.teo");
	}
	srune_count++;
}

CRepelUndead::~CRepelUndead() {
	
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

void CRepelUndead::Create(Vec3f aeSrc, float afBeta) {
	
	SetDuration(ulDuration);
	eTarget = eSrc = aeSrc;
	fBeta = afBeta;
	fBetaRad = radians(fBeta);
	fBetaRadCos = (float)cos(fBetaRad);
	fBetaRadSin = (float)sin(fBetaRad);
	fSize = 1;
	bDone = true;
}

void CRepelUndead::Update(unsigned long _ulTime) {
	
	ulCurrentTime += _ulTime;
	if(spellinstance < 0) {
		return;
	}
	
	eSrc = entities[spells[spellinstance].target]->pos;
	
	if(spells[spellinstance].target == 0) {
		fBeta = player.angle.getPitch();
	} else {
		fBeta = entities[spells[spellinstance].target]->angle.getPitch();
	}
}

void CRepelUndead::Render() {
	
	if(ulCurrentTime >= ulDuration)
		return;
	
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	Anglef  eObjAngle;
	Vec3f  eObjPos;
	Vec3f  eObjScale;
	Color3f rgbObjColor;

	eObjAngle.setPitch(fBeta);
	eObjAngle.setYaw(0);
	eObjAngle.setRoll(0);
	eObjPos.x = eSrc.x;
	eObjPos.y = eSrc.y - 5.f;
	eObjPos.z = eSrc.z;
	rgbObjColor.r = 0.6f;
	rgbObjColor.g = 0.6f;
	rgbObjColor.b = 0.8f;

	float vv = 1.f + (EEsin(arxtime.get_updated() * ( 1.0f / 1000 ))); 
	vv *= ( 1.0f / 2 );
	vv += 1.1f;
	eObjScale.z = vv;
	eObjScale.y = vv;
	eObjScale.x = vv;
	
	if(ssol) {
		DrawEERIEObjEx(ssol, &eObjAngle, &eObjPos, &eObjScale, rgbObjColor);
	}
	
	vv *= 100.f;
	
	for(int n = 0; n < 4; n++) {
		
		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}
		
		float dx = -EEsin(frand2() * 360.f) * vv;
		float dz =  EEcos(frand2() * 360.f) * vv;
		pd->ov = eSrc + Vec3f(dx, 0.f, dz);
		pd->move = Vec3f(0.8f * frand2(), -4.f * rnd(), 0.8f * frand2());
		pd->scale = Vec3f(-0.1f);
		pd->tolive = Random::get(2600, 3200);
		pd->tc = tex_p2;
		pd->siz = 0.3f;
		pd->rgb = Color3f(.4f, .4f, .6f);
	}
	
	if(lLightId == -1) {
		lLightId = GetFreeDynLight();
	}
	
	if(lLightId != -1) {
		long id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallend = 350.f;
		DynLight[id].fallstart = 150.f;
		DynLight[id].rgb = Color3f(0.8f, 0.8f, 1.f);
		DynLight[id].pos = eSrc + Vec3f(0.f, -50.f, 0.f);
		DynLight[id].duration = 200;
		DynLight[id].time_creation = (unsigned long)(arxtime);
	}
}

//-----------------------------------------------------------------------------
//	LEVITATION
//-----------------------------------------------------------------------------
EERIE_3DOBJ * stone1 = NULL;
long stone1_count = 0;
EERIE_3DOBJ * stone0 = NULL;
long stone0_count = 0;

CLevitate::CLevitate()
{
	int nb = 2;

	while(nb--) {
		this->cone[nb].coned3d = NULL;
		this->cone[nb].coneind = NULL;
		this->cone[nb].conevertex = NULL;
	}

	if(!stone0) {
		stone0 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone01.teo");
	}

	stone0_count++;

	if(!stone1) {
		stone1 = loadObject("graph/obj3d/interactive/fix_inter/fx_raise_dead/stone02.teo");
	}

	stone1_count++;
}

CLevitate::~CLevitate()
{
	stone0_count--;

	if(stone0 && stone0_count <= 0) {
		stone0_count = 0;
		delete stone0;
		stone0 = NULL;
	}

	stone1_count--;

	if(stone1 && stone1_count <= 0) {
		stone1_count = 0;
		delete stone1;
		stone1 = NULL;
	}
}

void CLevitate::CreateConeStrip(float rbase, float rhaut, float hauteur, int def, int numcone) {
	
	T_CONE & c = cone[numcone];
	
	free(c.coned3d);
	free(c.conevertex);
	free(c.coneind);
	
	c.conenbvertex = def * 2 + 2;
	c.conenbfaces = def * 2 + 2;
	c.coned3d = (TexturedVertex *)malloc(c.conenbvertex * sizeof(TexturedVertex));
	c.conevertex = (Vec3f *)malloc(c.conenbvertex * sizeof(Vec3f));
	c.coneind = (unsigned short *)malloc(c.conenbvertex * sizeof(unsigned short));
	
	Vec3f * vertex = c.conevertex;
	unsigned short * pind = c.coneind;
	unsigned short ind = 0;
	int nb;
	float a = 0.f;
	float da = 360.f / (float)def;
	nb = this->cone[numcone].conenbvertex >> 1;
	
	while(nb) {
		*pind++ = ind++;
		*pind++ = ind++;
		*vertex++ = Vec3f(rhaut * EEcos(radians(a)), -hauteur, rhaut * EEsin(radians(a)));
		*vertex++ = Vec3f(rbase * EEcos(radians(a)), 0.f, rbase * EEsin(radians(a)));
		a += da;
		nb--;
	}
}

void CLevitate::Create(int def, float rbase, float rhaut, float hauteur, Vec3f * pos, unsigned long _ulDuration)
{
	SetDuration(_ulDuration);

	if(def < 3)
		return;

	this->CreateConeStrip(rbase, rhaut, hauteur, def, 0);
	this->CreateConeStrip(rbase, rhaut * 1.5f, hauteur * 0.5f, def, 1);

	this->key = 0;
	this->pos = *pos;
	this->rbase = rbase;
	this->rhaut = rhaut;
	this->hauteur = hauteur;
	this->currdurationang = 0;
	this->scale = 0.f;
	this->ang = 0.f;
	this->def = (short)def;
	this->tsouffle = TextureContainer::Load("graph/obj3d/textures/(fx)_sebsouffle");

	this->timestone = 0;
	this->nbstone = 0;


	this->stone[0] = stone0;
	this->stone[1] = stone1;

	int nb = 256;

	while(nb--) {
		this->tstone[nb].actif = 0;
	}
}

void CLevitate::AddStone(Vec3f * pos) {
	
	if(arxtime.is_paused() || nbstone > 255) {
		return;
	}
	
	int nb = 256;
	while(nb--) {
		if(!tstone[nb].actif) {
			nbstone++;
			tstone[nb].actif = 1;
			tstone[nb].numstone = rand() & 1;
			tstone[nb].pos = *pos;
			tstone[nb].yvel = rnd() * -5.f;
			tstone[nb].ang = Anglef(rnd() * 360.f, rnd() * 360.f, rnd() * 360.f);
			tstone[nb].angvel = Anglef(5.f * rnd(), 6.f * rnd(), 3.f * rnd());
			tstone[nb].scale = Vec3f(0.2f + rnd() * 0.3f);
			tstone[nb].time = Random::get(2000, 2500);
			tstone[nb].currtime = 0;
			break;
		}
	}
}

void CLevitate::DrawStone()
{
	GRenderer->SetBlendFunc(Renderer::BlendInvDstColor, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	int	nb = 256;

	while(nb--) {
		if(this->tstone[nb].actif) {
			float a = (float)this->tstone[nb].currtime / (float)this->tstone[nb].time;

			if(a > 1.f) {
				a = 1.f;
				this->tstone[nb].actif = 0;
			}

			int col = Color4f(Color3f::white, 1.f - a).toBGRA();

			if(this->stone[this->tstone[nb].numstone])
				DrawEERIEObjExEx(this->stone[this->tstone[nb].numstone], &this->tstone[nb].ang, &this->tstone[nb].pos, &this->tstone[nb].scale, col);
			
			PARTICLE_DEF * pd = createParticle();
			if(pd) {
				pd->ov = tstone[nb].pos;
				pd->move = Vec3f(0.f, 3.f * rnd(), 0.f);
				pd->siz = 3.f + 3.f * rnd();
				pd->tolive = 1000;
				pd->timcreation = -(long(arxtime) + 1000l); // TODO WTF?
				pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			
			//update mvt
			if(!arxtime.is_paused()) {
				a = (((float)this->currframetime) * 100.f) / (float)this->tstone[nb].time;
				tstone[nb].pos.y += tstone[nb].yvel * a;
				tstone[nb].ang += tstone[nb].angvel * a;

				this->tstone[nb].yvel *= 1.f - (1.f / 100.f);

				this->tstone[nb].currtime += this->currframetime;
			}
		}
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}

void CLevitate::Update(unsigned long _ulTime)
{
	float	a;

	//animation cone
	if(!arxtime.is_paused())
		this->currdurationang += _ulTime;

	this->ang = (float)this->currdurationang / 1000.f;

	if(this->ang > 1.f) {
		this->currdurationang = 0;
		this->ang = 1.f;
	}

	if (!arxtime.is_paused()) ulCurrentTime += _ulTime;

	switch(this->key) {
		case 0:
			//monté du cone
			a = (float) ulCurrentTime / 1000.f;

			if(a > 1.f) {
				a = 0.f;
				this->key++;
			}

			this->scale = a;
			break;
		case 1:
			//animation cone
			this->scale = (float)ulCurrentTime / (float)ulDuration;

			if(ulCurrentTime >= ulDuration) {
				this->scale = 1.f;
				this->key++;
			}

			break;
	}

	if(!arxtime.is_paused()) {
		this->currframetime = _ulTime;
		this->timestone -= _ulTime;
	}

	if(this->timestone <= 0) {
		this->timestone = Random::get(50, 150);
		Vec3f	pos;

		float r = this->rbase * frand2();
		pos.x = this->pos.x + r; 
		pos.y = this->pos.y;
		pos.z = this->pos.z + r; 
		this->AddStone(&pos);
	}
}

void CLevitate::Render()
{
	if(this->key > 1)
		return;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);

	//calcul du cone
	TexturedVertex d3dvs, *d3dv;
	Vec3f	* vertex;
	int			nb, nbc, col;
	float		ddu = this->ang;
	float		u = ddu, du = .99999999f / (float)this->def;

	switch(this->key) {
		case 0:
			nbc = 2;

			while(nbc--) {
				vertex = this->cone[nbc].conevertex;
				d3dv = this->cone[nbc].coned3d;
				nb = (this->cone[nbc].conenbvertex) >> 1;

				while(nb) {
					d3dvs.p.x = this->pos.x + (vertex + 1)->x + ((vertex->x - (vertex + 1)->x) * this->scale);
					d3dvs.p.y = this->pos.y + (vertex + 1)->y + ((vertex->y - (vertex + 1)->y) * this->scale);
					d3dvs.p.z = this->pos.z + (vertex + 1)->z + ((vertex->z - (vertex + 1)->z) * this->scale);
					
					EE_RT2(&d3dvs, d3dv);

					float fRandom	= rnd() * 80.f;

					col = checked_range_cast<int>(fRandom);

					if(!arxtime.is_paused())
						d3dv->color = Color::grayb(col).toBGR(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 0.f;
					vertex++;
					d3dv++;

					d3dvs.p.x = this->pos.x + vertex->x;
					d3dvs.p.y = this->pos.y;
					d3dvs.p.z = this->pos.z + vertex->z;
					
					EE_RT2(&d3dvs, d3dv);

					fRandom = rnd() * 80.f;

					col = checked_range_cast<int>(fRandom);

					if(!arxtime.is_paused())
						d3dv->color = Color::black.toBGR(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 0.9999999f;
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}

			nbc = 3;
			while(nbc--) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					break;
				}
				
				float a = radians(360.f * rnd());
				pd->ov = pos + Vec3f(rbase * EEcos(a), 0.f, rbase * EEsin(a));
				float t = fdist(pd->ov, pos);
				pd->move = Vec3f((5.f + 5.f * rnd()) * ((pd->ov.x - pos.x) / t), 3.f * rnd(),
				                 (5.f + 5.f * rnd()) * ((pd->ov.z - pos.z) / t));
				pd->siz = 30.f + 30.f * rnd();
				pd->tolive = 3000;
				pd->timcreation = -(long(arxtime) + 3000l); // TODO WTF
				pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			break;
		
		case 1:
			nbc = 2;

			while(nbc--) {
				vertex = this->cone[nbc].conevertex;
				d3dv = this->cone[nbc].coned3d;
				nb = (this->cone[nbc].conenbvertex) >> 1;

				while(nb) {
					d3dvs.p = this->pos + *vertex;
	
					EE_RT2(&d3dvs, d3dv);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::grayb(col).toBGR(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 0.f;
					vertex++;
					d3dv++;

					d3dvs.p.x = this->pos.x + vertex->x;
					d3dvs.p.y = this->pos.y;
					d3dvs.p.z = this->pos.z + vertex->z;

					EE_RT2(&d3dvs, d3dv);
					col = Random::get(0, 80);

					if(!arxtime.is_paused())
						d3dv->color = Color::black.toBGR(col);

					d3dv->uv.x = u;
					d3dv->uv.y = 1; 
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}
			
			nbc = 10;
			while(nbc--) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					break;
				}
				
				float a = radians(360.f * rnd());
				pd->ov = pos + Vec3f(rbase * EEcos(a), 0.f, rbase * EEsin(a));
				float t = fdist(pd->ov, pos);
				pd->move = Vec3f((5.f + 5.f * rnd()) * ((pd->ov.x - pos.x) / t), 3.f * rnd(),
				                 (5.f + 5.f * rnd()) * ((pd->ov.z - pos.z) / t));
				pd->siz = 30.f + 30.f * rnd();
				pd->tolive = 3000;
				pd->timcreation = -(long(arxtime) + 3000l); // TODO WTF
				pd->special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
			}
			
			break;
	}

	//tracé du cone back
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapMirror);

	GRenderer->SetTexture(0, tsouffle);

	GRenderer->SetCulling(Renderer::CullCW);
	int i = cone[1].conenbfaces - 2;
	int j = 0;

	while(i--) {
		ARX_DrawPrimitive(&cone[1].coned3d[j], &cone[1].coned3d[j+1], &cone[1].coned3d[j+2]);
		j++;
	}

	i = cone[0].conenbfaces - 2;
	j = 0;

	while(i--) {
		ARX_DrawPrimitive(&cone[0].coned3d[j], &cone[0].coned3d[j+1], &cone[0].coned3d[j+2]);
		j++;
	}

	//tracé du cone front
	GRenderer->SetCulling(Renderer::CullCCW);
	
	i = cone[1].conenbfaces - 2;
	j = 0;

	while(i--) {
		ARX_DrawPrimitive(&cone[1].coned3d[j], &cone[1].coned3d[j+1], &cone[1].coned3d[j+2]);
		j++;
	}

	i = cone[0].conenbfaces - 2;
	j = 0;

	while(i--) {
		ARX_DrawPrimitive(&cone[0].coned3d[j], &cone[0].coned3d[j+1], &cone[0].coned3d[j+2]);
		j++;
	}

	//tracé des pierres
	GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendInvSrcAlpha);
	this->DrawStone();

	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	GRenderer->SetCulling(Renderer::CullNone);
}
