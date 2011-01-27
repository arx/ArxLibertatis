/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// CSpellFx_Lvl05.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 05
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEDraw.h>
#include <EERIEMath.h>

#include "ARX_CSpellFx.h"
#include <ARX_Damages.h>
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_SpellFx_Lvl03.h"

#include "ARX_Particles.h"
#include "ARX_CParticle.h"
#include "ARX_Spells.h"
#include "ARX_CParticles.h"
#include "ARX_CParticleParams.h"
#include "ARX_Fogs.h"
#include "ARX_Time.h"


#include "EERIELight.h"
#include "EERIEObject.h"

#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

extern CParticleManager * pParticleManager;


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
EERIE_3DOBJ * sfirewave = NULL;
long sfirewave_count = 0;
EERIE_3DOBJ * svoodoo = NULL;
long svoodoo_count = 0;
//-----------------------------------------------------------------------------
CCurePoison::CCurePoison(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	pPS = new CParticleSystem();
}

//-----------------------------------------------------------------------------
CCurePoison::~CCurePoison()
{

}

//-----------------------------------------------------------------------------
void CCurePoison::Create()
{
	SetAngle(0);

	eSrc.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eSrc.y = inter.iobj[spells[spellinstance].target]->pos.y;

	if (spells[spellinstance].target == 0)
		eSrc.y += 200;

	eSrc.z = inter.iobj[spells[spellinstance].target]->pos.z;

	pPS->SetPos(eSrc);
	CParticleParams cp;
	cp.iNbMax = 350;
	cp.fLife = 800;
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 0;
	cp.p3Pos.z = 100;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(5);
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
	cp.bTexInfo = FALSE;
	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph\\particles\\cure_poison.bmp", 0, 100); //5

	pPS->lLightId = GetFreeDynLight();

	if (pPS->lLightId != -1)
	{
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
		DynLight[id].time_creation = ARXTimeUL();
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	fSize = 1;
}

//---------------------------------------------------------------------
void CCurePoison::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if (ulCurrentTime >= ulDuration)
	{
		return;
	}

	eSrc.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eSrc.y = inter.iobj[spells[spellinstance].target]->pos.y;

	if (spells[spellinstance].target == 0)
		eSrc.y += 200;

	eSrc.z = inter.iobj[spells[spellinstance].target]->pos.z;


	unsigned long ulCalc = ulDuration - ulCurrentTime ;
	ARX_CHECK_LONG(ulCalc);
	long ff = 	ARX_CLEAN_WARN_CAST_LONG(ulCalc);



	if (ff < 1500)
	{
		pPS->uMaxParticles = 0;
		pPS->ulParticleSpawn = PARTICLE_CIRCULAR;
		pPS->p3ParticleGravity.x = 0;
		pPS->p3ParticleGravity.y = 0;
		pPS->p3ParticleGravity.z = 0;

		list<CParticle *>::iterator i;

		for (i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i)	
		{
			CParticle * pP = *i;

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

	pPS->SetPos(eSrc);
	pPS->Update(aulTime);

	if (pPS->lLightId == -1) pPS->lLightId = GetFreeDynLight();

	if (pPS->lLightId != -1)
	{
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
		DynLight[id].time_creation = ARXTimeUL();
		DynLight[id].extras = 0;
	}

}

//---------------------------------------------------------------------
float CCurePoison::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	pPS->Render(m_pd3dDevice, D3DBLEND_ONE, D3DBLEND_ONE);

	return 1;
}

//-----------------------------------------------------------------------------
CRuneOfGuarding::CRuneOfGuarding(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");

	if (!ssol)
	{
		ssol   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(ssol);
	}

	ssol_count++;

	if (!slight)
	{
		slight = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard02.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(slight);
	}

	slight_count++;

	if (!srune)
	{
		srune  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard03.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(srune);
	}

	srune_count++;
}

CRuneOfGuarding::~CRuneOfGuarding()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		ReleaseEERIE3DObj(ssol);
		ssol = NULL;
	}

	slight_count--;

	if (slight && (slight_count <= 0))
	{
		slight_count = 0;
		ReleaseEERIE3DObj(slight);
		slight = NULL;
	}

	srune_count--;

	if (srune && (srune_count <= 0))
	{
		srune_count = 0;
		ReleaseEERIE3DObj(srune);
		srune = NULL;
	}
}
//-----------------------------------------------------------------------------
void CRuneOfGuarding::Create(EERIE_3D _eSrc, float _fBeta)
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

	bDone = true;

	lLightId = GetFreeDynLight();

	if (lLightId != -1)
	{
		long id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 0.7f + 2.3f;
		DynLight[id].fallend = 500.f;
		DynLight[id].fallstart = 400.f;
		DynLight[id].rgb.r = 1.0f;
		DynLight[id].rgb.g = 0.2f;
		DynLight[id].rgb.b = 0.2f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].time_creation = ARXTimeUL();
		DynLight[id].duration = 200;
	}
}

//---------------------------------------------------------------------
void CRuneOfGuarding::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	float fa = 1.0f - rnd() * 0.15f;

	if (lLightId != -1)
	{
		long id = lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 0.7f + 2.3f * fa;
		DynLight[id].fallend = 350.f;
		DynLight[id].fallstart = 150.f;
		DynLight[id].rgb.r = 1.0f;
		DynLight[id].rgb.g = 0.2f;
		DynLight[id].rgb.b = 0.2f;
		DynLight[id].time_creation = ARXTimeUL();
		DynLight[id].duration = 200;
	}
}

