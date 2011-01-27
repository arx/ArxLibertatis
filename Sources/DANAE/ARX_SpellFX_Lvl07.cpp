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
// CSpellFx_Lvl07.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 07
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEDraw.h>
#include <EERIEMath.h>
#include <EERIEObject.h>

#include <ARX_Collisions.h>
#include <ARX_CSpellFx.h>
#include <ARX_Damages.h>
#include <ARX_Particles.h>
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_SpellFx_Lvl07.h"
#include <ARX_Spells.h>
#include "ARX_Time.h"


extern float _framedelay;

//------------------------------------------------------------------------------
CLightning::CLightning(TextureContainer * aTC) :
	lNbSegments(40),  
	invNbSegments(1.0f / 40.0f), 
	nbtotal(0),
	falpha(1.0f),
	fSize(100.0f),
	fLengthMin(5.0f),  
	fLengthMax(40.0f),  
	fAngleXMin(5.0f),
	fAngleXMax(32.0f),
	fAngleYMin(5.0f),
	fAngleYMax(32.0f),
	fAngleZMin(5.0f),
	fAngleZMax(32.0f),
	fDamage(1)
{
	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;
	SetColor1(1.0f, 0.0f, 0.0f);
	SetColor2(1.0f, 0.5f, 0.5f);

	SetColor1(1.0f, 1.0f, 1.0f);
	SetColor2(0.0f, 0.0f, 0.2f);

	tex_light = NULL;
}

//------------------------------------------------------------------------------
// Params une méchante struct
//------------------------------------------------------------------------------

