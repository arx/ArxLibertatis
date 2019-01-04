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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#ifndef ARX_ANIMATION_ANIMATIONFORMAT_H
#define ARX_ANIMATION_ANIMATIONFORMAT_H

#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"


#pragma pack(push, 1)


const u32 CURRENT_THEA_VERSION = 2015;

const size_t SIZE_NAME = 256;

typedef s32 TBOOL;

struct ArxQuat {
	
	f32 w;
	f32 x;
	f32 y;
	f32 z;
	
	operator glm::quat() const {
		return glm::quat(w, x, y, z);
	}
	
};


// THEO TEA Animation FILE FORMAT structures

const size_t THEO_SIZE_IDENTITY_ANIM = 20;
typedef s32 STBOOL;

struct THEA_HEADER {
	char identity[THEO_SIZE_IDENTITY_ANIM];
	u32 version;
	char anim_name[SIZE_NAME];
	s32 nb_frames;
	s32 nb_groups;
	s32 nb_key_frames;
};

struct THEA_KEYFRAME_2014 {
	s32 num_frame;
	s32 flag_frame;
	STBOOL master_key_frame;
	STBOOL key_frame;
	STBOOL key_move;
	STBOOL key_orient;
	STBOOL key_morph;
	s32 time_frame;
};

struct THEA_KEYFRAME_2015 {
	s32 num_frame;
	s32 flag_frame;
	char info_frame[SIZE_NAME];
	STBOOL master_key_frame;
	STBOOL key_frame;
	STBOOL key_move;
	STBOOL key_orient;
	STBOOL key_morph;
	s32 time_frame;
};

typedef SavedVec3 THEA_KEYMOVE;

struct THEO_GROUPANIM {
	TBOOL key_group;
	char angle[8]; // ignored
	ArxQuat Quaternion;
	SavedVec3 translate;
	SavedVec3 zoom;
};

struct THEA_SAMPLE {
	char sample_name[SIZE_NAME];
	s32 sample_size;
};


#pragma pack(pop)


#endif // ARX_ANIMATION_ANIMATIONFORMAT_H
