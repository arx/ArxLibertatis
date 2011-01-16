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
// CSpellFx_Lvl01.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 01
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "ARX_SpellFx_Lvl01.h"

#include <EERIEDraw.h>
#include <EERIEMath.h>
#include <EERIELight.h>
#include <EERIEObject.h>

#include <ARX_Collisions.h>
#include <ARX_CParticleParams.h>
#include <ARX_CParticles.h>
#include <ARX_CSpellFx.h>
#include <ARX_Damages.h>
#include <ARX_Particles.h>
#include <ARX_Sound.h>
#include <ARX_Spells.h>
#include "ARX_Time.h"
#include "ARX_Npc.h"
#include "ARX_SpellFx_Lvl05.h"

#include <DanaeSaveLoad.h>

#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

extern CParticleManager * pParticleManager;

 
extern long cur_mr;
//-----------------------------------------------------------------------------
void LaunchMagicMissileExplosion(EERIE_3D & _ePos, int t = 0, long spellinstance = -1)
{
	// système de partoches pour l'explosion
	CParticleSystem * pPS = new CParticleSystem();
	CParticleParams cp;
	cp.iNbMax = 100 + t * 50;
	cp.fLife = 1500;
	cp.fLifeRandom = 0;
	cp.p3Pos.x = 10;
	cp.p3Pos.y = 10;
	cp.p3Pos.z = 10;
	cp.p3Direction.x = 0;
	cp.p3Direction.y = -10;
	cp.p3Direction.z = 0;
	cp.fAngle = DEG2RAD(360);
	cp.fSpeed = 130;
	cp.fSpeedRandom = 100;
	cp.p3Gravity.x = 0;
	cp.p3Gravity.y = 10;
	cp.p3Gravity.z = 0;
	cp.fFlash = 0;
	cp.fRotation = 16;

	cp.fStartSize = 5;
	cp.fStartSizeRandom = 10;


	cp.fEndSize = 0;
	cp.fEndSizeRandom = 2;

	if ((spellinstance >= 0) && (spells[spellinstance].caster == 0) && (cur_mr == 3))
	{
		cp.fStartSize = 20;
		cp.fSpeed = 13;
		cp.fSpeedRandom = 10;
		cp.fStartColorRandom[0] = 0;
		cp.fStartColorRandom[1] = 0;
		cp.fStartColorRandom[2] = 0;
		cp.fStartColorRandom[3] = 0;

		cp.fStartColor[0] = 0;
		cp.fStartColor[1] = 0;
		cp.fStartColor[2] = 0;
		cp.fStartColor[3] = 0;
		cp.fEndColor[0] = 255;
		cp.fEndColor[1] = 40;
		cp.fEndColor[2] = 120;
		cp.fEndColor[3] = 10;//55;
		pPS->SetTexture("graph\\particles\\(fx)_mr.bmp", 0, 500);
	}
	else
	{
		cp.fStartColorRandom[0] = 100;
		cp.fStartColorRandom[1] = 100;
		cp.fStartColorRandom[2] = 100;
		cp.fStartColorRandom[3] = 100;

		cp.fStartColor[0] = 110;
		cp.fStartColor[1] = 110;
		cp.fStartColor[2] = 110;
		cp.fStartColor[3] = 110;
		cp.fEndColor[0] = 0;
		cp.fEndColor[1] = 0;
		cp.fEndColor[2] = 120;
		cp.fEndColor[3] = 10;
		pPS->SetTexture("graph\\particles\\magicexplosion.bmp", 0, 500);
	}

	cp.fEndColorRandom[0] = 50;
	cp.fEndColorRandom[1] = 50;
	cp.fEndColorRandom[2] = 50;
	cp.fEndColorRandom[3] = 50;

	pPS->SetParams(cp);
	pPS->ulParticleSpawn = 0;


	EERIE_3D eP;
	eP.x = _ePos.x;
	eP.y = _ePos.y;
	eP.z = _ePos.z;

	pPS->SetPos(eP);
	pPS->Update(0);
	pPS->iParticleNbMax = 0;

	long id = GetFreeDynLight();

	if (id != -1)
	{
		DynLight[id].exist = 1;
		DynLight[id].intensity = 2.3f;
		DynLight[id].fallstart = 250.f;
		DynLight[id].fallend   = 420.f;

		if ((spellinstance >= 0) && (spells[spellinstance].caster == 0) && (cur_mr == 3))
		{
			DynLight[id].rgb.r = 1.f;
			DynLight[id].rgb.g = 0.3f;
			DynLight[id].rgb.b = .8f;
		}
		else
		{
			DynLight[id].rgb.r = 0.f;
			DynLight[id].rgb.g = 0.f;
			DynLight[id].rgb.b = .8f;
		}

		DynLight[id].pos.x = eP.x;
		DynLight[id].pos.y = eP.y;
		DynLight[id].pos.z = eP.z;
		DynLight[id].duration = 1500;
	}

	if (pParticleManager)
	{
		pParticleManager->AddSystem(pPS);
	}

	ARX_SOUND_PlaySFX(SND_SPELL_MM_HIT, &_ePos);
}

