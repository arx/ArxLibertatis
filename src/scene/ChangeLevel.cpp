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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "scene/ChangeLevel.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <limits>
#include <ctime>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "ai/Paths.h"

#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Version.h"

#include "game/Camera.h"
#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/NPC.h"
#include "game/Spells.h"
#include "game/Player.h"
#include "game/Levels.h"
#include "game/Inventory.h"
#include "game/spell/Cheat.h"

#include "gui/LoadLevelScreen.h"
#include "gui/MiniMap.h"
#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/Notification.h"
#include "gui/Speech.h"
#include "gui/hud/SecondaryInventory.h"

#include "graphics/GlobalFog.h"
#include "graphics/Math.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/SaveBlock.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "physics/Physics.h"

#include "platform/Platform.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/SaveFormat.h"
#include "scene/Light.h"

#include "script/Script.h"
#include "script/ScriptEvent.h"

#include "util/String.h"


extern bool GLOBAL_MAGIC_MODE;

extern bool GMOD_RESET;

extern bool EXTERNALVIEW;
extern bool LOAD_N_ERASE;

static bool ARX_CHANGELEVEL_Push_Index(long num);
static bool ARX_CHANGELEVEL_PushLevel(long num, long newnum);
static bool ARX_CHANGELEVEL_PopLevel(long instance, bool reloadflag = false,
                                     const std::string & target = std::string(), float angle = 0.f);
static void ARX_CHANGELEVEL_Push_Globals();
static void ARX_CHANGELEVEL_Pop_Globals();
static long ARX_CHANGELEVEL_Push_Player(long level);
static long ARX_CHANGELEVEL_Push_AllIO(long level);
static long ARX_CHANGELEVEL_Push_IO(const Entity * io, long level);
static Entity * ARX_CHANGELEVEL_Pop_IO(const std::string & idString, EntityInstance instance);

static fs::path CURRENT_GAME_FILE;

struct Playthrough {
	
	std::time_t startTime;
	
	u64 uniqueId;
	
	u64 oldestALVersion;
	u64 newestALVersion;
	
};

long DONT_WANT_PLAYER_INZONE = 0;
static SaveBlock * g_currentSavedGame = NULL;
static Playthrough g_currentPlathrough;

static Entity * convertToValidIO(const std::string & idString) {
	
	if(idString.empty() || idString == "none") {
		return NULL;
	}
	
	arx_assert_msg(
		idString.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789") == std::string::npos,
		"bad interactive object id: \"%s\"", idString.c_str()
	);
	
	EntityHandle t = entities.getById(idString);
	
	if(t.handleData() > 0) {
		arx_assert_msg(ValidIONum(t), "got invalid IO num %ld", long(t.handleData()));
		return entities[t];
	}
	
	LogDebug("Call to ConvertToValidIO(" << idString << ")");
	
	size_t pos = idString.find_last_of('_');
	if(pos == std::string::npos || pos == idString.length() - 1) {
		return NULL;
	}
	pos = idString.find_first_not_of('0', pos + 1);
	if(pos == std::string::npos) {
		return NULL;
	}
	
	EntityInstance instance = -1;
	try {
		instance = boost::lexical_cast<EntityInstance>(idString.substr(pos));
	} catch(...) {
		LogError << "Could not parse entity ID: " << idString;
	}
	
	return ARX_CHANGELEVEL_Pop_IO(idString, instance);
}

template <size_t N>
static Entity * ConvertToValidIO(const char (&str)[N]) {
	return convertToValidIO(boost::to_lower_copy(util::loadString(str)));
}

template <size_t N>
static EntityHandle ReadTargetInfo(const char (&str)[N]) {
	
	std::string idString = boost::to_lower_copy(util::loadString(str));
	
	if(idString == "none") {
		return EntityHandle();
	} else if(idString == "self") {
		return EntityHandle_Self;
	} else if(idString == "player") {
		return EntityHandle_Player;
	} else {
		Entity * e = convertToValidIO(idString);
		return (e == NULL) ? EntityHandle() : e->index();
	}
}

static s32 GetIOAnimIdx2(const Entity * io, ANIM_HANDLE * anim) {
	
	if(!io || !anim) {
		return -1;
	}
	
	for(size_t i = 0; i < MAX_ANIMS; i++) {
		if(io->anims[i] == anim) {
			return s32(i);
		}
	}
	
	return -1;
}

bool ARX_Changelevel_CurGame_Clear() {
	
	if(g_currentSavedGame) {
		delete g_currentSavedGame, g_currentSavedGame = NULL;
	}
	
	if(CURRENT_GAME_FILE.empty()) {
		CURRENT_GAME_FILE = fs::getUserDir() / "current.sav";
	}
	
	// If there's a left over current game file, clear it
	if(fs::is_regular_file(CURRENT_GAME_FILE)) {
		if(!fs::remove(CURRENT_GAME_FILE)) {
			LogError << "Failed to remove current game file " << CURRENT_GAME_FILE;
			return false;
		}
	}
	
	return true;
}

static bool openCurrentSavedGameFile() {
	
	arx_assert(!CURRENT_GAME_FILE.empty());
	
	if(g_currentSavedGame) {
		// Already open...
		return true;
	}
	
	g_currentSavedGame = new SaveBlock(CURRENT_GAME_FILE);
	
	if(!g_currentSavedGame->open(true)) {
		LogError << "Error writing to save block " << CURRENT_GAME_FILE;
		return false;
	}
	
	return true;
}

bool ARX_CHANGELEVEL_StartNew() {
	
	g_currentPlathrough.startTime = std::time(NULL);
	g_currentPlathrough.uniqueId = Random::get(u64(1), std::numeric_limits<u64>::max());
	g_currentPlathrough.oldestALVersion = arx_version_number;
	g_currentPlathrough.newestALVersion = arx_version_number;
	
	g_gameTime.reset(0);
	
	if(!ARX_Changelevel_CurGame_Clear()) {
		return false;
	}
	
	if(!openCurrentSavedGameFile()) {
		return false;
	}
	
	return true;
}

bool currentSavedGameHasEntity(const std::string & idString) {
	if(g_currentSavedGame) {
		return g_currentSavedGame->hasFile(idString);
	} else {
		arx_assert(false);
		return false;
	}
}

void currentSavedGameStoreEntityDeletion(const std::string & idString) {
	
	if(!g_currentSavedGame) {
		arx_assert(false);
		return;
	}
	
	// Save a minimal entity save file containing just enough info so we know not to load it
	ARX_CHANGELEVEL_IO_SAVE ais;
	memset(&ais, 0, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	ais.version = ARX_GAMESAVE_VERSION;
	ais.show = SHOW_FLAG_DESTROYED;
	
	const char * data = reinterpret_cast<const char *>(&ais);
	g_currentSavedGame->save(idString, data, sizeof(ais));
	
}

void currentSavedGameRemoveEntity(const std::string & idString) {
	if(g_currentSavedGame) {
		g_currentSavedGame->remove(idString);
	}
}

void ARX_CHANGELEVEL_Change(const std::string & level, const std::string & target, float angle) {
	
	LogDebug("ARX_CHANGELEVEL_Change " << level << " " << target << " " << angle);
	
	progressBarSetTotal(238);
	progressBarReset();
	
	long num = GetLevelNumByName("level" + level);

	LoadLevelScreen(num);
	
	if(num == -1) {
		// fatality...
		LogWarning << "Internal Non-Fatal Error";
		return;
	}
	
	// not changing level, just teleported
	if(num == CURRENTLEVEL) {
		EntityHandle t = entities.getById(target);
		if(t.handleData() > 0 && entities[t]) {
			Vec3f pos = GetItemWorldPosition(entities[t]);
			g_moveto = player.pos = pos + player.baseOffset();
		}
		player.desiredangle.setYaw(angle);
		player.angle.setYaw(angle);
		return; // nothing more to do :)
	}
	
	ARX_PLAYER_Reset_Fall();
	
	progressBarAdvance();
	LoadLevelScreen(num);
	
	if(!openCurrentSavedGameFile()) {
		return;
	}
	
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, num);
	
	ARX_CHANGELEVEL_PopLevel(num, true, target, angle);
	
	DONT_WANT_PLAYER_INZONE = 1;
	ARX_PLAYER_RectifyPosition();
	GMOD_RESET = true;
	
}

static bool ARX_CHANGELEVEL_PushLevel(long num, long newnum) {
	
	LogDebug("ARX_CHANGELEVEL_PushLevel " << num << " " << newnum);
	
	ARX_SCRIPT_EventStackExecuteAll();
	
	// Close secondary inventory before leaving
	g_secondaryInventoryHud.close();
	
	// Now we can save our things
	if(!ARX_CHANGELEVEL_Push_Index(num)) {
		LogError << "Error saving index...";
		return false;
	}
	
	ARX_CHANGELEVEL_Push_Globals();
	
	if(ARX_CHANGELEVEL_Push_Player(newnum) != 1) {
		LogError << "Error saving player...";
		return false;
	}
	
	if(ARX_CHANGELEVEL_Push_AllIO(num) != 1) {
		LogError << "Error saving entities...";
		return false;
	}
	
	return true;
}

static bool IsPlayerEquipedWith(Entity * io) {
	
	if(!io) {
		return false;
	}
	
	EntityHandle num = io->index();
	
	if(io == player.torch) {
		return true;
	}
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i]) && player.equiped[i] == num) {
			return true;
		}
	}
	
	return false;
}

static bool ARX_CHANGELEVEL_Push_Index(long num) {
	
	size_t pos = 0;
	
	ARX_CHANGELEVEL_INDEX asi;
	memset(&asi, 0, sizeof(asi));
	asi.version       = ARX_GAMESAVE_VERSION;
	asi.nb_inter      = 0;
	asi.nb_paths      = s32(g_paths.size());
	asi.time          = toMsi(g_gameTime.now()); // TODO save/load time
	asi.nb_lights     = 0;
	asi.gmods_stacked = GLOBAL_MODS();
	asi.gmods_stacked.zclip = 6400.f;
	asi.gmods_desired = g_desiredFogParameters;
	asi.gmods_current = g_currentFogParameters;
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e != NULL
		   && !(e->ioflags & IO_NOSAVE)
		   && !IsInPlayerInventory(e)
		   && !IsPlayerEquipedWith(e)) {
			asi.nb_inter++;
		}
	}
	
	for(size_t i = 0; i < g_staticLightsMax; i++) {
		EERIE_LIGHT * el = g_staticLights[i];
		if(el && !el->m_isIgnitionLight) {
			asi.nb_lights++;
		}
	}
	
	size_t allocsize = sizeof(ARX_CHANGELEVEL_INDEX)
	                   + sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi.nb_inter
	                   + sizeof(ARX_CHANGELEVEL_PATH) * asi.nb_paths
	                   + sizeof(ARX_CHANGELEVEL_LIGHT) * asi.nb_lights;
	
	std::string playlist = ARX_SOUND_AmbianceSavePlayList();
	allocsize += playlist.size();
	
	std::vector<char> buffer(allocsize);
	char * dat = &buffer[0];
	
	asi.ambiances_data_size = playlist.size();
	memcpy(dat, &asi, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e != NULL
		   && !(e->ioflags & IO_NOSAVE)
		   && !IsInPlayerInventory(e)
		   && !IsPlayerEquipedWith(e)) {
			ARX_CHANGELEVEL_IO_INDEX aii;
			memset(&aii, 0, sizeof(aii));
			util::storeString(aii.filename, (e->classPath() + ".teo").string());
			aii.ident = e->instance();
			aii.level = num;
			aii.truelevel = num;
			aii.num = i; // !!!
			memcpy(dat + pos, &aii, sizeof(aii));
			pos += sizeof(aii);
		}
	}
	
	BOOST_FOREACH(const ARX_PATH * path, g_paths) {
		ARX_CHANGELEVEL_PATH * acp = reinterpret_cast<ARX_CHANGELEVEL_PATH *>(dat + pos);
		memset(acp, 0, sizeof(ARX_CHANGELEVEL_PATH));
		util::storeString(acp->name, path->name);
		util::storeString(acp->controled, path->controled);
		pos += sizeof(ARX_CHANGELEVEL_PATH);
	}
	
	memcpy(dat + pos, playlist.data(), playlist.size());
	pos += playlist.size();
	
	for(size_t i = 0; i < g_staticLightsMax; i++) {
		EERIE_LIGHT * el = g_staticLights[i];
		if(el != NULL && !el->m_isIgnitionLight) {
			ARX_CHANGELEVEL_LIGHT * acl = reinterpret_cast<ARX_CHANGELEVEL_LIGHT *>(dat + pos);
			memset(acl, 0, sizeof(ARX_CHANGELEVEL_LIGHT));
			acl->status = el->m_ignitionStatus;
			pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		}
	}
	
	arx_assert(pos <= allocsize);
	
	std::stringstream savefile;
	savefile << "lvl" << std::setfill('0') << std::setw(3) << num;
	bool ret = g_currentSavedGame->save(savefile.str(), dat, pos);
	
	return ret;
}

