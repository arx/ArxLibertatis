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

#ifndef ARX_MINIMAP_H
#define ARX_MINIMAP_H

#include "ARX_Interface.h"

#define MINIMAP_MAX_X 50
#define MINIMAP_MAX_Z 50

typedef struct
{
	TextureContainer	* tc;
	float				offsetx; // start of scene pos x
	float				offsety;
	float				xratio; // multiply x by xratio to obtain real-world pos
	float				yratio;
	float				width; // bitmap width/height
	float				height;
	unsigned char		revealed[MINIMAP_MAX_X][MINIMAP_MAX_Z];
} MINI_MAP_DATA;


const unsigned long MAX_MINIMAPS(32);
extern MINI_MAP_DATA minimap[MAX_MINIMAPS];
//-----------------------------------------------------------------------------
// MINIMAP
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Load_Offsets();
void ARX_MINIMAP_FirstInit();
void ARX_MINIMAP_Reset();
void ARX_MINIMAP_PurgeTC();
 
void ARX_MINIMAP_Show(LPDIRECT3DDEVICE7 m_pd3dDevice, long SHOWLEVEL, long flag, long fl2 = 0);
void ARX_MINIMAP_FirstInit();
void ARX_MINIMAP_PurgeTC();
void ARX_MINIMAP_ValidatePos(EERIE_3D * pos);
void ARX_MINIMAP_ValidatePlayerPos();
void ARX_MINIMAP_Reveal();

typedef struct
{
	float	x;
	float	y;
	long	lvl;
	char	string[64];
	_TCHAR * tstring;
} MAPMARKER_DATA;
extern MAPMARKER_DATA * Mapmarkers;
extern long Nb_Mapmarkers;

void ARX_MAPMARKER_Remove(char * temp);
void ARX_MAPMARKER_Add(float x, float y, long lvl, char * temp);
void ARX_MAPMARKER_Init();
#endif
