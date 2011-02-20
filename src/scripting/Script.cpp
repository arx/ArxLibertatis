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

#include "scripting/Script.h"

#include <stddef.h>

#include <cassert>
#include <iomanip>

#include "ai/Paths.h"
#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/NPC.h"
#include "scene/Scene.h"
#include "gui/Speech.h"
#include "core/Time.h"
#include "core/Localization.h"
#include "core/Dialog.h"
#include "core/Resource.h"
#include "io/IO.h"
#include "io/Logger.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/Math.h"

#include "scripting/ScriptEvent.h"

using std::sprintf;
using std::min;
using std::max;

//#define NEEDING_DEBUG 1
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
char var_text[256];
char SSEPARAMS[MAX_SSEPARAMS][64];
long FORBID_SCRIPT_IO_CREATION = 0;
long RELOADING = 0;
long NB_GLOBALS = 0;
SCR_TIMER * scr_timer = NULL;
long ActiveTimers = 0;

int strcasecmp( const std::string& str1, const std::string& str2 )
{
	return strcasecmp( str1.c_str(), str2.c_str() );
}

int strcmp( const std::string& str1, const std::string& str2 )
{
	return str1.compare( str2 );
}

//*************************************************************************************
// FindScriptPos																	//
// Looks for string in script, return pos. Search start position can be set using	//
// poss parameter.																	//
//*************************************************************************************
long FindScriptPos(const EERIE_SCRIPT * es, const std::string& str)
{

	if (!es->data) return -1;

	const char * pdest = strstr(es->data, str.c_str());
	
	if(!pdest) {
		return -1;
	}
	
	long result = pdest - es->data;

	assert(result >= 0);

	int len2 = str.length();
	
	assert(len2 + result <= es->size);

	if (es->data[result+len2] <= 32) return result;

	return -1;
}

/**
 * Finds the first occurence of str in the script that is followed
 * by a separator (a character of value less then or equal 32)
 * 
 * @return The position of str in the script or -1 if str was not found.
 */
long FindScriptPosGOTO(const EERIE_SCRIPT * es, const string & str) {
	
	if(!es->data) {
		return -1;
	}
	
	size_t result = 0;
	size_t len2 = str.length();
	
	while(true) {
		
		const char * pdest = strstr(es->data + result, str.c_str());
		if(!pdest) {
			return -1;
		}
		
		result = pdest - es->data;
		
		assert(result + len2 <= (size_t)es->size);
		
		if(es->data[result + len2] <= 32) {
			return result + len2;
		}
		
		result += len2;
	}
	
}

bool CharIn( const std::string& str, char _char)
{
	return ( str.find( _char ) != std::string::npos );
}

bool iCharIn( const std::string& str, char _char)
{
	std::string temp = str;
	MakeUpcase(temp);
	_char = ::toupper(_char);
	return ( temp.find( _char ) != std::string::npos );
/*
	char * s = string;
	MakeUpcase(string);

	while (*s)
	{
		if ((*s) == _char) return true;

		s++;
	}

	return false; */
}



extern long FOR_EXTERNAL_PEOPLE;

//*************************************************************************************
// SCRIPT Precomputed Label Offsets Management
long FindLabelPos(EERIE_SCRIPT * es, const std::string& string) {
	return FindScriptPosGOTO(es, ">>" + string);
}

long ARX_SCRIPT_SearchTextFromPos(EERIE_SCRIPT * es, const std::string& search, long startpos, std::string& tline, long * nline)
{
	static long lastline = 0;
	long curline;

	long curpos; // current pos in stream
	long curtpos; // current pos in current line;
	std::string curtline;


	if (es == NULL) return -1;

	if (es->data == NULL) return -1;

	if (es->size <= 0) return -1;

	if (startpos == 0) curline = lastline = 0;
	else curline = lastline;

	curpos = startpos;

	// Get a line from stream
	while (curpos < es->size)
	{
		curtline.clear();
		curtpos = 0;

		while ((curpos < es->size) && (es->data[curpos] != '\n'))
		{
			curtline[curtpos] = es->data[curpos];
			curtpos++; // advance in line
			curpos++; // advance in stream
		}

		curpos++;
		curline++;

		if (curtpos > 0)
		{
			MakeUpcase(curtline);

			if ( curtline.find( search) != std::string::npos )
			{
				*nline = curline;
				tline = curtline;
				lastline = curline;
				return curpos;
			}
		}
	}

	lastline = 0;
	return -1;
}


long SendMsgToAllIO(long msg, const char * dat)
{
	long ret = ACCEPT;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
		{
			if (SendIOScriptEvent(inter.iobj[i], msg, dat) == REFUSE)
				ret = REFUSE;
		}
	}

	return ret;
}

void ARX_SCRIPT_LaunchScriptSearch( std::string& search)
{
	ShowText.clear();
	long foundnb = 0;
	long size = 0;
	std::string tline;
	std::string toadd;
	std::string objname;
	long nline;
	INTERACTIVE_OBJ * io = NULL;
	MakeUpcase(search);

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			io = inter.iobj[i];

			if (i == 0) objname = "PLAYER";
			else
			{
				std::stringstream ss;
				ss << GetName(io->filename) << '_' << std::setfill('0') << std::setw(4) << io->ident;
				objname = ss.str();
				//sprintf(objname, "%s_%04d", GetName(io->filename).c_str(), io->ident);
			}

			long pos = 0;

			while (pos != -1)
			{

				pos = ARX_SCRIPT_SearchTextFromPos(&io->script, search, pos, tline, &nline);

				if (pos > 0)
				{
					std::stringstream ss;
					ss << objname << " - GLOBAL - Line " << std::setw(4) << nline
					   << std::setw(0) << " : " << tline << '\n';
					//sprintf(toadd, "%s - GLOBAL - Line %4d : %s\n", objname, nline, tline);
					toadd = ss.str();

					if (size + toadd.length() + 3 < 65535)
					{
						ShowText += toadd;
						foundnb++;
					}
					else
					{
						ShowText += "...";
						goto suite;
					}

					size += toadd.length();
				}
			}

			pos = 0;

			while (pos != -1)
			{
				pos = ARX_SCRIPT_SearchTextFromPos(&io->over_script, search.c_str(), pos, tline, &nline);

				if (pos > 0)
				{
					std::stringstream ss;
					ss << objname << " - LOCAL  - Line " << std::setw(4) << nline
					   << std::setw(0) << " : " << tline << '\n';
					toadd = ss.str();
					//toadd, "%s - LOCAL  - Line %4ld : %s\n", objname, nline, tline);

					if (size + toadd.length() + 3 < 65535)
					{
						ShowText += toadd;
						foundnb++;
					}
					else
					{
						ShowText += "...";
						goto suite;
					}

					size += toadd.length();
				}
			}
		}
	}

suite:
	;

	if (foundnb <= 0)
	{
		ShowText = "No Occurence Found...";
	}

	std::stringstream ss;
	ss << "Search Results for " << search << '(' << foundnb << " occurences)";
	ShowTextWindowtext = ss.str();
	//sprintf(ShowTextWindowtext, "Search Results for %s (%ld occurences)", search.c_str(), foundnb);


	DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, danaeApp.m_hWnd, (DLGPROC)ShowTextDlg);
}

void ARX_SCRIPT_SetMainEvent(INTERACTIVE_OBJ * io, const std::string& newevent)
{
	if (io == NULL) return;

	if (!strcasecmp(newevent, "MAIN"))
		io->mainevent[0] = 0;
	else strcpy(io->mainevent, newevent.c_str());
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
				ARX_SCRIPT_SetMainEvent(inter.iobj[num], "MAIN");
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
extern long GORE_MODE;
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
						if (inter.iobj[i]->mainevent[0])
							SendIOScriptEvent(inter.iobj[i], 0, "", inter.iobj[i]->mainevent);
						else SendIOScriptEvent(inter.iobj[i], SM_MAIN);
					}
			}
		}
	}
}
//*************************************************************************************
//*************************************************************************************

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

//*************************************************************************************
// Checks if a string (seek) is at the start of another string (text)
// returns 0 if "seek" is at the start of "text"
// else returns 1
//*************************************************************************************
long specialstrcmp( const std::string& text, const std::string& seek)
{
	
	if ( text.compare( 0, seek.length(), seek ) == 0 )
		return 0;

	return 1;
/*

	long len = strlen(seek);
	long len2 = strlen(text);

	if (len2 < len) return 1;

	for (long i = 0; i < len; i++)
	{
		if (text[i] != seek[i]) return 1;
	}

	return 0;*/
}

//*************************************************************************************
//*************************************************************************************
long GetSystemVar(EERIE_SCRIPT * es,INTERACTIVE_OBJ * io, const std::string& _name, std::string& txtcontent, float * fcontent,long * lcontent)
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
					*fcontent = (float)EEDistance3D(&player.pos, &io->pos);
					return TYPE_FLOAT;
				}
			}

			break;
		case '#':

			if (!name.compare("^#PLAYERDIST"))
			{
				if (io != NULL)
				{
					*lcontent = EEDistance3D(&player.pos, &io->pos);
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
				*lcontent = GORE_MODE;
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
							EERIE_3D pos, pos2;
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

					*fcontent = 99999999999.f;
					return TYPE_FLOAT;
				}

				*lcontent = 0;
				return TYPE_LONG;
			}

			if (!specialstrcmp(name, "^ININITPOS"))
			{
				EERIE_3D pos;

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
					{
						txtcontent = "PLAYER";
					}
					else
					{
						std::stringstream ss;
						ss << GetName(EVENT_SENDER->filename) << '_'
						   << std::setfill('0') << std::setw(4) << EVENT_SENDER->ident;
						txtcontent = ss.str();
					}
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
					for (long i = 0; i < MAX_ASPEECH; i++)
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
				if (io == inter.iobj[0]) txtcontent = "PLAYER";
				else
				{
					std::stringstream ss;
					ss << GetName(io->filename) << '_' << std::setfill('0') << std::setw(4) << io->ident;
					txtcontent = ss.str();
				}

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
				long id = GetSpellId(temp);

				if (id >= 0)
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
				{
					std::stringstream ss;
					ss << GetName(LASTSPAWNED->filename) << '_'
					   << std::setfill('0') << std::setw(4) << LASTSPAWNED->ident;
					txtcontent = ss.str();
				}
				else txtcontent = "NONE";

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
							EERIE_3D pos, pos2;
							GetItemWorldPosition(io, &pos);
							GetItemWorldPosition(inter.iobj[t], &pos2);
							*fcontent = (float)EEDistance3D(&pos, &pos2);

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
				long id = GetSpellId(temp);

				if (id >= 0)
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

				if (!strcasecmp(name.c_str(), "^PLAYERSPELL_INVISIBILITY"))
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
				{
					txtcontent = "PLAYER";
				}
				else if (ioo)
				{
					std::stringstream ss;
					ss << GetName(ioo->filename) << '_' << std::setfill('0') << std::setw(4) << ioo->ident;
					txtcontent = ss.str();
				}
				else 	txtcontent = "NONE";

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
					{
						std::stringstream ss;
						ss << GetName(inter.iobj[io->targetinfo]->filename) << '_'
						   << std::setfill('0') << std::setw(4) << inter.iobj[io->targetinfo]->ident;
						txtcontent = ss.str();
					}
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
		{
			txtcontent = "PLAYER";
		}
		else
		{
			std::stringstream ss;
			ss << GetName(io->filename) << '_' << std::setfill('0') << std::setw(4) << io->ident;
			txtcontent = ss.str();
		}

		return TYPE_TEXT;
	}

	*lcontent = 0;
	return TYPE_LONG;
}

