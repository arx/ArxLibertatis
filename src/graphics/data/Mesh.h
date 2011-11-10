/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_DATA_MESH_H
#define ARX_GRAPHICS_DATA_MESH_H

#include <set>

#include "graphics/GraphicsTypes.h"
// TODO move INTERCATIVE_OBJ somewhere else / move flags here
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/Spells.h"
#include "math/Rectangle.h"
#include "platform/Flags.h"
// TODO Remove when this header is cleaned up
#include "script/Script.h"

struct ARX_PATH;
struct SYMBOL_DRAW;
struct INVENTORY_DATA;
struct ARX_USE_PATH;
struct INTERACTIVE_OBJ;

void specialEE_RTP(TexturedVertex*,TexturedVertex*);
void EERIE_CreateMatriceProj(float _fWidth,float _fHeight,float _fFOV,float _fZNear,float _fZFar);


struct ANIM_HANDLE {
	
	ANIM_HANDLE();
	
	fs::path path; // empty path means an unallocated slot
	
	EERIE_ANIM ** anims;
	short alt_nb;
	
	long locks;
	
};

struct EERIE_TRANSFORM
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
};

struct EERIE_CAMERA {
	
	EERIE_TRANSFORM transform;
	Vec3f pos; // 0 4 8
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
	
	float xmod;
	float ymod;
	EERIEMATRIX matrix;
	Anglef angle;
	
	Vec3f d_pos;
	Anglef d_angle;
	Vec3f lasttarget;
	Vec3f lastpos;
	Vec3f translatetarget;
	bool lastinfovalid;
	Vec3f norm;
	Color3f fadecolor;
	Rect clip;
	float clipz0;
	float clipz1;
	long centerx;
	long centery;
	
	float smoothing;
	float AddX;
	float AddY;
	long Xsnap;
	long Zsnap;
	float Zdiv;
	
	long clip3D;
	long type;
	Color bkgcolor; // TODO was BGR!
	long nbdrawn;
	float cdepth;
	
	Anglef size;
	
};

#define ANCHOR_FLAG_GREEN_DRAW	1
#define ANCHOR_FLAG_BLOCKED		8

struct _ANCHOR_DATA
{
	Vec3f	pos;
	short		nblinked;
	short		flags;
	long	*	linked;
	float		radius;
	float		height;
};

struct EERIE_BKG_INFO
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
};

struct EERIE_SMINMAX
{
	short min;
	short max;
};

#define FBD_TREAT		1
#define FBD_NOTHING		2

struct FAST_BKG_DATA
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
};
#define MAX_BKGX	160
#define MAX_BKGZ	160
#define BKG_SIZX	100
#define BKG_SIZZ	100


struct EERIE_BACKGROUND
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
	Color3f ambient;
	Color3f ambient255;
	EERIE_SMINMAX *	minmax;
	long		  nbanchors;
	_ANCHOR_DATA * anchors;
	char		name[256];
};

struct IO_EQUIPITEM_ELEMENT {
	float value;
	short flags;
	short special; // TODO unused?
};

#define IO_EQUIPITEM_ELEMENT_Number 29

struct IO_EQUIPITEM {
	IO_EQUIPITEM_ELEMENT elements[IO_EQUIPITEM_ELEMENT_Number];
};

struct ANIM_USE
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
};

enum BehaviourFlag {
	BEHAVIOUR_NONE          = (1<<0), // no pathfind
	BEHAVIOUR_FRIENDLY      = (1<<1), // no pathfind
	BEHAVIOUR_MOVE_TO       = (1<<2),
	BEHAVIOUR_WANDER_AROUND = (1<<3), //behavior_param = distance
	BEHAVIOUR_FLEE          = (1<<4), //behavior_param = distance
	BEHAVIOUR_HIDE          = (1<<5), //behavior_param = distance
	BEHAVIOUR_LOOK_FOR      = (1<<6), //behavior_param = distance
	BEHAVIOUR_SNEAK         = (1<<7),
	BEHAVIOUR_FIGHT         = (1<<8),
	BEHAVIOUR_DISTANT       = (1<<9),
	BEHAVIOUR_MAGIC         = (1<<10),
	BEHAVIOUR_GUARD         = (1<<11),
	BEHAVIOUR_GO_HOME       = (1<<12),
	BEHAVIOUR_LOOK_AROUND   = (1<<13),
	BEHAVIOUR_STARE_AT      = (1<<14)
};
DECLARE_FLAGS(BehaviourFlag, Behaviour)
DECLARE_FLAGS_OPERATORS(Behaviour)

