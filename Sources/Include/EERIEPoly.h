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
// EERIEPoly
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//	Hum...hum
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#ifndef EERIEPOLY_H
#define EERIEPOLY_H


#include "EERIETypes.h"

void specialEE_RTP(D3DTLVERTEX*,D3DTLVERTEX*);
void EERIE_CreateMatriceProj(float _fWidth,float _fHeight,float _fFOV,float _fZNear,float _fZFar);

//-----------------------------------------------------------------------------
typedef struct
{
	char			path[256]; // path[0]==0 means an unallocated slot
	EERIE_ANIM **	anims;
	long *			sizes;
	short			alt_nb;
	long			locks;
} ANIM_HANDLE;

//minimap special infos
typedef struct
{
	short x;
	short y;
} EERIE_S2D;

typedef struct
{
	float posx;
	float posy;
	float posz;
	float ycos;
	float ysin;
	float xsin;
	float xcos;
	float use_focal;
	float xmod;
	float ymod;
	float zmod;
} EERIE_TRANSFORM;

typedef struct
{
	EERIE_TRANSFORM transform;
	EERIE_3D pos; // 0 4 8
	float	Ycos; // 12
	float	Ysin; // 16
	float	Xcos; // 20
	float	Xsin; // 24
	float	Zcos; // 28
	float	Zsin; // 32	
	float	focal;// 36
	float	use_focal;
	float	Zmul; // 40
	float posleft;// 44
	float postop; // 48 do not move/insert before this point !!!
	
	float	xmod;
	float	ymod;
	EERIEMATRIX matrix;
	EERIE_3D angle;

	EERIE_3D d_pos;
	EERIE_3D d_angle;
	EERIE_3D lasttarget;
	EERIE_3D lastpos;
	EERIE_3D translatetarget;
	BOOL	lastinfovalid;
	EERIE_3D norm;
	EERIE_RGB	fadecolor;
	RECT	clip;
	float	clipz0;
	float	clipz1;
	long	centerx;
	long	centery;
	
	float	smoothing;
	float	AddX;
	float	AddY;
	long	Xsnap;
	long	Zsnap;
	float	Zdiv;
	
	long	clip3D;
	long	type;
	long	bkgcolor;
	long	nbdrawn;
	float	cdepth;

	EERIE_3D	size;
} EERIE_CAMERA;

#define ANCHOR_FLAG_GREEN_DRAW	1
#define ANCHOR_FLAG_BLOCKED		8

typedef struct _ANCHOR_DATA
{
	EERIE_3D	pos;
	short		nblinked;
	short		flags;
	long	*	linked;
	float		radius;
	float		height;
} _ANCHOR_DATA;

typedef struct
{
	char				treat;
	char				nothing;
	short				nbpoly;
	short				nbianchors;
	short				nbpolyin;
	float				frustrum_miny;
	float				frustrum_maxy;	
	EERIEPOLY *			polydata;
	EERIEPOLY **		polyin;	
	long *				ianchors; // index on anchors list
	long				flags;
	float				tile_miny;
	float				tile_maxy;
} EERIE_BKG_INFO;

typedef struct
{
	short min;
	short max;
} EERIE_SMINMAX;

#define MAX_GOSUB 10
#define MAX_SHORTCUT 80
#define MAX_SCRIPTTIMERS 5
#define FBD_TREAT		1
#define FBD_NOTHING		2

typedef struct
{
	char				treat;
	char				nothing;
	short				nbpoly;
	short				nbianchors;
	short				nbpolyin;
	long				flags;
	float				frustrum_miny;
	float				frustrum_maxy;	
	EERIEPOLY *			polydata;
	EERIEPOLY **		polyin;	
	long *				ianchors; // index on anchors list
} FAST_BKG_DATA;
#define MAX_BKGX	160
#define MAX_BKGZ	160
#define BKG_SIZX	100
#define BKG_SIZZ	100


typedef struct 
{	
	FAST_BKG_DATA	fastdata[MAX_BKGX][MAX_BKGZ];
	long		exist;
	short		Xsize;
	short		Zsize;
	short		Xdiv;
	short		Zdiv;
	float		Xmul;
	float		Zmul;
	EERIE_BKG_INFO * Backg;
	EERIE_RGB	ambient;
	EERIE_RGB	ambient255;
	EERIE_SMINMAX *	minmax;
	long		  nbanchors;	
	_ANCHOR_DATA * anchors;
	char		name[256];
} EERIE_BACKGROUND;

typedef struct 
{	
	long	type;
	long	ival;
	float	fval;
	char *	text;  // for a TEXT type ival equals strlen(text).
	char 	name[64];
} SCRIPT_VAR;
typedef struct
{
	char *		string;
	long		idx;
} LABEL_INFO;

