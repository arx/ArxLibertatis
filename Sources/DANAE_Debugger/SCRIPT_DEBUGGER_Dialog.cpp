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
// version 0.1
// Script Debugger
// jeudi 28 juin 2001
//		merge + rajout check variables
//		fix sort + speed
// mercredi 27 juin 2001
//		fixs + merge
// mardi 26 juin 2001
//		merge + fix + interface revue
//		events combo -> edit multiline
//		timers combo -> edit multiline
// lundi 25 juin 2001
//		ok done + waiting to merge + no memory leak
// vendredi 22 juin
//		fix + memory check
//		+ tests memory leak + patch
// jeudi 21 juin
//		points d'entrée
// mercredi 20 juin
//		création DLL
//		interface

//-----------------------------------------------------------------------------

#include "SCRIPT_DEBUGGER_Dialog.h"
#include <commctrl.h>
#include <stdio.h>

#ifndef  __RESOURCE_H
#include "Resource.h"
#endif

HINSTANCE ghInstance;
HWND ghDialog;

char GlobalName[128];
char LocalName[128];


bool gbDialog = false;


BOOL CALLBACK SCRIPT_DEBUGGER_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


ScriptDebuggerUpdate sdu;

//
// tsu
//-----------------------------------------------------------------------------
struct info
{
	HWND hObjName;
	HWND hObjPos[3];
	HWND hTargetName;
	HWND hTargetPos[3];
	HWND hVariablesEnabled;
	HWND hGlobals;
	HWND hGlobalEdit;
	HWND hLocals;
	HWND hLocalEdit;
	HWND hBehavior;
	HWND hEventsEnabled;
	HWND hEvents;
	HWND hTimersEnabled;
	HWND hTimers;
};

info iInfo;

//-----------------------------------------------------------------------------
void InitList(HWND _hwnd)
{
	SendMessage(_hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM) LVS_EX_GRIDLINES);

	LVCOLUMN lvc;
	// Initialize the LVCOLUMN structure.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;// | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 145;//75
	lvc.pszText = "Name";
	SendMessage(_hwnd, LVM_INSERTCOLUMN, 0, (LPARAM) &lvc);
	lvc.cx = 50;//75
	lvc.pszText = "Value";
	SendMessage(_hwnd, LVM_INSERTCOLUMN, 1, (LPARAM) &lvc);
}

//-----------------------------------------------------------------------------
void InsertItem(HWND _hwnd, char * _name, char * _value)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText = "Should not be seen";


	lvi.iItem = 1;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM) strdup(_name);

	int r = SendMessage(_hwnd, LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM FAR *)(&lvi));

	ListView_SetItemText(_hwnd, r, 0, _name);
	ListView_SetItemText(_hwnd, r, 1, _value);
}

