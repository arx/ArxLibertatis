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
// ARX_NPC
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Minimap Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "ARX_Levels.h"
#include "ARX_Minimap.h"
#include "ARX_Text.h"
#include "EERIELight.h"
#include "EERIEPhysicsBox.h"
#include "EERIEDRAW.h"
#include "EERIEObject.h"
#include "Hermesmain.h"
#include "danae.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

MINI_MAP_DATA minimap[MAX_MINIMAPS];
float mini_offset_x[MAX_MINIMAPS];
float mini_offset_y[MAX_MINIMAPS];
float mapmaxy[32];

TextureContainer * pTexDetect = NULL;

extern long FOR_EXTERNAL_PEOPLE;

MAPMARKER_DATA * Mapmarkers = NULL;
long Nb_Mapmarkers = 0;


//-----------------------------------------------------------------------------
void ARX_MINIMAP_GetData(long SHOWLEVEL)
{
	if (minimap[SHOWLEVEL].tc == NULL)
	{
		char name[256];
		char LevelMap[256];
		GetLevelNameByNum(SHOWLEVEL, name);

		sprintf(LevelMap, "Graph\\Levels\\Level%s\\map.bmp", name);
		minimap[SHOWLEVEL].tc = MakeTCFromFile(LevelMap);

		if (minimap[SHOWLEVEL].tc) // 4 pix/meter
		{
			minimap[SHOWLEVEL].tc->Restore(GDevice);
			SpecialBorderSurface(minimap[SHOWLEVEL].tc, minimap[SHOWLEVEL].tc->m_dwOriginalWidth, minimap[SHOWLEVEL].tc->m_dwOriginalHeight);


			minimap[SHOWLEVEL].height = ARX_CLEAN_WARN_CAST_FLOAT(minimap[SHOWLEVEL].tc->m_dwHeight); 
			minimap[SHOWLEVEL].width = ARX_CLEAN_WARN_CAST_FLOAT(minimap[SHOWLEVEL].tc->m_dwWidth);


			float minx = FLT_MAX;
			float maxx = FLT_MIN;
			float miny = FLT_MAX;
			float maxy = FLT_MIN;
			EERIEPOLY * ep;
			EERIE_BKG_INFO * eg;

			for (long j = 0; j < ACTIVEBKG->Zsize; j++)
			{
				for (long i = 0; i < ACTIVEBKG->Xsize; i++)
				{
					eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

					for (long k = 0; k < eg->nbpoly; k++)
					{
						ep = &eg->polydata[k];

						if (ep)
						{
							minx = __min(minx, ep->min.x);
							maxx = __max(maxx, ep->max.x);
							miny = __min(miny, ep->min.z);
							maxy = __max(maxy, ep->max.z);
						}
					}
				}

				mapmaxy[SHOWLEVEL] = maxy;
				minimap[SHOWLEVEL].xratio = minx;
				minimap[SHOWLEVEL].yratio = miny;

				for (long iii = 0; iii < 32; iii++)
				{
					minimap[iii].offsetx = 0;
					minimap[iii].offsety = 0;
				}
			}
		}
	}
}
extern long Book_MapPage;
//-----------------------------------------------------------------------------
void ARX_MINIMAP_ValidatePos(EERIE_3D * pos)
{
	long SHOWLEVEL = ARX_LEVELS_GetRealNum(CURRENTLEVEL); 

	if ((SHOWLEVEL >= 0) && (SHOWLEVEL < 32))
	{
		if (minimap[SHOWLEVEL].tc == NULL)
		{
			ARX_MINIMAP_GetData(SHOWLEVEL);
		}

		if (minimap[CURRENTLEVEL].tc == NULL)
		{
			ARX_MINIMAP_GetData(CURRENTLEVEL);
		}

		if ((minimap[SHOWLEVEL].tc) && (minimap[SHOWLEVEL].tc->m_pddsSurface))
		{
			ARX_MINIMAP_Show(GDevice, ARX_LEVELS_GetRealNum(CURRENTLEVEL), 2);
		}
	}
}

