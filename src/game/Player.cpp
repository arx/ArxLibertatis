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

#include "gui/Hud.h"
#include "gui/Menu.h"
#include "gui/Text.h"
#include "gui/Speech.h"
#include "gui/Interface.h"
#include "gui/MiniMap.h"
#include "gui/hud/SecondaryInventory.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsModes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/PolyBoom.h"
#include "graphics/effects/Fade.h"
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/MagicFlare.h"
#include "graphics/particle/Spark.h"

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
#include "platform/profiler/Profiler.h"

#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"

extern long		HERO_SHOW_1ST;
extern bool		REQUEST_SPEECH_SKIP;
extern bool		DONT_ERASE_PLAYER;
extern bool		GLOBAL_MAGIC_MODE;

extern ParticleManager * pParticleManager;

static const float WORLD_GRAVITY = 0.1f;
static const float JUMP_GRAVITY = 0.02f;
static const float STEP_DISTANCE = 120.f;
static const float TARGET_DT = 1000.f / 30.f;

extern Vec3f PUSH_PLAYER_FORCE;
extern long COLLIDED_CLIMB_POLY;
extern long HERO_SHOW_1ST;

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
static ArxInstant LastHungerSample = ArxInstant_ZERO;
static unsigned long ROTATE_START = 0;

// Player Anims FLAGS/Vars
ANIM_HANDLE * herowaitbook = NULL;
ANIM_HANDLE * herowait_2h = NULL;

std::vector<KEYRING_SLOT> Keyring;

static unsigned long FALLING_TIME = 0;

std::vector<STRUCT_QUEST> PlayerQuest;