//---------------------------------------------------------------------
float CRuneOfGuarding::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
 

	float x = eSrc.x;
	float y = eSrc.y - 20;
	float z = eSrc.z;

	SETZWRITE(m_pd3dDevice, FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//----------------------------
	//	long color = D3DRGB(1,1,1);
	//	int size = 100;
	//----------------------------
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	float stiteangleb = (float) ulCurrentTime * fOneOnDuration * 120;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;

	float gtc = (float)ARX_TIME_Get();
	float v = EEsin(gtc * DIV1000) * DIV10;
	stiteangle.b = MAKEANGLE(gtc * DIV1000); 
	stitecolor.r = 0.4f - v;
	stitecolor.g = 0.4f - v;
	stitecolor.b = 0.6f - v;
	stitescale.x = 1;
	stitescale.y = -0.1f;
	stitescale.z = 1;

	if (slight)
		DrawEERIEObjEx(m_pd3dDevice, slight, &stiteangle, &stitepos, &stitescale, &stitecolor);

	stiteangle.b = stiteangleb;
	stitecolor.r = 0.6f;
	stitecolor.g = 0.f;
	stitecolor.b = 0.f;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;

	if (ssol)
		DrawEERIEObjEx(m_pd3dDevice, ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

	stitecolor.r = 0.6f;
	stitecolor.g = 0.3f;
	stitecolor.b = 0.45f;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;


	if (srune)
		DrawEERIEObjEx(m_pd3dDevice, srune, &stiteangle, &stitepos, &stitescale, &stitecolor);


	for (int n = 0; n < 4; n++)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist		=	1;
			particle[j].zdec		=	0;
 
			particle[j].ov.x		=	x + frand2() * 40; 
			particle[j].ov.y		=	y;
			particle[j].ov.z		=	z + frand2() * 40;
			particle[j].move.x		=	0.8f * frand2(); 
			particle[j].move.y		=	-4.f * rnd(); 
			particle[j].move.z		=	0.8f * frand2(); 
			particle[j].scale.x		=	-0.1f;
			particle[j].scale.y		=	-0.1f;
			particle[j].scale.z		=	-0.1f;
			particle[j].timcreation = lARXTime;
			particle[j].tolive		=	2600 + (unsigned long)(rnd() * 600.f);
			particle[j].tc			=	tex_p2;
			particle[j].siz			=	0.3f;
			particle[j].r			=	0.4f;
			particle[j].g			=	0.4f;
			particle[j].b			=	0.6f;
		}
	}

	return 1.0f - rnd() * 0.3f;
}

//-----------------------------------------------------------------------------
void LaunchPoisonExplosion(EERIE_3D * aePos)
{
	// système de partoches pour l'explosion
	CParticleSystem * pPS = new CParticleSystem();
	CParticleParams cp;
	cp.iNbMax = 80; 
	cp.fLife = 1500;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 5;
	cp.p3Pos.y = 5;
	cp.p3Pos.z = 5;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = 4;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(360);
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
	cp.bTexInfo = FALSE;
	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;
	pPS->SetTexture("graph\\particles\\big_greypouf.bmp", 0, 200);

	pPS->SetPos(*aePos);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	list<CParticle *>::iterator i;

	for (i = pPS->listParticle.begin(); i != pPS->listParticle.end(); ++i)
	{
		CParticle * pP = *i;

		if (pP->isAlive())
		{
			if (pP->p3Velocity.y >= 0.5f * 200)
				pP->p3Velocity.y = 0.5f * 200;

			if (pP->p3Velocity.y <= -0.5f * 200)
				pP->p3Velocity.y = -0.5f * 200;
		}
	}

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	}
}


//-----------------------------------------------------------------------------
CPoisonProjectile::CPoisonProjectile(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	fColor[0] = 1;
	fColor[1] = 1;
	fColor[2] = 0;

	fColor1[0] = 0.8f;
	fColor1[1] = 0.6f;
	fColor1[2] = 0.2f;
}

//-----------------------------------------------------------------------------
void CPoisonProjectile::Create(EERIE_3D _eSrc, float _fBeta)
{
	int i;

	SetDuration(ulDuration);

	SetAngle(_fBeta);

	eSrc.x = _eSrc.x;
	eSrc.y = _eSrc.y;
	eSrc.z = _eSrc.z;

	fSize = 1;
	bDone = true;
	bOk = false;

	eTarget.x = eSrc.x - fBetaRadSin * 850;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z + fBetaRadCos * 850;

	eMove.x = - fBetaRadSin * 2; 
	eMove.y = 0;
	eMove.z = + fBetaRadCos * 2; 

	EERIE_3D s, e, h;
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

	e.y += 0.f;

	pathways[0].sx = eSrc.x;
	pathways[0].sy = eSrc.y;
	pathways[0].sz = eSrc.z;
	pathways[9].sx = e.x;
	pathways[9].sy = e.y;
	pathways[9].sz = e.z;
	Split(pathways, 0, 9, 10 * fBetaRadCos, 10, 10, 10, 10 * fBetaRadSin, 10);

	if (0)
		for (i = 0; i < 10; i++)
		{
			if (pathways[i].sy >= eSrc.y + 150)
			{
				pathways[i].sy = eSrc.y + 150;
			}

			if (pathways[i].sy <= eSrc.y + 50)
			{
				pathways[i].sy = eSrc.y + 50;
			}
		}

	fTrail = -1;

	//-------------------------------------------------------------------------
	// système de partoches
	CParticleParams cp;
	cp.iNbMax = 5;
	cp.fLife = 2000;
	cp.fLifeRandom = 1000;
	cp.p3Pos.x = 0;
	cp.p3Pos.y = 0;
	cp.p3Pos.z = 0;
	cp.p3Direction.x = -eMove.x;
	cp.p3Direction.y = -eMove.y;
	cp.p3Direction.z = -eMove.z;
	cp.fAngle = 0;
	cp.fSpeed = 10;
	cp.fSpeedRandom = 10;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
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

	pPS.SetTexture("graph\\particles\\big_greypouf.bmp", 0, 200);

	pPS.fParticleFreq = -1;

	pPS.bParticleFollow = true;

	pPS.SetPos(eSrc);
	pPS.Update(0);
}

