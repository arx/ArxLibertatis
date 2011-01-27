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
// CSpellFx_Lvl04.cpp
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Spell FX Level 04
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
#include "ARX_CSpellFx.h"
#include "ARX_SpellFx_Lvl05.h"
#include "ARX_SpellFx_Lvl04.h"
#include "ARX_Particles.h"
#include "ARX_CParticles.h"
#include "ARX_spells.h"
#include "ARX_Time.h"


//-----------------------------------------------------------------------------
CBless::CBless(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(4000);
	ulCurrentTime = ulDuration + 1;

	tex_p1 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");
	tex_sol = MakeTCFromFile("Graph\\particles\\(Fx)_pentagram_bless.bmp");
}

//-----------------------------------------------------------------------------
void CBless::Create(EERIE_3D _eSrc, float _fBeta)
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

void CBless::Set_Angle(EERIE_3D angle)
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
float CBless::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y - 5;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETCULL(_pD3DDevice, D3DCULL_NONE);
	SETZWRITE(_pD3DDevice, FALSE);
	SETALPHABLEND(_pD3DDevice, TRUE);

	D3DTLVERTEX v[4];
	D3DTLVERTEX v3[4];

	float ff = ((float)spells[spellinstance].caster_level + 10) * 6.f;
	float fBetaRadCos = (float) cos(DEG2RAD(MAKEANGLE(player.angle.b))) * ff;
	float fBetaRadSin = (float) sin(DEG2RAD(MAKEANGLE(player.angle.b))) * ff;

	unsigned long color = D3DRGB(1, 1, 1);

	v[0].sx = x - fBetaRadCos - fBetaRadSin;
	v[0].sy = y;
	v[0].sz = z - fBetaRadSin + fBetaRadCos;
	v[1].sx = x + fBetaRadCos - fBetaRadSin;
	v[1].sy = y;
	v[1].sz = z + fBetaRadSin + fBetaRadCos;
	v[2].sx = x - fBetaRadCos + fBetaRadSin;
	v[2].sy = y;
	v[2].sz = z - fBetaRadSin - fBetaRadCos;
	v[3].sx = x + fBetaRadCos + fBetaRadSin;
	v[3].sy = y;
	v[3].sz = z + fBetaRadSin - fBetaRadCos;

	v3[0].color = color;
	v3[1].color = color;
	v3[2].color = color;
	v3[3].color = color;

	if (tex_sol && tex_sol->m_pddsSurface)
	{
		SETTC(_pD3DDevice, tex_sol);
	}

	v3[0].tu = 0;
	v3[0].tv = 0;
	v3[1].tu = 1.f;
	v3[1].tv = 0;
	v3[2].tu = 0;
	v3[2].tv = 1.f;
	v3[3].tu = 1.f;
	v3[3].tv = 1.f;

	EE_RT2(&v[0], &v3[0]);
	EE_RT2(&v[1], &v3[1]);
	EE_RT2(&v[2], &v3[2]);
	EE_RT2(&v[3], &v3[3]);
	ARX_DrawPrimitive_SoftClippZ(&v3[0],
	                             &v3[1],
	                             &v3[2]);
	ARX_DrawPrimitive_SoftClippZ(&v3[1],
	                             &v3[2],
	                             &v3[3]);
	
	//----------------------------
	SETALPHABLEND(_pD3DDevice, FALSE);

	for (i = 0; i < 12; i++)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
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
			particle[j].timcreation	=	lARXTime;
			particle[j].tc			=	tex_p1;
			particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].fparam		=	0.0000001f;
			particle[j].r			=	0.7f;
			particle[j].g			=	0.6f;
			particle[j].b			=	0.2f;
		}
	}

	SETCULL(_pD3DDevice, D3DCULL_NONE);
	SETZWRITE(_pD3DDevice, FALSE);
	SETALPHABLEND(_pD3DDevice, TRUE);

	return 1;
}

//-----------------------------------------------------------------------------
void CDispellField::Create(EERIE_3D aeSrc, float afBeta)
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

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CDispellField::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CDispellField::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y + 100.0f;
	float z = eSrc.z;


	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	SETZWRITE(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);

	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			x = inter.iobj[i]->pos.x;
			y = inter.iobj[i]->pos.y;
			z = inter.iobj[i]->pos.z;
		}
	}

	y -= 40;

	y = eSrc.y + 140;

	if (tex_p2 && tex_p2->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_p2);
	}

	//----------------------------
	y -= 40;
	
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	x = player.pos.x;
	y = player.pos.y + 80;
	z = player.pos.z;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;

	SETALPHABLEND(m_pd3dDevice, TRUE);

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
	SETALPHABLEND(m_pd3dDevice, TRUE);
	DrawEERIEObjEx(m_pd3dDevice, ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

	y = player.pos.y + 20;
	stitepos.y = y;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(m_pd3dDevice, srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

	return 1;
}

//-----------------------------------------------------------------------------
CTelekinesis::~CTelekinesis()
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
void CTelekinesis::Create(EERIE_3D aeSrc, float afBeta)
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

	fSize = 1;

	bDone = true;
}

//---------------------------------------------------------------------
void CTelekinesis::Update(unsigned long _ulTime)
{
	ulCurrentTime += _ulTime;
}

