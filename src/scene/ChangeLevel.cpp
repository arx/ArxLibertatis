/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_ChangeLevel
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Change Level
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
// TODO:
//	-Need to save GlobalMods for each level
//	-Need to restore Player inventory...
//	-Player inventory Items Must be pushed but Mustn't be in ANY Level Index...
//  -IO Tweaks ?
//	-Check Spawned Objects Ident... Might be dangerous
//
//////////////////////////////////////////////////////////////////////////////////////

#include "scene/ChangeLevel.h"

#include <iomanip>
#include <cassert>
#include <sstream>
#include <cstdio>

#include "ai/Paths.h"
#include "ai/PathFinderManager.h"

#include "core/Time.h"
#include "core/Dialog.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/NPC.h"
#include "game/Spells.h"
#include "game/Player.h"
#include "game/Levels.h"
#include "game/Inventory.h"

#include "gui/MiniMap.h"
#include "gui/Speech.h"

#include "graphics/d3dwrapper.h"
#include "graphics/Math.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/SaveBlock.h"

#include "physics/CollisionShapes.h"

#include "platform/String.h"

#include "scene/Interactive.h"
#include "scene/GameSound.h"
#include "scene/Object.h"
#include "scene/LoadLevel.h"
#include "scene/SaveFormat.h"
#include "scene/Light.h"

#include "scripting/ScriptEvent.h"

using std::string;


extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern long GLOBAL_MAGIC_MODE;
float FORCE_TIME_RESTORE = 0;
extern EERIE_3D	WILL_RESTORE_PLAYER_POSITION;
extern long WILL_RESTORE_PLAYER_POSITION_FLAG;
extern long NO_GMOD_RESET;

void DANAE_ReleaseAllDatasDynamic();

//-----------------------------------------------------------------------------
#define TYPE_NPC	1
#define TYPE_ITEM	2
#define TYPE_FIX	3
#define TYPE_CAMERA	4
#define TYPE_MARKER	5
//-----------------------------------------------------------------------------
extern long ARX_CONVERSATION;
extern HANDLE LIGHTTHREAD;
extern float PROGRESS_BAR_COUNT;
extern float OLD_PROGRESS_BAR_COUNT;
extern float PROGRESS_BAR_TOTAL;
extern long NO_PLAYER_POSITION_RESET;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern float InventoryDir;
extern char LOCAL_SAVENAME[64];
extern long HERO_SHOW_1ST;
extern long EXTERNALVIEW;
extern long LOAD_N_DONT_ERASE;
extern long DONT_LOAD_INTERS;
extern long FORBID_SCRIPT_IO_CREATION;
extern long NO_TIME_INIT;
extern long RELOADING;
extern long CHANGE_LEVEL_ICON;
extern long FOR_EXTERNAL_PEOPLE;
extern long TRUE_PLAYER_MOUSELOOK_ON;
extern int iTimeToDrawD7;
extern EERIE_3D LastValidPlayerPos;
#define MAX_IO_SAVELOAD	1500

static long ARX_CHANGELEVEL_PushLevel(long num, long newnum);
static long ARX_CHANGELEVEL_PopLevel(long num, long reloadflag = 0);
static long ARX_CHANGELEVEL_Push_Globals();
static long ARX_CHANGELEVEL_Pop_Globals();
static long ARX_CHANGELEVEL_Push_Player();
static long ARX_CHANGELEVEL_Push_AllIO();
static long ARX_CHANGELEVEL_Push_IO(const INTERACTIVE_OBJ * io);
static long ARX_CHANGELEVEL_Pop_IO(const string & ident);

//-----------------------------------------------------------------------------
struct TEMP_IO
{
  char ident[64];
};
TEMP_IO * tio = NULL;

long nb_tio = 0;
long NEW_LEVEL = -1;
long LAST_CHINSTANCE = 1; // temporary MUST return to -1;
char CurGamePath[256];
float ARX_CHANGELEVEL_DesiredTime = 0;
long CONVERT_CREATED = 0;
long DONT_WANT_PLAYER_INZONE = 0;
long FORBID_SAVE = 0;
long _FIRSTTIME = 0;
SaveBlock * _pSaveBlock = NULL;


ARX_CHANGELEVEL_IO_INDEX * idx_io = NULL;
long idx_io_nb = 0;
ARX_CHANGELEVEL_VARIABLE_SAVE 	*	index_variable = NULL;
 
ARX_CHANGELEVEL_INVENTORY_DATA_SAVE ** _Gaids = NULL;


extern long ARX_CONVERSATION;
extern HANDLE LIGHTTHREAD;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern EERIE_BACKGROUND bkrgnd;
long CURRENT_GAME_INSTANCE = -1;
char GameSavePath[256];
extern char LOCAL_SAVENAME[64];

static void ARX_GAMESAVE_CreateNewInstance() {
	char basepath[256];
	char testpath[256];
	long num = 1;
	sprintf(basepath, "Save%s\\", LOCAL_SAVENAME);

	for (;;)
	{
		sprintf(testpath, "%sSave%04ld", basepath, num);

		if (!DirectoryExist(testpath))
		{
			CreateDirectory(testpath, NULL);
			CURRENT_GAME_INSTANCE = num;
			ARX_GAMESAVE_MakePath();
			return;
		}
		else
		{
			//The directory may exist but may be empty after crash
			strcat(testpath, "\\GSAVE.SAV");
			if(!FileExist(testpath)) {
				CURRENT_GAME_INSTANCE = num;
				ARX_GAMESAVE_MakePath();
				return;
			}
		}

		num++;
	}
}

long NEED_LOG = 1; //0;

//-----------------------------------------------------------------------------
INTERACTIVE_OBJ * ConvertToValidIO(char * str)
{

	CONVERT_CREATED = 0;

	if ((!str)
			||	(str[0] == 0)) return NULL;

	long t = GetTargetByNameTarget(str);

	if (t < 0)
	{
		if ((NEED_LOG) && (strcasecmp(str, "none")))
		{
			LogDebug << "Call to ConvertToValidIO(" << str << ")";
		}

		t = ARX_CHANGELEVEL_Pop_IO(str);

		if (t < 0) return NULL;
	}

	inter.iobj[t]->level = (short)NEW_LEVEL; // Not really needed anymore...
	return (inter.iobj[t]);
}

//-----------------------------------------------------------------------------
long GetIOAnimIdx2(const INTERACTIVE_OBJ * io, ANIM_HANDLE * anim)
{
	if ((!io)
			||	(!anim))
		return -1;

	for (long i = 0; i < MAX_ANIMS; i++)
	{
		if (io->anims[i] == anim)
			return i;
	}

	return -1;
}
//--------------------------------------------------------------------------------------------
void ARX_CHANGELEVEL_MakePath() {
	sprintf(CurGamePath, "Save%s\\Cur%04ld\\", LOCAL_SAVENAME, LAST_CHINSTANCE);
	CreateFullPath(CurGamePath);
}

void ARX_GAMESAVE_MakePath() {
	sprintf(GameSavePath, "Save%s\\Save%04ld\\", LOCAL_SAVENAME, CURRENT_GAME_INSTANCE);
	CreateFullPath(GameSavePath);
}

//--------------------------------------------------------------------------------------------
void ARX_CHANGELEVEL_CreateNewInstance()
{
	char basepath[256];
	char testpath[256];
	long num = 1;
	sprintf(basepath, "Save%s\\", LOCAL_SAVENAME);

	for (;;)
	{
		sprintf(testpath, "%sCur%04ld", basepath, num);

		if (!DirectoryExist(testpath))
		{
			CreateDirectory(testpath, NULL);
			LAST_CHINSTANCE = num;
			ARX_CHANGELEVEL_MakePath();
			return;
		}

		num++;
	}
}
 
SaveBlock * GLOBAL_pSaveB = NULL;
void ARX_Changelevel_CurGame_Open() {
	
	if(GLOBAL_pSaveB) {
		ARX_Changelevel_CurGame_Close();
	}
	
	if(_pSaveBlock) {
		return;
	}
	
	string savefile = CurGamePath;
	savefile += "Gsave.sav";
	
	if(FileExist(savefile)) {
		
		GLOBAL_pSaveB = new SaveBlock(savefile);
		if(!GLOBAL_pSaveB->BeginRead()) {
			LogError << "cannot read cur game save file" << savefile;
		}
		
	} else {
		// this is normal when starting a new game
	}
}

bool ARX_Changelevel_CurGame_Seek(const std::string & ident) {
	if(_pSaveBlock) {
		return _pSaveBlock->hasFile( ident + ".sav" );
	} else if(GLOBAL_pSaveB) {
		return GLOBAL_pSaveB->hasFile( ident + ".sav" );
	} else {
		// this is normal when starting a new game
		return false;
	}
}

void ARX_Changelevel_CurGame_Close() {
	if(GLOBAL_pSaveB) {
		delete GLOBAL_pSaveB;
		GLOBAL_pSaveB = NULL;
	}
}

extern long JUST_RELOADED;
extern void DemoFileCheck();
extern long FINAL_COMMERCIAL_DEMO;
//--------------------------------------------------------------------------------------------
void ARX_CHANGELEVEL_Change( const std::string& level, const std::string& target, long angle, long confirm)
{
	LogDebug << "ARX_CHANGELEVEL_Change " << level << " " << target << " " << angle << " " << confirm;

	PROGRESS_BAR_TOTAL = 238; 
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;

	ARX_CHANGELEVEL_DesiredTime = ARX_TIME_Get();

	if (confirm)
	{
		if (!OKBox("Change Level ?", "CONFIRM")) return;
	}

	if (LAST_CHINSTANCE == -1)
	{
		ARX_CHANGELEVEL_CreateNewInstance();
	}
	else ARX_CHANGELEVEL_MakePath();

	FORBID_SAVE = 1;
	// CurGamePath contains current game temporary savepath
	char tex[256];
	sprintf(tex, "LEVEL%s", level.c_str());
	long num = GetLevelNumByName(tex);


	if ((FINAL_COMMERCIAL_DEMO)
			&&	(num != 10)
			&&	(num != 12)
			&&	(num != 15)
			&&	(num != 1))
		return;

	LoadLevelScreen(num);

	if (num == -1)
	{
		// fatality...
		LogWarning << "Internal Non-Fatal Error";
		return;
	}

	NEW_LEVEL = num;

	// TO RESTORE !!!!!!!!!!!!
	if (num == CURRENTLEVEL) // not changing level, just teleported
	{
		long t = GetTargetByNameTarget(target);

		if (t > 0)
		{
			EERIE_3D pos;

			if (inter.iobj[t])
				if (GetItemWorldPosition(inter.iobj[t], &pos))
				{
					moveto.x = player.pos.x = pos.x;
					moveto.y = player.pos.y = pos.y + PLAYER_BASE_HEIGHT;
					moveto.z = player.pos.z = pos.z;
					NO_PLAYER_POSITION_RESET = 1;
				}
		}

		player.desiredangle.b = player.angle.b = (float)angle;
		return; // nothing more to do :)
	}

	ARX_PLAYER_Reset_Fall();

	ARX_TIME_Pause();
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen(num);

	LogDebug << "Before ARX_CHANGELEVEL_PushLevel";
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, NEW_LEVEL);
	LogDebug << "After  ARX_CHANGELEVEL_PushLevel";


	LogDebug << "Before ARX_CHANGELEVEL_PopLevel";
	ARX_CHANGELEVEL_PopLevel(num, 1);
	LogDebug << "After  ARX_CHANGELEVEL_PopLevel";

	// Now restore player pos to destination
	long t = GetTargetByNameTarget(target);

	if (t > 0)
	{
		EERIE_3D pos;

		if (inter.iobj[t])
			if (GetItemWorldPosition(inter.iobj[t], &pos))
			{
				moveto.x = player.pos.x = pos.x;
				moveto.y = player.pos.y = pos.y + PLAYER_BASE_HEIGHT;
				moveto.z = player.pos.z = pos.z;
				NO_PLAYER_POSITION_RESET = 1;
				WILL_RESTORE_PLAYER_POSITION = moveto;
				WILL_RESTORE_PLAYER_POSITION_FLAG = 1;
			}
	}

	CURRENTLEVEL = NEW_LEVEL;
	player.desiredangle.b = player.angle.b = (float)angle;
	DONT_WANT_PLAYER_INZONE = 1;
	ARX_PLAYER_RectifyPosition();
	JUST_RELOADED = 1;
	NO_GMOD_RESET = 0;
	LogDebug << "-----------------------------------";
}

