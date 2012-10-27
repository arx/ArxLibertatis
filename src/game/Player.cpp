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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "game/Player.h"

#include <stddef.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>

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
#include "graphics/effects/Fog.h"
#include "graphics/particle/ParticleManager.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Random.h"
#include "math/Vector3.h"

#include "physics/Collisions.h"
#include "physics/Attractors.h"

#include "platform/Platform.h"

#include "scene/ChangeLevel.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/Object.h"

#include "script/Script.h"

using std::vector;

extern long		ARX_CONVERSATION;
extern long		HERO_SHOW_1ST;
extern long		REQUEST_SPEECH_SKIP;
extern long		CHANGE_LEVEL_ICON;
extern long		DONT_ERASE_PLAYER;
extern long		GLOBAL_MAGIC_MODE;
extern Vec3f	PUSH_PLAYER_FORCE;
extern QUAKE_FX_STRUCT QuakeFx;
extern Entity * CURRENT_TORCH;
extern Entity * CAMERACONTROLLER;
extern ParticleManager * pParticleManager;

extern unsigned long LAST_JUMP_ENDTIME;

static const float WORLD_GRAVITY = 0.1f;
static const float JUMP_GRAVITY = 0.02f;
static const float STEP_DISTANCE = 120.f;

extern Vec3f PUSH_PLAYER_FORCE;
extern bool bBookHalo;
extern bool bGoldHalo;
extern float InventoryX;
extern float InventoryDir;
extern long COLLIDED_CLIMB_POLY;
extern long HERO_SHOW_1ST;
extern long STARTED_A_GAME;
extern long SHOW_TORCH;
extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern unsigned long ulBookHaloTime;
extern unsigned long ulGoldHaloTime;
extern long cur_rf;

static const float ARX_PLAYER_SKILL_STEALTH_MAX = 100.f;

ARXCHARACTER player;
EERIE_3DOBJ * hero = NULL;
float currentdistance = 0.f;
float CURRENT_PLAYER_COLOR = 0;
float DeadCameraDistance = 0.f;
float PLAYER_ROTATION = 0;

long USE_PLAYERCOLLISIONS = 1;
long BLOCK_PLAYER_CONTROLS = 0;
long WILLRETURNTOCOMBATMODE = 0;
long DeadTime = 0;
unsigned long LastHungerSample = 0;
unsigned long ROTATE_START = 0;
long sp_max = 0;

// Player Anims FLAGS/Vars
ANIM_HANDLE * herowaitbook = NULL;
ANIM_HANDLE * herowait_2h = NULL;

ARX_NECKLACE necklace;

vector<KEYRING_SLOT> Keyring;
Entity * CURRENT_TORCH = NULL;

static unsigned long FALLING_TIME = 0;

vector<STRUCT_QUEST> PlayerQuest;
long FistParticles = 0;
void Manage_sp_max();
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
//*************************************************************************************
//*************************************************************************************
// KEYRING FUNCTIONS
//-------------------------------------------------------------------------------------

//*************************************************************************************
// void ARX_KEYRING_Init()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Init/Reset player Keyring structures
//*************************************************************************************
void ARX_KEYRING_Init() {
	Keyring.clear();
}
//*************************************************************************************
// void ARX_KEYRING_Add(char * key)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add a key to Keyring
//*************************************************************************************
void ARX_KEYRING_Add(const std::string & key) {
	Keyring.resize(Keyring.size() + 1);
	memset(&Keyring.back(), 0, sizeof(KEYRING_SLOT));
	strcpy(Keyring.back().slot, key.c_str());
}

//*************************************************************************************
// void ARX_KEYRING_Combine(Entity * io)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Sends COMBINE event to "io" for each keyring entry
//*************************************************************************************
void ARX_KEYRING_Combine(Entity * io) {
	for(size_t i = 0; i < Keyring.size(); i++) {
		if(SendIOScriptEvent(io, SM_COMBINE, Keyring[i].slot) == REFUSE) {
			return;
		}
	}
}
//-----------------------------------------------------------------------------
// KEYRING FUNCTIONS end
//******************************************************************************

//-----------------------------------------------------------------------------
//*************************************************************************************
// void ARX_PLAYER_FrontPos(EERIE_3D * pos)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Fills "pos" with player "front pos" for sound purpose
//*************************************************************************************
void ARX_PLAYER_FrontPos(Vec3f * pos)
{
	pos->x = player.pos.x - EEsin(radians(MAKEANGLE(player.angle.b))) * 100.f;
	pos->y = player.pos.y + 100.f; //-100.f;
	pos->z = player.pos.z + EEcos(radians(MAKEANGLE(player.angle.b))) * 100.f;
}

//*************************************************************************************
// void ARX_PLAYER_RectifyPosition()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Reset all extra-rotation groups of player
//*************************************************************************************
void ARX_PLAYER_RectifyPosition()
{
	Entity * io = entities.player();

	if ((io) && (io->_npcdata->ex_rotate))
	{
		for (long n = 0; n < MAX_EXTRA_ROTATE; n++)
		{
			io->_npcdata->ex_rotate->group_rotate[n].a = 0;
			io->_npcdata->ex_rotate->group_rotate[n].b = 0;
			io->_npcdata->ex_rotate->group_rotate[n].g = 0;
		}

		io->_npcdata->ex_rotate->flags = 0;
	}
}
//******************************************************************************
// PLAYER TORCH FUNCTIONS
//-----------------------------------------------------------------------------
void ARX_PLAYER_KillTorch() {
	
	ARX_SOUND_PlaySFX(SND_TORCH_END);
	ARX_SOUND_Stop(SND_TORCH_LOOP);
	
	giveToPlayer(CURRENT_TORCH);
	
	CURRENT_TORCH = NULL;
	SHOW_TORCH = 0;
	DynLight[0].exist = 0;
}

//-----------------------------------------------------------------------------
void ARX_PLAYER_ClickedOnTorch(Entity * io)
{
	if (io == NULL)
	{
		return;
	}

	if (CURRENT_TORCH == NULL)
	{
		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (ValidDynLight(io->ignit_light))
					DynLight[io->ignit_light].exist = 0;

				io->ignit_light = -1;

				if (io->ignit_sound != audio::INVALID_ID)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = audio::INVALID_ID;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;
			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			RemoveFromAllInventories(io);
			CURRENT_TORCH = io;
			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)
			{
				DRAGINTER = NULL;
			}
		}
	}
	else if (CURRENT_TORCH == io)
	{
		ARX_PLAYER_KillTorch();

	}
	else
	{
		ARX_PLAYER_KillTorch();

		if (io->durability > 0)
		{
			if (io->ignition > 0)
			{
				if (io->ignit_light != -1)
				{
					DynLight[io->ignit_light].exist = 0;
					io->ignit_light = -1;
				}

				if (io->ignit_sound != audio::INVALID_ID)
				{
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = audio::INVALID_ID;
				}

				io->ignition = 0;
			}

			SHOW_TORCH = 1;
			ARX_SOUND_PlaySFX(SND_TORCH_START);
			ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			RemoveFromAllInventories(io);
			CURRENT_TORCH = io;
			io->show = SHOW_FLAG_ON_PLAYER;

			if (DRAGINTER == io)
			{
				DRAGINTER = NULL;
			}
		}
	}
}

static void ARX_PLAYER_ManageTorch() {
	if (CURRENT_TORCH)
	{
		CURRENT_TORCH->ignition = 0;
		CURRENT_TORCH->durability -= FrameDiff * ( 1.0f / 10000 );

		if (CURRENT_TORCH->durability <= 0)
		{

			ARX_SPEECH_ReleaseIOSpeech(CURRENT_TORCH);
			// Need To Kill timers
			ARX_SCRIPT_Timer_Clear_By_IO(CURRENT_TORCH);
			CURRENT_TORCH->show = SHOW_FLAG_KILLED;
			CURRENT_TORCH->gameFlags &= ~GFLAG_ISINTREATZONE;
			RemoveFromAllInventories(CURRENT_TORCH);
			ARX_INTERACTIVE_DestroyDynamicInfo(CURRENT_TORCH);
			ARX_SOUND_PlaySFX(SND_TORCH_END);
			ARX_SOUND_Stop(SND_TORCH_LOOP);
			ARX_INTERACTIVE_DestroyIO(CURRENT_TORCH);
			CURRENT_TORCH = NULL;
			SHOW_TORCH = 0;
			DynLight[0].exist = 0;
		}
	}
}
//-----------------------------------------------------------------------------
// PLAYER TORCH FUNCTIONS end
//******************************************************************************


//*************************************************************************************
// void ARX_PLAYER_Quest_Init()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Init/Reset player Quest structures
//*************************************************************************************
void ARX_PLAYER_Quest_Init() {
	PlayerQuest.clear();
	gui::updateQuestBook();
}

//*************************************************************************************
// void ARX_Player_Rune_Add(unsigned long _ulRune)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add _ulRune to player runes
//*************************************************************************************
void ARX_Player_Rune_Add(RuneFlag _ulRune)
{
	int iNbSpells = 0;
	int iNbSpellsAfter = 0;

	for (size_t i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(player.rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpells ++;
			}
		}
	}

	player.rune_flags |= _ulRune;

	for (size_t i = 0; i < SPELL_COUNT; i++)
	{
		if (spellicons[i].bSecret == false)
		{
			long j = 0;
			bool bOk = true;

			while ((j < 4) && (spellicons[i].symbols[j] != 255))
			{
				if (!(player.rune_flags & (RuneFlag)(1 << spellicons[i].symbols[j])))
				{
					bOk = false;
				}

				j++;
			}

			if (bOk)
			{
				iNbSpellsAfter ++;
			}
		}
	}

	if (iNbSpellsAfter > iNbSpells)
	{
		MakeBookFX(DANAESIZX - INTERFACE_RATIO(35), DANAESIZY - INTERFACE_RATIO(148), 0.00001f);
		bBookHalo = true;
		ulBookHaloTime = 0;
	}
}

//*************************************************************************************
// void ARX_Player_Rune_Remove(unsigned long _ulRune
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Remove _ulRune from player runes
//*************************************************************************************
void ARX_Player_Rune_Remove(RuneFlag _ulRune)
{
	player.rune_flags &= ~_ulRune;
}

//*************************************************************************************
// void ARX_PLAYER_Quest_Add(char * quest)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Add quest "quest" to player Questbook
//*************************************************************************************
void ARX_PLAYER_Quest_Add(const std::string & quest, bool _bLoad) {
	
	PlayerQuest.push_back(STRUCT_QUEST());
	PlayerQuest.back().ident = quest;
	bBookHalo = !_bLoad;
	ulBookHaloTime = 0;
	
	gui::updateQuestBook();
}