void CLightning::BuildS(LIGHTNING * pLInfo)
{
	EERIE_3D astart;
	EERIE_3D avect;
	avect.x = avect.y = avect.z = 0.0f;
	avect.x = pLInfo->eVect.x;
	avect.y = pLInfo->eVect.y;
	avect.z = pLInfo->eVect.z;

	astart.x = pLInfo->eStart.x;
	astart.y = pLInfo->eStart.y;
	astart.z = pLInfo->eStart.z;

	if ((pLInfo->anb > 0) && (nbtotal < 2000))
	{
		nbtotal++;
		int moi = nbtotal;

		if (pLInfo->abFollow)
		{
			avect.x = eDest.x - pLInfo->eStart.x;
			avect.y = eDest.y - pLInfo->eStart.y;
			avect.z = eDest.z - pLInfo->eStart.z;
			TRUEVector_Normalize(&avect);
		}

		pLInfo->fSize *= 0.9f;

		if (pLInfo->fSize < fSizeMin) pLInfo->fSize = fSizeMin;

		if (pLInfo->fSize > fSizeMax) pLInfo->fSize = fSizeMax;

		float fAngleX = frand2() * (pLInfo->fAngleXMax - pLInfo->fAngleXMin) + pLInfo->fAngleXMin;
		float fAngleY = frand2() * (pLInfo->fAngleYMax - pLInfo->fAngleYMin) + pLInfo->fAngleYMin;
		float fAngleZ = frand2() * (pLInfo->fAngleZMax - pLInfo->fAngleZMin) + pLInfo->fAngleZMin;

		EERIE_3D av;
		av.x = (float) cos(acos(avect.x) - DEG2RAD(fAngleX));
		av.y = (float) sin(asin(avect.y) - DEG2RAD(fAngleY));
		av.z = (float) tan(atan(avect.z) - DEG2RAD(fAngleZ));

		TRUEVector_Normalize(&av);
		avect.x = av.x;
		avect.y = av.y;
		avect.z = av.z;

		float ts = rnd();
		av.x *= ts * (fLengthMax - fLengthMin) * pLInfo->anb * invNbSegments + fLengthMin;
		av.y *= ts * (fLengthMax - fLengthMin) * pLInfo->anb * invNbSegments + fLengthMin;
		av.z *= ts * (fLengthMax - fLengthMin) * pLInfo->anb * invNbSegments + fLengthMin;

		astart.x += av.x;
		astart.y += av.y;
		astart.z += av.z;
		pLInfo->eStart.x = astart.x;
		pLInfo->eStart.y = astart.y;
		pLInfo->eStart.z = astart.z;

		cnodetab[nbtotal].x = pLInfo->eStart.x;
		cnodetab[nbtotal].y = pLInfo->eStart.y;
		cnodetab[nbtotal].z = pLInfo->eStart.z;
		cnodetab[nbtotal].size = cnodetab[0].size * pLInfo->anb * invNbSegments;
		cnodetab[nbtotal].parent = pLInfo->aParent;

		int anb = pLInfo->anb;
		int anbrec = pLInfo->anbrec;

		float p = rnd();

		if ((p <= 0.15) && (pLInfo->anbrec < 7))
		{
			float m = rnd();

			if (pLInfo->abFollow)
			{
				pLInfo->eStart.x = astart.x;
				pLInfo->eStart.y = astart.y;
				pLInfo->eStart.z = astart.z;
				pLInfo->eVect.x = avect.x;
				pLInfo->eVect.y = avect.y;
				pLInfo->eVect.z = avect.z;
				pLInfo->abFollow = false;
				pLInfo->anb =  anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);

				pLInfo->eStart.x = astart.x;
				pLInfo->eStart.y = astart.y;
				pLInfo->eStart.z = astart.z;
				pLInfo->eVect.x = avect.x;
				pLInfo->eVect.y = avect.y;
				pLInfo->eVect.z = avect.z;
				pLInfo->abFollow = true;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);
			}
			else
			{
				pLInfo->abFollow = false;
				pLInfo->eStart.x = astart.x;
				pLInfo->eStart.y = astart.y;
				pLInfo->eStart.z = astart.z;
				pLInfo->eVect.x = avect.x;
				pLInfo->eVect.y = avect.y;
				pLInfo->eVect.z = avect.z;
				pLInfo->anb = anb - (int)(10 * (1 - m));
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);

				pLInfo->abFollow = false;
				pLInfo->eStart.x = astart.x;
				pLInfo->eStart.y = astart.y;
				pLInfo->eStart.z = astart.z;
				pLInfo->eVect.x = avect.x;
				pLInfo->eVect.y = avect.y;
				pLInfo->eVect.z = avect.z;
				pLInfo->anb = anb - (int)(10 * m);
				pLInfo->anbrec = anbrec + (int)(2 * m);
				pLInfo->aParent = moi;
				pLInfo->fAngleXMin = fAngleXMin;
				pLInfo->fAngleXMax = fAngleXMax;
				pLInfo->fAngleYMin = fAngleYMin;
				pLInfo->fAngleYMax = fAngleYMax;
				pLInfo->fAngleZMin = fAngleZMin;
				pLInfo->fAngleZMax = fAngleZMax;
				BuildS(pLInfo);
			}
		}
		
		else
		{
			if (rnd() <= 0.10)
			{
				pLInfo->abFollow = true;
			}

			pLInfo->eStart.x = astart.x;
			pLInfo->eStart.y = astart.y;
			pLInfo->eStart.z = astart.z;
			pLInfo->eVect.x = avect.x;
			pLInfo->eVect.y = avect.y;
			pLInfo->eVect.z = avect.z;
			pLInfo->anb = anb - 1;
			pLInfo->anbrec = anbrec;
			pLInfo->aParent = moi;
			pLInfo->fAngleXMin = fAngleXMin;
			pLInfo->fAngleXMax = fAngleXMax;
			pLInfo->fAngleYMin = fAngleYMin;
			pLInfo->fAngleYMax = fAngleYMax;
			pLInfo->fAngleZMin = fAngleZMin;
			pLInfo->fAngleZMax = fAngleZMax;
			BuildS(pLInfo);
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ACCESSEURS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CLightning::SetPosSrc(EERIE_3D aeSrc)
{
	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;
}

//-----------------------------------------------------------------------------
void CLightning::SetPosDest(EERIE_3D aeDest)
{
	eDest.x = aeDest.x;
	eDest.y = aeDest.y;
	eDest.z = aeDest.z;
}

//-----------------------------------------------------------------------------
void CLightning::SetColor1(float afR, float afG, float afB)
{
	fColor1[0] = afR;
	fColor1[1] = afG;
	fColor1[2] = afB;
}

//-----------------------------------------------------------------------------
void CLightning::SetColor2(float afR, float afG, float afB)
{
	fColor2[0] = afR;
	fColor2[1] = afG;
	fColor2[2] = afB;
}


float fTotoro = 0;
float fMySize = 2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CLightning::Create(EERIE_3D aeFrom, EERIE_3D aeTo, float beta)
{
	float fAlpha = 1.f;
	float fBeta = 0.f;
	
	SetDuration(ulDuration);
	SetPosSrc(aeFrom);
	SetPosDest(aeTo);

	nbtotal = 0;

	if (nbtotal == 0)
	{
		fbeta = fBeta; 
		falpha = fAlpha; 

		LIGHTNING LInfo;
		ZeroMemory(&LInfo, sizeof(LIGHTNING));

		LInfo.eStart.x = eSrc.x;
		LInfo.eStart.y = eSrc.y;
		LInfo.eStart.z = eSrc.z;
		LInfo.eEnd.x = eDest.x;
		LInfo.eEnd.y = eDest.y;
		LInfo.eEnd.z = eDest.z;
		LInfo.eVect.x = eDest.x - eSrc.x;
		LInfo.eVect.y = eDest.y - eSrc.y;
		LInfo.eVect.z = eDest.z - eSrc.z;
		LInfo.anb = lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fSize = fSize;
		LInfo.fAngleXMin = fAngleXMin;
		LInfo.fAngleXMax = fAngleXMax;
		LInfo.fAngleYMin = fAngleYMin;
		LInfo.fAngleYMax = fAngleYMax;
		LInfo.fAngleZMin = fAngleZMin;
		LInfo.fAngleZMax = fAngleZMax;

		cnodetab[0].x = eSrc.x;
		cnodetab[0].y = eSrc.y;
		cnodetab[0].z = eSrc.z;
		cnodetab[0].size = 15;
		cnodetab[0].parent = 0;

		BuildS(&LInfo);
	}


	float fRandom	= 500 + rnd() * 1000;
	ARX_CHECK_INT(fRandom);

	iTTL	= ARX_CLEAN_WARN_CAST_INT(fRandom);

}

//------------------------------------------------------------------------------
void CLightning::ReCreate()
{
	nbtotal = 0;

	if (nbtotal == 0)
	{

		falpha = 1.f;

		LIGHTNING LInfo;
		ZeroMemory(&LInfo, sizeof(LIGHTNING));

		LInfo.eStart.x = eSrc.x;
		LInfo.eStart.y = eSrc.y;
		LInfo.eStart.z = eSrc.z;
		LInfo.eEnd.x = eDest.x;
		LInfo.eEnd.y = eDest.y;
		LInfo.eEnd.z = eDest.z;
		LInfo.eVect.x = eDest.x - eSrc.x;
		LInfo.eVect.y = eDest.y - eSrc.y;
		LInfo.eVect.z = eDest.z - eSrc.z;
		LInfo.anb = lNbSegments;
		LInfo.anbrec = 0;
		LInfo.abFollow = true;
		LInfo.aParent = 0;
		LInfo.fSize = fSize;
		LInfo.fAngleXMin = fAngleXMin;
		LInfo.fAngleXMax = fAngleXMax;
		LInfo.fAngleYMin = fAngleYMin;
		LInfo.fAngleYMax = fAngleYMax;
		LInfo.fAngleZMin = fAngleZMin;
		LInfo.fAngleZMax = fAngleZMax;

		cnodetab[0].x = eSrc.x;
		cnodetab[0].y = eSrc.y;
		cnodetab[0].z = eSrc.z;
		cnodetab[0].size = 8;
		cnodetab[0].parent = 0;

		BuildS(&LInfo);
	}


	float fRandom	= 500 + rnd() * 1000;
	ARX_CHECK_INT(fRandom);

	iTTL = ARX_CLEAN_WARN_CAST_INT(fRandom);


}

//------------------------------------------------------------------------------
void CLightning::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	iTTL -= _ulTime;
	fTotoro += 8;

	if (fMySize > 0.3f)
		fMySize -= 0.1f;
}
void GetChestPos(long num, EERIE_3D * p)
{
	if (num == 0)
	{
		p->x = player.pos.x;
		p->y = player.pos.y + 70.f;
		p->z = player.pos.z;
		return;
	}

	if (ValidIONum(num))
	{
		long idx = GetGroupOriginByName(inter.iobj[num]->obj, "CHEST");

		if (idx >= 0)
		{
			p->x = inter.iobj[num]->obj->vertexlist3[idx].v.x;
			p->y = inter.iobj[num]->obj->vertexlist3[idx].v.y;
			p->z = inter.iobj[num]->obj->vertexlist3[idx].v.z;
		}
		else
		{
			p->x = inter.iobj[num]->pos.x;
			p->y = inter.iobj[num]->pos.y - 120.f;
			p->z = inter.iobj[num]->pos.z;
		}
	}
}
//------------------------------------------------------------------------------
float CLightning::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	D3DTLVERTEX v[4];
	D3DTLVERTEX v2[4];

	if (ulCurrentTime >= ulDuration) return 0.f;

	falpha = 1.f - (((float)(ulCurrentTime)) * fOneOnDuration); 

	if (falpha > 1.f) falpha = 1.f;

	if (iTTL <= 0)
	{
		fTotoro = 0;
		fMySize = 2;
		ReCreate();
	}

	falpha = 1;

	long i;

	EERIE_3D ePos;
	
	float fBeta = 0.f;
	falpha = 0.f;

	// Create hand position if a hand is defined
	//	spells[spellinstance].hand_group=inter.iobj[spells[spellinstance].caster]->obj->fastaccess.primary_attach;//GetActionPointIdx(inter.iobj[spells[spellinstance].caster]->obj,"PRIMARY_ATTACH");
	// Player source
	if (spells[spellinstance].type == SPELL_MASS_LIGHTNING_STRIKE)
	{


		ARX_CHECK( lSrc == -1 );	//ARX: jycorbel (2010-07-19) - We really need ePos when lSrc!=-1 ; in that case lSrc should be equal to -1 !
		ePos.x = 0.f;
		ePos.y = 0.f;
		ePos.z = 0.f;

	}
	else
	{
		if (spells[spellinstance].caster == 0)
		{
			long idx = GetGroupOriginByName(inter.iobj[spells[spellinstance].caster]->obj, "CHEST");

			if (idx >= 0)
			{
				spells[spellinstance].caster_pos.x = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.x;
				spells[spellinstance].caster_pos.y = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.y;
				spells[spellinstance].caster_pos.z = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.z;
			}
			else
			{
				spells[spellinstance].caster_pos.x = player.pos.x;
				spells[spellinstance].caster_pos.y = player.pos.y;
				spells[spellinstance].caster_pos.z = player.pos.z;
			}

			falpha = -player.angle.a;
			fBeta = player.angle.b;
		}
		// IO source
		else
		{
			long idx = GetGroupOriginByName(inter.iobj[spells[spellinstance].caster]->obj, "CHEST");

			if (idx >= 0)
			{
				spells[spellinstance].caster_pos.x = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.x;
				spells[spellinstance].caster_pos.y = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.y;
				spells[spellinstance].caster_pos.z = inter.iobj[spells[spellinstance].caster]->obj->vertexlist3[idx].v.z;
			}
			else
			{
				spells[spellinstance].caster_pos.x = inter.iobj[spells[spellinstance].caster]->pos.x;
				spells[spellinstance].caster_pos.y = inter.iobj[spells[spellinstance].caster]->pos.y;
				spells[spellinstance].caster_pos.z = inter.iobj[spells[spellinstance].caster]->pos.z;
			}

			fBeta = inter.iobj[spells[spellinstance].caster]->angle.b;
			INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].caster];

			if (ValidIONum(io->targetinfo)
			        &&	(io->targetinfo != spells[spellinstance].caster))
			{
				EERIE_3D * p1 = &spells[spellinstance].caster_pos;
				EERIE_3D p2;
				GetChestPos(io->targetinfo, &p2); 
				falpha = MAKEANGLE(RAD2DEG(GetAngle(p1->y, p1->z, p2.y, p2.z + TRUEDistance2D(p2.x, p2.z, p1->x, p1->z)))); //alpha entre orgn et dest;
			}
			else if (ValidIONum(spells[spellinstance].target))
			{
				EERIE_3D * p1 = &spells[spellinstance].caster_pos;
				EERIE_3D p2;
				GetChestPos(spells[spellinstance].target, &p2); //
				falpha = MAKEANGLE(RAD2DEG(GetAngle(p1->y, p1->z, p2.y, p2.z + TRUEDistance2D(p2.x, p2.z, p1->x, p1->z)))); //alpha entre orgn et dest;
			}
		}

		ePos.x = spells[spellinstance].caster_pos.x;
		ePos.y = spells[spellinstance].caster_pos.y;
		ePos.z = spells[spellinstance].caster_pos.z;
	}

	//-------------------------------------------------------------------------
	// rendu

	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);

	cnodetab[0].fx = frand2() * 1.5f * fMySize; //5
	cnodetab[0].fy = frand2() * 1.5f * fMySize; //5
	cnodetab[0].fz = frand2() * 1.5f * fMySize; //5


	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);
	SETTC(m_pd3dDevice, NULL);

	v2[0].color = v2[1].color = v2[2].color = v2[3].color = D3DRGB(1, 1, 1);

	float xx;
	float zz;
	float zzx, zzy, zzz;

	fbeta = fBeta + rnd() * 2 * fMySize; //0;

	for (i = 0; i < nbtotal; i++)
	{
		if (i > fTotoro) break;

		EERIE_3D astart;

		astart.x = cnodetab[cnodetab[i].parent].x + cnodetab[cnodetab[i].parent].fx;
		astart.y = cnodetab[cnodetab[i].parent].y + cnodetab[cnodetab[i].parent].fy;
		astart.z = cnodetab[cnodetab[i].parent].z + cnodetab[cnodetab[i].parent].fz;
		float temp = 1.5f * fMySize;
		zzx = cnodetab[cnodetab[i].parent].fx + frand2() * temp;
		zzy = cnodetab[cnodetab[i].parent].fx + frand2() * temp;
		zzz = cnodetab[cnodetab[i].parent].fx + frand2() * temp;

		zz = cnodetab[i].size + cnodetab[i].size * 0.3f * rnd();
		xx = (float)(cnodetab[i].size * cos(DEG2RAD(-fbeta)));

		cnodetab[i].fx = zzx ;
		cnodetab[i].fy = zzy ;
		cnodetab[i].fz = zzz ;

		float ax = cnodetab[i].x + zzx;
		float ay = cnodetab[i].y + zzy;
		float az = cnodetab[i].z + zzz;

		if (lSrc != -1)
		{
			EERIE_3D vv1, vv2;
			vv1.x = astart.x;
			vv1.y = astart.y;
			vv1.z = astart.z;
			VRotateX(&vv1, (falpha));  
			Vector_RotateY(&vv2, &vv1,  180 - MAKEANGLE(fBeta)); 
			astart.x = vv2.x;
			astart.y = vv2.y;
			astart.z = vv2.z;

			vv1.x = ax;
			vv1.y = ay;
			vv1.z = az;
			VRotateX(&vv1, (falpha)); 
			Vector_RotateY(&vv2, &vv1, 180 - MAKEANGLE(fBeta)); 
			ax = vv2.x;
			ay = vv2.y;
			az = vv2.z;

			astart.x += ePos.x;
			astart.y += ePos.y;
			astart.z += ePos.z;

			ax += ePos.x;
			ay += ePos.y;
			az += ePos.z;
		}

		if (((i >> 2) << 2) == i)
		{
			EERIE_SPHERE sphere;
			sphere.origin.x = ax;
			sphere.origin.y = ay;
			sphere.origin.z = az;
			sphere.radius = __min(cnodetab[i].size, 50.f);

			if (CheckAnythingInSphere(&sphere, spells[spellinstance].caster, CAS_NO_SAME_GROUP))
			{
				long si = ARX_DAMAGES_GetFree();

				if (si != -1)
				{
					Vector_Copy(&damages[si].pos, &sphere.origin);
					damages[si].radius = sphere.radius;
					damages[si].damages = fDamage * spells[spellinstance].caster_level * DIV3; 
					damages[si].area = DAMAGE_FULL;
					damages[si].duration = 1; 
					damages[si].source = spells[spellinstance].caster;
					damages[si].flags = DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
					damages[si].type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_LIGHTNING;
					damages[si].exist = TRUE;
				}
			}
		}

		// version 4 faces
		v2[0].color = v2[3].color = 0xFFFFFFFF;
		v2[1].color = v2[2].color = 0xFF00005A;

		v2[0].tu = 0.5f;
		v2[0].tv = 0;
		v2[1].tu = 0;
		v2[1].tv = 0;
		v2[2].tu = 0;
		v2[2].tv = 1;
		v2[3].tu = 0.5f;
		v2[3].tv = 1;

		v[0].sx = astart.x;
		v[0].sy = astart.y;
		v[0].sz = astart.z;

		v[1].sx = astart.x;
		v[1].sy = astart.y + zz;
		v[1].sz = astart.z;

		v[2].sx = ax;
		v[2].sy = ay + zz;
		v[2].sz = az;

		v[3].sx = ax;
		v[3].sy = ay;
		v[3].sz = az;

		EE_RT2(&v[0], &v2[0]);
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		EE_RT2(&v[3], &v2[3]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[2],
		                             &v2[3]);

		v2[0].tu = 0.5f;
		v2[0].tv = 0;
		v2[1].tu = 1.0f;
		v2[1].tv = 0;
		v2[2].tu = 1.0f;
		v2[2].tv = 1.0f;
		v2[3].tu = 0.5f;
		v2[3].tv = 1.0f;

		v[1].sx = astart.x;
		v[1].sy = astart.y - zz;
		v[1].sz = astart.z;

		v[2].sx = ax;
		v[2].sy = ay - zz;
		v[2].sz = az;
	
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[2],
		                             &v2[3]);

	
		zz *= (float) sin(DEG2RAD(fbeta));

		v2[1].tu = 1.0f;
		v2[1].tv = 0;
		v2[2].tu = 1.0f;
		v2[2].tv = 1.0f;

		v[1].sx = astart.x + xx;
		v[1].sy = astart.y;
		v[1].sz = astart.z + zz;

		v[2].sx = ax + xx;
		v[2].sy = ay;
		v[2].sz = az + zz;
		
		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[2],
		                             &v2[3]);

		v2[1].tu = 0;
		v2[1].tv = 0;
		v2[2].tu = 0;
		v2[2].tv = 1.0f;

		v[1].sx = astart.x - xx;
		v[1].sy = astart.y;
		v[1].sz = astart.z - zz;

		v[2].sx = ax - xx;
		v[2].sy = ay;
		v[2].sz = az - zz;

		EE_RT2(&v[1], &v2[1]);
		EE_RT2(&v[2], &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[1],
		                             &v2[2]);
		ARX_DrawPrimitive_SoftClippZ(&v2[0],
		                             &v2[2],
		                             &v2[3]);
	}

	SETZWRITE(m_pd3dDevice, TRUE);
	SETALPHABLEND(m_pd3dDevice, FALSE);
	return falpha;
}

