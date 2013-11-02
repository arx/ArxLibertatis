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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "scene/ChangeLevel.h"

#include <iomanip>
#include <sstream>
#include <cstdio>

#include <boost/algorithm/string/case_conv.hpp>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Config.h"
#include "core/Core.h"

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

#include "gui/MiniMap.h"
#include "gui/Interface.h"

#include "graphics/Math.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/SaveBlock.h"
#include "io/log/Logger.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/LoadLevel.h"
#include "scene/SaveFormat.h"
#include "scene/Light.h"

#include "script/ScriptEvent.h"

#include "util/String.h"

using std::string;

extern long GLOBAL_MAGIC_MODE;
float FORCE_TIME_RESTORE = 0;
extern Vec3f WILL_RESTORE_PLAYER_POSITION;
extern long WILL_RESTORE_PLAYER_POSITION_FLAG;
extern long NO_GMOD_RESET;

extern float PROGRESS_BAR_COUNT;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_TOTAL;
extern long NO_PLAYER_POSITION_RESET;
extern float InventoryDir;
extern long HERO_SHOW_1ST;
extern bool EXTERNALVIEW;
extern long LOAD_N_DONT_ERASE;
extern long FORBID_SCRIPT_IO_CREATION;
extern long NO_TIME_INIT;
extern long CHANGE_LEVEL_ICON;
extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern int iTimeToDrawD7;
extern Vec3f LastValidPlayerPos;
#define MAX_IO_SAVELOAD 1500

static bool ARX_CHANGELEVEL_Push_Index(long num);
static bool ARX_CHANGELEVEL_PushLevel(long num, long newnum);
static bool ARX_CHANGELEVEL_PopLevel(long num, long reloadflag = 0);
static void ARX_CHANGELEVEL_Push_Globals();
static void ARX_CHANGELEVEL_Pop_Globals();
static long ARX_CHANGELEVEL_Push_Player(long level);
static long ARX_CHANGELEVEL_Push_AllIO(long level);
static long ARX_CHANGELEVEL_Push_IO(const Entity * io, long level);
static Entity * ARX_CHANGELEVEL_Pop_IO(const string & ident, long num);

static fs::path CURRENT_GAME_FILE;

static float ARX_CHANGELEVEL_DesiredTime = 0;
static long CONVERT_CREATED = 0;
long DONT_WANT_PLAYER_INZONE = 0;
static SaveBlock * pSaveBlock = NULL;

static ARX_CHANGELEVEL_IO_INDEX * idx_io = NULL;
static ARX_CHANGELEVEL_INVENTORY_DATA_SAVE ** Gaids = NULL;

static Entity * convertToValidIO(const string & ident) {
	
	CONVERT_CREATED = 0;
	
	if(ident.empty() || ident == "none") {
		return NULL;
	}
	
	arx_assert_msg(
		ident.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789") == string::npos,
		"bad interactive object ident: \"%s\"", ident.c_str()
	);
	
	long t = entities.getById(ident);
	
	if(t > 0) {
		arx_assert_msg(ValidIONum(t), "got invalid IO num %ld", t);
		return entities[t];
	}
	
	LogDebug("Call to ConvertToValidIO(" << ident << ")");
	
	size_t pos = ident.find_last_of('_');
	if(pos == string::npos || pos == ident.length() - 1) {
		return NULL;
	}
	pos = ident.find_first_not_of('0', pos + 1);
	if(pos == string::npos) {
		return NULL;
	}
	
	return ARX_CHANGELEVEL_Pop_IO(ident, atoi(ident.substr(pos).c_str()));
}

template <size_t N>
static Entity * ConvertToValidIO(const char (&str)[N]) {
	return convertToValidIO(boost::to_lower_copy(util::loadString(str)));
}

template <size_t N>
static long ReadTargetInfo(const char (&str)[N]) {
	
	string ident = boost::to_lower_copy(util::loadString(str));
	
	if(ident == "none") {
		return -1;
	} else if(ident == "self") {
		return -2;
	} else if(ident == "player") {
		return 0;
	} else {
		Entity * e = convertToValidIO(ident);
		return (e == NULL) ? -1 : e->index();
	}
}

long GetIOAnimIdx2(const Entity * io, ANIM_HANDLE * anim) {
	
	if(!io || !anim) {
		return -1;
	}
	
	for(long i = 0; i < MAX_ANIMS; i++) {
		if(io->anims[i] == anim) {
			return i;
		}
	}
	
	return -1;
}

bool ARX_Changelevel_CurGame_Clear() {
	
	if(CURRENT_GAME_FILE.empty()) {
		CURRENT_GAME_FILE = fs::paths.user / "current.sav";
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

static SaveBlock * GLOBAL_pSaveB = NULL;

void ARX_Changelevel_CurGame_Open() {
	
	if(GLOBAL_pSaveB) {
		ARX_Changelevel_CurGame_Close();
	}
	
	if(pSaveBlock) {
		return;
	}
	
	if(CURRENT_GAME_FILE.empty() || !fs::exists(CURRENT_GAME_FILE)) {
		// TODO this is normal when starting a new game
		return;
	}
	
	GLOBAL_pSaveB = new SaveBlock(CURRENT_GAME_FILE);
	if(!GLOBAL_pSaveB->open()) {
		LogError << "Cannot read cur game save file" << CURRENT_GAME_FILE;
	}
}

bool ARX_Changelevel_CurGame_Seek(const std::string & ident) {
	if(pSaveBlock) {
		return pSaveBlock->hasFile(ident);
	} else if(GLOBAL_pSaveB) {
		return GLOBAL_pSaveB->hasFile(ident);
	} else {
		// this is normal when starting a new game
		return false;
	}
}

void ARX_Changelevel_CurGame_Close() {
	delete GLOBAL_pSaveB, GLOBAL_pSaveB = NULL;
}

extern long JUST_RELOADED;

void ARX_CHANGELEVEL_Change(const string & level, const string & target, long angle) {
	
	LogDebug("ARX_CHANGELEVEL_Change " << level << " " << target << " " << angle);
	
	PROGRESS_BAR_TOTAL = 238; 
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
	
	ARX_CHANGELEVEL_DesiredTime = arxtime.get_updated();
		
	long num = GetLevelNumByName("level" + level);

	LoadLevelScreen(num);
	
	if(num == -1) {
		// fatality...
		LogWarning << "Internal Non-Fatal Error";
		return;
	}
	
	// not changing level, just teleported
	if(num == CURRENTLEVEL) {
		long t = entities.getById(target);
		if(t > 0) {
			Vec3f pos;
			if(entities[t] && GetItemWorldPosition(entities[t], &pos)) {
				moveto = player.pos = pos + player.baseOffset();
				NO_PLAYER_POSITION_RESET = 1;
			}
		}
		player.desiredangle.setPitch(angle);
		player.angle.setPitch(angle);
		return; // nothing more to do :)
	}
	
	ARX_PLAYER_Reset_Fall();
	
	arxtime.pause();
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen(num);
	
	assert(!CURRENT_GAME_FILE.empty());
	pSaveBlock = new SaveBlock(CURRENT_GAME_FILE);
	
	if(!pSaveBlock->open(true)) {
		LogError << "Error writing to save block " << CURRENT_GAME_FILE;
		return;
	}
	
	LogDebug("Before ARX_CHANGELEVEL_PushLevel");
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, num);
	LogDebug("After  ARX_CHANGELEVEL_PushLevel");
	
	if(!pSaveBlock->flush("pld")) {
		LogError << "Could not complete the save.";
	}
	delete pSaveBlock, pSaveBlock = NULL;
	
	arxtime.resume();
	
	LogDebug("Before ARX_CHANGELEVEL_PopLevel");
	ARX_CHANGELEVEL_PopLevel(num, 1);
	LogDebug("After  ARX_CHANGELEVEL_PopLevel");
	
	// Now restore player pos to destination
	long t = entities.getById(target);
	if(t > 0 && entities[t]) {
		Vec3f pos;
		if(GetItemWorldPosition(entities[t], &pos)) {
			moveto = player.pos = pos + player.baseOffset();
			NO_PLAYER_POSITION_RESET = 1;
			WILL_RESTORE_PLAYER_POSITION = moveto;
			WILL_RESTORE_PLAYER_POSITION_FLAG = 1;
		}
	}
	
	CURRENTLEVEL = num;
	player.desiredangle.setPitch(angle);
	player.angle.setPitch(angle);
	DONT_WANT_PLAYER_INZONE = 1;
	ARX_PLAYER_RectifyPosition();
	JUST_RELOADED = 1;
	NO_GMOD_RESET = 0;
	LogDebug("-----------------------------------");
}

static bool ARX_CHANGELEVEL_PushLevel(long num, long newnum) {
	
	LogDebug("ARX_CHANGELEVEL_PushLevel " << num << " " << newnum);
	
	ARX_SCRIPT_EventStackExecuteAll();
	
	// Close secondary inventory before leaving
	if (SecondaryInventory != NULL)
	{
		Entity * io = SecondaryInventory->io;

		if (io != NULL)
		{
			InventoryDir = -1;
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}
	
	// Now we can save our things
	if(!ARX_CHANGELEVEL_Push_Index(num)) {
		LogError << "Error Saving Index...";
		arxtime.resume();
		return false;
	}
	
	ARX_CHANGELEVEL_Push_Globals();
	
	if(ARX_CHANGELEVEL_Push_Player(newnum) != 1) {
		LogError << "Error Saving Player...";
		arxtime.resume();
		return false;
	}
	
	if(ARX_CHANGELEVEL_Push_AllIO(num) != 1) {
		LogError << "Error Saving IOs...";
		arxtime.resume();
		return false;
	}
	
	return true;
}

bool IsPlayerEquipedWith(Entity * io) {
	
	if(!io) {
		return false;
	}
	
	long num = io->index();
	
	if(io == player.torch) {
		return true;
	}
	
	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] == num) {
			return true;
		}
	}
	
	return false;
}

extern GLOBAL_MODS stacked;
extern GLOBAL_MODS current;
extern GLOBAL_MODS desired;

