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
// ARX_Script
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Script Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "script/Script.h"

#include <stddef.h>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <algorithm>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Localisation.h"
#include "core/Dialog.h"
#include "core/Resource.h"
#include "core/Core.h"
#include "core/Config.h"

#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Inventory.h"

#include "gui/Speech.h"

#include "graphics/particle/ParticleEffects.h"
#include "graphics/Math.h"

#include "io/FilePath.h"
#include "io/Logger.h"
#include "io/PakReader.h"

#include "platform/String.h"

#include "scene/Scene.h"
#include "scene/Interactive.h"

#include "script/ScriptEvent.h"

using std::sprintf;
using std::min;
using std::max;
using std::transform;
using std::string;

#define MAX_SSEPARAMS 5

extern long FINAL_COMMERCIAL_DEMO;
extern long lChangeWeapon;
extern INTERACTIVE_OBJ * pIOChangeWeapon;

std::string ShowText;
std::string ShowText2;
std::string ShowTextWindowtext;
INTERACTIVE_OBJ * LASTSPAWNED = NULL;
INTERACTIVE_OBJ * EVENT_SENDER = NULL;
SCRIPT_VAR * svar = NULL;

char SSEPARAMS[MAX_SSEPARAMS][64];
long FORBID_SCRIPT_IO_CREATION = 0;
long NB_GLOBALS = 0;
SCR_TIMER * scr_timer = NULL;
long ActiveTimers = 0;

//*************************************************************************************
// FindScriptPos																	//
// Looks for string in script, return pos. Search start position can be set using	//
// poss parameter.																	//
//*************************************************************************************
long FindScriptPos(const EERIE_SCRIPT * es, const std::string& str)
{

	if (!es->data) return -1;

	const char * pdest = strcasestr(es->data, str.c_str());
	
	if(!pdest) {
		return -1;
	}
	
	long result = pdest - es->data;

	assert(result >= 0);

	int len2 = str.length();
	
	assert(len2 + result <= (int)es->size);

	if (es->data[result+len2] <= 32) return result;

	return -1;
}

ScriptResult SendMsgToAllIO(ScriptMessage msg, const string & params) {
	
	ScriptResult ret = ACCEPT;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
		{
			if (SendIOScriptEvent(inter.iobj[i], msg, params) == REFUSE)
				ret = REFUSE;
		}
	}

	return ret;
}

void ARX_SCRIPT_SetMainEvent(INTERACTIVE_OBJ * io, const string & newevent) {
	
	if(!io) {
		return;
	}
	
	if(newevent == "main") {
		io->mainevent[0] = 0;
	} else {
		strcpy(io->mainevent, newevent.c_str());
	}
}

//*************************************************************************************
//*************************************************************************************
void ARX_SCRIPT_ResetObject(INTERACTIVE_OBJ * io, long flags)
{
	// Now go for Script INIT/RESET depending on Mode
	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		if (inter.iobj[num] && inter.iobj[num]->script.data)
		{
			inter.iobj[num]->script.allowevents = 0;

			if (flags)	ScriptEvent::send(&inter.iobj[num]->script, SM_INIT, "", inter.iobj[num], "");


			if (inter.iobj[num])
				ARX_SCRIPT_SetMainEvent(inter.iobj[num], "main");
		}

		// Do the same for Local Script
		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			inter.iobj[num]->over_script.allowevents = 0;

			if (flags)	ScriptEvent::send(&inter.iobj[num]->over_script, SM_INIT, "", inter.iobj[num], "");


		}

		// Sends InitEnd Event
		if (flags)
		{
			if (inter.iobj[num] && inter.iobj[num]->script.data)
				ScriptEvent::send(&inter.iobj[num]->script, SM_INITEND, "", inter.iobj[num], "");

			if (inter.iobj[num] && inter.iobj[num]->over_script.data)
				ScriptEvent::send(&inter.iobj[num]->over_script, SM_INITEND, "", inter.iobj[num], "");
		}

		if (inter.iobj[num])
			inter.iobj[num]->GameFlags &= ~GFLAG_NEEDINIT;
	}
}
void ARX_SCRIPT_Reset(INTERACTIVE_OBJ * io, long flags)
{



	//Release Script Local Variables
	if (io->script.lvar)
	{
		for (long n = 0; n < io->script.nblvar; n++)
		{
			if (io->script.lvar[n].text)
			{
				free((void *)io->script.lvar[n].text);
				io->script.lvar[n].text = NULL;
			}
		}

		io->script.nblvar = 0;
		free((void *)io->script.lvar);
		io->script.lvar = NULL;
	}

	//Release Script Over-Script Local Variables
	if (io->over_script.lvar)
	{
		for (long n = 0; n < io->over_script.nblvar; n++)
		{
			if (io->over_script.lvar[n].text)
			{
				free((void *)io->over_script.lvar[n].text);
				io->over_script.lvar[n].text = NULL;
			}
		}

		io->over_script.nblvar = 0;
		free((void *)io->over_script.lvar);
		io->over_script.lvar = NULL;
	}

	if (!io->scriptload)
		ARX_SCRIPT_ResetObject(io, flags);
}
void ARX_SCRIPT_ResetAll(long flags)
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			if (!inter.iobj[i]->scriptload)
				ARX_SCRIPT_Reset(inter.iobj[i], flags);
		}
	}
}

extern long PauseScript;
//*************************************************************************************
//*************************************************************************************
void ARX_SCRIPT_AllowInterScriptExec()
{
	static long ppos = 0;

	if ((!PauseScript) && (!EDITMODE) && (!ARXPausedTime))
	{
		EVENT_SENDER = NULL;

		long numm = min(inter.nbmax, 10L);

		for (long n = 0; n < numm; n++)
		{
			long i = ppos;
			ppos++;

			if (ppos >= inter.nbmax)
			{
				ppos = 0;
				return;
			}

			{
				if (inter.iobj[i] != NULL)
					if (inter.iobj[i]->GameFlags & GFLAG_ISINTREATZONE)
					{
						if(inter.iobj[i]->mainevent[0])
							SendIOScriptEvent(inter.iobj[i], SM_NULL, "", inter.iobj[i]->mainevent);
						else SendIOScriptEvent(inter.iobj[i], SM_MAIN);
					}
			}
		}
	}
}

void ARX_SCRIPT_ReleaseLabels(EERIE_SCRIPT * es)
{
	if (!es) return;

	if (!es->labels) return;

	for (long i = 0; i < es->nb_labels; i++)
	{
		if (es->labels[i].string)
			free((void *)es->labels[i].string);
	}

	free((void *)es->labels);
	es->labels = NULL;
	es->nb_labels = 0;
}

void ReleaseScript(EERIE_SCRIPT * es)
{
	if (es == NULL) return;

	if (es->lvar)
	{
		for (long i = 0; i < es->nblvar; i++)
		{
			if (es->lvar[i].text != NULL)
			{
				free((void *)es->lvar[i].text);
				es->lvar[i].text = NULL;
			}
		}

		free((void *)es->lvar);
		es->lvar = NULL;
	}

	if (es->data != NULL)
	{
		free((void *)es->data);
		es->data = NULL;
	}

	ARX_SCRIPT_ReleaseLabels(es);
	memset(es->shortcut, 0, sizeof(long)*MAX_SHORTCUT);
}