//---------------------------------------------------------------------
void CPoisonProjectile::SetColor(float _fRed, float _fGreen, float _fBlue)
{
	fColor[0] = _fRed;
	fColor[1] = _fGreen;
	fColor[2] = _fBlue;
}

//---------------------------------------------------------------------
void CPoisonProjectile::SetColor1(float _fRed, float _fGreen, float _fBlue)
{
	fColor1[0] = _fRed;
	fColor1[1] = _fGreen;
	fColor1[2] = _fBlue;
}

//---------------------------------------------------------------------
void CPoisonProjectile::Update(unsigned long _ulTime)
{
	if (ulCurrentTime <= 2000)
	{
		ulCurrentTime += _ulTime;
	}

	// on passe de 5 à 100 partoches en 1.5secs
	if (ulCurrentTime < 750)
	{
		pPS.iParticleNbMax = 2;
		pPS.Update(_ulTime);
	}
	else
	{
		if (!bOk)
		{
			bOk = !bOk;

			// go
			CParticleParams cp;
			cp.iNbMax = 100;
			cp.fLife = 500;
			cp.fLifeRandom = 300;
			cp.p3Pos.x = fBetaRadSin * 20;
			cp.p3Pos.y = 0.f;
			cp.p3Pos.z = fBetaRadCos * 20;

			cp.p3Direction.x = -eMove.x;
			cp.p3Direction.y = -eMove.y;
			cp.p3Direction.z = -eMove.z;

			cp.fAngle = DEG2RAD(4);
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
			cp.bTexInfo = FALSE;
			pPSStream.SetParams(cp);
			pPSStream.ulParticleSpawn = 0;

			pPSStream.SetTexture("graph\\particles\\big_greypouf.bmp", 0, 200);

			pPSStream.fParticleFreq = 80;
			pPSStream.bParticleFollow = true;
		}

		pPSStream.Update(_ulTime);
		pPSStream.SetPos(eCurPos);

		pPS.Update(_ulTime);
		pPS.SetPos(eCurPos);

		fTrail = ((ulCurrentTime - 750) * (1.0f / (ulDuration - 750.0f))) * 9 * (BEZIERPrecision + 2);
	}
}

//---------------------------------------------------------------------
float CPoisonProjectile::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	//-------------------------------------------------------------------------
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	// ------------------------------------------------------------------------
	int n = BEZIERPrecision;
	float delta = 1.0f / n;

	EERIE_3D lastpos, newpos;
	EERIE_3D v;

	lastpos.x = pathways[0].sx;
	lastpos.y = pathways[0].sy;
	lastpos.z = pathways[0].sz;

	int arx_check_init = -1;
	newpos.x = 0;
	newpos.y = 0;
	newpos.z = 0;


	for (i = 0; i < 9; i++)
	{
		int kp = i;
		int kpprec = (i > 0) ? kp - 1 : kp ;
		int kpsuiv = kp + 1 ;
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

			float val = pathways[kpsuiv].sx;
			float p0 = 0.5f * (val - pathways[kpprec].sx) ;
			float p1 = 0.5f * (pathways[kpsuivsuiv].sx - pathways[kp].sx) ;
			v.x = f0 * pathways[kp].sx + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].sy ;
			p0 = 0.5f * (val - pathways[kpprec].sy);
			p1 = 0.5f * (pathways[kpsuivsuiv].sy - pathways[kp].sy) ;
			v.y = f0 * pathways[kp].sy + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].sz ;
			p0 = 0.5f * (val - pathways[kpprec].sz) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].sz - pathways[kp].sz) ;
			v.z = f0 * pathways[kp].sz + f1 * val + f2 * p0 + f3 * p1 ;

			newpos.x = v.x;
			newpos.y = v.y;
			newpos.z = v.z;

			if (!((fTrail - (i * n + toto)) > 70))
			{
				float c = 1.0f - ((fTrail - (i * n + toto)) / 70.0f);
				c += frand2() * 0.1f;

				if (c < 0) c = 0;

				if (c > 1) c = 1;
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
		}
	}

	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	EERIE_3D av;
	ARX_CHECK_NOT_NEG(arx_check_init);
	av.x = newpos.x - lastpos.x;
	av.y = newpos.y - lastpos.y;
	av.z = newpos.z - lastpos.z;

	TRUEVector_Normalize(&av);

	float bubu = GetAngle(av.x, av.z, 0, 0);
	float bubu1 = GetAngle(av.x, av.y, 0, 0);

	stitepos.x = lastpos.x;
	stitepos.y = lastpos.y;
	stitepos.z = lastpos.z;
	stiteangle.b = -RAD2DEG(bubu);
	stiteangle.a = 0;
	stiteangle.g = -90 - RAD2DEG(bubu1);
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;

	eCurPos.x = lastpos.x;
	eCurPos.y = lastpos.y;
	eCurPos.z = lastpos.z;

	if (fTrail >= (i * n))
	{
		LaunchPoisonExplosion(&lastpos);
	}

	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiPoisonProjectile::CMultiPoisonProjectile(LPDIRECT3DDEVICE7 m_pd3dDevice, long nbmissiles)
{
	SetDuration(2000);
	uiNumber = __min(5, nbmissiles);
	pTab	 = NULL;
	pTab	 = new CSpellFx*[uiNumber]();

	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		pTab[i] = new CPoisonProjectile(m_pd3dDevice);
		pTab[i]->spellinstance = this->spellinstance;
	}
}

