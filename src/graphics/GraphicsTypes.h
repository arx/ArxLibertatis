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

#ifndef ARX_GRAPHICS_GRAPHICSTYPES_H
#define ARX_GRAPHICS_GRAPHICSTYPES_H

#include <cmath>
#include <vector>

#include "audio/AudioTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Vertex.h"

#include "io/resource/ResourcePath.h"

#include "math/Vector.h"
#include "math/Angle.h"

#include "platform/Flags.h"

#include "Configure.h"

struct EERIE_3DOBJ;
class TextureContainer;
class Entity;
struct EERIE_LIGHT;

struct EERIE_TRI {
	Vec3f v[3];
};


#define TYP_SPECIAL1 1

enum PolyTypeFlag {
	POLY_NO_SHADOW    = (1<<0),
	POLY_DOUBLESIDED  = (1<<1),
	POLY_TRANS        = (1<<2),
	POLY_WATER        = (1<<3),
	POLY_GLOW         = (1<<4),
	POLY_IGNORE       = (1<<5),
	POLY_QUAD         = (1<<6),
	POLY_TILED        = (1<<7),
	POLY_METAL        = (1<<8),
	POLY_HIDE         = (1<<9),
	POLY_STONE        = (1<<10),
	POLY_WOOD         = (1<<11),
	POLY_GRAVEL       = (1<<12),
	POLY_EARTH        = (1<<13),
	POLY_NOCOL        = (1<<14),
	POLY_LAVA         = (1<<15),
	POLY_CLIMB        = (1<<16),
	POLY_FALL         = (1<<17),
	POLY_NOPATH       = (1<<18),
	POLY_NODRAW       = (1<<19),
	POLY_PRECISE_PATH = (1<<20),
	POLY_NO_CLIMB     = (1<<21),
	POLY_ANGULAR      = (1<<22),
	POLY_ANGULAR_IDX0 = (1<<23),
	POLY_ANGULAR_IDX1 = (1<<24),
	POLY_ANGULAR_IDX2 = (1<<25),
	POLY_ANGULAR_IDX3 = (1<<26),
	POLY_LATE_MIP     = (1<<27)
};
DECLARE_FLAGS(PolyTypeFlag, PolyType)
DECLARE_FLAGS_OPERATORS(PolyType)

struct EERIEPOLY {
	PolyType type;
	Vec3f		min;
	Vec3f		max;
	Vec3f		norm;
	Vec3f		norm2;
	TexturedVertex		v[4];
	TexturedVertex		tv[4];
	Vec3f		nrml[4];
	TextureContainer * tex;
	Vec3f		center;
	float			transval;
	float			area;
	short			room;
	short			misc;
	unsigned short	uslInd[4];
};

#define IOPOLYVERT 3
struct EERIE_FACE {
	
	PolyType facetype;
	short texid; //!< index into the objects texture list
	unsigned short vid[IOPOLYVERT];
	float u[IOPOLYVERT];
	float v[IOPOLYVERT];
	
	float transval;
	Vec3f norm;
	Vec3f nrmls[IOPOLYVERT];
	float temp;
	
	short ou[IOPOLYVERT];
	short ov[IOPOLYVERT];
	Color color[IOPOLYVERT];
	
};

struct NEIGHBOURS_DATA {
	short nb_Nvertex;
	short nb_Nfaces;
	short * Nvertex;
	short * Nfaces;
};

struct PROGRESSIVE_DATA {
	// ingame data
	short actual_collapse; // -1 = no collapse
	short need_computing;
	float collapse_ratio;
	// static data
	float collapse_cost;
	short collapse_candidate;
	short padd;
};

struct EERIE_SPRINGS {
	short startidx;
	short endidx;
	float restlength;
	float constant; // spring constant
	float damping; // spring damping
	long type;
};

#define CLOTHES_FLAG_NORMAL	0
#define CLOTHES_FLAG_FIX	1
#define CLOTHES_FLAG_NOCOL	2

struct CLOTHESVERTEX {
	
	short idx;
	unsigned char flags;
	char coll;
	Vec3f pos;
	Vec3f velocity;
	Vec3f force;
	float mass; // 1.f/mass
	
