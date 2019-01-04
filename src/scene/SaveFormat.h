/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_SCENE_SAVEFORMAT_H
#define ARX_SCENE_SAVEFORMAT_H

#include <algorithm>
#include <cstring>

#include "gui/MiniMap.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "graphics/GlobalFog.h"
#include "graphics/GraphicsFormat.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/MeshManipulation.h"
#include "platform/Platform.h"
#include "util/String.h"

const s32 SAVEFLAGS_EXTRA_ROTATE = 1;
const float ARX_GAMESAVE_VERSION = 1.005f;

enum SavedIOType {
	TYPE_NPC = 1,
	TYPE_ITEM = 2,
	TYPE_FIX = 3,
	TYPE_CAMERA = 4,
	TYPE_MARKER = 5
};

enum SystemFlag {
	SYSTEM_FLAG_TWEAKER_INFO  = 1 << 0,
	SYSTEM_FLAG_INVENTORY     = 1 << 1,
	SYSTEM_FLAG_EQUIPITEMDATA = 1 << 2,
	SYSTEM_FLAG_USEPATH       = 1 << 3
};

enum SavePlayerFlag {
	SP_MAX  = 1 << 0,
	SP_RF   = 1 << 2,
	SP_WEP  = 1 << 3,
	SP_MR   = 1 << 4,
	SP_ARM1 = 1 << 5,
	SP_ARM2 = 1 << 6,
	SP_ARM3 = 1 << 7,
	SP_SP   = 1 << 8,
	SP_SP2  = 1 << 9
};

enum VariableType {
	TYPE_UNKNOWN = 0, // does not exist !
	TYPE_G_TEXT = 1,
	TYPE_L_TEXT = 2,
	TYPE_G_LONG = 4,
	TYPE_L_LONG = 8,
	TYPE_G_FLOAT = 16,
	TYPE_L_FLOAT = 32
};


#pragma pack(push, 1)


const size_t SAVED_QUEST_SLOT_SIZE = 80;
const size_t SAVED_KEYRING_SLOT_SIZE = 64;
const size_t MAX_LINKED_SAVE = 16;
const size_t SIZE_ID = 64;
const size_t SAVED_MAX_STACKED_BEHAVIOR = 5;

struct SavedGlobalMods {
	
	s32 flags;
	SavedColor depthcolor;
	f32 zclip;
	char padding[136];
	
	SavedGlobalMods & operator=(const GLOBAL_MODS & b) {
		flags = b.flags;
		depthcolor = b.depthcolor;
		zclip = b.zclip;
		memset(padding, 0, sizeof(padding));
		return *this;
	}
	
	operator GLOBAL_MODS() const {
		GLOBAL_MODS a;
		a.flags = GMODFlags::load(flags); // TODO save/load flags
		a.depthcolor = depthcolor;
		a.zclip = zclip;
		return a;
	}
	
};

struct ARX_CHANGELEVEL_INDEX {
	f32 version;
	u32 time;
	s32 nb_inter;
	s32 nb_paths;
	s32 nb_lights;
	s32 ambiances_data_size;
	SavedGlobalMods gmods_stacked;
	SavedGlobalMods gmods_current;
	SavedGlobalMods gmods_desired;
	s32 padding[256];
};

struct ARX_CHANGELEVEL_LIGHT {
	s16 status;
	s16 padding[3];
};

struct ARX_CHANGELEVEL_IO_INDEX {
	char filename[256];
	s32 ident;
	s32 num; // unused TODO use this to restore spell targets?
	s16 level; // unused
	s16 truelevel; // unused
	s32 padding[257];
};

struct ARX_CHANGELEVEL_PATH {
	char name[64];
	char controled[64];
	s32 padd[64];
};

struct ARX_CHANGELEVEL_SAVE_GLOBALS {
	f32 version;
	s32 nb_globals;
	s32 padding[256];
};

struct SavedMapMarkerData {
	
	static const size_t STRING_SIZE = 64;
	
	f32 x;
	f32 y;
	s32 lvl;
	char name[STRING_SIZE];
	
	/* implicit */ SavedMapMarkerData(const MiniMap::MapMarkerData & b) {
		x = b.m_pos.x;
		y = b.m_pos.y;
		lvl = b.m_lvl;
		arx_assert(STRING_SIZE > b.m_name.length());
		util::storeString(name, b.m_name);
	}
	
};

struct SavedCylinder {
	
	SavedVec3 origin;
	f32 radius;
	f32 height;
	
