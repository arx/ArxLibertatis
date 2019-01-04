/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_NPC_H
#define ARX_GAME_NPC_H

#include <string>

#include "game/Entity.h"
#include "game/GameTypes.h"
#include "math/Types.h"
#include "util/Flags.h"

static const size_t MAX_STACKED_BEHAVIOR = 5;
static const size_t MAX_EXTRA_ROTATE = 4;

enum MoveMode {
	WALKMODE = 0,
	RUNMODE = 1,
	NOMOVEMODE = 2,
	SNEAKMODE = 3
};

enum BehaviourFlag {
	BEHAVIOUR_NONE          = 1 << 0, // no pathfind
	BEHAVIOUR_FRIENDLY      = 1 << 1, // no pathfind
	BEHAVIOUR_MOVE_TO       = 1 << 2,
	BEHAVIOUR_WANDER_AROUND = 1 << 3, // behavior_param = distance
	BEHAVIOUR_FLEE          = 1 << 4, // behavior_param = distance
	BEHAVIOUR_HIDE          = 1 << 5, // behavior_param = distance
	BEHAVIOUR_LOOK_FOR      = 1 << 6, // behavior_param = distance
	BEHAVIOUR_SNEAK         = 1 << 7,
	BEHAVIOUR_FIGHT         = 1 << 8,
	BEHAVIOUR_DISTANT       = 1 << 9,
	BEHAVIOUR_MAGIC         = 1 << 10,
	BEHAVIOUR_GUARD         = 1 << 11,
	BEHAVIOUR_GO_HOME       = 1 << 12,
	BEHAVIOUR_LOOK_AROUND   = 1 << 13,
	BEHAVIOUR_STARE_AT      = 1 << 14
};
DECLARE_FLAGS(BehaviourFlag, Behaviour)
DECLARE_FLAGS_OPERATORS(Behaviour)

struct IO_BEHAVIOR_DATA {
	
	long exist;
	Behaviour behavior;
	float behavior_param;
	EntityHandle target;
	MoveMode movemode;
	AnimLayer animlayer[MAX_ANIM_LAYERS];
	
	IO_BEHAVIOR_DATA()
		: exist(0)
		, behavior(0)
		, behavior_param(0.f)
		, movemode(WALKMODE)
	{ }
	
};

enum PathfindFlag {
	PATHFIND_ALWAYS    = 1 << 0,
	PATHFIND_ONCE      = 1 << 1,
	PATHFIND_NO_UPDATE = 1 << 2
};
DECLARE_FLAGS(PathfindFlag, PathfindFlags)
DECLARE_FLAGS_OPERATORS(PathfindFlags)

struct IO_PATHFIND {
	
	PathfindFlags flags;
	long listnb;
	long * list;
	unsigned short listpos;
	short pathwait;
	EntityHandle truetarget;
	
	IO_PATHFIND()
		: flags(0)
		, listnb(0)
		, list(NULL)
		, listpos(0)
		, pathwait(0)
		, truetarget(0) // TODO is this correct ? use EntityHandle() ?
	{ }
	
};

struct EERIE_EXTRA_ROTATE {
	ObjVertGroup group_number[MAX_EXTRA_ROTATE];
	Anglef group_rotate[MAX_EXTRA_ROTATE];
};

struct EERIE_EXTRA_SCALE {
	
	ObjVertGroup groupIndex;
	Vec3f scale;

	EERIE_EXTRA_SCALE()
		: scale(0.f)
	{ }
	
};

enum NPCFlag {
	NPCFLAG_BACKSTAB = 1 << 0
};
DECLARE_FLAGS(NPCFlag, NPCFlags)
DECLARE_FLAGS_OPERATORS(NPCFlags)

enum DismembermentFlag {
	FLAG_CUT_HEAD  = 1 << 0,
	FLAG_CUT_TORSO = 1 << 1,
	FLAG_CUT_LARM  = 1 << 2,
	FLAG_CUT_RARM  = 1 << 3,
	FLAG_CUT_LLEG  = 1 << 4,
	FLAG_CUT_RLEG  = 1 << 5
};

DECLARE_FLAGS(DismembermentFlag, DismembermentFlags)
DECLARE_FLAGS_OPERATORS(DismembermentFlags)

