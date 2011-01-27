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
//////////////////////////////////////////////////////////////////////////////////////
// ARX_ChangeLevel
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Change Level
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_CHANGELEVEL_H
#define ARX_CHANGELEVEL_H

#include "HERMES_ClusterSave.h"
#include "EERIETypes.h"
#include "EERIEPoly.h"
#include "ARX_Levels.h"
#include "ARX_Minimap.h"
#include "ARX_interactive.h"
#include "ARX_Spells.h"
#include "ARX_GlobalMods.h"

//-----------------------------------------------------------------------------
#define SIZE_ID	64
#define MAX_LINKED_SAVE	16
#define SAVEFLAGS_EXTRA_ROTATE 1

typedef struct
{
	long	type;
	float	fval;
	char 	name[SIZE_ID];
} ARX_VARIABLE_SAVE; // Aligned 1 2 4

typedef struct
{
	long			nblvar;
	unsigned long	lastcall;
	long			allowevents;
} ARX_SCRIPT_SAVE; // Aligned 1 2 4


typedef struct
{
	float	version;
	unsigned long time;
	long	nb_inter;
	long	nb_paths;
	long	nb_lights;
	long	ambiances_data_size;
	GLOBAL_MODS	gmods_stacked;
	GLOBAL_MODS	gmods_current;
	GLOBAL_MODS	gmods_desired;
	long	padd[256];

} ARX_CHANGELEVEL_INDEX;
typedef struct
{
	short	status;
	short	padd;
	float	lpadd;
} ARX_CHANGELEVEL_LIGHT;

typedef struct
{
	char	filename[256];
	long	ident;
	long	num;
	short	level;
	short	truelevel;
	long	unused; 
	long	padd[256];// new...
} ARX_CHANGELEVEL_IO_INDEX;

typedef struct
{
	char	name[64];
	char	controled[64];
	long	padd[64];
} ARX_CHANGELEVEL_PATH;

typedef struct
{
	float		version;
	long		nb_globals;
	long		padd[256];
} ARX_CHANGELEVEL_SAVE_GLOBALS;

typedef struct
{
	float	x;
	float	y;
	long	lvl;
	char	string[64];
} ARX_CHANGELEVEL_MAPMARKER_DATA;

typedef struct
{
	float	version;
	long			Current_Movement;
	long			Last_Movement;
	long			misc_flags;
	// Player Values
	float			Attribute_Strength;
	float			Attribute_Dexterity;
	float			Attribute_Constitution;
	float			Attribute_Mind;

	float			Skill_Stealth;
	float			Skill_Mecanism;
	float			Skill_Intuition;

	float			Skill_Etheral_Link;
	float			Skill_Object_Knowledge;
	float			Skill_Casting;

	float			Skill_Projectile;
	float			Skill_Close_Combat;
	float			Skill_Defense;

	float			Critical_Hit;
	long			AimTime;
	float	life;
	float	maxlife;
	float	mana;
	float	maxmana;
	long	level;
	short	Attribute_Redistribute;
	short	Skill_Redistribute;


	float			armor_class;
	float			resist_magic;
	float			resist_poison;
	long			xp;
	long			skin;
	unsigned long	rune_flags;
	float			damages;
	float			poison;
	float			hunger;
	EERIE_3D	pos;
	EERIE_3D	angle;
	EERIE_3D	size;

	char		inzone[SIZE_ID];
	char		rightIO[SIZE_ID];
	char		leftIO[SIZE_ID];
	char		equipsecondaryIO[SIZE_ID];
	char		equipshieldIO[SIZE_ID];
	char		curtorch[SIZE_ID];
	long		gold;
	long		falling;

	short	doingmagic;
	short	Interface;
	float	invisibility;
	ANIM_USE  useanim;
	IO_PHYSICS		physics;
	// Jump Sub-data
	unsigned long	jumpstarttime;
	long			jumpphase;	// 0 no jump, 1 doing anticipation anim

	char			id_inventory[3][INVENTORY_X][INVENTORY_Y][SIZE_ID];
	long			inventory_show[3][INVENTORY_X][INVENTORY_Y];
	MINI_MAP_DATA	minimap[MAX_MINIMAPS];
	char			equiped[MAX_EQUIPED][SIZE_ID];
	long			nb_PlayerQuest;
	char			anims[MAX_ANIMS][256];
	long			keyring_nb;
	long			playerflags;
	char			TELEPORT_TO_LEVEL[64];
	char			TELEPORT_TO_POSITION[64];
	long			TELEPORT_TO_ANGLE;
	long			CHANGE_LEVEL_ICON;
	short			bag;
	short			sp_flags;//padding;
	PRECAST_STRUCT  precast[MAX_PRECAST];
	long			Global_Magic_Mode;
	long			Nb_Mapmarkers;
	EERIE_3D		LAST_VALID_POS;
	long	padd[253];// new...
} ARX_CHANGELEVEL_PLAYER;