	operator Cylinder() const {
		Cylinder a;
		a.origin = origin.toVec3(), a.radius = radius, a.height = height;
		return a;
	}
	
	SavedCylinder & operator=(const Cylinder & b) {
		origin = b.origin, radius = b.radius, height = b.height;
		return *this;
	}
	
};

struct SavedIOPhysics {
	
	SavedCylinder cyl;
	SavedVec3 startpos;
	SavedVec3 targetpos;
	SavedVec3 velocity;
	SavedVec3 forces;
	
	operator IO_PHYSICS() const {
		IO_PHYSICS a;
		a.cyl = cyl;
		a.startpos = startpos.toVec3();
		a.targetpos = targetpos.toVec3();
		a.velocity = velocity.toVec3();
		a.forces = forces.toVec3();
		return a;
	}
	
	SavedIOPhysics & operator=(const IO_PHYSICS & b) {
		cyl = b.cyl;
		startpos = b.startpos;
		targetpos = b.targetpos;
		velocity = b.velocity;
		forces = b.forces;
		return *this;
	}
	
};

const size_t SAVED_MINIMAP_MAX_X = 50;
const size_t SAVED_MINIMAP_MAX_Z = 50;

struct SavedMiniMap {
	
	static const size_t MAX_X = 50;
	static const size_t MAX_Z = 50;
	
	u32 padding;
	f32 offsetx;
	f32 offsety;
	f32 xratio;
	f32 yratio;
	f32 width;
	f32 height;
	u8 revealed[MAX_X][MAX_Z];
	
	operator MiniMap::MiniMapData() const {
		MiniMap::MiniMapData a;
		a.m_texContainer = NULL;
		a.m_offset.x = offsetx;
		a.m_offset.y = offsety;
		a.m_ratio.x = xratio;
		a.m_ratio.y = yratio;
		a.m_size.x = width;
		a.m_size.y = height;
		ARX_STATIC_ASSERT(SavedMiniMap::MAX_X == MINIMAP_MAX_X, "array size mismatch");
		ARX_STATIC_ASSERT(SavedMiniMap::MAX_Z == MINIMAP_MAX_Z, "array size mismatch");
		std::copy(&revealed[0][0], &revealed[0][0] + (SavedMiniMap::MAX_X * SavedMiniMap::MAX_Z), &a.m_revealed[0][0]);
		return a;
	}
	
	SavedMiniMap & operator=(const MiniMap::MiniMapData & b) {
		padding = 0;
		offsetx = b.m_offset.x;
		offsety = b.m_offset.y;
		xratio = b.m_ratio.x;
		yratio = b.m_ratio.y;
		width = b.m_size.x;
		height = b.m_size.y;
		ARX_STATIC_ASSERT(SavedMiniMap::MAX_X == MINIMAP_MAX_X, "array size mismatch");
		ARX_STATIC_ASSERT(SavedMiniMap::MAX_Z == MINIMAP_MAX_Z, "array size mismatch");
		std::copy(&b.m_revealed[0][0], &b.m_revealed[0][0] + (SavedMiniMap::MAX_X * SavedMiniMap::MAX_Z), &revealed[0][0]);
		return *this;
	}
	
};

const size_t SAVED_MAX_PRECAST = 3;

struct SavedPrecast {
	
	s32 typ;
	s32 level;
	u32 launch_time;
	s32 flags;
	s32 duration;
	
	operator PRECAST_STRUCT() const {
		PRECAST_STRUCT a;
		a.typ = (typ < 0) ? SPELL_NONE : SpellType(typ); // TODO save/load enum
		a.level = level;
		a.launch_time = GameInstantMs(launch_time); // TODO save/load time
		a.flags = SpellcastFlags::load(flags); // TODO save/load flags
		if(duration >= 0) {
			a.duration = GameDurationMs(duration); // TODO save/load time
		} else {
			a.duration = GameDuration::ofRaw(-1);
		}
		return a;
	}
	
	SavedPrecast & operator=(const PRECAST_STRUCT & b) {
		typ = (b.typ == SPELL_NONE) ? -1 : b.typ;
		level = b.level;
		launch_time = toMsi(b.launch_time); // TODO save/load time
		flags = b.flags;
		if(b.duration >= 0) {
			duration = toMsi(b.duration); // TODO save/load time
		} else {
			duration = -1;
		}
		return *this;
	}
	
};