enum MoveMode {
	WALKMODE = 0,
	RUNMODE = 1,
	NOMOVEMODE = 2,
	SNEAKMODE = 3
};

#define MAX_ANIM_LAYERS	4
struct IO_BEHAVIOR_DATA {
	long			exist;
	Behaviour behavior;
	float			behavior_param;
	long			tactics;		// 0=none ; 1=side ; 2=side+back
	long			target;
	MoveMode movemode;
	ANIM_USE		animlayer[MAX_ANIM_LAYERS];
};


struct IO_SPELLCAST_DATA {
	Spell castingspell; // spell being casted...
	Rune symb[4]; // symbols to draw before casting...
	SpellcastFlags spell_flags;
	short spell_level;
	long target;
	long duration;
};

enum PathfindFlag {
	PATHFIND_ALWAYS    = (1<<0),
	PATHFIND_ONCE      = (1<<1),
	PATHFIND_NO_UPDATE = (1<<2)
};
DECLARE_FLAGS(PathfindFlag, PathfindFlags)
DECLARE_FLAGS_OPERATORS(PathfindFlags)

struct IO_PATHFIND {
	PathfindFlags flags;
	long	listnb;
	unsigned short * list;
	unsigned short listpos;
	short pathwait;
	long	truetarget;
};

#define MAX_EXTRA_ROTATE 4

struct EERIE_EXTRA_ROTATE {
	long flags;
	short group_number[MAX_EXTRA_ROTATE];
	Anglef group_rotate[MAX_EXTRA_ROTATE];
};

#define MAX_STACKED_BEHAVIOR 5

struct IO_NPCDATA {
	
	IO_NPCDATA();
	~IO_NPCDATA();
	
	float maxlife;
	float life;
	float maxmana;
	float mana;
	unsigned long reachedtime;
	long reachedtarget;	//Is target in REACHZONE ?
	INTERACTIVE_OBJ * weapon; // Linked Weapon (r-hand)
	long detect;
	MoveMode movemode;
	float armor_class;
	float absorb;
	float damages;
	float tohit;
	float aimtime;
	float critical;
	float reach;
	float backstab_skill;
	
	Behaviour behavior;
	float behavior_param;
	long tactics; // 0=none ; 1=side ; 2=side+back
	long xpvalue;
	long cut;
	
	float moveproblem;
	ItemType weapontype;
	long weaponinhand;
	long fightdecision;
	
	float look_around_inc;
	unsigned long collid_time;
	long collid_state;
	float speakpitch;
	float lastmouth;
	long ltemp;
	
	IO_BEHAVIOR_DATA stacked[MAX_STACKED_BEHAVIOR];
	float poisonned;
	unsigned char resist_poison;
	unsigned char resist_magic;
	unsigned char resist_fire;
	
	short strike_time;
	short walk_start_time;
	long aiming_start;
	long npcflags;
	IO_PATHFIND pathfind;
	EERIE_EXTRA_ROTATE * ex_rotate;
	Color blood_color;
	
	short SPLAT_DAMAGES;
	short SPLAT_TOT_NB;
	Vec3f last_splat_pos;
	float vvpos;
	
	float climb_count;
	float stare_factor;
	float fDetect;
	short cuts;
	short unused;
	
};

struct IO_ITEMDATA {
	IO_EQUIPITEM * equipitem; // Equipitem Datas
	long price;
	short maxcount; // max number cumulable
	short count; // current number
	char food_value;
	char stealvalue;
	short playerstacksize;
	short LightValue;
};

struct IO_FIXDATA {
	char trapvalue;
	char padd[3];
};


struct IO_CAMDATA {
	EERIE_CAMERA cam;
};

struct IO_HALO {
	Color3f color;
	float radius;
	unsigned long flags;
	long dynlight;
	Vec3f offset;
};

struct IO_PHYSICS
{
	EERIE_CYLINDER	cyl;
	Vec3f		startpos;
	Vec3f		targetpos;
	Vec3f		velocity;
	Vec3f		forces;
};

struct IO_TWEAKER_INFO {
	fs::path filename;
	std::string skintochange;
	fs::path skinchangeto;
};

struct TWEAK_INFO;

#define IO_NPC_AFLAG_HIT_BACKGROUND			1
#define IO_NPC_AFLAG_HIT_CLEAR				(1)

#define SFX_TYPE_YLSIDE_DEATH	1
#define SFX_TYPE_INCINERATE		2