typedef struct
{
	long			size;
	char *			data;
	long			sub[MAX_GOSUB];
	long			nblvar;
	SCRIPT_VAR *	lvar;
	unsigned long	lastcall;
	unsigned long	timers[MAX_SCRIPTTIMERS];
	long			allowevents;
	void *			master;
	long			shortcut[MAX_SHORTCUT];
	long			nb_labels;
	LABEL_INFO *	labels;
} EERIE_SCRIPT;

typedef struct
{
	float	value;
	short	flags;
	short	special;
} IO_EQUIPITEM_ELEMENT;

#define IO_EQUIPITEM_ELEMENT_Number				29
typedef struct
{
	IO_EQUIPITEM_ELEMENT elements[IO_EQUIPITEM_ELEMENT_Number];
} IO_EQUIPITEM;

typedef struct
{
	ANIM_HANDLE *	next_anim;
	ANIM_HANDLE *	cur_anim;
	short			altidx_next; // idx to alternate anims...
	short			altidx_cur; // idx to alternate anims...
	long			ctime;
	unsigned long	flags;
	unsigned long	nextflags;
	long			lastframe;
	float			pour;
	long			fr;
} ANIM_USE;

#define MAX_ANIM_LAYERS	4
typedef struct
{
	long			exist;
	unsigned long	behavior;
	float			behavior_param;
	long			tactics;		// 0=none ; 1=side ; 2=side+back
	long			target;
	long			movemode;
	ANIM_USE		animlayer[MAX_ANIM_LAYERS];
} IO_BEHAVIOR_DATA;


typedef struct
{
	long		castingspell; // spell being casted...
	unsigned char	symb[4]; // symbols to draw before casting...
	short		spell_flags;
	short		spell_level;
	long		target;
	long		duration;
} IO_SPELLCAST_DATA;

typedef struct
{
	unsigned long flags;
	long	listnb;	
	unsigned short * list;	
	unsigned short listpos;
	short pathwait;
	long	truetarget;
} IO_PATHFIND;

#define MAX_EXTRA_ROTATE 4

typedef struct
{
	long		flags;
	short		group_number[MAX_EXTRA_ROTATE];
	EERIE_3D	group_rotate[MAX_EXTRA_ROTATE];
} EERIE_EXTRA_ROTATE;

#define MAX_STACKED_BEHAVIOR 5

typedef struct
{
	float		maxlife;		
	float		life;			
	float		maxmana;		
	float		mana;			
	unsigned long	reachedtime;	
	long		reachedtarget;	//Is target in REACHZONE ?
	void *		weapon;			//Linked Weapon (r-hand)
	long		detect;			
	long		movemode;		
	float		armor_class;
	float		absorb;			
	float		damages;
	float		tohit;
	float		aimtime;
	float		critical;
	float		reach;
	float		backstab_skill;

	unsigned long	behavior;
	float			behavior_param;
	long			tactics;		 // 0=none ; 1=side ; 2=side+back
	long		xpvalue;
	long		cut;

	float				moveproblem;
	long		weapontype;
	long		weaponinhand;
	long		fightdecision;
	char		weaponname[256];

	float		look_around_inc;
	unsigned long collid_time;
	long		collid_state;
	float		speakpitch;
	float		lastmouth;
	long		ltemp;

	IO_BEHAVIOR_DATA	stacked[MAX_STACKED_BEHAVIOR];
	float		poisonned;
	unsigned char	resist_poison;
	unsigned char	resist_magic;
	unsigned char	resist_fire;
	unsigned char	padd;

	short		strike_time;
	short		walk_start_time;
	long		aiming_start;
	long		npcflags;
	IO_PATHFIND		pathfind;
	EERIE_EXTRA_ROTATE *	ex_rotate;
	D3DCOLOR	blood_color;

	short SPLAT_DAMAGES;
	short SPLAT_TOT_NB;
	EERIE_3D last_splat_pos;
	float vvpos;

	float climb_count;
	float stare_factor;
	float fDetect;
	short				cuts;
	short				unused;
} IO_NPCDATA;

typedef struct 
{
	IO_EQUIPITEM *		equipitem;			// Equipitem Datas
	long				price;	
	short				maxcount;			// max number cumulable
	short				count;				// current number	
	char				food_value;			
	char				stealvalue;
	short				playerstacksize;
	short				LightValue;
} IO_ITEMDATA;

typedef struct 
{
	char				trapvalue;
	char				padd[3];
} IO_FIXDATA;


typedef struct 
{	
	EERIE_CAMERA		cam;
} IO_CAMDATA;

typedef struct
{
	EERIE_RGB		color;
	float			radius;
	unsigned long	flags;
	long			dynlight;
	EERIE_3D		offset;
} IO_HALO;

typedef struct
{
	EERIE_CYLINDER	cyl;
	EERIE_3D		startpos;
	EERIE_3D		targetpos;
	EERIE_3D		velocity;
	EERIE_3D		forces;
} IO_PHYSICS;

typedef struct
{
	 char			filename[256];
	 char			skintochange[256];
	 char			skinchangeto[256];
} IO_TWEAKER_INFO;

