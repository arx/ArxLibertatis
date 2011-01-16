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
// ARX_Player
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Player management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_PLAYER_H
#define ARX_PLAYER_H

#include <TCHAR.h>
#include "EERIEpoly.h"
#include "arx_spells.h"

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_3DOBJ		*		lacet;
	EERIE_3DOBJ		*		runes[NB_RUNES];
	TextureContainer	*	pTexTab[NB_RUNES];
} ARX_NECKLACE;

typedef struct
{
	bool			bSpell;
	unsigned long	lTimeCreation;
	unsigned long	lDuration;
	int				iSpellSymbols[6];
	float			fPosX;
	float			fPosY;
} ARX_INTERFACE_MEMORIZE_SPELL;

typedef struct
{
	EERIE_3D		pos;
	EERIE_3D		angle;
	ANIM_USE		useanim;
	IO_PHYSICS		physics;
	// Jump Sub-data
	unsigned long	jumpstarttime;
	float			jumplastposition;
	//	float			jumpvelocity;
	long			jumpphase;	// 0 no jump, 1 doing anticipation anim
	// 2 moving_up	3 moving_down 4 finish_anim
	short			climbing;
	short			levitate;

	EERIE_3D		desiredangle;
	EERIE_3D		size;
	void 	*		inzone;

	long			falling;
	short			doingmagic;
	short			Interface;

	long			Current_Movement;
	long			Last_Movement;
	long			onfirmground;

	INTERACTIVE_OBJ * rightIO;
	INTERACTIVE_OBJ * leftIO;
	INTERACTIVE_OBJ * equipsecondaryIO;
	INTERACTIVE_OBJ * equipshieldIO;

	short			equiped[MAX_EQUIPED]; 

	// Modifier Values (Items, curses, etc...)
	float			Mod_Attribute_Strength;
	float			Mod_Attribute_Dexterity;
	float			Mod_Attribute_Constitution;
	float			Mod_Attribute_Mind;
	float			Mod_Skill_Stealth;
	float			Mod_Skill_Mecanism;
	float			Mod_Skill_Intuition;
	float			Mod_Skill_Etheral_Link;
	float			Mod_Skill_Object_Knowledge;
	float			Mod_Skill_Casting;
	float			Mod_Skill_Projectile;
	float			Mod_Skill_Close_Combat;
	float			Mod_Skill_Defense;
	float			Mod_armor_class;
	float			Mod_resist_magic;
	float			Mod_resist_poison;
	float			Mod_Critical_Hit;
	float			Mod_damages;
	float			Mod_life;
	float			Mod_maxlife;
	float			Mod_mana;
	float			Mod_maxmana;

	// Full Frame values (including items)
	float			Full_Attribute_Strength;
	float			Full_Attribute_Dexterity;
	float			Full_Attribute_Constitution;
	float			Full_Attribute_Mind;

	float			Full_Skill_Stealth;
	float			Full_Skill_Mecanism;
	float			Full_Skill_Intuition;

	float			Full_Skill_Etheral_Link;
	float			Full_Skill_Object_Knowledge;
	float			Full_Skill_Casting;

	float			Full_Skill_Projectile;
	float			Full_Skill_Close_Combat;
	float			Full_Skill_Defense;
	float			Full_armor_class;
	float			Full_resist_magic;
	float			Full_resist_poison;
	float			Full_Critical_Hit;
	float			Full_damages;
	long			Full_AimTime;
	long			Full_Weapon_Type;
	float			Full_life;
	float			Full_maxlife;
	float			Full_mana;
	float			Full_maxmana;


	// TRUE (naked) Player Values
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
	float			life;
	float			maxlife;
	float			mana;
	float			maxmana;

	// Player Old Values
	float			Old_Skill_Stealth;
	float			Old_Skill_Mecanism;
	float			Old_Skill_Intuition;

	float			Old_Skill_Etheral_Link;
	float			Old_Skill_Object_Knowledge;
	float			Old_Skill_Casting;

	float			Old_Skill_Projectile;
	float			Old_Skill_Close_Combat;
	float			Old_Skill_Defense;

	unsigned char	Attribute_Redistribute;
	unsigned char	Skill_Redistribute;


	unsigned char	level;


	unsigned char	armor_class;
	unsigned char	resist_magic;
	unsigned char	resist_poison;
	long			xp;
	char			skin;
	unsigned char	padd[3];

	unsigned long	rune_flags;
	TextureContainer * heads[5];
	float			damages;
	float			poison;
	float			hunger;
	float			grnd_color;
	long			playerflags;
	long			gold;
	short			bag;
	short			sp_flags;
	ARX_INTERFACE_MEMORIZE_SPELL SpellToMemorize;
} ARXCHARACTER;