static long ARX_CHANGELEVEL_Push_Index(ARX_CHANGELEVEL_INDEX * asi, long num);

static long ARX_CHANGELEVEL_PushLevel(long num, long newnum) {
	
	LogDebug << "ARX_CHANGELEVEL_PushLevel " << num << " " << newnum;
	
	ARX_CHANGELEVEL_INDEX asi;
	ARX_SCRIPT_EventStackExecuteAll();

	// Close secondary inventory before leaving
	if (SecondaryInventory != NULL)
	{
		INTERACTIVE_OBJ * io = (INTERACTIVE_OBJ *)SecondaryInventory->io;

		if (io != NULL)
		{
			InventoryDir = -1;
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}

	ForcePlayerInventoryObjectLevel(newnum);

	char sfile[256];
	sprintf(sfile, "%sGsave.sav", CurGamePath);
	_pSaveBlock = new SaveBlock(sfile);

	if (!_pSaveBlock->BeginSave()) {
		LogError << "Error writing to save block.";
		return -1;
	}

	// Now we can save our things
	if (ARX_CHANGELEVEL_Push_Index(&asi, num) != 1)
	{
		LogError << "Error Saving Index...";
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_Globals() != 1)
	{
		LogError << "Error Saving Globals...";
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_Player() != 1)
	{
		LogError << "Error Saving Player...";
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_AllIO() != 1)
	{
		LogError << "Error Saving IOs...";
		ARX_TIME_UnPause();
		return -1;
	}

	if(!_pSaveBlock->flush()) {
		LogError << "could not complete the save.";
	}
	
	delete _pSaveBlock;
	_pSaveBlock = NULL;
	ARX_TIME_UnPause();
	return 1;
}
bool IsPlayerEquipedWith(INTERACTIVE_OBJ * io)
{
	if (!io) return false;

	long num = GetInterNum(io);

	if (io == CURRENT_TORCH)
		return true;

	if (ValidIONum(num))
	{
		for (long i = 0; i < MAX_EQUIPED; i++)
		{
			if (player.equiped[i] == num)
			{
				return true;
			}
		}
	}

	return false;
}
extern GLOBAL_MODS stacked;
extern GLOBAL_MODS current;
extern GLOBAL_MODS desired;
//--------------------------------------------------------------------------------------------
static long ARX_CHANGELEVEL_Push_Index(ARX_CHANGELEVEL_INDEX * asi, long num) {
	
	long pos = 0;

	asi->version				= ARX_GAMESAVE_VERSION;
	asi->nb_inter				= 0;
	asi->nb_paths				= nbARXpaths;
	asi->time					= ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long''
	asi->nb_lights				= 0;

	asi->gmods_stacked = stacked;
	asi->gmods_desired = desired;
	asi->gmods_current = current;

	for (long i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
				&&	(!(inter.iobj[i]->ioflags & IO_NOSAVE))
				&&	(!IsInPlayerInventory(inter.iobj[i]))
				&&	(!IsPlayerEquipedWith(inter.iobj[i]))
		   )
		{
			asi->nb_inter++;
		}
	}
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT * el = GLight[i];
		if(el && !(el->type & TYP_SPECIAL1)) {
			asi->nb_lights++;
		}
	}
	
	char savefile[256];
	sprintf(savefile, "lvl%03ld.sav", num);

	long allocsize = sizeof(ARX_CHANGELEVEL_INDEX)
					 + sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter
					 + sizeof(ARX_CHANGELEVEL_PATH) * asi->nb_paths
					 + sizeof(ARX_CHANGELEVEL_LIGHT) * asi->nb_lights;

	void * playlist = NULL;
	unsigned long asize = 0;
	ARX_SOUND_AmbianceSavePlayList(&playlist, &asize);
	allocsize += asize;

	char * dat = new char[allocsize];

	asi->ambiances_data_size = asize;
	memcpy(dat, asi, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);

	ARX_CHANGELEVEL_IO_INDEX aii;

	for (int i = 1; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
				&&	(!(inter.iobj[i]->ioflags & IO_NOSAVE))
				&&	(!IsInPlayerInventory(inter.iobj[i]))
				&&	(!IsPlayerEquipedWith(inter.iobj[i]))
		   )
		{
			strcpy(aii.filename, inter.iobj[i]->filename);
			aii.ident = inter.iobj[i]->ident;
			aii.level = inter.iobj[i]->level;
			aii.truelevel = inter.iobj[i]->truelevel;
			aii.num = i; // !!!
			aii.unused = 0;
			memcpy(dat + pos, &aii, sizeof(ARX_CHANGELEVEL_IO_INDEX));
			pos += sizeof(ARX_CHANGELEVEL_IO_INDEX);
		}
	}

	ARX_CHANGELEVEL_PATH * acp;

	for (int i = 0; i < nbARXpaths; i++)
	{
		acp = (ARX_CHANGELEVEL_PATH *)(dat + pos);
		memset(acp, 0, sizeof(ARX_CHANGELEVEL_PATH));
		strcpy(acp->name, ARXpaths[i]->name);
		strcpy(acp->controled, ARXpaths[i]->controled);
		pos += sizeof(ARX_CHANGELEVEL_PATH);
	}

	if (asi->ambiances_data_size > 0)
	{
		memcpy((char *)(dat + pos), playlist, asi->ambiances_data_size);
		pos += asi->ambiances_data_size;
		free(playlist);
	}

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * el = GLight[i];

		if ((el != NULL) && (!(el->type & TYP_SPECIAL1)))
		{
			ARX_CHANGELEVEL_LIGHT * acl = (ARX_CHANGELEVEL_LIGHT *)(dat + pos);
			memset(acl, 0, sizeof(ARX_CHANGELEVEL_LIGHT));

			acl->status = el->status;
			pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		}
	}
	
	bool ret = _pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	return ret ? 1 : -1;
}
//--------------------------------------------------------------------------------------------
static long ARX_CHANGELEVEL_Push_Globals() {
	
	ARX_CHANGELEVEL_SAVE_GLOBALS acsg;
	long pos = 0;

	memset(&acsg, 0, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	acsg.nb_globals = NB_GLOBALS;
	acsg.version = ARX_GAMESAVE_VERSION;

	char savefile[256];
	sprintf(savefile, "Globals.sav");

	long allocsize = sizeof(ARX_VARIABLE_SAVE) * acsg.nb_globals
					 + sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS) + 1000
					 + 48000;

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
	
	_pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	return 1;
}

//--------------------------------------------------------------------------------------------
void FillIOIdent(char * tofill, INTERACTIVE_OBJ * io)
{
	if ((!io)
			||	(!ValidIOAddress(io))
			||	(io->filename[0] == 0)
	   )
		strcpy(tofill, "NONE");
	else
		sprintf(tofill, "%s", io->long_name().c_str() );
}

extern long sp_max;
extern long cur_rf;
extern long cur_mx;
extern long cur_mr;
extern long cur_pom;
extern long sp_wep;
extern long sp_arm;

static long ARX_CHANGELEVEL_Push_Player() {
	
	ARX_CHANGELEVEL_PLAYER * asp;

	long allocsize = sizeof(ARX_CHANGELEVEL_PLAYER) + 48000;
	allocsize += Keyring.size() * 64;
	allocsize += 80 * PlayerQuest.size();
	allocsize += sizeof(SavedMapMakerData) * Mapmarkers.size();

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
	FillIOIdent(asp->curtorch, CURRENT_TORCH);

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

	std::copy(minimap, minimap + SAVED_MAX_MINIMAPS, asp->minimap);

	asp->falling = player.falling;
	asp->gold = player.gold;
	asp->invisibility = inter.iobj[0]->invisibility;

	if (player.inzone)
	{
		// TODO does this do anything?
		ARX_PATH * ap = (ARX_PATH *)asp->inzone;
		strcpy(asp->inzone, ap->name);
	}

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
	asp->Nb_Mapmarkers = Mapmarkers.size();

	asp->LAST_VALID_POS.x = LastValidPlayerPos.x;
	asp->LAST_VALID_POS.y = LastValidPlayerPos.y;
	asp->LAST_VALID_POS.z = LastValidPlayerPos.z;

	for (int i = 0; i < MAX_ANIMS; i++)
	{
		memset(&asp->anims[i], 0, 256);

		if (inter.iobj[0]->anims[i] != NULL)
		{
			strcpy(asp->anims[i], inter.iobj[0]->anims[i]->path);
		}
	}

	for (long k = 0; k < MAX_EQUIPED; k++)
	{
		if (ValidIONum(player.equiped[k])
				&&	(player.equiped[k] > 0))
			FillIOIdent(asp->equiped[k], inter.iobj[player.equiped[k]]);
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
	
	for(size_t i = 0; i < Mapmarkers.size(); i++) {
		SavedMapMakerData acmd = Mapmarkers[i];
		memcpy((char *)(dat + pos), &acmd, sizeof(SavedMapMakerData));
		pos += sizeof(SavedMapMakerData);
	}
	
	LastValidPlayerPos.x = asp->LAST_VALID_POS.x;
	LastValidPlayerPos.y = asp->LAST_VALID_POS.y;
	LastValidPlayerPos.z = asp->LAST_VALID_POS.z;
	
	_pSaveBlock->save("player.sav", dat, pos);
	
	delete[] dat;
	
	for (int i = 1; i < inter.nbmax; i++)
	{
		if ((IsInPlayerInventory(inter.iobj[i]))
				||	(IsPlayerEquipedWith(inter.iobj[i])))
			ARX_CHANGELEVEL_Push_IO(inter.iobj[i]);
	}

	return 1;
}

static long ARX_CHANGELEVEL_Push_AllIO() {
	
	for (long i = 1; i < inter.nbmax; i++)
	{

		if ((inter.iobj[i] != NULL)
				&&	(!(inter.iobj[i]->ioflags & IO_NOSAVE))
				&&	(!IsInPlayerInventory(inter.iobj[i]))
				&&	(!IsPlayerEquipedWith(inter.iobj[i]))
		   )
		{
			ARX_CHANGELEVEL_Push_IO(inter.iobj[i]);
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
INTERACTIVE_OBJ * GetObjIOSource(EERIE_3DOBJ * obj)
{
	if (!obj)
		return NULL;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i]) && (inter.iobj[i]->obj))
		{
			if (inter.iobj[i]->obj == obj)
				return inter.iobj[i];
		}
	}

	return NULL;
}

void FillTargetInfo(char * info, long numtarget) {
	if(numtarget == -2) strcpy(info, "SELF");
	else if(numtarget == -1) strcpy(info, "NONE");
	else if(numtarget == 0) strcpy(info, "PLAYER");
	else if(ValidIONum(numtarget))
		FillIOIdent(info, (INTERACTIVE_OBJ *)inter.iobj[numtarget]);
	else strcpy(info, "NONE");
}

