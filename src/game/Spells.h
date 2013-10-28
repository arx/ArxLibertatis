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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_SPELLS_H
#define ARX_GAME_SPELLS_H

#include <stddef.h>
#include <string>

#include "audio/AudioTypes.h"
#include "math/Types.h"
#include "math/Angle.h"
#include "math/Random.h"
#include "math/Vector.h"
#include "platform/Flags.h"

class Entity;
class CSpellFx;
class TextureContainer;

// Spells symbol list
enum Rune {
	RUNE_AAM = 0,     // Create
	RUNE_NHI,         // Negate
	RUNE_MEGA,        // Improve
	RUNE_YOK,         // Fire
	RUNE_TAAR,        // Projectile
	RUNE_KAOM,        // Protection
	RUNE_VITAE,       // Life
	RUNE_VISTA,       // Vision
	RUNE_STREGUM,     // Magic
	RUNE_MORTE,       // Death
	RUNE_COSUM,       // Object
	RUNE_COMUNICATUM, // Communication
	RUNE_MOVIS,       // Movement
	RUNE_TEMPUS,      // Time
	RUNE_FOLGORA,     // Storm
	RUNE_SPACIUM,     // Space
	RUNE_TERA,        // Earth
	RUNE_CETRIUS,     // Poison
	RUNE_RHAA,        // Lower
	RUNE_FRIDD,       // Ice
	RUNE_AKBAA,       // Akbaa
	RUNE_NONE = 255
};
const size_t RUNE_COUNT = 21;

enum SpellcastFlag {
	SPELLCAST_FLAG_NODRAW         = (1<<0),
	SPELLCAST_FLAG_NOANIM         = (1<<1),
	SPELLCAST_FLAG_NOMANA         = (1<<2),
	SPELLCAST_FLAG_PRECAST        = (1<<3),
	SPELLCAST_FLAG_LAUNCHPRECAST  = (1<<4),
	SPELLCAST_FLAG_NOCHECKCANCAST = (1<<5),
	SPELLCAST_FLAG_NOSOUND        = (1<<6),
	SPELLCAST_FLAG_RESTORE        = (1<<7)
};
DECLARE_FLAGS(SpellcastFlag, SpellcastFlags)
DECLARE_FLAGS_OPERATORS(SpellcastFlags)

// Spell list
enum Spell {
	
	// LEVEL 1
	SPELL_MAGIC_SIGHT           ,//= 11,
	SPELL_MAGIC_MISSILE         ,//= 12,
	SPELL_IGNIT                 ,//= 13,
	SPELL_DOUSE                 ,//= 14,
	SPELL_ACTIVATE_PORTAL       ,//= 15,
	
	// LEVEL 2
	SPELL_HEAL                  ,//= 21,
	SPELL_DETECT_TRAP           ,//= 22,
	SPELL_ARMOR                 ,//= 23,
	SPELL_LOWER_ARMOR           ,//= 24,
	SPELL_HARM                  ,//= 25,
	
	// LEVEL 3
	SPELL_SPEED                 ,//= 31,
	SPELL_DISPELL_ILLUSION      ,//= 32,
	SPELL_FIREBALL              ,//= 33,
	SPELL_CREATE_FOOD           ,//= 34,
	SPELL_ICE_PROJECTILE        ,//= 35,
	
	// LEVEL 4
	SPELL_BLESS                 ,//= 41,
	SPELL_DISPELL_FIELD         ,//= 42,
	SPELL_FIRE_PROTECTION       ,//= 43,
	SPELL_TELEKINESIS           ,//= 44,
	SPELL_CURSE                 ,//= 45,
	SPELL_COLD_PROTECTION       ,//= 46,
	
	// LEVEL 5
	SPELL_RUNE_OF_GUARDING      ,//= 51,
	SPELL_LEVITATE              ,//= 52,
	SPELL_CURE_POISON           ,//= 53,
	SPELL_REPEL_UNDEAD          ,//= 54,
	SPELL_POISON_PROJECTILE     ,//= 55,
	
	// LEVEL 6
	SPELL_RISE_DEAD             ,//= 61,
	SPELL_PARALYSE              ,//= 62,
	SPELL_CREATE_FIELD          ,//= 63,
	SPELL_DISARM_TRAP           ,//= 64,
	SPELL_SLOW_DOWN             ,//= 65, //secret
	
	// LEVEL 7
	SPELL_FLYING_EYE            ,//= 71,
	SPELL_FIRE_FIELD            ,//= 72,
	SPELL_ICE_FIELD             ,//= 73,
	SPELL_LIGHTNING_STRIKE      ,//= 74,
	SPELL_CONFUSE               ,//= 75,
	