//*************************************************************************************
// void ARX_PLAYER_Remove_Invisibility()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Removes player invisibility by killing Invisibility spells on him
//*************************************************************************************
void ARX_PLAYER_Remove_Invisibility() {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist && spells[i].type == SPELL_INVISIBILITY && spells[i].caster == 0) {
			spells[i].tolive = 0;
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

//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Stealth Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Stealth(long type) {
	if(type == 0) {
		return player.Skill_Stealth + player.Attribute_Dexterity * 2.f;
	} else {
		return player.Skill_Stealth + player.Mod_Skill_Stealth
		       + player.Full_Attribute_Dexterity * 2.f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Mecanism Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Mecanism(long type) {
	if(type == 0) {
		return player.Skill_Mecanism + player.Attribute_Dexterity + player.Attribute_Mind;
	} else {
		return player.Skill_Mecanism + player.Mod_Skill_Mecanism
		       + player.Full_Attribute_Dexterity + player.Full_Attribute_Mind;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Intuition Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Intuition(long type) {
	if(type == 0) {
		return player.Skill_Intuition + player.Attribute_Mind * 2.f;
	} else {
		return player.Skill_Intuition + player.Mod_Skill_Intuition
		       + player.Full_Attribute_Mind * 2.f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Etheral Link Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Etheral_Link(long type) {
	if(type == 0) {
		return player.Skill_Etheral_Link + player.Attribute_Mind * 2.f;
	} else {
		return player.Skill_Etheral_Link + player.Mod_Skill_Etheral_Link
		       + player.Full_Attribute_Mind * 2.f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Object Knowledge Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Object_Knowledge(long type) {
	if(type == 0) {
		return player.Skill_Object_Knowledge + player.Attribute_Mind * 1.5f
		       + player.Attribute_Dexterity * 0.5f + player.Attribute_Strength * 0.5f;
	} else {
		return player.Skill_Object_Knowledge + player.Mod_Skill_Object_Knowledge
		       + player.Full_Attribute_Mind * 1.5f + player.Full_Attribute_Dexterity * 0.5f
		       + player.Full_Attribute_Strength * 0.5f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Casting Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Casting(long type) {
	if(type == 0) {
		return player.Skill_Casting + player.Attribute_Mind * 2.f;
	} else {
		return player.Skill_Casting + player.Mod_Skill_Casting
		       + player.Full_Attribute_Mind * 2.f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Projectile Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Projectile(long type) {
	if(type == 0) {
		return player.Skill_Projectile
		       + player.Attribute_Dexterity * 2.f + player.Attribute_Strength;
	} else {
		return player.Skill_Projectile + player.Mod_Skill_Projectile
		       + player.Full_Attribute_Dexterity * 2.f + player.Full_Attribute_Strength;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Close Combat Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Close_Combat(long type) {
	if(type == 0) {
		return player.Skill_Close_Combat
		       + player.Attribute_Dexterity + player.Attribute_Strength * 2.f;
	} else {
		return player.Skill_Close_Combat + player.Mod_Skill_Close_Combat
		       + player.Full_Attribute_Dexterity + player.Full_Attribute_Strength * 2.f;
	}
}
//*************************************************************************************
// FUNCTION/RESULT:
//   Returns player Defense Skill level (Plain (type==0) or Modified (type==1))
//*************************************************************************************
static float ARX_PLAYER_Get_Skill_Defense(long type) {
	if(type == 0) {
		return player.Skill_Defense + player.Attribute_Constitution * 3;
	} else {
		return player.Skill_Defense + player.Mod_Skill_Defense
		       + player.Full_Attribute_Constitution * 3;
	}
}

//*************************************************************************************
// void ARX_PLAYER_ComputePlayerStats()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Compute secondary attributes for player
//*************************************************************************************
static void ARX_PLAYER_ComputePlayerStats() {
	
	player.maxlife = (float)player.Attribute_Constitution * (float)(player.level + 2);
	player.maxmana = (float)player.Attribute_Mind * (float)(player.level + 1);
	float t = ARX_PLAYER_Get_Skill_Defense(0);


	float fCalc = t * ( 1.0f / 10 ) - 1 ;
	player.armor_class = checked_range_cast<unsigned char>(fCalc);


	if (player.armor_class < 1) player.armor_class = 1;

	player.resist_magic = (unsigned char)(float)(player.Attribute_Mind * 2.f
	                      * (1.f + (ARX_PLAYER_Get_Skill_Casting(0)) * ( 1.0f / 200 )));

	fCalc = player.Attribute_Constitution * 2 + ((ARX_PLAYER_Get_Skill_Defense(1) * ( 1.0f / 4 )));
	player.resist_poison = checked_range_cast<unsigned char>(fCalc);


	player.damages = (player.Attribute_Strength - 10) * ( 1.0f / 2 );

	if (player.damages < 1.f) player.damages = 1.f;

	player.AimTime = 1500;
}
extern long cur_mr;
extern long SPECIAL_PNUX;
//*************************************************************************************
// void ARX_PLAYER_ComputePlayerFullStats()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Compute FULL versions of player stats including Equiped Items
//   and spells, and any other effect altering them.
//*************************************************************************************
void ARX_PLAYER_ComputePlayerFullStats()
{
	ARX_PLAYER_ComputePlayerStats();
	player.Mod_Attribute_Strength = 0;
	player.Mod_Attribute_Dexterity = 0;
	player.Mod_Attribute_Constitution = 0;
	player.Mod_Attribute_Mind = 0;
	player.Mod_Skill_Stealth = 0;
	player.Mod_Skill_Mecanism = 0;
	player.Mod_Skill_Intuition = 0;
	player.Mod_Skill_Etheral_Link = 0;
	player.Mod_Skill_Object_Knowledge = 0;
	player.Mod_Skill_Casting = 0;
	player.Mod_Skill_Projectile = 0;
	player.Mod_Skill_Close_Combat = 0;
	player.Mod_Skill_Defense = 0;
	player.Mod_armor_class = 0;
	player.Mod_resist_magic = 0;
	player.Mod_resist_poison = 0;
	player.Mod_Critical_Hit = 0;
	player.Mod_damages = 0;

	ARX_EQUIPMENT_IdentifyAll();

	player.Full_Weapon_Type = ARX_EQUIPMENT_GetPlayerWeaponType();

	Entity * io = entities.player();
	// Check for Equipment Modificators to Attributes
	player.Mod_Attribute_Strength = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_STRENGTH, player.Attribute_Strength);
	player.Mod_Attribute_Dexterity = ARX_EQUIPMENT_Apply(
	                                     io, IO_EQUIPITEM_ELEMENT_DEXTERITY, player.Attribute_Dexterity);
	player.Mod_Attribute_Constitution = ARX_EQUIPMENT_Apply(
	                                        io, IO_EQUIPITEM_ELEMENT_CONSTITUTION, player.Attribute_Constitution);
	player.Mod_Attribute_Mind = ARX_EQUIPMENT_Apply(
	                                io, IO_EQUIPITEM_ELEMENT_MIND, player.Attribute_Mind);
	player.Mod_armor_class = ARX_EQUIPMENT_Apply(
	                             io, IO_EQUIPITEM_ELEMENT_Armor_Class, player.armor_class);

	// Check for Equipment Modificators to Skills
	player.Mod_Skill_Stealth = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Stealth, ARX_PLAYER_Get_Skill_Stealth(0));
	player.Mod_Skill_Mecanism = ARX_EQUIPMENT_Apply(
	                                io, IO_EQUIPITEM_ELEMENT_Mecanism, ARX_PLAYER_Get_Skill_Mecanism(0));
	player.Mod_Skill_Intuition = ARX_EQUIPMENT_Apply(
	                                 io, IO_EQUIPITEM_ELEMENT_Intuition, ARX_PLAYER_Get_Skill_Intuition(0));
	player.Mod_Skill_Etheral_Link = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_Etheral_Link, ARX_PLAYER_Get_Skill_Etheral_Link(0));
	player.Mod_Skill_Object_Knowledge = ARX_EQUIPMENT_Apply(
	                                        io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, ARX_PLAYER_Get_Skill_Object_Knowledge(0));
	player.Mod_Skill_Casting = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Casting, ARX_PLAYER_Get_Skill_Casting(0));
	player.Mod_Skill_Projectile = ARX_EQUIPMENT_Apply(
	                                  io, IO_EQUIPITEM_ELEMENT_Projectile, ARX_PLAYER_Get_Skill_Projectile(0));
	player.Mod_Skill_Close_Combat = ARX_EQUIPMENT_Apply(
	                                    io, IO_EQUIPITEM_ELEMENT_Close_Combat, ARX_PLAYER_Get_Skill_Close_Combat(0));
	player.Mod_Skill_Defense = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Defense, ARX_PLAYER_Get_Skill_Defense(0));

	player.Mod_resist_magic = ARX_EQUIPMENT_Apply(
	                              io, IO_EQUIPITEM_ELEMENT_Resist_Magic, player.resist_magic);
	player.Mod_resist_poison = ARX_EQUIPMENT_Apply(
	                               io, IO_EQUIPITEM_ELEMENT_Resist_Poison, player.resist_poison);
	player.Mod_Critical_Hit = ARX_EQUIPMENT_Apply(
	                              io, IO_EQUIPITEM_ELEMENT_Critical_Hit, player.Critical_Hit);
	player.Mod_damages = ARX_EQUIPMENT_Apply(
	                         io, IO_EQUIPITEM_ELEMENT_Damages, 0);


	//CHECK OVERFLOW
	float fFullAimTime	= ARX_EQUIPMENT_Apply(io, IO_EQUIPITEM_ELEMENT_AimTime, 0);
	float fCalcHandicap	= (player.Full_Attribute_Dexterity - 10.f) * 20.f;

	//CAST
	player.Full_AimTime = checked_range_cast<long>(fFullAimTime);

	if (player.Full_AimTime <= 0) player.Full_AimTime = player.AimTime;

	player.Full_AimTime -= checked_range_cast<long>(fCalcHandicap);


	if (player.Full_AimTime <= 1500) player.Full_AimTime = 1500;



	/// PERCENTILE.....
	player.Mod_Attribute_Strength += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_STRENGTH, player.Attribute_Strength + player.Mod_Attribute_Strength);
	player.Mod_Attribute_Dexterity += ARX_EQUIPMENT_ApplyPercent(
	                                      io, IO_EQUIPITEM_ELEMENT_DEXTERITY, player.Attribute_Dexterity + player.Mod_Attribute_Dexterity);
	player.Mod_Attribute_Constitution += ARX_EQUIPMENT_ApplyPercent(
	        io, IO_EQUIPITEM_ELEMENT_CONSTITUTION, player.Attribute_Constitution + player.Mod_Attribute_Constitution);
	player.Mod_Attribute_Mind += ARX_EQUIPMENT_ApplyPercent(
	                                 io, IO_EQUIPITEM_ELEMENT_MIND, player.Attribute_Mind + player.Mod_Attribute_Mind);
	player.Mod_armor_class += ARX_EQUIPMENT_ApplyPercent(
	                              io, IO_EQUIPITEM_ELEMENT_Armor_Class, player.armor_class + player.Mod_armor_class);

	// Check for Equipment Modificators to Skills
	player.Mod_Skill_Stealth += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Stealth, ARX_PLAYER_Get_Skill_Stealth(1));
	player.Mod_Skill_Mecanism += ARX_EQUIPMENT_ApplyPercent(
	                                 io, IO_EQUIPITEM_ELEMENT_Mecanism, ARX_PLAYER_Get_Skill_Mecanism(1));
	player.Mod_Skill_Intuition += ARX_EQUIPMENT_ApplyPercent(
	                                  io, IO_EQUIPITEM_ELEMENT_Intuition, ARX_PLAYER_Get_Skill_Intuition(1));
	player.Mod_Skill_Etheral_Link += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_Etheral_Link, ARX_PLAYER_Get_Skill_Etheral_Link(1));
	player.Mod_Skill_Object_Knowledge += ARX_EQUIPMENT_ApplyPercent(
	        io, IO_EQUIPITEM_ELEMENT_Object_Knowledge, ARX_PLAYER_Get_Skill_Object_Knowledge(1));
	player.Mod_Skill_Casting += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Casting, ARX_PLAYER_Get_Skill_Casting(1));
	player.Mod_Skill_Projectile += ARX_EQUIPMENT_ApplyPercent(
	                                   io, IO_EQUIPITEM_ELEMENT_Projectile, ARX_PLAYER_Get_Skill_Projectile(1));
	player.Mod_Skill_Close_Combat += ARX_EQUIPMENT_ApplyPercent(
	                                     io, IO_EQUIPITEM_ELEMENT_Close_Combat, ARX_PLAYER_Get_Skill_Close_Combat(1));
	player.Mod_Skill_Defense += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Defense, ARX_PLAYER_Get_Skill_Defense(1));

	player.Mod_resist_magic += ARX_EQUIPMENT_ApplyPercent(
	                               io, IO_EQUIPITEM_ELEMENT_Resist_Magic, player.resist_magic + player.Mod_resist_magic);
	player.Mod_resist_poison += ARX_EQUIPMENT_ApplyPercent(
	                                io, IO_EQUIPITEM_ELEMENT_Resist_Poison, player.resist_poison + player.Mod_resist_poison);
	player.Mod_Critical_Hit += ARX_EQUIPMENT_ApplyPercent(
	                               io, IO_EQUIPITEM_ELEMENT_Critical_Hit, player.Critical_Hit + player.Mod_Critical_Hit);
	player.Mod_damages += ARX_EQUIPMENT_ApplyPercent(
	                          io, IO_EQUIPITEM_ELEMENT_Damages, player.damages);
	//player.Full_AimTime=ARX_EQUIPMENT_ApplyPercent(
	//	io,IO_EQUIPITEM_ELEMENT_AimTime,0);



	// Check for Spell Modificators
	if (entities.player())
		for (long i = 0; i < entities.player()->nb_spells_on; i++)
		{
			long n = entities.player()->spells_on[i];

			if (spells[n].exist)
			{
				switch (spells[n].type)
				{
					case SPELL_ARMOR:
						player.Mod_armor_class += spells[n].caster_level;
						break;
					case SPELL_LOWER_ARMOR:
						player.Mod_armor_class -= spells[n].caster_level;
						break;
					case SPELL_CURSE:
						player.Mod_Attribute_Strength -= spells[n].caster_level;
						player.Mod_Attribute_Constitution -= spells[n].caster_level;
						player.Mod_Attribute_Dexterity -= spells[n].caster_level;
						player.Mod_Attribute_Mind -= spells[n].caster_level;
						break;
					case SPELL_BLESS:
						player.Mod_Attribute_Strength += spells[n].caster_level;
						player.Mod_Attribute_Dexterity += spells[n].caster_level;
						player.Mod_Attribute_Constitution += spells[n].caster_level;
						player.Mod_Attribute_Mind += spells[n].caster_level;
						break;
					default: break;
				}
			}
		}

	if (cur_mr == 3)
	{
		player.Mod_Attribute_Strength += 1;
		player.Mod_Attribute_Mind += 10;
		player.Mod_Attribute_Constitution += 1;
		player.Mod_Attribute_Dexterity += 10;
		player.Mod_Skill_Stealth += 5;
		player.Mod_Skill_Mecanism += 5;
		player.Mod_Skill_Intuition += 100;
		player.Mod_Skill_Etheral_Link += 100;
		player.Mod_Skill_Object_Knowledge += 100;
		player.Mod_Skill_Casting += 5;
		player.Mod_Skill_Projectile += 5;
		player.Mod_Skill_Close_Combat += 5;
		player.Mod_Skill_Defense += 100;
		player.Mod_resist_magic += 100;
		player.Mod_resist_poison += 100;
		player.Mod_Critical_Hit += 5;
		player.Mod_damages += 2;
		player.Mod_armor_class += 100;
		player.Full_AimTime = 100;
	}

	if (sp_max)
	{
		player.Mod_Attribute_Strength += 5;
		player.Mod_Attribute_Mind += 5;
		player.Mod_Attribute_Constitution += 5;
		player.Mod_Attribute_Dexterity += 5;
		player.Mod_Skill_Stealth += 50;
		player.Mod_Skill_Mecanism += 50;
		player.Mod_Skill_Intuition += 50;
		player.Mod_Skill_Etheral_Link += 50;
		player.Mod_Skill_Object_Knowledge += 50;
		player.Mod_Skill_Casting += 50;
		player.Mod_Skill_Projectile += 50;
		player.Mod_Skill_Close_Combat += 50;
		player.Mod_Skill_Defense += 50;
		player.Mod_resist_magic += 10;
		player.Mod_resist_poison += 10;
		player.Mod_Critical_Hit += 50;
		player.Mod_damages += 10;
		player.Mod_armor_class += 20;
		player.Full_AimTime = 100;
	}

	if (SPECIAL_PNUX)
	{
		player.Mod_Attribute_Strength += Random::get(0, 5);
		player.Mod_Attribute_Mind += Random::get(0, 5);
		player.Mod_Attribute_Constitution += Random::get(0, 5);
		player.Mod_Attribute_Dexterity += Random::get(0, 5);
		player.Mod_Skill_Stealth += Random::get(0, 20);
		player.Mod_Skill_Mecanism += Random::get(0, 20);
		player.Mod_Skill_Intuition += Random::get(0, 20);
		player.Mod_Skill_Etheral_Link += Random::get(0, 20);
		player.Mod_Skill_Object_Knowledge += Random::get(0, 20);
		player.Mod_Skill_Casting += Random::get(0, 20);
		player.Mod_Skill_Projectile += Random::get(0, 20);
		player.Mod_Skill_Close_Combat += Random::get(0, 20);
		player.Mod_Skill_Defense += Random::get(0, 30);
		player.Mod_resist_magic += Random::get(0, 20);
		player.Mod_resist_poison += Random::get(0, 20);
		player.Mod_Critical_Hit += Random::get(0, 20);
		player.Mod_damages += Random::get(0, 20);
		player.Mod_armor_class += Random::get(0, 20);
	}

	if (cur_rf == 3)
	{
		player.Mod_Attribute_Mind += 10;
		player.Mod_Skill_Casting += 100;
		player.Mod_Skill_Etheral_Link += 100;
		player.Mod_Skill_Object_Knowledge += 100;
		player.Mod_resist_magic += 20;
		player.Mod_resist_poison += 20;
		player.Mod_damages += 1;
		player.Mod_armor_class += 5;
	}

	player.Full_armor_class = player.armor_class + player.Mod_armor_class;

	if (player.Full_armor_class < 0) player.Full_armor_class = 0;

	player.Full_Attribute_Strength = player.Attribute_Strength + player.Mod_Attribute_Strength;

	if (player.Full_Attribute_Strength < 0) player.Full_Attribute_Strength = 0;

	player.Full_Attribute_Mind = player.Attribute_Mind + player.Mod_Attribute_Mind;

	if (player.Full_Attribute_Mind < 0) player.Full_Attribute_Mind = 0;

	player.Full_Attribute_Constitution = player.Attribute_Constitution + player.Mod_Attribute_Constitution;

	if (player.Full_Attribute_Constitution < 0) player.Full_Attribute_Constitution = 0;

	player.Full_Attribute_Dexterity = player.Attribute_Dexterity + player.Mod_Attribute_Dexterity;

	if (player.Full_Attribute_Dexterity < 0) player.Full_Attribute_Dexterity = 0;

	player.Full_Skill_Stealth = ARX_PLAYER_Get_Skill_Stealth(1);
	player.Full_Skill_Mecanism = ARX_PLAYER_Get_Skill_Mecanism(1);
	player.Full_Skill_Intuition = ARX_PLAYER_Get_Skill_Intuition(1);
	player.Full_Skill_Etheral_Link = ARX_PLAYER_Get_Skill_Etheral_Link(1);
	player.Full_Skill_Object_Knowledge = ARX_PLAYER_Get_Skill_Object_Knowledge(1);
	player.Full_Skill_Casting = ARX_PLAYER_Get_Skill_Casting(1);
	player.Full_Skill_Projectile = ARX_PLAYER_Get_Skill_Projectile(1);
	player.Full_Skill_Close_Combat = ARX_PLAYER_Get_Skill_Close_Combat(1);
	player.Full_Skill_Defense = ARX_PLAYER_Get_Skill_Defense(1);

	player.Full_resist_magic = player.resist_magic + player.Mod_resist_magic;

	if (player.Full_resist_magic < 0) player.Full_resist_magic = 0;

	player.Full_resist_poison = player.resist_poison + player.Mod_resist_poison;

	if (player.Full_resist_poison < 0) player.Full_resist_poison = 0;

	player.Full_Critical_Hit = player.Critical_Hit + player.Mod_Critical_Hit;

	if (player.Full_Critical_Hit < 0) player.Full_Critical_Hit = 0;

	player.Full_damages = player.damages + player.Mod_damages
	                      + player.Full_Skill_Close_Combat * ( 1.0f / 10 );

	if (player.Full_damages < 1) player.Full_damages = 1;

	player.Full_life = player.life;
	player.Full_maxlife = (float)player.Full_Attribute_Constitution * (float)(player.level + 2);
	player.life = std::min(player.life, player.Full_maxlife);
	player.Full_maxmana = (float)player.Full_Attribute_Mind * (float)(player.level + 1);
	player.mana = std::min(player.mana, player.Full_maxmana);
}

//*************************************************************************************
// void ARX_PLAYER_MakeFreshHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates a Fresh hero
//*************************************************************************************
void ARX_PLAYER_MakeFreshHero()
{
	player.Attribute_Strength = 6;
	player.Attribute_Mind = 6;
	player.Attribute_Dexterity = 6;
	player.Attribute_Constitution = 6;

	player.Old_Skill_Stealth			=	player.Skill_Stealth			= 0;
	player.Old_Skill_Mecanism			=	player.Skill_Mecanism			= 0;
	player.Old_Skill_Intuition			=	player.Skill_Intuition			= 0;
	player.Old_Skill_Etheral_Link		=	player.Skill_Etheral_Link		= 0;
	player.Old_Skill_Object_Knowledge	=	player.Skill_Object_Knowledge	= 0;
	player.Old_Skill_Casting			=	player.Skill_Casting			= 0;
	player.Old_Skill_Projectile			=	player.Skill_Projectile			= 0;
	player.Old_Skill_Close_Combat		=	player.Skill_Close_Combat		= 0;
	player.Old_Skill_Defense			=	player.Skill_Defense			= 0;

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
char SKIN_MOD = 0;
char QUICK_MOD = 0;

void ARX_SPSound() {
	ARX_SOUND_PlayCinematic("kra_zoha_equip", false);
}

void ARX_PLAYER_MakeSpHero()
{
	ARX_SPSound();
	player.Attribute_Strength = 12;
	player.Attribute_Mind = 12;
	player.Attribute_Dexterity = 12;
	player.Attribute_Constitution = 12;

	player.Old_Skill_Stealth			=	player.Skill_Stealth			= 5;
	player.Old_Skill_Mecanism			=	player.Skill_Mecanism			= 5;
	player.Old_Skill_Intuition			=	player.Skill_Intuition			= 5;
	player.Old_Skill_Etheral_Link		=	player.Skill_Etheral_Link		= 5;
	player.Old_Skill_Object_Knowledge	=	player.Skill_Object_Knowledge	= 5;
	player.Old_Skill_Casting			=	player.Skill_Casting			= 5;
	player.Old_Skill_Projectile			=	player.Skill_Projectile			= 5;
	player.Old_Skill_Close_Combat		=	player.Skill_Close_Combat		= 5;
	player.Old_Skill_Defense			=	player.Skill_Defense			= 5;

	player.Attribute_Redistribute = 6;
	player.Skill_Redistribute = 10;

	player.level = 1;
	player.xp = 0;
	player.poison = 0.f;
	player.hunger = 100.f;
	player.skin = 4;

	ARX_PLAYER_ComputePlayerStats();
	player.life = player.maxlife;
	player.mana = player.maxmana;

	player.rune_flags = RuneFlags::all();
	player.SpellToMemorize.bSpell = false;

	SKIN_MOD = 0;
	QUICK_MOD = 0;
}
//*************************************************************************************
// void ARX_PLAYER_MakePowerfullHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates a POWERFULL hero
//*************************************************************************************
void ARX_PLAYER_MakePowerfullHero()
{
	player.Attribute_Strength = 18;
	player.Attribute_Mind = 18;
	player.Attribute_Dexterity = 18;
	player.Attribute_Constitution = 18;

	player.Old_Skill_Stealth			=	player.Skill_Stealth			= 82;
	player.Old_Skill_Mecanism			=	player.Skill_Mecanism			= 82;
	player.Old_Skill_Intuition			=	player.Skill_Intuition			= 82;
	player.Old_Skill_Etheral_Link		=	player.Skill_Etheral_Link		= 82;
	player.Old_Skill_Object_Knowledge	=	player.Skill_Object_Knowledge	= 82;
	player.Old_Skill_Casting			=	player.Skill_Casting			= 82;
	player.Old_Skill_Projectile			=	player.Skill_Projectile			= 82;
	player.Old_Skill_Close_Combat		=	player.Skill_Close_Combat		= 82;
	player.Old_Skill_Defense			=	player.Skill_Defense			= 82;

	player.Attribute_Redistribute = 0;
	player.Skill_Redistribute = 0;

	player.level = 10;
	player.xp = 178000;
	player.poison = 0.f;
	player.hunger = 100.f;
	player.skin = 0;

	ARX_PLAYER_ComputePlayerStats();
	player.life = player.maxlife;
	player.mana = player.maxmana;

	player.rune_flags = RuneFlags::all();
	player.SpellToMemorize.bSpell = false;
}

//*************************************************************************************
// void ARX_PLAYER_MakeAverageHero()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Creates an Average hero
//*************************************************************************************
void ARX_PLAYER_MakeAverageHero()
{
	ARX_PLAYER_MakeFreshHero();

	player.Attribute_Strength		+= 4;
	player.Attribute_Mind			+= 4;
	player.Attribute_Dexterity		+= 4;
	player.Attribute_Constitution	+= 4;

	player.Skill_Stealth			+= 2;
	player.Skill_Mecanism			+= 2;
	player.Skill_Intuition			+= 2;
	player.Skill_Etheral_Link		+= 2;
	player.Skill_Object_Knowledge	+= 2;
	player.Skill_Casting			+= 2;
	player.Skill_Projectile			+= 2;
	player.Skill_Close_Combat		+= 2;
	player.Skill_Defense			+= 2;

	player.Attribute_Redistribute = 0;
	player.Skill_Redistribute = 0;

	player.level = 0;
	player.xp = 0;
	player.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

//*************************************************************************************
// void ARX_PLAYER_QuickGeneration()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Quickgenerate a random hero
//*************************************************************************************
void ARX_PLAYER_QuickGeneration() {
	
	char old_skin = player.skin;
	ARX_PLAYER_MakeFreshHero();
	player.skin = old_skin;

	while (player.Attribute_Redistribute)
	{
		float rn = rnd();

		if ((rn < 0.25f) && (player.Attribute_Strength < 18))
		{
			player.Attribute_Strength++;
			player.Attribute_Redistribute--;
		}
		else if ((rn < 0.5f) && (player.Attribute_Mind < 18))
		{
			player.Attribute_Mind++;
			player.Attribute_Redistribute--;
		}
		else if ((rn < 0.75f) && (player.Attribute_Dexterity < 18))
		{
			player.Attribute_Dexterity++;
			player.Attribute_Redistribute--;
		}
		else if (player.Attribute_Constitution < 18)
		{
			player.Attribute_Constitution++;
			player.Attribute_Redistribute--;
		}
	}

	while (player.Skill_Redistribute)
	{
		float rn = rnd();

		if ((rn < 0.1f) && (player.Skill_Stealth < 18))
		{
			player.Skill_Stealth++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.2f) && (player.Skill_Mecanism < 18))
		{
			player.Skill_Mecanism++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.3f) && (player.Skill_Intuition < 18))
		{
			player.Skill_Intuition++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.4f) && (player.Skill_Etheral_Link < 18))
		{
			player.Skill_Etheral_Link++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.5f) && (player.Skill_Object_Knowledge < 18))
		{
			player.Skill_Object_Knowledge++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.6f) && (player.Skill_Casting < 18))
		{
			player.Skill_Casting++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.7f) && (player.Skill_Projectile < 18))
		{
			player.Skill_Projectile++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.8f) && (player.Skill_Close_Combat < 18))
		{
			player.Skill_Close_Combat++;
			player.Skill_Redistribute--;
		}
		else if ((rn < 0.9f) && (player.Skill_Defense < 18))
		{
			player.Skill_Defense++;
			player.Skill_Redistribute--;
		}
	}

	player.level = 0;
	player.xp = 0;
	player.hunger = 100.f;

	ARX_PLAYER_ComputePlayerStats();
}