//-----------------------------------------------------------------------------
CMultiPoisonProjectile::~CMultiPoisonProjectile()
{
	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		if (pTab[i]->lLightId != -1)
		{
			DynLight[pTab[i]->lLightId].duration	  = 2000;
			DynLight[pTab[i]->lLightId].time_creation = ARXTimeUL();
			pTab[i]->lLightId						  = -1;
		}

		delete pTab[i];
	}

	delete [] pTab;
}

//-----------------------------------------------------------------------------
void CMultiPoisonProjectile::Create(EERIE_3D _eSrc, float _afBeta = 0)
{

	float afAlpha = 0.f;
	float afBeta = 0.f;

	spells[spellinstance].hand_group = inter.iobj[spells[spellinstance].caster]->obj->fastaccess.primary_attach;

	if (spells[spellinstance].hand_group != -1)
	{
		spells[spellinstance].hand_pos.x = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.x;
		spells[spellinstance].hand_pos.y = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.y;
		spells[spellinstance].hand_pos.z = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[spells[spellinstance].hand_group].v.z;
	}

	if (spells[spellinstance].caster == 0) // player
	{
		afBeta = player.angle.b;
		afAlpha = player.angle.a;

		if (spells[spellinstance].hand_group != -1)
		{
			_eSrc.x = spells[spellinstance].hand_pos.x - EEsin(DEG2RAD(afBeta)) * 90;
			_eSrc.y = spells[spellinstance].hand_pos.y;
			_eSrc.z = spells[spellinstance].hand_pos.z + EEcos(DEG2RAD(afBeta)) * 90;
		}
		else
		{
			_eSrc.x = player.pos.x - EEsin(DEG2RAD(afBeta)) * 90;
			_eSrc.y = player.pos.y;
			_eSrc.z = player.pos.z + EEcos(DEG2RAD(afBeta)) * 90;
		}
	}
	else
	{
		afBeta = inter.iobj[spells[spellinstance].caster]->angle.b;

		if (spells[spellinstance].hand_group != -1)
		{
			_eSrc.x = spells[spellinstance].hand_pos.x - EEsin(DEG2RAD(afBeta)) * 90;
			_eSrc.y = spells[spellinstance].hand_pos.y;
			_eSrc.z = spells[spellinstance].hand_pos.z + EEcos(DEG2RAD(afBeta)) * 90;
		}
		else
		{
			_eSrc.x = inter.iobj[spells[spellinstance].caster]->pos.x - EEsin(DEG2RAD(afBeta)) * 90;
			_eSrc.y = inter.iobj[spells[spellinstance].caster]->pos.y;
			_eSrc.z = inter.iobj[spells[spellinstance].caster]->pos.z + EEcos(DEG2RAD(afBeta)) * 90;
		}
	}


	long lMax = 0;

	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		pTab[i]->Create(_eSrc, afBeta + frand2() * 10.0f);
		long lTime = ulDuration + (long) rnd() * 5000;
		pTab[i]->SetDuration(lTime);
		lMax = max(lMax, lTime);

		CPoisonProjectile * pPP = (CPoisonProjectile *) pTab[i];

		pPP->SetColor(0.5f + rnd() * 0.5f, 0.5f + rnd() * 0.5f, 0.5f + rnd() * 0.5f);
		pPP->SetColor1(0.5f + rnd() * 0.5f, 0.5f + rnd() * 0.5f, 0.5f + rnd() * 0.5f);

		pPP->SetColor(0, 1, 0);
		pPP->SetColor1(0.3f, 0.8f, 0);
		pPP->lLightId = GetFreeDynLight();

		if (pPP->lLightId != -1)
		{
			long id						= pPP->lLightId;
			DynLight[id].exist			= 1;
			DynLight[id].intensity		= 2.3f;
			DynLight[id].fallend		= 250.f;
			DynLight[id].fallstart		= 150.f;
			DynLight[id].rgb.r			= 0;
			DynLight[id].rgb.g			= 1;
			DynLight[id].rgb.b			= 0;
			DynLight[id].pos.x			= pPP->eSrc.x;
			DynLight[id].pos.y			= pPP->eSrc.y;
			DynLight[id].pos.z			= pPP->eSrc.z;
			DynLight[id].time_creation	= ARXTimeUL();
			DynLight[id].duration		= 200;
		}

		pTab[i]->spellinstance = this->spellinstance;

	}

	SetDuration(lMax + 1000);
}

//-----------------------------------------------------------------------------
void CMultiPoisonProjectile::Update(unsigned long _ulTime)
{
	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		pTab[i]->Update(_ulTime);
	}
}