float AM_LASTPOS_x = -999999.f;
float AM_LASTPOS_z = -999999.f;
void ARX_MINIMAP_ValidatePlayerPos()
{
	if (BLOCK_PLAYER_CONTROLS) return;

	float dist = Distance2D(AM_LASTPOS_x, AM_LASTPOS_z, player.pos.x, player.pos.z);
	float req;

	if ((player.Interface & INTER_MAP) && (!(player.Interface & INTER_COMBATMODE)) && (Book_Mode == 2))
		req = 20.f;
	else req = 80.f;

	if (dist > req)
	{
		AM_LASTPOS_x = player.pos.x;
		AM_LASTPOS_z = player.pos.z;
		ARX_MINIMAP_ValidatePos(&player.pos);
	}
}

//-----------------------------------------------------------------------------
// MINIMAP
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Load_Offsets()
{
	char fic[256];
	sprintf(fic, "%s\\Graph\\Levels\\mini_offsets.ini", Project.workingdir);

	if (PAK_FileExist(fic))
	{
		long siz = 0;
		char * dat = (char *)PAK_FileLoadMalloc(fic, &siz);
		long pos = 0;

		if (dat)
		{
			for (long i = 0; i < 29; i++)
			{
				char t[512];
				sscanf(dat + pos, "%s %f %f", t, &mini_offset_x[i], &mini_offset_y[i]);

				while ((pos < siz) && (dat[pos] != 10)) pos++;

				pos++;

				if (pos >= siz) i = 100;
			}

			free(dat);
		}

	}

	mini_offset_x[0] = 0;
	mini_offset_y[0] = -0.5;
	mini_offset_x[1] = 0;
	mini_offset_y[1] = 0;
	mini_offset_x[14] = 130;
	mini_offset_y[14] = 0;
	mini_offset_x[15] = 31;
	mini_offset_y[15] = -3.5;
}

void ARX_MINIMAP_Reveal()
{
	for (long ii = 0; ii < MAX_MINIMAPS; ii++)
	{
		for (long j = 0; j < MINIMAP_MAX_Z; j++)
			for (long i = 0; i < MINIMAP_MAX_X; i++)
			{
				minimap[ii].revealed[i][j] = 255;
			}
	}
}
//-----------------------------------------------------------------------------
void ARX_MINIMAP_FirstInit()
{
	memset(minimap, 0, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);

	for (long i = 0; i < MAX_MINIMAPS; i++)
	{
		mini_offset_x[i] = 0;
		mini_offset_y[i] = 0;
	}

	ARX_MINIMAP_Load_Offsets();
}

//-----------------------------------------------------------------------------
void ARX_MINIMAP_Reset()
{
	for (long i = 0; i < MAX_MINIMAPS; i++)
	{
		if (minimap[i].tc)
		{
			D3DTextr_KillTexture(minimap[i].tc);
			minimap[i].tc = NULL;
		}
	}

	memset(minimap, 0, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);
}

