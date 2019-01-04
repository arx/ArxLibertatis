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

#ifndef ARX_SCENE_LEVELFORMAT_H
#define ARX_SCENE_LEVELFORMAT_H

#include "graphics/Color.h"
#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"

// File format version
const f32 DLH_CURRENT_VERSION = 1.44f;


#pragma pack(push, 1)


struct DANAE_LS_HEADER {
	
	f32 version;
	char ident[16];
	char lastuser[256];
	s32 time;
	SavedVec3 pos_edit;
	SavedAnglef angle_edit;
	s32 nb_scn;
	s32 nb_inter;
	s32 nb_nodes;
	s32 nb_nodeslinks;
	s32 nb_zones;
	s32 lighting;
	s32 Bpad[256];
	s32 nb_lights;
	s32 nb_fogs;
	
	s32 nb_bkgpolys;
	s32 nb_ignoredpolys;
	s32 nb_childpolys;
	s32 nb_paths;
	s32 pad[250];
	SavedVec3 offset;
	f32 fpad[253];
	char cpad[4096];
	s32 bpad[256];
	
};

struct DANAE_LS_SCENE {
	char name[512];
	s32 pad[16];
	f32 fpad[16];
};

struct DANAE_LS_LIGHTINGHEADER {
	s32 nb_values;
	s32 ViewMode; // unused
	s32 ModeLight; // unused
	s32 pad;
};

struct DANAE_LS_VLIGHTING {
	
	s32 r;
	s32 g;
	s32 b;
	
	operator ColorBGRA() const {
		return Color(u8(r), u8(g), u8(b)).toBGRA();
	}
	
};

// version 1.003f
struct DANAE_LS_LIGHT {
	SavedVec3 pos;
	SavedColor rgb;
	f32 fallstart;
	f32 fallend;
	f32 intensity;
	f32 i;
	SavedColor ex_flicker;
	f32 ex_radius;
	f32 ex_frequency;
	f32 ex_size;
	f32 ex_speed;
	f32 ex_flaresize;
	f32 fpadd[24];
	s32 extras;
	s32 lpadd[31];
};

struct DANAE_LS_FOG {
	SavedVec3 pos;
	SavedColor rgb;
	f32 size;
	s32 special;
	f32 scale;
	SavedVec3 move;
	SavedAnglef angle;
	f32 speed;
	f32 rotatespeed;
	s32 tolive;
	s32 blend;
	f32 frequency;
	f32 fpadd[32];
	s32 lpadd[32];
	char cpadd[256];
};

struct DANAE_LS_PATH {
	char name[64];
	s16 idx;
	s16 flags;
	SavedVec3 initpos;
	SavedVec3 pos;
	s32 nb_pathways;
	SavedColor rgb;
	f32 farclip;
	f32 reverb;
	f32 amb_max_vol;
	f32 fpadd[26];
	s32 height;
	s32 lpadd[31];
	char ambiance[128];
	char cpadd[128];
};

struct DANAE_LS_PATHWAYS {
	SavedVec3 rpos;
	s32 flag;
	u32 time;
	f32 fpadd[2];
	s32 lpadd[2];
	char cpadd[32];
};

// Lighting File Header
struct DANAE_LLF_HEADER {
	f32 version;
	char ident[16];
	char lastuser[256];
	s32 time;
	s32 nb_lights;
	s32 nb_Shadow_Polys;
	s32 nb_IGNORED_Polys;
	s32 nb_bkgpolys;
	s32 pad[256];
	f32 fpad[256];
	char cpad[4096];
	s32 bpad[256];
};

struct DANAE_LS_INTER {
	char name[512];
	SavedVec3 pos;
	SavedAnglef angle;
	s32 ident;
	s32 flags;
	s32 pad[14];
	f32 fpad[16];
};


#pragma pack(pop)


#endif // ARX_SCENE_LEVELFORMAT_H