//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR * GetFreeVarSlot(SCRIPT_VAR*& _svff, long& _nb)
{

	SCRIPT_VAR * svf = _svff;
	_svff = (SCRIPT_VAR *) realloc(svf, sizeof(SCRIPT_VAR) * ((_nb) + 1));
	svf = _svff;
	memset(&svf[_nb], 0, sizeof(SCRIPT_VAR));
	_nb++;
	return &svf[_nb-1];
}

//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR* GetVarAddress(SCRIPT_VAR svf[], long& nb, const std::string& name)
{
	for (long i = 0; i < nb; i++)
	{
		if (svf[i].type != 0)
		{
			if (!strcasecmp(name, svf[i].name))
				return &svf[i];
		}
	}

	return NULL;
}
//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//*************************************************************************************
long GETVarValueLong(SCRIPT_VAR*& svf, long& nb, const std::string& name)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->ival;
}
//*************************************************************************************
//*************************************************************************************
float GETVarValueFloat(SCRIPT_VAR*& svf, long& nb, const std::string& name)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->fval;
}
//*************************************************************************************
//*************************************************************************************
std::string GETVarValueText(SCRIPT_VAR*& svf, long& nb, const std::string& name)
{
	SCRIPT_VAR* tsv = GetVarAddress(svf, nb, name);

	if (!tsv) return "";

	return tsv->text;
}

//*************************************************************************************
//*************************************************************************************
std::string GetVarValueInterpretedAsText( std::string& temp1, EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io)
{
	float t1;
	long l1;

	if (temp1[0] == '^')
	{
		long lv;
		float fv;
		std::string tv;

		switch (GetSystemVar(esss,io,temp1,tv,&fv,&lv))//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				strcpy(var_text, tv.c_str());
				return var_text;
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
		l1 = GETVarValueLong(svar, NB_GLOBALS, temp1);
		sprintf(var_text, "%ld", l1);
		return var_text;
	}
	else if (temp1[0] == '\xA7')
	{
		l1 = GETVarValueLong(esss->lvar, esss->nblvar, temp1);
		sprintf(var_text, "%ld", l1);
		return var_text;
	}
	else if (temp1[0] == '&') t1 = GETVarValueFloat(svar, NB_GLOBALS, temp1);
	else if (temp1[0] == '@') t1 = GETVarValueFloat(esss->lvar, esss->nblvar, temp1);
	else if (temp1[0] == '$')
	{
		std::string tempo = GETVarValueText(svar, NB_GLOBALS, temp1);

		if (tempo.empty()) strcpy(var_text, "VOID");
		else strcpy(var_text, tempo.c_str());

		return var_text;
	}
	else if (temp1[0] == '\xA3')
	{
		std::string tempo = GETVarValueText(esss->lvar, esss->nblvar, temp1);

		if (tempo.empty()) strcpy(var_text, "VOID");
		else strcpy(var_text, tempo.c_str());

		return var_text;
	}
	else
	{
		strcpy(var_text, temp1.c_str());
		return var_text;
	}

	sprintf(var_text, "%f", t1);
	return var_text;
}

//*************************************************************************************
//*************************************************************************************
float GetVarValueInterpretedAsFloat( std::string& temp1, EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io)
{
	if (temp1[0] == '^')
	{
		long lv;
		float fv;
		std::string tv; 

		switch (GetSystemVar(esss,io,temp1,tv,&fv,&lv)) //Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				return (float)atof(tv.c_str());
				break;
			case TYPE_LONG:
				return (float)lv;
				break;
				return (fv);
				break;
		}

	}
	else if (temp1[0] == '#')	return (float)GETVarValueLong(svar, NB_GLOBALS, temp1);
	else if (temp1[0] == '\xA7') return (float)GETVarValueLong(esss->lvar, esss->nblvar, temp1);
	else if (temp1[0] == '&') return GETVarValueFloat(svar, NB_GLOBALS, temp1);
	else if (temp1[0] == '@') return GETVarValueFloat(esss->lvar, esss->nblvar, temp1);

	return (float)atof(temp1.c_str());
}
//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************
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