//-----------------------------------------------------------------------------
float CMultiPoisonProjectile::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
 

	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		float fa = pTab[i]->Render(m_pd3dDevice);

		CPoisonProjectile * pPoisonProjectile = (CPoisonProjectile *) pTab[i];

		if (pPoisonProjectile->lLightId != -1)
		{
			long id					= pPoisonProjectile->lLightId;
			DynLight[id].exist		= 1;
			DynLight[id].intensity	= 2.3f * fa;
			DynLight[id].fallend	= 250.f;
			DynLight[id].fallstart	= 150.f;
			DynLight[id].rgb.r		= 0.f;
			DynLight[id].rgb.g		= 1.0f;
			DynLight[id].rgb.b		= 0.f;
			DynLight[id].pos.x		= pPoisonProjectile->eCurPos.x;
			DynLight[id].pos.y		= pPoisonProjectile->eCurPos.y;
			DynLight[id].pos.z		= pPoisonProjectile->eCurPos.z;
			DynLight[id].time_creation = ARXTimeUL();
			DynLight[id].duration	= 200;
		}

		long t = ARX_DAMAGES_GetFree();
		AddPoisonFog(&pPoisonProjectile->eCurPos, spells[spellinstance].caster_level + 7);

		if ((t != -1)
		        &&	(spells[pTab[i]->spellinstance].timcreation + 1600 < ARXTimeUL()))
		{
			damages[t].pos.x  = pPoisonProjectile->eCurPos.x;
			damages[t].pos.y  = pPoisonProjectile->eCurPos.y;
			damages[t].pos.z  = pPoisonProjectile->eCurPos.z;
			damages[t].radius = 120.f;
			float v = spells[spellinstance].caster_level;
			v = 4.f + v * DIV10 * 6.f ;
			damages[t].damages	= v * DIV1000 * _framedelay;
			damages[t].area		= DAMAGE_FULL;
			damages[t].duration	= ARX_CLEAN_WARN_CAST_LONG(FrameDiff);
			damages[t].source	= spells[spellinstance].caster;
			damages[t].flags	= 0;
			damages[t].type		= DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_POISON;
			damages[t].exist	= TRUE;
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
CRepelUndead::CRepelUndead(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	ulCurrentTime = ulDuration + 1;

	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");

	if (!ssol) // Pentacle
	{
		ssol   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(ssol);
	}

	ssol_count++;

	if (!slight) // Twirl
	{
		slight = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard02.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(slight);
	}

	slight_count++; //runes

	if (!srune)
	{
		srune  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_rune_guard\\fx_rune_guard03.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(srune);
	}

	srune_count++;
}
CRepelUndead::~CRepelUndead()
{
	ssol_count--;

	if (ssol && (ssol_count <= 0))
	{
		ssol_count = 0;
		ReleaseEERIE3DObj(ssol);
		ssol = NULL;
	}

	slight_count--;

	if (slight && (slight_count <= 0))
	{
		slight_count = 0;
		ReleaseEERIE3DObj(slight);
		slight = NULL;
	}

	srune_count--;

	if (srune && (srune_count <= 0))
	{
		srune_count = 0;
		ReleaseEERIE3DObj(srune);
		srune = NULL;
	}
}
//-----------------------------------------------------------------------------
void CRepelUndead::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eTarget.x = eSrc.x = aeSrc.x;
	eTarget.y = eSrc.y = aeSrc.y;
	eTarget.z = eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CRepelUndead::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	if (spellinstance < 0) return;

	eSrc.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eSrc.y = inter.iobj[spells[spellinstance].target]->pos.y;
	eSrc.z = inter.iobj[spells[spellinstance].target]->pos.z;

	if (spells[spellinstance].target == 0)
		fBeta = player.angle.b;
	else
		fBeta = inter.iobj[spells[spellinstance].target]->angle.b;
}

