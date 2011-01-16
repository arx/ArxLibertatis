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
// ARX_Damages
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Damages management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_DAMAGES_H
#define ARX_DAMAGES_H

#include "EERIEPoly.h"

//-----------------------------------------------------------------------------
typedef struct
{
	short		exist;
	short		active;
	EERIE_3D	pos;
	float		damages;
	float		radius;
	unsigned long start_time;
	short		except[10];
	long		duration;	// in milliseconds
	// -1 for apply once
	// else damage *=framediff
	long		source;		// io index or -1 for player
	long		area;		// damage area type
	long		flags;		// damages flags
	long		type;		// damages type
	long		special;	// slowdown, paralysis...
	long		special_ID;	// for io localised immunities or any other customization
	unsigned long lastupd;
} DAMAGE_INFO;

//-----------------------------------------------------------------------------
#define DAMAGE_AREA 0
#define DAMAGE_FULL 1
#define DAMAGE_AREAHALF 2

#define DAMAGE_TYPE_GENERIC		0
#define DAMAGE_TYPE_FIRE		1
#define DAMAGE_TYPE_MAGICAL		(1<<1)
#define DAMAGE_TYPE_LIGHTNING	(1<<2)
#define DAMAGE_TYPE_COLD		(1<<3)
#define DAMAGE_TYPE_POISON		(1<<4)
#define DAMAGE_TYPE_GAS			(1<<5)
#define DAMAGE_TYPE_METAL		(1<<6)
#define DAMAGE_TYPE_WOOD		(1<<7)
#define DAMAGE_TYPE_STONE		(1<<8)
#define DAMAGE_TYPE_ACID		(1<<9)
#define DAMAGE_TYPE_ORGANIC		(1<<10)
#define DAMAGE_TYPE_PER_SECOND	(1<<11)
#define DAMAGE_TYPE_DRAIN_LIFE	(1<<12)
#define DAMAGE_TYPE_DRAIN_MANA	(1<<13)
#define DAMAGE_TYPE_PUSH		(1<<14)
#define DAMAGE_TYPE_FAKEFIRE	(1<<15)
#define DAMAGE_TYPE_FIELD		(1<<16)
#define DAMAGE_TYPE_NO_FIX		(1<<17)

#define DAMAGE_FLAG_DONT_HURT_SOURCE	1
#define DAMAGE_FLAG_ADD_VISUAL_FX		2   // depending on type
#define DAMAGE_FLAG_FOLLOW_SOURCE		4
#define	DAMAGE_NOT_FRAME_DEPENDANT		32
#define DAMAGE_SPAWN_BLOOD				64
#define MAX_DAMAGES	200

//-----------------------------------------------------------------------------
extern	DAMAGE_INFO	damages[MAX_DAMAGES];

//-----------------------------------------------------------------------------
void	CheckForIgnition(EERIE_3D * pos, float radius, long mode, long flag = 0);
 
 
BOOL	DoSphericDamage(EERIE_3D * pos, float dmg, float radius, long flags, long typ = 0, long numsource = -1);

void	ARX_DAMAGE_Reset_Blood_Info();
void	ARX_DAMAGE_Show_Hit_Blood(LPDIRECT3DDEVICE7 pd3dDevice);
void	ARX_DAMAGES_Reset();
long	ARX_DAMAGES_GetFree();
 
void	ARX_DAMAGES_UpdateAll();
float	ARX_DAMAGES_DamagePlayer(float dmg, long type, long source = -1, EERIE_3D * pos = NULL); 
void	ARX_DAMAGES_DamageFIX(INTERACTIVE_OBJ * io, float dmg, long source = -1, long flags = 0, EERIE_3D * pos = NULL);
float	ARX_DAMAGES_DamageNPC(INTERACTIVE_OBJ * io, float dmg, long source = -1, long flags = 0, EERIE_3D * pos = NULL); 
BOOL	ARX_DAMAGES_TryToDoDamage(EERIE_3D * pos, float dmg, float radius, long source); 
void	ARX_DAMAGES_ForceDeath(INTERACTIVE_OBJ * io_dead, INTERACTIVE_OBJ * io_killer);
void	ARX_DAMAGES_UpdateDamage(long j, float tim);
float	ARX_DAMAGES_DealDamages(long target, float dmg, long source, long flags, EERIE_3D * pos);

void ARX_DAMAGES_HealInter(INTERACTIVE_OBJ * io, float dmg);

// ON-SCREEN Blood Splat Management
 
void ARX_DAMAGES_SCREEN_SPLATS_Add(EERIE_3D * pos, float dmgs);
void ARX_DAMAGES_SCREEN_SPLATS_Init();
void ARX_DAMAGES_DurabilityCheck(INTERACTIVE_OBJ * io, float ratio);
void ARX_DAMAGES_DurabilityLoss(INTERACTIVE_OBJ * io, float loss);
void ARX_DAMAGES_DurabilityRestore(INTERACTIVE_OBJ * io, float ratio);
void ARX_DAMAGES_DamagePlayerEquipment(float damages);
float ARX_DAMAGES_ComputeRepairPrice(INTERACTIVE_OBJ * torepair, INTERACTIVE_OBJ * blacksmith);
void ARX_DAMAGES_IgnitIO(INTERACTIVE_OBJ * io, float dmg);
#endif