bool ARX_PLAYER_IsInFightMode() {
	arx_assert(entities.player());
	
	if (player.Interface & INTER_COMBATMODE) return true;

	const AnimLayer & layer1 = entities.player()->animlayer[1];
	
	if(layer1.cur_anim) {
		
		ANIM_HANDLE ** alist = entities.player()->anims;

		if(   layer1.cur_anim == alist[ANIM_BARE_READY]
		   || layer1.cur_anim == alist[ANIM_BARE_UNREADY]
		   || layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_1]
		   || layer1.cur_anim == alist[ANIM_DAGGER_READY_PART_2]
		   || layer1.cur_anim == alist[ANIM_DAGGER_UNREADY_PART_1]
		   || layer1.cur_anim == alist[ANIM_DAGGER_UNREADY_PART_2]
		   || layer1.cur_anim == alist[ANIM_1H_READY_PART_1]
		   || layer1.cur_anim == alist[ANIM_1H_READY_PART_2]
		   || layer1.cur_anim == alist[ANIM_1H_UNREADY_PART_1]
		   || layer1.cur_anim == alist[ANIM_1H_UNREADY_PART_2]
		   || layer1.cur_anim == alist[ANIM_2H_READY_PART_1]
		   || layer1.cur_anim == alist[ANIM_2H_READY_PART_2]
		   || layer1.cur_anim == alist[ANIM_2H_UNREADY_PART_1]
		   || layer1.cur_anim == alist[ANIM_2H_UNREADY_PART_2]
		   || layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_1]
		   || layer1.cur_anim == alist[ANIM_MISSILE_READY_PART_2]
		   || layer1.cur_anim == alist[ANIM_MISSILE_UNREADY_PART_1]
		   || layer1.cur_anim == alist[ANIM_MISSILE_UNREADY_PART_2]
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
 * \return player "front pos" for sound purpose
 */
Vec3f ARX_PLAYER_FrontPos() {
	Vec3f pos = player.pos;
	pos += angleToVectorXZ(player.angle.getPitch()) * 100.f;
	pos += Vec3f(0.f, 100.f, 0.f); // XXX use -100 here ?
	return pos;
}

/*!
 * \brief Reset all extra-rotation groups of player
 */
void ARX_PLAYER_RectifyPosition() {
	arx_assert(entities.player());
	
	Entity * io = entities.player();
	if(io->_npcdata->ex_rotate) {
		for(size_t n = 0; n < MAX_EXTRA_ROTATE; n++) {
			io->_npcdata->ex_rotate->group_rotate[n] = Anglef::ZERO;
		}
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
		player.torch->durability -= g_framedelay * ( 1.0f / 10000 );
		
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
				if(!player.hasRune(spellicons[i].symbols[j])) {
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
				if(!player.hasRune(spellicons[i].symbols[j])) {
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
		g_hudRoot.bookIconGui.requestFX();
		g_hudRoot.bookIconGui.requestHalo();
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
	
	if(!_bLoad)
		g_hudRoot.bookIconGui.requestHalo();
	
	gui::updateQuestBook();
}

/*!
 * \brief Removes player invisibility by killing Invisibility spells on him
 */
void ARX_PLAYER_Remove_Invisibility() {
	spells.endByCaster(PlayerEntityHandle, SPELL_INVISIBILITY);
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
	
	player.lifePool.max = player.m_attribute.constitution * (player.level + 2);
	player.manaPool.max = player.m_attribute.mind * (player.level + 1);
	
	float base_defense = player.m_skill.defense + player.m_attribute.constitution * 3;
	float armorClass = base_defense * ( 1.0f / 10 ) - 1 ;
	player.m_misc.armorClass = checked_range_cast<unsigned char>(armorClass);
	
	if(player.m_misc.armorClass < 1)
		player.m_misc.armorClass = 1;
	
	float base_casting = player.m_skill.casting + player.m_attribute.mind * 2.f;
	player.m_misc.resistMagic = (unsigned char)(float)(player.m_attribute.mind * 2.f
	                      * (1.f + base_casting * ( 1.0f / 200 )));

	float resistPoison = player.m_attribute.constitution * 2 + (base_defense * ( 1.0f / 4 ));
	player.m_misc.resistPoison = checked_range_cast<unsigned char>(resistPoison);


	player.m_misc.damages = (player.m_attribute.strength - 10) * ( 1.0f / 2 );
	
	if(player.m_misc.damages < 1.f)
		player.m_misc.damages = 1.f;
	
	player.AimTime = 1500;
	
	float base_close_combat = player.m_skill.closeCombat
	                          + player.m_attribute.dexterity + player.m_attribute.strength * 2.f;
	player.m_misc.criticalHit = (float)(player.m_attribute.dexterity - 9) * 2.f
	                      + base_close_combat * ( 1.0f / 5 );
}

/*!
 * \brief Compute FULL versions of player stats including Equiped Items and spells,
 *        and any other effect altering them.
 */
void ARX_PLAYER_ComputePlayerFullStats() {
	
	ARX_PLAYER_ComputePlayerStats();
	
	// Reset modifier values
	player.m_attributeMod = PlayerAttribute();
	player.m_skillMod = PlayerSkill();
	player.m_miscMod = PlayerMisc();
	
	// TODO why do this now and not after skills/stats have been calculated?
	ARX_EQUIPMENT_IdentifyAll();
	
	//CHECK OVERFLOW
	// TODO why not use relative modfiers?
	float fFullAimTime	= getEquipmentBaseModifier(IO_EQUIPITEM_ELEMENT_AimTime);
	float fCalcHandicap	= (player.m_attributeFull.dexterity - 10.f) * 20.f;

	//CAST
	player.Full_AimTime = checked_range_cast<long>(fFullAimTime);
	
	if(player.Full_AimTime <= 0)
		player.Full_AimTime = player.AimTime;
	
	player.Full_AimTime -= checked_range_cast<long>(fCalcHandicap);
	
	if(player.Full_AimTime <= 1500)
		player.Full_AimTime = 1500;
	
	// TODO make these calculations moddable
	
	/////////////////////////////////////////////////////////////////////////////////////
	// External modifiers
	
	// Calculate for modifiers from spells
		SpellBase * spell;
		
		spell = spells.getSpellOnTarget(PlayerEntityHandle, SPELL_ARMOR);
		if(spell) {
			player.m_miscMod.armorClass += spell->m_level;
		}
		
		spell = spells.getSpellOnTarget(PlayerEntityHandle, SPELL_LOWER_ARMOR);
		if(spell) {
			player.m_miscMod.armorClass -= spell->m_level;
		}
		
		spell = spells.getSpellOnTarget(PlayerEntityHandle, SPELL_CURSE);
		if(spell) {
			player.m_attributeMod.strength -= spell->m_level;
			player.m_attributeMod.constitution -= spell->m_level;
			player.m_attributeMod.dexterity -= spell->m_level;
			player.m_attributeMod.mind -= spell->m_level;
		}
	
		spell = spells.getSpellOnTarget(PlayerEntityHandle, SPELL_BLESS);
		if(spell) {
			player.m_attributeMod.strength += spell->m_level;
			player.m_attributeMod.dexterity += spell->m_level;
			player.m_attributeMod.constitution += spell->m_level;
			player.m_attributeMod.mind += spell->m_level;
		}
	
	// Calculate for modifiers from cheats
	if(cur_mr == 3) {
		PlayerAttribute attributeMod;
		attributeMod.strength = 1;
		attributeMod.mind = 10;
		attributeMod.constitution = 1;
		attributeMod.dexterity = 10;
		player.m_attributeMod.add(attributeMod);
		
		PlayerSkill skillMod;
		skillMod.stealth = 5;
		skillMod.mecanism = 5;
		skillMod.intuition = 100;
		skillMod.etheralLink = 100;
		skillMod.objectKnowledge = 100;
		skillMod.casting = 5;
		skillMod.projectile = 5;
		skillMod.closeCombat = 5;
		skillMod.defense = 100;
		player.m_skillMod.add(skillMod);
		
		PlayerMisc miscMod;
		miscMod.resistMagic = 100;
		miscMod.resistPoison = 100;
		miscMod.criticalHit = 5;
		miscMod.damages = 2;
		miscMod.armorClass = 100;
		player.m_miscMod.add(miscMod);
		
		player.Full_AimTime = 100;
	}
	if(sp_max) {
		PlayerAttribute attributeMod;
		attributeMod.strength = 5;
		attributeMod.mind = 5;
		attributeMod.constitution = 5;
		attributeMod.dexterity = 5;
		player.m_attributeMod.add(attributeMod);
		
		PlayerSkill skillMod;
		skillMod.stealth = 50;
		skillMod.mecanism = 50;
		skillMod.intuition = 50;
		skillMod.etheralLink = 50;
		skillMod.objectKnowledge = 50;
		skillMod.casting = 50;
		skillMod.projectile = 50;
		skillMod.closeCombat = 50;
		skillMod.defense = 50;
		player.m_skillMod.add(skillMod);
		
		PlayerMisc miscMod;
		miscMod.resistMagic = 10;
		miscMod.resistPoison = 10;
		miscMod.criticalHit = 50;
		miscMod.damages = 10;
		miscMod.armorClass = 20;
		player.m_miscMod.add(miscMod);
		
		player.Full_AimTime = 100;
	}
	if(player.m_cheatPnuxActive) {
		PlayerAttribute attributeMod;
		attributeMod.strength = Random::get(0, 5);
		attributeMod.mind = Random::get(0, 5);
		attributeMod.constitution = Random::get(0, 5);
		attributeMod.dexterity = Random::get(0, 5);
		player.m_attributeMod.add(attributeMod);
		
		PlayerSkill skillMod;
		skillMod.stealth = Random::get(0, 20);
		skillMod.mecanism = Random::get(0, 20);
		skillMod.intuition = Random::get(0, 20);
		skillMod.etheralLink = Random::get(0, 20);
		skillMod.objectKnowledge = Random::get(0, 20);
		skillMod.casting = Random::get(0, 20);
		skillMod.projectile = Random::get(0, 20);
		skillMod.closeCombat = Random::get(0, 20);
		skillMod.defense = Random::get(0, 30);
		player.m_skillMod.add(skillMod);
		
		PlayerMisc miscMod;
		miscMod.resistMagic = Random::get(0, 20);
		miscMod.resistPoison = Random::get(0, 20);
		miscMod.criticalHit = Random::get(0, 20);
		miscMod.damages = Random::get(0, 20);
		miscMod.armorClass = Random::get(0, 20);
		player.m_miscMod.add(miscMod);
	}
	if(cur_rf == 3) {
		PlayerAttribute attributeMod;
		attributeMod.mind = 10;
		player.m_attributeMod.add(attributeMod);
		
		PlayerSkill skillMod;
		skillMod.casting = 100;
		skillMod.etheralLink = 100;
		skillMod.objectKnowledge = 100;
		player.m_skillMod.add(skillMod);
		
		PlayerMisc miscMod;
		miscMod.resistMagic = 20;
		miscMod.resistPoison = 20;
		miscMod.damages = 1;
		miscMod.armorClass = 5;
		player.m_miscMod.add(miscMod);
	}
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Attributes
	
	// Calculate base attributes
	PlayerAttribute attributeBase;
	attributeBase.strength     = player.m_attribute.strength;
	attributeBase.dexterity    = player.m_attribute.dexterity;
	attributeBase.constitution = player.m_attribute.constitution;
	attributeBase.mind         = player.m_attribute.mind;
	
	// Calculate equipment modifiers for attributes
	player.m_attributeMod.strength += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_STRENGTH, attributeBase.strength
	);
	player.m_attributeMod.dexterity += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_DEXTERITY, attributeBase.dexterity
	);
	player.m_attributeMod.constitution += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_CONSTITUTION, attributeBase.constitution
	);
	player.m_attributeMod.mind += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_MIND, attributeBase.mind
	);
	
	// Calculate full alltributes
	player.m_attributeFull.strength = std::max(0.f, attributeBase.strength + player.m_attributeMod.strength);
	player.m_attributeFull.dexterity = std::max(0.f, attributeBase.dexterity + player.m_attributeMod.dexterity);
	player.m_attributeFull.constitution = std::max(0.f, attributeBase.constitution + player.m_attributeMod.constitution);
	player.m_attributeFull.mind = std::max(0.f, attributeBase.mind + player.m_attributeMod.mind);
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Skills
	
	// Calculate base skills
	PlayerSkill skillBase;
	skillBase.stealth          = player.m_skill.stealth
	                              + player.m_attributeFull.dexterity * 2.f;
	skillBase.mecanism         = player.m_skill.mecanism
	                              + player.m_attributeFull.dexterity
	                              + player.m_attributeFull.mind;
	skillBase.intuition        = player.m_skill.intuition
	                              + player.m_attributeFull.mind * 2.f;
	skillBase.etheralLink    = player.m_skill.etheralLink
	                              + player.m_attributeFull.mind * 2.f;
	skillBase.objectKnowledge = player.m_skill.objectKnowledge
	                              + player.m_attributeFull.mind * 1.5f
	                              + player.m_attributeFull.dexterity * 0.5f
	                              + player.m_attributeFull.strength * 0.5f;
	skillBase.casting          = player.m_skill.casting
	                              + player.m_attributeFull.mind * 2.f;
	skillBase.projectile       = player.m_skill.projectile
	                              + player.m_attributeFull.dexterity * 2.f
	                              + player.m_attributeFull.strength;
	skillBase.closeCombat     = player.m_skill.closeCombat
	                              + player.m_attributeFull.dexterity
	                              + player.m_attributeFull.strength * 2.f;
	skillBase.defense          = player.m_skill.defense
	                              + player.m_attributeFull.constitution * 3;
	
	// Calculate equipment modifiers for skills
	player.m_skillMod.stealth += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Stealth, skillBase.stealth
	);
	player.m_skillMod.mecanism += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Mecanism, skillBase.mecanism
	);
	player.m_skillMod.intuition += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Intuition, skillBase.intuition
	);
	player.m_skillMod.etheralLink += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Etheral_Link, skillBase.etheralLink
	);
	player.m_skillMod.objectKnowledge += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Object_Knowledge, skillBase.objectKnowledge
	);
	player.m_skillMod.casting += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Casting, skillBase.casting
	);
	player.m_skillMod.projectile += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Projectile, skillBase.projectile
	);
	player.m_skillMod.closeCombat += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Close_Combat, skillBase.closeCombat
	);
	player.m_skillMod.defense += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Defense, skillBase.defense
	);
	
	// Calculate full skills
	player.m_skillFull.stealth = skillBase.stealth + player.m_skillMod.stealth;
	player.m_skillFull.mecanism = skillBase.mecanism + player.m_skillMod.mecanism;
	player.m_skillFull.intuition = skillBase.intuition + player.m_skillMod.intuition;
	player.m_skillFull.etheralLink = skillBase.etheralLink + player.m_skillMod.etheralLink;
	player.m_skillFull.objectKnowledge = skillBase.objectKnowledge + player.m_skillMod.objectKnowledge;
	player.m_skillFull.casting = skillBase.casting + player.m_skillMod.casting;
	player.m_skillFull.projectile = skillBase.projectile + player.m_skillMod.projectile;
	player.m_skillFull.closeCombat = skillBase.closeCombat + player.m_skillMod.closeCombat;
	player.m_skillFull.defense = skillBase.defense + player.m_skillMod.defense;
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Other stats
	
	// Calculate base stats
	float base_armor_class   = std::max(1.f, player.m_skillFull.defense * 0.1f
	                           + -1.0f);
	float base_resist_magic  = player.m_attributeFull.mind * 2.f
	                           * (1.f + player.m_skillFull.casting * 0.005f); // TODO why *?
	float base_resist_poison = player.m_attributeFull.constitution * 2.f
	                           + player.m_skillFull.defense * 0.25f;
	float base_critical_hit  = player.m_attributeFull.dexterity * 2.f
	                           + player.m_skillFull.closeCombat * 0.2f
	                           + -18.f;
	float base_damages       = std::max(1.f, player.m_attributeFull.strength * 0.5f - 5.f);
	
	// Calculate equipment modifiers for stats
	player.m_miscMod.armorClass += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Armor_Class, base_armor_class
	);
	player.m_miscMod.resistMagic += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Resist_Magic, base_resist_magic
	);
	player.m_miscMod.resistPoison += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Resist_Poison, base_resist_poison
	);
	player.m_miscMod.criticalHit += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Critical_Hit, base_critical_hit
	);
	player.m_miscMod.damages += getEquipmentModifier(
		IO_EQUIPITEM_ELEMENT_Damages, base_damages
	);
	
	// Calculate full stats
	player.m_miscFull.armorClass = (int)std::max(0.f, base_armor_class + player.m_miscMod.armorClass);
	player.m_miscFull.resistMagic = (int)std::max(0.f, base_resist_magic + player.m_miscMod.resistMagic);
	player.m_miscFull.resistPoison = (int)std::max(0.f, base_resist_poison + player.m_miscMod.resistPoison);
	player.m_miscFull.criticalHit = std::max(0.f, base_critical_hit + player.m_miscMod.criticalHit);
	player.m_miscFull.damages = std::max(1.f, base_damages + player.m_miscMod.damages
	                                          + player.m_skillFull.closeCombat * 0.1f);
	
	
	/////////////////////////////////////////////////////////////////////////////////////
	
	player.Full_life = player.lifePool.current;
	player.Full_maxlife = player.m_attributeFull.constitution * (player.level + 2);
	player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
	player.Full_maxmana = player.m_attributeFull.mind * (player.level + 1);
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

	PlayerSkill skill;
	skill.stealth = 0;
	skill.mecanism = 0;
	skill.intuition = 0;
	skill.etheralLink = 0;
	skill.objectKnowledge = 0;
	skill.casting = 0;
	skill.projectile = 0;
	skill.closeCombat = 0;
	skill.defense = 0;
	
	player.m_skillOld = player.m_skill = skill;
	
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

	PlayerSkill skill;
	skill.stealth = 5;
	skill.mecanism = 5;
	skill.intuition = 5;
	skill.etheralLink = 5;
	skill.objectKnowledge = 5;
	skill.casting = 5;
	skill.projectile = 5;
	skill.closeCombat = 5;
	skill.defense = 5;
	
	player.m_skillOld = player.m_skill = skill;

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

	player.m_cheatSkinButtonClickCount = 0;
	player.m_cheatQuickGenButtonClickCount = 0;
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
		float rn = Random::getf();

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
		float rn = Random::getf();

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
long GetXPforLevel(short level)
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
	if(level < short(ARRAY_SIZE(XP_FOR_LEVEL)))
		xpNeeded = XP_FOR_LEVEL[level];
	else
		xpNeeded = level * 60000;
	return xpNeeded;	
}