//-----------------------------------------------------------------------------
CConfuse::~CConfuse()
{
	spapi_count--;

	if (spapi && (spapi_count <= 0))
	{
		spapi_count = 0;
		ReleaseEERIE3DObj(spapi);
		spapi = NULL;
	}
}
CConfuse::CConfuse(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(5000);
	ulCurrentTime = ulDuration + 1;

	tex_p1 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");
	tex_trail = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_bandelette_blue.bmp");

	if (!spapi)
	{
		spapi   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_papivolle\\fx_papivolle.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(spapi);
	}

	spapi_count++;

	char tex[256];

	MakeDir(tex, "Graph\\Obj3D\\Interactive\\Fix_inter\\fx_papivolle\\fx_papivolle.tea");
	ANIM_HANDLE * anim_papii = EERIE_ANIMMANAGER_Load(tex);

	fColor[0] = 0.3f;
	fColor[1] = 0.3f;
	fColor[2] = 0.8f;

	ANIM_Set(&au, anim_papii);
	au.next_anim = NULL;
	au.cur_anim = anim_papii;
	au.ctime = 0;
	au.flags = EA_LOOP;
	au.nextflags = 0;
	au.lastframe = 0;
	au.pour = 0;
	au.fr = 0;
	au.altidx_cur = 0;
	au.altidx_next = 0;
}

