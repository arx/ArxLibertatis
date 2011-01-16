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
// CSpellFx_Lvl10.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 10
//
// Refer to:
//		CSpellFx.h
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "danae.h"

#include <EERIEMath.h>
#include <EERIEDraw.h>

#include "ARX_Spells.h"
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl07.h"
#include "ARX_SpellFx_Lvl09.h"
#include "ARX_SpellFx_Lvl10.h"
#include "ARX_Particles.h"
#include "ARX_Time.h"


#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CMassLightning::CMassLightning(LPDIRECT3DDEVICE7 m_pd3dDevice, long nbmissiles)
{
	SetDuration(2000);
	pTab = new CLightning*[10];
	number = __min(10, nbmissiles);

	TextureContainer * pTex = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_lightning_red.bmp");

	for (int i = 0; i < number; i++)
	{
		pTab[i] = new CLightning(pTex);
		pTab[i]->fDamage = 2;
	}
}

//-----------------------------------------------------------------------------
void CMassLightning::Create(EERIE_3D aePos, float afBeta = 0)
{
	long lMax = 0;
	EERIE_3D eTarget;
	float ft = 360.0f / (float)number;

	for (int i = 0; i < number; i++)
	{
		eTarget.x = aePos.x - EEsin(DEG2RAD(i * ft)) * 500.0f;
		eTarget.y = aePos.y;
		eTarget.z = aePos.z + EEcos(DEG2RAD(i * ft)) * 500.0f;
		pTab[i]->Create(aePos, eTarget, 0);
		long lTime = (long)(ulDuration + rnd() * 5000.0f);
		pTab[i]->SetDuration(lTime);
		lMax = max(lMax, lTime);
		pTab[i]->spellinstance = this->spellinstance;
		pTab[i]->SetColor1(1.f, 0.75f, 0.75f); // milieu
		pTab[i]->SetColor2(0.3f, 0.f, 0.f); // extrémités
	}

	SetDuration(lMax + 1000);
}

//-----------------------------------------------------------------------------
void CMassLightning::Update(unsigned long _ulTime)
{
	for (int i = 0; i < number; i++)
	{
		pTab[i]->Update(_ulTime);
	}
}