#define MAX_ANIMS 200 // max loadable anims per character

// ARX_COLLISIONS flags (cylinder move)
enum CollisionFlag {
	CFLAG_LEVITATE          = (1<<0),
	CFLAG_NO_INTERCOL       = (1<<1),
	CFLAG_SPECIAL           = (1<<2),
	CFLAG_EASY_SLIDING      = (1<<3),
	CFLAG_CLIMBING          = (1<<4),
	CFLAG_JUST_TEST         = (1<<5),
	CFLAG_NPC               = (1<<6),
	CFLAG_PLAYER            = (1<<7),
	CFLAG_RETURN_HEIGHT     = (1<<8),
	CFLAG_EXTRA_PRECISION   = (1<<9),
	CFLAG_CHECK_VALID_POS   = (1<<10),
	CFLAG_ANCHOR_GENERATION = (1<<11),
	CFLAG_COLLIDE_NOCOL     = (1<<12),
	CFLAG_NO_NPC_COLLIDE    = (1<<13),
	CFLAG_NO_HEIGHT_MOD     = (1<<14)
};
DECLARE_FLAGS(CollisionFlag, CollisionFlags)
DECLARE_FLAGS_OPERATORS(CollisionFlags)

enum IOCollisionFlag {
	COLLIDE_WITH_PLAYER = (1<<0),
	COLLIDE_WITH_WORLD  = (1<<1)
};
DECLARE_FLAGS(IOCollisionFlag, IOCollisionFlags)
DECLARE_FLAGS_OPERATORS(IOCollisionFlags)

struct INTERACTIVE_OBJ {
	
	explicit INTERACTIVE_OBJ(long num);
	~INTERACTIVE_OBJ();
	
	long num; // Nuky - 25/01/11 - cache the InterNum to speed up GetInterNum()
	
	long ioflags; // IO type
	Vec3f lastpos; // IO last position
	Vec3f pos; // IO position
	Vec3f move;
	Vec3f lastmove;
	Vec3f forcedmove;
	
	Anglef angle; // IO angle
	IO_PHYSICS physics;	// Movement Collision Data
	short room;
	short room_flags; // 1==need_update
	float original_height;
	float original_radius;
	TextureContainer * inv; // Object Icon
	EERIE_3DOBJ * obj; // IO Mesh data
	ANIM_HANDLE * anims[MAX_ANIMS];	// Object Animations
	ANIM_USE animlayer[MAX_ANIM_LAYERS];
	Vec3f * lastanimvertex; // Last Animation Positions of Vertex
	long nb_lastanimvertex;
	unsigned long lastanimtime;
	
	EERIE_3D_BBOX bbox3D;
	Vec2s bbox1; // 2D bounding box1
	Vec2s bbox2; // 2D bounding box2
	fs::path usemesh; // Alternate Mesh/path
	EERIE_3DOBJ * tweaky; // tweaked original obj backup
	audio::SourceId sound;
	ItemType type_flags; // object type (weapon,goblin...)
	long scriptload; // Is This object Loaded by Script ?
	Vec3f target; // Target position
	long targetinfo; // Target Type/Ident
	
	union {
		IO_ITEMDATA * _itemdata; // ITEM Datas
		IO_FIXDATA * _fixdata; // FIX Datas
		IO_NPCDATA * _npcdata; // NPC Datas
		IO_CAMDATA * _camdata; // Camera Datas
	};
	
	INVENTORY_DATA * inventory; // Inventory Data
	short show; // Show Status (In Scene, In Inventory...)
	IOCollisionFlags collision; // collision type
	std::string mainevent;
	Color3f infracolor; // Improve Vision Color (Heat)
	long changeanim;
	
	long ident; // Ident num
	float weight;
	std::string locname; //localisation
	unsigned short EditorFlags; // 1 NOTSAVED 2 selected
	unsigned short GameFlags; // GFLAGS
	Vec3f velocity; // velocity
	float fall;

	long stopped;
	Vec3f initpos; // Initial Position
	Anglef initangle; // Initial Angle
	fs::path filename;
	float scale;
	
	ARX_USE_PATH * usepath;
	SYMBOL_DRAW * symboldraw;
	short dynlight;
	short lastspeechflag;
	ARX_PATH * inzone;
	IO_HALO halo;
	IO_HALO halo_native;
	
	EERIE_SCRIPT script; // Primary Script
	EERIE_SCRIPT over_script; // Overriding Script
	short stat_count;
	short stat_sent;
	IO_TWEAKER_INFO * tweakerinfo; // optional tweaker infos
	Material material;
	
