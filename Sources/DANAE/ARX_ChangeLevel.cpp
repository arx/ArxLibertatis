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

//-----------------------------------------------------------------------------

#include "ARX_ChangeLevel.h"

#include "ARX_Damages.h"
#include "ARX_Equipment.h"
#include "ARX_Interactive.h"
#include "ARX_Minimap.h"
#include "ARX_NPC.h"
#include "ARX_Particles.h"
#include "ARX_Paths.h"
#include "ARX_Sound.h"
#include "ARX_Speech.h"
#include "ARX_Spells.h"
#include "ARX_time.h"

#include <HERMESMain.h>

#include <EERIEMath.h>
#include <EERIEObject.h>
#include <EERIEPathfinder.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)

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
extern long FORCE_IO_INDEX;
extern long FORBID_SCRIPT_IO_CREATION;
extern long NO_TIME_INIT;
extern long RELOADING;
extern long CHANGE_LEVEL_ICON;
extern long FOR_EXTERNAL_PEOPLE;
extern long TRUE_PLAYER_MOUSELOOK_ON;
extern int iTimeToDrawD7;
extern EERIE_3D LastValidPlayerPos;
#define MAX_IO_SAVELOAD	1500
//-----------------------------------------------------------------------------
typedef struct
{
  char ident[64];
} TEMP_IO;
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
CSaveBlock * _pSaveBlock = NULL;


ARX_CHANGELEVEL_IO_INDEX * idx_io = NULL;
long idx_io_nb = 0;
ARX_CHANGELEVEL_VARIABLE_SAVE 	*	index_variable = NULL;
 
ARX_CHANGELEVEL_INVENTORY_DATA_SAVE ** _Gaids = NULL;




char * StdBuffer = NULL;
long StdBuffer_size = 0;
char * GetStdBuffer(long size)
{
	if (size > StdBuffer_size)
	{
		StdBuffer = (char *)realloc(StdBuffer, size);
		StdBuffer_size = size;
	}

	return(StdBuffer);
}
void FreeStdBuffer()
{
	if (StdBuffer)
		free(StdBuffer);

	StdBuffer = NULL;
	StdBuffer_size = 0;
}



extern long ARX_CONVERSATION;
extern HANDLE LIGHTTHREAD;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern EERIE_BACKGROUND bkrgnd;
long CURRENT_GAME_INSTANCE = -1;
char GameSavePath[256];
extern char LOCAL_SAVENAME[64];
 
 
 
 
 
 
 
 
 
 
void ARX_GAMESAVE_MakePath()
{
	sprintf(GameSavePath, "%sSave%s\\", Project.workingdir, LOCAL_SAVENAME);

	if (!DirectoryExist(GameSavePath))
	{
		CreateDirectory(GameSavePath, NULL);
	}

	sprintf(GameSavePath, "%sSave%s\\Save%04d\\", Project.workingdir, LOCAL_SAVENAME, CURRENT_GAME_INSTANCE);

	if (!DirectoryExist(GameSavePath))
	{
		CreateDirectory(GameSavePath, NULL);
	}
}

void ARX_GAMESAVE_CreateNewInstance()
{
	char basepath[256];
	char testpath[256];
	long num = 1;
	sprintf(basepath, "%sSave%s\\", Project.workingdir, LOCAL_SAVENAME);

	while (1)
	{
		sprintf(testpath, "%sSave%04d", basepath, num);

		if (!DirectoryExist(testpath))
		{
			CreateDirectory(testpath, NULL);
			CURRENT_GAME_INSTANCE = num;
			ARX_GAMESAVE_MakePath();
			return;
		}
		else
		{
			//le directory peut exister mais peut être vide après un crash
			strcat(testpath, "\\GSAVE.SAV");
			FILE * f = fopen(testpath, "rb");

			if (!f) 				
			{
				CURRENT_GAME_INSTANCE = num;
				ARX_GAMESAVE_MakePath();
				return;
			}

			fclose(f);			
		}

		num++;
	}
}

long NEED_LOG = 0; //1;
void LogData(char * tex)
{
	if (!NEED_LOG) return;

	FILE * fic;

	if ((fic = fopen("c:\\ARXlog.txt", "a")) != NULL)
	{
		fprintf(fic, "%s\n", tex);
		fclose(fic);
	}
}

//-----------------------------------------------------------------------------
INTERACTIVE_OBJ * ConvertToValidIO(char * str)
{

	CONVERT_CREATED = 0;

	if ((!str)
	        ||	(str[0] == 0)) return NULL;

	long t = GetTargetByNameTarget(str);

	if (t < 0)
	{
		if ((NEED_LOG) && (stricmp(str, "none")))
		{
			char temp[256];
			sprintf(temp, "Call to ConvertToValidIO(%s)", str);
			LogData(temp);
		}

		t = ARX_CHANGELEVEL_Pop_IO(str);

		if (t < 0) return NULL;
	}

	inter.iobj[t]->level = (short)NEW_LEVEL; // Not really needed anymore...
	return (inter.iobj[t]);
}

//-----------------------------------------------------------------------------
long GetIOAnimIdx2(INTERACTIVE_OBJ * io, ANIM_HANDLE * anim)
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
void ARX_CHANGELEVEL_MakePath()
{
	sprintf(CurGamePath, "%sSave%s\\", Project.workingdir, LOCAL_SAVENAME);

	if (!DirectoryExist(CurGamePath))
	{
		CreateDirectory(CurGamePath, NULL);
	}

	sprintf(CurGamePath, "%sSave%s\\Cur%04d\\", Project.workingdir, LOCAL_SAVENAME, LAST_CHINSTANCE);

	if (!DirectoryExist(CurGamePath))
		CreateDirectory(CurGamePath, NULL);
}
//--------------------------------------------------------------------------------------------
void ARX_CHANGELEVEL_CreateNewInstance()
{
	char basepath[256];
	char testpath[256];
	long num = 1;
	sprintf(basepath, "%sSave%s\\", Project.workingdir, LOCAL_SAVENAME);

	while (1)
	{
		sprintf(testpath, "%sCur%04d", basepath, num);

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
 
CSaveBlock * GLOBAL_pSaveB = NULL;
void ARX_Changelevel_CurGame_Open()
{
	if (GLOBAL_pSaveB)
		ARX_Changelevel_CurGame_Close();

	char fic[256];
	sprintf(fic, "%sGsave.sav", CurGamePath);

	GLOBAL_pSaveB = new CSaveBlock(fic);
	GLOBAL_pSaveB->BeginRead();
	return;
}
bool ARX_Changelevel_CurGame_Seek(char * ident)
{
	if (GLOBAL_pSaveB)
	{
		char fic[256];
		sprintf(fic, "%s.sav", ident);

		if (GLOBAL_pSaveB->ExistFile(fic)) return true;
	}

	return false;
}
void ARX_Changelevel_CurGame_Close()
{
	if (GLOBAL_pSaveB)
	{
		GLOBAL_pSaveB->EndRead();
		delete GLOBAL_pSaveB;
		GLOBAL_pSaveB = NULL;
	}
}

extern long JUST_RELOADED;
extern void DemoFileCheck();
extern long FINAL_COMMERCIAL_DEMO;
//--------------------------------------------------------------------------------------------
void ARX_CHANGELEVEL_Change(char * level, char * target, long angle, long confirm)
{
	LogData("-----------------------------------");
	HERMES_DATE_TIME hdt;
	GetDate(&hdt);

	if (NEED_LOG)
	{
		LogData("ARX_CHANGELEVEL_Change");
		char tex[256];
		sprintf(tex, "Date: %02d/%02d/%d  Time: %dh%d", hdt.days, hdt.months, hdt.years, hdt.hours, hdt.mins);
		LogData(tex);
		sprintf(tex, "level %s target %s", level, target);
		LogData(tex);
	}

	DemoFileCheck();
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
	sprintf(tex, "LEVEL%s", level);
	long num = GetLevelNumByName(tex);


	if ((FINAL_COMMERCIAL_DEMO)
	        &&	(num != 10)
	        &&	(num != 12)
	        &&	(num != 15)
	        &&	(num != 1))
		return;

	LoadLevelScreen(GDevice, num);

	if (num == -1)
	{
		// fatality...
		ShowPopup("Internal Non-Fatal Error");
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
	LoadLevelScreen(GDevice, num);

	LogData("Before ARX_CHANGELEVEL_PushLevel");
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, NEW_LEVEL);
	LogData("After  ARX_CHANGELEVEL_PushLevel");


	LogData("Before ARX_CHANGELEVEL_PopLevel");
	ARX_CHANGELEVEL_PopLevel(num, 1);
	LogData("After  ARX_CHANGELEVEL_PopLevel");

	FreeStdBuffer();

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
				Vector_Copy(&WILL_RESTORE_PLAYER_POSITION, &moveto);
				WILL_RESTORE_PLAYER_POSITION_FLAG = 1;
			}
	}

	CURRENTLEVEL = NEW_LEVEL;
	player.desiredangle.b = player.angle.b = (float)angle;
	DONT_WANT_PLAYER_INZONE = 1;
	ARX_PLAYER_RectifyPosition();
	JUST_RELOADED = 1;
	NO_GMOD_RESET = 0;
	LogData("-----------------------------------");
}