//*************************************************************************************
// long GetXPforLevel(long level)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Returns necessary Experience for a given level
//*************************************************************************************
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

//*************************************************************************************
// void ARX_PLAYER_LEVEL_UP()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Manages Player Level Up event
//*************************************************************************************
void ARX_PLAYER_LEVEL_UP()
{
	ARX_SOUND_PlayInterface(SND_PLAYER_LEVEL_UP);
	player.level++;
	player.Skill_Redistribute += 15;
	player.Attribute_Redistribute++;
	ARX_PLAYER_ComputePlayerStats();
	player.life = player.maxlife;
	player.mana = player.maxmana;
	player.Old_Skill_Stealth			=	player.Skill_Stealth;
	player.Old_Skill_Mecanism			=	player.Skill_Mecanism;
	player.Old_Skill_Intuition			=	player.Skill_Intuition;
	player.Old_Skill_Etheral_Link		=	player.Skill_Etheral_Link;
	player.Old_Skill_Object_Knowledge	=	player.Skill_Object_Knowledge;
	player.Old_Skill_Casting			=	player.Skill_Casting;
	player.Old_Skill_Projectile			=	player.Skill_Projectile;
	player.Old_Skill_Close_Combat		=	player.Skill_Close_Combat;
	player.Old_Skill_Defense			=	player.Skill_Defense;
	SendIOScriptEvent(entities.player(), SM_NULL, "", "level_up");
}