typedef struct
{
	 char			name[64];
} IO_GROUP_DATA;

typedef struct
{
	long type;
	char param1[256];
	char param2[256];
} TWEAK_INFO;

#define IO_NPC_AFLAG_HIT_BACKGROUND			1
#define IO_NPC_AFLAG_HIT_CLEAR				(1)

#define SFX_TYPE_YLSIDE_DEATH	1
#define SFX_TYPE_INCINERATE		2

#define MAX_ANIMS 200		// max loadable anims per character

typedef struct
{
	long				ioflags;			// IO type
	EERIE_3D			lastpos;	// IO last position
	EERIE_3D			pos;		// IO position
	EERIE_3D			move;
	EERIE_3D			lastmove;
	EERIE_3D			forcedmove;

	EERIE_3D			angle;		// IO angle
	IO_PHYSICS			physics;	// Movement Collision Data	
	short				room;
	short				room_flags; // 1==need_update
	float				original_height;
	float				original_radius;
	TextureContainer *	inv;		// Object Icon
	EERIE_3DOBJ *		obj;		// IO Mesh data
	ANIM_HANDLE *		anims[MAX_ANIMS];	// Object Animations
	ANIM_USE			animlayer[MAX_ANIM_LAYERS];
	EERIE_3D *			lastanimvertex;		// Last Animation Positions of Vertex
	long				nb_lastanimvertex;
	unsigned long		lastanimtime;

	EERIE_3D_BBOX		bbox3D;
	EERIE_S2D			bbox1;		// 2D bounding box1
	EERIE_S2D			bbox2;		// 2D bounding box2
	char *				usemesh;	// Alternate Mesh/path
	EERIE_3DOBJ *		tweaky;		// tweaked original obj backup
	long				sound;

	unsigned long		type_flags;			// object type (weapon,goblin...)
	long				scriptload;			// Is This object Loaded by Script ?
	EERIE_3D			target;				// Target position	
	long				targetinfo;			// Target Type/Ident

	long				cstep;
	union 
	{
		IO_ITEMDATA *		_itemdata;			// ITEM Datas
		IO_FIXDATA	*		_fixdata;			// FIX Datas
		IO_NPCDATA *		_npcdata;			// NPC Datas
		IO_CAMDATA *		_camdata;			// Camera Datas
	};
	
	void *				inventory;			// Inventory Data
	short				show;				// Show Status (In Scene, In Inventory...)
	short				collision;			// collision type
	char 				mainevent[64];
	EERIE_RGB			infracolor;			// Improve Vision Color (Heat)
	long				changeanim;	
	
	long				ident;				// Ident num
	float				weight;
	char				locname[64];		//localisation
	unsigned short		EditorFlags; // 1 NOTSAVED 2 selected 
	unsigned short		GameFlags; // GFLAGS
	EERIE_3D			velocity;			// velocity
	float				fall;

	long				stopped;
	EERIE_3D			initpos;			// Initial Position
	EERIE_3D			initangle;			// Initial Angle
	char				filename[256];
	float				scale;
	
	void *				usepath;
	void *				symboldraw;
	short				dynlight;
	short				lastspeechflag;
	void *				inzone;
	IO_HALO				halo;
	IO_HALO				halo_native;
	
	EERIE_SCRIPT		script;				// Primary Script
	EERIE_SCRIPT		over_script;		// Overriding Script
	short				stat_count;
	short				stat_sent;
	IO_TWEAKER_INFO *	tweakerinfo; // optional tweaker infos
	long				material;
	

	IO_GROUP_DATA *		iogroups;	
	short				nb_iogroups;
	char				sizex;		// Inventory Icon sizeX
	char				sizey;		// Inventory Icon sizeY
	unsigned long		soundtime;
	unsigned long		soundcount;

	short				level;
	short				truelevel;
	unsigned long		sfx_time;
	unsigned long		collide_door_time;
	unsigned long		ouch_time;
	float				dmg_sum;

	IO_SPELLCAST_DATA	spellcast_data;
	short				flarecount;
	short				no_collide;
	float				invisibility;
	float				frameloss;
	float				basespeed;

	float				speed_modif;
	long *				spells_on;
	short				nb_spells_on;
	short				padding;
	long				damagedata;

	float				rubber;
	float				max_durability;
	float				durability;
	short				poisonous;
	short				poisonous_count;

	float				ignition;
	long				ignit_light;
	long				ignit_sound;
	float				head_rot;
	
	short				damager_damages;
	short				padding2;
	long				damager_type;
	char *				stepmaterial;
	char *				armormaterial;
	char *				weaponmaterial;
	char *				strikespeech;	

	short				sfx_flag;
	short				Tweak_nb;
	TWEAK_INFO	*		Tweaks;
	char				secretvalue;	
	char				padddd[3];

	char *				shop_category;
	float				shop_multiply;
	char *				inventory_skin;
	long				aflags;		// additionnal flags
	short				inzone_show;
	short				summoner;
	long spark_n_blood;
} INTERACTIVE_OBJ;