//-----------------------------------------------------------------------------
float CMassLightning::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	for (int i = 0; i < number; i++)
	{
		pTab[i]->Render(m_pd3dDevice);
	}

	return 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CControlTarget::CControlTarget(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(8000);
	ulCurrentTime = ulDuration + 1;

	tex_mm = MakeTCFromFile("Graph\\Obj3D\\textures\\(fx)_ctrl_target.bmp");

	fColor[0] = 1;
	fColor[1] = 1;
	fColor[2] = 0;

	fColor1[0] = 0.8f;
	fColor1[1] = 0.6f;
	fColor1[2] = 0.2f;
}
//-----------------------------------------------------------------------------
void CControlTarget::Create(EERIE_3D aeSrc, float afBeta)
{
	int i;

	SetDuration(ulDuration);

	eSrc.x = aeSrc.x;
	eSrc.y = aeSrc.y;
	eSrc.z = aeSrc.z;

	fBeta = afBeta;
	fBetaRad = DEG2RAD(fBeta);
	fBetaRadCos = (float) cos(fBetaRad);
	fBetaRadSin = (float) sin(fBetaRad);

	fSize = 1;
	bDone = true;

	eTarget.x = eSrc.x - fBetaRadSin * 1000;
	eTarget.y = eSrc.y + 100;
	eTarget.z = eSrc.z + fBetaRadCos * 1000;

	for (i = 1; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			eTarget.x = inter.iobj[i]->pos.x;
			eTarget.y = inter.iobj[i]->pos.y;
			eTarget.z = inter.iobj[i]->pos.z;
		}
	}

	end = 20 - 1;
	v1a[0].sx = eSrc.x;
	v1a[0].sy = eSrc.y + 100;
	v1a[0].sz = eSrc.z;
	v1a[end].sx = eTarget.x;
	v1a[end].sy = eTarget.y;
	v1a[end].sz = eTarget.z;

	EERIE_3D s, e, h;
	s.x = v1a[0].sx;
	s.y = v1a[0].sy;
	s.z = v1a[0].sz;
	e.x = v1a[end].sx;
	e.y = v1a[end].sy;
	e.z = v1a[end].sz;
	//	GenereArcElectrique(&s,&e,v1a,end,0);

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

	pathways[0].sx = eSrc.x;
	pathways[0].sy = eSrc.y + 100;
	pathways[0].sz = eSrc.z;
	pathways[9].sx = eTarget.x;
	pathways[9].sy = eTarget.y;
	pathways[9].sz = eTarget.z;
	Split(pathways, 0, 9, 150);

	for (i = 0; i < 9; i++)
	{
		if (pathways[i].sy >= eSrc.y + 150)
		{
			pathways[i].sy = eSrc.y + 150;
		}
	}

	fTrail = 0;
}
//-----------------------------------------------------------------------------
void CControlTarget::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CControlTarget::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	SETCULL(m_pd3dDevice, D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	//----------------------------
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
	m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	if (tex_mm && tex_mm->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_mm);
	}

	// -------------------
	fTrail += 1;

	if (fTrail >= 300) fTrail = 0;

	int n = BEZIERPrecision;
	float delta = 1.0f / n;

	fTrail = (ulCurrentTime * fOneOnDuration) * 9 * (n + 2);

	EERIE_3D lastpos, newpos;
	EERIE_3D v;

	int arx_check_init = -1;
	newpos.x = 0;
	newpos.y = 0;
	newpos.z = 0;


	lastpos.x = pathways[0].sx;
	lastpos.y = pathways[0].sy;
	lastpos.z = pathways[0].sz;

	for (i = 0; i < 9; i++)
	{
		int kp		= i;
		int kpprec	= (i > 0) ? kp - 1 : kp ;
		int kpsuiv	= kp + 1 ;
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
			p0 = 0.5f * (val - pathways[kpprec].sy) ;
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

				int j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist = 1;
					particle[j].zdec = 0;

					particle[j].ov.x		= lastpos.x;
					particle[j].ov.y		= lastpos.y;
					particle[j].ov.z		= lastpos.z;
					particle[j].move.x		= 0;
					particle[j].move.y		= 0;
					particle[j].move.z		= 0;
					particle[j].siz			= 5 * c;
					particle[j].tolive		= 10 + (unsigned long)(float)(rnd() * 100.f);
					particle[j].scale.x 	= 1;
					particle[j].scale.y 	= 1;
					particle[j].scale.z 	= 1;
					particle[j].timcreation = lARXTime;
					particle[j].tc			= tex_mm;
					particle[j].special 	= FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam		= 0.0000001f;
					particle[j].r			= c;
					particle[j].g			= c;
					particle[j].b			= c;
				}
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

			int j = ARX_PARTICLES_GetFree();

			if ((j != -1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist	= 1;
				particle[j].zdec	= 0;

				particle[j].ov.x	= lastpos.x;
				particle[j].ov.y	= lastpos.y;
				particle[j].ov.z	= lastpos.z;
				particle[j].move.x	= 0;
				particle[j].move.y	= 0;
				particle[j].move.z	= 0;
				particle[j].siz		= 5;
				particle[j].tolive	= 10 + (unsigned long)(float)(rnd() * 100.f);
				particle[j].scale.x = 1;
				particle[j].scale.y = 1;
				particle[j].scale.z = 1;
				particle[j].timcreation = lARXTime;
				particle[j].tc		= tex_mm;
				particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				particle[j].fparam	= 0.0000001f;
				particle[j].r		= 0.1f;
				particle[j].g		= 0.1f;
				particle[j].b		= 0.1f;
			}
		}
	}

	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	EERIE_3D av;
	ARX_CHECK_NOT_NEG(arx_check_init);
	av.x = lastpos.x - newpos.x;
	av.y = lastpos.y - newpos.y;
	av.z = lastpos.z - newpos.z;

	TRUEVector_Normalize(&av);

	float bubu = GetAngle(av.x, av.z, 0, 0);
	float bubu1 = GetAngle(av.x, av.y, 0, 0);

	stitepos.x = lastpos.x;
	stitepos.y = lastpos.y;
	stitepos.z = lastpos.z;
	stiteangle.b = 180 - RAD2DEG(bubu);
	stiteangle.a = 0;
	stiteangle.g = 90 - RAD2DEG(bubu1);
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 1;
	stitescale.y = 1;
	stitescale.z = 1;

	eCurPos.x = lastpos.x;
	eCurPos.y = lastpos.y;
	eCurPos.z = lastpos.z;

	return 1;
}

//---------------------------------------------------------------------
CMassIncinerate::~CMassIncinerate()
{
}

//---------------------------------------------------------------------
void CMassIncinerate::Create(EERIE_3D aePos, float afBeta = 0)
{
	aePos.y += 150.0f;

	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Create(aePos, i * 36.f);
	}
}

//---------------------------------------------------------------------
void CMassIncinerate::Update(unsigned long _ulTime)
{
	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Update(_ulTime);
	}

}

//---------------------------------------------------------------------
float CMassIncinerate::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	for (int i = 0; i < 10; i++)
	{
		pTabIncinerate[i]->Render(m_pd3dDevice);
	}

	return 0;
}

