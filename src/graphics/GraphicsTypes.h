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
#ifndef EERIETYPES_H
#define EERIETYPES_H

#include <cmath>
#include <vector>

#include "graphics/d3dwrapper.h"

#include "core/Common.h"

class TextureContainer;

#pragma pack(push,1)

#pragma pack(push,1)
struct EERIE_RGB
{
	float r;
	float g;
	float b;
}; // Aligned 1 2 4
#pragma pack(pop)


struct EERIE_QUAT
{
	float	x;
	float	y;
	float	z;
	float	w;
}; // Aligned 1 2 4 8

struct EERIE_2D
{
	union
	{
		float		x;
		float		a;
	};
	union
	{
		float		y;
		float		b;
	};
}; // Aligned 1 2 4 8

#pragma pack(push,1)
struct EERIE_3D
{
	union
	{
		float		x;
		float		a;
		float		yaw;
	};
	union
	{
		float		y;
		float		b;
		float		pitch;
	};
	union
	{
		float		z;
		float		g;
		float		roll;
	};

	void clear()
	{
		x = 0;
		y = 0;
		z = 0;
	}
}; // Aligned 1 2 4
#pragma pack(pop)

struct EERIE_TRI
{
	EERIE_3D v[3];
}; // Aligned 1 2 4

struct EERIE_2D_BBOX
{
	EERIE_2D min;
	EERIE_2D max;
}; // Aligned 1 2 4 8

struct EERIE_3D_BBOX
{
	EERIE_3D min;
	EERIE_3D max;
}; // Aligned 1 2 4

typedef s32 ArxSound;

struct EERIE_LIGHT
{
	char		exist;
	char		type;
	char		treat;
	char		selected;
	short		extras;
	short		status; // on/off 1/0
	EERIE_3D	pos;
	float		fallstart;
	float		fallend;
	float		falldiff;
	float		falldiffmul;
	float		precalc;
	EERIE_RGB	rgb255;
	float		intensity;
	EERIE_RGB	rgb;
	float		i;
	EERIE_3D	mins;
	EERIE_3D	maxs;
	float		temp;
	long		ltemp;
	EERIE_RGB	ex_flicker;
	float		ex_radius;
	float		ex_frequency;
	float		ex_size;
	float		ex_speed;
	float		ex_flaresize;
	long		tl;
	unsigned long	time_creation;
	long		duration; // will start to fade before the end of duration...
	ArxSound sample;
}; // Aligned 1 2 4

enum EERIE_TYPES_EXTRAS_MODE
{
	EXTRAS_SEMIDYNAMIC       = 0x00000001,
	EXTRAS_EXTINGUISHABLE    = 0x00000002,
	EXTRAS_STARTEXTINGUISHED = 0x00000004,
	EXTRAS_SPAWNFIRE         = 0x00000008,
	EXTRAS_SPAWNSMOKE        = 0x00000010,
	EXTRAS_OFF               = 0x00000020,
	EXTRAS_COLORLEGACY       = 0x00000040,
	EXTRAS_NOCASTED          = 0x00000080,
	EXTRAS_FIXFLARESIZE      = 0x00000100,
	EXTRAS_FIREPLACE         = 0x00000200,
	EXTRAS_NO_IGNIT          = 0x00000400,
	EXTRAS_FLARE	         = 0x00000800
};

#define TYP_SPECIAL1 1


//*************************************************************************************
// EERIE Types
//*************************************************************************************

struct EERIEMATRIX
{
	D3DVALUE _11, _12, _13, _14;
	D3DVALUE _21, _22, _23, _24;
	D3DVALUE _31, _32, _33, _34;
	D3DVALUE _41, _42, _43, _44;
}; // Aligned 1 2 4

struct EERIE_CYLINDER
{
	EERIE_3D	origin;
	float		radius;
	float		height;
}; // Aligned 1 2 4

struct EERIE_SPHERE
{
	EERIE_3D	origin;
	float		radius;
}; // Aligned 1 2 4

struct EERIEPOLY
{
	long 			type;	// at least 16 bits
	EERIE_3D		min;
	EERIE_3D		max;
	EERIE_3D		norm;
	EERIE_3D		norm2;
	D3DTLVERTEX		v[4];
	D3DTLVERTEX		tv[4];
	EERIE_3D		nrml[4];
	TextureContainer * tex;
	EERIE_3D		center;
	float			transval;
	float			area;
	short			room;
	short			misc;
	float			distbump;
	unsigned short	uslInd[4];
}; // Aligned 1 2 4