static bool ARX_CHANGELEVEL_Push_Index(long num) {
	
	long pos = 0;
	
	ARX_CHANGELEVEL_INDEX asi;
	memset(&asi, 0, sizeof(asi));
	asi.version       = ARX_GAMESAVE_VERSION;
	asi.nb_inter      = 0;
	asi.nb_paths      = nbARXpaths;
	asi.time          = arxtime.get_updated_ul();
	asi.nb_lights     = 0;
	asi.gmods_stacked = stacked;
	asi.gmods_desired = desired;
	asi.gmods_current = current;
	
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] != NULL
		   && !(entities[i]->ioflags & IO_NOSAVE)
		   && !IsInPlayerInventory(entities[i])
		   && !IsPlayerEquipedWith(entities[i])) {
			asi.nb_inter++;
		}
	}
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];
		if(el && !(el->type & TYP_SPECIAL1)) {
			asi.nb_lights++;
		}
	}
	
	long allocsize = sizeof(ARX_CHANGELEVEL_INDEX)
	                 + sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi.nb_inter
	                 + sizeof(ARX_CHANGELEVEL_PATH) * asi.nb_paths
	                 + sizeof(ARX_CHANGELEVEL_LIGHT) * asi.nb_lights;
	
	size_t asize = 0;
	char * playlist = ARX_SOUND_AmbianceSavePlayList(asize);
	allocsize += asize;
	
	char * dat = new char[allocsize];
	
	asi.ambiances_data_size = asize;
	memcpy(dat, &asi, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] != NULL
		   && !(entities[i]->ioflags & IO_NOSAVE)
		   && !IsInPlayerInventory(entities[i])
		   && !IsPlayerEquipedWith(entities[i])) {
			ARX_CHANGELEVEL_IO_INDEX aii;
			memset(&aii, 0, sizeof(aii));
			strncpy(aii.filename,
			        (entities[i]->classPath() + ".teo").string().c_str(),
			        sizeof(aii.filename));
			aii.ident = entities[i]->ident;
			aii.level = num;
			aii.truelevel = num;
			aii.num = i; // !!!
			memcpy(dat + pos, &aii, sizeof(aii));
			pos += sizeof(aii);
		}
	}
	
	for(int i = 0; i < nbARXpaths; i++) {
		ARX_CHANGELEVEL_PATH * acp = reinterpret_cast<ARX_CHANGELEVEL_PATH *>(dat + pos);
		memset(acp, 0, sizeof(ARX_CHANGELEVEL_PATH));
		strncpy(acp->name, ARXpaths[i]->name.c_str(), sizeof(acp->name));
		strncpy(acp->controled, ARXpaths[i]->controled.c_str(), sizeof(acp->controled));
		pos += sizeof(ARX_CHANGELEVEL_PATH);
	}
	
	if(asi.ambiances_data_size > 0) {
		memcpy((char *)(dat + pos), playlist, asi.ambiances_data_size);
		pos += asi.ambiances_data_size;
		free(playlist);
	}
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];
		if(el != NULL && !(el->type & TYP_SPECIAL1)) {
			ARX_CHANGELEVEL_LIGHT * acl = (ARX_CHANGELEVEL_LIGHT *)(dat + pos);
			memset(acl, 0, sizeof(ARX_CHANGELEVEL_LIGHT));
			acl->status = el->status;
			pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		}
	}
	
	char savefile[256];
	sprintf(savefile, "lvl%03ld", num);
	bool ret = pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	return ret;
}

static void ARX_CHANGELEVEL_Push_Globals() {
	
	ARX_CHANGELEVEL_SAVE_GLOBALS acsg;
	long pos = 0;
	
	memset(&acsg, 0, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	acsg.nb_globals = NB_GLOBALS;
	acsg.version = ARX_GAMESAVE_VERSION;
	
	long allocsize = sizeof(ARX_VARIABLE_SAVE) * acsg.nb_globals
	                 + sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS) + 1000 + 48000;
	
	char * dat = new char[allocsize];
	
	memcpy(dat, &acsg, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	long count;
	ARX_VARIABLE_SAVE avs;

	for (long i = 0; i < NB_GLOBALS; i++)
	{
		switch (svar[i].type)
		{
			case TYPE_G_TEXT:

				if ((svar[i].name[0] == '$') || (svar[i].name[0] == '\xA3'))
				{
					strcpy(avs.name, svar[i].name);

					if (svar[i].text)
						count = strlen(svar[i].text);
					else
						count = 0;

					avs.fval = (float)count; 
					avs.type = TYPE_G_TEXT;
					memcpy(dat + pos, &avs, sizeof(ARX_VARIABLE_SAVE));
					pos += sizeof(ARX_VARIABLE_SAVE);

					if (count > 0)
						memcpy(dat + pos, svar[i].text, count + 1); 

					pos += (long)avs.fval; 
				}
				else
					acsg.nb_globals--;

				break;
			case TYPE_G_LONG:

				if ((svar[i].name[0] == '#') || (svar[i].name[0] == '\xA7'))
				{
					strcpy(avs.name, svar[i].name);
					avs.fval = (float)svar[i].ival;
					avs.type = TYPE_G_LONG;
					memcpy(dat + pos, &avs, sizeof(ARX_VARIABLE_SAVE));
					pos += sizeof(ARX_VARIABLE_SAVE);
				}
				else
					acsg.nb_globals--;

				break;
			case TYPE_G_FLOAT:

				if ((svar[i].name[0] == '&') || (svar[i].name[0] == '@'))
				{
					strcpy(avs.name, svar[i].name);
					avs.fval = svar[i].fval;
					avs.type = TYPE_G_FLOAT;
					memcpy(dat + pos, &avs, sizeof(ARX_VARIABLE_SAVE));
					pos += sizeof(ARX_VARIABLE_SAVE);
				}
				else
					acsg.nb_globals--;

				break;
			default:
				acsg.nb_globals--;
				break;
		}
	}
	
	pSaveBlock->save("globals", dat, pos);
	
	delete[] dat;
}

template <size_t N>
void FillIOIdent(char (&tofill)[N], const Entity * io) {
	
	if(!io || !ValidIOAddress(io)) {
		BOOST_STATIC_ASSERT(N >= 4);
		strcpy(tofill, "none");
	} else {
		
		string ident = io->long_name();
		
		arx_assert(ident.length() <= N);
		strncpy(tofill, ident.c_str(),  N);
	}
}

static long ARX_CHANGELEVEL_Push_Player(long level) {
	
	ARX_CHANGELEVEL_PLAYER * asp;

	long allocsize = sizeof(ARX_CHANGELEVEL_PLAYER) + 48000;
	allocsize += Keyring.size() * 64;
	allocsize += 80 * PlayerQuest.size();
	allocsize += sizeof(SavedMapMarkerData) * g_miniMap.mapMarkerCount();

	char * dat = new char[allocsize];

	asp = (ARX_CHANGELEVEL_PLAYER *)dat;

	long pos = 0;
	pos += (sizeof(ARX_CHANGELEVEL_PLAYER));

	memset(asp, 0, sizeof(ARX_CHANGELEVEL_PLAYER));

	asp->AimTime = player.AimTime;
	asp->angle = player.angle;
	asp->armor_class = player.armor_class;
	asp->Attribute_Constitution = player.Attribute_Constitution;
	asp->Attribute_Dexterity = player.Attribute_Dexterity;
	asp->Attribute_Mind = player.Attribute_Mind;
	asp->Attribute_Strength = player.Attribute_Strength;
	asp->Critical_Hit = player.Critical_Hit;
	asp->Current_Movement = player.Current_Movement;
	asp->damages = player.damages;
	asp->doingmagic = player.doingmagic;
	asp->Interface = player.Interface;
	asp->playerflags = player.playerflags;

	if (TELEPORT_TO_LEVEL[0])
		strcpy(asp->TELEPORT_TO_LEVEL, TELEPORT_TO_LEVEL);
	else
		memset(asp->TELEPORT_TO_LEVEL, 0, 64);

	if (TELEPORT_TO_POSITION[0])
		strcpy(asp->TELEPORT_TO_POSITION, TELEPORT_TO_POSITION);
	else
		memset(asp->TELEPORT_TO_POSITION, 0, 64);

	asp->TELEPORT_TO_ANGLE = TELEPORT_TO_ANGLE;
	asp->CHANGE_LEVEL_ICON = CHANGE_LEVEL_ICON;
	asp->bag = player.bag;
	FillIOIdent(asp->equipsecondaryIO, player.equipsecondaryIO);
	FillIOIdent(asp->equipshieldIO, player.equipshieldIO);
	FillIOIdent(asp->leftIO, player.leftIO);
	FillIOIdent(asp->rightIO, player.rightIO);
	FillIOIdent(asp->curtorch, player.torch);

	std::copy(Precast, Precast + SAVED_MAX_PRECAST, asp->precast);

	//inventaires
	for(long iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t m = 0; m < INVENTORY_Y; m++) {
			for(size_t n = 0; n < INVENTORY_X; n++) {
				FillIOIdent(asp->id_inventory[iNbBag][n][m], inventory[iNbBag][n][m].io);
				asp->inventory_show[iNbBag][n][m] = inventory[iNbBag][n][m].show;
			}
		}
	}
    
    g_miniMap.save(asp->minimap, SAVED_MAX_MINIMAPS);

	asp->falling = player.falling;
	asp->gold = player.gold;
	asp->invisibility = entities.player()->invisibility;

	asp->jumpphase = player.jumpphase;
	asp->jumpstarttime = player.jumpstarttime;
	asp->Last_Movement = player.Last_Movement;
	asp->level = player.level;
	asp->life = player.life;
	asp->mana = player.mana;
	asp->maxlife = player.maxlife;
	asp->maxmana = player.maxmana;
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
	asp->resist_magic = player.resist_magic;
	asp->resist_poison = player.resist_poison;
	asp->Attribute_Redistribute = player.Attribute_Redistribute;
	asp->Skill_Redistribute = player.Skill_Redistribute;
	asp->rune_flags = player.rune_flags;
	asp->size = player.size;
	asp->Skill_Stealth = player.Skill_Stealth;
	asp->Skill_Mecanism = player.Skill_Mecanism;
	asp->Skill_Intuition = player.Skill_Intuition;
	asp->Skill_Etheral_Link = player.Skill_Etheral_Link;
	asp->Skill_Object_Knowledge = player.Skill_Object_Knowledge;
	asp->Skill_Casting = player.Skill_Casting;
	asp->Skill_Projectile = player.Skill_Projectile;
	asp->Skill_Close_Combat = player.Skill_Close_Combat;
	asp->Skill_Defense = player.Skill_Defense;
	asp->skin = player.skin;
	
	asp->xp = player.xp;
	asp->nb_PlayerQuest = PlayerQuest.size();
	asp->keyring_nb = Keyring.size();
	asp->Global_Magic_Mode = GLOBAL_MAGIC_MODE;
	asp->Nb_Mapmarkers = g_miniMap.mapMarkerCount();
	
	asp->LAST_VALID_POS = LastValidPlayerPos;
	
	for (int i = 0; i < MAX_ANIMS; i++)
	{
		memset(&asp->anims[i], 0, 256);

		if (entities.player()->anims[i] != NULL)
		{
			strncpy(asp->anims[i], entities.player()->anims[i]->path.string().c_str(), sizeof(asp->anims[i]));
		}
	}

	for (long k = 0; k < MAX_EQUIPED; k++)
	{
		if (ValidIONum(player.equiped[k])
				&&	(player.equiped[k] > 0))
			FillIOIdent(asp->equiped[k], entities[player.equiped[k]]);
		else
			strcpy(asp->equiped[k], "");
	}
	
	for(size_t i = 0; i < PlayerQuest.size(); i++) {
		memset(dat + pos, 0, 80);
		assert(PlayerQuest[i].ident.length() < 80);
		strcpy((char *)(dat + pos), PlayerQuest[i].ident.c_str());
		pos += 80;
	}
	
	for(size_t i = 0; i < Keyring.size(); i++) {
		assert(sizeof(Keyring[i].slot) == SAVED_KEYRING_SLOT_SIZE);
		memcpy((char *)(dat + pos), Keyring[i].slot, SAVED_KEYRING_SLOT_SIZE);
		pos += SAVED_KEYRING_SLOT_SIZE;
	}
	
	for(size_t i = 0; i < g_miniMap.mapMarkerCount(); i++) {
		SavedMapMarkerData * acmd = reinterpret_cast<SavedMapMarkerData *>(dat + pos);
		*acmd = g_miniMap.mapMarkerGet(i);
		pos += sizeof(SavedMapMarkerData);
	}
	
	LastValidPlayerPos = asp->LAST_VALID_POS.toVec3();
	
	pSaveBlock->save("player", dat, pos);
	
	delete[] dat;
	
	for(size_t i = 1; i < entities.size(); i++) {
		if(IsInPlayerInventory(entities[i]) || IsPlayerEquipedWith(entities[i])) {
			ARX_CHANGELEVEL_Push_IO(entities[i], level);
		}
	}

	return 1;
}