static VariableType getVariableType(const std::string & name) {
	
	switch(name.c_str()[0]) {
		case '$':    return TYPE_G_TEXT;
		case '\xA3': return TYPE_L_TEXT;
		case '#':    return TYPE_G_LONG;
		case '\xA7': return TYPE_L_LONG;
		case '&':    return TYPE_G_FLOAT;
		case '@':    return TYPE_L_FLOAT;
		default: return TYPE_UNKNOWN;
	}
	
}

static size_t getScriptVariableSaveSize(const SCRIPT_VARIABLES & variables) {
	
	size_t size = 0;
	
	for(size_t i = 0; i < variables.size(); i++) {
		size += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		VariableType type = getVariableType(variables[i].name);
		if(type == TYPE_G_TEXT || type == TYPE_L_TEXT) {
			size += size_t(float(variables[i].text.size() + 1));
		}
	}
	
	return size;
}

static void storeScriptVariables(char * dat, size_t & pos, const SCRIPT_VARIABLES & variables, size_t diff) {
	
	for(size_t i = 0; i < variables.size(); i++) {
		
		ARX_CHANGELEVEL_VARIABLE_SAVE * avs = reinterpret_cast<ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
		memset(avs, 0, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		
		util::storeStringTerminated(avs->name, variables[i].name);
		
		avs->type = getVariableType(variables[i].name);
		
		switch(avs->type) {
			
			case TYPE_G_TEXT:
			case TYPE_L_TEXT: {
				avs->fval = float(variables[i].text.size() + diff);
				arx_assert(size_t(avs->fval) == variables[i].text.size() + diff);
				break;
			}
			
			case TYPE_G_LONG:
			case TYPE_L_LONG: {
				avs->fval = float(variables[i].ival);
				break;
			}
			
			case TYPE_G_FLOAT:
			case TYPE_L_FLOAT: {
				avs->fval = variables[i].fval;
				break;
			}
			
			default: arx_unreachable();
			
		}
		
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		
		if(avs->type == TYPE_G_TEXT || avs->type == TYPE_L_TEXT) {
			memcpy(dat + pos, variables[i].text.c_str(), variables[i].text.size() + 1);
			pos += size_t(avs->fval);
		}
		
	}
	
}

static void ARX_CHANGELEVEL_Push_Globals() {
	
	ARX_CHANGELEVEL_SAVE_GLOBALS acsg;
	
	memset(&acsg, 0, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	acsg.nb_globals = svar.size();
	acsg.version = ARX_GAMESAVE_VERSION;
	
	size_t allocsize = sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS) + getScriptVariableSaveSize(svar);
	
	std::vector<char> buffer(allocsize);
	char * dat = &buffer[0];
	size_t pos = 0;
	
	memcpy(dat, &acsg, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	
	storeScriptVariables(dat, pos, svar, 0);
	
	arx_assert(pos <= buffer.size());
	
	g_currentSavedGame->save("globals", dat, pos);
	
}

template <size_t N>
static void storeIdString(char (&tofill)[N], const Entity * io) {
	
	if(!io || !ValidIOAddress(io)) {
		ARX_STATIC_ASSERT(N >= 4, "id string too short");
		util::storeString(tofill, "none");
	} else {
		
		std::string idString = io->idString();
		
		arx_assert(idString.length() <= N);
		util::storeString(tofill, idString);
	}
}

static long ARX_CHANGELEVEL_Push_Player(long level) {
	
	ARX_CHANGELEVEL_PLAYER * asp;

	long allocsize = sizeof(ARX_CHANGELEVEL_PLAYER) + 48000;
	allocsize += g_playerKeyring.size() * SAVED_KEYRING_SLOT_SIZE;
	allocsize += SAVED_QUEST_SLOT_SIZE * g_playerQuestLogEntries.size();
	allocsize += sizeof(SavedMapMarkerData) * g_miniMap.mapMarkerCount();
	
	std::vector<char> buffer(allocsize);
	char * dat = &buffer[0];
	
	asp = reinterpret_cast<ARX_CHANGELEVEL_PLAYER *>(dat);
	
	long pos = 0;
	pos += (sizeof(ARX_CHANGELEVEL_PLAYER));

	memset(asp, 0, sizeof(ARX_CHANGELEVEL_PLAYER));

	asp->AimTime = 1500;
	asp->angle = player.angle;
	asp->Attribute_Constitution = player.m_attribute.constitution;
	asp->Attribute_Dexterity = player.m_attribute.dexterity;
	asp->Attribute_Mind = player.m_attribute.mind;
	asp->Attribute_Strength = player.m_attribute.strength;
	asp->Current_Movement = player.m_currentMovement;
	asp->doingmagic = player.doingmagic;
	asp->Interface = player.Interface;
	asp->playerflags = player.playerflags;
	
	util::storeString(asp->TELEPORT_TO_LEVEL, TELEPORT_TO_LEVEL);
	util::storeString(asp->TELEPORT_TO_POSITION, TELEPORT_TO_POSITION);
	
	asp->TELEPORT_TO_ANGLE = TELEPORT_TO_ANGLE;
	switch(CHANGE_LEVEL_ICON) {
		case NoChangeLevel:      asp->CHANGE_LEVEL_ICON = -1; break;
		case ConfirmChangeLevel: asp->CHANGE_LEVEL_ICON = 1;  break;
		case ChangeLevelNow:     asp->CHANGE_LEVEL_ICON = 200; break;
	}
	asp->bag = player.m_bags;
	storeIdString(asp->equipsecondaryIO, NULL);
	storeIdString(asp->equipshieldIO, NULL);
	storeIdString(asp->leftIO, NULL);
	storeIdString(asp->rightIO, NULL);
	storeIdString(asp->curtorch, player.torch);
	
	for(size_t i = 0; i < SAVED_MAX_PRECAST; i++) {
		asp->precast[i].typ = SPELL_NONE;
		
		if(i < Precast.size()) {
			asp->precast[i] = Precast[i];
		}
	}
	
	for(size_t bag = 0; bag < INVENTORY_BAGS; bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		storeIdString(asp->id_inventory[bag][x][y], g_inventory[bag][x][y].io);
		asp->inventory_show[bag][x][y] = g_inventory[bag][x][y].show;
	}
	
	g_miniMap.save(asp->minimap, SAVED_MAX_MINIMAPS);
	
	asp->falling = player.falling;
	asp->gold = player.gold;
	asp->invisibility = entities.player()->invisibility;

	asp->jumpphase = player.jumpphase;
	
	GameInstant jumpstart = g_gameTime.now() + GameDurationUs(toUs(player.jumpstarttime - g_platformTime.frameStart()));
	asp->jumpstarttime = static_cast<u32>(toMsi(jumpstart)); // TODO save/load time
	asp->Last_Movement = player.m_lastMovement;
	asp->level = player.level;
	
	asp->life = player.lifePool.current;
	asp->mana = player.manaPool.current;
	asp->maxlife = player.lifePool.max;
	asp->maxmana = player.manaPool.max;
	
	asp->misc_flags = 0;

	if (player.onfirmground)
		asp->misc_flags |= 1;

	if (WILLRETURNTOCOMBATMODE)
		asp->misc_flags |= 2;

	asp->physics = player.physics;
	asp->poison = player.poison;
	asp->hunger = player.hunger;

	asp->sp_flags = 0;

	if (sp_arm == 1)
		asp->sp_flags |= SP_ARM1;

	if (sp_arm == 2)
		asp->sp_flags |= SP_ARM2;

	if (sp_arm == 3)
		asp->sp_flags |= SP_ARM3;

	if (sp_max)
		asp->sp_flags |= SP_MAX;

	if (cur_mr == 3)
		asp->sp_flags |= SP_MR;

	if (cur_rf == 3)
		asp->sp_flags |= SP_RF;

	if (sp_wep)
		asp->sp_flags |= SP_WEP;


	asp->pos = player.pos;
	asp->Attribute_Redistribute = player.Attribute_Redistribute;
	asp->Skill_Redistribute = player.Skill_Redistribute;
	asp->rune_flags = player.rune_flags;
	asp->size = player.size;
	asp->Skill_Stealth = player.m_skill.stealth;
	asp->Skill_Mecanism = player.m_skill.mecanism;
	asp->Skill_Intuition = player.m_skill.intuition;
	asp->Skill_Etheral_Link = player.m_skill.etheralLink;
	asp->Skill_Object_Knowledge = player.m_skill.objectKnowledge;
	asp->Skill_Casting = player.m_skill.casting;
	asp->Skill_Projectile = player.m_skill.projectile;
	asp->Skill_Close_Combat = player.m_skill.closeCombat;
	asp->Skill_Defense = player.m_skill.defense;
	asp->skin = player.skin;
	
	asp->xp = player.xp;
	asp->nb_PlayerQuest = g_playerQuestLogEntries.size();
	asp->keyring_nb = g_playerKeyring.size();
	asp->Global_Magic_Mode = GLOBAL_MAGIC_MODE;
	asp->Nb_Mapmarkers = g_miniMap.mapMarkerCount();
	
	asp->LAST_VALID_POS = LastValidPlayerPos;
	
	for(size_t i = 0; i < MAX_ANIMS; i++) {
		memset(&asp->anims[i], 0, 256);
		if(entities.player()->anims[i] != NULL) {
			util::storeString(asp->anims[i], entities.player()->anims[i]->path.string());
		}
	}
	
	for(size_t k = 0; k < MAX_EQUIPED; k++) {
		if(ValidIONum(player.equiped[k])) {
			storeIdString(asp->equiped[k], entities[player.equiped[k]]);
		} else {
			util::storeString(asp->equiped[k], std::string());
		}
	}
	
	for(size_t i = 0; i < g_playerQuestLogEntries.size(); i++) {
		memset(dat + pos, 0, SAVED_QUEST_SLOT_SIZE);
		assert(g_playerQuestLogEntries[i].length() < SAVED_QUEST_SLOT_SIZE);
		util::storeString(dat + pos, SAVED_QUEST_SLOT_SIZE, g_playerQuestLogEntries[i]);
		pos += SAVED_QUEST_SLOT_SIZE;
	}
	
	for(size_t i = 0; i < g_playerKeyring.size(); i++) {
		memset(dat + pos, 0, SAVED_KEYRING_SLOT_SIZE);
		assert(g_playerKeyring[i].length() < SAVED_KEYRING_SLOT_SIZE);
		util::storeString(dat + pos, SAVED_KEYRING_SLOT_SIZE, g_playerKeyring[i]);
		pos += SAVED_KEYRING_SLOT_SIZE;
	}
	
	for(size_t i = 0; i < g_miniMap.mapMarkerCount(); i++) {
		SavedMapMarkerData * acmd = reinterpret_cast<SavedMapMarkerData *>(dat + pos);
		*acmd = g_miniMap.mapMarkerGet(i);
		pos += sizeof(SavedMapMarkerData);
	}
	
	LastValidPlayerPos = asp->LAST_VALID_POS.toVec3();
	
	g_currentSavedGame->save("player", dat, pos);
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(IsInPlayerInventory(e) || IsPlayerEquipedWith(e)) {
			ARX_CHANGELEVEL_Push_IO(e, level);
		}
	}

	return 1;
}

static long ARX_CHANGELEVEL_Push_AllIO(long level) {
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		if(e && !(e->ioflags & IO_NOSAVE) && !IsInPlayerInventory(e) && !IsPlayerEquipedWith(e)) {
			ARX_CHANGELEVEL_Push_IO(e, level);
		}
	}
	
	return 1;
}