/*!
 * \brief Manages Player Level Up event
 */
static void ARX_PLAYER_LEVEL_UP() {
	ARX_SOUND_PlayInterface(SND_PLAYER_LEVEL_UP);
	player.level++;
	player.Skill_Redistribute += 15;
	player.Attribute_Redistribute++;
	ARX_PLAYER_ComputePlayerStats();
	player.lifePool.current = player.lifePool.max;
	player.manaPool.current = player.manaPool.max;
	player.m_skillOld = player.m_skill;
	SendIOScriptEvent(entities.player(), SM_NULL, "", "level_up");
}

/*!
 * \brief Modify player XP by adding "val" to it
 * \param val
 */
void ARX_PLAYER_Modify_XP(long val) {
	
	player.xp += val;
	
	for(short i = player.level + 1; i < 11; i++) {
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
	if(Random::getf(0.f, 100.f) > player.m_misc.resistPoison) {
		player.poison += val;
		ARX_SOUND_PlayInterface(SND_PLAYER_POISONED);
	}
}

/*!
 * \brief updates some player stats depending on time
 * \param Framedelay
 *
 * Updates: life/mana recovery, poison evolution, hunger, invisibility
 */
void ARX_PLAYER_FrameCheck(float Framedelay)
{
	ARX_PROFILE_FUNC();
	
	//	ARX_PLAYER_QuickGeneration();
	if(Framedelay > 0) {
		UpdateIOInvisibility(entities.player());
		// Natural LIFE recovery
		float inc = 0.00008f * Framedelay * (player.m_attributeFull.constitution + player.m_attributeFull.strength * ( 1.0f / 2 ) + player.m_skillFull.defense) * ( 1.0f / 50 );

		if(player.lifePool.current > 0.f) {
			float inc_hunger = 0.00008f * Framedelay * (player.m_attributeFull.constitution + player.m_attributeFull.strength * ( 1.0f / 2 )) * ( 1.0f / 50 );

			// Check for player hungry sample playing
			if((player.hunger > 10.f && player.hunger - inc_hunger <= 10.f)
					|| (player.hunger < 10.f && arxtime.now() > LastHungerSample + ArxDurationMs(180000)))
			{
				LastHungerSample = arxtime.now();

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
			player.manaPool.current += 0.00008f * Framedelay * ((player.m_attributeFull.mind + player.m_skillFull.etheralLink) * 10) * ( 1.0f / 100 ); //framedelay*( 1.0f / 1000 );

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

				if(Random::getf(0.f, 100.f) > player.m_misc.resistPoison + faster) {
					float dmg = cp * ( 1.0f / 3 );

					if(player.lifePool.current - dmg <= 0.f)
						ARX_DAMAGES_DamagePlayer(dmg, DAMAGE_TYPE_POISON, EntityHandle());
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
	
	Entity * io = new Entity("graph/obj3d/interactive/player/player", EntityInstance(-1));
	arx_assert(io->index().handleData() == 0, "player entity didn't get index 0");
	arx_assert(entities.player() == io);
	
	io->obj = hero;

	player.skin = 0;
	ARX_PLAYER_Restore_Skin();

	ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	ARX_INTERACTIVE_HideGore(entities.player(), 1);
	
	ANIM_Set(player.bookAnimation[0], herowaitbook);
	player.bookAnimation[0].flags |= EA_LOOP;
	
	//todo free
	io->_npcdata = new IO_NPCDATA;
	
	io->ioflags = IO_NPC;
	io->_npcdata->lifePool.max = io->_npcdata->lifePool.current = 10.f;
	io->_npcdata->vvpos = -99999.f;

	//todo free
	io->armormaterial = "leather";
	loadScript(io->script, resources->getFile("graph/obj3d/interactive/player/player.asl"));

	if ((EERIE_OBJECT_GetGroup(io->obj, "head") != ObjVertGroup())
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "neck") != ObjVertGroup())
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "chest") != ObjVertGroup())
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "belt") != ObjVertGroup()))
	{
		io->_npcdata->ex_rotate = new EERIE_EXTRA_ROTATE();
		
		io->_npcdata->ex_rotate->group_number[0] = EERIE_OBJECT_GetGroup(io->obj, "head");
		io->_npcdata->ex_rotate->group_number[1] = EERIE_OBJECT_GetGroup(io->obj, "neck");
		io->_npcdata->ex_rotate->group_number[2] = EERIE_OBJECT_GetGroup(io->obj, "chest");
		io->_npcdata->ex_rotate->group_number[3] = EERIE_OBJECT_GetGroup(io->obj, "belt");
		
		for(size_t n = 0; n < MAX_EXTRA_ROTATE; n++) {
			io->_npcdata->ex_rotate->group_rotate[n] = Anglef::ZERO;
		}
	}

	ARX_INTERACTIVE_RemoveGoreOnIO(entities.player());
}