struct EERIE_OLD_VERTEX
{
	D3DTLVERTEX vert;
	EERIE_3D	v;
	EERIE_3D	norm;
}; // Aligned 1 2 4

struct EERIE_VERTEX
{
	EERIE_VERTEX() {}
	EERIE_VERTEX( EERIE_OLD_VERTEX& rhs )
	:
		vert(rhs.vert), v(rhs.v), norm(rhs.norm)
	{}

	D3DTLVERTEX vert;
	EERIE_3D	v;
	EERIE_3D	norm;
	EERIE_3D	vworld;
}; // Aligned 1 2 4

#define MATERIAL_NONE		0
#define MATERIAL_WEAPON		1
#define MATERIAL_FLESH		2
#define MATERIAL_METAL		3
#define MATERIAL_GLASS		4
#define MATERIAL_CLOTH		5
#define MATERIAL_WOOD		6
#define MATERIAL_EARTH		7
#define MATERIAL_WATER		8
#define MATERIAL_ICE		9
#define MATERIAL_GRAVEL		10
#define MATERIAL_STONE		11
#define MATERIAL_FOOT_LARGE	12
#define MATERIAL_FOOT_BARE	13
#define MATERIAL_FOOT_SHOE	14
#define MATERIAL_FOOT_METAL	15
#define MATERIAL_FOOT_STEALTH	16

#define POLY_NO_SHADOW		1
#define POLY_DOUBLESIDED	(1<<1)
#define POLY_TRANS			(1<<2)
#define POLY_WATER			(1<<3)
#define POLY_GLOW			(1<<4)

#define POLY_IGNORE			(1<<5)
#define POLY_QUAD			(1<<6)
#define POLY_TILED			(1<<7)
#define POLY_METAL			(1<<8)
#define POLY_HIDE			(1<<9)

#define POLY_STONE			(1<<10)
#define POLY_WOOD			(1<<11)
#define POLY_GRAVEL			(1<<12)
#define POLY_EARTH			(1<<13)
#define POLY_NOCOL			(1<<14)
#define POLY_LAVA			(1<<15)
#define POLY_CLIMB			(1<<16)
#define POLY_FALL			(1<<17)
#define POLY_NOPATH			(1<<18)
#define POLY_NODRAW			(1<<19)
#define POLY_PRECISE_PATH	(1<<20)
#define POLY_NO_CLIMB		(1<<21)
#define POLY_ANGULAR		(1<<22)
#define POLY_ANGULAR_IDX0	(1<<23)
#define POLY_ANGULAR_IDX1	(1<<24)
#define POLY_ANGULAR_IDX2	(1<<25)
#define POLY_ANGULAR_IDX3	(1<<26)
#define POLY_LATE_MIP		(1<<27)
#define IOPOLYVERT 3

struct EERIE_FACE
{
	long		facetype;	// 0 = flat  1 = text
							// 2 = Double-Side
	short		texid;
	unsigned short		vid[IOPOLYVERT];
	float		u[IOPOLYVERT];
	float		v[IOPOLYVERT];

	float		transval;
	EERIE_3D	norm;
	EERIE_3D	nrmls[IOPOLYVERT];
	float		temp;

	short		ou[IOPOLYVERT];
	short		ov[IOPOLYVERT];
	D3DCOLOR	color[IOPOLYVERT];

}; // Aligned 1 2 4

#define MAX_PFACE 16
struct EERIE_PFACE
{
	short		faceidx[MAX_PFACE];
	long		facetype;
	short		texid;  //long
	short		nbvert;
	float		transval;
	unsigned short		vid[MAX_PFACE];
	float		u[MAX_PFACE];
	float		v[MAX_PFACE];
	D3DCOLOR	color[MAX_PFACE];
};


//***********************************************************************
//*		BEGIN EERIE OBJECT STRUCTURES									*
//***********************************************************************
struct NEIGHBOURS_DATA
{
	short	nb_Nvertex;
	short	nb_Nfaces;
	short *	Nvertex;
	short *	Nfaces;
}; // Aligned 1 2 4

struct PROGRESSIVE_DATA
{
	// ingame data
	short	actual_collapse; // -1 = no collapse
	short	need_computing;
	float	collapse_ratio;
	// static data
	float	collapse_cost;
	short	collapse_candidate;
	short	padd;
}; // Aligned 1 2 4

struct EERIE_SPRINGS
{
	short	startidx;
	short	endidx;
	float	restlength;
	float	constant;	// spring constant
	float	damping;	// spring damping
	long	type;
}; // Aligned 1 2 4