// num = num in index file
//-----------------------------------------------------------------------------
static long ARX_CHANGELEVEL_Push_IO(const INTERACTIVE_OBJ * io) {
	
	// Check Valid IO
	if (!io)
		return -1;

	// Sets Savefile Name
	char savefile[256];
	sprintf(savefile, "%s_%04ld.sav", GetName(io->filename).c_str(), io->ident);

	// Define Type & Affiliated Structure Size
	long type;
	long struct_size;

	if (io->ioflags & IO_NPC)
	{
		type = TYPE_NPC;
		struct_size	= sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
	}
	else if (io->ioflags & IO_ITEM)
	{
		type = TYPE_ITEM;
		struct_size	= sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
	}
	else if (io->ioflags & IO_FIX)
	{
		type = TYPE_FIX;
		struct_size	= sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
	}
	else if (io->ioflags & IO_CAMERA)
	{
		type = TYPE_CAMERA;
		struct_size	= sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
	}
	else
	{
		type = TYPE_MARKER;
		struct_size	= sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
	}

	// Init Changelevel Main IO Save Structure
	ARX_CHANGELEVEL_IO_SAVE ais;
	memset(&ais, 0, sizeof(ARX_CHANGELEVEL_IO_SAVE));

	// Store IO Data in Main IO Save Structure
	ais.version = ARX_GAMESAVE_VERSION;
	ais.savesystem_type = type;
	ais.saveflags = 0;
	strcpy(ais.filename, io->filename);
	ais.ident = io->ident;
	ais.ioflags = io->ioflags;

	if ((ais.ioflags & IO_FREEZESCRIPT)
			&& ((ARX_SPELLS_GetSpellOn(io, SPELL_PARALYSE) >= 0)
				 || (ARX_SPELLS_GetSpellOn(io, SPELL_MASS_PARALYSE) >= 0)))
		ais.ioflags &= ~IO_FREEZESCRIPT;

	ais.pos = io->pos;

	if ((io->obj)
			&&	(io->obj->pbox)
			&&	(io->obj->pbox->active))
		ais.pos.y -= io->obj->pbox->vert[0].initpos.y;

	ais.lastpos = io->lastpos;
	ais.initpos = io->initpos;
	ais.initangle = io->initangle;
	ais.move = io->move;
	ais.lastmove = io->lastmove;
	ais.angle = io->angle;
	ais.scale = io->scale;
	ais.weight = io->weight;
	strcpy(ais.locname, io->locname);
	ais.EditorFlags = io->EditorFlags;
	ais.GameFlags = io->GameFlags;

	if(io == inter.iobj[0])
		ais.GameFlags &= ~GFLAG_INVISIBILITY;

	ais.material = io->material;
	ais.level = io->level;
	ais.truelevel = io->truelevel;
	ais.scriptload = io->scriptload;
	ais.show = io->show;
	ais.collision = io->collision;
	strcpy(ais.mainevent, io->mainevent);
	ais.velocity = io->velocity;
	ais.stopped = io->stopped;
	ais.basespeed = io->basespeed;
	ais.speed_modif = io->speed_modif;
	ais.frameloss = io->frameloss;
	ais.rubber = io->rubber;
	ais.max_durability = io->max_durability;
	ais.durability = io->durability;
	ais.poisonous = io->poisonous;
	ais.poisonous_count = io->poisonous_count;
	ais.head_rot = io->head_rot;
	ais.damager_damages = io->damager_damages;
	ais.nb_iogroups = io->nb_iogroups;
	ais.damager_type = io->damager_type;
	ais.type_flags = io->type_flags;
	ais.secretvalue = io->secretvalue;
	ais.shop_multiply = io->shop_multiply;
	ais.aflags = io->aflags;
	ais.original_height = io->original_height;
	ais.original_radius = io->original_radius;
	ais.ignition = io->ignition;

	if (io->usepath)
	{
		ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

		ais.usepath_aupflags = aup->aupflags;


		float ulCurTime = aup->_curtime;
		ARX_CHECK_ULONG(ulCurTime);
		ais.usepath_curtime = ARX_CLEAN_WARN_CAST_ULONG(ulCurTime) ;


		ais.usepath_initpos = aup->initpos;
		ais.usepath_lastWP = aup->lastWP;


		float ulStartTime = aup->_starttime;
		ARX_CHECK_ULONG(ulStartTime);
		ais.usepath_starttime = ARX_CLEAN_WARN_CAST_ULONG(ulStartTime);


		strcpy(ais.usepath_name, aup->path->name);
	}


	if (io->shop_category)
		strcpy(ais.shop_category, io->shop_category);
	else
		ais.shop_category[0] = 0;

	memset(ais.inventory_skin, 0, 128);

	if (io->inventory_skin)
		strcpy(ais.inventory_skin, io->inventory_skin);

	memset(ais.stepmaterial, 0, 128);

	if (io->stepmaterial)
		strcpy(ais.stepmaterial, io->stepmaterial);

	memset(ais.armormaterial, 0, 128);

	if (io->armormaterial)
		strcpy(ais.armormaterial, io->armormaterial);

	memset(ais.weaponmaterial, 0, 128);

	if (io->weaponmaterial)
		strcpy(ais.weaponmaterial, io->weaponmaterial);

	memset(ais.strikespeech, 0, 128);

	if (io->strikespeech)
		strcpy(ais.strikespeech, io->strikespeech);

	ais.nb_linked = 0;
	memset(&ais.linked_data, 0, sizeof(IO_LINKED_DATA)*MAX_LINKED_SAVE);

	// Save Animations
	for (long i = 0; i < MAX_ANIMS; i++)
	{
		memset(&ais.anims[i], 0, 256);

		if (io->anims[i] != NULL)
		{
			strcpy(ais.anims[i], io->anims[i]->path);
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
				ais.linked_data[count].modinfo = io->obj->linked[count].modinfo;
				FillIOIdent(ais.linked_data[count].linked_id, (INTERACTIVE_OBJ *)GetObjIOSource((EERIE_3DOBJ *)io->obj->linked[count].obj));
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
		ais.system_flags   |= SYSTEM_FLAG_INVENTORY;
	}

	if (type == TYPE_ITEM)
	{
		if (io->_itemdata->equipitem)
		{
			ais.system_flags   |= SYSTEM_FLAG_EQUIPITEMDATA;
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
		+ sizeof(SavedGroupData) * io->nb_iogroups
		+ sizeof(SavedTweakInfo) * io->Tweak_nb
		+ 48000;

	// Allocate Main Save Buffer
	char * dat = new char[allocsize];
	long pos = 0;

	ais.halo = io->halo_native;
	ais.Tweak_nb = io->Tweak_nb;
	memcpy(dat, &ais, sizeof(ARX_CHANGELEVEL_IO_SAVE));
	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);

	long timm = ARXTimeUL(); //treat warning C4244 conversion from 'float' to 'unsigned long''

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

					if (avs->fval > 0)
					{

						ARX_CHECK_SIZET(avs->fval);
						memset(dat + pos, 0, ARX_CLEAN_WARN_CAST_SIZET(avs->fval)); //count+1);


						if (count > 0)
							memcpy(dat + pos, io->script.lvar[i].text, count);

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

					if (avs->fval > 0)
					{

						ARX_CHECK_SIZET(avs->fval);
						memset(dat + pos, 0, ARX_CLEAN_WARN_CAST_SIZET(avs->fval)); //count+1);


						if (count > 0)
							memcpy(dat + pos, io->over_script.lvar[i].text, count); //+1);
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

			if (io->_npcdata->life > 0.f)
				FillIOIdent(as->id_weapon, (INTERACTIVE_OBJ *)io->_npcdata->weapon);
			else
				as->id_weapon[0] = 0;

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
			strcpy(as->weaponname, io->_npcdata->weaponname);
			as->weapontype = io->_npcdata->weapontype;
			as->xpvalue = io->_npcdata->xpvalue;
			
			assert(SAVED_MAX_STACKED_BEHAVIOR == MAX_STACKED_BEHAVIOR);
			std::copy(io->_npcdata->stacked, io->_npcdata->stacked + SAVED_MAX_STACKED_BEHAVIOR, as->stacked);

			for (size_t i = 0; i < SAVED_MAX_STACKED_BEHAVIOR; i++) {
				if(io->_npcdata->stacked[i].exist) {
					FillTargetInfo(as->stackedtarget[i], io->_npcdata->stacked[i].target);
				} else {
					strcpy(as->stackedtarget[i], "NONE");
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

			as->blood_color = io->_npcdata->blood_color;
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

		INVENTORY_DATA * inv = (INVENTORY_DATA *)io->inventory;
		FillIOIdent(aids->io, (INTERACTIVE_OBJ *)inv->io);
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

	for(long ii = 0; ii < io->nb_iogroups; ii++) {
		SavedGroupData sgd = io->iogroups[ii];
		memcpy(dat + pos, &sgd, sizeof(SavedGroupData));
		pos += sizeof(SavedGroupData);
	}

	for(int ii = 0; ii < io->Tweak_nb; ii++) {
		SavedTweakInfo sti = io->Tweaks[ii];
		memcpy(dat + pos, &sti, sizeof(SavedTweakInfo));
		pos += sizeof(SavedTweakInfo);
	}

	if((pos > allocsize)) {
		LogError << "SaveBuffer Overflow " << pos << " >> " << allocsize;
	}
	
	_pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	return 1;
}

static long ARX_CHANGELEVEL_Pop_Index(ARX_CHANGELEVEL_INDEX * asi, long num) {
	
	long pos = 0;
	std::string loadfile;
	std::stringstream ss;

	ss << "lvl" << std::setfill('0') << std::setw(3) << num << ".sav";
	loadfile = ss.str();

	size_t size;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	// TODO size is not used

	memcpy(asi, dat, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);

	if (asi->nb_inter)
	{
		idx_io = (ARX_CHANGELEVEL_IO_INDEX *) malloc(sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter);

		memcpy(idx_io, dat + pos, sizeof(ARX_CHANGELEVEL_IO_INDEX)*asi->nb_inter);
		pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	}
	else idx_io = NULL;

	// Skip Path info (used later !)
	pos += sizeof(ARX_CHANGELEVEL_PATH) * asi->nb_paths;

	// Restore Ambiances
	if (asi->ambiances_data_size)
	{
		void * playlist = (void *)(dat + pos);
		pos += asi->ambiances_data_size;

		ARX_SOUND_AmbianceRestorePlayList(playlist, asi->ambiances_data_size);
	}

	free(dat);
	return 1;
}
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Pop_Zones_n_Lights(ARX_CHANGELEVEL_INDEX * asi, long num)
{
	
	long pos = 0;
	std::string loadfile;
	std::stringstream ss;
	

	ss << "lvl" << std::setfill('0') << std::setw(3) << num << ".sav";
	loadfile = ss.str();
	
	size_t size;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	// TODO size not used

	// Skip Changelevel Index
	pos += sizeof(ARX_CHANGELEVEL_INDEX);
	// Skip Inter idx
	pos += sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter;
	// Now Restore Paths
	ARX_CHANGELEVEL_PATH * acp;

	for (int i = 0; i < asi->nb_paths; i++)
	{
		acp = (ARX_CHANGELEVEL_PATH *)(dat + pos);
		ARX_PATH * ap = ARX_PATH_GetAddressByName(acp->name);

		if (ap)
		{
			if (acp->controled[0] == 0)
				ap->controled[0] = 0;
			else
			{
				strcpy(ap->controled, acp->controled);
			}
		}

		pos += sizeof(ARX_CHANGELEVEL_PATH);
	}

	if (asi->ambiances_data_size > 0)
	{
		pos += asi->ambiances_data_size;
	}

	for (int i = 0; i < asi->nb_lights; i++)
	{
		ARX_CHANGELEVEL_LIGHT * acl = (ARX_CHANGELEVEL_LIGHT *)(dat + pos);
		pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		long count = 0;

		for(size_t j = 0; j < MAX_LIGHTS; j++) {
			
			EERIE_LIGHT * el = GLight[j];

			if ((el != NULL) && (!(el->type & TYP_SPECIAL1)))
			{
				if (count == i)
				{
					el->status = acl->status;
					j = MAX_LIGHTS + 1;
				}

				count++;
			}
		}
	}

	free(dat);
	return 1;
}
extern long NO_GMOD_RESET;
extern long FOR_EXTERNAL_PEOPLE;
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Pop_Level(ARX_CHANGELEVEL_INDEX * asi, long num, long FirstTime)
{
	char tex[256];
	char postxt[256];
	char lev[256];
	char ftemp[256];
	strcpy(postxt, "NONE");
	GetLevelNameByNum(num, lev);

	if (!FirstTime)
	{
		DONT_LOAD_INTERS = 1;
	}
	else
	{
		DONT_LOAD_INTERS = 0;
	}

	LOAD_N_DONT_ERASE = 1;

	sprintf(tex, "LEVEL%s", lev);
	sprintf(ftemp, "Graph\\Levels\\%s\\%s.DLF", tex, tex);

	if (!PAK_FileExist(ftemp))
	{
		LogError << "Unable To Find " << ftemp;
		return 0;
	}

	LoadLevelScreen(num);
	SetEditMode(1, false);

	if (ARX_CHANGELEVEL_Pop_Globals() != 1)
	{
		LogWarning << "Cannot Load Globals data";
	}

	DanaeLoadLevel(ftemp);
	CleanScriptLoadedIO();

	FirstFrame = 1;
	DONT_LOAD_INTERS = 0;

	if (FirstTime)
	{
		RestoreInitialIOStatus();
 

		for (long i = 1; i < inter.nbmax; i++)
		{
			if (inter.iobj[i] != NULL)
			{
				if (!inter.iobj[i]->scriptload)
					ARX_SCRIPT_Reset(inter.iobj[i], 1);
			}
		}

		EDITMODE = 0;
		BLOCK_PLAYER_CONTROLS = 0;
		ARX_INTERFACE_Reset();
		EERIE_ANIMMANAGER_PurgeUnused();
	}
	else
	{
		EDITMODE = 0;
		BLOCK_PLAYER_CONTROLS = 0;
		ARX_INTERFACE_Reset();
		EERIE_ANIMMANAGER_PurgeUnused();
	}


	stacked = asi->gmods_stacked;
	desired = asi->gmods_desired;
	current = asi->gmods_current;
	NO_GMOD_RESET = 1;
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);//asi.time);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	return 1;
}

static long ARX_CHANGELEVEL_Pop_Player(long instance) {
	
	ARX_CHANGELEVEL_PLAYER asp;
	
	const string & loadfile = "player.sav";
	
	size_t size;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	memcpy(&asp, dat, sizeof(ARX_CHANGELEVEL_PLAYER));
	
	player.AimTime = asp.AimTime;
	player.angle = asp.angle;
	player.desiredangle.a = player.angle.a;
	player.desiredangle.b = player.angle.b;
	
	ARX_CHECK_UCHAR(asp.armor_class);
	player.armor_class = static_cast<unsigned char>(asp.armor_class);
	
	player.Attribute_Constitution = asp.Attribute_Constitution;
	player.Attribute_Dexterity = asp.Attribute_Dexterity;
	player.Attribute_Mind = asp.Attribute_Mind;
	player.Attribute_Strength = asp.Attribute_Strength;
	player.Critical_Hit = asp.Critical_Hit;
	player.Current_Movement = Flag(asp.Current_Movement); // TODO save/load flags
	player.damages = asp.damages;
	player.doingmagic = asp.doingmagic;
	player.playerflags = Flag(asp.playerflags); // TODO save/load flags
	
	if(asp.TELEPORT_TO_LEVEL[0]) {
		strcpy(TELEPORT_TO_LEVEL, asp.TELEPORT_TO_LEVEL);
	} else {
		memset(TELEPORT_TO_LEVEL, 0, 64);
	}
	
	if(asp.TELEPORT_TO_POSITION[0]) {
		strcpy(TELEPORT_TO_POSITION, asp.TELEPORT_TO_POSITION);
	} else {
		memset(TELEPORT_TO_POSITION, 0, 64);
	}
	
	TELEPORT_TO_ANGLE = asp.TELEPORT_TO_ANGLE;
	CHANGE_LEVEL_ICON = asp.CHANGE_LEVEL_ICON;
	player.bag = asp.bag;
	assert(SAVED_MAX_PRECAST == MAX_PRECAST);
	std::copy(asp.precast, asp.precast + SAVED_MAX_PRECAST, Precast);
	player.Interface = asp.Interface;
	player.Interface &= ~INTER_MAP;
	player.falling = asp.falling;
	player.gold = asp.gold;
	inter.iobj[0]->invisibility = asp.invisibility;
	player.inzone = ARX_PATH_GetAddressByName(asp.inzone);;
	player.jumpphase = asp.jumpphase;
	player.jumpstarttime = asp.jumpstarttime;
	player.Last_Movement = Flag(asp.Last_Movement); // TODO save/load flags
	
	ARX_CHECK_UCHAR(asp.level);
	player.level = static_cast<unsigned char>(asp.level);
	
	player.life = asp.life;
	player.mana = asp.mana;
	player.maxlife = asp.maxlife;
	player.maxmana = asp.maxmana;
	
	player.onfirmground = (asp.misc_flags & 1) ? 1 : 0;
	WILLRETURNTOCOMBATMODE = (asp.misc_flags & 2) ? 1 : 0;
	
	player.physics = asp.physics;
	player.poison = asp.poison;
	player.hunger = asp.hunger;
	player.pos = asp.pos;
	
	if(asp.sp_flags & SP_ARM1) {
		sp_arm = 1;
	} else if(asp.sp_flags & SP_ARM2) {
		sp_arm = 2;
	} else if(asp.sp_flags & SP_ARM3) {
		sp_arm = 3;
	} else {
		sp_arm = 0;
	}
	
	if(asp.sp_flags & SP_MAX) {
		cur_mx = 3;
		sp_max = 1;
	} else {
		cur_mx = 0;
		sp_max = 0;
	}
	
	cur_mr = (asp.sp_flags & SP_MR) ? 3 : 0;
	cur_rf = (asp.sp_flags & SP_RF) ? 3 : 0;
	
	if(asp.sp_flags & SP_WEP) {
		cur_pom = 3;
		sp_wep = 1;
	} else {
		cur_pom = 0;
		sp_wep = 0;
	}
	
	if(inter.iobj[0]) {
		inter.iobj[0]->pos = player.pos;
		inter.iobj[0]->pos.y += 170.f;
	}
	
	WILL_RESTORE_PLAYER_POSITION = asp.pos;
	WILL_RESTORE_PLAYER_POSITION_FLAG = 1;
	
	ARX_CHECK_UCHAR(asp.resist_magic);
	ARX_CHECK_UCHAR(asp.resist_poison);
	player.resist_magic = static_cast<unsigned char>(asp.resist_magic);
	player.resist_poison = static_cast<unsigned char>(asp.resist_poison);
	
	ARX_CHECK_UCHAR(asp.Attribute_Redistribute);
	ARX_CHECK_UCHAR(asp.Skill_Redistribute);
	player.Attribute_Redistribute = static_cast<unsigned char>(asp.Attribute_Redistribute);
	player.Skill_Redistribute = static_cast<unsigned char>(asp.Skill_Redistribute);
	
	player.rune_flags = Flag(asp.rune_flags); // TODO save/load flags
	player.size = asp.size;
	player.Skill_Stealth = asp.Skill_Stealth;
	player.Skill_Mecanism = asp.Skill_Mecanism;
	player.Skill_Intuition = asp.Skill_Intuition;
	player.Skill_Etheral_Link = asp.Skill_Etheral_Link;
	player.Skill_Object_Knowledge = asp.Skill_Object_Knowledge;
	player.Skill_Casting = asp.Skill_Casting;
	player.Skill_Projectile = asp.Skill_Projectile;
	player.Skill_Close_Combat = asp.Skill_Close_Combat;
	player.Skill_Defense = asp.Skill_Defense;
	
	ARX_CHECK_CHAR(asp.skin);
	player.skin = static_cast<char>(asp.skin);
	
	player.xp = asp.xp;
	GLOBAL_MAGIC_MODE = asp.Global_Magic_Mode;
	
	ARX_MINIMAP_PurgeTC();
	assert(SAVED_MAX_MINIMAPS == MAX_MINIMAPS);
	std::copy(asp.minimap, asp.minimap + SAVED_MAX_MINIMAPS, minimap);
	
	INTERACTIVE_OBJ & io = *inter.iobj[0];
	assert(SAVED_MAX_ANIMS == MAX_ANIMS);
	for(size_t i = 0; i < SAVED_MAX_ANIMS; i++) {
		if(io.anims[i] != NULL) {
			ReleaseAnimFromIO(&io, i);
		}
		if(asp.anims[i][0]) {
			io.anims[i] = EERIE_ANIMMANAGER_Load(asp.anims[i]);
		}
	}
	
	assert(SAVED_INVENTORY_Y == INVENTORY_Y);
	assert(SAVED_INVENTORY_X == INVENTORY_X);
	for(size_t iNbBag = 0; iNbBag < 3; iNbBag++) {
		for(size_t m = 0; m < SAVED_INVENTORY_Y; m++) {
			for (size_t n = 0; n < SAVED_INVENTORY_X; n++) {
				inventory[iNbBag][n][m].io = ConvertToValidIO(asp.id_inventory[iNbBag][n][m]);
				inventory[iNbBag][n][m].show = asp.inventory_show[iNbBag][n][m];
			}
		}
	}
	
	size_t pos = sizeof(ARX_CHANGELEVEL_PLAYER);
	
	ARX_PLAYER_Quest_Init();
	for(int i = 0; i < asp.nb_PlayerQuest; i++) {
		ARX_PLAYER_Quest_Add((char *)(dat + pos), true);
		pos += 80;
	}
	
	ARX_KEYRING_Init();
	LogDebug << asp.keyring_nb;
	for(int i = 0; i < asp.keyring_nb; i++) {
		ARX_KEYRING_Add((char *)(dat + pos));
		pos += SAVED_KEYRING_SLOT_SIZE;
	}
	
	ARX_MAPMARKER_Init();
	Mapmarkers.resize(asp.Nb_Mapmarkers);
	for(int i = 0; i < asp.Nb_Mapmarkers; i++) {
		SavedMapMakerData acmd;
		memcpy(&acmd, dat + pos, sizeof(SavedMapMakerData));
		pos += sizeof(SavedMapMakerData);
		ARX_MAPMARKER_Add(acmd.x, acmd.y, acmd.lvl, acmd.string);
	}
	
	ARX_PLAYER_Restore_Skin();
	
	ARX_PLAYER_Modify_XP(0);	 
	
	free(dat);
	
	PROGRESS_BAR_COUNT += 10.f;
	LoadLevelScreen();
	
	// Restoring Player equipment...
	player.equipsecondaryIO = ConvertToValidIO(asp.equipsecondaryIO);
	player.equipshieldIO = ConvertToValidIO(asp.equipshieldIO);
	player.leftIO = ConvertToValidIO(asp.leftIO);
	player.rightIO = ConvertToValidIO(asp.rightIO);
	CURRENT_TORCH = ConvertToValidIO(asp.curtorch);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	
	assert(SAVED_MAX_EQUIPED == MAX_EQUIPED);
	for(size_t i = 0; i < SAVED_MAX_EQUIPED; i++) {
		player.equiped[i] = (short)GetInterNum(ConvertToValidIO(asp.equiped[i]));
		if(player.equiped[i] > 0 && ValidIONum(player.equiped[i])) {
			inter.iobj[player.equiped[i]]->level = (short)instance;
		} else {
			player.equiped[i] = 0;
		}
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	
	return 1;
}

//-----------------------------------------------------------------------------
void ReleaseTio()
{
	if (tio)
	{
		free(tio);
		tio = NULL;
	}

	nb_tio = 0;
}
extern long ARX_NPC_ApplyCuts(INTERACTIVE_OBJ * io);

//-----------------------------------------------------------------------------
static long ARX_CHANGELEVEL_Pop_IO(const string & ident) {
	
	if(!strcasecmp(ident, "NONE")) {
		return -1;
	}
	
	char loadfile[256];
	ARX_CHANGELEVEL_IO_SAVE * ais;
	long pos = 0;
	
	sprintf(loadfile, "%s.sav", ident.c_str());

	long t = GetTargetByNameTarget(ident);

	if (ValidIONum(t))
		return t;

	if (NEED_LOG)
	{
		LogDebug << "--> Before ARX_CHANGELEVEL_Pop_IO(" << ident << ")";
	}

	size_t size = 0;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	
	// TODO size not used

	ais = (ARX_CHANGELEVEL_IO_SAVE *)dat;

	if (ais->version != ARX_GAMESAVE_VERSION)
	{
		free(dat);

		LogError << "Invalid PopIO version";

		return -1;
	}

	pos += sizeof(ARX_CHANGELEVEL_IO_SAVE);

	if ((ais->show == SHOW_FLAG_DESTROYED)
			||	(ais->ioflags & IO_NOSAVE))
	{
		free(dat);
		return -1;
	}

	if (NEED_LOG)
	{
		LogDebug << "--> Phase2 ARX_CHANGELEVEL_Pop_IO(" << ident << ")";
	}

	long num = atoi(ident.c_str() + ident.length() - 4); // TODO this is ugly
	INTERACTIVE_OBJ * tmp = LoadInter_Ex(ais->filename, num, ais->pos, ais->angle, MSP);
	long idx = -1;
	INTERACTIVE_OBJ * io = NULL;

	if (tmp)
	{
		io = tmp;
		idx = GetInterNum(io);

		long  Gaids_Number = idx;
		_Gaids[Gaids_Number] = (ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *) malloc(sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));

		memset(_Gaids[Gaids_Number], 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));

		io->room_flags = 1;
		io->room = -1;
		io->no_collide = -1;
		io->ioflags = ais->ioflags;

		io->ioflags &= ~IO_FREEZESCRIPT;
		io->pos = ais->pos;
		io->lastpos = ais->lastpos;
		io->move = ais->move;
		io->lastmove = ais->lastmove;
		io->initpos = ais->initpos;
		io->initangle = ais->initangle;
		io->angle = ais->angle;
		io->scale = ais->scale;
		io->weight = ais->weight;
		strcpy(io->locname, ais->locname);
		io->EditorFlags = ais->EditorFlags;
		io->GameFlags = ais->GameFlags;
		io->material = ais->material;
		io->level = ais->level;
		io->truelevel = ais->truelevel;

		// Script data
		io->scriptload = ais->scriptload;
		io->show = ais->show;
		io->collision = ais->collision;
		strcpy(io->mainevent, ais->mainevent);

		// Physics data
		io->velocity = ais->velocity;
		io->stopped = ais->stopped;
		io->basespeed = 1; 
		io->speed_modif = 0.f; 
		io->frameloss = 0; 
		io->rubber = ais->rubber;
		io->max_durability = ais->max_durability;
		io->durability = ais->durability;
		io->poisonous = ais->poisonous;
		io->poisonous_count = ais->poisonous_count;
		io->head_rot = ais->head_rot;
		io->damager_damages = ais->damager_damages;
		io->nb_iogroups = ais->nb_iogroups;
		io->damager_type = Flag(ais->damager_type); // TODO save/load flags
		io->type_flags = Flag(ais->type_flags); // TODO save/load flags
		io->secretvalue = ais->secretvalue;
		io->shop_multiply = ais->shop_multiply;
		io->aflags = ais->aflags;
		io->original_height = ais->original_height;
		io->original_radius = ais->original_radius;
		io->ignition = ais->ignition;

		if (ais->system_flags & SYSTEM_FLAG_USEPATH)
		{
			io->usepath = (void *)malloc(sizeof(ARX_USE_PATH));
			ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

			aup->aupflags = Flag(ais->usepath_aupflags); // TODO save/load flags
			aup->_curtime = ARX_CLEAN_WARN_CAST_FLOAT(ais->usepath_curtime);
			aup->initpos = ais->usepath_initpos;
			aup->lastWP = ais->usepath_lastWP;
			aup->_starttime = ARX_CLEAN_WARN_CAST_FLOAT(ais->usepath_starttime);
			ARX_PATH * ap = ARX_PATH_GetAddressByName(ais->usepath_name);
			aup->path = ap;
		}

		if (ais->shop_category[0])
			io->shop_category = strdup(ais->shop_category);
		else
			io->shop_category = NULL;

		io->halo_native = ais->halo;
		io->halo_native.dynlight = -1;
		io->halo.dynlight = -1;
		ARX_HALO_SetToNative(io);

		if (ais->inventory_skin[0])
		{
			io->inventory_skin = (char *) malloc(strlen(ais->inventory_skin) + 1);

			strcpy(io->inventory_skin, ais->inventory_skin);
		}
		else io->stepmaterial = NULL;

		if (ais->stepmaterial[0])
		{
			io->stepmaterial = (char *) malloc(strlen(ais->stepmaterial) + 1);

			strcpy(io->stepmaterial, ais->stepmaterial);
		}
		else io->stepmaterial = NULL;

		if (ais->armormaterial[0])
		{
			io->armormaterial = (char *) malloc(strlen(ais->armormaterial) + 1);

			strcpy(io->armormaterial, ais->armormaterial);
		}
		else io->armormaterial = NULL;

		if (ais->weaponmaterial[0])
		{
			io->weaponmaterial = (char *) malloc(strlen(ais->weaponmaterial) + 1);

			strcpy(io->weaponmaterial, ais->weaponmaterial);
		}
		else io->weaponmaterial = NULL;

		if (ais->strikespeech[0])
		{
			io->strikespeech = (char *) malloc(strlen(ais->strikespeech) + 1);

			strcpy(io->strikespeech, ais->strikespeech);
		}
		else io->strikespeech = NULL;


		for (long i = 0; i < MAX_ANIMS; i++)
		{
			if (io->anims[i] != NULL)
			{
				ReleaseAnimFromIO(io, i);
			}

			if (ais->anims[i][0])
			{
				char tex[256];

				if (strlen(ais->anims[i]) > 256)
				{
					continue;
				}

				io->anims[i] = EERIE_ANIMMANAGER_Load(ais->anims[i]);

				if (io->anims[i] == NULL)
				{
					if (io->ioflags & IO_NPC)
						sprintf(tex, "GRAPH\\OBJ3D\\ANIMS\\NPC\\%s%s", GetName(ais->anims[i]).c_str(), GetExt(ais->anims[i]).c_str());
					else
						sprintf(tex, "GRAPH\\OBJ3D\\ANIMS\\FIX_Inter\\%s%s", GetName(ais->anims[i]).c_str(), GetExt(ais->anims[i]).c_str());

					io->anims[i] = EERIE_ANIMMANAGER_Load(tex);
				}
			}
		}

		io->spellcast_data = ais->spellcast_data;
		io->physics = ais->physics;
		assert(SAVED_MAX_ANIM_LAYERS == MAX_ANIM_LAYERS);
		std::copy(ais->animlayer, ais->animlayer + SAVED_MAX_ANIM_LAYERS, io->animlayer);

		for (long k = 0; k < MAX_ANIM_LAYERS; k++)
		{
			long nn = (long)ais->animlayer[k].cur_anim;

			if (nn == -1) io->animlayer[k].cur_anim = NULL;
			else io->animlayer[k].cur_anim = (ANIM_HANDLE *)io->anims[nn];

			nn = (long)ais->animlayer[k].next_anim;

			if (nn == -1) io->animlayer[k].next_anim = NULL;
			else io->animlayer[k].next_anim = (ANIM_HANDLE *)io->anims[nn];
		}

		// Target Info
		strcpy(_Gaids[Gaids_Number]->targetinfo, ais->id_targetinfo);

		ARX_SCRIPT_Timer_Clear_By_IO(io);

		for (int i = 0; i < ais->nbtimers; i++)
		{
			ARX_CHANGELEVEL_TIMERS_SAVE * ats = (ARX_CHANGELEVEL_TIMERS_SAVE *)(dat + pos);

			ARX_CHECK_SHORT(ats->flags);
			short sFlags = ARX_CLEAN_WARN_CAST_SHORT(ats->flags);


			long num = ARX_SCRIPT_Timer_GetFree();

			if (num != -1)
			{
				ActiveTimers++;

				if (ats->script)
					scr_timer[num].es = &io->over_script;
				else
					scr_timer[num].es = &io->script;

				scr_timer[num].flags = sFlags;
				scr_timer[num].exist = 1;
				scr_timer[num].io = io;
				scr_timer[num].msecs = ats->msecs;
				scr_timer[num].name = ats->name;
				scr_timer[num].pos = ats->pos;


				float tt = ARX_CHANGELEVEL_DesiredTime + ats->tim;

				if (tt < 0) scr_timer[num].tim = 0; //;
				else
				{
					ARX_CHECK_ULONG(tt);
					scr_timer[num].tim = ARX_CLEAN_WARN_CAST_ULONG(tt) ;
				}

				scr_timer[num].times = ats->times;

			}

			pos += sizeof(ARX_CHANGELEVEL_TIMERS_SAVE);
		}




		//////////////////
		ARX_CHANGELEVEL_SCRIPT_SAVE * ass = (ARX_CHANGELEVEL_SCRIPT_SAVE *)(dat + pos);

		io->script.allowevents = Flag(ass->allowevents); // TODO save/load flags
		io->script.nblvar = 0;

		if (io->script.lvar)
		{
			free(io->script.lvar);
			io->script.lvar = NULL;
		}

		if (ass->nblvar > 0)
		{
			io->script.lvar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * ass->nblvar);
		}

		else io->script.lvar = NULL;

		io->script.nblvar = ass->nblvar;
		long ERRCOUNT = 0;

		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);

		for (int i = 0; i < ass->nblvar; i++)
		{
			ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
			memset(&io->script.lvar[i], 0, sizeof(SCRIPT_VAR)); 
		retry:
			;

			switch (avs->type)
			{
				case TYPE_L_TEXT:
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = avs->fval;
					io->script.lvar[i].ival = avs->fval;
					io->script.lvar[i].type = TYPE_L_TEXT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if (io->script.lvar[i].ival)
					{
						io->script.lvar[i].text = (char *) malloc(io->script.lvar[i].ival + 1);

						memset(io->script.lvar[i].text, 0, io->script.lvar[i].ival + 1);
						memcpy(io->script.lvar[i].text, dat + pos, io->script.lvar[i].ival);
						pos += io->script.lvar[i].ival;
						io->script.lvar[i].ival = strlen(io->script.lvar[i].text) + 1;

						if (io->script.lvar[i].text[0] == '\xCC')
							io->script.lvar[i].text[0] = 0;
					}
					else
					{
						io->script.lvar[i].text = NULL;
						io->script.lvar[i].ival = 0;
					}

					break;
				case TYPE_L_LONG:
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = avs->fval;
					io->script.lvar[i].ival = avs->fval;
					io->script.lvar[i].type = TYPE_L_LONG;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				case TYPE_L_FLOAT:
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = avs->fval;
					io->script.lvar[i].ival = avs->fval;
					io->script.lvar[i].type = TYPE_L_FLOAT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				default:

					if ((avs->name[0] == '$') || (avs->name[0] == '\xA3'))
					{
						avs->type = TYPE_L_TEXT;
						goto retry;
					}

		if ((avs->name[0] == '#') || (avs->name[0] == '\xA7'))
					{
						avs->type = TYPE_L_LONG;
						goto retry;
					}

					if ((avs->name[0] == '&') || (avs->name[0] == '@'))
					{
						avs->type = TYPE_L_FLOAT;
						goto retry;
					}

					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = 0;
					io->script.lvar[i].ival = 0;
					io->script.lvar[i].type = TYPE_L_LONG;
					ERRCOUNT++;
					ass->nblvar = i;

					goto corrupted;
				
					break;
			}
		}


		ass = (ARX_CHANGELEVEL_SCRIPT_SAVE *)(dat + pos);

		io->over_script.allowevents = Flag(ass->allowevents); // TODO save/load flags

		io->over_script.nblvar = 0; 

		if (io->over_script.lvar)
		{
			free(io->over_script.lvar);
			io->over_script.lvar = NULL;
		}

		if (ass->nblvar)
		{
			io->over_script.lvar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * ass->nblvar);
		}
		//"Script Var"
		else io->over_script.lvar = NULL;

		io->over_script.nblvar = ass->nblvar;

		pos += sizeof(ARX_CHANGELEVEL_SCRIPT_SAVE);

		for (int i = 0; i < ass->nblvar; i++)
		{
			ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
			memset(&io->over_script.lvar[i], 0, sizeof(SCRIPT_VAR)); 
		retry2:
			;

			switch (avs->type)
			{
				case TYPE_L_TEXT:
					strcpy(io->over_script.lvar[i].name, avs->name);
					io->over_script.lvar[i].fval = avs->fval;
					io->over_script.lvar[i].ival = avs->fval;
					io->over_script.lvar[i].type = TYPE_L_TEXT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if (io->over_script.lvar[i].ival)
					{
						io->over_script.lvar[i].text = (char *) malloc(io->over_script.lvar[i].ival + 1);

						memset(io->over_script.lvar[i].text, 0, io->over_script.lvar[i].ival + 1);
						memcpy(io->over_script.lvar[i].text, dat + pos, io->over_script.lvar[i].ival);
						pos += io->over_script.lvar[i].ival;
						io->over_script.lvar[i].ival = strlen(io->over_script.lvar[i].text) + 1;
					}
					else
					{
						io->over_script.lvar[i].text = NULL;
						io->over_script.lvar[i].ival = 0;
					}

					break;
					
				case TYPE_L_LONG:
					strcpy(io->over_script.lvar[i].name, avs->name);
					io->over_script.lvar[i].fval = avs->fval;
					io->over_script.lvar[i].ival = avs->fval;
					io->over_script.lvar[i].type = TYPE_L_LONG;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				case TYPE_L_FLOAT:
					strcpy(io->over_script.lvar[i].name, avs->name);
					io->over_script.lvar[i].fval = avs->fval;
					io->over_script.lvar[i].ival = avs->fval;
					io->over_script.lvar[i].type = TYPE_L_FLOAT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				default:

				if ((avs->name[0] == '$') || (avs->name[0] == '\xA3'))
				{
					avs->type = TYPE_L_TEXT;
						goto retry2;
					}

					if ((avs->name[0] == '#') || (avs->name[0] == 's'))
					{
						avs->type = TYPE_L_LONG;
						goto retry2;
					}

					if ((avs->name[0] == '&') || (avs->name[0] == '@'))
					{
						avs->type = TYPE_L_FLOAT;
						goto retry2;
					}

					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = 0;
					io->script.lvar[i].ival = 0;
					io->script.lvar[i].type = TYPE_L_LONG;
					ERRCOUNT++;
					ass->nblvar = i;
					goto corrupted;
					break;
			}
		}

		_Gaids[Gaids_Number]->weapon[0] = 0;
	

		switch (ais->savesystem_type)
		{
			case TYPE_NPC:
				ARX_CHANGELEVEL_NPC_IO_SAVE * as;
				as = (ARX_CHANGELEVEL_NPC_IO_SAVE *)(dat + pos);
				{
					io->_npcdata->absorb = as->absorb;
					io->_npcdata->aimtime = as->aimtime;
					io->_npcdata->armor_class = as->armor_class;
					io->_npcdata->behavior = as->behavior;
					io->_npcdata->behavior_param = as->behavior_param;
					io->_npcdata->collid_state = as->collid_state;
					io->_npcdata->collid_time = as->collid_time;
					io->_npcdata->cut = as->cut;
					io->_npcdata->damages = as->damages;
					io->_npcdata->detect = as->detect;
					io->_npcdata->fightdecision = as->fightdecision;

					strcpy(_Gaids[Gaids_Number]->weapon, as->id_weapon);

					io->_npcdata->lastmouth = as->lastmouth;
					io->_npcdata->life = as->life;
					io->_npcdata->look_around_inc = as->look_around_inc;
					io->_npcdata->mana = as->mana;
					io->_npcdata->maxlife = as->maxlife;
					io->_npcdata->maxmana = as->maxmana;
					io->_npcdata->movemode = as->movemode;
					io->_npcdata->moveproblem = as->moveproblem;
					io->_npcdata->reachedtarget = as->reachedtarget;
					io->_npcdata->speakpitch = as->speakpitch;
					io->_npcdata->tactics = as->tactics;
					io->_npcdata->tohit = as->tohit;
					io->_npcdata->weaponinhand = as->weaponinhand;
					strcpy(io->_npcdata->weaponname, as->weaponname);
					io->_npcdata->weapontype = Flag(as->weapontype); // TODO save/load flags
					io->_npcdata->xpvalue = as->xpvalue;
					
					assert(SAVED_MAX_STACKED_BEHAVIOR == MAX_STACKED_BEHAVIOR);
					std::copy(as->stacked, as->stacked + SAVED_MAX_STACKED_BEHAVIOR, io->_npcdata->stacked);

					for (long iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++)
					{
						strcpy(_Gaids[Gaids_Number]->stackedtarget[iii], as->stackedtarget[iii]);
					}

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
					io->_npcdata->npcflags = as->npcflags;
					io->_npcdata->fDetect = as->fDetect;
					io->_npcdata->cuts = as->cuts;

					memset(&io->_npcdata->pathfind, 0, sizeof(IO_PATHFIND));

					io->_npcdata->pathfind.truetarget = as->pathfind.truetarget;

					if (ais->saveflags & SAVEFLAGS_EXTRA_ROTATE)
					{
						if (io->_npcdata->ex_rotate == NULL)
						{
							io->_npcdata->ex_rotate = (EERIE_EXTRA_ROTATE *) malloc(sizeof(EERIE_EXTRA_ROTATE));
						}

						*io->_npcdata->ex_rotate = as->ex_rotate;
					}

					io->_npcdata->blood_color = as->blood_color;
				}
				pos += sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE);
				break;
			case TYPE_ITEM:
				ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
				ai = (ARX_CHANGELEVEL_ITEM_IO_SAVE *)(dat + pos);
				io->_itemdata->price = ai->price;
				io->_itemdata->count = ai->count;
				io->_itemdata->maxcount = ai->maxcount;
				io->_itemdata->food_value = ai->food_value;
				io->_itemdata->stealvalue = ai->stealvalue;
				io->_itemdata->playerstacksize = ai->playerstacksize;
				io->_itemdata->LightValue = ai->LightValue;

				if (ais->system_flags  & SYSTEM_FLAG_EQUIPITEMDATA)
				{
					if (io->_itemdata->equipitem)
						free(io->_itemdata->equipitem);

					io->_itemdata->equipitem = (IO_EQUIPITEM *)malloc(sizeof(IO_EQUIPITEM));
					*io->_itemdata->equipitem = ai->equipitem;
				}
				else io->_itemdata->equipitem = NULL;

				pos += sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE);
				break;
			case TYPE_FIX:
				ARX_CHANGELEVEL_FIX_IO_SAVE * af;
				af = (ARX_CHANGELEVEL_FIX_IO_SAVE *)(dat + pos);
				io->_fixdata->trapvalue = af->trapvalue;
				pos += sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE);
				break;
			case TYPE_CAMERA:
				ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
				ac = (ARX_CHANGELEVEL_CAMERA_IO_SAVE *)(dat + pos);
				io->_camdata->cam = ac->cam;
				pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
				break;
			case TYPE_MARKER:
				pos += sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE);
				break;
		}

		if (ais->system_flags & SYSTEM_FLAG_INVENTORY)
		{

			memcpy(_Gaids[Gaids_Number], dat + pos,
				   sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE)
				   - SIZE_ID - SIZE_ID - MAX_LINKED_SAVE * SIZE_ID
				   - SIZE_ID * MAX_STACKED_BEHAVIOR);

			pos += sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE);

			if (io->inventory == NULL)
			{
				io->inventory = (void *) malloc(sizeof(INVENTORY_DATA));
			}

			memset(io->inventory, 0, sizeof(INVENTORY_DATA));
		}
		else
		{
			if (io->inventory)
			{
				free(io->inventory);
				io->inventory = NULL;
			}
		}

		if (ais->system_flags & SYSTEM_FLAG_TWEAKER_INFO)
		{
			if (io->tweakerinfo)
				free(io->tweakerinfo);

			io->tweakerinfo = (IO_TWEAKER_INFO *) malloc(sizeof(IO_TWEAKER_INFO));

			SavedTweakerInfo sti;
			memcpy(&sti, dat + pos, sizeof(SavedTweakerInfo));
			pos += sizeof(SavedTweakerInfo);
			*io->tweakerinfo = sti;

		}

		if (io->iogroups) free(io->iogroups);

		io->iogroups = NULL;

		if (io->nb_iogroups > 0)
		{
			io->iogroups = (IO_GROUP_DATA *) malloc(sizeof(IO_GROUP_DATA) * io->nb_iogroups);

			for(long i = 0; i < io->nb_iogroups; i++) {
				SavedGroupData sgd;
				memcpy(&sgd, dat + pos, sizeof(SavedGroupData));
				pos += sizeof(SavedGroupData);
				io->iogroups[i] = sgd;
			}
		}

		io->Tweak_nb = ais->Tweak_nb;

		if (io->Tweak_nb)
		{
			io->Tweaks = (TWEAK_INFO *)malloc(sizeof(TWEAK_INFO) * io->Tweak_nb);

			for(long i = 0; i < io->Tweak_nb; i++) {
				SavedTweakInfo sti;
				memcpy(&sti, dat + pos, sizeof(SavedTweakInfo));
				pos += sizeof(SavedTweakInfo);
				io->Tweaks[i] = sti;
			}
		}

		ARX_INTERACTIVE_APPLY_TWEAK_INFO(io);


		if (io->obj)
		{
			io->obj->nblinked = ais->nb_linked;

			if (io->obj->nblinked)
			{
				if (io->obj->linked)
					free(io->obj->linked);

				io->obj->linked = (EERIE_LINKED *) malloc(sizeof(EERIE_LINKED) * (io->obj->nblinked));

				for (long n = 0; n < ais->nb_linked; n++)
				{
					io->obj->linked[n].lgroup = ais->linked_data[n].lgroup;
					io->obj->linked[n].lidx = ais->linked_data[n].lidx;
					io->obj->linked[n].lidx2 = ais->linked_data[n].lidx2;
					io->obj->linked[n].modinfo = ais->linked_data[n].modinfo;
					strcpy(_Gaids[Gaids_Number]->linked_id[n], ais->linked_data[n].linked_id);
					io->obj->linked[n].io = NULL;
					io->obj->linked[n].obj = NULL;
				}
			}
		}

		long hidegore;

		if ((io->ioflags & IO_NPC)
				&&	(io->_npcdata->life > 0.f))
			hidegore = 1;
		else
			hidegore = 0;

		ARX_INTERACTIVE_HideGore(io, hidegore);

	}
	else
	{
		LogError << "CHANGELEVEL Error: Unable to load " << ident;
	}


	free(dat);
	CONVERT_CREATED = 1;

	if (NEED_LOG)
	{
		LogDebug << "--> After  ARX_CHANGELEVEL_Pop_IO(" << ident << ")";
	}

	return GetInterNum(tmp);