//-----------------------------------------------------------------------------
void CConfuse::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	SetAngle(afBeta);

	fSize = 1;
	bDone = true;
	
	eTarget.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eTarget.y = inter.iobj[spells[spellinstance].target]->pos.y;
	eTarget.z = inter.iobj[spells[spellinstance].target]->pos.z;

	end = 20 - 1;
}

//---------------------------------------------------------------------
void CConfuse::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
	iElapsedTime = _ulTime;
}

//---------------------------------------------------------------------
float CConfuse::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	eTarget.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eTarget.y = inter.iobj[spells[spellinstance].target]->pos.y;
	eTarget.z = inter.iobj[spells[spellinstance].target]->pos.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETZWRITE(m_pd3dDevice, FALSE);

	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//-------------------------------------------------------------------------
	if (tex_trail && tex_trail->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_trail);
	}

	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	eCurPos.x = inter.iobj[spells[spellinstance].target]->pos.x;
	eCurPos.y = inter.iobj[spells[spellinstance].target]->pos.y;

	if (spells[spellinstance].target != 0)
		eCurPos.y += inter.iobj[spells[spellinstance].target]->physics.cyl.height - 30.f;

	eCurPos.z = inter.iobj[spells[spellinstance].target]->pos.z;

	long idx = inter.iobj[spells[spellinstance].target]->obj->fastaccess.head_group_origin;

	if (idx >= 0)
	{
		eCurPos.x = inter.iobj[spells[spellinstance].target]->obj->vertexlist3[idx].v.x;
		eCurPos.y = inter.iobj[spells[spellinstance].target]->obj->vertexlist3[idx].v.y - 50.f;
		eCurPos.z = inter.iobj[spells[spellinstance].target]->obj->vertexlist3[idx].v.z;
	}

	stitepos.x = eCurPos.x;
	stitepos.y = eCurPos.y;
	stitepos.z = eCurPos.z;

	stiteangle.b = -RAD2DEG(ARX_TIME_Get() * DIV500);
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 1;
	stitescale.y = 1;
	stitescale.z = 1;
	DrawEERIEObjEx(m_pd3dDevice, spapi, &stiteangle, &stitepos, &stitescale, &stitecolor);

	long j = -1;

	for (i = 0; i < 6; i++)
	{
		j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist = 1;
			particle[j].zdec = 0;

			float ang				=	DEG2RAD(rnd() * 360.f);
			float rad				=	rnd() * 15.f;
			particle[j].ov.x		=	stitepos.x - EEsin(ang) * rad;
			particle[j].ov.y		=	stitepos.y;
			particle[j].ov.z		=	stitepos.z + EEcos(ang) * rad;
			particle[j].move.x		=	0;
			particle[j].move.y		=	rnd() * 3 + 1;
			particle[j].move.z		=	0;
			particle[j].siz			=	0.25f;
			particle[j].tolive		=	2300 + (unsigned long)(float)(rnd() * 1000.f);
			particle[j].scale.x		=	1;
			particle[j].scale.y		=	1;
			particle[j].scale.z		=	1;
			particle[j].timcreation	=	lARXTime;
			particle[j].tc			=	tex_p1;
			particle[j].special		=	PARTICLE_GOLDRAIN | FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].fparam		=	0.0000001f;
			float t1				=	rnd() * 0.4f + 0.4f;
			float t2				=	rnd() * 0.6f + 0.2f;
			float t3				=	rnd() * 0.4f + 0.4f;

			while ((EEfabs(t1 - t2) > 0.3f) && (EEfabs(t2 - t3) > 0.3f))
			{
				t1 = rnd() * 0.4f + 0.4f;
				t2 = rnd() * 0.6f + 0.2f;
				t3 = rnd() * 0.4f + 0.4f;
			}

			particle[j].r = t1 * 0.8f;
			particle[j].g = t2 * 0.8f;
			particle[j].b = t3 * 0.8f;
		}
	}

	if (this->lLightId == -1)
		this->lLightId = GetFreeDynLight();

	if (this->lLightId != -1)
	{
		long id = this->lLightId;
		DynLight[id].exist = 1;
		DynLight[id].intensity = 1.3f;
		DynLight[id].fallstart = 180.f;
		DynLight[id].fallend   = 420.f;
		DynLight[id].rgb.r = 0.3f + rnd() * DIV5;
		DynLight[id].rgb.g = 0.3f;
		DynLight[id].rgb.b = 0.5f + rnd() * DIV5;
		DynLight[id].pos.x = stitepos.x;
		DynLight[id].pos.y = stitepos.y;
		DynLight[id].pos.z = stitepos.z;
		DynLight[id].duration = 200;
		DynLight[id].extras = 0;
	}

	return 1;
}
//-----------------------------------------------------------------------------
//
//	FIRE FIELD
//
//-----------------------------------------------------------------------------
CFireField::CFireField()
{
}