//*************************************************************************************
// void ARX_PLAYER_Modify_XP(long val)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Modify player XP by adding "val" to it
//*************************************************************************************
void ARX_PLAYER_Modify_XP(long val) {
	
	player.xp += val;
	
	for (long i = player.level + 1; i < 11; i++) {
		if(player.xp >= GetXPforLevel(i)) {
			ARX_PLAYER_LEVEL_UP();
		}
	}
}

//*************************************************************************************
// void ARX_PLAYER_Poison(float val)
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Function to poison player by "val" poison level
//*************************************************************************************
void ARX_PLAYER_Poison(float val)
{
	// Make a poison saving throw to see if player is affected
	if (rnd() * 100.f > player.resist_poison)
	{
		player.poison += val;
		ARX_SOUND_PlayInterface(SND_PLAYER_POISONED);
	}
}
long PLAYER_PARALYSED = 0;

// FUNCTION/RESULT:
//   updates some player stats depending on time:
//		.life/mana recovery
//		.poison evolution
//		.hunger check
//		.invisibility
void ARX_PLAYER_FrameCheck(float Framedelay)
{
	//	ARX_PLAYER_QuickGeneration();
	if (Framedelay > 0)
	{
		UpdateIOInvisibility(entities.player());
		// Natural LIFE recovery
		float inc = 0.00008f * Framedelay * (player.Full_Attribute_Constitution + player.Full_Attribute_Strength * ( 1.0f / 2 ) + player.Full_Skill_Defense) * ( 1.0f / 50 );

		if (player.life > 0.f)
		{
			float inc_hunger = 0.00008f * Framedelay * (player.Full_Attribute_Constitution + player.Full_Attribute_Strength * ( 1.0f / 2 )) * ( 1.0f / 50 );

			// Check for player hungry sample playing
			if (((player.hunger > 10.f) && (player.hunger - inc_hunger <= 10.f))
			        || ((player.hunger < 10.f) && (float(arxtime) > LastHungerSample + 180000)))
			{
				LastHungerSample = (unsigned long)(arxtime);

				if (!BLOCK_PLAYER_CONTROLS)
				{
					bool bOk = true;

					for(size_t i = 0; i < MAX_ASPEECH; i++) {
						if(aspeech[i].exist && (aspeech[i].io == entities.player())) {
							bOk = false;
						}
					}

					if (bOk)
						ARX_SPEECH_AddSpeech(entities.player(), "player_off_hungry", ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
				}
			}

			player.hunger -= inc_hunger * .5f; //*.7f;

			if (player.hunger < -10.f) player.hunger = -10.f;

			if (!BLOCK_PLAYER_CONTROLS)
			{
				if (player.hunger < 0.f) player.life -= inc * ( 1.0f / 2 );
				else player.life += inc;
			}

			// Natural MANA recovery
			player.mana += 0.00008f * Framedelay * ((player.Full_Attribute_Mind + player.Full_Skill_Etheral_Link) * 10) * ( 1.0f / 100 ); //framedelay*( 1.0f / 1000 );

			if (player.mana > player.Full_maxmana) player.mana = player.Full_maxmana;
		}

		//float pmaxlife=(float)player.Full_Attribute_Constitution*(float)(player.level+2);
		if (player.life > player.Full_maxlife) player.life = player.Full_maxlife;

		// Now Checks Poison Progression
		if (!BLOCK_PLAYER_CONTROLS)
			if (player.poison > 0.f)
			{
				float cp = player.poison;
				cp *= ( 1.0f / 2 ) * Framedelay * ( 1.0f / 1000 ) * ( 1.0f / 2 );
				float faster = 10.f - player.poison;

				if (faster < 0.f) faster = 0.f;

				if (rnd() * 100.f > player.resist_poison + faster)
				{
					float dmg = cp * ( 1.0f / 3 );

					if (player.life - dmg <= 0.f) ARX_DAMAGES_DamagePlayer(dmg, DAMAGE_TYPE_POISON, -1);
					else player.life -= dmg;

					player.poison -= cp * ( 1.0f / 10 );
				}
				else player.poison -= cp;
			}

		if (player.poison < 0.1f) player.poison = 0.f;
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

	if (PLAYER_SKIN_TC && !tx.empty())
		PLAYER_SKIN_TC->LoadFile(tx);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_hero_head");
	if (tmpTC && !tx2.empty())
		tmpTC->LoadFile(tx2);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_chainmail_mithril_hero_head");
	if (tmpTC && !tx3.empty())
		tmpTC->LoadFile(tx3);

	tmpTC = TextureContainer::Find("graph/obj3d/textures/npc_human_leather_hero_head");
	if (tmpTC && !tx4.empty())
		tmpTC->LoadFile(tx4);
}

//Load Mesh & anims for hero
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
	arx_assert_msg(io->index() == 0, "player entity didn't get index 0");
	arx_assert(entities.player() == io);
	
	io->obj = hero;

	player.skin = 0;
	ARX_PLAYER_Restore_Skin();

	ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	ARX_INTERACTIVE_HideGore(entities.player(), 1);
	io->ident = -1;

	//todo free
	io->_npcdata = new IO_NPCDATA;
	
	io->ioflags = IO_NPC;
	io->_npcdata->maxlife = io->_npcdata->life = 10.f;
	io->_npcdata->vvpos = -99999.f;

	//todo free
	io->armormaterial = "leather";
	loadScript(io->script, resources->getFile("graph/obj3d/interactive/player/player.asl"));

	if ((EERIE_OBJECT_GetGroup(io->obj, "head") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "neck") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "chest") != -1)
	        &&	(EERIE_OBJECT_GetGroup(io->obj, "belt") != -1))
	{
		io->_npcdata->ex_rotate = (EERIE_EXTRA_ROTATE *)malloc(sizeof(EERIE_EXTRA_ROTATE));

		if(io->_npcdata->ex_rotate)
		{
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
	}

	ARX_INTERACTIVE_RemoveGoreOnIO(entities.player());
}
float Falling_Height = 0;
void ARX_PLAYER_StartFall()
{
	FALLING_TIME = 1;
	Falling_Height = 50.f;
	float yy;
	EERIEPOLY * ep = CheckInPoly(player.pos.x, player.pos.y, player.pos.z, &yy);

	if (ep)
	{
		Falling_Height = player.pos.y;
	}
}