corrupted:
	LogError << "Save File Is Corrupted, Trying to Fix " << ident;

	free(dat);
	io->inventory = NULL; 
	RestoreInitialIOStatusOfIO(io);
	SendInitScriptEvent(io);

	return idx;
}
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_PopAllIO(ARX_CHANGELEVEL_INDEX * asi) {
	
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
		char tempo[256];
		sprintf(tempo, "%s_%04d", GetName(idx_io[i].filename).c_str(), idx_io[i].ident);
		ARX_CHANGELEVEL_Pop_IO(tempo);
	}
	
	return 1;
}
extern void GetIOCyl(INTERACTIVE_OBJ * io, EERIE_CYLINDER * cyl);
//-----------------------------------------------------------------------------

long ReadTargetInfo(char * info) {
	if(!strcasecmp(info, "NONE")) return -1;
	else if(!strcasecmp(info, "SELF")) return -2;
	else if(!strcasecmp(info, "PLAYER")) return 0;
	else return GetInterNum(ConvertToValidIO(info));
}

long ARX_CHANGELEVEL_PopAllIO_FINISH(long reloadflag)
{
	unsigned char * treated = (unsigned char *) malloc(sizeof(unsigned char) * MAX_IO_SAVELOAD);

	memset(treated, 0, sizeof(unsigned char)*MAX_IO_SAVELOAD); 
	long converted = 1;

	while (converted)
	{
		converted = 0;

		for (long it = 1; it < MAX_IO_SAVELOAD; it++)
		{
			if (it < inter.nbmax)
			{
				INTERACTIVE_OBJ * io = inter.iobj[it];

				if ((io) && (treated[it] == 0))
				{
					treated[it] = 1;
					ARX_CHANGELEVEL_INVENTORY_DATA_SAVE * aids = _Gaids[it];

					if (_Gaids[it])
					{
						if (io->inventory)
						{
							INVENTORY_DATA * inv = (INVENTORY_DATA *)io->inventory;
							inv->io = ConvertToValidIO(aids->io);
							converted += CONVERT_CREATED;

							if ((aids->sizex != 3)
									||	(aids->sizey != 11))
							{
								inv->sizex = 3;
								inv->sizey = 11;

								for (long m = 0; m < inv->sizex; m++)
									for (long n = 0; n < inv->sizey; n++)
									{
										inv->slot[m][n].io = NULL;
										inv->slot[m][n].show = 0;
									}
							}
							else
							{
								inv->sizex = aids->sizex;
								inv->sizey = aids->sizey;

								for (long m = 0; m < inv->sizex; m++)
									for (long n = 0; n < inv->sizey; n++)
									{
										inv->slot[m][n].io = ConvertToValidIO(aids->slot_io[m][n]);
										converted += CONVERT_CREATED;
										inv->slot[m][n].show = aids->slot_show[m][n];
									}
							}
						}

						if ((io->obj) && (io->obj->nblinked))
						{
							for (long n = 0; n < io->obj->nblinked; n++)
							{
								INTERACTIVE_OBJ * iooo = ConvertToValidIO(aids->linked_id[n]);

								if (iooo)
								{
									io->obj->linked[n].io = iooo;
									io->obj->linked[n].obj = iooo->obj;
								}
							}
						}

						if (io->ioflags & IO_NPC)
						{
							io->_npcdata->weapon = ConvertToValidIO(aids->weapon);
							converted += CONVERT_CREATED;

							if (io->_npcdata->weaponinhand == 1)
								SetWeapon_On(io);
							else SetWeapon_Back(io);
						}

						io->targetinfo = ReadTargetInfo(aids->targetinfo);

						if (io->ioflags & IO_NPC)
						{
							for (long iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++) {
								io->_npcdata->stacked[iii].target = ReadTargetInfo(aids->stackedtarget[iii]);
							}
						}

						if (io->ioflags & IO_NPC)
							if (io->_npcdata->behavior == BEHAVIOUR_NONE)
								io->targetinfo = -1;
					}
				}
			}
		}
	}

	free(treated);

	if (reloadflag)
	{
		for (long i = 0; i < inter.nbmax; i++)
		{
			if (inter.iobj[i])
			{
				if (inter.iobj[i]->script.data != NULL)
				{
					ScriptEvent::send(&inter.iobj[i]->script, SM_RELOAD, "CHANGE", inter.iobj[i], "");
				}

				if (inter.iobj[i]
						&&	inter.iobj[i]->over_script.data)
				{
					ScriptEvent::send(&inter.iobj[i]->over_script, SM_RELOAD, "CHANGE", inter.iobj[i], "");
				}

				if (inter.iobj[i]
						&&	(inter.iobj[i]->ioflags & IO_NPC))
				{
					if (ValidIONum(inter.iobj[i]->targetinfo))
					{
						if (inter.iobj[i]->_npcdata->behavior != BEHAVIOUR_NONE)
						{
							GetIOCyl(inter.iobj[i], &inter.iobj[i]->physics.cyl);
							GetTargetPos(inter.iobj[i]);
							ARX_NPC_LaunchPathfind(inter.iobj[i], inter.iobj[i]->targetinfo); //io->_npcdata->pathfind.truetarget);
						}
					}
				}
			}
		}
	}
	else if (_FIRSTTIME)
	{

		for (long i = 0; i < inter.nbmax; i++)
		{
			if (inter.iobj[i])
			{
				if (inter.iobj[i]->script.data)
				{
					ScriptEvent::send(&inter.iobj[i]->script, SM_INIT, "", inter.iobj[i], "");
				}

				if (inter.iobj[i]
						&&	inter.iobj[i]->over_script.data)
				{
					ScriptEvent::send(&inter.iobj[i]->over_script, SM_INIT, "", inter.iobj[i], "");
				}

				if (inter.iobj[i]
						&&	inter.iobj[i]->script.data)
				{
					ScriptEvent::send(&inter.iobj[i]->script, SM_INITEND, "", inter.iobj[i], "");
				}

				if (inter.iobj[i]
						&&	inter.iobj[i]->over_script.data)
				{
					ScriptEvent::send(&inter.iobj[i]->over_script, SM_INITEND, "", inter.iobj[i], "");
				}
			}
		}
	}
	else
	{
		for (long i = 0; i < inter.nbmax; i++)
		{
			if (inter.iobj[i])
			{
				if (inter.iobj[i]
						&&	(inter.iobj[i]->ioflags & IO_NPC))
				{
					if (ValidIONum(inter.iobj[i]->targetinfo))
					{
						if (inter.iobj[i]->_npcdata->behavior != BEHAVIOUR_NONE)
						{
							GetIOCyl(inter.iobj[i], &inter.iobj[i]->physics.cyl);
							GetTargetPos(inter.iobj[i]);
							ARX_NPC_LaunchPathfind(inter.iobj[i], inter.iobj[i]->targetinfo); //io->_npcdata->pathfind.truetarget);
						}
					}
				}
			}
		}
	}

	return 1;
}

