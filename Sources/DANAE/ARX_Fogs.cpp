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
// ARX_Fogs.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Fog Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////////////
#include "ARX_Fogs.h"
#include "ARX_Particles.h"
#include "ARX_time.h"
#include "ARX_menu2.h"

#include "EERIEMath.h"
#include "EERIEDraw.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


EERIE_3DOBJ * fogobj = NULL;
extern FOG_DEF fogcopy;
FOG_DEF fogs[MAX_FOG];

extern CMenuConfig * pMenuConfig;

//*************************************************************************************
// Used to Set 3D Object Visual for Fogs
//*************************************************************************************
void ARX_FOGS_Set_Object(EERIE_3DOBJ * _fogobj)
{
	fogobj = _fogobj;
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_FirstInit()
{
	memset(&fogcopy, 0, sizeof(FOG_DEF));
	fogcopy.frequency = 17.f;
	fogcopy.rgb.r = 0.3f;
	fogcopy.rgb.g = 0.3f;
	fogcopy.rgb.b = 0.5f;
	fogcopy.rotatespeed = 0.001f;
	fogcopy.scale = 8.f;
	fogcopy.size = 80.f;
	fogcopy.speed = 1.f;
	fogcopy.tolive = 4500;
	ARX_FOGS_Clear();
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_Clear()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		memset(&fogs[i], 0, sizeof(FOG_DEF));
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_TranslateSelected(EERIE_3D * trans)
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].selected)
		{
			fogs[i].pos.x += trans->x;
			fogs[i].pos.y += trans->y;
			fogs[i].pos.z += trans->z;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_UnselectAll()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		fogs[i].selected = 0;
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_Select(long n)
{
	if (fogs[n].selected) fogs[n].selected = 0;
	else fogs[n].selected = 1;
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_KillByIndex(long num)
{
	if ((num >= 0) && (num < MAX_FOG))
	{
		memset(&fogs[num], 0, sizeof(FOG_DEF));
	}
}

void ARX_FOGS_KillSelected()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].selected)
		{
			ARX_FOGS_KillByIndex(i);
		}
	}
}
//*************************************************************************************
//*************************************************************************************
long ARX_FOGS_GetFree()
{
	for (long i = 0; i < MAX_FOG; i++)
	{
		if (!fogs[i].exist)  return i;
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
long ARX_FOGS_Count()
{
	long count = 0;

	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)  count++;
	}

	return count;
}
void ARX_FOGS_TimeReset()
{
}
void AddPoisonFog(EERIE_3D * pos, float power)
{
	int iDiv = 2;

	if (pMenuConfig)
	{
		iDiv += 2 - pMenuConfig->iLevelOfDetails;
	}

	float flDiv =	ARX_CLEAN_WARN_CAST_FLOAT(1 << iDiv);
	ARX_CHECK_LONG(FrameDiff / flDiv);
	long count	=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff / flDiv);

	if (count < 1) count = 1;

	while (count) 
	{
		count--;
		long j = ARX_PARTICLES_GetFree();

		if ((j != -1) && (!ARXPausedTimer) && (rnd() * 2000.f < power))
		{
			ParticleCount++;
			particle[j].special		=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			particle[j].exist		=	TRUE;
			particle[j].zdec		=	0;
			particle[j].ov.x		=	pos->x + 100.f - 200.f * rnd();
			particle[j].ov.y		=	pos->y + 100.f - 200.f * rnd();
			particle[j].ov.z		=	pos->z + 100.f - 200.f * rnd();
			float speed				=	1.f;
			float fval				=	speed * DIV5;
			particle[j].scale.x		=	particle[j].scale.y		=	particle[j].scale.z		=	10.f;
			particle[j].move.x		=	(speed - rnd()) * fval;
			particle[j].move.y		=	(speed - speed * rnd()) * DIV15;
			particle[j].move.z		=	(speed - rnd()) * fval;
			particle[j].scale.x		=	particle[j].scale.y		=	8;
			particle[j].timcreation	=	ARX_CLEAN_WARN_CAST_LONG(ARX_TIME_Get());
			particle[j].tolive		=	4500 + (unsigned long)(rnd() * 4500);
			particle[j].tc			=	TC_smoke;
			particle[j].siz			=	(80 + rnd() * 80 * 2.f) * DIV3;
			particle[j].r			=	rnd() * DIV3;
			particle[j].g			=	1.f;
			particle[j].b			=	rnd() * DIV10;
			particle[j].fparam		=	0.001f;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_Render(long init)
{
	if (ARXPausedTimer) return;

	int iDiv = 2;

	if (pMenuConfig)
	{
		iDiv += 2 - pMenuConfig->iLevelOfDetails;
	}

	float flDiv = ARX_CLEAN_WARN_CAST_FLOAT(1 << iDiv);

	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)
		{
			unsigned long offs;
			float fval;

			offs = 0;
		

			ARX_CHECK_LONG(FrameDiff / flDiv);
			long count	=	ARX_CLEAN_WARN_CAST_LONG(FrameDiff / flDiv);


			if (count < 1) count = 1;

			while (count)
			{
				count--;
				long j = ARX_PARTICLES_GetFree();

				if ((j != -1) && (rnd() * 2000.f < (float)fogs[i].frequency))
				{
					ParticleCount++;
					particle[j].special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					particle[j].exist = TRUE;
					particle[j].zdec = 0;

					if (fogs[i].special & FOG_DIRECTIONAL)
					{
						particle[j].ov.x = fogs[i].pos.x;
						particle[j].ov.y = fogs[i].pos.y;
						particle[j].ov.z = fogs[i].pos.z;
						fval = fogs[i].speed * DIV10;
						particle[j].move.x = (fogs[i].move.x * fval);
						particle[j].move.y = (fogs[i].move.y * fval);
						particle[j].move.z = (fogs[i].move.z * fval);
					}
					else
					{
						particle[j].ov.x = fogs[i].pos.x + 100.f - 200.f * rnd();
						particle[j].ov.y = fogs[i].pos.y + 100.f - 200.f * rnd();
						particle[j].ov.z = fogs[i].pos.z + 100.f - 200.f * rnd();
						fval = fogs[i].speed * DIV5;
						particle[j].move.x = (fogs[i].speed - rnd() * 2.f) * fval;
						particle[j].move.y = (fogs[i].speed - rnd() * 2.f) * DIV15;
						particle[j].move.z = (fogs[i].speed - rnd() * 2.f) * fval;
					}

					particle[j].scale.x		=	particle[j].scale.y		=	particle[j].scale.z		=	fogs[i].scale;
					particle[j].timcreation	=	lARXTime;
					particle[j].tolive		=	fogs[i].tolive + (unsigned long)(rnd() * fogs[i].tolive);
					particle[j].tc			=	TC_smoke;
					particle[j].siz			=	(fogs[i].size + rnd() * fogs[i].size * 2.f) * DIV3;
					particle[j].r			=	fogs[i].rgb.r;
					particle[j].g			=	fogs[i].rgb.g;
					particle[j].b			=	fogs[i].rgb.b;
					particle[j].fparam		=	fogs[i].rotatespeed;
				}

				fogs[i].lastupdate = ARXTimeUL(); 
			}
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void ARX_FOGS_RenderAll(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	EERIE_3D angle;
	Vector_Init(&angle); 

	SETALPHABLEND(m_pd3dDevice, FALSE);

	for (long i = 0; i < MAX_FOG; i++)
	{
		if (fogs[i].exist)
		{
			if (fogobj)
				DrawEERIEInter(m_pd3dDevice, fogobj, &angle, &fogs[i].pos, NULL);

			Vector_Copy(&fogs[i].bboxmin, &BBOXMIN);
			Vector_Copy(&fogs[i].bboxmax, &BBOXMAX);

			if (fogs[i].special & FOG_DIRECTIONAL)
			{
				EERIE_3D orgn, dest;
				orgn.x = fogs[i].pos.x;
				orgn.y = fogs[i].pos.y;
				orgn.z = fogs[i].pos.z;
				dest.x = orgn.x + fogs[i].move.x * 50.f;
				dest.y = orgn.y + fogs[i].move.y * 50.f;
				dest.z = orgn.z + fogs[i].move.z * 50.f;
				EERIEDraw3DLine(m_pd3dDevice, &orgn, &dest, EERIECOLOR_WHITE); 
			}

			if (fogs[i].selected)
			{
				EERIEDraw2DLine(m_pd3dDevice, fogs[i].bboxmin.x, fogs[i].bboxmin.y, fogs[i].bboxmax.x, fogs[i].bboxmin.y, 0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, fogs[i].bboxmax.x, fogs[i].bboxmin.y, fogs[i].bboxmax.x, fogs[i].bboxmax.y, 0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, fogs[i].bboxmax.x, fogs[i].bboxmax.y, fogs[i].bboxmin.x, fogs[i].bboxmax.y, 0.01f, EERIECOLOR_YELLOW);
				EERIEDraw2DLine(m_pd3dDevice, fogs[i].bboxmin.x, fogs[i].bboxmax.y, fogs[i].bboxmin.x, fogs[i].bboxmin.y, 0.01f, EERIECOLOR_YELLOW);
			}
		}
	}
}