//*************************************************************************************
// void ARX_PLAYER_BecomesDead()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Called When player has just died
//*************************************************************************************
void ARX_PLAYER_BecomesDead()
{
	STARTED_A_GAME = 0;
	// a mettre au final
	BLOCK_PLAYER_CONTROLS = 1;

	if (entities.player())
	{
		player.Interface &= ~INTER_COMBATMODE;
		player.Interface = 0;
		DeadCameraDistance = 40.f;
		DeadTime = 0;
	}

	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist && spells[i].caster == 0) {
			spells[i].tolive = 0;
		}
	}
}

float LASTPLAYERA = 0;
extern long ON_PLATFORM;
long LAST_ON_PLATFORM = 0;
extern long MOVE_PRECEDENCE;
extern long EXTERNALVIEW;

// Manages Player visual
// Choose the set of animations to use to represent current player
// situation.
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
	
	static long special[3];
	long light = 0;
	
	if(entities.player()) {
		
		Entity * io = entities.player();
		
		if(!BLOCK_PLAYER_CONTROLS && sp_max) {
			io->halo.color = Color3f::red;
			io->halo.flags |= HALO_ACTIVE | HALO_DYNLIGHT;
			io->halo.radius = 20.f;
			player.life += float(FrameDiff) * 0.1f;
			player.life = std::min(player.life, player.maxlife);
			player.mana += float(FrameDiff) * 0.1f;
			player.mana = std::min(player.mana, player.maxmana);
		}
		
		if(cur_mr == 3) {
			player.life += float(FrameDiff) * 0.05f;
			player.life = std::min(player.life, player.maxlife);
			player.mana += float(FrameDiff) * 0.05f;
			player.mana = std::min(player.mana, player.maxmana);
		}
		
		io->pos = player.basePosition();
		
		if(player.jumpphase == NotJumping && !LAST_ON_PLATFORM) {
			float t;
			EERIEPOLY * ep = CheckInPolyPrecis(player.pos.x, player.pos.y, player.pos.z, &t);
			if(ep && io->pos.y > t - 30.f && io->pos.y < t) {
				player.onfirmground = 1;
			}
		}
		
		ComputeVVPos(io);
		io->pos.y = io->_npcdata->vvpos;
		
		if(!(player.Current_Movement & PLAYER_CROUCH) && player.physics.cyl.height > -150.f) {
			float old = player.physics.cyl.height;
			player.physics.cyl.height = player.baseHeight();
			player.physics.cyl.origin = player.basePosition();
			float anything = CheckAnythingInCylinder(&player.physics.cyl, entities.player());
			if(anything < 0.f) {
				player.Current_Movement |= PLAYER_CROUCH;
				player.physics.cyl.height = old;
			}
		}
		
		if(player.life > 0) {
			io->angle = Anglef(0.f, 180.f - player.angle.b, 0.f);
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
				io->move = io->lastmove = Vec3f::ZERO;
			} else {
				ause0->flags &= ~EA_STATICANIM;
				player.pos = moveto = player.pos + io->move;
				io->pos = player.basePosition();
				goto nochanges;
			}
		}
		
		ANIM_HANDLE * ChangeMoveAnim = NULL;
		ANIM_HANDLE * ChangeMoveAnim2 = NULL;
		long ChangeMA_Loop = 1;
		long ChangeMA_Stopend = 0;
		
		if(io->ioflags & IO_FREEZESCRIPT) {
			goto nochanges;
		}
		
		if(player.life <= 0) {
			HERO_SHOW_1ST = -1;
			io->animlayer[1].cur_anim = NULL;
			ChangeMoveAnim = alist[ANIM_DIE];
			ChangeMA_Loop = 0;
			ChangeMA_Stopend = 1;
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
			ChangeMA_Loop = 1;
		}
		
		if(ROTATE_START && player.angle.a > 60.f && player.angle.a < 180.f
		   && LASTPLAYERA > 60.f && LASTPLAYERA < 180.f) {
			if (PLAYER_ROTATION < 0)
			{
				if (player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_LEFT_FIGHT];
				else	ChangeMoveAnim = alist[ANIM_U_TURN_LEFT];
			}
			else
			{
				if (player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT_FIGHT];
				else	ChangeMoveAnim = alist[ANIM_U_TURN_RIGHT];
			}

			ChangeMA_Loop = 1;

			if ((ause0->cur_anim == alist[ANIM_U_TURN_LEFT])
			        ||	(ause0->cur_anim == alist[ANIM_U_TURN_LEFT_FIGHT]))
			{
				float fv = PLAYER_ROTATION * 5;
				long vv = fv;
				io->frameloss -= fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime -= vv;

				if (ause0->ctime < 0)
					ause0->ctime = 0;
			}
			else if ((ause0->cur_anim == alist[ANIM_U_TURN_RIGHT])
			         ||	(ause0->cur_anim == alist[ANIM_U_TURN_RIGHT_FIGHT]))
			{
				long vv = PLAYER_ROTATION * 5;
				float fv = PLAYER_ROTATION * 5;
				io->frameloss += fv - (float)vv;

				if (io->frameloss < 0) io->frameloss = 0;

				ause0->ctime += vv;

				if (ause0->ctime < 0)
					ause0->ctime = 0;
			}
		}

		LASTPLAYERA = player.angle.a;

		{
			long tmove = player.Current_Movement;

			if (((tmove & PLAYER_MOVE_STRAFE_LEFT)
			        && (tmove & PLAYER_MOVE_STRAFE_RIGHT)))
			{
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;
			}

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_LEFT)
				tmove &= ~PLAYER_MOVE_STRAFE_RIGHT;

			if (MOVE_PRECEDENCE == PLAYER_MOVE_STRAFE_RIGHT)
				tmove &= ~PLAYER_MOVE_STRAFE_LEFT;

			if (MOVE_PRECEDENCE == PLAYER_MOVE_WALK_FORWARD)
				tmove &= ~PLAYER_MOVE_WALK_BACKWARD;

			if (player.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				tmove = PLAYER_MOVE_WALK_FORWARD;

			{
				if (tmove & PLAYER_MOVE_STRAFE_LEFT)
				{
					if (player.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_LEFT];
					else if (player.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_LEFT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_LEFT];
				}

				if (tmove & PLAYER_MOVE_STRAFE_RIGHT)
				{
					if (player.Interface & INTER_COMBATMODE)
						ChangeMoveAnim = alist[ANIM_FIGHT_STRAFE_RIGHT];
					else if (player.Current_Movement & PLAYER_MOVE_STEALTH)
						ChangeMoveAnim = alist[ANIM_STRAFE_RIGHT];
					else
						ChangeMoveAnim = alist[ANIM_STRAFE_RUN_RIGHT];
				}
			}

			if (tmove & PLAYER_MOVE_WALK_BACKWARD)
			{
				if (player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_BACKWARD];
				else if (player.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else if (player.Current_Movement & PLAYER_CROUCH)
					ChangeMoveAnim = alist[ANIM_WALK_BACKWARD];
				else
					ChangeMoveAnim = alist[ANIM_RUN_BACKWARD];
			}

			if (tmove & PLAYER_MOVE_WALK_FORWARD)
			{

				if (player.Interface & INTER_COMBATMODE)
					ChangeMoveAnim = alist[ANIM_FIGHT_WALK_FORWARD];
				else if (player.Current_Movement & PLAYER_MOVE_STEALTH)
					ChangeMoveAnim = alist[ANIM_WALK];
				else
					ChangeMoveAnim = alist[ANIM_RUN];
			}

		}

		if (ChangeMoveAnim == NULL)
		{
			if (EXTERNALVIEW)
				ChangeMoveAnim = alist[ANIM_WAIT];
			else
				ChangeMoveAnim = alist[ANIM_WAIT_SHORT];

			ChangeMA_Loop = 1;
		}

		// Finally update anim
		if ((ause1->cur_anim == NULL)
		        &&	((ause0->cur_anim == alist[ANIM_WAIT])
		             ||	(ause0->cur_anim == alist[ANIM_WAIT_SHORT]))
		        &&	(!(player.Current_Movement & PLAYER_CROUCH)))
		{
			if ((player.Current_Movement & PLAYER_LEAN_LEFT)
			        &&	(player.Current_Movement & PLAYER_LEAN_RIGHT))
			{
			}
			else
			{
				if (player.Current_Movement & PLAYER_LEAN_LEFT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_LEFT];
					//ChangeMA_Loop=0;
				}

				if (player.Current_Movement & PLAYER_LEAN_RIGHT)
				{
					ChangeMoveAnim2 = alist[ANIM_LEAN_RIGHT];
				}
			}
		}

		if ((ChangeMoveAnim2 == NULL)
		        &&	(ause3->cur_anim)
		        &&	((ause3->cur_anim == alist[ANIM_LEAN_RIGHT])
		             ||	(ause3->cur_anim == alist[ANIM_LEAN_LEFT]))
		   )
		{
			AcquireLastAnim(io);
			ause3->cur_anim = NULL;
		}

		if ((player.Current_Movement & PLAYER_CROUCH) && !(player.Last_Movement & PLAYER_CROUCH)
		        && !player.levitate)
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_START];
			ChangeMA_Loop = 0;
		}
		else if (!(player.Current_Movement & PLAYER_CROUCH) && (player.Last_Movement & PLAYER_CROUCH))
		{
			ChangeMoveAnim = alist[ANIM_CROUCH_END];
			ChangeMA_Loop = 0;
		}
		else if (player.Current_Movement & PLAYER_CROUCH)
		{
			if (ause0->cur_anim == alist[ANIM_CROUCH_START])
			{
				if (!(ause0->flags & EA_ANIMEND))
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_START];
					ChangeMA_Loop = 0;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
					player.physics.cyl.height = player.crouchHeight();
				}
			}
			else
			{
				if ((ChangeMoveAnim == alist[ANIM_STRAFE_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_STRAFE_RUN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_LEFT])
				   )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_LEFT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_STRAFE_RIGHT])
				         ||	(ChangeMoveAnim == alist[ANIM_STRAFE_RUN_RIGHT])
				         ||	(ChangeMoveAnim == alist[ANIM_FIGHT_STRAFE_RIGHT])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_STRAFE_RIGHT];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK])
				         ||	(ChangeMoveAnim == alist[ANIM_RUN])
				         ||	(ChangeMoveAnim == alist[ANIM_FIGHT_WALK_FORWARD])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK];
					ChangeMA_Loop = 1;
				}
				else if ((ChangeMoveAnim == alist[ANIM_WALK_BACKWARD])
				         || (ChangeMoveAnim == alist[ANIM_FIGHT_WALK_BACKWARD])
				        )
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WALK_BACKWARD];
					ChangeMA_Loop = 1;
				}
				else
				{
					ChangeMoveAnim = alist[ANIM_CROUCH_WAIT];
					ChangeMA_Loop = 1;
				}
			}
		}

		if (ause0->cur_anim == alist[ANIM_CROUCH_END])
		{
			if (!(ause0->flags & EA_ANIMEND))
				goto nochanges;
		}

	retry:
		;

		if (ARX_SPELLS_ExistAnyInstance(SPELL_FLYING_EYE))
		{
			FistParticles |= 1;
		}
		else FistParticles &= ~1;

		if (FistParticles)
		{
			light = 1;

			if (FistParticles & 1)
			{
				ChangeMoveAnim = alist[ANIM_MEDITATION];
				ChangeMA_Loop = 1;
			}

			EERIE_3DOBJ * eobj = io->obj;
			long pouet = 2;

			while (pouet)
			{
				long id;

				if (pouet == 2)
					id = io->obj->fastaccess.primary_attach;
				else
					id = GetActionPointIdx(io->obj, "left_attach");

				pouet--;

				if (id != -1)
				{
					if (special[pouet] == -1)
					{
						special[pouet] = GetFreeDynLight();
					}

					if (special[pouet] != -1)
					{
						EERIE_LIGHT * el = &DynLight[special[pouet]];
						el->intensity = 1.3f;
						el->exist = 1;
						el->fallend = 180.f;
						el->fallstart = 50.f;

						if (FistParticles & 2)
						{
							el->rgb.r = 1.f;
							el->rgb.g = 0.3f;
							el->rgb.b = 0.2f;
						}
						else
						{
							el->rgb.r = 0.7f;
							el->rgb.g = 0.3f;
							el->rgb.b = 1.f;
						}

						el->pos.x = eobj->vertexlist3[id].v.x;
						el->pos.y = eobj->vertexlist3[id].v.y;
						el->pos.z = eobj->vertexlist3[id].v.z;
					}
					else
					{
						LogWarning << "Maximum number of dynamic lights exceeded.";
						/*
						EERIE_LIGHT * el = &DynLight[special[pouet]];
						el->intensity = 1.3f + rnd() * 0.2f;
						el->fallend = 175.f + rnd() * 10.f;

						if (FistParticles & 2)
						{
							el->fallstart *= 2.f;
							el->fallend *= 3.f;
						}
						*/
					}

					for (long kk = 0; kk < 2; kk++)
					{
						Vec3f  target;
						target.x = eobj->vertexlist3[id].v.x;
						target.y = eobj->vertexlist3[id].v.y;
						target.z = eobj->vertexlist3[id].v.z;
						long j = ARX_PARTICLES_GetFree();

						if ((j != -1) && (!arxtime.is_paused()))
						{
							ParticleCount++;
							particle[j].exist = true;
							particle[j].zdec = 0;

							particle[j].ov.x	=	target.x + 1.f - rnd() * 2.f;
							particle[j].ov.y	=	target.y + 1.f - rnd() * 2.f;
							particle[j].ov.z	=	target.z + 1.f - rnd() * 2.f;
							particle[j].move.x	=	0.1f - 0.2f * rnd();
							particle[j].move.y	=	-2.2f * rnd();
							particle[j].move.z	=	0.1f - 0.2f * rnd();
							particle[j].siz		=	5.f;
							particle[j].tolive	=	Random::get(1500, 3500);
							particle[j].scale.x	=	0.2f;
							particle[j].scale.y	=	0.2f;
							particle[j].scale.z	=	0.2f;
							particle[j].timcreation	=	(long)arxtime;
							particle[j].tc		=	TC_smoke;
							particle[j].special	=	FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;// | SUBSTRACT;
							particle[j].sourceionum	=	0;
							particle[j].source	=	&eobj->vertexlist3[id].v;
							particle[j].fparam	=	0.0000001f;

							if(FistParticles & 2) {
								particle[j].move.y *= 2.f;
								particle[j].rgb = Color3f(1.f - rnd() * .1f, .3f + rnd() * .1f, .2f - rnd() * .1f);
							} else {
								particle[j].rgb = Color3f(.7f - rnd() * .1f, .3f - rnd() * .1f, 1.f - rnd() * .1f);
							}
						}
					}
				}
			}

			goto makechanges;
		}
		else if (ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0)
		{
			ChangeMoveAnim = alist[ANIM_LEVITATE];
			ChangeMA_Loop = 1;
			goto makechanges;
			
		} else if(player.jumpphase != NotJumping) {
			
			switch(player.jumpphase) {
				
				case NotJumping: break;
				
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
					if(ause0->cur_anim == alist[ANIM_JUMP_END_PART2]
					   && (ause0->flags & EA_ANIMEND)) {
						AcquireLastAnim(io);
						player.jumpphase = NotJumping;
						goto retry;
					} else if(ause0->cur_anim == alist[ANIM_JUMP_END_PART2]
					         && EEfabs(player.physics.velocity.x)
					             + EEfabs(player.physics.velocity.z) > 4
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

			if ((ChangeMoveAnim) && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
					ause0->flags |= EA_EXCONTROL;
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}
		else
		{
		makechanges:
			;

			if ((ChangeMoveAnim) && (ChangeMoveAnim != ause0->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause0);
				ause0->cur_anim = ChangeMoveAnim;
				ause0->flags = EA_STATICANIM;

				if (ChangeMA_Loop)		ause0->flags |= EA_LOOP;

				if (ChangeMA_Stopend)	ause0->flags |= EA_STOPEND;

				if ((ChangeMoveAnim == alist[ANIM_U_TURN_LEFT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_RIGHT_FIGHT])
				        ||	(ChangeMoveAnim == alist[ANIM_U_TURN_LEFT_FIGHT]))
					ause0->flags |= EA_EXCONTROL;
			}

			if ((ChangeMoveAnim2) && (ChangeMoveAnim2 != ause3->cur_anim))
			{
				AcquireLastAnim(io);
				ResetAnim(ause3);
				ause3->cur_anim = ChangeMoveAnim2;
				ause3->flags = EA_STATICANIM;
			}
		}

		memcpy(&io->physics, &player.physics, sizeof(IO_PHYSICS));
	}

nochanges:
	;
	player.Last_Movement = player.Current_Movement;

	if (!light)
	{
		if (special[2] != -1)
		{
			DynLight[special[2]].exist = 0;
			special[2] = -1;
		}

		if (special[1] != -1)
		{
			DynLight[special[1]].exist = 0;
			special[1] = -1;
		}
	}
}

// Init Local Player Data
void ARX_PLAYER_InitPlayer() {
	player.Interface = INTER_MINIBOOK | INTER_MINIBACK | INTER_LIFE_MANA;
	player.physics.cyl.height = player.baseHeight();
	player.physics.cyl.radius = player.baseRadius();
	player.life = player.maxlife = player.Full_maxlife = 100.f;
	player.mana = player.maxmana = player.Full_maxmana = 100.f;
	player.falling = 0;
	player.rightIO = NULL;
	player.leftIO = NULL;
	player.equipsecondaryIO = NULL;
	player.equipshieldIO = NULL;
	player.gold = 0;
	player.bag = 1;
	player.doingmagic = 0;
	ARX_PLAYER_MakeFreshHero();
}

// Forces player orientation to look at an IO
void ForcePlayerLookAtIO(Entity * io) {
	
	// Validity Check
	if (!io) return;

	EERIE_CAMERA tcam;
	Vec3f target;

	long id = entities.player()->obj->fastaccess.view_attach;

	if(id != -1) {
		tcam.pos = entities.player()->obj->vertexlist3[id].v;
	} else {
		tcam.pos = player.pos;
	}

	id = io->obj->fastaccess.view_attach;

	if(id != -1) {
		target = io->obj->vertexlist3[id].v;
	}
	else
	{
		target = io->pos;
	}

	// For the case of not already computed Vlist3... !
	if(fartherThan(target, io->pos, 400.f)) {
		target = io->pos;
	}

	SetTargetCamera(&tcam, target.x, target.y, target.z);
	player.desiredangle.a = player.angle.a = MAKEANGLE(-tcam.angle.a);
	player.desiredangle.b = player.angle.b = MAKEANGLE(tcam.angle.b - 180.f);
	player.angle.g = 0;
}
extern float PLAYER_ARMS_FOCAL;
extern long CURRENT_BASE_FOCAL;
extern long TRAP_DETECT;
extern long TRAP_SECRET;


//*************************************************************************************
// void ARX_PLAYER_Frame_Update()
//-------------------------------------------------------------------------------------
// FUNCTION/RESULT:
//   Updates Many player infos each frame...
//*************************************************************************************
void ARX_PLAYER_Frame_Update()
{
	if (ARX_SPELLS_GetSpellOn(entities.player(), SPELL_PARALYSE) >= 0)
	{
		PLAYER_PARALYSED = 1;
	}
	else
	{
		entities.player()->ioflags &= ~IO_FREEZESCRIPT;
		PLAYER_PARALYSED = 0;
	}

	// Reset player moveto info
	moveto.x = player.pos.x;
	moveto.y = player.pos.y;
	moveto.z = player.pos.z;

	// Reset current movement flags
	player.Current_Movement = 0;

	// Updates player angles to desired angles
	player.angle.a = player.desiredangle.a;
	player.angle.b = player.desiredangle.b;

	// Updates player Extra-Rotate Informations
	Entity * io;
	io = entities.player();

	if ((io) && (io->_npcdata->ex_rotate))
	{
		float v = player.angle.a;

		if (v > 160) v = -(360 - v);

		if (player.Interface & INTER_COMBATMODE)
		{
			if (ARX_EQUIPMENT_GetPlayerWeaponType() == WEAPON_BOW)
			{
				io->_npcdata->ex_rotate->group_rotate[0].a = 0; //head
				io->_npcdata->ex_rotate->group_rotate[1].a = 0; //neck
				io->_npcdata->ex_rotate->group_rotate[2].a = 0; //chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v; //belt
			}
			else
			{
				v *= ( 1.0f / 10 ); 
				io->_npcdata->ex_rotate->group_rotate[0].a = v; //head
				io->_npcdata->ex_rotate->group_rotate[1].a = v; //neck
				io->_npcdata->ex_rotate->group_rotate[2].a = v * 4; //chest
				io->_npcdata->ex_rotate->group_rotate[3].a = v * 4; //belt
			}

		}
		else
		{
			v *= ( 1.0f / 4 ); 
			io->_npcdata->ex_rotate->group_rotate[0].a = v; //head
			io->_npcdata->ex_rotate->group_rotate[1].a = v; //neck
			io->_npcdata->ex_rotate->group_rotate[2].a = v; //chest
			io->_npcdata->ex_rotate->group_rotate[3].a = v; //belt*/
		}

		if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic == 2))
			io->_npcdata->ex_rotate->flags &= ~EXTRA_ROTATE_REALISTIC;
	}

	// Changes player ARMS focal depending on alpha angle.
	if (player.angle.a > 180)
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f;
	else
		PLAYER_ARMS_FOCAL = (float)CURRENT_BASE_FOCAL - 80.f + player.angle.a;

	PLAYER_ARMS_FOCAL = static_cast<float>(CURRENT_BASE_FOCAL);

	ARX_PLAYER_ComputePlayerFullStats();

	TRAP_DETECT = checked_range_cast<long>(player.Full_Skill_Mecanism);
	TRAP_SECRET = checked_range_cast<long>(player.Full_Skill_Intuition);

	if (ARX_SPELLS_GetSpellOn(entities.player(), SPELL_DETECT_TRAP) >= 0)
		TRAP_DETECT = 100;

	ModeLight |= MODE_DEPTHCUEING;

	ARX_PLAYER_ManageTorch();
}

