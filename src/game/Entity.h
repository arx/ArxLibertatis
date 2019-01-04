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

#ifndef ARX_GAME_ENTITY_H
#define ARX_GAME_ENTITY_H

#include <stddef.h>
#include <set>
#include <string>
#include <vector>

#include <boost/container/flat_set.hpp>

#include "animation/Animation.h"
#include "audio/AudioTypes.h"
#include "game/Damage.h" // TODO needed for DamageType
#include "game/EntityId.h"
#include "game/Spells.h" // TODO needed for Spell, Rune, SpellcastFlags
#include "graphics/Color.h"
#include "graphics/BaseGraphicsTypes.h"
#include "io/resource/ResourcePath.h"
#include "math/Vector.h"
#include "math/Angle.h"
#include "scene/Light.h"
#include "script/Script.h" // TODO remove this
#include "util/Flags.h"

class TextureContainer;
struct ANIM_HANDLE;
struct ARX_PATH;
struct ARX_USE_PATH;
struct EERIE_3DOBJ;
struct INVENTORY_DATA;
struct IO_CAMDATA;
struct IO_FIXDATA;
struct IO_ITEMDATA;
struct IO_NPCDATA;
struct TWEAK_INFO;

static const size_t MAX_ANIMS = 200; // max loadable anims per character
static const size_t MAX_ANIM_LAYERS = 4;
static const float BASE_RUBBER = 1.5f;

struct IO_PHYSICS {
	
	Cylinder cyl;
	Vec3f startpos;
	Vec3f targetpos;
	Vec3f velocity;
	Vec3f forces;
	
	IO_PHYSICS()
		: startpos(0.f)
		, targetpos(0.f)
		, velocity(0.f)
		, forces(0.f)
	{ }
	
};

enum IOCollisionFlag {
	COLLIDE_WITH_PLAYER = 1 << 0,
	COLLIDE_WITH_WORLD  = 1 << 1
};
DECLARE_FLAGS(IOCollisionFlag, IOCollisionFlags)
DECLARE_FLAGS_OPERATORS(IOCollisionFlags)

enum ItemTypeFlag {
	OBJECT_TYPE_WEAPON   = 1 << 0,
	OBJECT_TYPE_DAGGER   = 1 << 1,
	OBJECT_TYPE_1H       = 1 << 2,
	OBJECT_TYPE_2H       = 1 << 3,
	OBJECT_TYPE_BOW      = 1 << 4,
	OBJECT_TYPE_SHIELD   = 1 << 5,
	OBJECT_TYPE_FOOD     = 1 << 6,
	OBJECT_TYPE_GOLD     = 1 << 7,
	OBJECT_TYPE_ARMOR    = 1 << 8,
	OBJECT_TYPE_HELMET   = 1 << 9,
	OBJECT_TYPE_RING     = 1 << 10,
	OBJECT_TYPE_LEGGINGS = 1 << 11
};
DECLARE_FLAGS(ItemTypeFlag, ItemType)
DECLARE_FLAGS_OPERATORS(ItemType)

enum HaloFlag {
	HALO_ACTIVE   = 1 << 0,
	HALO_NEGATIVE = 1 << 1,
	HALO_DYNLIGHT = 1 << 2  //!< Unused
};
DECLARE_FLAGS(HaloFlag, HaloFlags)
DECLARE_FLAGS_OPERATORS(HaloFlags)

struct IO_HALO {
	
	Color3f color;
	float radius;
	HaloFlags flags;
	Vec3f offset;
	
	IO_HALO()
		: color(Color3f::black)
		, radius(0.f)
		, flags(0)
		, offset(0.f)
	{ }
	
};

struct IO_TWEAKER_INFO {
	res::path filename;
	std::string skintochange;
	res::path skinchangeto;
};

struct IO_SPELLCAST_DATA {
	
	SpellType castingspell; // spell being casted...
	Rune symb[4]; // symbols to draw before casting...
	SpellcastFlags spell_flags;
	short spell_level;
	EntityHandle target;
	GameDuration duration;
	
	IO_SPELLCAST_DATA()
		: castingspell(SPELL_NONE)
		, spell_level(0)
		, duration(0)
	{
		for(unsigned long j(0); j < 4; j++) {
			symb[j] = RUNE_NONE;
		}
	}
	
};

