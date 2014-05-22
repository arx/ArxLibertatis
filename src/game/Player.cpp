/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/Player.h"

#include <stddef.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>

#include <boost/container/flat_set.hpp>

#include "animation/Animation.h"
#include "animation/AnimationRender.h"

#include "cinematic/CinematicController.h"

#include "ai/PathFinderManager.h"
#include "ai/Paths.h"

#include "core/Application.h"
#include "core/Localisation.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Missile.h"
#include "game/NPC.h"
#include "game/spell/FlyingEye.h"
#include "game/spell/Cheat.h"
#include "game/effect/Quake.h"

#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "gui/MiniMap.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/MagicFlare.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Random.h"
#include "math/Vector.h"

#include "physics/Collisions.h"
#include "physics/Attractors.h"
#include "physics/Projectile.h"

#include "platform/Platform.h"

#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"

using std::vector;

extern bool		ARX_CONVERSATION;
extern long		HERO_SHOW_1ST;
extern long		REQUEST_SPEECH_SKIP;
extern long		CHANGE_LEVEL_ICON;
extern long		DONT_ERASE_PLAYER;
extern bool		GLOBAL_MAGIC_MODE;

extern Entity * CAMERACONTROLLER;
extern ParticleManager * pParticleManager;

extern unsigned long LAST_JUMP_ENDTIME;

static const float WORLD_GRAVITY = 0.1f;
static const float JUMP_GRAVITY = 0.02f;
static const float STEP_DISTANCE = 120.f;
static const float TARGET_DT = 1000.f / 30.f;

extern Vec3f PUSH_PLAYER_FORCE;
extern bool bBookHalo;
extern bool bGoldHalo;
extern float InventoryX;
extern float InventoryDir;
extern long COLLIDED_CLIMB_POLY;
extern long HERO_SHOW_1ST;
extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern unsigned long ulBookHaloTime;
extern unsigned long ulGoldHaloTime;

static const float ARX_PLAYER_SKILL_STEALTH_MAX = 100.f;

ARXCHARACTER player;
EERIE_3DOBJ * hero = NULL;
float currentdistance = 0.f;
float CURRENT_PLAYER_COLOR = 0;
float PLAYER_ROTATION = 0;

bool USE_PLAYERCOLLISIONS = true;
bool BLOCK_PLAYER_CONTROLS = false;
bool WILLRETURNTOCOMBATMODE = false;
long DeadTime = 0;
static unsigned long LastHungerSample = 0;
static unsigned long ROTATE_START = 0;

// Player Anims FLAGS/Vars
ANIM_HANDLE * herowaitbook = NULL;
ANIM_HANDLE * herowait_2h = NULL;

ARX_NECKLACE necklace;

vector<KEYRING_SLOT> Keyring;

static unsigned long FALLING_TIME = 0;

vector<STRUCT_QUEST> PlayerQuest;

bool ARX_PLAYER_IsInFightMode() {
	if (player.Interface & INTER_COMBATMODE) return true;

	if(entities.size() > 0 && entities.player()
	   && entities.player()->animlayer[1].cur_anim) {
		
		ANIM_USE * ause1 = &entities.player()->animlayer[1];
		ANIM_HANDLE ** alist = entities.player()->anims;

		if ((ause1->cur_anim	==	alist[ANIM_BARE_READY])
		        ||	(ause1->cur_anim	==	alist[ANIM_BARE_UNREADY])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_DAGGER_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_1H_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_2H_UNREADY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_READY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_READY_PART_2])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_UNREADY_PART_1])
		        ||	(ause1->cur_anim	==	alist[ANIM_MISSILE_UNREADY_PART_2])
		   )
			return true;
	}

	return false;
}

/*!
 * \brief Init/Reset player Keyring structures
 */
void ARX_KEYRING_Init() {
	Keyring.clear();
}

/*!
 * \brief Add a key to Keyring
 * \param key
 */
void ARX_KEYRING_Add(const std::string & key) {
	Keyring.resize(Keyring.size() + 1);
	memset(&Keyring.back(), 0, sizeof(KEYRING_SLOT));
	strcpy(Keyring.back().slot, key.c_str());
}

/*!
 * \brief Sends COMBINE event to "io" for each keyring entry
 * \param io
 */
void ARX_KEYRING_Combine(Entity * io) {
	for(size_t i = 0; i < Keyring.size(); i++) {
		if(SendIOScriptEvent(io, SM_COMBINE, Keyring[i].slot) == REFUSE) {
			return;
		}
	}
}

/*!
 * \brief Fills "pos" with player "front pos" for sound purpose
 * \param pos
 */
void ARX_PLAYER_FrontPos(Vec3f * pos)
{
	pos->x = player.pos.x - std::sin(radians(MAKEANGLE(player.angle.getPitch()))) * 100.f;
	pos->y = player.pos.y + 100.f; //-100.f;
	pos->z = player.pos.z + std::cos(radians(MAKEANGLE(player.angle.getPitch()))) * 100.f;
}

/*!
 * \brief Reset all extra-rotation groups of player
 */
void ARX_PLAYER_RectifyPosition() {
	Entity * io = entities.player();
	if(io && io->_npcdata->ex_rotate) {
		for(long n = 0; n < MAX_EXTRA_ROTATE; n++) {
			io->_npcdata->ex_rotate->group_rotate[n] = Anglef::ZERO;
		}
		io->_npcdata->ex_rotate->flags = 0;
	}
}

void ARX_PLAYER_KillTorch() {
	
	ARX_SOUND_PlaySFX(SND_TORCH_END);
	ARX_SOUND_Stop(SND_TORCH_LOOP);
	
	giveToPlayer(player.torch);
	
	player.torch = NULL;
	lightHandleGet(torchLightHandle)->exist = 0;
}

void ARX_PLAYER_ClickedOnTorch(Entity * io)
{
	if(!io)
		return;

	if(player.torch == io) {
		ARX_PLAYER_KillTorch();
		return;
	}

	if(player.torch)
		ARX_PLAYER_KillTorch();

	if(io->durability > 0) {
		if(io->ignition > 0) {
			lightHandleDestroy(io->ignit_light);

			if(io->ignit_sound != audio::INVALID_ID) {
				ARX_SOUND_Stop(io->ignit_sound);
				io->ignit_sound = audio::INVALID_ID;
			}

			io->ignition = 0;
		}

		ARX_SOUND_PlaySFX(SND_TORCH_START);
		ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
		RemoveFromAllInventories(io);
		player.torch = io;
		io->show = SHOW_FLAG_ON_PLAYER;

		if(DRAGINTER == io)
			DRAGINTER = NULL;
	}
}

static void ARX_PLAYER_ManageTorch() {
	
	if(player.torch) {
		
		player.torch->ignition = 0;
		player.torch->durability -= framedelay * ( 1.0f / 10000 );
		
		if(player.torch->durability <= 0) {
			ARX_SOUND_PlaySFX(SND_TORCH_END);
			ARX_SOUND_Stop(SND_TORCH_LOOP);
			player.torch->destroy();
			player.torch = NULL;
			lightHandleGet(torchLightHandle)->exist = 0;
		}
		
	}
}

/*!
 * \brief Init/Reset player Quest structures
 */
void ARX_PLAYER_Quest_Init() {
	PlayerQuest.clear();
	gui::updateQuestBook();
}

/*!
 * \brief Add _ulRune to player runes
 * \param _ulRune
 */
void ARX_Player_Rune_Add(RuneFlag _ulRune)
{
	int iNbSpells = 0;
	int iNbSpellsAfter = 0;

	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		if(spellicons[i].bSecret == false) {
			long j = 0;
			bool bOk = true;

			while(j < 4 && spellicons[i].symbols[j] != 255) {
				if(!(player.rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j]))) {
					bOk = false;
				}
				j++;
			}

			if(bOk) {
				iNbSpells ++;
			}
		}
	}

	player.rune_flags |= _ulRune;

	for(size_t i = 0; i < SPELL_TYPES_COUNT; i++) {
		if(spellicons[i].bSecret == false) {
			long j = 0;
			bool bOk = true;

			while(j < 4 && (spellicons[i].symbols[j] != 255)) {
				if(!(player.rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j]))) {
					bOk = false;
				}
				j++;
			}

			if(bOk) {
				iNbSpellsAfter ++;
			}
		}
	}
	
	if(iNbSpellsAfter > iNbSpells) {
		bookIconGuiRequestFX();
		bBookHalo = true;
		ulBookHaloTime = 0;
	}
}

/*!
 * \brief Remove _ulRune from player runes
 * \param _ulRune
 */
void ARX_Player_Rune_Remove(RuneFlag _ulRune)
{
	player.rune_flags &= ~_ulRune;
}

/*!
 * \brief Add quest "quest" to player Questbook
 * \param quest
 * \param _bLoad
 */
void ARX_PLAYER_Quest_Add(const std::string & quest, bool _bLoad) {
	
	PlayerQuest.push_back(STRUCT_QUEST());
	PlayerQuest.back().ident = quest;
	bBookHalo = !_bLoad;
	ulBookHaloTime = 0;
	
	gui::updateQuestBook();
}

/*!
 * \brief Removes player invisibility by killing Invisibility spells on him
 */
void ARX_PLAYER_Remove_Invisibility() {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist && spells[i].m_type == SPELL_INVISIBILITY && spells[i].m_caster == 0) {
			spells[i].m_tolive = 0;
		}
	}
}

/* TODO use this table instead of the copied functions below!
static const size_t max_skills = 9;
static const size_t max_attributes = 4;
static const float skill_attribute_factors[max_skills][max_attributes] = {
	// Str   Men   Dex   Con
	{ 0.0f, 0.0f, 2.0f, 0.0f }, // Stealth
	{ 0.0f, 1.0f, 1.0f, 0.0f }, // Technical
	{ 0.0f, 2.0f, 0.0f, 0.0f }, // Intuition
	{ 0.0f, 2.0f, 0.0f, 0.0f }, // Ethereal link
	{ 0.5f, 1.5f, 0.5f, 0.0f }, // Object knowledge
	{ 0.0f, 2.0f, 0.0f, 0.0f }, // Casting
	{ 2.0f, 0.0f, 1.0f, 0.0f }, // Close combat
	{ 1.0f, 0.0f, 2.0f, 0.0f }, // Projectile
	{ 0.0f, 0.0f, 0.0f, 1.0f }, // Defense
};
*/

/*!
 * \brief Compute secondary attributes for player
 */
static void ARX_PLAYER_ComputePlayerStats() {
	
	player.lifePool.max = (float)player.m_attribute.constitution * (float)(player.level + 2);
	player.manaPool.max = (float)player.m_attribute.mind * (float)(player.level + 1);
	
	float base_defense = player.m_skill.defense + player.m_attribute.constitution * 3;
	float fCalc = base_defense * ( 1.0f / 10 ) - 1 ;
	player.armor_class = checked_range_cast<unsigned char>(fCalc);


	if (player.armor_class < 1) player.armor_class = 1;

	float base_casting = player.m_skill.casting + player.m_attribute.mind * 2.f;
	player.resist_magic = (unsigned char)(float)(player.m_attribute.mind * 2.f
	                      * (1.f + base_casting * ( 1.0f / 200 )));

	fCalc = player.m_attribute.constitution * 2 + (base_defense * ( 1.0f / 4 ));
	player.resist_poison = checked_range_cast<unsigned char>(fCalc);


	player.damages = (player.m_attribute.strength - 10) * ( 1.0f / 2 );

	if (player.damages < 1.f) player.damages = 1.f;

	player.AimTime = 1500;
	
	float base_close_combat = player.m_skill.closeCombat
	                          + player.m_attribute.dexterity + player.m_attribute.strength * 2.f;
	player.Critical_Hit = (float)(player.m_attribute.dexterity - 9) * 2.f
	                      + base_close_combat * ( 1.0f / 5 );
}

