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

#include <stdlib.h>

#include "ARX_Script.h"


#include <HERMESMain.h>

#include <EERIEObject.h>
#include <EERIELinkedObj.h>
#include <EERIEPathfinder.h>
#include <EERIECollisionSpheres.h>

#include <ARX_C_Cinematique.h>
#include <ARX_Collisions.h>
#include <ARX_Damages.h>
#include <ARX_Equipment.h>
#include <ARX_GlobalMods.h>
#include <ARX_Interactive.h>
#include <ARX_Minimap.h>
#include <ARX_Missile.h>
#include <ARX_NPC.h>
#include <ARX_player.h>
#include <ARX_Particles.h>
#include <ARX_Paths.h>
#include <ARX_Scene.h>
#include <ARX_Sound.h>
#include <ARX_Special.h>
#include <ARX_Spells.h>
#include <ARX_Speech.h>
#include <ARX_Text.h>
#include <ARX_Time.h>
#include "danaedlg.h"

#include "Danae_resource.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long GLOBAL_MAGIC_MODE;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern long FINAL_COMMERCIAL_DEMO;

#define MAX_SSEPARAMS 5
extern long SP_DBG;
extern CINEMATIQUE * ControlCinematique;
extern EERIE_3D LASTCAMPOS, LASTCAMANGLE;
extern INTERACTIVE_OBJ * CAMERACONTROLLER;
extern char WILL_LAUNCH_CINE[256];
extern float InventoryDir;
extern long REFUSE_GAME_RETURN;
extern long FINAL_RELEASE;
extern long GAME_EDITOR;
extern long TELEPORT_TO_CONFIRM;
extern long CINE_PRELOAD;
extern long PLAY_LOADED_CINEMATIC;
extern long ARX_CONVERSATION;
extern long ARX_DEMO;
extern long CHANGE_LEVEL_ICON;
extern long FRAME_COUNT;

extern long lChangeWeapon;
extern INTERACTIVE_OBJ * pIOChangeWeapon;

extern float g_TimeStartCinemascope;

INTERACTIVE_OBJ * LASTSPAWNED = NULL;
INTERACTIVE_OBJ * EVENT_SENDER = NULL;
SCRIPT_VAR * svar = NULL;
char ShowTextWindowtext[128];
char ShowText[65536];
char ShowText2[65536];
char var_text[256];
char SSEPARAMS[MAX_SSEPARAMS][64];
long FORBID_SCRIPT_IO_CREATION = 0;
long RELOADING = 0;
long NB_GLOBALS = 0;
SCR_TIMER * scr_timer = NULL;
long ActiveTimers = 0;

LRESULT CALLBACK ShowTextDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ShowVarsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void ResetAllGlobalVars();

extern void ARX_MENU_Clicked_CREDITS();
extern void ARX_MENU_Launch(LPDIRECT3DDEVICE7 m_pd3dDevice);
void ARX_INTERFACE_Combat_Mode(long);
bool ARX_EQUIPMENT_IsPlayerEquip(INTERACTIVE_OBJ * _pIO);
//*************************************************************************************
// FindScriptPos																	//
// Looks for string in script, return pos. Search start position can be set using	//
// poss parameter.																	//
//*************************************************************************************
__forceinline long FindScriptPos(EERIE_SCRIPT * es, char * str, long poss)
{

	if (!es->data) return -1;

	char * pdest = strstr(es->data + poss, str);
	long result = pdest - es->data;

	if (result < 0) return -1;

	long len2 = strlen(str);

	if (es->data[result+len2] <= 32) return result;

	return -1;
}
__forceinline long FindScriptPosGOTO(EERIE_SCRIPT * es, char * str, long poss)
{
	if (!es->data) return -1;

	char * pdest = strstr(es->data + poss, str);
	long result;
	long len2 = strlen(str);
again:
	;
	result = pdest - es->data;

	if (result < 0) return -1;

	if (es->data[result+len2] <= 32) return result + len2;

	if (pdest = strstr(es->data + poss + result + len2, str))
		goto again;

	return -1;
}
BOOL CharIn(char * string, char _char)
{
	char * s = string;

	while (*s)
	{
		if ((*s) == _char) return true;

		s++;
	}

	return false;
}
BOOL iCharIn(char * string, char _char)
{
	char * s = string;
	MakeUpcase(string);

	while (*s)
	{
		if ((*s) == _char) return true;

		s++;
	}

	return false;
}
extern long FOR_EXTERNAL_PEOPLE;

//*************************************************************************************
// SCRIPT Precomputed Label Offsets Management
long FindLabelPos(EERIE_SCRIPT * es, char * string)
{
	char texx[128];
	sprintf(texx, ">>%s", string);
	return FindScriptPosGOTO(es, texx, 0);
}

void ComputeACSPos(ARX_CINEMATIC_SPEECH * acs, INTERACTIVE_OBJ * io, long ionum)
{
	if (!acs) return;

	if (io)
	{

		long id = io->obj->fastaccess.view_attach;

		if (id != -1)
		{
			acs->pos1.x = io->obj->vertexlist3[id].v.x;
			acs->pos1.y = io->obj->vertexlist3[id].v.y;
			acs->pos1.z = io->obj->vertexlist3[id].v.z;
		}
		else
		{
			acs->pos1.x = io->pos.x;
			acs->pos1.y = io->pos.y + io->physics.cyl.height;
			acs->pos1.z = io->pos.z;
		}
	}

	if (ValidIONum(ionum))
	{
		INTERACTIVE_OBJ *	ioo =	inter.iobj[ionum];
		long				id	=	ioo->obj->fastaccess.view_attach;

		if (id != -1)
		{
			acs->pos2.x	=	ioo->obj->vertexlist3[id].v.x;
			acs->pos2.y	=	ioo->obj->vertexlist3[id].v.y;
			acs->pos2.z	=	ioo->obj->vertexlist3[id].v.z;
		}
		else
		{
			acs->pos2.x	=	ioo->pos.x;
			acs->pos2.y	=	ioo->pos.y + ioo->physics.cyl.height;
			acs->pos2.z	=	ioo->pos.z;
		}
	}
}

void RemoveNumerics(char * tx)
{
	char dest[512];
	strcpy(dest, tx);
	long			pos			= 0;
	unsigned int	size_dest	= strlen(dest);

	for (unsigned long i = 0 ; i < size_dest ; i++)
	{
		if ((dest[i] < '0')
		        &&	(dest[i] > '9')
		   )
		{
			tx[pos++] = dest[i];
		}
	}

	tx[pos] = 0;
}


long ARX_SCRIPT_SearchTextFromPos(EERIE_SCRIPT * es, char * search, long startpos, char * tline, long * nline)
{
	static long lastline = 0;
	long curline;

	long curpos; // current pos in stream
	long curtpos; // current pos in current line;
	char curtline[4096];


	if (es == NULL) return -1;

	if (es->data == NULL) return -1;

	if (es->size <= 0) return -1;

	if (startpos == 0) curline = lastline = 0;
	else curline = lastline;

	curpos = startpos;

	// Get a line from stream
	while (curpos < es->size)
	{
		memset(curtline, 0, 4096);
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

			if (strstr(curtline, search))
			{
				*nline = curline;
				strcpy(tline, curtline);
				lastline = curline;
				return curpos;
			}
		}
	}

	lastline = 0;
	return -1;
}











void Stack_SendMsgToAllNPC_IO(long msg, char * dat)
{
	for (long i = 0; i < inter.nbmax; i++)
	{
		if ((inter.iobj[i]) && (inter.iobj[i]->ioflags & IO_NPC))
		{
			Stack_SendIOScriptEvent(inter.iobj[i], msg, dat, NULL);
		}
	}
}
long SendMsgToAllIO(long msg, char * dat)
{
	long ret = ACCEPT;

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i])
		{
			if (SendIOScriptEvent(inter.iobj[i], msg, dat, NULL) == REFUSE)
				ret = REFUSE;
		}
	}

	return ret;
}

void ARX_SCRIPT_LaunchScriptSearch(char * search)
{
	strcpy(ShowText, "");
	long foundnb = 0;
	long size = 0;
	char tline[4096];
	char toadd[4096];
	char objname[256];
	long nline;
	INTERACTIVE_OBJ * io = NULL;
	MakeUpcase(search);

	for (long i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			io = inter.iobj[i];

			if (i == 0) strcpy(objname, "PLAYER");
			else sprintf(objname, "%s_%04d", GetName(io->filename), io->ident);

			long pos = 0;

			while (pos != -1)
			{

				pos = ARX_SCRIPT_SearchTextFromPos(&io->script, search, pos, tline, &nline);

				if (pos > 0)
				{
					sprintf(toadd, "%s - GLOBAL - Line %4d : %s\n", objname, nline, tline);

					if (size + strlen(toadd) + 3 < 65535)
					{
						strcat(ShowText, toadd);
						foundnb++;
					}
					else
					{
						strcat(ShowText, "...");
						goto suite;
					}

					size += strlen(toadd);
				}
			}

			pos = 0;

			while (pos != -1)
			{
				pos = ARX_SCRIPT_SearchTextFromPos(&io->over_script, search, pos, tline, &nline);

				if (pos > 0)
				{
					sprintf(toadd, "%s - LOCAL  - Line %4d : %s\n", objname, nline, tline);

					if (size + strlen(toadd) + 3 < 65535)
					{
						strcat(ShowText, toadd);
						foundnb++;
					}
					else
					{
						strcat(ShowText, "...");
						goto suite;
					}

					size += strlen(toadd);
				}
			}
		}
	}