static Entity * GetObjIOSource(const EERIE_3DOBJ * obj) {
	
	if(!obj) {
		return NULL;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->obj) {
			if(e->obj == obj) {
				return e;
			}
		}
	}
	
	return NULL;
}

template <size_t N>
void FillTargetInfo(char (&info)[N], EntityHandle numtarget) {
	ARX_STATIC_ASSERT(N >= 6, "id string too short");
	if(numtarget == EntityHandle_Self) {
		util::storeString(info, "self");
	} else if(numtarget == EntityHandle()) {
		util::storeString(info, "none");
	} else if(numtarget == EntityHandle_Player) {
		util::storeString(info, "player");
	} else if(ValidIONum(numtarget)) {
		storeIdString(info, entities[numtarget]);
	} else {
		util::storeString(info, "none");
	}
}

static long ARX_CHANGELEVEL_Push_IO(const Entity * io, long level) {
	
	// Check Valid IO
	if(!io) {
		return -1;
	}
	
	arx_assert(io->show != SHOW_FLAG_DESTROYED);
	arx_assert(io->show != SHOW_FLAG_KILLED);
	
	// Sets Savefile Name
	std::string savefile = io->idString();
	
	// Define Type & Affiliated Structure Size
	SavedIOType type;
	long struct_size = 0;
	if(io->ioflags & IO_NPC) {
		type = TYPE_NPC;
		struct_size = sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
	} else if(io->ioflags & IO_ITEM) {
		type = TYPE_ITEM;
		struct_size = sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
	} else if(io->ioflags & IO_FIX) {
		type = TYPE_FIX;
		struct_size = sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
	} else if(io->ioflags & IO_CAMERA) {
		type = TYPE_CAMERA;
		struct_size = sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
	} else {
		type = TYPE_MARKER;
		struct_size = sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
	}
	
	// Init Changelevel Main IO Save Structure
	ARX_CHANGELEVEL_IO_SAVE ais;
	memset(&ais, 0, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	
	// Store IO Data in Main IO Save Structure
	ais.version = ARX_GAMESAVE_VERSION;
	ais.savesystem_type = s32(type);
	ais.saveflags = 0;
	util::storeString(ais.filename, (io->classPath() + ".teo").string());
	ais.ident = io->instance();
	ais.ioflags = io->ioflags;
	
	if(   (ais.ioflags & IO_FREEZESCRIPT)
	   && (spells.getSpellOnTarget(io->index(), SPELL_PARALYSE)
	    || spells.getSpellOnTarget(io->index(), SPELL_MASS_PARALYSE))
	) {
		ais.ioflags &= ~IO_FREEZESCRIPT;
	}

	ais.pos = io->pos;

	if(io->obj && io->obj->pbox && io->obj->pbox->active) {
		ais.pos.y -= io->obj->pbox->vert[0].initpos.y;
	}
	
	ais.lastpos = io->lastpos;
	ais.initpos = io->initpos;
	ais.initangle = io->initangle;
	ais.move = io->move;
	ais.lastmove = io->lastmove;
	ais.angle = io->angle;
	ais.scale = io->scale;
	ais.weight = io->weight;
	util::storeString(ais.locname, io->locname);
	ais.gameFlags = io->gameFlags;
	
	if(io == entities.player())
		ais.gameFlags &= ~GFLAG_INVISIBILITY;
	
	ais.material = io->material;
	ais.level = ais.truelevel = level;
	ais.scriptload = io->scriptload;
	ais.show = io->show;
	ais.collision = io->collision;
	if(io->mainevent != SM_MAIN) {
		util::storeString(ais.mainevent, io->mainevent.toString());
	}
	ais.velocity = Vec3f(0.f);
	ais.stopped = 0;
	ais.basespeed = io->basespeed;
	ais.speed_modif = io->speed_modif;
	ais.frameloss = 0.f;
	ais.rubber = io->rubber;
	ais.max_durability = io->max_durability;
	ais.durability = io->durability;
	ais.poisonous = io->poisonous;
	ais.poisonous_count = io->poisonous_count;
	ais.head_rot = io->head_rot;
	ais.damager_damages = io->damager_damages;
	ais.nb_iogroups = io->groups.size();
	ais.damager_type = io->damager_type;
	ais.type_flags = io->type_flags;
	ais.secretvalue = io->secretvalue;
	ais.shop_multiply = io->shop_multiply;
	ais.aflags = io->isHit ? 1 : 0;
	ais.original_height = io->original_height;
	ais.original_radius = io->original_radius;
	ais.ignition = io->ignition;
	
	if(io->usepath) {
		ARX_USE_PATH * aup = io->usepath;
		ais.usepath_aupflags = aup->aupflags;
		ais.usepath_curtime = checked_range_cast<u32>(toMsi(aup->_curtime)); // TODO save/load time
		ais.usepath_initpos = aup->initpos;
		ais.usepath_lastWP = aup->lastWP;
		ais.usepath_starttime = checked_range_cast<u32>(toMsi(aup->_starttime)); // TODO save/load time
		util::storeString(ais.usepath_name, aup->path->name);
	}
	
	util::storeString(ais.shop_category, io->shop_category);
	util::storeString(ais.inventory_skin, io->inventory_skin.string());
	util::storeString(ais.stepmaterial, io->stepmaterial);
	util::storeString(ais.armormaterial, io->armormaterial);
	util::storeString(ais.weaponmaterial, io->weaponmaterial);
	util::storeString(ais.strikespeech, io->strikespeech);
	
	ais.nb_linked = 0;
	memset(&ais.linked_data, 0, sizeof(IO_LINKED_DATA) * MAX_LINKED_SAVE);
	
	// Save Animations
	for(size_t i = 0; i < MAX_ANIMS; i++) {
		memset(&ais.anims[i], 0, 256);
		if (io->anims[i] != NULL) {
			util::storeString(ais.anims[i], io->anims[i]->path.string());
		}
	}
	
	// Save Linked Objects
	if(io->obj) {
		ais.nb_linked = 0;

		for(size_t n = 0; n < io->obj->linked.size(); n++) {
			if(GetObjIOSource(io->obj->linked[n].obj)) {
				ais.nb_linked++;
			}
		}

		if(size_t(ais.nb_linked) > MAX_LINKED_SAVE) {
			ais.nb_linked = MAX_LINKED_SAVE;
		}

		long count = 0;

		for(size_t n = 0; n < io->obj->linked.size(); n++) {
			if(GetObjIOSource(io->obj->linked[n].obj)) {
				ais.linked_data[count].lgroup = io->obj->linked[count].lgroup.handleData();
				ais.linked_data[count].lidx = io->obj->linked[count].lidx.handleData();
				ais.linked_data[count].lidx2 = io->obj->linked[count].lidx2.handleData();
				ais.linked_data[count].modinfo = SavedModInfo();
				storeIdString(ais.linked_data[count].linked_id, GetObjIOSource(io->obj->linked[count].obj));
				count++;
			}
		}
	}


	if(io->tweakerinfo) {
		ais.system_flags = SYSTEM_FLAG_TWEAKER_INFO;
	} else {
		ais.system_flags = 0;
	}

	if(io->inventory) {
		ais.system_flags |= SYSTEM_FLAG_INVENTORY;
	}

	if(type == TYPE_ITEM) {
		if(io->_itemdata->equipitem) {
			ais.system_flags |= SYSTEM_FLAG_EQUIPITEMDATA;
		}
	}

	if(io->usepath) {
		ais.system_flags |= SYSTEM_FLAG_USEPATH;
	}

	ais.physics = io->physics;
	ais.spellcast_data = io->spellcast_data;
	assert(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
	std::copy(io->animlayer, io->animlayer + SAVED_MAX_ANIM_LAYERS, ais.animlayer);
	for(size_t k = 0; k < MAX_ANIM_LAYERS; k++) {
		ais.animlayer[k].cur_anim = GetIOAnimIdx2(io, io->animlayer[k].cur_anim);
	}

	// Save Target Infos
	EntityHandle numtarget = io->targetinfo;

	if ((io->ioflags & IO_NPC) && (io->_npcdata->pathfind.listnb > 0) && ValidIONum(io->_npcdata->pathfind.truetarget))
	{
		numtarget = io->_npcdata->pathfind.truetarget;
	}

	FillTargetInfo(ais.id_targetinfo, numtarget);
	
	// Save Local Timers ?
	ais.nbtimers = 0;
	BOOST_FOREACH(const SCR_TIMER & timer, g_scriptTimers) {
		if(timer.exist && timer.io == io) {
			ais.nbtimers++;
		}
	}
	
	size_t allocsize =
		sizeof(ARX_CHANGELEVEL_IO_SAVE)
		+ sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE)
		+ sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE)
		+ getScriptVariableSaveSize(io->m_variables)
		+ struct_size
		+ sizeof(SavedTweakerInfo)
		+ sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE) + 1024
		+ sizeof(SavedGroupData) * io->groups.size()
		+ sizeof(SavedTweakInfo) * io->tweaks.size()
		+ 48000;

	// Allocate Main Save Buffer
	std::vector<char> buffer(allocsize);
	char * dat = &buffer[0];
	size_t pos = 0;

	ais.halo = io->halo_native;
	ais.Tweak_nb = io->tweaks.size();
	memcpy(dat, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	BOOST_FOREACH(const SCR_TIMER & timer, g_scriptTimers) {
		if(timer.exist) {
			if(timer.io == io) {
				ARX_CHANGELEVEL_TIMERS_SAVE * ats = reinterpret_cast<ARX_CHANGELEVEL_TIMERS_SAVE *>(dat + pos);
				memset(ats, 0, sizeof(ARX_CHANGELEVEL_TIMERS_SAVE));
				ats->interval = s32(toMsi(timer.interval)); // TODO save/load time
				util::storeString(ats->name, timer.name);
				ats->pos = s32(timer.pos);
				ats->script = (timer.es == &io->script) ? 0 : 1;
				ats->remaining = s32(toMsi((timer.start + timer.interval) - g_gameTime.now())); // TODO save/load time
				arx_assert(ats->remaining <= ats->interval);
				if(ats->remaining < 0) {
					ats->remaining = 0;
				}
				ats->count = s32(timer.count);
				ats->flags = timer.idle ? 1 : 0;
				pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
			}
		}
	}
	
	if(!io->script.valid) {
		ARX_CHANGELEVEL_SCRIPT_SAVE * ass = reinterpret_cast<ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		ass->allowevents = 0;
		ass->lastcall = 0;
		ass->nblvar = 0;
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	}
	
	{
		ARX_CHANGELEVEL_SCRIPT_SAVE * ass = reinterpret_cast<ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		ass->allowevents = io->m_disabledEvents;
		ass->lastcall = 0;
		ass->nblvar = io->m_variables.size();
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
		storeScriptVariables(dat, pos, io->m_variables, 1);
	}
	
	if(io->script.valid) {
		ARX_CHANGELEVEL_SCRIPT_SAVE * ass = reinterpret_cast<ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		ass->allowevents = 0;
		ass->lastcall = 0;
		ass->nblvar = 0;
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	}
	
	switch(type) {
		
		case TYPE_NPC: {
			
			ARX_CHANGELEVEL_NPC_IO_SAVE * as = reinterpret_cast<ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
			
			memset(as, 0, sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE));
			
			as->absorb = io->_npcdata->absorb;
			as->aimtime = static_cast<f32>(toMsi(io->_npcdata->aimtime)); // TODO save/load time
			as->armor_class = io->_npcdata->armor_class;
			as->behavior = io->_npcdata->behavior;
			as->behavior_param = io->_npcdata->behavior_param;
			as->collid_state = 0;
			as->collid_time = 0;
			as->cut = io->_npcdata->cut;
			as->damages = io->_npcdata->damages;
			as->detect = io->_npcdata->detect;
			as->fightdecision = io->_npcdata->fightdecision;

			if(io->_npcdata->lifePool.current > 0.f) {
				storeIdString(as->id_weapon, io->_npcdata->weapon);
			} else {
				as->id_weapon[0] = 0;
			}

			as->lastmouth = io->_npcdata->lastmouth;
			as->look_around_inc = io->_npcdata->look_around_inc;
			
			as->life = io->_npcdata->lifePool.current;
			as->mana = io->_npcdata->manaPool.current;
			as->maxlife = io->_npcdata->lifePool.max;
			as->maxmana = io->_npcdata->manaPool.max;
			
			as->movemode = io->_npcdata->movemode;
			as->moveproblem = io->_npcdata->moveproblem;
			as->reachedtarget = io->_npcdata->reachedtarget;
			as->speakpitch = io->_npcdata->speakpitch;
			as->tohit = io->_npcdata->tohit;
			as->weaponinhand = io->_npcdata->weaponinhand;
			as->weapontype = io->_npcdata->weapontype;
			as->xpvalue = io->_npcdata->xpvalue;
			
			assert(SAVED_MAX_STACKED_BEHAVIOR == MAX_STACKED_BEHAVIOR);
			std::copy(io->_npcdata->stacked, io->_npcdata->stacked + SAVED_MAX_STACKED_BEHAVIOR, as->stacked);
			
			for (size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
				if(io->_npcdata->stacked[i].exist) {
					FillTargetInfo(as->stackedtarget[i], io->_npcdata->stacked[i].target);
				} else {
					util::storeString(as->stackedtarget[i], "none");
				}
			}
			
			as->critical = io->_npcdata->critical;
			as->reach = io->_npcdata->reach;
			as->backstab_skill = io->_npcdata->backstab_skill;
			as->poisonned = io->_npcdata->poisonned;
			as->resist_poison = io->_npcdata->resist_poison;
			as->resist_magic = io->_npcdata->resist_magic;
			as->resist_fire = io->_npcdata->resist_fire;
			as->walk_start_time = checked_range_cast<s16>(toMsi(io->_npcdata->walk_start_time)); // TODO save/load time
			as->aiming_start = static_cast<s32>(toMsi(io->_npcdata->aiming_start)); // TODO save/load time
			as->npcflags = io->_npcdata->npcflags;
			as->pathfind = IO_PATHFIND();
			
			if(io->_npcdata->pathfind.listnb > 0) {
				as->pathfind.truetarget = io->_npcdata->pathfind.truetarget.handleData();
			} else {
				as->pathfind.truetarget = -1;
			}
			
			if(io->_npcdata->ex_rotate) {
				ais.saveflags |= SAVEFLAGS_EXTRA_ROTATE;
				as->ex_rotate = *io->_npcdata->ex_rotate;
			}
			
			as->blood_color = io->_npcdata->blood_color.toBGRA().t;
			as->fDetect = io->_npcdata->fDetect;
			as->cuts = io->_npcdata->cuts;
			
			break;
		}
		
		case TYPE_ITEM: {
			
			ARX_CHANGELEVEL_ITEM_IO_SAVE * ai = reinterpret_cast<ARX_CHANGELEVEL_ITEM_IO_SAVE *>(dat + pos);
			
			memset(ai, 0, sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE));
			
			ai->price = io->_itemdata->price;
			ai->count = io->_itemdata->count;
			ai->maxcount = io->_itemdata->maxcount;
			ai->food_value = io->_itemdata->food_value;
			ai->stealvalue = io->_itemdata->stealvalue;
			ai->playerstacksize = io->_itemdata->playerstacksize;
			ai->LightValue = io->_itemdata->LightValue;
			
			if(io->_itemdata->equipitem) {
				ai->equipitem = *io->_itemdata->equipitem;
			}
			
			break;
		}
		
		case TYPE_FIX: {
			ARX_CHANGELEVEL_FIX_IO_SAVE * af = reinterpret_cast<ARX_CHANGELEVEL_FIX_IO_SAVE *>(dat + pos);
			memset(af, 0, sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE));
			af->trapvalue = io->_fixdata->trapvalue;
			break;
		}
		
		case TYPE_CAMERA: {
			ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac = reinterpret_cast<ARX_CHANGELEVEL_CAMERA_IO_SAVE *>(dat + pos);
			*ac = *io->_camdata;
			break;
		}
		
		case TYPE_MARKER: {
			ARX_CHANGELEVEL_MARKER_IO_SAVE * am = reinterpret_cast<ARX_CHANGELEVEL_MARKER_IO_SAVE *>(dat + pos);
			am->dummy = 0;
			break;
		}
		
	}
	
	pos += struct_size;
	
	if(ais.system_flags & SYSTEM_FLAG_INVENTORY) {
		
		ARX_CHANGELEVEL_INVENTORY_DATA_SAVE * aids;
		aids = reinterpret_cast<ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
		
		memset(aids, 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));
		
		INVENTORY_DATA * inv = io->inventory;
		storeIdString(aids->io, inv->io);
		aids->sizex = inv->m_size.x;
		aids->sizey = inv->m_size.y;
		
		for(long x = 0; x < aids->sizex; x++)
		for(long y = 0; y < aids->sizey; y++) {
			aids->initio[x][y][0] = 0;
			
			if(inv->slot[x][y].io)
				storeIdString(aids->slot_io[x][y], inv->slot[x][y].io);
			else
				aids->slot_io[x][y][0] = 0;
			
			aids->slot_show[x][y] = inv->slot[x][y].show;
		}
		
		pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
	}
	
	if(io->tweakerinfo) {
		SavedTweakerInfo sti = *io->tweakerinfo;
		memcpy(dat + pos, &sti, sizeof(SavedTweakerInfo));
		pos += sizeof(SavedTweakerInfo);
	}
	
	for(std::set<std::string>::const_iterator i = io->groups.begin(); i != io->groups.end(); ++i) {
		SavedGroupData * sgd = reinterpret_cast<SavedGroupData *>(dat + pos);
		pos += sizeof(SavedGroupData);
		util::storeString(sgd->name, *i);
	}
	
	for(std::vector<TWEAK_INFO>::const_iterator i = io->tweaks.begin(); i != io->tweaks.end(); ++i) {
		SavedTweakInfo * sti = reinterpret_cast<SavedTweakInfo *>(dat + pos);
		pos += sizeof(SavedTweakInfo);
		*sti = *i;
	}
	
	arx_assert(pos <= buffer.size());
	
	g_currentSavedGame->save(savefile, dat, pos);
	
	return 1;
}