//-----------------------------------------------------------------------------
#define BEHAVIOUR_NONE			1		// no pathfind
#define BEHAVIOUR_FRIENDLY		(1<<1)		// no pathfind
#define BEHAVIOUR_MOVE_TO		(1<<2)	
#define BEHAVIOUR_WANDER_AROUND	(1<<3)	//behavior_param = distance
#define BEHAVIOUR_FLEE			(1<<4)	//behavior_param = distance
#define BEHAVIOUR_HIDE			(1<<5)	//behavior_param = distance
#define BEHAVIOUR_LOOK_FOR		(1<<6)	//behavior_param = distance
#define BEHAVIOUR_SNEAK			(1<<7)
#define BEHAVIOUR_FIGHT			(1<<8)
#define BEHAVIOUR_DISTANT		(1<<9)
#define BEHAVIOUR_MAGIC			(1<<10)
#define BEHAVIOUR_GUARD			(1<<11)
#define BEHAVIOUR_GO_HOME		(1<<12)
#define BEHAVIOUR_LOOK_AROUND	(1<<13)
#define BEHAVIOUR_STARE_AT		(1<<14)
#define MAX_EQUIPED 12
#define MORE_COMPATIBILITY

extern long MAX_ANIMATIONS;
#define MAX_TRANSPOL 512
#define MAX_INTERTRANSPOL 512

#define EERIECOLOR_RED		0xFFFF0000
#define EERIECOLOR_GREEN	0xFF00FF00
#define EERIECOLOR_BLUE		0xFF0000FF
#define EERIECOLOR_YELLOW	0xFFFFFF00
#define EERIECOLOR_CYAN		0xFF00FFFF
#define EERIECOLOR_MAGENTA	0xFFFF00FF
#define EERIECOLOR_BLACK	0xFF000000
#define EERIECOLOR_WHITE	0xFFFFFFFF

#define CAM_SUBJVIEW 0
#define CAM_TOPVIEW 1

// ARX_COLLISIONS flags (cylinder move)
#define CFLAG_LEVITATE			1
#define CFLAG_NO_INTERCOL		(1<<1)
#define CFLAG_SPECIAL			(1<<2)
#define CFLAG_EASY_SLIDING		(1<<3)
#define CFLAG_CLIMBING			(1<<4)
#define CFLAG_JUST_TEST			(1<<5)
#define CFLAG_NPC				(1<<6)
#define CFLAG_PLAYER			(1<<7)
#define CFLAG_RETURN_HEIGHT		(1<<8)
#define CFLAG_EXTRA_PRECISION	(1<<9)
#define CFLAG_CHECK_VALID_POS	(1<<10)
#define CFLAG_ANCHOR_GENERATION	(1<<11)
#define CFLAG_COLLIDE_NOCOL		(1<<12)
#define CFLAG_NO_NPC_COLLIDE	(1<<13)
#define CFLAG_NO_HEIGHT_MOD		(1<<14)

#define IO_UNDERWATER			1	
#define	IO_FREEZESCRIPT			(1<<1)
#define IO_ITEM					(1<<2)
#define IO_NPC					(1<<3)
#define IO_FIX					(1<<4)
#define IO_NOSHADOW				(1<<5)
#define IO_CAMERA				(1<<6)
#define IO_MARKER				(1<<7)
#define IO_ICONIC				(1<<8)
#define IO_NO_COLLISIONS		(1<<9)
#define IO_GOLD					(1<<10)
#define IO_INVULNERABILITY		(1<<11)
#define IO_NO_PHYSICS_INTERPOL	(1<<12)
#define IO_HIT					(1<<13)
#define IO_PHYSICAL_OFF			(1<<14)
#define IO_MOVABLE				(1<<15)
#define IO_UNIQUE				(1<<16)
#define IO_SHOP					(1<<17)
#define IO_BLACKSMITH			(1<<18)
#define IO_NOSAVE				(1<<19)
#define IO_FORCEDRAW			(1<<20)
#define IO_FIELD				(1<<21)
#define IO_BUMP					(1<<22)
#define IO_ANGULAR				(1<<23)
#define IO_BODY_CHUNK			(1<<24)
#define IO_ZMAP					(1<<25)
#define IO_INVERTED				(1<<26)
#define IO_JUST_COLLIDE			(1<<27)
#define IO_FIERY				(1<<28)
#define IO_NO_NPC_COLLIDE		(1<<29)
#define IO_CAN_COMBINE			(1<<30)

#define WALKMODE	0
#define RUNMODE		1
#define NOMOVEMODE	2
#define SNEAKMODE	3