//-----------------------------------------------------------------------------
CMagicMissile::CMagicMissile() : CSpellFx()
{
	tex_mm = NULL;
	bExplo = false;
	bMove = true;
}

//-----------------------------------------------------------------------------
CMagicMissile::CMagicMissile(LPDIRECT3DDEVICE7 m_pd3dDevice) : CSpellFx()
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	SetDuration(2000);
	ulCurrentTime = ulDuration + 1;

	tex_mm = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_bandelette_blue.bmp");

	if (!smissile)
	{
		smissile  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_magic_missile\\fx_magic_missile.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(smissile);
	}

	smissile_count++;

	fColor[0] = 1;
	fColor[1] = 1;
	fColor[2] = 1;

	fColor1[0] = 1;
	fColor1[1] = 1;
	fColor1[2] = 1;
	bExplo = false;
	bMove = true;
}

//-----------------------------------------------------------------------------
CMagicMissile::~CMagicMissile()
{
	smissile_count--;

	if (smissile && (smissile_count <= 0))
	{
		smissile_count = 0;
		ReleaseEERIE3DObj(smissile);
		smissile = NULL;
	}

	if (this->lLightId != -1)
	{
		this->lLightId = -1;
	}

	ARX_SOUND_Stop(snd_loop);
}

//-----------------------------------------------------------------------------
void CMagicMissile::Create(EERIE_3D aeSrc, EERIE_3D angles)
{
	int i;
	EERIE_3D s, e;

	SetDuration(ulDuration);
	SetAngle(angles.b);

	Vector_Copy(&this->angles, &angles);
	eCurPos.x = eSrc.x = aeSrc.x;
	eCurPos.y = eSrc.y = aeSrc.y;
	eCurPos.z = eSrc.z = aeSrc.z;


	fSize = 1;
	bDone = true;

	s.x = eSrc.x;
	s.y = eSrc.y;
	s.z = eSrc.z;
	e.x = eSrc.x;
	e.y = eSrc.y;
	e.z = eSrc.z;

	i = 0;

	i = 40;
	e.x -= fBetaRadSin * 50 * i;
	e.y += sin(DEG2RAD(MAKEANGLE(this->angles.a))) * 50 * i;
	e.z += fBetaRadCos * 50 * i;

	pathways[0].sx = eSrc.x;
	pathways[0].sy = eSrc.y;
	pathways[0].sz = eSrc.z;
	pathways[5].sx = e.x;
	pathways[5].sy = e.y;
	pathways[5].sz = e.z;
	Split(pathways, 0, 5, 50, 0.5f);

	for (i = 0; i < 6; i++)
	{
		if (pathways[i].sy >= eSrc.y + 150)
		{
			pathways[i].sy = eSrc.y + 150;
		}
	}

	fTrail = 0;

	iLength = 50;
	fOneOnLength = 1.0f / (float) iLength;
	iBezierPrecision = BEZIERPrecision;
	fOneOnBezierPrecision = 1.0f / (float) iBezierPrecision;
	bExplo = false;
	bMove = true;

	ARX_SOUND_PlaySFX(SND_SPELL_MM_CREATE, &eCurPos);
	ARX_SOUND_PlaySFX(SND_SPELL_MM_LAUNCH, &eCurPos);
	snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MM_LOOP, &eCurPos, 1.0F, ARX_SOUND_PLAY_LOOPED);
}

//-----------------------------------------------------------------------------
void CMagicMissile::SetColor(float faRed, float faGreen, float faBlue)
{
	fColor[0] = faRed;
	fColor[1] = faGreen;
	fColor[2] = faBlue;
}

//-----------------------------------------------------------------------------
void CMagicMissile::SetColor1(float faRed, float faGreen, float faBlue)
{
	fColor1[0] = faRed;
	fColor1[1] = faGreen;
	fColor1[2] = faBlue;
}

//-----------------------------------------------------------------------------
void CMagicMissile::SetTTL(unsigned long aulTTL)
{
	unsigned long t = ulCurrentTime;
	ulDuration = min(ulCurrentTime + aulTTL, ulDuration);
	SetDuration(ulDuration);
	ulCurrentTime = t;

	// Light
	if (lLightId != -1)
	{
		lLightId = -1;
	}
}


//-----------------------------------------------------------------------------
void CMagicMissile::Update(unsigned long aulTime)
{
	ARX_SOUND_RefreshPosition(snd_loop, &eCurPos);

	ulCurrentTime += aulTime;
}