static long ARX_CHANGELEVEL_Push_AllIO(long level) {
	
	for(size_t i = 1; i < entities.size(); i++) {
		
		if ((entities[i] != NULL)
				&&	(!(entities[i]->ioflags & IO_NOSAVE))
				&&	(!IsInPlayerInventory(entities[i]))
				&&	(!IsPlayerEquipedWith(entities[i]))
		   )
		{
			ARX_CHANGELEVEL_Push_IO(entities[i], level);
		}
	}

	return 1;
}

static Entity * GetObjIOSource(const EERIE_3DOBJ * obj) {
	
	if(!obj) {
		return NULL;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] && entities[i]->obj) {
			if(entities[i]->obj == obj) {
				return entities[i];
			}
		}
	}
	
	return NULL;
}

template <size_t N>
void FillTargetInfo(char (&info)[N], long numtarget) {
	BOOST_STATIC_ASSERT(N >= 6);
	if(numtarget == -2) {
		strcpy(info, "self");
	} else if(numtarget == -1) {
		strcpy(info, "none");
	} else if(numtarget == 0) {
		strcpy(info, "player");
	} else if(ValidIONum(numtarget)) {
		FillIOIdent(info, entities[numtarget]);
	} else {
		strcpy(info, "none");
	}
}

static long ARX_CHANGELEVEL_Push_IO(const Entity * io, long level) {
	
	// Check Valid IO
	if(!io) {
		return -1;
	}
	
	// Sets Savefile Name
	string savefile = io->long_name();
	
	// Define Type & Affiliated Structure Size
	long type;
	long struct_size;
	if(io->ioflags & IO_NPC) {
		type = TYPE_NPC;
		struct_size = sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
	} else if (io->ioflags & IO_ITEM) {
		type = TYPE_ITEM;
		struct_size = sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
	} else if (io->ioflags & IO_FIX) {
		type = TYPE_FIX;
		struct_size = sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
	} else if (io->ioflags & IO_CAMERA) {
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
	ais.savesystem_type = type;
	ais.saveflags = 0;
	strncpy(ais.filename, (io->classPath() + ".teo").string().c_str(),
	        sizeof(ais.filename));
	ais.ident = io->ident;
	ais.ioflags = io->ioflags;

	if ((ais.ioflags & IO_FREEZESCRIPT)
			&& ((ARX_SPELLS_GetSpellOn(io, SPELL_PARALYSE) >= 0)
				 || (ARX_SPELLS_GetSpellOn(io, SPELL_MASS_PARALYSE) >= 0)))
		ais.ioflags &= ~IO_FREEZESCRIPT;

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
	strncpy(ais.locname, io->locname.c_str(), sizeof(ais.locname));
	ais.gameFlags = io->gameFlags;

	if(io == entities.player())
		ais.gameFlags &= ~GFLAG_INVISIBILITY;

	ais.material = io->material;
	ais.level = ais.truelevel = level;
	ais.scriptload = io->scriptload;
	ais.show = io->show;
	ais.collision = io->collision;
	strncpy(ais.mainevent, io->mainevent.c_str(), sizeof(ais.mainevent));
	ais.velocity = io->velocity;
	ais.stopped = io->stopped;
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

	if (io->usepath)
	{
		ARX_USE_PATH * aup = io->usepath;
		
		ais.usepath_aupflags = aup->aupflags;
		
		ais.usepath_curtime = checked_range_cast<u32>(aup->_curtime);
		
		ais.usepath_initpos = aup->initpos;
		ais.usepath_lastWP = aup->lastWP;
		ais.usepath_starttime = checked_range_cast<u32>(aup->_starttime);
		
		strncpy(ais.usepath_name, aup->path->name.c_str(), sizeof(ais.usepath_name));
	}

	strncpy(ais.shop_category, io->shop_category.c_str(), sizeof(ais.shop_category));
	strncpy(ais.inventory_skin, io->inventory_skin.string().c_str(), sizeof(ais.inventory_skin));
	strncpy(ais.stepmaterial, io->stepmaterial.c_str(), sizeof(ais.stepmaterial));
	strncpy(ais.armormaterial, io->armormaterial.c_str(), sizeof(ais.armormaterial));
	strncpy(ais.weaponmaterial, io->weaponmaterial.c_str(), sizeof(ais.weaponmaterial));
	strncpy(ais.strikespeech, io->strikespeech.c_str(), sizeof(ais.strikespeech));

	ais.nb_linked = 0;
	memset(&ais.linked_data, 0, sizeof(IO_LINKED_DATA)*MAX_LINKED_SAVE);

	// Save Animations
	for (long i = 0; i < MAX_ANIMS; i++)
	{
		memset(&ais.anims[i], 0, 256);

		if (io->anims[i] != NULL)
		{
			strncpy(ais.anims[i], io->anims[i]->path.string().c_str(), sizeof(ais.anims[i]));
		}
	}

	// Save Linked Objects
	if (io->obj)
	{
		ais.nb_linked = 0;

		for (long n = 0; n < io->obj->nblinked; n++)
		{
			if (GetObjIOSource((EERIE_3DOBJ *)io->obj->linked[n].obj))
				ais.nb_linked++;
		}

		if ((size_t)ais.nb_linked > MAX_LINKED_SAVE) ais.nb_linked = MAX_LINKED_SAVE;

		long count = 0;

		for (int n = 0; n < io->obj->nblinked; n++)
		{
			if (GetObjIOSource((EERIE_3DOBJ *)io->obj->linked[n].obj))
			{
				ais.linked_data[count].lgroup = io->obj->linked[count].lgroup;
				ais.linked_data[count].lidx = io->obj->linked[count].lidx;
				ais.linked_data[count].lidx2 = io->obj->linked[count].lidx2;
				ais.linked_data[count].modinfo = SavedModInfo();
				FillIOIdent(ais.linked_data[count].linked_id, GetObjIOSource(io->obj->linked[count].obj));
				count++;
			}
		}
	}


	if (io->tweakerinfo)
	{
		ais.system_flags	= SYSTEM_FLAG_TWEAKER_INFO;
	}
	else ais.system_flags = 0;

	if (io->inventory)
	{
		ais.system_flags |= SYSTEM_FLAG_INVENTORY;
	}

	if (type == TYPE_ITEM)
	{
		if (io->_itemdata->equipitem)
		{
			ais.system_flags |= SYSTEM_FLAG_EQUIPITEMDATA;
		}
	}

	if (io->usepath)
		ais.system_flags |= SYSTEM_FLAG_USEPATH;

	ais.physics = io->physics;
	ais.spellcast_data = io->spellcast_data;
	assert(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
	std::copy(io->animlayer, io->animlayer + SAVED_MAX_ANIM_LAYERS, ais.animlayer);
	for(long k = 0; k < MAX_ANIM_LAYERS; k++) {
		ais.animlayer[k].cur_anim = GetIOAnimIdx2(io, io->animlayer[k].cur_anim);
		ais.animlayer[k].next_anim = GetIOAnimIdx2(io, io->animlayer[k].next_anim);
	}

	// Save Target Infos
	long numtarget = io->targetinfo;

	if ((io->ioflags & IO_NPC) && (io->_npcdata->pathfind.listnb > 0) && ValidIONum(io->_npcdata->pathfind.truetarget))
	{
		numtarget = io->_npcdata->pathfind.truetarget;
	}

	FillTargetInfo(ais.id_targetinfo, numtarget);

	// Save Local Timers ?
	long count = 0;

	for (int i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if (scr_timer[i].io == io)
			{
				count++;
			}
		}
	}

	ais.nbtimers = count;

	long allocsize =
		sizeof(ARX_CHANGELEVEL_IO_SAVE)
		+ sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE)
		+ io->script.nblvar * (sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE) + 500)
		+ sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE)
		+ io->over_script.nblvar * (sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE) + 500)
		+ struct_size
		+ sizeof(SavedTweakerInfo)
		+ sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE) + 1024
		+ sizeof(SavedGroupData) * io->groups.size()
		+ sizeof(SavedTweakInfo) * io->tweaks.size()
		+ 48000;

	// Allocate Main Save Buffer
	char * dat = new char[allocsize];
	long pos = 0;

	ais.halo = io->halo_native;
	ais.Tweak_nb = io->tweaks.size();
	memcpy(dat, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);

	long timm = (unsigned long)(arxtime); //treat warning C4244 conversion from 'float' to 'unsigned long''

	for (int i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if (scr_timer[i].io == io)
			{
				ARX_CHANGELEVEL_TIMERS_SAVE * ats = (ARX_CHANGELEVEL_TIMERS_SAVE *)(dat + pos);
				memset(ats, 0, sizeof(ARX_CHANGELEVEL_TIMERS_SAVE));
				ats->longinfo = scr_timer[i].longinfo;
				ats->msecs = scr_timer[i].msecs;
				strcpy(ats->name, scr_timer[i].name.c_str());
				ats->pos = scr_timer[i].pos;

				if (scr_timer[i].es == &io->script)
					ats->script = 0;
				else	ats->script = 1;

				ats->tim = (scr_timer[i].tim + scr_timer[i].msecs) - timm;

				if (ats->tim < 0) ats->tim = 0;

				//else ats->tim=-ats->tim;
				ats->times = scr_timer[i].times;
				ats->flags = scr_timer[i].flags;
				pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
			}
		}
	}

	ARX_CHANGELEVEL_SCRIPT_SAVE * ass = (ARX_CHANGELEVEL_SCRIPT_SAVE *)(dat + pos);
	ass->allowevents = io->script.allowevents;
	ass->lastcall = io->script.lastcall;
	ass->nblvar = io->script.nblvar;
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);

	for (int i = 0; i < io->script.nblvar; i++)
	{
		ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
		memset(avs, 0, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		long count;

		switch (io->script.lvar[i].type)
		{
			case TYPE_L_TEXT:

				if ((io->script.lvar[i].name[0] == '$') || (io->script.lvar[i].name[0] == '\xA3'))
				{
					strcpy(avs->name, io->script.lvar[i].name);

					if (io->script.lvar[i].text)
						count = strlen(io->script.lvar[i].text);
					else
						count = 0;

					avs->fval = (float)(count + 1);
					avs->type = TYPE_L_TEXT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if(avs->fval > 0) {
						memset(dat + pos, 0, checked_range_cast<size_t>(avs->fval)); //count+1);
						if(count > 0) {
							memcpy(dat + pos, io->script.lvar[i].text, count);
						}
					}

					pos += (long)avs->fval;
				}
				else
					ass->nblvar--;

				break;
			case TYPE_L_LONG:

				if ((io->script.lvar[i].name[0] == '#') || (io->script.lvar[i].name[0] == '\xA7'))
				{
					strcpy(avs->name, io->script.lvar[i].name);
					avs->fval = (float)io->script.lvar[i].ival;
					avs->type = TYPE_L_LONG;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				}
				else
					ass->nblvar--;

				break;
			case TYPE_L_FLOAT:

				if ((io->script.lvar[i].name[0] == '&') || (io->script.lvar[i].name[0] == '@'))
				{
					strcpy(avs->name, io->script.lvar[i].name);
					avs->fval = io->script.lvar[i].fval;
					avs->type = TYPE_L_FLOAT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				}
				else
					ass->nblvar--;

				break;
			default:
				ass->nblvar--;
				break;
		}
	}

	ass = (ARX_CHANGELEVEL_SCRIPT_SAVE *)(dat + pos);
	ass->allowevents = io->over_script.allowevents;
	ass->lastcall = io->over_script.lastcall;
	ass->nblvar = io->over_script.nblvar;
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);

	for (int i = 0; i < io->over_script.nblvar; i++)
	{
		ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
		memset(avs, 0, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		long count;

		switch (io->over_script.lvar[i].type)
		{
			case TYPE_L_TEXT:

				if ((io->script.lvar[i].name[0] == '$') || (io->script.lvar[i].name[0] == '\xA3'))
				{
					strcpy(avs->name, io->over_script.lvar[i].name);

					if (io->over_script.lvar[i].text)
						count = strlen(io->over_script.lvar[i].text);
					else
						count = 0;

					avs->fval	= (float)(count + 1);
					avs->type	= TYPE_L_TEXT;
					pos			+= sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if(avs->fval > 0) {
						memset(dat + pos, 0, checked_range_cast<size_t>(avs->fval));
						if(count > 0) {
							memcpy(dat + pos, io->over_script.lvar[i].text, count);
						}
					}

					pos		+= (long)avs->fval;
				}
				else
					ass->nblvar--;

				break;
			case TYPE_L_LONG:

				if ((io->script.lvar[i].name[0] == '#') || (io->script.lvar[i].name[0] == '\xA7'))
				{
					strcpy(avs->name, io->over_script.lvar[i].name);
					avs->fval	= (float)io->over_script.lvar[i].ival;
					avs->type	= TYPE_L_LONG;
					pos			+= sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				}
				else
					ass->nblvar--;

				break;
			case TYPE_L_FLOAT:

				if ((io->script.lvar[i].name[0] == '&') || (io->script.lvar[i].name[0] == '@'))
				{
					strcpy(avs->name, io->over_script.lvar[i].name);
					avs->fval	= io->over_script.lvar[i].fval;
					avs->type	= TYPE_L_FLOAT;
					pos			+= sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				}
				else
					ass->nblvar--;

				break;
			default:
				ass->nblvar--;
		}
	}

	switch (type)
	{
		case TYPE_NPC:
			ARX_CHANGELEVEL_NPC_IO_SAVE * as;
			as = (ARX_CHANGELEVEL_NPC_IO_SAVE *)(dat + pos);
			memset(as, 0, sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE));
			as->absorb = io->_npcdata->absorb;
			as->aimtime = io->_npcdata->aimtime;
			as->armor_class = io->_npcdata->armor_class;
			as->behavior = io->_npcdata->behavior;
			as->behavior_param = io->_npcdata->behavior_param;
			as->collid_state = io->_npcdata->collid_state;
			as->collid_time = io->_npcdata->collid_time;
			as->cut = io->_npcdata->cut;
			as->damages = io->_npcdata->damages;
			as->detect = io->_npcdata->detect;
			as->fightdecision = io->_npcdata->fightdecision;

			if(io->_npcdata->life > 0.f) {
				FillIOIdent(as->id_weapon, io->_npcdata->weapon);
			} else {
				as->id_weapon[0] = 0;
			}

			as->lastmouth = io->_npcdata->lastmouth;
			as->life = io->_npcdata->life;
			as->look_around_inc = io->_npcdata->look_around_inc;
			as->mana = io->_npcdata->mana;
			as->maxlife = io->_npcdata->maxlife;
			as->maxmana = io->_npcdata->maxmana;
			as->movemode = io->_npcdata->movemode;
			as->moveproblem = io->_npcdata->moveproblem;
			as->reachedtarget = io->_npcdata->reachedtarget;
			as->speakpitch = io->_npcdata->speakpitch;
			as->tactics = io->_npcdata->tactics;
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
					strcpy(as->stackedtarget[i], "none");
				}
			}

			as->critical = io->_npcdata->critical;
			as->reach = io->_npcdata->reach;
			as->backstab_skill = io->_npcdata->backstab_skill;
			as->poisonned = io->_npcdata->poisonned;
			as->resist_poison = io->_npcdata->resist_poison;
			as->resist_magic = io->_npcdata->resist_magic;
			as->resist_fire = io->_npcdata->resist_fire;
			as->strike_time = io->_npcdata->strike_time;
			as->walk_start_time = io->_npcdata->walk_start_time;
			as->aiming_start = io->_npcdata->aiming_start;
			as->npcflags = io->_npcdata->npcflags;
			memset(&as->pathfind, 0, sizeof(IO_PATHFIND));

			if (io->_npcdata->pathfind.listnb > 0)
				as->pathfind.truetarget = io->_npcdata->pathfind.truetarget;
			else as->pathfind.truetarget = -1;

			if(io->_npcdata->ex_rotate) {
				ais.saveflags |= SAVEFLAGS_EXTRA_ROTATE;
				as->ex_rotate = *io->_npcdata->ex_rotate;
			}

			as->blood_color = io->_npcdata->blood_color.toBGRA();
			as->fDetect = io->_npcdata->fDetect;
			as->cuts = io->_npcdata->cuts;
			pos += struct_size;
			break;
		case TYPE_ITEM:
			ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
			ai = (ARX_CHANGELEVEL_ITEM_IO_SAVE *)(dat + pos);
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

			pos += struct_size;
			break;
		case TYPE_FIX:
			ARX_CHANGELEVEL_FIX_IO_SAVE * af;
			af = (ARX_CHANGELEVEL_FIX_IO_SAVE *)(dat + pos);
			memset(af, 0, sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE));
			af->trapvalue			= io->_fixdata->trapvalue;
			pos += struct_size;
			break;
		case TYPE_CAMERA:
			ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
			ac = (ARX_CHANGELEVEL_CAMERA_IO_SAVE *)(dat + pos);
			ac->cam = io->_camdata->cam;
			pos += struct_size;
			break;
		case TYPE_MARKER:
			ARX_CHANGELEVEL_MARKER_IO_SAVE * am;
			am = (ARX_CHANGELEVEL_MARKER_IO_SAVE *)(dat + pos);
			am->dummy = 0;
			pos += struct_size;
			break;
	}

	if (ais.system_flags & SYSTEM_FLAG_INVENTORY)
	{
		ARX_CHANGELEVEL_INVENTORY_DATA_SAVE * aids;
		aids = (ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *)(dat + pos);
		memset(aids, 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));
		long m, n;

		INVENTORY_DATA * inv = io->inventory;
		FillIOIdent(aids->io, inv->io);
		aids->sizex = inv->sizex;
		aids->sizey = inv->sizey;

		for (m = 0; m < aids->sizex; m++)
			for (n = 0; n < aids->sizey; n++)
			{
				aids->initio[m][n][0] = 0;

				if (inv->slot[m][n].io)
					FillIOIdent(aids->slot_io[m][n], inv->slot[m][n].io);
				else
					aids->slot_io[m][n][0] = 0;
				
				aids->slot_show[m][n] = inv->slot[m][n].show;
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
		strncpy(sgd->name, i->c_str(), sizeof(sgd->name));
	}
	
	for(std::vector<TWEAK_INFO>::const_iterator i = io->tweaks.begin(); i != io->tweaks.end(); ++i) {
		SavedTweakInfo * sti = reinterpret_cast<SavedTweakInfo *>(dat + pos);
		pos += sizeof(SavedTweakInfo);
		*sti = *i;
	}
	
	if((pos > allocsize)) {
		LogError << "SaveBuffer Overflow " << pos << " >> " << allocsize;
	}
	
	pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	return 1;
}