	Vec3f t_pos;
	Vec3f t_velocity;
	Vec3f t_force;
	
	Vec3f lastpos;
	
};

struct CLOTHES_DATA {
	
	CLOTHESVERTEX * cvert;
	CLOTHESVERTEX * backup;
	short nb_cvert;
	std::vector<EERIE_SPRINGS> springs;
	
	CLOTHES_DATA() : cvert(NULL), backup(NULL), nb_cvert(0) { }
};

struct COLLISION_SPHERE {
	short idx;
	short flags; // TODO not used?
	float radius;
};

struct COLLISION_SPHERES_DATA {
	std::vector<COLLISION_SPHERE> spheres;
};

struct PHYSVERT
{
	Vec3f	initpos;
	Vec3f	temp;
	Vec3f	pos;
	Vec3f	velocity;
	Vec3f	force;
	Vec3f	inertia;
	float		mass;

	PHYSVERT()
		: initpos(Vec3f_ZERO)
		, temp(Vec3f_ZERO)
		, pos(Vec3f_ZERO)
		, velocity(Vec3f_ZERO)
		, force(Vec3f_ZERO)
		, inertia(Vec3f_ZERO)
		, mass(0.f)
	{}
};

struct PHYSICS_BOX_DATA
{
	PHYSVERT * vert;
	long	nb_physvert;
	short	active;
	short	stopcount;
	float	radius; //radius around vert[0].pos for spherical collision
	float	storedtiming;
};


struct EERIE_GROUPLIST {
	std::string name;
	long origin;
	std::vector<long> indexes;
	float siz;
	
	EERIE_GROUPLIST() : name(), origin(0), indexes(), siz(0.0f) { }
};

struct EERIE_ACTIONLIST {
	std::string name;
	long idx; //index vertex;
	long act; //action
	long sfx; //sfx
	
	EERIE_ACTIONLIST() : name(), idx(0), act(0), sfx(0) { }
};

struct CUB3D
{
	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;
};

struct EERIE_LINKED {
	long lgroup; //linked to group n° if lgroup=-1 NOLINK
	long lidx;
	long lidx2;
	EERIE_3DOBJ * obj;
	Entity * io;
};

struct EERIE_SELECTIONS {
	std::string name;
	std::vector<long> selected;
};

struct EERIE_FASTACCESS
{
	short	view_attach;
	short	primary_attach;

	short	left_attach;
	short	weapon_attach;

	short	secondary_attach;
	short	mouth_group;

	short	jaw_group;
	short	head_group_origin;

	short	head_group;
	short	mouth_group_origin;

	short	V_right;
	short	U_right;

	short	fire;
	short	sel_head;

	short	sel_chest;
	short	sel_leggings;

	short	carry_attach;
	short	padding_;
};

/////////////////////////////////////////////////////////////////////////////////

struct BoneTransform {
	EERIE_QUAT quat;
	Vec3f      trans;
	Vec3f      scale;
};

struct EERIE_BONE
{
	long				nb_idxvertices;
	long 		*		idxvertices;
	EERIE_GROUPLIST *	original_group;
	long				father;

	BoneTransform anim;
	BoneTransform last;
	BoneTransform init;

	Vec3f			transinit_global;
};

struct EERIE_C_DATA
{
	EERIE_BONE *	bones;
	long			nb_bones;
};

struct EERIE_3DPAD : public Vec3f {
	float w;
};

