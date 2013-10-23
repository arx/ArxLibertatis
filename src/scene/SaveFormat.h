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

#ifndef ARX_SCENE_SAVEFORMAT_H
#define ARX_SCENE_SAVEFORMAT_H

#include <algorithm>
#include <cstring>

#include <boost/static_assert.hpp>

#include "gui/MiniMap.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "graphics/GraphicsFormat.h"
#include "graphics/GraphicsModes.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/MeshManipulation.h"
#include "platform/Platform.h"

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
	SYSTEM_FLAG_TWEAKER_INFO  = (1<<0),
	SYSTEM_FLAG_INVENTORY     = (1<<1),
	SYSTEM_FLAG_EQUIPITEMDATA = (1<<2),
	SYSTEM_FLAG_USEPATH       = (1<<3)
};

enum SavePlayerFlag {
	SP_MAX  = (1<<0),
	SP_RF   = (1<<2),
	SP_WEP  = (1<<3),
	SP_MR   = (1<<4),
	SP_ARM1 = (1<<5),
	SP_ARM2 = (1<<6),
	SP_ARM3 = (1<<7),
	SP_SP   = (1<<8),
	SP_SP2  = (1<<9)
};


#pragma pack(push,1)


const size_t SAVED_KEYRING_SLOT_SIZE = 64;
const size_t MAX_LINKED_SAVE = 16;
const size_t SIZE_ID = 64;
const size_t SAVED_MAX_STACKED_BEHAVIOR = 5;

struct ARX_VARIABLE_SAVE {
	s32 type;
	f32 fval;
	char name[SIZE_ID];
};

struct ARX_SCRIPT_SAVE {
	s32 nblvar;
	u32 lastcall;
	s32 allowevents;
};

struct SavedGlobalMods {
	
	s32 flags;
	SavedColor depthcolor;
	f32 zclip;
	char padding[136];
	
	inline SavedGlobalMods & operator=(const GLOBAL_MODS & b) {
		flags = b.flags;
		depthcolor = b.depthcolor;
		zclip = b.zclip;
		memset(padding, 0, sizeof(padding));
		return *this;
	}
	
	inline operator GLOBAL_MODS() const {
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
	
	/* implicit */ SavedMapMarkerData(const MiniMap::MapMarkerData &b) {
		x = b.m_x;
		y = b.m_y;
		lvl = b.m_lvl;
		arx_assert(STRING_SIZE > b.m_name.length());
		strncpy(name, b.m_name.c_str(), STRING_SIZE);
	}
	
};

struct SavedCylinder {
	
	SavedVec3 origin;
	f32 radius;
	f32 height;
	
	inline operator EERIE_CYLINDER() const {
		EERIE_CYLINDER a;
		a.origin = origin.toVec3(), a.radius = radius, a.height = height;
		return a;
	}
	