//---------------------------------------------------------------------
float CTelekinesis::Render(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y + 100.0f;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
		/*		if (bDone)
				{
					EERIE_3D target,source;
					target.x=player.pos.x;// - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*1000.f;
					target.y=player.pos.y;//+30.f;
					target.z=player.pos.z;// + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*1000.f;
					source.x = x;
					source.y = y;
					source.z = z;
					if (pIncinerate)
					{
						pIncinerate->Create(source, MAKEANGLE(player.angle.b));
						pIncinerate->SetDuration(2000);
					}
					//DebugSphere(source.x,source.y,source.z,20,8000,0xFFFF0000);
					//DebugSphere(target.x,target.y,target.z,20,8000,0xFFFF0000);
					bDone = false;
				}
				else
				{
					return 0.f;
				}
		*/
	}

	//DumpMap();

	//SETTC(m_pd3dDevice,NULL);
	//SETCULL(m_pd3dDevice,D3DCULL_NONE);
	SETZWRITE(m_pd3dDevice, FALSE);
	//SETALPHABLEND(m_pd3dDevice, FALSE);
	SETALPHABLEND(m_pd3dDevice, TRUE);


	//	register INTERACTIVE_OBJ * io;
	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			x = inter.iobj[i]->pos.x;
			y = inter.iobj[i]->pos.y;
			z = inter.iobj[i]->pos.z;
		}
	}


	//----------------
	//	if (tex_p2 && tex_p2->m_pddsSurface)
	{
		//		SETTC(m_pd3dDevice, tex_p2->m_pddsSurface);
	}
	//for (long n=0; n<12; n++)

	/*	if (bGo)
		for (i=0; i<360; i++)
		{
			x = eSrc.x;
			y = eSrc.y;
			z = eSrc.z;

			float t = rnd();
			if (t<0.01f)
			{

				t = rnd();
				//if (t>0.5f)
					//y -= 240;

				int j=ARX_PARTICLES_GetFree();
				if ((j!=-1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					particle[j].exist=1;
					particle[j].zdec=0;

					//float randd=DEG2RAD((float)j*10.f);
					particle[j].ov.x = x + 5.f - rnd()*10.f;
					particle[j].ov.y = y + 5.f - rnd()*10.f;
					particle[j].ov.z = z + 5.f - rnd()*10.f;
					particle[j].move.x = 2.f - 4.f*rnd();
					particle[j].move.y = 2.f - 4.f*rnd();
					particle[j].move.z = 2.f - 4.f*rnd();
					particle[j].siz = 20.f;
					particle[j].tolive=2000+(unsigned long)(float)(rnd()*4000.f);
					particle[j].scale.x=1.f;
					particle[j].scale.y=1.f;
					particle[j].scale.z=1.f;
					particle[j].timcreation=ARXTime;//spells[i].lastupdate;
					particle[j].tc = tex_p2;
					particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].fparam=0.0000001f;
					particle[j].r=0.7f;
					particle[j].g=0.3f;
					particle[j].b=0.f;
				}
			}
		}
	*/
	y -= 40;

	y = eSrc.y + 140;

	//----------------------------
 
 

 
 

	if (tex_p2 && tex_p2->m_pddsSurface)
	{
		SETTC(m_pd3dDevice, tex_p2);
	}

	//SETTC(m_pd3dDevice, NULL);


	//----------------------------
	y -= 40;
	/*	DrawBillBoardPoly(m_pd3dDevice, x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*20, y+frand2()*20, z+frand2()*20, 60, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(m_pd3dDevice, x+frand2()*20, y+frand2()*20, z+frand2()*20, 60, tex_p2, D3DRGB(1,1,1));
	*/
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	x = player.pos.x;
	y = player.pos.y + 80;
	z = player.pos.z;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120; //+=(float)FrameDiff*0.1f;
	stiteangle.a = 0;//abs(cos (DEG2RAD(tPos[i].x)))*10;
	stiteangle.g = 0;//cos (DEG2RAD(tPos[i].x))*360;
	stitepos.x = x;//tPos[i].x;//player.pos.x;//-(float)EEsin(DEG2RAD(player.angle.b))*(100.f) ;
	stitepos.y = y;//player.pos.y+60.f-mov;
	stitepos.z = z;//tPos[i].z;//player.pos.z;//+(float)EEcos(DEG2RAD(player.angle.b))*(100.f) ;

	SETALPHABLEND(m_pd3dDevice, TRUE);

	stiteangle.b = -stiteangle.b * 1.5f;
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 1;
	stitescale.y = -0.1f;
	stitescale.z = 1;
	//	DrawEERIEObjEx(m_pd3dDevice,slight,&stiteangle,&stitepos,&stitescale,&stitecolor);

	stiteangle.b = -stiteangle.b;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;
	SETALPHABLEND(m_pd3dDevice, TRUE);
	DrawEERIEObjEx(m_pd3dDevice, ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

 

	y = player.pos.y + 20;
	stitepos.y = y;//player.pos.y+60.f-mov;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(m_pd3dDevice, srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

	return 1;
}

//-----------------------------------------------------------------------------
CCurse::~CCurse()
{
	svoodoo_count--;

	if (svoodoo && (svoodoo_count <= 0))
	{
		svoodoo_count = 0;
		ReleaseEERIE3DObj(svoodoo);
		svoodoo = NULL;
	}
}
CCurse::CCurse(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	eSrc.x = 0;
	eSrc.y = 0;
	eSrc.z = 0;

	eTarget.x = 0;
	eTarget.y = 0;
	eTarget.z = 0;

	SetDuration(3000);
	ulCurrentTime = ulDuration + 1;

	tex_p1 = MakeTCFromFile("Graph\\Obj3D\\textures\\(Fx)_tsu_blueting.bmp");

	if (!svoodoo)
	{
		svoodoo  = _LoadTheObj("Graph\\Obj3D\\Interactive\\Fix_inter\\fx_voodoodoll\\fx_voodoodoll.teo", NULL);
		EERIE_3DOBJ_RestoreTextures(svoodoo);
	}

	svoodoo_count++;
	//EERIE_3DOBJ_RestoreTextures(obj_voodoo);
}

//-----------------------------------------------------------------------------
void CCurse::Create(EERIE_3D aeSrc, float afBeta)
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

