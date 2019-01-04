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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_PLAYER_H
#define ARX_GAME_PLAYER_H

#include <stddef.h>
#include <string>
#include <vector>

#include <boost/array.hpp>

#include "game/Entity.h"
#include "game/Spells.h"
#include "game/GameTypes.h"
#include "gui/book/Necklace.h"
#include "math/Types.h"
#include "util/Flags.h"

struct EERIE_3DOBJ;
class TextureContainer;

static const size_t MAX_EQUIPED = 12;

struct ARX_INTERFACE_MEMORIZE_SPELL {
	bool bSpell;
	GameInstant lTimeCreation;
	Rune iSpellSymbols[6];
	
	ARX_INTERFACE_MEMORIZE_SPELL()
		: bSpell(false)
		, lTimeCreation(0)
	{
		for(size_t i = 0; i < ARRAY_SIZE(iSpellSymbols); i++) {
			iSpellSymbols[i] = RUNE_NONE;
		}
	}
};

enum PlayerInterfaceFlag {
	INTER_PLAYERBOOK   = 1 << 0,
	INTER_INVENTORY    = 1 << 1,
	INTER_INVENTORYALL = 1 << 2,
	INTER_MINIBOOK     = 1 << 3,
	INTER_MINIBACK     = 1 << 4,
	INTER_LIFE_MANA    = 1 << 5,
	INTER_COMBATMODE   = 1 << 6,
	INTER_NOTE         = 1 << 7, // TODO remove
	INTER_STEAL        = 1 << 8,
	INTER_NO_STRIKE    = 1 << 9
};
DECLARE_FLAGS(PlayerInterfaceFlag, PlayerInterfaceFlags)
DECLARE_FLAGS_OPERATORS(PlayerInterfaceFlags)

enum PlayerMovementFlag {
	PLAYER_MOVE_WALK_FORWARD  = 1 << 0,
	PLAYER_MOVE_WALK_BACKWARD = 1 << 1,
	PLAYER_MOVE_STRAFE_LEFT   = 1 << 2,
	PLAYER_MOVE_STRAFE_RIGHT  = 1 << 3,
	PLAYER_MOVE_JUMP          = 1 << 4,
	PLAYER_MOVE_STEALTH       = 1 << 5,
	PLAYER_ROTATE             = 1 << 6,
	PLAYER_CROUCH             = 1 << 7,
	PLAYER_LEAN_LEFT          = 1 << 8,
	PLAYER_LEAN_RIGHT         = 1 << 9
};
DECLARE_FLAGS(PlayerMovementFlag, PlayerMovement)
DECLARE_FLAGS_OPERATORS(PlayerMovement)

enum PlayerFlag {
	PLAYERFLAGS_NO_MANA_DRAIN   = 1 << 0,
	PLAYERFLAGS_INVULNERABILITY = 1 << 1
};
DECLARE_FLAGS(PlayerFlag, PlayerFlags)
DECLARE_FLAGS_OPERATORS(PlayerFlags)

enum RuneFlag {
	FLAG_AAM         = 1 << RUNE_AAM,
	FLAG_CETRIUS     = 1 << RUNE_CETRIUS,
	FLAG_COMUNICATUM = 1 << RUNE_COMUNICATUM,
	FLAG_COSUM       = 1 << RUNE_COSUM,
	FLAG_FOLGORA     = 1 << RUNE_FOLGORA,
	FLAG_FRIDD       = 1 << RUNE_FRIDD,
	FLAG_KAOM        = 1 << RUNE_KAOM,
	FLAG_MEGA        = 1 << RUNE_MEGA,
	FLAG_MORTE       = 1 << RUNE_MORTE,
	FLAG_MOVIS       = 1 << RUNE_MOVIS,
	FLAG_NHI         = 1 << RUNE_NHI,
	FLAG_RHAA        = 1 << RUNE_RHAA,
	FLAG_SPACIUM     = 1 << RUNE_SPACIUM,
	FLAG_STREGUM     = 1 << RUNE_STREGUM,
	FLAG_TAAR        = 1 << RUNE_TAAR,
	FLAG_TEMPUS      = 1 << RUNE_TEMPUS,
	FLAG_TERA        = 1 << RUNE_TERA,
	FLAG_VISTA       = 1 << RUNE_VISTA,
	FLAG_VITAE       = 1 << RUNE_VITAE,
	FLAG_YOK         = 1 << RUNE_YOK
};
DECLARE_FLAGS(RuneFlag, RuneFlags)
DECLARE_FLAGS_OPERATORS(RuneFlags)

enum JumpPhase {
	NotJumping = 0,
	JumpStart = 1,
	JumpAscending = 2,
	JumpDescending = 4,
	JumpEnd = 5
};