//-----------------------------------------------------------------------------
BOOL CALLBACK SCRIPT_DEBUGGER_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		case WM_INITDIALOG:
		{
			//-------------------------------------------------------------------------
			iInfo.hObjName			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_OBJ_NAME);
			iInfo.hObjPos[0]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_OBJ_POSX);
			iInfo.hObjPos[1]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_OBJ_POSY);
			iInfo.hObjPos[2]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_OBJ_POSZ);
			iInfo.hTargetName		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_TARGET_NAME);
			iInfo.hTargetPos[0]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_TARGET_POSX);
			iInfo.hTargetPos[1]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_TARGET_POSY);
			iInfo.hTargetPos[2]		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_TARGET_POSZ);
			iInfo.hVariablesEnabled = GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_CHECK_VARIABLES);
			iInfo.hGlobals			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_LIST_GLOBALS);
			iInfo.hGlobalEdit		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_GLOBAL);
			iInfo.hLocals			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_LIST_LOCALS);
			iInfo.hLocalEdit		= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_LOCAL);
			iInfo.hBehavior			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_BEHAVIOR);
			iInfo.hEventsEnabled	= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_CHECK_EVENTS);
			iInfo.hEvents			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_EVENTS);
			iInfo.hTimersEnabled	= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_CHECK_TIMERS);
			iInfo.hTimers			= GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_EDIT_TIMERS);

			SendMessage(iInfo.hVariablesEnabled, BM_SETCHECK, (WPARAM) true, (LPARAM) 0);
			SendMessage(iInfo.hEventsEnabled, BM_SETCHECK, (WPARAM) true, (LPARAM) 0);
			SendMessage(iInfo.hTimersEnabled, BM_SETCHECK, (WPARAM) true, (LPARAM) 0);

			HWND hwnd = GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_LIST_GLOBALS);
			InitList(hwnd);

			hwnd = GetDlgItem(hWnd, IDC_SCRIPT_DEBUGGER_LIST_LOCALS);
			InitList(hwnd);
		}
		break;
		case WM_NOTIFY:
		{
			NMHDR * mf = (NMHDR *) lParam;

			if (mf->code == NM_CLICK)
			{
				LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
				char name[256];
				char value[256];

				if (lpnmitem->iItem >= 0)
					if (lpnmitem->iItem < ListView_GetItemCount(mf->hwndFrom))
					{
						ListView_GetItemText(mf->hwndFrom, lpnmitem->iItem, 0, name, 256);
						ListView_GetItemText(mf->hwndFrom, lpnmitem->iItem, 1, value, 256);

						if (mf->hwndFrom == iInfo.hGlobals)
						{
							strcpy(GlobalName, name);
						}
						else if (mf->hwndFrom == iInfo.hLocals)
						{
							strcpy(LocalName, name);
						}

						if (mf->hwndFrom == iInfo.hGlobals)
							SendMessage(iInfo.hGlobalEdit, WM_SETTEXT, 0, (LPARAM) value);

						if (mf->hwndFrom == iInfo.hLocals)
							SendMessage(iInfo.hLocalEdit, WM_SETTEXT, 0, (LPARAM) value);
					}
			}

			sdu.bVariables = false;
			int iRes = SendMessage(iInfo.hVariablesEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bVariables = true;

			sdu.bEvents = false;
			iRes = SendMessage(iInfo.hEventsEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bEvents = true;

			sdu.bTimers = false;
			iRes = SendMessage(iInfo.hTimersEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bTimers = true;
		}
		break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_SCRIPT_DEBUGGER_BUTTON_PAUSE:
				{
					sdu.bPause = true;
				}
				break;
				case IDC_SCRIPT_DEBUGGER_BUTTON_STEP:
				{
					sdu.bStep = true;
				}
				break;
				case IDC_SCRIPT_DEBUGGER_BUTTON_GLOBAL_UPDATE:
				{
					sdu.bUpdateGlobalVar = true;
				}
				break;
				case IDC_SCRIPT_DEBUGGER_BUTTON_LOCAL_UPDATE:
				{
					sdu.bUpdateLocalVar = true;
				}
				break;
				default:
				{
					//SCRIPT_DEBUGGER_Update_Params();
				} break;
			}

			sdu.bVariables = false;
			int iRes = SendMessage(iInfo.hVariablesEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bVariables = true;

			sdu.bEvents = false;
			iRes = SendMessage(iInfo.hEventsEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bEvents = true;

			sdu.bTimers = false;
			iRes = SendMessage(iInfo.hTimersEnabled, BM_GETCHECK, 0, 0);

			if (iRes == BST_CHECKED) sdu.bTimers = true;
		}
		break;
		case WM_CLOSE:
		case WM_DESTROY:
		{
			//PostQuitMessage(0);
			gbDialog = false;
			Sleep(100);
			EndDialog(hWnd, TRUE);

		}
		break;
		default:
			return FALSE;
			//DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
	}

	return TRUE;
}


//-----------------------------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE _hModule, DWORD _fdwreason, LPVOID _lpReserved)
//-----------------------------------------------------------------------------
{
	switch (_fdwreason)
	{
		case DLL_PROCESS_ATTACH:
		{
			// Récupération du handle d'Instance du Module
			InitCommonControls();
			ghInstance = _hModule;
			break;
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
LPSTR APIFUNC SCRIPT_DEBUGGER_GetName()
{
	return "SCRIPT DEBUGGER";
}

//-----------------------------------------------------------------------------
unsigned long APIFUNC SCRIPT_DEBUGGER_GetVersion()
{
	return MAKELONG(1, 0);
}

//-----------------------------------------------------------------------------
void APIFUNC SCRIPT_DEBUGGER_CreateDialog(HWND _hWindow, ScriptDebuggerInfos & _s)
{
	if (!gbDialog)
	{
		ZeroMemory(&iInfo, sizeof(info));


		long dw = GetLastError();
		ghDialog = CreateDialog(ghInstance, MAKEINTRESOURCE(IDD_SCRIPT_DEBUGGER), _hWindow, (DLGPROC) SCRIPT_DEBUGGER_Proc);
		dw = GetLastError();
		ShowWindow(ghDialog, SW_SHOW);

		sdu.bPause = false;
		sdu.bStep  = false;
		sdu.bUpdateGlobalVar = false;
		sdu.bUpdateLocalVar  = false;
		sdu.bVariables = true;
		sdu.bEvents = true;
		sdu.bTimers = true;

		gbDialog = true;
	}
}

//-----------------------------------------------------------------------------
int CALLBACK MyCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	char * a = (char *) lParam1;
	char * b = (char *) lParam2;
	return strcmp(a, b);
}

//-----------------------------------------------------------------------------
void APIFUNC SCRIPT_DEBUGGER_SetParams(ScriptDebuggerInfos & _s)
{
	if (_s.bClear)
	{
		_s.bClear = false;
		SendMessage(iInfo.hGlobals, LVM_DELETEALLITEMS, 0, 0);
		SendMessage(iInfo.hLocals, LVM_DELETEALLITEMS, 0, 0);
		SendMessage(iInfo.hObjName, WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hTargetName, WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hTargetPos[0], WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hTargetPos[1], WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hTargetPos[2], WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hBehavior, WM_SETTEXT, 0, (LPARAM) "");
		SendMessage(iInfo.hTimers, WM_SETTEXT, (WPARAM) 0, (LPARAM) "");
		SendMessage(iInfo.hEvents, WM_SETTEXT, (WPARAM) 0, (LPARAM) "");
	}

	int i;
	SendMessage(iInfo.hObjName, WM_SETTEXT, 0, (LPARAM) _s.lpszObjName);
	SendMessage(iInfo.hObjPos[0], WM_SETTEXT, 0, (LPARAM) _s.p3ObjPos[0]);
	SendMessage(iInfo.hObjPos[1], WM_SETTEXT, 0, (LPARAM) _s.p3ObjPos[1]);
	SendMessage(iInfo.hObjPos[2], WM_SETTEXT, 0, (LPARAM) _s.p3ObjPos[2]);

	SendMessage(iInfo.hTargetName, WM_SETTEXT, 0, (LPARAM) _s.lpszTargetName);
	SendMessage(iInfo.hTargetPos[0], WM_SETTEXT, 0, (LPARAM) _s.p3TargetPos[0]);
	SendMessage(iInfo.hTargetPos[1], WM_SETTEXT, 0, (LPARAM) _s.p3TargetPos[1]);
	SendMessage(iInfo.hTargetPos[2], WM_SETTEXT, 0, (LPARAM) _s.p3TargetPos[2]);

	//SendMessage(iInfo.hGlobals, LVM_DELETEALLITEMS, 0, 0);
	if (sdu.bVariables)
	{
		// Global Vars
		for (i = 0; i < _s.iNbGlobals; i++)
		{
			int t = -1;
			char buf[256];

			for (int j = 0; j < ListView_GetItemCount(iInfo.hGlobals); j++)
			{
				ListView_GetItemText(iInfo.hGlobals, j, 0, buf, 256);

				if (strcmp(buf, _s.pGlobalVars[i].lpszVarName) == 0)
				{
					t = j;
				}
			}

			if (t != -1)
			{
				ListView_SetItemText(iInfo.hGlobals, t, 0, _s.pGlobalVars[i].lpszVarName);
				ListView_SetItemText(iInfo.hGlobals, t, 1, _s.pGlobalVars[i].lpszVarValue);
			}
			else
			{
				InsertItem(iInfo.hGlobals, _s.pGlobalVars[i].lpszVarName, _s.pGlobalVars[i].lpszVarValue);
				ListView_SortItems(iInfo.hGlobals, MyCompareFunc, 0);
			}

		}

		// Local Vars
		if (_s.iNbLocals == 0) SendMessage(iInfo.hLocals, LVM_DELETEALLITEMS, 0, 0);

		for (i = 0; i < _s.iNbLocals; i++)
		{
			int t = -1;
			char buf[256];

			for (int j = 0; j < ListView_GetItemCount(iInfo.hLocals); j++)
			{
				ListView_GetItemText(iInfo.hLocals, j, 0, buf, 256);

				if (strcmp(buf, _s.pLocalVars[i].lpszVarName) == 0)
				{
					t = j;
				}
			}

			if (t != -1)
			{
				ListView_SetItemText(iInfo.hLocals, t, 0, _s.pLocalVars[i].lpszVarName);
				ListView_SetItemText(iInfo.hLocals, t, 1, _s.pLocalVars[i].lpszVarValue);
			}
			else
			{
				InsertItem(iInfo.hLocals, _s.pLocalVars[i].lpszVarName, _s.pLocalVars[i].lpszVarValue);
				ListView_SortItems(iInfo.hLocals, MyCompareFunc, 0);
			}
		}
	}

	SendMessage(iInfo.hBehavior, WM_SETTEXT, 0, (LPARAM) _s.lpszBehavior);

	if (sdu.bEvents)
	{
		if (_s.bEvents)
		{
			int iSize = SendMessage(iInfo.hEvents, WM_GETTEXTLENGTH, 0, 0);
			SendMessage(iInfo.hEvents, EM_SETSEL, (WPARAM) iSize, (LPARAM) iSize);
			SendMessage(iInfo.hEvents, EM_REPLACESEL, (WPARAM) true, (LPARAM) _s.lpszEvents);
			// slide tout en bas
			SendMessage(iInfo.hEvents, WM_VSCROLL, SB_BOTTOM, 0);
		}
	}

	if (sdu.bTimers)
	{
		if (_s.bTimers)
		{
			SendMessage(iInfo.hTimers, WM_SETTEXT, (WPARAM) 0, (LPARAM) _s.lpszTimers);
			// slide tout en bas
			SendMessage(iInfo.hTimers, WM_VSCROLL, SB_BOTTOM, 0);
		}
	}

	UpdateWindow(ghDialog);
}

//-----------------------------------------------------------------------------
void APIFUNC SCRIPT_DEBUGGER_GetParams(ScriptDebuggerUpdate & _pp)
{
	sdu.bEvents = false;
	int iRes = SendMessage(iInfo.hEventsEnabled, BM_GETCHECK, 0, 0);

	if (iRes == BST_CHECKED) sdu.bEvents = true;

	sdu.bTimers = false;
	iRes = SendMessage(iInfo.hTimersEnabled, BM_GETCHECK, 0, 0);

	if (iRes == BST_CHECKED) sdu.bTimers = true;

	_pp.bVariables = sdu.bVariables;
	_pp.bEvents = sdu.bEvents;
	_pp.bTimers = sdu.bTimers;
	_pp.bPause = sdu.bPause;
	_pp.bStep = sdu.bStep;


	char dest[256];

	if (_pp.bUpdateGlobalVar = sdu.bUpdateGlobalVar)
	{
		GetWindowText(iInfo.hGlobalEdit, dest, 255);
		strcpy(_pp.globalVar.lpszVarValue, dest);
		strcpy(_pp.globalVar.lpszVarName, GlobalName);
	}

	if (_pp.bUpdateLocalVar = sdu.bUpdateLocalVar)
	{
		GetWindowText(iInfo.hLocalEdit, dest, 255);
		strcpy(_pp.localVar.lpszVarValue, dest);
		strcpy(_pp.localVar.lpszVarName, LocalName);
	}

	sdu.bUpdateGlobalVar = false;
	sdu.bUpdateLocalVar  = false;
	sdu.bPause = false;
	sdu.bStep = false;
}

//-----------------------------------------------------------------------------
bool APIFUNC SCRIPT_DEBUGGER_WindowOpened()
{
	return gbDialog;
}

//-----------------------------------------------------------------------------
void APIFUNC SCRIPT_DEBUGGER_Destroy()
{
	if (gbDialog)
		SendMessage(ghDialog, WM_CLOSE, 0, 0);
}

