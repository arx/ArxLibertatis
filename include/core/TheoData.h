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
///////////////////////////////////////////////////////////////////////////////
// THEODATA
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//		THEO file structures.
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#ifndef THEODATA_H
#define THEODATA_H

#define CURRENT_THEO_VERSION	3011
#define CURRENT_SCENE_VERSION	3024
#define CURRENT_THEA_VERSION	2015

#define SAVE_MAP_IN_OBJECT		0
#define SAVE_MAP_BMP			1
#define SAVE_MAP_TGA			2


#define SIZE_IDENTITY_OBJECT	17
#define SIZE_NAME				256

#define TBOOL int

#pragma pack(push,1)
struct THEO_3D
{
	float			x;
	float			y;
	float			z;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct ArxQuat
{
	float	w;
	float	x;
	float	y;
	float	z;
};
#pragma pack(pop)

//***********************************************************************
//*		BEGIN THEO SCN FILE FORMAT structures							*
//***********************************************************************
#pragma pack(push,1)
struct TSCN_HEADER
{
	unsigned long		version;
	long				maps_seek;
	long				object_seek;
	long				anim_seek;
	long				path_seek;
	long				cam_seek;
	long				light_seek;
	long				particle_seek;
	long				groups_seek; //Added version 3007;
	long				nb_maps;
	unsigned long		type_write;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct TSCN_OBJHEADER
{
	long				next_obj;
	char				object_name[SIZE_NAME];
	TBOOL				object_state;
	TBOOL				object_state_module;
	TBOOL				object_freeze;
	TBOOL				object_selected;

}; // Aligned 1 2 4
#pragma pack(pop)

#define	LIGHT_DIRECTIONAL	0
#define	LIGHT_OMNI			1
//#define	LIGHT_OMNI			5
#define LIGHT_OMNI_SHADOW	2
#define LIGHT_SPOT			3
//#define LIGHT_SPOT			6
#define LIGHT_SPOT_SHADOW	4

#pragma pack(push,1)
struct TSCN_LIGHT
{
	char				light_name[SIZE_NAME];
	TBOOL				light_state;
	unsigned long		light_type;
	THEO_3D				pos;
	long				red;
	long				green;
	long				blue;
	float				falloff;
	float				hotspot;
	THEO_3D				spot_target;
	float				intensity;
	float				saturation;
	float				radiosity;
	TBOOL				shadow;
	float				factor_size;
	long				hallow_nummap;
	float				hallow_ray;
	float				hallow_zmarge;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct TSCN_LIGHT_3019
{
	char				light_name[SIZE_NAME];
	TBOOL				light_state;
	TBOOL				light_selected;
	unsigned long		light_type;
	THEO_3D				pos;
	long				red;
	long				green;
	long				blue;
	float				falloff;
	float				hotspot;
	THEO_3D				spot_target;
	float				intensity;
	float				saturation;
	float				radiosity;
	TBOOL				shadow;
	float				factor_size;
	long				hallow_nummap;
	float				hallow_ray;
	float				hallow_zmarge;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct TSCN_LIGHT_3024
{
	char				light_name[SIZE_NAME];
	TBOOL				light_state;
	TBOOL				light_selected;
	TBOOL				light_Freeze;
	unsigned long		light_type;
	THEO_3D				pos;
	long				red;
	long				green;
	long				blue;
	float				falloff;
	float				hotspot;
	THEO_3D				spot_target;
	float				intensity;
	float				saturation;
	float				radiosity;
	TBOOL				shadow;
	float				factor_size;
	long				hallow_nummap;
	float				hallow_ray;
	float				hallow_zmarge;
}; // Aligned 1 2 4
#pragma pack(pop)

//***********************************************************************
//*		END THEO SCN FILE FORMAT structures								*
//***********************************************************************


//***********************************************************************
//*		BEGIN THEO TEO FILE FORMAT structures							*
//***********************************************************************

#pragma pack(push,1)
struct THEO_HEADER
{
	char			identity[SIZE_IDENTITY_OBJECT];
	unsigned long	version;
	long			maps_seek;
	long			object_seek;
	long			nb_maps;
	unsigned long	type_write;
}; // NOT ALIGNED (SIZE_IDENTITY_OBJECT)
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_TEXTURE
{
	char			texture_name[SIZE_NAME];
	long			dx;	//largeur de la texture
	long			dy;	//hauteur de la texture
	unsigned long	bpp; //nb de bits par pixel
	unsigned char *	map_data;  //dx*dy*sizeof(logtype)
	unsigned long	map_type;
	long			reflect_map;
	float			water_intensity;
	long			mipmap_level;
	unsigned long	color_mask;
	TBOOL			animated_map;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_SAVE_MAPS_IN
{
	char			texture_name[SIZE_NAME];
	unsigned long	map_type;
	long			reflect_map;
	float			water_intensity;
	long			mipmap_level;
	TBOOL			animated_map;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_SAVE_MAPS_IN_3019
{
	char			texture_name[SIZE_NAME];
	unsigned long	map_type;
	long			reflect_map;
	float			water_intensity;
	long			mipmap_level;
	unsigned long	color_mask;
	TBOOL			animated_map;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_OFFSETS
{
	long			vertex_seek;
	long			action_point_seek;
	long			lines_seek;
	long			faces_seek;
	long			extras_seek;
	long			groups_seek;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_NB
{
	long			nb_vertex;
	long			nb_action_point;
	long			nb_lines;
	long			nb_faces;
	long			nb_groups;
	unsigned long	channel;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_VERTEX
{
	float			x;
	float			y;
	float			z;
	TBOOL			hide;
	TBOOL			freeze;
	TBOOL			isselected;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_ACTION_POINT
{
	char			name[SIZE_NAME];
	long			vert_index;
	long			action;
	long			num_sfx;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
typedef struct
{
	long			u1;
	long			v1;
	long			u2;
	long			v2;
	long			u3;
	long			v3;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_FACE_RGB
{
	long r;
	long g;
	long b;
};  // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_FACES
{
	unsigned long	color;
	long			index1;
	long			index2;
	long			index3;
	TBOOL			ismap;
	THEO_FACE_UV	liste_uv;
	long			element_uv;
	long			num_map;
	float			tile_x;
	float			tile_y;
	float			user_tile_x;
	float			user_tile_y;
	long			flag;
	long			collision_type;
	TBOOL		 	rgb; //au vertex
	THEO_FACE_RGB	rgb1;
	THEO_FACE_RGB	rgb2;
	THEO_FACE_RGB	rgb3;
	TBOOL			double_side;
	//TBOOL			transparency;
	unsigned long	transparency;
	float			trans;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_FACES_3006
{
	unsigned long	color;
	long			index1;
	long			index2;
	long			index3;
	TBOOL			ismap;
	THEO_FACE_UV	liste_uv;
	long			element_uv;
	long			num_map;
	float			tile_x;
	float			tile_y;
	float			user_tile_x;
	float			user_tile_y;
	long			flag;
	long			collision_type;
	TBOOL		 	rgb;			//au vertex
	THEO_FACE_RGB	rgb1;
	THEO_FACE_RGB	rgb2;
	THEO_FACE_RGB	rgb3;
	TBOOL			double_side;
	TBOOL			hide;
	TBOOL			transparency;
	float			trans;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_EXTRA_DATA
{
	long			alpha;
	long			beta;
	long			gamma;
	float			posx;
	float			posy;
	float			posz;
	long			origin_index;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_EXTRA_DATA_3005
{
	long			alpha;
	long			beta;
	long			gamma;
	ArxQuat			quat;
	float			posx;
	float			posy;
	float			posz;
	long			origin_index;
}; // Aligned 1 2 4
#pragma pack(pop)

#define THEO_OBJECT_TYPE_3D			0
#define THEO_OBJECT_TYPE_SPRITE		1

#define THEO_RENDER_TYPE_MAP			0
#define THEO_RENDER_TYPE_MAP_LIGHT		1
#define THEO_RENDER_TYPE_MAP_GOURAUD	2
#define THEO_RENDER_TYPE_SOLID			3
#define THEO_RENDER_TYPE_LIGHT			4
#define THEO_RENDER_TYPE_GOURAUD		5
#define THEO_RENDER_TYPE_WIRE			6
#define THEO_RENDER_TYPE_VERTEX			7
#define THEO_RENDER_TYPE_BBOX			8
#define THEO_RENDER_TYPE_PREDATOR		9
#define THEO_RENDER_TYPE_CHROME			10

#pragma pack(push,1)
struct THEO_GROUPS
{
	long			origin;
	long			nb_index;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_GROUPS_3011
{
	long			origin;
	long			symetric;
	TBOOL			lock_alpha;
	TBOOL			lock_beta;
	TBOOL			lock_gamma;
	long			alphamin;
	long			betamin;
	long			gammamin;
	long			alphamax;
	long			betamax;
	long			gammamax;
	long			nb_index;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_SELECTED
{
	char			name[SIZE_NAME];
	long			nb_index;
}; // Aligned 1 2 4
#pragma pack(pop)

//***********************************************************************
//*		END THEO TEO FILE FORMAT structures								*
//***********************************************************************

//***********************************************************************
//*		BEGIN THEO TEA Animation FILE FORMAT structures					*
//***********************************************************************
#define		THEO_SIZE_IDENTITY_ANIM 20
#define		STBOOL					int

#pragma pack(push,1)
struct THEA_HEADER
{
	char			identity[THEO_SIZE_IDENTITY_ANIM];
	unsigned long	version;
	char			anim_name[SIZE_NAME];
	long			nb_frames;
	long			nb_groups;
	long			nb_key_frames;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEA_KEYFRAME
{
	long			num_frame;
	long			flag_frame;
	STBOOL			master_key_frame;
	STBOOL			key_frame; //image clef
	STBOOL			key_move;
	STBOOL			key_orient;
	STBOOL			key_morph;
	long			time_frame;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEA_KEYFRAME_2015
{
	long			num_frame;
	long			flag_frame;
	char			info_frame[SIZE_NAME];
	STBOOL			master_key_frame;
	STBOOL			key_frame; //image clef
	STBOOL			key_move;
	STBOOL			key_orient;
	STBOOL			key_morph;
	long			time_frame;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEA_KEYMOVE
{
	float			x;
	float			y;
	float			z;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEA_MORPH
{
	long		num_list_morph;
	long		next_list_morph;
	long		nb_inter;
	long		start_frame;
}; // Aligned 1 2 4 8
#pragma pack(pop)

#pragma pack(push,1)
struct THEO_GROUPANIM
{
	TBOOL		key_group;
	char angle[8]; // ignored
	ArxQuat		Quaternion;
	THEO_3D		translate;
	THEO_3D		zoom;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct THEA_SAMPLE
{
	char			sample_name[SIZE_NAME];
	long			sample_size;
}; // Aligned 1 2 4
#pragma pack(pop)

//***********************************************************************
//*            END THEO TEA Animation FILE FORMAT structures            *
//***********************************************************************

#endif // THEODATA_H