//-----------------------------------------------------------------------------
float CMagicMissile::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;
 
	EERIE_3D lastpos, newpos;
	EERIE_3D v;
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;
	EERIE_3D av;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	// Set Appropriate Renderstates -------------------------------------------
	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	// Set Texture ------------------------------------------------------------
	if (tex_mm && tex_mm->m_pddsSurface)
	{
		if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
			SETTC(m_pd3dDevice, NULL);
		else
			SETTC(m_pd3dDevice, tex_mm);
	}

	// ------------------------------------------------------------------------

	if (bMove)
	{
		fTrail = (ulCurrentTime * fOneOnDuration) * (iBezierPrecision + 2) * 5;
	}

	lastpos.x = pathways[0].sx;
	lastpos.y = pathways[0].sy;
	lastpos.z = pathways[0].sz;

	Vector_Copy(&newpos, &lastpos);

	for (i = 0; i < 5; i++)
	{
		int kp = i;
		int kpprec = (i > 0) ? kp - 1 : kp ;
		int kpsuiv = kp + 1 ;
		int kpsuivsuiv = (i < (5 - 2)) ? kpsuiv + 1 : kpsuiv;

		for (int toto = 1; toto < iBezierPrecision; toto++)
		{
			if (fTrail < i * iBezierPrecision + toto) break;

			float t = toto * fOneOnBezierPrecision;

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
			p0 = 0.5f * (val - pathways[kpprec].sy) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].sy - pathways[kp].sy) ;
			v.y = f0 * pathways[kp].sy + f1 * val + f2 * p0 + f3 * p1 ;

			val = pathways[kpsuiv].sz ;
			p0 = 0.5f * (val - pathways[kpprec].sz) ;
			p1 = 0.5f * (pathways[kpsuivsuiv].sz - pathways[kp].sz) ;
			v.z = f0 * pathways[kp].sz + f1 * val + f2 * p0 + f3 * p1 ;

			Vector_Copy(&newpos, &v);

			if (!((fTrail - (i * iBezierPrecision + toto)) > iLength))
			{
				float c;

				if (fTrail < iLength)
				{
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / fTrail);
				}
				else
				{
					c = 1.0f - ((fTrail - (i * iBezierPrecision + toto)) / (float)iLength);
				}

				float fsize = c;
				float alpha = c - 0.2f;

				if (alpha < 0.2f) alpha = 0.2f;

				c += frand2() * 0.1f;

				if (c < 0) c = 0;
				else if (c > 1) c = 1;

				int color = D3DRGB(c * fColor[0] * alpha, c * fColor[1] * alpha, c * fColor[2] * alpha);

				if (fsize < 0.5f)
					fsize = fsize * 2 * 3;
				else
					fsize = (1.0f - fsize + 0.5f) * 2 * (3 * 0.5f);

				float fs = fsize * 6 + rnd() * 0.3f;
				float fe = fsize * 6 + rnd() * 0.3f;
				Draw3DLineTex(m_pd3dDevice, lastpos, newpos, color, fs, fe);
			}

			EERIE_3D temp_vector;
			Vector_Copy(&temp_vector, &lastpos);
			Vector_Copy(&lastpos, &newpos);
			Vector_Copy(&newpos, &temp_vector);
		}
	}

	av.x = newpos.x - lastpos.x;
	av.y = newpos.y - lastpos.y;
	av.z = newpos.z - lastpos.z;

	float bubu = GetAngle(av.x, av.z, 0, 0);
	float bubu1 = GetAngle(av.x, av.y, 0, 0);

	Vector_Copy(&stitepos, &lastpos);

	stiteangle.b = -RAD2DEG(bubu);
	stiteangle.a = 0;
	stiteangle.g = -(RAD2DEG(bubu1));

	if (av.x < 0)
		stiteangle.g -= 90;

	if (av.x > 0)
		stiteangle.g += 90;

	if (stiteangle.g < 0)
		stiteangle.g += 360.0f;

	if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
	{
		stitecolor.r = 1.f;
		stitecolor.g = 0.f;
		stitecolor.b = 0.2f;
	}
	else
	{
		stitecolor.r = 0.3f;
		stitecolor.g = 0.3f;
		stitecolor.b = 0.5f;
	}

	Vector_Init(&stitescale, 1, 1, 1);
	{
		if ((smissile))
			DrawEERIEObjEx(m_pd3dDevice, smissile, &stiteangle, &stitepos, &stitescale, &stitecolor);
	}

	Vector_Copy(&eCurPos, &lastpos);

	return 1 - 0.5f * rnd();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMultiMagicMissile::CMultiMagicMissile(LPDIRECT3DDEVICE7 m_pd3dDevice, long nbmissiles) : CSpellFx()
{
	SetDuration(2000);
	uiNumber = nbmissiles;
	pTab = NULL;
	pTab = new CSpellFx*[uiNumber]();


	if (pTab)
	{
		for (UINT i = 0 ; i < uiNumber ; i++)
		{
			pTab[i] = NULL;
			pTab[i] = new CMagicMissile(m_pd3dDevice);
			pTab[i]->spellinstance = this->spellinstance;
		}
	}
}

//-----------------------------------------------------------------------------
CMultiMagicMissile::~CMultiMagicMissile()
{
	for (UINT i = 0 ; i < uiNumber ; i++)
	{
		if (pTab[i])
		{
			if (pTab[i]->lLightId != -1)
			{
				// no need to kill it because it's a duration light !
				pTab[i]->lLightId = -1;
			}

			delete pTab[i];
		}
	}

	delete [] pTab;
}