static long ARX_CHANGELEVEL_Pop_Globals() {
	
	ARX_CHANGELEVEL_SAVE_GLOBALS * acsg;
	
	long pos = 0;
	std::string loadfile = "Globals.sav";
	

	ARX_SCRIPT_Free_All_Global_Variables();
	
	size_t size;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to Open " << loadfile << " for Read...";
		return -1;
	}
	
	acsg = (ARX_CHANGELEVEL_SAVE_GLOBALS *)(dat);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);

	if (acsg->version != ARX_GAMESAVE_VERSION)
	{
		free(dat);
		LogError << "Invalid version: " << loadfile;
		return -1;
	}

	if (acsg->nb_globals > 0)
	{
		svar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * acsg->nb_globals);
	}
	else svar = NULL;

	NB_GLOBALS = acsg->nb_globals;

	for (int i = 0; i < NB_GLOBALS; i++)
	{
		ARX_CHANGELEVEL_VARIABLE_SAVE * av = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);

		switch (av->type)
		{
			case TYPE_G_TEXT:
				strcpy(svar[i].name, av->name);
				svar[i].fval = av->fval;
				svar[i].ival = av->fval;
				svar[i].type = TYPE_G_TEXT;

				if (svar[i].ival)
				{
					svar[i].text = (char *) malloc(svar[i].ival + 1);
					memset(svar[i].text, 0, svar[i].ival + 1);

					memcpy(svar[i].text, dat + pos + sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE), svar[i].ival);

					if (svar[i].text[0] == '\xCC')
						svar[i].text[0] = 0;
				}
				else
					svar[i].text = NULL;

				pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

				if (svar[i].text)
					svar[i].ival = strlen(svar[i].text) + 1;
				else
					svar[i].ival = 0;

				pos += (long)av->fval; 
				break;
			case TYPE_G_LONG:
				strcpy(svar[i].name, av->name);
				svar[i].ival = av->fval;
				svar[i].fval = av->fval;
				svar[i].text = NULL;
				svar[i].type = TYPE_G_LONG;
				pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				break;
			case TYPE_G_FLOAT:
				strcpy(svar[i].name, av->name);
				svar[i].ival = av->fval;
				svar[i].fval = av->fval;
				svar[i].text = NULL;
				svar[i].type = TYPE_G_FLOAT;
				pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				break;
			default:
			{
				//at this level is strange
				svar[i].fval = 0.f;
				svar[i].text = NULL;
				svar[i].type = TYPE_G_TEXT;
			}
			break;
		}
	}

	free(dat);
	return 1;
}
//-----------------------------------------------------------------------------
void ReleaseGaids()
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if (_Gaids[i] != NULL)
			free(_Gaids[i]);
	}

	free(_Gaids);
	_Gaids = NULL;
}