float Falling_Height = 0;

static void ARX_PLAYER_StartFall() {
	
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
	arx_assert(entities.player());
		
	// a mettre au final
	BLOCK_PLAYER_CONTROLS = true;

	player.Interface &= ~INTER_COMBATMODE;
	player.Interface = 0;
	DeadTime = 0;
	
	spells.endByCaster(PlayerEntityHandle);
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
	arx_assert(entities.player());
	
	ARX_PROFILE_FUNC();
	
	ArxInstant now = arxtime.now();
	
	if(player.m_currentMovement & PLAYER_ROTATE) {
		if(ROTATE_START == 0) {
			ROTATE_START = now;
		}
	} else if (ROTATE_START) {
		float elapsed = float(now) - ROTATE_START;
		if(elapsed > 100) {
			ROTATE_START = 0;
		}
	}
	
	{
	Entity * io = entities.player();
	
	if(!BLOCK_PLAYER_CONTROLS && sp_max) {
		io->halo.color = Color3f::red;
		io->halo.flags |= HALO_ACTIVE | HALO_DYNLIGHT;
		io->halo.radius = 20.f;
		player.lifePool.current += g_framedelay * 0.1f;
		player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
		player.manaPool.current += g_framedelay * 0.1f;
		player.manaPool.current = std::min(player.manaPool.current, player.Full_maxmana);
	}
	
	if(cur_mr == 3) {
		player.lifePool.current += g_framedelay * 0.05f;
		player.lifePool.current = std::min(player.lifePool.current, player.Full_maxlife);
		player.manaPool.current += g_framedelay * 0.05f;
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
	
	if(!(player.m_currentMovement & PLAYER_CROUCH) && player.physics.cyl.height > -150.f) {
		float old = player.physics.cyl.height;
		player.physics.cyl.height = player.baseHeight();
		player.physics.cyl.origin = player.basePosition();
		float anything = CheckAnythingInCylinder(player.physics.cyl, entities.player());
		if(anything < 0.f) {
			player.m_currentMovement |= PLAYER_CROUCH;
			player.physics.cyl.height = old;
		}
	}
	
	if(player.lifePool.current > 0) {
		io->angle = Anglef(0.f, 180.f - player.angle.getPitch(), 0.f);
	}
	
	io->gameFlags |= GFLAG_ISINTREATZONE;
	
	AnimLayer & layer0 = io->animlayer[0];
	AnimLayer & layer1 = io->animlayer[1];
	AnimLayer & layer2 = io->animlayer[2];
	AnimLayer & layer3 = io->animlayer[3];
	
	layer0.next_anim = NULL;
	layer1.next_anim = NULL;
	layer2.next_anim = NULL;
	layer3.next_anim = NULL;
	
	ANIM_HANDLE ** alist = io->anims;
	
	if(layer0.flags & EA_FORCEPLAY) {
		if(layer0.flags & EA_ANIMEND) {
			layer0.flags &= ~EA_FORCEPLAY;
			layer0.flags |= EA_STATICANIM;
			io->move = io->lastmove = Vec3f_ZERO;
		} else {
			layer0.flags &= ~EA_STATICANIM;
			player.pos = g_moveto = player.pos + io->move;
			io->pos = player.basePosition();
			goto nochanges;
		}
	}
	
	ANIM_HANDLE * request0_anim = NULL;
	ANIM_HANDLE * request3_anim = NULL;
	bool request0_loop = true;
	bool request0_stopend = false;
	
	if(io->ioflags & IO_FREEZESCRIPT) {
		goto nochanges;
	}
	
	if(player.lifePool.current <= 0) {
		HERO_SHOW_1ST = -1;
		io->animlayer[1].cur_anim = NULL;
		request0_anim = alist[ANIM_DIE];
		request0_loop = false;
		request0_stopend = true;
		goto makechanges;
	}
	
	if(   player.m_currentMovement == 0
	   || player.m_currentMovement == PLAYER_MOVE_STEALTH
	   || (player.m_currentMovement & PLAYER_ROTATE)
	) {
		if(player.Interface & INTER_COMBATMODE) {
			request0_anim = alist[ANIM_FIGHT_WAIT];
		} else if(EXTERNALVIEW) {
			request0_anim = alist[ANIM_WAIT];
		} else {
			request0_anim = alist[ANIM_WAIT_SHORT];
		}
		
		request0_loop = true;
	}
	
	if(ROTATE_START
	   && player.angle.getYaw() > 60.f
	   && player.angle.getYaw() < 180.f
	   && LASTPLAYERA > 60.f
	   && LASTPLAYERA < 180.f
	) {
		if(PLAYER_ROTATION < 0) {
			if(player.Interface & INTER_COMBATMODE)
				request0_anim = alist[ANIM_U_TURN_LEFT_FIGHT];
			else
				request0_anim = alist[ANIM_U_TURN_LEFT];
		} else {
			if(player.Interface & INTER_COMBATMODE)
				request0_anim = alist[ANIM_U_TURN_RIGHT_FIGHT];
			else
				request0_anim = alist[ANIM_U_TURN_RIGHT];
		}
		
		request0_loop = true;
		
		if(layer0.cur_anim == alist[ANIM_U_TURN_LEFT]
		   || layer0.cur_anim == alist[ANIM_U_TURN_LEFT_FIGHT])
		{
			float fv = PLAYER_ROTATION * 5;
			long vv = fv;
			io->frameloss -= fv - (float)vv;
			
			if (io->frameloss < 0) io->frameloss = 0;
			
			layer0.ctime -= vv;
			
			if(layer0.ctime < 0)
				layer0.ctime = 0;
		}
		else if(layer0.cur_anim == alist[ANIM_U_TURN_RIGHT]
				 ||	layer0.cur_anim == alist[ANIM_U_TURN_RIGHT_FIGHT])
		{
			float fv = PLAYER_ROTATION * 5;
			long vv = fv;
			io->frameloss += fv - (float)vv;
			
			if (io->frameloss < 0) io->frameloss = 0;
			
			layer0.ctime += vv;
			
			if(layer0.ctime < 0)
				layer0.ctime = 0;
		}
	}
	
	LASTPLAYERA = player.angle.getYaw();
	
	{
	long tmove = player.m_currentMovement;
	
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
	
	if(player.m_currentMovement & PLAYER_MOVE_WALK_FORWARD)
		tmove = PLAYER_MOVE_WALK_FORWARD;
	
	if(tmove & PLAYER_MOVE_STRAFE_LEFT) {
		if(player.Interface & INTER_COMBATMODE)
			request0_anim = alist[ANIM_FIGHT_STRAFE_LEFT];
		else if(player.m_currentMovement & PLAYER_MOVE_STEALTH)
			request0_anim = alist[ANIM_STRAFE_LEFT];
		else
			request0_anim = alist[ANIM_STRAFE_RUN_LEFT];
	}
	
	if(tmove & PLAYER_MOVE_STRAFE_RIGHT) {
		if(player.Interface & INTER_COMBATMODE)
			request0_anim = alist[ANIM_FIGHT_STRAFE_RIGHT];
		else if(player.m_currentMovement & PLAYER_MOVE_STEALTH)
			request0_anim = alist[ANIM_STRAFE_RIGHT];
		else
			request0_anim = alist[ANIM_STRAFE_RUN_RIGHT];
	}
	
	if(tmove & PLAYER_MOVE_WALK_BACKWARD) {
		if(player.Interface & INTER_COMBATMODE)
			request0_anim = alist[ANIM_FIGHT_WALK_BACKWARD];
		else if(player.m_currentMovement & PLAYER_MOVE_STEALTH)
			request0_anim = alist[ANIM_WALK_BACKWARD];
		else if(player.m_currentMovement & PLAYER_CROUCH)
			request0_anim = alist[ANIM_WALK_BACKWARD];
		else
			request0_anim = alist[ANIM_RUN_BACKWARD];
	}
	
	if(tmove & PLAYER_MOVE_WALK_FORWARD) {
		if(player.Interface & INTER_COMBATMODE)
			request0_anim = alist[ANIM_FIGHT_WALK_FORWARD];
		else if(player.m_currentMovement & PLAYER_MOVE_STEALTH)
			request0_anim = alist[ANIM_WALK];
		else
			request0_anim = alist[ANIM_RUN];
	}
	}
	
	if(!request0_anim) {
		if(EXTERNALVIEW)
			request0_anim = alist[ANIM_WAIT];
		else
			request0_anim = alist[ANIM_WAIT_SHORT];
		
		request0_loop = true;
	}
	
	// Finally update anim
	if(layer1.cur_anim == NULL
	   && (layer0.cur_anim == alist[ANIM_WAIT] || layer0.cur_anim == alist[ANIM_WAIT_SHORT])
	   && !(player.m_currentMovement & PLAYER_CROUCH)
	) {
		if ((player.m_currentMovement & PLAYER_LEAN_LEFT)
				&&	(player.m_currentMovement & PLAYER_LEAN_RIGHT))
		{
		} else {
			if(player.m_currentMovement & PLAYER_LEAN_LEFT) {
				request3_anim = alist[ANIM_LEAN_LEFT];
				//ChangeMA_Loop=0;
			}
			
			if(player.m_currentMovement & PLAYER_LEAN_RIGHT) {
				request3_anim = alist[ANIM_LEAN_RIGHT];
			}
		}
	}
	
	if(request3_anim == NULL
	   && layer3.cur_anim
	   && (layer3.cur_anim == alist[ANIM_LEAN_RIGHT] || layer3.cur_anim == alist[ANIM_LEAN_LEFT])
	) {
		AcquireLastAnim(io);
		layer3.cur_anim = NULL;
	}
	
	if((player.m_currentMovement & PLAYER_CROUCH) && !(player.Last_Movement & PLAYER_CROUCH)
			&& !player.levitate)
	{
		request0_anim = alist[ANIM_CROUCH_START];
		request0_loop = false;
	}
	else if(!(player.m_currentMovement & PLAYER_CROUCH) && (player.Last_Movement & PLAYER_CROUCH))
	{
		request0_anim = alist[ANIM_CROUCH_END];
		request0_loop = false;
	} else if(player.m_currentMovement & PLAYER_CROUCH) {
		if(layer0.cur_anim == alist[ANIM_CROUCH_START]) {
			if(!(layer0.flags & EA_ANIMEND)) {
				request0_anim = alist[ANIM_CROUCH_START];
				request0_loop = false;
			} else {
				request0_anim = alist[ANIM_CROUCH_WAIT];
				request0_loop = true;
				player.physics.cyl.height = player.crouchHeight();
			}
		} else {
			if(   request0_anim == alist[ANIM_STRAFE_LEFT]
			   || request0_anim == alist[ANIM_STRAFE_RUN_LEFT]
			   || request0_anim == alist[ANIM_FIGHT_STRAFE_LEFT]
			) {
				request0_anim = alist[ANIM_CROUCH_STRAFE_LEFT];
				request0_loop = true;
			} else if(   request0_anim == alist[ANIM_STRAFE_RIGHT]
			          || request0_anim == alist[ANIM_STRAFE_RUN_RIGHT]
			          || request0_anim == alist[ANIM_FIGHT_STRAFE_RIGHT]
			) {
				request0_anim = alist[ANIM_CROUCH_STRAFE_RIGHT];
				request0_loop = true;
			} else if(   request0_anim == alist[ANIM_WALK]
			          || request0_anim == alist[ANIM_RUN]
			          || request0_anim == alist[ANIM_FIGHT_WALK_FORWARD]
			) {
				request0_anim = alist[ANIM_CROUCH_WALK];
				request0_loop = true;
			} else if(   request0_anim == alist[ANIM_WALK_BACKWARD]
			          || request0_anim == alist[ANIM_FIGHT_WALK_BACKWARD]
			) {
				request0_anim = alist[ANIM_CROUCH_WALK_BACKWARD];
				request0_loop = true;
			} else {
				request0_anim = alist[ANIM_CROUCH_WAIT];
				request0_loop = true;
			}
		}
	}
	
	if(layer0.cur_anim == alist[ANIM_CROUCH_END]) {
		if(!(layer0.flags & EA_ANIMEND))
			goto nochanges;
	}
	
retry:
	;
	
	if(spells.ExistAnyInstanceForThisCaster(SPELL_FLYING_EYE, PlayerEntityHandle)) {
		request0_anim = alist[ANIM_MEDITATION];
		request0_loop = true;
		goto makechanges;
	} else if(spells.getSpellOnTarget(io->index(), SPELL_LEVITATE)) {
		request0_anim = alist[ANIM_LEVITATE];
		request0_loop = true;
		goto makechanges;
	} else if(player.jumpphase != NotJumping) {
		
		switch(player.jumpphase) {
			case NotJumping:
			break;
			case JumpStart: { // Anticipation
				FALLING_TIME = 0;
				player.jumpphase = JumpAscending;
				request0_anim = alist[ANIM_JUMP_UP];
				player.jumpstarttime = arxtime.now();
				player.jumplastposition = -1.f;
				break;
			}
			case JumpAscending: { // Moving up
				request0_anim = alist[ANIM_JUMP_UP];
				if(player.jumplastposition >= 1.f) {
					player.jumpphase = JumpDescending;
					request0_anim = alist[ANIM_JUMP_CYCLE];
					ARX_PLAYER_StartFall();
				}
				break;
			}
			case JumpDescending: { // Post-synch
				LAST_JUMP_ENDTIME = arxtime.now();
				if((layer0.cur_anim == alist[ANIM_JUMP_END] && (layer0.flags & EA_ANIMEND))
				   || player.onfirmground) {
					player.jumpphase = JumpEnd;
					request0_anim = alist[ANIM_JUMP_END_PART2];
				} else {
					request0_anim = alist[ANIM_JUMP_END];
				}
				break;
			}
			case JumpEnd: { // Post-synch
				LAST_JUMP_ENDTIME = arxtime.now();
				if(layer0.cur_anim == alist[ANIM_JUMP_END_PART2] && (layer0.flags & EA_ANIMEND)) {
					AcquireLastAnim(io);
					player.jumpphase = NotJumping;
					goto retry;
				} else if(layer0.cur_anim == alist[ANIM_JUMP_END_PART2]
						 && glm::abs(player.physics.velocity.x)
							 + glm::abs(player.physics.velocity.z) > (4.f/TARGET_DT)
						 && layer0.ctime > 1) {
					AcquireLastAnim(io);
					player.jumpphase = NotJumping;
					goto retry;
				} else {
					request0_anim = alist[ANIM_JUMP_END_PART2];
				}
				break;
			}
		}
		
		request0_loop = false;
		
		goto makechanges;
	} else {
	makechanges:
		;
		
		if(request0_anim && request0_anim != layer0.cur_anim) {
			AcquireLastAnim(io);
			ResetAnim(layer0);
			layer0.cur_anim = request0_anim;
			layer0.flags = EA_STATICANIM;
			
			if(request0_loop)
				layer0.flags |= EA_LOOP;
			
			if(request0_stopend)
				layer0.flags |= EA_STOPEND;
			
			if(request0_anim == alist[ANIM_U_TURN_LEFT]
			   || request0_anim == alist[ANIM_U_TURN_RIGHT]
			   || request0_anim == alist[ANIM_U_TURN_RIGHT_FIGHT]
			   || request0_anim == alist[ANIM_U_TURN_LEFT_FIGHT]
			) {
				layer0.flags |= EA_EXCONTROL;
			}
		}
		
		if(request3_anim && request3_anim != layer3.cur_anim) {
			AcquireLastAnim(io);
			ResetAnim(layer3);
			layer3.cur_anim = request3_anim;
			layer3.flags = EA_STATICANIM;
		}
	}
	
	io->physics = player.physics;
	}
	
nochanges:
	;
	player.Last_Movement = player.m_currentMovement;
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

	ActionPoint id = entities.player()->obj->fastaccess.view_attach;

	if(id != ActionPoint()) {
		tcam.orgTrans.pos = actionPointPosition(entities.player()->obj, id);
	} else {
		tcam.orgTrans.pos = player.pos;
	}

	id = io->obj->fastaccess.view_attach;

	if(id != ActionPoint()) {
		target = actionPointPosition(io->obj, id);
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

/*!
 * \brief Updates Many player infos each frame
 */
void ARX_PLAYER_Frame_Update()
{
	ARX_PROFILE_FUNC();
	
	if(spells.getSpellOnTarget(PlayerEntityHandle, SPELL_PARALYSE)) {
		player.m_paralysed = true;
	} else {
		entities.player()->ioflags &= ~IO_FREEZESCRIPT;
		player.m_paralysed = false;
	}

	// Reset player moveto info
	g_moveto = player.pos;

	// Reset current movement flags
	player.m_currentMovement = 0;

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
	}

	ARX_PLAYER_ComputePlayerFullStats();

	player.TRAP_DETECT = player.m_skillFull.mecanism;
	player.TRAP_SECRET = player.m_skillFull.intuition;

	if(spells.getSpellOnTarget(PlayerEntityHandle, SPELL_DETECT_TRAP))
		player.TRAP_DETECT = 100.f;

	ARX_PLAYER_ManageTorch();
}

/*!
 * \brief Emit player step noise
 */
static void ARX_PLAYER_MakeStepNoise() {
	
	if(spells.getSpellOnTarget(PlayerEntityHandle, SPELL_LEVITATE)) {
		return;
	}
	
	if(USE_PLAYERCOLLISIONS) {	
		float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT;
		float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT;
		
		if(player.m_currentMovement & PLAYER_MOVE_STEALTH) {
			float skill_stealth = player.m_skillFull.stealth / ARX_PLAYER_SKILL_STEALTH_MAX;
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
ArxInstant REQUEST_JUMP = ArxInstant_ZERO;
extern float Original_framedelay;

ArxInstant LAST_JUMP_ENDTIME = ArxInstant_ZERO;

static bool Valid_Jump_Pos() {
	
	if(LAST_ON_PLATFORM || player.climbing) {
		return true;
	}
	
	Cylinder tmpp = Cylinder(player.basePosition(), player.physics.cyl.radius * 0.85f, player.physics.cyl.height);
	
	float tmp = CheckAnythingInCylinder(tmpp, entities.player(),
	                                    CFLAG_PLAYER | CFLAG_JUST_TEST);
	if(tmp <= 20.f) {
		return true;
	}
	
	long hum = 0;
	for(float vv = 0; vv < 360.f; vv += 20.f) {
		tmpp.origin = player.basePosition();
		tmpp.origin += angleToVectorXZ(vv) * 20.f;
		
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
	
	ARX_PROFILE_FUNC();
	
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
	if(REQUEST_JUMP != ArxInstant_ZERO) {
		if((player.m_currentMovement & PLAYER_CROUCH)
		   || player.physics.cyl.height > player.baseHeight()) {
			float old = player.physics.cyl.height;
			player.physics.cyl.height = player.baseHeight();
			player.physics.cyl.origin = player.basePosition();
			float anything = CheckAnythingInCylinder(player.physics.cyl, entities.player(),
			                                         CFLAG_JUST_TEST);
			if(anything < 0.f) {
				player.m_currentMovement |= PLAYER_CROUCH;
				player.physics.cyl.height = old;
				REQUEST_JUMP = ArxInstant_ZERO;
			} else {
				bGCroucheToggle = false;
				player.m_currentMovement &= ~PLAYER_CROUCH;
				player.physics.cyl.height = player.baseHeight();
			}
		}
		
		if(!Valid_Jump_Pos()) {
			REQUEST_JUMP = ArxInstant_ZERO;
		}
		
		if(REQUEST_JUMP != ArxInstant_ZERO) {
			ArxDuration t = arxtime.now() - REQUEST_JUMP;
			if(t >= ArxDuration_ZERO && t <= ArxDurationMs(350)) {
				REQUEST_JUMP = ArxInstant_ZERO;
				ARX_NPC_SpawnAudibleSound(player.pos, entities.player());
				ARX_SPEECH_Launch_No_Unicode_Seek("player_jump", entities.player());
				player.onfirmground = false;
				player.jumpphase = JumpStart;
			}
		}
	}
	
	if(entities.player()->_npcdata->climb_count != 0.f && g_framedelay > 0) {
		entities.player()->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * g_framedelay * 0.1f;
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
					
					spells.endByTarget(PlayerEntityHandle, SPELL_LEVITATE);
				}
			}
			
			if(player.physics.cyl.height == player.levitateHeight()) {
				levitate = CFLAG_LEVITATE;
				player.climbing = false;
				bGCroucheToggle = false;
				player.m_currentMovement &= ~PLAYER_CROUCH;
			}
			
		} else if(player.physics.cyl.height == player.levitateHeight()) {
			player.physics.cyl.height = player.baseHeight();
		}
		
		if(player.jumpphase != JumpAscending && !levitate) {
			player.physics.cyl.origin = player.basePosition();
		}
		
		if(glm::abs(lastposy - player.pos.y) < DeltaTime * 0.1f) {
			TRUE_FIRM_GROUND = 1;
		} else {
			TRUE_FIRM_GROUND = 0;
		}
		
		lastposy = player.pos.y;
		float anything;
		Cylinder testcyl = player.physics.cyl;
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
		
		Cylinder cyl = Cylinder(player.basePosition() + Vec3f(0.f, 1.f, 0.f), player.physics.cyl.radius, player.physics.cyl.height);
		
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
						ARX_DAMAGES_DamagePlayer(dmg, 0, EntityHandle());
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
		if(arxtime.now() - LAST_JUMP_ENDTIME < ArxDurationMs(600)) {
			jump_mul = 0.5f;
			if(arxtime.now() - LAST_JUMP_ENDTIME >= ArxDurationMs(300)) {
				jump_mul += (float)(LAST_JUMP_ENDTIME + ArxDurationMs(300) - arxtime.now()) * (1.f / 300);
				if(jump_mul > 1.f) {
					jump_mul = 1.f;
				}
			}
		}
		
		Vec3f impulse = g_moveto - player.pos;
		if(impulse != Vec3f_ZERO) {
			
			const AnimLayer & layer0 = entities.player()->animlayer[0];
			float scale = 1.25f / 1000;
			if(layer0.cur_anim) {
				if(player.jumpphase != NotJumping) {
					if(player.m_currentMovement & PLAYER_MOVE_WALK_BACKWARD) {
						scale = 0.8f / 1000;
					} else if(player.m_currentMovement & PLAYER_MOVE_WALK_FORWARD) {
						scale = 7.9f / 1000;
					} else if(player.m_currentMovement & PLAYER_MOVE_STRAFE_LEFT) {
						scale = 2.6f / 1000;
					} else if(player.m_currentMovement & PLAYER_MOVE_STRAFE_RIGHT) {
						scale = 2.6f / 1000;
					} else {
						scale = 0.2f / 1000;
					}
				} else if(levitate && !player.climbing) {
					scale = 0.875f / 1000;
				} else {
					short idx = layer0.altidx_cur;
					Vec3f mv = GetAnimTotalTranslate(layer0.cur_anim, idx);
					float time = layer0.cur_anim->anims[idx]->anim_time;
					scale = glm::length(mv) / time * 0.0125f;
				}
			}
			
			impulse *= scale / glm::length(impulse) * jump_mul;
		}
		
		if(player.jumpphase != NotJumping) {
			// No Vertical Interpolation
			entities.player()->_npcdata->vvpos = -99999.f;
			if(player.jumpphase == JumpAscending) {
				g_moveto.y = player.pos.y;
				player.physics.velocity.y = 0;
			}
		}
		
		if(player.climbing) {
			player.physics.velocity.x = 0.f;
			player.physics.velocity.y *= 0.5f;
			player.physics.velocity.z = 0.f;
			if(player.m_currentMovement & PLAYER_MOVE_WALK_FORWARD) {
				g_moveto.x = player.pos.x;
				g_moveto.z = player.pos.z;
			}
			if(player.m_currentMovement & PLAYER_MOVE_WALK_BACKWARD) {
				impulse.x = 0;
				impulse.z = 0;
				g_moveto.x = player.pos.x;
				g_moveto.z = player.pos.z;
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
				if((ep->type & POLY_LAVA) && glm::abs(epcentery - (player.pos.y - player.baseHeight())) < 30) {
					float mul = 1.f - (glm::abs(epcentery - (player.pos.y - player.baseHeight())) * (1.0f / 30));
					const float LAVA_DAMAGE = 10.f;
					float damages = LAVA_DAMAGE * g_framedelay * 0.01f * mul;
					damages = ARX_SPELLS_ApplyFireProtection(entities.player(), damages);
					ARX_DAMAGES_DamagePlayer(damages, DAMAGE_TYPE_FIRE, PlayerEntityHandle);
					ARX_DAMAGES_DamagePlayerEquipment(damages);

					Vec3f pos = player.basePosition();
					ARX_PARTICLES_Spawn_Lava_Burn(pos, entities.player());
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
		if(glm::abs(player.physics.velocity.x) < 0.001f) {
			player.physics.velocity.x = 0;
		}
		if(glm::abs(player.physics.velocity.z) < 0.001f) {
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
			if(player.m_currentMovement & PLAYER_MOVE_WALK_FORWARD) {
				player.physics.velocity.y = -0.2f;
			}
			if(player.m_currentMovement & PLAYER_MOVE_WALK_BACKWARD) {
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
		if(glm::abs(player.physics.velocity.x) < 0.001f
		   && glm::abs(player.physics.velocity.z) < 0.001f
		   && player.onfirmground
		   && player.jumpphase == NotJumping
		) {
			g_moveto = player.pos;
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
					player.jumpstarttime = arxtime.now();
				}
				
				const float jump_up_time = 200.f;
				const float jump_up_height = 130.f;
				const ArxInstant now = arxtime.now();
				const unsigned long elapsed = now - player.jumpstarttime;
				float position = glm::clamp(float(elapsed) / jump_up_time, 0.f, 1.f);
				
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
								ARX_DAMAGES_DamagePlayer(dmg, 0, EntityHandle());
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
				if(player.m_currentMovement
				   && player.m_currentMovement != PLAYER_ROTATE
				   && !(player.m_currentMovement & PLAYER_MOVE_WALK_FORWARD)
				   && !(player.m_currentMovement & PLAYER_MOVE_WALK_BACKWARD)
				) {
					player.climbing = false;
				}
				
				if((player.m_currentMovement & PLAYER_MOVE_WALK_BACKWARD) && !test) {
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
			
			g_moveto = player.physics.cyl.origin + player.baseOffset();
			d = glm::distance(player.pos, g_moveto);
		}
	} else {
		Vec3f vect = g_moveto - player.pos;
		float divv = glm::length(vect);
		if(divv > 0.f) {
			float mul = g_framedelay * 0.001f * 200.f;
			divv = mul / divv;
			vect *= divv;
			g_moveto = player.pos + vect;
		}
		
		player.onfirmground = false;
	}
	
	if(player.pos == g_moveto) {
		d = 0.f;
	}
	
	// Emit Stepsound
	if(USE_PLAYERCOLLISIONS) {
		if(player.m_currentMovement & PLAYER_CROUCH) {
			d *= 2.f;
		}
		currentdistance += d;
		if(player.jumpphase == NotJumping && !player.falling
		   && currentdistance >= STEP_DISTANCE) {
			ARX_PLAYER_MakeStepNoise();
		}
	}
	
	// Finally update player pos !
	player.pos = g_moveto;
	
lasuite:
	;

	// Get Player position color
	float grnd_color = GetColorz(Vec3f(player.pos.x, player.pos.y + 90, player.pos.z)) - 15.f;
	if(CURRENT_PLAYER_COLOR < grnd_color) {
		CURRENT_PLAYER_COLOR += g_framedelay * (1.0f / 8);
		CURRENT_PLAYER_COLOR = std::min(CURRENT_PLAYER_COLOR, grnd_color);
	}
	if(CURRENT_PLAYER_COLOR > grnd_color) {
		CURRENT_PLAYER_COLOR -= g_framedelay * (1.0f / 4);
		CURRENT_PLAYER_COLOR = std::max(CURRENT_PLAYER_COLOR, grnd_color);
	}
	
	g_secondaryInventoryHud.updateFader();
}

/*!
 * \brief Manage Player Death Visual
 */
void ARX_PLAYER_Manage_Death() {
	if(DeadTime <= 2000)
		return;

	player.m_paralysed = false;
	float ratio = (float)(DeadTime - 2000) * ( 1.0f / 5000 );

	if(ratio >= 1.f) {
		ARX_MENU_Launch(false);
		DeadTime = 0;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(BlendZero, BlendInvSrcColor);
	EERIEDrawBitmap(Rectf(g_size), 0.000091f, NULL, Color3f::gray(ratio).to<u8>());
}

/*!
 * \brief Specific for color checks
 * \return
 */
float GetPlayerStealth() {
	return 15 + player.m_skillFull.stealth * ( 1.0f / 10 );
}

/*!
 * \brief Force Player to standard stance
 * \param val
 */
void ARX_PLAYER_PutPlayerInNormalStance() {
	
	if(player.m_currentMovement & PLAYER_CROUCH) {
		player.m_currentMovement &= ~PLAYER_CROUCH;
	}
	
	player.m_currentMovement = 0;
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
}

/*!
 * \brief Add gold to player purse
 * \param _lValue
 */
void ARX_PLAYER_AddGold(long _lValue) {
	player.gold += _lValue;
	g_hudRoot.purseIconGui.requestHalo();
}

void ARX_PLAYER_AddGold(Entity * gold) {
	
	arx_assert(gold->ioflags & IO_GOLD);
	
	ARX_PLAYER_AddGold(gold->_itemdata->price * std::max((short)1, gold->_itemdata->count));
	
	ARX_SOUND_PlayInterface(SND_GOLD);
	
	gold->gameFlags &= ~GFLAG_ISINTREATZONE;
	
	gold->destroy();
}

void ARX_PLAYER_Start_New_Quest() {
	
	player.m_cheatSkinButtonClickCount = 0;
	player.m_cheatQuickGenButtonClickCount = 0;
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
	   && player.m_skillFull.stealth >= _io->_itemdata->stealvalue
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

void ARX_PLAYER_Invulnerability(long flag) {

	if(flag)
		player.playerflags |= PLAYERFLAGS_INVULNERABILITY;
	else
		player.playerflags &= ~PLAYERFLAGS_INVULNERABILITY;
}

extern Entity * FlyingOverIO;

void ARX_GAME_Reset(long type) {
	arx_assert(entities.player());
	
	DeadTime = 0;
	
	LastValidPlayerPos = Vec3f_ZERO;
	
	entities.player()->speed_modif = 0;
	
	LAST_JUMP_ENDTIME = ArxInstant_ZERO;
	FlyingOverIO = NULL;
	g_miniMap.mapMarkerInit();
	ClearDynLights();

	if(!DONT_ERASE_PLAYER) {
		entities.player()->halo.flags = 0;
	}

	entities.player()->gameFlags &= ~GFLAG_INVISIBILITY;
	
	ARX_PLAYER_Invulnerability(0);
	player.m_paralysed = false;

	ARX_PLAYER_Reset_Fall();

	player.levitate = false;
	player.m_telekinesis = false;
	player.onfirmground = false;
	TRUE_FIRM_GROUND = 0;

	lastposy = -99999999999.f;

	ioSteal = NULL;

	GLOBAL_SLOWDOWN = 1.f;

	CheatReset();
	
	entities.player()->spellcast_data.castingspell = SPELL_NONE;
	
	ARX_INTERFACE_NoteClear();
	player.Interface = INTER_LIFE_MANA | INTER_MINIBACK | INTER_MINIBOOK;

	// Interactive DynData
	ARX_INTERACTIVE_ClearAllDynData();

	// PolyBooms
	PolyBoomClear();

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
	ARX_SCRIPT_ResetAll(false);
	
	// Speech Things
	REQUEST_SPEECH_SKIP = false;
	ARX_SPEECH_ClearAll();
	ARX_SPEECH_Reset();

	// Spells
	ARX_SPELLS_Precast_Reset();
	ARX_SPELLS_CancelSpellTarget();

	spells.clearAll();
	ARX_SPELLS_ClearAllSymbolDraw();
	ARX_SPELLS_ResetRecognition();

	// Particles
	ARX_PARTICLES_ClearAll();
	ParticleSparkClear();
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
	player.jumpstarttime = ArxInstant_ZERO;
	player.jumpphase = NotJumping;
	player.inzone = NULL;

	RemoveQuakeFX();
	player.m_improve = false;

	if(eyeball.exist) {
		eyeball.exist = -100;
	}
	
	entities.player()->ouch_time = ArxInstant_ZERO;
	entities.player()->invisibility = 0.f;
	
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