extern long SPECIAL_PNUX;

/*!
 * \brief Compute FULL versions of player stats including Equiped Items and spells,
 *        and any other effect altering them.
 */
void ARX_PLAYER_ComputePlayerFullStats() {
	
	ARX_PLAYER_ComputePlayerStats();
	
	// Reset modifier values
	player.m_attributeMod.strength = 0;
	player.m_attributeMod.dexterity = 0;
	player.m_attributeMod.constitution = 0;
	player.m_attributeMod.mind = 0;
	player.m_skillMod.stealth = 0;
	player.m_skillMod.mecanism = 0;
	player.m_skillMod.intuition = 0;
	player.m_skillMod.etheralLink = 0;
	player.m_skillMod.objectKnowledge = 0;
	player.m_skillMod.casting = 0;
	player.m_skillMod.projectile = 0;
	player.m_skillMod.closeCombat = 0;
	player.m_skillMod.defense = 0;
	player.Mod_armor_class = 0;
	player.Mod_resist_magic = 0;
	player.Mod_resist_poison = 0;
	player.Mod_Critical_Hit = 0;
	player.Mod_damages = 0;
	
	// TODO why do this now and not after skills/stats have been calculated?
	ARX_EQUIPMENT_IdentifyAll();

	player.Full_Weapon_Type = ARX_EQUIPMENT_GetPlayerWeaponType();



	//CHECK OVERFLOW
	// TODO why not use relative modfiers?
	float fFullAimTime	= getEquipmentBaseModifier(IO_EQUIPITEM_ELEMENT_AimTime);
	float fCalcHandicap	= (player.Full_Attribute_Dexterity - 10.f) * 20.f;

	//CAST
	player.Full_AimTime = checked_range_cast<long>(fFullAimTime);

	if (player.Full_AimTime <= 0) player.Full_AimTime = player.AimTime;

	player.Full_AimTime -= checked_range_cast<long>(fCalcHandicap);


	if (player.Full_AimTime <= 1500) player.Full_AimTime = 1500;
	
	// TODO make these calculations moddable
	
	/////////////////////////////////////////////////////////////////////////////////////
	// External modifiers
	
	// Calculate for modifiers from spells
	if(entities.player()) {	
		boost::container::flat_set<long>::const_iterator it;
		for(it = entities.player()->spellsOn.begin(); it != entities.player()->spellsOn.end(); ++it) {
			
			long spellHandle = *it;
			if(!spellHandleIsValid(spellHandle)) {
				continue;
			}
			
			long n = spellHandle;
			switch (spells[n].m_type) {
				case SPELL_ARMOR:
					player.Mod_armor_class += spells[n].m_caster_level;
					break;
				case SPELL_LOWER_ARMOR:
					player.Mod_armor_class -= spells[n].m_caster_level;
					break;
				case SPELL_CURSE:
					player.m_attributeMod.strength -= spells[n].m_caster_level;
					player.m_attributeMod.constitution -= spells[n].m_caster_level;
					player.m_attributeMod.dexterity -= spells[n].m_caster_level;
					player.m_attributeMod.mind -= spells[n].m_caster_level;
					break;
				case SPELL_BLESS:
					player.m_attributeMod.strength += spells[n].m_caster_level;
					player.m_attributeMod.dexterity += spells[n].m_caster_level;
					player.m_attributeMod.constitution += spells[n].m_caster_level;
					player.m_attributeMod.mind += spells[n].m_caster_level;
					break;
				default: break;
			}
		}
	}
	
	// Calculate for modifiers from cheats
	if(cur_mr == 3) {
		player.m_attributeMod.strength += 1;
		player.m_attributeMod.mind += 10;
		player.m_attributeMod.constitution += 1;
		player.m_attributeMod.dexterity += 10;
		player.m_skillMod.stealth += 5;
		player.m_skillMod.mecanism += 5;
		player.m_skillMod.intuition += 100;
		player.m_skillMod.etheralLink += 100;
		player.m_skillMod.objectKnowledge += 100;
		player.m_skillMod.casting += 5;
		player.m_skillMod.projectile += 5;
		player.m_skillMod.closeCombat += 5;
		player.m_skillMod.defense += 100;
		player.Mod_resist_magic += 100;
		player.Mod_resist_poison += 100;
		player.Mod_Critical_Hit += 5;
		player.Mod_damages += 2;
		player.Mod_armor_class += 100;
		player.Full_AimTime = 100;
	}
	if(sp_max) {
		player.m_attributeMod.strength += 5;
		player.m_attributeMod.mind += 5;
		player.m_attributeMod.constitution += 5;
		player.m_attributeMod.dexterity += 5;
		player.m_skillMod.stealth += 50;
		player.m_skillMod.mecanism += 50;
		player.m_skillMod.intuition += 50;
		player.m_skillMod.etheralLink += 50;
		player.m_skillMod.objectKnowledge += 50;
		player.m_skillMod.casting += 50;
		player.m_skillMod.projectile += 50;
		player.m_skillMod.closeCombat += 50;
		player.m_skillMod.defense += 50;
		player.Mod_resist_magic += 10;
		player.Mod_resist_poison += 10;
		player.Mod_Critical_Hit += 50;
		player.Mod_damages += 10;
		player.Mod_armor_class += 20;
		player.Full_AimTime = 100;
	}
	if(SPECIAL_PNUX) {
		player.m_attributeMod.strength += Random::get(0, 5);
		player.m_attributeMod.mind += Random::get(0, 5);
		player.m_attributeMod.constitution += Random::get(0, 5);
		player.m_attributeMod.dexterity += Random::get(0, 5);
		player.m_skillMod.stealth += Random::get(0, 20);
		player.m_skillMod.mecanism += Random::get(0, 20);
		player.m_skillMod.intuition += Random::get(0, 20);
		player.m_skillMod.etheralLink += Random::get(0, 20);
		player.m_skillMod.objectKnowledge += Random::get(0, 20);
		player.m_skillMod.casting += Random::get(0, 20);
		player.m_skillMod.projectile += Random::get(0, 20);
		player.m_skillMod.closeCombat += Random::get(0, 20);
		player.m_skillMod.defense += Random::get(0, 30);
		player.Mod_resist_magic += Random::get(0, 20);
		player.Mod_resist_poison += Random::get(0, 20);
		player.Mod_Critical_Hit += Random::get(0, 20);
		player.Mod_damages += Random::get(0, 20);
		player.Mod_armor_class += Random::get(0, 20);
	}
	if(cur_rf == 3) {
		player.m_attributeMod.mind += 10;
		player.m_skillMod.casting += 100;
		player.m_skillMod.etheralLink += 100;
		player.m_skillMod.objectKnowledge += 100;
		player.Mod_resist_magic += 20;
		player.Mod_resist_poison += 20;
		player.Mod_damages += 1;
		player.Mod_armor_class += 5;
	}
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Attributes
	
	// Calculate base attributes
	float base_strength     = player.m_attribute.strength;
	float base_dexterity    = player.m_attribute.dexterity;
	float base_constitution = player.m_attribute.constitution;
	float base_mind         = player.m_attribute.mind;
	
	// Calculate equipment modifiers for attributes
	player.m_attributeMod.strength += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_STRENGTH, base_strength
	);
	player.m_attributeMod.dexterity += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_DEXTERITY, base_dexterity
	);
	player.m_attributeMod.constitution += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_CONSTITUTION, base_constitution
	);
	player.m_attributeMod.mind += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_MIND, base_mind
	);
	
	// Calculate full alltributes
	player.Full_Attribute_Strength = std::max(0.f, base_strength + player.m_attributeMod.strength);
	player.Full_Attribute_Dexterity = std::max(0.f, base_dexterity + player.m_attributeMod.dexterity);
	player.Full_Attribute_Constitution = std::max(0.f, base_constitution + player.m_attributeMod.constitution);
	player.Full_Attribute_Mind = std::max(0.f, base_mind + player.m_attributeMod.mind);
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Skills
	
	// Calculate base skills
	float base_stealth          = player.m_skill.stealth
	                              + player.Full_Attribute_Dexterity * 2.f;
	float base_mecanism         = player.m_skill.mecanism
	                              + player.Full_Attribute_Dexterity
	                              + player.Full_Attribute_Mind;
	float base_intuition        = player.m_skill.intuition
	                              + player.Full_Attribute_Mind * 2.f;
	float base_ethereal_link    = player.m_skill.etheralLink
	                              + player.Full_Attribute_Mind * 2.f;
	float base_object_knowledge = player.m_skill.objectKnowledge
	                              + player.Full_Attribute_Mind * 1.5f
	                              + player.Full_Attribute_Dexterity * 0.5f
	                              + player.Full_Attribute_Strength * 0.5f;
	float base_casting          = player.m_skill.casting
	                              + player.Full_Attribute_Mind * 2.f;
	float base_projectile       = player.m_skill.projectile
	                              + player.Full_Attribute_Dexterity * 2.f
	                              + player.Full_Attribute_Strength;
	float base_close_combat     = player.m_skill.closeCombat
	                              + player.Full_Attribute_Dexterity
	                              + player.Full_Attribute_Strength * 2.f;
	float base_defense          = player.m_skill.defense
	                              + player.Full_Attribute_Constitution * 3;
	
	// Calculate equipment modifiers for skills
	player.m_skillMod.stealth += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Stealth, base_stealth
	);
	player.m_skillMod.mecanism += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Mecanism, base_mecanism
	);
	player.m_skillMod.intuition += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Intuition, base_intuition
	);
	player.m_skillMod.etheralLink += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Etheral_Link, base_ethereal_link
	);
	player.m_skillMod.objectKnowledge += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Object_Knowledge, base_object_knowledge
	);
	player.m_skillMod.casting += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Casting, base_casting
	);
	player.m_skillMod.projectile += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Projectile, base_projectile
	);
	player.m_skillMod.closeCombat += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Close_Combat, base_close_combat
	);
	player.m_skillMod.defense += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Defense, base_defense
	);
	
	// Calculate full skills
	player.Full_Skill_Stealth = base_stealth + player.m_skillMod.stealth;
	player.Full_Skill_Mecanism = base_mecanism + player.m_skillMod.mecanism;
	player.Full_Skill_Intuition = base_intuition + player.m_skillMod.intuition;
	player.Full_Skill_Etheral_Link = base_ethereal_link + player.m_skillMod.etheralLink;
	player.Full_Skill_Object_Knowledge = base_object_knowledge + player.m_skillMod.objectKnowledge;
	player.Full_Skill_Casting = base_casting + player.m_skillMod.casting;
	player.Full_Skill_Projectile = base_projectile + player.m_skillMod.projectile;
	player.Full_Skill_Close_Combat = base_close_combat + player.m_skillMod.closeCombat;
	player.Full_Skill_Defense = base_defense + player.m_skillMod.defense;
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Other stats
	
	// Calculate base stats
	float base_armor_class   = std::max(1.f, player.Full_Skill_Defense * 0.1f
	                           + -1.0f);
	float base_resist_magic  = player.Full_Attribute_Mind * 2.f
	                           * (1.f + player.Full_Skill_Casting * 0.005f); // TODO why *?
	float base_resist_poison = player.Full_Attribute_Constitution * 2.f
	                           + player.Full_Skill_Defense * 0.25f;
	float base_critical_hit  = player.Full_Attribute_Dexterity * 2.f
	                           + player.Full_Skill_Close_Combat * 0.2f
	                           + -18.f;
	float base_damages       = std::max(1.f, player.Full_Attribute_Strength * 0.5f
	                           + player.Full_Skill_Close_Combat * 0.1f
	                           + -5.f);
	
	// Calculate equipment modifiers for stats
	player.Mod_armor_class += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Armor_Class, base_armor_class
	);
	player.Mod_resist_magic += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Resist_Magic, base_resist_magic
	);
	player.Mod_resist_poison += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Resist_Poison, base_resist_poison
	);
	player.Mod_Critical_Hit += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Critical_Hit, base_critical_hit
	);
	player.Mod_damages += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Damages, base_damages
	);
	
	// Calculate full stats
	player.Full_armor_class = (int)std::max(0.f, base_armor_class + player.Mod_armor_class);
	player.Full_resist_magic = (int)std::max(0.f, base_resist_magic + player.Mod_resist_magic);
	player.Full_resist_poison = (int)std::max(0.f, base_resist_poison + player.Mod_resist_poison);
	player.Full_Critical_Hit = std::max(0.f, base_critical_hit + player.Mod_Critical_Hit);
	player.Full_damages = std::max(0.f, base_damages + player.Mod_damages);
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	
	player.Full_life = player.lifePool.current;
	player.Full_maxlife = (float)player.Full_Attribute_Constitution * (float)(player.level + 2);
	player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
	player.Full_maxmana = (float)player.Full_Attribute_Mind * (float)(player.level + 1);
	player.manaPool.current = std::min(player.manaPool.current, player.Full_maxmana);
}

