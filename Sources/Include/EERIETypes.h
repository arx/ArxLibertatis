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

#define D3D_OVERLOADS
#include <d3d.h>
#include <ARX_Common.h>

#pragma inline_depth (255)
#pragma inline_recursion (on)
#pragma auto_inline (on)

class TextureContainer;

typedef struct
{
	float r;
	float g;
	float b;
} EERIE_RGB; // Aligned 1 2 4


typedef struct
{
	float	x;
	float	y;
	float	z;
	float	w;
} EERIE_QUAT; // Aligned 1 2 4 8

typedef struct
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
} EERIE_2D; // Aligned 1 2 4 8

typedef struct
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
} EERIE_3D; // Aligned 1 2 4

typedef struct
{
	EERIE_3D v[3];
} EERIE_TRI; // Aligned 1 2 4

typedef struct
{
	EERIE_2D min;
	EERIE_2D max;
} EERIE_2D_BBOX; // Aligned 1 2 4 8

typedef struct
{
	EERIE_3D min;
	EERIE_3D max;
} EERIE_3D_BBOX; // Aligned 1 2 4

typedef struct
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
	long sample;
} EERIE_LIGHT; // Aligned 1 2 4

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

typedef struct _EERIEMATRIX
{
	D3DVALUE _11, _12, _13, _14;
	D3DVALUE _21, _22, _23, _24;
	D3DVALUE _31, _32, _33, _34;
	D3DVALUE _41, _42, _43, _44;
} EERIEMATRIX ; // Aligned 1 2 4

typedef struct
{
	EERIE_3D	origin;
	float		radius;
	float		height;
} EERIE_CYLINDER; // Aligned 1 2 4

typedef struct
{
	EERIE_3D	origin;
	float		radius;
} EERIE_SPHERE; // Aligned 1 2 4

typedef struct
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
} EERIEPOLY; // Aligned 1 2 4
 
typedef struct
{
	D3DTLVERTEX vert;
	EERIE_3D	v;
	EERIE_3D	norm;
	EERIE_3D	vworld;
} EERIE_VERTEX; // Aligned 1 2 4

typedef struct
{
	D3DTLVERTEX vert;
	EERIE_3D	v;
	EERIE_3D	norm;
} EERIE_OLD_VERTEX; // Aligned 1 2 4

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

typedef struct
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

} EERIE_FACE; // Aligned 1 2 4

#define MAX_PFACE 16
typedef struct
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
} EERIE_PFACE;

typedef struct
{
	long		facetype;	// 0 = flat  1 = text
							// 2 = Double-Side
	D3DCOLOR	rgb[IOPOLYVERT];
	unsigned short		vid[IOPOLYVERT];
	short		texid; 
	float		u[IOPOLYVERT];
	float		v[IOPOLYVERT];
	short		ou[IOPOLYVERT];
	short		ov[IOPOLYVERT];

	float		transval;
	EERIE_3D	norm;
	EERIE_3D	nrmls[IOPOLYVERT];
	float		temp;

} EERIE_FACE_FTL; // Aligned 1 2 4

//***********************************************************************
//*		BEGIN EERIE OBJECT STRUCTURES									*
//***********************************************************************
typedef struct
{
	short	nb_Nvertex;
	short	nb_Nfaces;
	short *	Nvertex;
	short *	Nfaces;
} NEIGHBOURS_DATA; // Aligned 1 2 4

typedef struct
{
	// ingame data
	short	actual_collapse; // -1 = no collapse
	short	need_computing;
	float	collapse_ratio;
	// static data
	float	collapse_cost;
	short	collapse_candidate;
	short	padd;
} PROGRESSIVE_DATA; // Aligned 1 2 4

typedef struct
{
	short	startidx;
	short	endidx;
	float	restlength;
	float	constant;	// spring constant
	float	damping;	// spring damping
	long	type;
} EERIE_SPRINGS; // Aligned 1 2 4

#define CLOTHES_FLAG_NORMAL	0
#define CLOTHES_FLAG_FIX	1
#define CLOTHES_FLAG_NOCOL	2

typedef struct
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
	
} CLOTHESVERTEX; // Aligned 1 2 4

typedef struct
{
	CLOTHESVERTEX *	cvert;
	CLOTHESVERTEX *	backup;
	short			nb_cvert;
	short			nb_springs;
	EERIE_SPRINGS 	* springs;
} CLOTHES_DATA; // Aligned 1 2 4

typedef struct
{
	short			idx;
	short			flags;
	float			radius;
} COLLISION_SPHERE; // Aligned 1 2 4

typedef struct
{
	long				nb_spheres;
	COLLISION_SPHERE *	spheres;
} COLLISION_SPHERES_DATA; // Aligned 1 2 4

typedef struct
{
	EERIE_3D	initpos;
	EERIE_3D	temp;
	EERIE_3D	pos;
	EERIE_3D	velocity;
	EERIE_3D	force;
	EERIE_3D	inertia;
	float		mass;
} PHYSVERT; // Aligned 1 2 4

typedef struct
{
	PHYSVERT * vert;
	long	nb_physvert;
	short	active;
	short	stopcount;
	float	radius; //radius around vert[0].pos for spherical collision
	float	storedtiming;
} PHYSICS_BOX_DATA; // Aligned 1 2 4


typedef struct
{
	long			sx;
	long			sy;
	unsigned long	bpp;
	unsigned char * bmpdata;
} EERIE_MAP; // Aligned 1 2 4


typedef struct
{
	char		name[256];
	long		origin;
	long		nb_index;
	long 	*	indexes;
	float		siz;
} EERIE_GROUPLIST; // Aligned 1 2 4