struct EERIE_3DOBJ
{
	EERIE_3DOBJ()
	{
		point0 = pos = Vec3f_ZERO;
		angle = Anglef::ZERO;

		origin = 0;
		ident = 0;
		nbgroups = 0;

		vertexlocal = NULL;

		grouplist = NULL;

		originaltextures = NULL;
		linked = NULL;

		// TODO Make default constructor possible
		cub.xmin = 0;
		cub.xmax = 0;
		cub.ymin = 0;
		cub.ymax = 0;
		cub.zmin = 0;
		cub.zmax = 0;

		// TODO Default constructor
		quat.x = quat.y = quat.z = quat.w = 0;
		nblinked = 0;

		pbox = NULL;
		pdata = NULL;
		ndata = NULL;
		cdata = NULL;
		sdata = NULL;

		fastaccess.view_attach = 0;
		fastaccess.primary_attach = 0;
		fastaccess.left_attach = 0;
		fastaccess.weapon_attach = 0;
		fastaccess.secondary_attach = 0;
		fastaccess.mouth_group = 0;
		fastaccess.jaw_group = 0;
		fastaccess.head_group_origin = 0;
		fastaccess.head_group = 0;
		fastaccess.mouth_group_origin = 0;
		fastaccess.V_right = 0;
		fastaccess.U_right = 0;
		fastaccess.fire = 0;
		fastaccess.sel_head = 0;
		fastaccess.sel_chest = 0;
		fastaccess.sel_leggings = 0;
		fastaccess.carry_attach = 0;
		fastaccess.padding_ = 0;

		c_data = NULL;
	}
	
	void clear();
	
	~EERIE_3DOBJ();
	
	std::string name;
	res::path file;
	Vec3f pos;
	Vec3f point0;
	Anglef angle;
	long origin;
	long ident;
	long nbgroups;
	EERIE_3DPAD * vertexlocal;
	std::vector<EERIE_VERTEX> vertexlist;
	std::vector<EERIE_VERTEX> vertexlist3;

	std::vector<EERIE_FACE> facelist;
	EERIE_GROUPLIST * grouplist;
	std::vector<EERIE_ACTIONLIST> actionlist;
	std::vector<EERIE_SELECTIONS> selections;
	std::vector<TextureContainer*> texturecontainer;

	char * originaltextures;
	CUB3D cub;
	EERIE_QUAT quat;
	EERIE_LINKED * linked;
	long nblinked;


	PHYSICS_BOX_DATA * pbox;
	PROGRESSIVE_DATA * pdata;
	NEIGHBOURS_DATA * ndata;
	CLOTHES_DATA * cdata;
	COLLISION_SPHERES_DATA * sdata;
	EERIE_FASTACCESS fastaccess;
	EERIE_C_DATA * c_data;
	
};


struct EERIE_3DSCENE {
	long nbobj;
	EERIE_3DOBJ ** objs;
	Vec3f pos;
	Vec3f point0;
	long nbtex;
	TextureContainer ** texturecontainer;
	long nblight;
	EERIE_LIGHT ** light;
	CUB3D cub;
};


#if BUILD_EDIT_LOADSAVE
const size_t MAX_SCENES = 64;
struct EERIE_MULTI3DSCENE {
	long nb_scenes;
	EERIE_3DSCENE * scenes[MAX_SCENES];
	CUB3D cub;
	Vec3f pos;
	Vec3f point0;
};
#endif

//-------------------------------------------------------------------------
//Portal Data;

struct EERIE_PORTALS
{
	EERIEPOLY	poly;
	long		room_1; // facing normal
	long		room_2;
	short		useportal;
	short		paddy;
};

struct EP_DATA {
	short px;
	short py;
	short idx;
	short padd;
};

struct EERIE_ROOM_DATA {
	long nb_portals;
	long * portals;
	long nb_polys;
	EP_DATA * epdata;
	Vec3f center;
	float radius;
	unsigned short * pussIndice;
	VertexBuffer<SMY_VERTEX> * pVertexBuffer;
	unsigned long usNbTextures;
	TextureContainer ** ppTextureContainer;
};

struct EERIE_PORTAL_DATA
{
	long nb_rooms;
	inline long roomsize() {
		return nb_rooms + 1;
	}

	EERIE_ROOM_DATA * room;
	std::vector<EERIE_PORTALS> portals;
};

struct SMY_ZMAPPINFO
{
	TexturedVertex pVertex[3];
	float		uv[6];
	float		color[3];
};

struct SMY_ARXMAT
{
	unsigned long uslStartVertex;
	unsigned long uslNbVertex;

	enum TransparencyType {
		Opaque = 0,
		Blended,
		Multiplicative,
		Additive,
		Subtractive
	};

	unsigned long offset[5];
	unsigned long count[5];
};

extern bool USE_PORTALS;
extern EERIE_PORTAL_DATA * portals;

#endif // ARX_GRAPHICS_GRAPHICSTYPES_H
