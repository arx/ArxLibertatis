/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GRAPHICS_DATA_FTLFORMAT_H
#define ARX_GRAPHICS_DATA_FTLFORMAT_H

#include <cstring>

#include <boost/algorithm/string/case_conv.hpp>

#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"
#include "util/String.h"

#define CURRENT_FTL_VERSION  0.83257f

// FTL File Structures Definitions

// FTL FILE Structure:
//
// ARX_FTL_PRIMARY_HEADER
// Checksum
// --> All the following data is then compressed and must be expanded
// ARX_FTL_SECONDARY_HEADER;
// -> Then depending on offsets just read data directly.


#pragma pack(push, 1)


struct ARX_FTL_PRIMARY_HEADER {
	char ident[4]; // FTL
	f32 version; // starting with version 1.0f
};

struct ARX_FTL_SECONDARY_HEADER {
	s32 offset_3Ddata; // -1 = no
	s32 offset_cylinder; // -1 = no
	s32 offset_progressive_data; // -1 = no
	s32 offset_clothes_data; // -1 = no
	s32 offset_collision_spheres; // -1 = no
	s32 offset_physics_box; // -1 = no
};

struct ARX_FTL_3D_DATA_HEADER {
	s32 nb_vertex;
	s32 nb_faces;
	s32 nb_maps;
	s32 nb_groups;
	s32 nb_action;
	s32 nb_selections; // data will follow this order
	s32 origin; // TODO this is always >= 0 replace with u32
	char name[256];
};

struct Texture_Container_FTL {
	char name[256];
};

const size_t IOPOLYVERT_FTL = 3;

struct EERIE_FACE_FTL {
	
	s32 facetype; // 0 = flat, 1 = text, 2 = Double-Side
	
	u32 rgb[IOPOLYVERT_FTL];
	u16 vid[IOPOLYVERT_FTL];
	s16 texid;
	f32 u[IOPOLYVERT_FTL];
	f32 v[IOPOLYVERT_FTL];
	s16 ou[IOPOLYVERT_FTL];
	s16 ov[IOPOLYVERT_FTL];
	
	f32 transval;
	SavedVec3 norm;
	SavedVec3 nrmls[IOPOLYVERT_FTL];
	f32 temp;
	
};

struct EERIE_GROUPLIST_FTL {
	
	char name[256];
	s32 origin; // TODO this is always positive use u32 ?
	s32 nb_index;
	s32 indexes;
	f32 siz;
	
};

struct EERIE_ACTIONLIST_FTL {
	
	char name[256];
	s32 idx; // index vertex;
	s32 action;
	s32 sfx;
	
	operator EERIE_ACTIONLIST() const {
		EERIE_ACTIONLIST a;
		a.name = boost::to_lower_copy(util::loadString(name));
		a.idx = ActionPoint(idx);
		a.act = action;
		a.sfx = sfx;
		return a;
	}
	
};

struct EERIE_SELECTIONS_FTL {
	
	char name[64];
	s32 nb_selected;
	s32 selected;
	
};

struct EERIE_OLD_VERTEX {
	
	char unused[32];
	SavedVec3 v;
	SavedVec3 norm;
	
	operator EERIE_VERTEX() const {
		EERIE_VERTEX a;
		a.v = v.toVec3(), a.norm = norm.toVec3();
		return a;
	}
	
};

#pragma pack(pop)


#endif // ARX_GRAPHICS_DATA_FTLFORMAT_H