//---------------------------------------------------------------------
float CCurse::Render(LPDIRECT3DDEVICE7 m_pd3dDevice, EERIE_3D * pos)
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y;// + 200.0f;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		/*		if (bDone)
				{
					EERIE_3D target,source;
					target.x=player.pos.x;// - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*1000.f;
					target.y=player.pos.y;//+30.f;
					target.z=player.pos.z;// + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*1000.f;
					source.x = x;
					source.y = y;
					source.z = z;
					if (pIncinerate)
					{
						pIncinerate->Create(source, MAKEANGLE(player.angle.b));
						pIncinerate->SetDuration(2000);
					}
					//DebugSphere(source.x,source.y,source.z,20,8000,0xFFFF0000);
					//DebugSphere(target.x,target.y,target.z,20,8000,0xFFFF0000);
					bDone = false;
				}
				else
				{
					return 0.f;
				}
		*/
	}

	SETCULL(m_pd3dDevice, D3DCULL_CW);
	SETZWRITE(m_pd3dDevice, TRUE);

	x = pos->x;
	y = pos->y;
	z = pos->z;
	//----------------------------
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;
	SETALPHABLEND(m_pd3dDevice, FALSE);


	stiteangle.b = fRot;
	stiteangle.a = 0;
	stiteangle.g = 0;
	stitepos.x = x;
	stitepos.y = y;
	stitepos.z = z;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 1;
	stitescale.y = 1;
	stitescale.z = 1;

	if (svoodoo)
		DrawEERIEObjEx(m_pd3dDevice, svoodoo , &stiteangle, &stitepos, &stitescale, &stitecolor);

	for (i = 0; i < 4; i++)
	{
		int j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			PARTICLE_DEF * pd = &particle[j];
			pd->exist		=	1;
			pd->zdec		=	0;
			pd->ov.x		=	x;
			pd->ov.y		=	y;
			pd->ov.z		=	z;
			pd->move.x		=	2.f * frand2();
			pd->move.y		=	rnd() * -10.f - 10.f; 
			pd->move.z		=	2.f * frand2();
			pd->siz			=	0.015f;
			pd->tolive		=	1000 + (unsigned long)(float)(rnd() * 600.f);
			pd->scale.x		=	1.f;
			pd->scale.y		=	1.f;
			pd->scale.z		=	1.f;
			pd->timcreation	=	lARXTime;
			pd->tc			=	tex_p1;
			pd->special		=	ROTATING | MODULATE_ROTATION | DISSIPATING | SUBSTRACT | GRAVITY;
			pd->fparam		=	0.0000001f;
			pd->r			=	1.f;
			pd->g			=	1.f;
			pd->b			=	1.f;
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
void CFireProtection::Create(long _ulDuration, int _iNpc)
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
	if (!ARXPausedTimer) ulCurrentTime += _ulTime;

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
float CFireProtection::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
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
	if (!ARXPausedTimer) ulCurrentTime += _ulTime;

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

//-----------------------------------------------------------------------------
float CColdProtection::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	return 0;
}



