/*
 * Copyright 2014-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_MAGIC_SPELL_H
#define ARX_GAME_MAGIC_SPELL_H

#include <boost/noncopyable.hpp>

#include "game/Damage.h"

#include "graphics/effects/SpellEffects.h"
#include "platform/Alignment.h"
#include "scene/Light.h"
#include "util/Flags.h"

class TextureContainer;

// Spell list
enum SpellType {
	
	// LEVEL 1
	SPELL_MAGIC_SIGHT,           // = 11,
	SPELL_MAGIC_MISSILE,         // = 12,
	SPELL_IGNIT,                 // = 13,
	SPELL_DOUSE,                 // = 14,
	SPELL_ACTIVATE_PORTAL,       // = 15,
	
	// LEVEL 2
	SPELL_HEAL,                  // = 21,
	SPELL_DETECT_TRAP,           // = 22,
	SPELL_ARMOR,                 // = 23,
	SPELL_LOWER_ARMOR,           // = 24,
	SPELL_HARM,                  // = 25,
	
	// LEVEL 3
	SPELL_SPEED,                 // = 31,
	SPELL_DISPELL_ILLUSION,      // = 32,
	SPELL_FIREBALL,              // = 33,
	SPELL_CREATE_FOOD,           // = 34,
	SPELL_ICE_PROJECTILE,        // = 35,
	
	// LEVEL 4
	SPELL_BLESS,                 // = 41,
	SPELL_DISPELL_FIELD,         // = 42,
	SPELL_FIRE_PROTECTION,       // = 43,
	SPELL_TELEKINESIS,           // = 44,
	SPELL_CURSE,                 // = 45,
	SPELL_COLD_PROTECTION,       // = 46,
	
	// LEVEL 5
	SPELL_RUNE_OF_GUARDING,      // = 51,
	SPELL_LEVITATE,              // = 52,
	SPELL_CURE_POISON,           // = 53,
	SPELL_REPEL_UNDEAD,          // = 54,
	SPELL_POISON_PROJECTILE,     // = 55,
	
	// LEVEL 6
	SPELL_RISE_DEAD,             // = 61,
	SPELL_PARALYSE,              // = 62,
	SPELL_CREATE_FIELD,          // = 63,
	SPELL_DISARM_TRAP,           // = 64,
	SPELL_SLOW_DOWN,             // = 65, //secret
	
	// LEVEL 7
	SPELL_FLYING_EYE,            // = 71,
	SPELL_FIRE_FIELD,            // = 72,
	SPELL_ICE_FIELD,             // = 73,
	SPELL_LIGHTNING_STRIKE,      // = 74,
	SPELL_CONFUSE,               // = 75,
	
	// LEVEL 8
	SPELL_INVISIBILITY,          // = 81,
	SPELL_MANA_DRAIN,            // = 82,
	SPELL_EXPLOSION,             // = 83,
	SPELL_ENCHANT_WEAPON,        // = 84,
	SPELL_LIFE_DRAIN,            // = 85, //secret
	
	// LEVEL 9
	SPELL_SUMMON_CREATURE,       // = 91,
	SPELL_NEGATE_MAGIC,          // = 92,
	SPELL_INCINERATE,            // = 93,
	SPELL_MASS_PARALYSE,         // = 94,
	
	// LEVEL 10
	SPELL_MASS_LIGHTNING_STRIKE, // = 101,
	SPELL_CONTROL_TARGET,        // = 102,
	SPELL_FREEZE_TIME,           // = 103,
	SPELL_MASS_INCINERATE,       // = 104
	
	SPELL_FAKE_SUMMON,           // special =105
	
	SPELL_NONE = -1
};

const size_t SPELL_TYPES_COUNT = SPELL_FAKE_SUMMON + 1;

enum SpellcastFlag {
	SPELLCAST_FLAG_NODRAW         = 1 << 0,
	SPELLCAST_FLAG_NOANIM         = 1 << 1,
	SPELLCAST_FLAG_NOMANA         = 1 << 2,
	SPELLCAST_FLAG_PRECAST        = 1 << 3,
	SPELLCAST_FLAG_LAUNCHPRECAST  = 1 << 4,
	SPELLCAST_FLAG_NOCHECKCANCAST = 1 << 5,
	SPELLCAST_FLAG_NOSOUND        = 1 << 6,
	SPELLCAST_FLAG_RESTORE        = 1 << 7
};
DECLARE_FLAGS(SpellcastFlag, SpellcastFlags)
DECLARE_FLAGS_OPERATORS(SpellcastFlags)

class ARX_ALIGNAS(16) SpellBase : private boost::noncopyable {
	
public:
	
	// We can't use ARX_ALIGNOF(glm::mat4x4) directly because MSVC sucks
	ARX_STATIC_ASSERT(ARX_ALIGNOF(glm::mat4x4) <= 16, "need to increase alignment");
	
	SpellBase();
	virtual ~SpellBase() {}
	
	virtual bool CanLaunch() {
		return true;
	}
	virtual void Launch() = 0;
	virtual void End() {
		
	}
	
	virtual void Update() {
	}
	
	virtual Vec3f getPosition();
	Vec3f getCasterPosition();
	Vec3f getTargetPosition();
	
	void updateCasterHand();
	void updateCasterPosition();
	
	void requestEnd() {
		m_hasDuration = true;
		m_duration = 0;
	}
	
	SpellHandle m_thisHandle;
	
	EntityHandle m_caster; //!< Number of the source interactive obj (0==player)
	EntityHandle m_target; //!< Number of the target interactive obj if any
	float m_level; //!< Level of Magic 1-10
	
	ActionPoint m_hand_group;
	Vec3f m_hand_pos; //!< Only valid if hand_group>=0
	Vec3f m_caster_pos;
	
	SpellType m_type;
	
	GameInstant m_timcreation;
	
	bool m_hasDuration;
	GameDuration m_duration;
	GameDuration m_elapsed;
	
	float m_fManaCostPerSecond;
	
	SpellcastFlags m_flags;
	audio::SourcedSample m_snd_loop;
	
	GameDuration m_launchDuration;

	
	std::vector<EntityHandle> m_targets;
	
protected:
	Vec3f getTargetPos(EntityHandle source, EntityHandle target);
	
public:
	ARX_USE_ALIGNED_NEW(SpellBase)
};

#endif // ARX_GAME_MAGIC_SPELL_H