	inline SavedCylinder & operator=(const EERIE_CYLINDER & b) {
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
	
	inline operator IO_PHYSICS() const {
		IO_PHYSICS a;
		a.cyl = cyl;
		a.startpos = startpos.toVec3();
		a.targetpos = targetpos.toVec3();
		a.velocity = velocity.toVec3();
		a.forces = forces.toVec3();
		return a;
	}
	
	inline SavedIOPhysics & operator=(const IO_PHYSICS & b) {
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
	
	inline operator MiniMap::MiniMapData() const {
		MiniMap::MiniMapData a;
		a.m_texContainer = NULL;
		a.m_offsetX = offsetx;
		a.m_offsetY = offsety;
		a.m_ratioX = xratio;
		a.m_ratioY = yratio;
		a.m_width = width;
		a.m_height = height;
		BOOST_STATIC_ASSERT(SavedMiniMap::MAX_X == MINIMAP_MAX_X);
		BOOST_STATIC_ASSERT(SavedMiniMap::MAX_Z == MINIMAP_MAX_Z);
		std::copy(&revealed[0][0], &revealed[0][0] + (SavedMiniMap::MAX_X * SavedMiniMap::MAX_Z), &a.m_revealed[0][0]);
		return a;
	}
	
	inline SavedMiniMap & operator=(const MiniMap::MiniMapData & b) {
		padding = 0;
		offsetx = b.m_offsetX;
		offsety = b.m_offsetY;
		xratio = b.m_ratioX;
		yratio = b.m_ratioY;
		width = b.m_width;
		height = b.m_height;
		BOOST_STATIC_ASSERT(SavedMiniMap::MAX_X == MINIMAP_MAX_X);
		BOOST_STATIC_ASSERT(SavedMiniMap::MAX_Z == MINIMAP_MAX_Z);
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
	
	inline operator PRECAST_STRUCT() const {
		PRECAST_STRUCT a;
		a.typ = (typ < 0) ? SPELL_NONE : (Spell)typ; // TODO save/load enum
		a.level = level;
		a.launch_time = launch_time;
		a.flags = SpellcastFlags::load(flags); // TODO save/load flags
		a.duration = duration;
		return a;
	}
	
	inline SavedPrecast & operator=(const PRECAST_STRUCT & b) {
		typ = (b.typ == SPELL_NONE) ? -1 : b.typ;
		level = b.level;
		launch_time = b.launch_time;
		flags = b.flags;
		duration = b.duration;
		return *this;
	}
	
};

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
	
	f32 Critical_Hit;
	s32 AimTime;
	f32 life;
	f32 maxlife;
	f32 mana;
	f32 maxmana;
	s32 level;
	s16 Attribute_Redistribute;
	s16 Skill_Redistribute;
	
	f32 armor_class;
	f32 resist_magic;
	f32 resist_poison;
	s32 xp;
	s32 skin;
	u32 rune_flags;
	f32 damages;
	f32 poison;
	f32 hunger;
	SavedVec3 pos;
	SavedAnglef angle;
	SavedVec3 size;
	
	char inzone[SIZE_ID];
	char rightIO[SIZE_ID];
	char leftIO[SIZE_ID];
	char equipsecondaryIO[SIZE_ID];
	char equipshieldIO[SIZE_ID];
	char curtorch[SIZE_ID];
	s32 gold;
	s32 falling;
	
	s16	doingmagic;
	s16	Interface;
	f32 invisibility;
	s8 useanim[36]; // padding
	SavedIOPhysics physics;
	// Jump Sub-data
	u32 jumpstarttime;
	s32 jumpphase;	// 0 no jump, 1 doing anticipation anim
	
	char id_inventory[3][SAVED_INVENTORY_X][SAVED_INVENTORY_Y][SIZE_ID];
	s32 inventory_show[3][SAVED_INVENTORY_X][SAVED_INVENTORY_Y];
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
	/// limit...
	char weapon[SIZE_ID];
	char targetinfo[SIZE_ID];
	char linked_id[MAX_LINKED_SAVE][SIZE_ID];
	char stackedtarget[SAVED_MAX_STACKED_BEHAVIOR][SIZE_ID];
	
};

struct ARX_CHANGELEVEL_TIMERS_SAVE {
	
	char name[SIZE_ID];
	s32 times;
	s32 msecs;
	s32 pos;
	s32 tim;
	s32 script; // 0 = global ** 1 = local
	s32 longinfo;
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
	
	inline operator ANIM_USE() const {
		ANIM_USE a;
		a.next_anim = NULL;
		a.cur_anim = NULL;
		a.altidx_next = altidx_next;
		a.altidx_cur = altidx_cur;
		a.ctime = ctime;
		a.flags = AnimUseType::load(flags);
		a.nextflags = AnimUseType::load(nextflags);
		a.lastframe = lastframe;
		a.pour = pour;
		a.fr = fr;
		return a;
	}
	
	inline SavedAnimUse & operator=(const ANIM_USE & b) {
		next_anim = 0;
		cur_anim = 0;
		altidx_next = b.altidx_next;
		altidx_cur = b.altidx_cur;
		ctime = b.ctime;
		flags = b.flags;
		nextflags = b.nextflags;
		lastframe = b.lastframe;
		pour = b.pour;
		fr = b.fr;
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
	
	inline operator IO_SPELLCAST_DATA() const {
		IO_SPELLCAST_DATA a;
		a.castingspell = (castingspell < 0) ? SPELL_NONE : (Spell)castingspell; // TODO save/load enum
		BOOST_STATIC_ASSERT(ARRAY_SIZE(a.symb) == 4);
		a.symb[0] = (Rune)symb[0]; // TODO save/load enum
		a.symb[1] = (Rune)symb[1];
		a.symb[2] = (Rune)symb[2];
		a.symb[3] = (Rune)symb[3];
		a.spell_flags = SpellcastFlags::load(spell_flags); // TODO save/load flags
		a.spell_level = spell_level;
		a.target = target; // TODO saved internum not valid after loading
		a.duration = duration;
		return a;
	}
	
	inline SavedSpellcastData & operator=(const IO_SPELLCAST_DATA & b) {
		castingspell = (b.castingspell == SPELL_NONE) ? -1 : b.castingspell;
		BOOST_STATIC_ASSERT(ARRAY_SIZE(b.symb) == 4);
		std::copy(b.symb, b.symb + 4, symb);
		spell_flags = b.spell_flags;
		spell_level = b.spell_level;
		target = b.target;
		duration = b.duration;
		return *this;
	}
	
};

struct SavedHalo {
	
	SavedColor color;
	f32 radius;
	u32 flags;
	s32 dynlight;
	SavedVec3 offset;
	
	inline operator IO_HALO() const {
		IO_HALO a;
		a.color = color;
		a.radius = radius;
		a.flags = HaloFlags::load(flags); // TODO save/load flags
		a.dynlight = dynlight;
		a.offset = offset.toVec3();
		return a;
	}
	
	inline SavedHalo & operator=(const IO_HALO & b) {
		color = b.color;
		radius = b.radius;
		flags = b.flags;
		dynlight = b.dynlight;
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
	s32 ioflags;//type;
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
	SavedVec3 velocity;
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
	f32 frameloss; // TODO remove
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
	s32 tactics; // 0=none ; 1=side ; 2=side+back
	s32 target;
	s32 movemode;
	SavedAnimUse animlayer[SAVED_MAX_ANIM_LAYERS];
	
	inline operator IO_BEHAVIOR_DATA() const {
		IO_BEHAVIOR_DATA a;
		a.exist = exist;
		a.behavior = Behaviour::load(behavior); // TODO save/load flags
		a.behavior_param = behavior_param;
		a.tactics = tactics;
		a.target = target;
		a.movemode = (MoveMode)movemode; // TODO save/load enum
		BOOST_STATIC_ASSERT(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
		std::copy(animlayer, animlayer + SAVED_MAX_ANIM_LAYERS, a.animlayer);
		return a;
	}

	inline SavedBehaviour & operator=(const IO_BEHAVIOR_DATA & b) {
		exist = b.exist;
		behavior = b.behavior;
		behavior_param = b.behavior_param;
		tactics = b.tactics;
		target = b.target;
		movemode = b.movemode;
		BOOST_STATIC_ASSERT(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
		std::copy(b.animlayer, b.animlayer + SAVED_MAX_ANIM_LAYERS, animlayer);
		return *this;
	}
	
};

struct SavedPathfindTarget {
	
	u32 padding[4];
	s32 truetarget;
	
	inline operator IO_PATHFIND() const {
		IO_PATHFIND a;
		a.flags = 0;
		a.listnb = a.listpos = a.pathwait = 0;
		a.list = NULL;
		a.truetarget = truetarget;
		return a;
	}
	
	inline SavedPathfindTarget & operator=(const IO_PATHFIND & b) {
		padding[0] = padding[1] = padding[2] = padding[3] = 0;
		truetarget = b.truetarget;
		return *this;
	}
	
};

const size_t SAVED_MAX_EXTRA_ROTATE = 4;

struct SavedExtraRotate {
	
	s32 flags;
	s16 group_number[SAVED_MAX_EXTRA_ROTATE];
	SavedAnglef group_rotate[SAVED_MAX_EXTRA_ROTATE];
	
	inline operator EERIE_EXTRA_ROTATE() const {
		EERIE_EXTRA_ROTATE a;
		a.flags = ExtraRotateFlags::load(flags); // TODO save/load flags
		BOOST_STATIC_ASSERT(SAVED_MAX_EXTRA_ROTATE == MAX_EXTRA_ROTATE);
		std::copy(group_number, group_number + SAVED_MAX_EXTRA_ROTATE, a.group_number);
		std::copy(group_rotate, group_rotate + SAVED_MAX_EXTRA_ROTATE, a.group_rotate);
		return a;
	}
	
	inline SavedExtraRotate & operator=(const EERIE_EXTRA_ROTATE & b) {
		flags = b.flags;
		BOOST_STATIC_ASSERT(SAVED_MAX_EXTRA_ROTATE == MAX_EXTRA_ROTATE);
		std::copy(b.group_number, b.group_number + SAVED_MAX_EXTRA_ROTATE, group_number);
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
	s32 tactics;
	s32 xpvalue;
	s32 cut;
	f32 moveproblem;
	s32 weapontype;
	s32 weaponinhand;
	s32 fightdecision;
	char padding[256];
	f32 look_around_inc;
	u32 collid_time;
	s32 collid_state;
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
	
	s16 strike_time;
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
	
	inline operator IO_EQUIPITEM_ELEMENT() const {
		IO_EQUIPITEM_ELEMENT a;
		a.value = value;
		a.flags = EquipmentModifierFlags::load(flags); // TODO save/load flags
		a.special = EquipmentModifiedSpecialType(special); // TODO save/load enum
		return a;
	}
	
	inline SavedEquipItemElement & operator=(const IO_EQUIPITEM_ELEMENT & b) {
		value = b.value;
		flags = b.flags;
		special = b.special;
		return *this;
	}
	
};

const size_t SAVED_IO_EQUIPITEM_ELEMENT_Number = 29;

struct SavedEquipItem {
	
	SavedEquipItemElement elements[SAVED_IO_EQUIPITEM_ELEMENT_Number];
	
	inline operator IO_EQUIPITEM() const {
		IO_EQUIPITEM a;
		BOOST_STATIC_ASSERT(SAVED_IO_EQUIPITEM_ELEMENT_Number == IO_EQUIPITEM_ELEMENT_Number);
		std::copy(elements, elements + SAVED_IO_EQUIPITEM_ELEMENT_Number, a.elements);
		return a;
	}
	
	inline SavedEquipItem & operator=(const IO_EQUIPITEM & b) {
		BOOST_STATIC_ASSERT(SAVED_IO_EQUIPITEM_ELEMENT_Number == IO_EQUIPITEM_ELEMENT_Number);
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
	
	inline operator Rect() const {
		Rect a;
		a.left = left;
		a.top = top;
		a.right = right;
		a.bottom = bottom;
		return a;
	}
	
	inline SavedRect & operator=(const Rect & b) {
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
		strncpy(filename, b.filename.string().c_str(), sizeof(filename));
		arx_assert(b.skintochange.length() <= sizeof(skintochange));
		strncpy(skintochange, b.skintochange.c_str(), sizeof(skintochange));
		arx_assert(b.skinchangeto.filename().length() <= sizeof(skinchangeto));
		strncpy(skinchangeto, b.skinchangeto.string().c_str(), sizeof(skinchangeto));
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
		strncpy(param1, b.param1.string().c_str(), sizeof(param1));
		arx_assert(b.param2.string().length() <=PARAM_SIZE);
		strncpy(param2, b.param2.string().c_str(), sizeof(param2));
	}
	
};

struct SavedMatrix {
	
	f32 _11, _12, _13, _14;
	f32 _21, _22, _23, _24;
	f32 _31, _32, _33, _34;
	f32 _41, _42, _43, _44;
	
	inline operator EERIEMATRIX() const {
		EERIEMATRIX a;
		a._11 = _11, a._12 = _12, a._13 = _13, a._14 = _14;
		a._21 = _21, a._22 = _22, a._23 = _23, a._24 = _24;
		a._31 = _31, a._32 = _32, a._33 = _33, a._34 = _34;
		a._41 = _41, a._42 = _42, a._43 = _43, a._44 = _44;
		return a;
	}
	
	inline SavedMatrix & operator=(const EERIEMATRIX & b) {
		_11 = b._11, _12 = b._12, _13 = b._13, _14 = b._14;
		_21 = b._21, _22 = b._22, _23 = b._23, _24 = b._24;
		_31 = b._31, _32 = b._32, _33 = b._33, _34 = b._34;
		_41 = b._41, _42 = b._42, _43 = b._43, _44 = b._44;
		return *this;
	}
	
};

struct SavedTransform {
	
	SavedVec3 pos;
	f32 ycos;
	f32 ysin;
	f32 xsin;
	f32 xcos;
	f32 use_focal; //TODO Remove
	f32 xmod;
	f32 ymod;
	f32 zmod;
	
	inline operator EERIE_TRANSFORM() const {
		EERIE_TRANSFORM a;
		a.pos = pos.toVec3();
		a.ycos = ycos, a.ysin = ysin, a.xsin = xsin, a.xcos = xcos;
		a.mod.x = xmod, a.mod.y = ymod;
		return a;
	}
	
	inline SavedTransform & operator=(const EERIE_TRANSFORM & b) {
		pos = b.pos;
		ycos = b.ycos, ysin = b.ysin, xsin = b.xsin, xcos = b.xcos;
		use_focal = 0.f;
		xmod = b.mod.x, ymod = b.mod.y, zmod = 0.f;
		return *this;
	}
	
};

struct SavedCamera {
	
	SavedTransform transform;
	SavedVec3 pos;
	f32 Ycos;
	f32 Ysin;
	f32 Xcos;
	f32 Xsin;
	f32 Zcos;
	f32 Zsin;
	f32 focal;
	f32 use_focal; //TODO Remove
	f32 Zmul; //TODO Remove
	f32 posleft; //TODO Remove
	f32 postop; //TODO Remove
	
	f32 xmod;
	f32 ymod;
	SavedMatrix matrix; //TODO Remove
	SavedAnglef angle;
	
	SavedVec3 d_pos;
	SavedAnglef d_angle;
	SavedVec3 lasttarget;
	SavedVec3 lastpos;
	SavedVec3 translatetarget;
	s32 lastinfovalid;
	SavedVec3 norm; //TODO Remove
	SavedColor fadecolor;
	SavedRect clip;
	f32 clipz0; //TODO Remove
	f32 clipz1; //TODO Remove
	s32 centerx;
	s32 centery;
	
	f32 smoothing;
	f32 AddX;
	f32 AddY;
	s32 Xsnap; //TODO Remove
	s32 Zsnap; //TODO Remove
	f32 Zdiv; //TODO Remove
	
	s32 clip3D;
	s32 type; //TODO Remove
	s32 bkgcolor;
	s32 nbdrawn; //TODO Remove
	f32 cdepth;
	
	SavedAnglef size;
	
	inline operator EERIE_CAMERA() const {
		
		EERIE_CAMERA a;
		
		a.orgTrans = transform;
		a.orgTrans.zcos = Zcos;
		a.orgTrans.zsin = Zsin;
		a.focal = focal;
		
		a.angle = angle;
		
		a.d_pos = d_pos.toVec3(), a.d_angle = d_angle;
		a.lasttarget = lasttarget.toVec3(), a.lastpos = lastpos.toVec3();
		a.translatetarget = translatetarget.toVec3();
		a.lastinfovalid = lastinfovalid != 0;
		a.fadecolor = fadecolor, a.clip = clip;
		a.center = Vec2i(centerx, centery);
		
		a.smoothing = smoothing;
		
		a.bkgcolor = Color::fromBGRA(bkgcolor);
		a.cdepth = cdepth;
		
		a.size = size;
		
		return a;
	}
	
	inline SavedCamera & operator=(const EERIE_CAMERA & b) {
		
		transform = b.orgTrans;

		//TODO Remove
		pos = b.orgTrans.pos;
		Ycos = b.orgTrans.ycos, Ysin = b.orgTrans.ysin;
		Xcos = b.orgTrans.xcos, Xsin = b.orgTrans.xsin;
		Zcos = b.orgTrans.zcos, Zsin = b.orgTrans.zsin;

		use_focal = 0.f;

		posleft = b.orgTrans.mod.x;
		postop  = b.orgTrans.mod.y;

		focal = b.focal;
		Zmul = 0.f;


		xmod = 0.f, ymod = 0.f;
		matrix = EERIEMATRIX();
		angle = b.angle;
		
		d_pos = b.d_pos, d_angle = b.d_angle;
		lasttarget = b.lasttarget, lastpos = b.lastpos;
		translatetarget = b.translatetarget;
		lastinfovalid = b.lastinfovalid;
		norm = Vec3f(0,0,0); //TODO Remove
		fadecolor = b.fadecolor, clip = b.clip;
		clipz0 = 0.0f, clipz1 = 0.0f;
		centerx = b.center.x, centery = b.center.y;
		
		smoothing = b.smoothing;
		AddX = 0.f, AddY = 0.f;
		Xsnap = 0;
		Zsnap = 0;
		Zdiv = 0.f;
		
		clip3D = 0;
		type = CAM_SUBJVIEW;
		bkgcolor = b.bkgcolor.toBGRA();
		nbdrawn = 0;
		cdepth = b.cdepth;
		
		size = b.size;
		
		return *this;
	}
	
};

struct ARX_CHANGELEVEL_CAMERA_IO_SAVE {
	SavedCamera cam;
};

struct ARX_CHANGELEVEL_PLAYER_LEVEL_DATA {
	f32 version;
	char name[256];
	s32 level;
	u32 time;
	s32 padd[32];
};


#pragma pack(pop)


#endif // ARX_SCENE_SAVEFORMAT_H