struct PlayerAttribute {
	float strength;
	float dexterity;
	float constitution;
	float mind;
	
	PlayerAttribute()
		: strength(0)
		, dexterity(0)
		, constitution(0)
		, mind(0)
	{}
	
	void add(const PlayerAttribute & other) {
		strength += other.strength;
		dexterity += other.dexterity;
		constitution += other.constitution;
		mind += other.mind;
	}
};

struct PlayerSkill {
	float stealth;
	float mecanism;
	float intuition;
	
	float etheralLink;
	float objectKnowledge;
	float casting;
	
	float projectile;
	float closeCombat;
	float defense;
	
	PlayerSkill()
		: stealth(0)
		, mecanism(0)
		, intuition(0)
		, etheralLink(0)
		, objectKnowledge(0)
		, casting(0)
		, projectile(0)
		, closeCombat(0)
		, defense(0)
	{}
	
	void add(const PlayerSkill & other) {
		stealth += other.stealth;
		mecanism += other.mecanism;
		intuition += other.intuition;
		
		etheralLink += other.etheralLink;
		objectKnowledge += other.objectKnowledge;
		casting += other.casting;
		
		projectile += other.projectile;
		closeCombat += other.closeCombat;
		defense += other.defense;
	}
};

struct PlayerMisc {
	float armorClass;
	float resistMagic;
	float resistPoison;
	float criticalHit;
	float damages;
	
	PlayerMisc()
		: armorClass(0)
		, resistMagic(0)
		, resistPoison(0)
		, criticalHit(0)
		, damages(0)
	{}
	
	void add(const PlayerMisc & other) {
		armorClass += other.armorClass;
		resistMagic += other.resistMagic;
		resistPoison += other.resistPoison;
		criticalHit += other.criticalHit;
		damages += other.damages;
	}
};

struct ARXCHARACTER {
	
	Vec3f pos;
	Anglef angle;
	IO_PHYSICS physics;
	
	AnimLayer bookAnimation[MAX_ANIM_LAYERS];
	
	long m_strikeDirection;
	AnimationDuration m_weaponBlocked;
	
	// Jump Sub-data
	PlatformInstant jumpstarttime;
	float jumplastposition;
	JumpPhase jumpphase;
	
	bool climbing;
	
	bool m_paralysed;
	
	// Updated by spells
	bool levitate;
	bool m_telekinesis;
	bool m_improve;
	
	Anglef desiredangle;
	Vec3f size;
	ARX_PATH * inzone;
	
	bool falling;
	short doingmagic;
	PlayerInterfaceFlags Interface;
	
	PlayerMovement m_currentMovement;
	PlayerMovement m_lastMovement;
	bool onfirmground;
	
	Color3f m_torchColor;
	Entity * torch;
	
	EntityHandle equiped[MAX_EQUIPED];
	
	// Modifier Values (Items, curses, etc...)
	PlayerAttribute m_attributeMod;
	PlayerSkill m_skillMod;
	PlayerMisc m_miscMod;
	
	// Full Frame values (including items)
	PlayerAttribute m_attributeFull;
	PlayerSkill m_skillFull;
	PlayerMisc m_miscFull;
	
	float m_bowAimRatio;
	
	float m_strikeAimRatio;
	PlatformDuration Full_AimTime;
	
	float Full_life;
	float Full_maxlife;
	float Full_maxmana;
	
	// true (naked) Player Values
	PlayerAttribute m_attribute;
	PlayerSkill m_skill;
	
	PlatformDuration m_aimTime;
	
	ResourcePool lifePool;
	ResourcePool manaPool;
	
	// Player Old Values
	PlayerSkill m_skillOld;
	
	unsigned char Attribute_Redistribute;
	unsigned char Skill_Redistribute;
	
	short level;
	
	long xp;
	char skin;
	
	RuneFlags rune_flags;
	bool hasRune(Rune rune) {
		return (rune_flags & RuneFlag(1 << rune)) != 0;
	}
	
	
	boost::array<TextureContainer *, 5> heads;
	float poison;
	float hunger;
	PlayerFlags playerflags;
	long gold;
	short m_bags;
	ARX_INTERFACE_MEMORIZE_SPELL SpellToMemorize;

	float TRAP_DETECT;
	float TRAP_SECRET;
	
	long m_cheatPnuxActive;
	
	GameDuration DeadTime;
	
	audio::SourcedSample magic_ambient;
	audio::SourcedSample magic_draw;
	audio::SourcedSample torch_loop;
	