/*!
 * \brief Creates a Fresh hero
 */
void ARX_PLAYER_MakeFreshHero()
{
	player.m_attribute.strength = 6;
	player.m_attribute.mind = 6;
	player.m_attribute.dexterity = 6;
	player.m_attribute.constitution = 6;

	player.Old_Skill_Stealth			=	player.m_skill.stealth			= 0;
	player.Old_Skill_Mecanism			=	player.m_skill.mecanism			= 0;
	player.Old_Skill_Intuition			=	player.m_skill.intuition			= 0;
	player.Old_Skill_Etheral_Link		=	player.m_skill.etheralLink		= 0;
	player.Old_Skill_Object_Knowledge	=	player.m_skill.objectKnowledge	= 0;
	player.Old_Skill_Casting			=	player.m_skill.casting			= 0;
	player.Old_Skill_Projectile			=	player.m_skill.projectile			= 0;
	player.Old_Skill_Close_Combat		=	player.m_skill.closeCombat		= 0;
	player.Old_Skill_Defense			=	player.m_skill.defense			= 0;

	player.Attribute_Redistribute = 16;
	player.Skill_Redistribute = 18;

	player.level = 0;
	player.xp = 0;
	player.poison = 0.f;
	player.hunger = 100.f;
	player.skin = 0;
	player.bag = 1;

	ARX_PLAYER_ComputePlayerStats();
	player.rune_flags = 0;

	player.SpellToMemorize.bSpell = false;
}
s8 SKIN_MOD = 0;
char QUICK_MOD = 0;

void ARX_SPSound() {
	ARX_SOUND_PlayCinematic("kra_zoha_equip", false);
}

void ARX_PLAYER_MakeSpHero()
{
	ARX_SPSound();
	player.m_attribute.strength = 12;
	player.m_attribute.mind = 12;
	player.m_attribute.dexterity = 12;
	player.m_attribute.constitution = 12;

	player.Old_Skill_Stealth			=	player.m_skill.stealth			= 5;
	player.Old_Skill_Mecanism			=	player.m_skill.mecanism			= 5;
	player.Old_Skill_Intuition			=	player.m_skill.intuition			= 5;
	player.Old_Skill_Etheral_Link		=	player.m_skill.etheralLink		= 5;
	player.Old_Skill_Object_Knowledge	=	player.m_skill.objectKnowledge	= 5;
	player.Old_Skill_Casting			=	player.m_skill.casting			= 5;
	player.Old_Skill_Projectile			=	player.m_skill.projectile			= 5;
	player.Old_Skill_Close_Combat		=	player.m_skill.closeCombat		= 5;
	player.Old_Skill_Defense			=	player.m_skill.defense			= 5;

	player.Attribute_Redistribute = 6;
	player.Skill_Redistribute = 10;

	player.level = 1;
	player.xp = 0;
	player.poison = 0.f;
	player.hunger = 100.f;
	player.skin = 4;

	ARX_PLAYER_ComputePlayerStats();
	player.lifePool.current = player.lifePool.max;
	player.manaPool.current = player.manaPool.max;

	player.rune_flags = RuneFlags::all();
	player.SpellToMemorize.bSpell = false;

	SKIN_MOD = 0;
	QUICK_MOD = 0;
}

/*!
 * \brief Creates an Average hero
 */
void ARX_PLAYER_MakeAverageHero()
{
	ARX_PLAYER_MakeFreshHero();

	player.m_attribute.strength		+= 4;
	player.m_attribute.mind			+= 4;
	player.m_attribute.dexterity		+= 4;
	player.m_attribute.constitution	+= 4;

	player.m_skill.stealth			+= 2;
	player.m_skill.mecanism			+= 2;
	player.m_skill.intuition			+= 2;
	player.m_skill.etheralLink		+= 2;
	player.m_skill.objectKnowledge	+= 2;
	player.m_skill.casting			+= 2;
	player.m_skill.projectile			+= 2;
	player.m_skill.closeCombat		+= 2;
	player.m_skill.defense			+= 2;

	player.Attribute_Redistribute = 0;
	player.Skill_Redistribute = 0;

	player.level = 0;
	player.xp = 0;
	player.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

/*!
 * \brief Quickgenerate a random hero
 */
void ARX_PLAYER_QuickGeneration() {
	
	char old_skin = player.skin;
	ARX_PLAYER_MakeFreshHero();
	player.skin = old_skin;

	while(player.Attribute_Redistribute) {
		float rn = rnd();

		if(rn < 0.25f && player.m_attribute.strength < 18) {
			player.m_attribute.strength++;
			player.Attribute_Redistribute--;
		} else if(rn < 0.5f && player.m_attribute.mind < 18) {
			player.m_attribute.mind++;
			player.Attribute_Redistribute--;
		} else if(rn < 0.75f && player.m_attribute.dexterity < 18) {
			player.m_attribute.dexterity++;
			player.Attribute_Redistribute--;
		} else if(player.m_attribute.constitution < 18) {
			player.m_attribute.constitution++;
			player.Attribute_Redistribute--;
		}
	}

	while(player.Skill_Redistribute) {
		float rn = rnd();

		if(rn < 0.1f && player.m_skill.stealth < 18) {
			player.m_skill.stealth++;
			player.Skill_Redistribute--;
		} else if(rn < 0.2f && player.m_skill.mecanism < 18) {
			player.m_skill.mecanism++;
			player.Skill_Redistribute--;
		} else if(rn < 0.3f && player.m_skill.intuition < 18) {
			player.m_skill.intuition++;
			player.Skill_Redistribute--;
		} else if(rn < 0.4f && player.m_skill.etheralLink < 18) {
			player.m_skill.etheralLink++;
			player.Skill_Redistribute--;
		} else if(rn < 0.5f && player.m_skill.objectKnowledge < 18) {
			player.m_skill.objectKnowledge++;
			player.Skill_Redistribute--;
		} else if(rn < 0.6f && player.m_skill.casting < 18) {
			player.m_skill.casting++;
			player.Skill_Redistribute--;
		} else if(rn < 0.7f && player.m_skill.projectile < 18) {
			player.m_skill.projectile++;
			player.Skill_Redistribute--;
		} else if(rn < 0.8f && player.m_skill.closeCombat < 18) {
			player.m_skill.closeCombat++;
			player.Skill_Redistribute--;
		} else if(rn < 0.9f && player.m_skill.defense < 18) {
			player.m_skill.defense++;
			player.Skill_Redistribute--;
		}
	}

	player.level = 0;
	player.xp = 0;
	player.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

/*!
 * \brief Returns necessary Experience for a given level
 * \param level
 * \return
 */
long GetXPforLevel(long level)
{
	const long XP_FOR_LEVEL[] = { 
		0, 
		2000,
		4000,
		6000,
		10000,
		16000,
		26000,
		42000,
		68000,
		110000,
		178000,
		300000,
		450000,
		600000,
		750000 
	};

	long xpNeeded;
	if(level < (long)ARRAY_SIZE(XP_FOR_LEVEL))
		xpNeeded = XP_FOR_LEVEL[level];
	else
		xpNeeded = level * 60000;
	return xpNeeded;	
}

/*!
 * \brief Manages Player Level Up event
 */
void ARX_PLAYER_LEVEL_UP()
{
	ARX_SOUND_PlayInterface(SND_PLAYER_LEVEL_UP);
	player.level++;
	player.Skill_Redistribute += 15;
	player.Attribute_Redistribute++;
	ARX_PLAYER_ComputePlayerStats();
	player.lifePool.current = player.lifePool.max;
	player.manaPool.current = player.manaPool.max;
	player.Old_Skill_Stealth			=	player.m_skill.stealth;
	player.Old_Skill_Mecanism			=	player.m_skill.mecanism;
	player.Old_Skill_Intuition			=	player.m_skill.intuition;
	player.Old_Skill_Etheral_Link		=	player.m_skill.etheralLink;
	player.Old_Skill_Object_Knowledge	=	player.m_skill.objectKnowledge;
	player.Old_Skill_Casting			=	player.m_skill.casting;
	player.Old_Skill_Projectile			=	player.m_skill.projectile;
	player.Old_Skill_Close_Combat		=	player.m_skill.closeCombat;
	player.Old_Skill_Defense			=	player.m_skill.defense;
	SendIOScriptEvent(entities.player(), SM_NULL, "", "level_up");
}

/*!
 * \brief Modify player XP by adding "val" to it
 * \param val
 */
void ARX_PLAYER_Modify_XP(long val) {
	
	player.xp += val;
	
	for(long i = player.level + 1; i < 11; i++) {
		if(player.xp >= GetXPforLevel(i)) {
			ARX_PLAYER_LEVEL_UP();
		}
	}
}

/*!
 * \brief Function to poison player by "val" poison level
 * \param val
 */
void ARX_PLAYER_Poison(float val) {
	// Make a poison saving throw to see if player is affected
	if(rnd() * 100.f > player.resist_poison) {
		player.poison += val;
		ARX_SOUND_PlayInterface(SND_PLAYER_POISONED);
	}
}

long PLAYER_PARALYSED = 0;

/*!
 * \brief updates some player stats depending on time
 * \param Framedelay
 *
 * Updates: life/mana recovery, poison evolution, hunger, invisibility
 */
void ARX_PLAYER_FrameCheck(float Framedelay)
{
	//	ARX_PLAYER_QuickGeneration();
	if(Framedelay > 0) {
		UpdateIOInvisibility(entities.player());
		// Natural LIFE recovery
		float inc = 0.00008f * Framedelay * (player.Full_Attribute_Constitution + player.Full_Attribute_Strength * ( 1.0f / 2 ) + player.Full_Skill_Defense) * ( 1.0f / 50 );

		if(player.lifePool.current > 0.f) {
			float inc_hunger = 0.00008f * Framedelay * (player.Full_Attribute_Constitution + player.Full_Attribute_Strength * ( 1.0f / 2 )) * ( 1.0f / 50 );

			// Check for player hungry sample playing
			if((player.hunger > 10.f && player.hunger - inc_hunger <= 10.f)
					|| (player.hunger < 10.f && float(arxtime) > LastHungerSample + 180000))
			{
				LastHungerSample = (unsigned long)(arxtime);

				if(!BLOCK_PLAYER_CONTROLS) {
					bool bOk = true;

					for(size_t i = 0; i < MAX_ASPEECH; i++) {
						if(aspeech[i].exist && (aspeech[i].io == entities.player())) {
							bOk = false;
						}
					}

					if(bOk)
						ARX_SPEECH_AddSpeech(entities.player(), "player_off_hungry", ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
				}
			}

			player.hunger -= inc_hunger * .5f; //*.7f;

			if(player.hunger < -10.f)
				player.hunger = -10.f;

			if(!BLOCK_PLAYER_CONTROLS) {
				if(player.hunger < 0.f)
					player.lifePool.current -= inc * ( 1.0f / 2 );
				else
					player.lifePool.current += inc;
			}

			// Natural MANA recovery
			player.manaPool.current += 0.00008f * Framedelay * ((player.Full_Attribute_Mind + player.Full_Skill_Etheral_Link) * 10) * ( 1.0f / 100 ); //framedelay*( 1.0f / 1000 );

			if(player.manaPool.current > player.Full_maxmana)
				player.manaPool.current = player.Full_maxmana;
		}

		//float pmaxlife=(float)player.Full_Attribute_Constitution*(float)(player.level+2);
		if(player.lifePool.current > player.Full_maxlife)
			player.lifePool.current = player.Full_maxlife;

		// Now Checks Poison Progression
		if(!BLOCK_PLAYER_CONTROLS)
			if(player.poison > 0.f) {
				float cp = player.poison;
				cp *= ( 1.0f / 2 ) * Framedelay * ( 1.0f / 1000 ) * ( 1.0f / 2 );
				float faster = 10.f - player.poison;

				if(faster < 0.f)
					faster = 0.f;

				if(rnd() * 100.f > player.resist_poison + faster) {
					float dmg = cp * ( 1.0f / 3 );

					if(player.lifePool.current - dmg <= 0.f)
						ARX_DAMAGES_DamagePlayer(dmg, DAMAGE_TYPE_POISON, -1);
					else
						player.lifePool.current -= dmg;

					player.poison -= cp * ( 1.0f / 10 );
				} else {
					player.poison -= cp;
				}
			}

		if(player.poison < 0.1f)
			player.poison = 0.f;
	}
}
TextureContainer * PLAYER_SKIN_TC = NULL;

void ARX_PLAYER_Restore_Skin() {
	
	res::path tx;
	res::path tx2;
	res::path tx3;
	res::path tx4;
	
	switch(player.skin) {
		case 0:
			tx  = "graph/obj3d/textures/npc_human_base_hero_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero_head";
			break;
		case 1:
			tx  = "graph/obj3d/textures/npc_human_base_hero2_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero2_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero2_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero2_head";
			break;
		case 2:
			tx  = "graph/obj3d/textures/npc_human_base_hero3_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero3_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero3_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero3_head";
			break;
		case 3:
			tx  = "graph/obj3d/textures/npc_human_base_hero4_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero4_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero4_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero4_head";
			break;
		case 4:
			tx  = "graph/obj3d/textures/npc_human_cm_hero_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero_head";
			break;
		case 5:
			tx  = "graph/obj3d/textures/npc_human__base_hero_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero_head";
			break;
		case 6: //just in case
			tx  = "graph/obj3d/textures/npc_human__base_hero_head";
			tx2 = "graph/obj3d/textures/npc_human_chainmail_hero_head";
			tx3 = "graph/obj3d/textures/npc_human_chainmail_mithril_hero_head";
			tx4 = "graph/obj3d/textures/npc_human_leather_hero_head";
			break;
	}

	TextureContainer * tmpTC;
	
	// TODO maybe it would be better to replace the textures in the player object instead of replacing the texture data for all objects that use these textures

	if(PLAYER_SKIN_TC && !tx.empty())
		PLAYER_SKIN_TC->LoadFile(tx);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_hero_head");
	if(tmpTC && !tx2.empty())
		tmpTC->LoadFile(tx2);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_mithril_hero_head");
	if(tmpTC && !tx3.empty())
		tmpTC->LoadFile(tx3);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_leather_hero_head");
	if(tmpTC && !tx4.empty())
		tmpTC->LoadFile(tx4);
}

/*!
 * \brief Load Mesh & anims for hero
 */
void ARX_PLAYER_LoadHeroAnimsAndMesh(){
	
	const char OBJECT_HUMAN_BASE[] = "graph/obj3d/interactive/npc/human_base/human_base.teo"; 
	hero = loadObject(OBJECT_HUMAN_BASE, false);
	PLAYER_SKIN_TC = TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero_head");

	const char ANIM_WAIT_BOOK[] = "graph/obj3d/anims/npc/human_wait_book.tea";
	herowaitbook = EERIE_ANIMMANAGER_Load(ANIM_WAIT_BOOK);
	const char ANIM_WAIT_NORMAL[] = "graph/obj3d/anims/npc/human_normal_wait.tea";
	EERIE_ANIMMANAGER_Load(ANIM_WAIT_NORMAL);
	const char ANIM_WAIT_TWOHANDED[] = "graph/obj3d/anims/npc/human_wait_book_2handed.tea";
	herowait_2h = EERIE_ANIMMANAGER_Load(ANIM_WAIT_TWOHANDED);
	
	Entity * io = new Entity("graph/obj3d/interactive/player/player");
	arx_assert(io->index() == 0, "player entity didn't get index 0");
	arx_assert(entities.player() == io);
	
	io->obj = hero;

	player.skin = 0;
	ARX_PLAYER_Restore_Skin();

	ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	ARX_INTERACTIVE_HideGore(entities.player(), 1);
	io->ident = -1;
	
	ANIM_Set(&player.bookAnimation[0], herowaitbook);
	player.bookAnimation[0].flags |= EA_LOOP;
	
	//todo free
	io->_npcdata = new IO_NPCDATA;
	
	io->ioflags = IO_NPC;
	io->_npcdata->lifePool.max = io->_npcdata->lifePool.current = 10.f;
	io->_npcdata->vvpos = -99999.f;

	//todo free
	io->armormaterial = "leather";
	loadScript(io->script, resources->getFile("graph/obj3d/interactive/player/player.asl"));

	if ((EERIE_OBJECT_GetGroup(io->obj, "head") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "neck") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "chest") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "belt") != -1))
	{
		io->_npcdata->ex_rotate = new EERIE_EXTRA_ROTATE();
		
		io->_npcdata->ex_rotate->group_number[0] = (short)EERIE_OBJECT_GetGroup(io->obj, "head");
		io->_npcdata->ex_rotate->group_number[1] = (short)EERIE_OBJECT_GetGroup(io->obj, "neck");
		io->_npcdata->ex_rotate->group_number[2] = (short)EERIE_OBJECT_GetGroup(io->obj, "chest");
		io->_npcdata->ex_rotate->group_number[3] = (short)EERIE_OBJECT_GetGroup(io->obj, "belt");
		
		for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
		{
			io->_npcdata->ex_rotate->group_rotate[n] = Anglef::ZERO;
		}
		
		io->_npcdata->ex_rotate->flags = 0;
	}

	ARX_INTERACTIVE_RemoveGoreOnIO(entities.player());
}