#define SP_MAX		1
#define SP_RF		4
#define SP_WEP		8
#define SP_MR		16
#define SP_ARM1		32
#define SP_ARM2		64
#define SP_ARM3		128
#define SP_SP		256
#define SP_SP2		512


typedef struct
{
	char slot[64];
} KEYRING_SLOT;

//////////////////////////////////////////////
// Quests Management (QuestLogBook)
//////////////////////////////////////////////
#define MAX_QUESTS 100
typedef struct
{
	char	* ident;
	_TCHAR	* localised;
} STRUCT_QUEST;

//-----------------------------------------------------------------------------
#define PLAYER_MOVE_WALK_FORWARD	1
#define PLAYER_MOVE_WALK_BACKWARD	(1<<1)
#define PLAYER_MOVE_STRAFE_LEFT		(1<<2)
#define PLAYER_MOVE_STRAFE_RIGHT	(1<<3)
#define PLAYER_MOVE_JUMP			(1<<4)
#define PLAYER_MOVE_STEALTH			(1<<5)
#define PLAYER_ROTATE				(1<<6)
#define PLAYER_CROUCH				(1<<7)
#define PLAYER_LEAN_LEFT			(1<<8)
#define PLAYER_LEAN_RIGHT			(1<<9)

#define PLAYERFLAGS_NO_MANA_DRAIN	1
#define PLAYERFLAGS_INVULNERABILITY	2

#define ANIM1ST_WAIT				0
#define ANIM1ST_DRAW_1H				1
#define ANIM1ST_DRAW_1H_OFF			2
#define ANIM1ST_RIGHT_STRIKE_1H		3
#define ANIM1ST_LEFT_STRIKE_1H		4
#define ANIM1ST_AIM_LEFT_1H			5
#define ANIM1ST_AIM_RIGHT_1H		6
#define ANIM1ST_WAIT_2H				7
#define ANIM1ST_DRAW_2H				8
#define ANIM1ST_DRAW_2H_OFF			9
#define ANIM1ST_RIGHT_STRIKE_2H		10
#define ANIM1ST_LEFT_STRIKE_2H		11
#define ANIM1ST_AIM_LEFT_2H			12
#define ANIM1ST_AIM_RIGHT_2H		13
#define ANIM1ST_WAIT_BARE			14
#define ANIM1ST_DRAW_BARE			15
#define ANIM1ST_DRAW_BARE_OFF		16
#define ANIM1ST_RIGHT_STRIKE_BARE	17
#define ANIM1ST_LEFT_STRIKE_BARE	18
#define ANIM1ST_AIM_LEFT_BARE		19
#define ANIM1ST_AIM_RIGHT_BARE		20

#define ANIM1ST_CAST_ON				21
#define ANIM1ST_CAST_OFF			22
#define ANIM1ST_CAST_MIDDLE			23
#define ANIM1ST_CAST_WAIT			24
#define ANIM1ST_SHIELD_START		25
#define ANIM1ST_SHIELD_HOLD			26
#define ANIM1ST_SHIELD_END			27
#define ANIM1ST_TALK				28

#define MAX_ANIMS1ST 32

#define FLAG_AAM			(1<<(RUNE_AAM))
#define FLAG_CETRIUS		(1<<(RUNE_CETRIUS))
#define FLAG_COMUNICATUM	(1<<(RUNE_COMUNICATUM))
#define FLAG_COSUM			(1<<(RUNE_COSUM))
#define FLAG_FOLGORA		(1<<(RUNE_FOLGORA))
#define FLAG_FRIDD			(1<<(RUNE_FRIDD))
#define FLAG_KAOM			(1<<(RUNE_KAOM))
#define FLAG_MEGA			(1<<(RUNE_MEGA))
#define FLAG_MORTE			(1<<(RUNE_MORTE))
#define FLAG_MOVIS			(1<<(RUNE_MOVIS))
#define FLAG_NHI			(1<<(RUNE_NHI))
#define FLAG_RHAA			(1<<(RUNE_RHAA))
#define FLAG_SPACIUM		(1<<(RUNE_SPACIUM))
#define FLAG_STREGUM		(1<<(RUNE_STREGUM))
#define FLAG_TAAR			(1<<(RUNE_TAAR))
#define FLAG_TEMPUS			(1<<(RUNE_TEMPUS))
#define FLAG_TERA			(1<<(RUNE_TERA))
#define FLAG_VISTA			(1<<(RUNE_VISTA))
#define FLAG_VITAE			(1<<(RUNE_VITAE))
#define FLAG_YOK			(1<<(RUNE_YOK))

