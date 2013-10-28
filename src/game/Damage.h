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

#ifndef ARX_GAME_DAMAGE_H
#define ARX_GAME_DAMAGE_H

#include <stddef.h>

#include "math/Types.h"
#include "math/Vector.h"
#include "platform/Flags.h"

class Entity;

enum DamageTypeFlag {
	DAMAGE_TYPE_GENERIC    = 0,
	DAMAGE_TYPE_FIRE       = (1<<0),
	DAMAGE_TYPE_MAGICAL    = (1<<1),
	DAMAGE_TYPE_LIGHTNING  = (1<<2),
	DAMAGE_TYPE_COLD       = (1<<3),
	DAMAGE_TYPE_POISON     = (1<<4),
	DAMAGE_TYPE_GAS        = (1<<5),
	DAMAGE_TYPE_METAL      = (1<<6),
	DAMAGE_TYPE_WOOD       = (1<<7),
	DAMAGE_TYPE_STONE      = (1<<8),
	DAMAGE_TYPE_ACID       = (1<<9),
	DAMAGE_TYPE_ORGANIC    = (1<<10),
	DAMAGE_TYPE_PER_SECOND = (1<<11),
	DAMAGE_TYPE_DRAIN_LIFE = (1<<12),
	DAMAGE_TYPE_DRAIN_MANA = (1<<13),
	DAMAGE_TYPE_PUSH       = (1<<14),
	DAMAGE_TYPE_FAKEFIRE   = (1<<15),
	DAMAGE_TYPE_FIELD      = (1<<16),
	DAMAGE_TYPE_NO_FIX     = (1<<17)
};
DECLARE_FLAGS(DamageTypeFlag, DamageType)
DECLARE_FLAGS_OPERATORS(DamageType)

enum DamageArea {
	DAMAGE_AREA = 0,
	DAMAGE_FULL = 1,
	DAMAGE_AREAHALF = 2
};

enum DamageFlag {
	DAMAGE_FLAG_DONT_HURT_SOURCE = (1<<0),
	DAMAGE_FLAG_ADD_VISUAL_FX    = (1<<1), // depending on type
	DAMAGE_FLAG_FOLLOW_SOURCE    = (1<<2),
	DAMAGE_NOT_FRAME_DEPENDANT   = (1<<5),
	DAMAGE_SPAWN_BLOOD           = (1<<6)
};
DECLARE_FLAGS(DamageFlag, DamageFlags)
DECLARE_FLAGS_OPERATORS(DamageFlags)

struct DAMAGE_INFO {
	short exist;
	short active;
	Vec3f pos;
	float damages;
	float radius;
	unsigned long start_time;
	short except[10];
	long duration;	// in milliseconds
	// -1 for apply once
	// else damage *=framediff
	long source; // io index or -1 for player
	DamageArea area; // damage area type
	DamageFlags flags; // damages flags
	DamageType type; // damages type
	long special; // slowdown, paralysis...
	long special_ID; // for io localised immunities or any other customization
	unsigned long lastupd;
};

const size_t MAX_DAMAGES = 200;

extern DAMAGE_INFO damages[MAX_DAMAGES];

/*!
 * mode=true ON mode=false  OFF
 * flag & 1 no lights;
 * flag & 2 Only affects small sources
 */
void CheckForIgnition(Vec3f * pos, float radius, bool mode, long flag = 0);

bool DoSphericDamage(Vec3f * pos, float dmg, float radius, DamageArea flags, DamageType typ = 0, long numsource = -1);

void ARX_DAMAGE_Reset_Blood_Info();
void ARX_DAMAGE_Show_Hit_Blood();
void ARX_DAMAGES_Reset();
long ARX_DAMAGES_GetFree();
 
void ARX_DAMAGES_UpdateAll();
float ARX_DAMAGES_DamagePlayer(float dmg, DamageType type, long source = -1); 
void ARX_DAMAGES_DamageFIX(Entity * io, float dmg, long source = -1, long flags = 0);
float ARX_DAMAGES_DamageNPC(Entity * io, float dmg, long source = -1, long flags = 0, Vec3f * pos = NULL); 
bool ARX_DAMAGES_TryToDoDamage(Vec3f * pos, float dmg, float radius, long source); 
void ARX_DAMAGES_ForceDeath(Entity * io_dead, Entity * io_killer);
void ARX_DAMAGES_UpdateDamage(long j, float tim);
float ARX_DAMAGES_DealDamages(long target, float dmg, long source, DamageType flags, Vec3f * pos);

void ARX_DAMAGES_HealInter(Entity * io, float dmg);

void ARX_DAMAGES_DurabilityCheck(Entity * io, float ratio);
void ARX_DAMAGES_DurabilityLoss(Entity * io, float loss);
void ARX_DAMAGES_DurabilityRestore(Entity * io, float ratio);
void ARX_DAMAGES_DamagePlayerEquipment(float damages);
float ARX_DAMAGES_ComputeRepairPrice(Entity * torepair, Entity * blacksmith);

#endif // ARX_GAME_DAMAGE_H