	std::set<std::string> groups;
	char sizex; // Inventory Icon sizeX
	char sizey; // Inventory Icon sizeY
	unsigned long soundtime;
	unsigned long soundcount;
	
	short level;
	short truelevel;
	unsigned long sfx_time;
	unsigned long collide_door_time;
	unsigned long ouch_time;
	float dmg_sum;
	
	IO_SPELLCAST_DATA spellcast_data;
	short flarecount;
	short no_collide;
	float invisibility;
	float frameloss;
	float basespeed;
	
	float speed_modif;
	long * spells_on;
	short nb_spells_on;
	long damagedata;
	
	float rubber;
	float max_durability;
	float durability;
	short poisonous;
	short poisonous_count;
	
	float ignition;
	long ignit_light;
	audio::SampleId ignit_sound;
	float head_rot;
	
	short damager_damages;
	DamageType damager_type;
	std::string stepmaterial;
	std::string armormaterial;
	std::string weaponmaterial;
	std::string strikespeech;
	
	short sfx_flag;
	std::vector<TWEAK_INFO> tweaks;
	char secretvalue;
	
	std::string shop_category;
	float shop_multiply;
	fs::path inventory_skin;
	long aflags; // additionnal flags
	short inzone_show;
	short summoner;
	long spark_n_blood;
	
	/*!
	 * Return the short name for this Object where only the name
	 * of the file is returned
	 * @return The name of the file at the end of the filename path
	 */
	std::string short_name() const;
	
	/*!
	 *  Returns the long name for this Object where the short name
	 * is combined with the identifying number
	 * in the form of "%s_%04ld"
	 * @return The short name combined with a 4 digit ident, padded with 0
	 */
	std::string long_name() const;
	
	/*!
	 *  Returns the full name for this Object where the
	 * directory portion of the filename member is combined
	 * with the the result of long_name()
	 * @return The directory of filename + long_name()
	 */
	fs::path full_name() const;
	
};

#define MAX_EQUIPED 12
#define MORE_COMPATIBILITY

#define MAX_TRANSPOL 512
#define MAX_INTERTRANSPOL 512

#define CAM_SUBJVIEW 0
#define CAM_TOPVIEW 1

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
#define TEXTURE3		1 // poly texture 3
#define FLAT3			2 // poly flat 3

//-----------------------------------------------------------------------------

extern long EERIEDrawnPolys;
extern long EERIEInit;
extern Vec3f BBOXMIN,BBOXMAX;
extern EERIE_BACKGROUND * ACTIVEBKG;
extern EERIE_CAMERA * ACTIVECAM;
extern long USE_FAST_SCENES;

extern float Xratio;
extern float Yratio;
extern long INTERTRANSPOLYSPOS;

extern TexturedVertex InterTransPol[MAX_INTERTRANSPOL][4];
extern EERIE_FACE * InterTransFace[MAX_INTERTRANSPOL];
extern TextureContainer * InterTransTC[MAX_INTERTRANSPOL];

//-----------------------------------------------------------------------------
float FirstPolyPosY(float x,float z);
void SetActiveCamera(EERIE_CAMERA* cam);
//	INTERACTIVE_OBJ Struct End
//****************************************************************************

void AcquireLastAnim(INTERACTIVE_OBJ * io);
void FinishAnim(INTERACTIVE_OBJ * io,ANIM_HANDLE * eanim);
bool Visible(Vec3f * orgn, Vec3f * dest,EERIEPOLY * epp,Vec3f * hit);
void FaceTarget(INTERACTIVE_OBJ * io);

void DebugSphere(float x, float y, float z, float siz, long tim, Color color);

EERIEPOLY * CheckTopPoly(float x,float y,float z);
EERIEPOLY * CheckPolyOnTop(float x,float y,float z);
EERIEPOLY * CheckInPoly(float x,float y,float z,float * needY = NULL);
EERIEPOLY * EECheckInPoly(const Vec3f * pos,float * needY = NULL);
EERIEPOLY * CheckInPolyIn(float x,float y,float z);
EERIEPOLY * CheckInPolyPrecis(float x,float y,float z,float * needY = NULL);

/**
 * Check if the given condition is under water.
 * 
 * @return The lowest water polygon pos is under, or NULL if pos is not under water.
 **/
EERIEPOLY * EEIsUnderWater(const Vec3f * pos);