//-----------------------------------------------------------------------------
//	INTERACTIVE_OBJ Structs Start 
//-----------------------------------------------------------------------------
#define IO_EQUIPITEM_ELEMENT_STRENGTH			0
#define IO_EQUIPITEM_ELEMENT_DEXTERITY			1
#define IO_EQUIPITEM_ELEMENT_CONSTITUTION		2
#define IO_EQUIPITEM_ELEMENT_MIND				3
#define IO_EQUIPITEM_ELEMENT_Stealth			4
#define IO_EQUIPITEM_ELEMENT_Mecanism			5
#define IO_EQUIPITEM_ELEMENT_Intuition			6
#define IO_EQUIPITEM_ELEMENT_Etheral_Link		7
#define IO_EQUIPITEM_ELEMENT_Object_Knowledge	8
#define IO_EQUIPITEM_ELEMENT_Casting			9
#define IO_EQUIPITEM_ELEMENT_Projectile			10
#define IO_EQUIPITEM_ELEMENT_Close_Combat		11
#define IO_EQUIPITEM_ELEMENT_Defense			12
#define IO_EQUIPITEM_ELEMENT_Armor_Class		13
#define IO_EQUIPITEM_ELEMENT_Resist_Magic		14
#define IO_EQUIPITEM_ELEMENT_Resist_Poison		15
#define IO_EQUIPITEM_ELEMENT_Critical_Hit		16
#define IO_EQUIPITEM_ELEMENT_Damages			17
#define IO_EQUIPITEM_ELEMENT_Duration			18
#define IO_EQUIPITEM_ELEMENT_AimTime			19
#define IO_EQUIPITEM_ELEMENT_Identify_Value		20
#define IO_EQUIPITEM_ELEMENT_Life				21
#define IO_EQUIPITEM_ELEMENT_Mana				22
#define IO_EQUIPITEM_ELEMENT_MaxLife			23
#define IO_EQUIPITEM_ELEMENT_MaxMana			24
#define IO_EQUIPITEM_ELEMENT_SPECIAL_1			25	
#define IO_EQUIPITEM_ELEMENT_SPECIAL_2			26
#define IO_EQUIPITEM_ELEMENT_SPECIAL_3			27
#define IO_EQUIPITEM_ELEMENT_SPECIAL_4			28
#define IO_SPECIAL_ELEM_NONE		0	
#define IO_SPECIAL_ELEM_PARALYZE	1
#define IO_SPECIAL_ELEM_DRAIN_LIFE	2
#define IO_ELEMENT_FLAG_NONE		0
#define IO_ELEMENT_FLAG_PERCENT		1

// EditorFlags
#define EFLAG_NOTSAVED			1
#define EFLAG_SELECTED			2

// GameFlags 16 Bits !!!
#define GFLAG_INTERACTIVITY		1
#define GFLAG_ISINTREATZONE		(1<<1)
#define GFLAG_WASINTREATZONE	(1<<2)
#define GFLAG_NEEDINIT			(1<<3)
#define GFLAG_INTERACTIVITYHIDE	(1<<4)
#define GFLAG_DOOR				(1<<5)
#define GFLAG_INVISIBILITY		(1<<6)
#define GFLAG_NO_PHYS_IO_COL	(1<<7)
#define GFLAG_VIEW_BLOCKER		(1<<8)
#define GFLAG_PLATFORM			(1<<9)
#define GFLAG_ELEVATOR			(1<<10)
#define GFLAG_MEGAHIDE			(1<<11)
#define GFLAG_HIDEWEAPON		(1<<12)
#define GFLAG_NOGORE			(1<<13)
#define GFLAG_GOREEXPLODE		(1<<14)
#define GFLAG_NOCOMPUTATION		(1<<15)

#define MAX_EXTRA_ROTATE 4
#define EXTRA_ROTATE_REALISTIC 1
#define NPCFLAG_BACKSTAB	1
#define HALO_ACTIVE		1
#define HALO_NEGATIVE	2
#define HALO_DYNLIGHT	4

#define TRANSFORMED		2
#define ALPHABLEND		1
#define NOCULL			2
#define TEXTURE3		1 // poly texturé 3
#define FLAT3			2 // poly flat 3

//-----------------------------------------------------------------------------

extern long EERIEDrawnPolys;
extern float PULSS;
extern long EERIEInit;
extern long TRUECLIPPING;
extern EERIE_3D BBOXMIN,BBOXMAX;
extern EERIE_BACKGROUND * ACTIVEBKG;
extern EERIE_CAMERA * ACTIVECAM;
extern long USE_FAST_SCENES;

extern float Xratio;
extern float Yratio;
extern long INTERTRANSPOLYSPOS;

extern D3DTLVERTEX InterTransPol[MAX_INTERTRANSPOL][4];
extern EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
extern TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

//-----------------------------------------------------------------------------
float FirstPolyPosY(float x,float z);
void SetActiveCamera(EERIE_CAMERA* cam);
void SetNextAnim(INTERACTIVE_OBJ * io,ANIM_HANDLE * ea,long layer=0,long loop=0);
//	INTERACTIVE_OBJ Struct End 
//****************************************************************************

