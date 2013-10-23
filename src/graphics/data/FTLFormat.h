/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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


#pragma pack(push,1)


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

struct ARX_FTL_PROGRESSIVE_DATA_HEADER {
	s32 nb_vertex;
};

struct ARX_FTL_CLOTHES_DATA_HEADER {
	s32 nb_cvert;
	s32 nb_springs;
};

struct ARX_FTL_COLLISION_SPHERES_DATA_HEADER {
	s32 nb_spheres;
};

struct ARX_FTL_3D_DATA_HEADER {
	s32 nb_vertex;
	s32 nb_faces;
	s32 nb_maps;
	s32 nb_groups;
	s32 nb_action;
	s32 nb_selections; // data will follow this order
	s32 origin;
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
	s32 origin;
	s32 nb_index;
	s32 indexes;
	f32 siz;
	
	EERIE_GROUPLIST_FTL & operator=(const EERIE_GROUPLIST & b) {
		strcpy(name, b.name.c_str());
		origin = b.origin;
		nb_index = b.indexes.size();
		indexes = 0;
		siz = b.siz;
		return *this;
	}
	
};

struct EERIE_ACTIONLIST_FTL {
	
	char name[256];
	s32 idx; // index vertex;
	s32 action;
	s32 sfx;
	
	inline operator EERIE_ACTIONLIST() const {
		EERIE_ACTIONLIST a;
		a.name = boost::to_lower_copy(util::loadString(name));
		a.idx = idx;
		a.act = action;
		a.sfx = sfx;
		return a;
	}
	
	inline EERIE_ACTIONLIST_FTL & operator=(const EERIE_ACTIONLIST & b) {
		strcpy(name, b.name.c_str());
		idx = b.idx;
		action = b.act;
		sfx = b.sfx;
		return *this;
	}
	
};

struct EERIE_SELECTIONS_FTL {
	
	char name[64];
	s32 nb_selected;
	s32 selected;
	
	inline EERIE_SELECTIONS_FTL & operator=(const EERIE_SELECTIONS & b) {
		strcpy(name, b.name.c_str());
		nb_selected = b.selected.size();
		selected = 0;
		return *this;
	}
	
};

struct COLLISION_SPHERE_FTL {
	
	s16 idx;
	s16 flags;
	f32 radius;
	
	inline operator COLLISION_SPHERE() const {
		COLLISION_SPHERE a;
		a.idx = idx;
		a.flags = flags;
		a.radius = radius;
		return a;
	}
	
	inline COLLISION_SPHERE_FTL & operator=(const COLLISION_SPHERE & b) {
		idx = b.idx;
		flags = b.flags;
		radius = b.radius;
		return *this;
	}
	
};

struct EERIE_SPRINGS_FTL {
	
	s16 startidx;
	s16 endidx;
	f32 restlength;
	f32 constant; // spring constant
	f32 damping; // spring damping
	s32 type;
	
	inline operator EERIE_SPRINGS() const {
		EERIE_SPRINGS a;
		a.startidx = startidx;
		a.endidx = endidx;
		a.restlength = restlength;
		a.constant = constant;
		a.damping = damping;
		a.type = type;
		return a;
	}
	
	inline EERIE_SPRINGS_FTL & operator=(const EERIE_SPRINGS & b) {
		startidx = b.startidx;
		endidx = b.endidx;
		restlength = b.restlength;
		constant = b.constant;
		damping = b.damping;
		type = b.type;
		return *this;
	}
	
};

struct EERIE_OLD_VERTEX {
	
	SavedTextureVertex vert;
	SavedVec3 v;
	SavedVec3 norm;
	
	inline operator EERIE_VERTEX() const {
		EERIE_VERTEX a;
		a.vert = vert, a.v = v.toVec3(), a.norm = norm.toVec3();
		return a;
	}
	
	inline EERIE_OLD_VERTEX & operator=(const EERIE_VERTEX & b) {
		vert = b.vert, v = b.v, norm = b.norm;
		return *this;
	}
	
};

struct CLOTHESVERTEX_FTL {
	
	s16 idx;
	u8 flags;
	s8 coll;
	SavedVec3 pos;
	SavedVec3 velocity;
	SavedVec3 force;
	f32 mass;
	
	SavedVec3 t_pos;
	SavedVec3 t_velocity;
	SavedVec3 t_force;
	
	SavedVec3 lastpos;
	
	inline operator CLOTHESVERTEX() const {
		CLOTHESVERTEX a;
		a.idx = idx;
		a.flags = flags;
		a.coll = coll;
		a.pos = pos.toVec3();
		a.velocity = velocity.toVec3();
		a.force = force.toVec3();
		a.mass = mass;
		a.t_pos = t_pos.toVec3();
		a.t_velocity = t_velocity.toVec3();
		a.t_force = t_force.toVec3();
		a.lastpos = lastpos.toVec3();
		return a;
	}
	
	inline CLOTHESVERTEX_FTL & operator=(const CLOTHESVERTEX & b) {
		idx = b.idx;
		flags = b.flags;
		coll = b.coll;
		pos = b.pos;
		velocity = b.velocity;
		force = b.force;
		mass = b.mass;
		t_pos = b.t_pos;
		t_velocity = b.t_velocity;
		t_force = b.t_force;
		lastpos = b.lastpos;
		return *this;
	}
	
};


#pragma pack(pop)


#endif // ARX_GRAPHICS_DATA_FTLFORMAT_H