//-----------------------------------------------------------------------------
void CMultiMagicMissile::Create(EERIE_3D aePos, EERIE_3D angles)
{
	long lMax = 0;

	if (pTab)
	{

		float afAlpha = angles.a;
		float afBeta = angles.b;
		spells[spellinstance].hand_group = GetActionPointIdx(inter.iobj[spells[spellinstance].caster]->obj, "PRIMARY_ATTACH");

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
			EERIE_3D vector;
			vector.x = -EEsin(DEG2RAD(afBeta)) * EEcos(DEG2RAD(afAlpha)) * 60.f;
			vector.y = EEsin(DEG2RAD(afAlpha)) * 60.f;
			vector.z = EEcos(DEG2RAD(afBeta)) * EEcos(DEG2RAD(afAlpha)) * 60.f;

			if (spells[spellinstance].hand_group != -1)
			{
				aePos.x = spells[spellinstance].hand_pos.x + vector.x; 
				aePos.y = spells[spellinstance].hand_pos.y + vector.y; 
				aePos.z = spells[spellinstance].hand_pos.z + vector.z; 
			}
			else
			{
				aePos.x = player.pos.x - EEsin(DEG2RAD(afBeta)) + vector.x; 
				aePos.y = player.pos.y + vector.y; //;
				aePos.z = player.pos.z + EEcos(DEG2RAD(afBeta)) + vector.z; 
			}
		}
		else
		{
			afAlpha = 0;
			afBeta = inter.iobj[spells[spellinstance].caster]->angle.b;
			EERIE_3D vector;
			vector.x = -EEsin(DEG2RAD(afBeta)) * EEcos(DEG2RAD(afAlpha)) * 60;
			vector.y = EEsin(DEG2RAD(afAlpha)) * 60;
			vector.z = EEcos(DEG2RAD(afBeta)) * EEcos(DEG2RAD(afAlpha)) * 60;

			if (spells[spellinstance].hand_group != -1)
			{
				aePos.x = spells[spellinstance].hand_pos.x + vector.x; 
				aePos.y = spells[spellinstance].hand_pos.y + vector.y; 
				aePos.z = spells[spellinstance].hand_pos.z + vector.z; 
			}
			else
			{
				aePos.x = inter.iobj[spells[spellinstance].caster]->pos.x + vector.x; 
				aePos.y = inter.iobj[spells[spellinstance].caster]->pos.y + vector.y; 
				aePos.z = inter.iobj[spells[spellinstance].caster]->pos.z + vector.z; 
			}

			INTERACTIVE_OBJ * io = inter.iobj[spells[spellinstance].caster];

			if (ValidIONum(io->targetinfo))
			{
				EERIE_3D * p1 = &spells[spellinstance].caster_pos;
				EERIE_3D * p2 = &inter.iobj[io->targetinfo]->pos;
				afAlpha = -(RAD2DEG(GetAngle(p1->y, p1->z, p2->y, p2->z + TRUEDistance2D(p2->x, p2->z, p1->x, p1->z)))); //alpha entre orgn et dest;
			}
			else if (ValidIONum(spells[spellinstance].target))
			{
				EERIE_3D * p1 = &spells[spellinstance].caster_pos;
				EERIE_3D * p2 = &inter.iobj[spells[spellinstance].target]->pos;
				afAlpha = -(RAD2DEG(GetAngle(p1->y, p1->z, p2->y, p2->z + TRUEDistance2D(p2->x, p2->z, p1->x, p1->z)))); //alpha entre orgn et dest;
			}
		}

		for (UINT i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				EERIE_3D angles;
				angles.a = afAlpha;
				angles.b = afBeta;
				angles.g = 0;

				if (i > 0)
				{
					angles.a += frand2() * 4.0f;
					angles.b += frand2() * 6.0f;
				}

				pTab[i]->Create(aePos, angles);  

				float	fTime	= ulDuration + frand2() * 1000.0f;
				ARX_CHECK_LONG(fTime);
				long	lTime	= ARX_CLEAN_WARN_CAST_LONG(fTime);

				lTime		= max(1000, lTime);
				lMax		= max(lMax, lTime);

				CMagicMissile * pMM = (CMagicMissile *)pTab[i];

				pMM->SetDuration(lTime);

				if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
				{
					pMM->SetColor(0.9f, 0.2f, 0.5f);
					pMM->SetColor1(0.9f, 0.2f, 0.5f);
				}
				else
				{
					pMM->SetColor(0.9f + rnd() * 0.1f, 0.9f + rnd() * 0.1f, 0.7f + rnd() * 0.3f);
					pMM->SetColor1(0.9f + rnd() * 0.1f, 0.9f + rnd() * 0.1f, 0.7f + rnd() * 0.3f);
				}

				pTab[i]->lLightId = GetFreeDynLight();

				if (pTab[i]->lLightId != -1)
				{
					EERIE_LIGHT * el = &DynLight[pTab[i]->lLightId];
					el->exist		= 1;
					el->intensity	= 0.7f + 2.3f;
					el->fallend		= 190.f;
					el->fallstart	= 80.f;

					if ((spells[spellinstance].caster == 0) && (cur_mr == 3))
					{
						el->rgb.r = 1;
						el->rgb.g = 0.3f;
						el->rgb.b = 0.8f;
					}
					else
					{
						el->rgb.r = 0;
						el->rgb.g = 0;
						el->rgb.b = 1;
					}

					el->pos.x	 = pMM->eSrc.x;
					el->pos.y	 = pMM->eSrc.y;
					el->pos.z	 = pMM->eSrc.z;
					el->duration = 300;
				}

			}
		}
	}

	SetDuration(lMax + 1000);
}