static long ARX_CHANGELEVEL_Pop_Index(ARX_CHANGELEVEL_INDEX * asi, long num) {
	
	size_t pos = 0;
	std::string loadfile;
	std::stringstream ss;
	
	ss << "lvl" << std::setfill('0') << std::setw(3) << num;
	loadfile = ss.str();
	
	size_t size; // TODO size is not used
	char * dat = pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	memcpy(asi, dat, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	
	arx_assert(idx_io == NULL);
	if(asi->nb_inter) {
		idx_io = new ARX_CHANGELEVEL_IO_INDEX[asi->nb_inter];
		memcpy(idx_io, dat + pos, sizeof(ARX_CHANGELEVEL_IO_INDEX)*asi->nb_inter);
		pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	} else {
		idx_io = NULL;
	}
	
	// Skip Path info (used later !)
	pos += sizeof(ARX_CHANGELEVEL_PATH) * asi->nb_paths;
	
	// Restore Ambiances
	if(asi->ambiances_data_size) {
		ARX_SOUND_AmbianceRestorePlayList(dat + pos, asi->ambiances_data_size);
		pos += asi->ambiances_data_size;
	}
	
	ARX_UNUSED(pos);
	arx_assert(pos <= size);
	
	free(dat);
	
	return 1;
}

long ARX_CHANGELEVEL_Pop_Zones_n_Lights(ARX_CHANGELEVEL_INDEX * asi, long num) {
	
	std::stringstream ss;
	ss << "lvl" << std::setfill('0') << std::setw(3) << num;
	std::string loadfile = ss.str();
	
	size_t size; // TODO size not used
	// TODO this has already been loaded and decompressed in ARX_CHANGELEVEL_Pop_Index!
	char * dat = pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	size_t pos = 0;
	// Skip Changelevel Index
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

		for(size_t j = 0; j < MAX_LIGHTS; j++) {
			EERIE_LIGHT * el = GLight[j];
			if(el && !(el->type & TYP_SPECIAL1)) {
				if(count == i) {
					el->status = acl->status;
					break;
				}
				count++;
			}
		}
	}
	
	free(dat);
	
	return 1;
}