//--------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////						SAVING 								////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------
long ARX_CHANGELEVEL_PushLevel(long num, long newnum)
{
	ARX_CHANGELEVEL_INDEX asi;
	ARX_SCRIPT_EventStackExecuteAll();

	// Close secondary inventory before leaving
	if (SecondaryInventory != NULL)
	{
		INTERACTIVE_OBJ * io = (INTERACTIVE_OBJ *)SecondaryInventory->io;

		if (io != NULL)
		{
			InventoryDir = -1;
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE, "");
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}

	ForcePlayerInventoryObjectLevel(newnum);
	char lvltxt[64];
	sprintf(lvltxt, "l%03d", num);

	char sfile[256];
	sprintf(sfile, "%sGsave.sav", CurGamePath);
	_pSaveBlock = new CSaveBlock(sfile);

	if (!_pSaveBlock->BeginSave(true, 1)) return -1;

	// Now we can save our things
	if (ARX_CHANGELEVEL_Push_Index(&asi, num) != 1)
	{
		ShowPopup("Error Saving Index...");
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_Globals(num) != 1)
	{
		ShowPopup("Error Saving Globals...");
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_Player(num) != 1)
	{
		ShowPopup("Error Saving Player...");
		ARX_TIME_UnPause();
		return -1;
	}

	if (ARX_CHANGELEVEL_Push_AllIO(num) != 1)
	{
		ShowPopup("Error Saving IOs...");
		ARX_TIME_UnPause();
		return -1;
	}

	_pSaveBlock->EndSave();
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
long ARX_CHANGELEVEL_Push_Index(ARX_CHANGELEVEL_INDEX * asi, long num)
{
	unsigned char * dat;
	long pos = 0;

	asi->version				= ARX_GAMESAVE_VERSION;
	asi->nb_inter				= 0;
	asi->nb_paths				= nbARXpaths;
	asi->time					= ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long''
	asi->nb_lights				= 0;

	memcpy(&asi->gmods_stacked, &stacked, sizeof(GLOBAL_MODS));
	memcpy(&asi->gmods_desired, &desired, sizeof(GLOBAL_MODS));
	memcpy(&asi->gmods_current, &current, sizeof(GLOBAL_MODS));

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

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT * el = GLight[i];

		if ((el != NULL) && (!(el->type & TYP_SPECIAL1)))
			asi->nb_lights++;
	}

	char savefile[256];
	sprintf(savefile, "lvl%03d.sav", num);

	long allocsize = sizeof(ARX_CHANGELEVEL_INDEX)
	                 + sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter
	                 + sizeof(ARX_CHANGELEVEL_PATH) * asi->nb_paths
	                 + sizeof(ARX_CHANGELEVEL_LIGHT) * asi->nb_lights;
	+48000;

	void * playlist = NULL;
	unsigned long asize = 0;
	ARX_SOUND_AmbianceSavePlayList(&playlist, &asize);
	allocsize += asize;

retry:
	;
	dat = (unsigned char *)malloc(allocsize);

	if (!dat)
	{
		if (HERMES_Memory_Emergency_Out(allocsize, "ChangeLevel_PushIndex"))
			goto retry;
	}

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

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		EERIE_LIGHT * el = GLight[i];

		if ((el != NULL) && (!(el->type & TYP_SPECIAL1)))
		{
			ARX_CHANGELEVEL_LIGHT * acl = (ARX_CHANGELEVEL_LIGHT *)(dat + pos);
			memset(acl, 0, sizeof(ARX_CHANGELEVEL_LIGHT));

			acl->status = el->status;
			pos += sizeof(ARX_CHANGELEVEL_LIGHT);
		}
	}

	char * compressed = NULL;
	long cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);
	free(dat);

	for (int i = 0; i < cpr_pos; i += 2)
		compressed[i] = ~compressed[i];

	bool ret = _pSaveBlock->Save(savefile, compressed, cpr_pos);
	free(compressed);

	if (!ret) return -1;

	return 1;
}
//--------------------------------------------------------------------------------------------
long ARX_CHANGELEVEL_Push_Globals(long num)
{
	ARX_CHANGELEVEL_SAVE_GLOBALS acsg;
	unsigned char * dat;
	long pos = 0;

	memset(&acsg, 0, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	acsg.nb_globals = NB_GLOBALS;
	acsg.version = ARX_GAMESAVE_VERSION;

	char savefile[256];
	sprintf(savefile, "Globals.sav");

	long allocsize = sizeof(ARX_VARIABLE_SAVE) * acsg.nb_globals
	                 + sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS) + 1000
	                 + 48000;

retry:
	;
	dat = (unsigned char *)malloc(allocsize);

	if (!dat)
	{
		if (HERMES_Memory_Emergency_Out(allocsize, "ChangeLevel_PushGlobals"))
			goto retry;
	}

	memcpy(dat, &acsg, sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS));
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);
	long count;
	ARX_VARIABLE_SAVE avs;

	for (long i = 0; i < NB_GLOBALS; i++)
	{
		switch (svar[i].type)
		{
			case TYPE_G_TEXT:

				if ((svar[i].name[0] == '$') || (svar[i].name[0] == '£'))
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

				if ((svar[i].name[0] == '#') || (svar[i].name[0] == '§'))
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

	char * compressed = NULL;
	long cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);
	free(dat);

	for (int i = 0; i < cpr_pos; i += 2)
		compressed[i] = ~compressed[i];

	_pSaveBlock->Save(savefile, compressed, cpr_pos);
	free(compressed);
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
		sprintf(tofill, "%s_%04d", GetName(io->filename), io->ident);
}
extern long sp_max;
extern long cur_rf;
extern long cur_mx;
extern long cur_mr;
extern long cur_pom;
extern long sp_wep;
extern long sp_arm;
//--------------------------------------------------------------------------------------------
long ARX_CHANGELEVEL_Push_Player(long num)
{
	ARX_CHANGELEVEL_PLAYER * asp;

	unsigned char * dat;
	long allocsize = sizeof(ARX_CHANGELEVEL_PLAYER) + Keyring_Number * sizeof(KEYRING_SLOT) + 48000;
	allocsize += 80 * nb_PlayerQuest;
	allocsize += sizeof(ARX_CHANGELEVEL_MAPMARKER_DATA) * Nb_Mapmarkers;


retry:
	;
	dat = (unsigned char *) malloc(allocsize);

	if (!dat)
	{
		if (HERMES_Memory_Emergency_Out(allocsize, "ChangeLevel_PushPlayer"))
			goto retry;
	}

	asp = (ARX_CHANGELEVEL_PLAYER *)dat;

	long pos = 0;
	pos += (sizeof(ARX_CHANGELEVEL_PLAYER));

	memset(asp, 0, sizeof(ARX_CHANGELEVEL_PLAYER));

	asp->AimTime					= player.AimTime;
	asp->angle.a					= player.angle.a;
	asp->angle.b					= player.angle.b;
	asp->angle.g					= player.angle.g;
	asp->armor_class				= player.armor_class;
	asp->Attribute_Constitution		= player.Attribute_Constitution;
	asp->Attribute_Dexterity		= player.Attribute_Dexterity;
	asp->Attribute_Mind				= player.Attribute_Mind;
	asp->Attribute_Strength			= player.Attribute_Strength;
	asp->Critical_Hit				= player.Critical_Hit;
	asp->Current_Movement			= player.Current_Movement;
	asp->damages					= player.damages;
	asp->doingmagic					= player.doingmagic;
	asp->Interface					= player.Interface;
	asp->playerflags				= player.playerflags;

	if (TELEPORT_TO_LEVEL[0])
		strcpy(asp->TELEPORT_TO_LEVEL, TELEPORT_TO_LEVEL);
	else
		memset(asp->TELEPORT_TO_LEVEL, 0, 64);

	if (TELEPORT_TO_POSITION[0])
		strcpy(asp->TELEPORT_TO_POSITION, TELEPORT_TO_POSITION);
	else
		memset(asp->TELEPORT_TO_POSITION, 0, 64);

	asp->TELEPORT_TO_ANGLE			= TELEPORT_TO_ANGLE;
	asp->CHANGE_LEVEL_ICON			= CHANGE_LEVEL_ICON;
	asp->bag						= player.bag;
	FillIOIdent(asp->equipsecondaryIO,	player.equipsecondaryIO);
	FillIOIdent(asp->equipshieldIO,		player.equipshieldIO);
	FillIOIdent(asp->leftIO,			player.leftIO);
	FillIOIdent(asp->rightIO,			player.rightIO);
	FillIOIdent(asp->curtorch,			CURRENT_TORCH);

	memcpy(&asp->precast, &Precast, sizeof(PRECAST_STRUCT)*MAX_PRECAST);

	//inventaires
	for (long iNbBag = 0; iNbBag < 3; iNbBag++)
		for (long m = 0; m < INVENTORY_Y; m++)
			for (long n = 0; n < INVENTORY_X; n++)
			{
				FillIOIdent(asp->id_inventory[iNbBag][n][m], inventory[iNbBag][n][m].io);
				asp->inventory_show[iNbBag][n][m] = inventory[iNbBag][n][m].show;
			}

	memcpy(asp->minimap, minimap, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);

	for (long i = 0; i < MAX_MINIMAPS; i++)
		asp->minimap[i].tc = 0;

	asp->falling				= player.falling;
	asp->gold					= player.gold;
	asp->invisibility			= inter.iobj[0]->invisibility;

	if (player.inzone)
	{
		ARX_PATH * ap = (ARX_PATH *)asp->inzone;
		strcpy(asp->inzone, ap->name);
	}

	asp->jumpphase				= player.jumpphase;
	asp->jumpstarttime			= player.jumpstarttime;
	asp->Last_Movement			= player.Last_Movement;
	asp->level					= player.level;
	asp->life					= player.life;
	asp->mana					= player.mana;
	asp->maxlife				= player.maxlife;
	asp->maxmana				= player.maxmana;
	asp->misc_flags = 0;

	if (player.onfirmground)
		asp->misc_flags |= 1;

	if (WILLRETURNTOCOMBATMODE)
		asp->misc_flags |= 2;

	memcpy(&asp->physics, &player.physics, sizeof(IO_PHYSICS));
	asp->poison					= player.poison;
	asp->hunger					= player.hunger;

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


	asp->pos.x					= player.pos.x;
	asp->pos.y					= player.pos.y;
	asp->pos.z					= player.pos.z;
	asp->resist_magic			= player.resist_magic;
	asp->resist_poison			= player.resist_poison;
	asp->Attribute_Redistribute	= player.Attribute_Redistribute;
	asp->Skill_Redistribute		= player.Skill_Redistribute;
	asp->rune_flags				= player.rune_flags;
	asp->size.x					= player.size.x;
	asp->size.y					= player.size.y;
	asp->size.z					= player.size.z;
	asp->Skill_Stealth			= player.Skill_Stealth;
	asp->Skill_Mecanism			= player.Skill_Mecanism;
	asp->Skill_Intuition		= player.Skill_Intuition;
	asp->Skill_Etheral_Link		= player.Skill_Etheral_Link;
	asp->Skill_Object_Knowledge	= player.Skill_Object_Knowledge;
	asp->Skill_Casting			= player.Skill_Casting;
	asp->Skill_Projectile		= player.Skill_Projectile;
	asp->Skill_Close_Combat		= player.Skill_Close_Combat;
	asp->Skill_Defense			= player.Skill_Defense;
	asp->skin					= player.skin;

	asp->xp						= player.xp;
	asp->nb_PlayerQuest			= nb_PlayerQuest;
	asp->keyring_nb				= Keyring_Number;
	asp->Global_Magic_Mode		= GLOBAL_MAGIC_MODE;
	asp->Nb_Mapmarkers			= Nb_Mapmarkers;

	asp->LAST_VALID_POS.x = LastValidPlayerPos.x;
	asp->LAST_VALID_POS.y = LastValidPlayerPos.y;
	asp->LAST_VALID_POS.z = LastValidPlayerPos.z;

	for (int i = 0; i < MAX_ANIMS; i++)
	{
		memset(&asp->anims[i], 0, 256);

		if (inter.iobj[0]->anims[i] != NULL)
		{
			strcpy(asp->anims[i], inter.iobj[0]->anims[i]->path + strlen(Project.workingdir));
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

	for (int i = 0; i < nb_PlayerQuest; i++)
	{
		memset(dat + pos, 0, 80);
		strcpy((char *)(dat + pos), PlayerQuest[i].ident);
		pos += 80;
	}

	for (int i = 0; i < asp->keyring_nb; i++)
	{
		memcpy((char *)(dat + pos), &Keyring[i], sizeof(KEYRING_SLOT));
		pos += sizeof(KEYRING_SLOT);
	}

	for (int i = 0; i < asp->Nb_Mapmarkers; i++)
	{
		memcpy((char *)(dat + pos), &Mapmarkers[i], sizeof(ARX_CHANGELEVEL_MAPMARKER_DATA));
		pos += sizeof(ARX_CHANGELEVEL_MAPMARKER_DATA);
	}

	LastValidPlayerPos.x = asp->LAST_VALID_POS.x;
	LastValidPlayerPos.y = asp->LAST_VALID_POS.y;
	LastValidPlayerPos.z = asp->LAST_VALID_POS.z;


	char savefile[256];
	sprintf(savefile, "player.sav");

	char * compressed = NULL;
	long cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);
	free(dat);

	for (int i = 0; i < cpr_pos; i += 2)
		compressed[i] = ~compressed[i];

	_pSaveBlock->Save(savefile, compressed, cpr_pos);
	free(compressed);

	for (int i = 1; i < inter.nbmax; i++)
	{
		if ((IsInPlayerInventory(inter.iobj[i]))
		        ||	(IsPlayerEquipedWith(inter.iobj[i])))
			ARX_CHANGELEVEL_Push_IO(inter.iobj[i]);
	}

	return 1;
}


//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Push_AllIO(long num)
{
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

// num = num in index file
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Push_IO(INTERACTIVE_OBJ * io)
{
	// Check Valid IO
	if (!io)
		return -1;

	// Sets Savefile Name
	char savefile[256];
	sprintf(savefile, "%s_%04d.sav", GetName(io->filename), io->ident);

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

	// Main Buffer
	unsigned char * dat	= NULL;
	long pos			= 0;

	// Store IO Data in Main IO Save Structure
	ais.version			= ARX_GAMESAVE_VERSION;
	ais.savesystem_type	= type;
	ais.saveflags		= 0;
	strcpy(ais.filename, io->filename);
	ais.ident			= io->ident;
	ais.ioflags			= io->ioflags;

	if ((ais.ioflags & IO_FREEZESCRIPT)
	        &&	((ARX_SPELLS_GetSpellOn(io, SPELL_PARALYSE) >= 0)
	             ||	(ARX_SPELLS_GetSpellOn(io, SPELL_MASS_PARALYSE) >= 0)))
		ais.ioflags &= ~IO_FREEZESCRIPT;

	ais.pos.x			= io->pos.x;
	ais.pos.y			= io->pos.y;
	ais.pos.z			= io->pos.z;

	if ((io->obj)
	        &&	(io->obj->pbox)
	        &&	(io->obj->pbox->active))
		ais.pos.y -= io->obj->pbox->vert[0].initpos.y;

	ais.lastpos.x		= io->lastpos.x;
	ais.lastpos.y		= io->lastpos.y;
	ais.lastpos.z		= io->lastpos.z;
	ais.initpos.x		= io->initpos.x;
	ais.initpos.y		= io->initpos.y;
	ais.initpos.z		= io->initpos.z;
	ais.initangle.x		= io->initangle.x;
	ais.initangle.y		= io->initangle.y;
	ais.initangle.z		= io->initangle.z;
	ais.move.x			= io->move.x;
	ais.move.y			= io->move.y;
	ais.move.z			= io->move.z;
	ais.lastmove.x		= io->lastmove.x;
	ais.lastmove.y		= io->lastmove.y;
	ais.lastmove.z		= io->lastmove.z;
	ais.angle.a			= io->angle.a;
	ais.angle.b			= io->angle.b;
	ais.angle.g			= io->angle.g;
	ais.scale			= io->scale;
	ais.weight			= io->weight;
	strcpy(ais.locname, io->locname);
	ais.EditorFlags		= io->EditorFlags;
	ais.GameFlags		= io->GameFlags;

	if (io == inter.iobj[0])
		ais.GameFlags &= ~GFLAG_INVISIBILITY;

	ais.material		= io->material;
	ais.level			= io->level;
	ais.truelevel		= io->truelevel;
	ais.scriptload		= io->scriptload;
	ais.show			= io->show;
	ais.collision		= io->collision;
	strcpy(ais.mainevent, io->mainevent);
	ais.velocity.x		= io->velocity.x;
	ais.velocity.y		= io->velocity.y;
	ais.velocity.z		= io->velocity.z;
	ais.stopped			= io->stopped;
	ais.basespeed		= io->basespeed;
	ais.speed_modif		= io->speed_modif;
	ais.frameloss		= io->frameloss;
	ais.rubber			= io->rubber;
	ais.max_durability	= io->max_durability;
	ais.durability		= io->durability;
	ais.poisonous		= io->poisonous;
	ais.poisonous_count	= io->poisonous_count;
	ais.head_rot		= io->head_rot;
	ais.damager_damages = io->damager_damages;
	ais.nb_iogroups		= io->nb_iogroups;
	ais.damager_type	= io->damager_type;
	ais.type_flags		= io->type_flags;
	ais.secretvalue		= io->secretvalue;
	ais.shop_multiply	= io->shop_multiply;
	ais.aflags			= io->aflags;
	ais.original_height	= io->original_height;
	ais.original_radius	= io->original_radius;
	ais.ignition		= io->ignition;

	if (io->usepath)
	{
		ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

		ais.usepath_aupflags = aup->aupflags;


		float ulCurTime = aup->_curtime;
		ARX_CHECK_ULONG(ulCurTime);
		ais.usepath_curtime = ARX_CLEAN_WARN_CAST_ULONG(ulCurTime) ;


		Vector_Copy(&ais.usepath_initpos, &aup->initpos);
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
			strcpy(ais.anims[i], io->anims[i]->path + strlen(Project.workingdir));
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

		if (ais.nb_linked > MAX_LINKED_SAVE) ais.nb_linked = MAX_LINKED_SAVE;

		long count = 0;

		for (int n = 0; n < io->obj->nblinked; n++)
		{
			if (GetObjIOSource((EERIE_3DOBJ *)io->obj->linked[n].obj))
			{
				ais.linked_data[count].lgroup = io->obj->linked[count].lgroup;
				ais.linked_data[count].lidx = io->obj->linked[count].lidx;
				ais.linked_data[count].lidx2 = io->obj->linked[count].lidx2;
				memcpy(&ais.linked_data[count].modinfo, &io->obj->linked[count].modinfo, sizeof(EERIE_MOD_INFO));
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

	memcpy(&ais.physics, &io->physics, sizeof(IO_PHYSICS));
	memcpy(&ais.spellcast_data, &io->spellcast_data, sizeof(IO_SPELLCAST_DATA));
	memcpy(&ais.animlayer, &io->animlayer, sizeof(ANIM_USE)*MAX_ANIM_LAYERS);

	for (long k = 0; k < MAX_ANIM_LAYERS; k++)
	{
		ais.animlayer[k].cur_anim = (ANIM_HANDLE *)GetIOAnimIdx2(io, io->animlayer[k].cur_anim);
		ais.animlayer[k].next_anim = (ANIM_HANDLE *)GetIOAnimIdx2(io, io->animlayer[k].next_anim);
	}

	// Save Target Infos
	long numtarget = io->targetinfo;

	if ((io->ioflags & IO_NPC) && (io->_npcdata->pathfind.listnb > 0) && ValidIONum(io->_npcdata->pathfind.truetarget))
	{
		numtarget = io->_npcdata->pathfind.truetarget;
	}

	if (numtarget == -2) strcpy(ais.id_targetinfo, "SELF");
	else if (numtarget == -1) strcpy(ais.id_targetinfo, "NONE");
	else if (numtarget == 0) strcpy(ais.id_targetinfo, "PLAYER");
	else if (ValidIONum(numtarget))
		FillIOIdent(ais.id_targetinfo, (INTERACTIVE_OBJ *)inter.iobj[numtarget]);
	else strcpy(ais.id_targetinfo, "NONE");

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
	    + sizeof(IO_TWEAKER_INFO)
	    + sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE) + 1024
	    + sizeof(IO_GROUP_DATA) * io->nb_iogroups
	    + sizeof(TWEAK_INFO) * io->Tweak_nb
	    + 48000;

	// Allocate Main Save Buffer
	dat = (unsigned char *) malloc(allocsize);

	if (!dat) HERMES_Memory_Emergency_Out();

	memcpy(&ais.halo, &io->halo_native, sizeof(IO_HALO));
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
				strcpy(ats->name, scr_timer[i].name);
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
	long posi = 0;

	for (int i = 0; i < io->script.nblvar; i++)
	{
		ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
		memset(avs, 0, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		long count;

		switch (io->script.lvar[i].type)
		{
			case TYPE_L_TEXT:

				if ((io->script.lvar[i].name[0] == '$') || (io->script.lvar[i].name[0] == '£'))
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

				if ((io->script.lvar[i].name[0] == '#') || (io->script.lvar[i].name[0] == '§'))
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
	posi = 0;

	for (int i = 0; i < io->over_script.nblvar; i++)
	{
		ARX_CHANGELEVEL_VARIABLE_SAVE * avs = (ARX_CHANGELEVEL_VARIABLE_SAVE *)(dat + pos);
		memset(avs, 0, sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE));
		long count;

		switch (io->over_script.lvar[i].type)
		{
			case TYPE_L_TEXT:

				if ((io->script.lvar[i].name[0] == '$') || (io->script.lvar[i].name[0] == '£'))
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

				if ((io->script.lvar[i].name[0] == '#') || (io->script.lvar[i].name[0] == '§'))
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

	long iii;

	switch (type)
	{
		case TYPE_NPC:
			ARX_CHANGELEVEL_NPC_IO_SAVE * as;
			as = (ARX_CHANGELEVEL_NPC_IO_SAVE *)(dat + pos);
			memset(as, 0, sizeof(ARX_CHANGELEVEL_NPC_IO_SAVE));
			as->absorb =				io->_npcdata->absorb;
			as->aimtime =			io->_npcdata->aimtime;
			as->armor_class =		io->_npcdata->armor_class;
			as->behavior =			io->_npcdata->behavior;
			as->behavior_param =		io->_npcdata->behavior_param;
			as->collid_state =		io->_npcdata->collid_state;
			as->collid_time =		io->_npcdata->collid_time;
			as->cut =				io->_npcdata->cut;
			as->damages =			io->_npcdata->damages;
			as->detect =				io->_npcdata->detect;
			as->fightdecision =		io->_npcdata->fightdecision;

			if (io->_npcdata->life > 0.f)
				FillIOIdent(as->id_weapon, (INTERACTIVE_OBJ *)io->_npcdata->weapon);
			else
				as->id_weapon[0] = 0;

			as->lastmouth =			io->_npcdata->lastmouth;
			as->life =				io->_npcdata->life;
			as->look_around_inc =	io->_npcdata->look_around_inc;
			as->mana =				io->_npcdata->mana;
			as->maxlife =			io->_npcdata->maxlife;
			as->maxmana =			io->_npcdata->maxmana;
			as->movemode =			io->_npcdata->movemode;
			as->moveproblem =		io->_npcdata->moveproblem;
			as->reachedtarget =		io->_npcdata->reachedtarget;
			as->speakpitch =			io->_npcdata->speakpitch;
			as->tactics =			io->_npcdata->tactics;
			as->tohit =				io->_npcdata->tohit;
			as->weaponinhand =		io->_npcdata->weaponinhand;
			strcpy(as->weaponname, io->_npcdata->weaponname);
			as->weapontype =			io->_npcdata->weapontype;
			as->xpvalue =			io->_npcdata->xpvalue;
			memcpy(as->stacked, io->_npcdata->stacked, sizeof(IO_BEHAVIOR_DATA)*MAX_STACKED_BEHAVIOR);

			for (iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++)
			{
				if ((io->_npcdata->stacked[iii].exist)
				        && (ValidIONum(io->_npcdata->stacked[iii].target)
				            || io->_npcdata->stacked[iii].target == -2))
				{
					long trg = io->_npcdata->stacked[iii].target;

					if (trg < 0) trg = GetInterNum(io);

					if (ValidIONum(trg))
						FillIOIdent(as->stackedtarget[iii], (INTERACTIVE_OBJ *)inter.iobj[trg]);
					else
						strcpy(as->stackedtarget[iii], "NONE");
				}
				else strcpy(as->stackedtarget[iii], "NONE");
			}

			as->critical			= io->_npcdata->critical;
			as->reach				= io->_npcdata->reach;
			as->backstab_skill		= io->_npcdata->backstab_skill;
			as->poisonned			= io->_npcdata->poisonned;
			as->resist_poison		= io->_npcdata->resist_poison;
			as->resist_magic		= io->_npcdata->resist_magic;
			as->resist_fire			= io->_npcdata->resist_fire;
			as->strike_time			= io->_npcdata->strike_time;
			as->walk_start_time		= io->_npcdata->walk_start_time;
			as->aiming_start		= io->_npcdata->aiming_start;
			as->npcflags			= io->_npcdata->npcflags;
			memset(&as->pathfind, 0, sizeof(IO_PATHFIND));

			if (io->_npcdata->pathfind.listnb > 0)
				as->pathfind.truetarget = io->_npcdata->pathfind.truetarget;
			else as->pathfind.truetarget = -1;

			if (io->_npcdata->ex_rotate)
			{
				ais.saveflags		|= SAVEFLAGS_EXTRA_ROTATE;
				memcpy(&as->ex_rotate, io->_npcdata->ex_rotate, sizeof(EERIE_EXTRA_ROTATE));
			}

			as->blood_color = io->_npcdata->blood_color;
			as->fDetect				= io->_npcdata->fDetect;
			as->cuts				= io->_npcdata->cuts;
			pos += struct_size;
			break;
		case TYPE_ITEM:
			ARX_CHANGELEVEL_ITEM_IO_SAVE * ai;
			ai = (ARX_CHANGELEVEL_ITEM_IO_SAVE *)(dat + pos);
			memset(ai, 0, sizeof(ARX_CHANGELEVEL_ITEM_IO_SAVE));
			ai->price				= io->_itemdata->price;
			ai->count				= io->_itemdata->count;
			ai->maxcount			= io->_itemdata->maxcount;
			ai->food_value			= io->_itemdata->food_value;
			ai->stealvalue			= io->_itemdata->stealvalue;
			ai->playerstacksize		= io->_itemdata->playerstacksize;
			ai->LightValue			= io->_itemdata->LightValue;

			if (io->_itemdata->equipitem)
			{
				memcpy(&ai->equipitem, io->_itemdata->equipitem, sizeof(IO_EQUIPITEM));
			}

			pos						+= struct_size;
			break;
		case TYPE_FIX:
			ARX_CHANGELEVEL_FIX_IO_SAVE * af;
			af = (ARX_CHANGELEVEL_FIX_IO_SAVE *)(dat + pos);
			memset(af, 0, sizeof(ARX_CHANGELEVEL_FIX_IO_SAVE));
			af->trapvalue			= io->_fixdata->trapvalue;
			pos						+= struct_size;
			break;
		case TYPE_CAMERA:
			ARX_CHANGELEVEL_CAMERA_IO_SAVE * ac;
			ac = (ARX_CHANGELEVEL_CAMERA_IO_SAVE *)(dat + pos);
			memset(ac, 0, sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE));
			memcpy(&ac->cam, io->_camdata, sizeof(EERIE_CAMERA));
			pos						+= struct_size;
			break;
		case TYPE_MARKER:
			ARX_CHANGELEVEL_MARKER_IO_SAVE * am;
			am = (ARX_CHANGELEVEL_MARKER_IO_SAVE *)(dat + pos);
			memset(am, 0, sizeof(ARX_CHANGELEVEL_MARKER_IO_SAVE));
			am->dummy				= 0;
			pos						+= struct_size;
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

	if (io->tweakerinfo)
	{
		memcpy(dat + pos, io->tweakerinfo, sizeof(IO_TWEAKER_INFO));
		pos += sizeof(IO_TWEAKER_INFO);
	}

	for (long ii = 0; ii < io->nb_iogroups; ii++)
	{
		IO_GROUP_DATA * igd = (IO_GROUP_DATA *)(dat + pos);
		pos += sizeof(IO_GROUP_DATA);
		memcpy(igd, &io->iogroups[ii], sizeof(IO_GROUP_DATA));
	}


	for (int ii = 0; ii < io->Tweak_nb; ii++)
	{
		TWEAK_INFO * ti = (TWEAK_INFO *)(dat + pos);
		pos += sizeof(TWEAK_INFO);
		memcpy(ti, &io->Tweaks[ii], sizeof(TWEAK_INFO));
	}

	if ((pos > allocsize) && (!FOR_EXTERNAL_PEOPLE))
	{
		char tex[256];
		sprintf(tex, "SaveBuffer Overflow %d >> %d", pos, allocsize);
		ShowPopup(tex);
	}

	char * compressed = NULL;
	long cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);
	free(dat);

	for (int i = 0; i < cpr_pos; i += 2)
		compressed[i] = ~compressed[i];

	_pSaveBlock->Save(savefile, compressed, cpr_pos);
	free(compressed);
	return 1;
}

//-----------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////						LOADING								////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Pop_Index(ARX_CHANGELEVEL_INDEX * asi, long num)
{
	unsigned char * dat;
	long pos = 0;
	char loadfile[256];
	char _error[256];
	
	sprintf(loadfile, "lvl%03d.sav", num);
	long size;
	size = _pSaveBlock->GetSize(loadfile);

	if (size <= 0)
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);
		ShowPopup(_error);
		return -1;
	}

	char * compressed = (char *) GetStdBuffer(size); 

	if (!compressed) HERMES_Memory_Emergency_Out();

	if (!_pSaveBlock->Read(loadfile, (char *)compressed))
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);
		ShowPopup(_error);
		return -1;
	}

	for (long i = 0; i < size; i += 2)
		compressed[i] = ~compressed[i];

	long ssize = size;
 
	dat = (unsigned char *)STD_Explode(compressed, ssize, &size); 

	memcpy(asi, dat, sizeof(ARX_CHANGELEVEL_INDEX));
	pos += sizeof(ARX_CHANGELEVEL_INDEX);

	if (asi->nb_inter)
	{
		idx_io = (ARX_CHANGELEVEL_IO_INDEX *) malloc(sizeof(ARX_CHANGELEVEL_IO_INDEX) * asi->nb_inter);

		if (!idx_io) HERMES_Memory_Emergency_Out();

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
	unsigned char * dat;
	long pos = 0;
	char loadfile[256];
	char _error[256];
	long size;

	sprintf(loadfile, "lvl%03d.sav", num);
	size = _pSaveBlock->GetSize(loadfile);

	if (size < 0)
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);
		ShowPopup(_error);
		return -1;
	}

	char * compressed = (char *) GetStdBuffer(size); 

	if (!compressed) HERMES_Memory_Emergency_Out();

	if (!_pSaveBlock->Read(loadfile, (char *)compressed))
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);
		ShowPopup(_error);
		return -1;
	}

	for (long i = 0; i < size; i += 2)
		compressed[i] = ~compressed[i];

	long ssize = size;

	dat = (unsigned char *)STD_Explode(compressed, ssize, &size); //pos,&cpr_pos);

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

		for (long j = 0; j < MAX_LIGHTS; j++)
		{
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
	sprintf(ftemp, "%sGraph\\Levels\\%s\\%s.DLF", Project.workingdir, tex, tex);

	if (!PAK_FileExist(ftemp))
	{
		sprintf(tex, "Unable To Find %s", ftemp);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(tex);

		return 0;
	}

	LoadLevelScreen(GDevice, num);
	SetEditMode(1, false);

	if (ARX_CHANGELEVEL_Pop_Globals() != 1)
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Cannot Load Globals data");
	}

	DanaeLoadLevel(GDevice, ftemp);
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


	memcpy(&stacked, &asi->gmods_stacked, sizeof(GLOBAL_MODS));
	memcpy(&desired, &asi->gmods_desired, sizeof(GLOBAL_MODS));
	memcpy(&current, &asi->gmods_current, sizeof(GLOBAL_MODS));
	NO_GMOD_RESET = 1;
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);//asi.time);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	return 1;
}
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Pop_Player(ARX_CHANGELEVEL_INDEX * asi, ARX_CHANGELEVEL_PLAYER * asp)
{
	char loadfile[256];
	char _error[256];

	sprintf(loadfile, "player.sav");
	long size = _pSaveBlock->GetSize(loadfile);

	if (size <= 0)
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);
		ShowPopup(_error);
		return -1;
	}

	char * compressed = (char *) GetStdBuffer(size); 

	if (!compressed) HERMES_Memory_Emergency_Out();

	_pSaveBlock->Read(loadfile, (char *)compressed);

	for (long i = 0; i < size; i += 2)
		compressed[i] = ~compressed[i];

	long ssize = size;
 
	char * dat = (char *)STD_Explode(compressed, ssize, &size); 
	memcpy(asp, dat, sizeof(ARX_CHANGELEVEL_PLAYER));
	//free(compressed);

	player.AimTime					= asp->AimTime;
	player.desiredangle.a = player.angle.a = asp->angle.a;
	player.desiredangle.b = player.angle.b = asp->angle.b;
	player.angle.g					= asp->angle.g;


	ARX_CHECK_UCHAR(asp->armor_class);
	player.armor_class				= ARX_CLEAN_WARN_CAST_UCHAR(asp->armor_class);


	player.Attribute_Constitution	= asp->Attribute_Constitution;
	player.Attribute_Dexterity		= asp->Attribute_Dexterity;
	player.Attribute_Mind			= asp->Attribute_Mind;
	player.Attribute_Strength		= asp->Attribute_Strength;
	player.Critical_Hit				= asp->Critical_Hit;
	player.Current_Movement			= asp->Current_Movement;
	player.damages					= asp->damages;
	player.doingmagic				= asp->doingmagic;
	player.playerflags				= asp->playerflags;

	if (asp->TELEPORT_TO_LEVEL[0]) strcpy(TELEPORT_TO_LEVEL, asp->TELEPORT_TO_LEVEL);
	else memset(TELEPORT_TO_LEVEL, 0, 64);

	if (asp->TELEPORT_TO_POSITION[0]) strcpy(TELEPORT_TO_POSITION, asp->TELEPORT_TO_POSITION);
	else memset(TELEPORT_TO_POSITION, 0, 64);

	TELEPORT_TO_ANGLE = asp->TELEPORT_TO_ANGLE;
	CHANGE_LEVEL_ICON = asp->CHANGE_LEVEL_ICON;
	player.bag = asp->bag;
	memcpy(&Precast, &asp->precast, sizeof(PRECAST_STRUCT)*MAX_PRECAST);
	player.Interface				= asp->Interface;
	player.Interface &= ~INTER_MAP;
	player.falling					= asp->falling;
	player.gold						= asp->gold;
	inter.iobj[0]->invisibility		= asp->invisibility;
	ARX_PATH * ap = ARX_PATH_GetAddressByName(asp->inzone);
	player.inzone = ap;
	player.jumpphase				= asp->jumpphase;
	player.jumpstarttime			= asp->jumpstarttime;
	player.Last_Movement			= asp->Last_Movement;


	ARX_CHECK_UCHAR(asp->level);
	player.level					= ARX_CLEAN_WARN_CAST_UCHAR(asp->level);


	player.life						= asp->life;
	player.mana						= asp->mana;
	player.maxlife					= asp->maxlife;
	player.maxmana					= asp->maxmana;

	if (asp->misc_flags & 1)
		player.onfirmground = 1;
	else
		player.onfirmground = 0;

	if (asp->misc_flags & 2)
		WILLRETURNTOCOMBATMODE = 1;
	else
		WILLRETURNTOCOMBATMODE = 0;

	memcpy(&player.physics, &asp->physics, sizeof(IO_PHYSICS));
	player.poison					= asp->poison;
	player.hunger					= asp->hunger;
	player.pos.x					= asp->pos.x;
	player.pos.y					= asp->pos.y;
	player.pos.z					= asp->pos.z;

	if (asp->sp_flags & SP_ARM1)
		sp_arm = 1;
	else if (asp->sp_flags & SP_ARM2)
		sp_arm = 2;
	else if (asp->sp_flags & SP_ARM3)
		sp_arm = 3;
	else
		sp_arm = 0;

	if (asp->sp_flags & SP_MAX)
	{
		cur_mx = 3;
		sp_max = 1;
	}
	else
	{
		cur_mx = 0;
		sp_max = 0;
	}

	if (asp->sp_flags & SP_MR)
	{
		cur_mr = 3;
	}
	else
	{
		cur_mr = 0;
	}

	if (asp->sp_flags & SP_RF)
		cur_rf = 3;
	else
		cur_rf = 0;

	if (asp->sp_flags & SP_WEP)
	{
		cur_pom = 3;
		sp_wep = 1;
	}
	else
	{
		cur_pom = 0;
		sp_wep = 0;
	}



	if (inter.iobj[0])
	{
		Vector_Copy(&inter.iobj[0]->pos, &player.pos);
		inter.iobj[0]->pos.y += 170.f;
	}

	Vector_Copy(&WILL_RESTORE_PLAYER_POSITION, &asp->pos);
	WILL_RESTORE_PLAYER_POSITION_FLAG = 1;


	ARX_CHECK_UCHAR(asp->resist_magic);
	ARX_CHECK_UCHAR(asp->resist_poison);
	player.resist_magic				= ARX_CLEAN_WARN_CAST_UCHAR(asp->resist_magic);
	player.resist_poison			= ARX_CLEAN_WARN_CAST_UCHAR(asp->resist_poison);



	ARX_CHECK_UCHAR(asp->Attribute_Redistribute);
	ARX_CHECK_UCHAR(asp->Skill_Redistribute);

	player.Attribute_Redistribute	= ARX_CLEAN_WARN_CAST_UCHAR(asp->Attribute_Redistribute);
	player.Skill_Redistribute		= ARX_CLEAN_WARN_CAST_UCHAR(asp->Skill_Redistribute);


	player.rune_flags				= asp->rune_flags;
	player.size.x					= asp->size.x;
	player.size.y					= asp->size.y;
	player.size.z					= asp->size.z;
	player.Skill_Stealth			= asp->Skill_Stealth;
	player.Skill_Mecanism			= asp->Skill_Mecanism;
	player.Skill_Intuition			= asp->Skill_Intuition;
	player.Skill_Etheral_Link		= asp->Skill_Etheral_Link;
	player.Skill_Object_Knowledge	= asp->Skill_Object_Knowledge;
	player.Skill_Casting			= asp->Skill_Casting;
	player.Skill_Projectile			= asp->Skill_Projectile;
	player.Skill_Close_Combat		= asp->Skill_Close_Combat;
	player.Skill_Defense			= asp->Skill_Defense;


	ARX_CHECK_CHAR(asp->skin);
	player.skin						= ARX_CLEAN_WARN_CAST_CHAR(asp->skin);


	player.xp						= asp->xp;
	GLOBAL_MAGIC_MODE				= asp->Global_Magic_Mode;

	ARX_MINIMAP_PurgeTC();
	memcpy(minimap, asp->minimap, sizeof(MINI_MAP_DATA)*MAX_MINIMAPS);

	INTERACTIVE_OBJ * io = inter.iobj[0];

	for (int i = 0; i < MAX_ANIMS; i++)
	{
		if (io->anims[i] != NULL)
		{
			ReleaseAnimFromIO(io, i);
		}

		if (asp->anims[i][0])
		{
			char tex[256];
			sprintf(tex, "%s%s", Project.workingdir, asp->anims[i]);
			io->anims[i] = EERIE_ANIMMANAGER_Load(tex);
		}
	}

	for (int iNbBag = 0; iNbBag < 3; iNbBag++)
	{
		for (long m = 0; m < INVENTORY_Y; m++)
			for (long n = 0; n < INVENTORY_X; n++)
			{
				inventory[iNbBag][n][m].io = ConvertToValidIO(asp->id_inventory[iNbBag][n][m]);
			}
	}

	ARX_PLAYER_Quest_Init();
	long pos = sizeof(ARX_CHANGELEVEL_PLAYER);

	for (int i = 0; i < asp->nb_PlayerQuest; i++)
	{
		ARX_PLAYER_Quest_Add((char *)(dat + pos), true);
		pos += 80;
	}

	ARX_KEYRING_Init();

	for (int i = 0; i < asp->keyring_nb; i++)
	{
		char * key = (char *)(dat + pos);
		pos += sizeof(KEYRING_SLOT);
		ARX_KEYRING_Add(key);
	}

	ARX_MAPMARKER_Init();

	for (int i = 0; i < asp->Nb_Mapmarkers; i++)
	{
		ARX_CHANGELEVEL_MAPMARKER_DATA * acmd = (ARX_CHANGELEVEL_MAPMARKER_DATA *)(dat + pos);
		ARX_MAPMARKER_Add(acmd->x, acmd->y, acmd->lvl, acmd->string);
		memcpy((char *)(dat + pos), &Mapmarkers[i], sizeof(ARX_CHANGELEVEL_MAPMARKER_DATA));
		pos += sizeof(ARX_CHANGELEVEL_MAPMARKER_DATA);
	}

	ARX_PLAYER_Restore_Skin();

	ARX_PLAYER_Modify_XP(0);	 

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
long ARX_CHANGELEVEL_Pop_IO(char * ident)
{
	if (!stricmp(ident, "NONE")) return -1;

	char loadfile[256];
	ARX_CHANGELEVEL_IO_SAVE * ais;
	unsigned char * dat;
	long pos = 0;
	long size = 0;

	sprintf(loadfile, "%s.sav", ident);

	long t = GetTargetByNameTarget(ident);

	if (ValidIONum(t))
		return t;

	if (NEED_LOG)
	{
		char temp[256];
		sprintf(temp, "--> Before ARX_CHANGELEVEL_Pop_IO(%s)", ident);
		LogData(temp);
	}

	size = _pSaveBlock->GetSize(loadfile);

	if (size < 0)
	{
		char _error[256];
		sprintf(_error, "Unable to Open %s for Read...", loadfile);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(_error);

		return -1;
	}

	char * compressed = (char *)GetStdBuffer(size); 

	if (!compressed) HERMES_Memory_Emergency_Out();

	if (!_pSaveBlock->Read(loadfile, (char *)compressed))
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Unable to Read Data");

		return -1;
	}

	for (long i = 0; i < size; i += 2)
		compressed[i] = ~compressed[i];

	long ssize = size;
 
	dat = (unsigned char *)STD_Explode(compressed, ssize, &size);


	// Ignore object if can't explode file
	if (!dat)
	{
		char tcText[256];
		sprintf(tcText, "%s", ident, 0);
		MessageBox(NULL, tcText, "Error while loading...", 0);
		return -1;
	}

	ais = (ARX_CHANGELEVEL_IO_SAVE *)dat;

	if (ais->version != ARX_GAMESAVE_VERSION)
	{
		free(dat);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Invalid PopIO version");

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
		char temp[256];
		sprintf(temp, "--> Phase2 ARX_CHANGELEVEL_Pop_IO(%s)", ident);
		LogData(temp);
	}

	DANAE_LS_INTER dli;
	dli.angle.a = ais->angle.a;
	dli.angle.b = ais->angle.b;
	dli.angle.g = ais->angle.g;
	long num = atoi(ident + strlen(ident) - 4);

	dli.ident = num;
	strcpy(dli.name, ais->filename);
	dli.pos.x = ais->pos.x;
	dli.pos.y = ais->pos.y;
	dli.pos.z = ais->pos.z;
	INTERACTIVE_OBJ * tmp = LoadInter_Ex(&dli, &MSP);
	long idx = -1;
	INTERACTIVE_OBJ * io = NULL;

	if (tmp)
	{
		io = tmp;
		idx = GetInterNum(io);

		long  Gaids_Number = idx;
		_Gaids[Gaids_Number] = (ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *) malloc(sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));

		if (!_Gaids[Gaids_Number]) HERMES_Memory_Emergency_Out();

		memset(_Gaids[Gaids_Number], 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE));

		io->room_flags = 1;
		io->room = -1;
		io->no_collide = -1;
		io->ioflags = ais->ioflags;

		io->ioflags &= ~IO_FREEZESCRIPT;
		io->pos.x = ais->pos.x;
		io->pos.y = ais->pos.y;
		io->pos.z = ais->pos.z;
		io->lastpos.x = ais->lastpos.x;
		io->lastpos.y = ais->lastpos.y;
		io->lastpos.z = ais->lastpos.z;
		io->move.x = ais->move.x;
		io->move.y = ais->move.y;
		io->move.z = ais->move.z;
		io->lastmove.x = ais->lastmove.x;
		io->lastmove.y = ais->lastmove.y;
		io->lastmove.z = ais->lastmove.z;
		io->initpos.x = ais->initpos.x;
		io->initpos.y = ais->initpos.y;
		io->initpos.z = ais->initpos.z;
		io->initangle.x = ais->initangle.x;
		io->initangle.y = ais->initangle.y;
		io->initangle.z = ais->initangle.z;
		io->angle.a = ais->angle.a;
		io->angle.b = ais->angle.b;
		io->angle.g = ais->angle.g;
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
		io->velocity.x = ais->velocity.x;
		io->velocity.y = ais->velocity.y;
		io->velocity.z = ais->velocity.z;
		io->stopped	= ais->stopped;
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
		io->nb_iogroups		= ais->nb_iogroups;
		io->damager_type	= ais->damager_type;
		io->type_flags		= ais->type_flags;
		io->secretvalue		= ais->secretvalue;
		io->shop_multiply	= ais->shop_multiply;
		io->aflags			= ais->aflags;
		io->original_height	= ais->original_height;
		io->original_radius	= ais->original_radius;
		io->ignition		= ais->ignition;

		if (ais->system_flags & SYSTEM_FLAG_USEPATH)
		{
			io->usepath = (void *)malloc(sizeof(ARX_USE_PATH));
			ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

			aup->aupflags	=	ais->usepath_aupflags;
			aup->_curtime	=	ARX_CLEAN_WARN_CAST_FLOAT(ais->usepath_curtime);
			Vector_Copy(&aup->initpos, &ais->usepath_initpos);
			aup->lastWP		=	ais->usepath_lastWP;
			aup->_starttime	=	ARX_CLEAN_WARN_CAST_FLOAT(ais->usepath_starttime);
			ARX_PATH * ap = ARX_PATH_GetAddressByName(ais->usepath_name);
			aup->path = ap;
		}

		if (ais->shop_category[0])
			io->shop_category = strdup(ais->shop_category);
		else
			io->shop_category = NULL;

		memcpy(&io->halo_native, &ais->halo, sizeof(IO_HALO));
		io->halo_native.dynlight = -1;
		io->halo.dynlight = -1;
		ARX_HALO_SetToNative(io);

		if (ais->inventory_skin[0])
		{
			io->inventory_skin = (char *) malloc(strlen(ais->inventory_skin) + 1);

			if (!io->inventory_skin) HERMES_Memory_Emergency_Out();

			strcpy(io->inventory_skin, ais->inventory_skin);
		}
		else io->stepmaterial = NULL;

		if (ais->stepmaterial[0])
		{
			io->stepmaterial = (char *) malloc(strlen(ais->stepmaterial) + 1);

			if (!io->stepmaterial) HERMES_Memory_Emergency_Out();

			strcpy(io->stepmaterial, ais->stepmaterial);
		}
		else io->stepmaterial = NULL;

		if (ais->armormaterial[0])
		{
			io->armormaterial = (char *) malloc(strlen(ais->armormaterial) + 1);

			if (!io->armormaterial) HERMES_Memory_Emergency_Out();

			strcpy(io->armormaterial, ais->armormaterial);
		}
		else io->armormaterial = NULL;

		if (ais->weaponmaterial[0])
		{
			io->weaponmaterial = (char *) malloc(strlen(ais->weaponmaterial) + 1);

			if (!io->weaponmaterial) HERMES_Memory_Emergency_Out();

			strcpy(io->weaponmaterial, ais->weaponmaterial);
		}
		else io->weaponmaterial = NULL;

		if (ais->strikespeech[0])
		{
			io->strikespeech = (char *) malloc(strlen(ais->strikespeech) + 1);

			if (!io->strikespeech) HERMES_Memory_Emergency_Out();

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

				if ((strlen(Project.workingdir) + strlen(ais->anims[i])) > 256)
				{
					continue;
				}

				sprintf(tex, "%s%s", Project.workingdir, ais->anims[i]);
				io->anims[i] = EERIE_ANIMMANAGER_Load(tex);

				if (io->anims[i] == NULL)
				{
					if (io->ioflags & IO_NPC)
						sprintf(tex, "%sGRAPH\\OBJ3D\\ANIMS\\NPC\\%s%s", Project.workingdir, GetName(ais->anims[i]), GetExt(ais->anims[i]));
					else
						sprintf(tex, "%sGRAPH\\OBJ3D\\ANIMS\\FIX_Inter\\%s%s", Project.workingdir, GetName(ais->anims[i]), GetExt(ais->anims[i]));

					io->anims[i] = EERIE_ANIMMANAGER_Load(tex);
				}
			}
		}

		memcpy(&io->spellcast_data, &ais->spellcast_data, sizeof(IO_SPELLCAST_DATA));
		memcpy(&io->physics, &ais->physics, sizeof(IO_PHYSICS));
		memcpy(&io->animlayer, &ais->animlayer, sizeof(ANIM_USE)*MAX_ANIM_LAYERS);

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
				scr_timer[num].namelength = strlen(ats->name) + 1;
				scr_timer[num].name = (char *) malloc(scr_timer[num].namelength);

				if (!scr_timer[num].name) HERMES_Memory_Emergency_Out();

				strcpy(scr_timer[num].name, ats->name);
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

		io->script.allowevents = ass->allowevents;
		io->script.nblvar = 0;

		if (io->script.lvar)
		{
			free(io->script.lvar);
			io->script.lvar = NULL;
		}

		if (ass->nblvar > 0)
		{
			io->script.lvar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * ass->nblvar);

			if (!io->script.lvar) HERMES_Memory_Emergency_Out();
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
					F2L(avs->fval, &io->script.lvar[i].ival);
					io->script.lvar[i].type = TYPE_L_TEXT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if (io->script.lvar[i].ival)
					{
						io->script.lvar[i].text = (char *) malloc(io->script.lvar[i].ival + 1);

						if (!io->script.lvar[i].text) HERMES_Memory_Emergency_Out();

						memset(io->script.lvar[i].text, 0, io->script.lvar[i].ival + 1);
						memcpy(io->script.lvar[i].text, dat + pos, io->script.lvar[i].ival);
						pos += io->script.lvar[i].ival;
						io->script.lvar[i].ival = strlen(io->script.lvar[i].text) + 1;

						if (io->script.lvar[i].text[0] == 'Ì')
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
					F2L(avs->fval, &io->script.lvar[i].ival);
					io->script.lvar[i].type = TYPE_L_LONG;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				case TYPE_L_FLOAT:
					strcpy(io->script.lvar[i].name, avs->name);
					io->script.lvar[i].fval = avs->fval;
					F2L(avs->fval, &io->script.lvar[i].ival);
					io->script.lvar[i].type = TYPE_L_FLOAT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				default:

					if ((avs->name[0] == '$') || (avs->name[0] == '£'))
					{
						avs->type = TYPE_L_TEXT;
						goto retry;
					}

					if ((avs->name[0] == '#') || (avs->name[0] == '§'))
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

		io->over_script.allowevents = ass->allowevents;

		io->over_script.nblvar = 0; 

		if (io->over_script.lvar)
		{
			free(io->over_script.lvar);
			io->over_script.lvar = NULL;
		}

		if (ass->nblvar)
		{
			io->over_script.lvar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * ass->nblvar);

			if (!io->over_script.lvar) HERMES_Memory_Emergency_Out();
		}
		//"Script Var"
		else io->over_script.lvar = NULL;

		io->over_script.nblvar = ass->nblvar;

		pos += sizeof(ARX_SCRIPT_SAVE);

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
					F2L(avs->fval, &io->over_script.lvar[i].ival);
					io->over_script.lvar[i].type = TYPE_L_TEXT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);

					if (io->over_script.lvar[i].ival)
					{
						io->over_script.lvar[i].text = (char *) malloc(io->over_script.lvar[i].ival + 1);

						if (!io->over_script.lvar[i].text) HERMES_Memory_Emergency_Out();

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
					F2L(avs->fval, &io->over_script.lvar[i].ival);
					io->over_script.lvar[i].type = TYPE_L_LONG;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				case TYPE_L_FLOAT:
					strcpy(io->over_script.lvar[i].name, avs->name);
					io->over_script.lvar[i].fval = avs->fval;
					F2L(avs->fval, &io->over_script.lvar[i].ival);
					io->over_script.lvar[i].type = TYPE_L_FLOAT;
					pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
					break;
				default:

					if ((avs->name[0] == '$') || (avs->name[0] == '£'))
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
					io->_npcdata->weapontype = as->weapontype;
					io->_npcdata->xpvalue = as->xpvalue;
					memcpy(io->_npcdata->stacked, as->stacked, sizeof(IO_BEHAVIOR_DATA)*MAX_STACKED_BEHAVIOR);

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

							if (!io->_npcdata->ex_rotate) HERMES_Memory_Emergency_Out();
						}

						memcpy(io->_npcdata->ex_rotate, &as->ex_rotate, sizeof(EERIE_EXTRA_ROTATE));
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
					memcpy(io->_itemdata->equipitem, &ai->equipitem, sizeof(IO_EQUIPITEM));
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
				memcpy(io->_camdata, &ac->cam, sizeof(EERIE_CAMERA));
				pos += sizeof(ARX_CHANGELEVEL_CAMERA_IO_SAVE);
				break;
			case TYPE_MARKER:
				ARX_CHANGELEVEL_MARKER_IO_SAVE * am;
				am = (ARX_CHANGELEVEL_MARKER_IO_SAVE *)(dat + pos);
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

				if (!io->inventory) HERMES_Memory_Emergency_Out();
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

			//{
			io->tweakerinfo = (IO_TWEAKER_INFO *) malloc(sizeof(IO_TWEAKER_INFO));

			if (!io->tweakerinfo) HERMES_Memory_Emergency_Out();

			//}
			memcpy(io->tweakerinfo, dat + pos, sizeof(IO_TWEAKER_INFO));
			pos += sizeof(IO_TWEAKER_INFO);

		}

		if (io->iogroups) free(io->iogroups);

		io->iogroups = NULL;

		if (io->nb_iogroups > 0)
		{
			io->iogroups = (IO_GROUP_DATA *) malloc(sizeof(IO_GROUP_DATA) * io->nb_iogroups);

			if (!io->iogroups) HERMES_Memory_Emergency_Out();

			IO_GROUP_DATA * igd = (IO_GROUP_DATA *)(dat + pos);
			memcpy(io->iogroups, igd, sizeof(IO_GROUP_DATA)*io->nb_iogroups);
			pos += sizeof(IO_GROUP_DATA) * io->nb_iogroups;
		}

		io->Tweak_nb = ais->Tweak_nb;

		if (io->Tweak_nb)
		{
			io->Tweaks = (TWEAK_INFO *)malloc(sizeof(TWEAK_INFO) * io->Tweak_nb);

			if (!io->Tweaks) HERMES_Memory_Emergency_Out();

			TWEAK_INFO * ti = (TWEAK_INFO *)(dat + pos);
			pos += sizeof(TWEAK_INFO) * io->Tweak_nb;
			memcpy(io->Tweaks, ti, sizeof(TWEAK_INFO)*io->Tweak_nb);
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

				if (!io->obj->linked) HERMES_Memory_Emergency_Out();

				for (long n = 0; n < ais->nb_linked; n++)
				{
					io->obj->linked[n].lgroup = ais->linked_data[n].lgroup;
					io->obj->linked[n].lidx = ais->linked_data[n].lidx;
					io->obj->linked[n].lidx2 = ais->linked_data[n].lidx2;
					memcpy(&io->obj->linked[n].modinfo, &ais->linked_data[n].modinfo, sizeof(EERIE_MOD_INFO));
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
		char temp[512];
		sprintf(temp, "CHANGELEVEL Error: Unable to load %s", ident);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(temp);
	}


	free(dat);
	CONVERT_CREATED = 1;

	if (NEED_LOG)
	{
		char temp[256];
		sprintf(temp, "--> After  ARX_CHANGELEVEL_Pop_IO(%s)", ident);
		LogData(temp);
	}

	return GetInterNum(tmp);
corrupted:
	char cstring[256];
	sprintf(cstring, "Save File Is Corrupted\nTrying to Fix %s", ident);
	ShowPopup(cstring);//"Save file is corrupted.");

	free(dat);
	io->inventory = NULL; 
	RestoreInitialIOStatusOfIO(io);
	SendInitScriptEvent(io);

	return idx;
}
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_PopAllIO(ARX_CHANGELEVEL_INDEX * asi)
{
	float increment = 0;

	if (asi->nb_inter > 0)
	{
		increment = (60.f / (float)asi->nb_inter);
	}
	else
	{
		PROGRESS_BAR_COUNT += 60;
		LoadLevelScreen();
	}

	for (long i = 0; i < asi->nb_inter; i++)
	{
		if ((i == 4) || (i == 62))
		{
			i = i;
		}

		PROGRESS_BAR_COUNT += increment;
		LoadLevelScreen();
		char tempo[256];
		sprintf(tempo, "%s_%04d", GetName(idx_io[i].filename), idx_io[i].ident);
		ARX_CHANGELEVEL_Pop_IO(tempo);
	}

	return 1;
}
extern void GetIOCyl(INTERACTIVE_OBJ * io, EERIE_CYLINDER * cyl);
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_PopAllIO_FINISH(ARX_CHANGELEVEL_INDEX * asi, long reloadflag)
{
	unsigned char * treated = (unsigned char *) malloc(sizeof(unsigned char) * MAX_IO_SAVELOAD);

	if (!treated) HERMES_Memory_Emergency_Out();

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

						if (!stricmp(aids->targetinfo, "NONE")) io->targetinfo = -1;
						else if (!stricmp(aids->targetinfo, "SELF")) io->targetinfo = -2;
						else if (!stricmp(aids->targetinfo, "PLAYER")) io->targetinfo = 0;
						else
							io->targetinfo = GetInterNum(ConvertToValidIO(aids->targetinfo));

						if (io->ioflags & IO_NPC)
						{
							for (long iii = 0; iii < MAX_STACKED_BEHAVIOR; iii++)
							{
								if (!stricmp(aids->stackedtarget[iii], "NONE")) io->_npcdata->stacked[iii].target = -1;
								else if (!stricmp(aids->stackedtarget[iii], "SELF")) io->_npcdata->stacked[iii].target = -2;
								else if (!stricmp(aids->stackedtarget[iii], "PLAYER")) io->_npcdata->stacked[iii].target = 0;
								else
									io->_npcdata->stacked[iii].target = GetInterNum(ConvertToValidIO(aids->stackedtarget[iii]));
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
					SendScriptEvent(&inter.iobj[i]->script, SM_RELOAD, "CHANGE", inter.iobj[i], NULL);
				}

				if (inter.iobj[i]
				        &&	inter.iobj[i]->over_script.data)
				{
					SendScriptEvent(&inter.iobj[i]->over_script, SM_RELOAD, "CHANGE", inter.iobj[i], NULL);
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
					SendScriptEvent(&inter.iobj[i]->script, SM_INIT, "", inter.iobj[i], NULL);
				}

				if (inter.iobj[i]
				        &&	inter.iobj[i]->over_script.data)
				{
					SendScriptEvent(&inter.iobj[i]->over_script, SM_INIT, "", inter.iobj[i], NULL);
				}

				if (inter.iobj[i]
				        &&	inter.iobj[i]->script.data)
				{
					SendScriptEvent(&inter.iobj[i]->script, SM_INITEND, "", inter.iobj[i], NULL);
				}

				if (inter.iobj[i]
				        &&	inter.iobj[i]->over_script.data)
				{
					SendScriptEvent(&inter.iobj[i]->over_script, SM_INITEND, "", inter.iobj[i], NULL);
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
//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Pop_Globals()
{
	ARX_CHANGELEVEL_SAVE_GLOBALS * acsg;
	unsigned char * dat;
	long pos = 0;
	char loadfile[256];
	long size;
	char _error[256];

	ARX_SCRIPT_Free_All_Global_Variables();
	sprintf(loadfile, "Globals.sav");
	size = _pSaveBlock->GetSize(loadfile);

	if (size < 0)
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(_error);

		return -1;
	}

	char * compressed = (char *) GetStdBuffer(size); 

	if (!compressed) HERMES_Memory_Emergency_Out();

	if (!_pSaveBlock->Read(loadfile, (char *)compressed))
	{
		sprintf(_error, "Unable to Open %s for Read...", loadfile);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(_error);

		return -1;
	}

	for (long i = 0; i < size; i += 2)
	{
		compressed[i] = ~compressed[i];
	}

	long ssize = size;
 
	dat = (unsigned char *)STD_Explode(compressed, ssize, &size);
	acsg = (ARX_CHANGELEVEL_SAVE_GLOBALS *)(dat);
	pos += sizeof(ARX_CHANGELEVEL_SAVE_GLOBALS);

	if (acsg->version != ARX_GAMESAVE_VERSION)
	{
		free(dat);
		sprintf(_error, "Invalid version: %s...", loadfile);

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup(_error);

		return -1;
	}

	if (acsg->nb_globals > 0)
	{
		svar = (SCRIPT_VAR *) malloc(sizeof(SCRIPT_VAR) * acsg->nb_globals);

		if (!svar) HERMES_Memory_Emergency_Out();
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
				F2L(av->fval, &svar[i].ival);
				svar[i].type = TYPE_G_TEXT;

				if (svar[i].ival)
				{
					svar[i].text = (char *) malloc(svar[i].ival + 1);
					memset(svar[i].text, 0, svar[i].ival + 1);

					if (!svar[i].text) HERMES_Memory_Emergency_Out();

					memcpy(svar[i].text, dat + pos + sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE), svar[i].ival);

					if (svar[i].text[0] == 'Ì')
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
				F2L(av->fval, &svar[i].ival);
				svar[i].fval = av->fval;
				svar[i].text = NULL;
				svar[i].type = TYPE_G_LONG;
				pos += sizeof(ARX_CHANGELEVEL_VARIABLE_SAVE);
				break;
			case TYPE_G_FLOAT:
				strcpy(svar[i].name, av->name);
				F2L(av->fval, &svar[i].ival);
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
#include <EERIECollisionSpheres.h>
#include "DanaeDlg.h"
void ReplaceSpecifics(char * text);

//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_PopLevel(long instance, long reloadflag)
{
	DANAE_ReleaseAllDatasDynamic();

	LogData("Before ARX_CHANGELEVEL_PopLevel Alloc'n'Free");

	if (_Gaids) ReleaseGaids();

	_Gaids = (ARX_CHANGELEVEL_INVENTORY_DATA_SAVE **) malloc(sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *) * MAX_IO_SAVELOAD);

	if (!_Gaids) HERMES_Memory_Emergency_Out();

	memset(_Gaids, 0, sizeof(ARX_CHANGELEVEL_INVENTORY_DATA_SAVE *)*MAX_IO_SAVELOAD);

	ARX_CHANGELEVEL_INDEX asi;
	ARX_CHANGELEVEL_PLAYER asp;

	CURRENT_GAME_INSTANCE = instance;
	ARX_CHANGELEVEL_MakePath();

	if (!DirectoryExist(CurGamePath))
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Cannot Load this game: Directory Not Found");

		RELOADING = 0;
		ReleaseGaids();
		return -1;
	}

	LogData("After  ARX_CHANGELEVEL_PopLevel Alloc'n'Free");

	// Clears All Scene contents...
	LogData("Before DANAE ClearAll");
	DanaeClearAll();
	LogData("After  DANAE ClearAll");

	ARX_TIME_Pause();
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;


	// Now we can load our things...
	char loadfile[256];
	long FirstTime;
	sprintf(loadfile, "lvl%03d.sav", instance);

	LogData("Before Saveblock Access");
	// Open Saveblock for read
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", CurGamePath);
	_pSaveBlock = new CSaveBlock(sfile);
	_pSaveBlock->BeginRead();
	LogData("After  Saveblock Access");

	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen(GDevice, instance);

	// first time in this level ?
	if (!FileExist(sfile))
	{
		FirstTime = 1;
		FORBID_SCRIPT_IO_CREATION = 0;
		NO_PLAYER_POSITION_RESET = 0;
	}
	else if (_pSaveBlock->GetSize(loadfile) < 0)
	{
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

	_FIRSTTIME = FirstTime;
	NEW_LEVEL = instance;

	if (!FirstTime)
	{
		LogData("Before ARX_CHANGELEVEL_Pop_Index");

		if (ARX_CHANGELEVEL_Pop_Index(&asi, instance) != 1)
		{
			FreeStdBuffer();

			if (!FOR_EXTERNAL_PEOPLE)
				ShowPopup("Cannot Load Index data");

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

		LogData("After  ARX_CHANGELEVEL_Pop_Index");

		if (asi.version != ARX_GAMESAVE_VERSION)
		{
			FreeStdBuffer();
			ShowPopup("Invalid Save Version...");
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
	LoadLevelScreen(GDevice, instance);
	LogData("Before ARX_CHANGELEVEL_Pop_Level");

	if (ARX_CHANGELEVEL_Pop_Level(&asi, instance, FirstTime) != 1)
	{
		FreeStdBuffer();

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Cannot Load Level data");

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



	LogData("After  ARX_CHANGELEVEL_Pop_Index");
	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(GDevice, instance);

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
		LogData("Before ARX_CHANGELEVEL_PopAllIO");

		if (ARX_CHANGELEVEL_PopAllIO(&asi) != 1)
		{
			FreeStdBuffer();

			if (!FOR_EXTERNAL_PEOPLE)
				ShowPopup("Cannot Load IO data");

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

		LogData("After  ARX_CHANGELEVEL_PopAllIO");
	}

	PROGRESS_BAR_COUNT += 20.f;
	LoadLevelScreen(GDevice, instance);
	LogData("Before ARX_CHANGELEVEL_Pop_Player");

	if (ARX_CHANGELEVEL_Pop_Player(&asi, &asp) != 1)
	{
		FreeStdBuffer();

		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Cannot Load Player data");

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

	LogData("After  ARX_CHANGELEVEL_Pop_Player");
	PROGRESS_BAR_COUNT += 10.f;
	LoadLevelScreen();
	LogData("Before Misc Code");

	// Restoring Player equipment...
	player.equipsecondaryIO = ConvertToValidIO(asp.equipsecondaryIO);
	player.equipshieldIO = ConvertToValidIO(asp.equipshieldIO);
	player.leftIO = ConvertToValidIO(asp.leftIO);
	player.rightIO = ConvertToValidIO(asp.rightIO);
	CURRENT_TORCH = ConvertToValidIO(asp.curtorch);
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();

	for (int iNbBag = 0; iNbBag < 3; iNbBag++)
	{
		for (long m = 0; m < INVENTORY_Y; m++)
			for (long n = 0; n < INVENTORY_X; n++)
			{
				inventory[iNbBag][n][m].io = ConvertToValidIO(asp.id_inventory[iNbBag][n][m]);
				inventory[iNbBag][n][m].show = asp.inventory_show[iNbBag][n][m];
			}
	}

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		player.equiped[i] = (short)GetInterNum(ConvertToValidIO(asp.equiped[i]));

		if ((player.equiped[i] > 0) && (ValidIONum(player.equiped[i])))
			inter.iobj[player.equiped[i]]->level = (short)instance;
		else player.equiped[i] = 0;
	}

	PROGRESS_BAR_COUNT += 2.f;
	LoadLevelScreen();
	LogData("After Misc Code");

	LogData("Before ARX_CHANGELEVEL_PopAllIO_FINISH");
	// Restoring all Missing Objects required by other objects...
	ARX_CHANGELEVEL_PopAllIO_FINISH(&asi, reloadflag);
	LogData("After  ARX_CHANGELEVEL_PopAllIO_FINISH");
	PROGRESS_BAR_COUNT += 15.f;
	LoadLevelScreen();
	ReleaseGaids();
	PROGRESS_BAR_COUNT += 3.f;
	LoadLevelScreen();

	if (!FirstTime)
	{
		LogData("Before ARX_CHANGELEVEL_Pop_Zones_n_Lights");
		ARX_CHANGELEVEL_Pop_Zones_n_Lights(&asi, instance);
		LogData("After  ARX_CHANGELEVEL_Pop_Zones_n_Lights");
	}

	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();
	LogData("Before Player Misc Init");
	ForcePlayerInventoryObjectLevel(instance);
	ARX_EQUIPMENT_RecreatePlayerMesh();
	PROGRESS_BAR_COUNT += 1.f;
	LoadLevelScreen();

	ReleaseTio();
	ARX_TIME_Force_Time_Restore(ARX_CHANGELEVEL_DesiredTime);

	NO_TIME_INIT = 1;
	FORCE_TIME_RESTORE = ARX_CHANGELEVEL_DesiredTime;
	LogData("After  Player Misc Init");

	LogData("Before Memory Release");

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
	FreeStdBuffer();
	LogData("After  Memory Release");

	LogData("Before SaveBlock Release");
	_pSaveBlock->EndRead();
	delete _pSaveBlock;
	_pSaveBlock = NULL;
	LogData("After  SaveBlock Release");

	LogData("Before Final Inits");
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
	LogData("After  Final Inits");
	return 1;
}





//-----------------------------------------------------------------------------
// copie un rep (récursif sous reps) dans un autre en créant les reps
// écrase les fichiers pour les mettre à jour
void CopyDirectory(char * _lpszSrc, char * _lpszDest)
{
	CreateDirectory(_lpszDest, NULL);

	//	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	printf("Target file is %s.\n", _lpszSrc);

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
				CopyFile(s, d, false);
			}
		}
	}
	while (FindNextFile(hFind, FindFileData) > 0);

	if (hFind)
		FindClose(hFind);
}

//-----------------------------------------------------------------------------
///////////////////////// SAVE LOAD
long ARX_CHANGELEVEL_Save(long instance, char * name)
{

	ARX_TIME_Pause();

	if (instance <= 0)
	{
		ARX_GAMESAVE_CreateNewInstance();
		instance = CURRENT_GAME_INSTANCE;
	}

	CURRENT_GAME_INSTANCE = instance;

	if (instance == -1)
	{
		// fatality...
		ShowPopup("Internal Non-Fatal Error");
		return 0;
	}

	if (CURRENTLEVEL == -1)
	{
		// fatality...
		ShowPopup("Internal Non-Fatal Error");
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
			SendIOScriptEvent(io, SM_INVENTORY2_CLOSE, "");
			TSecondaryInventory = SecondaryInventory;
			SecondaryInventory = NULL;
		}
	}

	ARX_CHANGELEVEL_MakePath();
	ARX_CHANGELEVEL_PushLevel(CURRENTLEVEL, CURRENTLEVEL);
	KillAllDirectory(GameSavePath); 
	CopyDirectory(CurGamePath, GameSavePath);

	//on copie le fichier temporaire bmp dans le repertoire
	char tcSrc[256];
	char tcDst[256];
	sprintf(tcSrc, "%sSCT_0.BMP", Project.workingdir);
	sprintf(tcDst, "%sGSAVE.BMP", GameSavePath);
	CopyFile(tcSrc, tcDst, FALSE);
	DeleteFile(tcSrc);

	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;
	memset(&pld, 0, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	pld.level = CURRENTLEVEL;
	strcpy(pld.name, name);
	pld.version = ARX_GAMESAVE_VERSION;
	pld.time = ARX_TIME_GetUL(); //treat warning C4244 conversion from 'float' to 'unsigned long''
	ARX_CHANGELEVEL_Set_Player_LevelData(&pld, GameSavePath);
	ARX_TIME_UnPause();
	return 1;
}

//-----------------------------------------------------------------------------
long ARX_CHANGELEVEL_Set_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA * pld, char * path)
{
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", path);
	_pSaveBlock = new CSaveBlock(sfile);

	if (!_pSaveBlock->BeginSave(true, 1)) return -1;

	if (!DirectoryExist(path)) return -1;

	unsigned char * dat;
	dat = (unsigned char *) malloc(sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));

	if (!dat) HERMES_Memory_Emergency_Out();

	memcpy(dat, pld, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));
	long pos = sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA);
	char savefile[256];
	sprintf(savefile, "pld.sav");

	char * compressed = NULL;
	long cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);

	for (long i = 0; i < cpr_pos; i += 2)
		compressed[i] = ~compressed[i];

	_pSaveBlock->Save(savefile, compressed, cpr_pos);
	free(compressed);

	_pSaveBlock->EndSave();
	delete _pSaveBlock;
	_pSaveBlock = 0;

	free(dat);
	return 1;
}
//------------------------------------------------------------------------------
// ARX_CHANGELEVEL_Get_Player_LevelData: Retreives Player Level Data
// VERIFIED: Cyril 30/11/2001
//------------------------------------------------------------------------------
long ARX_CHANGELEVEL_Get_Player_LevelData(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA * pld, char * path)
{
	// Checks For Directory
	if (!DirectoryExist(path)) return -1;

	char loadfile[256];
	char _error[256];
	long size;
	unsigned char * dat;

	// Open Save Block
	char sfile[256];
	sprintf(sfile, "%sGsave.sav", path);
	_pSaveBlock = new CSaveBlock(sfile);

	if (!_pSaveBlock->BeginRead()) return -1;

	// Get Size
	sprintf(loadfile, "pld.sav");
	size = _pSaveBlock->GetSize(loadfile);

	// Checks for Void/Invalid File
	if (size <= 0)
	{
		sprintf(_error, "Unable to Open %s for Read1...", loadfile);
		ShowPopup(_error);
		_pSaveBlock->EndRead();
		delete _pSaveBlock;
		_pSaveBlock = 0;
		return -1;
	}

	// Allocate Necessary Size
	char * compressed = (char *) malloc(size);

	if (!compressed) HERMES_Memory_Emergency_Out();

	// Read Block
	if (!_pSaveBlock->Read(loadfile, (char *)compressed))
	{
		free(compressed);
		sprintf(_error, "Unable to Open %s for Read2...", loadfile);
		ShowPopup(_error);
		_pSaveBlock->EndRead();
		delete _pSaveBlock;
		_pSaveBlock = 0;
		return -1;
	}

	// Un-Crypt
	for (long i = 0; i < size; i += 2)
		compressed[i] = ~compressed[i];

	// Explode File
	long ssize = size;
 
	dat = (unsigned char *)STD_Explode(compressed, ssize, &size); //pos,&cpr_pos);
	free(compressed);

	if (dat == NULL)
	{
		sprintf(_error, "Unable to Explode %s...", loadfile);
		ShowPopup(_error);
		_pSaveBlock->EndRead();
		delete _pSaveBlock;
		_pSaveBlock = 0;
		return -1;
	}

	// Finishes Read
	_pSaveBlock->EndRead();
	delete _pSaveBlock;
	_pSaveBlock = 0;

	// Stores Data in pld
	memcpy(pld, dat, sizeof(ARX_CHANGELEVEL_PLAYER_LEVEL_DATA));

	if (pld->version != ARX_GAMESAVE_VERSION)
	{
		ShowPopup("Invalid GameSave Version");
		free(dat);
		return -1;
	}

	// Release Data
	free(dat);
	return 1;
}
long DONT_CLEAR_SCENE;
extern long STARTED_A_GAME;
//------------------------------------------------------------------------------
// ARX_CHANGELEVEL_Load: Load a GameSave
//------------------------------------------------------------------------------
long ARX_CHANGELEVEL_Load(long instance)
{
	LogData("-----------------------------------");
	HERMES_DATE_TIME hdt;
	GetDate(&hdt);

	if (NEED_LOG)
	{
		LogData("ARX_CHANGELEVEL_Load");
		char tex[256];
		sprintf(tex, "Date: %02d/%02d/%d  Time: %dh%d", hdt.days, hdt.months, hdt.years, hdt.hours, hdt.mins);
		LogData(tex);
	}

	iTimeToDrawD7 = -3000;

	PROGRESS_BAR_TOTAL = 238; 
	OLD_PROGRESS_BAR_COUNT = PROGRESS_BAR_COUNT = 0;

	// Forbid Saving
	FORBID_SAVE = 1;
	ARX_TIME_Pause();

	// Checks Instance
	if (instance <= -1)
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Internal Non-Fatal Error");

		return -1;
	}

	// Checks/Create GameSavePath
	CURRENT_GAME_INSTANCE = instance;
	ARX_GAMESAVE_MakePath();

	if (!DirectoryExist(GameSavePath))
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Unknown SavePath");

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

	if (ARX_CHANGELEVEL_Get_Player_LevelData(&pld, CurGamePath) == 1)
	{
		PROGRESS_BAR_COUNT += 2.f;
		LoadLevelScreen(GDevice, pld.level);

		if (pld.level == CURRENTLEVEL)
			DONT_CLEAR_SCENE = 1;
		else
			DONT_CLEAR_SCENE = 0;


		float fPldTime = ARX_CLEAN_WARN_CAST_FLOAT(pld.time);
		DanaeClearLevel();
		PROGRESS_BAR_COUNT			+=	2.f;
		LoadLevelScreen(GDevice, pld.level);
		CURRENTLEVEL				=	pld.level;
		ARX_CHANGELEVEL_DesiredTime	=	fPldTime;
		ARX_CHANGELEVEL_PopLevel(pld.level, 0);
		FreeStdBuffer();
		ARX_TIME_Force_Time_Restore(fPldTime);
		NO_TIME_INIT				=	1;
		FORCE_TIME_RESTORE			=	fPldTime;
		DONT_CLEAR_SCENE			=	0;

	}
	else
	{
		if (!FOR_EXTERNAL_PEOPLE)
			ShowPopup("Error Loading Level...");

		return -1;
	}

	STARTED_A_GAME = 1;
	BLOCK_PLAYER_CONTROLS = 0;
	player.Interface &= ~INTER_COMBATMODE;

	if (inter.iobj[0]) inter.iobj[0]->animlayer[1].cur_anim = NULL;

	JUST_RELOADED = 1;
	return 1;
}
//------------------------------------------------------------------------------
// ARX_CHANGELEVEL_GetInfo: Retreives Name & Time of a Saved game in "path"
//------------------------------------------------------------------------------
long ARX_CHANGELEVEL_GetInfo(char * path, char * name, float * version, long * level, unsigned long * time)
{
	if ((!path) || (!name) || (!version) || (!level) || (!time)) return -1;

	ARX_CHANGELEVEL_PLAYER_LEVEL_DATA pld;

	if (ARX_CHANGELEVEL_Get_Player_LevelData(&pld, path) == 1)
	{
		strcpy(name, pld.name);
		*version = pld.version;
		*level = pld.level;
		*time = (pld.time / 1000);
		return 1;
	}
	else
	{
		strcpy(name, "Invalid Save...");
		*time = 0;
		*level = 0;
		*version = 0;
		return -1;
	}
}
//------------------------------------------------------------------------------