//-----------------------------------------------------------------------------
CFireField::~CFireField()
{
}

//-----------------------------------------------------------------------------
void CFireField::Create(float largeur, EERIE_3D * pos, int _ulDuration)
{
	this->key = 0;

	SetDuration(_ulDuration);

	pos->y -= 50;

	this->pos = *pos;
	this->demilargeur = largeur * .5f;

	CParticleParams cp;
	cp.iNbMax = 100;
	cp.fLife = 2000;
	cp.fLifeRandom = 1000;
	cp.p3Pos.x = 80;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 80;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = 2;
	cp.p3Direction.z = 0;
	cp.fAngle = 0;
	cp.fSpeed = 0;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;
	cp.bRotationRandomDirection = false;
	cp.bRotationRandomStart = false;

	cp.fStartSize = 10;
	cp.fStartSizeRandom = 3;
	cp.fStartColor[0] = 25;
	cp.fStartColor[1] = 25;
	cp.fStartColor[2] = 25;
	cp.fStartColor[3] = 50;
	cp.fStartColorRandom[0] = 51;
	cp.fStartColorRandom[1] = 51;
	cp.fStartColorRandom[2] = 51;
	cp.fStartColorRandom[3] = 101;
	cp.bStartLock = false;

	cp.fEndSize = 10;
	cp.fEndSizeRandom = 3;
	cp.fEndColor[0] = 25;
	cp.fEndColor[1] = 25;
	cp.fEndColor[2] = 25;
	cp.fEndColor[3] = 50; 
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 100; 
	cp.bEndLock = false;
	cp.bTexLoop = true;

	cp.iBlendMode = 3;

	pPSStream.SetParams(cp);
	pPSStream.ulParticleSpawn = 0;

	pPSStream.SetTexture("graph\\particles\\firebase", 4, 100);

	pPSStream.fParticleFreq = 150.0f;
	pPSStream.SetPos(*pos);
	pPSStream.Update(0);

	//-------------------------------------------------------------------------

	cp.iNbMax = 50;
	cp.fLife = 1000;
	cp.fLifeRandom = 500;
	cp.p3Pos.x = 100;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 100;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -2;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(10);
	cp.fSpeed = 0;
	cp.fSpeedRandom = 0;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 0;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 0;
	cp.bRotationRandomDirection = false;
	cp.bRotationRandomStart = false;

	cp.fStartSize = 10;
	cp.fStartSizeRandom = 10;
	cp.fStartColor[0] = 40;
	cp.fStartColor[1] = 40;
	cp.fStartColor[2] = 40;
	cp.fStartColor[3] = 50; 
	cp.fStartColorRandom[0] = 51;
	cp.fStartColorRandom[1] = 51;
	cp.fStartColorRandom[2] = 51;
	cp.fStartColorRandom[3] = 100; 
	cp.bStartLock = false;

	cp.fEndSize = 10;
	cp.fEndSizeRandom = 10;
	cp.fEndColor[0] = 0;
	cp.fEndColor[1] = 0;
	cp.fEndColor[2] = 0;
	cp.fEndColor[3] = 50;
	cp.fEndColorRandom[0] = 0;
	cp.fEndColorRandom[1] = 0;
	cp.fEndColorRandom[2] = 0;
	cp.fEndColorRandom[3] = 100; 
	cp.bEndLock = false;
	cp.bTexLoop = false;

	cp.iBlendMode = 0;

	pPSStream1.SetParams(cp);
	pPSStream1.ulParticleSpawn = 0;

	pPSStream1.SetTexture("graph\\particles\\fire.bmp", 0, 500);

	pPSStream1.fParticleFreq = 150.0f;
	EERIE_3D ea;
	ea.x = pos->x;
	ea.z = pos->y + 10; 
	ea.y = pos->z;
	pPSStream1.SetPos(ea);
	pPSStream1.Update(0);
}