long ARX_CHANGELEVEL_Pop_Level(ARX_CHANGELEVEL_INDEX * asi, long num, bool firstTime) {
	
	char levelId[256];
	GetLevelNameByNum(num, levelId);
	string levelFile = string("graph/levels/level") + levelId + "/level" + levelId + ".dlf";
	
	LOAD_N_DONT_ERASE = 1;
	
	if(!resources->getFile(levelFile)) {
		LogError << "Unable To Find " << levelFile;
		return 0;
	}
	
	LoadLevelScreen(num);
	SetEditMode(1, false);
	
	ARX_CHANGELEVEL_Pop_Globals();
	
	DanaeLoadLevel(levelFile, firstTime);
	CleanScriptLoadedIO();
	
	FirstFrame = true;
	
	if(firstTime) {
		
		RestoreInitialIOStatus();
		
		for(size_t i = 1; i < entities.size(); i++) {
			if(entities[i] && !entities[i]->scriptload) {
				ARX_SCRIPT_Reset(entities[i], 1);
			}
		}
		
		BLOCK_PLAYER_CONTROLS = false;
		ARX_INTERFACE_Reset();
		EERIE_ANIMMANAGER_PurgeUnused();
		
	} else {
		BLOCK_PLAYER_CONTROLS = false;
		ARX_INTERFACE_Reset();
		EERIE_ANIMMANAGER_PurgeUnused();
	}
	
	stacked = asi->gmods_stacked;
	desired = asi->gmods_desired;
	current = asi->gmods_current;
	NO_GMOD_RESET = 1;
	arxtime.force_time_restore(ARX_CHANGELEVEL_DesiredTime);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	
	return 1;
}

static long ARX_CHANGELEVEL_Pop_Player() {
	
	const string & loadfile = "player";
	
	size_t size;
	char * dat = pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	if(size < sizeof(ARX_CHANGELEVEL_PLAYER)) {
		LogError << "Truncated data";
		return -1;
	}
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_PLAYER * asp = reinterpret_cast<const ARX_CHANGELEVEL_PLAYER *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_PLAYER);
	
	player.AimTime = asp->AimTime;
	player.angle = asp->angle;
	player.desiredangle = player.angle;
	
	player.armor_class = checked_range_cast<unsigned char>(asp->armor_class);
	
	player.Attribute_Constitution = asp->Attribute_Constitution;
	player.Attribute_Dexterity = asp->Attribute_Dexterity;
	player.Attribute_Mind = asp->Attribute_Mind;
	player.Attribute_Strength = asp->Attribute_Strength;
	player.Critical_Hit = asp->Critical_Hit;
	player.Current_Movement = PlayerMovement::load(asp->Current_Movement); // TODO save/load flags
	player.damages = asp->damages;
	player.doingmagic = asp->doingmagic;
	player.playerflags = PlayerFlags::load(asp->playerflags); // TODO save/load flags
	
	if(asp->TELEPORT_TO_LEVEL[0]) {
		strcpy(TELEPORT_TO_LEVEL, boost::to_lower_copy(util::loadString(asp->TELEPORT_TO_LEVEL)).c_str());
	} else {
		memset(TELEPORT_TO_LEVEL, 0, 64);
	}
	
	if(asp->TELEPORT_TO_POSITION[0]) {
		strcpy(TELEPORT_TO_POSITION, boost::to_lower_copy(util::loadString(asp->TELEPORT_TO_POSITION)).c_str());
	} else {
		memset(TELEPORT_TO_POSITION, 0, 64);
	}
	
	TELEPORT_TO_ANGLE = asp->TELEPORT_TO_ANGLE;
	CHANGE_LEVEL_ICON = asp->CHANGE_LEVEL_ICON;
	player.bag = asp->bag;
	assert(SAVED_MAX_PRECAST == MAX_PRECAST);
	std::copy(asp->precast, asp->precast + SAVED_MAX_PRECAST, Precast);
	player.Interface = asp->Interface;
	player.Interface &= ~INTER_MAP;
	player.falling = asp->falling;
	player.gold = asp->gold;
	entities.player()->invisibility = asp->invisibility;
	player.inzone = ARX_PATH_GetAddressByName(boost::to_lower_copy(util::loadString(asp->inzone)));
	player.jumpphase = JumpPhase(asp->jumpphase); // TODO save/load enum
	player.jumpstarttime = asp->jumpstarttime;
	player.Last_Movement = PlayerMovement::load(asp->Last_Movement); // TODO save/load flags
	
	player.level = checked_range_cast<unsigned char>(asp->level);
	
	player.life = asp->life;
	player.mana = asp->mana;
	player.maxlife = asp->maxlife;
	player.maxmana = asp->maxmana;
	
	player.onfirmground = (asp->misc_flags & 1) ? 1 : 0;
	WILLRETURNTOCOMBATMODE = (asp->misc_flags & 2) ? 1 : 0;
	
	player.physics = asp->physics;
	player.poison = asp->poison;
	player.hunger = asp->hunger;
	player.pos = asp->pos.toVec3();
	
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
	
	if(entities.player()) {
		entities.player()->pos = player.basePosition();
	}
	
	WILL_RESTORE_PLAYER_POSITION = asp->pos.toVec3();
	WILL_RESTORE_PLAYER_POSITION_FLAG = 1;
	
	player.resist_magic = checked_range_cast<unsigned char>(asp->resist_magic);
	player.resist_poison = checked_range_cast<unsigned char>(asp->resist_poison);
	
	player.Attribute_Redistribute = checked_range_cast<unsigned char>(asp->Attribute_Redistribute);
	player.Skill_Redistribute = checked_range_cast<unsigned char>(asp->Skill_Redistribute);
	
	player.rune_flags = RuneFlags::load(asp->rune_flags); // TODO save/load flags
	player.size = asp->size.toVec3();
	player.Skill_Stealth = asp->Skill_Stealth;
	player.Skill_Mecanism = asp->Skill_Mecanism;
	player.Skill_Intuition = asp->Skill_Intuition;
	player.Skill_Etheral_Link = asp->Skill_Etheral_Link;
	player.Skill_Object_Knowledge = asp->Skill_Object_Knowledge;
	player.Skill_Casting = asp->Skill_Casting;
	player.Skill_Projectile = asp->Skill_Projectile;
	player.Skill_Close_Combat = asp->Skill_Close_Combat;
	player.Skill_Defense = asp->Skill_Defense;
	
	player.skin = checked_range_cast<char>(asp->skin);
	
	player.xp = asp->xp;
	GLOBAL_MAGIC_MODE = asp->Global_Magic_Mode;
	
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
	
	assert(SAVED_INVENTORY_Y == INVENTORY_Y);
	assert(SAVED_INVENTORY_X == INVENTORY_X);
	for(size_t iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t m = 0; m < SAVED_INVENTORY_Y; m++) {
			for (size_t n = 0; n < SAVED_INVENTORY_X; n++) {
				inventory[iNbBag][n][m].io = ConvertToValidIO(asp->id_inventory[iNbBag][n][m]);
				inventory[iNbBag][n][m].show = asp->inventory_show[iNbBag][n][m];
			}
		}
	}
	
	if(size < pos + (asp->nb_PlayerQuest * 80)) {
		LogError << "Truncated data";
		return -1;
	}
	ARX_PLAYER_Quest_Init();
	for(int i = 0; i < asp->nb_PlayerQuest; i++) {
		ARX_PLAYER_Quest_Add(script::loadUnlocalized(boost::to_lower_copy(util::loadString(dat + pos, 80))), true);
		pos += 80;
	}
	
	if(size < pos + (asp->keyring_nb * SAVED_KEYRING_SLOT_SIZE)) {
		LogError << "Truncated data";
		return -1;
	}
	ARX_KEYRING_Init();
	LogDebug(asp->keyring_nb);
	for(int i = 0; i < asp->keyring_nb; i++) {
		ARX_KEYRING_Add(boost::to_lower_copy(util::loadString(dat + pos, SAVED_KEYRING_SLOT_SIZE)));
		pos += SAVED_KEYRING_SLOT_SIZE;
	}
	
	if(size < pos + (asp->Nb_Mapmarkers * sizeof(SavedMapMarkerData))) {
		LogError << "Truncated data";
		return -1;
	}
	g_miniMap.mapMarkerInit(asp->Nb_Mapmarkers);
	for(int i = 0; i < asp->Nb_Mapmarkers; i++) {
		const SavedMapMarkerData * acmd = reinterpret_cast<const SavedMapMarkerData *>(dat + pos);
		pos += sizeof(SavedMapMarkerData);
		g_miniMap.mapMarkerAdd(acmd->x, acmd->y, acmd->lvl, script::loadUnlocalized(boost::to_lower_copy(util::loadString(acmd->name))));
	}
	
	ARX_PLAYER_Restore_Skin();
	
	ARX_PLAYER_Modify_XP(0);
	
	PROGRESS_BAR_COUNT += 10.f;
	LoadLevelScreen();
	
	// Restoring Player equipment...
	player.equipsecondaryIO = ConvertToValidIO(asp->equipsecondaryIO);
	player.equipshieldIO = ConvertToValidIO(asp->equipshieldIO);
	player.leftIO = ConvertToValidIO(asp->leftIO);
	player.rightIO = ConvertToValidIO(asp->rightIO);
	player.torch = ConvertToValidIO(asp->curtorch);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	assert(SAVED_MAX_EQUIPED == MAX_EQUIPED);
	for(size_t i = 0; i < SAVED_MAX_EQUIPED; i++) {
		Entity * e = ConvertToValidIO(asp->equiped[i]);
		player.equiped[i] = (e == NULL) ? -1 : e->index();
		if(!ValidIONum(player.equiped[i])) {
			player.equiped[i] = 0;
		}
	}
	
	free(dat);
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	return 1;
}

static bool loadScriptVariables(SCRIPT_VAR * var, long & n, const char * dat, size_t & pos, VariableType ttext, VariableType tlong, VariableType tfloat) {
	
	for(long i = 0; i < n; i++) {
		
		const ARX_CHANGELEVEL_VARIABLE_SAVE * avs;
		avs = reinterpret_cast<const ARX_CHANGELEVEL_VARIABLE_SAVE *>(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
		
		string name = boost::to_lower_copy(util::loadString(avs->name));
		strcpy(var[i].name, name.c_str());
		
		if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789", 1) != string::npos) {
			LogWarning << "Unexpected variable name \"" << name.substr(1) << '"';
		}
		
		VariableType type;
		if(avs->type == ttext || avs->type == tlong || avs->type == tfloat) {
			type = (VariableType)avs->type;
		} else if(avs->name[0] == '$' || avs->name[0] == '\xA3') {
			type = ttext;
		} else if(avs->name[0] == '&' || avs->name[0] == '@') {
			type = tfloat;
		} else if(avs->name[0] == '#' || avs->name[0] == 's') {
			type = tlong;
		} else {
			LogError << "Unknown script variable type: " << avs->type;
			n = i;
			return false;
		}
		
		var[i].fval = avs->fval;
		var[i].ival = (long)avs->fval;
		var[i].type = type;
		
		if(type == ttext) {
			if(var[i].ival) {
				var[i].text = strdup(boost::to_lower_copy(util::loadString(dat + pos, var[i].ival)).c_str());
				pos += var[i].ival;
				if(var[i].text[0] == '\xCC') {
					var[i].text[0] = 0;
				}
				var[i].ival = strlen(var[i].text) + 1;
			}
		}
		
		LogDebug(((type & (TYPE_G_TEXT|TYPE_G_LONG|TYPE_G_FLOAT)) ? "global " : "local ") << ((type & (TYPE_L_TEXT|TYPE_G_TEXT)) ? "text" : (type & (TYPE_L_LONG|TYPE_G_LONG)) ? "long" : (type & (TYPE_L_FLOAT|TYPE_G_FLOAT)) ? "float" : "unknown") << " \"" << util::loadString(var[i].name).substr(1) << "\" = " << var[i].fval << ' ' << Logger::nullstr(var[i].text));
		
	}
	
	return true;
}