//###########################################################################
/*CFieldProtectionPentagram::CFieldProtectionPentagram()
{
	int nb=256;
	while(nb--)
	{
		this->tfieldprotection[nb].actif=0;
	}

	this->d3dpenta=NULL;
	this->vertexpenta=NULL;
	this->indpenta=NULL;
}

//-----------------------------------------------------------------------------
CFieldProtectionPentagram::~CFieldProtectionPentagram()
{
	if(this->vertexpenta) free((void*)this->vertexpenta);
	if(this->d3dpenta) free((void*)this->d3dpenta);
	if(this->indpenta) free((void*)this->indpenta);
}

//-----------------------------------------------------------------------------
void CFieldProtectionPentagram::Create(EERIE_3D *pos,float rayon,char type,int duration,int durationp)
{
	this->pos=*pos;
	this->rayon=rayon;
	this->type=type;
	this->nbfieldprotection=0;
	this->duration=duration;
	this->currduration=0;
	this->durationp=durationp;
	this->angpat=0.f;
	if(type)
	{
		this->t=MakeTCFromFile("Graph/Obj3D/textures/(Fx)_pentagram1.bmp");
	}
	else
	{
		this->t=MakeTCFromFile("Graph/Obj3D/textures/(Fx)_pentagram2.bmp");
	}

	//on crée le penta subdiv
	int		nbs=(int)(rayon*2.f/50.f);
	if((rayon*2.f-((float)(nbs*50)))>0.f)
	{
		nbs++;
	}

	this->nbptpenta=(nbs+1)*(nbs+1);
	this->nbfacepenta=nbs*nbs;

	if(this->vertexpenta) free((void*)this->vertexpenta);
	if(this->d3dpenta) free((void*)this->d3dpenta);
	if(this->indpenta) free((void*)this->indpenta);
	this->vertexpenta=(EERIE_3D*)malloc(this->nbptpenta*sizeof(EERIE_3D));
	this->d3dpenta=(D3DTLVERTEX*)malloc(this->nbptpenta*sizeof(D3DTLVERTEX));
	this->indpenta=(unsigned short*)malloc(this->nbfacepenta*sizeof(unsigned short)*3*2);

	EERIE_3D	*vertex=this->vertexpenta;
	D3DTLVERTEX	*d3d=this->d3dpenta;
	float u,v,dxz,duv,x,z;
	z=-rayon;
	duv=0.999999999f/(float)nbs;
	dxz=rayon*2.f/(float)nbs;
	v=0.f;
	int nbs2=nbs;
	while(nbs2--)
	{
		x=-rayon;
		u=0.f;
		int nbs3=nbs;
		while(nbs3--)
		{
			vertex->x=x;
			vertex->y=0.f;
			vertex->z=z;
			d3d->tu=u;
			d3d->tv=v;

			u+=duv;
			x+=dxz;

			vertex++;
			d3d++;
		}
		vertex->x=rayon;
		vertex->y=0.f;
		vertex->z=z;
		d3d->tu=0.9999999f;
		d3d->tv=v;
		vertex++;
		d3d++;

		v+=duv;
		z+=dxz;
	}
	x=-rayon;
	u=0.f;
	int nbs3=nbs;
	while(nbs3--)
	{
		vertex->x=x;
		vertex->y=0.f;
		vertex->z=rayon;
		d3d->tu=u;
		d3d->tv=0.999999999f;

		u+=duv;
		x+=dxz;

		vertex++;
		d3d++;
	}
	vertex->x=rayon;
	vertex->y=0.f;
	vertex->z=rayon;
	d3d->tu=0.9999999f;
	d3d->tv=0.9999999f;

	unsigned short *pind=this->indpenta,ind=0;
	nbs2=nbs;
	while(nbs2--)
	{
		int nbs3=nbs;
		while(nbs3--)
		{
			*pind++=ind;
			*pind++=ind+nbs+1;
			*pind++=ind+nbs+2;

			*pind++=ind++;
			*pind++=ind+nbs+1;
			*pind++=ind;
		}
		ind++;
	}
}

//-----------------------------------------------------------------------------
void CFieldProtectionPentagram::Update(int t)
{
	//pentagram
	this->angpat+=4.f*(float)t/25.f;

	//prend les players
	if(this->currduration<this->duration)
	{
		int	nb=inter.nbmax;
		while(nb--)
		{
			if(inter.iobj[nb])
			{
				float d=Distance3D(this->pos.x,this->pos.y,this->pos.z,inter.iobj[nb]->pos.x,inter.iobj[nb]->pos.y,inter.iobj[nb]->pos.z);
				if(d<=this->rayon)
				{
					BOOL	nexist=TRUE;
					int		nb2=256;
					while(nb2--)
					{
						if((this->tfieldprotection[nb2].actif)&&(this->tfieldprotection[nb2].id==nb))
						{
							nexist=FALSE;
							break;
						}
					}

					if(nexist)
					{
						int num=this->GetFree();
						if(num>=0)
						{
							EERIE_3D target;
							target.x=inter.iobj[nb]->pos.x;
							target.y=inter.iobj[nb]->pos.y;
							target.z=inter.iobj[nb]->pos.z;

							this->tfieldprotection[num].actif=1;
							this->tfieldprotection[num].id=nb;
							this->tfieldprotection[num].fprotection=new CFieldProtection();
							(this->tfieldprotection[num].fprotection)->Create(16,50.f,200.f,&target,this->durationp,this->type);
							this->nbfieldprotection++;
						}
					}
				}
			}
		}

		nb=256;
		while(nb--)
		{
			if(this->tfieldprotection[nb].actif)
			{
				if(this->tfieldprotection[nb].fprotection->EndFx())
				{
					this->tfieldprotection[nb].actif=0;
				}
				else
				{
					this->tfieldprotection[nb].fprotection->Update(t);
				}
			}
		}
	}

	if(!ARXPausedTimer) this->currduration+=t;
}

//-----------------------------------------------------------------------------
void CFieldProtectionPentagram::Render(LPDIRECT3DDEVICE7 device)
{
	if(this->currduration>this->duration) return;

	SETALPHABLEND(device,TRUE);
	SETCULL(device,D3DCULL_NONE);
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);

	//affichage du pentagram
	D3DTLVERTEX v[4],v2[4];
	float fBetaRad = DEG2RAD(this->angpat);
	float fBetaRadCos = (float) cos(fBetaRad);
	float fBetaRadSin = (float) sin(fBetaRad);

	EERIE_3D	*vertex=this->vertexpenta;
	D3DTLVERTEX	*d3d=this->d3dpenta,d3ds;
	int color=RGBA_MAKE(127,127,127,255);
	int nb=nbptpenta;
	while(nb--)
	{
		d3ds.sx = pos.x + fBetaRadCos*vertex->x + fBetaRadSin*vertex->z;
		d3ds.sy = pos.y;
		d3ds.sz = pos.z - fBetaRadSin*vertex->x + fBetaRadCos*vertex->z;

		EE_RTP(&d3ds,d3d);

		vertex++;
		d3d++;
	}
	if(this->t)
	{
		device->SetTexture(0,this->t->m_pddsSurface);
	}
	else
	{
		device->SetTexture(0,NULL);
	}
	device->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, this->d3dpenta, this->nbptpenta, this->indpenta, this->nbfacepenta*3*2, 0 );

	//affichage des protections
	nb=256;
	while(nb--)
	{
		if(this->tfieldprotection[nb].actif)
		{
			this->tfieldprotection[nb].fprotection->Render(device);

			EERIE_3D target;
			target.x=inter.iobj[this->tfieldprotection[nb].id]->pos.x;
			target.y=inter.iobj[this->tfieldprotection[nb].id]->pos.y;
			target.z=inter.iobj[this->tfieldprotection[nb].id]->pos.z;
			this->tfieldprotection[nb].fprotection->ChangePosition(&target);
		}
	}

	SETALPHABLEND(device,FALSE);
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO);
}

//-----------------------------------------------------------------------------
//###########################################################################
CFieldProtection::CFieldProtection()
{
	this->vcylinder=NULL;
	this->icylinder=NULL;
	this->localvertex=NULL;
	this->bloby=NULL;
}
//-----------------------------------------------------------------------------
CFieldProtection::~CFieldProtection()
{
	if(this->vcylinder) free((void*)this->vcylinder);
	if(this->icylinder) free((void*)this->icylinder);
	if(this->localvertex) free((void*)this->localvertex);
	if(this->bloby) free((void*)this->bloby);
}
//-----------------------------------------------------------------------------
void CFieldProtection::CreateStripCylinder(void)
{
float		da=360.f/(float)this->def;
float		a=0.f,hm,hp;
int			nb;
unsigned short		*ind,indc;
EERIE_3D	*lvertex;

	//on efface l'ancien cylindre
	if(this->vcylinder) free((void*)this->vcylinder);
	if(this->icylinder) free((void*)this->icylinder);
	if(this->localvertex) free((void*)this->localvertex);

	//on alloue le nouveau cylindre
	nb=this->def<<1;
	this->nbvertexcyl=nb;
	this->localvertex=(EERIE_3D*)malloc(nb*sizeof(EERIE_3D));
	this->vcylinder=(D3DTLVERTEX*)malloc(nb*sizeof(D3DTLVERTEX));
	this->icylinder=(unsigned short*)malloc((nb+2)*sizeof(short));

	//on le crée
	hp=-this->hauteur;
	hm=0.f;

	ind=this->icylinder;
	indc=0;

	lvertex=this->localvertex;
	nb=this->def;
	while(nb)
	{
		*ind++=indc++;
		*ind++=indc++;

		lvertex->x=this->rayon*EEsin(DEG2RAD(a));
		lvertex->y=hm;
		lvertex->z=this->rayon*EEcos(DEG2RAD(a));
		lvertex++;
		lvertex->x=(lvertex-1)->x;
		lvertex->y=hp;
		lvertex->z=(lvertex-1)->z;
		lvertex++;
		a+=da;
		nb--;
	}
	*ind++=0;
	*ind=1;
}
//-----------------------------------------------------------------------------
void CFieldProtection::CreateTListCylinder(void)
{
float		da=360.f/(float)this->def;
float		a,hm,hp;
int			nb;
unsigned short		*ind,indc;
EERIE_3D	*lvertex;

	//on efface l'ancien cylindre
	if(this->vcylinder) free((void*)this->vcylinder);
	if(this->icylinder) free((void*)this->icylinder);
	if(this->localvertex) free((void*)this->localvertex);

	//on alloue le nouveau cylindre
	this->nbvertexcyl=this->def*(this->def+1);
	this->nbfacecyl=(this->def*this->def)<<1;
	this->localvertex=(EERIE_3D*)malloc(this->nbvertexcyl*sizeof(EERIE_3D));
	this->vcylinder=(D3DTLVERTEX*)malloc(this->nbvertexcyl*sizeof(D3DTLVERTEX));
	this->icylinder=(unsigned short*)malloc(this->nbfacecyl*sizeof(short)*3);
	this->bloby=(float*)malloc((this->def+1)*sizeof(float));

	float *ft=this->bloby,df=rnd()*360.f,ddf=180.f/(float)(this->def+1);
	nb=this->def+1;
	while(nb)
	{
		*ft++=df;
		df+=ddf;
		nb--;
	}

	//on le crée
	hp=-this->hauteur;
	hm=this->hauteur/(float)def;

	lvertex=this->localvertex;
	int nb2=this->def+1;
	while(nb2--)
	{
		a=0.f;
		nb=this->def;
		while(nb)
		{
			lvertex->x=this->rayon*EEsin(DEG2RAD(a));
			lvertex->y=hp;
			lvertex->z=this->rayon*EEcos(DEG2RAD(a));
			lvertex++;
			a+=da;
			nb--;
		}
		hp+=hm;
	}

	//on cree la liste d'indice
	ind=this->icylinder,indc=0;
	nb2=this->def;
	while(nb2--)
	{
		int oldind=indc;
		nb=this->def-1;
		while(nb--)
		{
			*ind++=indc;
			*ind++=indc+this->def;
			*ind++=indc+this->def+1;

			*ind++=indc++;
			*ind++=indc+this->def;
			*ind++=indc;
		}
		*ind++=indc;
		*ind++=indc+this->def;
		*ind++=oldind+this->def;

		*ind++=indc;
		*ind++=oldind+this->def;
		*ind++=oldind;

		indc++;
	}
}
//-----------------------------------------------------------------------------
void CFieldProtection::Create(char def,float rayon,float hauteur,EERIE_3D *pos,int duration,char flag)
{
	this->key=0;
	this->def=def;
	this->rayon=rayon;
	this->hauteur=hauteur;
	this->pos=*pos;
	this->currduration=0;
	this->duration=duration;
//	this->CreateStripCylinder();
	this->CreateTListCylinder();
	if(flag)
	{
		this->tf=MakeTCFromFile("Graph\\Obj3D\\Textures\\(FX)_cold_protec.bmp");
		this->tp=MakeTCFromFile("Graph\\Particles\\fire.bmp");
	}
	else
	{
		this->tf=MakeTCFromFile("Graph\\Obj3D\\Textures\\(FX)_fire_protec.bmp");
		this->tp=MakeTCFromFile("Graph\\Particles\\healing.bmp");
	}

	this->flagdv=this->flagdu=0;
	this->timedv=this->timedu=0;
	this->angdv=this->angdu=0.f;
}
//-----------------------------------------------------------------------------
void CFieldProtection::Update(int t)
{
float			a,b,da;
D3DTLVERTEX		*d3dv,d3dvs;
EERIE_3D		*lvertex;
int				nb,j;

	int col=RGBA_MAKE(255,255,255,255);
/*
	switch(this->key)
	{
	case 0:
		a=(float)this->currduration/500.f;
		if(a>1.f)
		{
			this->key++;
			this->currduration=0;
			a=1.f;
		}
		this->interp=a;

		b=0.f;
		da=(0.999999f/(float)this->def);

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		nb=this->nbvertexcyl>>1;
		while(nb)
		{
			d3dvs.sx=lvertex->x+this->pos.x;	//pt du bas
			d3dvs.sy=lvertex->y+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;
			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;
			d3dv->tu=b;
			d3dv->tv=0.f;
			lvertex++;
			d3dv++;

			d3dvs.sx=lvertex->x+this->pos.x;	//pt du haut
			d3dvs.sy=(lvertex-1)->y+((lvertex->y-(lvertex-1)->y)*a)+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;

			j=ARX_PARTICLES_GetFree();
			if ((j!=-1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist=1;
				particle[j].zdec=0;

				particle[j].ov.x = d3dvs.sx;
				particle[j].ov.y = d3dvs.sy;
				particle[j].ov.z = d3dvs.sz;
				particle[j].move.x = 0.f;
				particle[j].move.y = -(3.f+4.f*rnd());
				particle[j].move.z = 0.f;
				particle[j].siz = 7.f+7.f*rnd();
				particle[j].tolive=500+(int)(200.f*rnd());
				particle[j].scale.x=1.f;
				particle[j].scale.y=1.f;
				particle[j].scale.z=1.f;
				particle[j].timcreation=ARXTime;
				particle[j].tc = this->tp;
				particle[j].special = FADE_IN_AND_OUT| ROTATING | MODULATE_ROTATION;// | DISSIPATING;
				particle[j].fparam=0.0000001f;
				particle[j].r=1.f;
				particle[j].g=1.f;
				particle[j].b=1.f;
			}

			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;
			d3dv->tu=b;
			d3dv->tv=0.999999f;

			lvertex++;
			d3dv++;
			b+=da;
			nb--;
		}
		break;
	case 1:
		//gestion time fx
		if(this->currduration>this->duration)
		{
			this->currduration=0;
			this->key++;
		}

		a=(float)this->timedv/5000.f;
		if(a>1.f)
		{
			this->timedv=this->timedu=0;
			a=1.f;
		}
		if(a>.5f)
		{
			b=(float)this->timedu/2500.f;
			this->timedu+=t;
			this->angdu=0.999999f*EEsin(DEG2RAD(360.f*a));
		}
		this->timedv+=t;
		this->angdv=0.999999f*2*EEsin(DEG2RAD(360.f*a));

		//calcul du cylindre
		a=this->angdu;
		da=(0.999999f/(float)this->def);
		b=this->angdv;

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		nb=this->nbvertexcyl>>1;
		while(nb)
		{
			d3dvs.sx=lvertex->x+this->pos.x;	//pt du bas
			d3dvs.sy=lvertex->y+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;
			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;
			d3dv->tu=a;
			d3dv->tv=b;
			lvertex++;
			d3dv++;

			d3dvs.sx=lvertex->x+this->pos.x;	//pt du haut
			d3dvs.sy=lvertex->y+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;
			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;		//TO DO:qq chose
			d3dv->tu=a;
			d3dv->tv=b+0.999999f;
			lvertex++;
			d3dv++;

			a+=da;
			nb--;
		}

		break;
	case 2:
		a=(float)this->currduration/500.f;
		if(a>1.f)
		{
			this->key++;
			this->currduration=0;
			a=1.f;
		}
		a=1.f-a;
		this->interp=a;

		b=0.f;
		da=(0.999999f/(float)this->def);

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		nb=this->nbvertexcyl>>1;
		while(nb)
		{
			d3dvs.sx=lvertex->x+this->pos.x;	//pt du bas
			d3dvs.sy=lvertex->y+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;
			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;
			d3dv->tu=b;
			d3dv->tv=0.f;
			lvertex++;
			d3dv++;

			d3dvs.sx=lvertex->x+this->pos.x;	//pt du haut
			d3dvs.sy=(lvertex-1)->y+((lvertex->y-(lvertex-1)->y)*a)+this->pos.y;
			d3dvs.sz=lvertex->z+this->pos.z;

			j=ARX_PARTICLES_GetFree();
			if ((j!=-1) && (!ARXPausedTimer))
			{
				ParticleCount++;
				particle[j].exist=1;
				particle[j].zdec=0;

				particle[j].ov.x = d3dvs.sx;
				particle[j].ov.y = d3dvs.sy;
				particle[j].ov.z = d3dvs.sz;
				particle[j].move.x = 0.f;
				particle[j].move.y = (3.f+4.f*rnd());
				particle[j].move.z = 0.f;
				particle[j].siz = 7.f+7.f*rnd();
				particle[j].tolive=500+(int)(200.f*rnd());
				particle[j].scale.x=1.f;
				particle[j].scale.y=1.f;
				particle[j].scale.z=1.f;
				particle[j].timcreation=ARXTime;
				particle[j].tc = this->tp;
				particle[j].special = FADE_IN_AND_OUT| ROTATING | MODULATE_ROTATION;// | DISSIPATING;
				particle[j].fparam=0.0000001f;
				particle[j].r=1.f;
				particle[j].g=1.f;
				particle[j].b=1.f;
			}

			EE_RTP(&d3dvs,d3dv);
			d3dv->color=col;
			d3dv->tu=b;
			d3dv->tv=0.999999f;

			lvertex++;
			d3dv++;
			b+=da;
			nb--;
		}
		break;
	}

	if(!ARXPausedTimer) this->currduration+=t;*/