const size_t SAVED_INVENTORY_BAGS = 3;
const size_t SAVED_INVENTORY_X = 16;
const size_t SAVED_INVENTORY_Y = 3;
const size_t SAVED_MAX_MINIMAPS = 32;
const size_t SAVED_MAX_EQUIPED = 12;
const size_t SAVED_MAX_ANIMS = 200;

struct ARX_CHANGELEVEL_PLAYER {
	
	f32 version;
	s32 Current_Movement;
	s32 Last_Movement;
	s32 misc_flags;
	// Player Values
	f32 Attribute_Strength;
	f32 Attribute_Dexterity;
	f32 Attribute_Constitution;
	f32 Attribute_Mind;
	
	f32 Skill_Stealth;
	f32 Skill_Mecanism;
	f32 Skill_Intuition;
	
	f32 Skill_Etheral_Link;
	f32 Skill_Object_Knowledge;
	f32 Skill_Casting;
	
	f32 Skill_Projectile;
	f32 Skill_Close_Combat;
	f32 Skill_Defense;
	
	f32 Critical_Hit; // TODO remove
	s32 AimTime; // TODO remove
	f32 life;
	f32 maxlife;
	f32 mana;
	f32 maxmana;
	s32 level;
	s16 Attribute_Redistribute;
	s16 Skill_Redistribute;
	
	f32 armor_class; // TODO remove
	f32 resist_magic; // TODO remove
	f32 resist_poison; // TODO remove
	s32 xp;
	s32 skin;
	u32 rune_flags;
	f32 damages; // TODO remove
	f32 poison;
	f32 hunger;
	SavedVec3 pos;
	SavedAnglef angle;
	SavedVec3 size;
	
	char inzone[SIZE_ID];
	char rightIO[SIZE_ID]; // TODO remove
	char leftIO[SIZE_ID]; // TODO remove
	char equipsecondaryIO[SIZE_ID]; // TODO remove
	char equipshieldIO[SIZE_ID]; // TODO remove
	char curtorch[SIZE_ID];
	s32 gold;
	s32 falling;
	
	s16 doingmagic;
	s16 Interface;
	f32 invisibility;
	s8 useanim[36]; // padding
	SavedIOPhysics physics;
	// Jump Sub-data
	u32 jumpstarttime;
	s32 jumpphase; // 0 no jump, 1 doing anticipation anim
	
	char id_inventory[SAVED_INVENTORY_BAGS][SAVED_INVENTORY_X][SAVED_INVENTORY_Y][SIZE_ID];
	s32 inventory_show[SAVED_INVENTORY_BAGS][SAVED_INVENTORY_X][SAVED_INVENTORY_Y];
	SavedMiniMap minimap[SAVED_MAX_MINIMAPS];
	char equiped[SAVED_MAX_EQUIPED][SIZE_ID];
	s32 nb_PlayerQuest;
	char anims[SAVED_MAX_ANIMS][256];
	s32 keyring_nb;
	s32 playerflags;
	char TELEPORT_TO_LEVEL[64];
	char TELEPORT_TO_POSITION[64];
	s32 TELEPORT_TO_ANGLE;
	s32 CHANGE_LEVEL_ICON;
	s16 bag;
	s16 sp_flags; // combinations of SavePlayerFlag
	SavedPrecast  precast[SAVED_MAX_PRECAST];
	s32 Global_Magic_Mode;
	s32 Nb_Mapmarkers;
	SavedVec3 LAST_VALID_POS;
	s32 padding[253];
	
};

struct ARX_CHANGELEVEL_INVENTORY_DATA_SAVE {
	
	char io[SIZE_ID];
	s32 sizex;
	s32 sizey;
	char slot_io[20][20][SIZE_ID];
	s32 slot_show[20][20];
	char initio[20][20][SIZE_ID];
	char weapon[SIZE_ID];
	char targetinfo[SIZE_ID];
	char linked_id[MAX_LINKED_SAVE][SIZE_ID];
	char stackedtarget[SAVED_MAX_STACKED_BEHAVIOR][SIZE_ID];
	
};

struct ARX_CHANGELEVEL_TIMERS_SAVE {
	
	char name[SIZE_ID];
	s32 count;
	s32 interval;
	s32 pos;
	s32 remaining;
	s32 script; // 0 = global ** 1 = local
	s32 longinfo; // TODO Remove
	s32 flags;
	
};

// each IO needs an ARX_IO_SAVE struct
// followed by 0...n ARX_VARIABLE_SAVE structs + text
// followed by 0...n ARX_TIMERS_SAVE structs
// followed by an ARX_[type]_IO_SAVE struct