static bool loadScriptData(EERIE_SCRIPT & script, const char * dat, size_t & pos) {
	
	const ARX_CHANGELEVEL_SCRIPT_SAVE * ass;
	ass = reinterpret_cast<const ARX_CHANGELEVEL_SCRIPT_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);
	
	script.allowevents = DisabledEvents::load(ass->allowevents); // TODO save/load flags
	script.nblvar = ass->nblvar;
	
	free(script.lvar), script.lvar = NULL;
	if(ass->nblvar > 0) {
		script.lvar = (SCRIPT_VAR *)malloc(sizeof(SCRIPT_VAR) * script.nblvar);
		memset(script.lvar, 0, sizeof(SCRIPT_VAR)* script.nblvar);
	}
	
	return loadScriptVariables(script.lvar, script.nblvar, dat, pos,
	                           TYPE_L_TEXT, TYPE_L_LONG, TYPE_L_FLOAT);
}

static Entity * ARX_CHANGELEVEL_Pop_IO(const string & ident, long num) {
	
	LogDebug("--> loading interactive object " << ident);
	
	size_t size = 0; // TODO size not used
	char * dat = pSaveBlock->load(ident, size);
	if(!dat) {
		LogError << "Unable to Open " << ident << " for Read...";
		return NULL;
	}
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_IO_SAVE * ais = reinterpret_cast<const ARX_CHANGELEVEL_IO_SAVE *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);
	
	if(ais->version != ARX_GAMESAVE_VERSION) {
		LogError << "Invalid PopIO version " << ais->version;
		free(dat);
		return NULL;
	}
	
	if(ais->show == SHOW_FLAG_DESTROYED || (ais->ioflags & IO_NOSAVE)) {
		free(dat);
		return NULL;
	}
	
	std::string path = util::loadString(ais->filename);
	if((!path.empty() && path[0] == '\\')
	   || (path.length() >= 3 && isalpha(path[0]) && path[1] == ':'
		     && path[2] == '\\')) {
		// Old save files stored absolute paths,
		// strip everything before 'graph' when loading.
		boost::to_lower(path);
		size_t pos = path.find("graph");
		if(pos != std::string::npos) {
			path = path.substr(pos);
		}
	}
	
	// TODO when we bump the save version, remove the ext in the save file
	// if no class names contain dots, we might get away without changing the ver
	res::path classPath = res::path::load(path).remove_ext();
	Entity * io = LoadInter_Ex(classPath, num, ais->pos.toVec3(), ais->angle, MSP);
	
	if(!io) {
		LogError << "CHANGELEVEL Error: Unable to load " << ident;
	} else {
		
		long Gaids_Number = io->index();
		Gaids[Gaids_Number] = new ARX_CHANGELEVEL_INVENTORY_DATA_SAVE;
		
		memset(Gaids[Gaids_Number], 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));
		
		io->room_flags = 1;
		io->room = -1;
		io->no_collide = -1;
		io->ioflags = EntityFlags::load(ais->ioflags); // TODO save/load flags
		
		io->ioflags &= ~IO_FREEZESCRIPT;
		io->pos = ais->pos.toVec3();
		io->lastpos = ais->lastpos.toVec3();
		io->move = ais->move.toVec3();
		io->lastmove = ais->lastmove.toVec3();
		io->initpos = ais->initpos.toVec3();
		io->initangle = ais->initangle;
		io->angle = ais->angle;
		io->scale = ais->scale;
		io->weight = ais->weight;
		io->locname = script::loadUnlocalized(boost::to_lower_copy(util::loadString(ais->locname)));
		io->gameFlags = GameFlags::load(ais->gameFlags); // TODO save/load flags
		io->material = (Material)ais->material; // TODO save/load enum
		
		// Script data
		io->scriptload = ais->scriptload;
		io->show = EntityVisilibity(ais->show); // TODO save/load enum
		io->collision = IOCollisionFlags::load(ais->collision); // TODO save/load flags
		io->mainevent = boost::to_lower_copy(util::loadString(ais->mainevent));
		
		// Physics data
		io->velocity = ais->velocity.toVec3();
		io->stopped = ais->stopped;
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
			ARX_USE_PATH * aup = io->usepath = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
			aup->aupflags = UsePathFlags::load(ais->usepath_aupflags); // TODO save/load flags
			aup->_curtime = static_cast<float>(ais->usepath_curtime);
			aup->initpos = ais->usepath_initpos.toVec3();
			aup->lastWP = ais->usepath_lastWP;
			aup->_starttime = static_cast<float>(ais->usepath_starttime);
			aup->path = ARX_PATH_GetAddressByName(boost::to_lower_copy(util::loadString(ais->usepath_name)));
		}
		
		io->shop_category = boost::to_lower_copy(util::loadString(ais->shop_category));
		
		io->halo_native = ais->halo;
		io->halo_native.dynlight = -1;
		io->halo.dynlight = -1;
		ARX_HALO_SetToNative(io);
		
		io->inventory_skin = res::path::load(util::loadString(ais->inventory_skin));
		io->stepmaterial = boost::to_lower_copy(util::loadString(ais->stepmaterial));
		io->armormaterial = boost::to_lower_copy(util::loadString(ais->armormaterial));
		io->weaponmaterial = boost::to_lower_copy(util::loadString(ais->weaponmaterial));
		io->strikespeech = script::loadUnlocalized(boost::to_lower_copy(util::loadString(ais->strikespeech)));
		
		for(long i = 0; i < MAX_ANIMS; i++) {
			
			if(io->anims[i] != NULL) {
				ReleaseAnimFromIO(io, i);
			}
			
			if(ais->anims[i][0] == '\0') {
				continue;
			}
			
			res::path path = res::path::load(util::loadString(ais->anims[i]));
			
			io->anims[i] = EERIE_ANIMMANAGER_Load(path);
			if(io->anims[i]) {
				continue;
			}
			
			if(io->ioflags & IO_NPC) {
				path = res::path("graph/obj3d/anims/npc") / path.filename();
			} else {
				path = res::path("graph/obj3d/anims/fix_inter") / path.filename();
			}
			
			io->anims[i] = EERIE_ANIMMANAGER_Load(path);
			if(!io->anims[i]) {
				LogWarning << "Error loading animation " << path;
			}
		}
		
		io->spellcast_data = ais->spellcast_data;
		io->physics = ais->physics;
		assert(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
		std::copy(ais->animlayer, ais->animlayer + SAVED_MAX_ANIM_LAYERS, io->animlayer);
		
		for(long k = 0; k < MAX_ANIM_LAYERS; k++) {
			
			long nn = (long)ais->animlayer[k].cur_anim;
			if(nn == -1) {
				io->animlayer[k].cur_anim = NULL;
			} else {
				io->animlayer[k].cur_anim = io->anims[nn];
				if(io->animlayer[k].cur_anim && io->animlayer[k].altidx_cur >= io->animlayer[k].cur_anim->alt_nb) {
					LogWarning << "Out of bounds animation alternative index " << io->animlayer[k].altidx_cur << " for " << io->animlayer[k].cur_anim->path << ", resetting to 0";
					io->animlayer[k].altidx_cur = 0;
				}
			}
			
			nn = (long)ais->animlayer[k].next_anim;
			if(nn == -1) {
				io->animlayer[k].next_anim = NULL;
			} else {
				io->animlayer[k].next_anim = io->anims[nn];
				if(io->animlayer[k].next_anim && io->animlayer[k].altidx_next >= io->animlayer[k].next_anim->alt_nb) {
					LogWarning << "Out of bounds animation alternative index " << io->animlayer[k].altidx_next << " for " << io->animlayer[k].next_anim->path << ", resetting to 0";
					io->animlayer[k].altidx_next = 0;
				}
			}
		}
		
		// Target Info
		memcpy(Gaids[Gaids_Number]->targetinfo, ais->id_targetinfo, SIZE_ID);
		
		ARX_SCRIPT_Timer_Clear_By_IO(io);
		
		for(int i = 0; i < ais->nbtimers; i++) {
			
			const ARX_CHANGELEVEL_TIMERS_SAVE * ats;
			ats = reinterpret_cast<const ARX_CHANGELEVEL_TIMERS_SAVE *>(dat + pos);
			pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
			
			short sFlags = checked_range_cast<short>(ats->flags);
			
			long num = ARX_SCRIPT_Timer_GetFree();
			if(num == -1) {
				continue;
			}
			
			ActiveTimers++;
			
			if(ats->script) {
				scr_timer[num].es = &io->over_script;
			} else {
				scr_timer[num].es = &io->script;
			}
			
			scr_timer[num].flags = sFlags;
			scr_timer[num].exist = 1;
			scr_timer[num].io = io;
			scr_timer[num].msecs = ats->msecs;
			scr_timer[num].name = boost::to_lower_copy(util::loadString(ats->name));
			scr_timer[num].pos = ats->pos;
			// TODO if the script has changed since the last save, this position may be invalid
			
			float tt = ARX_CHANGELEVEL_DesiredTime + ats->tim;
			if(tt < 0) {
				scr_timer[num].tim = 0;
			} else {
				scr_timer[num].tim = checked_range_cast<unsigned long>(tt);
			}
			
			scr_timer[num].times = ats->times;
		}
		
		if(!loadScriptData(io->script, dat, pos) || !loadScriptData(io->over_script, dat, pos)) {
				LogError << "Save file is corrupted, trying to fix " << ident;
				free(dat);
				io->inventory = NULL;
				RestoreInitialIOStatusOfIO(io);
				SendInitScriptEvent(io);
				return io;
		}
		
		Gaids[Gaids_Number]->weapon[0] = '\0';
		
		switch (ais->savesystem_type) {
			
			case TYPE_NPC: {
				
				const ARX_CHANGELEVEL_NPC_IO_SAVE * as;
				as = reinterpret_cast<const ARX_CHANGELEVEL_NPC_IO_SAVE *>(dat + pos);
				pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
				
				io->_npcdata->absorb = as->absorb;
				io->_npcdata->aimtime = as->aimtime;
				io->_npcdata->armor_class = as->armor_class;
				io->_npcdata->behavior = Behaviour::load(as->behavior); // TODO save/load flags
				io->_npcdata->behavior_param = as->behavior_param;
				io->_npcdata->collid_state = as->collid_state;
				io->_npcdata->collid_time = as->collid_time;
				io->_npcdata->cut = as->cut;
				io->_npcdata->damages = as->damages;
				io->_npcdata->detect = as->detect;
				io->_npcdata->fightdecision = as->fightdecision;
				
				memcpy(Gaids[Gaids_Number]->weapon, as->id_weapon, SIZE_ID);
				
				io->_npcdata->lastmouth = as->lastmouth;
				io->_npcdata->life = as->life;
				io->_npcdata->look_around_inc = as->look_around_inc;
				io->_npcdata->mana = as->mana;
				io->_npcdata->maxlife = as->maxlife;
				io->_npcdata->maxmana = as->maxmana;
				io->_npcdata->movemode = (MoveMode)as->movemode; // TODO save/load enum
				io->_npcdata->moveproblem = as->moveproblem;
				io->_npcdata->reachedtarget = as->reachedtarget;
				io->_npcdata->speakpitch = as->speakpitch;
				io->_npcdata->tactics = as->tactics;
				io->_npcdata->tohit = as->tohit;
				io->_npcdata->weaponinhand = as->weaponinhand;
				io->_npcdata->weapontype = ItemType::load(as->weapontype); // TODO save/load flags
				io->_npcdata->xpvalue = as->xpvalue;
				
				assert(SAVED_MAX_STACKED_BEHAVIOR == MAX_STACKED_BEHAVIOR);
				std::copy(as->stacked, as->stacked + SAVED_MAX_STACKED_BEHAVIOR, io->_npcdata->stacked);
				// TODO properly load stacked animations
				
				memcpy(Gaids[Gaids_Number]->stackedtarget, as->stackedtarget, SIZE_ID * SAVED_MAX_STACKED_BEHAVIOR);
				
				io->_npcdata->critical = as->critical;
				io->_npcdata->reach = as->reach;
				io->_npcdata->backstab_skill = as->backstab_skill;
				io->_npcdata->poisonned = as->poisonned;
				io->_npcdata->resist_poison = as->resist_poison;
				io->_npcdata->resist_magic = as->resist_magic;
				io->_npcdata->resist_fire = as->resist_fire;
				io->_npcdata->strike_time = as->strike_time;
				io->_npcdata->walk_start_time = as->walk_start_time;
				io->_npcdata->aiming_start = as->aiming_start;
				io->_npcdata->npcflags = NPCFlags::load(as->npcflags); // TODO save/load flags
				io->_npcdata->fDetect = as->fDetect;
				io->_npcdata->cuts = as->cuts;
				
				memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));
				
				io->_npcdata->pathfind.truetarget = as->pathfind.truetarget;
				
				if(ais->saveflags & SAVEFLAGS_EXTRA_ROTATE) {
					if(io->_npcdata->ex_rotate == NULL) {
						io->_npcdata->ex_rotate = new EERIE_EXTRA_ROTATE();
					}
					*io->_npcdata->ex_rotate = as->ex_rotate;
				}
				
				io->_npcdata->blood_color = Color::fromBGRA(as->blood_color);
				
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
				
				free(io->_itemdata->equipitem), io->_itemdata->equipitem = NULL;
				if(ais->system_flags & SYSTEM_FLAG_EQUIPITEMDATA) {
					io->_itemdata->equipitem = (IO_EQUIPITEM *)malloc(sizeof(IO_EQUIPITEM));
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
				io->_camdata->cam = ac->cam;
				break;
			}
			
			case TYPE_MARKER: {
				pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
				break;
			}
		}
		
		if(ais->system_flags & SYSTEM_FLAG_INVENTORY) {
			
			memcpy(Gaids[Gaids_Number], dat + pos, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE)
			       - SIZE_ID - SIZE_ID - MAX_LINKED_SAVE * SIZE_ID - SIZE_ID * SAVED_MAX_STACKED_BEHAVIOR);
			pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);
			
			if(io->inventory == NULL) {
				io->inventory = (INVENTORY_DATA *)malloc(sizeof(INVENTORY_DATA));
			}
			memset(io->inventory, 0, sizeof(INVENTORY_DATA));
			
		} else {
			free(io->inventory), io->inventory = NULL;
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
			
			io->obj->nblinked = ais->nb_linked;
			
			if(io->obj->nblinked) {
				
				free(io->obj->linked);
				io->obj->linked = (EERIE_LINKED *)malloc(sizeof(EERIE_LINKED) * io->obj->nblinked);
				
				for(long n = 0; n < ais->nb_linked; n++) {
					io->obj->linked[n].lgroup = ais->linked_data[n].lgroup;
					io->obj->linked[n].lidx = ais->linked_data[n].lidx;
					io->obj->linked[n].lidx2 = ais->linked_data[n].lidx2;
					memcpy(Gaids[Gaids_Number]->linked_id[n], ais->linked_data[n].linked_id, SIZE_ID);
					io->obj->linked[n].io = NULL;
					io->obj->linked[n].obj = NULL;
				}
			}
		}
		
		long hidegore = ((io->ioflags & IO_NPC) && io->_npcdata->life > 0.f) ? 1 : 0;
		ARX_INTERACTIVE_HideGore(io, hidegore);
		
	}
	
	free(dat);
	CONVERT_CREATED = 1;
	
	return io;
}