/*
	switch(this->key)
	{
	case 0:
		a=(float)this->currduration/500.f;
		if(a>1.f)
		{
			this->key++;
			this->currduration=0;
			a=1.f;
		}
		this->interp=a;

		float h;
		h=-this->hauteur*a;
		float dh;
		dh=(this->hauteur/(float)def)*a;
		b=0.f;
		da=(0.999999f/(float)this->def);

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		int nb2;
		nb2=this->def+1;
		while(nb2--)
		{
			a=0.f;
			nb=this->def;
			while(nb)
			{
				d3dvs.sx=lvertex->x+this->pos.x;	//pt du haut
				d3dvs.sy=h+this->pos.y;
				d3dvs.sz=lvertex->z+this->pos.z;

				EE_RTP(&d3dvs,d3dv);
				d3dv->color=col;
				d3dv->tu=a;
				d3dv->tv=b;

				lvertex++;
				d3dv++;
				a+=da;
				nb--;
			}

			b+=da;
			h+=dh;
		}
		break;
	case 1:
		//gestion time fx
		if(this->currduration>this->duration)
		{
			this->currduration=0;
			this->key++;
		}

		a=(float)this->timedv/5000.f;
		if(a>1.f)
		{
			a=1.f;
			this->timedv=0;
		}
		b=(float)this->timedu/2500.f;
		this->timedu+=t;
		this->angdu=0.999999f*EEsin(DEG2RAD(360.f*a));
		this->timedv+=t;

		//calcul du cylindre
		da=(0.999999f/(float)this->def);
		b=this->angdv;

		float *ft;
		ft=this->bloby;
		float speed;
		speed=6.f*((float)t)/25.f;

	a=b=0.f;

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		nb2=this->def+1;
		while(nb2--)
		{
			float depl=(1.f+EEsin(DEG2RAD(*ft)))*.5f;

			a=this->angdu;
			nb=this->def;
			while(nb)
			{
				d3dvs.sx=lvertex->x+lvertex->x*depl+this->pos.x;	//pt du haut
				d3dvs.sy=lvertex->y+this->pos.y;
				d3dvs.sz=lvertex->z+lvertex->z*depl+this->pos.z;

				if((!nb2||nb2==this->def)&&(rnd()>.5f))
				{
					j=ARX_PARTICLES_GetFree();
					if ((j!=-1) && (!ARXPausedTimer))
					{
						ParticleCount++;
						particle[j].exist=1;
						particle[j].zdec=0;

						particle[j].ov.x = d3dvs.sx;
						particle[j].ov.y = d3dvs.sy;
						particle[j].ov.z = d3dvs.sz;
						particle[j].move.x = 0.f;
						particle[j].move.y = 1.f+rnd();
						particle[j].move.z = 0.f;
						particle[j].siz = 4.f+8.f*rnd();
						particle[j].tolive=500+(int)(200.f*rnd());
						particle[j].scale.x=1.f;
						particle[j].scale.y=1.f;
						particle[j].scale.z=1.f;
						particle[j].timcreation=ARXTime;
						particle[j].tc = this->tp;
						particle[j].special = FADE_IN_AND_OUT| ROTATING | MODULATE_ROTATION;// | DISSIPATING;
						particle[j].fparam=0.0000001f;
						float c=0.499999999f+rnd()*.5f;
						particle[j].r=c;
						particle[j].g=c;
						particle[j].b=c;
					}
				}

				if(depl>0.9f)
				{
					float tt=1.f/sqrt(lvertex->x*lvertex->x+lvertex->y*lvertex->y+lvertex->z*lvertex->z);

					j=ARX_PARTICLES_GetFree();
					if ((j!=-1) && (!ARXPausedTimer))
					{
						ParticleCount++;
						particle[j].exist=1;
						particle[j].zdec=0;

						particle[j].ov.x = d3dvs.sx;
						particle[j].ov.y = d3dvs.sy;
						particle[j].ov.z = d3dvs.sz;
						particle[j].move.x = lvertex->x*tt;
						particle[j].move.y = 0.f;
						particle[j].move.z = lvertex->z*tt;
						particle[j].siz = 1.f+2.f*rnd();
						particle[j].tolive=500+(int)(200.f*rnd());
						particle[j].scale.x=1.f;
						particle[j].scale.y=1.f;
						particle[j].scale.z=1.f;
						particle[j].timcreation=ARXTime;
						particle[j].tc = this->tp;
						particle[j].special = FADE_IN_AND_OUT| ROTATING | MODULATE_ROTATION;// | DISSIPATING;
						particle[j].fparam=0.0000001f;
						float c=0.499999999f+rnd()*.5f;
						particle[j].r=c;
						particle[j].g=c;
						particle[j].b=c;
					}
				}

				EE_RTP(&d3dvs,d3dv);
				d3dv->color=col;
				d3dv->tu=a;
				d3dv->tv=b;

				lvertex++;
				d3dv++;
				a+=da;
				nb--;
			}

			*ft+=speed;
			ft++;
			b+=da;
		}
		break;
	case 2:
		a=(float)this->currduration/500.f;
		if(a>1.f)
		{
			this->key++;
			this->currduration=0;
			a=1.f;
		}
		a=1.f-a;
		this->interp=a;

		h=-this->hauteur*a;
		dh=(this->hauteur/(float)def)*a;
		b=0.f;
		da=(0.999999f/(float)this->def);

		lvertex=this->localvertex;
		d3dv=this->vcylinder;
		nb2=this->def+1;
		while(nb2--)
		{
			a=0.f;
			nb=this->def;
			while(nb)
			{
				d3dvs.sx=lvertex->x+this->pos.x;	//pt du haut
				d3dvs.sy=h+this->pos.y;
				d3dvs.sz=lvertex->z+this->pos.z;

				EE_RTP(&d3dvs,d3dv);
				d3dv->color=col;
				d3dv->tu=a;
				d3dv->tv=b;

				lvertex++;
				d3dv++;
				a+=da;
				nb--;
			}

			b+=da;
			h+=dh;
		}
		break;
	}

	if(!ARXPausedTimer) this->currduration+=t;
}
//-----------------------------------------------------------------------------
void CFieldProtection::Render(LPDIRECT3DDEVICE7 device)
{
	if(this->key>2) return ;

	SETALPHABLEND(device,TRUE);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);

	//tracé du cylindre
	if(this->tf) device->SetTexture(0,this->tf->m_pddsSurface);
	else device->SetTexture(0,NULL);

	SETCULL(device,D3DCULL_CCW);
//	device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,this->vcylinder,this->nbvertexcyl,(unsigned short *)this->icylinder,this->nbvertexcyl+2,0);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,this->vcylinder,this->nbvertexcyl,(unsigned short *)this->icylinder,this->nbfacecyl*3,0);
	SETCULL(device,D3DCULL_CW);
//	device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,this->vcylinder,this->nbvertexcyl,(unsigned short *)this->icylinder,this->nbvertexcyl+2,0);
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,this->vcylinder,this->nbvertexcyl,(unsigned short *)this->icylinder,this->nbfacecyl*3,0);

	device->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO);
	SETALPHABLEND(device,FALSE);
	SETCULL(device,D3DCULL_NONE);
}
/*--------------------------------------------------------------------------*/


