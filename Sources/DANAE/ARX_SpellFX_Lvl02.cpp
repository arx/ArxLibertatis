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
// CSpellFx_Lvl02.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 02
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEMath.h>
#include <EERIELight.h>
#include "ARX_Spells.h"
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl02.h"
#include "ARX_Particles.h"
#include "ARX_CParticles.h"
#include "ARX_CParticle.h"
#include "ARX_CParticleParams.h"
#include "ARX_Time.h"


#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

extern CParticleManager * pParticleManager;

//-----------------------------------------------------------------------------
CHeal::CHeal(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	pPS = new CParticleSystem();
}

//-----------------------------------------------------------------------------
CHeal::~CHeal()
{
	// kill ParticleSystem
	if (pPS)
	{
		delete pPS;
		pPS = NULL;
	}
}

//-----------------------------------------------------------------------------
void CHeal::Create()
{
	SetAngle(MAKEANGLE(player.angle.b));

	if (spells[spellinstance].caster == 0)
	{
		eSrc.x = player.pos.x;
		eSrc.y = player.pos.y;
		eSrc.z = player.pos.z;
	}
	else
	{
		eSrc.x = inter.iobj[spells[spellinstance].caster]->pos.x;
		eSrc.y = inter.iobj[spells[spellinstance].caster]->pos.y;
		eSrc.z = inter.iobj[spells[spellinstance].caster]->pos.z;
	}

	pPS->lLightId = GetFreeDynLight();

	if (pPS->lLightId != -1)
	{
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.4f;
		DynLight[id].rgb.g = 0.4f;
		DynLight[id].rgb.b = 1.0f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	pPS->SetPos(eSrc);
	CParticleParams cp;
	cp.iNbMax = 350;
	cp.fLife = 800;
	cp.fLifeRandom = 2000;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 200;
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

	cp.fStartSize = 8;
	cp.fStartSizeRandom = 8;
	cp.fStartColor[0] = 205;
	cp.fStartColor[1] = 205;
	cp.fStartColor[2] = 255;
	cp.fStartColor[3] = 245;
	cp.fStartColorRandom[0] = 50;
	cp.fStartColorRandom[1] = 50;
	cp.fStartColorRandom[2] = 0;
	cp.fStartColorRandom[3] = 10;

	cp.fEndSize = 6;
	cp.fEndSizeRandom = 4;
	cp.fEndColor[0] = 20;
	cp.fEndColor[1] = 20;
	cp.fEndColor[2] = 30;
	cp.fEndColor[3] = 0;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 40;
	cp.fEndColorRandom[3] = 0;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = PARTICLE_CIRCULAR | PARTICLE_BORDER;
	pPS->SetTexture("graph\\particles\\heal_0005.bmp", 0, 100); 

	fSize = 1;
}

//---------------------------------------------------------------------
void CHeal::Update(unsigned long aulTime)
{
	ulCurrentTime += aulTime;

	if (ulCurrentTime >= ulDuration)
	{
		return;
	}

	if (spells[spellinstance].caster == 0)
	{
		eSrc.x = player.pos.x;
		eSrc.y = player.pos.y;
		eSrc.z = player.pos.z;
	}
	else
	{
		if (ValidIONum(spells[spellinstance].target))
		{
			eSrc.x = inter.iobj[spells[spellinstance].target]->pos.x;
			eSrc.y = inter.iobj[spells[spellinstance].target]->pos.y;
			eSrc.z = inter.iobj[spells[spellinstance].target]->pos.z;
		}
	}

	if (pPS->lLightId == -1)
		pPS->lLightId = GetFreeDynLight();

	if (pPS->lLightId != -1)
	{
		long id = pPS->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 200.f;
		DynLight[id].fallend   = 350.f;
		DynLight[id].rgb.r = 0.4f;
		DynLight[id].rgb.g = 0.4f;
		DynLight[id].rgb.b = 1.0f;
		DynLight[id].pos.x = eSrc.x;
		DynLight[id].pos.y = eSrc.y - 50.f;
		DynLight[id].pos.z = eSrc.z;
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

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
}

//---------------------------------------------------------------------
float CHeal::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	pPS->Render(m_pd3dDevice, D3DBLEND_ONE, D3DBLEND_ONE);

	return 1;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
float CHarm::Render(LPDIRECT3DDEVICE7 device)
{
	return 0;
}

//-----------------------------------------------------------------------------
// ARMOR
//-----------------------------------------------------------------------------
CArmor::CArmor()
{
}

//-----------------------------------------------------------------------------
CArmor::~CArmor()
{
}

//-----------------------------------------------------------------------------
void CArmor::Create(long _ulDuration, int _iNpc)
{
	SetDuration(_ulDuration);

	if (spellinstance != -1)
	{

		INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].caster];

		if ((io) && (!io->halo.flags & HALO_ACTIVE))
		{
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 0.5f;
			io->halo.color.g = 0.5f;
			io->halo.color.b = 0.25f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			spells[spellinstance].longinfo = 1;
		}
		else spells[spellinstance].longinfo = 0;
	}
}

//-----------------------------------------------------------------------------
void CArmor::Update(unsigned long _ulTime)
{
	if (!ARXPausedTimer) ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
float CArmor::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	return 0;
}


//-----------------------------------------------------------------------------
// LOWER ARMOR
//-----------------------------------------------------------------------------
CLowerArmor::CLowerArmor()
{
}

//-----------------------------------------------------------------------------
CLowerArmor::~CLowerArmor()
{
}

//-----------------------------------------------------------------------------
void CLowerArmor::Create(long _ulDuration, int _iNpc)
{
	SetDuration(_ulDuration);

	if (spellinstance != -1)
	{
		INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].target];

		if ((io) && (!io->halo.flags & HALO_ACTIVE))
		{
			io->halo.flags |= HALO_ACTIVE;
			io->halo.color.r = 1.f;
			io->halo.color.g = 0.05f;
			io->halo.color.b = 0.0f;
			io->halo.radius = 45.f;
			io->halo.dynlight = -1;
			spells[spellinstance].longinfo = 1;
		}
		else spells[spellinstance].longinfo = 0;
	}
}

//-----------------------------------------------------------------------------
void CLowerArmor::Update(unsigned long _ulTime)
{
	if (!ARXPausedTimer) ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
float CLowerArmor::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	return 0;
}
