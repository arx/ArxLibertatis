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

#include "graphics/spells/Spells04.h"

#include "core/Core.h"
#include "core/Time.h"

#include "game/Spells.h"
#include "game/Player.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/spells/Spells05.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleManager.h"

#include "scene/Object.h"
#include "scene/Interactive.h"
#include "scene/LoadLevel.h"

//-----------------------------------------------------------------------------
CBless::CBless()
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
float CBless::Render()
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y - 5;
	float z = eSrc.z;

	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	D3DTLVERTEX v[4];
	D3DTLVERTEX v3[4];

	float ff = ((float)spells[spellinstance].caster_level + 10) * 6.f;
	float fBetaRadCos = (float) cos(radians(MAKEANGLE(player.angle.b))) * ff;
	float fBetaRadSin = (float) sin(radians(MAKEANGLE(player.angle.b))) * ff;

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
		SETTC(tex_sol);
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
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

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

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

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
	fBetaRad = radians(fBeta);
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
float CDispellField::Render()
{
	int i = 0;

	float x = eSrc.x;
	float y = eSrc.y + 100.0f;
	float z = eSrc.z;


	if (ulCurrentTime >= ulDuration)
	{
		return 0.f;
	}

	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

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
		SETTC(tex_p2);
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

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

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
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

	y = player.pos.y + 20;
	stitepos.y = y;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

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
	fBetaRad = radians(fBeta);
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
float CTelekinesis::Render()
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
					target.x=player.pos.x;// - EEsin(radians(MAKEANGLE(player.angle.b)))*1000.f;
					target.y=player.pos.y;//+30.f;
					target.z=player.pos.z;// + EEcos(radians(MAKEANGLE(player.angle.b)))*1000.f;
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

	//GRenderer->ResetTexture(0);
	//GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	//GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);


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
		//		SETTC(tex_p2->m_pddsSurface);
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

					//float randd=radians((float)j*10.f);
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
		SETTC(tex_p2);
	}

	//GRenderer->ResetTexture(0);


	//----------------------------
	y -= 40;
	/*	DrawBillBoardPoly(x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*10, y+frand2()*10, z+frand2()*10, 40, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*20, y+frand2()*20, z+frand2()*20, 60, tex_p2, D3DRGB(1,1,1));
		DrawBillBoardPoly(x+frand2()*20, y+frand2()*20, z+frand2()*20, 60, tex_p2, D3DRGB(1,1,1));
	*/
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;

	x = player.pos.x;
	y = player.pos.y + 80;
	z = player.pos.z;

	stiteangle.b = (float) ulCurrentTime * fOneOnDuration * 120; //+=(float)FrameDiff*0.1f;
	stiteangle.a = 0;//abs(cos (radians(tPos[i].x)))*10;
	stiteangle.g = 0;//cos (radians(tPos[i].x))*360;
	stitepos.x = x;//tPos[i].x;//player.pos.x;//-(float)EEsin(radians(player.angle.b))*(100.f) ;
	stitepos.y = y;//player.pos.y+60.f-mov;
	stitepos.z = z;//tPos[i].z;//player.pos.z;//+(float)EEcos(radians(player.angle.b))*(100.f) ;

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	stiteangle.b = -stiteangle.b * 1.5f;
	stitecolor.r = 0.7f;
	stitecolor.g = 0.7f;
	stitecolor.b = 0.7f;
	stitescale.x = 1;
	stitescale.y = -0.1f;
	stitescale.z = 1;
	//	DrawEERIEObjEx(slight,&stiteangle,&stitepos,&stitescale,&stitecolor);

	stiteangle.b = -stiteangle.b;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.x = 2;
	stitescale.y = 2;
	stitescale.z = 2;
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	DrawEERIEObjEx(ssol, &stiteangle, &stitepos, &stitescale, &stitecolor);

 

	y = player.pos.y + 20;
	stitepos.y = y;//player.pos.y+60.f-mov;
	stitecolor.r = 1;
	stitecolor.g = 1;
	stitecolor.b = 1;
	stitescale.z = 1.8f;
	stitescale.y = 1.8f;
	stitescale.x = 1.8f;
	DrawEERIEObjEx(srune, &stiteangle, &stitepos, &stitescale, &stitecolor);

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
CCurse::CCurse()
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
	fBetaRad = radians(fBeta);
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
float CCurse::Render(EERIE_3D * pos)
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
					target.x=player.pos.x;// - EEsin(radians(MAKEANGLE(player.angle.b)))*1000.f;
					target.y=player.pos.y;//+30.f;
					target.z=player.pos.z;// + EEcos(radians(MAKEANGLE(player.angle.b)))*1000.f;
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

	GRenderer->SetCulling(Renderer::CullCW);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);

	x = pos->x;
	y = pos->y;
	z = pos->z;
	//----------------------------
	EERIE_3D stiteangle;
	EERIE_3D stitepos;
	EERIE_3D stitescale;
	EERIE_RGB stitecolor;
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);


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
		DrawEERIEObjEx(svoodoo , &stiteangle, &stitepos, &stitescale, &stitecolor);

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
void CFireProtection::Create(long _ulDuration)
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
float CFireProtection::Render() {
	
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

float CColdProtection::Render() {
	
	return 0;
}
