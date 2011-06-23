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

#include "gui/MiniMap.h"

#include <cstdio>

#include "core/Core.h"
#include "core/Localisation.h"

#include "game/Levels.h"
#include "game/Player.h"

#include "gui/Text.h"

#include "graphics/Draw.h"

#include "io/PakManager.h"
#include "io/Logger.h"

#include "physics/Box.h"

#include "scene/Light.h"
#include "scene/Object.h"
#include "scene/Interactive.h"

using std::min;
using std::max;

MINI_MAP_DATA minimap[MAX_MINIMAPS];
float mini_offset_x[MAX_MINIMAPS];
float mini_offset_y[MAX_MINIMAPS];
float mapmaxy[32];

TextureContainer * pTexDetect = NULL;

extern long FOR_EXTERNAL_PEOPLE;

std::vector<MAPMARKER_DATA> Mapmarkers;


//-----------------------------------------------------------------------------
void ARX_MINIMAP_GetData(long SHOWLEVEL)
{
	if (minimap[SHOWLEVEL].tc == NULL)
	{
		char name[256];
		char LevelMap[256];
		GetLevelNameByNum(SHOWLEVEL, name);

		sprintf(LevelMap, "Graph\\Levels\\Level%s\\map.bmp", name);
		minimap[SHOWLEVEL].tc = TextureContainer::Load(LevelMap);

		if (minimap[SHOWLEVEL].tc) // 4 pix/meter
		{
			//TODO-RENDERING: SpecialBorderSurface...
			//SpecialBorderSurface(minimap[SHOWLEVEL].tc, minimap[SHOWLEVEL].tc->m_dwWidth, minimap[SHOWLEVEL].tc->m_dwHeight);

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
							minx = min(minx, ep->min.x);
							maxx = max(maxx, ep->max.x);
							miny = min(miny, ep->min.z);
							maxy = max(maxy, ep->max.z);
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
void ARX_MINIMAP_ValidatePos() {
	
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

		if (minimap[SHOWLEVEL].tc)
		{
			ARX_MINIMAP_Show( ARX_LEVELS_GetRealNum(CURRENTLEVEL), 2);
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

	if ((player.Interface & INTER_MAP) && (!(player.Interface & INTER_COMBATMODE)) && (Book_Mode == BOOKMODE_MINIMAP))
		req = 20.f;
	else req = 80.f;

	if (dist > req)
	{
		AM_LASTPOS_x = player.pos.x;
		AM_LASTPOS_z = player.pos.z;
		ARX_MINIMAP_ValidatePos();
	}
}

//-----------------------------------------------------------------------------
// MINIMAP
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Load_Offsets()
{
	const char INI_MINI_OFFSETS[] = "Graph\\Levels\\mini_offsets.ini";

	if (PAK_FileExist(INI_MINI_OFFSETS))
	{
		size_t siz = 0;
		char * dat = (char *)PAK_FileLoadMallocZero(INI_MINI_OFFSETS, siz);

		if(dat) {
			size_t pos = 0;
			for (long i = 0; i < 29; i++)
			{
				char t[512];
				int nRead = sscanf(dat + pos, "%s %f %f", t, &mini_offset_x[i], &mini_offset_y[i]);
				if(nRead != 3) {
					LogError << "Error parsing line " << i << " of mini_offsets.ini: read " << nRead;
				}

				while ((pos < siz) && (dat[pos] != '\n')) pos++;

				pos++;

				if (pos >= siz) break;
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
	for (size_t ii = 0; ii < MAX_MINIMAPS; ii++)
	{
		for (size_t j = 0; j < MINIMAP_MAX_Z; j++)
			for (size_t i = 0; i < MINIMAP_MAX_X; i++)
			{
				minimap[ii].revealed[i][j] = 255;
			}
	}
}
//-----------------------------------------------------------------------------
void ARX_MINIMAP_FirstInit()
{
	memset(minimap, 0, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);

	for (size_t i = 0; i < MAX_MINIMAPS; i++)
	{
		mini_offset_x[i] = 0;
		mini_offset_y[i] = 0;
	}

	ARX_MINIMAP_Load_Offsets();
}

//-----------------------------------------------------------------------------
void ARX_MINIMAP_Reset()
{
	for (size_t i = 0; i < MAX_MINIMAPS; i++)
	{
		if (minimap[i].tc)
		{
			delete minimap[i].tc;
			minimap[i].tc = NULL;
		}
	}

	memset(minimap, 0, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);
}

//-----------------------------------------------------------------------------
void ARX_MINIMAP_PurgeTC()
{
	for (size_t i = 0; i < MAX_MINIMAPS; i++)
	{
		if (minimap[i].tc)
		{
			delete minimap[i].tc;
			minimap[i].tc = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
 
 
 
TextureContainer * MapMarkerTc = NULL;

float DECALY = -150;
float DECALX = +40;
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Show(long SHOWLEVEL, long flag, long fl2)
{
	// Nuky - centralized some constants and dezoomed ingame minimap
	static const int FL2_SIZE = 300;
	static const int FL2_LEFT = 390;
	static const int FL2_RIGHT = 590;
	static const int FL2_TOP = 135;
	static const int FL2_BOTTOM = 295;
	static const float FL2_PLAYERSIZE = 4.f;

	if (!pTexDetect)
		pTexDetect = TextureContainer::Load("Graph\\particles\\flare.bmp");

	//	SHOWLEVEL=8;
	// First Load Minimap TC & DATA if needed
	if (minimap[SHOWLEVEL].tc == NULL)
	{
		ARX_MINIMAP_GetData(SHOWLEVEL);
	}

	if (minimap[SHOWLEVEL].tc)
	{
		float sstartx, sstarty;
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
				casex = (FL2_SIZE) / ((float)MINIMAP_MAX_X);
				casey = (FL2_SIZE) / ((float)MINIMAP_MAX_Z);
				ratiooo = FL2_SIZE / 250.f;
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
		
			px = startx + ((player.pos.x + ofx - ofx2) * ( 1.0f / 100 ) * casex
			               + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x ; //( 1.0f / 100 )*2;
			py = starty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * ( 1.0f / 100 ) * casey
			               - (player.pos.z + ofy - ofy2) * ( 1.0f / 100 ) * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z ;    //( 1.0f / 100 )*2;

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


		TexturedVertex verts[4];
		GRenderer->SetTexture(0, minimap[SHOWLEVEL].tc);

		for (long k = 0; k < 4; k++)
		{
			verts[k].color = 0xFFFFFFFF;
			verts[k].rhw = 1;
			verts[k].sz = 0.00001f;
		}

		float div = ( 1.0f / 25 );
		TextureContainer * tc = minimap[SHOWLEVEL].tc;
		float dw = 1.f / tc->m_dwDeviceWidth; 
		float dh = 1.f / tc->m_dwDeviceHeight;
		
		float vx2 = 4.f * dw * mod_x;
		float vy2 = 4.f * dh * mod_z;

		float _px;
		Rect boundaries;
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
					ARX_CHECK_LONG((FL2_LEFT + MOD20)*Xratio);
					ARX_CHECK_LONG((FL2_RIGHT - MOD20)*Xratio);
					ARX_CHECK_LONG((FL2_TOP + MOD20)*Yratio);
					ARX_CHECK_LONG((FL2_BOTTOM - MOD20)*Yratio);

					//CAST
					boundaries.left		=	ARX_CLEAN_WARN_CAST_LONG((FL2_LEFT + MOD20) * Xratio);
					boundaries.right	=	ARX_CLEAN_WARN_CAST_LONG((FL2_RIGHT - MOD20) * Xratio);
					boundaries.top		=	ARX_CLEAN_WARN_CAST_LONG((FL2_TOP + MOD20) * Yratio);
					boundaries.bottom	=	ARX_CLEAN_WARN_CAST_LONG((FL2_BOTTOM - MOD20) * Yratio);
				}
			}

			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
			GRenderer->SetRenderState(Renderer::DepthTest, false);
			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

			if (fl2)
			{
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendInvSrcColor);
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

						if	((posx < FL2_LEFT * Xratio)
						        ||	(posx > FL2_RIGHT * Xratio)
						        ||	(posy < FL2_TOP * Yratio)
						        ||	(posy > FL2_BOTTOM * Yratio))
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
						float d = Distance2D(posx * divXratio + casex * ( 1.0f / 2 ), posy * divYratio /*-casey * 2 * Yratio*/, px, py);

						if (d <= 6.f)
						{
							long r;
							float vv = (6 - d) * ( 1.0f / 6 );

							if (vv >= 0.5f)
								vv = 1.f;
							else if (vv > 0.f)
								vv = vv * 2.f;
							else
								vv = 0.f;

							r = vv * 255.f;


							long ucLevel =  max(r, (long)minimap[SHOWLEVEL].revealed[i][j]);
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
						else v = ((float)minimap[SHOWLEVEL].revealed[i][j]) * ( 1.0f / 255 );

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

						if (fl2) verts[0].color = D3DRGB(v * ( 1.0f / 2 ), v * ( 1.0f / 2 ), v * ( 1.0f / 2 ));
						else
							verts[0].color = D3DRGB(v, v, v);

						oo += v;

						if ((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j < 0) || (j >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[min((int)i+1, MINIMAP_MAX_X-1)][j]) * ( 1.0f / 255 );

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

						if (fl2) verts[1].color = D3DRGB(v * ( 1.0f / 2 ), v * ( 1.0f / 2 ), v * ( 1.0f / 2 ));
						else
							verts[1].color = D3DRGB(v, v, v);

						oo += v;

						if ((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[min((int)i+1, MINIMAP_MAX_X-1)][min((int)j+1, MINIMAP_MAX_Z-1)]) * ( 1.0f / 255 );

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
						

						if (fl2) verts[2].color = D3DRGB(v * ( 1.0f / 2 ), v * ( 1.0f / 2 ), v * ( 1.0f / 2 ));
						else
							verts[2].color = D3DRGB(v, v, v);

						oo += v;

						if ((i < 0) || (i >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) v = 0;
						else v = ((float)minimap[SHOWLEVEL].revealed[i][min((int)j+1, MINIMAP_MAX_Z-1)]) * ( 1.0f / 255 );

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

						if (fl2) verts[3].color = D3DRGB(v * ( 1.0f / 2 ), v * ( 1.0f / 2 ), v * ( 1.0f / 2 ));
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

							EERIEDRAWPRIM(Renderer::TriangleFan, verts, 4);
						}
					}
				}
			}
		}

		if (flag != 2)
		{
			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			GRenderer->SetRenderState(Renderer::DepthTest, true);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);

			if(SHOWLEVEL == ARX_LEVELS_GetRealNum(CURRENTLEVEL)) {
				
				// Now Draws Playerpos/angle
				verts[0].color = 0xFFFF0000;
				verts[1].color = 0xFFFF0000;
				verts[2].color = 0xFFFF0000;
				float val;

				if (flag == 1) val = 6.f;
				else val = 3.f;

				if (fl2) val = FL2_PLAYERSIZE;

				float rx = 0.f;
				float ry = -val * 1.8f;
				float rx2 = -val * ( 1.0f / 2 );
				float ry2 = val;
				float rx3 = val * ( 1.0f / 2 );
				float ry3 = val;

				float angle = radians(player.angle.b);
				float ca = EEcos(angle);
				float sa = EEsin(angle);

				verts[0].sx = (px + rx2 * ca + ry2 * sa) * Xratio;
				verts[0].sy = (py + ry2 * ca - rx2 * sa) * Yratio;
				verts[1].sx = (px + rx * ca + ry * sa) * Xratio;
				verts[1].sy = (py + ry * ca - rx * sa) * Yratio;
				verts[2].sx = (px + rx3 * ca + ry3 * sa) * Xratio;
				verts[2].sy = (py + ry3 * ca - rx3 * sa) * Yratio;

				GRenderer->ResetTexture(0);

				if (fl2)
				{
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					verts[0].sx += DECALX * Xratio;
					verts[0].sy += DECALY * Yratio;
					verts[1].sx += DECALX * Xratio;
					verts[1].sy += DECALY * Yratio;
					verts[2].sx += DECALX * Xratio;
					verts[2].sy += DECALY * Yratio;
				}

				EERIEDRAWPRIM(Renderer::TriangleFan, verts);

				if (fl2) GRenderer->SetRenderState(Renderer::AlphaBlending, false);
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
								
									fpx = sstartx + ((inter.iobj[lnpc]->pos.x - 100 + ofx - ofx2) * ( 1.0f / 100 ) * casex
									                 + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x; 
									fpy = sstarty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * ( 1.0f / 100 ) * casey
									                 - (inter.iobj[lnpc]->pos.z + 200 + ofy - ofy2) * ( 1.0f / 100 ) * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z; 

									if (flag == 1)
									{

										fpx = startx + ((inter.iobj[lnpc]->pos.x - 100 + ofx - ofx2) * ( 1.0f / 100 ) * casex
										                + mini_offset_x[CURRENTLEVEL] * ratiooo * mod_x) / mod_x; 
										fpy = starty + ((mapmaxy[SHOWLEVEL] - ofy - ofy2) * ( 1.0f / 100 ) * casey
										                - (inter.iobj[lnpc]->pos.z + 200 + ofy - ofy2) * ( 1.0f / 100 ) * casey + mini_offset_y[CURRENTLEVEL] * ratiooo * mod_z) / mod_z; 


									}

									float d = Distance2D(player.pos.x, player.pos.z, inter.iobj[lnpc]->pos.x, inter.iobj[lnpc]->pos.z);


									if ((d <= 800) && (fabs(inter.iobj[0]->pos.y - inter.iobj[lnpc]->pos.y) < 250.f))
									{
										float col = 1.f;

										if (d > 600.f)
										{
											col = 1.f - (d - 600.f) * ( 1.0f / 200 );
										}

										if (!fl2)
										{
											GRenderer->SetRenderState(Renderer::AlphaBlending, true);
											GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
										}
										else
											GRenderer->SetRenderState(Renderer::AlphaBlending, true);

										if (fl2)
										{
											fpx += DECALX * Xratio;
											fpy += (DECALY + 15) * Yratio;
										}

										fpx *= Xratio;
										fpy *= Yratio;
										EERIEDrawBitmap(fpx, fpy, 5.f * ratiooo, 5.f * ratiooo, 0, pTexDetect,
										                Color3f(col, 0, 0).to<u8>());

										if (!fl2)
											GRenderer->SetRenderState(Renderer::AlphaBlending, false);
									}
								}
							}
			}
		}

		if (flag == 0)
			for (size_t i = 0; i < Mapmarkers.size(); i++)
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

					if (!fl2 && MouseInRect(verts[0].sx, verts[0].sy, verts[2].sx, verts[2].sy))
					{
						if (Mapmarkers[i].tstring.empty())
						{
							std::string output;
							MakeLocalised(Mapmarkers[i].string, output);
							Mapmarkers[i].tstring = output;
						}

						if ( !Mapmarkers[i].tstring.empty() )
						{
							std::string output;
							MakeLocalised( Mapmarkers[i].string, output);
							Mapmarkers[i].tstring = output;
						}

						if ( !Mapmarkers[i].tstring.empty() )
						{
							Rect bRect(140, 290, 140 + 205, 358);

							float fLeft		= (bRect.left) * Xratio ;
							float fRight	= (bRect.right) * Xratio ;
							float fTop		= (bRect.top) * Yratio ;
							float fBottom	= (bRect.bottom) * Yratio ;

							ARX_CHECK_INT(fLeft);
							ARX_CHECK_INT(fRight);
							ARX_CHECK_INT(fTop);
							ARX_CHECK_INT(fBottom);

							Rect rRect = Rect(Rect::Num(fLeft), Rect::Num(fTop), Rect::Num(fRight), Rect::Num(fBottom));

							long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, Mapmarkers[i].tstring, rRect);

							char Page_Buffer[256];
							//_tcsncpy(Page_Buffer, Mapmarkers[i].tstring, lLengthDraw);
							strncpy( Page_Buffer, Mapmarkers[i].tstring.c_str(), lLengthDraw );
							Page_Buffer[lLengthDraw] = '\0';

							DrawBookTextInRect(hFontInGameNote, float(bRect.left), float(bRect.top), float(bRect.right), Page_Buffer, Color::none);
						}
					}

					if (MapMarkerTc == NULL)
						MapMarkerTc = TextureContainer::Load("Graph\\interface\\icons\\mapmarker.bmp");

					GRenderer->SetTexture(0, MapMarkerTc);

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

					EERIEDRAWPRIM(Renderer::TriangleFan, verts, 4);
				}
			}
	}
}

void ARX_MAPMARKER_Init() {
	Mapmarkers.clear();
}

long ARX_MAPMARKER_Get( const std::string& str) {
	for(size_t i = 0; i < Mapmarkers.size(); i++) {
		if(!strcasecmp(Mapmarkers[i].string, str.c_str()))
			return i;
	}
	return -1;
}

void ARX_MAPMARKER_Add(float x, float y, long lvl, const std::string& temp)
{
	long num = ARX_MAPMARKER_Get(temp);

	if (num >= 0) // already exists
	{
		Mapmarkers[num].lvl = lvl;
		Mapmarkers[num].x = x;
		Mapmarkers[num].y = y;

		if (!Mapmarkers[num].tstring.empty())
			Mapmarkers[num].tstring.clear();

		strcpy(Mapmarkers[num].string, temp.c_str());
		return;
	}
	
	Mapmarkers.push_back(MAPMARKER_DATA());
	
	Mapmarkers.back().lvl = lvl;
	Mapmarkers.back().x = x;
	Mapmarkers.back().y = y;
	Mapmarkers.back().tstring.clear();
	strcpy(Mapmarkers.back().string, temp.c_str());
}

void ARX_MAPMARKER_Remove( const std::string& temp)
{
	long num = ARX_MAPMARKER_Get(temp);

	if (num < 0) return; // Doesn't exists

	Mapmarkers.erase( Mapmarkers.begin() + num );
}