float Falling_Height = 0;
void ARX_PLAYER_StartFall()
{
	FALLING_TIME = 1;
	Falling_Height = 50.f;
	EERIEPOLY * ep = CheckInPoly(player.pos);

	if(ep) {
		Falling_Height = player.pos.y;
	}
}

/*!
 * \brief Called When player has just died
 */
void ARX_PLAYER_BecomesDead() {
	// a mettre au final
	BLOCK_PLAYER_CONTROLS = true;

	if(entities.player()) {
		player.Interface &= ~INTER_COMBATMODE;
		player.Interface = 0;
		DeadTime = 0;
	}

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist && spells[i].m_caster == 0) {
			spells[i].m_tolive = 0;
		}
	}
}

float LASTPLAYERA = 0;
extern long ON_PLATFORM;
long LAST_ON_PLATFORM = 0;
extern long MOVE_PRECEDENCE;
extern bool EXTERNALVIEW;

/*!
 * \brief Choose the set of animations to use to represent current player situation.
 */
void ARX_PLAYER_Manage_Visual() {
	
	unsigned long tim = (unsigned long)(arxtime);
	
	if(player.Current_Movement & PLAYER_ROTATE) {
		if(ROTATE_START == 0) {
			ROTATE_START = tim;
		}
	} else if (ROTATE_START) {
		float diff = (float)tim - (float)ROTATE_START;
		if(diff > 100) {
			ROTATE_START = 0;
		}
	}
	
	if(entities.player()) {
		
		Entity * io = entities.player();
		
		if(!BLOCK_PLAYER_CONTROLS && sp_max) {
			io->halo.color = Color3f::red;
			io->halo.flags |= HALO_ACTIVE | HALO_DYNLIGHT;
			io->halo.radius = 20.f;
			player.lifePool.current += float(framedelay) * 0.1f;
			player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
			player.manaPool.current += float(framedelay) * 0.1f;
			player.manaPool.current = std::min(player.manaPool.current, player.Full_maxmana);
		}
		
		if(cur_mr == 3) {
			player.lifePool.current += float(framedelay) * 0.05f;
			player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
			player.manaPool.current += float(framedelay) * 0.05f;
			player.manaPool.current = std::min(player.manaPool.current, player.Full_maxmana);
		}
		
		io->pos = player.basePosition();
		
		if(player.jumpphase == NotJumping && !LAST_ON_PLATFORM) {
			float t;
			EERIEPOLY * ep = CheckInPoly(player.pos, &t);
			if(ep && io->pos.y > t - 30.f && io->pos.y < t) {
				player.onfirmground = true;
			}
		}
		
		ComputeVVPos(io);
		io->pos.y = io->_npcdata->vvpos;
		
		if(!(player.Current_Movement & PLAYER_CROUCH) && player.physics.cyl.height > -150.f) {
			float old = player.physics.cyl.height;
			player.physics.cyl.height = player.baseHeight();
			player.physics.cyl.origin = player.basePosition();
			float anything = CheckAnythingInCylinder(player.physics.cyl, entities.player());
			if(anything < 0.f) {
				player.Current_Movement |= PLAYER_CROUCH;
				player.physics.cyl.height = old;
			}
		}
		
		if(player.lifePool.current > 0) {
			io->angle = Anglef(0.f, 180.f - player.angle.getPitch(), 0.f);
		}
		
		io->gameFlags |= GFLAG_ISINTREATZONE;
		
		ANIM_USE * ause0 = &io->animlayer[0];
		ANIM_USE * ause1 = &io->animlayer[1];
		ANIM_USE * ause3 = &io->animlayer[3];
		
		ause0->next_anim = NULL;
		entities.player()->animlayer[1].next_anim = NULL;
		entities.player()->animlayer[2].next_anim = NULL;
		entities.player()->animlayer[3].next_anim = NULL;
		ANIM_HANDLE ** alist = io->anims;
		
		if(ause0->flags & EA_FORCEPLAY) {
			if(ause0->flags & EA_ANIMEND) {
				ause0->flags &= ~EA_FORCEPLAY;
				ause0->flags |= EA_STATICANIM;
				io->move = io->lastmove = Vec3f_ZERO;
			} else {
				ause0->flags &= ~EA_STATICANIM;
				player.pos = moveto = player.pos + io->move;
				io->pos = player.basePosition();
				goto nochanges;
			}
		}
		
		ANIM_HANDLE * ChangeMoveAnim = NULL;
		ANIM_HANDLE * ChangeMoveAnim2 = NULL;
		bool ChangeMA_Loop = true;
		bool ChangeMA_Stopend = false;
		
		if(io->ioflags & IO_FREEZESCRIPT) {
			goto nochanges;
		}
		
		if(player.lifePool.current <= 0) {
			HERO_SHOW_1ST = -1;
			io->animlayer[1].cur_anim = NULL;
			ChangeMoveAnim = alist[ANIM_DIE];
			ChangeMA_Loop = false;
			ChangeMA_Stopend = true;
			goto makechanges;
		}
		
		if(player.Current_Movement == 0 || player.Current_Movement == PLAYER_MOVE_STEALTH
		   || (player.Current_Movement & PLAYER_ROTATE)) {
			if(player.Interface & INTER_COMBATMODE) {
				ChangeMoveAnim = alist[ANIM_FIGHT_WAIT];
			} else if(EXTERNALVIEW) {
				ChangeMoveAnim = alist[ANIM_WAIT];
			} else {
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];
			}

			ChangeMA_Loop = true;
		}
		
		if(ROTATE_START
		   && player.angle.getYaw() > 60.f
		   && player.angle.getYaw() < 180.f
		   && LASTPLAYERA > 60.f
		   && LASTPLAYERA < 180.f
		) {
			if(PLAYER_ROTATION < 0) {
				if(player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT_FIGHT];
				else
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT];
			} else {
				if(player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT_FIGHT];
				else
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT];
			}

			ChangeMA_Loop = true;

			if(ause0->cur_anim == alist[ANIM_U_TURN_LEFT]
			   || ause0->cur_anim == alist[ANIM_U_TURN_LEFT_FIGHT])
			{
				float fv = PLAYER_ROTATION * 5;
				long vv = fv;
				io->frameloss -= fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime -= vv;

				if(ause0->ctime < 0)
					ause0->ctime = 0;
			}
			else if(ause0->cur_anim == alist[ANIM_U_TURN_RIGHT]
					 ||	ause0->cur_anim == alist[ANIM_U_TURN_RIGHT_FIGHT])
			{
				long vv = PLAYER_ROTATION * 5;
				float fv = PLAYER_ROTATION * 5;
				io->frameloss += fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime += vv;

				if(ause0->ctime < 0)
					ause0->ctime = 0;
			}
		}

		LASTPLAYERA = player.angle.getYaw();

		{
			long tmove = player.Current_Movement;

			if((tmove & PLAYER_MOVE_STRAFE_LEFT) && (tmove & PLAYER_MOVE_STRAFE_RIGHT)) {
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;
			}

			if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT)
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;

			if(MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT)
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;

			if(MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD)
				tmove &= ~PLAYER_MOVE_WALK_BACKWARD;

			if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				tmove = PLAYER_MOVE_WALK_FORWARD;

				if(tmove & PLAYER_MOVE_STRAFE_LEFT) {
					if(player.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_LEFT];
					else if(player.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_LEFT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_LEFT];
				}

				if(tmove & PLAYER_MOVE_STRAFE_RIGHT) {
					if(player.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_RIGHT];
					else if(player.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_RIGHT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_RIGHT];
				}

			if(tmove & PLAYER_MOVE_WALK_BACKWARD) {
				if(player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_BACKWARD];
				else if(player.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else if(player.Current_Movement & PLAYER_CROUCH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else
					ChangeMoveAnim = alist[ANIM_RUN_BACKWARD];
			}

			if(tmove & PLAYER_MOVE_WALK_FORWARD) {
				if(player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_FORWARD];
				else if(player.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK];
				else
					ChangeMoveAnim = alist[ANIM_RUN];
			}
		}

		if(!ChangeMoveAnim) {
			if(EXTERNALVIEW)
				ChangeMoveAnim = alist[ANIM_WAIT];
			else
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];

			ChangeMA_Loop = true;
		}

		// Finally update anim
		if(ause1->cur_anim == NULL
		   && (ause0->cur_anim == alist[ANIM_WAIT] || ause0->cur_anim == alist[ANIM_WAIT_SHORT])
		   && !(player.Current_Movement & PLAYER_CROUCH)
		) {
			if ((player.Current_Movement & PLAYER_LEAN_LEFT)
			        &&	(player.Current_Movement & PLAYER_LEAN_RIGHT))
			{
			} else {
				if(player.Current_Movement & PLAYER_LEAN_LEFT) {
					ChangeMoveAnim2 = alist[ANIM_LEAN_LEFT];
					//ChangeMA_Loop=0;
				}

				if(player.Current_Movement & PLAYER_LEAN_RIGHT) {
					ChangeMoveAnim2 = alist[ANIM_LEAN_RIGHT];
				}
			}
		}

		if(ChangeMoveAnim2 == NULL
		   && ause3->cur_anim
		   && (ause3->cur_anim == alist[ANIM_LEAN_RIGHT] || ause3->cur_anim == alist[ANIM_LEAN_LEFT])
		) {
			AcquireLastAnim(io);
			ause3->cur_anim = NULL;
		}

		if((player.Current_Movement & PLAYER_CROUCH) && !(player.Last_Movement & PLAYER_CROUCH)
		        && !player.levitate)
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_START];
			ChangeMA_Loop = false;
		}
		else if(!(player.Current_Movement & PLAYER_CROUCH) && (player.Last_Movement & PLAYER_CROUCH))
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_END];
			ChangeMA_Loop = false;
		} else if(player.Current_Movement & PLAYER_CROUCH) {
			if(ause0->cur_anim == alist[ANIM_CROUCH_START]) {
				if(!(ause0->flags & EA_ANIMEND)) {
					ChangeMoveAnim = alist[ANIM_CROUCH_START];
					ChangeMA_Loop = false;
				} else {
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = true;
					player.physics.cyl.height = player.crouchHeight();
				}
			} else {
				if(ChangeMoveAnim == alist[ANIM_STRAFE_LEFT]
				   || ChangeMoveAnim == alist[ANIM_STRAFE_RUN_LEFT]
				   || ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_LEFT]
				) {
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_LEFT];
					ChangeMA_Loop = true;
				} else if(ChangeMoveAnim == alist[ANIM_STRAFE_RIGHT]
						 || ChangeMoveAnim == alist[ANIM_STRAFE_RUN_RIGHT]
						 || ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_RIGHT]
				) {
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_RIGHT];
					ChangeMA_Loop = true;
				} else if(ChangeMoveAnim == alist[ANIM_WALK]
						 || ChangeMoveAnim == alist[ANIM_RUN]
						 || ChangeMoveAnim == alist[ANIM_FIGHT_WALK_FORWARD]
				) {
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK];
					ChangeMA_Loop = true;
				} else if(ChangeMoveAnim == alist[ANIM_WALK_BACKWARD]
						 || ChangeMoveAnim == alist[ANIM_FIGHT_WALK_BACKWARD]
				) {
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK_BACKWARD];
					ChangeMA_Loop = true;
				} else {
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = true;
				}
			}
		}

		if(ause0->cur_anim == alist[ANIM_CROUCH_END]) {
			if(!(ause0->flags & EA_ANIMEND))
				goto nochanges;
		}

	retry:
		;

		if(ARX_SPELLS_ExistAnyInstance(SPELL_FLYING_EYE)) {
			ChangeMoveAnim = alist[ANIM_MEDITATION];
			ChangeMA_Loop = true;
			goto makechanges;
		} else if(ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0) {
			ChangeMoveAnim = alist[ANIM_LEVITATE];
			ChangeMA_Loop = true;
			goto makechanges;
		} else if(player.jumpphase != NotJumping) {
			
			switch(player.jumpphase) {
				case NotJumping:
				break;
				case JumpStart: { // Anticipation
					FALLING_TIME = 0;
					player.jumpphase = JumpAscending;
					ChangeMoveAnim = alist[ANIM_JUMP_UP];
					player.jumpstarttime = (unsigned long)(arxtime);
					player.jumplastposition = -1.f;
					break;
				}
				case JumpAscending: { // Moving up
					ChangeMoveAnim = alist[ANIM_JUMP_UP];
					if(player.jumplastposition >= 1.f) {
						player.jumpphase = JumpDescending;
						ChangeMoveAnim = alist[ANIM_JUMP_CYCLE];
						ARX_PLAYER_StartFall();
					}
					break;
				}
				case JumpDescending: { // Post-synch
					LAST_JUMP_ENDTIME = (unsigned long)(arxtime);
					if((ause0->cur_anim == alist[ANIM_JUMP_END] && (ause0->flags & EA_ANIMEND))
					   || player.onfirmground) {
						player.jumpphase = JumpEnd;
						ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];
					} else {
						ChangeMoveAnim = alist[ANIM_JUMP_END];
					}
					break;
				}
				case JumpEnd: { // Post-synch
					LAST_JUMP_ENDTIME = (unsigned long)(arxtime);
					if(ause0->cur_anim == alist[ANIM_JUMP_END_PART2] && (ause0->flags & EA_ANIMEND)) {
						AcquireLastAnim(io);
						player.jumpphase = NotJumping;
						goto retry;
					} else if(ause0->cur_anim == alist[ANIM_JUMP_END_PART2]
					         && EEfabs(player.physics.velocity.x)
					             + EEfabs(player.physics.velocity.z) > (4.f/TARGET_DT)
					         && ause0->ctime > 1) {
						AcquireLastAnim(io);
						player.jumpphase = NotJumping;
						goto retry;
					} else {
						ChangeMoveAnim = alist[ANIM_JUMP_END_PART2];
					}
					break;
				}
			}

			if(ChangeMoveAnim && ChangeMoveAnim != ause0->cur_anim) {
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]
				) {
					ause0->flags |= EA_EXCONTROL;
				}
			}

			if(ChangeMoveAnim2 && ChangeMoveAnim2 != ause3->cur_anim) {
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		} else {
		makechanges:
			;

			if(ChangeMoveAnim && ChangeMoveAnim != ause0->cur_anim) {
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if(ChangeMA_Loop)
					ause0->flags |= EA_LOOP;

				if(ChangeMA_Stopend)
					ause0->flags |= EA_STOPEND;

				if(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT]
				   || ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]
				) {
					ause0->flags |= EA_EXCONTROL;
				}
			}

			if(ChangeMoveAnim2 && ChangeMoveAnim2 != ause3->cur_anim) {
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}

		io->physics = player.physics;
	}

