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

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "..\DANAE_Debugger\SCRIPT_DEBUGGER_Dialog.h"
#include "arx_interactive.h"
#include "arx_time.h"
#include "eeriepoly.h"
#include <HERMESMain.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

typedef void (APIENTRY * CREATEDIALOG)(HWND, ScriptDebuggerInfos &);
typedef void (APIENTRY * SETPARAMS)(ScriptDebuggerInfos &);
typedef bool (APIENTRY * DD_EXISTS)();
typedef void (APIENTRY * DD_KILL)();
typedef void (APIENTRY * DD_GETPARAMS)(ScriptDebuggerUpdate &);
typedef unsigned long(APIENTRY * DD_GETVERSION)();
CREATEDIALOG DD_DebugDialog = NULL;
SETPARAMS DD_SetParams = NULL;
DD_EXISTS	DD_Exists = NULL;
DD_KILL		DD_Kill = NULL;
DD_GETPARAMS DD_GetParams = NULL;
DD_GETVERSION DD_GetVersion = NULL;

void DANAE_DEBUGGER_Launch(HWND hWnd)
{
	HMODULE hm = LoadLibrary("ARX_SCRIPT_DEBUGGER");

	if (!(DD_GetVersion =	(DD_GETVERSION) GetProcAddress(hm, "SCRIPT_DEBUGGER_GetVersion"))) goto invalid;

	unsigned long version;
	version = DD_GetVersion();

	if (version != MAKELONG(1, 0))  goto invalid;

	if (!(DD_DebugDialog =	(CREATEDIALOG) GetProcAddress(hm, "SCRIPT_DEBUGGER_CreateDialog")))  goto invalid;

	if (!(DD_SetParams =	(SETPARAMS) GetProcAddress(hm, "SCRIPT_DEBUGGER_SetParams"))) goto invalid;

	if (!(DD_Exists =	(DD_EXISTS) GetProcAddress(hm, "SCRIPT_DEBUGGER_WindowOpened"))) goto invalid;

	if (!(DD_Kill =	(DD_KILL) GetProcAddress(hm, "SCRIPT_DEBUGGER_Destroy"))) goto invalid;

	if (!(DD_GetParams =	(DD_GETPARAMS) GetProcAddress(hm, "SCRIPT_DEBUGGER_GetParams"))) goto invalid;

	printf("%lu\n", GetLastError());
	printf("yo\n");

	if (DD_DebugDialog)
	{
		ScriptDebuggerInfos s;
		memset(&s, 0, sizeof(ScriptDebuggerInfos));
		DD_DebugDialog(hWnd, s);
	}

	return;

invalid:
	;
	ShowPopup("ARX_SCRIPT_DEBUGGER: Invalid DLL version.");
	return;
}
long NEED_DEBUGGER_CLEAR;
extern long LastSelectedIONum;
extern INTERACTIVE_OBJ * IO_DEBUG;
void DANAE_DEBUGGER_Update()
{
	static INTERACTIVE_OBJ * lastio = NULL;
	static long MODIFFF = 1;

	if (!DD_Exists) return;

	if (!DD_Exists()) return;

	int i;
	char buffer[20];
	ScriptDebuggerUpdate su;

	memset(&su, 0, sizeof(ScriptDebuggerUpdate));
	char p1[256];
	char p2[256];
	char p3[256];
	char p4[256];

	su.globalVar.lpszVarName = p1;
	su.globalVar.lpszVarValue = p2;
	su.localVar.lpszVarName = p3;
	su.localVar.lpszVarValue = p4;

	DD_GetParams(su);

	ScriptDebuggerInfos s;
	memset(&s, 0, sizeof(ScriptDebuggerInfos));

	INTERACTIVE_OBJ * io;

	if (ValidIONum(LastSelectedIONum))
		io = inter.iobj[LastSelectedIONum];
	else io = NULL;

	IO_DEBUG = io;

	if (lastio != io)
	{
		NEED_DEBUGGER_CLEAR = 1;
		memset(&s, 0, sizeof(ScriptDebuggerInfos));
		s.lpszObjName = strdup("No Object Selected");
		DD_SetParams(s);
		free(s.lpszObjName);
		MODIFFF = 1;
	}

	lastio = io;

	if (NEED_DEBUGGER_CLEAR)
	{
		s.bClear = 1;
		NEED_DEBUGGER_CLEAR = 0;
	}

	if (LastSelectedIONum < 0)
	{
		memset(&s, 0, sizeof(ScriptDebuggerInfos));
		s.lpszObjName = strdup("No Object Selected");
		DD_SetParams(s);
		free(s.lpszObjName);
		goto suite;
	}

	if ((ARXPausedTimer) && (!MODIFFF))	goto suite;

	MODIFFF = 0;

	char temp[256];

	sprintf(temp, "%s_%04d", GetName(io->filename), io->ident);
	s.lpszObjName = strdup(temp);
	sprintf(buffer, "%5.0f", io->pos.x);
	s.p3ObjPos[0] = strdup(buffer);
	sprintf(buffer, "%5.0f", io->pos.y);
	s.p3ObjPos[1] = strdup(buffer);
	sprintf(buffer, "%5.0f", io->pos.z);
	s.p3ObjPos[2] = strdup(buffer);

	INTERACTIVE_OBJ * tio;
	tio = NULL;

	if (ValidIONum(io->targetinfo))
		tio = inter.iobj[io->targetinfo];
	else if (io->targetinfo == -2)
		tio = io;

	if (tio)
	{
		sprintf(temp, "%s_%04d", GetName(tio->filename), tio->ident);
		s.lpszTargetName = strdup(temp);
		sprintf(buffer, "%5.0f", tio->pos.x);
		s.p3TargetPos[0] = strdup(buffer);
		sprintf(buffer, "%5.0f", tio->pos.y);
		s.p3TargetPos[1] = strdup(buffer);
		sprintf(buffer, "%5.0f", tio->pos.z);
		s.p3TargetPos[2] = strdup(buffer);
	}
	else
	{
		s.lpszTargetName = strdup("None");
		s.p3TargetPos[0] = strdup(" ");
		s.p3TargetPos[1] = strdup(" ");
		s.p3TargetPos[2] = strdup(" ");
	}

	if (su.bVariables)
	{
		s.iNbGlobals = NB_GLOBALS;
		Vars * vg;
		vg = NULL;

		if (s.iNbGlobals)
		{
			vg = new Vars[s.iNbGlobals];

			for (i = 0; i < s.iNbGlobals; i++)
			{
				vg[i].lpszVarName = strdup(svar[i].name);

				switch (svar[i].type)
				{
				case TYPE_G_TEXT:
					vg[i].lpszVarValue = strdup(svar[i].text);
					break;
				case TYPE_G_LONG:
					sprintf(temp, "%d", svar[i].ival);
					vg[i].lpszVarValue = strdup(temp);
					break;
				case TYPE_G_FLOAT:
					sprintf(temp, "%f", svar[i].fval);
					vg[i].lpszVarValue = strdup(temp);
					break;
				default:
					sprintf(temp, " ");
					vg[i].lpszVarValue = strdup(temp);
					break;
				}
			}
		}

		s.pGlobalVars = vg;

		EERIE_SCRIPT * es;
		es = &io->script;

		s.iNbLocals = es->nblvar;
		Vars * vl;
		vl = NULL;

		if (s.iNbLocals)
		{
			vl = new Vars[s.iNbLocals];

			for (i = 0; i < s.iNbLocals; i++)
			{
				vl[i].lpszVarName = strdup(es->lvar[i].name);

				switch (es->lvar[i].type)
				{
				case TYPE_L_TEXT:
					sprintf(temp, "%s", es->lvar[i].text);
					vl[i].lpszVarValue = strdup(temp);
					break;
				case TYPE_L_LONG:
					sprintf(temp, "%d", es->lvar[i].ival);
					vl[i].lpszVarValue = strdup(temp);
					break;
				case TYPE_L_FLOAT:
					sprintf(temp, "%f", es->lvar[i].fval);
					vl[i].lpszVarValue = strdup(temp);
					break;
				default:
					sprintf(temp, " ");
					vl[i].lpszVarValue = strdup(temp);
					break;
				}
			}
		}

		s.pLocalVars = vl;
	}

	if (io->ioflags & IO_NPC)
	{
		if (io->_npcdata->behavior & BEHAVIOUR_MOVE_TO)
			strcpy(temp, "MOVE_TO");
		else if (io->_npcdata->behavior & BEHAVIOUR_GO_HOME)
			strcpy(temp, "GO_HOME");
		else if (io->_npcdata->behavior & BEHAVIOUR_FLEE)
			strcpy(temp, "FLEE");
		else if (io->_npcdata->behavior & BEHAVIOUR_LOOK_FOR)
			strcpy(temp, "LOOK_FOR");
		else if (io->_npcdata->behavior & BEHAVIOUR_HIDE)
			strcpy(temp, "HIDE");
		else if (io->_npcdata->behavior & BEHAVIOUR_WANDER_AROUND)
			strcpy(temp, "WANDER_AROUND");
		else if (io->_npcdata->behavior & BEHAVIOUR_GUARD)
			strcpy(temp, "GUARD");
		else if (io->_npcdata->behavior & BEHAVIOUR_FRIENDLY)
			strcpy(temp, "FRIENDLY");
		else strcpy(temp, "NONE");

		if (io->_npcdata->behavior & BEHAVIOUR_LOOK_AROUND)
			strcat(temp, " Look_Around");

		if (io->_npcdata->behavior & BEHAVIOUR_SNEAK)
			strcat(temp, " Sneak");

		if (io->_npcdata->behavior & BEHAVIOUR_DISTANT)
			strcat(temp, " Distant");

		if (io->_npcdata->behavior & BEHAVIOUR_FIGHT)
			strcat(temp, " Fight");

		if (io->_npcdata->behavior & BEHAVIOUR_MAGIC)
			strcat(temp, " Magic");

		s.lpszBehavior = strdup(temp);
	}
	else
	{
		s.lpszBehavior = strdup("Not an NPC...");
	}

	s.bEvents = false;

	if (su.bEvents)
	{
		s.bEvents = true;
		s.lpszEvents = (char *) malloc(BIG_DEBUG_POS + 1);
		memcpy(s.lpszEvents, BIG_DEBUG_STRING, BIG_DEBUG_POS + 1);
		BIG_DEBUG_POS = 0;
		BIG_DEBUG_STRING[0] = 0;
	}

	long count;
	count = 0;

	for (i = 0; i < MAX_TIMER_SCRIPT; i++)
	{
		if ((scr_timer[i].exist)  && (scr_timer[i].io == io))
			count++;
	}

	s.bTimers = false;
	s.lpszTimers = NULL;

	if (su.bTimers)
	{
		s.bTimers = true;
		char buf[16000];
		ZeroMemory(buf, 16000);
		long num;
		num = 0;

		for (i = 0; i < MAX_TIMER_SCRIPT; i++)
		{
			if ((scr_timer[i].exist)  && (scr_timer[i].io == io))
			{
				sprintf(temp, "%s %d %dms\r\n", scr_timer[i].name, scr_timer[i].times, scr_timer[i].msecs);
				strcat(buf, temp);
			}
		}

		s.lpszTimers = strdup(buf);
	}

	DD_SetParams(s);

	if (s.lpszObjName) free(s.lpszObjName);

	if (s.p3ObjPos[0]) free(s.p3ObjPos[0]);

	if (s.p3ObjPos[1]) free(s.p3ObjPos[1]);

	if (s.p3ObjPos[2]) free(s.p3ObjPos[2]);

	if (s.lpszTargetName) free(s.lpszTargetName);

	if (s.p3TargetPos[0]) free(s.p3TargetPos[0]);

	if (s.p3TargetPos[1]) free(s.p3TargetPos[1]);

	if (s.p3TargetPos[2]) free(s.p3TargetPos[2]);

	if (su.bVariables)
	{
		for (i = 0; i < s.iNbGlobals; i++)
		{
			if (s.pGlobalVars[i].lpszVarName) free(s.pGlobalVars[i].lpszVarName);

			if (s.pGlobalVars[i].lpszVarValue) free(s.pGlobalVars[i].lpszVarValue);
		}

		if (s.iNbGlobals) delete [] s.pGlobalVars;

		for (i = 0; i < s.iNbLocals; i++)
		{
			if (s.pLocalVars[i].lpszVarName) free(s.pLocalVars[i].lpszVarName);

			if (s.pLocalVars[i].lpszVarValue) free(s.pLocalVars[i].lpszVarValue);
		}

		if (s.iNbLocals) delete [] s.pLocalVars;
	}

	if (s.lpszBehavior) free(s.lpszBehavior);

	if (s.bEvents)
	{
		if (s.lpszEvents) free(s.lpszEvents);
	}

	if (s.bTimers)
	{
		if (s.lpszTimers) free(s.lpszTimers);
	}

suite:
	;

	if (su.bPause)
	{
		if (!ARXPausedTimer)
		{
			ARX_TIME_Pause();
		}
		else
		{
			ARX_TIME_UnPause();
		}
	}

	if (su.bStep)
	{
		if (ARXPausedTimer)
		{
			ARXTotalPausedTime -= 100;
			MODIFFF = 1;
		}
	}

	if (su.bUpdateGlobalVar)
	{
		ARX_SCRIPT_SetVar(NULL, su.globalVar.lpszVarName, su.globalVar.lpszVarValue);
		MODIFFF = 1;
	}

	if ((su.bUpdateLocalVar) && (io))
	{
		ARX_SCRIPT_SetVar(io, su.localVar.lpszVarName, su.localVar.lpszVarValue);
		MODIFFF = 1;
	}
}