suite:
	;

	if (foundnb <= 0)
	{
		strcpy(ShowText, "No Occurence Found...");
	}

	sprintf(ShowTextWindowtext, "Search Results for %s (%d occurences)", search, foundnb);


	DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXTBIG, danaeApp.m_hWnd, (DLGPROC)ShowTextDlg);
}
void ARX_SCRIPT_SetMainEvent(INTERACTIVE_OBJ * io, char * newevent)
{
	if (io == NULL) return;






	if (!stricmp(newevent, "MAIN"))
		io->mainevent[0] = 0;
	else strcpy(io->mainevent, newevent);
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

			if (flags)	SendScriptEvent(&inter.iobj[num]->script, SM_INIT, "", inter.iobj[num], NULL);


			if (inter.iobj[num])
				ARX_SCRIPT_SetMainEvent(inter.iobj[num], "MAIN");
		}

		// Do the same for Local Script
		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			inter.iobj[num]->over_script.allowevents = 0;

			if (flags)	SendScriptEvent(&inter.iobj[num]->over_script, SM_INIT, "", inter.iobj[num], NULL);


		}

		// Sends InitEnd Event
		if (flags)
		{
			if (inter.iobj[num] && inter.iobj[num]->script.data)
				SendScriptEvent(&inter.iobj[num]->script, SM_INITEND, "", inter.iobj[num], NULL);

			if (inter.iobj[num] && inter.iobj[num]->over_script.data)
				SendScriptEvent(&inter.iobj[num]->over_script, SM_INITEND, "", inter.iobj[num], NULL);
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

		long numm = __min(inter.nbmax, 10);

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
						else SendIOScriptEvent(inter.iobj[i], SM_MAIN, "", NULL);
					}
			}
		}
	}
}
//*************************************************************************************
//*************************************************************************************
BOOL IsElement(char * seek, char * text)
{
	char tex[1024];
	memcpy(tex, text, 1023);
	tex[1023] = 0;
	char * token = strtok(tex, " ");

	while (token != NULL)
	{
		if (!strcmp(token, seek)) return TRUE;

		token = strtok(NULL, " ");
	}

	return FALSE;

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














































void ARX_SCRIPT_ComputeShortcuts(EERIE_SCRIPT * es)
{
	long nb = __min(MAX_SHORTCUT, SM_MAXCMD);

	for (long j = 1; j < nb; j++)
	{
		es->shortcut[j] = FindScriptPos(es, AS_EVENT[j].name, 0);

		if (es->shortcut[j] >= 0)
		{
			char dest[256];
			GetNextWord(es, es->shortcut[j], dest);

			if (!stricmp(dest, "{"))
			{
				GetNextWord(es, es->shortcut[j], dest);

				if (!stricmp(dest, "ACCEPT"))
				{
					es->shortcut[j] = -1;
				}
			}
		}

	}
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
//*************************************************************************************













//*************************************************************************************
// Checks if a string (seek) is at the start of another string (text)
// returns 0 if "seek" is at the start of "text"
// else returns 1
//*************************************************************************************
long specialstrcmp(char * text, char * seek)
{



	long len = strlen(seek);
	long len2 = strlen(text);

	if (len2 < len) return 1;

	for (long i = 0; i < len; i++)
	{
		if (text[i] != seek[i]) return 1;
	}

	return 0;



}
#define TYPE_TEXT	1
#define TYPE_FLOAT	2
#define TYPE_LONG	3
//*************************************************************************************
//*************************************************************************************
long GetSystemVar(EERIE_SCRIPT * es,INTERACTIVE_OBJ * io,char * name,char * txtcontent,unsigned int txtcontentSize,float * fcontent,long * lcontent)
{
	MakeUpcase(name);

	switch (name[1])
	{
		case '$':

			if (!strcmp(name, "^$PARAM1"))
			{
				strcpy(txtcontent, SSEPARAMS[0]);
				return TYPE_TEXT;
			}

			if (!strcmp(name, "^$PARAM2"))
			{
				strcpy(txtcontent, SSEPARAMS[1]);
				return TYPE_TEXT;
			}

			if (!strcmp(name, "^$PARAM3"))
			{
				strcpy(txtcontent, SSEPARAMS[2]);
				return TYPE_TEXT;
			}

			if (!strcmp(name, "^$OBJONTOP"))
			{
				strcpy(txtcontent, "NONE");

				if (io)	MakeTopObjString(io,txtcontent,txtcontentSize);//ARX: xrichter (2010-08-04) - Fix corrupted stack

				return TYPE_TEXT;
			}

			break;
		case '&':

			if (!strcmp(name, "^&PARAM1"))
			{
				*fcontent = (float)atof(SSEPARAMS[0]);
				return TYPE_FLOAT;
			}

			if (!strcmp(name, "^&PARAM2"))
			{
				*fcontent = (float)atof(SSEPARAMS[1]);
				return TYPE_FLOAT;
			}

			if (!strcmp(name, "^&PARAM3"))
			{
				*fcontent = (float)atof(SSEPARAMS[2]);
				return TYPE_FLOAT;
			}

			if (!strcmp(name, "^&PLAYERDIST"))
			{
				if (io)
				{
					*fcontent = (float)EEDistance3D(&player.pos, &io->pos);
					return TYPE_FLOAT;
				}
			}

			break;
		case '#':

			if (!strcmp(name, "^#PLAYERDIST"))
			{
				if (io != NULL)
				{
					F2L(EEDistance3D(&player.pos, &io->pos), lcontent);
					return TYPE_LONG;
				}
			}

			if (!strcmp(name, "^#PARAM1"))
			{
				*lcontent = atol(SSEPARAMS[0]);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^#PARAM2"))
			{
				*lcontent = atol(SSEPARAMS[1]);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^#PARAM3"))
			{
				*lcontent = atol(SSEPARAMS[2]);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^#TIMER1"))
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

			if (!strcmp(name, "^#TIMER2"))
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

			if (!strcmp(name, "^#TIMER3"))
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

			if (!strcmp(name, "^#TIMER4"))
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

			if (!strcmp(name, "^GORE"))
			{
				*lcontent = GORE_MODE;
				return TYPE_LONG;
			}

			if (!strcmp(name, "^GAMEDAYS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 864000000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^GAMEHOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 3600000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^GAMEMINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 60000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^GAMESECONDS"))
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

			if (!strcmp(name, "^ARXDAYS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 7200000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXHOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 600000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXMINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 10000);
				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXSECONDS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 1000);
				*lcontent *= 6;
				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXTIME_HOURS"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 600000);

				while (*lcontent > 12) *lcontent -= 12;

				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXTIME_MINUTES"))
			{
				*lcontent = ARX_CLEAN_WARN_CAST_LONG(ARXTime / 10000);

				while (*lcontent > 60) *lcontent -= 60;

				return TYPE_LONG;
			}

			if (!strcmp(name, "^ARXTIME_SECONDS"))
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

				if (!stricmp(temp, "AAM"))
				{
					*lcontent = player.rune_flags & FLAG_AAM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "CETRIUS"))
				{
					*lcontent = player.rune_flags & FLAG_CETRIUS;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "COMUNICATUM"))
				{
					*lcontent = player.rune_flags & FLAG_COMUNICATUM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "COSUM"))
				{
					*lcontent = player.rune_flags & FLAG_COSUM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "FOLGORA"))
				{
					*lcontent = player.rune_flags & FLAG_FOLGORA;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "FRIDD"))
				{
					*lcontent = player.rune_flags & FLAG_FRIDD;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "KAOM"))
				{
					*lcontent = player.rune_flags & FLAG_KAOM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "MEGA"))
				{
					*lcontent = player.rune_flags & FLAG_MEGA;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "MORTE"))
				{
					*lcontent = player.rune_flags & FLAG_MORTE;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "MOVIS"))
				{
					*lcontent = player.rune_flags & FLAG_MOVIS;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "NHI"))
				{
					*lcontent = player.rune_flags & FLAG_NHI;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "RHAA"))
				{
					*lcontent = player.rune_flags & FLAG_RHAA;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "SPACIUM"))
				{
					*lcontent = player.rune_flags & FLAG_SPACIUM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "STREGUM"))
				{
					*lcontent = player.rune_flags & FLAG_STREGUM;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "TAAR"))
				{
					*lcontent = player.rune_flags & FLAG_TAAR;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "TEMPUS"))
				{
					*lcontent = player.rune_flags & FLAG_TEMPUS;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "TERA"))
				{
					*lcontent = player.rune_flags & FLAG_TERA;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "VISTA"))
				{
					*lcontent = player.rune_flags & FLAG_VISTA;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "VITAE"))
				{
					*lcontent = player.rune_flags & FLAG_VITAE;
					return TYPE_LONG;
				}
				else if (!stricmp(temp, "YOK"))
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
					strcpy(txtcontent, "");

					if (io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
						strcat(txtcontent, "L");

					if (io->_npcdata->behavior & BEHAVIOUR_SNEAK)
						strcat(txtcontent, "S");

					if (io->_npcdata->behavior & BEHAVIOUR_DISTANT)
						strcat(txtcontent, "D");

					if (io->_npcdata->behavior & BEHAVIOUR_MAGIC)
						strcat(txtcontent, "M");

					if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
						strcat(txtcontent, "F");

					if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
						strcat(txtcontent, "H");

					if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
						strcat(txtcontent, "R");

					if (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
						strcat(txtcontent, "T");

					if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
						strcat(txtcontent, "E");

					if (io->_npcdata->behavior & BEHAVIOUR_LOOK_FOR)
						strcat(txtcontent, "O");

					if (io->_npcdata->behavior & BEHAVIOUR_HIDE)
						strcat(txtcontent, "I");

					if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
						strcat(txtcontent, "W");

					if (io->_npcdata->behavior & BEHAVIOUR_GUARD)
						strcat(txtcontent, "U");

					if (io->_npcdata->behavior & BEHAVIOUR_STARE_AT)
						strcat(txtcontent, "A");
				}
				else strcpy(txtcontent, "");

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
						strcpy(txtcontent, "PLAYER");
					}
					else
					{
						char temp[256];
						strcpy(temp, GetName(EVENT_SENDER->filename));
						sprintf(txtcontent, "%s_%04d", temp, EVENT_SENDER->ident);
					}
				}
				else 	strcpy(txtcontent, "NONE");

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
				if (io == inter.iobj[0]) strcpy(txtcontent, "PLAYER");
				else
				{
					char temp[256];
					strcpy(temp, GetName(io->filename));
					sprintf(txtcontent, "%s_%04d", temp, io->ident);
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
					for (long i = 0; i < MAX_SPELLS; i++)
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
					char temp[256];
					strcpy(temp, GetName(LASTSPAWNED->filename));
					sprintf(txtcontent, "%s_%04d", temp, LASTSPAWNED->ident);
				}
				else strcpy(txtcontent, "NONE");

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
					strcpy(txtcontent, "NONE");
					return TYPE_TEXT;
				}

				strcpy(txtcontent, op->name);
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
				for (long i = 0; i < MAX_SPELLS; i++)
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
					for (long i = 0; i < MAX_SPELLS; i++)
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

				if (!stricmp(name, "^PLAYERSPELL_INVISIBILITY"))
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
					strcpy(txtcontent, "PLAYER");
				}
				else if (ioo)
				{
					char temp[256];
					strcpy(temp, GetName(ioo->filename));
					sprintf(txtcontent, "%s_%04d", temp, ioo->ident);
				}
				else 	strcpy(txtcontent, "NONE");

				return TYPE_TEXT;
			}

			break;
		case 'T':

			if (!specialstrcmp(name, "^TARGET"))
			{
				if (io->targetinfo == 0) strcpy(txtcontent, "PLAYER");
				else
				{
					if (!ValidIONum(io->targetinfo))
						strcpy(txtcontent, "NONE");
					else
					{
						char temp[256];
						strcpy(temp, GetName(inter.iobj[io->targetinfo]->filename));
						sprintf(txtcontent, "%s_%04d", temp, inter.iobj[io->targetinfo]->ident);
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
			strcpy(txtcontent, "PLAYER");
		}
		else
		{
			char temp[256];
			strcpy(temp, GetName(io->filename));
			sprintf(txtcontent, "%s_%04d", temp, io->ident);
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










//*************************************************************************************
//*************************************************************************************









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
SCRIPT_VAR * GetFreeVarSlot(SCRIPT_VAR ** _svff, long * _nb)
{

	SCRIPT_VAR * svf = (SCRIPT_VAR *) * _svff;
	*_svff = (SCRIPT_VAR *) realloc(svf, sizeof(SCRIPT_VAR) * ((*_nb) + 1));
	svf = (SCRIPT_VAR *) * _svff;
	memset(&svf[*_nb], 0, sizeof(SCRIPT_VAR));
	(*_nb) ++;
	return &svf[(*_nb)-1];
}

//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR * GetVarAddress(SCRIPT_VAR * svf, long * nb, char * name)
{
	if (!svf)
		return NULL;

	for (long i = 0; i < (*nb); i++)
	{
		if (svf[i].type != 0)
		{
			if (!stricmp(name, svf[i].name))
				return &svf[i];
		}
	}

	return NULL;
}
//*************************************************************************************
//*************************************************************************************
long GetVarNum(SCRIPT_VAR * svf, long * nb, char * name)
{
	if (!svf) return -1;

	for (long i = 0; i < *nb; i++)
	{
		if ((svf[i].type != 0) && (svf[i].name))
		{
			if (!strcmp(name, svf[i].name)) return i;
		}
	}

	return -1;
}
//*************************************************************************************
//*************************************************************************************
BOOL UNSETVar(SCRIPT_VAR * svf, long * nb, char * name)
{
	long i = GetVarNum(svf, nb, name);

	if (i < 0) return FALSE;

	long n = *nb;

	if (svf[i].text)
	{
		free((void *)svf[i].text);
		svf[i].text = NULL;
	}

	if (n - 1 - i > 0)
	{
		memcpy(&svf[i], &svf[i+1], sizeof(SCRIPT_VAR)*(n - i - 1));
	}

	svf = (SCRIPT_VAR *)realloc(svf, sizeof(SCRIPT_VAR) * (n - 1));
	(*nb)--;
	return TRUE;
}
//*************************************************************************************
//*************************************************************************************
long GETVarValueLong(SCRIPT_VAR ** svf, long * nb, char * name)
{
	SCRIPT_VAR * tsv;
	tsv = GetVarAddress(*svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->ival;
}
//*************************************************************************************
//*************************************************************************************
float GETVarValueFloat(SCRIPT_VAR ** svf, long * nb, char * name)
{
	SCRIPT_VAR * tsv;
	tsv = GetVarAddress(*svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->fval;
}
//*************************************************************************************
//*************************************************************************************
char * GETVarValueText(SCRIPT_VAR ** svf, long * nb, char * name)
{
	SCRIPT_VAR * tsv;
	tsv = GetVarAddress(*svf, nb, name);

	if (tsv == NULL) return 0;

	return tsv->text;
}

//*************************************************************************************
//*************************************************************************************
char * GetVarValueInterpretedAsText(char * temp1, EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io)
{
	float t1;
	long l1;

	if (temp1[0] == '^')
	{
		const unsigned int tvSize = 64 ;
		long lv; float fv; char tv[tvSize];

		switch (GetSystemVar(esss,io,temp1,tv,tvSize,&fv,&lv))//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				strcpy(var_text, tv);
				return var_text;
				break;
			case TYPE_LONG:
				sprintf(var_text, "%d", lv);
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
		l1 = GETVarValueLong(&svar, &NB_GLOBALS, temp1);
		sprintf(var_text, "%d", l1);
		return var_text;
	}
	else if (temp1[0] == '§')
	{
		l1 = GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);
		sprintf(var_text, "%d", l1);
		return var_text;
	}
	else if (temp1[0] == '&') t1 = GETVarValueFloat(&svar, &NB_GLOBALS, temp1);
	else if (temp1[0] == '@') t1 = GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);
	else if (temp1[0] == '$')
	{
		char * tempo = GETVarValueText(&svar, &NB_GLOBALS, temp1);

		if (tempo == NULL) strcpy(var_text, "VOID");
		else strcpy(var_text, tempo);

		return var_text;
	}
	else if (temp1[0] == '£')
	{
		char * tempo = GETVarValueText(&esss->lvar, &esss->nblvar, temp1);

		if (tempo == NULL) strcpy(var_text, "VOID");
		else strcpy(var_text, tempo);

		return var_text;
	}
	else
	{
		strcpy(var_text, temp1);
		return var_text;
	}

	sprintf(var_text, "%f", t1);
	return var_text;
}

//*************************************************************************************
//*************************************************************************************
float GetVarValueInterpretedAsFloat(char * temp1, EERIE_SCRIPT * esss, INTERACTIVE_OBJ * io)
{
	if (temp1[0] == '^')
	{
		const unsigned int tvSize = 64 ;
		long lv; float fv; char tv[tvSize];
		switch (GetSystemVar(esss,io,temp1,tv,tvSize,&fv,&lv)) //Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				return (float)atof(tv);
				break;
			case TYPE_LONG:
				return (float)lv;
				break;
				return (fv);
				break;
		}

	}
	else if (temp1[0] == '#')	return (float)GETVarValueLong(&svar, &NB_GLOBALS, temp1);
	else if (temp1[0] == '§') return (float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);
	else if (temp1[0] == '&') return GETVarValueFloat(&svar, &NB_GLOBALS, temp1);
	else if (temp1[0] == '@') return GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);

	return (float)atof(temp1);
}
//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR * SETVarValueLong(SCRIPT_VAR ** svf, long * nb, char * name, long val)
{
	SCRIPT_VAR * tsv = GetVarAddress(*svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		strcpy(tsv->name, name);
	}

	tsv->ival = val;
	return tsv;
}
//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR * SETVarValueFloat(SCRIPT_VAR ** svf, long * nb, char * name, float val)
{
	SCRIPT_VAR * tsv;
	tsv = GetVarAddress(*svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		strcpy(tsv->name, name);
	}

	tsv->fval = val;
	return tsv;
}
//*************************************************************************************
//*************************************************************************************
SCRIPT_VAR * SETVarValueText(SCRIPT_VAR ** svf, long * nb, char * name, char * val)
{
	SCRIPT_VAR * tsv;
	tsv = GetVarAddress(*svf, nb, name);

	if (!tsv)
	{
		tsv = GetFreeVarSlot(svf, nb);

		if (!tsv)
			return NULL;

		tsv->text = NULL;
		strcpy(tsv->name, name);
	}

	if (tsv->text)
	{
		free((void *)tsv->text);
		tsv->text = NULL;
	}

	tsv->ival = strlen(val) + 1;

	if (tsv->ival)
		tsv->text = strdup(val);
	else
		tsv->text = NULL;


	return tsv;
}

SCRIPT_EVENT AS_EVENT[] =
{
	{"ON NULL"} ,
	{"ON INIT"} ,
	{"ON INVENTORYIN"} ,
	{"ON INVENTORYOUT"} ,
	{"ON INVENTORYUSE"} ,
	{"ON SCENEUSE"} ,
	{"ON EQUIPIN"} ,
	{"ON EQUIPOUT"},
	{"ON MAIN"},
	{"ON RESET"},
	{"ON CHAT"},
	{"ON ACTION"},
	{"ON DEAD"},
	{"ON REACHEDTARGET"},
	{"ON FIGHT"},
	{"ON FLEE"},
	{"ON HIT"},
	{"ON DIE"},
	{"ON LOSTTARGET"},
	{"ON TREATIN"},
	{"ON TREATOUT"},
	{"ON MOVE"},
	{"ON DETECTPLAYER"},
	{"ON UNDETECTPLAYER"},
	{"ON COMBINE"},
	{"ON NPC_FOLLOW"},
	{"ON NPC_FIGHT"},
	{"ON NPC_STAY"},
	{"ON INVENTORY2_OPEN"},
	{"ON INVENTORY2_CLOSE"},
	{"ON CUSTOM"},
	{"ON ENTER_ZONE"},
	{"ON LEAVE_ZONE"},
	{"ON INITEND"} ,
	{"ON CLICKED"} ,
	{"ON INSIDEZONE"},
	{"ON CONTROLLEDZONE_INSIDE"},
	{"ON LEAVEZONE"},
	{"ON CONTROLLEDZONE_LEAVE"},
	{"ON ENTERZONE"},
	{"ON CONTROLLEDZONE_ENTER"},
	{"ON LOAD"} ,
	{"ON SPELLCAST"} ,
	{"ON RELOAD"} ,
	{"ON COLLIDE_DOOR"} ,
	{"ON OUCH"} ,
	{"ON HEAR"} ,
	{"ON SUMMONED"} ,
	{"ON SPELLEND"} ,
	{"ON SPELLDECISION"} ,
	{"ON STRIKE"} ,
	{"ON COLLISION_ERROR"} ,
	{"ON WAYPOINT"} ,
	{"ON PATHEND"} ,
	{"ON CRITICAL"} ,
	{"ON COLLIDE_NPC"} ,
	{"ON BACKSTAB"} ,
	{"ON AGGRESSION"} ,
	{"ON COLLISION_ERROR_DETAIL"} ,
	{"ON GAME_READY"} ,
	{"ON CINE_END"} ,
	{"ON KEY_PRESSED"} ,
	{"ON CONTROLS_ON"} ,
	{"ON CONTROLS_OFF"} ,
	{"ON PATHFINDER_FAILURE"} ,
	{"ON PATHFINDER_SUCCESS"} ,
	{"ON TRAP_DISARMED"} ,
	{"ON BOOK_OPEN"} ,
	{"ON BOOK_CLOSE"} ,
	{"ON IDENTIFY"} ,
	{"ON BREAK"} ,
	{"ON STEAL"} ,
	{"ON COLLIDE_FIELD"},
	{"ON CURSORMODE"},
	{"ON EXPLORATIONMODE"},
	{NULL}
};

//*************************************************************************************
//*************************************************************************************
long GetNumAnim(char * name)
{


	char c	= ARX_CLEAN_WARN_CAST_CHAR(toupper(name[0]));


	switch (c)
	{
		case 'W':

			if (!stricmp(name, "WAIT"))		return ANIM_WAIT;

			if (!stricmp(name, "WAIT2"))		return ANIM_WAIT2;

			if (!stricmp(name, "WALK"))		return ANIM_WALK;

			if (!stricmp(name, "WALK1"))		return ANIM_WALK;

			if (!stricmp(name, "WALK2"))		return ANIM_WALK2;

			if (!stricmp(name, "WALK3"))		return ANIM_WALK3;

			if (!stricmp(name, "WALK_BACKWARD"))				return ANIM_WALK_BACKWARD;

			if (!stricmp(name, "WALK_MINISTEP"))			return ANIM_WALK_MINISTEP;

			if (!stricmp(name, "WAIT_SHORT"))			return ANIM_WAIT_SHORT;

			if (!stricmp(name, "WALK_SNEAK"))			return ANIM_WALK_SNEAK;

			break;
		case 'A':

			if (!stricmp(name, "ACTION"))		return ANIM_ACTION;
			else if (!stricmp(name, "ACTION1"))	return ANIM_ACTION;
			else if (!stricmp(name, "ACTION2"))	return ANIM_ACTION2;
			else if (!stricmp(name, "ACTION3"))	return ANIM_ACTION3;
			else if (!stricmp(name, "ACTION4"))	return ANIM_ACTION4;
			else if (!stricmp(name, "ACTION5"))	return ANIM_ACTION5;
			else if (!stricmp(name, "ACTION6"))	return ANIM_ACTION6;
			else if (!stricmp(name, "ACTION7"))	return ANIM_ACTION7;
			else if (!stricmp(name, "ACTION8"))	return ANIM_ACTION8;
			else if (!stricmp(name, "ACTION9"))	return ANIM_ACTION9;
			else if (!stricmp(name, "ACTION10"))	return ANIM_ACTION10;

			break;
		case 'H':

			if (!stricmp(name, "HIT1"))		return ANIM_HIT1;

			if (!stricmp(name, "HIT"))		return ANIM_HIT1;

			if (!stricmp(name, "HOLD_TORCH"))			return ANIM_HOLD_TORCH;

			if (!stricmp(name, "HIT_SHORT"))				return ANIM_HIT_SHORT;

			break;
		case 'S':

			if (!stricmp(name, "STRIKE1"))	return ANIM_STRIKE1;

			if (!stricmp(name, "STRIKE"))	return ANIM_STRIKE1;

			if (!stricmp(name, "SHIELD_START"))			return ANIM_SHIELD_START;

			if (!stricmp(name, "SHIELD_CYCLE"))			return ANIM_SHIELD_CYCLE;

			if (!stricmp(name, "SHIELD_HIT"))			return ANIM_SHIELD_HIT;

			if (!stricmp(name, "SHIELD_END"))			return ANIM_SHIELD_END;

			if (!stricmp(name, "STRAFE_RIGHT"))			return ANIM_STRAFE_RIGHT;

			if (!stricmp(name, "STRAFE_LEFT"))			return ANIM_STRAFE_LEFT;

			if (!stricmp(name, "STRAFE_RUN_LEFT"))		return ANIM_STRAFE_RUN_LEFT;

			if (!stricmp(name, "STRAFE_RUN_RIGHT"))		return ANIM_STRAFE_RUN_RIGHT;

			break;
		case 'D':

			if (!stricmp(name, "DIE"))		return ANIM_DIE;

			if (!stricmp(name, "DAGGER_READY_PART_1"))		return ANIM_DAGGER_READY_PART_1;

			if (!stricmp(name, "DAGGER_READY_PART_2"))		return ANIM_DAGGER_READY_PART_2;

			if (!stricmp(name, "DAGGER_UNREADY_PART_1"))		return ANIM_DAGGER_UNREADY_PART_1;

			if (!stricmp(name, "DAGGER_UNREADY_PART_2"))		return ANIM_DAGGER_UNREADY_PART_2;

			if (!stricmp(name, "DAGGER_WAIT"))				return ANIM_DAGGER_WAIT;

			if (!stricmp(name, "DAGGER_STRIKE_LEFT_START"))	return ANIM_DAGGER_STRIKE_LEFT_START;

			if (!stricmp(name, "DAGGER_STRIKE_LEFT_CYCLE"))	return ANIM_DAGGER_STRIKE_LEFT_CYCLE;

			if (!stricmp(name, "DAGGER_STRIKE_LEFT"))		return ANIM_DAGGER_STRIKE_LEFT;

			if (!stricmp(name, "DAGGER_STRIKE_RIGHT_START"))	return ANIM_DAGGER_STRIKE_RIGHT_START;

			if (!stricmp(name, "DAGGER_STRIKE_RIGHT_CYCLE"))	return ANIM_DAGGER_STRIKE_RIGHT_CYCLE;

			if (!stricmp(name, "DAGGER_STRIKE_RIGHT"))		return ANIM_DAGGER_STRIKE_RIGHT;

			if (!stricmp(name, "DAGGER_STRIKE_TOP_START"))	return ANIM_DAGGER_STRIKE_TOP_START;

			if (!stricmp(name, "DAGGER_STRIKE_TOP_CYCLE"))	return ANIM_DAGGER_STRIKE_TOP_CYCLE;

			if (!stricmp(name, "DAGGER_STRIKE_TOP"))			return ANIM_DAGGER_STRIKE_TOP;

			if (!stricmp(name, "DAGGER_STRIKE_BOTTOM_START")) return ANIM_DAGGER_STRIKE_BOTTOM_START;

			if (!stricmp(name, "DAGGER_STRIKE_BOTTOM_CYCLE")) return ANIM_DAGGER_STRIKE_BOTTOM_CYCLE;

			if (!stricmp(name, "DAGGER_STRIKE_BOTTOM"))		return ANIM_DAGGER_STRIKE_BOTTOM;

			if (!stricmp(name, "DEATH_CRITICAL"))		return ANIM_DEATH_CRITICAL;

			break;
		case 'R':

			if (!stricmp(name, "RUN"))		return ANIM_RUN;

			if (!stricmp(name, "RUN1"))		return ANIM_RUN;

			if (!stricmp(name, "RUN2"))		return ANIM_RUN2;

			if (!stricmp(name, "RUN3"))		return ANIM_RUN3;

			if (!stricmp(name, "RUN_BACKWARD"))		return ANIM_RUN_BACKWARD;

			break;
		case 'T':

			if (!stricmp(name, "TALK_NEUTRAL"))		return ANIM_TALK_NEUTRAL;

			if (!stricmp(name, "TALK_ANGRY"))		return ANIM_TALK_ANGRY;

			if (!stricmp(name, "TALK_HAPPY"))		return ANIM_TALK_HAPPY;

			if (!stricmp(name, "TALK_NEUTRAL_HEAD"))		return ANIM_TALK_NEUTRAL_HEAD;

			if (!stricmp(name, "TALK_ANGRY_HEAD"))		return ANIM_TALK_ANGRY_HEAD;

			if (!stricmp(name, "TALK_HAPPY_HEAD"))		return ANIM_TALK_HAPPY_HEAD;

			break;
		case 'B':

			if (!stricmp(name, "BARE_READY"))				return ANIM_BARE_READY;

			if (!stricmp(name, "BARE_UNREADY"))				return ANIM_BARE_UNREADY;

			if (!stricmp(name, "BARE_WAIT"))					return ANIM_BARE_WAIT;

			if (!stricmp(name, "BARE_STRIKE_LEFT_START"))	return ANIM_BARE_STRIKE_LEFT_START;

			if (!stricmp(name, "BARE_STRIKE_LEFT_CYCLE"))	return ANIM_BARE_STRIKE_LEFT_CYCLE;

			if (!stricmp(name, "BARE_STRIKE_LEFT"))			return ANIM_BARE_STRIKE_LEFT;

			if (!stricmp(name, "BARE_STRIKE_RIGHT_START"))	return ANIM_BARE_STRIKE_RIGHT_START;

			if (!stricmp(name, "BARE_STRIKE_RIGHT_CYCLE"))	return ANIM_BARE_STRIKE_RIGHT_CYCLE;

			if (!stricmp(name, "BARE_STRIKE_RIGHT"))			return ANIM_BARE_STRIKE_RIGHT;

			if (!stricmp(name, "BARE_STRIKE_TOP_START"))		return ANIM_BARE_STRIKE_TOP_START;

			if (!stricmp(name, "BARE_STRIKE_TOP_CYCLE"))		return ANIM_BARE_STRIKE_TOP_CYCLE;

			if (!stricmp(name, "BARE_STRIKE_TOP"))			return ANIM_BARE_STRIKE_TOP;

			if (!stricmp(name, "BARE_STRIKE_BOTTOM_START"))	return ANIM_BARE_STRIKE_BOTTOM_START;

			if (!stricmp(name, "BARE_STRIKE_BOTTOM_CYCLE"))	return ANIM_BARE_STRIKE_BOTTOM_CYCLE;

			if (!stricmp(name, "BARE_STRIKE_BOTTOM"))		return ANIM_BARE_STRIKE_BOTTOM;

			break;
		case '1':

			if (!stricmp(name, "1H_READY_PART_1"))			return ANIM_1H_READY_PART_1;

			if (!stricmp(name, "1H_READY_PART_2"))			return ANIM_1H_READY_PART_2;

			if (!stricmp(name, "1H_UNREADY_PART_1"))			return ANIM_1H_UNREADY_PART_1;

			if (!stricmp(name, "1H_UNREADY_PART_2"))			return ANIM_1H_UNREADY_PART_2;

			if (!stricmp(name, "1H_WAIT"))					return ANIM_1H_WAIT;

			if (!stricmp(name, "1H_STRIKE_LEFT_START"))		return ANIM_1H_STRIKE_LEFT_START;

			if (!stricmp(name, "1H_STRIKE_LEFT_CYCLE"))		return ANIM_1H_STRIKE_LEFT_CYCLE;

			if (!stricmp(name, "1H_STRIKE_LEFT"))			return ANIM_1H_STRIKE_LEFT;

			if (!stricmp(name, "1H_STRIKE_RIGHT_START"))		return ANIM_1H_STRIKE_RIGHT_START;

			if (!stricmp(name, "1H_STRIKE_RIGHT_CYCLE"))		return ANIM_1H_STRIKE_RIGHT_CYCLE;

			if (!stricmp(name, "1H_STRIKE_RIGHT"))			return ANIM_1H_STRIKE_RIGHT;

			if (!stricmp(name, "1H_STRIKE_TOP_START"))		return ANIM_1H_STRIKE_TOP_START;

			if (!stricmp(name, "1H_STRIKE_TOP_CYCLE"))		return ANIM_1H_STRIKE_TOP_CYCLE;

			if (!stricmp(name, "1H_STRIKE_TOP"))				return ANIM_1H_STRIKE_TOP;

			if (!stricmp(name, "1H_STRIKE_BOTTOM_START"))	return ANIM_1H_STRIKE_BOTTOM_START;

			if (!stricmp(name, "1H_STRIKE_BOTTOM_CYCLE"))	return ANIM_1H_STRIKE_BOTTOM_CYCLE;

			if (!stricmp(name, "1H_STRIKE_BOTTOM"))			return ANIM_1H_STRIKE_BOTTOM;

			break;
		case '2':

			if (!stricmp(name, "2H_READY_PART_1"))			return ANIM_2H_READY_PART_1;

			if (!stricmp(name, "2H_READY_PART_2"))			return ANIM_2H_READY_PART_2;

			if (!stricmp(name, "2H_UNREADY_PART_1"))			return ANIM_2H_UNREADY_PART_1;

			if (!stricmp(name, "2H_UNREADY_PART_2"))			return ANIM_2H_UNREADY_PART_2;

			if (!stricmp(name, "2H_WAIT"))					return ANIM_2H_WAIT;

			if (!stricmp(name, "2H_STRIKE_LEFT_START"))		return ANIM_2H_STRIKE_LEFT_START;

			if (!stricmp(name, "2H_STRIKE_LEFT_CYCLE"))		return ANIM_2H_STRIKE_LEFT_CYCLE;

			if (!stricmp(name, "2H_STRIKE_LEFT"))			return ANIM_2H_STRIKE_LEFT;

			if (!stricmp(name, "2H_STRIKE_RIGHT_START"))		return ANIM_2H_STRIKE_RIGHT_START;

			if (!stricmp(name, "2H_STRIKE_RIGHT_CYCLE"))		return ANIM_2H_STRIKE_RIGHT_CYCLE;

			if (!stricmp(name, "2H_STRIKE_RIGHT"))			return ANIM_2H_STRIKE_RIGHT;

			if (!stricmp(name, "2H_STRIKE_TOP_START"))		return ANIM_2H_STRIKE_TOP_START;

			if (!stricmp(name, "2H_STRIKE_TOP_CYCLE"))		return ANIM_2H_STRIKE_TOP_CYCLE;

			if (!stricmp(name, "2H_STRIKE_TOP"))				return ANIM_2H_STRIKE_TOP;

			if (!stricmp(name, "2H_STRIKE_BOTTOM_START"))	return ANIM_2H_STRIKE_BOTTOM_START;

			if (!stricmp(name, "2H_STRIKE_BOTTOM_CYCLE"))	return ANIM_2H_STRIKE_BOTTOM_CYCLE;

			if (!stricmp(name, "2H_STRIKE_BOTTOM"))			return ANIM_2H_STRIKE_BOTTOM;

			break;
		case 'M':

			if (!stricmp(name, "MISSILE_READY_PART_1"))		return ANIM_MISSILE_READY_PART_1;

			if (!stricmp(name, "MISSILE_READY_PART_2"))		return ANIM_MISSILE_READY_PART_2;

			if (!stricmp(name, "MISSILE_UNREADY_PART_1"))	return ANIM_MISSILE_UNREADY_PART_1;

			if (!stricmp(name, "MISSILE_UNREADY_PART_2"))	return ANIM_MISSILE_UNREADY_PART_2;

			if (!stricmp(name, "MISSILE_WAIT"))				return ANIM_MISSILE_WAIT;

			if (!stricmp(name, "MISSILE_STRIKE_PART_1"))		return ANIM_MISSILE_STRIKE_PART_1;

			if (!stricmp(name, "MISSILE_STRIKE_PART_2"))		return ANIM_MISSILE_STRIKE_PART_2;

			if (!stricmp(name, "MISSILE_STRIKE_CYCLE"))		return ANIM_MISSILE_STRIKE_CYCLE;

			if (!stricmp(name, "MISSILE_STRIKE"))			return ANIM_MISSILE_STRIKE;

			if (!stricmp(name, "MEDITATION"))			return ANIM_MEDITATION;

			break;
		case 'C':

			if (!stricmp(name, "CAST_START"))			return ANIM_CAST_START;

			if (!stricmp(name, "CAST_CYCLE"))			return ANIM_CAST_CYCLE;

			if (!stricmp(name, "CAST"))					return ANIM_CAST;

			if (!stricmp(name, "CAST_END"))				return ANIM_CAST_END;

			if (!stricmp(name, "CROUCH"))				return ANIM_CROUCH;

			if (!stricmp(name, "CROUCH_WALK"))			return ANIM_CROUCH_WALK;

			if (!stricmp(name, "CROUCH_WALK_BACKWARD"))	return ANIM_CROUCH_WALK_BACKWARD;

			if (!stricmp(name, "CROUCH_STRAFE_LEFT"))	return ANIM_CROUCH_STRAFE_LEFT;

			if (!stricmp(name, "CROUCH_STRAFE_RIGHT"))	return ANIM_CROUCH_STRAFE_RIGHT;

			if (!stricmp(name, "CROUCH_START"))			return ANIM_CROUCH_START;

			if (!stricmp(name, "CROUCH_WAIT"))			return ANIM_CROUCH_WAIT;

			if (!stricmp(name, "CROUCH_END"))			return ANIM_CROUCH_END;

			break;
		case 'L':

			if (!stricmp(name, "LEAN_RIGHT"))			return ANIM_LEAN_RIGHT;

			if (!stricmp(name, "LEAN_LEFT"))				return ANIM_LEAN_LEFT;

			if (!stricmp(name, "LEVITATE"))				return ANIM_LEVITATE;

			break;
		case 'J':

			if (!stricmp(name, "JUMP"))					return ANIM_JUMP;

			if (!stricmp(name, "JUMP_ANTICIPATION"))		return ANIM_JUMP_ANTICIPATION;

			if (!stricmp(name, "JUMP_UP"))				return ANIM_JUMP_UP;

			if (!stricmp(name, "JUMP_CYCLE"))			return ANIM_JUMP_CYCLE;

			if (!stricmp(name, "JUMP_END"))				return ANIM_JUMP_END;

			if (!stricmp(name, "JUMP_END_PART2"))		return ANIM_JUMP_END_PART2;

			break;
		case 'F':

			if (!stricmp(name, "FIGHT_WALK_FORWARD"))	return ANIM_FIGHT_WALK_FORWARD;

			if (!stricmp(name, "FIGHT_WALK_BACKWARD"))	return ANIM_FIGHT_WALK_BACKWARD;

			if (!stricmp(name, "FIGHT_WALK_MINISTEP"))	return ANIM_FIGHT_WALK_MINISTEP;

			if (!stricmp(name, "FIGHT_STRAFE_RIGHT"))	return ANIM_FIGHT_STRAFE_RIGHT;

			if (!stricmp(name, "FIGHT_STRAFE_LEFT"))		return ANIM_FIGHT_STRAFE_LEFT;

			if (!stricmp(name, "FIGHT_WAIT"))			return ANIM_FIGHT_WAIT;

			break;
		case 'G':

			if (!stricmp(name, "GRUNT"))					return ANIM_GRUNT;

			break;
		case 'U':

			if (!stricmp(name, "U_TURN_LEFT"))			return ANIM_U_TURN_LEFT;

			if (!stricmp(name, "U_TURN_RIGHT"))			return ANIM_U_TURN_RIGHT;

			if (!stricmp(name, "U_TURN_LEFT_FIGHT"))		return ANIM_U_TURN_LEFT_FIGHT;

			if (!stricmp(name, "U_TURN_RIGHT_FIGHT"))	return ANIM_U_TURN_RIGHT_FIGHT;

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

long GetNextWord(EERIE_SCRIPT * es, long i, char * temp, long flags)
{

	// Avoids negative position...
	if (i < 0) return -1;

	// Avoids position superior to script size
	if (i >= es->size) return -1;

	long tildes = 0;	// number of tildes
	long old = i;		// stores start pos in old
	LINEEND = 0;		// Global LINEEND set to 0 (no LINEEND for now)
	long j = 0;
	unsigned char * esdat = (unsigned char *)es->data;

	// First ignores spaces & unused chars
	while ((i < es->size) &&
	        ((esdat[i] <= 32)	|| (esdat[i] == '(') || (esdat[i] == ')'))
	      )
	{
		if (es->data[i] == '\n') LINEEND = 1;

		i++;
	}

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

				temp[j] = esdat[i];
				i++;

				if (i >= es->size) return -1;

				j++;
			}

			temp[j] = 0;
			return i + 1;
		}
		else
		{
			temp[j] = esdat[i];

			if (esdat[i] == '~') tildes++;
		}

		i++;

		if (i >= es->size) return -1;

		j++;
	}

	temp[j] = 0;

	if (i == old) return -1;

	// Now retreives Tilded chars...
	if ((!(flags & 2))
	        &&	(tildes > 0)
	   )
	{
		long _pos = 0;
		long _size = strlen(temp);

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
					char tildedd[256];
					char interp[256];
					char result[512];
					memcpy(tildedd, temp + start, end - start + 1);
					tildedd[end-start+1] = 0;

					if (es->master)
						strcpy(interp, GetVarValueInterpretedAsText(tildedd, (EERIE_SCRIPT *)es->master, _CURIO));
					else
						strcpy(interp, GetVarValueInterpretedAsText(tildedd, es, _CURIO));

					if (start == 1) strcpy(result, "");
					else
					{
						memcpy(result, temp, start - 1);
						result[start-1] = 0;
					}

					strcat(result, interp);

					if (end + 2 < _size)
						strcat(result, temp + end + 2);

					_pos = -1;
					strcpy(temp, result);
					_size = strlen(temp);
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
long GetNextWord_Interpreted(INTERACTIVE_OBJ * io, EERIE_SCRIPT * es, long i, char * temp)
{
	long pos=GetNextWord(es,i,temp);
	if	(temp[0]=='^') {
		const unsigned int tvSize = 64 ;
		long lv; float fv; char tv[tvSize];
		switch (GetSystemVar(es,io,temp,tv,tvSize,&fv,&lv)) //Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
		{
			case TYPE_TEXT:
				strcpy(temp, tv);
				break;
			case TYPE_LONG:
				sprintf(temp, "%d", lv);
				break;
			case TYPE_FLOAT:
				sprintf(temp, "%f", fv);
				break;
		}
	}
	else if	(temp[0] == '#')
	{
		sprintf(temp, "%d", GETVarValueLong(&svar, &NB_GLOBALS, temp));
	}
	else if (temp[0] == '§')
	{
		sprintf(temp, "%d", GETVarValueLong(&es->lvar, &es->nblvar, temp));
	}
	else if (temp[0] == '&')
	{
		sprintf(temp, "%f", GETVarValueFloat(&svar, &NB_GLOBALS, temp));
	}
	else if (temp[0] == '@')
	{
		sprintf(temp, "%f", GETVarValueFloat(&es->lvar, &es->nblvar, temp));
	}
	else if (temp[0] == '$')
	{
		char * tempo = GETVarValueText(&svar, &NB_GLOBALS, temp);

		if (tempo == NULL) temp[0] = 0;
		else strcpy(temp, tempo);
	}
	else if (temp[0] == '£')
	{
		char * tempo = GETVarValueText(&es->lvar, &es->nblvar, temp);

		if (tempo == NULL) temp[0] = 0;
		else strcpy(temp, tempo);
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
	char temp[512];
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

	if (!strcmp(temp, "ELSE"))
	{
		pos = tpos;
	}

	return pos;
}
//*************************************************************************************
//*************************************************************************************
BOOL IsGlobal(char c)
{
	if ((c == '$') || (c == '#') || (c == '&')) return TRUE;

	return FALSE;
}

//*************************************************************************************
//*************************************************************************************
void MakeGlobalText(char * tx)
{
	char texx[256];

	for (long i = 0; i < NB_GLOBALS; i++)
	{
		switch (svar[i].type)
		{
			case TYPE_G_TEXT:
				strcat(tx, svar[i].name);
				strcat(tx, " = ");
				strcat(tx, svar[i].text);
				strcat(tx, "\r\n");
				break;
			case TYPE_G_LONG:
				strcat(tx, svar[i].name);
				strcat(tx, " = ");
				sprintf(texx, "%d", svar[i].ival);
				strcat(tx, texx);
				strcat(tx, "\r\n");
				break;
			case TYPE_G_FLOAT:
				strcat(tx, svar[i].name);
				strcat(tx, " = ");
				sprintf(texx, "%f", svar[i].fval);
				strcat(tx, texx);
				strcat(tx, "\r\n");
				break;
		}
	}
}
//*************************************************************************************
//*************************************************************************************
void MakeLocalText(EERIE_SCRIPT * es, char * tx)
{
	char texx[256];

	if (es->master != NULL) es = (EERIE_SCRIPT *)es->master;

	if (es->lvar == NULL) return;

	for (long i = 0; i < es->nblvar; i++)
	{
		switch (es->lvar[i].type)
		{
			case TYPE_L_TEXT:
				strcat(tx, es->lvar[i].name);
				strcat(tx, " = ");
				strcat(tx, es->lvar[i].text);
				strcat(tx, "\r\n");
				break;
			case TYPE_L_LONG:
				strcat(tx, es->lvar[i].name);
				strcat(tx, " = ");
				sprintf(texx, "%d", es->lvar[i].ival);
				strcat(tx, texx);
				strcat(tx, "\r\n");
				break;
			case TYPE_L_FLOAT:
				strcat(tx, es->lvar[i].name);
				strcat(tx, " = ");
				sprintf(texx, "%f", es->lvar[i].fval);
				strcat(tx, texx);
				strcat(tx, "\r\n");
				break;
		}
	}
}

extern INTERACTIVE_OBJ * LastSelectedIO;
//*************************************************************************************
//*************************************************************************************






//*************************************************************************************
//*************************************************************************************

void SetNextAnim(INTERACTIVE_OBJ * io, ANIM_HANDLE * ea, long layer, long loop, long flags)
{
	if (!ea) return;

	if (!io) return;

	if (IsDeadNPC(io)) return;

	if (!(flags & 1))
		AcquireLastAnim(io);

	FinishAnim(io, io->animlayer[layer].cur_anim);
	ANIM_Set(&io->animlayer[layer], ea);
	io->animlayer[layer].next_anim = NULL;

	if (loop)
		io->animlayer[layer].flags |= EA_LOOP;
	else
		io->animlayer[layer].flags &= ~EA_LOOP;

	io->animlayer[layer].flags |= EA_FORCEPLAY;
}

//*************************************************************************************
//*************************************************************************************
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
//*************************************************************************************
//*************************************************************************************






extern EERIE_BACKGROUND * ACTIVEBKG;
//*************************************************************************************
//*************************************************************************************
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
				io->_camdata->cam.lastinfovalid = FALSE;
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

								for (long k = 0; k < ioo->obj->nbvertex; k += 2)
								{
									dist = EEDistance3D(&pos, &inter.iobj[i]->obj->vertexlist3[k].v);

									if ((dist <= dist_limit)
									        &&	(EEfabs(pos.y - inter.iobj[i]->obj->vertexlist3[k].v.y) < 60.f))
									{
										count++;

										if (dist < mindist) mindist = dist;
									}
								}

								float ratio = ((float)count / ((float)ioo->obj->nbvertex * DIV2));

								if (ioo->ioflags & IO_NPC)
								{

									if (mindist <= dist_limit)
									{
										ARX_EQUIPMENT_ComputeDamages(io, NULL, ioo, ratioaim);
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


//*************************************************************************************
//*************************************************************************************

BOOL HasVisibility(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo)
{
	register float x0, y0, z0;
	register float x1, y1, z1;
	x0 = io->pos.x;
	y0 = io->pos.y;
	z0 = io->pos.z;
	x1 = ioo->pos.x;
	y1 = ioo->pos.y;
	z1 = ioo->pos.z;
	float dist = Distance3D(x0, y0, z0, x1, y1, z1);

	if (dist > 20000) return FALSE;

	float ab = MAKEANGLE(io->angle.b);
	EERIE_3D orgn, dest;

	orgn.x = x0;
	orgn.y = y0 - 90.f;
	orgn.z = z0;
	dest.x = x1;
	dest.y = y1 - 90.f;
	dest.z = z1;
	float aa = GetAngle(orgn.x, orgn.z, dest.x, dest.z);
	aa = MAKEANGLE(RAD2DEG(aa));

	if ((aa < ab + 90.f) && (aa > ab - 90.f))
	{
		//font
		ARX_TEXT_Draw(GDevice, InBookFont, 300, 320, 0, 0, _T("VISIBLE"), D3DRGB(1.f, 0.f, 0.f));
		return TRUE;
	}

	return FALSE;
}
//*************************************************************************************
//*************************************************************************************

//*************************************************************************************
//*************************************************************************************
void ShowScriptError(char * tx, char * cmd)
{
	char text[512];
	sprintf(text, "SCRIPT ERROR\n%s\n\n%s", tx, cmd);
	ShowPopup(text);
}
void MakeStandard(char * str)
{
	long i = 0;
	long pos = 0;

	while (1)
	{
		if (str[i] != '_')
		{

			str[pos++] = ARX_CLEAN_WARN_CAST_CHAR(toupper(str[i]));

		}

		if (str[i] == 0) return;

		i++;
	}
}

//*************************************************************************************
//*************************************************************************************

long MakeLocalised(char * text, _TCHAR * output, long maxsize, long lastspeechflag)
{
	if (text[0] == 0)
	{
		_tcscpy(output, _T("ERROR"));
		return 0;
	}

	_TCHAR __text[256];
	MultiByteToWideChar(CP_ACP, 0, text, -1, __text, 256);
	return HERMES_UNICODE_GetProfileString(__text, _T("string"), _T("error"), output, maxsize, NULL, lastspeechflag);
}

//-----------------------------------------------------------------------------
long ARX_SPEECH_AddLocalised(INTERACTIVE_OBJ * io, char * _lpszText, long duration)
{
	_TCHAR __output[4096];
	memset(__output, 0, 4096);
	_TCHAR __text[256];
	MultiByteToWideChar(CP_ACP, 0, _lpszText, -1, __text, 256);

	HERMES_UNICODE_GetProfileString(
	    __text,
	    _T("string"),
	    _T("Not Found"),
	    __output,
	    4095,
	    NULL,
	    -1
	);
	return (ARX_SPEECH_Add(io, __output, duration));
}

//*************************************************************************************
// SendScriptEvent																	//
// Sends a event to a script.														//
// returns ACCEPT to accept default EVENT processing								//
// returns REFUSE to refuse default EVENT processing								//
//*************************************************************************************
void MakeSSEPARAMS(char * params)
{
	for (long i = 0; i < MAX_SSEPARAMS; i++)
	{
		SSEPARAMS[i][0] = 0;
	}

	if ((params == NULL) || (params[0] == 0)) return;

	char * token;
	long pos = 0;
	token = strtok(params, " ");

	while (token != NULL)
	{
		if (pos < MAX_SSEPARAMS) strcpy(SSEPARAMS[pos], token);

		pos++;
		token = strtok(NULL, " ");
	}
}

long GLOB = 0;

//*************************************************************************************
//*************************************************************************************
long NotifyIOEvent(INTERACTIVE_OBJ * io, long msg, char * params)
{
	if (SendIOScriptEvent(io, msg, "") != REFUSE)
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
typedef struct
{
	INTERACTIVE_OBJ *	sender;
	long				exist;
	INTERACTIVE_OBJ *	io;
	long				msg;
	char 		*		params;
	char 		*		eventname;
} STACKED_EVENT;
STACKED_EVENT eventstack[MAX_EVENT_STACK];
void ARX_SCRIPT_EventStackInit()
{
	memset(eventstack, 0, sizeof(STACKED_EVENT)*MAX_EVENT_STACK);
}
void ARX_SCRIPT_EventStackClear()
{
	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if (eventstack[i].exist)
		{
			if (eventstack[i].params)
				free(eventstack[i].params);

			if (eventstack[i].eventname)
				free(eventstack[i].eventname);

			eventstack[i].sender = NULL;
			eventstack[i].exist = 0;
			eventstack[i].io = NULL;
			eventstack[i].msg = 0;
			eventstack[i].params = NULL;
			eventstack[i].eventname = NULL;
		}
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
				if (eventstack[i].params)
					free(eventstack[i].params);

				if (eventstack[i].eventname)
					free(eventstack[i].eventname);

				eventstack[i].sender = NULL;
				eventstack[i].exist = 0;
				eventstack[i].io = NULL;
				eventstack[i].msg = 0;
				eventstack[i].params = NULL;
				eventstack[i].eventname = NULL;
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

			if (eventstack[i].params)
				free(eventstack[i].params);

			if (eventstack[i].eventname)
				free(eventstack[i].eventname);

			eventstack[i].sender = NULL;
			eventstack[i].exist = 0;
			eventstack[i].io = NULL;
			eventstack[i].msg = 0;
			eventstack[i].params = NULL;
			eventstack[i].eventname = NULL;
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
void Stack_SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, char * params, char * eventname)
{
	for (long i = 0; i < MAX_EVENT_STACK; i++)
	{
		if (!eventstack[i].exist)
		{
			if (strlen(params) != 0)
			{
				eventstack[i].params = (char *)malloc(strlen(params) + 1);
				strcpy(eventstack[i].params, params);
			}
			else
			{
				eventstack[i].params = NULL;
			}

			if ((eventname) && (strlen(eventname) != 0))
			{
				eventstack[i].eventname = (char *)malloc(strlen(eventname) + 1);
				strcpy(eventstack[i].eventname, eventname);
			}
			else
			{
				eventstack[i].eventname = NULL;
			}

			eventstack[i].sender = EVENT_SENDER;
			eventstack[i].io = io;
			eventstack[i].msg = msg;
			eventstack[i].exist = 1;

			return;
		}
	}
}
long SendIOScriptEventReverse(INTERACTIVE_OBJ * io, long msg, char * params, char * eventname)
{
	// checks invalid IO
	if (!io) return -1;

	long num = GetInterNum(io);

	if (ValidIONum(num))
	{
		// if this IO only has a Local script, send event to it
		if (inter.iobj[num] && !inter.iobj[num]->over_script.data)
		{
			GLOB = 0;
			return SendScriptEvent(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
		}

		// If this IO has a Global script send to Local (if exists)
		// then to local if no overriden by Local
		if (inter.iobj[num] && (SendScriptEvent(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname) != REFUSE))
		{
			GLOB = 0;

			if (inter.iobj[num])
				return (SendScriptEvent(&inter.iobj[num]->over_script, msg, params, inter.iobj[num], eventname));
			else
				return REFUSE;
		}

		GLOB = 0;
	}

	// Refused further processing.
	return REFUSE;
}
long SendIOScriptEvent(INTERACTIVE_OBJ * io, long msg, char * params, char * eventname)
{
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
				SendIOScriptEventReverse(inter.iobj[num], msg, params, eventname);
				EVENT_SENDER = oes;
			}
		}

		// if this IO only has a Local script, send event to it
		if (inter.iobj[num] && !inter.iobj[num]->over_script.data)
		{
			GLOB = 0;
			long ret = SendScriptEvent(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname);
			EVENT_SENDER = oes;
			return ret;
		}

		// If this IO has a Global script send to Local (if exists)
		// then to Global if no overriden by Local
		if (inter.iobj[num] && (SendScriptEvent(&inter.iobj[num]->over_script, msg, params, inter.iobj[num], eventname) != REFUSE))
		{
			EVENT_SENDER = oes;
			GLOB = 0;

			if (inter.iobj[num])
			{
				long ret = (SendScriptEvent(&inter.iobj[num]->script, msg, params, inter.iobj[num], eventname));
				EVENT_SENDER = oes;
				return ret;
			}
			else
				return REFUSE;
		}

		GLOB = 0;
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
			GLOB = 0;
			SendScriptEvent(&inter.iobj[num]->script, SM_INIT, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			GLOB = 0;
			SendScriptEvent(&inter.iobj[num]->over_script, SM_INIT, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num] && inter.iobj[num]->script.data)
		{
			GLOB = 0;
			SendScriptEvent(&inter.iobj[num]->script, SM_INITEND, "", inter.iobj[num], NULL);
		}

		if (inter.iobj[num] && inter.iobj[num]->over_script.data)
		{
			GLOB = 0;
			SendScriptEvent(&inter.iobj[num]->over_script, SM_INITEND, "", inter.iobj[num], NULL);
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
			if (scr_timer[i].name)
			{
				if (!stricmp(scr_timer[i].name, texx))
				{
					return i;
				}
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
		sprintf(texx, "TIMER_%d", i);
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
		if (scr_timer[timer_idx].name)
		{
			free(scr_timer[timer_idx].name);
			scr_timer[timer_idx].name = NULL;
		}

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
			        (!stricmp(timername, scr_timer[i].name)))
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

	if (scr_timer) free(scr_timer);

	//todo free
	scr_timer = (SCR_TIMER *)malloc(sizeof(SCR_TIMER) * MAX_TIMER_SCRIPT);
	memset(scr_timer, 0, sizeof(SCR_TIMER)*MAX_TIMER_SCRIPT);
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
























long ARX_SCRIPT_GetSystemIOScript(INTERACTIVE_OBJ * io, char * name)
{
	if (ActiveTimers)
	{
		for (long i = 0; i < MAX_TIMER_SCRIPT; i++)
		{
			if (scr_timer[i].exist)
			{
				if ((scr_timer[i].io == io) && (!stricmp(scr_timer[i].name, name)))
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
		pos.y = io->pos.y + io->physics.cyl.height * DIV2;
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

		st->msecs = ARX_CAST_LONG(st->msecs * DIV2);


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
						if (!stricmp(st->name, "_R_A_T_"))
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

						SendScriptEvent(es, SM_EXECUTELINE, "", io, NULL, pos);
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
	return 1;

	if (SYNTAXCHECKING == 0) return 1;

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
		if (cb > 0) sprintf(tem, "Global - Warning: Invalid Number of Closing Brackets. %d '}' missed\n", cb);
		else sprintf(tem, "Global - Warning: Invalid Number of Opening Brackets. %d '{' missed\n", -cb);

		if (strlen(tem) + strlen(errstring) < 65480) strcat(errstring, tem);
		else stoppingdebug = 1;

		warnings++;
	}

	char temp[256];
	char temp1[64];
	char temp2[64];
	char temp3[64];

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

				if (!strcmp(temp, "OBJECTHIDE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "ON"))
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

				if (!strcmp(temp, "BOOK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "BEHAVIOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "STACK"))
					{
					}
					else if (!stricmp(temp, "UNSTACK"))
					{
					}
					else if (!stricmp(temp, "UNSTACKALL"))
					{
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
						}

						if (!stricmp(temp, "FLEE"))
							pos = GetNextWord(es, pos, temp);
						else if (!stricmp(temp, "LOOK_FOR"))
							pos = GetNextWord(es, pos, temp);
						else if (!stricmp(temp, "HIDE"))
							pos = GetNextWord(es, pos, temp);
						else if (!stricmp(temp, "WANDER_AROUND"))
							pos = GetNextWord(es, pos, temp);
					}
				}
				else unknowncommand = 1;

				break;
			case 'A':

				if (!strcmp(temp, "ADDBAG"))
				{
				}

				if (!strcmp(temp, "ACTIVATEPHYSICS"))
				{
				}

				if (!strcmp(temp, "AMBIANCE"))
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
				else if (!strcmp(temp, "ATTRACTOR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);

					if (stricmp(temp, "OFF"))
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "ACCEPT"))
				{
				}
				else if (!strcmp(temp, "ANCHORBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "ADDXP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "ADDGOLD"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "ATTACHNPCTOPLAYER"))
				{
				}
				else if (!strcmp(temp, "ATTACH"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
					pos = GetNextWord(es, pos, temp); // source action_point
					pos = GetNextWord(es, pos, temp); // target IO
					pos = GetNextWord(es, pos, temp); // target action_point
				}
				else unknowncommand = 1;

				break;
			case 'G':

				if (!strcmp(temp, "GOTO"))
				{
					char texx[64];

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						sprintf(tem, "Line %04d - Error: 'GOTO': No Label specified\n-- %s", currentline, curlinetext);
						errors++;
					}
					else
					{
						sprintf(texx, ">>%s", temp);
						long ppos = FindLabelPos(es, texx);

						if (ppos == -1)
						{
							sprintf(tem, "Line %04d - Error: 'GOTO': Label %s NOT FOUND in script\n-- %s", currentline, texx, curlinetext);
							errors++;
						}
					}
				}
				else if (!strcmp(temp, "GOSUB"))
				{
					char texx[64];

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						sprintf(tem, "Line %04d - Error: 'GOSUB': No Label specified\n-- %s", currentline, curlinetext);
						errors++;
					}
					else
					{
						sprintf(texx, ">>%s", temp);
						pos = FindLabelPos(es, texx);

						if (pos == -1)
						{
							sprintf(tem, "Line %04d - Error: 'GOSUB': Label %s NOT FOUND in script\n-- %s", currentline, texx, curlinetext);
							errors++;
						}
					}
				}
				else if (!strcmp(temp, "GMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else
					unknowncommand = 1;

				break;
			case 'R':

				if (!strcmp(temp, "REFUSE"))
				{
				}
				else if (!strcmp(temp, "REVIVE"))
				{
					long tmp = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = tmp;
					}
				}
				else if (!strcmp(temp, "RIDICULOUS"))
				{
				}
				else if (!strcmp(temp, "REPAIR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "RANDOM"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "RETURN"))
					{		}
				else if (!strcmp(temp, "REPLACEME"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "ROTATE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "RUNE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'C':

				if (!strcmp(temp, "CINE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "COLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "CAMERACONTROL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "CONVERSATION"))
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
				else if (!strcmp(temp, "CAMERAACTIVATE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "CAMERASMOOTHING"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "CINEMASCOPE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "CAMERAFOCAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "CAMERATRANSLATETARGET"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (strcmp(temp, "CLOSE_STEAL_BAG"))   // temp != "CLOSE_STEAL_BAG"
				{
					unknowncommand = 1;
				}

				break;
			case 'Q':

				if (!strcmp(temp, "QUAKE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "QUEST"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'N':

				if (!strcmp(temp, "NOP"))
				{
				}
				else if (!strcmp(temp, "NOTE"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'S':

				if (!strcmp(temp, "SPELLCAST"))
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
				else if (!strcmp(temp, "SPEAK")) // speak say_ident actions
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "KILLALL"))
					{
					}
					else if (temp[0] == '-')
					{
						if ((strstr(temp2, "C")) || (strstr(temp2, "c")))
						{
							pos = GetNextWord(es, pos, temp2);

							if (!stricmp(temp2, "ZOOM"))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!stricmp(temp2, "CCCTALKER_L"))
							         || (!stricmp(temp2, "CCCTALKER_R")))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!stricmp(temp2, "CCCLISTENER_L"))
							         || (!stricmp(temp2, "CCCLISTENER_R")))
							{
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
								pos = GetNextWord(es, pos, temp2);
							}
							else if ((!stricmp(temp2, "SIDE"))
							         || (!stricmp(temp2, "SIDE_L"))
							         || (!stricmp(temp2, "SIDE_R")))
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
				else if (!strcmp(temp, "SHOPCATEGORY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SHOPMULTIPLY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPOISONOUS"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPLATFORM"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETGORE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETUNIQUE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETBLACKSMITH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETDETECT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETELEVATOR"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETTRAP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSECRET"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTEAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETLIGHT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETBLOOD"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSPEAKPITCH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSPEED"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETGROUP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "SETNPCSTAT"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETXPVALUE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETNAME"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPLAYERTWEAK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "SKIN"))
						pos = GetNextWord(es, pos, temp);

					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if ((!strcmp(temp, "SETSTATUS")) || (!strcmp(temp, "SETMAINEVENT")))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETMOVEMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SPAWN"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!strcmp(temp, "NPC")) || (!strcmp(temp, "ITEM")) || (!strcmp(temp, "FIX")))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
						pos = GetNextWord(es, pos, temp); // spawn position.
					}
				}
				else if (!strcmp(temp, "SETOBJECTTYPE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}
				}
				else if (!strcmp(temp, "SETRIGHTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETLEFTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETHUNGER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSHIELD"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETTWOHANDED"))
				{
				}
				else if (!strcmp(temp, "SETINTERACTIVITY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETEQUIP"))
				{
					char temp2[64];
					char temp3[64];
					pos = GetNextWord(es, pos, temp3);

					if (temp3[0] == '-')
					{
						if (!stricmp(temp3, "-r")) { }
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
				else if (!strcmp(temp, "SETONEHANDED"))
				{
				}
				else if (!strcmp(temp, "SETWEAPON"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETLIFE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETDURABILITY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
						pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPATH"))
				{
					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETTARGET"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					if (!strcmp(temp, "PATH"))
					{
					}
					else if (!strcmp(temp, "PLAYER"))
					{
					}
					else if (!strcmp(temp, "NONE"))
					{
					}
					else if (!strcmp(temp, "NODE"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "OBJECT"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else
					{
						sprintf(tem, "Line %04d - Error: 'SET_TARGET': param1 '%s' is an invalid parameter\n-- %s", currentline, temp, curlinetext);
						errors++;
					}

				}
				else if (!strcmp(temp, "STARTTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "STOPTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SENDEVENT"))
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
				else if (!strcmp(temp, "SET"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						pos = GetNextWord(es, pos, temp);
					}

					pos = GetNextWord(es, pos, temp2);
				}
				else if (!strcmp(temp, "SAY"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTEPMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETARMORMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETWEAPONMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTRIKESPEECH"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETANGULAR"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPLAYERCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETPLAYERCONTROLS"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETWORLDCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSHADOW"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETDETACHABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTACKABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSHOP"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETMAXCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETWEIGHT"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETEVENT"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp);
					MakeUpcase(temp2);
				}
				else if (!strcmp(temp, "SETPRICE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETINTERNALNAME"))
				{
					pos = GetNextWord(es, pos, temp);
					sprintf(tem, "Line %04d - Warning: 'SET_INTERNAL_NAME': Obsolete Command.\n-- %s", currentline, curlinetext);
					warnings++;
				}
				else if (!strcmp(temp, "SHOWGLOBALS"))
				{
				}
				else if (!strcmp(temp, "SHOWLOCALS"))
				{
				}
				else if (!strcmp(temp, "SHOWVARS"))
				{
				}
				else if (!strcmp(temp, "SETIRCOLOR"))
				{
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSCALE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SPECIALFX"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "PLAYER_APPEARS"))
					{
					}
					else if (!strcmp(temp, "HEAL"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "MANA"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "NEWSPELL"))
					{
						pos = GetNextWord(es, pos, temp);
					}
					else if (!strcmp(temp, "TORCH"))
					{
					}
					else if (!strcmp(temp, "FIERY"))
					{
					}
					else if (!strcmp(temp, "FIERYOFF"))
					{
					}
					else
					{
						sprintf(tem, "Line %04d - Error: 'SPECIAL_FX': param1 '%s' is an invalid parameter.\n-- %s", currentline, temp1, curlinetext);
						errors++;
					}
				}
				else if (!strcmp(temp, "SETBUMP"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!strcmp(temp, "ON")) || (!strcmp(temp, "OFF")))
					{
					}
					else pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETZMAP"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((!strcmp(temp, "ON")) || (!strcmp(temp, "OFF")))
					{
					}
					else pos = GetNextWord(es, pos, temp);
				}
				else unknowncommand = 1;

				break;
			case 'Z':

				if (!strcmp(temp, "ZONEPARAM"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "STACK"))
					{
					}
					else if (!stricmp(temp, "UNSTACK"))
					{
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
						}

						if (!stricmp(temp, "RGB"))
						{
							pos = GetNextWord(es, pos, temp);
							pos = GetNextWord(es, pos, temp);
							pos = GetNextWord(es, pos, temp);
						}
						else if (!stricmp(temp, "ZCLIP"))
						{
							pos = GetNextWord(es, pos, temp);
						}
						else if (!stricmp(temp, "AMBIANCE"))
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
					else if	(!stricmp(temp2, "isclass"))	{}
					else if	(!stricmp(temp2, "isgroup"))	{}
					else if	(!stricmp(temp2, "!isgroup"))	{}
					else if	(!stricmp(temp2, "iselement"))	{}
					else if	(!stricmp(temp2, "isin"))	{}
					else if	(!stricmp(temp2, "istype"))	{}
					else
					{
						sprintf(tem, "Line %04d - Error: 'IF': Unknown Operator %s found.\n-- %s", currentline, temp2, curlinetext);
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
					sprintf(tem, "Line %04d - Warning: 'IF_EXIST_INTERNAL': Obsolete Command.\n-- %s", currentline, curlinetext);
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

					if (0 != stricmp(temp, "behind"))
					{
						char temp2[256];

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
					sprintf(tem, "Line %04d - Warning: 'TARGET_PLAYER_POS': Obsolete Command Please Use SET_TARGET PLAYER.\n-- %s", currentline, curlinetext);
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
						sprintf(tem, "Line %04d - Error: 'TWEAK %s': Unknown parameter %s found.\n-- %s", currentline, temp, temp, curlinetext);
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

					if (!stricmp(temp, "OUT"))
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

					if ((!stricmp(temp, "remove")) || (!stricmp(temp, "-r")))
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
			sprintf(tem, "Line %04d - Error: Unknown Command '%s'\n-- %s", currentline, temp, curlinetext);
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
			strcpy(temp, GetName(io->filename));
			sprintf(title, "%s_%04d", temp, io->ident);
			strcat(title, " LOCAL SCRIPT.");
		}
		else
		{
			strcpy(temp, GetName(io->filename));
			sprintf(title, "%s_%04d", temp, io->ident);
			strcat(title, " CLASS SCRIPT.");
		}

		LastErrorPopup = ShowErrorPopup(title, errstring);
	}
	else LastErrorPopup = NULL;

	return returnvalue;
}
HWND LastErrorPopupNO1 = NULL;
HWND LastErrorPopupNO2 = NULL;

extern HWND CDP_IOOptions;
extern INTERACTIVE_OBJ * CDP_EditIO;
BOOL CheckScriptSyntax_Loading(INTERACTIVE_OBJ * io)
{
	return TRUE;

	if (CheckScriptSyntax(io) != TRUE)
		if (!CDP_IOOptions)
		{
			CDP_EditIO = io;

			ARX_TIME_Pause();
			danaeApp.Pause(TRUE);
			DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
			          MAKEINTRESOURCE(IDD_SCRIPTDIALOG), danaeApp.m_hWnd, IOOptionsProc);
			danaeApp.Pause(FALSE);
			ARX_TIME_UnPause();
			LastErrorPopupNO1 = NULL;
			LastErrorPopupNO2 = NULL;
		}

	return TRUE;
}
BOOL CheckScriptSyntax(INTERACTIVE_OBJ * io)
{
	if (SYNTAXCHECKING == 0) return TRUE;

	long s1 = LaunchScriptCheck(&io->script, io);
	LastErrorPopupNO1 = LastErrorPopup;
	long s2 = LaunchScriptCheck(&io->over_script, io);
	LastErrorPopupNO2 = LastErrorPopup;

	if (s1 + s2 < 2) return FALSE;

	return TRUE; // no errors.
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
BOOL IsIOGroup(INTERACTIVE_OBJ * io, char * group)
{
	for (long i = 0; i < io->nb_iogroups; i++)
	{
		if ((io->iogroups[i].name)
		        &&	(!stricmp(group, io->iogroups[i].name)))
			return TRUE;
	}

	return FALSE;
}

void ARX_IOGROUP_Release(INTERACTIVE_OBJ * io)
{
	if (io->iogroups) free(io->iogroups);

	io->iogroups = NULL;
	io->nb_iogroups = 0;
}
void ARX_IOGROUP_Remove(INTERACTIVE_OBJ * io, char * group)
{
	if (group == NULL) return;

	if (group[0] == 0) return;

	long toremove = -1;

	for (long i = 0; i < io->nb_iogroups; i++)
	{
		if ((io->iogroups[i].name)
		        &&	(!stricmp(group, io->iogroups[i].name)))
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
void ARX_IOGROUP_Add(INTERACTIVE_OBJ * io, char * group)
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
char BIG_DEBUG_STRING[BIG_DEBUG_SIZE];
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
long SendScriptEvent(EERIE_SCRIPT * es, long msg, char * params, INTERACTIVE_OBJ * io, char * evname, long info)
{
	if (io)
	{
		if ((io->GameFlags & GFLAG_MEGAHIDE)
		        &&	(msg != SM_RELOAD))
			return ACCEPT;

		if (io->show == SHOW_FLAG_DESTROYED) // destroyed
			return ACCEPT;
	}



#ifdef NEEDING_DEBUG

	if (DEBUGG) NEED_DEBUG = 1;
	else NEED_DEBUG = 0;

	if (IO_DEBUG == io) NEED_DEBUG |= 2;

#endif
	Event_Total_Count++;

	if (io)
	{
		io->stat_count++;

		if (io->ioflags & IO_FREEZESCRIPT)
		{
			if (msg == SM_LOAD) return ACCEPT;

			return REFUSE;
		}

		if (io->ioflags & IO_NPC)
		{
			if ((io->_npcdata->life <= 0.f) && (msg != SM_DEAD) && (msg != SM_DIE) && (msg != SM_EXECUTELINE) && (msg != SM_RELOAD)
			        && (msg != SM_EXECUTELINE)
			        && (msg != SM_INVENTORY2_OPEN) && (msg != SM_INVENTORY2_CLOSE))
				return ACCEPT;
		}

		//changement d'armes si on casse
		if (((io->ioflags & IO_FIX)	||	(io->ioflags & IO_ITEM))
		        &&	(msg == SM_BREAK))
		{
			ManageCasseDArme(io);
		}
	}

	_CURIO = io;
	LINEEND = 0;

	if (((EDITMODE) || (PauseScript))
	        && (msg != SM_LOAD)
	        && (msg != SM_INIT)
	        && (msg != SM_INITEND))
		return ACCEPT;

	long ret = ACCEPT;
	char temp[256];
	char cmd[256];
	char eventname[64];
	long brackets = 0;
	long pos;

	// Retrieves in esss script pointer to script holding variables.
	EERIE_SCRIPT * esss = (EERIE_SCRIPT *)es->master;

	if (esss == NULL) esss = es;

	// Finds script position to execute code...
	if (evname != NULL)
	{
		strcpy(eventname, "ON ");
		strcat(eventname, evname);
		pos = FindScriptPos(es, eventname, 0);
	}
	else
	{
		if (msg == SM_EXECUTELINE)
			pos = info;
		else
		{
			switch (msg)
			{
				case SM_COLLIDE_NPC:

					if (esss->allowevents & DISABLE_COLLIDE_NPC) return REFUSE;

					break;
				case SM_CHAT:

					if (esss->allowevents & DISABLE_CHAT) return REFUSE;

					break;
				case SM_HIT:

					if (esss->allowevents & DISABLE_HIT) return REFUSE;

					break;
				case SM_INVENTORY2_OPEN:

					if (esss->allowevents & DISABLE_INVENTORY2_OPEN) return REFUSE;

					break;
				case SM_HEAR:

					if (esss->allowevents & DISABLE_HEAR) return REFUSE;

					break;
				case SM_UNDETECTPLAYER:
				case SM_DETECTPLAYER:

					if (esss->allowevents & DISABLE_DETECT) return REFUSE;

					break;
				case SM_AGGRESSION:

					if (esss->allowevents & DISABLE_AGGRESSION) return REFUSE;

					break;
				case SM_MAIN:

					if (esss->allowevents & DISABLE_MAIN) return REFUSE;

					break;
				case SM_CURSORMODE:

					if (esss->allowevents & DISABLE_CURSORMODE) return REFUSE;

					break;
				case SM_EXPLORATIONMODE:

					if (esss->allowevents & DISABLE_EXPLORATIONMODE) return REFUSE;

					break;
				case SM_KEY_PRESSED:
				{
					float dwCurrTime = ARX_TIME_Get();

					if ((dwCurrTime - g_TimeStartCinemascope) < 3000)
					{
						return REFUSE;
					}
				}
				break;
			}

			if (msg < MAX_SHORTCUT)
				pos = es->shortcut[msg];
			else
			{
				if (((msg >= SM_MAXCMD))
				        && (msg != SM_EXECUTELINE) && (!evname))
				{

					return ACCEPT;
				}

				pos = FindScriptPos(es, AS_EVENT[msg].name, 0);
			}
		}
	}

	if (pos <= -1)
	{
		GLOB = 1;
		return ACCEPT;
	}

	GLOB = 0;



	MakeSSEPARAMS(params);


	if (msg != SM_EXECUTELINE)
	{
		if (evname)
		{
			pos += strlen(eventname); // adding 'ON ' length
#ifdef NEEDING_DEBUG

			if (NEED_DEBUG)
			{
				DEBUG_Notify("\r\n");
				sprintf(cmd, "%s received_______________________________________________________________________________", eventname);
				DEBUG_Notify(cmd);
			}

#endif
		}
		else
		{
			pos += strlen(AS_EVENT[msg].name);
#ifdef NEEDING_DEBUG

			if (NEED_DEBUG)
			{
				DEBUG_Notify("\r\n");
				sprintf(cmd, "%s received_________________________________________________________________________________", AS_EVENT[msg].name);
				DEBUG_Notify(cmd);
			}

#endif
		}

		if ((pos = GetNextWord(es, pos, temp)) < 0) return ACCEPT;

		if (temp[0] != '{')
		{
#ifdef NEEDING_DEBUG

			if (NEED_DEBUG)
			{
				DEBUG_Notify("ERROR: No bracket after event");
			}

#endif
			return ACCEPT;
		}
		else brackets = 1;
	}
	else
	{
#ifdef NEEDING_DEBUG

		if (NEED_DEBUG)
		{
			DEBUG_Notify("\r\n");
			sprintf(cmd, "EXECUTELINE received______________________________________________________________________________");
			DEBUG_Notify(cmd);
		}

#endif
		brackets = 0;
	}

	while (pos >= 0)
	{

		cmd[0] = 0;

		if (pos >= es->size - 1) 	return ACCEPT;

		if ((pos = GetNextWord(es, pos, temp)) < 0) 	return ACCEPT;

		if ((msg == SM_EXECUTELINE) && (LINEEND == 1)) return ACCEPT;

		MakeStandard(temp);

		switch (temp[0])
		{
			case '}':
				brackets--;
				break;
			case '{':
				brackets++;
				break;
			case '/':

				if (temp[1] == '/') pos = GotoNextLine(es, pos);

				break;
			case '>':

				if (temp[1] == '>') pos = GotoNextLine(es, pos);

				break;
			case 'B':

				if (!strcmp(temp, "BEHAVIOR"))
				{
					unsigned long behavior = 0; //BEHAVIOUR_NONE;
					float behavior_param = 0.f;
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "BEHAVIOR %s ", temp);

#endif

					if (!stricmp(temp, "STACK"))
					{
						ARX_NPC_Behaviour_Stack(io);
					}
					else if (!stricmp(temp, "UNSTACK"))
					{
						ARX_NPC_Behaviour_UnStack(io);
					}
					else if (!stricmp(temp, "UNSTACKALL"))
					{
						ARX_NPC_Behaviour_Reset(io);
					}
					else
					{
						if (temp[0] == '-')
						{

							if (iCharIn(temp, 'L'))
								behavior |= BEHAVIOUR_LOOK_AROUND;

							if (iCharIn(temp, 'S'))
								behavior |= BEHAVIOUR_SNEAK;

							if (iCharIn(temp, 'D'))
								behavior |= BEHAVIOUR_DISTANT;

							if (iCharIn(temp, 'M'))
								behavior |= BEHAVIOUR_MAGIC;

							if (iCharIn(temp, 'F'))
								behavior |= BEHAVIOUR_FIGHT;

							if (iCharIn(temp, 'A'))
								behavior |= BEHAVIOUR_STARE_AT;

							if (CharIn(temp, '0') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							if (CharIn(temp, '1') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							if (CharIn(temp, '2') && io && (io->ioflags & IO_NPC))
								io->_npcdata->tactics = 0;

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
							}

#endif
						}


						if (!stricmp(temp, "GO_HOME"))
							behavior |= BEHAVIOUR_GO_HOME;
						else if (!stricmp(temp, "FRIENDLY"))
						{
							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = NOMOVEMODE;

							behavior |= BEHAVIOUR_FRIENDLY;
						}
						else if (!stricmp(temp, "MOVE_TO"))
						{
							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;

							behavior |= BEHAVIOUR_MOVE_TO;
						}
						else if (!stricmp(temp, "FLEE"))
						{
							behavior |= BEHAVIOUR_FLEE;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = RUNMODE;
						}
						else if (!stricmp(temp, "LOOK_FOR"))
						{
							behavior |= BEHAVIOUR_LOOK_FOR;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!stricmp(temp, "HIDE"))
						{
							behavior |= BEHAVIOUR_HIDE;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!stricmp(temp, "WANDER_AROUND"))
						{
							behavior |= BEHAVIOUR_WANDER_AROUND;
							pos = GetNextWord(es, pos, temp);
							behavior_param = GetVarValueInterpretedAsFloat(temp, esss, io);

							if ((io) && (io->ioflags & IO_NPC)) io->_npcdata->movemode = WALKMODE;
						}
						else if (!stricmp(temp, "GUARD"))
						{
							behavior |= BEHAVIOUR_GUARD;

							if (io)
							{
								io->targetinfo = -2;

								if (io->ioflags & IO_NPC) io->_npcdata->movemode = NOMOVEMODE;
							}
						}

						if ((io) && (io->ioflags & IO_NPC))
						{

							ARX_CHECK_LONG(behavior_param);
							ARX_NPC_Behaviour_Change(io, behavior, ARX_CLEAN_WARN_CAST_LONG(behavior_param));


						}
					}
				}

				if (!strcmp(temp, "BOOK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) //MAGIC
							Book_Mode = 2;

						if (iCharIn(temp, 'E')) //Equip
							Book_Mode = 1;

						if (iCharIn(temp, 'M')) //Map
							Book_Mode = 3;

						pos = GetNextWord(es, pos, temp);
					}

					if (!stricmp(temp, "OPEN"))
					{
						ARX_INTERFACE_BookOpenClose(1);
					}
					else if (!stricmp(temp, "CLOSE"))
					{
						ARX_INTERFACE_BookOpenClose(2);
					}
				}

				break;
			case 'A':

				if (!strcmp(temp, "ACCEPT"))
				{
					ret = ACCEPT;
					ClearSubStack(es);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						DEBUG_Notify("  ACCEPT");
					}

#endif
					goto end;
				}
				else if (!strcmp(temp, "ADDBAG"))
				{
					ARX_PLAYER_AddBag();
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ADD_BAG ");
						strcat(cmd, temp);
					}

#endif
				}
				else if (!strcmp(temp, "ACTIVATEPHYSICS"))
				{
					ARX_INTERACTIVE_ActivatePhysics(GetInterNum(io));
				}
				else if (!strcmp(temp, "ADDXP"))
				{
					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);
					ARX_PLAYER_Modify_XP((long)val);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ADD_XP ");
						strcat(cmd, temp);
					}

#endif
				}
				else if (!strcmp(temp, "ADDGOLD"))
				{
					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (val != 0) ARX_SOUND_PlayInterface(SND_GOLD);


					ARX_CHECK_LONG(val);
					ARX_PLAYER_AddGold(ARX_CLEAN_WARN_CAST_LONG(val));

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ADD_GOLD ");
						strcat(cmd, temp);
					}

#endif
				}
				else if (!strcmp(temp, "ATTRACTOR"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io);

					pos = GetNextWord(es, pos, temp);
					float val = 0.f;
					float val2 = 0.f;

					if (stricmp(temp, "OFF"))
					{
						val = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						val2 = GetVarValueInterpretedAsFloat(temp, esss, io);
					}

					ARX_SPECIAL_ATTRACTORS_Add(t, val, val2);
				}
				else if (!strcmp(temp, "AMBIANCE"))
				{
					float volume(1.0F);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "AMBIANCE ");
					}

#endif

					pos = GetNextWord(es, pos, temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
						strcat(cmd, " ");
					}

#endif

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'V'))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif
							volume = GetVarValueInterpretedAsFloat(temp, esss, io);

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_PlayScriptAmbiance(temp, ARX_SOUND_PLAY_LOOPED, volume * DIV100);
						}
						else if (iCharIn(temp, 'N'))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_PlayScriptAmbiance(temp, ARX_SOUND_PLAY_ONCE);
						}
						else if (iCharIn(temp, 'M'))
						{
							char temp2[256];

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp2);
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_SetAmbianceTrackStatus(temp, temp2, 1); //1 = Mute

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif
						}
						else if (iCharIn(temp, 'U'))
						{
							char temp2[256];

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp2);
								strcat(cmd, " ");
							}

#endif

							ARX_SOUND_SetAmbianceTrackStatus(temp, temp2, 0);//0 = unmute

							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, temp);
								strcat(cmd, " ");
							}

#endif
						}
					}
					else if (!stricmp(temp, "KILL"))
						ARX_SOUND_KillAmbiances();
					else
						ARX_SOUND_PlayScriptAmbiance(temp);
				}
				else if (!strcmp(temp, "ANCHORBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if ((!stricmp(temp, "ON")) || (!stricmp(temp, "YES")))
						{
							ANCHOR_BLOCK_By_IO(io, 1);
						}
						else
							ANCHOR_BLOCK_By_IO(io, 0);
					}
				}
				else if (!strcmp(temp, "ATTACHNPCTOPLAYER"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ATTACH_NPC_TO_PLAYER ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "ATTACH"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ATTACH ");
					}

#endif
					char temp1[64];
					char temp2[64];
					pos = GetNextWord(es, pos, temp); // Source IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
						strcat(cmd, " ");
					}

#endif
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp1); // source action_point
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp1);
						strcat(cmd, " ");
					}

#endif
					pos = GetNextWord(es, pos, temp); // target IO
					long t2 = GetTargetByNameTarget(temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
						strcat(cmd, " ");
					}

#endif

					if (t2 == -2) t2 = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp2); // target action_point
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
						strcat(cmd, " ");
					}

#endif

					if (ARX_INTERACTIVE_Attach(t, t2, temp1, temp2))
					{
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, "--> success");
						}

#endif
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG)
					{
						strcat(cmd, "--> failure");
					}

#endif
				}

				break;
			case 'G':

				if (!strcmp(temp, "GOTO"))
				{
					if (msg == SM_EXECUTELINE) msg = SM_DUMMY;

					if ((pos = GetNextWord(es, pos, temp)) == -1)
					{
						ret = ACCEPT;
						goto end;
					}

					pos = FindLabelPos(es, temp);

					if (pos == -1) return ACCEPT;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "GOTO %s", temp);

#endif
				}
				else if (!strcmp(temp, "GOSUB"))
				{
					if (msg == SM_EXECUTELINE) msg = SM_DUMMY;

					if ((pos = GetNextWord(es, pos, temp)) == -1) return ACCEPT;

					if (!InSubStack(es, pos)) return BIGERROR;

					pos = FindLabelPos(es, temp);

					if (pos == -1) return ACCEPT;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "GOSUB %s", temp);

#endif
				}
				else if (!strcmp(temp, "GMODE"))
				{
					pos = GetNextWord(es, pos, temp);
				}

				break;
			case 'R':

				if (!strcmp(temp, "REFUSE"))
				{
					ClearSubStack(es);
					ret = REFUSE;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) DEBUG_Notify("  REFUSE");

#endif
					goto end;
				}
				else if (!strcmp(temp, "REVIVE"))
				{
					long tmp = GetNextWord(es, pos, temp);
					long init = 0;

					if (temp[0] == '-')
					{
						pos = tmp;

						if ((iCharIn(temp, 'I'))) init = 1;
					}

					ARX_NPC_Revive(io, init);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) DEBUG_Notify("REVIVE");

#endif
					goto end;
				}
				else if (!strcmp(temp, "RIDICULOUS"))
				{
					ARX_PLAYER_MakeFreshHero();
				}
				else if (!strcmp(temp, "REPAIR"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp);
					float val = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (val < 0.f) val = 0.f;
					else if (val > 100.f) val = 100.f;

					if (ValidIONum(t))
						ARX_DAMAGES_DurabilityRestore(inter.iobj[t], val);
				}
				else if (!strcmp(temp, "RANDOM"))
				{
					char temp1[64];
					pos = GetNextWord(es, pos, temp1);
					float val = (float)atof(temp1);

					if (val < 0.f) val = 0.f;
					else if (val > 100.f) val = 100.f;

					float t = rnd() * 100.f;

					if (val < t)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RANDOM %s", temp1);

#endif
				}
				else if (!strcmp(temp, "RETURN"))
				{
					if ((pos = GetSubStack(es)) == -1) return BIGERROR;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RETURN");

#endif
				}
				else if (!strcmp(temp, "REPLACEME"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{

						char tex[256];
						char tex2[256];

						if (io->ioflags & IO_NPC)
							sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\NPC\\%s.teo", Project.workingdir, temp);
						else if (io->ioflags & IO_FIX)
							sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\FIX_INTER\\%s.teo", Project.workingdir, temp);
						else
							sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\Items\\%s.teo", Project.workingdir, temp);

						File_Standardize(tex2, tex);
						EERIE_3D last_angle;
						memcpy(&last_angle, &io->angle, sizeof(EERIE_3D));
						INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddInteractive(GDevice, tex, -1); //AddItem(GDevice,tex);

						if (ioo != NULL)
						{
							LASTSPAWNED = ioo;
							ioo->scriptload = 1;
							ioo->initpos.x = io->initpos.x;
							ioo->initpos.y = io->initpos.y;
							ioo->initpos.z = io->initpos.z;
							ioo->pos.x = io->pos.x;
							ioo->pos.y = io->pos.y;
							ioo->pos.z = io->pos.z;
							ioo->angle.a = io->angle.a;
							ioo->angle.b = io->angle.b;
							ioo->angle.g = io->angle.g;
							ioo->move.x = io->move.x;
							ioo->move.y = io->move.y;
							ioo->move.z = io->move.z;
							ioo->show = io->show;

							if (io == DRAGINTER)
								Set_DragInter(ioo);

							long neww = GetInterNum(ioo);
							long oldd = GetInterNum(io);

							if (io->ioflags & IO_ITEM)
							{
								if (io->_itemdata->count > 1)
								{
									io->_itemdata->count--;
									SendInitScriptEvent(ioo);
									CheckForInventoryReplaceMe(ioo, io);
								}
								else goto finishit;
							}
							else
							{
							finishit:
								;

								for (long i = 0; i < MAX_SPELLS; i++)
								{
									if ((spells[i].exist) && (spells[i].caster == oldd))
									{
										spells[i].caster = neww;
									}
								}

								io->show = SHOW_FLAG_KILLED;
								ReplaceInAllInventories(io, ioo);
								SendInitScriptEvent(ioo);
								memcpy(&ioo->angle, &last_angle, sizeof(EERIE_3D));
								TREATZONE_AddIO(ioo, neww);

								for (int i = 0; i < MAX_EQUIPED; i++)
								{
									if	((player.equiped[i] != 0)
									        &&	ValidIONum(player.equiped[i]))
									{
										INTERACTIVE_OBJ * equiped = inter.iobj[player.equiped[i]];

										if	(equiped == io)
										{
											ARX_EQUIPMENT_UnEquip(inter.iobj[0], io, 1);
											ARX_EQUIPMENT_Equip(inter.iobj[0], ioo);
										}
									}
								}

								if (io->scriptload)
								{
									ReleaseInter(io);
									return REFUSE;
								}

								TREATZONE_RemoveIO(io);
								return REFUSE;
							}
						}

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "REPLACE_ME %s", temp);

#endif
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "REPLACE_ME %s --> Failure Not An IO", temp);

#endif
				}
				else if (!strcmp(temp, "ROTATE"))
				{
					if (io != NULL)
					{
						char temp1[64];
						char temp2[64];
						char temp3[64];
						float t1, t2, t3;



						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);

						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						t2 = GetVarValueInterpretedAsFloat(temp2, esss, io);
						t3 = GetVarValueInterpretedAsFloat(temp3, esss, io);
						io->angle.a += t1;
						io->angle.b += t2;
						io->angle.g += t3;

						if (io->nb_lastanimvertex != io->obj->nbvertex)
						{
							free(io->lastanimvertex);
							io->lastanimvertex = NULL;
						}

						io->lastanimtime = 0;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "RUNE"))
				{
					pos		 = GetNextWord(es, pos, temp);
					long add = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) add = 1;

						if (iCharIn(temp, 'R')) add = -1;

						pos = GetNextWord(es, pos, temp);
					}
					if( !add && strcmp( temp, "ALL" ) )	//ARX: jycorbel (2010-07-19) - In case of "RUNE ALL" cheat, add is useless it is no risky to keep it uninitialized.
					{
						ARX_CHECK_NO_ENTRY(); //add used without being initialized, remove rune
					}

					if (!stricmp(temp, "AAM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_AAM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_AAM);
					}
					else if (!stricmp(temp, "CETRIUS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_CETRIUS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_CETRIUS);
					}
					else if (!stricmp(temp, "COMUNICATUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_COMUNICATUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_COMUNICATUM);
					}
					else if (!stricmp(temp, "COSUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_COSUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_COSUM);
					}
					else if (!stricmp(temp, "FOLGORA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_FOLGORA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_FOLGORA);
					}
					else if (!stricmp(temp, "FRIDD"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_FRIDD);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_FRIDD);
					}
					else if (!stricmp(temp, "KAOM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_KAOM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_KAOM);
					}
					else if (!stricmp(temp, "MEGA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MEGA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MEGA);
					}
					else if (!stricmp(temp, "MORTE"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MORTE);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MORTE);
					}
					else if (!stricmp(temp, "MOVIS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_MOVIS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_MOVIS);
					}
					else if (!stricmp(temp, "NHI"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_NHI);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_NHI);
					}
					else if (!stricmp(temp, "RHAA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_RHAA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_RHAA);
					}
					else if (!stricmp(temp, "SPACIUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_SPACIUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_SPACIUM);
					}
					else if (!stricmp(temp, "STREGUM"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_STREGUM);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_STREGUM);
					}
					else if (!stricmp(temp, "TAAR"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TAAR);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TAAR);
					}
					else if (!stricmp(temp, "TEMPUS"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TEMPUS);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TEMPUS);
					}
					else if (!stricmp(temp, "TERA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_TERA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_TERA);
					}
					else if (!stricmp(temp, "VISTA"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_VISTA);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_VISTA);
					}
					else if (!stricmp(temp, "VITAE"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_VITAE);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_VITAE);
					}
					else if (!stricmp(temp, "YOK"))
					{
						if (add == 1) ARX_Player_Rune_Add(FLAG_YOK);
						else if (add == -1) ARX_Player_Rune_Remove(FLAG_YOK);
					}
					else if (!stricmp(temp, "ALL"))
						ARX_PLAYER_Rune_Add_All();

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "RUNE %d %s", add, temp);

#endif
				}

				break;
			case 'C':

				if (!strcmp(temp, "CINE")) //CAMERA_ACTIVATE
				{
					long preload = 0;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CINE ");
					}

#endif
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp);
					}