//*************************************************************************************
//*************************************************************************************
long GetNumAnim( const std::string& name)
{


	char c	= ARX_CLEAN_WARN_CAST_CHAR(toupper(name[0]));


	switch (c)
	{
		case 'W':

			if (!strcasecmp(name, "WAIT"))		return ANIM_WAIT;

			if (!strcasecmp(name, "WAIT2"))		return ANIM_WAIT2;

			if (!strcasecmp(name, "WALK"))		return ANIM_WALK;

			if (!strcasecmp(name, "WALK1"))		return ANIM_WALK;

			if (!strcasecmp(name, "WALK2"))		return ANIM_WALK2;

			if (!strcasecmp(name, "WALK3"))		return ANIM_WALK3;

			if (!strcasecmp(name, "WALK_BACKWARD"))				return ANIM_WALK_BACKWARD;

			if (!strcasecmp(name, "WALK_MINISTEP"))			return ANIM_WALK_MINISTEP;

			if (!strcasecmp(name, "WAIT_SHORT"))			return ANIM_WAIT_SHORT;

			if (!strcasecmp(name, "WALK_SNEAK"))			return ANIM_WALK_SNEAK;

			break;
		case 'A':

			if (!strcasecmp(name, "ACTION"))		return ANIM_ACTION;
			else if (!strcasecmp(name, "ACTION1"))	return ANIM_ACTION;
			else if (!strcasecmp(name, "ACTION2"))	return ANIM_ACTION2;
			else if (!strcasecmp(name, "ACTION3"))	return ANIM_ACTION3;
			else if (!strcasecmp(name, "ACTION4"))	return ANIM_ACTION4;
			else if (!strcasecmp(name, "ACTION5"))	return ANIM_ACTION5;
			else if (!strcasecmp(name, "ACTION6"))	return ANIM_ACTION6;
			else if (!strcasecmp(name, "ACTION7"))	return ANIM_ACTION7;
			else if (!strcasecmp(name, "ACTION8"))	return ANIM_ACTION8;
			else if (!strcasecmp(name, "ACTION9"))	return ANIM_ACTION9;
			else if (!strcasecmp(name, "ACTION10"))	return ANIM_ACTION10;

			break;
		case 'H':

			if (!strcasecmp(name, "HIT1"))		return ANIM_HIT1;

			if (!strcasecmp(name, "HIT"))		return ANIM_HIT1;

			if (!strcasecmp(name, "HOLD_TORCH"))			return ANIM_HOLD_TORCH;

			if (!strcasecmp(name, "HIT_SHORT"))				return ANIM_HIT_SHORT;

			break;
		case 'S':

			if (!strcasecmp(name, "STRIKE1"))	return ANIM_STRIKE1;

			if (!strcasecmp(name, "STRIKE"))	return ANIM_STRIKE1;

			if (!strcasecmp(name, "SHIELD_START"))			return ANIM_SHIELD_START;

			if (!strcasecmp(name, "SHIELD_CYCLE"))			return ANIM_SHIELD_CYCLE;

			if (!strcasecmp(name, "SHIELD_HIT"))			return ANIM_SHIELD_HIT;

			if (!strcasecmp(name, "SHIELD_END"))			return ANIM_SHIELD_END;

			if (!strcasecmp(name, "STRAFE_RIGHT"))			return ANIM_STRAFE_RIGHT;

			if (!strcasecmp(name, "STRAFE_LEFT"))			return ANIM_STRAFE_LEFT;

			if (!strcasecmp(name, "STRAFE_RUN_LEFT"))		return ANIM_STRAFE_RUN_LEFT;

			if (!strcasecmp(name, "STRAFE_RUN_RIGHT"))		return ANIM_STRAFE_RUN_RIGHT;

			break;
		case 'D':

			if (!strcasecmp(name, "DIE"))		return ANIM_DIE;

			if (!strcasecmp(name, "DAGGER_READY_PART_1"))		return ANIM_DAGGER_READY_PART_1;

			if (!strcasecmp(name, "DAGGER_READY_PART_2"))		return ANIM_DAGGER_READY_PART_2;

			if (!strcasecmp(name, "DAGGER_UNREADY_PART_1"))		return ANIM_DAGGER_UNREADY_PART_1;

			if (!strcasecmp(name, "DAGGER_UNREADY_PART_2"))		return ANIM_DAGGER_UNREADY_PART_2;

			if (!strcasecmp(name, "DAGGER_WAIT"))				return ANIM_DAGGER_WAIT;

			if (!strcasecmp(name, "DAGGER_STRIKE_LEFT_START"))	return ANIM_DAGGER_STRIKE_LEFT_START;

			if (!strcasecmp(name, "DAGGER_STRIKE_LEFT_CYCLE"))	return ANIM_DAGGER_STRIKE_LEFT_CYCLE;

			if (!strcasecmp(name, "DAGGER_STRIKE_LEFT"))		return ANIM_DAGGER_STRIKE_LEFT;

			if (!strcasecmp(name, "DAGGER_STRIKE_RIGHT_START"))	return ANIM_DAGGER_STRIKE_RIGHT_START;

			if (!strcasecmp(name, "DAGGER_STRIKE_RIGHT_CYCLE"))	return ANIM_DAGGER_STRIKE_RIGHT_CYCLE;

			if (!strcasecmp(name, "DAGGER_STRIKE_RIGHT"))		return ANIM_DAGGER_STRIKE_RIGHT;

			if (!strcasecmp(name, "DAGGER_STRIKE_TOP_START"))	return ANIM_DAGGER_STRIKE_TOP_START;

			if (!strcasecmp(name, "DAGGER_STRIKE_TOP_CYCLE"))	return ANIM_DAGGER_STRIKE_TOP_CYCLE;

			if (!strcasecmp(name, "DAGGER_STRIKE_TOP"))			return ANIM_DAGGER_STRIKE_TOP;

			if (!strcasecmp(name, "DAGGER_STRIKE_BOTTOM_START")) return ANIM_DAGGER_STRIKE_BOTTOM_START;

			if (!strcasecmp(name, "DAGGER_STRIKE_BOTTOM_CYCLE")) return ANIM_DAGGER_STRIKE_BOTTOM_CYCLE;

			if (!strcasecmp(name, "DAGGER_STRIKE_BOTTOM"))		return ANIM_DAGGER_STRIKE_BOTTOM;

			if (!strcasecmp(name, "DEATH_CRITICAL"))		return ANIM_DEATH_CRITICAL;

			break;
		case 'R':

			if (!strcasecmp(name, "RUN"))		return ANIM_RUN;

			if (!strcasecmp(name, "RUN1"))		return ANIM_RUN;

			if (!strcasecmp(name, "RUN2"))		return ANIM_RUN2;

			if (!strcasecmp(name, "RUN3"))		return ANIM_RUN3;

			if (!strcasecmp(name, "RUN_BACKWARD"))		return ANIM_RUN_BACKWARD;

			break;
		case 'T':

			if (!strcasecmp(name, "TALK_NEUTRAL"))		return ANIM_TALK_NEUTRAL;

			if (!strcasecmp(name, "TALK_ANGRY"))		return ANIM_TALK_ANGRY;

			if (!strcasecmp(name, "TALK_HAPPY"))		return ANIM_TALK_HAPPY;

			if (!strcasecmp(name, "TALK_NEUTRAL_HEAD"))		return ANIM_TALK_NEUTRAL_HEAD;

			if (!strcasecmp(name, "TALK_ANGRY_HEAD"))		return ANIM_TALK_ANGRY_HEAD;

			if (!strcasecmp(name, "TALK_HAPPY_HEAD"))		return ANIM_TALK_HAPPY_HEAD;

			break;
		case 'B':

			if (!strcasecmp(name, "BARE_READY"))				return ANIM_BARE_READY;

			if (!strcasecmp(name, "BARE_UNREADY"))				return ANIM_BARE_UNREADY;

			if (!strcasecmp(name, "BARE_WAIT"))					return ANIM_BARE_WAIT;

			if (!strcasecmp(name, "BARE_STRIKE_LEFT_START"))	return ANIM_BARE_STRIKE_LEFT_START;

			if (!strcasecmp(name, "BARE_STRIKE_LEFT_CYCLE"))	return ANIM_BARE_STRIKE_LEFT_CYCLE;

			if (!strcasecmp(name, "BARE_STRIKE_LEFT"))			return ANIM_BARE_STRIKE_LEFT;

			if (!strcasecmp(name, "BARE_STRIKE_RIGHT_START"))	return ANIM_BARE_STRIKE_RIGHT_START;

			if (!strcasecmp(name, "BARE_STRIKE_RIGHT_CYCLE"))	return ANIM_BARE_STRIKE_RIGHT_CYCLE;

			if (!strcasecmp(name, "BARE_STRIKE_RIGHT"))			return ANIM_BARE_STRIKE_RIGHT;

			if (!strcasecmp(name, "BARE_STRIKE_TOP_START"))		return ANIM_BARE_STRIKE_TOP_START;

			if (!strcasecmp(name, "BARE_STRIKE_TOP_CYCLE"))		return ANIM_BARE_STRIKE_TOP_CYCLE;

			if (!strcasecmp(name, "BARE_STRIKE_TOP"))			return ANIM_BARE_STRIKE_TOP;

			if (!strcasecmp(name, "BARE_STRIKE_BOTTOM_START"))	return ANIM_BARE_STRIKE_BOTTOM_START;

			if (!strcasecmp(name, "BARE_STRIKE_BOTTOM_CYCLE"))	return ANIM_BARE_STRIKE_BOTTOM_CYCLE;

			if (!strcasecmp(name, "BARE_STRIKE_BOTTOM"))		return ANIM_BARE_STRIKE_BOTTOM;

			break;
		case '1':

			if (!strcasecmp(name, "1H_READY_PART_1"))			return ANIM_1H_READY_PART_1;

			if (!strcasecmp(name, "1H_READY_PART_2"))			return ANIM_1H_READY_PART_2;

			if (!strcasecmp(name, "1H_UNREADY_PART_1"))			return ANIM_1H_UNREADY_PART_1;

			if (!strcasecmp(name, "1H_UNREADY_PART_2"))			return ANIM_1H_UNREADY_PART_2;

			if (!strcasecmp(name, "1H_WAIT"))					return ANIM_1H_WAIT;

			if (!strcasecmp(name, "1H_STRIKE_LEFT_START"))		return ANIM_1H_STRIKE_LEFT_START;

			if (!strcasecmp(name, "1H_STRIKE_LEFT_CYCLE"))		return ANIM_1H_STRIKE_LEFT_CYCLE;

			if (!strcasecmp(name, "1H_STRIKE_LEFT"))			return ANIM_1H_STRIKE_LEFT;

			if (!strcasecmp(name, "1H_STRIKE_RIGHT_START"))		return ANIM_1H_STRIKE_RIGHT_START;

			if (!strcasecmp(name, "1H_STRIKE_RIGHT_CYCLE"))		return ANIM_1H_STRIKE_RIGHT_CYCLE;

			if (!strcasecmp(name, "1H_STRIKE_RIGHT"))			return ANIM_1H_STRIKE_RIGHT;

			if (!strcasecmp(name, "1H_STRIKE_TOP_START"))		return ANIM_1H_STRIKE_TOP_START;

			if (!strcasecmp(name, "1H_STRIKE_TOP_CYCLE"))		return ANIM_1H_STRIKE_TOP_CYCLE;

			if (!strcasecmp(name, "1H_STRIKE_TOP"))				return ANIM_1H_STRIKE_TOP;

			if (!strcasecmp(name, "1H_STRIKE_BOTTOM_START"))	return ANIM_1H_STRIKE_BOTTOM_START;

			if (!strcasecmp(name, "1H_STRIKE_BOTTOM_CYCLE"))	return ANIM_1H_STRIKE_BOTTOM_CYCLE;

			if (!strcasecmp(name, "1H_STRIKE_BOTTOM"))			return ANIM_1H_STRIKE_BOTTOM;

			break;
		case '2':

			if (!strcasecmp(name, "2H_READY_PART_1"))			return ANIM_2H_READY_PART_1;

			if (!strcasecmp(name, "2H_READY_PART_2"))			return ANIM_2H_READY_PART_2;

			if (!strcasecmp(name, "2H_UNREADY_PART_1"))			return ANIM_2H_UNREADY_PART_1;

			if (!strcasecmp(name, "2H_UNREADY_PART_2"))			return ANIM_2H_UNREADY_PART_2;

			if (!strcasecmp(name, "2H_WAIT"))					return ANIM_2H_WAIT;

			if (!strcasecmp(name, "2H_STRIKE_LEFT_START"))		return ANIM_2H_STRIKE_LEFT_START;

			if (!strcasecmp(name, "2H_STRIKE_LEFT_CYCLE"))		return ANIM_2H_STRIKE_LEFT_CYCLE;

			if (!strcasecmp(name, "2H_STRIKE_LEFT"))			return ANIM_2H_STRIKE_LEFT;

			if (!strcasecmp(name, "2H_STRIKE_RIGHT_START"))		return ANIM_2H_STRIKE_RIGHT_START;

			if (!strcasecmp(name, "2H_STRIKE_RIGHT_CYCLE"))		return ANIM_2H_STRIKE_RIGHT_CYCLE;

			if (!strcasecmp(name, "2H_STRIKE_RIGHT"))			return ANIM_2H_STRIKE_RIGHT;

			if (!strcasecmp(name, "2H_STRIKE_TOP_START"))		return ANIM_2H_STRIKE_TOP_START;

			if (!strcasecmp(name, "2H_STRIKE_TOP_CYCLE"))		return ANIM_2H_STRIKE_TOP_CYCLE;

			if (!strcasecmp(name, "2H_STRIKE_TOP"))				return ANIM_2H_STRIKE_TOP;

			if (!strcasecmp(name, "2H_STRIKE_BOTTOM_START"))	return ANIM_2H_STRIKE_BOTTOM_START;

			if (!strcasecmp(name, "2H_STRIKE_BOTTOM_CYCLE"))	return ANIM_2H_STRIKE_BOTTOM_CYCLE;

			if (!strcasecmp(name, "2H_STRIKE_BOTTOM"))			return ANIM_2H_STRIKE_BOTTOM;

			break;
		case 'M':

			if (!strcasecmp(name, "MISSILE_READY_PART_1"))		return ANIM_MISSILE_READY_PART_1;

			if (!strcasecmp(name, "MISSILE_READY_PART_2"))		return ANIM_MISSILE_READY_PART_2;

			if (!strcasecmp(name, "MISSILE_UNREADY_PART_1"))	return ANIM_MISSILE_UNREADY_PART_1;

			if (!strcasecmp(name, "MISSILE_UNREADY_PART_2"))	return ANIM_MISSILE_UNREADY_PART_2;

			if (!strcasecmp(name, "MISSILE_WAIT"))				return ANIM_MISSILE_WAIT;

			if (!strcasecmp(name, "MISSILE_STRIKE_PART_1"))		return ANIM_MISSILE_STRIKE_PART_1;

			if (!strcasecmp(name, "MISSILE_STRIKE_PART_2"))		return ANIM_MISSILE_STRIKE_PART_2;

			if (!strcasecmp(name, "MISSILE_STRIKE_CYCLE"))		return ANIM_MISSILE_STRIKE_CYCLE;

			if (!strcasecmp(name, "MISSILE_STRIKE"))			return ANIM_MISSILE_STRIKE;

			if (!strcasecmp(name, "MEDITATION"))			return ANIM_MEDITATION;

			break;
		case 'C':

			if (!strcasecmp(name, "CAST_START"))			return ANIM_CAST_START;

			if (!strcasecmp(name, "CAST_CYCLE"))			return ANIM_CAST_CYCLE;

			if (!strcasecmp(name, "CAST"))					return ANIM_CAST;

			if (!strcasecmp(name, "CAST_END"))				return ANIM_CAST_END;

			if (!strcasecmp(name, "CROUCH"))				return ANIM_CROUCH;

			if (!strcasecmp(name, "CROUCH_WALK"))			return ANIM_CROUCH_WALK;

			if (!strcasecmp(name, "CROUCH_WALK_BACKWARD"))	return ANIM_CROUCH_WALK_BACKWARD;

			if (!strcasecmp(name, "CROUCH_STRAFE_LEFT"))	return ANIM_CROUCH_STRAFE_LEFT;

			if (!strcasecmp(name, "CROUCH_STRAFE_RIGHT"))	return ANIM_CROUCH_STRAFE_RIGHT;

			if (!strcasecmp(name, "CROUCH_START"))			return ANIM_CROUCH_START;

			if (!strcasecmp(name, "CROUCH_WAIT"))			return ANIM_CROUCH_WAIT;

			if (!strcasecmp(name, "CROUCH_END"))			return ANIM_CROUCH_END;

			break;
		case 'L':

			if (!strcasecmp(name, "LEAN_RIGHT"))			return ANIM_LEAN_RIGHT;

			if (!strcasecmp(name, "LEAN_LEFT"))				return ANIM_LEAN_LEFT;

			if (!strcasecmp(name, "LEVITATE"))				return ANIM_LEVITATE;

			break;
		case 'J':

			if (!strcasecmp(name, "JUMP"))					return ANIM_JUMP;

			if (!strcasecmp(name, "JUMP_ANTICIPATION"))		return ANIM_JUMP_ANTICIPATION;

			if (!strcasecmp(name, "JUMP_UP"))				return ANIM_JUMP_UP;

			if (!strcasecmp(name, "JUMP_CYCLE"))			return ANIM_JUMP_CYCLE;

			if (!strcasecmp(name, "JUMP_END"))				return ANIM_JUMP_END;

			if (!strcasecmp(name, "JUMP_END_PART2"))		return ANIM_JUMP_END_PART2;

			break;
		case 'F':

			if (!strcasecmp(name, "FIGHT_WALK_FORWARD"))	return ANIM_FIGHT_WALK_FORWARD;

			if (!strcasecmp(name, "FIGHT_WALK_BACKWARD"))	return ANIM_FIGHT_WALK_BACKWARD;

			if (!strcasecmp(name, "FIGHT_WALK_MINISTEP"))	return ANIM_FIGHT_WALK_MINISTEP;

			if (!strcasecmp(name, "FIGHT_STRAFE_RIGHT"))	return ANIM_FIGHT_STRAFE_RIGHT;

			if (!strcasecmp(name, "FIGHT_STRAFE_LEFT"))		return ANIM_FIGHT_STRAFE_LEFT;

			if (!strcasecmp(name, "FIGHT_WAIT"))			return ANIM_FIGHT_WAIT;

			break;
		case 'G':

			if (!strcasecmp(name, "GRUNT"))					return ANIM_GRUNT;

			break;
		case 'U':

			if (!strcasecmp(name, "U_TURN_LEFT"))			return ANIM_U_TURN_LEFT;

			if (!strcasecmp(name, "U_TURN_RIGHT"))			return ANIM_U_TURN_RIGHT;

			if (!strcasecmp(name, "U_TURN_LEFT_FIGHT"))		return ANIM_U_TURN_LEFT_FIGHT;

			if (!strcasecmp(name, "U_TURN_RIGHT_FIGHT"))	return ANIM_U_TURN_RIGHT_FIGHT;

			break;
	}

	return -1;
}