nochanges:
	;
	player.Last_Movement = player.Current_Movement;
}

/*!
 * \brief Init Local Player Data
 */
void ARX_PLAYER_InitPlayer() {
	player.Interface = INTER_MINIBOOK | INTER_MINIBACK | INTER_LIFE_MANA;
	player.physics.cyl.height = player.baseHeight();
	player.physics.cyl.radius = player.baseRadius();
	player.lifePool.current = player.lifePool.max = player.Full_maxlife = 100.f;
	player.manaPool.current = player.manaPool.max = player.Full_maxmana = 100.f;
	player.falling = false;
	player.rightIO = NULL;
	player.leftIO = NULL;
	player.equipsecondaryIO = NULL;
	player.equipshieldIO = NULL;
	player.torch = NULL;
	player.gold = 0;
	player.bag = 1;
	player.doingmagic = 0;
	ARX_PLAYER_MakeFreshHero();
}

/*!
 * \brief Forces player orientation to look at an IO
 * \param io
 */
void ForcePlayerLookAtIO(Entity * io) {
	
	arx_assert(io);

	EERIE_CAMERA tcam;
	Vec3f target;

	long id = entities.player()->obj->fastaccess.view_attach;

	if(id != -1) {
		tcam.orgTrans.pos = entities.player()->obj->vertexlist3[id].v;
	} else {
		tcam.orgTrans.pos = player.pos;
	}

	id = io->obj->fastaccess.view_attach;

	if(id != -1) {
		target = io->obj->vertexlist3[id].v;
	} else {
		target = io->pos;
	}

	// For the case of not already computed Vlist3... !
	if(fartherThan(target, io->pos, 400.f)) {
		target = io->pos;
	}

	tcam.setTargetCamera(target);
	player.angle.setYaw(MAKEANGLE(-tcam.angle.getYaw()));
	player.angle.setPitch(MAKEANGLE(tcam.angle.getPitch() - 180.f));
	player.angle.setRoll(0);
	player.desiredangle = player.angle;
}
extern float PLAYER_ARMS_FOCAL;
extern long CURRENT_BASE_FOCAL;

/*!
 * \brief Updates Many player infos each frame
 */