#define CLOTHES_FLAG_NORMAL	0
#define CLOTHES_FLAG_FIX	1
#define CLOTHES_FLAG_NOCOL	2

struct CLOTHESVERTEX
{
	short		idx;
	char		flags;
	char		coll;
	EERIE_3D	pos;
	EERIE_3D	velocity;
	EERIE_3D	force;
	float		mass; // 1.f/mass

	EERIE_3D	t_pos;
	EERIE_3D	t_velocity;
	EERIE_3D	t_force;

	EERIE_3D	lastpos;

}; // Aligned 1 2 4

struct CLOTHES_DATA
{
	CLOTHESVERTEX *	cvert;
	CLOTHESVERTEX *	backup;
	short			nb_cvert;
	short			nb_springs;
	EERIE_SPRINGS 	* springs;
}; // Aligned 1 2 4

struct COLLISION_SPHERE
{
	short			idx;
	short			flags;
	float			radius;
}; // Aligned 1 2 4

struct COLLISION_SPHERES_DATA
{
	long				nb_spheres;
	COLLISION_SPHERE *	spheres;
}; // Aligned 1 2 4

struct PHYSVERT
{
	EERIE_3D	initpos;
	EERIE_3D	temp;
	EERIE_3D	pos;
	EERIE_3D	velocity;
	EERIE_3D	force;
	EERIE_3D	inertia;
	float		mass;
}; // Aligned 1 2 4

struct PHYSICS_BOX_DATA
{
	PHYSVERT * vert;
	long	nb_physvert;
	short	active;
	short	stopcount;
	float	radius; //radius around vert[0].pos for spherical collision
	float	storedtiming;
}; // Aligned 1 2 4


struct EERIE_GROUPLIST {
	std::string name;
	long origin;
	long nb_index;
	long * indexes;
	float siz;
	
	EERIE_GROUPLIST() : name(), origin(0), nb_index(0), indexes(NULL), siz(0.0f) { };
};

struct EERIE_ACTIONLIST {
	std::string name;
	long idx; //index vertex;
	long act; //action
	long sfx; //sfx
	
	EERIE_ACTIONLIST() : name(), idx(0), act(0), sfx(0) { };
};

struct CUB3D
{
	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;
}; // Aligned 1 2 4

struct EERIE_MOD_INFO
{
	long			link_origin;
	EERIE_3D		link_position;
	EERIE_3D		scale;
	EERIE_3D		rot;
	unsigned long	flags;
}; // Aligned 1 2 4

struct EERIE_LINKED
{
	long			lgroup; //linked to group n° if lgroup=-1 NOLINK
	long			lidx;
	long			lidx2;
	void *	obj;
	EERIE_MOD_INFO	modinfo;
	void * io;
}; // Aligned 1 2 4


struct EERIE_SELECTIONS {
	std::string name;
	long nb_selected;
	long * selected;
	
	EERIE_SELECTIONS() : name(), nb_selected(0), selected(NULL) { };
};

#define DRAWFLAG_HIGHLIGHT	1

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
	short	__padd;
};

/////////////////////////////////////////////////////////////////////////////////
struct EERIE_BONE
{
	long				nb_idxvertices;
	long 		*		idxvertices;
	EERIE_GROUPLIST *	original_group;
	long				father;
	EERIE_QUAT			quatanim;
	EERIE_3D			transanim;
	EERIE_3D			scaleanim;
	EERIE_QUAT			quatlast;
	EERIE_3D			translast;
	EERIE_3D			scalelast;
	EERIE_QUAT			quatinit;
	EERIE_3D			transinit;
	EERIE_3D			scaleinit;
	EERIE_3D			transinit_global;
};

struct EERIE_C_DATA
{
	EERIE_BONE *	bones;
	long			nb_bones;
};
//////////////////////////////////////////////////////////////////////////////////
struct EERIE_3DPAD
{
	float x;
	float y;
	float z;
	float w;
};

struct EERIE_3DOBJ
{
	EERIE_3DOBJ()
	{
		// TODO Make it possible to use a default
		// conststructor for these
		pos.x = pos.y = pos.z = 0;
		point0 = pos;
		angle = pos;

		origin = 0;
		ident = 0;
		nbvertex = 0;
		true_nbvertex = 0;
		nbfaces = 0;
		nbpfaces = 0;
		nbmaps = 0;
		nbgroups = 0;
		drawflags = 0;

		vertexlocal = 0;
		vertexlist = 0;
		vertexlist3 = 0;

		facelist = 0;
		pfacelist = 0;
		grouplist = 0;
		texturecontainer = 0;

		originaltextures = 0;
		linked = 0;

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

		pbox = 0;
		pdata = 0;
		ndata = 0;
		cdata = 0;
		sdata = 0;

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
		fastaccess.__padd = 0;

		c_data = 0;
	}
	
