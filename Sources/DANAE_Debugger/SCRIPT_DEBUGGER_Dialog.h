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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef SCRIPT_DEBUGGER_H
#define SCRIPT_DEBUGGER_H

#include <windows.h>
#include <ARX_Common.h>

#ifndef APIDLL
#define APIFUNC __declspec(dllimport) WINAPI
#else
#define APIFUNC __declspec(dllexport) WINAPI
#endif

struct Vars
{
	char * lpszVarName;
	char * lpszVarValue;
};

struct ScriptDebuggerUpdate
{
	bool bVariables;
	bool bEvents;
	bool bTimers;
	bool bPause;
	bool bStep;
	bool bUpdateGlobalVar;
	Vars globalVar;
	bool bUpdateLocalVar;
	Vars localVar;
};

struct ScriptDebuggerInfos
{
	bool bClear;
	//obj inter
	char * lpszObjName;
	char * p3ObjPos[3];
	//target
	char * lpszTargetName;
	char * p3TargetPos[3];
	//globals
	int iNbGlobals;
	Vars * pGlobalVars;
	//locals
	int iNbLocals;
	Vars * pLocalVars;
	//behavior
	char * lpszBehavior;
	//events
	bool bEvents;
	char * lpszEvents;
	//timers
	bool bTimers;
	char * lpszTimers;
};


//-----------------------------------------------------------------------------
LPSTR			APIFUNC SCRIPT_DEBUGGER_GetName();

//-----------------------------------------------------------------------------
unsigned long	APIFUNC SCRIPT_DEBUGGER_GetVersion();

//-----------------------------------------------------------------------------
// Création de la boite de dialogue
void			APIFUNC SCRIPT_DEBUGGER_CreateDialog(HWND, ScriptDebuggerInfos &);

//-----------------------------------------------------------------------------
void			APIFUNC	SCRIPT_DEBUGGER_SetParams(ScriptDebuggerInfos &);

//-----------------------------------------------------------------------------
void			APIFUNC SCRIPT_DEBUGGER_GetParams(ScriptDebuggerUpdate &);

//-----------------------------------------------------------------------------
bool			APIFUNC SCRIPT_DEBUGGER_WindowOpened();

//-----------------------------------------------------------------------------
void			APIFUNC SCRIPT_DEBUGGER_Destroy();

#endif