static void ARX_CHANGELEVEL_PopAllIO(ARX_CHANGELEVEL_INDEX * asi) {
	
	float increment = 0;
	if(asi->nb_inter > 0) {
		increment = (60.f / (float)asi->nb_inter);
	} else {
		PROGRESS_BAR_COUNT += 60;
		LoadLevelScreen();
	}
	
	for (long i = 0; i < asi->nb_inter; i++) {
		
		PROGRESS_BAR_COUNT += increment;
		LoadLevelScreen();
		
		std::ostringstream oss;
		oss << res::path::load(util::loadString(idx_io[i].filename)).basename() << '_'
		    << std::setfill('0') << std::setw(4) << idx_io[i].ident;
		if(entities.getById(oss.str()) < 0) {
			ARX_CHANGELEVEL_Pop_IO(oss.str(), idx_io[i].ident);
		}
	}
}

extern void GetIOCyl(Entity * io, EERIE_CYLINDER * cyl);

static void ARX_CHANGELEVEL_PopAllIO_FINISH(long reloadflag, bool firstTime) {
	
	bool * treated = new bool[MAX_IO_SAVELOAD];
	memset(treated, 0, sizeof(unsigned char)*MAX_IO_SAVELOAD);
	
	long converted = 1;
	while(converted) {
		converted = 0;
		
		for(size_t it = 1; it < MAX_IO_SAVELOAD && it < entities.size(); it++) {
			Entity * io = entities[it];
			
			if(!io || treated[it]) {
				continue;
			}
			
			treated[it] = true;
			
			const ARX_CHANGELEVEL_INVENTORY_DATA_SAVE * aids = Gaids[it];
			if(!aids) {
				continue;
			}
			
			if(io->inventory) {
				
				INVENTORY_DATA * inv = io->inventory;
				inv->io = ConvertToValidIO(aids->io);
				converted += CONVERT_CREATED;
				
				inv->sizex = 3;
				inv->sizey = 11;
				if(aids->sizex != 3 || aids->sizey != 11) {
					for(long m = 0; m < inv->sizex; m++) {
						for(long n = 0; n < inv->sizey; n++) {
							inv->slot[m][n].io = NULL;
							inv->slot[m][n].show = 0;
						}
					}
				} else {
					for(long m = 0; m < inv->sizex; m++) {
						for(long n = 0; n < inv->sizey; n++) {
							inv->slot[m][n].io = ConvertToValidIO(aids->slot_io[m][n]);
							converted += CONVERT_CREATED;
							inv->slot[m][n].show = aids->slot_show[m][n];
						}
					}
				}
			}
			
			if(io->obj && io->obj->nblinked) {
				for(long n = 0; n < io->obj->nblinked; n++) {
					Entity * iooo = ConvertToValidIO(aids->linked_id[n]);
					if(iooo) {
						io->obj->linked[n].io = iooo;
						io->obj->linked[n].obj = iooo->obj;
					}
				}
			}
			
			if(io->ioflags & IO_NPC) {
				io->_npcdata->weapon = ConvertToValidIO(aids->weapon);
				converted += CONVERT_CREATED;
				if(io->_npcdata->weaponinhand == 1) {
					SetWeapon_On(io);
				} else {
					SetWeapon_Back(io);
				}
			}
			
			io->targetinfo = ReadTargetInfo(aids->targetinfo);
			if((io->ioflags & IO_NPC) && io->_npcdata->behavior == BEHAVIOUR_NONE) {
				io->targetinfo = -1;
			}
			
			if(io->ioflags & IO_NPC) {
				for(long iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++) {
					io->_npcdata->stacked[iii].target = ReadTargetInfo(aids->stackedtarget[iii]);
				}
			}
			
		}
	}
	
	delete[] treated;
	
	if(reloadflag) {
		
		for(size_t i = 0; i < entities.size(); i++) {
			
			if(!entities[i]) {
				continue;
			}
			
			if(entities[i]->script.data != NULL) {
				ScriptEvent::send(&entities[i]->script, SM_RELOAD, "change", entities[i], "");
			}
			
			if(entities[i] && entities[i]->over_script.data) {
				ScriptEvent::send(&entities[i]->over_script, SM_RELOAD, "change", entities[i], "");
			}
			
			if(entities[i] && (entities[i]->ioflags & IO_NPC) && ValidIONum(entities[i]->targetinfo)) {
				if(entities[i]->_npcdata->behavior != BEHAVIOUR_NONE) {
					GetIOCyl(entities[i], &entities[i]->physics.cyl);
					GetTargetPos(entities[i]);
					ARX_NPC_LaunchPathfind(entities[i], entities[i]->targetinfo);
				}
			}
		}
		
	} else if (firstTime) {
		
		for(size_t i = 0; i < entities.size(); i++) {
			
			if(!entities[i]) {
				continue;
			}
			
			if(entities[i]->script.data) {
				ScriptEvent::send(&entities[i]->script, SM_INIT, "", entities[i], "");
			}
			
			if(entities[i] && entities[i]->over_script.data) {
				ScriptEvent::send(&entities[i]->over_script, SM_INIT, "", entities[i], "");
			}
			
			if(entities[i] && entities[i]->script.data) {
				ScriptEvent::send(&entities[i]->script, SM_INITEND, "", entities[i], "");
			}
			
			if(entities[i] && entities[i]->over_script.data) {
				ScriptEvent::send(&entities[i]->over_script, SM_INITEND, "", entities[i], "");
			}
		}
		
	} else {
		
		for(size_t i = 0; i < entities.size(); i++) {
			
			if(!entities[i]) {
				continue;
			}
			
			if(entities[i] && (entities[i]->ioflags & IO_NPC) && ValidIONum(entities[i]->targetinfo)) {
				if(entities[i]->_npcdata->behavior != BEHAVIOUR_NONE) {
					GetIOCyl(entities[i], &entities[i]->physics.cyl);
					GetTargetPos(entities[i]);
					ARX_NPC_LaunchPathfind(entities[i], entities[i]->targetinfo);
				}
			}
		}
	}
	
}