	ARXCHARACTER()
		: pos(0.f)
		, m_strikeDirection(0)
		, m_weaponBlocked(AnimationDuration::ofRaw(-1)) // FIXME inband signaling
		, jumpstarttime(0)
		, jumplastposition(0.f)
		, jumpphase(NotJumping)
		, climbing(false)
		, m_paralysed(false)
		, levitate(false)
		, m_telekinesis(false)
		, m_improve(false)
		, size(0.f)
		, inzone(NULL)
		, falling(false)
		, doingmagic(0)
		, onfirmground(false)
		, torch(NULL)
		, m_bowAimRatio(0.f)
		, m_strikeAimRatio(0.f)
		, Full_AimTime(0)
		, Full_life(0)
		, Full_maxlife(0)
		, Full_maxmana(0)
		, m_aimTime(0)
		, Attribute_Redistribute(0)
		, Skill_Redistribute(0)
		, level(0)
		, xp(0)
		, skin(0)
		, poison(0)
		, hunger(0)
		, gold(0)
		, m_bags(0)
		, TRAP_DETECT(0)
		, TRAP_SECRET(0)
		, m_cheatPnuxActive(0)
		, DeadTime(0)
	{
		heads.fill(NULL);
	}
	
	static float baseRadius() { return 52.f; }
	static float baseHeight() { return -170.f; }
	static float crouchHeight() { return -120.f; }
	static float levitateHeight() { return -220.f; }
	
	static Vec3f baseOffset() { return Vec3f(0.f, baseHeight(), 0.f); }
	
	Vec3f basePosition() {
		return Vec3f(pos.x, pos.y - baseHeight(), pos.z);
	}
	
	Cylinder baseCylinder() {
		return Cylinder(basePosition(), baseRadius(), baseHeight());
	}
	
	bool isAiming() { return m_aimTime > 0; }
	
};

extern float CURRENT_PLAYER_COLOR;

extern ARXCHARACTER player;
extern EERIE_3DOBJ * hero;
extern ANIM_HANDLE * herowaitbook;
extern ANIM_HANDLE * herowait_2h;
extern std::vector<std::string> g_playerQuestLogEntries;
extern std::vector<std::string> g_playerKeyring;

extern bool BLOCK_PLAYER_CONTROLS;
extern bool USE_PLAYERCOLLISIONS;
extern bool WILLRETURNTOCOMBATMODE;
extern PlatformInstant LAST_JUMP_ENDTIME;

void ARX_PLAYER_MakeSpHero();
void ARX_PLAYER_LoadHeroAnimsAndMesh();
void ARX_PLAYER_InitPlayer();
void ARX_PLAYER_BecomesDead();
void ARX_PLAYER_ClickedOnTorch(Entity * io);
void ARX_PLAYER_RectifyPosition();
void ARX_PLAYER_Frame_Update();
void ARX_PLAYER_Manage_Movement();
void ARX_PLAYER_Manage_Death();
void ARX_PLAYER_Quest_Add(const std::string & quest);
void ARX_PLAYER_Quest_Init();
Vec3f ARX_PLAYER_FrontPos();
void ARX_PLAYER_ComputePlayerFullStats();
void ARX_PLAYER_MakeFreshHero();
void ARX_PLAYER_QuickGeneration();
void ARX_PLAYER_MakeAverageHero();
void ARX_PLAYER_Modify_XP(long val);
void ARX_PLAYER_FrameCheck(PlatformDuration delta);
void ARX_PLAYER_Poison(float val);
void ARX_PLAYER_Manage_Visual();
void ARX_PLAYER_Remove_Invisibility();
void ARX_Player_Rune_Add(RuneFlag rune);
void ARX_Player_Rune_Remove(RuneFlag rune);
void ARX_PLAYER_AddGold(long value);
void ARX_PLAYER_AddGold(Entity * gold);
void ARX_PLAYER_AddBag();
bool ARX_PLAYER_CanStealItem(Entity * item);

void ARX_KEYRING_Init();
void ARX_KEYRING_Add(const std::string & key);

void ARX_PLAYER_Reset_Fall();
void ARX_PLAYER_KillTorch();
void ARX_PLAYER_PutPlayerInNormalStance();
void ARX_PLAYER_Start_New_Quest();
void ARX_PLAYER_Rune_Add_All();
 
void ARX_PLAYER_Restore_Skin();
float GetPlayerStealth();

void ARX_GAME_Reset();
long GetXPforLevel(short level);
bool ARX_PLAYER_IsInFightMode();
void ARX_PLAYER_Invulnerability(long flag);

void ForcePlayerLookAtIO(Entity * io);

void ARX_SPSound();

#endif // ARX_GAME_PLAYER_H