	// LEVEL 8
	SPELL_INVISIBILITY          ,//= 81,
	SPELL_MANA_DRAIN            ,//= 82,
	SPELL_EXPLOSION             ,//= 83,
	SPELL_ENCHANT_WEAPON        ,//= 84,
	SPELL_LIFE_DRAIN            ,//= 85, //secret
	
	// LEVEL 9
	SPELL_SUMMON_CREATURE       ,//= 91,
	SPELL_NEGATE_MAGIC          ,//= 92,
	SPELL_INCINERATE            ,//= 93,
	SPELL_MASS_PARALYSE         ,//= 94,
	
	// LEVEL 10
	SPELL_MASS_LIGHTNING_STRIKE ,//= 101,
	SPELL_CONTROL_TARGET        ,//= 102,
	SPELL_FREEZE_TIME           ,//= 103,
	SPELL_MASS_INCINERATE       ,//= 104
	
	SPELL_FAKE_SUMMON           ,// special =105
	
	// LEVEL ZOB
	SPELL_TELEPORT = SPELL_FAKE_SUMMON + 2, // TODO explicit value for savegame compatability
	
	SPELL_NONE = -1
};

const size_t SPELL_COUNT = SPELL_FAKE_SUMMON + 1;

struct PRECAST_STRUCT {
	Spell typ;
	long level;
	unsigned long launch_time;
	SpellcastFlags flags;
	long duration;
};

const size_t MAX_PRECAST = 3;
extern PRECAST_STRUCT Precast[MAX_PRECAST];

void ARX_SPELLS_Precast_Reset();
void ARX_SPELLS_Precast_Launch(long num);

Spell GetSpellId(const std::string & spell);
void TryToCastSpell(Entity * io, Spell spellid, long level, long target, SpellcastFlags flags, long duration);
void ARX_SPELLS_Precast_Check();

const size_t MAX_SPELL_SYMBOLS = 6;
extern Rune SpellSymbol[MAX_SPELL_SYMBOLS];
extern size_t CurrSpellSymbol;

struct SPELL {
	
	bool exist;
	long caster; //!< Number of the source interactive obj (0==player)
	long target; //!< Number of the target interactive obj if any
	float caster_level; //!< Level of Magic 1-10
	
	long hand_group;
	Vec3f hand_pos; //!< Only valid if hand_group>=0
	Vec3f caster_pos;
	Vec3f target_pos;
	
	float fdata; //!< Specific use for each spell
	
	Spell type;
	Vec3f vsource; // TODO this is used but never set
	
	Vec3f move;
	Vec3f scale;
	float siz;
	unsigned long timcreation;
	unsigned long lastupdate;
	unsigned long tolive;
	
	TextureContainer * tc;
	long longinfo;
	long longinfo2;
	bool bDuration;
	float fManaCostPerSecond;
	
	SpellcastFlags flags;
	audio::SourceId snd_loop;
	CSpellFx * pSpellFx;
	void * misc;
};

const size_t MAX_SPELLS = 20;
extern SPELL spells[MAX_SPELLS];

extern long CurrPoint;

bool ARX_SPELLS_Launch(Spell typ, long source, SpellcastFlags flags = 0, long level = -1, long target = -1, long duration = -1);
void ARX_SPELLS_ResetRecognition();
void ARX_SPELLS_AddPoint(const Vec2s & pos);
void ARX_SPELLS_AbortSpellSound();
void ARX_SPELLS_Init();
void ARX_SPELLS_ClearAll();
void ARX_SPELLS_Update();

void ARX_SPELLS_Kill(long i);
long ARX_SPELLS_GetInstance(Spell typ);
void ARX_SPELLS_ManageMagic();

void ARX_SPELLS_RequestSymbolDraw(Entity * io, const std::string & name, float duration);
void ARX_SPELLS_UpdateSymbolDraw();
void ARX_SPELLS_ClearAllSymbolDraw();

void ARX_SPELLS_Init_Rects();

bool ARX_SPELLS_ExistAnyInstance(Spell typ);
void ARX_SPELLS_RemoveAllSpellsOn(Entity * io);
long ARX_SPELLS_GetSpellOn(const Entity * io, Spell spellid);
long ARX_SPELLS_GetInstanceForThisCaster(Spell typ, long caster);

void ARX_SPELLS_CancelSpellTarget();
void ARX_SPELLS_LaunchSpellTarget(Entity * io);
float ARX_SPELLS_ApplyFireProtection(Entity * io, float damages);
float ARX_SPELLS_ApplyColdProtection(Entity * io, float damages);
void ARX_SPELLS_FizzleAllSpellsFromCaster(long num);

#endif // ARX_GAME_SPELLS_H