	void clear();
	
	std::string name;
	std::string file;
	EERIE_3D			pos;
	EERIE_3D			point0;
	EERIE_3D			angle;
	long				origin;
	long				ident;
	long				nbvertex;
	long				true_nbvertex;
	long				nbfaces;
	long				nbpfaces;
	long				nbmaps;
	long				nbgroups;
	unsigned long		drawflags;
	EERIE_3DPAD 	*	vertexlocal;
	EERIE_VERTEX 	*	vertexlist;
	EERIE_VERTEX 	*	vertexlist3;

	EERIE_FACE 	*	facelist;
	EERIE_PFACE 	*	pfacelist;
	EERIE_GROUPLIST *	grouplist;
	std::vector<EERIE_ACTIONLIST> actionlist;
	std::vector<EERIE_SELECTIONS> selections;
	TextureContainer ** texturecontainer;

	char 		*		originaltextures;
	CUB3D				cub;
	EERIE_QUAT			quat;
	EERIE_LINKED 	*	linked;
	long				nblinked;


	PHYSICS_BOX_DATA *	pbox;
	PROGRESSIVE_DATA 		*		pdata;
	NEIGHBOURS_DATA 		*		ndata;
	CLOTHES_DATA 		*			cdata;
	COLLISION_SPHERES_DATA *	sdata;
	EERIE_FASTACCESS	fastaccess;
	EERIE_C_DATA 	*	c_data;

}; // Aligned 1 2 4


struct EERIE_3DSCENE
{
	long			nbobj;
	EERIE_3DOBJ **	objs;
	EERIE_3D		pos;
	EERIE_3D		point0;
	long			nbtex;
	TextureContainer ** texturecontainer;
	long			nblight;
	EERIE_LIGHT  ** light;
	float			ambient_r;
	float			ambient_g;
	float			ambient_b;
	CUB3D			cub;
}; // Aligned 1 2 4

#define MAX_SCENES 64
struct EERIE_MULTI3DSCENE
{
	long	nb_scenes;
	EERIE_3DSCENE * scenes[MAX_SCENES];
	CUB3D			cub;
	EERIE_3D		pos;
	EERIE_3D		point0;
}; // Aligned 1 2 4

struct EERIE_FRAME
{
	long		num_frame;
	long		flag;
	int			master_key_frame;
	short		f_translate; //int
	short		f_rotate; //int
	float		time;
	EERIE_3D	translate;
	EERIE_QUAT	quat;
	ArxSound	sample;
}; // Aligned 1 2 4



struct EERIE_GROUP
{
	int		key;
	EERIE_3D	translate;
	EERIE_QUAT	quat;
	EERIE_3D	zoom;
}; // Aligned 1 2 4

// Animation playing flags
#define EA_LOOP			1	// Must be looped at end (indefinitely...)
#define EA_REVERSE		2	// Is played reversed (from end to start)
#define EA_PAUSED		4	// Is paused
#define EA_ANIMEND		8	// Has just finished
#define	EA_STATICANIM	16	// Is a static Anim (no movement offset returned).
#define	EA_STOPEND		32	// Must Be Stopped at end.
#define EA_FORCEPLAY	64	// User controlled... MUST be played...
#define EA_EXCONTROL	128	// ctime externally set, no update.
struct EERIE_ANIM
{
	float		anim_time;
	unsigned long	flag;
	long		nb_groups;
	long		nb_key_frames;
	EERIE_FRAME *	frames;
	EERIE_GROUP  *  groups;
	unsigned char *	voidgroups;
}; // Aligned 1 2 4

//-------------------------------------------------------------------------
//Portal Data;
#pragma pack(push,1)
struct SAVE_EERIEPOLY
{
	long 			type;	// at least 16 bits
	EERIE_3D		min;
	EERIE_3D		max;
	EERIE_3D		norm;
	EERIE_3D		norm2;
	D3DTLVERTEX		v[4];
	D3DTLVERTEX		tv[4];
	EERIE_3D		nrml[4];
	TextureContainer * tex;
	EERIE_3D		center;
	float			transval;
	float			area;
	short			room;
	short			misc;
}; // Aligned 1 2 4
#pragma pack(pop)