//-----------------------------------------------------------------------------
// FIRE PROTECTION
//-----------------------------------------------------------------------------
/*CFireProtection::CFireProtection()
{
	//this->tr=NULL;//MakeTCFromFile("Graph\\Particles\\losange.bmp");
	tex_p = MakeTCFromFile("Graph\\Obj3D\\Textures\\(Fx)_fire_armor.bmp");
	//("Graph\\Obj3D\\Textures\\(Fx)_ice_armor.bmp");
	//MakeTCFromFile("Graph\\Particles\\fire.bmp");
	tex_r = MakeTCFromFile("Graph\\Obj3D\\Textures\\(Fx)_fire_armor.bmp");
}

//-----------------------------------------------------------------------------
void CFireProtection::Create(float rayon,EERIE_3D *pos,float hperso,int _ulDuration,int dec)
{
	this->key=1;
	this->rayon=rayon;
	this->pos=*pos;
	SetDuration(_ulDuration);

	this->nbrubandef=0;
	this->hpersosurdeux=hperso*.5f;
	this->dec=dec;

	this->pos.y+=this->hpersosurdeux;

/*	this->first=-1;
	this->angruban=0.f;
	this->sang=8.f;
	this->h=0.f;

	this->first2=-1;
	this->angruban2=180.f;
	this->sang2=-8.f;
	this->h2=0.f;
*/
/*	int nb=256;
	while(nb--)
	{
		this->truban[nb].actif=0;
	}
}

//-----------------------------------------------------------------------------
void CFireProtection::ChangePos(EERIE_3D *pos)
{
	this->pos=*pos;
	this->pos.y+=this->hpersosurdeux;
}

//-----------------------------------------------------------------------------
void CFireProtection::AddRubanDef(float angruban,float sang,float r,float g,float b,float r2,float g2,float b2)
{
	if(this->nbrubandef>255) return;
	this->trubandef[this->nbrubandef].first=-1;
	this->trubandef[this->nbrubandef].angruban=angruban;
	this->trubandef[this->nbrubandef].h=0.f;
	this->trubandef[this->nbrubandef].sang=sang;
	this->trubandef[this->nbrubandef].r=r;
	this->trubandef[this->nbrubandef].g=g;
	this->trubandef[this->nbrubandef].b=b;
	this->trubandef[this->nbrubandef].r2=r2;
	this->trubandef[this->nbrubandef].g2=g2;
	this->trubandef[this->nbrubandef].b2=b2;
	this->nbrubandef++;
}

//-----------------------------------------------------------------------------
int CFireProtection::GetFreeRuban(void)
{
	int nb=256;

	while(nb--)
	{
		if(!this->truban[nb].actif) return nb;
	}

	return -1;
}

//-----------------------------------------------------------------------------
void CFireProtection::AddRuban(int *f,float *ang,float *h,float s,int t)
{
int	num;

	num=this->GetFreeRuban();
	if(num>=0)
	{
		this->truban[num].actif=1;
		this->truban[num].pos.x=this->pos.x+this->rayon*EEcos(DEG2RAD(*ang))+this->rayon*.5f*(float)tanh(DEG2RAD(*ang));
		this->truban[num].pos.y=this->pos.y+*h;
		this->truban[num].pos.z=this->pos.z+this->rayon*EEsin(DEG2RAD(*ang))+this->rayon*.3f*(float)tanh(DEG2RAD(*ang));

		if(*f<0)
		{
			*f=num;
			this->truban[num].next=-1;
		}
		else
		{
			this->truban[num].next=*f;
			*f=num;
		}

		int nb=0,oldnum;
		while(num!=-1)
		{
			nb++;
			oldnum=num;
			num=this->truban[num].next;
		}

		//pas plus de 4 pass.
		if(nb>this->dec)
		{
			this->truban[oldnum].actif=0;
			num=*f;
			nb-=2;
			while(nb--)
			{
				num=this->truban[num].next;
			}
			this->truban[num].next=-1;
		}
	}

	*h=this->hpersosurdeux*EEsin(DEG2RAD(*ang*.2f));
	*ang+=(s*(float)t)/25.f;
}

//-----------------------------------------------------------------------------
void CFireProtection::Update(unsigned long _ulTime,EERIE_3D *pos)
{
int	nb,num;

/*	switch(this->key)
	{
	case 0:
		break;
	case 1:*/