//-----------------------------------------------------------------------------
extern ARXCHARACTER player;
extern ARX_NECKLACE necklace;
extern EERIE_3DOBJ * hero;
extern ANIM_HANDLE * heroanim;
extern ANIM_HANDLE * heroanim2;
extern ANIM_HANDLE * herowaitbook;
extern ANIM_HANDLE * herowait;
extern ANIM_HANDLE * herowait2;
extern ANIM_HANDLE * herowalk;
extern ANIM_HANDLE * herorun;
extern STRUCT_QUEST * PlayerQuest;
extern KEYRING_SLOT * Keyring;
extern float DeadCameraDistance;
extern long nb_PlayerQuest;
extern long Keyring_Number;
extern long BLOCK_PLAYER_CONTROLS;
extern long USE_PLAYERCOLLISIONS;
extern long WILLRETURNTOCOMBATMODE;

//-----------------------------------------------------------------------------
void	ARX_PLAYER_MakeSpHero();
void	ARX_PLAYER_LoadHeroAnimsAndMesh();
void	ARX_PLAYER_InitPlayer();
void	ARX_PLAYER_BecomesDead();
void	ARX_PLAYER_ClickedOnTorch(INTERACTIVE_OBJ * io);
void	ARX_PLAYER_ManageTorch();
void	ARX_PLAYER_RectifyPosition();
void	ARX_PLAYER_Frame_Update();
void	ARX_PLAYER_Manage_Movement();
void	ARX_PLAYER_Manage_Death();
void	ARX_PLAYER_GotoAnyPoly();
void	ARX_PLAYER_Quest_Add(char * quest, bool _bLoad = false);
void	ARX_PLAYER_Quest_Init();
void	ARX_PLAYER_Quest_FirstInit();
void	ARX_PLAYER_FrontPos(EERIE_3D * pos);
void	ARX_PLAYER_MakePowerfullHero();
void	ARX_PLAYER_ComputePlayerStats();
void	ARX_PLAYER_ComputePlayerFullStats();
void	ARX_PLAYER_MakeFreshHero();
void	ARX_PLAYER_QuickGeneration();
void	ARX_PLAYER_MakeAverageHero();
void	ARX_PLAYER_Modify_XP(long val);
void	ARX_PLAYER_FrameCheck(float _framedelay);
void	ARX_PLAYER_Poison(float val);
void	ARX_PLAYER_Manage_Visual();
void	ARX_PLAYER_Remove_Invisibility();
void	ARX_Player_Rune_Add(unsigned long);
void	ARX_Player_Rune_Remove(unsigned long);
void	ARX_PLAYER_AddGold(long);
void	ARX_PLAYER_AddBag();
bool	ARX_PLAYER_CanStealItem(INTERACTIVE_OBJ *);


void	ARX_KEYRING_Init();
void	ARX_KEYRING_Add(char * key);
void	ARX_KEYRING_Combine(INTERACTIVE_OBJ * io);

void	ARX_PLAYER_Reset_Fall();
void	ARX_PLAYER_KillTorch();
void	ARX_PLAYER_ClickedOnTorch(INTERACTIVE_OBJ * io);
void	ARX_PLAYER_PutPlayerInNormalStance(long val);
void	ARX_PLAYER_Start_New_Quest();
void	ARX_PLAYER_Rune_Add_All();
float	ARX_PLAYER_Get_Skill_Stealth(long type);
float	ARX_PLAYER_Get_Skill_Mecanism(long type);
float	ARX_PLAYER_Get_Skill_Intuition(long type);
float	ARX_PLAYER_Get_Skill_Etheral_Link(long type);
float	ARX_PLAYER_Get_Skill_Object_Knowledge(long type);
float	ARX_PLAYER_Get_Skill_Casting(long type);
float	ARX_PLAYER_Get_Skill_Projectile(long type);
float	ARX_PLAYER_Get_Skill_Close_Combat(long type);
float	ARX_PLAYER_Get_Skill_Defense(long type);
 
void ARX_PLAYER_Restore_Skin();
float	GetPlayerStealth();

void ARX_GAME_Reset(long type = 0);
void Manage_sp_max();
long GetXPforLevel(long level);
bool ARX_PLAYER_IsInFightMode();
void ARX_PLAYER_Invulnerability(long flag);
#endif