#endif

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							preload = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (!stricmp(temp, "KILL"))
					{
						DANAE_KillCinematic();
					}
					else if (!stricmp(temp, "PLAY"))
					{
						PLAY_LOADED_CINEMATIC = 1;
						ARX_TIME_Pause();
					}
					else
					{
						{
							char temp1[256];
							char temp2[256];
							sprintf(temp1, "%sGraph\\interface\\illustrations\\", Project.workingdir);
							strcpy(temp2, temp1);
							strcat(temp2, temp);
							strcat(temp2, ".cin");
							strcat(temp, ".cin");

							if (PAK_FileExist(temp2))
							{
								strcpy(WILL_LAUNCH_CINE, temp);
								CINE_PRELOAD = preload;
							}
						}
					}
				}
				else if (!strcmp(temp, "COLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "COLLISION ");
						strcat(cmd, temp);
					}

#endif

					if (io)
					{
						if ((!stricmp(temp, "ON")) || (!stricmp(temp, "YES")))
						{
							if (io->ioflags & IO_NO_COLLISIONS)
							{
								long col = 0;

								for (long kkk = 0; kkk < inter.nbmax; kkk++)
								{
									INTERACTIVE_OBJ * ioo = inter.iobj[kkk];

									if (ioo)
									{
										if (IsCollidingIO(io, ioo))
										{
											INTERACTIVE_OBJ * oes = EVENT_SENDER;
											EVENT_SENDER = ioo;
											Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR_DETAIL, "", NULL);
											EVENT_SENDER = oes;
											col = 1;
										}
									}
								}

								if (col)
								{
									INTERACTIVE_OBJ * oes = EVENT_SENDER;
									EVENT_SENDER = NULL;
									Stack_SendIOScriptEvent(io, SM_COLLISION_ERROR, "", NULL);
									EVENT_SENDER = oes;
								}
							}

							io->ioflags &= ~IO_NO_COLLISIONS;
						}
						else
							io->ioflags |= IO_NO_COLLISIONS;
					}
				}
				else if (!strcmp(temp, "CAMERACONTROL"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_CONTROL ");
						strcat(cmd, temp);
					}

#endif

					if (!stricmp(temp, "ON")) CAMERACONTROLLER = io;
					else CAMERACONTROLLER = NULL;
				}
				else if (!strcmp(temp, "CONVERSATION"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CONVERSATION ");
						strcat(cmd, temp);
					}

#endif
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
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					if (!strcmp(temp, "ON"))
					{
						ARX_CONVERSATION = 1;
					}
					else
					{
						ARX_CONVERSATION = 0;
					}

					if ((nb_people) && ARX_CONVERSATION)
					{
						main_conversation.actors_nb = nb_people;

						for (long j = 0; j < nb_people; j++)
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							long t = GetTargetByNameTarget(temp);

							if (t == -2) //self
							{
								for (long k = 0; k < inter.nbmax; k++)
								{
									if (io == inter.iobj[k])
									{
										t = k;
										break;
									}
								}
							}

							main_conversation.actors[j] = t;
						}
					}
				}
				else if (!strcmp(temp, "CAMERAACTIVATE"))
				{
					FRAME_COUNT = -1;
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_ACTIVATE ");
						strcat(cmd, temp);
					}