typedef struct
{
	char io[SIZE_ID];
	long sizex;
	long sizey;
	char slot_io[20][20][SIZE_ID];
	long slot_show[20][20];
	char initio[20][20][SIZE_ID];
	/// limit...
	char weapon[SIZE_ID];
	char targetinfo[SIZE_ID];
	char linked_id[MAX_LINKED_SAVE][SIZE_ID];
	char stackedtarget[MAX_STACKED_BEHAVIOR][SIZE_ID];
} ARX_CHANGELEVEL_INVENTORY_DATA_SAVE;

typedef struct
{
	char	name[SIZE_ID];
	long	times;
	long	msecs;
	long	pos;
	long	tim;
	long	script; // 0 = global ** 1 = local
	long	longinfo;
	long	flags;
} ARX_CHANGELEVEL_TIMERS_SAVE;

//	each IO needs an ARX_IO_SAVE struct
//  followed by 0...n ARX_VARIABLE_SAVE structs + text
//  followed by 0...n ARX_TIMERS_SAVE structs
//  followed by an ARX_[type]_IO_SAVE struct
//
typedef struct
{
	long			nblvar;
	unsigned long	lastcall;
	long			allowevents;
} ARX_CHANGELEVEL_SCRIPT_SAVE;

typedef struct
{
	long	type;
	float	fval;
	char 	name[SIZE_ID];
} ARX_CHANGELEVEL_VARIABLE_SAVE;


typedef struct
{
	long			lgroup; //linked to group n° if lgroup=-1 NOLINK
	long			lidx;
	long			lidx2;
	EERIE_MOD_INFO	modinfo;
	char 			linked_id[SIZE_ID];
} IO_LINKED_DATA;

typedef struct
{
	long			savesystem_type;
	long			saveflags;
	float			version;
	char			filename[256];
	long			ident;
	long			ioflags;//type;
	EERIE_3D		pos;
	EERIE_3D		initpos;
	EERIE_3D		lastpos;
	EERIE_3D		move;
	EERIE_3D		lastmove;
	EERIE_3D		angle;
	EERIE_3D		initangle;
	float			scale;
	unsigned long	savetime;
	float			weight;

	char			locname[64];
	unsigned short	EditorFlags;
	unsigned short	GameFlags;
	long			material;
	short			level;
	short			truelevel;
	long			nbtimers;
	// Script data
	long			scriptload;
	short			show;
	short			collision;
	char			mainevent[64];
	// Physics data
	EERIE_3D		velocity;
	long			stopped;
	IO_PHYSICS		physics;
	float			original_radius;
	float			original_height;
	// Anims data
	char			anims[MAX_ANIMS][256];
	ANIM_USE		animlayer[MAX_ANIM_LAYERS];
	// Target Info
	char	id_targetinfo[SIZE_ID];
	long			inventory;
	// Group Info
	long			system_flags;
	float			basespeed;
	float			speed_modif;
	float			frameloss;
	IO_SPELLCAST_DATA	spellcast_data;

	float				rubber;
	float				max_durability;
	float				durability;
	short				poisonous;
	short				poisonous_count;

	long				nb_linked;
	IO_LINKED_DATA		linked_data[MAX_LINKED_SAVE];

	float				head_rot;

	short				damager_damages;
	short				nb_iogroups;
	long				damager_type;

	unsigned long		type_flags;
	char				stepmaterial[128];
	char				armormaterial[128];
	char				weaponmaterial[128];
	char				strikespeech[128];
	short				Tweak_nb;
	short				padd;
	IO_HALO				halo;
	char				secretvalue;
	char				paddd[3];
	char				shop_category[128];
	float				shop_multiply;
	long				aflags;
	float				ignition;
	char				inventory_skin[128];
	// TO ADD:
	char				usepath_name[SIZE_ID];
	unsigned long		usepath_starttime;
	unsigned long		usepath_curtime;
	long				usepath_aupflags;
	EERIE_3D			usepath_initpos;
	long				usepath_lastWP;
	long	padddd[64];// new...
} ARX_CHANGELEVEL_IO_SAVE;