long LINEEND;
INTERACTIVE_OBJ * _CURIO = NULL;
//*************************************************************************************
//returns pos after this word.														//
// -1 failure																		//
// flags =0 standard																//
//		 =1 no INTERPRETATION (except for ~ )										//
//*************************************************************************************
/* TODO Clean up this mess */
long GetNextWord( EERIE_SCRIPT * es, long i, std::string& temp, long flags )
{

	// Avoids negative position...
	if (i < 0) return -1;

	// Avoids position superior to script size
	if (i >= es->size) return -1;

	long tildes = 0;	// number of tildes
	long old = i;		// stores start pos in old
	LINEEND = 0;		// Global LINEEND set to 0 (no LINEEND for now)
	unsigned char * esdat = (unsigned char *)es->data;

	// First ignores spaces & unused chars
	while ((i < es->size) &&
			((esdat[i] <= 32)	|| (esdat[i] == '(') || (esdat[i] == ')'))
		  )
	{
		if (es->data[i] == '\n') LINEEND = 1;

		i++;
	}

	temp.clear();
	// now take chars until it finds a space or unused char
	while ((esdat[i] > 32)
			&& (esdat[i] != '(')
			&& (esdat[i] != ')')
		  )
	{


		if (esdat[i] == '"')
		{
			i++;

			if (i >= es->size) return -1;

			while ((esdat[i] != '"') && (!LINEEND))
			{
				if (esdat[i] == '\n') LINEEND = 1;
				else if (esdat[i] == '~') tildes++;

				temp.push_back(esdat[i]);
				i++;

				if (i >= es->size) return -1;

			}

			return i + 1;
		}
		else
		{
			temp.push_back(esdat[i]);

			if (esdat[i] == '~') tildes++;
		}

		i++;

		if (i >= es->size) return -1;

	}

	if (i == old) return -1;

	// Now retreives Tilded chars...
	if ((!(flags & 2))
			&&	(tildes > 0)
	   )
	{
		long _pos = 0;
		long _size = temp.length();

		while ((_pos < _size) && (tildes >= 2))
		{
			if (temp[_pos] == '~')
			{
				long start = _pos + 1;
				_pos++;

				while ((_pos < _size) && (temp[_pos] != '~'))
				{
					_pos++;
				}

				long end = _pos - 1;

				// Found A tilded string...
				if (end > start)
				{
					std::string tildedd;
					char interp[256];
					char result[512];
					tildedd.assign( temp.substr( start, end - start + 1) );
					tildedd[end-start+1] = 0;

					if (es->master)
						strcpy(interp, GetVarValueInterpretedAsText(tildedd, (EERIE_SCRIPT *)es->master, _CURIO).c_str());
					else
						strcpy(interp, GetVarValueInterpretedAsText(tildedd, es, _CURIO).c_str());

					if (start == 1) strcpy(result, "");
					else
					{
						memcpy(result, temp.c_str(), start - 1);
						result[start-1] = 0;
					}

					strcat(result, interp);

					if (end + 2 < _size)
						strcat(result, temp.c_str() + end + 2);

					_pos = -1;
					temp = result;
					_size = temp.length();
					tildes -= 2;
				}
			}

			_pos++;
		}
	}

	return i;
}


//*************************************************************************************
//*************************************************************************************
long GetNextWord_Interpreted( INTERACTIVE_OBJ * io, EERIE_SCRIPT * es, long i, std::string& temp )
{
	long pos=GetNextWord(es,i,temp);
	std::stringstream ss;
	if	(temp[0]=='^') {
		long lv;
		float fv;
		std::string tv;

		switch (GetSystemVar(es,io,temp,tv,&fv,&lv)) //Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				temp = tv;
				break;
			case TYPE_LONG:
				ss << lv;
				temp = ss.str();
				break;
			case TYPE_FLOAT:
				ss << fv;
				temp = ss.str();
				break;
		}
	}
	else if	(temp[0] == '#')
	{
		ss << GETVarValueLong(svar, NB_GLOBALS, temp);
		temp = ss.str();
	}
	else if (temp[0] == '\xA7')
	{
		ss << GETVarValueLong(es->lvar, es->nblvar, temp);
		temp = ss.str();
	}
	else if (temp[0] == '&')
	{
		ss << GETVarValueFloat(svar, NB_GLOBALS, temp);
		temp = ss.str();
	}
	else if (temp[0] == '@')
	{
		ss << GETVarValueFloat(es->lvar, es->nblvar, temp);
		temp = ss.str();
	}
	else if (temp[0] == '$')
	{
		temp = GETVarValueText(svar, NB_GLOBALS, temp);
	}
	else if (temp[0] == '\xA3')
	{
		temp = GETVarValueText(es->lvar, es->nblvar, temp);
	}

	return pos;
}

//*************************************************************************************
//*************************************************************************************
long GotoNextLine(EERIE_SCRIPT * es, long pos)
{
	while ((es->data[pos] != '\n'))
	{
		if (es->data[pos] == '\0') return -1;

		pos++;
	}

	pos++;
	return pos;
}

//*************************************************************************************
//*************************************************************************************
long SkipNextStatement(EERIE_SCRIPT * es, long pos)
{
	std::string temp;
	long brack = 1;
	long tpos;
	pos = GetNextWord(es, pos, temp);

	if (temp[0] == '{')
	{
		while (brack > 0)
		{
			pos = GetNextWord(es, pos, temp);

			if (pos < 0) return -1;

			if (temp[0] == '{') brack++;

			if (temp[0] == '}') brack--;
		}
	}
	else pos = GotoNextLine(es, pos);

	tpos = GetNextWord(es, pos, temp);
	MakeUpcase(temp);

	if (!temp.compare("ELSE"))
	{
		pos = tpos;
	}

	return pos;
}

void MakeGlobalText( std::string& tx)
{
	char texx[256];

	for (long i = 0; i < NB_GLOBALS; i++)
	{
		switch (svar[i].type)
		{
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
		}
	}
}
//*************************************************************************************
//*************************************************************************************
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
		}
	}
}