#endif

					if (!stricmp(temp, "NONE"))
					{
						MasterCamera.exist = 0;
					}
					else
					{
						FRAME_COUNT = 0;
						long t = GetTargetByNameTarget(temp);

						if (t == -2) t = GetInterNum(io);

						if (t != -1)
						{
							if (inter.iobj[t]->ioflags & IO_CAMERA)
							{
								MasterCamera.exist |= 2;
								MasterCamera.want_io = inter.iobj[t];
								MasterCamera.want_aup = (ARX_USE_PATH *)inter.iobj[t]->usepath;
								MasterCamera.want_cam = &inter.iobj[t]->_camdata->cam;
							}

						}
					}
				}
				else if (!strcmp(temp, "CAMERASMOOTHING"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_SMOOTHING ");
						strcat(cmd, temp);
					}

#endif

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							float fo = GetVarValueInterpretedAsFloat(temp, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->smoothing = fo;
						}
					}
				}
				else if (!strcmp(temp, "CINEMASCOPE"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CINEMASCOPE ");
						strcat(cmd, temp);
					}

#endif
					long smooth = 0;

					if (temp[0] == '-')
					{
						if ((temp[1] == 's') || (temp[1] == 'S'))
						{
							smooth = 1;
						}

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					if (!stricmp(temp, "ON"))
					{
						ARX_INTERFACE_SetCinemascope(1, smooth);
					}
					else ARX_INTERFACE_SetCinemascope(0, smooth);
				}
				else if (!strcmp(temp, "CAMERAFOCAL"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "CAMERA_FOCAL ");
						strcat(cmd, temp);
					}

#endif

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							float fo = GetVarValueInterpretedAsFloat(temp, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->focal = fo;

							if (cam->focal < 100) cam->focal = 100;
							else if (cam->focal > 800) cam->focal = 800;
						}
					}
				}
				else if (!strcmp(temp, "CAMERATRANSLATETARGET"))
				{
					char temp2[64];
					char temp3[64];

					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);

					if (io != NULL)
					{
						if (io->ioflags & IO_CAMERA)
						{
							EERIE_3D fo;
							fo.x = GetVarValueInterpretedAsFloat(temp, esss, io);
							fo.y = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fo.z = GetVarValueInterpretedAsFloat(temp3, esss, io);
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->translatetarget.x = fo.x;
							cam->translatetarget.y = fo.y;
							cam->translatetarget.z = fo.z;
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "CAMERA_TRANSLATE_TARGET %s %s %s", temp, temp2, temp3);

#endif
				}
				else if (!strcmp(temp, "CLOSESTEALBAG"))
				{
					if ((io) && (io->ioflags & IO_NPC))
					{
						if (player.Interface & INTER_STEAL)
						{
							INTERACTIVE_OBJ * pio = NULL;

							if (SecondaryInventory != NULL)
							{
								pio = (INTERACTIVE_OBJ *)SecondaryInventory->io;
							}
							else if (player.Interface & INTER_STEAL)
							{
								pio = ioSteal;
							}

							if ((pio != NULL) && (pio == ioSteal))
							{
								InventoryDir = -1;
								SendIOScriptEvent(pio, SM_INVENTORY2_CLOSE, "");
								TSecondaryInventory = SecondaryInventory;
								SecondaryInventory = NULL;
							}

						}
					}
				}

				break;
			case 'Q':

				if (!strcmp(temp, "QUAKE"))
				{
					float f1, f2, f3; // intensity duration period
					pos = GetNextWord(es, pos, temp);
					f1 = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					f2 = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					f3 = GetVarValueInterpretedAsFloat(temp, esss, io);
					AddQuakeFX(f1, f2, f3, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "QUAKE");

#endif
				}
				else if (!strcmp(temp, "QUEST"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PLAYER_Quest_Add(temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "QUEST %s", temp);

#endif
				}

				break;
			case 'N':

				if (!strcmp(temp, "NOP"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "NOP");

#endif
				}
				else if (!strcmp(temp, "NOTE"))
				{
					ARX_INTERFACE_NOTE_TYPE type = NOTE_TYPE_UNDEFINED;
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "NOTE"))
						type = NOTE_TYPE_NOTE;

					if (!stricmp(temp, "NOTICE"))
						type = NOTE_TYPE_NOTICE;

					if (!stricmp(temp, "BOOK"))
						type = NOTE_TYPE_BOOK;

					pos = GetNextWord(es, pos, temp);

					if (player.Interface & INTER_NOTE)
						ARX_INTERFACE_NoteClose();
					else
					{
						ARX_INTERFACE_NoteOpen(type, temp);
					}
				}

				break;
			case 'S':

				if (!strcmp(temp, "SPELLCAST"))
				{

					char temp2[256];
					long duration = -1;
					long flags = 0;
					long dur = 0;
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'K'))
						{
							pos = GetNextWord(es, pos, temp); //spell id
							long spellid = GetSpellId(temp);
							long from = GetInterNum(io);

							if (ValidIONum(from))
							{
								long sp = ARX_SPELLS_GetInstanceForThisCaster(spellid, from);

								if (sp >= 0)
								{
									spells[sp].tolive = 0;
								}
							}

							goto suite;
						}

						if (iCharIn(temp, 'D'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							pos = GetNextWord(es, pos, temp2); // duration
							duration = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);

							if (duration <= 0)
							{
								duration = 99999999;
							}

							dur = 1;
						}

						if (iCharIn(temp, 'X'))
						{
							flags |= SPELLCAST_FLAG_NOSOUND;
						}

						if (iCharIn(temp, 'M'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NODRAW;
						}

						if (iCharIn(temp, 'S'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOANIM;
						}

						if (iCharIn(temp, 'F'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOMANA;
						}

						if (iCharIn(temp, 'Z'))
						{
							flags |= SPELLCAST_FLAG_RESTORE;
						}

						pos = GetNextWord(es, pos, temp); // level
					}

					long level;
					level = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

					if (level < 1) level = 1;
					else if (level > 10) level = 10;

					if (!dur)
						duration = 1000 + level * 2000;

					pos = GetNextWord(es, pos, temp); //spell id
					long spellid;
					spellid = GetSpellId(temp);
					pos = GetNextWord(es, pos, temp); //spell target
					long t;
					t = GetTargetByNameTarget(temp);

					if (t <= -1) t = GetInterNum(io);

					if ((t >= 0)
					        &&	(t < inter.nbmax)
					        &&	(spellid != -1))
					{
						if (io != inter.iobj[0])
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
						}

						TryToCastSpell(io, spellid, level, t, flags, duration);
					}

				suite:
					;
				}
				else if (!strcmp(temp, "SPEAK")) // speak say_ident actions
				{
					ARX_CINEMATIC_SPEECH acs;
					acs.type = ARX_CINE_SPEECH_NONE;


					char temp2[256];
					long ttt;

					long player		=	0;
					long voixoff	=	0;
					long notext		=	0;

					pos = GetNextWord(es, pos, temp2);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPEAK ");
						strcat(cmd, temp2);
					}