typedef struct
{
	float	maxlife;
	float	life;
	float	maxmana;
	float	mana;
	long	reachedtarget;
	char	id_weapon[SIZE_ID];
	long	detect;
	long	movemode;
	float	armor_class;
	float	absorb;
	float	damages;
	float	tohit;
	float	aimtime;
	unsigned long behavior;
	float			behavior_param;
	long	tactics;
	long	xpvalue;
	long	cut;
	float	moveproblem;
	long	weapontype;
	long	weaponinhand;
	long	fightdecision;
	char	weaponname[256];
	float	look_around_inc;
	unsigned long collid_time;
	long	collid_state;
	float	speakpitch;
	float	lastmouth;
	IO_BEHAVIOR_DATA	stacked[MAX_STACKED_BEHAVIOR];
	char weapon[SIZE_ID];

	float	critical;
	float	reach;
	float	backstab_skill;
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
	EERIE_EXTRA_ROTATE 	ex_rotate;
	D3DCOLOR	blood_color;
	char stackedtarget[MAX_STACKED_BEHAVIOR][SIZE_ID];
	float		fDetect;
	short		cuts;
	short		spadd;
	long	paddd[63];// new...
} ARX_CHANGELEVEL_NPC_IO_SAVE;

typedef struct
{
	long	price;
	short	maxcount;
	short	count;
	char	food_value;
	char	stealvalue;
	short	playerstacksize;
	short	LightValue;
	IO_EQUIPITEM equipitem;
	long	padd[64];// new...
} ARX_CHANGELEVEL_ITEM_IO_SAVE;

typedef struct
{
	char	trapvalue;
	char	padd[3];
	long	paddd[64];// new...
} ARX_CHANGELEVEL_FIX_IO_SAVE;

typedef struct
{
	long	dummy;
} ARX_CHANGELEVEL_MARKER_IO_SAVE;

typedef struct
{
	EERIE_CAMERA	cam;
} ARX_CHANGELEVEL_CAMERA_IO_SAVE;

typedef struct
{
	float	version;
	char	name[256];
	long	level;
	unsigned long time;
	long	padd[32];
} ARX_CHANGELEVEL_PLAYER_LEVEL_DATA;

//-----------------------------------------------------------------------------
#define SYSTEM_FLAG_TWEAKER_INFO	1
#define SYSTEM_FLAG_INVENTORY		2
#define SYSTEM_FLAG_EQUIPITEMDATA	4
#define SYSTEM_FLAG_USEPATH			8
#define ARX_GAMESAVE_VERSION 1.005f

extern long LAST_CHINSTANCE;
extern char CurGamePath[256];
extern long CURRENT_GAME_INSTANCE;
extern char GameSavePath[256];
extern long FORBID_SAVE;

//-----------------------------------------------------------------------------
void ARX_GAMESAVE_MakePath();
void ARX_GAMESAVE_CreateNewInstance();
void ARX_GAMESAVE_GetInfo(char * path, char * name, long * time);
long ARX_GAMESAVE_Save(long instance, char * name);
long ARX_GAMESAVE_Load(long instance);

void ARX_CHANGELEVEL_MakePath();
long ARX_CHANGELEVEL_PushLevel(long num, long newnum);
long ARX_CHANGELEVEL_PopLevel(long num, long reloadflag = 0);
long ARX_CHANGELEVEL_Push_Index(ARX_CHANGELEVEL_INDEX * asi, long num);
long ARX_CHANGELEVEL_Push_Globals(long num);
long ARX_CHANGELEVEL_Pop_Globals();
long ARX_CHANGELEVEL_Push_Player(long num);
void ARX_CHANGELEVEL_Change(char * level, char * target, long angle, long confirm);
long ARX_CHANGELEVEL_Push_AllIO(long num);
long ARX_CHANGELEVEL_Push_IO(INTERACTIVE_OBJ * io);
long ARX_CHANGELEVEL_Pop_IO(char * ident);
void ARX_CHANGELEVEL_TestLoad(char * level, char * target);
void ARX_CHANGELEVEL_TestSave(char * level, char * target);
long ARX_CHANGELEVEL_GetInfo(char * path, char * name, float * version, long * level, unsigned long * time);
long ARX_CHANGELEVEL_Set_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA * pld, char * path);
long ARX_CHANGELEVEL_Get_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA * pld, char * path);
long ARX_CHANGELEVEL_Load(long instance);
long ARX_CHANGELEVEL_Save(long instance, char * name);
 

void ARX_Changelevel_CurGame_Open();
bool ARX_Changelevel_CurGame_Seek(char * ident);
void ARX_Changelevel_CurGame_Close();

#endif