//		if(ARXPausedTimer) break;
/*
		if( ulCurrentTime > ulDuration)
		{
			this->key++;
		}

		this->ChangePos(pos);

		num=0;
		nb=this->nbrubandef;
		while(nb--)
		{
			this->AddRuban(&this->trubandef[num].first,&this->trubandef[num].angruban,&this->trubandef[num].h,this->trubandef[num].sang, _ulTime);
			num++;
		}
//		break;
//	}

	if(!ARXPausedTimer) ulCurrentTime += _ulTime;
}

//-----------------------------------------------------------------------------
void CFireProtection::DrawRuban(LPDIRECT3DDEVICE7 _pD3DDevice,int num,float r,float g,float b,float r2,float g2,float b2)
{
int numsuiv;

	float	size=5.f,dsize=5.f/(float)this->dec;
	int		r1=((int)(r*255.f))<<16;
	int		g1=((int)(g*255.f))<<16;
	int		b1=((int)(b*255.f))<<16;
	int		rr2=((int)(r2*255.f))<<16;
	int		gg2=((int)(g2*255.f))<<16;
	int		bb2=((int)(b2*255.f))<<16;
	int		dr=(rr2-r1)/this->dec;
	int		dg=(gg2-g1)/this->dec;
	int		db=(bb2-b1)/this->dec;

	if((num>=0)&&(this->truban[num].next>=0))
	{
		float t=Distance3D(	this->truban[num].pos.x,this->truban[num].pos.y,this->truban[num].pos.z,
							this->truban[this->truban[num].next].pos.x,this->truban[this->truban[num].next].pos.y,this->truban[this->truban[num].next].pos.z);
		t=1.f/t;

/*		int j=ARX_PARTICLES_GetFree();
		if ((j!=-1) && (!ARXPausedTimer))
		{
			ParticleCount++;
			particle[j].exist=TRUE;
			particle[j].zdec=0;
			float randd=rnd()*360.f;
			particle[j].ov.x=this->truban[num].pos.x;
			particle[j].ov.y=this->truban[num].pos.y;
			particle[j].ov.z=this->truban[num].pos.z;
			particle[j].move.x=(this->truban[num].pos.x-this->truban[this->truban[num].next].pos.x)*t;
			particle[j].move.y=(this->truban[num].pos.y-this->truban[this->truban[num].next].pos.y)*t;
			particle[j].move.z=(this->truban[num].pos.z-this->truban[this->truban[num].next].pos.z)*t;
			particle[j].scale.x=1.f;
			particle[j].scale.y=1.f;
			particle[j].scale.z=1.f;
			particle[j].timcreation = ARXTime;
			particle[j].tolive=500+(unsigned long)(rnd()*300.f);
			particle[j].tc = tex_p;
			particle[j].siz=.5f+1.f*rnd();
			particle[j].r=r;
			particle[j].g=g;
			particle[j].b=b;
		}
		*/