static const ARX_CHANGELEVEL_INDEX * ARX_CHANGELEVEL_Pop_Index(const std::string & buffer) {
	
	const char * dat = buffer.data();
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_INDEX * asi = reinterpret_cast<const ARX_CHANGELEVEL_INDEX *>(dat);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	// Skip entity list and path info (used later !)
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	pos += sizeof(ARX_CHANGELEVEL_PATH) * asi->nb_paths;
	
	// Restore Ambiances
	if(asi->ambiances_data_size) {
		ARX_SOUND_AmbianceRestorePlayList(dat + pos, size_t(asi->ambiances_data_size));
		pos += size_t(asi->ambiances_data_size);
	}
	
	ARX_UNUSED(pos);
	arx_assert(pos <= buffer.size());
	
	return asi;
}

static void ARX_CHANGELEVEL_Pop_Zones_n_Lights(const std::string & buffer) {
	
	const char * dat = buffer.data();
	size_t pos = 0;
	
	// Skip Changelevel Index
	const ARX_CHANGELEVEL_INDEX * asi = reinterpret_cast<const ARX_CHANGELEVEL_INDEX *>(dat);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	// Skip Inter idx
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	
	// Now Restore Paths
	for(int i = 0; i < asi->nb_paths; i++) {
		
		const ARX_CHANGELEVEL_PATH * acp = reinterpret_cast<const ARX_CHANGELEVEL_PATH *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_PATH);
		
		ARX_PATH * ap = ARX_PATH_GetAddressByName(boost::to_lower_copy(util::loadString(acp->name)));
		
		if(ap) {
			ap->controled = boost::to_lower_copy(util::loadString(acp->controled));
		}
	}
	
	if(asi->ambiances_data_size > 0) {
		pos += asi->ambiances_data_size;
	}
	
	for(int i = 0; i < asi->nb_lights; i++) {
		
		const ARX_CHANGELEVEL_LIGHT * acl = reinterpret_cast<const ARX_CHANGELEVEL_LIGHT *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		
		long count = 0;

		for(size_t j = 0; j < g_staticLightsMax; j++) {
			EERIE_LIGHT * el = g_staticLights[j];
			if(el && !el->m_isIgnitionLight) {
				if(count == i) {
					el->m_ignitionStatus = (acl->status != 0);
					break;
				}
				count++;
			}
		}
	}
	
}

static long ARX_CHANGELEVEL_Pop_Level(long num, bool firstTime) {
	
	const char * levelId = GetLevelNameByNum(num);
	std::string levelFile = std::string("graph/levels/level") + levelId + "/level" + levelId + ".dlf";
	
	LOAD_N_ERASE = false;
	
	if(!g_resources->getFile(levelFile)) {
		LogError << "Unable To Find " << levelFile;
		return 0;
	}
	
	LoadLevelScreen(num);
	SetEditMode();
	ARX_PATH_ComputeAllBoundingBoxes();
	
	ARX_CHANGELEVEL_Pop_Globals();
	
	DanaeLoadLevel(levelFile, firstTime);
	CleanScriptLoadedIO();
	
	if(firstTime) {
		
		RestoreInitialIOStatus();
		
		for(size_t i = 1; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * e = entities[handle];
			
			if(e && !e->scriptload) {
				ARX_SCRIPT_Reset(e, true);
			}
		}
	}
	
	BLOCK_PLAYER_CONTROLS = false;
	ARX_INTERFACE_Reset();
	EERIE_ANIMMANAGER_PurgeUnused();
	
	return 1;
}