extern long NODIRCREATION;

static long ARX_CHANGELEVEL_PopLevel(long instance, long reloadflag) {
	
	DANAE_ReleaseAllDatasDynamic();

	LogDebug << "Before ARX_CHANGELEVEL_PopLevel Alloc'n'Free";

	if (_Gaids) ReleaseGaids();

	_Gaids = (ARX_CHANGELEVEL_INVENTORY_DATA_SAVE **) malloc(sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *) * MAX_IO_SAVELOAD);

	memset(_Gaids, 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *)*MAX_IO_SAVELOAD);

	ARX_CHANGELEVEL_INDEX asi;

	CURRENT_GAME_INSTANCE = instance;
	ARX_CHANGELEVEL_MakePath();

	if (!DirectoryExist(CurGamePath))
	{
		LogError << "Cannot Load this game: Directory Not Found: " << CurGamePath;

		RELOADING = 0;
		ReleaseGaids();
		return -1;
	}

	LogDebug << "After  ARX_CHANGELEVEL_PopLevel Alloc'n'Free";

	// Clears All Scene contents...
	LogDebug << "Before DANAE ClearAll";
	DanaeClearAll();
	LogDebug << "After  DANAE ClearAll";

	ARX_TIME_Pause();
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;


	// Now we can load our things...
	char loadfile[256];
	long FirstTime;
	sprintf(loadfile, "lvl%03ld.sav", instance);
	
	// Open Saveblock for read
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", CurGamePath);
	_pSaveBlock = new SaveBlock(sfile);
	
	// first time in this level ?
	if (!_pSaveBlock->BeginRead()) {
		LogDebug << "don't have save block \"" << sfile << "\"";
		FirstTime = 1;
		FORBID_SCRIPT_IO_CREATION = 0;
		NO_PLAYER_POSITION_RESET = 0;
	}
	else if (!_pSaveBlock->hasFile(loadfile))
	{
		LogDebug << "don't have level \"" << loadfile << "\" save block";
		FirstTime = 1;
		FORBID_SCRIPT_IO_CREATION = 0;
		NO_PLAYER_POSITION_RESET = 0;
	}
	else
	{
		FirstTime = 0;
		FORBID_SCRIPT_IO_CREATION = 1;
		NO_PLAYER_POSITION_RESET = 1;
	}
	
	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen(instance);
	
	LogDebug << "FirstTime = " << FirstTime;

	_FIRSTTIME = FirstTime;
	NEW_LEVEL = instance;

	if (!FirstTime)
	{
		LogDebug << "Before ARX_CHANGELEVEL_Pop_Index";

		if (ARX_CHANGELEVEL_Pop_Index(&asi, instance) != 1)
		{

			LogError << "Cannot Load Index data";

			ARX_TIME_UnPause();

			if (idx_io)
				free(idx_io);

			idx_io = NULL;

			if (index_variable)
				free(index_variable);

			index_variable = NULL;
			idx_io_nb = 0;
			ReleaseTio();
			RELOADING = 0;
			ReleaseGaids();
			FORBID_SCRIPT_IO_CREATION = 0;
			return -1;
		}

		LogDebug << "After  ARX_CHANGELEVEL_Pop_Index";

		if (asi.version != ARX_GAMESAVE_VERSION)
		{
			LogError << "Invalid Save Version...";
			ARX_TIME_UnPause();

			if (idx_io)
				free(idx_io);

			idx_io = NULL;

			if (index_variable)
				free(index_variable);

			index_variable = NULL;
			idx_io_nb = 0;
			ReleaseTio();
			RELOADING = 0;
			ReleaseGaids();
			FORBID_SCRIPT_IO_CREATION = 0;
			return -1;
		}

		idx_io_nb = asi.nb_inter;
	}
	else
	{
		idx_io_nb = 0;
		idx_io = NULL;
	}

	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen(instance);
	LogDebug << "Before ARX_CHANGELEVEL_Pop_Level";

	if (ARX_CHANGELEVEL_Pop_Level(&asi, instance, FirstTime) != 1)
	{

		LogError << "Cannot Load Level data";

		ARX_TIME_UnPause();

		if (idx_io)
			free(idx_io);

		idx_io = NULL;

		if (index_variable)
			free(index_variable);

		index_variable = NULL;
		idx_io_nb = 0;
		ReleaseTio();
		RELOADING = 0;
		ReleaseGaids();
		FORBID_SCRIPT_IO_CREATION = 0;
		return -1;
	}



	LogDebug << "After  ARX_CHANGELEVEL_Pop_Index";
	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(instance);

	if (FirstTime)
	{


		ARX_CHECK_ULONG(ARX_CHANGELEVEL_DesiredTime);
		unsigned long ulDTime = ARX_CLEAN_WARN_CAST_ULONG(ARX_CHANGELEVEL_DesiredTime);

		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
		{
			if (scr_timer[i].exist)
			{
				scr_timer[i].tim = ulDTime;
			}
		}



	}

	if (!FirstTime)
	{
		LogDebug << "Before ARX_CHANGELEVEL_PopAllIO";

		if (ARX_CHANGELEVEL_PopAllIO(&asi) != 1)
		{

			LogError << "Cannot Load IO data";

			ARX_TIME_UnPause();

			if (idx_io)
				free(idx_io);

			idx_io = NULL;

			if (index_variable)
				free(index_variable);

			index_variable = NULL;
			idx_io_nb = 0;
			ReleaseTio();
			RELOADING = 0;
			ReleaseGaids();
			FORBID_SCRIPT_IO_CREATION = 0;
			return -1;
		}

		LogDebug << "After  ARX_CHANGELEVEL_PopAllIO";
	}

	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(instance);
	LogDebug << "Before ARX_CHANGELEVEL_Pop_Player";

	if(ARX_CHANGELEVEL_Pop_Player(instance) != 1) {

		LogError << "Cannot Load Player data";

		ARX_TIME_UnPause();

		if (idx_io)
			free(idx_io);

		idx_io = NULL;

		if (index_variable)
			free(index_variable);

		index_variable = NULL;
		idx_io_nb = 0;
		ReleaseTio();
		RELOADING = 0;
		ReleaseGaids();
		FORBID_SCRIPT_IO_CREATION = 0;
		return -1;
	}

	LogDebug << "After  ARX_CHANGELEVEL_Pop_Player";

	LogDebug << "Before ARX_CHANGELEVEL_PopAllIO_FINISH";
	// Restoring all Missing Objects required by other objects...
	ARX_CHANGELEVEL_PopAllIO_FINISH(reloadflag);
	LogDebug << "After  ARX_CHANGELEVEL_PopAllIO_FINISH";
	PROGRESS_BAR_COUNT += 15.f;
	LoadLevelScreen();
	ReleaseGaids();
	PROGRESS_BAR_COUNT += 3.f;
	LoadLevelScreen();

	if (!FirstTime)
	{
		LogDebug << "Before ARX_CHANGELEVEL_Pop_Zones_n_Lights";
		ARX_CHANGELEVEL_Pop_Zones_n_Lights(&asi, instance);
		LogDebug << "After  ARX_CHANGELEVEL_Pop_Zones_n_Lights";
	}

	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	LogDebug << "Before Player Misc Init";
	ForcePlayerInventoryObjectLevel(instance);
	ARX_EQUIPMENT_RecreatePlayerMesh();
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();

	ReleaseTio();
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);

	NO_TIME_INIT = 1;
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	LogDebug << "After  Player Misc Init";

	LogDebug << "Before Memory Release";

	if (idx_io)
		free(idx_io);

	idx_io = NULL;

	if (index_variable)
		free(index_variable);

	index_variable = NULL;
	idx_io_nb = 0;
	RELOADING = 0;
	
	FORBID_SCRIPT_IO_CREATION = 0;

	NO_TIME_INIT = 1;
	LogDebug << "After  Memory Release";

	LogDebug << "Before SaveBlock Release";
	delete _pSaveBlock;
	_pSaveBlock = NULL;
	LogDebug << "After  SaveBlock Release";

	LogDebug << "Before Final Inits";
	HERO_SHOW_1ST = -1;

	if (EXTERNALVIEW)
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 0);
	}

	if (!EXTERNALVIEW)
	{
		ARX_INTERACTIVE_Show_Hide_1st(inter.iobj[0], 1);
	}

	ARX_INTERACTIVE_HideGore(inter.iobj[0], 1);
	TRUE_PLAYER_MOUSELOOK_ON = 0;
	player.Interface &= ~INTER_COMBATMODE;
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	LogDebug << "After  Final Inits";
	return 1;
}