// Emit player step noise
void ARX_PLAYER_MakeStepNoise() {
	
	if(ARX_SPELLS_GetSpellOn(entities.player(), SPELL_LEVITATE) >= 0) {
		return;
	}
	
	if(USE_PLAYERCOLLISIONS) {
		
		float volume = ARX_NPC_AUDIBLE_VOLUME_DEFAULT;
		float factor = ARX_NPC_AUDIBLE_FACTOR_DEFAULT;
		
		if(player.Current_Movement & PLAYER_MOVE_STEALTH) {
			float skill_stealth;
			skill_stealth = player.Full_Skill_Stealth / ARX_PLAYER_SKILL_STEALTH_MAX;
			volume -= ARX_NPC_AUDIBLE_VOLUME_RANGE * skill_stealth;
			factor += ARX_NPC_AUDIBLE_FACTOR_RANGE * skill_stealth;
		}
		
		Vec3f pos = player.basePosition();
		ARX_NPC_NeedStepSound(entities.player(), &pos, volume, factor);
	}
	
	while(currentdistance >= STEP_DISTANCE) {
		currentdistance -= STEP_DISTANCE;
	}
}

extern bool bGCroucheToggle;
extern float MAX_ALLOWED_PER_SECOND;

long LAST_FIRM_GROUND = 1;
long TRUE_FIRM_GROUND = 1;
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
	float tmp = CheckAnythingInCylinder(&tmpp, entities.player(),
	                                    CFLAG_PLAYER | CFLAG_JUST_TEST);
	if(tmp <= 20.f) {
		return true;
	}
	
	long hum = 0;
	for(float vv = 0; vv < 360.f; vv += 20.f) {
		tmpp.origin = player.basePosition();
		tmpp.origin += Vec3f(-EEsin(radians(vv)) * 20.f, 0.f, EEcos(radians(vv)) * 20.f);
		tmpp.radius = player.physics.cyl.radius;
		float anything = CheckAnythingInCylinder(&tmpp, entities.player(), CFLAG_JUST_TEST);
		if(anything > 10) {
			hum = 1;
			break;
		}
	}
	if(!hum) {
		return true;
	}
	
	if(COLLIDED_CLIMB_POLY) {
		player.climbing = 1;
		return true;
	}
	
	return (tmp <= 50.f);
}