//-----------------------------------------------------------------------------
void CFireField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;

	pPSStream.Update(_ulTime);
	pPSStream1.Update(_ulTime);
}

/*--------------------------------------------------------------------------*/
float CFireField::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	if (this->key > 1) return 0;

	SETALPHABLEND(_pD3DDevice, TRUE);
	SETZWRITE(_pD3DDevice, FALSE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	SETCULL(_pD3DDevice, D3DCULL_NONE);
	SETZWRITE(_pD3DDevice, FALSE);
	SETALPHABLEND(_pD3DDevice, TRUE);

	pPSStream.Render(_pD3DDevice, D3DBLEND_ONE, D3DBLEND_ONE);
	pPSStream1.Render(_pD3DDevice, D3DBLEND_ONE, D3DBLEND_ONE);

	_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	SETALPHABLEND(_pD3DDevice, FALSE);
	SETZWRITE(_pD3DDevice, TRUE);

	return 0;
}

//-----------------------------------------------------------------------------
CIceField::~CIceField()
{
	smotte_count--;

	if (smotte && (smotte_count <= 0))
	{
		smotte_count = 0;
		ReleaseEERIE3DObj(smotte);
		smotte = NULL;
	}

	stite_count--;

	if (stite && (stite_count <= 0))
	{
		stite_count = 0;
		ReleaseEERIE3DObj(stite);
		stite = NULL;
	}
}