extern INTERACTIVE_OBJ * LastSelectedIO;

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

		ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;
		aup->_curtime += smoothing + 100;
		EERIE_3D tp;
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
			EERIE_3D pos;

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
//*************************************************************************************
void CheckHit(INTERACTIVE_OBJ * io, float ratioaim)
{

	if (io == NULL) return;




	{
		EERIE_3D ppos, pos, from, to;
		Vector_Init(&from, 0.f, 0.f, -90.f);
		Vector_RotateY(&to, &from, MAKEANGLE(180.f - io->angle.b));
		ppos.x = io->pos.x;
		pos.x = ppos.x + to.x;
		ppos.y = io->pos.y - (80.f);
		pos.y = ppos.y + to.y;
		ppos.z = io->pos.z;
		pos.z = ppos.z + to.z;

		if (DEBUGNPCMOVE) EERIEDrawTrue3DLine(GDevice, &ppos, &pos, D3DRGB(1.f, 0.f, 0.f));

		float dmg;

		if (io->ioflags & IO_NPC)
		{
			dmg = io->_npcdata->damages;
		}
		else dmg = 40.f;



		long i = io->targetinfo;
		float dist;

		if (!ValidIONum(i)) return;

		{
			INTERACTIVE_OBJ * ioo = inter.iobj[i];

			if (! ioo) return;

			if (ioo->ioflags & IO_MARKER) return;

			if (ioo->ioflags & IO_CAMERA) return;


			if (ioo->GameFlags & GFLAG_ISINTREATZONE)
				if (ioo->show == SHOW_FLAG_IN_SCENE)
					if (ioo->obj)
						if (ioo->pos.y >	(io->pos.y + io->physics.cyl.height))
							if (io->pos.y >	(ioo->pos.y + ioo->physics.cyl.height))
							{
								float dist_limit = io->_npcdata->reach + io->physics.cyl.radius;
								long count = 0;
								float mindist = FLT_MAX;

								for (size_t k = 0; k < ioo->obj->vertexlist.size(); k += 2)
								{
									dist = EEDistance3D(&pos, &inter.iobj[i]->obj->vertexlist3[k].v);

									if ((dist <= dist_limit)
											&&	(EEfabs(pos.y - inter.iobj[i]->obj->vertexlist3[k].v.y) < 60.f))
									{
										count++;

										if (dist < mindist) mindist = dist;
									}
								}

								float ratio = ((float)count / ((float)ioo->obj->vertexlist.size() * ( 1.0f / 2 )));

								if (ioo->ioflags & IO_NPC)
								{

									if (mindist <= dist_limit)
									{
										ARX_EQUIPMENT_ComputeDamages(io, ioo, ratioaim);
									}

								}
								else
								{
									dist = EEDistance3D(&pos, &ioo->pos);

									if (mindist <= 120.f)
									{
										ARX_DAMAGES_DamageFIX(ioo, dmg * ratio, GetInterNum(io), 0);
									}
								}
							}
		}


	}
}

void MakeStandard( std::string& str)
{
	size_t i = 0;
	
	while ( i < str.length() )
	{
		if (str[i] != '_')
		{
			str[i] = toupper(str[i]);
			i++;
		} else {
			str.erase(i, 1);
		}
	}
}

//*************************************************************************************
//*************************************************************************************

long MakeLocalised( const std::string& text, std::string& output, long lastspeechflag)
{
	if ( text.empty() )
	{
		output = "ERROR";
		return 0;
	}

	std::string __text;
	return HERMES_UNICODE_GetProfileString(text, "error", output);
}