static long ARX_CHANGELEVEL_Pop_Player(const std::string & target, float angle) {
	
	std::string buffer = g_currentSavedGame->load("player");
	if(buffer.empty()) {
		LogError << "Unable to read player";
		return -1;
	}
	
	if(buffer.size() < sizeof(ARX_CHANGELEVEL_PLAYER)) {
		LogError << "Truncated data";
		return -1;
	}
	
	const char * dat = buffer.data();
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_PLAYER * asp = reinterpret_cast<const ARX_CHANGELEVEL_PLAYER *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_PLAYER);
	
	if(target.empty()) {
		player.angle = asp->angle;
		player.pos = asp->pos.toVec3();
	} else {
		if(Entity * targetEntity = entities.getById(target, NULL)) {
			player.pos = GetItemWorldPosition(targetEntity) + player.baseOffset();
		}
		player.desiredangle.setYaw(angle);
		player.angle.setYaw(angle);
	}
	
	g_moveto = player.pos;
	entities.player()->pos = player.basePosition();
	player.desiredangle = player.angle;
	
	player.m_attribute.constitution = asp->Attribute_Constitution;
	player.m_attribute.dexterity = asp->Attribute_Dexterity;
	player.m_attribute.mind = asp->Attribute_Mind;
	player.m_attribute.strength = asp->Attribute_Strength;
	player.m_currentMovement = PlayerMovement::load(asp->Current_Movement); // TODO save/load flags
	player.doingmagic = asp->doingmagic;
	player.playerflags = PlayerFlags::load(asp->playerflags); // TODO save/load flags
	
	if(asp->TELEPORT_TO_LEVEL[0]) {
		TELEPORT_TO_LEVEL = boost::to_lower_copy(util::loadString(asp->TELEPORT_TO_LEVEL));
	} else {
		TELEPORT_TO_LEVEL.clear();
	}
	
	if(asp->TELEPORT_TO_POSITION[0]) {
		TELEPORT_TO_POSITION = boost::to_lower_copy(util::loadString(asp->TELEPORT_TO_POSITION));
	} else {
		TELEPORT_TO_POSITION.clear();
	}
	
	TELEPORT_TO_ANGLE = asp->TELEPORT_TO_ANGLE;
	
	if(asp->CHANGE_LEVEL_ICON < 0) {
		CHANGE_LEVEL_ICON = NoChangeLevel;
	} else if(asp->CHANGE_LEVEL_ICON == 200) {
		CHANGE_LEVEL_ICON = ChangeLevelNow;
	} else {
		CHANGE_LEVEL_ICON = ConfirmChangeLevel;
	}
	
	player.m_bags = asp->bag;
	
	for(size_t i = 0; i < SAVED_MAX_PRECAST; i++) {
		PRECAST_STRUCT precastSlot = asp->precast[i];
		
		if(precastSlot.typ == SPELL_NONE)
			continue;
		
		Precast.push_back(precastSlot);
	}
	
	player.Interface = PlayerInterfaceFlags::load(asp->Interface); // TODO save/load flags
	player.Interface &= ~INTER_PLAYERBOOK;
	player.falling = (asp->falling != 0);
	player.gold = asp->gold;
	entities.player()->invisibility = asp->invisibility;
	player.inzone = ARX_PATH_GetAddressByName(boost::to_lower_copy(util::loadString(asp->inzone)));
	player.jumpphase = JumpPhase(asp->jumpphase); // TODO save/load enum
	PlatformInstant jumpstart = g_platformTime.frameStart()
	                            + PlatformDurationUs(toUs(GameInstantMs(asp->jumpstarttime) - g_gameTime.now()));
	player.jumpstarttime = jumpstart; // TODO save/load time
	player.m_lastMovement = PlayerMovement::load(asp->Last_Movement); // TODO save/load flags
	
	player.level = checked_range_cast<short>(asp->level);
	
	player.lifePool.current = asp->life;
	player.manaPool.current = asp->mana;
	player.lifePool.max = asp->maxlife;
	player.manaPool.max = asp->maxmana;
	
	player.onfirmground = (asp->misc_flags & 1) != 0;
	WILLRETURNTOCOMBATMODE = (asp->misc_flags & 2) != 0;
	
	player.physics = asp->physics;
	player.poison = asp->poison;
	player.hunger = std::min(asp->hunger, 100.f);
	
	if(asp->sp_flags & SP_ARM1) {
		sp_arm = 1;
	} else if(asp->sp_flags & SP_ARM2) {
		sp_arm = 2;
	} else if(asp->sp_flags & SP_ARM3) {
		sp_arm = 3;
	} else {
		sp_arm = 0;
	}
	
	if(asp->sp_flags & SP_MAX) {
		cur_mx = 3;
		sp_max = 1;
	} else {
		cur_mx = 0;
		sp_max = 0;
	}
	
	cur_mr = (asp->sp_flags & SP_MR) ? 3 : 0;
	cur_rf = (asp->sp_flags & SP_RF) ? 3 : 0;
	
	if(asp->sp_flags & SP_WEP) {
		cur_pom = 3;
		sp_wep = 1;
	} else {
		cur_pom = 0;
		sp_wep = 0;
	}
	
	player.Attribute_Redistribute = glm::clamp(asp->Attribute_Redistribute, s16(0), s16(std::numeric_limits<unsigned char>::max()));
	player.Skill_Redistribute = glm::clamp(asp->Skill_Redistribute, s16(0), s16(std::numeric_limits<unsigned char>::max()));
	
	player.rune_flags = RuneFlags::load(asp->rune_flags); // TODO save/load flags
	player.size = asp->size.toVec3();
	player.m_skill.stealth = asp->Skill_Stealth;
	player.m_skill.mecanism = asp->Skill_Mecanism;
	player.m_skill.intuition = asp->Skill_Intuition;
	player.m_skill.etheralLink = asp->Skill_Etheral_Link;
	player.m_skill.objectKnowledge = asp->Skill_Object_Knowledge;
	player.m_skill.casting = asp->Skill_Casting;
	player.m_skill.projectile = asp->Skill_Projectile;
	player.m_skill.closeCombat = asp->Skill_Close_Combat;
	player.m_skill.defense = asp->Skill_Defense;
	
	player.skin = checked_range_cast<char>(asp->skin);
	
	player.xp = asp->xp;
	GLOBAL_MAGIC_MODE = (asp->Global_Magic_Mode != 0);
	
	g_miniMap.purgeTexContainer();
	assert(SAVED_MAX_MINIMAPS == MAX_MINIMAP_LEVELS);
	g_miniMap.load(asp->minimap, SAVED_MAX_MINIMAPS);
	
	Entity & io = *entities.player();
	assert(SAVED_MAX_ANIMS == MAX_ANIMS);
	for(size_t i = 0; i < SAVED_MAX_ANIMS; i++) {
		if(io.anims[i] != NULL) {
			ReleaseAnimFromIO(&io, i);
		}
		if(asp->anims[i][0]) {
			io.anims[i] = EERIE_ANIMMANAGER_Load(res::path::load(util::loadString(asp->anims[i])));
		}
	}
	
	assert(SAVED_INVENTORY_BAGS == INVENTORY_BAGS);
	assert(SAVED_INVENTORY_Y == INVENTORY_Y);
	assert(SAVED_INVENTORY_X == INVENTORY_X);
	
	for(size_t bag = 0; bag < SAVED_INVENTORY_BAGS; bag++)
	for(size_t y = 0; y < SAVED_INVENTORY_Y; y++)
	for(size_t x = 0; x < SAVED_INVENTORY_X; x++) {
		g_inventory[bag][x][y].io = ConvertToValidIO(asp->id_inventory[bag][x][y]);
		g_inventory[bag][x][y].show = asp->inventory_show[bag][x][y] != 0;
	}
	
	if(buffer.size() < pos + (asp->nb_PlayerQuest * SAVED_QUEST_SLOT_SIZE)) {
		LogError << "Truncated data";
		return -1;
	}
	ARX_PLAYER_Quest_Init();
	for(int i = 0; i < asp->nb_PlayerQuest; i++) {
		ARX_PLAYER_Quest_Add(script::loadUnlocalized(boost::to_lower_copy(util::loadString(dat + pos, SAVED_QUEST_SLOT_SIZE))));
		pos += SAVED_QUEST_SLOT_SIZE;
	}
	
	if(buffer.size() < pos + (asp->keyring_nb * SAVED_KEYRING_SLOT_SIZE)) {
		LogError << "Truncated data";
		return -1;
	}
	ARX_KEYRING_Init();
	for(int i = 0; i < asp->keyring_nb; i++) {
		ARX_KEYRING_Add(boost::to_lower_copy(util::loadString(dat + pos, SAVED_KEYRING_SLOT_SIZE)));
		pos += SAVED_KEYRING_SLOT_SIZE;
	}
	
	if(buffer.size() < pos + (asp->Nb_Mapmarkers * sizeof(SavedMapMarkerData))) {
		LogError << "Truncated data";
		return -1;
	}
	g_miniMap.mapMarkerInit(asp->Nb_Mapmarkers);
	for(int i = 0; i < asp->Nb_Mapmarkers; i++) {
		const SavedMapMarkerData * acmd = reinterpret_cast<const SavedMapMarkerData *>(dat + pos);
		pos += sizeof(SavedMapMarkerData);
		g_miniMap.mapMarkerAdd(Vec2f(acmd->x, acmd->y), acmd->lvl, script::loadUnlocalized(boost::to_lower_copy(util::loadString(acmd->name))));
	}
	
	ARX_PLAYER_Restore_Skin();
	
	ARX_PLAYER_Modify_XP(0);
	
	progressBarAdvance(10.f);
	LoadLevelScreen();
	
	// Restoring Player equipment...
	player.torch = ConvertToValidIO(asp->curtorch);
	progressBarAdvance();
	LoadLevelScreen();
	
	assert(SAVED_MAX_EQUIPED == MAX_EQUIPED);
	for(size_t i = 0; i < SAVED_MAX_EQUIPED; i++) {
		Entity * e = ConvertToValidIO(asp->equiped[i]);
		player.equiped[i] = (e == NULL) ? EntityHandle() : e->index();
		if(!ValidIONum(player.equiped[i])) {
			player.equiped[i] = EntityHandle();
		}
	}
	
	progressBarAdvance(2.f);
	LoadLevelScreen();
	
	return 1;
}