struct IO_NPCDATA {
	
	IO_NPCDATA();
	~IO_NPCDATA();
	
	ResourcePool lifePool;
	ResourcePool manaPool;
	
	GameInstant reachedtime;
	long reachedtarget; // Is target in REACHZONE?
	Entity * weapon; // Linked Weapon (r-hand)
	long detect;
	MoveMode movemode;
	float armor_class;
	float absorb;
	float damages;
	float tohit;
	GameDuration aimtime;
	float critical;
	float reach;
	float backstab_skill;
	
	Behaviour behavior;
	float behavior_param;
	long xpvalue;
	long cut;
	
	float moveproblem;
	ItemType weapontype;
	long weaponinhand;
	long fightdecision;
	
	float look_around_inc;
	float speakpitch;
	float lastmouth;
	long ltemp;
	
	IO_BEHAVIOR_DATA stacked[MAX_STACKED_BEHAVIOR];
	float poisonned;
	unsigned char resist_poison;
	unsigned char resist_magic;
	unsigned char resist_fire;
	
	GameDuration walk_start_time;
	GameInstant aiming_start;
	NPCFlags npcflags;
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
	DismembermentFlags cuts;
	
};

const float ARX_NPC_AUDIBLE_VOLUME_MIN = 0.94f;
const float ARX_NPC_AUDIBLE_VOLUME_MAX = 1.f;
const float ARX_NPC_AUDIBLE_VOLUME_DEFAULT = ARX_NPC_AUDIBLE_VOLUME_MAX;
const float ARX_NPC_AUDIBLE_VOLUME_RANGE = ARX_NPC_AUDIBLE_VOLUME_MAX - ARX_NPC_AUDIBLE_VOLUME_MIN;
const float ARX_NPC_AUDIBLE_FACTOR_MIN = 1.f;
const float ARX_NPC_AUDIBLE_FACTOR_MAX = 4.5f;
const float ARX_NPC_AUDIBLE_FACTOR_DEFAULT = ARX_NPC_AUDIBLE_FACTOR_MIN;
const float ARX_NPC_AUDIBLE_FACTOR_RANGE = ARX_NPC_AUDIBLE_FACTOR_MAX - ARX_NPC_AUDIBLE_FACTOR_MIN;
const float ARX_NPC_AUDIBLE_PRESENCE_DEFAULT = 1.f;

void ARX_NPC_Revive(Entity * io, bool init);
bool ARX_NPC_SetStat(Entity & io, const std::string & statname, float value);
bool ARX_NPC_LaunchPathfind(Entity * io, EntityHandle target);
bool IsDeadNPC(const Entity & io);

void FaceTarget2(Entity * io);
void ARX_NPC_Behaviour_Stack(Entity * io);
void ARX_NPC_Behaviour_UnStack(Entity * io);
void ARX_NPC_Behaviour_Reset(Entity * io);
void ARX_NPC_Behaviour_ResetAll();
void ARX_NPC_Behaviour_Change(Entity * io, Behaviour behavior, long behavior_param);
void ARX_NPC_ChangeMoveMode(Entity * io, MoveMode MOVEMODE);
void ARX_NPC_SpawnAudibleSound(const Vec3f & pos, Entity * source,
                               float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT,
                               float presence = ARX_NPC_AUDIBLE_PRESENCE_DEFAULT);
void ARX_NPC_NeedStepSound(Entity * io, const Vec3f & pos,
                           float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT,
                           float power = ARX_NPC_AUDIBLE_FACTOR_DEFAULT);

Entity * ARX_NPC_GetFirstNPCInSight(Entity * ioo);
void CheckNPC(Entity & io);
void ManageIgnition(Entity & io);
void ManageIgnition_2(Entity & io);

void ARX_NPC_Kill_Spell_Launch(Entity * io);

void ARX_PHYSICS_Apply();

void GetTargetPos(Entity * io, unsigned long smoothing = 0);

float GetIOHeight(Entity * io);
float GetIORadius(const Entity * io);

Cylinder GetIOCyl(Entity * io);

bool isEnemy(const Entity * entity);

#endif // ARX_GAME_NPC_H