#endif

					long mood			=	ANIM_TALK_NEUTRAL;
					long unbreakable	=	0;
					MakeUpcase(temp2);

					if (!stricmp(temp2, "KILLALL"))
					{
						ARX_SPEECH_Reset();
					}
					else
					{
						if (temp2[0] == '-')
						{
							if (iCharIn(temp2, 'T')) notext		=	1;

							if (iCharIn(temp2, 'U')) unbreakable =	1;

							if (iCharIn(temp2, 'P')) player		=	1;

							if (iCharIn(temp2, 'H')) mood		=	ANIM_TALK_HAPPY;

							if (iCharIn(temp2, 'A')) mood		=	ANIM_TALK_ANGRY;

							if (iCharIn(temp2, 'O'))
							{
								voixoff	=	2;
								
								//Crash when we set speak pitch to 1, 
								//Variable use for a division, 0 is not possible
								//To find
							}

							if (iCharIn(temp2, 'C'))
							{
								FRAME_COUNT	=	0;
								pos			=	GetNextWord(es, pos, temp2);

								if (!stricmp(temp2, "KEEP"))
								{
									acs.type	=	ARX_CINE_SPEECH_KEEP;
									acs.pos1.x	=	LASTCAMPOS.x;
									acs.pos1.y	=	LASTCAMPOS.y;
									acs.pos1.z	=	LASTCAMPOS.z;
									acs.pos2.a	=	LASTCAMANGLE.a;
									acs.pos2.b	=	LASTCAMANGLE.b;
									acs.pos2.g	=	LASTCAMANGLE.g;
								}

								if (!stricmp(temp2, "ZOOM"))
								{
									acs.type			=	ARX_CINE_SPEECH_ZOOM;
									pos					=	GetNextWord(es, pos, temp2);
									acs.startangle.a	=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.startangle.b	=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endangle.a		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endangle.b		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.startpos		=	GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos					=	GetNextWord(es, pos, temp2);
									acs.endpos			=	GetVarValueInterpretedAsFloat(temp2, esss, io);


									//ARX_CHECK_NO_ENTRY(); //ARX: xrichter (2010-07-20) - temp2 is often (always?) a string number and GetTargetByNameTarget return -1. To be careful if temp2 is not a string number, we choose to test GetTargetByNameTarget return value.
									acs.ionum			=	GetTargetByNameTarget(temp2);

									if (acs.ionum == -2)   //means temp2 is "me" or "self"
										acs.ionum		=	GetInterNum(io);



									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, -1);
								}
								else if ((!stricmp(temp2, "CCCTALKER_L"))
								         || (!stricmp(temp2, "CCCTALKER_R")))
								{
									if (!stricmp(temp2, "CCCTALKER_R"))
										acs.type = ARX_CINE_SPEECH_CCCTALKER_R;
									else acs.type = ARX_CINE_SPEECH_CCCTALKER_L;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
								else if ((!stricmp(temp2, "CCCLISTENER_L"))
								         || (!stricmp(temp2, "CCCLISTENER_R")))
								{
									if (!stricmp(temp2, "CCCLISTENER_R"))
										acs.type = ARX_CINE_SPEECH_CCCLISTENER_R;
									else acs.type = ARX_CINE_SPEECH_CCCLISTENER_L;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
								else if ((!stricmp(temp2, "SIDE"))
								         || (!stricmp(temp2, "SIDE_L"))
								         || (!stricmp(temp2, "SIDE_R")))
								{
									if (!stricmp(temp2, "SIDE_L"))
										acs.type = ARX_CINE_SPEECH_SIDE_LEFT;
									else acs.type = ARX_CINE_SPEECH_SIDE;

									pos = GetNextWord(es, pos, temp2);
									acs.ionum = GetTargetByNameTarget(temp2);

									if (acs.ionum == -2) acs.ionum = GetInterNum(io);

									pos = GetNextWord(es, pos, temp2);
									acs.startpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									pos = GetNextWord(es, pos, temp2);
									acs.endpos = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//startdist
									pos = GetNextWord(es, pos, temp2);
									acs.f0 = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//enddist
									pos = GetNextWord(es, pos, temp2);
									acs.f1 = GetVarValueInterpretedAsFloat(temp2, esss, io);
									//height modifier
									pos = GetNextWord(es, pos, temp2);
									acs.f2 = GetVarValueInterpretedAsFloat(temp2, esss, io);

									if (player)
										ComputeACSPos(&acs, inter.iobj[0], acs.ionum);
									else ComputeACSPos(&acs, io, acs.ionum);
								}
							}

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif
						}

						long speechnum;
						char * temp1 = GetVarValueInterpretedAsText(temp2, esss, io);

						if (!strcmp(temp2, "[]"))
						{
							ARX_SPEECH_ClearIOSpeech(io);
						}
						else
						{
							if (notext) voixoff |= ARX_SPEECH_FLAG_NOTEXT;

							if (!CINEMASCOPE) voixoff |= ARX_SPEECH_FLAG_NOTEXT;

							if (player)
							{
								speechnum = ARX_SPEECH_AddSpeech(inter.iobj[0], temp1, PARAM_LOCALISED, mood, voixoff);
							}
							else
								speechnum = ARX_SPEECH_AddSpeech(io, temp1, PARAM_LOCALISED, mood, voixoff);

							ttt = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							if ((!LINEEND) && (speechnum >= 0))
							{
								char timername[128];
								char timername2[128];
								ARX_SCRIPT_Timer_GetDefaultName(timername2);
								sprintf(timername, "SPEAK_%s", timername2);
								aspeech[speechnum].scrpos = pos;
								aspeech[speechnum].es = es;
								aspeech[speechnum].ioscript = io;

								if (unbreakable) aspeech[speechnum].flags |= ARX_SPEECH_FLAG_UNBREAKABLE;

								memcpy(&aspeech[speechnum].cine, &acs, sizeof(ARX_CINEMATIC_SPEECH));
								pos = GotoNextLine(es, pos);
							}

							LINEEND = 0;
						}
					}
				}
				else if (!strcmp(temp, "SHOPCATEGORY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->shop_category) free(io->shop_category);

					io->shop_category = NULL;
					io->shop_category = (char *)malloc(strlen(temp) + 1);

					if (io->shop_category)
						strcpy(io->shop_category, temp);
				}
				else if (!strcmp(temp, "SHOPMULTIPLY"))
				{
					pos = GetNextWord(es, pos, temp);
					io->shop_multiply = GetVarValueInterpretedAsFloat(temp, esss, io);
				}
				else if (!strcmp(temp, "SETPOISONOUS"))
				{
					float poisonous = 0.f;
					float poisonous_count = 0.f;
					pos = GetNextWord(es, pos, temp);
					poisonous = GetVarValueInterpretedAsFloat(temp, esss, io);;
					pos = GetNextWord(es, pos, temp);
					poisonous_count = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (poisonous_count == 0)
					{
						io->poisonous_count = 0;
					}
					else
					{

						ARX_CHECK_SHORT(poisonous);
						ARX_CHECK_SHORT(poisonous_count);

						io->poisonous		= ARX_CLEAN_WARN_CAST_SHORT(poisonous);
						io->poisonous_count = ARX_CLEAN_WARN_CAST_SHORT(poisonous_count);

					}

				}
				else if (!strcmp(temp, "SETPLATFORM"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->GameFlags |= GFLAG_PLATFORM;
					}
					else io->GameFlags &= ~GFLAG_PLATFORM;
				}
				else if (!strcmp(temp, "SETGORE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->GameFlags &= ~GFLAG_NOGORE;
					}
					else io->GameFlags |= GFLAG_NOGORE;
				}
				else if (!strcmp(temp, "SETUNIQUE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags |= IO_UNIQUE;
					}
					else io->ioflags &= ~IO_UNIQUE;
				}
				else if (!strcmp(temp, "SETBLACKSMITH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags |= IO_BLACKSMITH;
					}
					else io->ioflags &= ~IO_BLACKSMITH;
				}
				else if (!strcmp(temp, "SETELEVATOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->GameFlags |= GFLAG_ELEVATOR;
					}
					else io->GameFlags &= ~GFLAG_ELEVATOR;
				}
				else if (!strcmp(temp, "SETTRAP")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_FIX))
					{
						if (!stricmp(temp, "off"))
						{
							io->_fixdata->trapvalue = -1;
						}
						else
						{
							io->_fixdata->trapvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_fixdata->trapvalue < -1) io->_fixdata->trapvalue = -1;

							if (io->_fixdata->trapvalue > 100) io->_fixdata->trapvalue = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETSECRET")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_FIX))
					{
						if (!stricmp(temp, "off"))
						{
							io->secretvalue = -1;
						}
						else
						{
							io->secretvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->secretvalue < -1) io->secretvalue = -1;

							if (io->secretvalue > 100) io->secretvalue = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETDETECT")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if (!stricmp(temp, "off"))
						{
							io->_npcdata->fDetect = -1;
						}
						else
						{
							io->_npcdata->fDetect = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_npcdata->fDetect < -1)	io->_npcdata->fDetect = -1;

							if (io->_npcdata->fDetect > 100) io->_npcdata->fDetect = 100;
						}
					}
				}
				else if (!strcmp(temp, "SETSTEAL")) // -1 = off
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!stricmp(temp, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->stealvalue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->stealvalue < -1)	io->_itemdata->stealvalue = -1;

							if (io->_itemdata->stealvalue > 100) io->_itemdata->stealvalue = 100;

							if (io->_itemdata->stealvalue == 100) io->_itemdata->stealvalue = -1;
						}
					}
				}
				else if (!strcmp(temp, "SETLIGHT")) // -1 = off  for ITEM only
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						if (!stricmp(temp, "off"))
						{
							io->_itemdata->stealvalue = -1;
						}
						else
						{
							io->_itemdata->LightValue = (char)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->LightValue < -1)  io->_itemdata->LightValue = -1;

							if (io->_itemdata->LightValue > 1) io->_itemdata->LightValue = 1;
						}
					}
				}
				else if (!strcmp(temp, "SETBLOOD"))
				{
					pos = GetNextWord(es, pos, temp);
					float r = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					float g = GetVarValueInterpretedAsFloat(temp, esss, io);
					pos = GetNextWord(es, pos, temp);
					float b = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (io->ioflags & IO_NPC)
					{
						io->_npcdata->blood_color = D3DRGB(r, g, b);
					}
				}
				else if (!strcmp(temp, "SETMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						io->material = ARX_MATERIAL_GetIdByName(temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSPEAKPITCH"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->speakpitch = GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_npcdata->speakpitch < 0.6f) io->_npcdata->speakpitch = 0.6f;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SPEAK_PITCH %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETFOOD"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->ioflags & IO_ITEM)
					{
						io->_itemdata->food_value = (char)GetVarValueInterpretedAsFloat(temp, esss, io);
					}
				}
				else if (!strcmp(temp, "SETSPEED"))
				{
					pos = GetNextWord(es, pos, temp);
					io->basespeed = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (io->basespeed < 0.f) io->basespeed = 0.f;

					if (io->basespeed > 10.f) io->basespeed = 10.f;
				}
				else if (!strcmp(temp, "SETSTAREFACTOR"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->stare_factor = GetVarValueInterpretedAsFloat(temp, esss, io);
					}
				}
				else if (!strcmp(temp, "SETGROUP"))
				{
					pos = GetNextWord(es, pos, temp);
					long remove = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R'))
							remove = 1;

						pos = GetNextWord(es, pos, temp);
					}

					char * temp1 = GetVarValueInterpretedAsText(temp, esss, io);

					if (remove)
					{
						if (!stricmp(temp1, "DOOR")) io->GameFlags &= ~GFLAG_DOOR;

						ARX_IOGROUP_Remove(io, temp1);
					}
					else
					{
						if (!stricmp(temp1, "DOOR")) io->GameFlags |= GFLAG_DOOR;

						ARX_IOGROUP_Add(io, temp1);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_GROUP %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETNPCSTAT"))
				{
					char temp2[256];
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					ARX_NPC_SetStat(io, temp, GetVarValueInterpretedAsFloat(temp2, esss, io));
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_NPC_STAT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETXPVALUE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io && (io->ioflags & IO_NPC))
					{
						io->_npcdata->xpvalue = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_npcdata->xpvalue < 0) io->_npcdata->xpvalue = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_XP_VALUE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETNAME"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						strcpy(io->locname, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETNAME %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETPLAYERTWEAK"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_PLAYER_TWEAK ");
						strcat(cmd, temp);
					}