/*	}

	while(1)
	{
		numsuiv=this->truban[num].next;
		if((num>=0)&&(numsuiv>=0))
		{
			Draw3DLineTex3(_pD3DDevice,this->truban[num].pos,this->truban[numsuiv].pos,size,size-dsize,
				RGBA_MAKE(r1>>16,g1>>16,b1>>16,1),
				RGBA_MAKE((r1+dr)>>16,(g1+dg)>>16,(b1+db)>>16,1));
			r1+=dr;
			g1+=dg;
			b1+=db;
			size-=dsize;
		}
		else
		{
			break;
		}
		num=numsuiv;
	}
}

//-----------------------------------------------------------------------------
float CFireProtection::Render(LPDIRECT3DDEVICE7 _pD3DDevice)
{
	SETCULL(_pD3DDevice,D3DCULL_NONE);
	SETALPHABLEND(_pD3DDevice,TRUE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);

	if(tex_p)
	{
		SETTC(_pD3DDevice, tex_p->m_pddsSurface);
	}
	else
	{
		SETTC(_pD3DDevice,NULL);
	}
	int nb = nbrubandef,num=0;
	while(nb--)
	{
		DrawRuban(	_pD3DDevice, trubandef[num].first,
					trubandef[num].r,  trubandef[num].g,  trubandef[num].b,
					trubandef[num].r2, trubandef[num].g2, trubandef[num].b2);
		num++;
	}

	SETALPHABLEND(_pD3DDevice,FALSE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO);

	return 0;
}

//-----------------------------------------------------------------------------
// COLD PROTECTION
//-----------------------------------------------------------------------------
CColdProtection::CColdProtection()
{
	tex_p = MakeTCFromFile("Graph\\Obj3D\\Textures\\(Fx)_ice_armor.bmp");
	tex_r = MakeTCFromFile("Graph\\Obj3D\\Textures\\(Fx)_ice_armor.bmp");
}

//-----------------------------------------------------------------------------
CColdProtection::~CColdProtection()
{
}
*/