typedef struct
{
	char			name[256];
	long			idx; //index vertex;
	long			act; //action
	long			sfx; //sfx
} EERIE_ACTIONLIST; // Aligned 1 2 4

typedef struct
{
	float	xmin;
	float	xmax;
	float	ymin;
	float	ymax;
	float	zmin;
	float	zmax;
} CUB3D; // Aligned 1 2 4

typedef struct
{
	long			link_origin;
	EERIE_3D		link_position;
	EERIE_3D		scale;
	EERIE_3D		rot;
	unsigned long	flags;
} EERIE_MOD_INFO; // Aligned 1 2 4

typedef struct
{
	long			lgroup; //linked to group n° if lgroup=-1 NOLINK
	long			lidx;
	long			lidx2;
	void *	obj;
	EERIE_MOD_INFO	modinfo;
	void * io;
} EERIE_LINKED; // Aligned 1 2 4


typedef struct
{
	char	name[64];
	long	nb_selected;
	long *	selected;
} EERIE_SELECTIONS; // Aligned 1 2 4

#define DRAWFLAG_HIGHLIGHT	1

typedef struct
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
} EERIE_FASTACCESS;

/////////////////////////////////////////////////////////////////////////////////
typedef struct
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
} EERIE_BONE;

typedef struct
{
	EERIE_BONE *	bones;
	long			nb_bones;
} EERIE_C_DATA;
//////////////////////////////////////////////////////////////////////////////////
typedef struct
{
	float x;
	float y;
	float z;
	float w;
} EERIE_3DPAD ;
typedef struct
{
	char				name[256];
	char				file[256];
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
	long				nbaction;
	long				nbselections;
	unsigned long		drawflags;
	EERIE_3DPAD 	*	vertexlocal;
	EERIE_VERTEX 	*	vertexlist;
	EERIE_VERTEX 	*	vertexlist3;

	EERIE_FACE 	*	facelist;
	EERIE_PFACE 	*	pfacelist;
	EERIE_MAP 	*		maplist;
	EERIE_GROUPLIST *	grouplist;
	EERIE_ACTIONLIST *	actionlist;
	EERIE_SELECTIONS *	selections;
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

} EERIE_3DOBJ; // Aligned 1 2 4


typedef struct
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
} EERIE_3DSCENE; // Aligned 1 2 4


#define MAX_SCENES 64
typedef struct
{
	long	nb_scenes;
	EERIE_3DSCENE * scenes[MAX_SCENES];
	CUB3D			cub;
	EERIE_3D		pos;
	EERIE_3D		point0;
} EERIE_MULTI3DSCENE; // Aligned 1 2 4


typedef struct
{
	long		num_frame;
	long		flag;
	int			master_key_frame;
	short		f_translate; //int
	short		f_rotate; //int
	float		time;
	EERIE_3D	translate;
	EERIE_QUAT	quat;
	long		sample;
} EERIE_FRAME; // Aligned 1 2 4



typedef struct
{
	int		key;
	EERIE_3D	translate;
	EERIE_QUAT	quat;
	EERIE_3D	zoom;
} EERIE_GROUP; // Aligned 1 2 4

// Animation playing flags
#define EA_LOOP			1	// Must be looped at end (indefinitely...)
#define EA_REVERSE		2	// Is played reversed (from end to start)
#define EA_PAUSED		4	// Is paused
#define EA_ANIMEND		8	// Has just finished
#define	EA_STATICANIM	16	// Is a static Anim (no movement offset returned).
#define	EA_STOPEND		32	// Must Be Stopped at end.
#define EA_FORCEPLAY	64	// User controlled... MUST be played...
#define EA_EXCONTROL	128	// ctime externally set, no update.
typedef struct
{
	float		anim_time;
	unsigned long	flag;
	long		nb_groups;
	long		nb_key_frames;
	EERIE_FRAME *	frames;
	EERIE_GROUP  *  groups;
	unsigned char *	voidgroups;
} EERIE_ANIM; // Aligned 1 2 4

//-------------------------------------------------------------------------
//Portal Data;
typedef struct
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
} SAVE_EERIEPOLY; // Aligned 1 2 4

typedef struct
{
	SAVE_EERIEPOLY	poly;
	long		room_1; // facing normal
	long		room_2;
	short		useportal;
	short		paddy;
} EERIE_SAVE_PORTALS;

typedef struct
{
	EERIEPOLY	poly;
	long		room_1; // facing normal
	long		room_2;
	short		useportal;
	short		paddy;
} EERIE_PORTALS;


typedef struct
{
	short	px;
	short	py;
	short	idx;
	short	padd;
} EP_DATA;

typedef struct
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
} EERIE_ROOM_DATA;

typedef struct
{
	long nb_portals;
	long nb_polys;
	long padd[6];
} EERIE_SAVE_ROOM_DATA;

typedef struct
{
	long nb_rooms;
	EERIE_ROOM_DATA * room;
	long nb_total;	// of portals
	EERIE_PORTALS * portals;
} EERIE_PORTAL_DATA;


#define ARX_D3DVERTEX D3DTLVERTEX


typedef struct
{
	float	x, y, z;
	int		color;
	float	tu, tv;
} SMY_D3DVERTEX;

typedef struct
{
	float	x, y, z;
	int		color;
	float	tu, tv;
	float	tu2, tv2;
	float	tu3, tv3;
} SMY_D3DVERTEX3;

typedef struct
{
	float	x, y, z;
	float	rhw;
	int		color;
	float	tu, tv;
	float	tu2, tv2;
	float	tu3, tv3;
} SMY_D3DVERTEX3_T;

typedef struct
{
	D3DTLVERTEX pD3DVertex[3];
	float		uv[6];
	float		color[3];
} SMY_ZMAPPINFO;

typedef struct
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
} SMY_ARXMAT;

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