static bool loadScriptVariables(SCRIPT_VARIABLES & var, const char * dat, size_t & pos) {
	
	for(size_t i = 0; i < var.size(); i++) {
		
		const ARX_CHANGELEVEL_VARIABLE_SAVE * avs;
		avs = reinterpret_cast<const ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		
		var[i].name = boost::to_lower_copy(util::loadString(avs->name));
		
		VariableType type = getVariableType(var[i].name);
		if(type == TYPE_UNKNOWN || type != VariableType(avs->type)) {
			LogError << "Unexpected variable type: \"" << var[i].name << "\" vs. " << avs->type;
			var.resize(i);
			return false;
		}
		
		if(var[i].name.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789", 1) != std::string::npos) {
			LogWarning << "Unexpected variable name \"" << var[i].name << '"';
		}
		
		var[i].fval = avs->fval;
		var[i].ival = long(avs->fval);
		
		if(type == TYPE_G_TEXT || type == TYPE_L_TEXT) {
			if(var[i].ival) {
				var[i].text = boost::to_lower_copy(util::loadString(dat + pos, long(avs->fval)));
				pos += long(avs->fval);
			}
		}
		
		LogDebug(((type & (TYPE_G_TEXT | TYPE_G_LONG | TYPE_G_FLOAT)) ? "global " : "local ")
		          << ((type & (TYPE_L_TEXT | TYPE_G_TEXT)) ? "text"
		              : (type & (TYPE_L_LONG | TYPE_G_LONG)) ? "long"
		              : (type & (TYPE_L_FLOAT | TYPE_G_FLOAT)) ? "float" : "unknown")
		          << " \"" << var[i].name << "\" = " << var[i].fval << ' ' << var[i].text);
		
	}
	
	return true;
}