void AcquireLastAnim(INTERACTIVE_OBJ * io);
void FinishAnim(INTERACTIVE_OBJ * io,ANIM_HANDLE * eanim);
BOOL Visible(EERIE_3D * orgn, EERIE_3D * dest,EERIEPOLY * epp,EERIE_3D * hit);
void EERIEDrawTrue3DLine(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3D * orgn, EERIE_3D * dest, D3DCOLOR col);
void FaceTarget(INTERACTIVE_OBJ * io);

void DebugSphere(float x,float y,float z,float siz,long tim,D3DCOLOR color);

EERIEPOLY * CheckTopPoly(float x,float y,float z);
EERIEPOLY * CheckPolyOnTop(float x,float y,float z);
EERIEPOLY * CheckInPoly(float x,float y,float z,float * needY=NULL);
EERIEPOLY * EECheckInPoly(EERIE_3D * pos,float * needY=NULL);
EERIEPOLY * CheckInPolyIn(float x,float y,float z);
EERIEPOLY * CheckInPolyPrecis(float x,float y,float z,float * needY=NULL);

EERIEPOLY * EEIsUnderWater(EERIE_3D * pos);
BOOL GetTruePolyY(EERIEPOLY * ep,EERIE_3D * pos,float * ret);
EERIEPOLY * EEIsUnderWaterFast(EERIE_3D * pos);
BOOL IsVertexIdxInGroup(EERIE_3DOBJ * eobj,long idx,long grs);
 
D3DCOLOR GetColorz(float x,float y,float z);
int PointIn2DPolyXZ(EERIEPOLY * ep, float x, float z);

float Distance2D(float x0, float y0, float x1, float y1);
float Distance3D(float x0, float y0, float z0, float x1, float y1, float z1);
 
int EERIELaunchRay2(EERIE_3D * orgn, EERIE_3D * dest,  EERIE_3D * hit, EERIEPOLY * tp, long flag);
int EERIELaunchRay3(EERIE_3D * orgn, EERIE_3D * dest,  EERIE_3D * hit, EERIEPOLY * tp, long flag);
float GetGroundY(EERIE_3D * pos);
void EE_IRTP(D3DTLVERTEX *in,D3DTLVERTEX *out);
void EE_RTT(D3DTLVERTEX *in,D3DTLVERTEX *out);
void _EERIERTPPoly(EERIEPOLY *ep);
void extEE_RTP(D3DTLVERTEX *in,D3DTLVERTEX *out);
void MakeColorz(INTERACTIVE_OBJ * io);

void EE_RotateX(D3DTLVERTEX *in,D3DTLVERTEX *out,float c, float s);
void EE_RotateY(D3DTLVERTEX *in,D3DTLVERTEX *out,float c, float s);
void EE_RotateZ(D3DTLVERTEX *in,D3DTLVERTEX *out,float c, float s);
extern void EE_RTP(D3DTLVERTEX *in,D3DTLVERTEX *out);

void GetAnimTotalTranslate( ANIM_HANDLE * eanim,long alt_idx,EERIE_3D * pos);

long PhysicalDrawBkgVLine(EERIE_3D * orgn,EERIE_3D * dest);
long AnchorData_GetNearest(EERIE_3D *pos,EERIE_CYLINDER * cyl);

// FAST SAVE LOAD
BOOL FastSceneLoad(char * path);
BOOL FastSceneSave(char * path,EERIE_MULTI3DSCENE * ms);
BOOL CheckUniqueIdent(char * pathh);
BOOL CreateUniqueIdent(char * pathh);


//****************************************************************************
// DRAWING FUNCTIONS START

void DrawLinkedObj(			LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_LINKED * el,
							D3DCOLOR col,
							long typ );
void DrawEERIEObjEx(		LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_3DOBJ * eobj,
							EERIE_3D * angle,
							EERIE_3D  * pos,
							EERIE_3D * scale,
							EERIE_RGB * col
						);
void DrawEERIEObjExEx(LPDIRECT3DDEVICE7 pd3dDevice,
						EERIE_3DOBJ * eobj,
						EERIE_3D * angle,
						EERIE_3D  * pos,
						EERIE_3D * scale,
						int coll
						);
void DrawEERIEInter(		LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_3DOBJ * eobj,
							EERIE_3D * angle,
							EERIE_3D  * pos,
							INTERACTIVE_OBJ * io,
							EERIE_MOD_INFO * modinfo=NULL
						);
 
void DrawEERIEInterMatrix(	LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_3DOBJ * eobj,
							EERIEMATRIX * mat,
							EERIE_3D  * pos,
							INTERACTIVE_OBJ * io,
							EERIE_3D * angle,
							EERIE_MOD_INFO * modinfo=NULL
						);
void EERIEDrawAnimQuat(		LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_3DOBJ * eobj,
							ANIM_USE * eanim,
							EERIE_3D * angle,
							EERIE_3D  * pos,
							unsigned long time,
							INTERACTIVE_OBJ * io,
							D3DCOLOR col,
							long typ=0
						);