void ARX_PLAYER_Frame_Update()
{
	if(ARX_SPELLS_GetSpellOn(entities.player(), SPELL_PARALYSE) >= 0) {
		PLAYER_PARALYSED = 1;
	} else {
		entities.player()->ioflags &= ~IO_FREEZESCRIPT;
		PLAYER_PARALYSED = 0;
	}

	// Reset player moveto info
	moveto = player.pos;

	// Reset current movement flags
	player.Current_Movement = 0;

	// Updates player angles to desired angles
	player.angle = player.desiredangle;

	// Updates player Extra-Rotate Informations
	Entity *io = entities.player();

	if(io && io->_npcdata->ex_rotate) {
		EERIE_EXTRA_ROTATE * extraRotation = io->_npcdata->ex_rotate;

		float v = player.angle.getYaw();

		if(v > 160)
			v = -(360 - v);

		if(player.Interface & INTER_COMBATMODE) {
			if (ARX_EQUIPMENT_GetPlayerWeaponType() == WEAPON_BOW) {
				extraRotation->group_rotate[0].setYaw(0); //head
				extraRotation->group_rotate[1].setYaw(0); //neck
				extraRotation->group_rotate[2].setYaw(0); //chest
				extraRotation->group_rotate[3].setYaw(v); //belt
			} else {
				v *= ( 1.0f / 10 ); 
				extraRotation->group_rotate[0].setYaw(v); //head
				extraRotation->group_rotate[1].setYaw(v); //neck
				extraRotation->group_rotate[2].setYaw(v * 4); //chest
				extraRotation->group_rotate[3].setYaw(v * 4); //belt
			}
		} else {
			v *= ( 1.0f / 4 ); 
			extraRotation->group_rotate[0].setYaw(v); //head
			extraRotation->group_rotate[1].setYaw(v); //neck
			extraRotation->group_rotate[2].setYaw(v); //chest
			extraRotation->group_rotate[3].setYaw(v); //belt*/
		}

		if((player.Interface & INTER_COMBATMODE) || player.doingmagic == 2)
			extraRotation->flags &= ~EXTRA_ROTATE_REALISTIC;
	}

	PLAYER_ARMS_FOCAL = static_cast<float>(CURRENT_BASE_FOCAL);

	ARX_PLAYER_ComputePlayerFullStats();

	player.TRAP_DETECT = player.Full_Skill_Mecanism;
	player.TRAP_SECRET = player.Full_Skill_Intuition;

	if(ARX_SPELLS_GetSpellOn(entities.player(), SPELL_DETECT_TRAP) >= 0)
		player.TRAP_DETECT = 100.f;

	ModeLight |= MODE_DEPTHCUEING;

	ARX_PLAYER_ManageTorch();
}

/*!
 * \brief Emit player step noise
 */
static void ARX_PLAYER_MakeStepNoise() {
	
	if(ARX_SPELLS_GetSpellOn(entities.player(), SPELL_LEVITATE) >= 0) {
		return;
	}
	
	if(USE_PLAYERCOLLISIONS) {	
		float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT;
		float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT;
		
		if(player.Current_Movement & PLAYER_MOVE_STEALTH) {
			float skill_stealth = player.Full_Skill_Stealth / ARX_PLAYER_SKILL_STEALTH_MAX;
			volume -= ARX_NPC_AUDIBLE_VOLUME_RANGE * skill_stealth;
			factor += ARX_NPC_AUDIBLE_FACTOR_RANGE * skill_stealth;
		}
		
		Vec3f pos = player.basePosition();
		ARX_NPC_NeedStepSound(entities.player(), pos, volume, factor);
	}
	
	while(currentdistance >= STEP_DISTANCE) {
		currentdistance -= STEP_DISTANCE;
	}
}

extern bool bGCroucheToggle;
extern float MAX_ALLOWED_PER_SECOND;

static long LAST_FIRM_GROUND = 1;
static long TRUE_FIRM_GROUND = 1;
float lastposy = -9999999.f;
unsigned long REQUEST_JUMP = 0;
extern float GLOBAL_SLOWDOWN;
extern float Original_framedelay;

unsigned long LAST_JUMP_ENDTIME = 0;

bool Valid_Jump_Pos() {
	
	if(LAST_ON_PLATFORM || player.climbing) {
		return true;
	}
	
	EERIE_CYLINDER tmpp;
	tmpp.height = player.physics.cyl.height;
	tmpp.origin = player.basePosition();
	tmpp.radius = player.physics.cyl.radius * 0.85f;
	float tmp = CheckAnythingInCylinder(tmpp, entities.player(),
	                                    CFLAG_PLAYER | CFLAG_JUST_TEST);
	if(tmp <= 20.f) {
		return true;
	}
	
	long hum = 0;
	for(float vv = 0; vv < 360.f; vv += 20.f) {
		tmpp.origin = player.basePosition();
		tmpp.origin += Vec3f(-std::sin(radians(vv)) * 20.f, 0.f, std::cos(radians(vv)) * 20.f);
		tmpp.radius = player.physics.cyl.radius;
		float anything = CheckAnythingInCylinder(tmpp, entities.player(), CFLAG_JUST_TEST);
		if(anything > 10) {
			hum = 1;
			break;
		}
	}
	if(!hum) {
		return true;
	}
	
	if(COLLIDED_CLIMB_POLY) {
		player.climbing = true;
		return true;
	}
	
	return (tmp <= 50.f);
}

void PlayerMovementIterate(float DelatTime);

void ARX_PLAYER_Manage_Movement() {
	
	// Is our player able to move ?
	if(cinematicBorder.isActive() || BLOCK_PLAYER_CONTROLS || !entities.player())
		return;

	// Compute current player speedfactor
	float speedfactor = entities.player()->basespeed + entities.player()->speed_modif;

	if(speedfactor < 0)
		speedfactor = 0;

	// Compute time things
	const float FIXED_TIMESTEP = 25.f;
	const float MAX_FRAME_TIME = 200.f;

	static float StoredTime = 0;

	float DeltaTime = std::min(Original_framedelay, MAX_FRAME_TIME);
	DeltaTime = StoredTime + DeltaTime * speedfactor;
	
	if(player.jumpphase != NotJumping) {
		while(DeltaTime > FIXED_TIMESTEP) {
			/*
			 * TODO: should be PlayerMovementIterate(FIXED_TIMESTEP);
			 * However, jump forward movement is only applied the the first
			 * iteration, so we need this to not completely break the jump
			 * at lower framerates.
			 * Should only cause minor differences at higher framerates.
			 * Fix this once PlayerMovementIterate has been cleaned up!
			 */
			PlayerMovementIterate(DeltaTime);
			DeltaTime -= FIXED_TIMESTEP;
		}
	} else {
		PlayerMovementIterate(DeltaTime);
		DeltaTime = 0;
	}
	
	StoredTime = DeltaTime;
}