enum EntityFlag {
	IO_UNDERWATER          = 1 << 0,
	IO_FREEZESCRIPT        = 1 << 1,
	IO_ITEM                = 1 << 2,
	IO_NPC                 = 1 << 3,
	IO_FIX                 = 1 << 4,
	IO_NOSHADOW            = 1 << 5,
	IO_CAMERA              = 1 << 6,
	IO_MARKER              = 1 << 7,
	IO_ICONIC              = 1 << 8,
	IO_NO_COLLISIONS       = 1 << 9,
	IO_GOLD                = 1 << 10,
	IO_INVULNERABILITY     = 1 << 11,
	IO_NO_PHYSICS_INTERPOL = 1 << 12,
	IO_HIT                 = 1 << 13,
	IO_PHYSICAL_OFF        = 1 << 14,
	IO_MOVABLE             = 1 << 15,
	IO_UNIQUE              = 1 << 16,
	IO_SHOP                = 1 << 17,
	IO_BLACKSMITH          = 1 << 18,
	IO_NOSAVE              = 1 << 19,
	IO_FORCEDRAW           = 1 << 20,
	IO_FIELD               = 1 << 21,
	IO_BUMP                = 1 << 22,
	IO_ANGULAR             = 1 << 23,
	IO_BODY_CHUNK          = 1 << 24,
	// IO_ZMAP Currently has no effect, but keep for now as it affects save state
	IO_ZMAP                = 1 << 25,
	IO_INVERTED            = 1 << 26,
	IO_JUST_COLLIDE        = 1 << 27,
	IO_FIERY               = 1 << 28,
	IO_NO_NPC_COLLIDE      = 1 << 29,
	IO_CAN_COMBINE         = 1 << 30
};
DECLARE_FLAGS(EntityFlag, EntityFlags)
DECLARE_FLAGS_OPERATORS(EntityFlags)

enum EntitySfxFlag {
	SFX_TYPE_YLSIDE_DEATH = 1 << 0,
	SFX_TYPE_INCINERATE   = 1 << 1
};
DECLARE_FLAGS(EntitySfxFlag, EntitySfxFlags)
DECLARE_FLAGS_OPERATORS(EntitySfxFlags)

// TODO 16-bit due to save format
enum GameFlag {
	GFLAG_INTERACTIVITY     = 1 << 0,
	GFLAG_ISINTREATZONE     = 1 << 1,
	GFLAG_WASINTREATZONE    = 1 << 2,
	GFLAG_NEEDINIT          = 1 << 3,
	GFLAG_INTERACTIVITYHIDE = 1 << 4,
	GFLAG_DOOR              = 1 << 5,
	GFLAG_INVISIBILITY      = 1 << 6,
	GFLAG_NO_PHYS_IO_COL    = 1 << 7,
	GFLAG_VIEW_BLOCKER      = 1 << 8,
	GFLAG_PLATFORM          = 1 << 9,
	GFLAG_ELEVATOR          = 1 << 10,
	GFLAG_MEGAHIDE          = 1 << 11,
	GFLAG_HIDEWEAPON        = 1 << 12,
	GFLAG_NOGORE            = 1 << 13,
	GFLAG_GOREEXPLODE       = 1 << 14,
	GFLAG_NOCOMPUTATION     = 1 << 15
};
DECLARE_FLAGS(GameFlag, GameFlags)
DECLARE_FLAGS_OPERATORS(GameFlags)

enum EntityVisilibity {
	SHOW_FLAG_NOT_DRAWN    = 0,
	SHOW_FLAG_IN_SCENE     = 1,
	SHOW_FLAG_LINKED       = 2,
	SHOW_FLAG_IN_INVENTORY = 4,
	SHOW_FLAG_HIDDEN       = 5,
	SHOW_FLAG_TELEPORTING  = 6,
	SHOW_FLAG_KILLED       = 7, // Deprecated, use SHOW_FLAG_DESTROYED instead
	SHOW_FLAG_MEGAHIDE     = 8,
	SHOW_FLAG_ON_PLAYER    = 9,
	SHOW_FLAG_DESTROYED    = 255 // Only used in save files
};

struct AnimationBlendStatus {
	bool m_active;
	GameInstant lastanimtime;
};

class Entity {
	
public:
	
	explicit Entity(const res::path & classPath, EntityInstance instance);
	~Entity();
	
	EntityFlags ioflags; // IO type
	Vec3f lastpos; // IO last position
	Vec3f pos; // IO position
	Vec3f move;
	Vec3f lastmove;
	Vec3f forcedmove;
	
	Anglef angle; // IO angle
	IO_PHYSICS physics; // Movement Collision Data
	short room;
	bool requestRoomUpdate;
	float original_height;
	float original_radius;
	TextureContainer * m_icon; // Object Icon
	EERIE_3DOBJ * obj; // IO Mesh data
	ANIM_HANDLE * anims[MAX_ANIMS]; // Object Animations
	AnimLayer animlayer[MAX_ANIM_LAYERS];

	AnimationBlendStatus animBlend;
	
	EERIE_3D_BBOX bbox3D;
	EERIE_2D_BBOX bbox2D;

	res::path usemesh; // Alternate Mesh/path
	EERIE_3DOBJ * tweaky; // tweaked original obj backup
	audio::SourcedSample m_sound;
	ItemType type_flags; // object type (weapon,goblin...)
	long scriptload; // Is This object Loaded by Script ?
	Vec3f target; // Target position
	EntityHandle targetinfo; // Target Type/Ident
	