void EERIEDrawAnimQuatPos(	LPDIRECT3DDEVICE7 pd3dDevice,
							EERIE_3DOBJ * eobj,
							ANIM_USE * eanim,
							EERIE_3D * angle,
							EERIE_3D  * pos,
							INTERACTIVE_OBJ * io,
							D3DCOLOR col,
							long typ=0
						);
// DRAWING FUNCTIONS END
//****************************************************************************


//****************************************************************************
// BACKGROUND MANAGEMENT FUNCTIONS START
long BKG_CountPolys(EERIE_BACKGROUND * eb);
long BKG_CountChildPolys(EERIE_BACKGROUND * eb);
long BKG_CountIgnoredPolys(EERIE_BACKGROUND * eb);
void SceneAddObjToBackground(EERIE_3DOBJ * eobj);

void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms);
void ClearBackground(EERIE_BACKGROUND * eb);
int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv);
int BkgAddPoly(EERIEPOLY * ep);

void EERIEAddPolyToBackground(D3DTLVERTEX *vert,D3DTLVERTEX *vert2,TextureContainer * tex, long render,float transval);
void EERIEAddPoly(D3DTLVERTEX * vert, D3DTLVERTEX * vert2, TextureContainer * tex, long render, float transval);
// BACKGROUND MANAGEMENT FUNCTIONS END
//****************************************************************************


//****************************************************************************
// RENDERING FUNCTIONS START
void SETALPHABLEND(LPDIRECT3DDEVICE7 pd3dDevice,DWORD state);
void SETTEXTURE0(LPDIRECT3DDEVICE7 pd3dDevice,IDirectDrawSurface7 * tex);
void SETCULL(LPDIRECT3DDEVICE7 pd3dDevice,DWORD state);
// RENDERING FUNCTIONS END
//****************************************************************************


//****************************************************************************
// LIGHT FUNCTIONS START
void _RecalcLightZone(float x,float y,float z,long siz);
void RecalcLightZone(float x,float y,float z,long siz);
void EERIEPrecalcLights(long minx=0,long minz=0,long maxx=99999,long maxz=99999);
void EERIERemovePrecalcLights();
void PrecalcDynamicLighting(long x0,long x1,long z0,long z1);
void ApplyDynLight(EERIEPOLY *ep);
long GetFreeDynLight();
// LIGHT FUNCTIONS END
//****************************************************************************


//****************************************************************************
// CAMERA FUNCTIONS START
void SetTargetCamera(EERIE_CAMERA * cam,float x,float y, float z);
void PrepareCamera(EERIE_CAMERA *cam);
void SP_PrepareCamera(EERIE_CAMERA * cam);
void PrepareActiveCamera();
// CAMERA FUNCTIONS END
//****************************************************************************


void ApplyLight(EERIEPOLY *ep);
long MakeTopObjString(INTERACTIVE_OBJ * io, char * dest, unsigned int destSize);
void DeclareEGInfo(float x,float y,float z);
BOOL TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj);
void ApplyWaterFXToVertex(EERIE_3D * odtv,D3DTLVERTEX * dtv,float power);
int BackFaceCull2D(D3DTLVERTEX * tv);
void ResetAnim(ANIM_USE * eanim);

//*************************************************************************************
//*************************************************************************************

	
__forceinline long EERIERTPPoly(EERIEPOLY *ep)
{
	specialEE_RTP(&ep->v[0],&ep->tv[0]);
	specialEE_RTP(&ep->v[1],&ep->tv[1]);
	specialEE_RTP(&ep->v[2],&ep->tv[2]);	

	if (ep->type & POLY_QUAD) 
	{
		specialEE_RTP(&ep->v[3],&ep->tv[3]);	

		if ((ep->tv[0].sz<=0.f) &&
			(ep->tv[1].sz<=0.f) &&
			(ep->tv[2].sz<=0.f) &&
			(ep->tv[3].sz<=0.f) ) 
		{
			return 0;
		}
	}
	else
	{
		if ((ep->tv[0].sz<=0.f) &&
			(ep->tv[1].sz<=0.f) &&
			(ep->tv[2].sz<=0.f)  ) 
		{
			return 0;
		}
	}

	return 1;
}


void EE_RTP3(EERIE_3D * in, EERIE_3D * out, EERIE_CAMERA * cam);
		
void ReleaseAnimFromIO(INTERACTIVE_OBJ * io,long num);

void ShadowPolys_ClearZone(EERIE_BACKGROUND * eb,long x0, long y0, long x1, long y1);
void HALO_IO_DynLight_Update(INTERACTIVE_OBJ * io);
short ANIM_GetAltIdx(ANIM_HANDLE * ah,long old);
void ANIM_Set(ANIM_USE * au,ANIM_HANDLE * anim);
void WriteMSEData(char * path,EERIE_MULTI3DSCENE * ms);