struct ARX_CHANGELEVEL_SCRIPT_SAVE {
	s32 nblvar;
	u32 lastcall;
	s32 allowevents;
};

struct ARX_CHANGELEVEL_VARIABLE_SAVE {
	s32 type;
	f32 fval;
	char name[SIZE_ID];
};

// TODO Remove
struct SavedModInfo {
	
	s32 link_origin;
	SavedVec3 link_position;
	SavedVec3 scale;
	SavedAnglef rot;
	u32 flags;
};

struct IO_LINKED_DATA {
	s32 lgroup; // linked to group n° if lgroup=-1 NOLINK
	s32 lidx;
	s32 lidx2;
	SavedModInfo modinfo;
	char linked_id[SIZE_ID];
};

const size_t SAVED_MAX_ANIM_LAYERS = 4;

struct SavedAnimUse {
	
	s32 next_anim;
	s32 cur_anim;
	s16 altidx_next; // idx to alternate anims...
	s16 altidx_cur; // idx to alternate anims...
	s32 ctime;
	u32 flags;
	u32 nextflags;
	s32 lastframe;
	f32 pour;
	s32 fr;
	
	operator AnimLayer() const {
		AnimLayer a;
		a.cur_anim = NULL;
		a.altidx_cur = altidx_cur;
		a.ctime = AnimationDurationMs(ctime);
		a.flags = AnimUseType::load(flags);
		a.lastframe = lastframe;
		a.currentInterpolation = pour;
		a.currentFrame = fr;
		return a;
	}
	
	SavedAnimUse & operator=(const AnimLayer & b) {
		next_anim = -1;
		cur_anim = 0;
		altidx_next = 0;
		altidx_cur = b.altidx_cur;
		ctime = toMsi(b.ctime);
		flags = b.flags;
		nextflags = 0;
		lastframe = b.lastframe;
		pour = b.currentInterpolation;
		fr = b.currentFrame;
		return *this;
	}
	
};

struct SavedSpellcastData {
	
	s32 castingspell; // spell being casted...
	u8 symb[4]; // symbols to draw before casting...
	s16 spell_flags;
	s16 spell_level;
	s32 target;
	s32 duration;
	
	operator IO_SPELLCAST_DATA() const {
		IO_SPELLCAST_DATA a;
		a.castingspell = (castingspell < 0) ? SPELL_NONE : SpellType(castingspell); // TODO save/load enum
		ARX_STATIC_ASSERT(ARRAY_SIZE(a.symb) == 4, "array size mismatch");
		a.symb[0] = Rune(symb[0]); // TODO save/load enum
		a.symb[1] = Rune(symb[1]);
		a.symb[2] = Rune(symb[2]);
		a.symb[3] = Rune(symb[3]);
		a.spell_flags = SpellcastFlags::load(spell_flags); // TODO save/load flags
		a.spell_level = spell_level;
		a.target = EntityHandle(target); // TODO saved internum not valid after loading
		a.duration = GameDurationMs(duration); // TODO save/load time
		return a;
	}
	
	SavedSpellcastData & operator=(const IO_SPELLCAST_DATA & b) {
		castingspell = (b.castingspell == SPELL_NONE) ? -1 : b.castingspell;
		ARX_STATIC_ASSERT(ARRAY_SIZE(b.symb) == 4, "array size mismatch");
		std::copy(b.symb, b.symb + 4, symb);
		spell_flags = b.spell_flags;
		spell_level = b.spell_level;
		target = b.target.handleData();
		duration = toMsi(b.duration); // TODO save/load time
		return *this;
	}
	
};

struct SavedHalo {
	
	SavedColor color;
	f32 radius;
	u32 flags;
	s32 dynlight; // unused
	SavedVec3 offset;
	
	operator IO_HALO() const {
		IO_HALO a;
		a.color = color;
		a.radius = radius;
		a.flags = HaloFlags::load(flags); // TODO save/load flags
		a.offset = offset.toVec3();
		return a;
	}
	