//-----------------------------------------------------------------------------
// copy a dir (recursive sub reps) in another, creating reps
// overwrites files for update
void CopyDirectory(char * _lpszSrc, char * _lpszDest)
{
	CreateDirectory(_lpszDest, NULL);

	//	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	LogDebug << "CopyDirectory " << _lpszSrc << " to " << _lpszDest;

	char path[256];
	ZeroMemory(path, 256);
	strcpy(path, _lpszSrc);
	strcat(path, "*.*");

	char tTemp[sizeof(WIN32_FIND_DATA)+2];
	WIN32_FIND_DATA * FindFileData = (WIN32_FIND_DATA *)tTemp;

	hFind = FindFirstFile(path, FindFileData);

	do
	{
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (strcmp(FindFileData->cFileName, ".") == 0) continue;

				if (strcmp(FindFileData->cFileName, "..") == 0) continue;

				char s[256];
				char d[256];
				ZeroMemory(s, 256);
				ZeroMemory(d, 256);
				strcpy(s, _lpszSrc);
				strcpy(d, _lpszDest);
				strcat(s, FindFileData->cFileName);
				strcat(d, FindFileData->cFileName);
				strcat(s, "\\");
				strcat(d, "\\");
				CopyDirectory(s, d);
			}
			else
			{
				char s[256];
				char d[256];
				ZeroMemory(s, 256);
				ZeroMemory(d, 256);
				strcpy(s, _lpszSrc);
				strcpy(d, _lpszDest);
				strcat(s, FindFileData->cFileName);
				strcat(d, FindFileData->cFileName);
				LogDebug << "copy file " << s << " to " << d;
				CopyFile(s, d, false);
			}
		}
	}
	while (FindNextFile(hFind, FindFileData) > 0);

	if (hFind)
		FindClose(hFind);
}