CIceField::CIceField(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(1000);
	ulCurrentTime = ulDuration + 1;

	iNumber = 50;

	tex_p1 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");
	tex_p2 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_bluepouf.bmp");

	if (!stite)
	{
		stite   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\Stalagmite\\motte.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(stite);
	}

	stite_count++;

	if (!smotte)
	{
		smotte   = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\Stalagmite\\motte.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(smotte);
	}

	smotte_count++;
}

//-----------------------------------------------------------------------------
void CIceField::Create(EERIE_3D aeSrc, float afBeta)
{
	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	eTarget.x = eSrc.x;
	eTarget.y = eSrc.y;
	eTarget.z = eSrc.z;

	iNumber = 50;
	fSize = 1;

	float	xmin, ymin, zmin;

	for (int i = 0 ; i < iNumber ; i++)
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

		if (tType[i] == 0)
		{
			tPos[i].x = eSrc.x + frand2() * 80;
			tPos[i].y = eSrc.y;
			tPos[i].z = eSrc.z + frand2() * 80;
		}
		else
		{
			tPos[i].x = eSrc.x + frand2() * 120;
			tPos[i].y = eSrc.y;
			tPos[i].z = eSrc.z + frand2() * 120;
		}
	}

	int j = 0;
	iMax  = iNumber;
	
	j = 50;

	iMax = j;

	iNumber = j;
}

