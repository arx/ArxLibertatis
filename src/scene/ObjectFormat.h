/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_SCENE_OBJECTFORMAT_H
#define ARX_SCENE_OBJECTFORMAT_H

#include "graphics/GraphicsFormat.h"
#include "platform/Platform.h"


#pragma pack(push,1)


const u32 CURRENT_THEO_VERSION = 3011;
const u32 CURRENT_SCENE_VERSION = 3024;
const u32 CURRENT_THEA_VERSION = 2015;

#define SAVE_MAP_IN_OBJECT 0
#define SAVE_MAP_BMP       1
#define SAVE_MAP_TGA       2

const size_t SIZE_NAME = 256;
const size_t SIZE_IDENTITY_OBJECT = 17;

// Magic conversion constant used for loading rotations in TEO files.
const float THEO_ROTCONVERT = 0.087890625f;

typedef s32 TBOOL;

struct ArxQuat {
	
	f32 w;
	f32 x;
	f32 y;
	f32 z;
	
	operator glm::quat() const {
		glm::quat a;
		a.x = x;
		a.y = y;
		a.z = z;
		a.w = w;
		return a;
	}
	
	ArxQuat & operator=(const glm::quat & b) {
		x = b.x;
		y = b.y;
		z = b.z;
		w = b.w;
		return *this;
	}
	
};

struct TheoAngle {
	s32 alpha;
	s32 beta;
	s32 gamma;
};


// THEO SCN FILE FORMAT structures

struct TSCN_HEADER {
	u32 version;
	s32 maps_seek;
	s32 object_seek;
	s32 anim_seek;
	s32 path_seek;
	s32 cam_seek;
	s32 light_seek;
	s32 particle_seek;
	s32 groups_seek; // Added version 3007;
	s32 nb_maps;
	u32 type_write;
};

struct TSCN_OBJHEADER {
	s32 next_obj;
	char object_name[SIZE_NAME];
	TBOOL object_state;
	TBOOL object_state_module;
	TBOOL object_freeze;
	TBOOL object_selected;
};

const u32 LIGHT_DIRECTIONAL = 0;
const u32 LIGHT_OMNI = 1;
const u32 LIGHT_OMNI_SHADOW = 2;
const u32 LIGHT_SPOT = 3;
const u32 LIGHT_SPOT_SHADOW = 4;

struct TSCN_LIGHT {
	char light_name[SIZE_NAME];
	TBOOL light_state;
	u32 light_type;
	SavedVec3 pos;
	s32 red;
	s32 green;
	s32 blue;
	f32 falloff;
	f32 hotspot;
	SavedVec3 spot_target;
	f32 intensity;
	f32 saturation;
	f32 radiosity;
	TBOOL shadow;
	f32 factor_size;
	s32 hallow_nummap;
	f32 hallow_ray;
	f32 hallow_zmarge;
};

struct TSCN_LIGHT_3019 {
	char light_name[SIZE_NAME];
	TBOOL light_state;
	TBOOL light_selected;
	u32 light_type;
	SavedVec3 pos;
	s32 red;
	s32 green;
	s32 blue;
	f32 falloff;
	f32 hotspot;
	SavedVec3 spot_target;
	f32 intensity;
	f32 saturation;
	f32 radiosity;
	TBOOL shadow;
	f32 factor_size;
	s32 hallow_nummap;
	f32 hallow_ray;
	f32 hallow_zmarge;
};

struct TSCN_LIGHT_3024 {
	char light_name[SIZE_NAME];
	TBOOL light_state;
	TBOOL light_selected;
	TBOOL light_Freeze;
	u32 light_type;
	SavedVec3 pos;
	s32 red;
	s32 green;
	s32 blue;
	f32 falloff;
	f32 hotspot;
	SavedVec3 spot_target;
	f32 intensity;
	f32 saturation;
	f32 radiosity;
	TBOOL shadow;
	f32 factor_size;
	s32 hallow_nummap;
	f32 hallow_ray;
	f32 hallow_zmarge;
};


// THEO TEO FILE FORMAT structures

struct THEO_HEADER {
	char identity[SIZE_IDENTITY_OBJECT];
	u32 version;
	s32 maps_seek;
	s32 object_seek;
	s32 nb_maps;
	u32 type_write;
};

struct THEO_TEXTURE {
	char texture_name[SIZE_NAME];
	s32 dx; // texture width
	s32 dy; // texture height
	u32 bpp; // number of bits per pixel
	u32 map_data; // dx*dy*sizeof(logtype)
	u32 map_type;
	s32 reflect_map;
	f32 water_intensity;
	s32 mipmap_level;
	u32 color_mask;
	TBOOL animated_map;
};