#endif

					if (io->tweakerinfo == NULL)
					{
						io->tweakerinfo = (IO_TWEAKER_INFO *)malloc(sizeof(IO_TWEAKER_INFO));

						if (io->tweakerinfo)
						{
							memset(io->tweakerinfo, 0, sizeof(IO_TWEAKER_INFO));
						}
					}

					if (!stricmp(temp, "SKIN"))
					{
						char temp2[256];
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->skintochange, temp);
							strcpy(io->tweakerinfo->skinchangeto, temp2);
						}
					}
					else	// Mesh Tweaker...
					{
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif

						if (io->tweakerinfo)
						{
							strcpy(io->tweakerinfo->filename, temp);
						}
					}
				}
				else if (!strcmp(temp, "SETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(temp);

					if (ap != NULL)
					{
						char title[64];
						strcpy(temp, GetName(io->filename));
						sprintf(title, "%s_%04d", temp, io->ident);
						strcpy(ap->controled, title);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_CONTROLLED_ZONE %s", temp);

#endif
				}
				else if ((!strcmp(temp, "SETSTATUS")) || (!strcmp(temp, "SETMAINEVENT")))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_SCRIPT_SetMainEvent(io, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETMAINEVENT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETMOVEMODE"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io != NULL) && (io->ioflags & IO_NPC))
					{
						if (!strcmp(temp, "WALK"))	ARX_NPC_ChangeMoveMode(io, WALKMODE);

						if (!strcmp(temp, "RUN"))	ARX_NPC_ChangeMoveMode(io, RUNMODE);

						if (!strcmp(temp, "NONE"))	ARX_NPC_ChangeMoveMode(io, NOMOVEMODE);

						if (!strcmp(temp, "SNEAK"))	ARX_NPC_ChangeMoveMode(io, SNEAKMODE);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SETMOVEMODE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SPAWN"))
				{
					char temp2[64];
					char tmptext[256];
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPAWN ");
						strcat(cmd, temp);
					}

#endif
					MakeUpcase(temp);

					if (!strcmp(temp, "NPC"))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
						pos = GetNextWord(es, pos, temp2); // object ident for position
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\NPC\\%s", Project.workingdir, temp);
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddNPC(GDevice, tmptext, IO_IMMEDIATELOAD);

								if (ioo)
								{
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos.x = inter.iobj[t]->pos.x;
									ioo->pos.y = inter.iobj[t]->pos.y;
									ioo->pos.z = inter.iobj[t]->pos.z;

									ioo->angle.a = inter.iobj[t]->angle.a;
									ioo->angle.b = inter.iobj[t]->angle.b;
									ioo->angle.g = inter.iobj[t]->angle.g;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);

									if (inter.iobj[t]->ioflags & IO_NPC)
									{
										float dist = inter.iobj[t]->physics.cyl.radius + ioo->physics.cyl.radius + 10;
										EERIE_3D ofs;
										ofs.x = -EEsin(DEG2RAD(inter.iobj[t]->angle.b)) * dist;
										ofs.y = 0.f;
										ofs.z = EEcos(DEG2RAD(inter.iobj[t]->angle.b)) * dist;
										ioo->pos.x += ofs.x;
										ioo->pos.z += ofs.z;
									}

									TREATZONE_AddIO(ioo, GetInterNum(ioo));
								}
							}
						}
					}
					else if (!strcmp(temp, "ITEM"))
					{
						pos = GetNextWord(es, pos, temp); // object to spawn.
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
						pos = GetNextWord(es, pos, temp2); // object ident for position
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);

						if ((t >= 0) && (t < inter.nbmax))
						{
							char tex2[256];
							sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\ITEMS\\%s", Project.workingdir, temp);
							File_Standardize(tex2, tmptext);
							INTERACTIVE_OBJ * ioo;

							if (FORBID_SCRIPT_IO_CREATION == 0)
							{
								ioo = AddItem(GDevice, tmptext, IO_IMMEDIATELOAD);

								if (ioo)
								{
									MakeTemporaryIOIdent(ioo);
									LASTSPAWNED = ioo;
									ioo->scriptload = 1;
									ioo->pos.x = inter.iobj[t]->pos.x;
									ioo->pos.y = inter.iobj[t]->pos.y;
									ioo->pos.z = inter.iobj[t]->pos.z;
									ioo->angle.a = inter.iobj[t]->angle.a;
									ioo->angle.b = inter.iobj[t]->angle.b;
									ioo->angle.g = inter.iobj[t]->angle.g;
									MakeTemporaryIOIdent(ioo);
									SendInitScriptEvent(ioo);
								}

								TREATZONE_AddIO(ioo, GetInterNum(ioo));
							}
						}
					}
					else if (!strcmp(temp, "FIREBALL"))
					{
						GetTargetPos(io);
						EERIE_3D pos;
						pos.x = io->pos.x;
						pos.y = io->pos.y;
						pos.z = io->pos.z;

						if (io->ioflags & IO_NPC) pos.y -= 80.f;

						ARX_MISSILES_Spawn(io, MISSILE_FIREBALL, &pos, &io->target);
					}
				}
				else if (!strcmp(temp, "SETOBJECTTYPE"))
				{
					pos = GetNextWord(es, pos, temp);
					long val = 1; // flag to add

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R'))
						{
							val = 0; // flag to remove
						}

						pos = GetNextWord(es, pos, temp);
					}

					ARX_EQUIPMENT_SetObjectType(io, temp, val);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_OBJECT_TYPE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETRIGHTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_RIGHT_HAND %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETHUNGER"))
				{
					pos = GetNextWord(es, pos, temp);
					player.hunger = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_HUNGER %s", temp);

#endif
				}

				else if (!strcmp(temp, "SETLEFTHAND"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LEFT_HAND %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETSHIELD"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHIELD %s ...OBSOLETE...", temp);

#endif
				}
				else if (!strcmp(temp, "SETTWOHANDED"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_TWO_HANDED ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "SETINTERACTIVITY"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "NONE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}
					else if (!stricmp(temp, "HIDE"))
					{
						io->GameFlags &= ~GFLAG_INTERACTIVITY;
						io->GameFlags |= GFLAG_INTERACTIVITYHIDE;
					}
					else
					{
						io->GameFlags |= GFLAG_INTERACTIVITY;
						io->GameFlags &= ~GFLAG_INTERACTIVITYHIDE;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_INTERACTIVITY %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETEQUIP"))
				{
					char temp2[128];
					temp2[0] = 0;
					char temp3[128];
					pos = GetNextWord(es, pos, temp3);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_EQUIP ");
						strcat(cmd, temp3);
					}

#endif

					if (temp3[0] == '-')
					{
						if (!stricmp(temp3, "-r"))  ARX_EQUIPMENT_Remove_All_Special(io);
						else
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif
						}
					}
					else
					{
						strcpy(temp, temp3);
						pos = GetNextWord(es, pos, temp2);
						temp3[0] = 0;
					}

					short flag = 0;

					if (temp2[0])
					{
						if (temp2[strlen(temp2)-1] == '%') flag = 1;
					}
					else flag = 0;

					ARX_EQUIPMENT_SetEquip(io, temp3, temp, GetVarValueInterpretedAsFloat(temp2, esss, io), flag);
				}
				else if (!strcmp(temp, "SETONEHANDED"))
				{

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_ONE_HANDED ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "SETWEAPON"))
				{
					pos = GetNextWord(es, pos, temp);
					io->GameFlags &= ~GFLAG_HIDEWEAPON;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'H'))	// Hide Weapon
						{
							io->GameFlags |= GFLAG_HIDEWEAPON;
						}

						pos = GetNextWord(es, pos, temp);
					}

					if ((io) && (io->ioflags & IO_NPC))
					{
						// temporarily removed for Alpha
						strcpy(io->_npcdata->weaponname, temp);
						Prepare_SetWeapon(io, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WEAPON %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETLIFE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						if (io->ioflags & IO_NPC)
						{
							io->_npcdata->maxlife = io->_npcdata->life = GetVarValueInterpretedAsFloat(temp, esss, io);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LIFE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETDURABILITY"))
				{
					long current = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'C')) current = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (io != NULL)
					{
						if (!(io->ioflags & IO_NPC))
						{
							if (current) io->durability = GetVarValueInterpretedAsFloat(temp, esss, io);
							else io->max_durability = io->durability = GetVarValueInterpretedAsFloat(temp, esss, io);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_LIFE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETPATH"))
				{
					long wormspecific = 0;
					long followdir = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'W'))
						{
							wormspecific = 1;
						}

						if (iCharIn(temp, 'F'))
						{
							followdir = 1;
						}

						pos = GetNextWord(es, pos, temp);
					}

					if (io != NULL)
					{

						if (!stricmp(temp, "NONE"))
						{
							if (io->usepath != NULL)
							{
								free(io->usepath);
								io->usepath = NULL;
							}
						}
						else
						{
							ARX_PATH * ap = ARX_PATH_GetAddressByName(temp);

							if ((ap != NULL) && (ap != io->usepath))
							{
								if (io->usepath != NULL)
								{
									free(io->usepath);
									io->usepath = NULL;
								}

								ARX_USE_PATH * aup = (ARX_USE_PATH *)malloc(sizeof(ARX_USE_PATH));
								aup->_starttime = aup->_curtime = ARXTime;
								aup->aupflags = ARX_USEPATH_FORWARD;

								if (wormspecific)
								{
									aup->aupflags |= ARX_USEPATH_WORM_SPECIFIC | ARX_USEPATH_FLAG_ADDSTARTPOS;
								}

								if (followdir) aup->aupflags |= ARX_USEPATH_FOLLOW_DIRECTION;

								aup->initpos.x = io->initpos.x;
								aup->initpos.y = io->initpos.y;
								aup->initpos.z = io->initpos.z;
								aup->lastWP = -1;
								aup->path = ap;
								io->usepath = (void *)aup;
							}
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PATH %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETTARGET"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET_TARGET ");
						strcat(cmd, temp);
					}

#endif

					if ((io) && (io->ioflags & IO_NPC))
					{
						io->_npcdata->pathfind.flags &= ~PATHFIND_ALWAYS;
						io->_npcdata->pathfind.flags &= ~PATHFIND_ONCE;
						io->_npcdata->pathfind.flags &= ~PATHFIND_NO_UPDATE;
					}

					if (temp[0] == '-')
					{
						if ((io) && (io->ioflags & IO_NPC))
						{
							if (iCharIn(temp, 'S'))
								io->_npcdata->pathfind.flags |= PATHFIND_ONCE;

							if (iCharIn(temp, 'A'))
								io->_npcdata->pathfind.flags |= PATHFIND_ALWAYS;

							if (iCharIn(temp, 'N'))
								io->_npcdata->pathfind.flags |= PATHFIND_NO_UPDATE;
						}

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					char * temp1 = GetVarValueInterpretedAsText(temp, esss, io);

					if (io != NULL)
					{
						long old_target = -12;

						if ((io) && (io->ioflags & IO_NPC))
						{
							if (io->_npcdata->reachedtarget)
								old_target = io->targetinfo;

							if ((io->_npcdata->behavior & BEHAVIOUR_FLEE) ||
							        (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND))
								old_target = -12;
						}

						if (!stricmp(temp, "OBJECT"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							temp1 = GetVarValueInterpretedAsText(temp, esss, io);
						}

						long t = GetTargetByNameTarget(temp1);

						if (t == -2) t = GetInterNum(io);

						if (io->ioflags & IO_CAMERA)
						{
							EERIE_CAMERA * cam = (EERIE_CAMERA *)io->_camdata;
							cam->translatetarget.x = 0.f;
							cam->translatetarget.y = 0.f;
							cam->translatetarget.z = 0.f;
						}

						if (ValidIONum(t))
						{
							io->targetinfo = t; //TARGET_PATH;
							GetTargetPos(io);
						}

						if (!stricmp(temp1, "PATH"))
						{
							io->targetinfo = TARGET_PATH;
							GetTargetPos(io);
						}
						else if (!stricmp(temp1, "NONE"))
						{
							io->targetinfo = TARGET_NONE;
						}

						if (old_target != t)
						{
							if (io->ioflags & IO_NPC) io->_npcdata->reachedtarget = 0;

							ARX_NPC_LaunchPathfind(io, t);
						}
					}
				}
				else if (!strcmp(temp, "STARTTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = -1;

					if (!strcmp(temp, "TIMER1")) t = 0;

					if (!strcmp(temp, "TIMER2")) t = 1;

					if (!strcmp(temp, "TIMER3")) t = 2;

					if (!strcmp(temp, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = ARXTimeUL();

						if (esss->timers[t] == 0) esss->timers[t] = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "START_TIMER %s", temp);

#endif
				}
				else if (!strcmp(temp, "STOPTIMER"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = -1;

					if (!strcmp(temp, "TIMER1")) t = 0;

					if (!strcmp(temp, "TIMER2")) t = 1;

					if (!strcmp(temp, "TIMER3")) t = 2;

					if (!strcmp(temp, "TIMER4")) t = 3;

					if (t > -1)
					{
						esss->timers[t] = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "STOP_TIMER %s", temp);

#endif
				}
				else if (!strcmp(temp, "SENDEVENT"))
				{
					char * evt = NULL;
					char temp1[64];
					char temp2[64];
					char temp3[64];
					char zonename[128];
					pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SEND_EVENT ");
						strcat(cmd, temp);
					}

#endif
					long radius = 0;
					long zone = 0;
					long group = 0;
					char groupname[64];
#define SEND_NPC	1
#define SEND_ITEM	2
#define SEND_FIX	4
					long sendto = SEND_NPC;

					if (temp1[0] == '-')
					{
						if (iCharIn(temp1, 'G'))
							group = 1;

						if (iCharIn(temp1, 'F'))
							sendto = SEND_FIX;

						if (iCharIn(temp1, 'I'))
						{
							if (sendto == SEND_NPC)
								sendto = SEND_ITEM;
							else sendto |= SEND_ITEM;
						}

						if (iCharIn(temp1, 'N'))
							sendto |= SEND_NPC;

						if (iCharIn(temp1, 'R'))
							radius = 1;

						if (iCharIn(temp1, 'Z'))
						{
							zone = 1;
						}

						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif

						if (group)
						{
							char * temp6 = GetVarValueInterpretedAsText(temp1, esss, io);
							strcpy(groupname, temp6);
							pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp1);
							}

#endif
						}
					}

					float rad = 0;

					if ((group) && (!zone) && (!radius))
					{
					}
					else
					{
						pos = GetNextWord_Interpreted(io, es, pos, temp2);

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif

						if (zone) strcpy(zonename, temp2);

						if (radius) rad = GetVarValueInterpretedAsFloat(temp2, esss, io);
					}

					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
					}

#endif

					long i = 0;

					while (i < SM_MAXCMD)
					{
						if (!strcmp(temp1, AS_EVENT[i].name + 3))
						{
							break;
						}

						i++;
					}

					if (i >= SM_MAXCMD)
					{
						evt = temp1;
					}
					else
					{
						evt = AS_EVENT[i].name + 3;
					}

					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = io;

					if (radius)   // SEND EVENT TO ALL OBJECTS IN A RADIUS
					{
						EERIE_3D _pos, _pos2;

						for (long l = 0 ; l < inter.nbmax ; l++)
						{
							if ((inter.iobj[l] != NULL)
							        && (inter.iobj[l] != io)
							        &&	!(inter.iobj[l]->ioflags & IO_CAMERA)
							        &&	!(inter.iobj[l]->ioflags & IO_MARKER)
							        && ((!group) || (IsIOGroup(inter.iobj[l], groupname)))
							   )
							{
								if (((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
								        ||	((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
								        ||	((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM)))
								{
									GetItemWorldPosition(inter.iobj[l], &_pos);
									GetItemWorldPosition(io, &_pos2);

									if (EEDistance3D(&_pos, &_pos2) <= rad)
									{
										io->stat_sent++;
										Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
									}
								}
							}
						}
					}
					else if (zone) // SEND EVENT TO ALL OBJECTS IN A ZONE
					{
						ARX_PATH * ap = ARX_PATH_GetAddressByName(zonename);

						if (ap != NULL)
						{
							EERIE_3D _pos;

							for (long l = 0; l < inter.nbmax; l++)
							{
								if ((inter.iobj[l])
								        &&	!(inter.iobj[l]->ioflags & IO_CAMERA)
								        &&	!(inter.iobj[l]->ioflags & IO_MARKER)
								        &&	((!group) || (IsIOGroup(inter.iobj[l], groupname)))
								   )
								{
									if (((sendto & SEND_NPC) && (inter.iobj[l]->ioflags & IO_NPC))
									        ||	((sendto & SEND_FIX) && (inter.iobj[l]->ioflags & IO_FIX))
									        ||	((sendto & SEND_ITEM) && (inter.iobj[l]->ioflags & IO_ITEM)))
									{
										GetItemWorldPosition(inter.iobj[l], &_pos);

										if (ARX_PATH_IsPosInZone(ap, _pos.x, _pos.y, _pos.z))
										{
											io->stat_sent++;
											Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
										}
									}
								}
							}
						}
					}
					else if	(group) // sends an event to all members of a group
					{
						for (long l = 0; l < inter.nbmax; l++)
						{
							if ((inter.iobj[l] != NULL)
							        && (inter.iobj[l] != io)
							        && (IsIOGroup(inter.iobj[l], groupname))
							   )
							{
								io->stat_sent++;
								Stack_SendIOScriptEvent(inter.iobj[l], 0, temp3, evt);
							}
						}
					}
					else // SINGLE OBJECT EVENT
					{
						long t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);



						if (ValidIONum(t))
						{
							io->stat_sent++;
							Stack_SendIOScriptEvent(inter.iobj[t], 0, temp3, evt);
						}
					}

					EVENT_SENDER = oes;
				}
				else if (!strcmp(temp, "SET"))
				{
					char temp2[64];
					char tempp[256];
					long ival;
					float fval;
					SCRIPT_VAR * sv = NULL;
					long a = 0;
					pos = GetNextWord(es, pos, temp, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SET ");
						strcat(cmd, temp);
					}

#endif

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'A')) a = 1;

						pos = GetNextWord(es, pos, temp, 1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					pos = GetNextWord(es, pos, temp2);

					switch (temp[0])
					{
						case '$': // GLOBAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io));

							if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(&svar, &NB_GLOBALS, temp, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_TEXT;

							break;
						case '£': // LOCAL TEXT
							strcpy(tempp, GetVarValueInterpretedAsText(temp2, esss, io));

							if (a) RemoveNumerics(tempp);

							sv = SETVarValueText(&esss->lvar, &esss->nblvar, temp, tempp);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_TEXT;

							break;
						case '#': // GLOBAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(&svar, &NB_GLOBALS, temp, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_LONG;

							break;
						case '§': // LOCAL LONG
							ival = (long)GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueLong(&esss->lvar, &esss->nblvar, temp, ival);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(&svar, &NB_GLOBALS, temp, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp, fval);

							if (sv == NULL)
							{
								ShowScriptError("Unable to Set Variable Value", cmd);
							}
							else sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET %s %s", temp, temp2);

#endif
				}
				else if (!strcmp(temp, "SAY"))
				{
					//DO NOTHING
				}
				else if (!strcmp(temp, "SETANGULAR"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))	io->ioflags |= IO_ANGULAR;
					else	io->ioflags &= ~IO_ANGULAR;
				}
				else if (!strcmp(temp, "SETPLAYERCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->collision |= 1;
					else	io->collision &= ~1;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PLAYER_COLLISION %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSTEPMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->stepmaterial)
						{
							free((void *)io->stepmaterial);
							io->stepmaterial = NULL;
						}

						io->stepmaterial = (char *)malloc(strlen(temp) + 1);
						strcpy(io->stepmaterial, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETARMORMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->armormaterial)
						{
							free((void *)io->armormaterial);
							io->armormaterial = NULL;
						}

						io->armormaterial = (char *)malloc(strlen(temp) + 1);
						strcpy(io->armormaterial, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_ARMOR_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWEAPONMATERIAL"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->weaponmaterial)
							free(io->weaponmaterial);

						io->weaponmaterial = NULL;
						io->weaponmaterial = strdup(temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSTRIKESPEECH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->strikespeech) free(io->strikespeech);

						io->strikespeech = NULL;
						io->strikespeech = strdup(temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_STEP_MATERIAL %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETPLAYERCONTROLS"))
				{
					INTERACTIVE_OBJ * oes = EVENT_SENDER;
					EVENT_SENDER = io;
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))
					{
						if (BLOCK_PLAYER_CONTROLS)
						{
							Stack_SendMsgToAllNPC_IO(SM_CONTROLS_ON, "");
						}

						BLOCK_PLAYER_CONTROLS = 0;
					}
					else
					{
						if (!BLOCK_PLAYER_CONTROLS)
						{
							ARX_PLAYER_PutPlayerInNormalStance(0);
							Stack_SendMsgToAllNPC_IO(SM_CONTROLS_OFF, "");
							ARX_SPELLS_FizzleAllSpellsFromCaster(0);
						}

						BLOCK_PLAYER_CONTROLS = 1;
						player.Interface &= ~INTER_COMBATMODE;
					}

					EVENT_SENDER = oes;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PLAYER_CONTROLS %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWORLDCOLLISION"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->collision |= 2;
					else io->collision &= ~2;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WORLD_COLLISION %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETSHADOW"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (!strcmp(temp, "ON"))	io->ioflags &= ~IO_NOSHADOW;
					else io->ioflags |= IO_NOSHADOW;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHADOW %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETDETACHABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSTACKABLE"))
				{
					pos = GetNextWord(es, pos, temp);
				}
				else if (!strcmp(temp, "SETSHOP"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if ((!strcmp(temp, "ON")) || (!strcmp(temp, "YES")))
						io->ioflags |= IO_SHOP;
					else	io->ioflags &= ~IO_SHOP;

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_SHOP %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETMAXCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->maxcount = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->maxcount < 1) io->_itemdata->maxcount = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_MAX_COUNT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETCOUNT"))
				{
					pos = GetNextWord(es, pos, temp);
					float c = (float)atof(temp);

					if (c < 1.f) c = 1.f;

					if ((io != NULL) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->count = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->count < 1) io->_itemdata->count = 1;

						if (io->_itemdata->count > io->_itemdata->maxcount) io->_itemdata->count = io->_itemdata->maxcount;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_COUNT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETWEIGHT"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{
						io->weight = GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->weight < 0.f) io->weight = 0.f;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_WEIGHT %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETTRANSPARENCY"))
				{
					pos = GetNextWord(es, pos, temp);

					io->invisibility = 1.f + GetVarValueInterpretedAsFloat(temp, esss, io) * DIV100;

					if (io->invisibility == 1.f) io->invisibility = 0;
				}
				else if (!strcmp(temp, "SETEVENT"))
				{
					char temp2[64];
					long t;
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp);
					MakeUpcase(temp2);

					if ((!strcmp(temp2, "ON")) || (!strcmp(temp2, "YES"))) t = 1;
					else t = 0;

					if (!strcmp(temp, "COLLIDE_NPC"))
					{
						if (t) esss->allowevents &= ~DISABLE_COLLIDE_NPC;
						else esss->allowevents |= DISABLE_COLLIDE_NPC;
					}

					if (!strcmp(temp, "CHAT"))
					{
						if (t) esss->allowevents &= ~DISABLE_CHAT;
						else esss->allowevents |= DISABLE_CHAT;
					}

					if (!strcmp(temp, "HIT"))
					{
						if (t) esss->allowevents &= ~DISABLE_HIT;
						else esss->allowevents |= DISABLE_HIT;
					}

					if (!strcmp(temp, "INVENTORY2_OPEN"))
					{
						if (t) esss->allowevents &= ~DISABLE_INVENTORY2_OPEN ;
						else esss->allowevents |= DISABLE_INVENTORY2_OPEN ;
					}

					if (!strcmp(temp, "DETECTPLAYER"))
					{
						if (t) esss->allowevents &= ~DISABLE_DETECT ;
						else esss->allowevents |= DISABLE_DETECT;
					}

					if (!strcmp(temp, "HEAR"))
					{
						if (t) esss->allowevents &= ~DISABLE_HEAR ;
						else esss->allowevents |= DISABLE_HEAR ;
					}

					if (!strcmp(temp, "AGGRESSION"))
					{
						if (t) esss->allowevents &= ~DISABLE_AGGRESSION ;
						else esss->allowevents |= DISABLE_AGGRESSION ;
					}

					if (!strcmp(temp, "MAIN"))
					{
						if (t) esss->allowevents &= ~DISABLE_MAIN ;
						else esss->allowevents |= DISABLE_MAIN ;
					}

					if (!strcmp(temp, "CURSORMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_CURSORMODE ;
						else esss->allowevents |= DISABLE_CURSORMODE ;
					}

					if (!strcmp(temp, "EXPLORATIONMODE"))
					{
						if (t) esss->allowevents &= ~DISABLE_EXPLORATIONMODE ;
						else esss->allowevents |= DISABLE_EXPLORATIONMODE ;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_EVENT %s %s", temp, temp2);

#endif
				}
				else if (!strcmp(temp, "SETPRICE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io)
					{
						if (io->ioflags & IO_ITEM)
						{
							io->_itemdata->price = (long)GetVarValueInterpretedAsFloat(temp, esss, io);

							if (io->_itemdata->price < 0) io->_itemdata->price = 0;
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SET_PRICE %s", temp);

#endif
				}
				else if (!strcmp(temp, "SETINTERNALNAME"))
				{
					pos = GetNextWord(es, pos, temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ERROR: SETINTERNALNAME %s - NOT AN IO !!!", temp);

#endif
				}
				else if (!strcmp(temp, "SHOWGLOBALS"))
				{
					strcpy(ShowText, "");
					MakeGlobalText(ShowText);
					strcpy(ShowTextWindowtext, "Global Variables");

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWGLOBALS");

#endif
				}
				else if (!strcmp(temp, "SHOWLOCALS"))
				{
					strcpy(ShowText, "");
					MakeLocalText(es, ShowText);
					strcpy(ShowTextWindowtext, "Local Variables");

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWTEXT, NULL, (DLGPROC)ShowTextDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWLOCALS");

#endif
				}
				else if (!strcmp(temp, "SHOWVARS"))
				{
					strcpy(ShowText, "");
					strcpy(ShowText2, "");
					MakeGlobalText(ShowText);
					MakeLocalText(es, ShowText2);
					strcpy(ShowTextWindowtext, "Variables");

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
						DialogBox(hInstance, (LPCTSTR)IDD_SHOWVARS, NULL, (DLGPROC)ShowVarsDlg);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "SHOWVARS");

#endif
				}
				else if (!strcmp(temp, "SETIRCOLOR"))
				{
					if (io != NULL)
					{
						char temp1[64];
						char temp2[64];
						char temp3[64];
						float t1, t2, t3;

						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);
						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						t2 = GetVarValueInterpretedAsFloat(temp2, esss, io);
						t3 = GetVarValueInterpretedAsFloat(temp3, esss, io);
						io->infracolor.r = t1;
						io->infracolor.g = t2;
						io->infracolor.b = t3;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "SETSCALE"))
				{
					if (io != NULL)
					{
						char temp1[64];
						float t1;
						pos = GetNextWord(es, pos, temp1);
						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						io->scale = t1 * DIV100;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SET_SCALE %s", temp1);

#endif
					}
				}
				else if (!strcmp(temp, "STEALNPC"))
				{
					if (player.Interface & INTER_STEAL)
					{
						SendIOScriptEvent(ioSteal, SM_STEAL, "OFF");
					}

					player.Interface |= INTER_STEAL;
					InventoryDir = 1;
					ioSteal = io;
				}
				else if (!strcmp(temp, "SPECIALFX"))
				{
					char temp1[256];
					pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "SPECIAL_FX ");
						strcat(cmd, temp);
					}

#endif

					if (!stricmp(temp1, "YLSIDE_DEATH"))
					{
						SetYlsideDeath(io);
					}
					else if (!stricmp(temp1, "PLAYER_APPEARS"))
					{
						MakePlayerAppearsFX(io);
					}
					else if (!strcmp(temp1, "HEAL"))
					{
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif

						if (!BLOCK_PLAYER_CONTROLS)
							player.life += (float)atof(temp1);

						if (player.life > player.Full_maxlife) player.life = player.Full_maxlife;

						if (player.life < 0.f) player.life = 0.f;
					}
					else if (!strcmp(temp1, "MANA"))
					{
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif
						player.mana += (float)atof(temp1);

						if (player.mana > player.Full_maxmana) player.mana = player.Full_maxmana;

						if (player.mana < 0.f) player.mana = 0.f;
					}
					else if (!strcmp(temp1, "NEWSPELL"))
					{
						MakeBookFX(DANAESIZX - INTERFACE_RATIO(35), DANAESIZY - INTERFACE_RATIO(148), 0.00001f);
						pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp1);
						}

#endif
					}
					else if (!strcmp(temp1, "TORCH"))
					{
						if ((io) && (io->ioflags & IO_ITEM))
						{
							INTERACTIVE_OBJ * ioo = io;

							if (io->_itemdata->count > 1)
							{
								ioo = CloneIOItem(io);
								MakeTemporaryIOIdent(ioo);
								ioo->show = SHOW_FLAG_IN_INVENTORY;
								ioo->scriptload = 1;
								ioo->_itemdata->count = 1;
								io->_itemdata->count--;
							}

							ARX_PLAYER_ClickedOnTorch(ioo);
						}
					}
					else if (!strcmp(temp1, "FIERY"))
					{
						io->ioflags |= IO_FIERY;
					}
					else if (!strcmp(temp1, "FIERYOFF"))
					{
						io->ioflags &= ~IO_FIERY;
					}
					else if (!strcmp(temp1, "TORCHON"))
					{
						//DO NOTHING
					}
					else if (!strcmp(temp1, "TORCHOFF"))
					{
						if (CURRENT_TORCH)
							ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
					}
				}
				else if (!strcmp(temp, "SETBUMP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags |= IO_BUMP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETBUMP ON");

#endif
					}
					else if (!stricmp(temp, "OFF"))
					{
						io->ioflags &= ~IO_BUMP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETBUMP OFF");

#endif
					}
				}
				else if (!strcmp(temp, "SETZMAP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags |= IO_ZMAP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETMAP ON");

#endif
					}
					else if (!stricmp(temp, "OFF"))
					{
						io->ioflags &= ~IO_ZMAP;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "SETMAP OFF");

#endif
					}
				}

				break;
			case 'Z':

				if (!strcmp(temp, "ZONEPARAM"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "ZONE_PARAM ");
						strcat(cmd, temp);
					}

#endif

					if (!stricmp(temp, "STACK"))
					{
						ARX_GLOBALMODS_Stack();
					}
					else if (!stricmp(temp, "UNSTACK"))
					{
						ARX_GLOBALMODS_UnStack();
					}
					else
					{
						if (temp[0] == '-')
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
						}

						if (!stricmp(temp, "RGB"))
						{
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.r = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.g = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp);
							desired.depthcolor.b = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							desired.flags |= GMOD_DCOLOR;
						}
						else if (!stricmp(temp, "ZCLIP"))
						{
							pos = GetNextWord(es, pos, temp);
							desired.zclip = GetVarValueInterpretedAsFloat(temp, esss, io);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							desired.flags |= GMOD_ZCLIP;
						}
						else if (!stricmp(temp, "AMBIANCE"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							ARX_SOUND_PlayZoneAmbiance(temp);
						}

					}
				}

				break;
			case 'K':

				if (!strcmp(temp, "KILLME"))
				{
					if (io) // can only kill IOs
					{
						if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1))
						{
							io->_itemdata->count--;
						}
						else
						{
							io->show = SHOW_FLAG_KILLED;
							io->GameFlags &= ~GFLAG_ISINTREATZONE;
							RemoveFromAllInventories(io);
							ARX_DAMAGES_ForceDeath(io, EVENT_SENDER);
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "KILLME");

#endif
				}
				else if (!strcmp(temp, "KEYRINGADD"))
				{
					char temp2[256];
					pos = GetNextWord(es, pos, temp2);
					strcpy(temp, GetVarValueInterpretedAsText(temp2, esss, io));
					ARX_KEYRING_Add(temp);
				}

				break;
			case 'F':

				if (!strcmp(temp, "FORCEANIM"))
				{
					char temp2[256];
					long num;
					pos = GetNextWord(es, pos, temp2);
					num = GetNumAnim(temp2);

					if (num > -1)
					{
						if (io != NULL)
							if (io->anims[num] != NULL)
							{
								ForceAnim(io, io->anims[num]);
								CheckSetAnimOutOfTreatZone(io, 0);
							}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCEANIM %s", temp2);

#endif
				}
				else if (!strcmp(temp, "FORCEANGLE"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io != NULL)
					{

						io->angle.b = MAKEANGLE(GetVarValueInterpretedAsFloat(temp, esss, io));
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCEANGLE %s", temp);

#endif
				}
				else if (!strcmp(temp, "FORCEDEATH"))
				{
					char temp2[256];
					long t;
					pos = GetNextWord(es, pos, temp2);
					MakeUpcase(temp2);

					if ((!strcmp(temp2, "ME")) || (!strcmp(temp2, "SELF")))
					{
						t = GetInterNum(io);
					}
					else
					{
						t = GetTargetByNameTarget(temp2);

						if (t == -2) t = GetInterNum(io);
					}

					if (t > 0)
					{
						ARX_DAMAGES_ForceDeath(inter.iobj[t], io);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "FORCE_DEATH %s", temp2);

#endif
				}

				break;
			case 'P':

				if (!strcmp(temp, "PLAYERLOOKAT"))
				{
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io);

					if (ValidIONum(t))
					{
						ForcePlayerLookAtIO(inter.iobj[t]);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PLAYERLOOKAT %s", temp);

#endif
				}
				else if (!strcmp(temp, "PLAYERSTACKSIZE"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_ITEM))
					{
						io->_itemdata->playerstacksize = (short)GetVarValueInterpretedAsFloat(temp, esss, io);

						if (io->_itemdata->playerstacksize < 1)
							io->_itemdata->playerstacksize = 1;

						if (io->_itemdata->playerstacksize > 100)
							io->_itemdata->playerstacksize = 100;
					}
				}
				else if (!strcmp(temp, "PRECAST"))
				{
					char temp2[256];
					long duration = -1;
					long flags = 0;
					long dur = 0;
					pos = GetNextWord(es, pos, temp); // switch or level

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'D'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							pos = GetNextWord(es, pos, temp2); // duration
							F2L(GetVarValueInterpretedAsFloat(temp2, esss, io), &duration);
							dur = 1;
						}

						if (iCharIn(temp, 'F'))
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
							flags |= SPELLCAST_FLAG_NOMANA;
						}

						pos = GetNextWord(es, pos, temp); // level
					}

					long level;
					F2L(GetVarValueInterpretedAsFloat(temp, esss, io), &level);

					if (level < 1) level = 1;
					else if (level > 10) level = 10;

					pos = GetNextWord(es, pos, temp); //spell id
					long spellid;
					spellid = GetSpellId(temp);

					if (spellid != -1)
					{
						flags |= SPELLCAST_FLAG_PRECAST;

						if (!dur)
							duration = 2000 + level * 2000;

						if (io != inter.iobj[0])
						{
							flags |= SPELLCAST_FLAG_NOCHECKCANCAST;
						}

						flags |= SPELLCAST_FLAG_NOANIM;
						TryToCastSpell(inter.iobj[0], spellid, level, -1, flags, duration);
					}
				}
				else if (!strcmp(temp, "POISON"))
				{
					pos = GetNextWord(es, pos, temp);
					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);
					ARX_PLAYER_Poison(fval);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POISON %s", temp);

#endif
				}
				else if (!strcmp(temp, "PLAYERMANADRAIN"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
						player.playerflags &= ~PLAYERFLAGS_NO_MANA_DRAIN;
					else
						player.playerflags |= PLAYERFLAGS_NO_MANA_DRAIN;
				}
				else if (!strcmp(temp, "PATHFIND"))
				{
					char temp2[128];
					pos = GetNextWord(es, pos, temp2);
					long t = GetTargetByNameTarget(temp2);
					ARX_NPC_LaunchPathfind(io, t);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "PATHFIND %s", temp2);

#endif
				}
				else if (!strcmp(temp, "PLAYANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					char temp2[256];
					long num;
					long nu = 0;
					long loop = 0;
					long execute = 0;
					long nointerpol = 0;
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAY_ANIM ");
						strcat(cmd, temp);
					}

#endif

					if (temp2[0] == '-')
					{
						if (CharIn(temp2, '1')) nu = 0;

						if (CharIn(temp2, '2')) nu = 1;

						if (CharIn(temp2, '3')) nu = 2;

						if (iCharIn(temp2, 'L')) loop = 1;

						if (iCharIn(temp2, 'N')) nointerpol = 1;

						if (iCharIn(temp2, 'E')) execute = 1;

						if (iCharIn(temp2, 'P'))
						{
							iot = inter.iobj[0];
							iot->move.x = iot->lastmove.x = 0.f;
							iot->move.y = iot->lastmove.y = 0.f;
							iot->move.z = iot->lastmove.z = 0.f;

						}

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					if (!stricmp(temp2, "NONE"))
					{
						if (iot != NULL)
						{
							iot->animlayer[nu].cur_anim = NULL;
							iot->animlayer[nu].next_anim = NULL;
						}
					}
					else
					{
						num = GetNumAnim(temp2);

						if (num > -1)
						{
							if (iot != NULL)
								if (iot->anims[num] != NULL)
								{
									iot->ioflags |= IO_NO_PHYSICS_INTERPOL;
									SetNextAnim(iot, iot->anims[num], nu, loop, nointerpol);

									if (!loop)
										CheckSetAnimOutOfTreatZone(iot, nu);

									{
										if (iot == inter.iobj[0])
											iot->animlayer[nu].flags &= ~EA_STATICANIM;

										if (execute)
										{
											char timername[64];
											char timername2[64];
											ARX_SCRIPT_Timer_GetDefaultName(timername2);
											sprintf(timername, "ANIM_%s", timername2);
											long num2 = ARX_SCRIPT_Timer_GetFree();

											if (num2 > -1)
											{
												memset(&scr_timer[num2], 0, sizeof(SCR_TIMER));
												ActiveTimers++;
												scr_timer[num2].es = es;
												scr_timer[num2].exist = 1;
												scr_timer[num2].io = io;
												scr_timer[num2].msecs = (long)__max(iot->anims[num]->anims[iot->animlayer[nu].altidx_cur]->anim_time, 1000);
												scr_timer[num2].namelength = strlen(timername) + 1;
												scr_timer[num2].name = (char *)malloc(scr_timer[num2].namelength);
												strcpy(scr_timer[num2].name, timername);
												scr_timer[num2].pos = pos;
												scr_timer[num2].tim = ARXTimeUL();
												scr_timer[num2].times = 1;
												scr_timer[num2].longinfo = 0; //numsound;
											}

											pos = GotoNextLine(es, pos);
										}
									}
								}
						}
					}
				}
				else if (!strcmp(temp, "PLAYERINTERFACE"))
				{
					char temp2[256];
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAYER_INTERFACE ");
						strcat(cmd, temp);
					}

#endif
					long smooth = 0;

					if (temp2[0] == '-')
					{
						if ((temp2[1] == 's') || (temp2[1] == 'S'))
						{
							smooth = 1;
						}

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					if (!stricmp(temp2, "HIDE"))
						ARX_INTERFACE_PlayerInterfaceModify(0, smooth);

					if (!stricmp(temp2, "SHOW"))
						ARX_INTERFACE_PlayerInterfaceModify(1, smooth);

				}
				else if (!strcmp(temp, "PLAY"))
				{
					unsigned long loop(ARX_SOUND_PLAY_ONCE);
					char temp2[256];
					float pitch(1.0F);
					bool unique(false);
					bool stop(false);
					bool no_pos(false);

					pos = GetNextWord(es, pos, temp2);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "PLAY ");
						strcat(cmd, temp);
					}

#endif

					if (temp2[0] == '-')
					{
						if (iCharIn(temp2, 'I')) unique = true;
						else if (iCharIn(temp2, 'L')) loop = ARX_SOUND_PLAY_LOOPED;
						else if (iCharIn(temp2, 'P')) pitch = 0.9F + 0.2F * rnd();
						else if (iCharIn(temp2, 'S')) stop = true;
						else if (iCharIn(temp2, 'O')) no_pos = true;

						pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp2);
						}

#endif
					}

					char * temp1 = GetVarValueInterpretedAsText(temp2, esss, io);

					if (io)
					{
						SetExt(temp1, ".wav");

						if (stop)
						{
							ARX_SOUND_Stop(io->sound);
							io->sound = ARX_SOUND_INVALID_RESOURCE;
						}
						else
						{
							if (unique && io->sound != ARX_SOUND_INVALID_RESOURCE)
								ARX_SOUND_Stop(io->sound);

							long num;

							if (no_pos || SM_INVENTORYUSE == msg)
								num = ARX_SOUND_PlayScript(temp1, NULL, pitch, loop);
							else
								num = ARX_SOUND_PlayScript(temp1, io, pitch, loop);

							if (unique) io->sound = num;

#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								if (num == ARX_SOUND_INVALID_RESOURCE)
									sprintf(cmd, "PLAY %s - Success DanaePlaySample", temp1);
								else
									sprintf(cmd, "PLAY %s - UNABLE TO LOAD FILE", temp1);
							}

#endif
						}
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) strcat(cmd, "ERROR: - NOT AN IO !!!");

#endif
				}
				else if (!strcmp(temp, "PLAYSPEECH"))
				{
					char temp2[256];

					pos = GetNextWord(es, pos, temp2);

					long num = ARX_SOUND_PlaySpeech(temp2, io && io->show == 1 ? io : NULL);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (num == ARX_SOUND_INVALID_RESOURCE)
							sprintf(cmd, "PLAYSPEECH %s - UNABLE TO LOAD FILE", temp2);
						else
							sprintf(cmd, "PLAYSPEECH %s - Success DanaePlaySample", temp2);
					}

#endif
				}
				else if (!strcmp(temp, "POPUP"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!(danaeApp.kbd.inkey[INKEY_LEFTSHIFT]) && !(danaeApp.kbd.inkey[INKEY_RIGHTSHIFT]))
					{
						ARX_TIME_Pause();
						ShowPopup(temp);
						ARX_TIME_UnPause();
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "POPUP %s", temp);

#endif
				}
				else if (!strcmp(temp, "PHYSICAL"))
				{
					char temp2[64];
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags &= ~IO_PHYSICAL_OFF;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL ON");

#endif
					}
					else if (!stricmp(temp, "OFF"))
					{
						io->ioflags |= IO_PHYSICAL_OFF;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL OFF");

#endif
					}
					else
					{
						pos = GetNextWord(es, pos, temp2);
						float fval = GetVarValueInterpretedAsFloat(temp2, esss, io);

						if (!stricmp(temp, "HEIGHT"))
						{
							if (io)
							{
								io->original_height = -fval;

								if (io->original_height > -30) io->original_height = -30;

								if (io->original_height < -165) io->original_height = -165;

								io->physics.cyl.height = io->original_height * io->scale;
							}
						}
						else if (!stricmp(temp, "RADIUS"))
						{
							if (io)
							{
								io->original_radius = fval;

								if (io->original_radius < 10) io->original_radius = 10;
								else if (io->original_radius > 40) io->original_radius = 40;

								io->physics.cyl.radius = io->original_radius * io->scale;

							}
						}

#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "PHYSICAL %s %s", temp, temp2);

#endif
					}
				}

				break;
			case 'L':

				if (!strcmp(temp, "LOADANIM"))
				{
					INTERACTIVE_OBJ * iot = io;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							iot = inter.iobj[0];

						pos = GetNextWord(es, pos, temp);
					}

					char temp2[256];
					long flag;

					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LOADANIM %s %s", temp, temp2);