ValueType GetSystemVar(const EERIE_SCRIPT * es, INTERACTIVE_OBJ * io, const string & _name, std::string& txtcontent, float * fcontent,long * lcontent)
{
	std::string name = _name;
	MakeUpcase(name);

	switch (name[1])
	{
		case '$':

			if (!name.compare("^$PARAM1"))
			{
				txtcontent = SSEPARAMS[0];
				return TYPE_TEXT;
			}

			if (!name.compare("^$PARAM2"))
			{
				txtcontent = SSEPARAMS[1];
				return TYPE_TEXT;
			}

			if (!name.compare("^$PARAM3"))
			{
				txtcontent = SSEPARAMS[2];
				return TYPE_TEXT;
			}

			if (!name.compare("^$OBJONTOP"))
			{
				txtcontent = "NONE";

				if (io)	MakeTopObjString(io,txtcontent);

				return TYPE_TEXT;
			}

			break;
		case '&':

			if (!name.compare("^&PARAM1"))
			{
				*fcontent = (float)atof(SSEPARAMS[0]);
				return TYPE_FLOAT;
			}

			if (!name.compare("^&PARAM2"))
			{
				*fcontent = (float)atof(SSEPARAMS[1]);
				return TYPE_FLOAT;
			}

			if (!name.compare("^&PARAM3"))
			{
				*fcontent = (float)atof(SSEPARAMS[2]);
				return TYPE_FLOAT;
			}

			if (!name.compare("^&PLAYERDIST"))
			{
				if (io)
				{
					*fcontent = fdist(player.pos, io->pos);
					return TYPE_FLOAT;
				}
			}

			break;
		case '#':

			if (!name.compare("^#PLAYERDIST"))
			{
				if (io != NULL)
				{
					*lcontent = (long)fdist(player.pos, io->pos);
					return TYPE_LONG;
				}
			}

			if (!name.compare("^#PARAM1"))
			{
				*lcontent = atol(SSEPARAMS[0]);
				return TYPE_LONG;
			}

			if (!name.compare("^#PARAM2"))
			{
				*lcontent = atol(SSEPARAMS[1]);
				return TYPE_LONG;
			}

			if (!name.compare("^#PARAM3"))
			{
				*lcontent = atol(SSEPARAMS[2]);
				return TYPE_LONG;
			}

			if (!name.compare("^#TIMER1"))
			{
				if (io != NULL)
				{
					if (io->script.timers[0] == 0) *lcontent = 0;
					else
					{
						unsigned long t = ARXTimeUL() - es->timers[0];
						*lcontent = (long)t;
					}
				}
				else *lcontent = 0;

				return TYPE_LONG;
			}

			if (!name.compare("^#TIMER2"))
			{
				if (io != NULL)
				{
					if (io->script.timers[1] == 0) *lcontent = 0;
					else
					{
						unsigned long t = ARXTimeUL() - es->timers[1];
						*lcontent = (long)t;
					}
				}
				else *lcontent = 0;

				return TYPE_LONG;
			}

			if (!name.compare("^#TIMER3"))
			{
				if (io != NULL)
				{
					if (io->script.timers[2] == 0) *lcontent = 0;
					else
					{
						unsigned long t = ARXTimeUL() - es->timers[2];
						*lcontent = (long)t;
					}
				}
				else *lcontent = 0;

				return TYPE_LONG;
			}

			if (!name.compare("^#TIMER4"))
			{
				if (io != NULL)
				{
					if (io->script.timers[3] == 0) *lcontent = 0;
					else
					{
						unsigned long t = ARXTimeUL() - es->timers[3];
						*lcontent = (long)t;
					}
				}
				else *lcontent = 0;

				return TYPE_LONG;
			}


			break;
		case 'G':

			if (!name.compare("^GORE"))
			{
				*lcontent = config.misc.gore ? 1 : 0;
				return TYPE_LONG;
			}

			if (!name.compare("^GAMEDAYS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 864000000);
				return TYPE_LONG;
			}

			if (!name.compare("^GAMEHOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 3600000);
				return TYPE_LONG;
			}

			if (!name.compare("^GAMEMINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 60000);
				return TYPE_LONG;
			}

			if (!name.compare("^GAMESECONDS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 1000);
				return TYPE_LONG;
			}

			break;
		case 'A':

			if (!specialstrcmp(name, "^AMOUNT"))
			{
				if ((io) && (io->ioflags & IO_ITEM))
				{
					*fcontent = io->_itemdata->count;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!name.compare("^ARXDAYS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 7200000);
				return TYPE_LONG;
			}

			if (!name.compare("^ARXHOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 600000);
				return TYPE_LONG;
			}

			if (!name.compare("^ARXMINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 10000);
				return TYPE_LONG;
			}

			if (!name.compare("^ARXSECONDS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 1000);
				*lcontent *= 6;
				return TYPE_LONG;
			}

			if (!name.compare("^ARXTIME_HOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 600000);

				while (*lcontent > 12) *lcontent -= 12;

				return TYPE_LONG;
			}

			if (!name.compare("^ARXTIME_MINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 10000);

				while (*lcontent > 60) *lcontent -= 60;

				return TYPE_LONG;
			}

			if (!name.compare("^ARXTIME_SECONDS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime * 6 / 1000);

				while (*lcontent > 60) *lcontent -= 60;

				return TYPE_LONG;
			}


			break;
		case 'R':

			if (!specialstrcmp(name, "^REALDIST_"))
			{
				if (io)
				{
					char * obj = &name[10];

					if (!strcmp(obj, "PLAYER"))
					{
						if (io->room_flags & 1)
							UpdateIORoom(io);

						long Player_Room = ARX_PORTALS_GetRoomNumForPosition(&player.pos, 1);

						*fcontent = SP_GetRoomDist(&io->pos, &player.pos, io->room, Player_Room);

						return TYPE_FLOAT;
					}

					long t = GetTargetByNameTarget(obj);

					if (ValidIONum(t))
					{
						if (((io->show == SHOW_FLAG_IN_SCENE) || (io->show == SHOW_FLAG_IN_INVENTORY))
								&& ((inter.iobj[t]->show == SHOW_FLAG_IN_SCENE) || (inter.iobj[t]->show == SHOW_FLAG_IN_INVENTORY))
						   )
						{
							Vec3f pos, pos2;
							GetItemWorldPosition(io, &pos);
							GetItemWorldPosition(inter.iobj[t], &pos2);

							if (io->room_flags & 1)
								UpdateIORoom(io);

							if (inter.iobj[t]->room_flags & 1)
								UpdateIORoom(inter.iobj[t]);

							*fcontent = SP_GetRoomDist(&pos, &pos2, io->room, inter.iobj[t]->room);
						}
						else // Out of this world item
							*fcontent = 99999999999.f;

						return TYPE_FLOAT;
					}

					*fcontent = 99999999999.f;
					return TYPE_FLOAT;
				}
			}

			if (!specialstrcmp(name, "^REPAIRPRICE_"))
			{
				char * obj = &name[13];
				long t = GetTargetByNameTarget(obj);

				if (ValidIONum(t))
				{
					*fcontent = ARX_DAMAGES_ComputeRepairPrice(inter.iobj[t], io);
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^RND_"))
			{
				char * max = &name[5];

				if (max[0])
				{
					float t = (float)atof(max);
					*fcontent = t * rnd();
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^RUNE_"))
			{
				char * temp = &name[6];

				if (!strcasecmp(temp, "AAM"))
				{
					*lcontent = player.rune_flags & FLAG_AAM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "CETRIUS"))
				{
					*lcontent = player.rune_flags & FLAG_CETRIUS;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "COMUNICATUM"))
				{
					*lcontent = player.rune_flags & FLAG_COMUNICATUM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "COSUM"))
				{
					*lcontent = player.rune_flags & FLAG_COSUM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "FOLGORA"))
				{
					*lcontent = player.rune_flags & FLAG_FOLGORA;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "FRIDD"))
				{
					*lcontent = player.rune_flags & FLAG_FRIDD;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "KAOM"))
				{
					*lcontent = player.rune_flags & FLAG_KAOM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "MEGA"))
				{
					*lcontent = player.rune_flags & FLAG_MEGA;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "MORTE"))
				{
					*lcontent = player.rune_flags & FLAG_MORTE;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "MOVIS"))
				{
					*lcontent = player.rune_flags & FLAG_MOVIS;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "NHI"))
				{
					*lcontent = player.rune_flags & FLAG_NHI;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "RHAA"))
				{
					*lcontent = player.rune_flags & FLAG_RHAA;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "SPACIUM"))
				{
					*lcontent = player.rune_flags & FLAG_SPACIUM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "STREGUM"))
				{
					*lcontent = player.rune_flags & FLAG_STREGUM;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "TAAR"))
				{
					*lcontent = player.rune_flags & FLAG_TAAR;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "TEMPUS"))
				{
					*lcontent = player.rune_flags & FLAG_TEMPUS;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "TERA"))
				{
					*lcontent = player.rune_flags & FLAG_TERA;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "VISTA"))
				{
					*lcontent = player.rune_flags & FLAG_VISTA;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "VITAE"))
				{
					*lcontent = player.rune_flags & FLAG_VITAE;
					return TYPE_LONG;
				}
				else if (!strcasecmp(temp, "YOK"))
				{
					*lcontent = player.rune_flags & FLAG_YOK;
					return TYPE_LONG;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			break;
		case 'I':

			if (!specialstrcmp(name, "^INZONE_"))
			{
				if (io)
				{
					char * zone = &name[8];
					ARX_PATH * ap = ARX_PATH_GetAddressByName(zone);

					if (ap == NULL)
					{
						*lcontent = 0;
						return TYPE_LONG;
					}
					else
					{
						if (ARX_PATH_IsPosInZone(ap, io->pos.x, io->pos.y, io->pos.z))
							*lcontent = 1;
						else
							*lcontent = 0;

						return TYPE_LONG;
					}

					// Nuky - unreachable code
					//*fcontent = 99999999999.f;
					//return TYPE_FLOAT;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^ININITPOS"))
			{
				Vec3f pos;

				if (io
						&&	GetItemWorldPosition(io, &pos)
						&&	(pos.x == io->initpos.x)
						&&	(pos.y == io->initpos.y)
						&&	(pos.z == io->initpos.z))
				{
					*lcontent = 1;
					return TYPE_LONG;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^INPLAYERINVENTORY"))
			{
				*lcontent = 0;

				if ((io)
						&&	(io->ioflags & IO_ITEM)
						&&	(IsInPlayerInventory(io)))
					*lcontent = 1;

				return TYPE_LONG;
			}

			break;
		case 'B':

			if (!specialstrcmp(name, "^BEHAVIOR"))
			{
				if (io && (io->ioflags & IO_NPC))
				{
					txtcontent = "";

					if (io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
						txtcontent += "L";

					if (io->_npcdata->behavior & BEHAVIOUR_SNEAK)
						txtcontent += "S";

					if (io->_npcdata->behavior & BEHAVIOUR_DISTANT)
						txtcontent += "D";

					if (io->_npcdata->behavior & BEHAVIOUR_MAGIC)
						txtcontent += "M";

					if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
						txtcontent += "F";

					if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
						txtcontent += "H";

					if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
						txtcontent += "R";

					if (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
						txtcontent += "T";

					if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
						txtcontent += "E";

					if (io->_npcdata->behavior & BEHAVIOUR_LOOK_FOR)
						txtcontent += "O";

					if (io->_npcdata->behavior & BEHAVIOUR_HIDE)
						txtcontent += "I";

					if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
						txtcontent += "W";

					if (io->_npcdata->behavior & BEHAVIOUR_GUARD)
						txtcontent += "U";

					if (io->_npcdata->behavior & BEHAVIOUR_STARE_AT)
						txtcontent += "A";
				}
				else txtcontent = "";

				return TYPE_TEXT;
			}

			break;
		case 'S':

			if (!specialstrcmp(name, "^SENDER"))
			{
				if (EVENT_SENDER)
				{
					if (EVENT_SENDER == inter.iobj[0])
						txtcontent = "PLAYER";
					else
						txtcontent = EVENT_SENDER->long_name();
				}
				else 	txtcontent = "NONE";

				return TYPE_TEXT;
			}

			if (!specialstrcmp(name, "^SCALE"))
			{
				if (io)
				{
					*fcontent = io->scale * 100.f;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^SPEAKING"))
			{
				if (io)
				{
					for (size_t i = 0; i < MAX_ASPEECH; i++)
					{
						if (aspeech[i].exist)
						{
							if (io == aspeech[i].io)
							{
								*lcontent = 1;
								return TYPE_LONG;
							}
						}
					}

					*lcontent = 0;
					return TYPE_LONG;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			break;
		case 'M':

			if (!specialstrcmp(name, "^ME"))
			{
				if (io == inter.iobj[0])
					txtcontent = "PLAYER";
				else
					txtcontent = io->long_name();

				return TYPE_TEXT;
			}

			if (!specialstrcmp(name, "^MAXLIFE"))
			{
				if ((io) && (io->ioflags & IO_NPC))
				{
					*fcontent = io->_npcdata->maxlife;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^MANA"))
			{
				if ((io) && (io->ioflags & IO_NPC))
				{
					*fcontent = io->_npcdata->mana;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^MAXMANA"))
			{
				if ((io) && (io->ioflags & IO_NPC))
				{
					*fcontent = io->_npcdata->maxmana;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^MYSPELL_"))
			{
				char * temp = &name[9];
				Spell id = GetSpellId(toLowercase(temp));

				if(id != SPELL_NONE)
				{
					for (size_t i = 0; i < MAX_SPELLS; i++)
					{
						if (spells[i].exist)
						{
							if (spells[i].type == id)
								if ((spells[i].caster >= 0) && (spells[i].caster < inter.nbmax)
										&& (io == inter.iobj[spells[i].caster]))
								{
									*lcontent = 1;
									return TYPE_LONG;
								}
						}
					}
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^MAXDURABILITY"))
			{
				if (io)
				{
					*fcontent = io->max_durability;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			break;
		case 'L':

			if (!specialstrcmp(name, "^LIFE"))
			{
				if ((io) && (io->ioflags & IO_NPC))
				{
					*fcontent = io->_npcdata->life;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^LAST_SPAWNED"))
			{
				if (LASTSPAWNED)
					txtcontent = LASTSPAWNED->long_name();
				else
					txtcontent = "NONE";

				return TYPE_TEXT;
			}

			break;
		case 'D':

			if (!specialstrcmp(name, "^DIST_"))
			{
				if (io)
				{
					char * obj = &name[6];

					if (!strcmp(obj, "PLAYER"))
					{
						*fcontent = (float)Distance3D(player.pos.x, player.pos.y, player.pos.z,
													  io->pos.x, io->pos.y, io->pos.z);
						return TYPE_FLOAT;
					}

					long t = GetTargetByNameTarget(obj);

					if (ValidIONum(t))
					{
						if (((io->show == SHOW_FLAG_IN_SCENE) || (io->show == SHOW_FLAG_IN_INVENTORY))
								&& ((inter.iobj[t]->show == SHOW_FLAG_IN_SCENE) || (inter.iobj[t]->show == SHOW_FLAG_IN_INVENTORY))
						   )
						{
							Vec3f pos, pos2;
							GetItemWorldPosition(io, &pos);
							GetItemWorldPosition(inter.iobj[t], &pos2);
							*fcontent = fdist(pos, pos2);

						}
						else // Out of this world item
							*fcontent = 99999999999.f;

						return TYPE_FLOAT;
					}

					*fcontent = 99999999999.f;
					return TYPE_FLOAT;
				}
			}

			if (!specialstrcmp(name, "^DEMO"))
			{
				*lcontent = FINAL_COMMERCIAL_DEMO;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^DURABILITY"))
			{
				if (io)
				{
					*fcontent = io->durability;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			break;
		case 'P':

			if (!specialstrcmp(name, "^PRICE"))
			{
				if ((io) && (io->ioflags & IO_ITEM))
				{
					*fcontent = ARX_CLEAN_WARN_CAST_FLOAT(io->_itemdata->price);
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_ZONE"))
			{
				ARX_PATH * op = (ARX_PATH *)player.inzone;

				if (op == NULL)
				{
					txtcontent = "NONE";
					return TYPE_TEXT;
				}

				txtcontent = op->name;
				return TYPE_TEXT;
			}

			if (!specialstrcmp(name, "^PLAYER_LIFE"))
			{
				*fcontent = player.Full_life;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^POISONED"))
			{
				if ((io) && (io->ioflags & IO_NPC))
				{
					*fcontent = io->_npcdata->poisonned;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^POISONOUS"))
			{
				if (io)
				{
					*fcontent = io->poisonous;
					return TYPE_FLOAT;
				}

				*fcontent = 0;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^POSSESS_"))
			{
				char * obj = &name[9];
				long t = GetTargetByNameTarget(obj);

				if (ValidIONum(t))
				{
					if (IsInPlayerInventory(inter.iobj[t]))
					{
						*lcontent = 1;
						return TYPE_LONG;
					}

					for (long i = 0; i < MAX_EQUIPED; i++)
					{
						if (player.equiped[i] == t)
						{
							*lcontent = 2;
							return TYPE_LONG;
						}
					}
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^PLAYER_GOLD"))
			{
				*fcontent = ARX_CLEAN_WARN_CAST_FLOAT(player.gold);
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_MAXLIFE"))
			{
				*fcontent = player.Full_maxlife;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_ATTRIBUTE_STRENGTH"))
			{
				*fcontent = player.Full_Attribute_Strength;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_ATTRIBUTE_DEXTERITY"))
			{
				*fcontent = player.Full_Attribute_Dexterity;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_ATTRIBUTE_CONSTITUTION"))
			{
				*fcontent = player.Full_Attribute_Constitution;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_ATTRIBUTE_MIND"))
			{
				*fcontent = player.Full_Attribute_Mind;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_STEALTH"))
			{
				*fcontent = player.Full_Skill_Stealth;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_MECANISM"))
			{
				*fcontent = player.Full_Skill_Mecanism;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_INTUITION"))
			{
				*fcontent = player.Full_Skill_Intuition;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_ETHERAL_LINK"))
			{
				*fcontent = player.Full_Skill_Etheral_Link;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_OBJECT_KNOWLEDGE"))
			{
				*fcontent = player.Full_Skill_Object_Knowledge;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_CASTING"))
			{
				*fcontent = player.Full_Skill_Casting;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_PROJECTILE"))
			{
				*fcontent = player.Full_Skill_Projectile;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_CLOSE_COMBAT"))
			{
				*fcontent = player.Full_Skill_Close_Combat;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_SKILL_DEFENSE"))
			{
				*fcontent = player.Full_Skill_Defense;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_HUNGER"))
			{
				*fcontent = player.hunger;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYER_POISON"))
			{
				*fcontent = player.poison;
				return TYPE_FLOAT;
			}

			if (!specialstrcmp(name, "^PLAYERCASTING"))
			{
				for (size_t i = 0; i < MAX_SPELLS; i++)
				{
					if (spells[i].exist)
					{
						if (spells[i].caster == 0)
						{
							if ((spells[i].type == SPELL_LIFE_DRAIN)
									||	(spells[i].type == SPELL_HARM)
									||	(spells[i].type == SPELL_FIRE_FIELD)
									||	(spells[i].type == SPELL_ICE_FIELD)
									||	(spells[i].type == SPELL_LIGHTNING_STRIKE)
									||	(spells[i].type == SPELL_MASS_LIGHTNING_STRIKE)
							   )
							{
								*lcontent = 1;
								return TYPE_LONG;
							}
						}
					}
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^PLAYERSPELL_"))
			{
				char * temp = &name[13];
				Spell id = GetSpellId(toLowercase(temp));

				if (id != SPELL_NONE)
				{
					for (size_t i = 0; i < MAX_SPELLS; i++)
					{
						if (spells[i].exist)
						{
							if (spells[i].type == id)
								if (spells[i].caster == 0)
								{
									*lcontent = 1;
									return TYPE_LONG;
								}
						}
					}
				}

				if (!strcasecmp(name, "^PLAYERSPELL_INVISIBILITY"))
				{
					if (inter.iobj[0]->invisibility > 0.3f)
					{
						*lcontent = 1;
						return TYPE_LONG;
					}
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			break;
		case 'N':

			if (!specialstrcmp(name, "^NPCINSIGHT"))
			{
				INTERACTIVE_OBJ * ioo = ARX_NPC_GetFirstNPCInSight(io);

				if (ioo == inter.iobj[0])
					txtcontent = "PLAYER";
				else if (ioo)
					txtcontent = ioo->long_name();
				else
					txtcontent = "NONE";

				return TYPE_TEXT;
			}

			break;
		case 'T':

			if (!specialstrcmp(name, "^TARGET"))
			{
				if (io->targetinfo == 0) txtcontent = "PLAYER";
				else
				{
					if (!ValidIONum(io->targetinfo))
						txtcontent = "NONE";
					else
						txtcontent = inter.iobj[io->targetinfo]->long_name();
				}

				return TYPE_TEXT;
			}

			break;
		case 'F':

			if (!specialstrcmp(name, "^FOCAL"))
			{
				if ((io != NULL) && (io->ioflags & IO_CAMERA))
				{
					EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
					*fcontent = cam->focal;
					return TYPE_FLOAT;
				}
			}

			if (!specialstrcmp(name, "^FIGHTING"))
			{
				if (ARX_PLAYER_IsInFightMode())
				{
					*lcontent = 1;
					return TYPE_LONG;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			break;
	}


	if (!specialstrcmp(name, " "))
	{
		if (io == inter.iobj[0])
			txtcontent = "PLAYER";
		else
			txtcontent = io->long_name();

		return TYPE_TEXT;
	}

	*lcontent = 0;
	return TYPE_LONG;
}

void ARX_SCRIPT_Free_All_Global_Variables()
{
	if (svar)
	{
		for (long i = 0; i < NB_GLOBALS; i++)
		{
			if (svar[i].text)
			{
				free(svar[i].text);
				svar[i].text = NULL;
			}
		}

		free(svar);
		svar = NULL;
		NB_GLOBALS = 0;
	}

}

void CloneLocalVars(INTERACTIVE_OBJ * ioo, INTERACTIVE_OBJ * io)
{
	if (!ioo) return;

	if (!io) return;

	if (ioo->script.lvar)
	{
		for (long n = 0; n < ioo->script.nblvar; n++)
		{
			if (ioo->script.lvar[n].text)
			{
				free((void *)ioo->script.lvar[n].text);
				ioo->script.lvar[n].text = NULL;
			}
		}

		ioo->script.nblvar = 0;
		free((void *)ioo->script.lvar);
		ioo->script.lvar = NULL;
	}

	if (io->script.lvar)
	{
		ioo->script.nblvar = io->script.nblvar;
		ioo->script.lvar = (SCRIPT_VAR *)malloc(sizeof(SCRIPT_VAR) * io->script.nblvar);

		for (long n = 0; n < io->script.nblvar; n++)
		{
			memcpy(&ioo->script.lvar[n], &io->script.lvar[n], sizeof(SCRIPT_VAR));

			if (io->script.lvar[n].text)
			{
				ioo->script.lvar[n].text = (char *)malloc(strlen(io->script.lvar[n].text) + 1);
				strcpy(ioo->script.lvar[n].text, io->script.lvar[n].text);
			}
		}
	}
}

SCRIPT_VAR * GetFreeVarSlot(SCRIPT_VAR*& _svff, long& _nb)
{

	SCRIPT_VAR * svf = _svff;
	_svff = (SCRIPT_VAR *) realloc(svf, sizeof(SCRIPT_VAR) * ((_nb) + 1));
	svf = _svff;
	memset(&svf[_nb], 0, sizeof(SCRIPT_VAR));
	_nb++;
	return &svf[_nb-1];
}

SCRIPT_VAR * GetVarAddress(SCRIPT_VAR svf[], size_t nb, const string & name) {
	
	for(size_t i = 0; i < nb; i++) {
		if(svf[i].type != TYPE_UNKNOWN) {
			if(!strcasecmp(name, svf[i].name)) {
				return &svf[i];
			}
		}
	}

	return NULL;
}

long GETVarValueLong(SCRIPT_VAR svf[], size_t nb, const string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->ival;
}

float GETVarValueFloat(SCRIPT_VAR svf[], size_t nb, const string & name) {
	
	const SCRIPT_VAR * tsv = GetVarAddress(svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->fval;
}

std::string GETVarValueText(SCRIPT_VAR svf[], size_t nb, const string & name) {
	
	const SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (!tsv) return "";

	return tsv->text;
}

string GetVarValueInterpretedAsText(const string & temp1, const EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io) {
	
	char var_text[256];
	float t1;

	if(!temp1.empty())
	{
		if (temp1[0] == '^')
		{
			long lv;
			float fv;
			std::string tv;

			switch (GetSystemVar(esss,io,temp1,tv,&fv,&lv))//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
			{
				case TYPE_TEXT:
					return tv;
					break;
				case TYPE_LONG:
					sprintf(var_text, "%ld", lv);
					return var_text;
					break;
				default:
					sprintf(var_text, "%f", fv);
					return var_text;
					break;
			}

		}
		else if (temp1[0] == '#')
		{
			long l1 = GETVarValueLong(svar, NB_GLOBALS, temp1);
			sprintf(var_text, "%ld", l1);
			return var_text;
		}
		else if (temp1[0] == '\xA7')
		{
			long l1 = GETVarValueLong(esss->lvar, esss->nblvar, temp1);
			sprintf(var_text, "%ld", l1);
			return var_text;
		}
		else if (temp1[0] == '&') t1 = GETVarValueFloat(svar, NB_GLOBALS, temp1);
		else if (temp1[0] == '@') t1 = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);
		else if (temp1[0] == '$')
		{
			SCRIPT_VAR * var = GetVarAddress(svar, NB_GLOBALS, temp1);

			if (!var) return "VOID";
			else return var->text;
		}
		else if (temp1[0] == '\xA3')
		{
			SCRIPT_VAR * var = GetVarAddress(esss->lvar, esss->nblvar, temp1);

			if (!var) return "VOID";
			else return var->text;
		}
		else
		{
			return temp1;
		}
	}
	else
	{
		return "";
	}

	sprintf(var_text, "%f", t1);
	return var_text;
}

float GetVarValueInterpretedAsFloat(const string & temp1, const EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io) {
	
	if(temp1[0] == '^') {
		long lv;
		float fv;
		std::string tv; 
		switch (GetSystemVar(esss,io,temp1,tv,&fv,&lv)) {
			case TYPE_TEXT:
				return (float)atof(tv.c_str());
			case TYPE_LONG:
				return (float)lv;
				// TODO unreachable code (should it be case TYPE_FLOAT: ?)
				//return (fv);
			default:
				break;
		}
	} else if(temp1[0] == '#') {
		return (float)GETVarValueLong(svar, NB_GLOBALS, temp1);
	} else if(temp1[0] == '\xA7') {
		return (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);
	} else if(temp1[0] == '&') {
		return GETVarValueFloat(svar, NB_GLOBALS, temp1);
	} else if(temp1[0] == '@') {
		return GETVarValueFloat(esss->lvar, esss->nblvar, temp1);
	}
	
	return (float)atof(temp1.c_str());
}

SCRIPT_VAR* SETVarValueLong(SCRIPT_VAR*& svf, long& nb, const std::string& name, long val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		strcpy(tsv->name, name.c_str());
	}

	tsv->ival = val;
	return tsv;
}

SCRIPT_VAR* SETVarValueFloat(SCRIPT_VAR*& svf, long& nb, const std::string& name, float val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		strcpy(tsv->name, name.c_str());
	}

	tsv->fval = val;
	return tsv;
}

SCRIPT_VAR* SETVarValueText(SCRIPT_VAR*& svf, long& nb, const std::string& name, const std::string& val)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		strcpy(tsv->name, name.c_str());
	}

	if (tsv->text)
	{
		free((void *)tsv->text);
		tsv->text = NULL;
	}

	tsv->ival = val.length() + 1;

	if (tsv->ival)
		tsv->text = strdup(val.c_str());
	else
		tsv->text = NULL;


	return tsv;
}

namespace {

typedef std::map<string, AnimationNumber> Animations;
Animations animations;

}

void initAnimationNumbers() {
	
	animations["wait"] = ANIM_WAIT;
	animations["wait2"] = ANIM_WAIT2;
	animations["walk"] = ANIM_WALK;
	animations["walk1"] = ANIM_WALK;
	animations["walk2"] = ANIM_WALK2;
	animations["walk3"] = ANIM_WALK3;
	animations["walk_backward"] = ANIM_WALK_BACKWARD;
	animations["walk_ministep"] = ANIM_WALK_MINISTEP;
	animations["wait_short"] = ANIM_WAIT_SHORT;
	animations["walk_sneak"] = ANIM_WALK_SNEAK;
	animations["action"] = ANIM_ACTION;
	animations["action1"] = ANIM_ACTION;
	animations["action2"] = ANIM_ACTION2;
	animations["action3"] = ANIM_ACTION3;
	animations["action4"] = ANIM_ACTION4;
	animations["action5"] = ANIM_ACTION5;
	animations["action6"] = ANIM_ACTION6;
	animations["action7"] = ANIM_ACTION7;
	animations["action8"] = ANIM_ACTION8;
	animations["action9"] = ANIM_ACTION9;
	animations["action10"] = ANIM_ACTION10;
	animations["hit1"] = ANIM_HIT1;
	animations["hit"] = ANIM_HIT1;
	animations["hold_torch"] = ANIM_HOLD_TORCH;
	animations["hit_short"] = ANIM_HIT_SHORT;
	animations["strike1"] = ANIM_STRIKE1;
	animations["strike"] = ANIM_STRIKE1;
	animations["shield_start"] = ANIM_SHIELD_START;
	animations["shield_cycle"] = ANIM_SHIELD_CYCLE;
	animations["shield_hit"] = ANIM_SHIELD_HIT;
	animations["shield_end"] = ANIM_SHIELD_END;
	animations["strafe_right"] = ANIM_STRAFE_RIGHT;
	animations["strafe_left"] = ANIM_STRAFE_LEFT;
	animations["strafe_run_left"] = ANIM_STRAFE_RUN_LEFT;
	animations["strafe_run_right"] = ANIM_STRAFE_RUN_RIGHT;
	animations["die"] = ANIM_DIE;
	animations["dagger_ready_part_1"] = ANIM_DAGGER_READY_PART_1;
	animations["dagger_ready_part_2"] = ANIM_DAGGER_READY_PART_2;
	animations["dagger_unready_part_1"] = ANIM_DAGGER_UNREADY_PART_1;
	animations["dagger_unready_part_2"] = ANIM_DAGGER_UNREADY_PART_2;
	animations["dagger_wait"] = ANIM_DAGGER_WAIT;
	animations["dagger_strike_left_start"] = ANIM_DAGGER_STRIKE_LEFT_START;
	animations["dagger_strike_left_cycle"] = ANIM_DAGGER_STRIKE_LEFT_CYCLE;
	animations["dagger_strike_left"] = ANIM_DAGGER_STRIKE_LEFT;
	animations["dagger_strike_right_start"] = ANIM_DAGGER_STRIKE_RIGHT_START;
	animations["dagger_strike_right_cycle"] = ANIM_DAGGER_STRIKE_RIGHT_CYCLE;
	animations["dagger_strike_right"] = ANIM_DAGGER_STRIKE_RIGHT;
	animations["dagger_strike_top_start"] = ANIM_DAGGER_STRIKE_TOP_START;
	animations["dagger_strike_top_cycle"] = ANIM_DAGGER_STRIKE_TOP_CYCLE;
	animations["dagger_strike_top"] = ANIM_DAGGER_STRIKE_TOP;
	animations["dagger_strike_bottom_start"] = ANIM_DAGGER_STRIKE_BOTTOM_START;
	animations["dagger_strike_bottom_cycle"] = ANIM_DAGGER_STRIKE_BOTTOM_CYCLE;
	animations["dagger_strike_bottom"] = ANIM_DAGGER_STRIKE_BOTTOM;
	animations["death_critical"] = ANIM_DEATH_CRITICAL;
	animations["run"] = ANIM_RUN;
	animations["run1"] = ANIM_RUN;
	animations["run2"] = ANIM_RUN2;
	animations["run3"] = ANIM_RUN3;
	animations["run_backward"] = ANIM_RUN_BACKWARD;
	animations["talk_neutral"] = ANIM_TALK_NEUTRAL;
	animations["talk_angry"] = ANIM_TALK_ANGRY;
	animations["talk_happy"] = ANIM_TALK_HAPPY;
	animations["talk_neutral_head"] = ANIM_TALK_NEUTRAL_HEAD;
	animations["talk_angry_head"] = ANIM_TALK_ANGRY_HEAD;
	animations["talk_happy_head"] = ANIM_TALK_HAPPY_HEAD;
	animations["bare_ready"] = ANIM_BARE_READY;
	animations["bare_unready"] = ANIM_BARE_UNREADY;
	animations["bare_wait"] = ANIM_BARE_WAIT;
	animations["bare_strike_left_start"] = ANIM_BARE_STRIKE_LEFT_START;
	animations["bare_strike_left_cycle"] = ANIM_BARE_STRIKE_LEFT_CYCLE;
	animations["bare_strike_left"] = ANIM_BARE_STRIKE_LEFT;
	animations["bare_strike_right_start"] = ANIM_BARE_STRIKE_RIGHT_START;
	animations["bare_strike_right_cycle"] = ANIM_BARE_STRIKE_RIGHT_CYCLE;
	animations["bare_strike_right"] = ANIM_BARE_STRIKE_RIGHT;
	animations["bare_strike_top_start"] = ANIM_BARE_STRIKE_TOP_START;
	animations["bare_strike_top_cycle"] = ANIM_BARE_STRIKE_TOP_CYCLE;
	animations["bare_strike_top"] = ANIM_BARE_STRIKE_TOP;
	animations["bare_strike_bottom_start"] = ANIM_BARE_STRIKE_BOTTOM_START;
	animations["bare_strike_bottom_cycle"] = ANIM_BARE_STRIKE_BOTTOM_CYCLE;
	animations["bare_strike_bottom"] = ANIM_BARE_STRIKE_BOTTOM;
	animations["1h_ready_part_1"] = ANIM_1H_READY_PART_1;
	animations["1h_ready_part_2"] = ANIM_1H_READY_PART_2;
	animations["1h_unready_part_1"] = ANIM_1H_UNREADY_PART_1;
	animations["1h_unready_part_2"] = ANIM_1H_UNREADY_PART_2;
	animations["1h_wait"] = ANIM_1H_WAIT;
	animations["1h_strike_left_start"] = ANIM_1H_STRIKE_LEFT_START;
	animations["1h_strike_left_cycle"] = ANIM_1H_STRIKE_LEFT_CYCLE;
	animations["1h_strike_left"] = ANIM_1H_STRIKE_LEFT;
	animations["1h_strike_right_start"] = ANIM_1H_STRIKE_RIGHT_START;
	animations["1h_strike_right_cycle"] = ANIM_1H_STRIKE_RIGHT_CYCLE;
	animations["1h_strike_right"] = ANIM_1H_STRIKE_RIGHT;
	animations["1h_strike_top_start"] = ANIM_1H_STRIKE_TOP_START;
	animations["1h_strike_top_cycle"] = ANIM_1H_STRIKE_TOP_CYCLE;
	animations["1h_strike_top"] = ANIM_1H_STRIKE_TOP;
	animations["1h_strike_bottom_start"] = ANIM_1H_STRIKE_BOTTOM_START;
	animations["1h_strike_bottom_cycle"] = ANIM_1H_STRIKE_BOTTOM_CYCLE;
	animations["1h_strike_bottom"] = ANIM_1H_STRIKE_BOTTOM;
	animations["2h_ready_part_1"] = ANIM_2H_READY_PART_1;
	animations["2h_ready_part_2"] = ANIM_2H_READY_PART_2;
	animations["2h_unready_part_1"] = ANIM_2H_UNREADY_PART_1;
	animations["2h_unready_part_2"] = ANIM_2H_UNREADY_PART_2;
	animations["2h_wait"] = ANIM_2H_WAIT;
	animations["2h_strike_left_start"] = ANIM_2H_STRIKE_LEFT_START;
	animations["2h_strike_left_cycle"] = ANIM_2H_STRIKE_LEFT_CYCLE;
	animations["2h_strike_left"] = ANIM_2H_STRIKE_LEFT;
	animations["2h_strike_right_start"] = ANIM_2H_STRIKE_RIGHT_START;
	animations["2h_strike_right_cycle"] = ANIM_2H_STRIKE_RIGHT_CYCLE;
	animations["2h_strike_right"] = ANIM_2H_STRIKE_RIGHT;
	animations["2h_strike_top_start"] = ANIM_2H_STRIKE_TOP_START;
	animations["2h_strike_top_cycle"] = ANIM_2H_STRIKE_TOP_CYCLE;
	animations["2h_strike_top"] = ANIM_2H_STRIKE_TOP;
	animations["2h_strike_bottom_start"] = ANIM_2H_STRIKE_BOTTOM_START;
	animations["2h_strike_bottom_cycle"] = ANIM_2H_STRIKE_BOTTOM_CYCLE;
	animations["2h_strike_bottom"] = ANIM_2H_STRIKE_BOTTOM;
	animations["missile_ready_part_1"] = ANIM_MISSILE_READY_PART_1;
	animations["missile_ready_part_2"] = ANIM_MISSILE_READY_PART_2;
	animations["missile_unready_part_1"] = ANIM_MISSILE_UNREADY_PART_1;
	animations["missile_unready_part_2"] = ANIM_MISSILE_UNREADY_PART_2;
	animations["missile_wait"] = ANIM_MISSILE_WAIT;
	animations["missile_strike_part_1"] = ANIM_MISSILE_STRIKE_PART_1;
	animations["missile_strike_part_2"] = ANIM_MISSILE_STRIKE_PART_2;
	animations["missile_strike_cycle"] = ANIM_MISSILE_STRIKE_CYCLE;
	animations["missile_strike"] = ANIM_MISSILE_STRIKE;
	animations["meditation"] = ANIM_MEDITATION;
	animations["cast_start"] = ANIM_CAST_START;
	animations["cast_cycle"] = ANIM_CAST_CYCLE;
	animations["cast"] = ANIM_CAST;
	animations["cast_end"] = ANIM_CAST_END;
	animations["crouch"] = ANIM_CROUCH;
	animations["crouch_walk"] = ANIM_CROUCH_WALK;
	animations["crouch_walk_backward"] = ANIM_CROUCH_WALK_BACKWARD;
	animations["crouch_strafe_left"] = ANIM_CROUCH_STRAFE_LEFT;
	animations["crouch_strafe_right"] = ANIM_CROUCH_STRAFE_RIGHT;
	animations["crouch_start"] = ANIM_CROUCH_START;
	animations["crouch_wait"] = ANIM_CROUCH_WAIT;
	animations["crouch_end"] = ANIM_CROUCH_END;
	animations["lean_right"] = ANIM_LEAN_RIGHT;
	animations["lean_left"] = ANIM_LEAN_LEFT;
	animations["levitate"] = ANIM_LEVITATE;
	animations["jump"] = ANIM_JUMP;
	animations["jump_anticipation"] = ANIM_JUMP_ANTICIPATION;
	animations["jump_up"] = ANIM_JUMP_UP;
	animations["jump_cycle"] = ANIM_JUMP_CYCLE;
	animations["jump_end"] = ANIM_JUMP_END;
	animations["jump_end_part2"] = ANIM_JUMP_END_PART2;
	animations["fight_walk_forward"] = ANIM_FIGHT_WALK_FORWARD;
	animations["fight_walk_backward"] = ANIM_FIGHT_WALK_BACKWARD;
	animations["fight_walk_ministep"] = ANIM_FIGHT_WALK_MINISTEP;
	animations["fight_strafe_right"] = ANIM_FIGHT_STRAFE_RIGHT;
	animations["fight_strafe_left"] = ANIM_FIGHT_STRAFE_LEFT;
	animations["fight_wait"] = ANIM_FIGHT_WAIT;
	animations["grunt"] = ANIM_GRUNT;
	animations["u_turn_left"] = ANIM_U_TURN_LEFT;
	animations["u_turn_right"] = ANIM_U_TURN_RIGHT;
	animations["u_turn_left_fight"] = ANIM_U_TURN_LEFT_FIGHT;
	animations["u_turn_right_fight"] = ANIM_U_TURN_RIGHT_FIGHT;
	
}

AnimationNumber GetNumAnim(const string & name) {
	
	Animations::const_iterator it = animations.find(name);
	
	return (it == animations.end()) ? ANIM_NONE : it->second;
}

void MakeGlobalText(std::string & tx)
{
	char texx[256];

	for(long i = 0; i < NB_GLOBALS; i++) {
		switch(svar[i].type) {
			case TYPE_G_TEXT:
				tx += svar[i].name;
				tx += " = ";
				tx += svar[i].text;
				tx += "\r\n";
				break;
			case TYPE_G_LONG:
				tx += svar[i].name;
				tx += " = ";
				sprintf(texx, "%ld", svar[i].ival);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_G_FLOAT:
				tx += svar[i].name;
				tx += " = ";
				sprintf(texx, "%f", svar[i].fval);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_UNKNOWN:
			case TYPE_L_TEXT:
			case TYPE_L_LONG:
			case TYPE_L_FLOAT:
				break;
		}
	}
}

void MakeLocalText(EERIE_SCRIPT * es, std::string& tx)
{
	char texx[256];

	if (es->master != NULL) es = (EERIE_SCRIPT *)es->master;

	if (es->lvar == NULL) return;

	for (long i = 0; i < es->nblvar; i++)
	{
		switch (es->lvar[i].type)
		{
			case TYPE_L_TEXT:
				tx += es->lvar[i].name;
				tx += " = ";
				tx += es->lvar[i].text;
				tx += "\r\n";
				break;
			case TYPE_L_LONG:
				tx += es->lvar[i].name;
				tx += " = ";
				sprintf(texx, "%ld", es->lvar[i].ival);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_L_FLOAT:
				tx += es->lvar[i].name;
				tx += " = ";
				sprintf(texx, "%f", es->lvar[i].fval);
				tx += texx;
				tx += "\r\n";
				break;
			case TYPE_UNKNOWN:
			case TYPE_G_TEXT:
			case TYPE_G_LONG:
			case TYPE_G_FLOAT:
				break;
		}
	}
}

void ForceAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea)
{
	if (ea == NULL) return;

	if (io == NULL) return;

	if ((io->animlayer[0].cur_anim)
			&& (io->animlayer[0].cur_anim != io->anims[ANIM_DIE])
			&& (io->animlayer[0].cur_anim != io->anims[ANIM_HIT1]))
		AcquireLastAnim(io);

	FinishAnim(io, io->animlayer[0].cur_anim);
	io->lastmove.x = 0;
	io->lastmove.y = 0;
	io->lastmove.z = 0;
	ANIM_Set(&io->animlayer[0], ea);
	io->animlayer[0].flags |= EA_FORCEPLAY;
	io->animlayer[0].nextflags = 0;
}

extern EERIE_BACKGROUND * ACTIVEBKG;

void GetTargetPos(INTERACTIVE_OBJ * io, unsigned long smoothing)
{
	if (io == NULL) return;

	if (io->ioflags & IO_NPC)
	{
		if (io->_npcdata->behavior & BEHAVIOUR_NONE)
		{
			io->target.x = io->pos.x;
			io->target.y = io->pos.y;
			io->target.z = io->pos.z;
			return;
		}

		if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
		{
			if (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb)
			{
				long pos = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos];
				io->target.x = ACTIVEBKG->anchors[pos].pos.x;
				io->target.y = ACTIVEBKG->anchors[pos].pos.y;
				io->target.z = ACTIVEBKG->anchors[pos].pos.z;
				return;
			}

			io->target.x = io->initpos.x;
			io->target.y = io->initpos.y;
			io->target.z = io->initpos.z;
			return;
		}

		if ((io->_npcdata) && (io->_npcdata->pathfind.listnb != -1) && (io->_npcdata->pathfind.list)
				&& (!(io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)))// Targeting Anchors !
		{
			if (io->_npcdata->pathfind.listpos < io->_npcdata->pathfind.listnb)
			{
				long pos = io->_npcdata->pathfind.list[io->_npcdata->pathfind.listpos];
				io->target.x = ACTIVEBKG->anchors[pos].pos.x;
				io->target.y = ACTIVEBKG->anchors[pos].pos.y;
				io->target.z = ACTIVEBKG->anchors[pos].pos.z;
			}
			else if (ValidIONum(io->_npcdata->pathfind.truetarget))
			{
				INTERACTIVE_OBJ * ioo = inter.iobj[io->_npcdata->pathfind.truetarget];
				io->target.x = ioo->pos.x;
				io->target.y = ioo->pos.y;
				io->target.z = ioo->pos.z;
			}


			return;
		}
	}



	if (io->targetinfo == TARGET_PATH)
	{
		if (io->usepath == NULL)
		{
			io->target.x = io->pos.x;
			io->target.y = io->pos.y;
			io->target.z = io->pos.z;
			return;
		}

		ARX_USE_PATH * aup = io->usepath;
		aup->_curtime += smoothing + 100;
		Vec3f tp;
		long wp = ARX_PATHS_Interpolate(aup, &tp);

		if (wp < 0)
		{
			if (io->ioflags & IO_CAMERA)
				io->_camdata->cam.lastinfovalid = false;
		}
		else
		{

			io->target.x = tp.x;
			io->target.y = tp.y;
			io->target.z = tp.z;

		}

		return;
	}

	if (io->targetinfo == TARGET_NONE)
	{
		io->target.x = io->pos.x;
		io->target.y = io->pos.y;
		io->target.z = io->pos.z;
		return;
	}








	if ((io->targetinfo == TARGET_PLAYER) || (io->targetinfo == -1))
	{
		io->target.x = player.pos.x;
		io->target.y = player.pos.y + player.size.y;
		io->target.z = player.pos.z;
		return;
	}
	else
	{
		if (ValidIONum(io->targetinfo))
		{
			Vec3f pos;

			if (GetItemWorldPosition(inter.iobj[io->targetinfo], &pos))
			{
				io->target.x = pos.x;
				io->target.y = pos.y;
				io->target.z = pos.z;
				return;
			}

			io->target.x = inter.iobj[io->targetinfo]->pos.x;
			io->target.y = inter.iobj[io->targetinfo]->pos.y;
			io->target.z = inter.iobj[io->targetinfo]->pos.z;
			return;
		}
	}

	io->target.x = io->pos.x;
	io->target.y = io->pos.y;
	io->target.z = io->pos.z;
}

//*************************************************************************************
// ScriptEvent::send																	//
// Sends a event to a script.														//
// returns ACCEPT to accept default EVENT processing								//
// returns REFUSE to refuse default EVENT processing								//
//*************************************************************************************
void MakeSSEPARAMS(const char * params)
{
	// LogDebug << "MakeSSEPARAMS " << Logger::nullstr(params);
	
	for (long i = 0; i < MAX_SSEPARAMS; i++)
	{
		SSEPARAMS[i][0] = 0;
	}

	if(params == NULL) {
		return;
	}

	long pos = 0;

	while(*params != '\0' && pos < MAX_SSEPARAMS) {
		
		size_t tokensize = 0;
		while(params[tokensize] != ' ' && params[tokensize] != '\0') {
			tokensize++;
		}
		
		assert(tokensize < 64 - 1);
		memcpy(SSEPARAMS[pos], params, tokensize);
		SSEPARAMS[pos][tokensize] = 0;
		
		params += tokensize;
		
		if(*params != '\0') {
			params++;
		}
		
		pos++;
	}
}

#define MAX_EVENT_STACK 800
struct STACKED_EVENT {
	INTERACTIVE_OBJ * sender;
	long              exist;
	INTERACTIVE_OBJ * io;
	ScriptMessage     msg;
	std::string       params;
	std::string       eventname;
};

STACKED_EVENT eventstack[MAX_EVENT_STACK];

void ARX_SCRIPT_EventStackInit()
{
	ARX_SCRIPT_EventStackClear( false ); // Clear everything in the stack
}
void ARX_SCRIPT_EventStackClear( bool check_exist )
{
	LogDebug << "Event Stack Clear";
	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if ( check_exist ) // If we're not blatantly clearing everything
			if ( !eventstack[i].exist ) // If the Stacked_Event is not being used
				continue; // Continue on to the next one

			// Otherwise, clear all the fields in this stacked_event
			eventstack[i].sender = NULL;
			eventstack[i].exist = 0;
			eventstack[i].io = NULL;
			eventstack[i].msg = SM_NULL;
			eventstack[i].params.clear();
			eventstack[i].eventname.clear();
	}
}

long STACK_FLOW = 8;

void ARX_SCRIPT_EventStackClearForIo(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if (eventstack[i].exist)
		{
			if (eventstack[i].io == io)
			{
				eventstack[i].sender = NULL;
				eventstack[i].exist = 0;
				eventstack[i].io = NULL;
				eventstack[i].msg = SM_NULL;
				eventstack[i].params.clear();
				eventstack[i].eventname.clear();
			}
		}
	}
}

void ARX_SCRIPT_EventStackExecute()
{
	long count = 0;

	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if (eventstack[i].exist)
		{
			if (!ValidIOAddress(eventstack[i].io))
				goto kill;

			if (ValidIOAddress(eventstack[i].sender))
				EVENT_SENDER = eventstack[i].sender;
			else
				EVENT_SENDER = NULL;

			SendIOScriptEvent(eventstack[i].io, eventstack[i].msg, eventstack[i].params, eventstack[i].eventname);
		kill:
			;

			eventstack[i].sender = NULL;
			eventstack[i].exist = 0;
			eventstack[i].io = NULL;
			eventstack[i].msg = SM_NULL;
			eventstack[i].params.clear();
			eventstack[i].eventname.clear();
			count++;

			if (count >= STACK_FLOW) return;
		}
	}
}

void ARX_SCRIPT_EventStackExecuteAll()
{
	STACK_FLOW = 9999999;
	ARX_SCRIPT_EventStackExecute();
	STACK_FLOW = 20;
}

void Stack_SendIOScriptEvent(INTERACTIVE_OBJ * io, ScriptMessage msg, const std::string& params, const std::string& eventname)
{
	// LogDebug << "Stack_SendIOScriptEvent "<< eventname;
	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if (!eventstack[i].exist)
		{
			eventstack[i].sender = EVENT_SENDER;
			eventstack[i].io = io;
			eventstack[i].msg = msg;
			eventstack[i].exist = 1;
			eventstack[i].params = params;
			eventstack[i].eventname = eventname;

			return;
		}
	}
}

ScriptResult SendIOScriptEventReverse(INTERACTIVE_OBJ * io, ScriptMessage msg, const std::string& params, const std::string& eventname)
{
	// LogDebug << "SendIOScriptEventReverse "<< eventname;
	// checks invalid IO
	if (!io) return REFUSE;

	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		// if this IO only has a Local script, send event to it
		if (inter.iobj[num] && !inter.iobj[num]->over_script.data)
		{
			return ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
		}

		// If this IO has a Global script send to Local (if exists)
		// then to local if no overriden by Local
		if (inter.iobj[num] && (ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname) != REFUSE))
		{

			if (inter.iobj[num])
				return (ScriptEvent::send(&inter.iobj[num]->over_script, msg, params, inter.iobj[num], eventname));
			else
				return REFUSE;
		}

	}

	// Refused further processing.
	return REFUSE;
}

ScriptResult SendIOScriptEvent(INTERACTIVE_OBJ * io, ScriptMessage msg, const std::string& params, const std::string& eventname)
{
	//if (msg != 8)
	//	LogDebug << "SendIOScriptEvent event '"<< eventname<<"' message " << msg;
	// checks invalid IO
	if (!io) return REFUSE;

	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		INTERACTIVE_OBJ * oes = EVENT_SENDER;

		if ((msg == SM_INIT) || (msg == SM_INITEND))
		{
			if (inter.iobj[num])
			{
				SendIOScriptEventReverse(inter.iobj[num], msg, params, eventname);
				EVENT_SENDER = oes;
			}
		}

		// if this IO only has a Local script, send event to it
		if (inter.iobj[num] && !inter.iobj[num]->over_script.data)
		{
			ScriptResult ret = ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
			EVENT_SENDER = oes;
			return ret;
		}

		// If this IO has a Global script send to Local (if exists)
		// then to Global if no overriden by Local
		if (inter.iobj[num] && ScriptEvent::send(&inter.iobj[num]->over_script, msg, params, inter.iobj[num], eventname) != REFUSE) {
			EVENT_SENDER = oes;

			if (inter.iobj[num])
			{
				ScriptResult ret = ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
				EVENT_SENDER = oes;
				return ret;
			}
			else
				return REFUSE;
		}

	}

	// Refused further processing.
	return REFUSE;
}

ScriptResult SendInitScriptEvent(INTERACTIVE_OBJ * io) {
	
	if (!io) return REFUSE;

	INTERACTIVE_OBJ * oes = EVENT_SENDER;
	EVENT_SENDER = NULL;
	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		if (inter.iobj[num] && inter.iobj[num]->script.data)
		{
			ScriptEvent::send(&inter.iobj[num]->script, SM_INIT, "", inter.iobj[num], "");
		}

		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			ScriptEvent::send(&inter.iobj[num]->over_script, SM_INIT, "", inter.iobj[num], "");
		}

		if (inter.iobj[num] && inter.iobj[num]->script.data)
		{
			ScriptEvent::send(&inter.iobj[num]->script, SM_INITEND, "", inter.iobj[num], "");
		}

		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			ScriptEvent::send(&inter.iobj[num]->over_script, SM_INITEND, "", inter.iobj[num], "");
		}
	}

	EVENT_SENDER = oes;
	return ACCEPT;
}

//! Checks if timer named texx exists.
static bool ARX_SCRIPT_Timer_Exist(const std::string & texx) {
	
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(scr_timer[i].exist) {
			if(scr_timer[i].name == texx) {
				return true;
			}
		}
	}
	
	return false;
}

string ARX_SCRIPT_Timer_GetDefaultName() {
	
	for(size_t i = 1; ; i++) {
		
		std::ostringstream oss;
		oss << "timer_" << i;
		
		if(!ARX_SCRIPT_Timer_Exist(oss.str())) {
			return oss.str();
		}
	}
}

//*************************************************************************************
// Get a free script timer
//*************************************************************************************
long ARX_SCRIPT_Timer_GetFree() {
	
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(!(scr_timer[i].exist))
			return i;
	}
	
	return -1;
}

//*************************************************************************************
// Count the number of active script timers...
//*************************************************************************************
long ARX_SCRIPT_CountTimers() {
	return ActiveTimers;
}

//*************************************************************************************
// ARX_SCRIPT_Timer_ClearByNum
// Clears a timer by its Index (long timer_idx) on the timers list
//*************************************************************************************
void ARX_SCRIPT_Timer_ClearByNum(long timer_idx) {
	if(scr_timer[timer_idx].exist) {
		scr_timer[timer_idx].name.clear();
		ActiveTimers--;
		scr_timer[timer_idx].exist = 0;
	}
}

void ARX_SCRIPT_Timer_Clear_By_Name_And_IO(const string & timername, INTERACTIVE_OBJ * io) {
	for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
		if(scr_timer[i].exist && scr_timer[i].io == io && scr_timer[i].name == timername) {
			ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

void ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if ((scr_timer[i].io == io) && (scr_timer[i].es == &io->over_script))
				ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

void ARX_SCRIPT_Timer_Clear_By_IO(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if (scr_timer[i].io == io)
				ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

//*************************************************************************************
// Initialise the timer list for the first time.
//*************************************************************************************
long MAX_TIMER_SCRIPT = 0;
void ARX_SCRIPT_Timer_FirstInit(long number)
{
	if (number < 100) number = 100;

	MAX_TIMER_SCRIPT = number;

	if (scr_timer) delete[] scr_timer;

	//todo free
	scr_timer = new SCR_TIMER[MAX_TIMER_SCRIPT];
	ActiveTimers = 0;
}

void ARX_SCRIPT_Timer_ClearAll()
{
	if (ActiveTimers)
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
			ARX_SCRIPT_Timer_ClearByNum(i);

	ActiveTimers = 0;
}

void ARX_SCRIPT_Timer_Clear_For_IO(INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if (scr_timer[i].io == io) ARX_SCRIPT_Timer_ClearByNum(i);
		}
	}
}

long ARX_SCRIPT_GetSystemIOScript(INTERACTIVE_OBJ * io, const std::string & name) {
	
	if(ActiveTimers) {
		for(long i = 0; i < MAX_TIMER_SCRIPT; i++) {
			if(scr_timer[i].exist && scr_timer[i].io == io && scr_timer[i].name == name) {
				return i;
			}
		}
	}
	
	return -1;
}

long Manage_Specific_RAT_Timer(SCR_TIMER * st)
{
	INTERACTIVE_OBJ * io = st->io;
	GetTargetPos(io);
	Vec3f target = io->target - io->pos;
	Vector_Normalize(&target);
	Vec3f targ;
	Vector_RotateY(&targ, &target, rnd() * 60.f - 30.f);
	target = io->target + targ * 100.f;

	if (ARX_INTERACTIVE_ConvertToValidPosForIO(io, &target))
	{
		ARX_INTERACTIVE_Teleport(io, &target);
		Vec3f pos;
		pos.x = io->pos.x;
		pos.y = io->pos.y + io->physics.cyl.height * ( 1.0f / 2 );
		pos.z = io->pos.z;
		ARX_PARTICLES_Add_Smoke(&pos, 3, 20);
		AddRandomSmoke(io, 20);
		MakeCoolFx(&io->pos);
		io->show = SHOW_FLAG_IN_SCENE;

		for (long kl = 0; kl < 10; kl++)
		{
			FaceTarget2(io);
		}

		io->GameFlags &= ~GFLAG_INVISIBILITY;
		st->times = 1;
	}
	else
	{
		st->times++;

		st->msecs = ARX_CAST_LONG(st->msecs * ( 1.0f / 2 ));


		if (st->msecs < 100) st->msecs = 100;

		return 1;
	}

	return 0;
}

void ARX_SCRIPT_Timer_Check()
{
	if (ActiveTimers)
	{
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
		{
			SCR_TIMER * st = &scr_timer[i];

			if (st->exist)
			{
				if (st->flags & 1)
				{
					if (!(st->io->GameFlags & GFLAG_ISINTREATZONE))
					{
						while (st->tim + st->msecs < ARXTime)
						{
							st->tim += st->msecs;
						}

						continue;
					}
				}

				if (st->tim + st->msecs <= ARXTime)
				{
					EERIE_SCRIPT * es = st->es;
					INTERACTIVE_OBJ * io = st->io;
					long pos = st->pos;

					if(!es) {
						if(st->name == "_r_a_t_") {
							if (Manage_Specific_RAT_Timer(st)) continue;
						}
					}

					if (st->times == 1)
					{
						ARX_SCRIPT_Timer_ClearByNum(i);
					}
					else
					{
						if (st->times != 0) st->times--;

						st->tim += st->msecs;
					}

					if ((es)
							&&	(ValidIOAddress(io)))
					{

						ScriptEvent::send(es, SM_EXECUTELINE, "", io, "", pos);
					}

				}
			}
		}
	}
}

void ARX_SCRIPT_Init_Event_Stats() {
	
	ScriptEvent::totalCount = 0;
	
	for(long i = 0; i < inter.nbmax; i++) {
		if(inter.iobj[i] != NULL) {
			inter.iobj[i]->stat_count = 0;
			inter.iobj[i]->stat_sent = 0;
		}
	}
}

bool IsIOGroup(INTERACTIVE_OBJ * io, const std::string& group)
{
	for (long i = 0; i < io->nb_iogroups; i++)
	{
		if ((io->iogroups[i].name)
				&&	(!strcasecmp(group, io->iogroups[i].name)))
			return true;
	}

	return false;
}

void ARX_IOGROUP_Release(INTERACTIVE_OBJ * io)
{
	if (io->iogroups) free(io->iogroups);

	io->iogroups = NULL;
	io->nb_iogroups = 0;
}

void ARX_IOGROUP_Remove(INTERACTIVE_OBJ * io, const std::string& group)
{
	if ( group.empty() ) return;

	long toremove = -1;

	for (long i = 0; i < io->nb_iogroups; i++)
	{
		if ((io->iogroups[i].name)
				&&	(!strcasecmp(group, io->iogroups[i].name)))
			toremove = i;
	}

	if (toremove == -1) return;

	if (io->nb_iogroups == 1)
	{
		free(io->iogroups);
		io->iogroups = NULL;
		io->nb_iogroups = 0;
		return;
	}

	IO_GROUP_DATA * temporary = (IO_GROUP_DATA *)malloc(sizeof(IO_GROUP_DATA) * (io->nb_iogroups - 1));
	long pos = 0;

	for (int i = 0; i < io->nb_iogroups; i++)
	{
		if (i != toremove)
			strcpy(temporary[pos++].name, io->iogroups[i].name);
	}

	free(io->iogroups);
	io->iogroups = temporary;
	io->nb_iogroups--;
}

void ARX_IOGROUP_Add( INTERACTIVE_OBJ * io, const std::string& group )
{
	if ( group.empty() ) return;

	if (IsIOGroup(io, group)) return;

	io->iogroups = (IO_GROUP_DATA *)realloc(io->iogroups, sizeof(IO_GROUP_DATA) * (io->nb_iogroups + 1));
	strcpy(io->iogroups[io->nb_iogroups].name, group.c_str());
	io->nb_iogroups++;
}

INTERACTIVE_OBJ * ARX_SCRIPT_Get_IO_Max_Events()
{
	long max = -1;
	long ionum = -1;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
				&&	(inter.iobj[i]->stat_count > max))
		{
			ionum = i;
			max = inter.iobj[i]->stat_count;
		}
	}

	if (max <= 0) return NULL;

	if (ionum > -1) return inter.iobj[ionum];

	return NULL;
}

INTERACTIVE_OBJ * ARX_SCRIPT_Get_IO_Max_Events_Sent()
{
	long max = -1;
	long ionum = -1;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i] != NULL)
				&&	(inter.iobj[i]->stat_sent > max))
		{
			ionum = i;
			max = inter.iobj[i]->stat_sent;
		}
	}

	if (max <= 0) return NULL;

	if (ionum > -1) return inter.iobj[ionum];

	return NULL;
}

void ManageCasseDArme(INTERACTIVE_OBJ * io)
{
	if((io->type_flags & OBJECT_TYPE_DAGGER) ||
			(io->type_flags & OBJECT_TYPE_1H) ||
			(io->type_flags & OBJECT_TYPE_2H) ||
			(io->type_flags & OBJECT_TYPE_BOW)) {
		
		if(player.bag) {
			INTERACTIVE_OBJ * pObjMin = NULL;
			INTERACTIVE_OBJ * pObjMax = NULL;
			INTERACTIVE_OBJ * pObjFIX = NULL;
			bool bStop = false;
			
			for (int iNbBag = 0; iNbBag < player.bag; iNbBag++) {
				for (size_t j = 0; j < INVENTORY_Y; j++) {
					for (size_t i = 0; i < INVENTORY_X; i++) {
						
						if ((inventory[iNbBag][i][j].io) &&
								(inventory[iNbBag][i][j].io != io) &&
								((inventory[iNbBag][i][j].io->type_flags & OBJECT_TYPE_DAGGER) ||
								 (inventory[iNbBag][i][j].io->type_flags & OBJECT_TYPE_1H) ||
								 (inventory[iNbBag][i][j].io->type_flags & OBJECT_TYPE_2H) ||
								 (inventory[iNbBag][i][j].io->type_flags & OBJECT_TYPE_BOW)))
						{

							if ((io->ioflags & IO_ITEM) &&
									(inventory[iNbBag][i][j].io->ioflags & IO_ITEM) &&
									(inventory[iNbBag][i][j].io->_itemdata->equipitem))
							{
								if (inventory[iNbBag][i][j].io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value == io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value)
								{
									pIOChangeWeapon = inventory[iNbBag][i][j].io;
									lChangeWeapon = 2;
									bStop = true;
								}
								else
								{
									if (inventory[iNbBag][i][j].io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value > io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value)
									{
										if (pObjMin)
										{
											if (inventory[iNbBag][i][j].io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value > pObjMin->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value)
											{
												pObjMin = inventory[iNbBag][i][j].io;
											}
										}
										else
										{
											pObjMin = inventory[iNbBag][i][j].io;
										}
									}
									else
									{
										if (inventory[iNbBag][i][j].io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value < io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value)
										{
											if (pObjMax)
											{
												if (inventory[iNbBag][i][j].io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value < pObjMax->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value)
												{
													pObjMax = inventory[iNbBag][i][j].io;
												}
											}
											else
											{
												pObjMax = inventory[iNbBag][i][j].io;
											}
										}
									}
								}
							}
							else
							{
								if (!pObjFIX)
								{
									pObjFIX = inventory[iNbBag][i][j].io;
								}
							}
						}

						if (bStop)
						{
							break;
						}
					}

					if (bStop)
					{
						break;
					}
				}

				if (bStop)
				{
					break;
				}
				else
				{
					if (pObjMax)
					{
						pIOChangeWeapon = pObjMax;
						lChangeWeapon = 2;
					}
					else
					{
						if (pObjMin)
						{
							pIOChangeWeapon = pObjMin;
							lChangeWeapon = 2;
						}
						else
						{
							if (pObjFIX)
							{
								pIOChangeWeapon = pObjFIX;
								lChangeWeapon = 2;
							}
						}
					}
				}
			}
		}
	}
}

void loadScript(EERIE_SCRIPT & script, PakFile * file) {
	
	if(!file) {
		return;
	}
	
	if(script.data) {
		free(script.data);
	}
	
	script.data = (char *)malloc(file->size() + 2);
	script.size = file->size();
	script.data[script.size] = script.data[script.size + 1] = '\0'; // TODO(case-sensitive) remove
	
	file->read(script.data);
	
	std::transform(script.data, script.data + script.size, script.data, ::tolower);
	
	script.allowevents = 0;
	script.nblvar = 0;
	if(script.lvar) {
		free(script.lvar), script.lvar = NULL;
	}
	
	script.master = NULL;
	
	for(size_t j = 0; j < MAX_SCRIPTTIMERS; j++) {
		script.timers[j] = 0;
	}
	
	ARX_SCRIPT_ComputeShortcuts(script);
	
}