#pragma pack(push,1)
struct EERIE_SAVE_PORTALS
{
	SAVE_EERIEPOLY	poly;
	long		room_1; // facing normal
	long		room_2;
	short		useportal;
	short		paddy;
};
#pragma pack(pop)

struct EERIE_PORTALS
{
	EERIEPOLY	poly;
	long		room_1; // facing normal
	long		room_2;
	short		useportal;
	short		paddy;
};

#pragma pack(push,1)
struct EP_DATA
{
	short	px;
	short	py;
	short	idx;
	short	padd;
};
#pragma pack(pop)

struct EERIE_ROOM_DATA
{
	long nb_portals;
	long * portals;
	long nb_polys;
	EP_DATA * epdata;
	EERIE_3D	center;
	float		radius;
	unsigned short		*		pussIndice;
	LPDIRECT3DVERTEXBUFFER7		pVertexBuffer;
	unsigned long				usNbTextures;
	TextureContainer		**	ppTextureContainer;
};

#pragma pack(push,1)
struct EERIE_SAVE_ROOM_DATA
{
	long nb_portals;
	long nb_polys;
	long padd[6];
};
#pragma pack(pop)

struct EERIE_PORTAL_DATA
{
	long nb_rooms;
	EERIE_ROOM_DATA * room;
	long nb_total;	// of portals
	EERIE_PORTALS * portals;
};

#pragma pack(pop)


#define ARX_D3DVERTEX D3DTLVERTEX


struct SMY_D3DVERTEX
{
	float	x, y, z;
	int		color;
	float	tu, tv;
};

struct SMY_D3DVERTEX3
{
	float	x, y, z;
	int		color;
	float	tu, tv;
	float	tu2, tv2;
	float	tu3, tv3;
};

struct SMY_D3DVERTEX3_T
{
	float	x, y, z;
	float	rhw;
	int		color;
	float	tu, tv;
	float	tu2, tv2;
	float	tu3, tv3;
};

struct SMY_ZMAPPINFO
{
	D3DTLVERTEX pD3DVertex[3];
	float		uv[6];
	float		color[3];
};

struct SMY_ARXMAT
{
	unsigned long uslStartVertex;
	unsigned long uslNbVertex;

	unsigned long uslStartCull;
	unsigned long uslNbIndiceCull;
	unsigned long uslStartNoCull;
	unsigned long uslNbIndiceNoCull;

	unsigned long uslStartCull_TNormalTrans;
	unsigned long uslNbIndiceCull_TNormalTrans;
	unsigned long uslStartNoCull_TNormalTrans;
	unsigned long uslNbIndiceNoCull_TNormalTrans;

	unsigned long uslStartCull_TMultiplicative;
	unsigned long uslNbIndiceCull_TMultiplicative;
	unsigned long uslStartNoCull_TMultiplicative;
	unsigned long uslNbIndiceNoCull_TMultiplicative;

	unsigned long uslStartCull_TAdditive;
	unsigned long uslNbIndiceCull_TAdditive;
	unsigned long uslStartNoCull_TAdditive;
	unsigned long uslNbIndiceNoCull_TAdditive;

	unsigned long uslStartCull_TSubstractive;
	unsigned long uslNbIndiceCull_TSubstractive;
	unsigned long uslStartNoCull_TSubstractive;
	unsigned long uslNbIndiceNoCull_TSubstractive;
};

class CMY_DYNAMIC_VERTEXBUFFER
{
	public:
		unsigned long			uslFormat;
		unsigned short			ussMaxVertex;
		unsigned short			ussNbVertex;
		unsigned short			ussNbIndice;
		LPDIRECT3DVERTEXBUFFER7	pVertexBuffer;
		unsigned short		*	pussIndice;
	public:
		CMY_DYNAMIC_VERTEXBUFFER(unsigned short, unsigned long);
		~CMY_DYNAMIC_VERTEXBUFFER();

		void * Lock(unsigned int);
		bool UnLock();
};

#define FVF_D3DVERTEX	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXTUREFORMAT2)
#define FVF_D3DVERTEX2	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXTUREFORMAT2)
#define FVF_D3DVERTEX3	(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX3|D3DFVF_TEXTUREFORMAT2)

#define FVF_D3DVERTEX_T		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1|D3DFVF_TEXTUREFORMAT2)
#define FVF_D3DVERTEX2_T	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXTUREFORMAT2)
#define FVF_D3DVERTEX3_T	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX3|D3DFVF_TEXTUREFORMAT2)

extern long USE_PORTALS;
extern EERIE_PORTAL_DATA * portals;

#endif
