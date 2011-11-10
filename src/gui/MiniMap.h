/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GUI_MINIMAP_H
#define ARX_GUI_MINIMAP_H

#include <string>
#include <vector>

class TextureContainer;

#define MINIMAP_MAX_X 50
#define MINIMAP_MAX_Z 50

struct MINI_MAP_DATA
{
    TextureContainer*   tc;
    float               offsetx; // start of scene pos x
    float               offsety;
    float               xratio; // multiply x by xratio to obtain real-world pos
    float               yratio;
    float               width; // bitmap width/height
    float               height;
    unsigned char       revealed[MINIMAP_MAX_X][MINIMAP_MAX_Z];
};


const unsigned long MAX_MINIMAPS(32);
extern MINI_MAP_DATA minimap[MAX_MINIMAPS];
//-----------------------------------------------------------------------------
// MINIMAP
//-----------------------------------------------------------------------------
void ARX_MINIMAP_Load_Offsets();
void ARX_MINIMAP_FirstInit();
void ARX_MINIMAP_Reset();
void ARX_MINIMAP_PurgeTC();
 
void ARX_MINIMAP_Show(long SHOWLEVEL, long flag, long fl2 = 0);
void ARX_MINIMAP_FirstInit();
void ARX_MINIMAP_PurgeTC();
void ARX_MINIMAP_ValidatePos();
void ARX_MINIMAP_ValidatePlayerPos();
void ARX_MINIMAP_Reveal();

struct MAPMARKER_DATA {
	float x;
	float y;
	long lvl;
	std::string name;
	std::string text;
};

extern std::vector<MAPMARKER_DATA> Mapmarkers;

void ARX_MAPMARKER_Remove(const std::string & temp);
void ARX_MAPMARKER_Add(float x, float y, long lvl, const std::string & temp);
void ARX_MAPMARKER_Init();

#endif // ARX_GUI_MINIMAP_H
