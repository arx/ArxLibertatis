/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H
#define ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H

#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"


#pragma pack(push, 1)


const float FTS_VERSION = 0.141f;

struct UNIQUE_HEADER {
	char path[256];
	s32 count;
	f32 version;
	s32 uncompressedsize;
	s32 pad[3];
};

struct UNIQUE_HEADER2 {
	char path[256];
};

struct UNIQUE_HEADER3 {
	UNIQUE_HEADER2 h2;
	char check[512];
};

struct FAST_VERTEX {
	f32 sy;
	f32 ssx;
	f32 ssz;
	f32 stu;
	f32 stv;
};

struct FAST_EERIEPOLY {
	FAST_VERTEX v[4];
	s32 tex;
	SavedVec3 norm;
	SavedVec3 norm2;
	SavedVec3 nrml[4];
	f32 transval;
	f32 area;
	s32 type;
	s16 room;
	s16 paddy;
};

struct FAST_SCENE_HEADER {
	f32 version;
	s32 sizex;
	s32 sizez;
	s32 nb_textures;
	s32 nb_polys;
	s32 nb_anchors;
	SavedVec3 playerpos;
	SavedVec3 Mscenepos;
	s32 nb_portals;
	s32 nb_rooms;
};

struct FAST_TEXTURE_CONTAINER {
	s32 tc;
	s32 temp;
	char fic[256];
};

enum FastAnchorFlag {
	FastAnchorFlagBlocked = 1 << 3
};

struct FAST_ANCHOR_DATA {
	SavedVec3 pos;
	f32 radius;
	f32 height;
	s16 nb_linked;
	s16 flags;
};

struct FAST_SCENE_INFO {
	s32 nbpoly;
	s32 nbianchors;
};

struct ROOM_DIST_DATA_SAVE {
	f32 distance; // -1 means use truedist
	SavedVec3 startpos;
	SavedVec3 endpos;
};

struct SAVE_EERIEPOLY {
	s32 type; // at least 16 bits
	SavedVec3 min;
	SavedVec3 max;
	SavedVec3 norm;
	SavedVec3 norm2;
	SavedTextureVertex v[4];
	char unused[32 * 4];
	SavedVec3 nrml[4];
	s32 tex;
	SavedVec3 center;
	f32 transval;
	f32 area;
	s16 room;
	s16 misc;
};

struct EERIE_SAVE_PORTALS {
	SAVE_EERIEPOLY poly;
	s32 room_1; // facing normal
	s32 room_2;
	s16 useportal;
	s16 paddy;
};

struct FAST_EP_DATA {
	
	s16 px;
	s16 py;
	s16 idx;
	s16 padd;
	
	operator EP_DATA() const {
		EP_DATA b;
		b.tile.x = px;
		b.tile.y = py;
		b.idx = idx;
		return b;
	}
	
};

struct EERIE_SAVE_ROOM_DATA {
	s32 nb_portals;
	s32 nb_polys;
	s32 padd[6];
};


#pragma pack(pop)


#endif // ARX_GRAPHICS_DATA_FASTSCENEFORMAT_H