#endif

					if (iot != NULL)
					{
						long num = -1;
						MakeUpcase(temp);
						num = GetNumAnim(temp);

						if ((num > -1) && (num < MAX_ANIMS))
						{


							if (iot->anims[num] != NULL)
							{
								ReleaseAnimFromIO(iot, num);
							}

							if (iot->anims[num] == NULL)
							{
								char tex2[256];
								char tex3[256];

								if ((iot == inter.iobj[0]) || (iot->ioflags & IO_NPC))
								{
									flag = TEA_NPC_SAMPLES;
									sprintf(tex3, "%sGraph\\Obj3D\\Anims\\npc\\%s", Project.workingdir, temp2);
								}
								else
								{
									flag = TEA_FIX_SAMPLES;
									sprintf(tex3, "%sGraph\\Obj3D\\Anims\\Fix_Inter\\%s", Project.workingdir, temp2);
								}

								SetExt(tex3, ".tea");
								File_Standardize(tex3, tex2);

								if (PAK_FileExist(tex2))
								{
									iot->anims[num] = EERIE_ANIMMANAGER_Load(tex2);

									if (iot->anims[num] == NULL)
									{
										char ttmp[512];
										sprintf(ttmp, "LOADANIM %s %s FAILED", temp, temp2);
										ForceSendConsole(ttmp, 1, 0, (HWND)1);
									}
								}
							}
							else
							{
								char ttmp[512];
								sprintf(ttmp, "LOADANIM %s %s FAILED", temp, temp2);
								ForceSendConsole(ttmp, 1, 0, (HWND)1);
							}
						}
					}
				}
				else if (!strcmp(temp, "LINKOBJTOME"))
				{
					pos = GetNextWord_Interpreted(io, es, pos, temp);
					long t = GetTargetByNameTarget(temp);
					pos = GetNextWord(es, pos, temp);

					if (ValidIONum(t))
						LinkObjToMe(io, inter.iobj[t], temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "LINKOBJTOME %d %s", t, temp);

#endif
				}

				break;
			case 'I':

				if ((temp[1] == 'F') && (temp[2] == 0))
				{
					const unsigned int tvSize = 256 ;
					char	temp3[tvSize];
					short	oper				= 0;
					short	failed				= 0;
					short	typ1, typ2;
					char	tvar1[tvSize], tvar2[tvSize];
					float	fvar1, fvar2;
					char *	tempo;

					fvar1 = fvar2 = 0;
					pos = GetNextWord(es, pos, temp);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "IF (");
						strcat(cmd, temp);
					}

#endif

					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
					}

#endif

					if	(!strcmp(temp3, "=="))			oper = OPER_EQUAL;
					else if (!strcmp(temp3, "!="))			oper = OPER_NOTEQUAL;
					else if (!strcmp(temp3, "<="))			oper = OPER_INFEQUAL;
					else if (!strcmp(temp3, "<"))			oper = OPER_INFERIOR;
					else if (!strcmp(temp3, ">="))			oper = OPER_SUPEQUAL;
					else if (!strcmp(temp3, ">"))			oper = OPER_SUPERIOR;
					else if	(!stricmp(temp3, "ISCLASS"))	oper = OPER_INCLASS;
					else if	(!stricmp(temp3, "ISELEMENT"))	oper = OPER_ISELEMENT;
					else if	(!stricmp(temp3, "ISIN"))		oper = OPER_ISIN;
					else if	(!stricmp(temp3, "ISTYPE"))		oper = OPER_ISTYPE;
					else if	(!stricmp(temp3, "ISGROUP"))	oper = OPER_ISGROUP;
					else if	(!stricmp(temp3, "!ISGROUP"))	oper = OPER_NOTISGROUP;


					pos = GetNextWord(es, pos, temp3);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp3);
						strcat(cmd, ") ");
					}

#endif

					switch (temp[0])
					{
						case '^':
							
							long lv; float fv; char tv[tvSize];	//Arx: xrichter (2010-08-04) - fix a crash when $OBJONTOP return to many object name inside tv
							switch ( GetSystemVar( esss, io, temp, tv,tvSize, &fv, &lv ) )
							{
								case TYPE_TEXT:
									typ1	=	TYPE_TEXT;
									strcpy(tvar1, tv);
									break;
								case TYPE_LONG:
									typ1	=	TYPE_FLOAT;
									fvar1	=	(float)lv;
									break;
								case TYPE_FLOAT:
									typ1	=	TYPE_FLOAT;
									fvar1	=	fv;
									break;

								default:
									ARX_CHECK_NO_ENTRY(); //typ1 are not initialized
									typ1	=	TYPE_TEXT;
									break;

							}

							break;
						case '#':
							typ1	=	TYPE_FLOAT;
							fvar1	=	(float)GETVarValueLong(&svar, &NB_GLOBALS, temp);
							break;
						case '§':
							typ1	=	TYPE_FLOAT;
							fvar1	=	(float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp);
							break;
						case '&':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(&svar, &NB_GLOBALS, temp);
							break;
						case '@':
							typ1	=	TYPE_FLOAT;
							fvar1	=	GETVarValueFloat(&esss->lvar, &esss->nblvar, temp);
							break;
						case '$':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(&svar, &NB_GLOBALS, temp);

							if (tempo == NULL) tvar1[0] = 0;
							else strcpy(tvar1, tempo);

							break;
						case '£':
							typ1	=	TYPE_TEXT;
							tempo	=	GETVarValueText(&esss->lvar, &esss->nblvar, temp);

							if (tempo == NULL) tvar1[0] = 0;
							else strcpy(tvar1, tempo);

							break;
						default:

							if ((oper == OPER_ISTYPE) || (oper == OPER_ISGROUP) || (oper == OPER_NOTISGROUP))
							{
								typ1 =	TYPE_TEXT;
								strcpy(tvar1, temp);
							}
							else
							{
								typ1 =	TYPE_FLOAT;
								fvar1 =	(float)atof(temp);
							}
					}

					switch ( temp3[0] )
					{
						case '^':
							
							long lv; float fv; char tv[tvSize];
							switch ( GetSystemVar( esss, io, temp3, tv,tvSize, &fv, &lv ) )
							{
								case TYPE_TEXT:
									typ2	=	TYPE_TEXT;
									strcpy(tvar2, tv);
									break;
								case TYPE_LONG:
									typ2	=	TYPE_FLOAT;
									fvar2	=	(float)lv;
									break;
								case TYPE_FLOAT:
									typ2	=	TYPE_FLOAT;
									fvar2	=	fv;
									break;

								default:
									ARX_CHECK_NO_ENTRY(); //typ1 is not initialized
									typ2	=	TYPE_TEXT;
									break;

							}

							break;
						case '#':
							typ2			=	TYPE_FLOAT;
							fvar2			=	(float)GETVarValueLong(&svar, &NB_GLOBALS, temp3);
							break;
						case '§':
							typ2			=	TYPE_FLOAT;
							fvar2			=	(float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp3);
							break;
						case '&':
							typ2			=	TYPE_FLOAT;
							fvar2			=	GETVarValueFloat(&svar, &NB_GLOBALS, temp3);
							break;
						case '@':
							typ2			=	TYPE_FLOAT;
							fvar2			=	GETVarValueFloat(&esss->lvar, &esss->nblvar, temp3);
							break;
						case '$':
							typ2			=	TYPE_TEXT;
							tempo			=	GETVarValueText(&svar, &NB_GLOBALS, temp3);

							if (tempo == NULL) tvar2[0] = 0;
							else strcpy(tvar2, tempo);

							break;
						case '£':
							typ2			=	TYPE_TEXT;
							tempo			=	GETVarValueText(&esss->lvar, &esss->nblvar, temp3);

							if (tempo == NULL) tvar2[0] = 0;
							else strcpy(tvar2, tempo);

							break;
						default:

							if (typ1 == TYPE_TEXT)
							{
								typ2		=	TYPE_TEXT;
								strcpy(tvar2, temp3);
							}
							else
							{
								typ2		=	TYPE_FLOAT;
								fvar2		=	(float)atof(temp3);
							}
					}

					failed = 0;

					switch (oper)
					{
						case OPER_ISELEMENT:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (IsElement(tvar1, tvar2)) failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_INCLASS:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if ((IsIn(tvar2, tvar1)) || (IsIn(tvar1, tvar2))) failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_ISGROUP:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									long t = GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									if (ValidIONum(t))
									{
										if (IsIOGroup(inter.iobj[t], tvar2)) failed = 0;
										else failed = 1;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_NOTISGROUP:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);

									long t = GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									if (ValidIONum(t))
									{
										if (IsIOGroup(inter.iobj[t], tvar2)) failed = 1;
										else failed = 0;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_ISTYPE:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);

									long t		=	GetTargetByNameTarget(tvar1);

									if (t == -2) t = GetInterNum(io);

									long flagg	=	ARX_EQUIPMENT_GetObjectTypeFlag(tvar2);

									if ((flagg != 0) && (ValidIONum(t)))
									{
										if (inter.iobj[t]->type_flags & flagg) failed = 0;
										else failed = 1;
									}
									else failed = 1;
								}
							}

							break;
						case OPER_ISIN:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (IsIn(tvar2, tvar1))  failed = 0;
									else failed = 1;
								}
							}

							break;
						case OPER_EQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									MakeUpcase(tvar1);
									MakeUpcase(tvar2);

									if (strcmp(tvar1, tvar2)) failed = 1;
								}
								else
								{
									if (fvar1 != fvar2) failed = 1;
								}
							}

							break;
						case OPER_NOTEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)
								{
									if (strcmp(tvar1, tvar2)) failed = 1;
								}
								else
								{
									if (fvar1 != fvar2) failed = 1;
								}
							}

							if (failed) failed = 0;
							else failed = 1;

							break;
						case OPER_INFEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)	 failed = 1;
								else
								{
									if (fvar1 > fvar2) failed = 1;
								}
							}

							break;
						case OPER_INFERIOR:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 >= fvar2)	failed = 1;
								}
							}

							break;
						case OPER_SUPEQUAL:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 < fvar2)	failed = 1;
								}
							}

							break;
						case OPER_SUPERIOR:

							if (typ1 != typ2) failed = 1;
							else
							{
								if (typ1 == TYPE_TEXT)		failed = 1;
								else
								{
									if (fvar1 <= fvar2)	failed = 1;
								}
							}

							break;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) strcat(cmd, " -> FALSE");
						else strcat(cmd, " -> TRUE");
					}

#endif
				}
				else if (!strcmp(temp, "INC"))
				{
					char temp1[64];
					char temp2[64];
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "INC ");
						strcat(cmd, temp1);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
					}

#endif

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '£': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&svar, &NB_GLOBALS, temp1);
							fval = fdval + fval;
							sv = SETVarValueLong(&svar, &NB_GLOBALS, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_G_LONG;

							break;
						case '§': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);
							fval = fdval + fval;
							sv = SETVarValueLong(&esss->lvar, &esss->nblvar, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&svar, &NB_GLOBALS, temp1);
							fval = fdval + fval;
							sv = SETVarValueFloat(&svar, &NB_GLOBALS, temp1, fval);

							if (sv != NULL) sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);
							fval = fdval + fval;
							sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval);

							if (sv != NULL) sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif
				}
				else if (!strcmp(temp, "IFEXISTINTERNAL"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (t != -1)
					{
						failed = 0;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) sprintf(cmd, "IFEXISTINTERNAL (%s) -> FALSE", temp);
						else sprintf(cmd, "IFEXISTINTERNAL (%s) -> TRUE", temp);
					}

#endif
				}
				else if (!strcmp(temp, "INVULNERABILITY"))
				{
					pos = GetNextWord(es, pos, temp);
					long player = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'P'))
							player = 1;

						pos = GetNextWord(es, pos, temp);
					}

					if (!stricmp(temp, "ON"))
					{
						if (player)
							ARX_PLAYER_Invulnerability(1);
						else
							io->ioflags |= IO_INVULNERABILITY;
					}
					else
					{
						if (player)
							ARX_PLAYER_Invulnerability(0);
						else
							io->ioflags &= ~IO_INVULNERABILITY;
					}
				}
				else if (!strcmp(temp, "INVERTEDOBJECT"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "ON"))
					{
						io->ioflags |= IO_INVERTED;
					}
					else io->ioflags &= ~IO_INVERTED;
				}
				else if (!strcmp(temp, "IFVISIBLE"))
				{
					long failed = 1;
					pos = GetNextWord(es, pos, temp);
					long t = GetTargetByNameTarget(temp);

					if (ValidIONum(t))
					{
						if (HasVisibility(io, inter.iobj[t]))
							failed = 0;
					}

					if (failed)
					{
						pos = SkipNextStatement(es, pos);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						if (failed) sprintf(cmd, "IFVISIBLE (%s) -> FALSE", temp);
						else sprintf(cmd, "IFVISIBLE (%s) -> TRUE", temp);
					}

#endif
				}
				else if (!strcmp(temp, "INVENTORY"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "INVENTORY ");
						strcat(cmd, temp);
					}

#endif
					MakeStandard(temp);
					long ion;
					ion = GetInterNum(io);

					if ((io != NULL) && (ion != -1))
						if (!stricmp(temp, "CREATE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								INVENTORY_DATA * id = (INVENTORY_DATA *)inter.iobj[ion]->inventory;

								for (long nj = 0; nj < id->sizey; nj++)
									for (long ni = 0; ni < id->sizex; ni++)
									{
										if (id->slot[ni][nj].io != NULL)
										{
											long tmp = GetInterNum(id->slot[ni][nj].io);

											if (tmp != -1)
											{
												if (inter.iobj[tmp]->scriptload)
												{
													RemoveFromAllInventories(inter.iobj[tmp]);
													ReleaseInter(inter.iobj[tmp]);
													inter.iobj[tmp] = NULL;
												}
												else inter.iobj[tmp]->show = SHOW_FLAG_KILLED;
											}

											id->slot[ni][nj].io = NULL;
										}
									}

								free(io->inventory);
								inter.iobj[ion]->inventory = NULL;
							}

							if (inter.iobj[ion]->inventory == NULL)
							{
								inter.iobj[ion]->inventory = malloc(sizeof(INVENTORY_DATA));
								memset(inter.iobj[ion]->inventory, 0, sizeof(INVENTORY_DATA));
								INVENTORY_DATA * id = (INVENTORY_DATA *)inter.iobj[ion]->inventory;
								id->sizex = 3;
								id->sizey = 11;
								id->io = inter.iobj[ion];
							}
						}
						else if (!strcmp(temp, "SKIN"))
						{
							char temp2[256];
							pos = GetNextWord(es, pos, temp2);

							if (io)
							{
								if (io->inventory_skin) free(io->inventory_skin);

								io->inventory_skin = strdup(temp2);
							}
						}
						else if (!strcmp(temp, "PLAYERADDFROMSCENE"))
						{
							char temp2[256];
							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							if (!RELOADING)
							{
								long t = GetTargetByNameTarget(temp2);

								if (t == -2) t = GetInterNum(io);

								if (ValidIONum(t))
								{
									RemoveFromAllInventories(inter.iobj[t]);
									inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;

									if (!CanBePutInInventory(inter.iobj[t]))
									{
										PutInFrontOfPlayer(inter.iobj[t], 1);
									}
								}
							}
						}
						else if ((!strcmp(temp, "PLAYERADD")) || (!strcmp(temp, "PLAYERADDMULTI")))
						{
							{
								char temp2[256];
								char tex[256];
								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif

								if (RELOADING)
								{
									if (!strcmp(temp, "PLAYERADDMULTI"))
									{
										pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

										if (NEED_DEBUG)
										{
											strcat(cmd, " ");
											strcat(cmd, temp2);
										}

#endif
									}
								}
								else
								{
									char tex2[256];
									sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\Items\\%s.teo", Project.workingdir, temp2);
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(GDevice, tex, IO_IMMEDIATELOAD);

										if (ioo != NULL)
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);

											if (!strcmp(temp, "PLAYERADDMULTI"))
											{
												pos = GetNextWord(es, pos, temp2);

												if (ioo->ioflags & IO_GOLD)
												{
													ioo->_itemdata->price = atoi(temp2);
												}
												else
												{
													ioo->_itemdata->maxcount = 9999;

													int iTemp = atoi(temp2);
													ARX_CHECK_SHORT(iTemp);

													ioo->_itemdata->count = ARX_CLEAN_WARN_CAST_SHORT(iTemp);


													if (ioo->_itemdata->count < 1) ioo->_itemdata->count = 1;
												}
											}

											ioo->show = SHOW_FLAG_IN_INVENTORY;

											if (!CanBePutInInventory(ioo))
											{
												PutInFrontOfPlayer(ioo, 1);
											}
										}
									}
									else
									{
										if (!strcmp(temp, "PLAYERADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(temp, "ADDFROMSCENE"))
						{
							long xx, yy;
							char temp2[256];

							pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp2);
							}

#endif

							if (!RELOADING)
							{
								long t = GetTargetByNameTarget(temp2);

								if (t == -2) t = GetInterNum(io);

								if (ValidIONum(t))
								{
									if (ARX_EQUIPMENT_IsPlayerEquip(inter.iobj[t]))
									{
										ARX_EQUIPMENT_UnEquip(inter.iobj[0], inter.iobj[t], 1);
									}
									else
									{
										RemoveFromAllInventories(inter.iobj[t]);
									}

									inter.iobj[t]->scriptload = 0;
									inter.iobj[t]->show = SHOW_FLAG_IN_INVENTORY;

									if (!CanBePutInSecondaryInventory((INVENTORY_DATA *)inter.iobj[ion]->inventory, inter.iobj[t], &xx, &yy))
									{
										PutInFrontOfPlayer(inter.iobj[t], 1);
									}
								}
							}

						}
						else if ((!strcmp(temp, "ADD")) || (!strcmp(temp, "ADDMULTI")))
						{
							char temp2[256];

							if (inter.iobj[ion]->inventory == NULL)
							{
								pos = GetNextWord(es, pos, temp2);

								if (!strcmp(temp, "ADDMULTI"))
									pos = GetNextWord(es, pos, temp2);
							}
							else if (inter.iobj[ion]->inventory != NULL)
							{
								char tex[256];

								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif

								if (RELOADING)
								{
									if (!strcmp(temp, "ADDMULTI"))
									{
										pos = GetNextWord(es, pos, temp2);
									}
								}
								else
								{
									char tex2[256];
									sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\Items\\%s.teo", Project.workingdir, temp2);
									File_Standardize(tex2, tex);

									if (FORBID_SCRIPT_IO_CREATION == 0)
									{
										long multi = -1;

										if (!strcmp(temp, "ADDMULTI"))
										{
											pos = GetNextWord(es, pos, temp2);
											multi = atoi(temp2);
										}

										INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)AddItem(GDevice, tex, IO_IMMEDIATELOAD);
										long xx, yy;

										if ((ioo != NULL)
										        &&	(multi != 0))
										{
											LASTSPAWNED = ioo;
											ioo->scriptload = 1;
											MakeTemporaryIOIdent(ioo);
											SendInitScriptEvent(ioo);


											if (!strcmp(temp, "ADDMULTI"))
											{
												if (ioo->ioflags & IO_GOLD)
												{
													ioo->_itemdata->price = multi;
												}
												else
												{
													ioo->_itemdata->maxcount = 9999;

													ARX_CHECK_SHORT(multi);
													ioo->_itemdata->count = ARX_CLEAN_WARN_CAST_SHORT(multi);


													if (ioo->_itemdata->count < 1) ioo->_itemdata->count = 1;
												}
											}

											ioo->show = SHOW_FLAG_IN_INVENTORY;

											if (!CanBePutInSecondaryInventory((INVENTORY_DATA *)inter.iobj[ion]->inventory, ioo, &xx, &yy))
											{
												PutInFrontOfPlayer(ioo, 1);
											}
										}
									}
									else
									{
										if (!strcmp(temp, "ADDMULTI"))
											pos = GetNextWord(es, pos, temp2);
									}
								}
							}
						}
						else if (!strcmp(temp, "DESTROY"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								if (SecondaryInventory == (INVENTORY_DATA *)inter.iobj[ion]->inventory)
									SecondaryInventory = NULL;

								free(inter.iobj[ion]->inventory);
								inter.iobj[ion]->inventory = NULL;
							}
						}
						else if (!strcmp(temp, "OPEN"))
						{
							if (SecondaryInventory != (INVENTORY_DATA *)inter.iobj[ion]->inventory)
							{
								SecondaryInventory = (INVENTORY_DATA *)inter.iobj[ion]->inventory;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
						else if (!strcmp(temp, "CLOSE"))
						{
							if (inter.iobj[ion]->inventory != NULL)
							{
								SecondaryInventory = NULL;
								ARX_SOUND_PlayInterface(SND_BACKPACK);
							}
						}
				}

				break;
			case 'O':

				if (!strcmp(temp, "OBJECTHIDE"))
				{
					char temp1[256];
					long megahide = 0;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'M'))
							megahide = 1;

						pos = GetNextWord(es, pos, temp);
					}

					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io);

					pos = GetNextWord(es, pos, temp1);

					if (ValidIONum(t))
					{
						inter.iobj[t]->GameFlags &= ~GFLAG_MEGAHIDE;

						if ((!stricmp(temp1, "ON")) || (!stricmp(temp1, "YES")))
						{
							if (megahide)
							{
								inter.iobj[t]->GameFlags |= GFLAG_MEGAHIDE;
								inter.iobj[t]->show = SHOW_FLAG_MEGAHIDE;
							}
							else inter.iobj[t]->show = SHOW_FLAG_HIDDEN;
						}
						else if ((inter.iobj[t]->show == SHOW_FLAG_MEGAHIDE)
						         ||	(inter.iobj[t]->show == SHOW_FLAG_HIDDEN))
						{
							inter.iobj[t]->show = SHOW_FLAG_IN_SCENE;

							if ((inter.iobj[t]->ioflags & IO_NPC)
							        &&	(inter.iobj[t]->_npcdata->life <= 0.f)
							   )
							{
								inter.iobj[t]->animlayer[0].cur_anim = inter.iobj[t]->anims[ANIM_DIE];
								inter.iobj[t]->animlayer[1].cur_anim = NULL;
								inter.iobj[t]->animlayer[2].cur_anim = NULL;
								inter.iobj[t]->animlayer[0].ctime = 9999999;
							}
						}
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "OBJECT_HIDE %s %s", temp, temp1);

#endif
				}

				break;
			case 'H':
			{
				if (!strcmp(temp, "HEROSAY"))
				{
					char tempp[256];
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'D'))
						{
							// do not show (debug say)
							if (FINAL_RELEASE)
							{
								pos = GetNextWord(es, pos, temp);
								goto nodraw;
							}
						}

						pos = GetNextWord(es, pos, temp);
					}

					strcpy(tempp, GetVarValueInterpretedAsText(temp, esss, io));

					if (tempp[0] == '[')
					{
						ARX_SPEECH_AddLocalised(NULL, tempp);
					}
					else
					{
						_TCHAR UText[512];
						MultiByteToWideChar(CP_ACP, 0, tempp, -1, UText, 256);
						ARX_SPEECH_Add(NULL, UText);
					}

				nodraw:
					;
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "HEROSAY %s", temp);

#endif
				}
				else if (!strcmp(temp, "HALO"))
				{
					pos = GetNextWord(es, pos, temp);
					MakeUpcase(temp);

					if (iCharIn(temp, 'O'))
					{
						if (io) io->halo_native.flags |= HALO_ACTIVE;
					}

					if (iCharIn(temp, 'F'))
					{
						if (io) io->halo_native.flags &= ~HALO_ACTIVE;
					}

					if (iCharIn(temp, 'N'))
					{
						if (io) io->halo_native.flags |= HALO_NEGATIVE;
					}
					else if (io) io->halo_native.flags &= ~HALO_NEGATIVE;

					if (iCharIn(temp, 'L'))
					{
						if (io) io->halo_native.flags |= HALO_DYNLIGHT;
					}
					else if (io)
					{
						io->halo_native.flags &= ~HALO_DYNLIGHT;

						if (ValidDynLight(io->halo_native.dynlight))
							DynLight[io->halo_native.dynlight].exist = 0;

						io->halo_native.dynlight = -1;
					}

					if (iCharIn(temp, 'C'))
					{
						char temp1[64];
						char temp2[64];
						char temp3[64];
						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);

						if (io)
						{
							io->halo_native.color.r = GetVarValueInterpretedAsFloat(temp1, esss, io);
							io->halo_native.color.g = GetVarValueInterpretedAsFloat(temp2, esss, io);
							io->halo_native.color.b = GetVarValueInterpretedAsFloat(temp3, esss, io);
						}
					}

					if (iCharIn(temp, 'S'))
					{
						char temp1[64];
						pos = GetNextWord(es, pos, temp1);

						if (io)
						{
							io->halo_native.radius = GetVarValueInterpretedAsFloat(temp1, esss, io);
						}
					}

					ARX_HALO_SetToNative(io);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "HALO %s", temp);

#endif
				}
			}
			break;
			case 'T':

				if (!strcmp(temp, "TELEPORT"))
				{
					char temp2[128];
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "TELEPORT ");
						strcat(cmd, temp);
					}

#endif

					if (!stricmp(temp, "behind"))
					{
						ARX_INTERACTIVE_TeleportBehindTarget(io);
					}
					else
					{
						TELEPORT_TO_CONFIRM = 1;
						long playr = 0;
						long initpos = 0;

						if (temp[0] == '-')
						{
							long angle = -1;

							if (iCharIn(temp, 'A'))
							{
								pos = GetNextWord(es, pos, temp2);

								float fangle = GetVarValueInterpretedAsFloat(temp2, esss, io);
								F2L(fangle, &angle);

								if (!iCharIn(temp, 'L'))
									player.desiredangle.b = player.angle.b = fangle;


							}

							if (iCharIn(temp, 'N'))
							{
								TELEPORT_TO_CONFIRM = 0;
							}

							if (iCharIn(temp, 'L'))
							{
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								strcpy(TELEPORT_TO_LEVEL, temp);
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								strcpy(TELEPORT_TO_POSITION, temp);

								if (angle == -1) TELEPORT_TO_ANGLE	=	ARX_CLEAN_WARN_CAST_LONG(player.angle.b);
								else TELEPORT_TO_ANGLE = angle;

								CHANGE_LEVEL_ICON = 1;

								if (!TELEPORT_TO_CONFIRM) CHANGE_LEVEL_ICON = 200;

								goto finishteleport;
							}

							if (iCharIn(temp, 'P'))
							{
								playr = 1;
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
							}

							if (iCharIn(temp, 'I'))
								initpos = 1;

						}

						if (!GAME_EDITOR) TELEPORT_TO_CONFIRM = 0;

						if (initpos == 0)
						{
							long t = GetTargetByNameTarget(temp);

							if (t == -2) t = GetInterNum(io);

							if ((t != -1) && (t != -2))
							{
								if (t == -3)
								{
									if ((io->show != SHOW_FLAG_LINKED)
									        &&	(io->show != SHOW_FLAG_HIDDEN)
									        &&	(io->show != SHOW_FLAG_MEGAHIDE)
									        &&	(io->show != SHOW_FLAG_DESTROYED)
									        &&	(io->show != SHOW_FLAG_KILLED))
										io->show = SHOW_FLAG_IN_SCENE;

									ARX_INTERACTIVE_Teleport(io, &player.pos);
								}
								else
								{
									if (inter.iobj[t] != NULL)
									{
										EERIE_3D pos;

										if (GetItemWorldPosition(inter.iobj[t], &pos))
										{
											if (playr)
											{
												ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
											}
											else
											{
												if ((io->ioflags & IO_NPC)
												        &&	(io->_npcdata->life <= 0))
												{
												}
												else
												{
													if ((io->show != SHOW_FLAG_HIDDEN)
													        &&	(io->show != SHOW_FLAG_MEGAHIDE))
														io->show = SHOW_FLAG_IN_SCENE;

													ARX_INTERACTIVE_Teleport(io, &pos);
												}
											}
										}
									}
								}
							}
						}
						else
						{
							if (io)
							{
								if (playr)
								{
									EERIE_3D pos;

									if (GetItemWorldPosition(io, &pos))
									{
										ARX_INTERACTIVE_Teleport(inter.iobj[0], &pos);
									}
								}
								else
								{
									if ((io->ioflags & IO_NPC)
									        &&	(io->_npcdata->life <= 0))
									{
									}
									else
									{
										if ((io->show != SHOW_FLAG_HIDDEN)
										        &&	(io->show != SHOW_FLAG_MEGAHIDE))
											io->show = SHOW_FLAG_IN_SCENE;

										ARX_INTERACTIVE_Teleport(io, &io->initpos);
									}
								}
							}
						}
					}

				finishteleport:
					;
				}
				else if (!strcmp(temp, "TARGETPLAYERPOS"))
				{
					if (io != NULL)
					{
						io->targetinfo = TARGET_PLAYER;
						GetTargetPos(io);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "TARGETPLAYERPOS");

#endif
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) sprintf(cmd, "ERROR: TARGETPLAYERPOS - NOT AN IO !!!");