static bool ARX_CHANGELEVEL_Set_Player_LevelData(const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld, const string & path);

long ARX_CHANGELEVEL_Save(long instance, const std::string& name)
{
	LogDebug << "ARX_CHANGELEVEL_Save " << instance << " " << name;

	ARX_TIME_Pause();

	if(instance <= 0) {
		ARX_GAMESAVE_CreateNewInstance();
		instance = CURRENT_GAME_INSTANCE;
		LogDebug << "Created new instance " << instance;
	}
	
	CURRENT_GAME_INSTANCE = instance;
	
	if(instance == -1) {
		// fatality...
		LogWarning << "Internal Non-Fatal Error";
		return 0;
	}

	if(CURRENTLEVEL == -1) {
		// fatality...
		LogWarning << "Internal Non-Fatal Error";
		return 0;
	}
	
	ARX_SCRIPT_EventStackExecuteAll();
	// fill GameSavePath with our savepath.
	ARX_GAMESAVE_MakePath();
	// Erase All directory content if overwriting a game
	CreateDirectory(GameSavePath, NULL);

	if (SecondaryInventory != NULL)
	{
		INTERACTIVE_OBJ * io = (INTERACTIVE_OBJ *)SecondaryInventory->io;

		if (io != NULL)
		{
			InventoryDir = -1;
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE);
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}

	ARX_CHANGELEVEL_MakePath();
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, CURRENTLEVEL);
	KillAllDirectory(GameSavePath); 
	CopyDirectory(CurGamePath, GameSavePath);

	//on copie le fichier temporaire bmp dans le repertoire
	const char tcSrc[] = "SCT_0.BMP";
	char tcDst[256];
	sprintf(tcDst, "%sGSAVE.BMP", GameSavePath);
	CopyFile(tcSrc, tcDst, false);
	DeleteFile(tcSrc);

	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	memset(&pld, 0, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	pld.level = CURRENTLEVEL;
	strcpy(pld.name, name.c_str());
	pld.version = ARX_GAMESAVE_VERSION;
	pld.time = ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long''
	ARX_CHANGELEVEL_Set_Player_LevelData(pld, GameSavePath);
	ARX_TIME_UnPause();
	return 1;
}

static bool ARX_CHANGELEVEL_Set_Player_LevelData(const ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld, const string & path)
{
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", path.c_str());
	
	// TODO why use a global here?
	assert(_pSaveBlock == NULL);
	_pSaveBlock = new SaveBlock(sfile);

	if (!_pSaveBlock->BeginSave()) return false;

	if (!DirectoryExist(path)) return false;

	char * dat = new char[sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA)];

	memcpy(dat, &pld, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	long pos = sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA);
	char savefile[256];
	sprintf(savefile, "pld.sav");
	
	_pSaveBlock->save(savefile, dat, pos);
	
	delete[] dat;
	
	_pSaveBlock->flush();
	delete _pSaveBlock;
	_pSaveBlock = NULL;
	
	return true;
}

static bool ARX_CHANGELEVEL_Get_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA & pld, const string path)
{
	// Checks For Directory
	if (!DirectoryExist(path)) return false;

	// Open Save Block
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", path.c_str());
	_pSaveBlock = new SaveBlock(sfile);

	if(!_pSaveBlock->BeginRead()) {
		LogError << "cannot open savefile to get player level data: " << sfile;
		return false;
	}

	// Get Size
	std::string loadfile = "pld.sav";
	
	size_t size;
	char * dat = _pSaveBlock->load(loadfile, size);
	if(!dat) {
		LogError << "Unable to open " << loadfile << " for read...";
		delete _pSaveBlock;
		_pSaveBlock = 0;
		return false;
	}

	// Finishes Read
	delete _pSaveBlock;
	_pSaveBlock = 0;

	// Stores Data in pld
	memcpy(&pld, dat, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));

	if (pld.version != ARX_GAMESAVE_VERSION)
	{
		LogError << "Invalid GameSave Version";
		free(dat);
		return false;
	}

	// Release Data
	free(dat);
	return true;
}
long DONT_CLEAR_SCENE;
extern long STARTED_A_GAME;
//------------------------------------------------------------------------------
// ARX_CHANGELEVEL_Load: 
//------------------------------------------------------------------------------
long ARX_CHANGELEVEL_Load(long instance) {
	
	LogDebug << "begin ARX_CHANGELEVEL_Load";
	
	iTimeToDrawD7 = -3000;
	
	PROGRESS_BAR_TOTAL = 238; 
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;
	
	// Forbid Saving
	FORBID_SAVE = 1;
	ARX_TIME_Pause();
	
	// Checks Instance
	if(instance <= -1) {
		LogWarning << "Internal Non-Fatal Error";
		return -1;
	}
	
	// Checks/Create GameSavePath
	CURRENT_GAME_INSTANCE = instance;
	ARX_GAMESAVE_MakePath();
	
	if(!DirectoryExist(GameSavePath)) {
		LogError << "Unknown SavePath: " << GameSavePath;
		return -1;
	}
	
	// Empty Directory
	ARX_CHANGELEVEL_MakePath();
	KillAllDirectory(CurGamePath);
	CreateDirectory(CurGamePath, NULL);
	
	// Copy SavePath to Current Game
	CopyDirectory(GameSavePath, CurGamePath);
	// Retrieves Player LevelData
	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, CurGamePath)) {
		PROGRESS_BAR_COUNT += 2.f;
		LoadLevelScreen(pld.level);
		
		DONT_CLEAR_SCENE = (pld.level == CURRENTLEVEL) ? 1 : 0;
		
		float fPldTime = ARX_CLEAN_WARN_CAST_FLOAT(pld.time);
		DanaeClearLevel();
		PROGRESS_BAR_COUNT += 2.f;
		LoadLevelScreen(pld.level);
		CURRENTLEVEL = pld.level;
		ARX_CHANGELEVEL_DesiredTime	=	fPldTime;
		ARX_CHANGELEVEL_PopLevel(pld.level, 0);
		ARX_TIME_Force_Time_Restore(fPldTime);
		NO_TIME_INIT = 1;
		FORCE_TIME_RESTORE = fPldTime;
		DONT_CLEAR_SCENE = 0;
		
	} else {
		LogError << "Error Loading Level...";
		return -1;
	}
	
	STARTED_A_GAME = 1;
	BLOCK_PLAYER_CONTROLS = 0;
	player.Interface &= ~INTER_COMBATMODE;
	
	if (inter.iobj[0]) inter.iobj[0]->animlayer[1].cur_anim = NULL;
	
	JUST_RELOADED = 1;
	
	LogDebug << "success ARX_CHANGELEVEL_Load";
	return 1;
}
//------------------------------------------------------------------------------
// ARX_CHANGELEVEL_GetInfo: Retreives Name & Time of a Saved game in "path"
//------------------------------------------------------------------------------
long ARX_CHANGELEVEL_GetInfo( const std::string& path, std::string& name, float& version, long& level, unsigned long& time)
{

	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;

	// IMPROVE this will load the whole save file FAT just to get one file!
	if(ARX_CHANGELEVEL_Get_Player_LevelData(pld, path)) {
		name = pld.name;
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
//------------------------------------------------------------------------------