/**
 * Check if the given condition is under water.
 * 
 * @return Any water polygon pos is under, or NULL if pos is not under water.
 **/
EERIEPOLY * EEIsUnderWaterFast(const Vec3f * pos);

bool GetTruePolyY(const EERIEPOLY * ep, const Vec3f * pos,float * ret);
bool IsAnyPolyThere(float x, float z);
bool IsVertexIdxInGroup(EERIE_3DOBJ * eobj,long idx,long grs);
EERIEPOLY * GetMinPoly(float x, float y, float z);
EERIEPOLY * GetMaxPoly(float x, float y, float z);
 
float GetColorz(float x, float y, float z);
int PointIn2DPolyXZ(const EERIEPOLY * ep, float x, float z);

int EERIELaunchRay2(Vec3f * orgn, Vec3f * dest,  Vec3f * hit, EERIEPOLY * tp, long flag);
int EERIELaunchRay3(Vec3f * orgn, Vec3f * dest,  Vec3f * hit, EERIEPOLY * tp, long flag);
float GetGroundY(Vec3f * pos);
void EE_IRTP(TexturedVertex *in,TexturedVertex *out);
void EE_RTT(TexturedVertex *in,TexturedVertex *out);

void extEE_RTP(TexturedVertex *in,TexturedVertex *out);
void MakeColorz(INTERACTIVE_OBJ * io);