static Entity * ARX_CHANGELEVEL_Pop_IO(const std::string & idString, EntityInstance instance) {
	
	LogDebug("--> loading interactive object " << idString);
	
	std::string buffer = g_currentSavedGame->load(idString);
	if(buffer.empty()) {
		LogError << "Unable to read " << idString;
		return NULL;
	}
	
	const char * dat = buffer.data();
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_IO_SAVE * ais = reinterpret_cast<const ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	if(ais->version != ARX_GAMESAVE_VERSION) {
		LogError << "Invalid PopIO version " << ais->version;
		return NULL;
	}
	
	if(ais->ioflags & IO_NOSAVE) {
		// This item should not have been saved, yet here it is :(
		LogWarning << "Tried to load entity that should never have been saved: " << idString;
		return NULL;
	}
	
	if(ais->show == SHOW_FLAG_DESTROYED || ais->show == SHOW_FLAG_KILLED) {
		// Not supposed to happen anymore, but older saves have these (this is harmless bloat)
		LogWarning << "Found destroyed entity " << idString << " in save file";
		return NULL;
	}
	
	std::string path = util::loadString(ais->filename);
	if((!path.empty() && path[0] == '\\')
	   || (path.length() >= 3 && isalpha(path[0]) && path[1] == ':'
		     && path[2] == '\\')) {
		// Old save files stored absolute paths,
		// strip everything before 'graph' when loading.
		boost::to_lower(path);
		size_t graphPos = path.find("graph");
		if(graphPos != std::string::npos) {
			path = path.substr(graphPos);
		}
	}
	
	// TODO when we bump the save version, remove the ext in the save file
	// if no class names contain dots, we might get away without changing the ver
	res::path classPath = res::path::load(path).remove_ext();
	Entity * io = LoadInter_Ex(classPath, instance, ais->pos.toVec3(), ais->angle);
	
	if(!io) {
		LogError << "CHANGELEVEL Error: Unable to load " << idString;
	} else {
		
		// The current entity should be visible at this point to preven infinite loops while resolving
		// related entities.
		arx_assert(entities.getById(idString) == io->index());
		
		io->requestRoomUpdate = true;
		io->room = -1;
		io->no_collide = EntityHandle();
		io->ioflags = EntityFlags::load(ais->ioflags); // TODO save/load flags
		
		io->ioflags &= ~IO_FREEZESCRIPT;
		
		io->initpos = ais->initpos.toVec3();
		arx_assert(isallfinite(io->initpos));
		
		io->pos = ais->pos.toVec3();
		if(!isallfinite(io->pos)) {
			LogWarning << "Found bad entity pos in " << io->idString();
			io->pos = io->initpos;
		}
		
		io->lastpos = ais->lastpos.toVec3();
		if(!isallfinite(io->lastpos)) {
			LogWarning << "Found bad entity lastpos in " << io->idString();
			io->lastpos = io->initpos;
		}
		
		io->move = ais->move.toVec3();
		if(!isallfinite(io->move)) {
			LogWarning << "Found bad entity move in " << io->idString();
			io->move = Vec3f(0.f);
		}

		io->lastmove = ais->lastmove.toVec3();
		if(!isallfinite(io->lastmove)) {
			LogWarning << "Found bad entity lastmove in " << io->idString();
			io->lastmove = Vec3f(0.f);
		}
		
		io->initangle = ais->initangle;
		io->angle = ais->angle;
		io->scale = ais->scale;
		io->weight = ais->weight;
		io->locname = script::loadUnlocalized(boost::to_lower_copy(util::loadString(ais->locname)));
		io->gameFlags = GameFlags::load(ais->gameFlags); // TODO save/load flags
		io->material = Material(ais->material); // TODO save/load enum
		
		// Script data
		io->scriptload = ais->scriptload;
		io->show = EntityVisilibity(ais->show); // TODO save/load enum
		io->collision = IOCollisionFlags::load(ais->collision); // TODO save/load flags
		io->mainevent = ScriptEventName::parse(boost::to_lower_copy(util::loadString(ais->mainevent)));
		if(io->mainevent == SM_NULL && io->mainevent.getName().empty()) {
			io->mainevent = SM_MAIN;
		}
		
		// Physics data
		io->basespeed = 1;
		io->speed_modif = 0.f; // TODO why are these not loaded from the savegame?
		io->rubber = ais->rubber;
		io->max_durability = ais->max_durability;
		io->durability = ais->durability;
		io->poisonous = ais->poisonous;
		io->poisonous_count = ais->poisonous_count;
		io->head_rot = ais->head_rot;
		io->damager_damages = ais->damager_damages;
		size_t nb_iogroups = ais->nb_iogroups;
		io->damager_type = DamageType::load(ais->damager_type); // TODO save/load flags
		io->type_flags = ItemType::load(ais->type_flags); // TODO save/load flags
		io->secretvalue = ais->secretvalue;
		io->shop_multiply = ais->shop_multiply;
		io->isHit = (ais->aflags & 1);
		io->original_height = ais->original_height;
		io->original_radius = ais->original_radius;
		io->ignition = ais->ignition;
		
		if(ais->system_flags & SYSTEM_FLAG_USEPATH) {
			ARX_USE_PATH * aup = io->usepath = new ARX_USE_PATH;
			aup->aupflags = UsePathFlags::load(ais->usepath_aupflags); // TODO save/load flags
			aup->_curtime = GameInstantMs(ais->usepath_curtime); // TODO save/load time
			aup->initpos = ais->usepath_initpos.toVec3();
			aup->lastWP = ais->usepath_lastWP;
			aup->_starttime = GameInstantMs(ais->usepath_starttime); // TODO save/load time
			aup->path = ARX_PATH_GetAddressByName(boost::to_lower_copy(util::loadString(ais->usepath_name)));
		}
		
		io->shop_category = boost::to_lower_copy(util::loadString(ais->shop_category));
		
		io->halo_native = ais->halo;
		ARX_HALO_SetToNative(io);
		
		io->inventory_skin = res::path::load(util::loadString(ais->inventory_skin));
		io->stepmaterial = boost::to_lower_copy(util::loadString(ais->stepmaterial));
		io->armormaterial = boost::to_lower_copy(util::loadString(ais->armormaterial));
		io->weaponmaterial = boost::to_lower_copy(util::loadString(ais->weaponmaterial));
		io->strikespeech = script::loadUnlocalized(boost::to_lower_copy(util::loadString(ais->strikespeech)));
		
		for(size_t i = 0; i < MAX_ANIMS; i++) {
			
			if(io->anims[i] != NULL) {
				ReleaseAnimFromIO(io, i);
			}
			
			if(ais->anims[i][0] == '\0') {
				continue;
			}
			
			res::path animPath = res::path::load(util::loadString(ais->anims[i]));
			
			io->anims[i] = EERIE_ANIMMANAGER_Load(animPath);
			if(io->anims[i]) {
				continue;
			}
			
			if(io->ioflags & IO_NPC) {
				animPath = res::path("graph/obj3d/anims/npc") / animPath.filename();
			} else {
				animPath = res::path("graph/obj3d/anims/fix_inter") / animPath.filename();
			}
			
			io->anims[i] = EERIE_ANIMMANAGER_Load(animPath);
			if(!io->anims[i]) {
				LogWarning << "Error loading animation " << animPath;
			}
			
		}
		
		io->spellcast_data = ais->spellcast_data;
		io->physics = ais->physics;
		
		if(!isallfinite(io->physics.cyl.origin)) {
			LogWarning << "Found bad entity physics.cyl.origin in " << io->idString();
			io->physics.cyl.origin = io->initpos;
		}
		
		assert(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
		std::copy(ais->animlayer, ais->animlayer + SAVED_MAX_ANIM_LAYERS, io->animlayer);
		
		for(size_t k = 0; k < MAX_ANIM_LAYERS; k++) {
			AnimLayer & layer = io->animlayer[k];
			
			s32 nn = ais->animlayer[k].cur_anim;
			if(nn == -1) {
				layer.cur_anim = NULL;
			} else {
				layer.cur_anim = io->anims[nn];
				if(layer.cur_anim && layer.altidx_cur >= layer.cur_anim->anims.size()) {
					LogWarning << "Out of bounds animation alternative index " << layer.altidx_cur << " for " << layer.cur_anim->path << ", resetting to 0";
					layer.altidx_cur = 0;
				}
			}
			
			arx_assert(ais->animlayer[k].next_anim == -1);
		}
		
		// Target Info
		io->targetinfo = ReadTargetInfo(ais->id_targetinfo);
		
		ARX_SCRIPT_Timer_Clear_For_IO(io);
		
		for(int i = 0; i < ais->nbtimers; i++) {
			
			const ARX_CHANGELEVEL_TIMERS_SAVE * ats;
			ats = reinterpret_cast<const ARX_CHANGELEVEL_TIMERS_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
			
			SCR_TIMER & timer = createScriptTimer(io, boost::to_lower_copy(util::loadString(ats->name)));
			
			if(timer.name == "_r_a_t_") {
				timer.es = NULL;
			} else {
				timer.es = ats->script ? &io->over_script : &io->script;
			}
			
			timer.idle = (ats->flags & 1) != 0;
			timer.interval = GameDurationMs(ats->interval); // TODO save/load time
			timer.pos = size_t(ats->pos);
			// TODO if the script has changed since the last save, this position may be invalid
			
			GameDuration remaining = GameDurationMs(ats->remaining);
			if(remaining > GameDurationMs(ats->interval)) {
				LogWarning << "Found bad script timer " << timer.name
				           << " for entity " << io->idString() << ": remaining time ("
				           << toMsi(remaining) << "ms) > interval (" << ats->interval << "ms) " << ats->flags;
				remaining = GameDurationMs(ats->interval);
			}
			
			const GameInstant tt = (g_gameTime.now() + remaining) - GameDurationMs(ats->interval);
			timer.start = tt;
			
			timer.count = ats->count;
			
		}
		
		io->m_variables.clear();
		
		bool scriptLoaded = true;
		
		// TODO store m_scriptTimers
		
		const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
		ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
		
		io->m_disabledEvents = DisabledEvents::load(ass->allowevents); // TODO save/load flags
		
		if(ass->nblvar != 0) {
			io->m_variables.resize(ass->nblvar);
			scriptLoaded = loadScriptVariables(io->m_variables, dat, pos);
		}
		
		const ARX_CHANGELEVEL_SCRIPT_SAVE * over_ass;
		over_ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
		
		DisabledEvents over_allowevents = DisabledEvents::load(over_ass->allowevents); // TODO save/load flags
		if(over_allowevents != 0) {
			LogWarning << "Unexpected allowevents for entity " << io->idString() << ": "
			           << io->m_disabledEvents << ' ' << over_allowevents;
			io->m_disabledEvents |= over_allowevents;
		}
		
		if(over_ass->nblvar != 0) {
			LogWarning << "Unexpected override script variables for entity " << idString;
			SCRIPT_VARIABLES variables;
			variables.resize(over_ass->nblvar);
			loadScriptVariables(variables, dat, pos);
			io->m_variables.insert(io->m_variables.end(), variables.begin(), variables.end());
		}
		
		if(!scriptLoaded) {
			LogError << "Save file is corrupted, trying to fix " << idString;
			RestoreInitialIOStatusOfIO(io);
			SendInitScriptEvent(io);
			return io;
		}
		
		switch (ais->savesystem_type) {
			
			case TYPE_NPC: {
				
				const ARX_CHANGELEVEL_NPC_IO_SAVE * as;
				as = reinterpret_cast<const ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
				pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
				
				io->_npcdata->absorb = as->absorb;
				io->_npcdata->aimtime = GameDurationMsf(as->aimtime);
				io->_npcdata->armor_class = as->armor_class;
				io->_npcdata->behavior = Behaviour::load(as->behavior); // TODO save/load flags
				io->_npcdata->behavior_param = as->behavior_param;
				io->_npcdata->cut = as->cut;
				io->_npcdata->damages = as->damages;
				io->_npcdata->detect = as->detect;
				io->_npcdata->fightdecision = as->fightdecision;
				
				io->_npcdata->weapon = ConvertToValidIO(as->id_weapon);
				
				io->_npcdata->lastmouth = as->lastmouth;
				io->_npcdata->look_around_inc = as->look_around_inc;
				
				io->_npcdata->lifePool.current = as->life;
				io->_npcdata->manaPool.current = as->mana;
				io->_npcdata->lifePool.max = as->maxlife;
				io->_npcdata->manaPool.max = as->maxmana;
				
				io->_npcdata->movemode = MoveMode(as->movemode); // TODO save/load enum
				io->_npcdata->moveproblem = as->moveproblem;
				io->_npcdata->reachedtarget = as->reachedtarget;
				io->_npcdata->speakpitch = as->speakpitch;
				io->_npcdata->tohit = as->tohit;
				io->_npcdata->weaponinhand = as->weaponinhand;
				io->_npcdata->weapontype = ItemType::load(as->weapontype); // TODO save/load flags
				io->_npcdata->xpvalue = as->xpvalue;
				
				assert(SAVED_MAX_STACKED_BEHAVIOR == MAX_STACKED_BEHAVIOR);
				std::copy(as->stacked, as->stacked + SAVED_MAX_STACKED_BEHAVIOR, io->_npcdata->stacked);
				// TODO properly load stacked animations
				
				for(size_t iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++) {
					io->_npcdata->stacked[iii].target = ReadTargetInfo(as->stackedtarget[iii]);
				}
				
				io->_npcdata->critical = as->critical;
				io->_npcdata->reach = as->reach;
				io->_npcdata->backstab_skill = as->backstab_skill;
				io->_npcdata->poisonned = as->poisonned;
				io->_npcdata->resist_poison = as->resist_poison;
				io->_npcdata->resist_magic = as->resist_magic;
				io->_npcdata->resist_fire = as->resist_fire;
				io->_npcdata->walk_start_time = GameDurationMs(as->walk_start_time); // TODO save/load time
				io->_npcdata->aiming_start = GameInstantMs(as->aiming_start); // TODO save/load time
				io->_npcdata->npcflags = NPCFlags::load(as->npcflags); // TODO save/load flags
				io->_npcdata->fDetect = as->fDetect;
				io->_npcdata->cuts = DismembermentFlags::load(as->cuts); // TODO save/load flags
				
				io->_npcdata->pathfind = IO_PATHFIND();
				
				io->_npcdata->pathfind.truetarget = EntityHandle(as->pathfind.truetarget);
				
				if(ais->saveflags & SAVEFLAGS_EXTRA_ROTATE) {
					if(io->_npcdata->ex_rotate == NULL) {
						io->_npcdata->ex_rotate = new EERIE_EXTRA_ROTATE();
					}
					*io->_npcdata->ex_rotate = as->ex_rotate;
				}
				
				io->_npcdata->blood_color = Color::fromBGRA(ColorBGRA(as->blood_color));
				
				break;
			}
			
			case TYPE_ITEM: {
				
				const ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
				ai = reinterpret_cast<const ARX_CHANGELEVEL_ITEM_IO_SAVE *>(dat + pos);
				pos += sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
				
				io->_itemdata->price = ai->price;
				io->_itemdata->count = ai->count;
				io->_itemdata->maxcount = ai->maxcount;
				io->_itemdata->food_value = ai->food_value;
				io->_itemdata->stealvalue = ai->stealvalue;
				io->_itemdata->playerstacksize = ai->playerstacksize;
				io->_itemdata->LightValue = ai->LightValue;
				
				delete io->_itemdata->equipitem;
				io->_itemdata->equipitem = NULL;
				
				if(ais->system_flags & SYSTEM_FLAG_EQUIPITEMDATA) {
					io->_itemdata->equipitem = new IO_EQUIPITEM;
					*io->_itemdata->equipitem = ai->equipitem;
				}
				
				break;
			}
			
			case TYPE_FIX: {
				const ARX_CHANGELEVEL_FIX_IO_SAVE * af;
				af = reinterpret_cast<const ARX_CHANGELEVEL_FIX_IO_SAVE *>(dat + pos);
				pos += sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
				io->_fixdata->trapvalue = af->trapvalue;
				break;
			}
			
			case TYPE_CAMERA: {
				const ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
				ac = reinterpret_cast<const ARX_CHANGELEVEL_CAMERA_IO_SAVE *>(dat + pos);
				pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
				*io->_camdata = *ac;
				break;
			}
			
			case TYPE_MARKER: {
				pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
				break;
			}
		}
		
		arx_assert(io->inventory == NULL);
		if(ais->system_flags & SYSTEM_FLAG_INVENTORY) {
			
			const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE * aids
				= reinterpret_cast<const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
			
			io->inventory = new INVENTORY_DATA();
			
			INVENTORY_DATA * inv = io->inventory;
			inv->io = ConvertToValidIO(aids->io);
			
			inv->m_size = Vec2s(3, 11);
			if(aids->sizex != 3 || aids->sizey != 11) {
				for(long x = 0; x < inv->m_size.x; x++)
				for(long y = 0; y < inv->m_size.y; y++) {
					inv->slot[x][y].io = NULL;
					inv->slot[x][y].show = false;
				}
			} else {
				for(long x = 0; x < inv->m_size.x; x++)
				for(long y = 0; y < inv->m_size.y; y++) {
					inv->slot[x][y].io = ConvertToValidIO(aids->slot_io[x][y]);
					inv->slot[x][y].show = aids->slot_show[x][y] != 0;
				}
			}
			
		}
		
		if(ais->system_flags & SYSTEM_FLAG_TWEAKER_INFO) {
			
			if(!io->tweakerinfo) {
				io->tweakerinfo = new IO_TWEAKER_INFO;
			}
			
			const SavedTweakerInfo * sti = reinterpret_cast<const SavedTweakerInfo *>(dat + pos);
			pos += sizeof(SavedTweakerInfo);
			
			io->tweakerinfo->filename = res::path::load(util::loadString(sti->filename));
			io->tweakerinfo->skintochange = boost::to_lower_copy(util::loadString(sti->skintochange));
			io->tweakerinfo->skinchangeto = res::path::load(util::loadString(sti->skinchangeto));
		}
		
		io->groups.clear();
		for(size_t i = 0; i < nb_iogroups; i++) {
			const SavedGroupData * sgd = reinterpret_cast<const SavedGroupData *>(dat + pos);
			pos += sizeof(SavedGroupData);
			io->groups.insert(boost::to_lower_copy(util::loadString(sgd->name)));
		}
		
		io->tweaks.resize(ais->Tweak_nb);
		for(size_t i = 0; i < io->tweaks.size(); i++) {
			const SavedTweakInfo * sti = reinterpret_cast<const SavedTweakInfo *>(dat + pos);
			pos += sizeof(SavedTweakInfo);
			
			io->tweaks[i].type = TweakType::load(sti->type); // TODO save/load flags
			io->tweaks[i].param1 = res::path::load(util::loadString(sti->param1));
			io->tweaks[i].param2 = res::path::load(util::loadString(sti->param2));
		}
		
		ARX_INTERACTIVE_APPLY_TWEAK_INFO(io);
		
		if(io->obj) {
			
			io->obj->linked.resize(ais->nb_linked);
			
			if(!io->obj->linked.empty()) {
				for(long n = 0; n < ais->nb_linked; n++) {
					io->obj->linked[n].lgroup = ObjVertGroup(ais->linked_data[n].lgroup);
					io->obj->linked[n].lidx = ActionPoint(ais->linked_data[n].lidx);
					io->obj->linked[n].lidx2 = ActionPoint(ais->linked_data[n].lidx2);
					Entity * iooo = ConvertToValidIO(ais->linked_data[n].linked_id);
					io->obj->linked[n].io = iooo;
					io->obj->linked[n].obj = iooo ? iooo->obj : NULL;
				}
			}
		}
		
		long hidegore = ((io->ioflags & IO_NPC) && io->_npcdata->lifePool.current > 0.f) ? 1 : 0;
		ARX_INTERACTIVE_HideGore(io, hidegore);
		
		if(io->ioflags & IO_NPC) {
			
			if(io->_npcdata->weaponinhand == 1) {
				SetWeapon_On(io);
			} else {
				SetWeapon_Back(io);
			}
			
			
			if(io->_npcdata->behavior == BEHAVIOUR_NONE) {
				io->targetinfo = EntityHandle();
			}
			
		}
		
	}
	
	arx_assert_msg(pos <= buffer.size(), "pos=%lu size=%lu", (unsigned long)pos, (unsigned long)buffer.size());
	
	return io;
}

static void ARX_CHANGELEVEL_PopAllIO(const std::string & buffer) {
	
	const char * dat = buffer.data();
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_INDEX * asi = reinterpret_cast<const ARX_CHANGELEVEL_INDEX *>(dat);
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	const ARX_CHANGELEVEL_IO_INDEX * idx_io = reinterpret_cast<const ARX_CHANGELEVEL_IO_INDEX *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	
	ARX_UNUSED(pos);
	arx_assert(pos <= buffer.size());
	
	float increment = 0;
	if(asi->nb_inter > 0) {
		increment = 60.f / float(asi->nb_inter);
	} else {
		progressBarAdvance(60);
		LoadLevelScreen();
	}
	
	for(long i = 0; i < asi->nb_inter; i++) {
		
		progressBarAdvance(increment);
		LoadLevelScreen();
		
		std::ostringstream oss;
		oss << res::path::load(util::loadString(idx_io[i].filename)).basename() << '_'
		    << std::setfill('0') << std::setw(4) << idx_io[i].ident;
		if(entities.getById(oss.str()).handleData() < 0) {
			ARX_CHANGELEVEL_Pop_IO(oss.str(), idx_io[i].ident);
		}
		
	}
	
}

static void ARX_CHANGELEVEL_PopAllIO_FINISH(bool reloadflag, bool firstTime) {
	
	bool movedOOBItems = false;
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * entity = entities[handle];
		
		if(!entity || !entity->obj || entity->show != SHOW_FLAG_IN_SCENE || entity->pos == entity->initpos) {
			continue;
		}
		
		if(!EERIE_PHYSICS_BOX_IsValidPosition(entity->pos - Vec3f(0.f, 20.f, 0.f))) {
			LogWarning << "Found entity " << entity->idString() << " outside the world";
			if((entity->ioflags & (IO_ITEM | IO_MOVABLE)) && (entity->gameFlags & GFLAG_INTERACTIVITY)) {
				PutInFrontOfPlayer(entity);
				movedOOBItems = true;
			} else {
				entity->pos = entity->initpos;
			}
		}
		
	}
	
	if(movedOOBItems) {
		notification_add("Found out of bounds items and dropped them in front of the player.");
	}
	
	if(reloadflag) {
		
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * e = entities[handle];
			
			if(!e) {
				continue;
			}
			
			if(e->script.valid) {
				ScriptEvent::send(&e->script, NULL, e, SM_RELOAD, "change");
			}
			
			if(entities[handle] && e->over_script.valid) {
				ScriptEvent::send(&e->over_script, NULL, e, SM_RELOAD, "change");
			}
			
			if(entities[handle] && (e->ioflags & IO_NPC) && ValidIONum(e->targetinfo)) {
				if(e->_npcdata->behavior != BEHAVIOUR_NONE) {
					e->physics.cyl = GetIOCyl(e);
					GetTargetPos(e);
					ARX_NPC_LaunchPathfind(e, e->targetinfo);
				}
			}
		}
		
	} else if(firstTime) {
		
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			if(entities[handle]) {
				SendInitScriptEvent(entities[handle]);
			}
		}
		
	} else {
		
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * e = entities[handle];
			
			if(!e) {
				continue;
			}
			
			if(e && (e->ioflags & IO_NPC) && ValidIONum(e->targetinfo)) {
				if(e->_npcdata->behavior != BEHAVIOUR_NONE) {
					e->physics.cyl = GetIOCyl(e);
					GetTargetPos(e);
					ARX_NPC_LaunchPathfind(e, e->targetinfo);
				}
			}
		}
	}
	
}