void PlayerMovementIterate(float DeltaTime) {
	
	// A jump is requested so let's go !
	if(REQUEST_JUMP) {
		if((player.Current_Movement & PLAYER_CROUCH)
		   || player.physics.cyl.height > player.baseHeight()) {
			float old = player.physics.cyl.height;
			player.physics.cyl.height = player.baseHeight();
			player.physics.cyl.origin = player.basePosition();
			float anything = CheckAnythingInCylinder(player.physics.cyl, entities.player(),
			                                         CFLAG_JUST_TEST);
			if(anything < 0.f) {
				player.Current_Movement |= PLAYER_CROUCH;
				player.physics.cyl.height = old;
				REQUEST_JUMP = 0;
			} else {
				bGCroucheToggle = false;
				player.Current_Movement &= ~PLAYER_CROUCH;
				player.physics.cyl.height = player.baseHeight();
			}
		}
		
		if(!Valid_Jump_Pos()) {
			REQUEST_JUMP = 0;
		}
		
		if(REQUEST_JUMP) {
			float t = (float)float(arxtime) - (float)REQUEST_JUMP;
			if(t >= 0.f && t <= 350.f) {
				REQUEST_JUMP = 0;
				ARX_NPC_SpawnAudibleSound(player.pos, entities.player());
				ARX_SPEECH_Launch_No_Unicode_Seek("player_jump", entities.player());
				player.onfirmground = false;
				player.jumpphase = JumpStart;
			}
		}
	}
	
	if(entities.player()->_npcdata->climb_count != 0.f && framedelay > 0) {
		entities.player()->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * framedelay * 0.1f;
		if(entities.player()->_npcdata->climb_count < 0) {
			entities.player()->_npcdata->climb_count = 0.f;
		}
	}
	
	float d = 0;
	
	if(USE_PLAYERCOLLISIONS) {
		CollisionFlags levitate = 0;
		if(player.climbing) {
			levitate = CFLAG_LEVITATE;
		}
		
		if(player.levitate) {
			if(player.physics.cyl.height != player.levitateHeight()) {
				float old = player.physics.cyl.height;
				player.physics.cyl.height = player.levitateHeight();
				player.physics.cyl.origin = player.basePosition();
				float anything = CheckAnythingInCylinder(player.physics.cyl, entities.player());
				if(anything < 0.f) {
					player.physics.cyl.height = old;
					long num = ARX_SPELLS_GetSpellOn(entities.player(), SPELL_LEVITATE);
					if(num != -1) {
						spells[num].m_tolive = 0;
					}
				}
			}
			
			if(player.physics.cyl.height == player.levitateHeight()) {
				levitate = CFLAG_LEVITATE;
				player.climbing = false;
				bGCroucheToggle = false;
				player.Current_Movement &= ~PLAYER_CROUCH;
			}
			
		} else if(player.physics.cyl.height == player.levitateHeight()) {
			player.physics.cyl.height = player.baseHeight();
		}
		
		if(player.jumpphase != JumpAscending && !levitate) {
			player.physics.cyl.origin = player.basePosition();
		}
		
		if(EEfabs(lastposy - player.pos.y) < DeltaTime * 0.1f) {
			TRUE_FIRM_GROUND = 1;
		} else {
			TRUE_FIRM_GROUND = 0;
		}
		
		lastposy = player.pos.y;
		float anything;
		EERIE_CYLINDER testcyl = player.physics.cyl;
		testcyl.origin.y += 3.f;
		ON_PLATFORM = 0;
		anything = CheckAnythingInCylinder(testcyl, entities.player(), 0);
		LAST_ON_PLATFORM = ON_PLATFORM;
	
		if(player.jumpphase != JumpAscending) {
			if(anything >= 0.f) {
				TRUE_FIRM_GROUND = 0;
			} else {
				TRUE_FIRM_GROUND = 1;
				testcyl.radius -= 30.f;
				testcyl.origin.y -= 10.f;
				anything = CheckAnythingInCylinder(testcyl, entities.player(), 0);
			}
		} else {
			TRUE_FIRM_GROUND = 0;
			LAST_ON_PLATFORM = 0;
		}
		
		EERIE_CYLINDER cyl;
		cyl.origin = player.basePosition() + Vec3f(0.f, 1.f, 0.f);
		cyl.radius = player.physics.cyl.radius;
		cyl.height = player.physics.cyl.height;
		float anything2 = CheckAnythingInCylinder(cyl, entities.player(), CFLAG_JUST_TEST | CFLAG_PLAYER); //-cyl->origin.y;
		
		if(   anything2 > -5
		   && player.physics.velocity.y > (15.f/TARGET_DT)
		   && !LAST_ON_PLATFORM
		   && !TRUE_FIRM_GROUND
		   && player.jumpphase == NotJumping
		   && !player.levitate
		   && anything > 80.f
		) {
			player.jumpphase = JumpDescending;
			if(!player.falling) {
				player.falling = true;
				ARX_PLAYER_StartFall();
			}
		} else if(!player.falling) {
			FALLING_TIME = 0;
		}
		
		if(player.jumpphase != NotJumping && player.levitate) {
			player.jumpphase = NotJumping;
			player.falling = false;
			Falling_Height = player.pos.y;
			FALLING_TIME = 0;
		}
		
		if(!LAST_FIRM_GROUND && TRUE_FIRM_GROUND) {
			player.jumpphase = NotJumping;
			if(FALLING_TIME > 0 && player.falling) {
				player.physics.velocity.x = 0.f;
				player.physics.velocity.z = 0.f;
				player.physics.forces.x = 0.f;
				player.physics.forces.z = 0.f;
				player.falling = false;
				float fh = player.pos.y - Falling_Height;
				if(fh > 400.f) {
					float dmg = (fh - 400.f) * (1.0f / 15);
					if(dmg > 0.f) {
						Falling_Height = player.pos.y;
						FALLING_TIME = 0;
						ARX_DAMAGES_DamagePlayer(dmg, 0, -1);
						ARX_DAMAGES_DamagePlayerEquipment(dmg);
					}
				}
			}
		}
		
		LAST_FIRM_GROUND = TRUE_FIRM_GROUND;
		player.onfirmground = (TRUE_FIRM_GROUND != 0);
		if(player.onfirmground && !player.falling) {
			FALLING_TIME = 0;
		}
		
		// Apply player impulse force
		
		float jump_mul = 1.f;
		if(float(arxtime) - LAST_JUMP_ENDTIME < 600) {
			jump_mul = 0.5f;
			if(float(arxtime) - LAST_JUMP_ENDTIME >= 300) {
				jump_mul += (float)(LAST_JUMP_ENDTIME + 300 - float(arxtime)) * (1.f / 300);
				if(jump_mul > 1.f) {
					jump_mul = 1.f;
				}
			}
		}
		
		Vec3f impulse = moveto - player.pos;
		if(impulse != Vec3f_ZERO) {
			
			float scale = 1.25f / 1000;
			if(entities.player()->animlayer[0].cur_anim) {
				if(player.jumpphase != NotJumping) {
					if(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) {
						scale = 0.8f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD) {
						scale = 7.9f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_STRAFE_LEFT) {
						scale = 2.6f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_STRAFE_RIGHT) {
						scale = 2.6f / 1000;
					} else {
						scale = 0.2f / 1000;
					}
				} else if(levitate && !player.climbing) {
					scale = 0.875f / 1000;
				} else {
					Vec3f mv;
					short idx = entities.player()->animlayer[0].altidx_cur;
					GetAnimTotalTranslate(entities.player()->animlayer[0].cur_anim, idx, &mv);
					float time = entities.player()->animlayer[0].cur_anim->anims[idx]->anim_time;
					scale = glm::length(mv) / time * 0.0125f;
				}
			}
			
			impulse *= scale / glm::length(impulse) * jump_mul;
		}
		
		if(player.jumpphase != NotJumping) {
			// No Vertical Interpolation
			entities.player()->_npcdata->vvpos = -99999.f;
			if(player.jumpphase == JumpAscending) {
				moveto.y = player.pos.y;
				player.physics.velocity.y = 0;
			}
		}
		
		if(player.climbing) {
			player.physics.velocity.x = 0.f;
			player.physics.velocity.y *= 0.5f;
			player.physics.velocity.z = 0.f;
			if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD) {
				moveto.x = player.pos.x;
				moveto.z = player.pos.z;
			}
			if(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) {
				impulse.x = 0;
				impulse.z = 0;
				moveto.x = player.pos.x;
				moveto.z = player.pos.z;
			}
		}
		
		player.physics.forces += impulse;
		
		// Apply Gravity force if not LEVITATING or JUMPING
		if(!levitate && player.jumpphase != JumpAscending && !LAST_ON_PLATFORM) {
			
			player.physics.forces.y += ((player.falling) ? JUMP_GRAVITY : WORLD_GRAVITY) / TARGET_DT;
			
			// Check for LAVA Damage !!!
			float epcentery;
			EERIEPOLY *ep = CheckInPoly(player.pos + Vec3f(0.f, 150.f, 0.f), &epcentery);
			if(ep) {
				if((ep->type & POLY_LAVA) && EEfabs(epcentery - (player.pos.y - player.baseHeight())) < 30) {
					float mul = 1.f - (EEfabs(epcentery - (player.pos.y - player.baseHeight())) * (1.0f / 30));
					const float LAVA_DAMAGE = 10.f;
					float damages = LAVA_DAMAGE * framedelay * 0.01f * mul;
					damages = ARX_SPELLS_ApplyFireProtection(entities.player(), damages);
					ARX_DAMAGES_DamagePlayer(damages, DAMAGE_TYPE_FIRE, 0);
					ARX_DAMAGES_DamagePlayerEquipment(damages);

					Vec3f pos = player.basePosition();
					ARX_PARTICLES_Spawn_Lava_Burn(&pos, entities.player());
				}
			}
		}
		
		// Apply velocity damping (natural velocity attenuation, stands for friction)
		float dampen = 1.f - (0.009f * DeltaTime);
		if(dampen < 0.001f) {
			dampen = 0.f;
		}
		player.physics.velocity.x *= dampen;
		player.physics.velocity.z *= dampen;
		if(EEfabs(player.physics.velocity.x) < 0.001f) {
			player.physics.velocity.x = 0;
		}
		if(EEfabs(player.physics.velocity.z) < 0.001f) {
			player.physics.velocity.z = 0;
		}
		
		// Apply attraction
		Vec3f attraction;
		ARX_SPECIAL_ATTRACTORS_ComputeForIO(*entities.player(), attraction);
		player.physics.forces += attraction / TARGET_DT;
		
		// Apply push player force
		player.physics.forces += PUSH_PLAYER_FORCE / TARGET_DT;
		PUSH_PLAYER_FORCE = Vec3f_ZERO;
		
		// Apply forces to velocity
		player.physics.velocity += player.physics.forces * DeltaTime;
		
		// Apply climbing velocity
		if(player.climbing) {
			if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD) {
				player.physics.velocity.y = -0.2f;
			}
			if(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) {
				player.physics.velocity.y = 0.2f;
			}
		}
		
		// Removes y velocity if on firm ground...
		if(player.onfirmground && !player.climbing) {
			player.physics.velocity.y = 0.f;
		}
		
		float posy;
		EERIEPOLY * ep = CheckInPoly(player.pos, &posy);
		if(ep == NULL) {
			player.physics.velocity.y = 0;
		} else if(!player.climbing && player.pos.y >= posy) {
			player.physics.velocity.y = 0;
		}

		// Reset forces
		player.physics.forces = Vec3f_ZERO;
		
		// Check if player is already on firm ground AND not moving
		if(EEfabs(player.physics.velocity.x) < 0.001f
		   && EEfabs(player.physics.velocity.z) < 0.001f
		   && player.onfirmground
		   && player.jumpphase == NotJumping
		) {
			moveto = player.pos;
			goto lasuite;
		} else {
			
			// Need to apply some physics/collision tests
			player.physics.cyl.origin = player.basePosition();
			player.physics.startpos = player.physics.cyl.origin;
			player.physics.targetpos = player.physics.startpos + player.physics.velocity * DeltaTime;
			
			// Jump impulse
			if(player.jumpphase == JumpAscending) {
				
				if(player.jumplastposition == -1.f) {
					player.jumplastposition = 0;
					player.jumpstarttime = (unsigned long)(arxtime);
				}
				
				const float jump_up_time = 200.f;
				const float jump_up_height = 130.f;
				long timee = (long)arxtime;
				float offset_time = (float)timee - (float)player.jumpstarttime;
				float position = clamp(offset_time / jump_up_time, 0.f, 1.f);
				
				float p = (position - player.jumplastposition) * jump_up_height;
				player.physics.targetpos.y -= p;
				player.jumplastposition = position;
				levitate = 0;
			}
			
			bool test;
			float PLAYER_CYLINDER_STEP = 40.f;
			if(player.climbing) {
				test = ARX_COLLISION_Move_Cylinder(&player.physics, entities.player(),
				                                   PLAYER_CYLINDER_STEP,
												   CFLAG_EASY_SLIDING | CFLAG_CLIMBING | CFLAG_PLAYER);
				
				if(!COLLIDED_CLIMB_POLY) {
					player.climbing = false;
				}
			} else {
				test = ARX_COLLISION_Move_Cylinder(&player.physics, entities.player(),
				                                   PLAYER_CYLINDER_STEP,
				                                   levitate | CFLAG_EASY_SLIDING | CFLAG_PLAYER);
				
				if(!test && !LAST_FIRM_GROUND && !TRUE_FIRM_GROUND) {
					player.physics.velocity.x = 0.f;
					player.physics.velocity.z = 0.f;
					player.physics.forces.x = 0.f;
					player.physics.forces.z = 0.f;
					if(FALLING_TIME > 0 && player.falling) {
						float fh = player.pos.y - Falling_Height;
						if(fh > 400.f) {
							float dmg = (fh - 400.f) * (1.f / 15);
							if(dmg > 0.f) {
								Falling_Height = (player.pos.y + Falling_Height * 2) * (1.f / 3);
								ARX_DAMAGES_DamagePlayer(dmg, 0, -1);
								ARX_DAMAGES_DamagePlayerEquipment(dmg);
							}
						}
					}
				}
				
				if(!test && player.jumpphase != NotJumping) {
					player.physics.startpos.x = player.physics.cyl.origin.x = player.pos.x;
					player.physics.startpos.z = player.physics.cyl.origin.z = player.pos.z;
					player.physics.targetpos.x = player.physics.startpos.x;
					player.physics.targetpos.z = player.physics.startpos.z;
					if(player.physics.targetpos.y != player.physics.startpos.y) {
						test = ARX_COLLISION_Move_Cylinder(&player.physics, entities.player(),
						                                   PLAYER_CYLINDER_STEP,
						                                   levitate | CFLAG_EASY_SLIDING
						                                   | CFLAG_PLAYER);
						entities.player()->_npcdata->vvpos = -99999.f;
					}
				}
			}
			
			if(COLLIDED_CLIMB_POLY) {
				player.climbing = true;
			}
			
			if(player.climbing) {
				if(player.Current_Movement
				   && player.Current_Movement != PLAYER_ROTATE
				   && !(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				   && !(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)
				) {
					player.climbing = false;
				}
				
				if((player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) && !test) {
					player.climbing = false;
				}
				
				if(player.climbing) {
					player.jumpphase = NotJumping;
					player.falling = false;
					FALLING_TIME = 0;
					Falling_Height = player.pos.y;
				}
			}
			
			if(player.jumpphase == JumpAscending) {
				player.climbing = false;
			}
			
			moveto = player.physics.cyl.origin + player.baseOffset();
			d = glm::distance(player.pos, moveto);
		}
	} else {
		Vec3f vect = moveto - player.pos;
		float divv = glm::length(vect);
		if(divv > 0.f) {
			float mul = (float)framedelay * 0.001f * 200.f;
			divv = mul / divv;
			vect *= divv;
			moveto = player.pos + vect;
		}
		
		player.onfirmground = false;
	}
	
	if(player.pos == moveto) {
		d = 0.f;
	}
	
	// Emit Stepsound
	if(USE_PLAYERCOLLISIONS) {
		if(player.Current_Movement & PLAYER_CROUCH) {
			d *= 2.f;
		}
		currentdistance += d;
		if(player.jumpphase == NotJumping && !player.falling
		   && currentdistance >= STEP_DISTANCE) {
			ARX_PLAYER_MakeStepNoise();
		}
	}
	
	// Finally update player pos !
	player.pos = moveto;
	