extern float player_climb;
extern float PLAYER_CLIMB_THRESHOLD;
void PlayerMovementIterate(float DelatTime);

void ARX_PLAYER_Manage_Movement()
{
	// Is our player able to move ?
	if ((CINEMASCOPE)
	        ||	(BLOCK_PLAYER_CONTROLS)
	        ||	(entities.player() == NULL))
		return;

	float speedfactor;
	// Compute current player speedfactor
	speedfactor = entities.player()->basespeed + entities.player()->speed_modif;

	if (speedfactor < 0) speedfactor = 0;

	if (cur_mr == 3) speedfactor += 0.5f;

	if (cur_rf == 3) speedfactor += 1.5f;

	static float StoredTime = 0;
	// Compute time things
	float DeltaTime = StoredTime;
	
	if(Original_framedelay > 0) {
		DeltaTime = StoredTime + (float)Original_framedelay * speedfactor;
	}
	
	if(EDITMODE) {
		DeltaTime = 25.f;
	}
	
	if(player.jumpphase != NotJumping) {
		while(DeltaTime > 25.f) {
			PlayerMovementIterate(DeltaTime);
			DeltaTime -= 25.f;
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
			float anything = CheckAnythingInCylinder(&player.physics.cyl, entities.player(),
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
				ARX_NPC_SpawnAudibleSound(&player.pos, entities.player());
				ARX_SPEECH_Launch_No_Unicode_Seek("player_jump", entities.player());
				player.onfirmground = 0;
				player.jumpphase = JumpStart;
			}
		}
		
	}
	
	if(entities.player()->_npcdata->climb_count != 0.f && FrameDiff > 0) {
		entities.player()->_npcdata->climb_count -= MAX_ALLOWED_PER_SECOND * FrameDiff * 0.1f;
		if(entities.player()->_npcdata->climb_count < 0) {
			entities.player()->_npcdata->climb_count = 0.f;
		}
	}
	
	float d = 0;
	
	if(!EDITMODE && USE_PLAYERCOLLISIONS) {
		
		CollisionFlags levitate = 0;
		if(player.climbing) {
			levitate = CFLAG_LEVITATE;
		}
		
		if(player.levitate) {
			
			if(player.physics.cyl.height != player.levitateHeight()) {
				float old = player.physics.cyl.height;
				player.physics.cyl.height = player.levitateHeight();
				player.physics.cyl.origin = player.basePosition();
				float anything = CheckAnythingInCylinder(&player.physics.cyl, entities.player());
				if(anything < 0.f) {
					player.physics.cyl.height = old;
					long num = ARX_SPELLS_GetSpellOn(entities.player(), SPELL_LEVITATE);
					if(num != -1) {
						spells[num].tolive = 0;
					}
				}
			}
			
			if(player.physics.cyl.height == player.levitateHeight()) {
				levitate = CFLAG_LEVITATE;
				player.climbing = 0;
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
		EERIE_CYLINDER testcyl;
		memcpy(&testcyl, &player.physics.cyl, sizeof(EERIE_CYLINDER));
		testcyl.origin.y += 3.f;
		ON_PLATFORM = 0;
		anything = CheckAnythingInCylinder(&testcyl, entities.player(), 0);
		LAST_ON_PLATFORM = ON_PLATFORM;
	
		if(player.jumpphase != JumpAscending) {
			if(anything >= 0.f) {
				TRUE_FIRM_GROUND = 0;
			} else {
				TRUE_FIRM_GROUND = 1;
				testcyl.radius -= 30.f;
				testcyl.origin.y -= 10.f;
				anything = CheckAnythingInCylinder(&testcyl, entities.player(), 0);
			}
		} else {
			TRUE_FIRM_GROUND = 0;
			LAST_ON_PLATFORM = 0;
		}
		
		EERIE_CYLINDER cyl;
		cyl.origin = player.basePosition() + Vec3f(0.f, 1.f, 0.f);
		cyl.radius = player.physics.cyl.radius;
		cyl.height = player.physics.cyl.height;
		float anything2 = CheckAnythingInCylinder(&cyl, entities.player(), CFLAG_JUST_TEST | CFLAG_PLAYER); //-cyl->origin.y;
		
		if(anything2 > -5 && player.physics.velocity.y > 15.f && !LAST_ON_PLATFORM
		  && !TRUE_FIRM_GROUND && player.jumpphase == NotJumping && !player.levitate
		  && anything > 80.f) {
			player.jumpphase = JumpDescending;
			if(!player.falling) {
				player.falling = 1;
				ARX_PLAYER_StartFall();
			}
		} else if(!player.falling) {
			FALLING_TIME = 0;
		}
		
		if(player.jumpphase != NotJumping && player.levitate) {
			player.jumpphase = NotJumping;
			player.falling = 0;
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
				player.falling = 0;
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
		player.onfirmground = TRUE_FIRM_GROUND;
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
		if(impulse != Vec3f::ZERO) {
			
			float scale = 1.25f / 1000;
			if(entities.player()->animlayer[0].cur_anim) {
				if(player.jumpphase != NotJumping) {
					if(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) {
						scale = 0.5f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD) {
						scale = 5.25f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_STRAFE_LEFT) {
						scale = 1.75f / 1000;
					} else if(player.Current_Movement & PLAYER_MOVE_STRAFE_RIGHT) {
						scale = 1.75f / 1000;
					} else {
						scale = 0.125f / 1000;
					}
				} else if(levitate && !player.climbing) {
					scale = 0.875f / 1000;
				} else {
					Vec3f mv;
					short idx = entities.player()->animlayer[0].altidx_cur;
					GetAnimTotalTranslate(entities.player()->animlayer[0].cur_anim, idx, &mv);
					float time = entities.player()->animlayer[0].cur_anim->anims[idx]->anim_time;
					scale = mv.length() / time * 0.0125f;
				}
			}
			
			impulse *= scale / impulse.length() * jump_mul * DeltaTime;
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
			
			player.physics.forces.y += (player.falling) ? JUMP_GRAVITY : WORLD_GRAVITY;
			
			// Check for LAVA Damage !!!
			float epcentery;
			EERIEPOLY * ep = CheckInPoly(player.pos.x, player.pos.y + 150.f, player.pos.z,
			                             &epcentery);
			if(ep) {
				if((ep->type & POLY_LAVA)
				   && EEfabs(epcentery - (player.pos.y - player.baseHeight())) < 30) {
					float mul = 1.f - (EEfabs(epcentery - (player.pos.y - player.baseHeight()))
					                   * (1.0f / 30));
					const float LAVA_DAMAGE = 10.f;
					float damages = LAVA_DAMAGE * FrameDiff * 0.01f * mul;
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
		player.physics.forces += attraction;
		
		// Apply push player force
		player.physics.forces += PUSH_PLAYER_FORCE;
		PUSH_PLAYER_FORCE = Vec3f::ZERO;
		
		// Apply forces to velocity
		player.physics.velocity += player.physics.forces * DeltaTime;
		
		// Apply climbing velocity
		if(player.climbing) {
			if(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD) {
				player.physics.velocity.y = -0.2f * DeltaTime;
			}
			if(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) {
				player.physics.velocity.y = 0.2f * DeltaTime;
			}
		}
		
		// Removes y velocity if on firm ground...
		if(player.onfirmground == 1 && !player.climbing) {
			player.physics.velocity.y = 0.f;
		}
		
		float posy;
		EERIEPOLY * ep = CheckInPolyPrecis(player.pos.x, player.pos.y, player.pos.z, &posy);
		if(ep == NULL) {
			player.physics.velocity.y = 0;
		} else if(!player.climbing && player.pos.y >= posy) {
			player.physics.velocity.y = 0;
		}
		
		// Reset forces
		player.physics.forces = Vec3f::ZERO;
		
		// Check if player is already on firm ground AND not moving
		if(EEfabs(player.physics.velocity.x) < 0.001f
		   && EEfabs(player.physics.velocity.z) < 0.001f
		   && player.onfirmground == 1 && player.jumpphase == NotJumping) {
			moveto = player.pos;
			goto lasuite;
		} else {
			
			// Need to apply some physics/collision tests
			player.physics.cyl.origin = player.basePosition();
			player.physics.startpos = player.physics.cyl.origin;
			player.physics.targetpos = player.physics.startpos + player.physics.velocity;
			
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
				float position = clamp((float)offset_time * (1.f / jump_up_time), 0.f, 1.f);
				
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
				                                   CFLAG_EASY_SLIDING | CFLAG_CLIMBING
				                                   | CFLAG_PLAYER);
				
				if(!COLLIDED_CLIMB_POLY) {
					player.climbing = 0;
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
				player.climbing = 1;
			}
			
			if(player.climbing) {
				
				if(player.Current_Movement && player.Current_Movement != PLAYER_ROTATE
				   && !(player.Current_Movement & PLAYER_MOVE_WALK_FORWARD)
				   && !(player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD)) {
					player.climbing = 0;
				}
				
				if((player.Current_Movement & PLAYER_MOVE_WALK_BACKWARD) && !test) {
					player.climbing = 0;
				}
				
				if(player.climbing) {
					player.jumpphase = NotJumping;
					player.falling = 0;
					FALLING_TIME = 0;
					Falling_Height = player.pos.y;
				}
			}
			
			if(player.jumpphase == JumpAscending) {
				player.climbing = 0;
			}
			
			moveto = player.physics.cyl.origin + player.baseOffset();
			d = dist(player.pos, moveto);
		}
		
	} else {
		
		if(!EDITMODE) {
			Vec3f vect = moveto - player.pos;
			float divv = vect.length();
			if(divv > 0.f) {
				float mul = (float)FrameDiff * 0.001f * 200.f;
				divv = mul / divv;
				vect *= divv;
				moveto = player.pos + vect;
			}
		}
		
		player.onfirmground = 0;
	}
	
	if(player.pos == moveto) {
		d = 0.f;
	}
	
	// Emit Stepsound
	if(USE_PLAYERCOLLISIONS && !EDITMODE) {
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
	float grnd_color = GetColorz(player.pos.x, player.pos.y + 90, player.pos.z) - 15.f;
	if(CURRENT_PLAYER_COLOR < grnd_color) {
		CURRENT_PLAYER_COLOR += FrameDiff * (1.0f / 8);
		CURRENT_PLAYER_COLOR = std::min(CURRENT_PLAYER_COLOR, grnd_color);
	}
	if(CURRENT_PLAYER_COLOR > grnd_color) {
		CURRENT_PLAYER_COLOR -= FrameDiff * (1.0f / 4);
		CURRENT_PLAYER_COLOR = std::max(CURRENT_PLAYER_COLOR, grnd_color);
	}
	
	if (InventoryDir != 0)
	{
		if ((player.Interface & INTER_COMBATMODE) || (player.doingmagic >= 2) || (InventoryDir == -1))
		{
			if (InventoryX > -160)
				InventoryX -= INTERFACE_RATIO(FrameDiff * ( 1.0f / 3 ));
		}
		else
		{
			if (InventoryX < 0)
				InventoryX += InventoryDir * INTERFACE_RATIO(FrameDiff * ( 1.0f / 3 ));
		}

		if (InventoryX <= -160)
		{
			InventoryX = -160;
			InventoryDir = 0;

			if (player.Interface & INTER_STEAL || ioSteal)
			{
				SendIOScriptEvent(ioSteal, SM_STEAL, "off");
				player.Interface &= ~INTER_STEAL;
				ioSteal = NULL;
			}

			SecondaryInventory = NULL;
			TSecondaryInventory = NULL;
			InventoryDir = 0;
		}
		else if (InventoryX >= 0)
		{
			InventoryX = 0;
			InventoryDir = 0;
		}
	}
}
//******************************************************************************
// Manage Player Death Visual
//******************************************************************************
void ARX_PLAYER_Manage_Death()
{
	PLAYER_PARALYSED = 0;
	float ratio = (float)(DeadTime - 2000) * ( 1.0f / 5000 );

	if (ratio >= 1.f) ratio = 1.f;

	if (ratio == 1.f)
	{
		ARX_MENU_Launch();
		DeadTime = 0;
	}

	{
		GRenderer->SetRenderState(Renderer::AlphaBlending, true);
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
		EERIEDrawBitmap( 0.f, 0.f, static_cast<float>(DANAESIZX), static_cast<float>(DANAESIZY), 0.000091f, NULL, Color::gray(ratio));

	}
}
//******************************************************************************
// Specific for color checks
//******************************************************************************
float GetPlayerStealth()
{
	return 15 + player.Full_Skill_Stealth * ( 1.0f / 10 );
}

// Teleport player to any poly...
void ARX_PLAYER_GotoAnyPoly() {
	for(long j = 0; j < ACTIVEBKG->Zsize; j++) {
		for(long i = 0; i < ACTIVEBKG->Xsize; i++) {
			EERIE_BKG_INFO * eg = &ACTIVEBKG->Backg[i + j * ACTIVEBKG->Xsize];
			if(eg->nbpoly) {
				player.pos.x = moveto.x = eg->polydata[0].center.x;
				player.pos.y = moveto.y = eg->polydata[0].center.y + player.baseHeight();
				player.pos.z = moveto.z = eg->polydata[0].center.z;
			}
		}
	}
}

// Force Player to standard stance... (Need some improvements...)
void ARX_PLAYER_PutPlayerInNormalStance(long val) {
	
	if(player.Current_Movement & PLAYER_CROUCH) {
		player.Current_Movement &= ~PLAYER_CROUCH;
	}
	
	player.Current_Movement = 0;
	ARX_PLAYER_RectifyPosition();
	
	if(player.jumpphase != NotJumping || player.falling) {
		player.physics.cyl.origin = player.basePosition();
		IO_PHYSICS phys;
		memcpy(&phys, &player.physics, sizeof(IO_PHYSICS));
		AttemptValidCylinderPos(&phys.cyl, entities.player(), CFLAG_RETURN_HEIGHT);
		player.pos.y = phys.cyl.origin.y + player.baseHeight();
		player.jumpphase = NotJumping;
		player.falling = 0;
	}
	
	if(player.Interface & INTER_COMBATMODE) {
		player.Interface &= ~INTER_COMBATMODE;
		ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
	}
	
	ARX_SOUND_Stop(SND_MAGIC_DRAW);
	
	if(!val) {
		for(size_t i = 0; i < MAX_SPELLS; i++) {
			if(spells[i].exist && (spells[i].caster == 0 || spells[i].target == 0)) {
				switch(spells[i].type) {
					case SPELL_MAGIC_SIGHT:
					case SPELL_LEVITATE:
					case SPELL_SPEED:
					case SPELL_FLYING_EYE:
						spells[i].tolive = 0;
						break;
					default: break;
				}
			}
		}
	}
}

// Add gold to player purse
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
	CURRENT_TORCH = NULL;
	entities.clear();
	SecondaryInventory = NULL;
	TSecondaryInventory = NULL;
	ARX_EQUIPMENT_UnEquipAllPlayer();
	
	ARX_Changelevel_CurGame_Clear();

	entities.player()->halo.flags = 0;
}

//-----------------------------------------------------------------------------
void ARX_PLAYER_AddBag()
{
	++player.bag;

	if (player.bag > 3)
		player.bag = 3;
}

//-----------------------------------------------------------------------------
bool ARX_PLAYER_CanStealItem(Entity * _io)
{
	if (_io->_itemdata->stealvalue > 0)
		if ((player.Full_Skill_Stealth >= _io->_itemdata->stealvalue)
		        &&	(_io->_itemdata->stealvalue < 100.f))
		{
			return true;
		}

	return false;
}
void ARX_PLAYER_Rune_Add_All()
{
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
extern long sp_wep;
extern long WILL_QUICKLOAD, WILL_QUICKSAVE;
extern long GLOBAL_Player_Room;
extern long cur_mx, cur_pom;
extern long sp_arm, cur_arm;
extern float sp_max_start;

void ARX_PLAYER_Invulnerability(long flag)
{
	if (flag)
		player.playerflags |= PLAYERFLAGS_INVULNERABILITY;
	else
		player.playerflags &= ~PLAYERFLAGS_INVULNERABILITY;
}
extern Entity * FlyingOverIO;
extern long cur_sm;
extern void ClearDynLights();

void ARX_GAME_Reset(long type) {
	
	DeadTime = 0;
	
	if(entities.player()) {
		entities.player()->speed_modif = 0;
	}
	
	LAST_JUMP_ENDTIME = 0;
	FlyingOverIO = NULL;
	ARX_MAPMARKER_Init();
	ClearDynLights();

	if(!DONT_ERASE_PLAYER && entities.player()) {
		entities.player()->halo.flags = 0;
	}

	if(entities.player())entities.player()->gameFlags &= ~GFLAG_INVISIBILITY;
	ARX_PLAYER_Invulnerability(0);
	GLOBAL_Player_Room = -1;
	PLAYER_PARALYSED = 0;

	ARX_PLAYER_Reset_Fall();

	player.levitate = 0;
	Project.telekinesis = 0;
	player.onfirmground = 0;
	TRUE_FIRM_GROUND = 0;
	sp_max_start = 0;
	lastposy = -99999999999.f;

	ioSteal = NULL;

	WILL_QUICKLOAD = 0;
	WILL_QUICKSAVE = 0;
	GLOBAL_SLOWDOWN = 1.f;

	PrecalcIOLighting(NULL, 0, 1);

	sp_arm = 0;
	cur_arm = 0;
	cur_sm = 0;
	sp_wep = 0;
	sp_max = 0;
	cur_mx = 0;
	cur_pom = 0;
	cur_rf = 0;
	cur_mr = 0;


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
	if (!(type & 1))
	{
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
	ARX_CONVERSATION = 0;

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
	ARX_FOGS_TimeReset();
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
	if (type & 1)
	{
		if (CURRENT_TORCH) ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
	}
	else
		CURRENT_TORCH = NULL;

	// Player Quests
	ARX_PLAYER_Quest_Init();

	// Player Keyring
	ARX_KEYRING_Init();

	// Player Init
	if (!DONT_ERASE_PLAYER)
	{
		ARX_MAPMARKER_Init();
		GLOBAL_MAGIC_MODE = 1;

		// Linked Objects
		if (!(type & 2))
		{
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
	BLOCK_PLAYER_CONTROLS = 0;
	HERO_SHOW_1ST = -1;
	PUSH_PLAYER_FORCE = Vec3f::ZERO;
	player.jumplastposition = 0;
	player.jumpstarttime = 0;
	player.jumpphase = NotJumping;
	player.inzone = NULL;

	QuakeFx.intensity = 0.f;
	Project.improve = 0;

	if (eyeball.exist) eyeball.exist = -100;

	if(entities.size() > 0 && entities.player()) {
		entities.player()->ouch_time = 0;
		entities.player()->invisibility = 0.f;
	}

	FADEDIR = 0;
	FADEDURATION = 0;
	FADESTART = 0;
	FADECOLOR = Color3f::black;

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
	player.falling = 0;
}



float sp_max_y[64];
Color sp_max_col[64];
char	sp_max_ch[64];
long sp_max_nb;
void Manage_sp_max()
{
	float v = float(arxtime) - sp_max_start;

	if ((sp_max_start != 0) && (v < 20000))
	{
		float modi = (20000 - v) * ( 1.0f / 2000 ) * ( 1.0f / 10 );
		float sizX = 16;
		float px = (float)DANAECENTERX - (float)sp_max_nb * ( 1.0f / 2 ) * sizX;
		float py = (float)DANAECENTERY;

		for (long i = 0; i < sp_max_nb; i++)
		{
			float dx = px + sizX * (float)i;
			float dy = py + sp_max_y[i];
			sp_max_y[i] = EEsin(dx + (float)float(arxtime) * ( 1.0f / 100 )) * 30.f * modi;
			std::string tex( 1, sp_max_ch[i] );

			UNICODE_ARXDrawTextCenter( hFontInBook, dx - 1, dy - 1, tex, Color::none );
			UNICODE_ARXDrawTextCenter( hFontInBook, dx + 1, dy + 1, tex, Color::none );
			UNICODE_ARXDrawTextCenter( hFontInBook, dx, dy, tex, sp_max_col[i] );

		}
	}
}