//-----------------------------------------------------------------------------
void CMultiMagicMissile::CheckCollision(float _fPlayer_Magic_Level)
{
	if (pTab)
	{
		for (UINT i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				CMagicMissile * pMM = (CMagicMissile *) pTab[i];

				if (pMM->bExplo == false)
				{
					EERIE_SPHERE sphere;
					sphere.origin.x = pMM->eCurPos.x;
					sphere.origin.y = pMM->eCurPos.y;
					sphere.origin.z = pMM->eCurPos.z;
					sphere.radius	= 10.f;

					if ((spellinstance != -1) && (CheckAnythingInSphere(&sphere, spells[spellinstance].caster, CAS_NO_SAME_GROUP)))
					{
 

						EERIE_3D eE3D;
						eE3D.x = pMM->eCurPos.x;
						eE3D.y = pMM->eCurPos.y;
						eE3D.z = pMM->eCurPos.z;
						LaunchMagicMissileExplosion(pMM->eCurPos, 0, spellinstance);
						ARX_NPC_SpawnAudibleSound(&pMM->eCurPos, inter.iobj[spells[spellinstance].caster]);

						pMM->SetTTL(1000);
						pMM->bExplo = true;
						pMM->bMove  = false;

						if (pMM->lLightId != -1)
						{
							pMM->lLightId = -1;
						}

						long ttt = ARX_DAMAGES_GetFree();

						if (ttt != -1)
						{
							damages[ttt].pos.x	= pMM->eCurPos.x;
							damages[ttt].pos.y	= pMM->eCurPos.y;
							damages[ttt].pos.z	= pMM->eCurPos.z;
							damages[ttt].radius	= 80.f;
							damages[ttt].damages = (4 + spells[spellinstance].caster_level * DIV5) * .8f; 
							damages[ttt].area	= DAMAGE_FULL;
							damages[ttt].duration = -1;
							damages[ttt].source	= spells[spellinstance].caster;
							damages[ttt].flags	= DAMAGE_FLAG_DONT_HURT_SOURCE;
							damages[ttt].type	= DAMAGE_TYPE_MAGICAL;
							damages[ttt].exist	= TRUE;
						}

						EERIE_RGB rgb;
						rgb.r = 0.3f;
						rgb.g = 0.3f;
						rgb.b = 0.45f;
						ARX_PARTICLES_Add_Smoke(&pMM->eCurPos, 0, 6, &rgb);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CMultiMagicMissile::Update(unsigned long _ulTime)
{
	if (pTab)
	{
		for (UINT i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				pTab[i]->Update(_ulTime);
			}
		}
	}
}

//-----------------------------------------------------------------------------
float CMultiMagicMissile::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	long nbmissiles	= 0;
 

	if (pTab)
	{
		for (UINT i = 0 ; i < uiNumber ; i++)
		{
			if (pTab[i])
			{
				float fa = pTab[i]->Render(m_pd3dDevice);

				CMagicMissile * pMM = (CMagicMissile *) pTab[i];

				if (pMM->lLightId != -1)
				{
					EERIE_LIGHT * el	= &DynLight[pMM->lLightId];
					el->intensity		= 0.7f + 2.3f * fa;
					el->pos.x			= pMM->eCurPos.x;
					el->pos.y			= pMM->eCurPos.y;
					el->pos.z			= pMM->eCurPos.z;
					el->time_creation	= ARXTimeUL();
				}

				if (pMM->bMove) nbmissiles++;
			}
		}
	}

	if (nbmissiles == 0) return -1;

	return 1;
}

//-----------------------------------------------------------------------------
// IGNIT
//-----------------------------------------------------------------------------
CIgnit::CIgnit()
{
}

//-----------------------------------------------------------------------------
CIgnit::~CIgnit()
{
	this->Kill();
}

//-----------------------------------------------------------------------------
void CIgnit::Kill(void)
{
	int nb = this->nblight;

	while (nb--)
	{
		int id = this->tablight[nb].idl;

		if (ValidDynLight(id))
			DynLight[id].exist = 0;

		this->tablight[nb].idl = -1;
	}

	this->nblight = 0;
}

//-----------------------------------------------------------------------------
void CIgnit::Create(EERIE_3D * posc, float perim, int speed)
{
	this->pos = *posc;
	this->perimetre = perim;
	this->nblight = 0;
	this->duration = speed;
	this->currduration = 0;
	this->key = 0;

	int	nb = 256;

	while (nb--)
	{
		this->tablight[nb].actif = 0;
		this->tablight[nb].idl = -1;
	}

	this->ChangeTexture(MakeTCFromFile("Graph\\Particles\\fire_hit.bmp"));
	this->ChangeRGBMask(1.f, 1.f, 1.f, RGBA_MAKE(255, 200, 0, 255));
}

//-----------------------------------------------------------------------------
void CIgnit::Action(int aiMode)
{


	ARX_CHECK_SHORT(aiMode);
	short sMode = ARX_CLEAN_WARN_CAST_SHORT(aiMode);

	for (int i = 0; i < nblight; i++)
	{
		GLight[tablight[i].iLightNum]->status = sMode;


		if (aiMode == 1)
		{
			ARX_SOUND_PlaySFX(SND_SPELL_IGNITE, &spells[spellinstance].caster_pos);
		}
		else if (aiMode == 0)
		{
			ARX_SOUND_PlaySFX(SND_SPELL_DOUSE, &spells[spellinstance].caster_pos);
		}
	}
}

//-----------------------------------------------------------------------------
void CIgnit::AddLight(int aiLight)
{
	if (ARXPausedTimer)  return;

	if (this->nblight > 255) return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight.x = GLight[aiLight]->pos.x;
	this->tablight[this->nblight].poslight.y = GLight[aiLight]->pos.y;
	this->tablight[this->nblight].poslight.z = GLight[aiLight]->pos.z;

	this->tablight[this->nblight].idl = GetFreeDynLight();

	if (this->tablight[this->nblight].idl > 0)
	{
		int id = this->tablight[this->nblight].idl;
		EERIE_LIGHT * el = &DynLight[id];
		el->exist = 1;
		el->intensity = 0.7f + 2.f * rnd();
		el->fallend = 400.f;
		el->fallstart = 300.f;
		el->rgb.r = this->r;
		el->rgb.g = this->g;
		el->rgb.b = this->b;
		el->pos.x = this->tablight[this->nblight].poslight.x;
		el->pos.y = this->tablight[this->nblight].poslight.y;
		el->pos.z = this->tablight[this->nblight].poslight.z;
	}

	this->nblight++;
}

//-----------------------------------------------------------------------------
void CIgnit::Update(unsigned long _ulTime) 
{
	float	a;
	int		nb;

	if (this->currduration >= this->duration)
	{
		this->key++;
	}

	switch (this->key)
	{
		case 0:
			a = (((float)this->currduration)) / ((float)this->duration);

			if (a >= 1.f) a = 1.f;

			nb = this->nblight;

			while (nb--)
			{
				if (this->tablight[nb].actif)
				{
					this->tablight[nb].posfx.x = this->pos.x + a * (this->tablight[nb].poslight.x - this->pos.x);
					this->tablight[nb].posfx.y = this->pos.y + a * (this->tablight[nb].poslight.y - this->pos.y);
					this->tablight[nb].posfx.z = this->pos.z + a * (this->tablight[nb].poslight.z - this->pos.z);

					int id = this->tablight[nb].idl;

					if (id > 0)
					{
						DynLight[id].intensity = 0.7f + 2.f * rnd();
						DynLight[id].pos.x = this->tablight[nb].posfx.x;
						DynLight[id].pos.y = this->tablight[nb].posfx.y;
						DynLight[id].pos.z = this->tablight[nb].posfx.z;
					}
				}
			}

			this->interp = a;
			break;
	}

	if (!ARXPausedTimer) this->currduration += _ulTime;
}

//-----------------------------------------------------------------------------
void CDoze::AddLightDoze(int aiLight)
{
	if (ARXPausedTimer)  return;

	if (this->nblight > 255) return;

	this->tablight[this->nblight].actif = 1;
	this->tablight[this->nblight].iLightNum = aiLight;
	this->tablight[this->nblight].poslight.x = GLight[aiLight]->pos.x;
	this->tablight[this->nblight].poslight.y = GLight[aiLight]->pos.y;
	this->tablight[this->nblight].poslight.z = GLight[aiLight]->pos.z;
	this->tablight[this->nblight].idl = -1;

	this->nblight++;
}

//-----------------------------------------------------------------------------
float CIgnit::Render(LPDIRECT3DDEVICE7 device)
{
	int		nb;

	switch (this->key)
	{
		case 0:

			if (this->currduration > this->duration) this->key++;

			if (!ARXPausedTimer)
			{
				float unsuri = (1.f - this->interp);
				nb = this->nblight;

				while (nb--)
				{
					if ((this->tablight[nb].actif) && (rnd() > .5f))
					{
						ARX_GenereSpheriqueEtincelles(&this->tablight[nb].posfx, rnd() * 20.f * unsuri, this->tp, this->r, this->g, this->b, this->mask);
					}
				}
			}

			break;
		default:
			break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
//							PORTALS
//-----------------------------------------------------------------------------
void Split(EERIE_3D * v, int a, int b, float yo)
{
	if (a != b)
	{
		int i = (int)((a + b) * 0.5f);

		if ((i != a) && (i != b))
		{
			v[i].x = (v[a].x + v[b].x) * 0.5f + yo * frand2(); 
			v[i].y = (v[a].y + v[b].y) * 0.5f; 
			v[i].z = (v[a].z + v[b].z) * 0.5f + yo * frand2();
			Split(v, a, i, yo * 0.7f);
			Split(v, i, b, yo * 0.7f);
		}
	}
}

//-----------------------------------------------------------------------------
void GenereArcElectrique(EERIE_3D * pos, EERIE_3D * end, EERIE_3D * tabdef, int nbseg)
{
	tabdef[0].x = pos->x;
	tabdef[0].y = pos->y;
	tabdef[0].z = pos->z;

	tabdef[nbseg-1].x = end->x;
	tabdef[nbseg-1].y = end->y;
	tabdef[nbseg-1].z = end->z;

	Split(tabdef, 0, nbseg - 1, 20);
}

//-----------------------------------------------------------------------------
void DrawArcElectrique(LPDIRECT3DDEVICE7 m_pd3dDevice, EERIE_3D * tabdef, int nbseg, TextureContainer * tex, float fBeta, int tsp)
{
	D3DTLVERTEX v[4];
	D3DTLVERTEX v2[4];

	long i;

	//-------------------------------------------------------------------------
	// rendu
	//	SETTC(m_pd3dDevice,NULL);
	SETCULL(m_pd3dDevice, D3DCULL_NONE);

	if (tex && tex->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex);
	}

	v2[0].color = v2[1].color = v2[2].color = v2[3].color = RGBA_MAKE(tsp, tsp, tsp, 255);

	float xx;
	float zz;

	for (i = 0; i < nbseg - 2; i++)
	{
		EERIE_3D astart;

		astart.x = tabdef[i].x;
		astart.y = tabdef[i].y;
		astart.z = tabdef[i].z;

		zz = 5; // size
		xx = (float)(5 * cos(DEG2RAD(fBeta)));

		float ax = tabdef[i+1].x;
		float ay = tabdef[i+1].y;
		float az = tabdef[i+1].z;

		// version 2 faces
		v2[0].tu = 0;
		v2[0].tv = 0;
		v2[1].tu = 1;
		v2[1].tv = 0;
		v2[2].tu = 1;
		v2[2].tv = 1;
		v2[3].tu = 0;
		v2[3].tv = 1;

		v[0].sx = astart.x;
		v[0].sy = astart.y + zz;
		v[0].sz = astart.z;

		v[1].sx = astart.x;
		v[1].sy = astart.y - zz;
		v[1].sz = astart.z;

		v[2].sx = ax;
		v[2].sy = ay - zz;
		v[2].sz = az;

		v[3].sx = ax;
		v[3].sy = ay + zz;
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

		zz *= (float) sin(DEG2RAD(fBeta));

		v[0].sx = astart.x + xx;
		v[0].sy = astart.y;
		v[0].sz = astart.z + zz;

		v[1].sx = astart.x - xx;
		v[1].sy = astart.y;
		v[1].sz = astart.z - zz;

		v[2].sx = ax - xx;
		v[2].sy = ay;
		v[2].sz = az - zz;

		v[3].sx = ax + xx;
		v[3].sy = ay;
		v[3].sz = az + zz;

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
	}
}

/*--------------------------------------------------------------------------*/
CPortal::~CPortal()
{
	if (this->sphereind) free((void *)this->sphereind);

	if (this->spherevertex) free((void *)this->spherevertex);

	if (this->sphered3d) free((void *)this->sphered3d);

	int nb = 256;

	while (nb--)
	{
		if (this->tabeclair[nb].seg) free((void *)this->tabeclair[nb].seg);
	}
}

//-----------------------------------------------------------------------------
void CPortal::AddNewEclair(EERIE_3D * endpos, int nbseg, int duration, int numpt)
{
	if (ARXPausedTimer) return;

	int	nb = 256;

	if ((this->nbeclair > 255) || (nbseg > 256)) return;


	short sNbSeg = ARX_CLEAN_WARN_CAST_SHORT(nbseg);

	while (nb--)
	{
		if (!this->tabeclair[nb].actif)
		{
			this->nbeclair++;
			this->tabeclair[nb].actif = 1;
			this->tabeclair[nb].duration = duration;
			this->tabeclair[nb].currduration = 0;
			this->tabeclair[nb].nbseg = sNbSeg;
			this->tabeclair[nb].numpt = numpt;

			GenereArcElectrique(&this->pos, endpos, this->tabeclair[nb].seg, this->tabeclair[nb].nbseg);
			break;
		}



	}
}
/*--------------------------------------------------------------------------*/
void CPortal::DrawAllEclair(LPDIRECT3DDEVICE7 device)
{
	int nb = 256;

	while (nb--)
	{
		if (this->tabeclair[nb].actif)
		{
			float a;

			a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if (a < 0.f) a = 0.f;

			DrawArcElectrique(device, this->tabeclair[nb].seg, this->tabeclair[nb].nbseg, this->te, rnd() * 360.f, (int)(255.f * a));

			if (!ARXPausedTimer) this->tabeclair[nb].currduration += this->currframe;

			if (this->tabeclair[nb].currduration >= this->tabeclair[nb].duration)
			{
				this->tabeclair[nb].actif = 0;
				this->nbeclair--;
			}
		}
	}
}
/*--------------------------------------------------------------------------*/
void CPortal::Update(unsigned long _ulTime)
{
	float a;

	switch (this->key)
	{
		case 0:
			a = (float)this->currduration / (float)this->duration;

			if (a > 1.f)
			{
				a = 1.f;
				this->key++;
			}

			this->pos.x = this->sphereposdep.x + (this->sphereposend.x - this->sphereposdep.x) * a;
			this->pos.y = this->sphereposdep.y + (this->sphereposend.y - this->sphereposdep.y) * a;
			this->pos.z = this->sphereposdep.z + (this->sphereposend.z - this->sphereposdep.z) * a;
			this->spherealpha = a * .5f;

			if (!ARXPausedTimer) this->currduration += _ulTime;

			break;
		case 1:
			this->spherealpha = 0.5f + rnd();

			if (this->spherealpha > 1.f) this->spherealpha = 1.f;

			this->spherealpha *= .25f;

			//getion eclair dans boule
			this->currframe = _ulTime;

			if (!ARXPausedTimer) this->timeneweclair -= _ulTime;

			if (this->timeneweclair <= 0)
			{
				this->timeneweclair = (int)(100.f + rnd() * 200.f);

				EERIE_3D endpos;
				int	numpt = (int)(rnd() * (float)(this->spherenbpt - 1));
				endpos.x = this->spherevertex[numpt].x + this->pos.x;
				endpos.y = this->spherevertex[numpt].y + this->pos.y;
				endpos.z = this->spherevertex[numpt].z + this->pos.z;

				this->AddNewEclair(&endpos, 32, (int)(1000.f + 1000.f * rnd()), numpt);
			}

			break;
	}

	if (this->lLightId >= 0)
	{
		DynLight[this->lLightId].pos.x = this->pos.x;
		DynLight[this->lLightId].pos.y = this->pos.y;
		DynLight[this->lLightId].pos.z = this->pos.z;
		DynLight[this->lLightId].intensity = 0.7f + 2.f * rnd();
	}
}
/*--------------------------------------------------------------------------*/
float CPortal::Render(LPDIRECT3DDEVICE7 device)
{
	SETALPHABLEND(device, TRUE);
	SETZWRITE(device, FALSE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	//calcul sphere
	int			nb = this->spherenbpt;
	D3DTLVERTEX * v = this->sphered3d, d3dvs;
	EERIE_3D	* pt = this->spherevertex;
	int col = RGBA_MAKE(0, (int)(200.f * this->spherealpha), (int)(255.f * this->spherealpha), 255);

	while (nb)
	{
		d3dvs.sx = pt->x + this->pos.x;	//pt du bas
		d3dvs.sy = pt->y + this->pos.y;
		d3dvs.sz = pt->z + this->pos.z;
		EE_RTP(&d3dvs, v);

		if (!ARXPausedTimer) v->color = col;

		v++;
		pt++;
		nb--;
	}

	//update les couleurs aux impacts
	nb = 256;

	while (nb--)
	{
		if (this->tabeclair[nb].actif)
		{
			float a;

			a = 1.f - ((float)this->tabeclair[nb].currduration / (float)this->tabeclair[nb].duration);

			if (a < 0.f) a = 0.f;

			if (this->tabeclair[nb].numpt >= 0)
			{
				int r = (int)((0.f + (255.f - 0.f) * a) * this->spherealpha * 3.f);

				if (r > 255) r = 255;

				int g = (int)((200.f + (255.f - 200.f) * a) * this->spherealpha * 3.f);

				if (g > 255) g = 255;

				int b = (int)(255.f * this->spherealpha * 3.f);

				if (b > 255) b = 255;

				if (!ARXPausedTimer) this->sphered3d[this->tabeclair[nb].numpt].color = RGBA_MAKE(r, g, b, 255);
			}

		}
	}


	//affichage de la sphere back
	SETCULL(device, D3DCULL_CW);
	device->SetTexture(0, NULL);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->sphered3d, this->spherenbpt, (unsigned short *)this->sphereind, this->spherenbfaces * 3, 0);

	//affichage eclair
	this->DrawAllEclair(device);

	//affichage des particules à l'interieur
	if (rnd() > .25f)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist = 1;
			particle[j].zdec = 0;

			float a = rnd() * 360.f;
			float b = rnd() * 360.f;
			float rr = this->r * (rnd() + .25f) * 0.05f;

			particle[j].ov.x	=	this->pos.x;
			particle[j].ov.y	=	this->pos.y;
			particle[j].ov.z	=	this->pos.z;
			particle[j].move.x	=	rr * EEsin(DEG2RAD(a)) * EEcos(DEG2RAD(b));
			particle[j].move.y	=	rr * EEcos(DEG2RAD(a));
			particle[j].move.z	=	rr * EEsin(DEG2RAD(a)) * EEsin(DEG2RAD(b));
			particle[j].siz		=	10.f;
			particle[j].tolive	=	1000 + (unsigned long)(float)(rnd() * 1000.f);
			particle[j].scale.x	=	1.f;
			particle[j].scale.y	=	1.f;
			particle[j].scale.z	=	1.f;
			particle[j].timcreation	=	lARXTime;
			particle[j].tc		=	tp;
			particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].fparam	=	0.0000001f;
			particle[j].r		=	1.f;
			particle[j].g		=	1.f;
			particle[j].b		=	1.f;
		}
	}

	//affichage de la sphere front
	SETCULL(device, D3DCULL_CCW);
	device->SetTexture(0, NULL);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->sphered3d, this->spherenbpt, (unsigned short *)this->sphereind, this->spherenbfaces * 3, 0);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
	SETALPHABLEND(device, FALSE);
	SETCULL(device, D3DCULL_NONE);
	SETZWRITE(device, TRUE);

	return 0;
}
/*--------------------------------------------------------------------------*/