void EE_RotateX(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RotateY(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RotateZ(TexturedVertex *in,TexturedVertex *out,float c, float s);
void EE_RTP(TexturedVertex *in,TexturedVertex *out);

void GetAnimTotalTranslate( ANIM_HANDLE * eanim,long alt_idx,Vec3f * pos);

long PhysicalDrawBkgVLine(Vec3f * orgn,Vec3f * dest);

// FAST SAVE LOAD
bool FastSceneLoad(const fs::path & path);

//****************************************************************************
// DRAWING FUNCTIONS START

void DrawEERIEObjEx(EERIE_3DOBJ * eobj, Anglef * angle, Vec3f * pos, Vec3f * scale, Color3f * col);
void DrawEERIEObjExEx(EERIE_3DOBJ * eobj, Anglef * angle, Vec3f * pos, Vec3f * scale, int coll);
// DRAWING FUNCTIONS END
//****************************************************************************


//****************************************************************************
// BACKGROUND MANAGEMENT FUNCTIONS START
long BKG_CountPolys(EERIE_BACKGROUND * eb);
long BKG_CountChildPolys(EERIE_BACKGROUND * eb);
long BKG_CountIgnoredPolys(EERIE_BACKGROUND * eb);

#ifdef BUILD_EDIT_LOADSAVE
void SceneAddMultiScnToBackground(EERIE_MULTI3DSCENE * ms);
#endif

void ClearBackground(EERIE_BACKGROUND * eb);
int InitBkg(EERIE_BACKGROUND * eb, short sx, short sz, short Xdiv, short Zdiv);

void EERIEAddPoly(TexturedVertex * vert, TexturedVertex * vert2, TextureContainer * tex, long render, float transval);
// BACKGROUND MANAGEMENT FUNCTIONS END
//****************************************************************************


//****************************************************************************
// LIGHT FUNCTIONS START
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

//****************************************************************************
// BBOX FUNCTIONS START
__inline void ResetBBox3D(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		io->bbox3D.min.x=99999999.f;
		io->bbox3D.min.y=99999999.f;
		io->bbox3D.min.z=99999999.f;
		io->bbox3D.max.x=-99999999.f;
		io->bbox3D.max.y=-99999999.f;
		io->bbox3D.max.z=-99999999.f;
	}
}

__inline void AddToBBox3D(INTERACTIVE_OBJ * io,Vec3f * pos)
{
	if (io)
	{
		io->bbox3D.min.x=std::min(io->bbox3D.min.x,pos->x);
		io->bbox3D.min.y=std::min(io->bbox3D.min.y,pos->y);
		io->bbox3D.min.z=std::min(io->bbox3D.min.z,pos->z);
		io->bbox3D.max.x=std::max(io->bbox3D.max.x,pos->x);
		io->bbox3D.max.y=std::max(io->bbox3D.max.y,pos->y);
		io->bbox3D.max.z=std::max(io->bbox3D.max.z,pos->z);
	}
}
// BBOX FUNCTIONS END
//****************************************************************************

void ApplyLight(EERIEPOLY *ep);
long MakeTopObjString(INTERACTIVE_OBJ * io, std::string& dest);
void DeclareEGInfo(float x, float z);
bool TryToQuadify(EERIEPOLY * ep,EERIE_3DOBJ * eobj);
void ApplyWaterFXToVertex(Vec3f * odtv,TexturedVertex * dtv,float power);
int BackFaceCull2D(TexturedVertex * tv);
void ResetAnim(ANIM_USE * eanim);

//*************************************************************************************
//*************************************************************************************

long EERIERTPPoly(EERIEPOLY *ep);

void EE_RTP3(Vec3f * in, Vec3f * out, EERIE_CAMERA * cam);

void ReleaseAnimFromIO(INTERACTIVE_OBJ * io,long num);

void ShadowPolys_ClearZone(EERIE_BACKGROUND * eb,long x0, long y0, long x1, long y1);
short ANIM_GetAltIdx(ANIM_HANDLE * ah,long old);
void ANIM_Set(ANIM_USE * au,ANIM_HANDLE * anim);

bool LittleAngularDiff(Vec3f * norm,Vec3f * norm2);
void RecalcLight(EERIE_LIGHT * el);
void CreatePWorld(long x0,long x1,long z0,long z1);
void ComputeSworld();
float PtIn2DPolyProj(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
float PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);

void ResetWorlds();
float GetSWorld(float x,float y,float z);

void EERIE_ANIMMANAGER_PurgeUnused();
void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load(const fs::path & path);
ANIM_HANDLE * EERIE_ANIMMANAGER_Load_NoWarning(const fs::path & path);
void BkgAddShadowPoly(EERIEPOLY * ep,EERIEPOLY * father);

EERIEPOLY * GetMinNextPoly(long i,long j,EERIEPOLY * ep);

long GetVertexPos(INTERACTIVE_OBJ * io,long id,Vec3f * pos);
void PrepareBackgroundNRMLs();
void DrawInWorld();
long CountBkgVertex();
void CreateInWorld();
void EERIE_LIGHT_ChangeLighting();
void SetCameraDepth(float depth);

extern void EERIETreatPoint(TexturedVertex *in,TexturedVertex *out);
extern void EERIETreatPoint2(TexturedVertex *in,TexturedVertex *out);
void _YRotatePoint(Vec3f *in,Vec3f *out,float c, float s);
void _XRotatePoint(Vec3f *in,Vec3f *out,float c, float s);
bool RayCollidingPoly(Vec3f * orgn,Vec3f * dest,EERIEPOLY * ep,Vec3f * hit);

void EERIEPOLY_Compute_PolyIn();
void F_PrepareCamera(EERIE_CAMERA * cam);

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

struct EERIE_FRUSTRUM_PLANE
{
	float	a;
	float	b;
	float	c;
	float	d; // dist to origin
};

struct EERIE_FRUSTRUM
{
	EERIE_FRUSTRUM_PLANE plane[4];
	long nb;
};

struct EERIE_FRUSTRUM_DATA
{
	long nb_frustrums;
	EERIE_FRUSTRUM frustrums[MAX_FRUSTRUMS];
};

struct PORTAL_ROOM_DRAW
{
	short			count;
	short			flags;
	EERIE_2D_BBOX	bbox;
	EERIE_FRUSTRUM_DATA	frustrum;
};
extern PORTAL_ROOM_DRAW * RoomDraw;
extern long NbRoomDraw;

// Default Mode for Portals when found
#define DEFAULT_PORTAL_MODE 3

#define NPC_ITEMS__AMBIENT_VALUE_255	35
#define NPC_ITEMS__AMBIENT_VALUE		0.1372549f

struct ROOM_DIST_DATA
{
	float	distance; // -1 means use truedist
	Vec3f startpos;
	Vec3f endpos;
};

extern ROOM_DIST_DATA * RoomDistance;
extern long NbRoomDistance;

void UpdateIORoom(INTERACTIVE_OBJ * io);
float SP_GetRoomDist(Vec3f * pos,Vec3f * c_pos,long io_room,long Cam_Room);
float CEDRIC_PtIn2DPolyProjV2(EERIE_3DOBJ * obj,EERIE_FACE * ef, float x, float z);
void EERIE_PORTAL_ReleaseOnlyVertexBuffer();
void ComputePortalVertexBuffer();
bool GetNameInfo( const std::string& name1,long& type,long& val1,long& val2);

struct TILE_LIGHTS
{
	short			num;
	short			max;
	EERIE_LIGHT **	el;
};

#endif // ARX_GRAPHICS_DATA_MESH_H