static void ARX_CHANGELEVEL_Pop_Globals() {
	
	ARX_SCRIPT_Free_All_Global_Variables();
	
	size_t size;
	char * dat = pSaveBlock->load("globals", size);
	if(!dat) {
		LogError << "Unable to Open globals for Read...";
		return;
	}
	
	size_t pos = 0;
	
	const ARX_CHANGELEVEL_SAVE_GLOBALS * acsg;
	acsg = reinterpret_cast<const ARX_CHANGELEVEL_SAVE_GLOBALS *>(dat + pos);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	
	if(acsg->version != ARX_GAMESAVE_VERSION) {
		free(dat);
		LogError << "Invalid version: globals";
		return;
	}
	
	arx_assert(!svar);
	if(acsg->nb_globals > 0) {
		svar = (SCRIPT_VAR *)malloc(sizeof(SCRIPT_VAR) * acsg->nb_globals);
		memset(svar, 0, sizeof(SCRIPT_VAR)* acsg->nb_globals);
	}
	
	NB_GLOBALS = acsg->nb_globals;
	
	bool ret = loadScriptVariables(svar, NB_GLOBALS, dat, pos, TYPE_G_TEXT, TYPE_G_LONG, TYPE_G_FLOAT);
	if(!ret) {
		LogError << "Error loading globals";
	}
	
	free(dat);
}

static void ReleaseGaids() {
	
	for(size_t i = 0; i < entities.size(); i++) {
		delete Gaids[i];
	}
	
	delete[] Gaids, Gaids = NULL;
}

static void ARX_CHANGELEVEL_PopLevel_Abort() {
	
	delete pSaveBlock, pSaveBlock = NULL;
	
	arxtime.resume();
	
	delete[] idx_io, idx_io = NULL;
	
	ReleaseGaids();
	FORBID_SCRIPT_IO_CREATION = 0;
}

static bool ARX_CHANGELEVEL_PopLevel(long instance, long reloadflag) {
	
	DANAE_ReleaseAllDatasDynamic();
	
	LogDebug("Before ARX_CHANGELEVEL_PopLevel Alloc'n'Free");
	
	if(Gaids) {
		ReleaseGaids();
	}
	
	Gaids = new ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *[MAX_IO_SAVELOAD];
	memset(Gaids, 0, sizeof(*Gaids) * MAX_IO_SAVELOAD);
	
	ARX_CHANGELEVEL_INDEX asi;
	
	LogDebug("After  ARX_CHANGELEVEL_PopLevel Alloc'n'Free");
	
	// Clears All Scene contents...
	LogDebug("Before DANAE ClearAll");
	DanaeClearAll();
	LogDebug("After  DANAE ClearAll");
	
	arxtime.pause();
	arxtime.force_time_restore(ARX_CHANGELEVEL_DesiredTime);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	
	// Now we can load our things...
	std::ostringstream loadfile;
	loadfile << "lvl" << std::setfill('0') << std::setw(3) << instance;
	
	// Open Saveblock for read
	pSaveBlock = new SaveBlock(CURRENT_GAME_FILE);
	
	// first time in this level ?
	bool firstTime;
	if(!pSaveBlock->open() || !pSaveBlock->hasFile(loadfile.str())) {
		firstTime = true;
		FORBID_SCRIPT_IO_CREATION = 0;
		NO_PLAYER_POSITION_RESET = 0;
	} else {
		firstTime = false;
		FORBID_SCRIPT_IO_CREATION = 1;
		NO_PLAYER_POSITION_RESET = 1;
	}
	LogDebug("firstTime = " << firstTime);
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen(instance);
	
	arx_assert(idx_io == NULL);
	
	if(!firstTime) {
		
		LogDebug("Before ARX_CHANGELEVEL_Pop_Index");
		if(ARX_CHANGELEVEL_Pop_Index(&asi, instance) != 1) {
			LogError << "Cannot Load Index data";
			ARX_CHANGELEVEL_PopLevel_Abort();
			return false;
		}
		
		LogDebug("After  ARX_CHANGELEVEL_Pop_Index");
		if(asi.version != ARX_GAMESAVE_VERSION) {
			LogError << "Invalid Save Version...";
			ARX_CHANGELEVEL_PopLevel_Abort();
			return false;
		}
		
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen(instance);
	
	LogDebug("Before ARX_CHANGELEVEL_Pop_Level");
	if(ARX_CHANGELEVEL_Pop_Level(&asi, instance, firstTime) != 1) {
		LogError << "Cannot Load Level data";
		ARX_CHANGELEVEL_PopLevel_Abort();
		return false;
	}
	
	LogDebug("After  ARX_CHANGELEVEL_Pop_Index");
	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(instance);
	
	if(firstTime) {
		unsigned long ulDTime = checked_range_cast<unsigned long>(ARX_CHANGELEVEL_DesiredTime);
		for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
			if(scr_timer[i].exist) {
				scr_timer[i].tim = ulDTime;
			}
		}
	} else {
		LogDebug("Before ARX_CHANGELEVEL_PopAllIO");
		ARX_CHANGELEVEL_PopAllIO(&asi);
		LogDebug("After  ARX_CHANGELEVEL_PopAllIO");
	}
	
	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(instance);
	LogDebug("Before ARX_CHANGELEVEL_Pop_Player");
	
	if(ARX_CHANGELEVEL_Pop_Player() != 1) {
		LogError << "Cannot Load Player data";
		ARX_CHANGELEVEL_PopLevel_Abort();
		return false;
	}
	LogDebug("After  ARX_CHANGELEVEL_Pop_Player");
	
	LogDebug("Before ARX_CHANGELEVEL_PopAllIO_FINISH");
	// Restoring all Missing Objects required by other objects...
	ARX_CHANGELEVEL_PopAllIO_FINISH(reloadflag, firstTime);
	LogDebug("After  ARX_CHANGELEVEL_PopAllIO_FINISH");
	
	PROGRESS_BAR_COUNT += 15.f;
	LoadLevelScreen();
	
	ReleaseGaids();
	
	PROGRESS_BAR_COUNT += 3.f;
	LoadLevelScreen();
	
	if(!firstTime) {
		LogDebug("Before ARX_CHANGELEVEL_Pop_Zones_n_Lights");
		ARX_CHANGELEVEL_Pop_Zones_n_Lights(&asi, instance);
		LogDebug("After  ARX_CHANGELEVEL_Pop_Zones_n_Lights");
	}
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	LogDebug("Before Player Misc Init");
	ARX_EQUIPMENT_RecreatePlayerMesh();
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	arxtime.force_time_restore(ARX_CHANGELEVEL_DesiredTime);
	NO_TIME_INIT = 1;
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	LogDebug("After  Player Misc Init");
	
	LogDebug("Before Memory Release");
	delete[] idx_io, idx_io = NULL;
	FORBID_SCRIPT_IO_CREATION = 0;
	NO_TIME_INIT = 1;
	LogDebug("After  Memory Release");
	
	LogDebug("Before SaveBlock Release");
	delete pSaveBlock;
	pSaveBlock = NULL;
	LogDebug("After  SaveBlock Release");
	
	LogDebug("Before Final Inits");
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
	
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	LogDebug("After  Final Inits");
	
	return true;
}

bool ARX_CHANGELEVEL_Save(const string & name, const fs::path & savefile) {
	
	arx_assert(!savefile.empty() && fs::exists(savefile.parent()));
	
	LogDebug("ARX_CHANGELEVEL_Save " << savefile << " " << name);
	
	arxtime.pause();
	
	if(CURRENTLEVEL == -1) {
		LogWarning << "Internal Non-Fatal Error";
		return false;
	}
	
	pSaveBlock = new SaveBlock(CURRENT_GAME_FILE);
	
	if(!pSaveBlock->open(true)) {
		LogError << "Opening savegame " << CURRENT_GAME_FILE;
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
	strncpy(pld.name, name.c_str(), sizeof(pld.name));
	pld.version = ARX_GAMESAVE_VERSION;
	pld.time = arxtime.get_updated_ul();
	
	const char * dat = reinterpret_cast<const char *>(&pld);
	pSaveBlock->save("pld", dat, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	
	// Close the savegame file
	
	if(!pSaveBlock->flush("pld")) {
		LogError << "Could not complete the save";
		return false;
	}
	delete pSaveBlock, pSaveBlock = NULL;
	
	arxtime.resume();
	
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
	
	size_t size;
	char * dat = SaveBlock::load(savefile, "pld", size);
	if(!dat) {
		LogError << "Unable to open pld in " << savefile;
		return false;
	}
	
	// Stores Data in pld
	memcpy(&pld, dat, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	
	if(pld.version != ARX_GAMESAVE_VERSION) {
		LogError << "Invalid GameSave Version";
		free(dat);
		return false;
	}
	
	// Release Data
	free(dat);
	
	return true;
}

long ARX_CHANGELEVEL_Load(const fs::path & savefile) {
	
	LogDebug("begin ARX_CHANGELEVEL_Load " << savefile);
	
	iTimeToDrawD7 = -3000;
	
	PROGRESS_BAR_TOTAL = 238; 
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
	
	arxtime.pause();
	
	if(!ARX_Changelevel_CurGame_Clear()) {
		return -1;
	}
	
	assert(!CURRENT_GAME_FILE.empty());
	
	// Copy SavePath to Current Game
	if(!fs::copy_file(savefile, CURRENT_GAME_FILE)) {
		LogWarning << "Failed to create copy savegame " << savefile << " to " << CURRENT_GAME_FILE;
		return -1;
	}
	
	// Retrieves Player LevelData
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, CURRENT_GAME_FILE)) {
		PROGRESS_BAR_COUNT += 2.f;
		LoadLevelScreen(pld.level);
		
		float fPldTime = static_cast<float>(pld.time);
		DanaeClearLevel();
		PROGRESS_BAR_COUNT += 2.f;
		LoadLevelScreen(pld.level);
		CURRENTLEVEL = pld.level;
		ARX_CHANGELEVEL_DesiredTime	=	fPldTime;
		ARX_CHANGELEVEL_PopLevel(pld.level, 0);
		arxtime.force_time_restore(fPldTime);
		NO_TIME_INIT = 1;
		FORCE_TIME_RESTORE = fPldTime;
		
	} else {
		LogError << "Error Loading Level...";
		return -1;
	}
	
	BLOCK_PLAYER_CONTROLS = false;
	player.Interface &= ~INTER_COMBATMODE;
	
	if (entities.player()) entities.player()->animlayer[1].cur_anim = NULL;
	
	JUST_RELOADED = 1;
	
	LogDebug("success ARX_CHANGELEVEL_Load");
	return 1;
}

long ARX_CHANGELEVEL_GetInfo(const fs::path & savefile, string & name, float & version,
                             long & level, unsigned long & time) {
	
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	
	// IMPROVE this will load the whole save file FAT just to get one file!
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, savefile)) {
		name = util::loadString(pld.name);
		version = pld.version;
		level = pld.level;
		time = (pld.time / 1000);
		return 1;
	} else {
		name = "Invalid Save...";
		time = 0;
		level = 0;
		version = 0;
		return -1;
	}
}