	// TODO remove and use inheritance instead
	union {
		IO_ITEMDATA * _itemdata; // ITEM Datas
		IO_FIXDATA * _fixdata; // FIX Datas
		IO_NPCDATA * _npcdata; // NPC Datas
		IO_CAMDATA * _camdata; // Camera Datas
	};
	
	INVENTORY_DATA * inventory; // Inventory Data
	EntityVisilibity show; // Show status (in scene, in inventory...)
	IOCollisionFlags collision; // collision type
	ScriptEventName mainevent;
	Color3f infracolor; // Improve Vision Color (Heat)
	
	float weight;
	std::string locname; // Localisation key for the entity's name
	GameFlags gameFlags;
	float fall;

	Vec3f initpos; // Initial Position
	Anglef initangle; // Initial Angle
	float scale;
	
	ARX_USE_PATH * usepath;
	SYMBOL_DRAW * symboldraw;
	LightHandle dynlight;
	short lastspeechflag;
	ARX_PATH * inzone;
	IO_HALO halo;
	IO_HALO halo_native;
	
	SCRIPT_VARIABLES m_variables;
	GameInstant m_scriptTimers[MAX_SCRIPTTIMERS];
	DisabledEvents m_disabledEvents;
	EERIE_SCRIPT script; // Primary Script
	EERIE_SCRIPT over_script; // Overriding Script
	short stat_count;
	short stat_sent;
	IO_TWEAKER_INFO * tweakerinfo; // optional tweaker infos
	Material material;
	
	std::set<std::string> groups;
	Vec2s m_inventorySize;// Inventory Icon size
	GameInstant soundtime;
	unsigned long soundcount;
	
	GameInstant sfx_time;
	GameInstant collide_door_time;
	GameInstant ouch_time;
	float dmg_sum;
	
	IO_SPELLCAST_DATA spellcast_data;
	short flarecount;
	EntityHandle no_collide;
	float invisibility;
	float basespeed;
	
	float speed_modif;
	
	float rubber;
	float max_durability;
	float durability;
	short poisonous;
	short poisonous_count;
	
	float ignition;
	LightHandle ignit_light;
	audio::SourcedSample ignit_sound;
	float head_rot;
	
	short damager_damages;
	DamageType damager_type;
	std::string stepmaterial;
	std::string armormaterial;
	std::string weaponmaterial;
	std::string strikespeech;
	
	EntitySfxFlags sfx_flag;
	std::vector<TWEAK_INFO> tweaks;
	s8 secretvalue;
	
	std::string shop_category;
	float shop_multiply;
	res::path inventory_skin;
	long isHit;
	short inzone_show;
	EntityHandle summoner;
	long spark_n_blood;

	Color3f special_color;
	Color3f highlightColor;
	
	const EntityId & id() const { return m_id; }
	
	/*!
	 * Get the class name for this entity
	 *
	 * This is equal to the last component of the class path
	 * 
	 * \return the entity's class name
	 */
	std::string className() const { return id().className(); }
	
	/*!
	 * Get the instance number of this entity
	 * 
	 * This is a unique number for this entity amongst all entities of the
	 * same class (name) in the entire game.
	 * 
	 * \return the entity's instance number
	 */
	EntityInstance instance() const { return id().instance(); }
	
	/*!
	 *  Returns the long name for this entity where the short name
	 * is combined with the identifying number
	 * in the form of "%s_%04ld"
	 * \return The short name combined with a 4 digit ident, padded with 0
	 */
	std::string idString() const { return id().string(); }
	
	/*!
	 *  Returns the full name for this entity where the
	 * directory portion of the filename member is combined
	 * with the the result of idString()
	 * \return The directory of filename + idString()
	 */
	res::path instancePath() const;
	
	//! \return the index of this Entity in the EntityManager
	EntityHandle index() const { return EntityHandle(m_index); }
	
	/*!
	 * Marks the entity as destroyed.
	 * 
	 * If the entity was loaded by a script, the entity is deleted.
	 * Otherwise the entity is kept so that the id won't be reused.
	 */
	void destroy();
	
	/*!
	 * If the entity is an item, decrease the stack count.
	 * If the count reaches zero or if the entity is not an item, call destroy().
	 */
	void destroyOne();
	
	/*!
	 * Get the class path for this entity.
	 *
	 * \return the full path to this entity's class
	 */
	const res::path & classPath() const { return m_classPath; }
	
private:
	
	//! Remove any remaining references to this entity.
	void cleanReferences();
	
	size_t m_index; //!< index of this Entity in the EntityManager
	
	const EntityId m_id; //!< unique ID for this entity
	const res::path m_classPath; //!< the full path to this entity's class
	
};

inline Vec3f actionPointPosition(const EERIE_3DOBJ * obj, ActionPoint ap) {
	return obj->vertexWorldPositions[ap.handleData()].v;
}

// TODO move this somewhere else
struct IO_FIXDATA {
	s8 trapvalue;
	char padd[3];
};

#endif // ARX_GAME_ENTITY_H