	SavedHalo & operator=(const IO_HALO & b) {
		color = b.color;
		radius = b.radius;
		flags = b.flags;
		offset = b.offset;
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_IO_SAVE {
	
	s32 savesystem_type;
	s32 saveflags;
	f32 version;
	char filename[256];
	s32 ident;
	s32 ioflags;
	SavedVec3 pos;
	SavedVec3 initpos;
	SavedVec3 lastpos;
	SavedVec3 move;
	SavedVec3 lastmove;
	SavedAnglef angle;
	SavedAnglef initangle;
	f32 scale;
	u32 savetime; // unused
	f32 weight;
	
	char locname[64];
	u16 EditorFlags;
	u16 gameFlags;
	s32 material;
	s16 level; // unused
	s16 truelevel; // unused
	s32 nbtimers;
	// Script data
	s32 scriptload;
	s16 show;
	s16 collision;
	char mainevent[64];
	// Physics data
	SavedVec3 velocity; // unused
	s32 stopped;
	SavedIOPhysics physics;
	f32 original_radius;
	f32 original_height;
	// Anims data
	char anims[SAVED_MAX_ANIMS][256];
	SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS];
	// Target Info
	char id_targetinfo[SIZE_ID];
	s32 inventory; // unused
	// Group Info
	s32 system_flags;
	f32 basespeed;
	f32 speed_modif;
	f32 frameloss;
	SavedSpellcastData spellcast_data;
	
	f32 rubber;
	f32 max_durability;
	f32 durability;
	s16 poisonous;
	s16 poisonous_count;
	
	s32 nb_linked;
	IO_LINKED_DATA linked_data[MAX_LINKED_SAVE];
	
	f32 head_rot;
	
	s16 damager_damages;
	s16 nb_iogroups;
	s32 damager_type;
	
	u32 type_flags;
	char stepmaterial[128];
	char armormaterial[128];
	char weaponmaterial[128];
	char strikespeech[128];
	s16 Tweak_nb;
	s16 padd;
	SavedHalo halo;
	s8 secretvalue;
	char paddd[3];
	char shop_category[128];
	f32 shop_multiply;
	s32 aflags;
	f32 ignition;
	char inventory_skin[128];
	// TO ADD:
	char usepath_name[SIZE_ID];
	u32 usepath_starttime;
	u32 usepath_curtime;
	s32 usepath_aupflags;
	SavedVec3 usepath_initpos;
	s32 usepath_lastWP;
	s32 padddd[64]; // new...
	
};

struct SavedBehaviour {
	
	s32 exist;
	u32 behavior;
	f32 behavior_param;
	s32 tactics; // TODO remove
	s32 target;
	s32 movemode;
	SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS];
	
	operator IO_BEHAVIOR_DATA() const {
		IO_BEHAVIOR_DATA a;
		a.exist = exist;
		a.behavior = Behaviour::load(behavior); // TODO save/load flags
		a.behavior_param = behavior_param;
		a.target = EntityHandle(target);
		a.movemode = MoveMode(movemode); // TODO save/load enum
		ARX_STATIC_ASSERT(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS, "array size mismatch");
		std::copy(animlayer, animlayer + SAVED_MAX_ANIM_LAYERS, a.animlayer);
		return a;
	}
	
	SavedBehaviour & operator=(const IO_BEHAVIOR_DATA & b) {
		exist = b.exist;
		behavior = b.behavior;
		behavior_param = b.behavior_param;
		tactics = 0;
		target = b.target.handleData();
		movemode = b.movemode;
		ARX_STATIC_ASSERT(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS, "array size mismatch");
		std::copy(b.animlayer, b.animlayer + SAVED_MAX_ANIM_LAYERS, animlayer);
		return *this;
	}
	
};

struct SavedPathfindTarget {
	
	u32 padding[4];
	s32 truetarget;
	
	operator IO_PATHFIND() const {
		IO_PATHFIND a;
		a.flags = 0;
		a.listnb = a.listpos = a.pathwait = 0;
		a.list = NULL;
		a.truetarget = EntityHandle(truetarget);
		return a;
	}
	
	SavedPathfindTarget & operator=(const IO_PATHFIND & b) {
		padding[0] = padding[1] = padding[2] = padding[3] = 0;
		truetarget = b.truetarget.handleData();
		return *this;
	}
	
};

const size_t SAVED_MAX_EXTRA_ROTATE = 4;

inline ObjVertGroup saved_toObjGroup(short value) {
	return ObjVertGroup(value);
}

inline short saved_fromObjGroup(ObjVertGroup value) {
	return value.handleData();
}

struct SavedExtraRotate {
	
	s32 flags;
	s16 group_number[SAVED_MAX_EXTRA_ROTATE];
	SavedAnglef group_rotate[SAVED_MAX_EXTRA_ROTATE];
	