static void ARX_CHANGELEVEL_Pop_Globals() {
	
	ARX_SCRIPT_Free_All_Global_Variables();
	
	std::string buffer = g_currentSavedGame->load("globals");
	if(buffer.empty()) {
		LogError << "Unable to read globals";
		return;
	}
	
	const char * dat = buffer.data();
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_SAVE_GLOBALS * acsg;
	acsg = reinterpret_cast<const ARX_CHANGELEVEL_SAVE_GLOBALS *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	
	if(acsg->version != ARX_GAMESAVE_VERSION) {
		LogError << "Invalid version: globals";
		return;
	}
	
	svar.clear();
	svar.resize(acsg->nb_globals);
		
	bool ret = loadScriptVariables(svar, dat, pos);
	if(!ret) {
		LogError << "Error loading globals";
	}
	
	arx_assert_msg(pos <= buffer.size(), "pos=%lu size=%lu", (unsigned long)pos, (unsigned long)buffer.size());
	
}

static bool ARX_CHANGELEVEL_PopLevel(long instance, bool reloadflag, const std::string & target, float angle) {
	
	DanaeClearLevel();
	
	// Open Saveblock for read
	if(!openCurrentSavedGameFile()) {
		FORBID_SCRIPT_IO_CREATION = 0;
		return false;
	}
	
	progressBarAdvance(2.f);
	LoadLevelScreen(instance);
	
	// Now we can load our things...
	std::ostringstream loadfile;
	loadfile << "lvl" << std::setfill('0') << std::setw(3) << instance;
	std::string levelSave = g_currentSavedGame->load(loadfile.str());
	
	// first time in this level ?
	bool firstTime = levelSave.empty();
	FORBID_SCRIPT_IO_CREATION = firstTime ? 0 : 1;
	LogDebug("firstTime = " << firstTime);
	
	const ARX_CHANGELEVEL_INDEX * asi;
	if(!firstTime) {
		asi = ARX_CHANGELEVEL_Pop_Index(levelSave);
		if(asi->version != ARX_GAMESAVE_VERSION) {
			LogError << "Invalid save version...";
			FORBID_SCRIPT_IO_CREATION = 0;
			return false;
		}
	}
	
	progressBarAdvance(2.f);
	LoadLevelScreen(instance);
	
	if(ARX_CHANGELEVEL_Pop_Level(instance, firstTime) != 1) {
		LogError << "Error loading level data...";
		FORBID_SCRIPT_IO_CREATION = 0;
		return false;
	}
	if(!firstTime) {
		g_desiredFogParameters = asi->gmods_desired;
		g_currentFogParameters = asi->gmods_current;
		GMOD_RESET = false;
	}
	
	progressBarAdvance(20.f);
	LoadLevelScreen(instance);
	
	if(firstTime) {
		BOOST_FOREACH(SCR_TIMER & timer, g_scriptTimers) {
			if(timer.exist) {
				timer.start = g_gameTime.now();
			}
		}
	} else {
		ARX_CHANGELEVEL_PopAllIO(levelSave);
	}
	
	progressBarAdvance(20.f);
	LoadLevelScreen(instance);
	
	if(ARX_CHANGELEVEL_Pop_Player(target, angle) != 1) {
		LogError << "Error loading player data...";
		FORBID_SCRIPT_IO_CREATION = 0;
		return false;
	}
	
	// Restoring all Missing Objects required by other objects...
	ARX_CHANGELEVEL_PopAllIO_FINISH(reloadflag, firstTime);
	
	progressBarAdvance(15.f);
	LoadLevelScreen();
	
	progressBarAdvance(3.f);
	LoadLevelScreen();
	
	if(!firstTime) {
		ARX_CHANGELEVEL_Pop_Zones_n_Lights(levelSave);
	}
	
	progressBarAdvance();
	LoadLevelScreen();
	
	ARX_EQUIPMENT_RecreatePlayerMesh();
	
	progressBarAdvance();
	LoadLevelScreen();
	
	FORBID_SCRIPT_IO_CREATION = 0;
	
	HERO_SHOW_1ST = -1;
	
	if(EXTERNALVIEW) {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	} else {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 1);
	}
	
	ARX_INTERACTIVE_HideGore(entities.player(), 1);

	// default to mouselook true, inventory/book closed
	TRUE_PLAYER_MOUSELOOK_ON = true;
	// disable combat mode
	player.Interface &= ~INTER_COMBATMODE;
	ARX_EQUIPMENT_AttachPlayerWeaponToBack();
	
	progressBarAdvance();
	LoadLevelScreen();
	
	return true;
}

bool ARX_CHANGELEVEL_Save(const std::string & name, const fs::path & savefile) {
	
	arx_assert(!savefile.empty() && fs::exists(savefile.parent()));
	
	LogDebug("ARX_CHANGELEVEL_Save " << savefile << " " << name);
	
	if(CURRENTLEVEL == -1) {
		return false;
	}
	
	if(!openCurrentSavedGameFile()) {
		return false;
	}
	
	// Save the current level
	
	if(!ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, CURRENTLEVEL)) {
		LogWarning << "Could not save the level";
		return false;
	}
	
	// Save the savegame name and level id
	
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	memset(&pld, 0, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	pld.level = CURRENTLEVEL;
	util::storeString(pld.name, name);
	pld.version = ARX_GAMESAVE_VERSION;
	pld.time = toMsi(g_gameTime.now()); // TODO save/load time
	pld.playthroughStart = s64(g_currentPlathrough.startTime);
	pld.playthroughId = g_currentPlathrough.uniqueId;
	pld.oldestALVersion = std::min(g_currentPlathrough.oldestALVersion, arx_version_number);
	pld.newestALVersion = std::max(g_currentPlathrough.newestALVersion, arx_version_number);
	pld.lastALVersion = arx_version_number;
	std::string engineVersion = arx_name + " " + arx_version;
	util::storeString(pld.lastEngineVersion, engineVersion);
	
	const char * dat = reinterpret_cast<const char *>(&pld);
	g_currentSavedGame->save("pld", dat, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	
	// Close the savegame file
	
	if(!g_currentSavedGame->flush("pld")) {
		LogError << "Could not complete the save";
		return false;
	}
	
	// Copy the savegame and screenshot to the final destination, overwriting previous files
	if(!fs::copy_file(CURRENT_GAME_FILE, savefile, true)) {
		LogWarning << "Failed to copy save " << CURRENT_GAME_FILE <<" to " << savefile;
		return false;
	}
	
	return true;
}

static bool ARX_CHANGELEVEL_Get_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld,
                                                 const fs::path & savefile) {
	
	// Checks For Directory
	if(!fs::is_regular_file(savefile)) {
		return false;
	}
	
	std::string buffer = SaveBlock::load(savefile, "pld");
	if(buffer.size() < sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA)) {
		LogError << "Unable to open pld in " << savefile;
		return false;
	}
	
	// Stores Data in pld
	std::memcpy(&pld, buffer.data(), sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		LogError << "Invalid GameSave Version";
		return false;
	}
	
	return true;
}

void ARX_CHANGELEVEL_Load(const fs::path & savefile) {
	arx_assert(entities.player());
	
	LogInfo << "Loading save " << savefile;
	
	LogDebug("begin ARX_CHANGELEVEL_Load " << savefile);
	
	progressBarSetTotal(238);
	progressBarReset();
	
	if(!ARX_Changelevel_CurGame_Clear()) {
		return;
	}
	
	assert(!CURRENT_GAME_FILE.empty());
	
	// Copy SavePath to Current Game
	if(!fs::copy_file(savefile, CURRENT_GAME_FILE)) {
		LogWarning << "Failed to create copy savegame " << savefile << " to " << CURRENT_GAME_FILE;
		return;
	}
	
	// Retrieves Player LevelData
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, CURRENT_GAME_FILE)) {
		progressBarAdvance(2.f);
		LoadLevelScreen(pld.level);
		
		g_currentPlathrough.startTime = std::time_t(pld.playthroughStart);
		g_currentPlathrough.uniqueId = pld.playthroughId;
		g_currentPlathrough.oldestALVersion = pld.oldestALVersion;
		g_currentPlathrough.newestALVersion = pld.newestALVersion;
		
		g_gameTime.reset(GameInstantMs(pld.time));
		
		progressBarAdvance(2.f);
		LoadLevelScreen(pld.level);
		ARX_CHANGELEVEL_PopLevel(pld.level, false);
		
	} else {
		LogError << "Error Loading Level...";
		return;
	}
	
	BLOCK_PLAYER_CONTROLS = false;
	player.Interface &= ~INTER_COMBATMODE;
	ARX_EQUIPMENT_AttachPlayerWeaponToBack();
	
	entities.player()->animlayer[1].cur_anim = NULL;
	
}

long ARX_CHANGELEVEL_GetInfo(const fs::path & savefile, std::string & name, float & version,
                             long & level) {
	
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	
	// IMPROVE this will load the whole save file FAT just to get one file!
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, savefile)) {
		name = util::loadString(pld.name);
		version = pld.version;
		level = pld.level;
		return 1;
	} else {
		name = "Invalid Save...";
		level = 0;
		version = 0;
		return -1;
	}
}