#endif
				}
				else if (!strcmp(temp, "TWEAK"))
				{
					if (io != NULL)
					{
						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcpy(cmd, "TWEAK ");
							strcat(cmd, temp);
						}

#endif

						long tw;

						if (!stricmp(temp, "SKIN"))
						{
							char temp1[256];
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif
							pos = GetNextWord(es, pos, temp1);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp1);
							}

#endif

							if (io)
							{
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_SKIN, temp, temp1);
								EERIE_MESH_TWEAK_Skin(io->obj, temp, temp1);
							}
						}
						else if (!stricmp(temp, "ICON"))
						{
							pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp);
							}

#endif

							if (io)
							{
								ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_ICON, temp, NULL);
								ARX_INTERACTIVE_TWEAK_Icon(io, temp);
							}
						}
						else
						{
							tw = TWEAK_ERROR;

							if (!stricmp(temp, "HEAD"))
							{
								if (io->ident == 33)
								{
									tw = tw;
								}

								tw = TWEAK_HEAD;
							}
							else if (!stricmp(temp, "TORSO"))
								tw = TWEAK_TORSO;
							else if (!stricmp(temp, "LEGS"))
								tw = TWEAK_LEGS;
							else if (!stricmp(temp, "ALL"))
								tw = TWEAK_ALL;
							else if (!stricmp(temp, "UPPER"))
								tw = TWEAK_UPPER;
							else if (!stricmp(temp, "LOWER"))
								tw = TWEAK_LOWER;
							else if (!stricmp(temp, "UP_LO"))
								tw = TWEAK_UP_LO;

							if (!stricmp(temp, "REMOVE"))
							{
								tw = TWEAK_REMOVE;
								ARX_INTERACTIVE_MEMO_TWEAK(io, tw, NULL, NULL);
								EERIE_MESH_TWEAK_Do(io, tw, NULL);
							}
							else
							{
								pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp);
								}

#endif
								char path[512];

								if (io->usemesh != NULL)
									strcpy(path, io->usemesh);
								else strcpy(path, io->filename);

								RemoveName(path);
								strcat(path, "Tweaks\\");
								strcat(path, temp);
								strcat(path, ".teo");

								if (tw != TWEAK_ERROR)
								{
									ARX_INTERACTIVE_MEMO_TWEAK(io, tw, path + strlen(Project.workingdir), NULL);
									EERIE_MESH_TWEAK_Do(io, tw, path);
								}
							}
						}
					}

#ifdef NEEDING_DEBUG
					else if (NEED_DEBUG) sprintf(cmd, "ERROR: TWEAK - NOT AN IO !!!");

#endif
				}
				else if ((temp[1] == 'I') && (temp[2] == 'M') && (temp[3] == 'E') && (temp[4] == 'R'))
				{
					// Timer -m nbtimes duration commands
					char timername[64];
					char temp2[64];
					char temp3[64];

					long times = 0;
					long msecs = 0;

					// Checks if the timer is named by caller of if it needs a default name
					if (strlen(temp) > 5)
						strcpy(timername, temp + 5);
					else ARX_SCRIPT_Timer_GetDefaultName(timername);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, temp);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp2);
					}

#endif

					// We start by clearing instances of this timer. (Timers are unique by
					// a combination of name & IO).
					if (!stricmp(temp2, "KILL_LOCAL"))
					{
						ARX_SCRIPT_Timer_Clear_All_Locals_For_IO(io);
					}
					else
					{
						long mili = 0;
						long idle = 0;
						ARX_SCRIPT_Timer_Clear_By_Name_And_IO(timername, io);

						if (stricmp(temp2, "OFF"))
						{
							if (temp2[0] == '-')
							{
								if (iCharIn(temp2, 'M'))
									mili = 1;

								if (iCharIn(temp2, 'I'))
									idle = 1;

								pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

								if (NEED_DEBUG)
								{
									strcat(cmd, " ");
									strcat(cmd, temp2);
								}

#endif
							}

							times = atoi(temp2);
							pos = GetNextWord(es, pos, temp3);
#ifdef NEEDING_DEBUG

							if (NEED_DEBUG)
							{
								strcat(cmd, " ");
								strcat(cmd, temp3);
							}

#endif
							msecs = atoi(temp3);

							if (!mili) msecs *= 1000;

							long num = ARX_SCRIPT_Timer_GetFree();

							if (num != -1)
							{
								ActiveTimers++;
								scr_timer[num].es = es;
								scr_timer[num].exist = 1;
								scr_timer[num].io = io;
								scr_timer[num].msecs = msecs;
								scr_timer[num].namelength = strlen(timername) + 1;
								scr_timer[num].name = (char *)malloc(scr_timer[num].namelength);
								strcpy(scr_timer[num].name, timername);
								scr_timer[num].pos = pos;
								scr_timer[num].tim = ARXTimeUL();
								scr_timer[num].times = times;

								if ((idle) && io)
									scr_timer[num].flags = 1;
								else
									scr_timer[num].flags = 0;
							}

							pos = GotoNextLine(es, pos);
						}
					}
				}

				break;
			case 'V':

				if (!strcmp(temp, "VIEWBLOCK"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "VIEWBLOCK ");
						strcat(cmd, temp);
					}

#endif

					if (io)
					{
						if (!stricmp(temp, "ON"))
						{
							io->GameFlags |= GFLAG_VIEW_BLOCKER;
						}
						else io->GameFlags &= ~GFLAG_VIEW_BLOCKER;
					}
				}

				break;
			case 'W':

				if (!strcmp(temp, "WORLDFADE"))
				{
					char temp1[64];
					pos = GetNextWord(es, pos, temp);
					pos = GetNextWord(es, pos, temp1); //duration
					F2L(GetVarValueInterpretedAsFloat(temp1, esss, io), &FADEDURATION);
					FADESTART = ARX_TIME_GetUL();

					if (!stricmp(temp, "OUT"))
					{

						char temp2[64];
						char temp3[64];

						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);
						FADECOLOR.r = GetVarValueInterpretedAsFloat(temp1, esss, io);
						FADECOLOR.g = GetVarValueInterpretedAsFloat(temp2, esss, io);
						FADECOLOR.b = GetVarValueInterpretedAsFloat(temp3, esss, io);
						FADEDIR = -1;
					}
					else
					{
						FADEDIR = 1;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "WORLD_FADE %s %s", temp, temp1);

#endif
				}
				else if (!strcmp(temp, "WEAPON"))
				{
					pos = GetNextWord(es, pos, temp);

					if ((io) && (io->ioflags & IO_NPC))
					{
						if ((!stricmp(temp, "DRAW")) || (!stricmp(temp, "ON")))
						{
							if (io->_npcdata->weaponinhand == 0)
							{
								AcquireLastAnim(io);
								FinishAnim(io, io->animlayer[1].cur_anim);
								io->animlayer[1].cur_anim = NULL;
								io->_npcdata->weaponinhand = -1;
							}
						}
						else
						{
							if (io->_npcdata->weaponinhand == 1)
							{
								AcquireLastAnim(io);
								FinishAnim(io, io->animlayer[1].cur_anim);
								io->animlayer[1].cur_anim = NULL;
								io->_npcdata->weaponinhand = 2;
							}
						}
					}
				}

				break;
			case 'U':

				if (!strcmp(temp, "USEMESH"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_INTERACTIVE_MEMO_TWEAK(io, TWEAK_TYPE_MESH, temp, NULL);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "USE_MESH ");
						strcat(cmd, temp);
					}

#endif
					ARX_INTERACTIVE_USEMESH(io, temp);

					char tex[256];
					char tex1[256];
					char tex2[256];

					if (io->ioflags & IO_NPC)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\NPC\\%s", Project.workingdir, temp);
					else if (io->ioflags & IO_FIX)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\FIX_INTER\\%s", Project.workingdir, temp);
					else if (io->ioflags & IO_ITEM)	sprintf(tex2, "%sGraph\\Obj3D\\Interactive\\Items\\%s", Project.workingdir, temp);
					else tex2[0] = 0;

					File_Standardize(tex2, tex);

					if (tex[0] != 0)
					{
						if (io->usemesh == NULL)
							io->usemesh = (char *)malloc(256);

						strcpy(io->usemesh, tex);

						if (io->obj != NULL)
						{
							ReleaseEERIE3DObj(io->obj);
							io->obj = NULL;
						}

						char tex2[256];
						sprintf(tex2, "%sGraph\\Obj3D\\Textures\\", Project.workingdir);
						File_Standardize(tex2, tex1);

						if (io->ioflags & IO_FIX)
							io->obj = TheoToEerie_Fast(tex1, tex, TTE_NO_NDATA | TTE_NO_PHYSICS_BOX, GDevice);
						else if (io->ioflags & IO_NPC)
							io->obj = TheoToEerie_Fast(tex1, tex, TTE_NO_PHYSICS_BOX | TTE_NPC, GDevice);
						else
							io->obj = TheoToEerie_Fast(tex1, tex, 0, GDevice);

						EERIE_COLLISION_Cylinder_Create(io);
					}
				}

				if (!strcmp(temp, "UNSET"))
				{
					pos = GetNextWord(es, pos, temp, 1);

					if (IsGlobal(temp[0]))
					{
						UNSETVar(svar, &NB_GLOBALS, temp);
					}
					else
					{
						UNSETVar(esss->lvar, &esss->nblvar, temp);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET %s", temp);

#endif
				}
				else if (!strcmp(temp, "USEPATH"))
				{
					pos = GetNextWord(es, pos, temp);

					if (io->usepath)
					{
						ARX_USE_PATH * aup = (ARX_USE_PATH *)io->usepath;

						if (iCharIn(temp, 'B'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags |= ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(temp, 'F'))
						{
							aup->aupflags &= ~ARX_USEPATH_PAUSE;
							aup->aupflags |= ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}

						if (iCharIn(temp, 'P'))
						{
							aup->aupflags |= ARX_USEPATH_PAUSE;
							aup->aupflags &= ~ARX_USEPATH_FORWARD;
							aup->aupflags &= ~ARX_USEPATH_BACKWARD;
						}
					}
				}
				else if (!strcmp(temp, "UNSETCONTROLLEDZONE"))
				{
					pos = GetNextWord(es, pos, temp);
					ARX_PATH * ap = ARX_PATH_GetAddressByName(temp);

					if (ap != NULL)
					{
						ap->controled[0] = 0;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "UNSET_CONTROLLED_ZONE %s", temp);

#endif
				}

				break;
			case 'E':

				if (!strcmp(temp, "ELSE"))
				{
					pos = SkipNextStatement(es, pos);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "ELSE");

#endif
				}
				else if (!strcmp(temp, "ENDINTRO"))
				{
					ARX_INTERFACE_EndIntro();
				}
				else if (!strcmp(temp, "ENDGAME"))
				{
					REFUSE_GAME_RETURN = 1;

					if (FINAL_COMMERCIAL_DEMO)
						ARX_INTERFACE_EndIntro();
					else
					{
						ARX_SOUND_MixerStop(ARX_SOUND_MixerGame);
						ARX_MENU_Launch(GDevice);
						ARX_MENU_Clicked_CREDITS();
					}
				}
				else if (!strcmp(temp, "EATME"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "EATME");

#endif

					if (io) // can only kill IOs
					{
						if (io->ioflags & IO_ITEM)
						{
							player.hunger += io->_itemdata->food_value * 4;

							if (player.hunger > 100.f) player.hunger = 100.f;
						}

						if ((io->ioflags & IO_ITEM) && (io->_itemdata->count > 1))
						{
							io->_itemdata->count--;
						}
						else
						{
							io->show = SHOW_FLAG_KILLED;
							io->GameFlags &= ~GFLAG_ISINTREATZONE;
							RemoveFromAllInventories(io);
							ARX_DAMAGES_ForceDeath(io, EVENT_SENDER);
						}
					}
				}
				else if (!strcmp(temp, "EQUIP"))
				{
					pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "EQUIP ");
						strcat(cmd, temp);
					}

#endif
					long unequip = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'R')) unequip = 1;

						pos = GetNextWord(es, pos, temp);
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG)
						{
							strcat(cmd, " ");
							strcat(cmd, temp);
						}

#endif
					}

					long t = GetTargetByNameTarget(temp);

					if (t == -3) t = 0;

					if (ValidIONum(t))
					{
						if (unequip)
						{
							INTERACTIVE_OBJ * oes = EVENT_SENDER;
							EVENT_SENDER = inter.iobj[t];
							Stack_SendIOScriptEvent(io, SM_EQUIPOUT, "", NULL);
							EVENT_SENDER = oes;
							ARX_EQUIPMENT_UnEquip(inter.iobj[t], io);
						}
						else
						{
							INTERACTIVE_OBJ * oes = EVENT_SENDER;
							EVENT_SENDER = inter.iobj[t];
							Stack_SendIOScriptEvent(io, SM_EQUIPIN, "", NULL);
							EVENT_SENDER = oes;
							ARX_EQUIPMENT_Equip(inter.iobj[t], io);
						}
					}
				}

				break;
			case 'M':

				if (
				    (!strcmp(temp, "MUL")))
				{
					char temp1[64];
					char temp2[64];
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "MUL ");
						strcat(cmd, temp1);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, temp2);
					}

#endif

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '£': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&svar, &NB_GLOBALS, temp1);
							fval = fval * fdval;
							sv = SETVarValueLong(&svar, &NB_GLOBALS, temp1, (long)fval);

							if (sv)
								sv->type = TYPE_G_LONG;

							break;
						case '§': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);
							fval = fval * fdval;
							sv = SETVarValueLong(&esss->lvar, &esss->nblvar, temp1, (long)fval);

							if (sv)
								sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&svar, &NB_GLOBALS, temp1);
							fval = fdval * fval;
							sv = SETVarValueFloat(&svar, &NB_GLOBALS, temp1, fval);

							if (sv)
								sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);
							fval = fdval * fval;
							sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval);

							if (sv)
								sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif

				}
				else if (!strcmp(temp, "MOVE"))
				{
					if (io != NULL)
					{
						char temp1[64];
						char temp2[64];
						char temp3[64];
						float t1, t2, t3;



						pos = GetNextWord(es, pos, temp1);
						pos = GetNextWord(es, pos, temp2);
						pos = GetNextWord(es, pos, temp3);

						t1 = GetVarValueInterpretedAsFloat(temp1, esss, io);
						t2 = GetVarValueInterpretedAsFloat(temp2, esss, io);
						t3 = GetVarValueInterpretedAsFloat(temp3, esss, io);

						io->pos.x += t1;
						io->pos.y += t2;
						io->pos.z += t3;
#ifdef NEEDING_DEBUG

						if (NEED_DEBUG) sprintf(cmd, "%s %s %s %s", temp, temp1, temp2, temp3);

#endif
					}
				}
				else if (!strcmp(temp, "MAGIC"))
				{
					pos = GetNextWord(es, pos, temp);

					if (!stricmp(temp, "OFF"))
						GLOBAL_MAGIC_MODE = 0;
					else
						GLOBAL_MAGIC_MODE = 1;
				}
				else if (!strcmp(temp, "MAPMARKER"))
				{
					float x, y, t;
					long lvl;

					pos = GetNextWord(es, pos, temp);

					if ((!stricmp(temp, "remove")) || (!stricmp(temp, "-r")))
					{
						pos = GetNextWord(es, pos, temp);
						ARX_MAPMARKER_Remove(temp);
					}
					else
					{
						x = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						y = GetVarValueInterpretedAsFloat(temp, esss, io);
						pos = GetNextWord(es, pos, temp);
						t = GetVarValueInterpretedAsFloat(temp, esss, io);
						F2L(t, &lvl);
						pos = GetNextWord(es, pos, temp);
						ARX_MAPMARKER_Add(x, y, lvl, temp);
					}
				}

				break;
			case '-':
			case '+':

				if ((!strcmp(temp, "++")) ||
				        (!strcmp(temp, "--")))
				{
					SCRIPT_VAR * sv = NULL;
					char temp1[64];
					long	ival;
					float	fval;
					pos = GetNextWord(es, pos, temp1);

					switch (temp1[0])
					{
						case '#':
							ival = GETVarValueLong(&svar, &NB_GLOBALS, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueLong(&svar, &NB_GLOBALS, temp1, ival - 1);
							}
							else
							{
								sv = SETVarValueLong(&svar, &NB_GLOBALS, temp1, ival + 1);
							}

							break;
						case '§':
							ival = GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueLong(&esss->lvar, &esss->nblvar, temp1, ival - 1);
							}
							else
							{
								sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, ival + 1.f);
							}

							break;
						case '&':
							fval = GETVarValueFloat(&svar, &NB_GLOBALS, temp1);
							ARX_CHECK_NO_ENTRY();

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueFloat(&svar, &NB_GLOBALS, temp1, fval  - 1.f);
							}
							else
							{
								sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval + 1.f);
							}

							break;
						case '@':
							fval = GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);

							if (!strcmp(temp, "--"))
							{
								sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval - 1.f);
							}
							else
							{
								sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval + 1.f);
							}

							break;
						default:
							return BIGERROR;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s", temp, temp1);

#endif
				}

			case 'D':

				if (
				    (!strcmp(temp, "DEC")) ||
				    (!strcmp(temp, "DIV")))
				{
					char temp1[64];
					char temp2[64];
					float fval;
					float fdval;
					SCRIPT_VAR * sv = NULL;

					pos = GetNextWord(es, pos, temp1, 1);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, temp);
						strcat(cmd, " ");
						strcat(cmd, temp1);
					}

#endif
					pos = GetNextWord(es, pos, temp2);
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp2);
					}

#endif

					switch (temp1[0])
					{
						case '$': // GLOBAL TEXT
						case '£': // LOCAL TEXT
							ShowScriptError("Unable to execute this\nOperation on a String", cmd);
							break;
						case '#': // GLOBAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&svar, &NB_GLOBALS, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(&svar, &NB_GLOBALS, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_G_LONG;

							break;
						case '§': // LOCAL LONG
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = (float)GETVarValueLong(&esss->lvar, &esss->nblvar, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueLong(&esss->lvar, &esss->nblvar, temp1, (long)fval);

							if (sv != NULL) sv->type = TYPE_L_LONG;

							break;
						case '&': // GLOBAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&svar, &NB_GLOBALS, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(&svar, &NB_GLOBALS, temp1, fval);

							if (sv != NULL) sv->type = TYPE_G_FLOAT;

							break;
						case '@': // LOCAL FLOAT
							fval = GetVarValueInterpretedAsFloat(temp2, esss, io);
							fdval = GETVarValueFloat(&esss->lvar, &esss->nblvar, temp1);

							if (!strcmp(temp, "DEC")) fval = fdval - fval;
							else if (!strcmp(temp, "DIV"))
							{
								if (fval != 0.f)	fval = fdval / fval;
							}

							sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, temp1, fval);

							if (sv != NULL) sv->type = TYPE_L_FLOAT;

							break;
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "%s %s %s", temp, temp1, temp2);

#endif
				}
				else if (!strcmp(temp, "DESTROY"))
				{
					char temp2[256];
					pos = GetNextWord(es, pos, temp2); // Source IO
					strcpy(temp, GetVarValueInterpretedAsText(temp2, esss, io));
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					if (ValidIONum(t)) // can only kill IOs
					{
						long self = 0;
						INTERACTIVE_OBJ * ioo = inter.iobj[t];

						if (io == ioo) self = 1;

						ARX_INTERACTIVE_DestroyIO(ioo);

						if (self) return ACCEPT; // Cannot process further...
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DESTROY");

#endif
				}
				else if (!strcmp(temp, "DETACHNPCFROMPLAYER"))
				{
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DETACH_NPC_FROM_PLAYER ...OBSOLETE...");

#endif
				}
				else if (!strcmp(temp, "DODAMAGE"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
					long type = 0;

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'F'))
							type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(temp, 'M'))
							type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(temp, 'P'))
							type |= DAMAGE_TYPE_POISON;

						if (iCharIn(temp, 'L'))
							type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(temp, 'C'))
							type |= DAMAGE_TYPE_COLD;

						if (iCharIn(temp, 'G'))
							type |= DAMAGE_TYPE_GAS;

						if (iCharIn(temp, 'E'))
							type |= DAMAGE_TYPE_METAL;

						if (iCharIn(temp, 'W'))
							type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(temp, 'S'))
							type |= DAMAGE_TYPE_STONE;

						if (iCharIn(temp, 'A'))
							type |= DAMAGE_TYPE_ACID;

						if (iCharIn(temp, 'O'))
							type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(temp, 'R'))
							type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(temp, 'N'))
							type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(temp, 'U'))
							type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, temp);
					}

					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp);
					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);

					if (ValidIONum(t))
						ARX_DAMAGES_DealDamages(t, fval, GetInterNum(io), type, &inter.iobj[t]->pos);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DODAMAGE");

#endif
				}
				else if (!strcmp(temp, "DAMAGER"))
				{
					io->damager_type = DAMAGE_TYPE_PER_SECOND;
					pos = GetNextWord(es, pos, temp);

					if (temp[0] == '-')
					{
						if (iCharIn(temp, 'F'))
							io->damager_type |= DAMAGE_TYPE_FIRE;

						if (iCharIn(temp, 'M'))
							io->damager_type |= DAMAGE_TYPE_MAGICAL;

						if (iCharIn(temp, 'P'))
							io->damager_type |= DAMAGE_TYPE_POISON;

						if (iCharIn(temp, 'L'))
							io->damager_type |= DAMAGE_TYPE_LIGHTNING;

						if (iCharIn(temp, 'C'))
							io->damager_type |= DAMAGE_TYPE_COLD;

						if (iCharIn(temp, 'G'))
							io->damager_type |= DAMAGE_TYPE_GAS;

						if (iCharIn(temp, 'E'))
							io->damager_type |= DAMAGE_TYPE_METAL;

						if (iCharIn(temp, 'W'))
							io->damager_type |= DAMAGE_TYPE_WOOD;

						if (iCharIn(temp, 'S'))
							io->damager_type |= DAMAGE_TYPE_STONE;

						if (iCharIn(temp, 'A'))
							io->damager_type |= DAMAGE_TYPE_ACID;

						if (iCharIn(temp, 'O'))
							io->damager_type |= DAMAGE_TYPE_ORGANIC;

						if (iCharIn(temp, 'R'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_LIFE;

						if (iCharIn(temp, 'N'))
							io->damager_type |= DAMAGE_TYPE_DRAIN_MANA;

						if (iCharIn(temp, 'U'))
							io->damager_type |= DAMAGE_TYPE_PUSH;

						pos = GetNextWord(es, pos, temp);
					}

					float fval = GetVarValueInterpretedAsFloat(temp, esss, io);



					ARX_CHECK_SHORT(fval);

					io->damager_damages = ARX_CLEAN_WARN_CAST_SHORT(fval);

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DAMAGER");

#endif
				}
				else if (!strcmp(temp, "DETACH"))
				{
					pos = GetNextWord(es, pos, temp); // Source IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcpy(cmd, "DETACH ");
						strcat(cmd, temp);
					}

#endif
					long t = GetTargetByNameTarget(temp);

					if (t == -2) t = GetInterNum(io); //self

					pos = GetNextWord(es, pos, temp); // target IO
#ifdef NEEDING_DEBUG

					if (NEED_DEBUG)
					{
						strcat(cmd, " ");
						strcat(cmd, temp);
					}

#endif
					long t2 = GetTargetByNameTarget(temp);

					if (t2 == -2) t2 = GetInterNum(io); //self

					ARX_INTERACTIVE_Detach(t, t2);
				}

				else if (!strcmp(temp, "DRAWSYMBOL")) // DRAWSYMBOL symbol duration
				{
					char temp1[64];
					char temp2[64];
					pos = GetNextWord(es, pos, temp1);
					pos = GetNextWord(es, pos, temp2);

					if (io != NULL)
					{
						MakeUpcase(temp1);
						float dur = GetVarValueInterpretedAsFloat(temp2, esss, io);
						ARX_SPELLS_RequestSymbolDraw(io, temp1, dur);
					}

#ifdef NEEDING_DEBUG

					if (NEED_DEBUG) sprintf(cmd, "DRAW_SYMBOL %s %s", temp1, temp2);

#endif
				}

				break;
			default:

				if (io)
				{
					char temp2[256];
					char temp3[256];
					char temp4[256];
					long ppos = pos;
					pos = GetNextWord(es, pos, temp2);
					pos = GetNextWord(es, pos, temp3);
					pos = GetNextWord(es, pos, temp4);
					sprintf(cmd, "SCRIPT ERROR: %s_%04d %s %s %s %s [char %d]", GetName(io->filename), io->ident, temp, temp2, temp3, temp4, ppos);

					if (!ERROR_Log(cmd));
					else ShowPopup(cmd);

					io->ioflags |= IO_FREEZESCRIPT;
					return REFUSE;
				}

#ifdef NEEDING_DEBUG

				if (NEED_DEBUG)
				{
					sprintf(cmd, "ERROR: %s UNKNOWN COMMAND !!!", temp);
					DEBUG_Notify(cmd);
				}

#endif

		}

#ifdef NEEDING_DEBUG

		if ((NEED_DEBUG) && (cmd[0] != 0))
		{
			char temp[256];
			sprintf(temp, "  %s", cmd);
			DEBUG_Notify(temp);
		}

#endif
	}

end:
	;

#ifdef NEEDING_DEBUG

	if (NEED_DEBUG)
	{
		if (msg != SM_EXECUTELINE)
		{
			if (evname) sprintf(cmd, "%s EVENT Successfully Finished___________________________________________", eventname);
			else if (msg != SM_DUMMY) sprintf(cmd, "%s EVENT Successfully Finished___________________________________________________", AS_EVENT[msg].name);
			else  sprintf(cmd, "Dummy EVENT Successfully Finished___________________________________________________");

			DEBUG_Notify(cmd);
		}
		else
		{
			sprintf(cmd, "EXECUTELINE Successfully Finished_________________________________________________________________");
			DEBUG_Notify(cmd);
		}
	}

#endif
	return ret;
}

BOOL InSubStack(EERIE_SCRIPT * es, long pos)
{
	for (long i = 0; i < MAX_GOSUB; i++)
	{
		if (es->sub[i] == -1)
		{
			es->sub[i] = pos;
			return TRUE;
		}
	}

	return FALSE;
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



	ARX_SCRIPT_ComputeShortcuts(es);
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
			SetWindowText(hDlg, ShowTextWindowtext);
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
			SendMessage(thWnd, WM_SETFONT, (WPARAM) GetStockObject(ANSI_FIXED_FONT), TRUE);
			SetWindowText(thWnd, ShowText);

			return TRUE;
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

	return FALSE;
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
			SetWindowText(hDlg, ShowTextWindowtext);
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT);
			SetWindowText(thWnd, ShowText);
			thWnd = GetDlgItem(hDlg, IDC_SHOWTEXT2);
			SetWindowText(thWnd, ShowText2);
			return TRUE;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, LOWORD(wParam));
					break;
			}

			break;
	}

	return FALSE;
}

void ARX_SCRIPT_SetVar(INTERACTIVE_OBJ * io, char * name, char * content)
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

			sv = SETVarValueText(&svar, &NB_GLOBALS, name, content);

			if (sv != NULL)
				sv->type = TYPE_G_TEXT;

			break;
		case '£': // LOCAL TEXT

			if (io == NULL) return;

			sv = SETVarValueText(&esss->lvar, &esss->nblvar, name, content);

			if (sv != NULL)
				sv->type = TYPE_L_TEXT;

			break;
		case '#': // GLOBAL LONG

			if (io) return;

			ival = atoi(content);
			sv = SETVarValueLong(&svar, &NB_GLOBALS, name, ival);

			if (sv != NULL)
				sv->type = TYPE_G_LONG;

			break;
		case '§': // LOCAL LONG

			if (io == NULL) return;

			ival = atoi(content);
			sv = SETVarValueLong(&esss->lvar, &esss->nblvar, name, ival);

			if (sv != NULL)
				sv->type = TYPE_L_LONG;

			break;
		case '&': // GLOBAL FLOAT

			if (io) return;

			fval = (float)atof(content);
			sv = SETVarValueFloat(&svar, &NB_GLOBALS, name, fval);

			if (sv != NULL)
				sv->type = TYPE_G_FLOAT;

			break;
		case '@': // LOCAL FLOAT

			if (io == NULL) return;

			fval = (float)atof(content);
			sv = SETVarValueFloat(&esss->lvar, &esss->nblvar, name, fval);

			if (sv != NULL)
				sv->type = TYPE_L_FLOAT;

			break;
	}
}