	operator EERIE_EXTRA_ROTATE() const {
		EERIE_EXTRA_ROTATE a;
		ARX_STATIC_ASSERT(SAVED_MAX_EXTRA_ROTATE == MAX_EXTRA_ROTATE, "array size mismatch");
		std::transform(group_number, group_number + SAVED_MAX_EXTRA_ROTATE, a.group_number, saved_toObjGroup);
		std::copy(group_rotate, group_rotate + SAVED_MAX_EXTRA_ROTATE, a.group_rotate);
		return a;
	}
	
	SavedExtraRotate & operator=(const EERIE_EXTRA_ROTATE & b) {
		flags = 0;
		ARX_STATIC_ASSERT(SAVED_MAX_EXTRA_ROTATE == MAX_EXTRA_ROTATE, "array size mismatch");
		std::transform(b.group_number, b.group_number + SAVED_MAX_EXTRA_ROTATE, group_number, saved_fromObjGroup);
		std::copy(b.group_rotate, b.group_rotate + SAVED_MAX_EXTRA_ROTATE, group_rotate);
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_NPC_IO_SAVE {
	
	f32 maxlife;
	f32 life;
	f32 maxmana;
	f32 mana;
	s32 reachedtarget;
	char id_weapon[SIZE_ID];
	s32 detect;
	s32 movemode;
	f32 armor_class;
	f32 absorb;
	f32 damages;
	f32 tohit;
	f32 aimtime;
	u32 behavior;
	f32 behavior_param;
	s32 tactics; // TODO remove
	s32 xpvalue;
	s32 cut;
	f32 moveproblem;
	s32 weapontype;
	s32 weaponinhand;
	s32 fightdecision;
	char padding[256];
	f32 look_around_inc;
	u32 collid_time; // TODO remove
	s32 collid_state; // TODO remove
	f32 speakpitch;
	f32 lastmouth;
	SavedBehaviour stacked[SAVED_MAX_STACKED_BEHAVIOR];
	char weapon[SIZE_ID];
	
	f32 critical;
	f32 reach;
	f32 backstab_skill;
	f32 poisonned;
	u8 resist_poison;
	u8 resist_magic;
	u8 resist_fire;
	u8 padd;
	
	s16 strike_time; // TODO Remove
	s16 walk_start_time;
	s32 aiming_start;
	s32 npcflags;
	SavedPathfindTarget pathfind;
	SavedExtraRotate ex_rotate;
	u32 blood_color;
	char stackedtarget[SAVED_MAX_STACKED_BEHAVIOR][SIZE_ID];
	f32 fDetect;
	s16 cuts;
	s16 spadd;
	s32 paddd[63]; // new...
	
};

struct SavedEquipItemElement {
	
	f32 value;
	s16 flags;
	s16 special;
	
	operator IO_EQUIPITEM_ELEMENT() const {
		IO_EQUIPITEM_ELEMENT a;
		a.value = value;
		a.flags = EquipmentModifierFlags::load(flags); // TODO save/load flags
		a.special = EquipmentModifiedSpecialType(special); // TODO save/load enum
		return a;
	}
	
	SavedEquipItemElement & operator=(const IO_EQUIPITEM_ELEMENT & b) {
		value = b.value;
		flags = s16(b.flags); // TODO save/load flags
		special = s16(b.special); // TODO save/load enum
		return *this;
	}
	
};

const size_t SAVED_IO_EQUIPITEM_ELEMENT_Number = 29;

struct SavedEquipItem {
	
	SavedEquipItemElement elements[SAVED_IO_EQUIPITEM_ELEMENT_Number];
	
	operator IO_EQUIPITEM() const {
		IO_EQUIPITEM a;
		ARX_STATIC_ASSERT(SAVED_IO_EQUIPITEM_ELEMENT_Number == IO_EQUIPITEM_ELEMENT_Number,
		                  "array size mismatch");
		std::copy(elements, elements + SAVED_IO_EQUIPITEM_ELEMENT_Number, a.elements);
		return a;
	}
	
	SavedEquipItem & operator=(const IO_EQUIPITEM & b) {
		ARX_STATIC_ASSERT(SAVED_IO_EQUIPITEM_ELEMENT_Number == IO_EQUIPITEM_ELEMENT_Number,
		                  "array size mismatch");
		std::copy(b.elements, b.elements + SAVED_IO_EQUIPITEM_ELEMENT_Number, elements);
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_ITEM_IO_SAVE {
	s32 price;
	s16 maxcount;
	s16 count;
	char food_value;
	char stealvalue;
	s16 playerstacksize;
	s16 LightValue;
	SavedEquipItem equipitem;
	s32 padd[64]; // new...
};

struct ARX_CHANGELEVEL_FIX_IO_SAVE {
	s8 trapvalue;
	char padd[3];
	s32 paddd[64]; // new...
};

struct ARX_CHANGELEVEL_MARKER_IO_SAVE {
	s32 dummy;
};

struct SavedRect {
	
	s32 left;
	s32 top;
	s32 right;
	s32 bottom;
	
	operator Rect() const {
		Rect a;
		a.left = left;
		a.top = top;
		a.right = right;
		a.bottom = bottom;
		return a;
	}
	
	SavedRect & operator=(const Rect & b) {
		left = b.left;
		top = b.top;
		right = b.right;
		bottom = b.bottom;
		return *this;
	}
	
};

struct SavedTweakerInfo {
	
	static const size_t NAME_SIZE = 256;
	
	char filename[NAME_SIZE];
	char skintochange[NAME_SIZE];
	char skinchangeto[NAME_SIZE];
	
	/* implicit */ SavedTweakerInfo(const IO_TWEAKER_INFO & b) {
		arx_assert(b.filename.string().length() <= sizeof(filename));
		util::storeString(filename, b.filename.string());
		arx_assert(b.skintochange.length() <= sizeof(skintochange));
		util::storeString(skintochange, b.skintochange);
		arx_assert(b.skinchangeto.filename().length() <= sizeof(skinchangeto));
		util::storeString(skinchangeto, b.skinchangeto.string());
	}
	
};

struct SavedGroupData {
	
	static const size_t NAME_SIZE = 64;
	
	char name[NAME_SIZE];
	
};

struct SavedTweakInfo {
	
	static const size_t PARAM_SIZE = 256;
	
	s32 type;
	char param1[PARAM_SIZE];
	char param2[PARAM_SIZE];
	
	/* implicit */ SavedTweakInfo(const TWEAK_INFO & b) {
		type = b.type;
		arx_assert(b.param1.string().length() <= PARAM_SIZE);
		util::storeString(param1, b.param1.string());
		arx_assert(b.param2.string().length() <= PARAM_SIZE);
		util::storeString(param2, b.param2.string());
	}
	
};

struct SavedMatrix {
	
	f32 _11, _12, _13, _14;
	f32 _21, _22, _23, _24;
	f32 _31, _32, _33, _34;
	f32 _41, _42, _43, _44;
	
	operator glm::mat4x4() const {
		glm::mat4x4 a;
		a[0][0] = _11, a[0][1] = _12, a[0][2] = _13, a[0][3] = _14;
		a[1][0] = _21, a[1][1] = _22, a[1][2] = _23, a[1][3] = _24;
		a[2][0] = _31, a[2][1] = _32, a[2][2] = _33, a[2][3] = _34;
		a[3][0] = _41, a[3][1] = _42, a[3][2] = _43, a[3][3] = _44;
		return a;
	}
	
	SavedMatrix & operator=(const glm::mat4x4 & b) {
		_11 = b[0][0], _12 = b[0][1], _13 = b[0][2], _14 = b[0][3];
		_21 = b[1][0], _22 = b[1][1], _23 = b[1][2], _24 = b[1][3];
		_31 = b[2][0], _32 = b[2][1], _33 = b[2][2], _34 = b[2][3];
		_41 = b[3][0], _42 = b[3][1], _43 = b[3][2], _44 = b[3][3];
		return *this;
	}
	
};

#define CAM_SUBJVIEW 0
#define CAM_TOPVIEW  1

struct SavedCamera {
	
	SavedVec3 pos;
	f32 ycos; // TODO Remove
	f32 ysin; // TODO Remove
	f32 xsin; // TODO Remove
	f32 xcos; // TODO Remove
	f32 use_focal1; // TODO Remove
	f32 xmod; // TODO Remove
	f32 ymod; // TODO Remove
	f32 zmod; // TODO Remove
	
	SavedVec3 pos2; // TODO Remove
	f32 Ycos; // TODO Remove
	f32 Ysin; // TODO Remove
	f32 Xcos; // TODO Remove
	f32 Xsin; // TODO Remove
	f32 Zcos; // TODO Remove
	f32 Zsin; // TODO Remove
	f32 focal;
	f32 use_focal; // TODO Remove
	f32 Zmul; // TODO Remove
	f32 posleft; // TODO Remove
	f32 postop; // TODO Remove
	
	f32 xmod2; // TODO Remove
	f32 ymod2; // TODO Remove
	SavedMatrix matrix; // TODO Remove
	SavedAnglef angle;
	
	SavedVec3 d_pos; // TODO Remove
	SavedAnglef d_angle; // TODO Remove
	SavedVec3 lasttarget;
	SavedVec3 lastpos; // TODO Remove
	SavedVec3 translatetarget;
	s32 lastinfovalid;
	SavedVec3 norm; // TODO Remove
	SavedColor fadecolor; // TODO Remove
	SavedRect clip; // TODO Remove
	f32 clipz0; // TODO Remove
	f32 clipz1; // TODO Remove
	s32 centerx; // TODO Remove
	s32 centery; // TODO Remove
	
	f32 smoothing;
	f32 AddX; // TODO Remove
	f32 AddY; // TODO Remove
	s32 Xsnap; // TODO Remove
	s32 Zsnap; // TODO Remove
	f32 Zdiv; // TODO Remove
	
	s32 clip3D; // TODO Remove
	s32 type; // TODO Remove
	u32 bkgcolor; // TODO Remove
	s32 nbdrawn; // TODO Remove
	f32 cdepth;
	
	SavedAnglef size; // TODO Remove
	
	operator Camera() const {
		
		Camera a;
		
		a.m_pos = pos.toVec3();
		a.focal = focal;
		a.angle = angle;
		a.cdepth = cdepth;
		
		return a;
	}
	
	SavedCamera & operator=(const Camera & b) {
		
		pos = b.m_pos;
		pos2 = b.m_pos;
		lastpos = b.m_pos;
		focal = b.focal;
		angle = b.angle;
		cdepth = b.cdepth;
		
		ycos = 0.f;
		ysin = 0.f;
		xsin = 0.f;
		xcos = 0.f;
		use_focal1 = 0.f;
		xmod = 0.f;
		ymod = 0.f;
		zmod = 0.f;
		
		Ycos = 0.f;
		Ysin = 0.f;
		Xcos = 0.f;
		Xsin = 0.f;
		Zcos = 0.f;
		Zsin = 0.f;
		
		use_focal = 0.f;
		
		posleft = 0.f;
		postop = 0.f;
		
		Zmul = 0.f;
		
		xmod2 = 0.f;
		ymod2 = 0.f;
		matrix = glm::mat4x4(1.f);
		
		d_pos = Vec3f(0.f);
		d_angle = Anglef();
		norm = Vec3f(0.f);
		fadecolor = Color3f::black;
		clip = Rect::ZERO;
		clipz0 = 0.0f;
		clipz1 = 0.0f;
		centerx = 0;
		centery = 0;
		
		AddX = 0.f;
		AddY = 0.f;
		Xsnap = 0;
		Zsnap = 0;
		Zdiv = 0.f;
		
		clip3D = 0;
		type = CAM_SUBJVIEW;
		bkgcolor = Color::none.toBGRA().t;
		nbdrawn = 0;
		
		size = Anglef();
		
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_CAMERA_IO_SAVE {
	
	SavedCamera cam;
	
	operator IO_CAMDATA() const {
		
		IO_CAMDATA a;
		
		a.cam = cam;
		
		a.lasttarget = cam.lasttarget.toVec3();
		a.translatetarget = cam.translatetarget.toVec3();
		a.lastinfovalid = cam.lastinfovalid != 0;
		a.smoothing = cam.smoothing;
		
		return a;
	}
	
	ARX_CHANGELEVEL_CAMERA_IO_SAVE & operator=(const IO_CAMDATA & b) {
		
		cam = b.cam;
		
		cam.lasttarget = b.lasttarget;
		cam.translatetarget = b.translatetarget;
		cam.lastinfovalid = b.lastinfovalid;
		cam.smoothing = b.smoothing;
		
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_PLAYER_LEVEL_DATA {
	f32 version;
	char name[256];
	s32 level;
	s64 time; // 32-bit before Arx Libertatis 1.2
	s64 playthroughStart; // new in Arx Libertatis 1.2
	u64 playthroughId; // new in Arx Libertatis 1.2
	u64 oldestALVersion; // new in Arx Libertatis 1.2
	u64 newestALVersion; // new in Arx Libertatis 1.2
	u64 lastALVersion; // new in Arx Libertatis 1.2
	char lastEngineVersion[64]; // new in Arx Libertatis 1.2
	s32 padd[5];
};


#pragma pack(pop)


#endif // ARX_SCENE_SAVEFORMAT_H