//-----------------------------------------------------------------------------
long ARX_SPEECH_AddLocalised(INTERACTIVE_OBJ * io, const std::string& text, long duration)
{
	std::string output;

	HERMES_UNICODE_GetProfileString(
		text,
		"Not Found",
		output );
	return (ARX_SPEECH_Add(io, output, duration));
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

	if ((params == NULL)) return;

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

//*************************************************************************************
//*************************************************************************************
long NotifyIOEvent(INTERACTIVE_OBJ * io, long msg)
{
	if (SendIOScriptEvent(io, msg) != REFUSE)
	{
		switch (msg)
		{
			case SM_DIE:
			{
				if (io && ValidIOAddress(io))
				{
					io->infracolor.b = 1.f;
					io->infracolor.g = 0.f;
					io->infracolor.r = 0.f;
				}
			}
		}

		return ACCEPT;
	}

	return REFUSE;
}
//*************************************************************************************
//*************************************************************************************
#define MAX_EVENT_STACK 800
struct STACKED_EVENT
{
	INTERACTIVE_OBJ* sender;
	long             exist;
	INTERACTIVE_OBJ* io;
	long             msg;
	std::string      params;
	std::string      eventname;
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
			eventstack[i].msg = 0;
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
				eventstack[i].msg = 0;
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
			eventstack[i].msg = 0;
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

void Stack_SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, const std::string& params, const std::string& eventname)
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

long SendIOScriptEventReverse(INTERACTIVE_OBJ * io, long msg, const std::string& params, const std::string& eventname)
{
	// LogDebug << "SendIOScriptEventReverse "<< eventname;
	// checks invalid IO
	if (!io) return -1;

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

long SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, const std::string& params, const std::string& eventname)
{
	//if (msg != 8)
	//	LogDebug << "SendIOScriptEvent event '"<< eventname<<"' message " << msg;
	// checks invalid IO
	if (!io) return -1;

	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		INTERACTIVE_OBJ * oes = EVENT_SENDER;

		if ((msg == SM_INIT) || (msg == SM_INITEND))
		{
			if (inter.iobj[num])
			{
				SendIOScriptEventReverse(inter.iobj[num], msg, params.c_str(), eventname.c_str());
				EVENT_SENDER = oes;
			}
		}

		// if this IO only has a Local script, send event to it
		if (inter.iobj[num] && !inter.iobj[num]->over_script.data)
		{
			long ret = ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
			EVENT_SENDER = oes;
			return ret;
		}

		// If this IO has a Global script send to Local (if exists)
		// then to Global if no overriden by Local
		if (inter.iobj[num] && (ScriptEvent::send(&inter.iobj[num]->over_script, msg, params, inter.iobj[num], eventname) != REFUSE))
		{
			EVENT_SENDER = oes;

			if (inter.iobj[num])
			{
				long ret = (ScriptEvent::send(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname));
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








long SendInitScriptEvent(INTERACTIVE_OBJ * io)
{
	if (!io) return -1;

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

//*************************************************************************************
//*************************************************************************************
// Checks if timer named texx exists. If so returns timer index else return -1.
long ARX_SCRIPT_Timer_Exist(char * texx)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if ( !scr_timer[i].name.compare(texx) )
			{
				return i;
			}
		}
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
// Generates a random name for an unnamed timer
void ARX_SCRIPT_Timer_GetDefaultName(char * tx)
{
	long i = 1;
	char texx[64];

	while (1)
	{
		sprintf(texx, "TIMER_%ld", i);
		i++;

		if (ARX_SCRIPT_Timer_Exist(texx) == -1)
		{
			strcpy(tx, texx);
			return;
		}
	}
}
//*************************************************************************************
// Get a free script timer
//*************************************************************************************
long ARX_SCRIPT_Timer_GetFree()
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (!(scr_timer[i].exist))
			return i;
	}

	return -1;
}
//*************************************************************************************
// Count the number of active script timers...
//*************************************************************************************
long ARX_SCRIPT_CountTimers()
{
	return ActiveTimers;
}
//*************************************************************************************
// ARX_SCRIPT_Timer_ClearByNum
// Clears a timer by its Index (long timer_idx) on the timers list
//*************************************************************************************
void ARX_SCRIPT_Timer_ClearByNum(long timer_idx)
{
	if (scr_timer[timer_idx].exist)
	{
		scr_timer[timer_idx].name.clear();
		ActiveTimers--;
		scr_timer[timer_idx].namelength = 0;
		scr_timer[timer_idx].exist = 0;
	}
}
//*************************************************************************************
// ARX_SCRIPT_Timer_ForceCallnKill
//
// Forces a timer to die after having called one time its callback script position
// by its Index (long timer_idx) on the timers list
//*************************************************************************************













//*************************************************************************************
//*************************************************************************************
void ARX_SCRIPT_Timer_Clear_By_Name_And_IO(char * timername, INTERACTIVE_OBJ * io)
{
	for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if (scr_timer[i].exist)
		{
			if ((scr_timer[i].io == io) &&
					(!scr_timer[i].name.compare(timername)))
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
//*************************************************************************************
//*************************************************************************************
void ARX_SCRIPT_Timer_ClearAll()
{
	if (ActiveTimers)
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
			ARX_SCRIPT_Timer_ClearByNum(i);

	ActiveTimers = 0;
}
//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************
























long ARX_SCRIPT_GetSystemIOScript(INTERACTIVE_OBJ * io, const std::string& name)
{
	if (ActiveTimers)
	{
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
		{
			if (scr_timer[i].exist)
			{
				if ((scr_timer[i].io == io) && (!scr_timer[i].name.compare(name)))
				{
					return i;
				}
			}
		}
	}

	return -1;
}

long Manage_Specific_RAT_Timer(SCR_TIMER * st)
{
	INTERACTIVE_OBJ * io = st->io;
	GetTargetPos(io);
	EERIE_3D target;
	target.x = io->target.x - io->pos.x;
	target.y = io->target.y - io->pos.y;
	target.z = io->target.z - io->pos.z;
	Vector_Normalize(&target);
	EERIE_3D targ;
	Vector_RotateY(&targ, &target, rnd() * 60.f - 30.f);
	target.x = io->target.x + targ.x * 100.f;
	target.y = io->target.y + targ.y * 100.f;
	target.z = io->target.z + targ.z * 100.f;

	if (ARX_INTERACTIVE_ConvertToValidPosForIO(io, &target))
	{
		ARX_INTERACTIVE_Teleport(io, &target);
		EERIE_3D pos;
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

//*************************************************************************************
//*************************************************************************************
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

					if (!es)
					{
						if (!st->name.compare("_R_A_T_"))
						{
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
long CountBrackets(EERIE_SCRIPT * es)
{
	long count = 0;

	for (long i = 0; i < es->size; i++)
	{
		if ((es->data[i] == '/') && (i < es->size - 1) && (es->data[i+1] == '/'))
		{
			i = GotoNextLine(es, i);

			if (i == -1) return count;
		}

		if (es->data[i] == '{') count++;

		if (es->data[i] == '}') count--;
	}

	return count;
}
HWND LastErrorPopup = NULL;
extern long SHOWWARNINGS;
long GetCurrentLine(EERIE_SCRIPT * es, long poss)
{
	long pos = 0;
	long linecount = -1;

	while ((pos < poss) && (pos < es->size))
	{
		pos = GotoNextLine(es, pos);

		if (pos == -1) return -1;

		linecount++;
	}

	if (linecount == -1) return -1;

	return linecount + 1;
}
long GetLastLineNum(EERIE_SCRIPT * es)
{
	long pos = 0;
	long linecount = -1;

	while (pos < es->size)
	{
		pos = GotoNextLine(es, pos);

		if (pos == -1) return linecount + 2;

		linecount++;
	}

	if (linecount == -1) return -1;

	return linecount + 2;
}

void GetLineAsText(EERIE_SCRIPT * es, long curline, char * tex)
{
	long curpos = 0;
	long pos = 1;
	long linecount = -1;

	while ((linecount + 2 < curline) && (pos < es->size))
	{
		pos = GotoNextLine(es, pos);

		if (pos == -1)
		{
			strcpy(tex, "Internal ERROR...");
			return ;
		}

		linecount++;
	}

	while (pos + curpos < es->size)
	{
		if ((es->data[pos+curpos] == '\n') || (es->data[pos+curpos] == '\0'))
		{
			tex[curpos] = 0;
			return;
		}

		tex[curpos] = es->data[pos+curpos];
		curpos++;
	}

	strcpy(tex, "Internal ERROR...");
}
extern long SYNTAXCHECKING;
long LaunchScriptCheck(EERIE_SCRIPT * es, INTERACTIVE_OBJ * io)
{
//	return 1;

//	if (SYNTAXCHECKING == 0) return 1;
	//TODO(lubosz): Make validator functional
	LogDebug << "LaunchScriptCheck";

	if (io->ioflags & IO_FREEZESCRIPT) return 1;

	long errors = 0;
	long warnings = 0;
	long stoppingdebug = 0;
	long brackets = 0;
	char errstring[65535];
	char tem[256];

	if (io == NULL) return 1;

	if (es == NULL) return 1;

	if (!es->data) return 1;

	if (es->data[0] == 0) return 1;

	long returnvalue = 1;
	errstring[0] = 0;
	long cb = CountBrackets(es);

	if (cb != 0)
	{
		if (cb > 0) sprintf(tem, "Global - Warning: Invalid Number of Closing Brackets. %ld '}' missed\n", cb);
		else sprintf(tem, "Global - Warning: Invalid Number of Opening Brackets. %ld '{' missed\n", -cb);

		if (strlen(tem) + strlen(errstring) < 65480) strcat(errstring, tem);
		else stoppingdebug = 1;

		warnings++;
	}

	std::string temp;
	std::string temp1;
	std::string temp2;
	std::string temp3;

	char curlinetext[512];
	long pos = 0;
	_CURIO = io;

	while (((pos = GetNextWord(es, pos, temp)) >= 0) && (pos >= 0) && (pos < es->size - 1))
	{
		MakeStandard(temp);
		long currentline = GetCurrentLine(es, pos);

		if (currentline == -1)
		{
			currentline = GetLastLineNum(es);
		}

		memset(curlinetext, 0, 512);
		GetLineAsText(es, currentline, curlinetext);
		tem[0] = 0;
		long unknowncommand = 0;

		switch (temp[0])
		{
			case '}':
				brackets--;
				break;
			case 'O':

				if (!temp.compare("OBJECTHIDE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("ON"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case '{':
				brackets++;
				break;
			case '/':

				if (temp[1] == '/') pos = GotoNextLine(es, pos);
				else unknowncommand = 1;

				break;
			case '>':

				if (temp[1] == '>') pos = GotoNextLine(es, pos);
				else unknowncommand = 1;

				break;
			case 'B':

				if (!temp.compare("BOOK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("BEHAVIOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!temp.compare("STACK"))
					{
					}
					else if (!strcasecmp(temp.c_str(), "UNSTACK"))
					{
					}
					else if (!strcasecmp(temp.c_str(), "UNSTACKALL"))
					{
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
						}

						if (!strcasecmp(temp.c_str(), "FLEE"))
							pos = GetNextWord(es, pos, temp);
						else if (!strcasecmp(temp.c_str(), "LOOK_FOR"))
							pos = GetNextWord(es, pos, temp);
						else if (!strcasecmp(temp.c_str(), "HIDE"))
							pos = GetNextWord(es, pos, temp);
						else if (!strcasecmp(temp.c_str(), "WANDER_AROUND"))
							pos = GetNextWord(es, pos, temp);
					}
				}
				else unknowncommand = 1;

				break;
			case 'A':

				if (!temp.compare("ADDBAG"))
				{
				}

				if (!temp.compare("ACTIVATEPHYSICS"))
				{
				}

				if (!temp.compare("AMBIANCE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'V'))
						{
							pos = GetNextWord(es, pos, temp);

							pos = GetNextWord(es, pos, temp);
						}
						else if (iCharIn(temp, 'N'))
						{
							pos = GetNextWord(es, pos, temp);
						}
						else if (iCharIn(temp, 'M'))
						{
							pos = GetNextWord(es, pos, temp);

							pos = GetNextWord(es, pos, temp);

						}
						else if (iCharIn(temp, 'U'))
						{

							pos = GetNextWord(es, pos, temp);

							pos = GetNextWord(es, pos, temp);

						}
					}
				}
				else if (!temp.compare("ATTRACTOR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);

					if (strcasecmp(temp, "OFF"))
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("ACCEPT"))
				{
				}
				else if (!temp.compare("ANCHORBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("ADDXP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("ADDGOLD"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("ATTACHNPCTOPLAYER"))
				{
				}
				else if (!temp.compare("ATTACH"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
					pos = GetNextWord(es, pos, temp); // source action_point
					pos = GetNextWord(es, pos, temp); // target IO
					pos = GetNextWord(es, pos, temp); // target action_point
				}
				else unknowncommand = 1;

				break;
			case 'G':

				if (!temp.compare("GOTO"))
				{
					char texx[64];

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						sprintf(tem, "Line %04ld - Error: 'GOTO': No Label specified\n-- %s", currentline, curlinetext);
						errors++;
					}
					else
					{
						sprintf(texx, ">>%s", temp.c_str());
						long ppos = FindLabelPos(es, texx);

						if (ppos == -1)
						{
							sprintf(tem, "Line %04ld - Error: 'GOTO': Label %s NOT FOUND in script\n-- %s", currentline, texx, curlinetext);
							errors++;
						}
					}
				}
				else if (!temp.compare("GOSUB"))
				{
					char texx[64];

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						sprintf(tem, "Line %04ld - Error: 'GOSUB': No Label specified\n-- %s", currentline, curlinetext);
						errors++;
					}
					else
					{
						sprintf(texx, ">>%s", temp.c_str());
						pos = FindLabelPos(es, texx);

						if (pos == -1)
						{
							sprintf(tem, "Line %04ld - Error: 'GOSUB': Label %s NOT FOUND in script\n-- %s", currentline, texx, curlinetext);
							errors++;
						}
					}
				}
				else if (!temp.compare("GMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else
					unknowncommand = 1;

				break;
			case 'R':

				if (!temp.compare("REFUSE"))
				{
				}
				else if (!temp.compare("REVIVE"))
				{
					long tmp = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = tmp;
					}
				}
				else if (!temp.compare("RIDICULOUS"))
				{
				}
				else if (!temp.compare("REPAIR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("RANDOM"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("RETURN"))
					{		}
				else if (!temp.compare("REPLACEME"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("ROTATE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("RUNE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'C':

				if (!temp.compare("CINE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("COLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("CAMERACONTROL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("CONVERSATION"))
				{
					pos = GetNextWord(es, pos, temp);
					long nb_people = 0;

					if (temp[0] == '-')
					{
						if (CharIn(temp, '0')) nb_people = 0;

						if (CharIn(temp, '1')) nb_people = 1;

						if (CharIn(temp, '2')) nb_people = 2;

						if (CharIn(temp, '3')) nb_people = 3;

						if (CharIn(temp, '4')) nb_people = 4;

						if (CharIn(temp, '5')) nb_people = 5;

						if (CharIn(temp, '6')) nb_people = 6;

						if (CharIn(temp, '7')) nb_people = 7;

						if (CharIn(temp, '8')) nb_people = 8;

						if (CharIn(temp, '9')) nb_people = 9;

						pos = GetNextWord(es, pos, temp);
					}

					if (nb_people)
					{
						for (long j = 0; j < nb_people; j++)
						{
							pos = GetNextWord(es, pos, temp);
						}
					}
				}
				else if (!temp.compare("CAMERAACTIVATE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("CAMERASMOOTHING"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("CINEMASCOPE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("CAMERAFOCAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("CAMERATRANSLATETARGET"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (temp.compare("CLOSE_STEAL_BAG"))   // temp != "CLOSE_STEAL_BAG"
				{
					unknowncommand = 1;
				}

				break;
			case 'Q':

				if (!temp.compare("QUAKE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("QUEST"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'N':

				if (!temp.compare("NOP"))
				{
				}
				else if (!temp.compare("NOTE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'S':

				if (!temp.compare("SPELLCAST"))
				{
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp1, 'K'))
						{
							pos = GetNextWord(es, pos, temp); //spell id
							goto suite;
						}

						if (iCharIn(temp1, 'D'))
							pos = GetNextWord(es, pos, temp); // duration

						pos = GetNextWord(es, pos, temp); // level
					}

					pos = GetNextWord(es, pos, temp); //spell id
					pos = GetNextWord(es, pos, temp); //spell target
				suite:
					;
				}
				else if (!temp.compare("SPEAK")) // speak say_ident actions
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "KILLALL"))
					{
					}
					else if (temp[0] == '-')
					{
						if ((temp2.find("C") != std::string::npos) || (temp2.find("c") != std::string::npos))
						{
							pos = GetNextWord(es, pos, temp2);

							if (!strcasecmp(temp2, "ZOOM"))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!strcasecmp(temp2, "CCCTALKER_L"))
									 || (!strcasecmp(temp2, "CCCTALKER_R")))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!strcasecmp(temp2, "CCCLISTENER_L"))
									 || (!strcasecmp(temp2, "CCCLISTENER_R")))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!strcasecmp(temp2, "SIDE"))
									 || (!strcasecmp(temp2, "SIDE_L"))
									 || (!strcasecmp(temp2, "SIDE_R")))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
						}

						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("SHOPCATEGORY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SHOPMULTIPLY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPOISONOUS"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPLATFORM"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETGORE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETUNIQUE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETBLACKSMITH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETDETECT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETELEVATOR"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETTRAP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSECRET"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSTEAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETLIGHT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETBLOOD"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSPEAKPITCH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSPEED"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETGROUP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("SETNPCSTAT"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETXPVALUE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETNAME"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPLAYERTWEAK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp.c_str(), "SKIN"))
						pos = GetNextWord(es, pos, temp);

					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if ((!temp.compare("SETSTATUS")) || (!temp.compare("SETMAINEVENT")))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETMOVEMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SPAWN"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!temp.compare("NPC")) || (!temp.compare("ITEM")) || (!temp.compare("FIX")))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
						pos = GetNextWord(es, pos, temp); // spawn position.
					}
				}
				else if (!temp.compare("SETOBJECTTYPE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!temp.compare("SETRIGHTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETLEFTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETHUNGER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSHIELD"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETTWOHANDED"))
				{
				}
				else if (!temp.compare("SETINTERACTIVITY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETEQUIP"))
				{
					std::string temp2;
					std::string temp3;
					pos = GetNextWord(es, pos, temp3);

					if (temp3[0] == '-')
					{
						if (!strcasecmp(temp3, "-r")) { }
						else
						{
							pos = GetNextWord(es, pos, temp);
							pos = GetNextWord(es, pos, temp2);
						}
					}
					else
					{
						pos = GetNextWord(es, pos, temp2);

					}
				}
				else if (!temp.compare("SETONEHANDED"))
				{
				}
				else if (!temp.compare("SETWEAPON"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETLIFE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETDURABILITY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPATH"))
				{
					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETTARGET"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					if (!temp.compare("PATH"))
					{
					}
					else if (!temp.compare("PLAYER"))
					{
					}
					else if (!temp.compare("NONE"))
					{
					}
					else if (!temp.compare("NODE"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!temp.compare("OBJECT"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else
					{
						sprintf(tem, "Line %04ld - Error: 'SET_TARGET': param1 '%s' is an invalid parameter\n-- %s", currentline, temp.c_str(), curlinetext);
						errors++;
					}

				}
				else if (!temp.compare("STARTTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("STOPTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SENDEVENT"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R'))
							pos = GetNextWord(es, pos, temp);

						if (iCharIn(temp, 'G'))
							pos = GetNextWord(es, pos, temp);

						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord_Interpreted(io, es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SET"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp2);
				}
				else if (!temp.compare("SAY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSTEPMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETARMORMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETWEAPONMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSTRIKESPEECH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETANGULAR"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPLAYERCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETPLAYERCONTROLS"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETWORLDCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSHADOW"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETDETACHABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSTACKABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSHOP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETMAXCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETWEIGHT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETEVENT"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp);
					MakeUpcase(temp2);
				}
				else if (!temp.compare("SETPRICE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETINTERNALNAME"))
				{
					pos = GetNextWord(es, pos, temp);
					sprintf(tem, "Line %04ld - Warning: 'SET_INTERNAL_NAME': Obsolete Command.\n-- %s", currentline, curlinetext);
					warnings++;
				}
				else if (!temp.compare("SHOWGLOBALS"))
				{
				}
				else if (!temp.compare("SHOWLOCALS"))
				{
				}
				else if (!temp.compare("SHOWVARS"))
				{
				}
				else if (!temp.compare("SETIRCOLOR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETSCALE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SPECIALFX"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "PLAYER_APPEARS"))
					{
					}
					else if (!temp.compare("HEAL"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!temp.compare("MANA"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!temp.compare("NEWSPELL"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!temp.compare("TORCH"))
					{
					}
					else if (!temp.compare("FIERY"))
					{
					}
					else if (!temp.compare("FIERYOFF"))
					{
					}
					else
					{
						sprintf(tem, "Line %04ld - Error: 'SPECIAL_FX': param1 '%s' is an invalid parameter.\n-- %s", currentline, temp1.c_str(), curlinetext);
						errors++;
					}
				}
				else if (!temp.compare("SETBUMP"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!temp.compare("ON")) || (!temp.compare("OFF")))
					{
					}
					else pos = GetNextWord(es, pos, temp);
				}
				else if (!temp.compare("SETZMAP"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!temp.compare("ON")) || (!temp.compare("OFF")))
					{
					}
					else pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'Z':

				if (!temp.compare("ZONEPARAM"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcasecmp(temp, "STACK"))
					{
					}
					else if (!strcasecmp(temp, "UNSTACK"))
					{
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
						}

						if (!strcasecmp(temp, "RGB"))
						{
							pos = GetNextWord(es, pos, temp);
							pos = GetNextWord(es, pos, temp);
							pos = GetNextWord(es, pos, temp);
						}
						else if (!strcasecmp(temp, "ZCLIP"))
						{
							pos = GetNextWord(es, pos, temp);
						}
						else if (!strcasecmp(temp, "AMBIANCE"))
						{
							pos = GetNextWord(es, pos, temp);
						}

					}
				}
				else unknowncommand = 1;

				break;
			case 'K':

				if (!strcmp(temp, "KILLME"))
				{
				}
				else if (!strcmp(temp, "KEYRINGADD"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else unknowncommand = 1;

				break;
			case 'F':

				if (!strcmp(temp, "FORCEANIM"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "FORCEANGLE"))
				{
					pos = GetNextWord(es, pos, temp2);
				}

				else if (!strcmp(temp, "FORCEDEATH"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else unknowncommand = 1;

				break;
			case 'P':

				if (!strcmp(temp, "PLAYERLOOKAT"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAYERSTACKSIZE"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PRECAST"))
				{
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'D'))
						{
							pos = GetNextWord(es, pos, temp2); // duration
						}

						pos = GetNextWord(es, pos, temp); // level
					}

					pos = GetNextWord(es, pos, temp); //spell id
				}
				else if (!strcmp(temp, "POISON"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAYERMANADRAIN"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PATHFIND"))
				{
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAYANIM"))
				{
					pos = GetNextWord(es, pos, temp2);

					if (temp2[0] == '-')
						pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAYERINTERFACE"))
				{
					pos = GetNextWord(es, pos, temp2);

					if (temp2[0] == '-')
						pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAY"))
				{
					pos = GetNextWord(es, pos, temp2);

					if (temp2[0] == '-') pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "PLAYSPEECH"))
				{
					pos = GetNextWord(es, pos, temp2);

				}
				else if (!strcmp(temp, "POPUP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "PHYSICAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!strcmp(temp, "ON")) || (!strcmp(temp, "OFF")))
					{
					}
					else pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'L':

				if (!strcmp(temp, "LOADANIM"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "LINKOBJTOME"))
				{
					pos = GetNextWord_Interpreted(io, es, pos, temp);
					pos = GetNextWord(es, pos, temp1);
				}
				else unknowncommand = 1;

				break;
			case 'I':

				if (!strcmp(temp, "IF"))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);


					MakeUpcase(temp2);

					if	(!strcmp(temp2, "==")) {}
					else if (!strcmp(temp2, "!="))	{}
					else if (!strcmp(temp2, "<="))	{}
					else if (!strcmp(temp2, "<"))	{}
					else if (!strcmp(temp2, ">="))	{}
					else if (!strcmp(temp2, ">"))	{}
					else if	(!strcasecmp(temp2, "isclass"))	{}
					else if	(!strcasecmp(temp2, "isgroup"))	{}
					else if	(!strcasecmp(temp2, "!isgroup"))	{}
					else if	(!strcasecmp(temp2, "iselement"))	{}
					else if	(!strcasecmp(temp2, "isin"))	{}
					else if	(!strcasecmp(temp2, "istype"))	{}
					else
					{
						sprintf(tem, "Line %04ld - Error: 'IF': Unknown Operator %s found.\n-- %s", currentline, temp2.c_str(), curlinetext);
						errors++;
					}
				}
				else if (!strcmp(temp, "INC"))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "IFEXISTINTERNAL"))
				{
					pos = GetNextWord(es, pos, temp);
					sprintf(tem, "Line %04ld - Warning: 'IF_EXIST_INTERNAL': Obsolete Command.\n-- %s", currentline, curlinetext);
					warnings++;
				}
				else if (!strcmp(temp, "IFVISIBLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "INVERTEDOBJECT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "INVULNERABILITY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "INVENTORY"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeStandard(temp);

					if (!strcmp(temp, "CREATE"))
					{
					}
					else if (!strcmp(temp, "SKIN"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "PLAYERADDFROMSCENE"))
					{
						pos = GetNextWord(es, pos, temp2);
					}
					else if (!strcmp(temp, "ADDFROMSCENE"))
					{
						pos = GetNextWord(es, pos, temp2);
					}
					else if ((!strcmp(temp, "PLAYERADD")) || (!strcmp(temp, "PLAYERADDMULTI")))
					{
						pos = GetNextWord(es, pos, temp2);

						if (!strcmp(temp, "PLAYERADDMULTI"))
						{
							pos = GetNextWord(es, pos, temp2);
						}
					}
					else if ((!strcmp(temp, "ADD")) || (!strcmp(temp, "ADDMULTI")))
					{
						pos = GetNextWord(es, pos, temp2);

						if (!strcmp(temp, "ADDMULTI"))
						{
							pos = GetNextWord(es, pos, temp2);
						}
					}
					else if (!strcmp(temp, "DESTROY"))
					{
					}
					else if (!strcmp(temp, "OPEN"))
					{
					}
					else if (!strcmp(temp, "CLOSE"))
					{
					}
				}
				else unknowncommand = 1;

				break;
			case 'H':

				if (!strcmp(temp, "HEROSAY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else unknowncommand = 1;

				break;
			case 'T':

				if (!strcmp(temp, "TELEPORT"))
				{

					pos = GetNextWord(es, pos, temp);

					if (0 != strcasecmp(temp, "behind"))
					{
						std::string temp2;

						if (temp[0] == '-')
						{
							if (iCharIn(temp, 'A'))
							{
								pos = GetNextWord(es, pos, temp2);
							}

							if (iCharIn(temp, 'L'))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}

							if (iCharIn(temp, 'P'))
							{
								pos = GetNextWord(es, pos, temp2);
							}
						}

					}
				}
				else if (!strcmp(temp, "TARGETPLAYERPOS"))
				{
					sprintf(tem, "Line %04ld - Warning: 'TARGET_PLAYER_POS': Obsolete Command Please Use SET_TARGET PLAYER.\n-- %s", currentline, curlinetext);
					warnings++;
				}
				else if (!strcmp(temp, "TWEAK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!strcmp(temp, "HEAD")) {}
					else if (!strcmp(temp, "TORSO")) {}
					else if (!strcmp(temp, "LEGS")) {}
					else if (!strcmp(temp, "ALL")) {}
					else if (!strcmp(temp, "UPPER")) {}
					else if (!strcmp(temp, "LOWER")) {}
					else if (!strcmp(temp, "UP_LO")) {}

					if (!strcmp(temp, "REMOVE"))
					{
					}
					else if (!strcmp(temp, "SKIN"))
					{
						pos = GetNextWord(es, pos, temp);
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "ICON"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else
					{
						sprintf(tem, "Line %04ld - Error: 'TWEAK %s': Unknown parameter %s found.\n-- %s", currentline, temp.c_str(), temp.c_str(), curlinetext);
						errors++;
					}
				}
				else if ((temp[1] == 'I') && (temp[2] == 'M') && (temp[3] == 'E') && (temp[4] == 'R'))
				{
					// Timer -m nbtimes duration commands
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp2);

					if (!strcmp(temp2, "KILL_LOCAL"))
					{

					}
					else
					{
						if (!strcmp(temp2, "OFF"))
						{
						}
						else
						{
							if (temp2[0] == '-')
							{
								pos = GetNextWord(es, pos, temp2);
							}

							pos = GetNextWord(es, pos, temp3);
						}
					}
				}
				else unknowncommand = 1;

				break;
			case 'V':

				if (!strcmp(temp, "VIEWBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'W':

				if (!strcmp(temp, "WORLDFADE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp1);

					if (!strcasecmp(temp, "OUT"))
					{
						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);
					}
				}
				else if (!strcmp(temp, "WEAPON"))
				{
					pos = GetNextWord(es, pos, temp);

				}
				else unknowncommand = 1;

				break;
			case 'U':

				if (!strcmp(temp, "UNSET"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "USEMESH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "USEPATH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "UNSETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;

			case 'E':

				if (!strcmp(temp, "ELSE"))
				{
				}
				else if (!strcmp(temp, "ENDINTRO"))
				{
				}
				else if (!strcmp(temp, "ENDGAME"))
				{
				}
				else if (!strcmp(temp, "EATME"))
				{
				}
				else if (!strcmp(temp, "EQUIP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else unknowncommand = 1;

				break;
			case 'M':

				if (!strcmp(temp, "MUL"))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "MAPMARKER"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!strcasecmp(temp, "remove")) || (!strcasecmp(temp, "-r")))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else
					{
						pos = GetNextWord(es, pos, temp);
						pos = GetNextWord(es, pos, temp);
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "MOVE"))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);
				}
				else if (!strcmp(temp, "MAGIC"))
				{
					pos = GetNextWord(es, pos, temp1);
				}
				else unknowncommand = 1;

				break;
			case '-':
			case '+':

				if ((!strcmp(temp, "++")) ||
						(!strcmp(temp, "--")))
				{
					pos = GetNextWord(es, pos, temp1);
				}
				else unknowncommand = 1;

				break;
			case 'D':

				if (
					(!strcmp(temp, "DEC")) ||
					(!strcmp(temp, "DIV")))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "DESTROY"))
				{
					pos = GetNextWord(es, pos, temp1);
				}
				else if (!strcmp(temp, "DETACHNPCFROMPLAYER"))
				{
				}
				else if (!strcmp(temp, "DODAMAGE"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "DAMAGER"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "DETACH"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
					pos = GetNextWord(es, pos, temp); // target IO
				}
				else if (!strcmp(temp, "DRAWSYMBOL"))
				{
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);
				}
				else unknowncommand = 1;

				break;
			default:
				unknowncommand = 1;

		}

		if (unknowncommand)
		{
			sprintf(tem, "Line %04ld - Error: Unknown Command '%s'\n-- %s", currentline, temp.c_str(), curlinetext);
			errors++;
		}

		if (strlen(tem) + strlen(errstring) < 65480) strcat(errstring, tem);
		else stoppingdebug = 1;
	}

	if (stoppingdebug) strcat(errstring, "\nToo Many Errors. Stopping Syntax Check...");


	if (errstring[0] == 0) returnvalue = 1;
	else returnvalue = 0;

	if ((errors > 0) || ((warnings > 0) && (SHOWWARNINGS)))
	{
		char title[512];

		if (es == &io->over_script)
		{
			temp = GetName(io->filename);
			sprintf(title, "%s_%04ld", temp.c_str(), io->ident);
			strcat(title, " LOCAL SCRIPT.");
		}
		else
		{
			temp = GetName(io->filename);
			sprintf(title, "%s_%04ld", temp.c_str(), io->ident);
			strcat(title, " CLASS SCRIPT.");
		}

		LastErrorPopup = ShowErrorPopup(title, errstring);
	}
	else LastErrorPopup = NULL;

	LogDebug << "Tem" << tem;
	return returnvalue;
}
HWND LastErrorPopupNO1 = NULL;
HWND LastErrorPopupNO2 = NULL;

extern HWND CDP_IOOptions;
extern INTERACTIVE_OBJ * CDP_EditIO;
bool CheckScriptSyntax_Loading(INTERACTIVE_OBJ * io)
{
	return true;

	if (CheckScriptSyntax(io) != true)
		if (!CDP_IOOptions)
		{
			CDP_EditIO = io;

			ARX_TIME_Pause();
			danaeApp.Pause(true);
			DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
					  MAKEINTRESOURCE(IDD_SCRIPTDIALOG), danaeApp.m_hWnd, IOOptionsProc);
			danaeApp.Pause(false);
			ARX_TIME_UnPause();
			LastErrorPopupNO1 = NULL;
			LastErrorPopupNO2 = NULL;
		}

	return true;
}
bool CheckScriptSyntax(INTERACTIVE_OBJ * io)
{
	if (SYNTAXCHECKING == 0) return true;

	long s1 = LaunchScriptCheck(&io->script, io);
	LastErrorPopupNO1 = LastErrorPopup;
	long s2 = LaunchScriptCheck(&io->over_script, io);
	LastErrorPopupNO2 = LastErrorPopup;

	if (s1 + s2 < 2) return false;

	return true; // no errors.
}
long Event_Total_Count = 0;

void ARX_SCRIPT_Init_Event_Stats()
{
	Event_Total_Count = 0;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			inter.iobj[i]->stat_count = 0;
			inter.iobj[i]->stat_sent = 0;
		}
	}
}
//*********************************************************************************************
//*********************************************************************************************
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

void ARX_IOGROUP_Add(INTERACTIVE_OBJ * io, const char * group)
{
	if (group == NULL) return;

	if (group[0] == 0) return;

	if (IsIOGroup(io, group)) return;

	io->iogroups = (IO_GROUP_DATA *)realloc(io->iogroups, sizeof(IO_GROUP_DATA) * (io->nb_iogroups + 1));
	strcpy(io->iogroups[io->nb_iogroups].name, group);
	io->nb_iogroups++;
}
//*********************************************************************************************
//*********************************************************************************************
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
long NEED_DEBUG = 0;
long BIG_DEBUG_POS = 0;

void ManageCasseDArme(INTERACTIVE_OBJ * io)
{
	if ((io->type_flags & OBJECT_TYPE_DAGGER) ||
			(io->type_flags & OBJECT_TYPE_1H) ||
			(io->type_flags & OBJECT_TYPE_2H) ||
			(io->type_flags & OBJECT_TYPE_BOW))
	{
		if (player.bag)
		{
			INTERACTIVE_OBJ * pObjMin = NULL;
			INTERACTIVE_OBJ * pObjMax = NULL;
			INTERACTIVE_OBJ * pObjFIX = NULL;
			bool bStop = false;

			for (int iNbBag = 0; iNbBag < player.bag; iNbBag++)
			{
				for (int j = 0; j < INVENTORY_Y; j++)
				{
					for (int i = 0; i < INVENTORY_X; i++)
					{
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


INTERACTIVE_OBJ * IO_DEBUG = NULL;
//*************************************************************************************
//*************************************************************************************


bool InSubStack(EERIE_SCRIPT * es, long pos)
{
	for (long i = 0; i < MAX_GOSUB; i++)
	{
		if (es->sub[i] == -1)
		{
			es->sub[i] = pos;
			return true;
		}
	}

	return false;
}
void ClearSubStack(EERIE_SCRIPT * es)
{
	for (long i = 0; i < MAX_GOSUB; i++)
		es->sub[i] = -1;
}
long GetSubStack(EERIE_SCRIPT * es)
{
	long ret;

	for (long i = MAX_GOSUB - 1; i >= 0; i--)
	{
		if (es->sub[i] != -1)
		{
			ret = es->sub[i];
			es->sub[i] = -1;
			return ret;
		}
	}

	return -1;
}

void InitScript(EERIE_SCRIPT * es)
{
	for (long i = 0; i < MAX_GOSUB; i++)
	{
		es->sub[i] = -1;
	}

	es->allowevents = 0;
	es->nblvar = 0;

	if (es->lvar)
	{
		free(es->lvar);
		es->lvar = NULL;
	}

	es->master = NULL;

	for (long j = 0; j < MAX_SCRIPTTIMERS; j++)
		es->timers[j] = 0;



	ARX_SCRIPT_ComputeShortcuts(*es);
}

LRESULT CALLBACK ShowTextDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;

	switch (message)
	{
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case WM_INITDIALOG:
			SendMessage(hDlg, WM_SIZE, 0, 0);
			SetWindowText(hDlg, ShowTextWindowtext.c_str());
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
			SendMessage(thWnd, WM_SETFONT, (WPARAM) GetStockObject(ANSI_FIXED_FONT), true);
			SetWindowText(thWnd, ShowText.c_str());

			return true;
		case WM_SIZE:
			break;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, LOWORD(wParam));
					break;
			}

			break;
	}

	return false;
}
LRESULT CALLBACK ShowVarsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;

	switch (message)
	{
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		case WM_INITDIALOG:
			SendMessage(hDlg, WM_SIZE, 0, 0);
			SetWindowText(hDlg, ShowTextWindowtext.c_str());
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
			SetWindowText(thWnd, ShowText.c_str());
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT2);
			SetWindowText(thWnd, ShowText2.c_str());
			return true;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, LOWORD(wParam));
					break;
			}

			break;
	}

	return false;
}

void ARX_SCRIPT_SetVar(INTERACTIVE_OBJ * io, const std::string& name, const std::string& content)
{
	EERIE_SCRIPT * esss = NULL;
	SCRIPT_VAR * sv = NULL;

	if (io) esss = &io->script;

	long ival;
	float fval;

	switch (name[0])
	{
		case '$': // GLOBAL TEXT

			if (io) return;

			sv = SETVarValueText(svar, NB_GLOBALS, name, content);

			if (sv != NULL)
				sv->type = TYPE_G_TEXT;

			break;
		case '\xA3': // LOCAL TEXT

			if (io == NULL) return;

			sv = SETVarValueText(esss->lvar, esss->nblvar, name, content);

			if (sv != NULL)
				sv->type = TYPE_L_TEXT;

			break;
		case '#': // GLOBAL LONG

			if (io) return;

			ival = atoi(content.c_str());
			sv = SETVarValueLong(svar, NB_GLOBALS, name, ival);

			if (sv != NULL)
				sv->type = TYPE_G_LONG;

			break;
		case '\xA7': // LOCAL LONG

			if (io == NULL) return;

			ival = atoi(content.c_str());
			sv = SETVarValueLong(esss->lvar, esss->nblvar, name, ival);

			if (sv != NULL)
				sv->type = TYPE_L_LONG;

			break;
		case '&': // GLOBAL float

			if (io) return;

			fval = (float)atof(content.c_str());
			sv = SETVarValueFloat(svar, NB_GLOBALS, name, fval);

			if (sv != NULL)
				sv->type = TYPE_G_FLOAT;

			break;
		case '@': // LOCAL float

			if (io == NULL) return;

			fval = (float)atof(content.c_str());
			sv = SETVarValueFloat(esss->lvar, esss->nblvar, name, fval);

			if (sv != NULL)
				sv->type = TYPE_L_FLOAT;

			break;
	}
}