lasuite:
	;

	// Get Player position color
	float grnd_color = GetColorz(Vec3f(player.pos.x, player.pos.y + 90, player.pos.z)) - 15.f;
	if(CURRENT_PLAYER_COLOR < grnd_color) {
		CURRENT_PLAYER_COLOR += framedelay * (1.0f / 8);
		CURRENT_PLAYER_COLOR = std::min(CURRENT_PLAYER_COLOR, grnd_color);
	}
	if(CURRENT_PLAYER_COLOR > grnd_color) {
		CURRENT_PLAYER_COLOR -= framedelay * (1.0f / 4);
		CURRENT_PLAYER_COLOR = std::max(CURRENT_PLAYER_COLOR, grnd_color);
	}
	
	if(InventoryDir != 0) {
		if((player.Interface & INTER_COMBATMODE) || player.doingmagic >= 2 || InventoryDir == -1) {
			if(InventoryX > -160)
				InventoryX -= INTERFACE_RATIO(framedelay * ( 1.0f / 3 ));
		} else {
			if(InventoryX < 0)
				InventoryX += InventoryDir * INTERFACE_RATIO(framedelay * ( 1.0f / 3 ));
		}

		if(InventoryX <= -160) {
			InventoryX = -160;
			InventoryDir = 0;

			if(player.Interface & INTER_STEAL || ioSteal) {
				SendIOScriptEvent(ioSteal, SM_STEAL, "off");
				player.Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}

			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			InventoryDir = 0;
		} else if(InventoryX >= 0) {
			InventoryX = 0;
			InventoryDir = 0;
		}
	}
}

/*!
 * \brief Manage Player Death Visual
 */
void ARX_PLAYER_Manage_Death() {
	if(DeadTime <= 2000)
		return;

	PLAYER_PARALYSED = 0;
	float ratio = (float)(DeadTime - 2000) * ( 1.0f / 5000 );

	if(ratio >= 1.f) {
		ARX_MENU_Launch(false);
		DeadTime = 0;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
	EERIEDrawBitmap(Rectf(g_size), 0.000091f, NULL, Color3f::gray(ratio).to<u8>());
}

/*!
 * \brief Specific for color checks
 * \return
 */
float GetPlayerStealth() {
	return 15 + player.Full_Skill_Stealth * ( 1.0f / 10 );
}

/*!
 * \brief Force Player to standard stance
 * \param val
 */
void ARX_PLAYER_PutPlayerInNormalStance(long val) {
	
	if(player.Current_Movement & PLAYER_CROUCH) {
		player.Current_Movement &= ~PLAYER_CROUCH;
	}
	
	player.Current_Movement = 0;
	ARX_PLAYER_RectifyPosition();
	
	if(player.jumpphase != NotJumping || player.falling) {
		player.physics.cyl.origin = player.basePosition();
		IO_PHYSICS phys = player.physics;
		AttemptValidCylinderPos(phys.cyl, entities.player(), CFLAG_RETURN_HEIGHT);
		player.pos.y = phys.cyl.origin.y + player.baseHeight();
		player.jumpphase = NotJumping;
		player.falling = false;
	}
	
	if(player.Interface & INTER_COMBATMODE) {
		player.Interface &= ~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}
	
	ARX_SOUND_Stop(SND_MAGIC_DRAW);
	
	if(!val) {
		for(size_t i = 0; i < MAX_SPELLS; i++) {
			if(spells[i].m_exist && (spells[i].m_caster == 0 || spells[i].m_target == 0)) {
				switch(spells[i].m_type) {
					case SPELL_MAGIC_SIGHT:
					case SPELL_LEVITATE:
					case SPELL_SPEED:
					case SPELL_FLYING_EYE:
						spells[i].m_tolive = 0;
						break;
					default: break;
				}
			}
		}
	}
}

/*!
 * \brief Add gold to player purse
 * \param _lValue
 */
void ARX_PLAYER_AddGold(long _lValue) {
	player.gold += _lValue;
	bGoldHalo = true;
	ulGoldHaloTime = 0;
}

void ARX_PLAYER_AddGold(Entity * gold) {
	
	arx_assert(gold->ioflags & IO_GOLD);
	
	ARX_PLAYER_AddGold(gold->_itemdata->price * max((short)1, gold->_itemdata->count));
	
	ARX_SOUND_PlayInterface(SND_GOLD);
	
	gold->gameFlags &= ~GFLAG_ISINTREATZONE;
	
	gold->destroy();
}

void ARX_PLAYER_Start_New_Quest() {
	
	SKIN_MOD = 0;
	QUICK_MOD = 0;
	EERIE_PATHFINDER_Clear();
	EERIE_PATHFINDER_Release();
	ARX_PLAYER_MakeFreshHero();
	player.torch = NULL;
	entities.clear();
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	ARX_EQUIPMENT_UnEquipAllPlayer();
	
	ARX_CHANGELEVEL_StartNew();
	
	entities.player()->halo.flags = 0;
}

void ARX_PLAYER_AddBag() {
	++player.bag;

	if(player.bag > 3)
		player.bag = 3;
}

bool ARX_PLAYER_CanStealItem(Entity * _io) {

	if(_io->_itemdata->stealvalue > 0
	   && player.Full_Skill_Stealth >= _io->_itemdata->stealvalue
	   && _io->_itemdata->stealvalue < 100.f
	) {
		return true;
	}

	return false;
}

void ARX_PLAYER_Rune_Add_All() {

	ARX_Player_Rune_Add(FLAG_AAM);
	ARX_Player_Rune_Add(FLAG_CETRIUS);
	ARX_Player_Rune_Add(FLAG_COMUNICATUM);
	ARX_Player_Rune_Add(FLAG_COSUM);
	ARX_Player_Rune_Add(FLAG_FOLGORA);
	ARX_Player_Rune_Add(FLAG_FRIDD);
	ARX_Player_Rune_Add(FLAG_KAOM);
	ARX_Player_Rune_Add(FLAG_MEGA);
	ARX_Player_Rune_Add(FLAG_MORTE);
	ARX_Player_Rune_Add(FLAG_MOVIS);
	ARX_Player_Rune_Add(FLAG_NHI);
	ARX_Player_Rune_Add(FLAG_RHAA);
	ARX_Player_Rune_Add(FLAG_SPACIUM);
	ARX_Player_Rune_Add(FLAG_STREGUM);
	ARX_Player_Rune_Add(FLAG_TAAR);
	ARX_Player_Rune_Add(FLAG_TEMPUS);
	ARX_Player_Rune_Add(FLAG_TERA);
	ARX_Player_Rune_Add(FLAG_VISTA);
	ARX_Player_Rune_Add(FLAG_VITAE);
	ARX_Player_Rune_Add(FLAG_YOK);
}

extern unsigned long LAST_PRECAST_TIME;

void ARX_PLAYER_Invulnerability(long flag) {

	if(flag)
		player.playerflags |= PLAYERFLAGS_INVULNERABILITY;
	else
		player.playerflags &= ~PLAYERFLAGS_INVULNERABILITY;
}

extern Entity * FlyingOverIO;

void ARX_GAME_Reset(long type) {
	
	DeadTime = 0;
	
	if(entities.player()) {
		entities.player()->speed_modif = 0;
	}
	
	LAST_JUMP_ENDTIME = 0;
	FlyingOverIO = NULL;
	g_miniMap.mapMarkerInit();
	ClearDynLights();

	if(!DONT_ERASE_PLAYER && entities.player()) {
		entities.player()->halo.flags = 0;
	}

	if(entities.player())entities.player()->gameFlags &= ~GFLAG_INVISIBILITY;
	ARX_PLAYER_Invulnerability(0);
	PLAYER_PARALYSED = 0;

	ARX_PLAYER_Reset_Fall();

	player.levitate = false;
	Project.telekinesis = 0;
	player.onfirmground = false;
	TRUE_FIRM_GROUND = 0;

	lastposy = -99999999999.f;

	ioSteal = NULL;

	GLOBAL_SLOWDOWN = 1.f;

	CheatReset();

	if(entities.player()) {
		entities.player()->spellcast_data.castingspell = SPELL_NONE;
	}

	LAST_PRECAST_TIME = 0;

	ARX_INTERFACE_NoteClear();
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;

	// Interactive DynData
	ARX_INTERACTIVE_ClearAllDynData();

	// PolyBooms
	ARX_BOOMS_ClearAllPolyBooms();

	// Magical Flares
	ARX_MAGICAL_FLARES_KillAll();

	// Thrown Objects
	ARX_THROWN_OBJECT_KillAll();

	// Pathfinder
	EERIE_PATHFINDER_Clear();

	// Sound
	if(!(type & 1)) {
		ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
		ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);
		ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
	}

	// Damages
	ARX_DAMAGE_Reset_Blood_Info();
	ARX_DAMAGES_Reset();

	// Scripts
	ARX_SCRIPT_Timer_ClearAll();
	ARX_SCRIPT_EventStackClear();
	ARX_SCRIPT_ResetAll(0);

	// Conversations
	ARX_CONVERSATION_Reset();
	ARX_CONVERSATION = false;

	// Speech Things
	REQUEST_SPEECH_SKIP = 0;
	ARX_SPEECH_ClearAll();
	ARX_SPEECH_Reset();

	// Spells
	ARX_SPELLS_Precast_Reset();
	ARX_SPELLS_CancelSpellTarget();

	ARX_SPELLS_ClearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_SPELLS_ResetRecognition();

	// Particles
	ARX_PARTICLES_ClearAll();
	if(pParticleManager)
		pParticleManager->Clear();

	// Fogs
	ARX_FOGS_Render();

	// Anchors
	ANCHOR_BLOCK_Clear();

	// Attractors
	ARX_SPECIAL_ATTRACTORS_Reset();

	// Cinematics
	DANAE_KillCinematic();

	// Paths
	ARX_PATH_ClearAllControled();
	ARX_PATH_ClearAllUsePath();

	// Player Torch
	if(type & 1) {
		if(player.torch)
			ARX_PLAYER_ClickedOnTorch(player.torch);
	} else {
		player.torch = NULL;
	}

	// Player Quests
	ARX_PLAYER_Quest_Init();

	// Player Keyring
	ARX_KEYRING_Init();

	// Player Init
	if(!DONT_ERASE_PLAYER) {
		g_miniMap.mapMarkerInit();
		GLOBAL_MAGIC_MODE = true;

		// Linked Objects
		if(!(type & 2)) {
			UnlinkAllLinkedObjects();
			ARX_EQUIPMENT_UnEquipAllPlayer();
		}

		ARX_EQUIPMENT_ReleaseAll(entities.player());

		ARX_PLAYER_InitPlayer();
		ARX_INTERACTIVE_RemoveGoreOnIO(entities.player());
		
		// default to mouselook on, inventory closed
		TRUE_PLAYER_MOUSELOOK_ON = true;

		// Player Inventory
		CleanInventory();
	}

	// Misc Player Vars.
	ROTATE_START = 0;
	BLOCK_PLAYER_CONTROLS = false;
	HERO_SHOW_1ST = -1;
	PUSH_PLAYER_FORCE = Vec3f_ZERO;
	player.jumplastposition = 0;
	player.jumpstarttime = 0;
	player.jumpphase = NotJumping;
	player.inzone = NULL;

	RemoveQuakeFX();
	Project.improve = 0;

	if(eyeball.exist) {
		eyeball.exist = -100;
	}

	if(entities.size() > 0 && entities.player()) {
		entities.player()->ouch_time = 0;
		entities.player()->invisibility = 0.f;
	}
	
	fadeReset();
	
	// GLOBALMods
	ARX_GLOBALMODS_Reset();

	// Missiles
	ARX_MISSILES_ClearAll();

	// IO PDL
	TOTIOPDL = 0;
	
	// Interface
	ARX_INTERFACE_Reset();
	ARX_INTERFACE_NoteClear();
	Set_DragInter(NULL);
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	MasterCamera.exist = 0;
	CHANGE_LEVEL_ICON = -1;
	
	CAMERACONTROLLER = NULL;
	
	// Kill Script Loaded IO
	CleanScriptLoadedIO();
	
	// ARX Timer
	arxtime.init();
	
	ClearTileLights();
}

void ARX_PLAYER_Reset_Fall()
{
	FALLING_TIME = 0;
	Falling_Height = 50.f;
	player.falling = false;
}