//---------------------------------------------------------------------
float CRepelUndead::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
 


	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETZWRITE(m_pd3dDevice, FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//----------------------------
	EERIE_3D  eObjAngle;
	EERIE_3D  eObjPos;
	EERIE_3D  eObjScale;
	EERIE_RGB rgbObjColor;

	eObjAngle.b = fBeta;
	eObjAngle.a = 0;
	eObjAngle.g = 0;
	eObjPos.x = eSrc.x;
	eObjPos.y = eSrc.y - 5.f;
	eObjPos.z = eSrc.z;
	rgbObjColor.r = 0.6f;
	rgbObjColor.g = 0.6f;
	rgbObjColor.b = 0.8f;

	float vv = 1.f + (EEsin(ARX_TIME_Get() * DIV1000)); 
	vv *= DIV2;
	vv += 1.1f;
	eObjScale.z = vv;
	eObjScale.y = vv;
	eObjScale.x = vv;

	if (ssol)
		DrawEERIEObjEx(m_pd3dDevice, ssol, &eObjAngle, &eObjPos, &eObjScale, &rgbObjColor);

	vv *= 100.f;

	for (int n = 0; n < 4; n++)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist		=	1;
			particle[j].zdec		=	0;
 
			particle[j].ov.x		=	eSrc.x - EEsin(frand2() * 360.f) * vv; 
			particle[j].ov.y		=	eSrc.y;
			particle[j].ov.z		=	eSrc.z + EEcos(frand2() * 360.f) * vv; 
			particle[j].move.x		=	0.8f * frand2(); 
			particle[j].move.y		=	-4.f * rnd(); 
			particle[j].move.z		=	0.8f * frand2(); 
			particle[j].scale.x		=	-0.1f;
			particle[j].scale.y		=	-0.1f;
			particle[j].scale.z		=	-0.1f;
			particle[j].timcreation =	lARXTime;
			particle[j].tolive		=	2600 + (unsigned long)(rnd() * 600.f);
			particle[j].tc			=	tex_p2;
			particle[j].siz			=	0.3f;
			particle[j].r			=	0.4f;
			particle[j].g			=	0.4f;
			particle[j].b			=	0.6f;
		}
	}

	if (this->lLightId == -1)
		this->lLightId = GetFreeDynLight();

	if (this->lLightId != -1)
	{
		long id = this->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallend = 350.f;
		DynLight[id].fallstart = 150.f;
		DynLight[id].rgb.r = 0.8f;
		DynLight[id].rgb.g = 0.8f;
		DynLight[id].rgb.b = 1;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].time_creation = ARXTimeUL();
	}

	return 1;
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

	while (nb--)
	{
		this->cone[nb].coned3d = NULL;
		this->cone[nb].coneind = NULL;
		this->cone[nb].conevertex = NULL;
	}

	if (stone0 == NULL)
	{
		stone0 = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_raise_dead\\stone01.teo", NULL);
	}

	stone0_count++;

	if (stone1 == NULL)
	{
		stone1 = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_raise_dead\\stone02.teo", NULL);
	}

	stone1_count++;
}
//-----------------------------------------------------------------------------
CLevitate::~CLevitate()
{
	stone0_count--;

	if (stone0 && (stone0_count <= 0))
	{
		stone0_count = 0;
		ReleaseEERIE3DObj(stone0);
		stone0 = NULL;
	}

	stone1_count--;

	if (stone1 && (stone1_count <= 0))
	{
		stone1_count = 0;
		ReleaseEERIE3DObj(stone1);
		stone1 = NULL;
	}
}
//-----------------------------------------------------------------------------
void CLevitate::CreateConeStrip(float rbase, float rhaut, float hauteur, int def, int numcone)
{
	if (this->cone[numcone].coned3d) free((void *)this->cone[numcone].coned3d);

	if (this->cone[numcone].conevertex) free((void *)this->cone[numcone].conevertex);

	if (this->cone[numcone].coneind) free((void *)this->cone[numcone].coneind);

	this->cone[numcone].conenbvertex = def * 2 + 2;
	this->cone[numcone].conenbfaces = def * 2 + 2;
	this->cone[numcone].coned3d = (D3DTLVERTEX *)malloc(this->cone[numcone].conenbvertex * sizeof(D3DTLVERTEX));
	this->cone[numcone].conevertex = (EERIE_3D *)malloc(this->cone[numcone].conenbvertex * sizeof(EERIE_3D));
	this->cone[numcone].coneind = (unsigned short *)malloc(this->cone[numcone].conenbvertex * sizeof(unsigned short));

	float			a, da;
	EERIE_3D	*	vertex = this->cone[numcone].conevertex;
	unsigned short	* pind = this->cone[numcone].coneind, ind = 0;
	int				nb;
	a = 0.f;
	da = 360.f / (float)def;
	nb = this->cone[numcone].conenbvertex >> 1;

	while (nb)
	{
		*pind++ = ind++;
		*pind++ = ind++;

		vertex->x = rhaut * EEcos(DEG2RAD(a));
		vertex->y = -hauteur;
		vertex->z = rhaut * EEsin(DEG2RAD(a));
		vertex++;
		vertex->x = rbase * EEcos(DEG2RAD(a));
		vertex->y = 0.f;
		vertex->z = rbase * EEsin(DEG2RAD(a));
		vertex++;

		a += da;
		nb--;
	}
}
/*--------------------------------------------------------------------------*/
void CLevitate::Create(int def, float rbase, float rhaut, float hauteur, EERIE_3D * pos, unsigned long _ulDuration)
{
	SetDuration(_ulDuration);

	if (def < 3) return;

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
	this->tsouffle = MakeTCFromFile("Graph\\Obj3D\\Textures\\(FX)_sebsouffle.bmp");

	this->timestone = 0;
	this->nbstone = 0;


	this->stone[0] = stone0;
	this->stone[1] = stone1;

	int nb = 256;

	while (nb--)
	{
		this->tstone[nb].actif = 0;
	}
}
/*--------------------------------------------------------------------------*/
void CLevitate::AddStone(EERIE_3D * pos)
{
	if (ARXPausedTimer) return;

	if (this->nbstone > 255) return;

	int	nb = 256;

	while (nb--)
	{
		if (!this->tstone[nb].actif)
		{
			this->nbstone++;

			this->tstone[nb].actif = 1;
			this->tstone[nb].numstone = rand() & 1;
			this->tstone[nb].pos = *pos;
			this->tstone[nb].yvel = rnd() * -5.f;
			this->tstone[nb].ang.a = rnd() * 360.f;
			this->tstone[nb].ang.b = rnd() * 360.f;
			this->tstone[nb].ang.g = rnd() * 360.f;
			this->tstone[nb].angvel.a = 5.f * rnd();
			this->tstone[nb].angvel.b = 6.f * rnd();
			this->tstone[nb].angvel.g = 3.f * rnd();
			float a = 0.2f + rnd() * 0.3f;
			this->tstone[nb].scale.x = a;
			this->tstone[nb].scale.y = a;
			this->tstone[nb].scale.z = a;
			this->tstone[nb].time = 2000 + (int)(500.f * rnd());
			this->tstone[nb].currtime = 0;
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/
void CLevitate::DrawStone(LPDIRECT3DDEVICE7 device)
{
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(device, TRUE);
	int	nb = 256;

	while (nb--)
	{
		if (this->tstone[nb].actif)
		{
			float a = (float)this->tstone[nb].currtime / (float)this->tstone[nb].time;

			if (a > 1.f)
			{
				a = 1.f;
				this->tstone[nb].actif = 0;
			}

			int col = RGBA_MAKE(255, 255, 255, (int)(255.f * (1.f - a)));

			if (this->stone[this->tstone[nb].numstone])
				DrawEERIEObjExEx(device, this->stone[this->tstone[nb].numstone], &this->tstone[nb].ang, &this->tstone[nb].pos, &this->tstone[nb].scale, col);

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist = 1;
				particle[j].zdec = 0;

				particle[j].ov = this->tstone[nb].pos;
				particle[j].move.x = 0.f;
				particle[j].move.y = 3.f * rnd();
				particle[j].move.z = 0.f;
				particle[j].siz = 3.f + 3.f * rnd();
				particle[j].tolive = 1000;
				particle[j].scale.x = 1.f;
				particle[j].scale.y = 1.f;
				particle[j].scale.z = 1.f;
				particle[j].timcreation = -(long)(ARXTime + 1000);
				particle[j].tc = NULL;
				particle[j].special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam = 0.0000001f;
				particle[j].r = 1.f;
				particle[j].g = 1.f;
				particle[j].b = 1.f;
			}


			//update mvt
			if (!ARXPausedTimer)
			{
				a = (((float)this->currframetime) * 100.f) / (float)this->tstone[nb].time;
				this->tstone[nb].pos.y += this->tstone[nb].yvel * a;
				this->tstone[nb].ang.a += this->tstone[nb].angvel.a * a;
				this->tstone[nb].ang.b += this->tstone[nb].angvel.b * a;
				this->tstone[nb].ang.g += this->tstone[nb].angvel.g * a;

				this->tstone[nb].yvel *= 1.f - (1.f / 100.f);

				this->tstone[nb].currtime += this->currframetime;
			}
		}
	}

	SETALPHABLEND(device, FALSE);
}

/*--------------------------------------------------------------------------*/
void CLevitate::Update(unsigned long _ulTime)
{
	float	a;

	//animation cone
	if (!ARXPausedTimer) this->currdurationang += _ulTime;

	this->ang = (float)this->currdurationang / 1000.f;

	if (this->ang > 1.f)
	{
		this->currdurationang = 0;
		this->ang = 1.f;
	}

	if (!ARXPausedTimer) ulCurrentTime += _ulTime;

	switch (this->key)
	{
		case 0:
			//monté du cone
			a = (float) ulCurrentTime / 1000.f;

			if (a > 1.f)
			{
				a = 0.f;
				this->key++;
			}

			this->scale = a;
			break;
		case 1:
			//animation cone
			this->scale = (float)ulCurrentTime / (float)ulDuration;

			if (ulCurrentTime >= ulDuration)
			{
				this->scale = 1.f;
				this->key++;
			}

			break;
	}

	if (!ARXPausedTimer)
	{
		this->currframetime = _ulTime;
		this->timestone -= _ulTime;
	}

	if (this->timestone <= 0)
	{
		this->timestone = 50 + (int)(rnd() * 100.f);
		EERIE_3D	pos;

		float r = this->rbase * frand2();
		pos.x = this->pos.x + r; 
		pos.y = this->pos.y;
		pos.z = this->pos.z + r; 
		this->AddStone(&pos);
	}
}

/*--------------------------------------------------------------------------*/
float CLevitate::Render(LPDIRECT3DDEVICE7 device)
{
	if (this->key > 1) return 0;

	SETALPHABLEND(device, TRUE);
	SETZWRITE(device, FALSE);

	//calcul du cone
	D3DTLVERTEX d3dvs, *d3dv;
	EERIE_3D	* vertex;
	int			nb, nbc, col;
	float		ddu = this->ang;
	float		u = ddu, du = .99999999f / (float)this->def;

	switch (this->key)
	{
		case 0:
			nbc = 2;

			while (nbc--)
			{
				vertex = this->cone[nbc].conevertex;
				d3dv = this->cone[nbc].coned3d;
				nb = (this->cone[nbc].conenbvertex) >> 1;

				while (nb)
				{
					d3dvs.sx = this->pos.x + (vertex + 1)->x + ((vertex->x - (vertex + 1)->x) * this->scale);
					d3dvs.sy = this->pos.y + (vertex + 1)->y + ((vertex->y - (vertex + 1)->y) * this->scale);
					d3dvs.sz = this->pos.z + (vertex + 1)->z + ((vertex->z - (vertex + 1)->z) * this->scale);
					
					EE_RT2(&d3dvs, d3dv);


					float fRandom	= rnd() * 80.f ;
					ARX_CHECK_INT(fRandom);

					col	= ARX_CLEAN_WARN_CAST_INT(fRandom);


					if (!ARXPausedTimer) d3dv->color = RGBA_MAKE(col, col, col, col);

					d3dv->tu = u;
					d3dv->tv = 0.f;
					vertex++;
					d3dv++;

					d3dvs.sx = this->pos.x + vertex->x;
					d3dvs.sy = this->pos.y;
					d3dvs.sz = this->pos.z + vertex->z;
					
					EE_RT2(&d3dvs, d3dv);


					fRandom = rnd() * 80.f ;
					ARX_CHECK_INT(fRandom);

					col = ARX_CLEAN_WARN_CAST_INT(fRandom);


					if (!ARXPausedTimer) d3dv->color = RGBA_MAKE(0, 0, 0, col);

					d3dv->tu = u;
					d3dv->tv = 0.9999999f;
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}

			nbc = 3;

			while (nbc--)
			{
				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					float a = DEG2RAD(360.f * rnd());

					particle[j].ov.x = this->pos.x + this->rbase * EEcos(a);
					particle[j].ov.y = this->pos.y;
					particle[j].ov.z = this->pos.z + this->rbase * EEsin(a);
					float t = EEDistance3D(&particle[j].ov, &this->pos);
					particle[j].move.x = (5.f + 5.f * rnd()) * ((particle[j].ov.x - this->pos.x) / t);
					particle[j].move.y = 3.f * rnd();
					particle[j].move.z = (5.f + 5.f * rnd()) * ((particle[j].ov.z - this->pos.z) / t);
					particle[j].siz = 30.f + 30.f * rnd();
					particle[j].tolive = 3000;
					particle[j].scale.x = 1.f;
					particle[j].scale.y = 1.f;
					particle[j].scale.z = 1.f;
					particle[j].timcreation = -(long)(ARXTime + 3000); //spells[i].lastupdate;
					particle[j].tc = NULL;
					particle[j].special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam = 0.0000001f;
					particle[j].r = 1.f;
					particle[j].g = 1.f;
					particle[j].b = 1.f;
				}
			}
			break;
		case 1:
			nbc = 2;

			while (nbc--)
			{
				vertex = this->cone[nbc].conevertex;
				d3dv = this->cone[nbc].coned3d;
				nb = (this->cone[nbc].conenbvertex) >> 1;

				while (nb)
				{
					d3dvs.sx = this->pos.x + vertex->x;
					d3dvs.sy = this->pos.y + vertex->y;
					d3dvs.sz = this->pos.z + vertex->z;
	
					EE_RT2(&d3dvs, d3dv);
					col = (int)(rnd() * 80.f);

					if (!ARXPausedTimer) d3dv->color = RGBA_MAKE(col, col, col, col);

					d3dv->tu = u;
					d3dv->tv = 0.f;
					vertex++;
					d3dv++;

					d3dvs.sx = this->pos.x + vertex->x;
					d3dvs.sy = this->pos.y;
					d3dvs.sz = this->pos.z + vertex->z;

					EE_RT2(&d3dvs, d3dv);
					col = (int)(rnd() * 80.f);

					if (!ARXPausedTimer) d3dv->color = RGBA_MAKE(0, 0, 0, col);

					d3dv->tu = u;
					d3dv->tv = 1; 
					vertex++;
					d3dv++;

					u += du;
					nb--;
				}

				u = ddu;
				du = -du;
			}

			nbc = 10;

			while (nbc--)
			{
				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					float a = DEG2RAD(360.f * rnd());

					particle[j].ov.x = this->pos.x + this->rbase * EEcos(a);
					particle[j].ov.y = this->pos.y;
					particle[j].ov.z = this->pos.z + this->rbase * EEsin(a);
					float t = EEDistance3D(&particle[j].ov, &this->pos);
					particle[j].move.x = (5.f + 5.f * rnd()) * ((particle[j].ov.x - this->pos.x) / t);
					particle[j].move.y = 3.f * rnd();
					particle[j].move.z = (5.f + 5.f * rnd()) * ((particle[j].ov.z - this->pos.z) / t);
					particle[j].siz = 30.f + 30.f * rnd();
					particle[j].tolive = 3000;
					particle[j].scale.x = 1.f;
					particle[j].scale.y = 1.f;
					particle[j].scale.z = 1.f;
					particle[j].timcreation = -(long)(ARXTime + 3000);
					particle[j].tc = NULL;
					particle[j].special = FIRE_TO_SMOKE | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam = 0.0000001f;
					particle[j].r = 1.f;
					particle[j].g = 1.f;
					particle[j].b = 1.f;
				}
			}

			break;
	}

	//tracé du cone back
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(device, TRUE);
	SETTEXTUREWRAPMODE(device, D3DTADDRESS_MIRROR);

	if (this->tsouffle) device->SetTexture(0, this->tsouffle->m_pddsSurface);
	else device->SetTexture(0, NULL);

	SETCULL(device, D3DCULL_CW);
	int i = cone[1].conenbfaces - 2;
	int j = 0;

	while (i--)
	{
		ARX_DrawPrimitive_SoftClippZ(&cone[1].coned3d[j],
		                             &cone[1].coned3d[j+1],
		                             &cone[1].coned3d[j+2]);
		j++;
	}

	i = cone[0].conenbfaces - 2;
	j = 0;

	while (i--)
	{
		ARX_DrawPrimitive_SoftClippZ(&cone[0].coned3d[j],
		                             &cone[0].coned3d[j+1],
		                             &cone[0].coned3d[j+2]);
		j++;
	}

	//tracé du cone front
	SETCULL(device, D3DCULL_CCW);
	
	i = cone[1].conenbfaces - 2;
	j = 0;

	while (i--)
	{
		ARX_DrawPrimitive_SoftClippZ(&cone[1].coned3d[j],
		                             &cone[1].coned3d[j+1],
		                             &cone[1].coned3d[j+2]);
		j++;
	}

	i = cone[0].conenbfaces - 2;
	j = 0;

	while (i--)
	{
		ARX_DrawPrimitive_SoftClippZ(&cone[0].coned3d[j],
		                             &cone[0].coned3d[j+1],
		                             &cone[0].coned3d[j+2]);
		j++;
	}

	//tracé des pierres
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->DrawStone(device);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	SETALPHABLEND(device, FALSE);
	SETZWRITE(device, TRUE);

	return 0;
}