BOOL LittleAngularDiff(EERIE_3D * norm,EERIE_3D * norm2);
void RecalcLight(EERIE_LIGHT * el);
void CreatePWorld(long x0,long x1,long z0,long z1);
void ComputeSworld();
float PtIn2DPolyProj(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
float PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);

void ResetWorlds();
float GetSWorld(float x,float y,float z);
  
void LaunchLightThread(long minx=0,long minz=0,long maxx=99999,long maxz=99999);

void EERIE_ANIMMANAGER_Init();
void EERIE_ANIMMANAGER_PurgeUnused();
void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim);
ANIM_HANDLE * EERIE_ANIMMANAGER_GetHandle(char * path);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load(char * path);
void BkgAddShadowPoly(EERIEPOLY * ep,EERIEPOLY * father);

EERIEPOLY * GetMinNextPoly(long i,long j,EERIEPOLY * ep);
 
long GetVertexPos(INTERACTIVE_OBJ * io,long id,EERIE_3D * pos);
void PrepareBackgroundNRMLs();
void DrawInWorld(LPDIRECT3DDEVICE7 pd3dDevice);
long CountBkgVertex();
void CreateInWorld();
void EERIE_LIGHT_ChangeLighting();
void SetCameraDepth(float depth);

extern void EERIETreatPoint(D3DTLVERTEX *in,D3DTLVERTEX *out);
extern void EERIETreatPoint2(D3DTLVERTEX *in,D3DTLVERTEX *out);
void _YRotatePoint(EERIE_3D *in,EERIE_3D *out,float c, float s);
void _XRotatePoint(EERIE_3D *in,EERIE_3D *out,float c, float s);
BOOL RayCollidingPoly(EERIE_3D * orgn,EERIE_3D * dest,EERIEPOLY * ep,EERIE_3D * hit);

void EERIEPOLY_Compute_PolyIn();
void F_PrepareCamera(EERIE_CAMERA * cam);
bool CylinderAboveInvalidZone(EERIE_CYLINDER * cyl);

float GetTileMinY(long i,long j);
float GetTileMaxY(long i,long j);

#define SHOW_FLAG_NOT_DRAWN				0
#define SHOW_FLAG_IN_SCENE				1
#define SHOW_FLAG_LINKED				2
#define SHOW_FLAG_IN_INVENTORY			4	// In Inventory
#define SHOW_FLAG_HIDDEN				5	// In Inventory
#define SHOW_FLAG_TELEPORTING			6	
#define SHOW_FLAG_KILLED				7	// Not Used Yet
#define SHOW_FLAG_MEGAHIDE				8
#define SHOW_FLAG_ON_PLAYER				9
#define SHOW_FLAG_DESTROYED				255

#define PORTAL_FLAG_DRAWN	1

#define MAX_FRUSTRUMS	32

typedef struct
{
	float	a;
	float	b;
	float	c;
	float	d; // dist to origin
} EERIE_FRUSTRUM_PLANE;

typedef struct
{
	EERIE_FRUSTRUM_PLANE plane[4];
	long nb; 
} EERIE_FRUSTRUM;

typedef struct
{
	long nb_frustrums;
	EERIE_FRUSTRUM frustrums[MAX_FRUSTRUMS];
} EERIE_FRUSTRUM_DATA;

typedef struct
{
	short			count;
	short			flags;
	EERIE_2D_BBOX	bbox;	
	EERIE_FRUSTRUM_DATA	frustrum;
} PORTAL_ROOM_DRAW;
extern PORTAL_ROOM_DRAW * RoomDraw;
extern long NbRoomDraw;

// Default Mode for Portals when found
#define DEFAULT_PORTAL_MODE 3

#define NPC_ITEMS__AMBIENT_VALUE_255	35
#define NPC_ITEMS__AMBIENT_VALUE		0.1372549f
 
typedef struct
{
	float	distance; // -1 means use truedist
	EERIE_3D startpos;
	EERIE_3D endpos;
} ROOM_DIST_DATA_SAVE;

typedef struct
{
	float	distance; // -1 means use truedist
	EERIE_3D startpos;
	EERIE_3D endpos;
} ROOM_DIST_DATA;

extern ROOM_DIST_DATA * RoomDistance;
extern long NbRoomDistance;

void UpdateIORoom(INTERACTIVE_OBJ * io);
float SP_GetRoomDist(EERIE_3D * pos,EERIE_3D * c_pos,long io_room,long Cam_Room);
float CEDRIC_PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
void EERIE_PORTAL_ReleaseOnlyVertexBuffer();
void ComputePortalVertexBuffer();
float GetRoomDistance(long i,long j,EERIE_3D * p1,EERIE_3D * p2);
BOOL GetNameInfo(char * name1,long * type,long * val1,long * val2);

typedef struct
{
	short			num;
	short			max;
	EERIE_LIGHT **	el;	
} TILE_LIGHTS;

#endif