//-----------------------------------------------------------------------------
void ARX_MINIMAP_PurgeTC()
{
	for (long i = 0; i < MAX_MINIMAPS; i++)
	{
		if (minimap[i].tc)
		{
			D3DTextr_KillTexture(minimap[i].tc);
			minimap[i].tc = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
 
 
 
TextureContainer * MapMarkerTc = NULL;

float DECALY = -150;
float DECALX = +40;
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Show(LPDIRECT3DDEVICE7 m_pd3dDevice, long SHOWLEVEL, long flag, long fl2)
{
	float sstartx, sstarty;

	if (!pTexDetect)
	{
		GetTextureFile("Graph\\particles\\flare.bmp");
		char temp[256];
		MakeDir(temp, "Graph\\particles\\flare.bmp");
		pTexDetect = D3DTextr_GetSurfaceContainer(temp);
	}

	//	SHOWLEVEL=8;
	// First Load Minimap TC & DATA if needed
	if (minimap[SHOWLEVEL].tc == NULL)
	{
		ARX_MINIMAP_GetData(SHOWLEVEL);
	}

	if ((minimap[SHOWLEVEL].tc) && (minimap[SHOWLEVEL].tc->m_pddsSurface))
	{
		float startx, starty, casex, casey, ratiooo;
		float mod_x = (float)MAX_BKGX / (float)MINIMAP_MAX_X;
		float mod_z = (float)MAX_BKGZ / (float)MINIMAP_MAX_Z;

		if (flag == 1)
		{


			startx = 0;
			starty = 0;
			casex = (900) / ((float)MINIMAP_MAX_X);
			casey = (900) / ((float)MINIMAP_MAX_Z);
			ratiooo = 900.f / 250.f;

			if (fl2)
			{
				casex = (600) / ((float)MINIMAP_MAX_X);
				casey = (600) / ((float)MINIMAP_MAX_Z);
				ratiooo = 600.f / 250.f;
			}

		}
		else
		{
			startx = (140); 
			starty = (120);
			casex = (250) / ((float)MINIMAP_MAX_X);
			casey = (250) / ((float)MINIMAP_MAX_Z);
			ratiooo = 1.f;
		}

		sstartx = startx;
		sstarty = starty;


		float ofx, ofx2, ofy, ofy2, px, py;
		px = py = 0.f;

		ofx		= mini_offset_x[CURRENTLEVEL];
		ofx2	= minimap[SHOWLEVEL].xratio;
		ofy		= mini_offset_y[CURRENTLEVEL];
		ofy2	= minimap[SHOWLEVEL].yratio;

		if ((SHOWLEVEL == ARX_LEVELS_GetRealNum(CURRENTLEVEL)) || (flag == 2))
		{
			// Computes playerpos
			ofx = mini_offset_x[CURRENTLEVEL];
			ofx2 = minimap[SHOWLEVEL].xratio;
			ofy = mini_offset_y[CURRENTLEVEL];
			ofy2 = minimap[SHOWLEVEL].yratio;
		
			px = startx + ((player.pos.x + ofx - ofx2) * DIV100 * casex
			               + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x ; //DIV100*2;
			py = starty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * DIV100 * casey
			               - (player.pos.z + ofy - ofy2) * DIV100 * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z ;    //DIV100*2;

			if (flag == 1)
			{
				sstartx = startx;
				sstarty = starty;

				startx = 490.f - px;
				starty = 220.f - py;
				px += startx;
				py += starty;
			}
		}


		D3DTLVERTEX verts[4];
		SETTC(m_pd3dDevice, minimap[SHOWLEVEL].tc);

		for (long k = 0; k < 4; k++)
		{
			verts[k].color = 0xFFFFFFFF;
			verts[k].rhw = 1;
			verts[k].sz = 0.00001f;
		}

		float div = DIV25;
		TextureContainer * tc = minimap[SHOWLEVEL].tc;
		float dw = 1.f / (float)max(tc->m_dwDeviceWidth, tc->m_dwOriginalWidth); 
		float dh = 1.f / (float)max(tc->m_dwDeviceHeight, tc->m_dwOriginalHeight);
		
		float vx2 = 4.f * dw * mod_x;
		float vy2 = 4.f * dh * mod_z;

		float _px;
		RECT boundaries;
		float MOD20, MOD20DIV, divXratio, divYratio;

		boundaries.bottom = boundaries.left = boundaries.right = boundaries.top = 0;
		MOD20 = MOD20DIV = divXratio = divYratio = 0.f;

		if (flag != 2)
		{

			if (flag == 1)
			{
				MOD20 = 20.f * Xratio;
				MOD20DIV = 1.f / (MOD20);
				//@PERF do if(fl2){}else{} to make 4 and not 8 flot op if fl2.

				ARX_CHECK_LONG((360 + MOD20)*Xratio);
				ARX_CHECK_LONG((555 - MOD20)*Xratio);
				ARX_CHECK_LONG((85 + MOD20)*Yratio);
				ARX_CHECK_LONG((355 - MOD20)*Yratio);

				//CAST
				boundaries.left		=	ARX_CLEAN_WARN_CAST_LONG((360 + MOD20) * Xratio);
				boundaries.right	=	ARX_CLEAN_WARN_CAST_LONG((555 - MOD20) * Xratio);
				boundaries.top		=	ARX_CLEAN_WARN_CAST_LONG((85 + MOD20) * Yratio);
				boundaries.bottom	=	ARX_CLEAN_WARN_CAST_LONG((355 - MOD20) * Yratio);

				if (fl2)
				{
					//CHECK (DEBUG)
					ARX_CHECK_LONG((390 + MOD20)*Xratio);
					ARX_CHECK_LONG((590 - MOD20)*Xratio);
					ARX_CHECK_LONG((135 + MOD20)*Yratio);
					ARX_CHECK_LONG((295 - MOD20)*Yratio);

					//CAST
					boundaries.left		=	ARX_CLEAN_WARN_CAST_LONG((390 + MOD20) * Xratio);
					boundaries.right	=	ARX_CLEAN_WARN_CAST_LONG((590 - MOD20) * Xratio);
					boundaries.top		=	ARX_CLEAN_WARN_CAST_LONG((135 + MOD20) * Yratio);
					boundaries.bottom	=	ARX_CLEAN_WARN_CAST_LONG((295 - MOD20) * Yratio);
				}
			}

			SETALPHABLEND(m_pd3dDevice, TRUE);
			m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
			m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
			SETTEXTUREWRAPMODE(m_pd3dDevice, D3DTADDRESS_CLAMP);

			if (fl2)
			{
				m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
				m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
			}
		}
		else
		{
			divXratio = 1.f / Xratio;
			divYratio = 1.f / Yratio;
		}

		for (long j = -2; j < MINIMAP_MAX_Z + 2; j++)
		{
			for (long i = -2; i < MINIMAP_MAX_X + 2; i++)
			{
				float vx, vy, vxx, vyy;
				vxx = ((float)i * (float)ACTIVEBKG->Xdiv * mod_x);
				vyy = ((float)j * (float)ACTIVEBKG->Zdiv * mod_z);
				vx = (vxx * div) * dw;
				vy = (vyy * div) * dh;

				long okay = 1;
				float posx = (startx + i * casex) * Xratio;
				float posy = (starty + j * casey) * Yratio;

				if (flag == 1)
				{

					if	((posx < 360 * Xratio)
					        ||	(posx > 555 * Xratio)
					        ||	(posy < 85 * Yratio)
					        ||	(posy > 355 * Yratio))
						okay = 0;

					if (fl2)
					{
						okay = 1;

						if	((posx < 390 * Xratio)
						        ||	(posx > 590 * Xratio)
						        ||	(posy < 135 * Yratio)
						        ||	(posy > 295 * Yratio))
							okay = 0;
					}

				}
				else
				{
					if ((posx > 345 * Xratio)
					        ||	(posy > 290 * Yratio))
						okay = 0;
				}

				if (okay)
				{
					if ((flag == 2)
					        && (i >= 0) && (i < MINIMAP_MAX_X)
					        && (j >= 0) && (j < MINIMAP_MAX_Z))
					{
						float d = Distance2D(posx * divXratio + casex * DIV2, posy * divYratio /*-casey * 2 * Yratio*/, px, py);

						if (d <= 6.f)
						{
							long r;
							float vv = (6 - d) * DIV6;

							if (vv >= 0.5f)
								vv = 1.f;
							else if (vv > 0.f)
								vv = vv * 2.f;
							else
								vv = 0.f;

							F2L((float)(vv * 255.f), &r);


							long ucLevel =  __max(r, minimap[SHOWLEVEL].revealed[i][j]);
							ARX_CHECK_UCHAR(ucLevel);

							minimap[SHOWLEVEL].revealed[i][j] = ARX_CLEAN_WARN_CAST_UCHAR(ucLevel);


						}
					}

					if (!FOR_EXTERNAL_PEOPLE)
					{
						if ((i >= 0) && (i < MINIMAP_MAX_X)
						        &&	(j >= 0) && (j < MINIMAP_MAX_Z))
						{
							minimap[SHOWLEVEL].revealed[i][j] = 255;
						}
					}

					verts[3].sx = verts[0].sx = (posx);
					verts[1].sy = verts[0].sy = (posy);
					verts[2].sx = verts[1].sx = posx + (casex * Xratio);
					verts[3].sy = verts[2].sy = posy + (casey * Yratio);

					verts[3].tu = verts[0].tu = vx;
					verts[1].tv = verts[0].tv = vy;
					verts[2].tu = verts[1].tu = vx + vx2;
					verts[3].tv = verts[2].tv = vy + vy2;

					if (flag != 2)
					{
						float v;
						float oo = 0.f;

						if ((i < 0) || (i >= MINIMAP_MAX_X) || (j < 0) || (j >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[i][j]) * DIV255;

						if (flag == 1)
						{
							long vert = 0;
							_px = verts[vert].sx - boundaries.left;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.right - verts[vert].sx;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = verts[vert].sy - boundaries.top;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.bottom - verts[vert].sy;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;
						}

						if (fl2) verts[0].color = D3DRGB(v * DIV2, v * DIV2, v * DIV2);
						else
							verts[0].color = D3DRGB(v, v, v);

						oo += v;

						if ((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j < 0) || (j >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[__min(i+1, MINIMAP_MAX_X-1)][j]) * DIV255;

						if (flag == 1)
						{
							long vert = 1;
							_px = verts[vert].sx - boundaries.left;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.right - verts[vert].sx;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = verts[vert].sy - boundaries.top;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.bottom - verts[vert].sy;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;
						}

						if (fl2) verts[1].color = D3DRGB(v * DIV2, v * DIV2, v * DIV2);
						else
							verts[1].color = D3DRGB(v, v, v);

						oo += v;

						if ((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[__min(i+1, MINIMAP_MAX_X-1)][__min(j+1, MINIMAP_MAX_Z-1)]) * DIV255;

						if (flag == 1)
						{
							long vert = 2;
							_px = verts[vert].sx - boundaries.left;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.right - verts[vert].sx;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = verts[vert].sy - boundaries.top;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.bottom - verts[vert].sy;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;
						}
						

						if (fl2) verts[2].color = D3DRGB(v * DIV2, v * DIV2, v * DIV2);
						else
							verts[2].color = D3DRGB(v, v, v);

						oo += v;

						if ((i < 0) || (i >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[i][__min(j+1, MINIMAP_MAX_Z-1)]) * DIV255;

						if (flag == 1)
						{
							long vert = 3;
							_px = verts[vert].sx - boundaries.left;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.right - verts[vert].sx;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = verts[vert].sy - boundaries.top;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;

							_px = boundaries.bottom - verts[vert].sy;

							if (_px < 0.f) v = 0.f;
							else if (_px < MOD20) v *= _px * MOD20DIV;
						}

						if (fl2) verts[3].color = D3DRGB(v * DIV2, v * DIV2, v * DIV2);
						else
							verts[3].color = D3DRGB(v, v, v);

						oo += v;

						if (oo > 0.f)
						{
							if (fl2)
							{
								verts[0].sx += DECALX * Xratio;
								verts[0].sy += DECALY * Yratio;
								verts[1].sx += DECALX * Xratio;
								verts[1].sy += DECALY * Yratio;
								verts[2].sx += DECALX * Xratio;
								verts[2].sy += DECALY * Yratio;
								verts[3].sx += DECALX * Xratio;
								verts[3].sy += DECALY * Yratio;
							}

							EERIEDRAWPRIM(GDevice, D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, 4, 0);
						}
					}
				}
			}
		}

		if (flag != 2)
		{
			m_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS , D3DTADDRESS_WRAP);
			m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL);

			SETALPHABLEND(m_pd3dDevice, FALSE);

			if ((SHOWLEVEL == ARX_LEVELS_GetRealNum(CURRENTLEVEL)))
			{
				// Now Draws Playerpos/angle
				verts[0].color = 0xFFFF0000;
				verts[1].color = 0xFFFF0000;
				verts[2].color = 0xFFFF0000;
				float val;

				if (flag == 1) val = 6.f;
				else val = 3.f;

				float rx = 0.f;
				float ry = -val * 1.8f;
				float rx2 = -val * DIV2;
				float ry2 = val;
				float rx3 = val * DIV2;
				float ry3 = val;

				float angle = DEG2RAD(player.angle.b);
				float ca = EEcos(angle);
				float sa = EEsin(angle);

				verts[0].sx = (px + rx2 * ca + ry2 * sa) * Xratio;
				verts[0].sy = (py + ry2 * ca - rx2 * sa) * Yratio;
				verts[1].sx = (px + rx * ca + ry * sa) * Xratio;
				verts[1].sy = (py + ry * ca - rx * sa) * Yratio;
				verts[2].sx = (px + rx3 * ca + ry3 * sa) * Xratio;
				verts[2].sy = (py + ry3 * ca - rx3 * sa) * Yratio;

				SETTC(GDevice, NULL);

				if (fl2)
				{
					SETALPHABLEND(m_pd3dDevice, TRUE);
					verts[0].sx += DECALX * Xratio;
					verts[0].sy += DECALY * Yratio;
					verts[1].sx += DECALX * Xratio;
					verts[1].sy += DECALY * Yratio;
					verts[2].sx += DECALX * Xratio;
					verts[2].sy += DECALY * Yratio;
				}

				EERIEDRAWPRIM(GDevice, D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, 3, 0);

				if (fl2) SETALPHABLEND(m_pd3dDevice, FALSE);
			}
		}

		// tsu
		for (long lnpc = 1; lnpc < inter.nbmax; lnpc++)
		{
			if ((inter.iobj[lnpc] != NULL) && (inter.iobj[lnpc]->ioflags & IO_NPC))
			{
				if (inter.iobj[lnpc]->_npcdata->life > 0.f)
					if (!((inter.iobj[lnpc]->GameFlags & GFLAG_MEGAHIDE) ||
					        (inter.iobj[lnpc]->show == SHOW_FLAG_MEGAHIDE))
					        && (inter.iobj[lnpc]->show == SHOW_FLAG_IN_SCENE))
						if (!(inter.iobj[lnpc]->show == SHOW_FLAG_HIDDEN))
							if (inter.iobj[lnpc]->_npcdata->fDetect >= 0)
							{
								if (player.Full_Skill_Etheral_Link >= inter.iobj[lnpc]->_npcdata->fDetect)
								{
									float fpx;
									float fpy;
								
									fpx = sstartx + ((inter.iobj[lnpc]->pos.x - 100 + ofx - ofx2) * DIV100 * casex
									                 + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x; 
									fpy = sstarty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * DIV100 * casey
									                 - (inter.iobj[lnpc]->pos.z + 200 + ofy - ofy2) * DIV100 * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z; 

									if (flag == 1)
									{

										fpx = startx + ((inter.iobj[lnpc]->pos.x - 100 + ofx - ofx2) * DIV100 * casex
										                + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x; 
										fpy = starty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * DIV100 * casey
										                - (inter.iobj[lnpc]->pos.z + 200 + ofy - ofy2) * DIV100 * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z; 


									}

									float d = Distance2D(player.pos.x, player.pos.z, inter.iobj[lnpc]->pos.x, inter.iobj[lnpc]->pos.z);

		
									if ((d <= 800) && (fabs(inter.iobj[0]->pos.y - inter.iobj[lnpc]->pos.y) < 250.f))
									{
										float col = 1.f;

										if (d > 600.f)
										{
											col = 1.f - (d - 600.f) * DIV200;
										}

										if (!fl2)
										{
											SETALPHABLEND(m_pd3dDevice, true);
											m_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
											m_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
										}
										else
											SETALPHABLEND(m_pd3dDevice, true);

										if (fl2)
										{
											fpx += DECALX * Xratio;
											fpy += (DECALY + 15) * Yratio;
										}

										fpx *= Xratio;
										fpy *= Yratio;
										EERIEDrawBitmap(GDevice, fpx, fpy,
										                5.f * ratiooo, 5.f * ratiooo, 0, pTexDetect, D3DRGB(col, 0, 0));

										if (!fl2)
											SETALPHABLEND(m_pd3dDevice, false);
									}
								}
							}
			}
		}

		if (flag == 0)
			for (long i = 0; i < Nb_Mapmarkers; i++)
			{
				if (Mapmarkers[i].lvl == SHOWLEVEL + 1)
				{
					float pos_x = Mapmarkers[i].x * 8 * ratiooo * ACTIVEBKG->Xmul * casex + startx;
					float pos_y = Mapmarkers[i].y * 8 * ratiooo * ACTIVEBKG->Zmul * casey + starty;
					float size = 5.f * ratiooo;
					verts[0].color = 0xFFFF0000;
					verts[1].color = 0xFFFF0000;
					verts[2].color = 0xFFFF0000;
					verts[3].color = 0xFFFF0000;
					verts[0].sx = (pos_x - size) * Xratio;
					verts[0].sy = (pos_y - size) * Yratio;
					verts[1].sx = (pos_x + size) * Xratio;
					verts[1].sy = (pos_y - size) * Yratio;
					verts[2].sx = (pos_x + size) * Xratio;
					verts[2].sy = (pos_y + size) * Yratio;
					verts[3].sx = (pos_x - size) * Xratio;
					verts[3].sy = (pos_y + size) * Yratio;
					verts[0].tu = 0.f;
					verts[0].tv = 0.f;
					verts[1].tu = 1.f;
					verts[1].tv = 0.f;
					verts[2].tu = 1.f;
					verts[2].tv = 1.f;
					verts[3].tu = 0.f;
					verts[3].tv = 1.f;

					if ((!fl2)
					        && (MouseInRect(verts[0].sx, verts[0].sy, verts[2].sx, verts[2].sy)))
					{
						if (!Mapmarkers[i].tstring)
						{
							_TCHAR output[4096];
							MakeLocalised(Mapmarkers[i].string, output, 4096, 0);
							Mapmarkers[i].tstring = (_TCHAR *)malloc((_tcslen(output) + 1) * sizeof(_TCHAR));
							ZeroMemory(Mapmarkers[i].tstring, (_tcslen(output) + 1)*sizeof(_TCHAR));
							_tcscpy(Mapmarkers[i].tstring, output);
						}

						if (Mapmarkers[i].tstring)
						{
							RECT rRect, bRect;
							SetRect(&bRect	, (140),	(290)
							        , (140 + 205),	(358));

							float fLeft		= (bRect.left) * Xratio ;
							float fRight	= (bRect.right) * Xratio ;
							float fTop		= (bRect.top) * Yratio ;
							float fBottom	= (bRect.bottom) * Yratio ;
							ARX_CHECK_INT(fLeft);
							ARX_CHECK_INT(fRight);
							ARX_CHECK_INT(fTop);
							ARX_CHECK_INT(fBottom);

							SetRect(&rRect
							        , ARX_CLEAN_WARN_CAST_INT(fLeft)
							        , ARX_CLEAN_WARN_CAST_INT(fTop)
							        , ARX_CLEAN_WARN_CAST_INT(fRight)
							        , ARX_CLEAN_WARN_CAST_INT(fBottom));


							long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(
							                       hFontInGameNote, Mapmarkers[i].tstring, 0, rRect);

							danaeApp.DANAEEndRender();
							_TCHAR	Page_Buffer[256];
							_tcsncpy(Page_Buffer, Mapmarkers[i].tstring, lLengthDraw);
							Page_Buffer[lLengthDraw] = _T('\0');

							DrawBookTextInRect(ARX_CLEAN_WARN_CAST_FLOAT(bRect.left), ARX_CLEAN_WARN_CAST_FLOAT(bRect.top),
							                   ARX_CLEAN_WARN_CAST_FLOAT(bRect.right), ARX_CLEAN_WARN_CAST_FLOAT(bRect.bottom),
							                   Page_Buffer, 0, 0x00FF00FF,
							                   hFontInGameNote);


							danaeApp.DANAEStartRender();
						}
					}

					if (MapMarkerTc == NULL)
					{
						MapMarkerTc = MakeTCFromFile("Graph\\interface\\icons\\mapmarker.bmp");
					}

					SETTC(GDevice, MapMarkerTc);

					if (fl2)
					{
						verts[0].sx += DECALX * Xratio;
						verts[0].sy += DECALY * Yratio;
						verts[1].sx += DECALX * Xratio;
						verts[1].sy += DECALY * Yratio;
						verts[2].sx += DECALX * Xratio;
						verts[2].sy += DECALY * Yratio;
						verts[3].sx += DECALX * Xratio;
						verts[3].sy += DECALY * Yratio;
					}

					EERIEDRAWPRIM(GDevice, D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE, verts, 4, 0);
				}
			}

	}
}

void ARX_MAPMARKER_Init()
{
	if (Mapmarkers)
	{
		for (long i = 0; i < Nb_Mapmarkers; i++)
		{
			if (Mapmarkers[i].tstring)
				free(Mapmarkers[i].tstring);

			Mapmarkers[i].tstring = NULL;
		}

		free(Mapmarkers);
	}

	Mapmarkers = NULL;
	Nb_Mapmarkers = 0;
}
long ARX_MAPMARKER_Get(char * str)
{
	for (long i = 0; i < Nb_Mapmarkers; i++)
	{
		if (!stricmp(Mapmarkers[i].string, str))
			return i;
	}

	return -1;
}
void ARX_MAPMARKER_Add(float x, float y, long lvl, char * temp)
{
	long num = ARX_MAPMARKER_Get(temp);

	if (num >= 0) // already exists
	{
		Mapmarkers[num].lvl = lvl;
		Mapmarkers[num].x = x;
		Mapmarkers[num].y = y;

		if (Mapmarkers[num].tstring)
			free(Mapmarkers[num].tstring);

		Mapmarkers[num].tstring = NULL;
		strcpy(Mapmarkers[num].string, temp);
		return;
	}

	Mapmarkers = (MAPMARKER_DATA *)realloc(Mapmarkers, sizeof(MAPMARKER_DATA) * (Nb_Mapmarkers + 1));

	// Memory Error Handling
	if (!Mapmarkers)
	{
		Nb_Mapmarkers = 0;
		return;
	}

	Mapmarkers[Nb_Mapmarkers].lvl = lvl;
	Mapmarkers[Nb_Mapmarkers].x = x;
	Mapmarkers[Nb_Mapmarkers].y = y;
	Mapmarkers[Nb_Mapmarkers].tstring = NULL;
	strcpy(Mapmarkers[Nb_Mapmarkers].string, temp);
	Nb_Mapmarkers++;
}
void ARX_MAPMARKER_Remove(char * temp)
{
	long num = ARX_MAPMARKER_Get(temp);

	if (num < 0) return; // Doesn't exists

	if (Nb_Mapmarkers <= 1)
	{
		ARX_MAPMARKER_Init();
		return;
	}

	if (Mapmarkers[num].tstring)
		free(Mapmarkers[num].tstring);

	Mapmarkers[num].tstring = NULL;

	for (long i = num; i < Nb_Mapmarkers - 1; i++)
	{
		memcpy(&Mapmarkers[i], &Mapmarkers[i+1], sizeof(MAPMARKER_DATA));
	}

	Mapmarkers = (MAPMARKER_DATA *)realloc(Mapmarkers, sizeof(MAPMARKER_DATA) * (Nb_Mapmarkers - 1));
	Nb_Mapmarkers--;
}