//---------------------------------------------------------------------
void CIceField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CIceField::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	
	SETZWRITE(m_pd3dDevice, TRUE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	iMax = (int)(iNumber); 

	if (iMax > iNumber) iMax = iNumber;

 
	float x = eSrc.x;
	float y = eSrc.y;
	float z = eSrc.z;

	for (i = 0; i < iMax; i++)
	{
		if (tSize[i].x < tSizeMax[i].x)
			tSize[i].x += 0.1f;

		if (tSize[i].x > tSizeMax[i].x)
			tSize[i].x = tSizeMax[i].x;

		if (tSize[i].y < tSizeMax[i].y)
			tSize[i].y += 0.1f;

		if (tSize[i].y > tSizeMax[i].y)
			tSize[i].y = tSizeMax[i].y;

		if (tSize[i].z < tSizeMax[i].z)
			tSize[i].z += 0.1f;

		if (tSize[i].z > tSizeMax[i].z)
			tSize[i].z = tSizeMax[i].z;

		EERIE_3D stiteangle;
		EERIE_3D stitepos;
		EERIE_3D stitescale;
		EERIE_RGB stitecolor;

		stiteangle.b = (float) cos(DEG2RAD(tPos[i].x)) * 360; 
		stiteangle.a = 0;
		stiteangle.g = 0;
		stitepos.x = tPos[i].x;
		stitepos.y = y;
		stitepos.z = tPos[i].z;

		float fcol = 1;

		if (abs(iMax - i) < 60)
		{
			fcol = 1;
		}
		else
		{
		}

		stitecolor.r = tSizeMax[i].y * fcol * 0.7f; 
		stitecolor.g = tSizeMax[i].y * fcol * 0.7f; 
		stitecolor.b = tSizeMax[i].y * fcol * 0.9f;

		if (stitecolor.r > 1) stitecolor.r = 1;

		if (stitecolor.g > 1) stitecolor.g = 1;

		if (stitecolor.b > 1) stitecolor.b = 1;

		stitescale.z = tSize[i].x;
		stitescale.y = tSize[i].y;
		stitescale.x = tSize[i].z;

		if (tType[i] == 0)
			DrawEERIEObjEx(m_pd3dDevice, smotte, &stiteangle, &stitepos, &stitescale, &stitecolor);
		else
			DrawEERIEObjEx(m_pd3dDevice, stite, &stiteangle, &stitepos, &stitescale, &stitecolor);
	}

	//----------------

	for (i = 0; i < iMax * 0.5f; i++)
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

				particle[j].ov.x		=	x + 5.f - rnd() * 10.f;
				particle[j].ov.y		=	y + 5.f - rnd() * 10.f;
				particle[j].ov.z		=	z + 5.f - rnd() * 10.f;
				particle[j].move.x		=	2.f - 4.f * rnd();
				particle[j].move.y		=	2.f - 4.f * rnd();
				particle[j].move.z		=	2.f - 4.f * rnd();
				particle[j].siz			=	20.f;
				particle[j].tolive		=	2000 + (unsigned long)(float)(rnd() * 4000.f);
				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation	=	lARXTime;
				particle[j].tc			=	tex_p2;
				particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam		=	0.0000001f;
				particle[j].r			=	0.7f;
				particle[j].g			=	0.7f;
				particle[j].b			=	1.f;
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

				particle[j].ov.x		=	x + 5.f - rnd() * 10.f;
				particle[j].ov.y		=	y + 5.f - rnd() * 10.f;
				particle[j].ov.z		=	z + 5.f - rnd() * 10.f;
				particle[j].move.x		=	0;
				particle[j].move.y		=	2.f - 4.f * rnd();
				particle[j].move.z		=	0;
				particle[j].siz			=	0.5f;
				particle[j].tolive		=	2000 + (unsigned long)(float)(rnd() * 4000.f);
				particle[j].scale.x		=	1.f;
				particle[j].scale.y		=	1.f;
				particle[j].scale.z		=	1.f;
				particle[j].timcreation = lARXTime;
				particle[j].tc			=	tex_p1;
				particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam		=	0.0000001f;
				particle[j].r			=	0.7f;
				particle[j].g			=	0.7f;
				particle[j].b			=	1.f;
			}
		}
	}

	//----------------

	return 1;
}