struct THEO_SAVE_MAPS_IN {
	char texture_name[SIZE_NAME];
	u32 map_type;
	s32 reflect_map;
	f32 water_intensity;
	s32 mipmap_level;
	TBOOL animated_map;
};

struct THEO_SAVE_MAPS_IN_3019 {
	char texture_name[SIZE_NAME];
	u32 map_type;
	s32 reflect_map;
	f32 water_intensity;
	s32 mipmap_level;
	u32 color_mask;
	TBOOL animated_map;
};

struct THEO_OFFSETS {
	s32 vertex_seek;
	s32 action_point_seek;
	s32 lines_seek;
	s32 faces_seek;
	s32 extras_seek;
	s32 groups_seek;
};

struct THEO_NB {
	s32 nb_vertex;
	s32 nb_action_point;
	s32 nb_lines;
	s32 nb_faces;
	s32 nb_groups;
	u32 channel;
};

struct THEO_VERTEX {
	SavedVec3 pos;
	TBOOL hide;
	TBOOL freeze;
	TBOOL isselected;
};

struct THEO_ACTION_POINT {
	char name[SIZE_NAME];
	s32 vert_index;
	s32 action;
	s32 num_sfx;
};

struct THEO_FACE_UV {
	s32 u1;
	s32 v1;
	s32 u2;
	s32 v2;
	s32 u3;
	s32 v3;
};

struct THEO_FACE_RGB {
	s32 r;
	s32 g;
	s32 b;
};

struct THEO_FACES {
	u32 color;
	s32 index1;
	s32 index2;
	s32 index3;
	TBOOL ismap;
	THEO_FACE_UV liste_uv;
	s32 element_uv;
	s32 num_map;
	f32 tile_x;
	f32 tile_y;
	f32 user_tile_x;
	f32 user_tile_y;
	s32 flag;
	s32 collision_type;
	TBOOL rgb; // vertex
	THEO_FACE_RGB rgb1;
	THEO_FACE_RGB rgb2;
	THEO_FACE_RGB rgb3;
	TBOOL double_side;
	u32 transparency;
	f32 trans;
};

struct THEO_FACES_3006 {
	u32 color;
	s32 index1;
	s32 index2;
	s32 index3;
	TBOOL ismap;
	THEO_FACE_UV liste_uv;
	s32 element_uv;
	s32 num_map;
	f32 tile_x;
	f32 tile_y;
	f32 user_tile_x;
	f32 user_tile_y;
	s32 flag;
	s32 collision_type;
	TBOOL rgb; // vertex
	THEO_FACE_RGB rgb1;
	THEO_FACE_RGB rgb2;
	THEO_FACE_RGB rgb3;
	TBOOL double_side;
	TBOOL hide;
	TBOOL transparency;
	f32 trans;
};

struct THEO_EXTRA_DATA {
	TheoAngle angle;
	SavedVec3 pos;
	s32 origin_index;
};

struct THEO_EXTRA_DATA_3005 {
	TheoAngle angle;
	ArxQuat quat;
	SavedVec3 pos;
	s32 origin_index;
};

enum TheoObjectType {
	THEO_OBJECT_TYPE_3D = 0,
	THEO_OBJECT_TYPE_SPRITE = 1
};

enum TheoRenderType {
	THEO_RENDER_TYPE_MAP = 0,
	THEO_RENDER_TYPE_MAP_LIGHT = 1,
	THEO_RENDER_TYPE_MAP_GOURAUD = 2,
	THEO_RENDER_TYPE_SOLID = 3,
	THEO_RENDER_TYPE_LIGHT = 4,
	THEO_RENDER_TYPE_GOURAUD = 5,
	THEO_RENDER_TYPE_WIRE = 6,
	THEO_RENDER_TYPE_VERTEX = 7,
	THEO_RENDER_TYPE_BBOX = 8,
	THEO_RENDER_TYPE_PREDATOR = 9,
	THEO_RENDER_TYPE_CHROME = 10
};

struct THEO_GROUPS {
	s32 origin;
	s32 nb_index;
};

struct THEO_GROUPS_3011 {
	s32 origin;
	s32 symetric;
	TBOOL lock_alpha;
	TBOOL lock_beta;
	TBOOL lock_gamma;
	TheoAngle min;
	TheoAngle max;
	s32 nb_index;
};

struct THEO_SELECTED {
	char name[SIZE_NAME];
	s32 nb_index;
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


#endif // ARX_SCENE_OBJECTFORMAT_H
