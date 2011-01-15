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
// DanaeDlg.CPP
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DANAE Dialog Box Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define _ARX_CEDITOR_  0
#include <DANAE_VERSION.h>
#include <stdio.h>
#include <windows.h>
#include <direct.h>

#include "Danae_Resource.h"
#include "Danaedlg.h"
#include "ARX_Interface.h"
#include "ARX_Paths.h"
#include "ARX_Sound.h"
#if _ARX_CEDITOR_
#include "Ceditor_Ceditor.h"
#endif // _ARX_CEDITOR_
#include "ARX_GlobalMods.h"
#include "ARX_Particles.h"
#include "ARX_Snapshot.h"
#include "ARX_Text.h"
#include "ARX_Time.h"

#include <EERIEUtil.h>
#include <EERIEClothes.h>
#include <EERIELight.h>
#include <EERIETexture.h>

#include <HERMESMain.h>
#include <EERIEMath.h>

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long CURRENTSNAPNUM;
extern long SnapShotMode;

extern long USE_D3DFOG;
extern long ARX_DEMO;
extern long NOCHECKSUM;
extern long ZMAPMODE;
extern long TreatAllIO;
extern long HIDEMAGICDUST;
extern long LaunchDemo;
extern long LIGHTPOWERUP;
extern float LPpower;
extern long D3DTRANSFORM;
extern long USE_PLAYERCOLLISIONS;
extern long A_FLARES;
extern long NODIRCREATION;
extern long MAPUPDATE;
extern long EXTERNALVIEWING;
extern long DYNAMIC_NORMALS;
extern long SHOWSHADOWS;
extern long HIPOLY;
extern long BLURTEXTURES;
extern long NOMIPMAPS;
extern long POINTINTERPOLATION;
extern long ForceIODraw;
extern long NEED_ANCHORS;
long HIDEANCHORS = 1;
extern long HERMES_KEEP_MEMORY_TRACE;
extern char * GTE_TITLE;
extern char * GTE_TEXT;
extern long GTE_SIZE;
extern float TIMEFACTOR;
extern long ALLOW_MESH_TWEAKING;
extern long DEBUG_MOLLESS;
extern long HIDESPEECH;
extern HWND MESH_REDUCTION_WINDOW;
extern HWND PRECALC;
extern HANDLE LIGHTTHREAD;
extern long PROGRESS_COUNT;
extern long PROGRESS_TOTAL;
extern long PAUSED_PRECALC;

COLORREF custcr[16];
long accepted = 0;

 
BOOL CALLBACK SnapShotDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

long InitMemorySnaps();
void FlushMemorySnaps(long snap);


#define CHECK 1
#define UNCHECK 0

//*************************************************************************************
//*************************************************************************************

void SetCheck(HWND hWnd, int id, long chk)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);

	if (chk == 0) SendMessage(thWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	else if (chk == 1) SendMessage(thWnd, BM_SETCHECK, BST_CHECKED, 0);
}

//*************************************************************************************
//*************************************************************************************

BOOL IsChecked(HWND hWnd, int id)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);

	if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED) return TRUE;

	return FALSE;
}

//*************************************************************************************
//*************************************************************************************

void SetClick(HWND hWnd, int id)
{
	HWND thWnd;
	thWnd = GetDlgItem(hWnd, id);
	SendMessage(thWnd, BM_CLICK, 0, 0);
}

float FORCED_REDUCTION_VALUE = 0.f;
BOOL CALLBACK MeshReductionProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam)
{
	HWND thWnd;
	long t;

	switch (uMsg)
	{
		case WM_NOTIFY:
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
			t = SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			FORCED_REDUCTION_VALUE = (float)t * DIV10000;
			break;
		case WM_CLOSE:
			MESH_REDUCTION_WINDOW = NULL;
			EndDialog(hWnd, TRUE);
			return FALSE;
		case WM_INITDIALOG:
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 10000));
			t = (long)(FORCED_REDUCTION_VALUE * 10000.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));
			return TRUE;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_REDUCTION);
					t = SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
					FORCED_REDUCTION_VALUE = (float)t * DIV10000;
					MESH_REDUCTION_WINDOW = NULL;
					EndDialog(hWnd, TRUE);
					break;
			}

			break;
	}

	return FALSE;
}

char ERRORSTRING[65535];
char ERRORTITLE[512];
HWND ShowErrorPopup(char * title, char * tex)
{
	strcpy(ERRORTITLE, title);
	strcpy(ERRORSTRING, tex);

	if (danaeApp.m_pFramework->m_bIsFullscreen)
	{
		ARX_TIME_Pause();
		danaeApp.Pause(TRUE);
		DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
		          MAKEINTRESOURCE(IDD_SCRIPTERROR), danaeApp.m_hWnd, IDDErrorLogProc);
		danaeApp.Pause(FALSE);
		ARX_TIME_UnPause();
		return NULL;
	}

	HWND hdl = CreateDialogParam((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
	                             MAKEINTRESOURCE(IDD_SCRIPTERROR), danaeApp.m_hWnd, IDDErrorLogProc, 0);
	return hdl;
}
BOOL CALLBACK IDDErrorLogProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
	HWND thWnd;

	switch (uMsg)
	{
		case WM_CLOSE:
			EndDialog(hWnd, TRUE);
			return FALSE;
		case WM_INITDIALOG:
			thWnd = GetDlgItem(hWnd, IDC_ERRORLOG);
			SetWindowText(thWnd, ERRORSTRING);
			thWnd = GetDlgItem(hWnd, IDC_ERRORSTRING);
			SetWindowText(thWnd, ERRORTITLE);
			return TRUE;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hWnd, TRUE);
					break;
			}

			break;
	}

	return FALSE;
}
extern long ARX_PATHS_HIERARCHYMOVE;

BOOL CALLBACK PathwayOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam)
{
	HWND thWnd;

	switch (uMsg)
	{
		case WM_CLOSE:
			CDP_PATHWAYS_Options = NULL;
			EndDialog(hWnd, TRUE);
			return FALSE;
		case WM_NOTIFY:
			char temp[256];
			float fval;
			long val;

			thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
			val = (long)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			fval = (float)(val) * DIV10;
			thWnd = GetDlgItem(hWnd, IDC_FCLIPTEXT);
			sprintf(temp, "%3.1f m", fval);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER2);
			val = (long)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_MAXVOLTEXT);
			sprintf(temp, "%d%%", val);
			SetWindowText(thWnd, temp);

			if (ARX_PATHS_SelectedAP)
			{
				D3DCOLOR col = EERIERGB(ARX_PATHS_SelectedAP->rgb.r, ARX_PATHS_SelectedAP->rgb.g, ARX_PATHS_SelectedAP->rgb.b);
				COLORREF rgbResult = ((col >> 16 & 255))
				                     | ((col >> 8 & 255) << 8)
				                     | ((col & 255) << 16);
				thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
				HDC dc;
				InvalidateRect(thWnd, NULL, TRUE);

				if (dc = GetDC(thWnd))
				{
					RECT rect;
					GetClientRect(thWnd, &rect);
					HBRUSH brush = CreateSolidBrush(rgbResult);
					SelectObject(dc, brush);
					FillRect(dc, &rect, brush);
					DeleteObject(brush);
					ValidateRect(thWnd, NULL);
					ReleaseDC(thWnd, dc);
				}
			}

			break;
		case WM_INITDIALOG:


			if (ARX_PATHS_HIERARCHYMOVE)
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
				SetWindowText(thWnd, "Hierarchy Edition");
			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
				SetWindowText(thWnd, "WayPoint Edition");
			}

			if ((ARX_PATHS_SelectedAP == NULL) ||
			        (ARX_PATHS_SelectedNum == -1))
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				ShowWindow(thWnd, SW_HIDE);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYBEZIER);
				ShowWindow(thWnd, SW_HIDE);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
				ShowWindow(thWnd, SW_SHOW);

			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				ShowWindow(thWnd, SW_SHOW);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYBEZIER);
				ShowWindow(thWnd, SW_SHOW);
				thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
				ShowWindow(thWnd, SW_SHOW);
			}

			if ((ARX_PATHS_SelectedAP != NULL) &&
			        (ARX_PATHS_SelectedNum != -1))
			{
				D3DCOLOR col = EERIERGB(ARX_PATHS_SelectedAP->rgb.r, ARX_PATHS_SelectedAP->rgb.g, ARX_PATHS_SelectedAP->rgb.b);
				COLORREF rgbResult = ((col >> 16 & 255))
				                     | ((col >> 8 & 255) << 8)
				                     | ((col & 255) << 16);
				thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
				HDC dc;
				InvalidateRect(thWnd, NULL, TRUE);

				if (dc = GetDC(thWnd))
				{
					RECT rect;
					GetClientRect(thWnd, &rect);
					HBRUSH brush = CreateSolidBrush(rgbResult);
					SelectObject(dc, brush);
					FillRect(dc, &rect, brush);
					DeleteObject(brush);
					ValidateRect(thWnd, NULL);
					ReleaseDC(thWnd, dc);
				}



				if (ARX_PATHS_SelectedAP->flags & PATH_LOOP)
				{
					thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
					SetWindowText(thWnd, "Loop");
				}
				else
				{
					thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
					SetWindowText(thWnd, "No Loop");
				}

				////////////////////////////////////////////////	NEW
				if (ARX_PATHS_SelectedAP->flags & PATH_AMBIANCE)
				{
					SetCheck(hWnd, IDC_AMBIANCE, CHECK);
					thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
					SetWindowText(thWnd, ARX_PATHS_SelectedAP->ambiance);
				}
				else
				{
					SetCheck(hWnd, IDC_AMBIANCE, UNCHECK);
					thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
					SetWindowText(thWnd, "NONE");
				}

				if (ARX_PATHS_SelectedAP->flags & PATH_RGB)
				{
					SetCheck(hWnd, IDC_FADECOLOR, CHECK);
				}
				else
				{
					SetCheck(hWnd, IDC_FADECOLOR, UNCHECK);
				}

				thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
				SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(10, 400));

				if (ARX_PATHS_SelectedAP->flags & PATH_FARCLIP)
				{
					SetCheck(hWnd, IDC_CLIPPINGFAR, CHECK);
					long t = ARX_CLEAN_WARN_CAST_LONG((long)ARX_PATHS_SelectedAP->farclip * DIV10);
					SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));
				}
				else
				{
					SetCheck(hWnd, IDC_CLIPPINGFAR, UNCHECK);
					long t = (long)280;
					SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));
				}

				thWnd = GetDlgItem(hWnd, IDC_SLIDER2);	//Reverb
				SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 100)); // %

				if (ARX_PATHS_SelectedAP->flags & PATH_REVERB)
				{
					SetCheck(hWnd, IDC_REVERB, CHECK);
				}
				else
				{
					SetCheck(hWnd, IDC_REVERB, UNCHECK);
				}

				long t;
				F2L((float)(ARX_PATHS_SelectedAP->amb_max_vol), &t);

				if (t <= 1) t = 100;

				SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));
				thWnd = GetDlgItem(hWnd, IDC_MAXVOLTEXT);
				sprintf(temp, "%d", t);
				SetWindowText(thWnd, temp);
				////////////////////////////////////////////////	END NEW

				if (ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1].flag == PATHWAY_BEZIER)
					SetCheck(hWnd, IDC_PATHWAYBEZIER, CHECK);
				else SetCheck(hWnd, IDC_PATHWAYBEZIER, UNCHECK);

				if (ARX_PATHS_SelectedAP->height != 0)
				{
					SetCheck(hWnd, IDC_ZONE, CHECK);
					thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
					char str[20];

					if (ARX_PATHS_SelectedAP->height < 0)
						SetWindowText(thWnd, "0");
					else
					{
						sprintf(str, "%d", ARX_PATHS_SelectedAP->height);
						SetWindowText(thWnd, str);
					}
				}
				else
				{
					SetCheck(hWnd, IDC_ZONE, UNCHECK);
					thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
					SetWindowText(thWnd, "0");
				}

				thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
				char str[20];
				sprintf(str, "%.2f", ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1]._time);
				SetWindowText(thWnd, str);

				thWnd = GetDlgItem(hWnd, IDC_EDITNAME);
				char str2[64];
				sprintf(str2, "%s", ARX_PATHS_SelectedAP->name);
				SetWindowText(thWnd, str2);
			}
			else
			{
				SetCheck(hWnd, IDC_ZONE, UNCHECK);
				thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
				SetWindowText(thWnd, "0");
			}

			return TRUE;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_COLORCHOOSE:

					if (ARX_PATHS_SelectedAP != NULL)
					{
						CHOOSECOLOR cc;
						cc.lStructSize = sizeof(CHOOSECOLOR);
						cc.hwndOwner = hWnd;
						cc.hInstance = 0; //Ignored
						D3DCOLOR col = EERIERGB(ARX_PATHS_SelectedAP->rgb.r, ARX_PATHS_SelectedAP->rgb.g, ARX_PATHS_SelectedAP->rgb.b);
						cc.rgbResult = ((col >> 16 & 255))
						               | ((col >> 8 & 255) << 8)
						               | ((col & 255) << 16);
						cc.lpCustColors = custcr;
						cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
						ChooseColor(&cc);
						ARX_PATHS_SelectedAP->rgb.r = (float)((long)(cc.rgbResult & 255)) * DIV255;
						ARX_PATHS_SelectedAP->rgb.g = (float)((long)((cc.rgbResult >> 8) & 255)) * DIV255;
						ARX_PATHS_SelectedAP->rgb.b = (float)((long)((cc.rgbResult >> 16) & 255)) * DIV255;
						thWnd = GetDlgItem(hWnd, IDC_SHOWCOLOR);
						HDC dc;
						InvalidateRect(thWnd, NULL, TRUE);

						if (dc = GetDC(thWnd))
						{
							RECT rect;
							GetClientRect(thWnd, &rect);
							HBRUSH brush = CreateSolidBrush(cc.rgbResult);
							SelectObject(dc, brush);
							FillRect(dc, &rect, brush);
							DeleteObject(brush);
							ValidateRect(thWnd, NULL);
							ReleaseDC(thWnd, dc);
						}
					}

					break;
				case IDC_PATHWAYLOOP:

					if ((ARX_PATHS_SelectedAP != NULL) &&
					        (ARX_PATHS_SelectedNum != -1))
					{
						if (ARX_PATHS_SelectedAP->flags & PATH_LOOP)
						{
							thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
							SetWindowText(thWnd, "No Loop");
							ARX_PATHS_SelectedAP->flags &= ~PATH_LOOP;
						}
						else
						{
							thWnd = GetDlgItem(hWnd, IDC_PATHWAYLOOP);
							SetWindowText(thWnd, "Loop");
							ARX_PATHS_SelectedAP->flags |= PATH_LOOP;
						}
					}

					break;
				case IDC_PATHWAYHIERARCHY:

					if (ARX_PATHS_HIERARCHYMOVE)
					{
						ARX_PATHS_HIERARCHYMOVE = 0;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
						SetWindowText(thWnd, "WayPoint Edition");
					}
					else
					{
						ARX_PATHS_HIERARCHYMOVE = ARX_PATH_HIERARCHY;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYHIERARCHY);
						SetWindowText(thWnd, "Hierarchy Edition");
					}

					break;
				case IDOK:
				case IDAPPLY:

					if ((ARX_PATHS_SelectedAP != NULL) &&
					        (ARX_PATHS_SelectedNum != -1))
					{
						if (IsChecked(hWnd, IDC_PATHWAYBEZIER))
							ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP, ARX_PATHS_SelectedNum, ARX_PATH_MOD_FLAGS, NULL, PATHWAY_BEZIER, 0);
						else ARX_PATHS_ModifyPathWay(ARX_PATHS_SelectedAP, ARX_PATHS_SelectedNum, ARX_PATH_MOD_FLAGS, NULL, PATHWAY_STANDARD, 0);

						HWND thWnd;
						thWnd = GetDlgItem(hWnd, IDC_PATHWAYTIME);
						char str[20];
						GetWindowText(thWnd, str, 20);
						ARX_PATHS_SelectedAP->pathways[ARX_PATHS_SelectedNum-1]._time = (float)atof(str);


						if (IsChecked(hWnd, IDC_ZONE))
						{
							thWnd = GetDlgItem(hWnd, IDC_EDITHEIGHT);
							char str[20];
							GetWindowText(thWnd, str, 20);
							float val = (float)atof(str);

							if (val <= 0)
							{
								ARX_PATHS_SelectedAP->height = -1;
							}
							else ARX_PATHS_SelectedAP->height = (long)val;
						}
						else
						{
							ARX_PATHS_SelectedAP->height = 0;
						}

						if (IsChecked(hWnd, IDC_AMBIANCE))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_AMBIANCE;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_AMBIANCE;
						}

						thWnd = GetDlgItem(hWnd, IDC_AMBIANCETEXT);
						GetWindowText(thWnd, ARX_PATHS_SelectedAP->ambiance, 127);

						if (IsChecked(hWnd, IDC_FADECOLOR))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_RGB;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_RGB;
						}

						if (IsChecked(hWnd, IDC_CLIPPINGFAR))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_FARCLIP;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_FARCLIP;
						}

						thWnd = GetDlgItem(hWnd, IDC_SLIDER1);
						val	=	SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
						fval =	(float)(val) * 10.f;
						ARX_PATHS_SelectedAP->farclip = fval;

						if (IsChecked(hWnd, IDC_REVERB))
						{
							ARX_PATHS_SelectedAP->flags |= PATH_REVERB;
						}
						else
						{
							ARX_PATHS_SelectedAP->flags &= ~PATH_REVERB;
						}

						thWnd = GetDlgItem(hWnd, IDC_SLIDER2);
						val	=	SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
						fval =	(float)(val);
						ARX_PATHS_SelectedAP->amb_max_vol = fval;
						thWnd = GetDlgItem(hWnd, IDC_EDITNAME);
						char str2[64];
						GetWindowText(thWnd, str2, 63);
						ARX_PATH * ap = ARX_PATHS_ExistName(str2);

						if ((ap != ARX_PATHS_SelectedAP) && (ap != NULL))
							ShowPopup("This Name is already used by another path, New name ignored...");
						else ARX_PATHS_ChangeName(ARX_PATHS_SelectedAP, str2);

						if (LOWORD(wParam) == IDOK)
						{
							CDP_PATHWAYS_Options = NULL;
							EndDialog(hWnd, TRUE);
						}
					}

					break;
				case IDCANCEL:
					CDP_PATHWAYS_Options = NULL;
					EndDialog(hWnd, TRUE);
					break;
			}

			break;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------------

HWND InterObjDlg = NULL;
HWND dlgTreeViewhWnd = NULL;
WNDPROC lpfnOldWndProc;
typedef struct
{
	HTREEITEM hti;
	char text[260];
	INTERACTIVE_OBJ * io;
} TVINFO;

#define MAXTVV 5000
TVINFO * tvv[MAXTVV];
long TVVcount = 0;
EERIE_3D TVCONTROLEDplayerpos;
long TVCONTROLED = 0;
HTREEITEM hfix = NULL;
HTREEITEM hitem = NULL;
HTREEITEM hnpc = NULL;
HTREEITEM hroot = NULL;
HTREEITEM hcam = NULL;
HTREEITEM hmarker = NULL;
HTREEITEM hpath = NULL;

void InterTreeViewGotoPosition(HTREEITEM hitem)
{
	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (tvv[i]->io != NULL)
				{
					TVCONTROLEDplayerpos.x = tvv[i]->io->pos.x + (float)EEsin(DEG2RAD(player.angle.b)) * 100.f;
					TVCONTROLEDplayerpos.y = tvv[i]->io->pos.y - 80.f;
					TVCONTROLEDplayerpos.z = tvv[i]->io->pos.z - (float)EEcos(DEG2RAD(player.angle.b)) * 100.f;
					TVCONTROLED = 1;
				}
			}
		}
	}

	for (int i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				ARX_PATH * ap = ARX_PATH_GetAddressByName(tvv[i]->text);

				if (ap != NULL)
				{
					TVCONTROLEDplayerpos.x = ap->initpos.x + (float)EEsin(DEG2RAD(player.angle.b)) * 100.f;
					TVCONTROLEDplayerpos.y = ap->initpos.y - 80.f;
					TVCONTROLEDplayerpos.z = ap->initpos.z - (float)EEcos(DEG2RAD(player.angle.b)) * 100.f;
					TVCONTROLED = 1;
				}
			}
		}
	}
}
void InterTreeSelectObject(HTREEITEM hitem)
{
	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (!stricmp(tvv[i]->text, "player"))
				{
					if (inter.iobj[0])
					{
						if (inter.iobj[0]->EditorFlags & EFLAG_SELECTED)
						{
							UnSelectIO(inter.iobj[0]);
						}
						else
						{
							SelectIO(inter.iobj[0]);
						}

						return;
					}
				}

				if (tvv[i]->io != NULL)
				{
					if (tvv[i]->io->EditorFlags & EFLAG_SELECTED)
					{
						UnSelectIO(tvv[i]->io);
					}
					else
					{
						SelectIO(tvv[i]->io);
					}
				}
			}
		}
	}

	for (int i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				ARX_PATH * ap = ARX_PATH_GetAddressByName(tvv[i]->text);

				if (ap != NULL)
				{
					ARX_PATHS_SelectedAP = ap;
					ARX_PATHS_SelectedNum = 1;
				}
			}
		}
	}
}

BOOL CALLBACK InteractiveObjDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam)
{
	HTREEITEM hitem;

	switch (uMsg)
	{
		case WM_MOVE:

			if ((danaeApp.m_pDeviceInfo->bWindowed) && (danaeApp.m_hWnd != NULL))
			{
				RECT rect1, rect2;
				GetWindowRect(danaeApp.m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_IO_DlgProc_POSX", posx);
				Danae_Registry_WriteValue("WND_IO_DlgProc_POSY", posy);
			}

			break;
		case WM_CLOSE:
			KillInterTreeView();
			return FALSE;
		case WM_INITDIALOG:

			if ((danaeApp.m_pDeviceInfo->bWindowed) && (danaeApp.m_hWnd != NULL))
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_IO_DlgProc_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_IO_DlgProc_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > 0)
				        && (posy < 800) && (posy > 0)
				   )
				{
					RECT rect1;
					GetWindowRect(danaeApp.m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			return TRUE;
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_GOTOOBJECT:
					hitem = TreeView_GetSelection(dlgTreeViewhWnd);
					InterTreeViewGotoPosition(hitem);
					break;
				case IDC_SELECTOBJECT:
					hitem = TreeView_GetSelection(dlgTreeViewhWnd);
					InterTreeSelectObject(hitem);
					break;
			}

			break;
	}

	return FALSE;
}
void InterTreeViewDisplayInfo(HTREEITEM hitem)
{
	char texx[512];

	for (long i = 0; i < TVVcount; i++)
	{
		if (tvv[i] != NULL)
		{
			if (tvv[i]->hti == hitem)
			{
				if (!stricmp(tvv[i]->text, "player"))
				{
					sprintf(texx, "%s", tvv[i]->text);
					SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
					return;
				}

				if (tvv[i]->io == NULL)
				{
					sprintf(texx, "%s", tvv[i]->text);
					SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
					return;
				}

				char typee[16];
				strcpy(typee, "X");

				if (tvv[i]->io->ioflags & IO_NPC) strcpy(typee, "NPC IO");

				if (tvv[i]->io->ioflags & IO_ITEM) strcpy(typee, "ITEM IO");

				if (tvv[i]->io->ioflags & IO_FIX) strcpy(typee, "FIX IO");

				if (tvv[i]->io->ioflags & IO_CAMERA) strcpy(typee, "Camera");

				if (tvv[i]->io->ioflags & IO_MARKER) strcpy(typee, "Marker");

				sprintf(texx, "Ident: %s\n%s NumIO %d\nPos X:%d Y:%d Z:%d\nShow %d Lvl %d (%d)"
				        , tvv[i]->text, typee, GetInterNum(tvv[i]->io), (long)tvv[i]->io->pos.x, (long)tvv[i]->io->pos.y, (long)tvv[i]->io->pos.z,
				        (long)tvv[i]->io->show, tvv[i]->io->level, tvv[i]->io->truelevel
				       );
				SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
				return;
			}
		}
	}

	strcpy(texx, "No Interactive Object Selected");
	SetDlgItemText(InterObjDlg, IDC_INTERTEXT, texx);
}
LONG CALLBACK InterTreeViewSubClassFunc(HWND hWnd,
                                        UINT uMsg, WORD wParam, LONG lParam)
{
	switch (uMsg)
	{

		case WM_LBUTTONUP:
			HTREEITEM hitem;
			hitem = TreeView_GetSelection(dlgTreeViewhWnd);
			InterTreeViewDisplayInfo(hitem);
			break;
	}

	return CallWindowProc((WNDPROC)lpfnOldWndProc, hWnd, uMsg, wParam,
	                      lParam);
}


void RemoveIOTVItem(HWND tvhwnd, INTERACTIVE_OBJ * io, char * name, long type)
{
	if (TVVcount != 0)
		if (io != NULL)
		{
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					if (tvv[i]->io == io)
					{
						TreeView_DeleteItem(tvhwnd, tvv[i]->hti);
						free(tvv[i]);
						tvv[i] = NULL;
						TVVcount--;

						while (i < TVVcount)
						{
							tvv[i] = tvv[i+1];
							i++;
						}

						return;
					}
				}
			}
		}
		else
		{
			// path removal
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					if (!stricmp(name, tvv[i]->text))
					{
						TreeView_DeleteItem(tvhwnd, tvv[i]->hti);
						free(tvv[i]);

						while (i < TVVcount - 1)
						{
							tvv[i] = tvv[i+1];
							i++;
						}

						TVVcount--;
						return;
					}
				}
			}

		}
}

void AddIOTVItem(HWND tvhwnd, INTERACTIVE_OBJ * io, char * name, long type)
{
	TVINSERTSTRUCT tis;
	HTREEITEM parent = NULL;
	char temp[512];
	char temp2[512];


	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	memset(tvv[TVVcount], 0, sizeof(TVINFO));

	if (type == IOTVTYPE_PLAYER) strcpy(temp, "PLAYER");
	else if (io != NULL)
	{
		strcpy(temp, GetName(io->filename));
		sprintf(temp2, "_%04d", io->ident);
		strcat(temp, temp2);
	}
	else strcpy(temp, name);

	strcpy(tvv[TVVcount]->text, temp);
	tvv[TVVcount]->io = io;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;

	if (type == IOTVTYPE_PLAYER) tis.hParent = hroot;
	else if (type == IOTVTYPE_PATH) tis.hParent = hpath;
	else if (io)
	{
		if (io->ioflags & IO_NPC) tis.hParent = hnpc;

		if (io->ioflags & IO_FIX) tis.hParent = hfix;

		if (io->ioflags & IO_ITEM) tis.hParent = hitem;

		if (io->ioflags & IO_CAMERA) tis.hParent = hcam;

		if (io->ioflags & IO_MARKER) tis.hParent = hmarker;
	}

	tis.hInsertAfter = TVI_SORT;
	parent = TreeView_InsertItem(tvhwnd, &tis);
	tvv[TVVcount]->hti = parent;
	TVVcount++;
	InterTreeViewDisplayInfo(parent);
}
void FillInterTreeView(HWND tvhwnd)
{
	long i;
	TVINSERTSTRUCT tis;

 
	HTREEITEM hti = NULL;

	TreeView_DeleteAllItems(tvhwnd);
	TreeView_SetBkColor(tvhwnd, 0x00000000);
	TreeView_SetTextColor(tvhwnd, 0x00FFFFFF);

	if (TVVcount != 0)
		for (i = 0; i < TVVcount; i++)
		{
			if (tvv[i] != NULL)
			{
				free(tvv[i]);
				tvv[i] = NULL;
			}
		}

	TVVcount = 0;
	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Root");
	tvv[TVVcount]->io = NULL;

	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = NULL;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hroot = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Camera");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hcam = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Marker");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hmarker = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "FIX");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hfix = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "ITEMS");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hitem = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "NPC");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hnpc = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	tvv[TVVcount] = (TVINFO *)malloc(sizeof(TVINFO));
	sprintf(tvv[TVVcount]->text, "Path");
	tvv[TVVcount]->io = NULL;
	memset(&tis, 0, sizeof(TVINSERTSTRUCT));
	tis.hParent = hroot;
	tis.hInsertAfter = TVI_SORT;
	tis.item.mask = TVIS_EXPANDED | TVIF_TEXT | TVIF_CHILDREN | TVIF_STATE;
	tis.item.pszText = tvv[TVVcount]->text;
	tis.item.cchTextMax = strlen(tvv[TVVcount]->text);
	tis.item.cChildren = 1;
	hti = TreeView_InsertItem(tvhwnd, &tis);
	hpath = hti;
	tvv[TVVcount]->hti = hti;
	TVVcount++;

	InterTreeViewDisplayInfo(hroot);

	AddIOTVItem(tvhwnd, inter.iobj[0], NULL, IOTVTYPE_PLAYER);

	for (i = 0; i < inter.nbmax; i++)
	{
		if (inter.iobj[i] != NULL)
		{
			AddIOTVItem(tvhwnd, inter.iobj[i], NULL, 0);

		}
	}

	TreeView_Expand(tvhwnd, hroot, TVE_EXPAND);
}
 
 
 
 
void InterTreeViewItemRemove(INTERACTIVE_OBJ * io, char * name, long type)
{
	if (InterObjDlg) RemoveIOTVItem(dlgTreeViewhWnd, io, name, type);
}
void InterTreeViewItemAdd(INTERACTIVE_OBJ * io, char * name, long type)
{
	if (InterObjDlg) AddIOTVItem(dlgTreeViewhWnd, io, name, type);
}

void LaunchInteractiveObjectsApp(HWND hwnd)
{
	if (!danaeApp.m_pDeviceInfo->bWindowed)
		return;

	if (InterObjDlg) return;

	InterObjDlg = CreateDialogParam(
	                  (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
	                  MAKEINTRESOURCE(IDD_INTERDLG),
	                  hwnd,
	                  InteractiveObjDlgProc, 0);

	dlgTreeViewhWnd = GetDlgItem(InterObjDlg, IDC_INTERTREEVIEW);
	lpfnOldWndProc = (WNDPROC)SetWindowLong(dlgTreeViewhWnd,
	                                        GWL_WNDPROC, (DWORD)InterTreeViewSubClassFunc);
	FillInterTreeView(dlgTreeViewhWnd);
	ShowWindow(InterObjDlg, SW_SHOW);
}
void KillInterTreeView()
{
	if (InterObjDlg)
	{
		if (TVVcount != 0)
			for (long i = 0; i < TVVcount; i++)
			{
				if (tvv[i] != NULL)
				{
					free(tvv[i]);
					tvv[i] = NULL;
				}
			}

		EndDialog(InterObjDlg, TRUE);
		InterObjDlg = NULL;
	}
}

char rett[128];
extern long FINAL_COMMERCIAL_DEMO;

//-----------------------------------------------------------------------------------
char * GetVersionString()
{
	char temp[128];
	char result[128];

	if (FINAL_COMMERCIAL_DEMO)
		strcpy(temp, " SCDaAe$!m^;o|(_______"); //Demo v1.0
	else
		strcpy(temp, _ARX_FINAL_VERSION_ );

	long length = strlen(temp);
	long pos = 0;

	for (long i = 0; i < length; i += 3)
	{
		if (temp[i] != '_')
			result[pos++] = temp[i];
		else
			result[pos++] = 0;
	}

	strcpy(rett, result);
	return rett;
}

//*************************************************************************************
// Sets DANAE Main Window Title
//*************************************************************************************
void SetWindowTitle(HWND hWnd, char * tex)
{
	char texx[512];
	strcpy(texx, tex);
	strcat(texx, GetVersionString());
	SetWindowText(hWnd, texx);
}

HWND SnapShotDlg = NULL;
void LaunchSnapShotParamApp(HWND hwnd)
{
	if (SnapShotDlg) return;

	SnapShotDlg = CreateDialogParam(
	                  (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
	                  MAKEINTRESOURCE(IDD_SNAPSHOT),
	                  hwnd,
	                  SnapShotDlgProc, 0);

}

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK SnapShotDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char temp[256];
	HWND thWnd;

	switch (uMsg)
	{
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_16BITS:

					if (snapshotdata.bits == 16) snapshotdata.bits = 24;
					else snapshotdata.bits = 16;

					thWnd = GetDlgItem(hWnd, IDC_16BITS);

					if (snapshotdata.bits == 16) SetWindowText(thWnd, "16 Bits");
					else SetWindowText(thWnd, "24 Bits");

					break;
				case IDC_SETPATH:
					HERMESFolderSelector(snapshotdata.path, "Choose Working Folder");
					thWnd = GetDlgItem(hWnd, IDC_SETPATH);
					SetWindowText(thWnd, snapshotdata.path);
					break;
				case IDOK:

					thWnd = GetDlgItem(hWnd, IDC_IMAGESSEC);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.imgsec = atoi(temp);

					if (snapshotdata.imgsec < 1) snapshotdata.imgsec = 1;

					if (snapshotdata.imgsec > 100) snapshotdata.imgsec = 100;

					thWnd = GetDlgItem(hWnd, IDC_XSIZE);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.xsize = atoi(temp);

					if (snapshotdata.xsize < 1) snapshotdata.xsize = 1;

					if (snapshotdata.xsize > 640) snapshotdata.xsize = 640;

					thWnd = GetDlgItem(hWnd, IDC_YSIZE);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.ysize = atoi(temp);

					if (snapshotdata.ysize < 1) snapshotdata.ysize = 1;

					if (snapshotdata.ysize > 480) snapshotdata.ysize = 480;


					if (IsChecked(hWnd, IDC_MEMORYCACHE))	snapshotdata.flag |= 1;
					else snapshotdata.flag &= ~1;

					thWnd = GetDlgItem(hWnd, IDC_EDITFILENAMES);
					GetWindowText(thWnd, temp, 128);
					strcpy(snapshotdata.filenames, temp);

					SnapShotDlg = NULL;
					EndDialog(hWnd, TRUE);
					break;
				case IDCANCELSNAP:

					if (SnapShotMode)
					{
						FlushMemorySnaps(0);
						SnapShotMode = 0;
						thWnd = GetDlgItem(hWnd, IDSTARTSNAPSHOT);
						SetWindowText(thWnd, "Start Snapshot");
						CURRENTSNAPNUM = 0;
					}

					break;
				case IDSTARTSNAPSHOT:
					thWnd = GetDlgItem(hWnd, IDC_IMAGESSEC);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.imgsec = atoi(temp);

					if (snapshotdata.imgsec < 1) snapshotdata.imgsec = 1;

					if (snapshotdata.imgsec > 100) snapshotdata.imgsec = 100;

					thWnd = GetDlgItem(hWnd, IDC_XSIZE);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.xsize = atoi(temp);

					if (snapshotdata.xsize < 1) snapshotdata.xsize = 1;

					if (snapshotdata.xsize > 640) snapshotdata.xsize = 640;

					thWnd = GetDlgItem(hWnd, IDC_YSIZE);
					GetWindowText(thWnd, temp, 5);
					snapshotdata.ysize = atoi(temp);

					if (snapshotdata.ysize < 1) snapshotdata.ysize = 1;

					if (snapshotdata.ysize > 480) snapshotdata.ysize = 480;

					if (IsChecked(hWnd, IDC_MEMORYCACHE))	snapshotdata.flag |= 1;
					else snapshotdata.flag &= ~1;

					thWnd = GetDlgItem(hWnd, IDC_EDITFILENAMES);
					GetWindowText(thWnd, temp, 128);
					strcpy(snapshotdata.filenames, temp);

					thWnd = GetDlgItem(hWnd, IDSTARTSNAPSHOT);

					if (SnapShotMode)
					{
						FlushMemorySnaps(1);
						SnapShotMode = 0;
						SetWindowText(thWnd, "Start Snapshot");
					}
					else
					{
						long nb = InitMemorySnaps();
						SnapShotMode = 1;
						char temp[64];
						sprintf(temp, "%d Stop", nb);
						SetWindowText(thWnd, temp);
					}

					CURRENTSNAPNUM = 0;


					break;
			}

			break;
		case WM_INITDIALOG:

			thWnd = GetDlgItem(hWnd, IDSTARTSNAPSHOT);

			if (SnapShotMode) SetWindowText(thWnd, "Stop Snapshot");
			else SetWindowText(thWnd, "Start Snapshot");

			thWnd = GetDlgItem(hWnd, IDC_16BITS);

			if (snapshotdata.bits == 16) SetWindowText(thWnd, "16 bits");
			else SetWindowText(thWnd, "24 bits");

			thWnd = GetDlgItem(hWnd, IDC_EDITFILENAMES);
			SetWindowText(thWnd, snapshotdata.filenames);
			thWnd = GetDlgItem(hWnd, IDC_SETPATH);
			SetWindowText(thWnd, snapshotdata.path);

			thWnd = GetDlgItem(hWnd, IDC_XSIZE);
			sprintf(temp, "%d", snapshotdata.xsize);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_YSIZE);
			sprintf(temp, "%d", snapshotdata.ysize);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_IMAGESSEC);
			sprintf(temp, "%d", snapshotdata.imgsec);
			SetWindowText(thWnd, temp);

			if (snapshotdata.flag & 1) SetCheck(hWnd, IDC_MEMORYCACHE, CHECK);

			return TRUE;
			break;
		case WM_CLOSE:
			SnapShotDlg = NULL;
			EndDialog(hWnd, TRUE);
			break;
	}

	return FALSE;

}

//*************************************************************************************

long THREAD_MINX = 0;
long THREAD_MINZ = 0;
long THREAD_MAXX = 0;
long THREAD_MAXZ = 0;

long PAUSED_PRECALC = 0;
HWND PRECALC = NULL;
long LIGHT_THREAD_STATUS = 0; // 0=not created EXITED_LIGHT_THREAD=0;
// 1=working
// 2=finished exited
// 3=immediate exit !
LPTHREAD_START_ROUTINE EERIE_LIGHT_LightProc(char * ts)
{
	LIGHT_THREAD_STATUS = 1;
	EERIEPrecalcLights(THREAD_MINX, THREAD_MINZ, THREAD_MAXX, THREAD_MAXZ);
	LIGHT_THREAD_STATUS = 2;

	ExitThread(1);
	return 0;
}

//*************************************************************************************
// "Clean" Kill for light thread
//*************************************************************************************
void KillLightThread()
{
	if (LIGHT_THREAD_STATUS == 2)
	{
		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if ((LIGHT_THREAD_STATUS == 1) || (LIGHT_THREAD_STATUS == 3))
	{
		LIGHT_THREAD_STATUS = 3;

		while (LIGHT_THREAD_STATUS != 2)
			Sleep(10);

		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}
}

//*************************************************************************************
//*************************************************************************************

void LaunchLightThread(long minx, long minz, long maxx, long maxz)
{
	char args;
	DWORD id;

	if (LIGHT_THREAD_STATUS == 2)
	{
		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if ((LIGHT_THREAD_STATUS == 1) || (LIGHT_THREAD_STATUS == 3))
	{
		LIGHT_THREAD_STATUS = 3;

		while (LIGHT_THREAD_STATUS != 2)
			Sleep(10);

		CloseHandle(LIGHTTHREAD);
		LIGHTTHREAD = NULL;
		LIGHT_THREAD_STATUS = 0;
	}

	if (PRECALC == NULL)
	{
		if (danaeApp.m_pFramework->m_bIsFullscreen)
		{

			ARX_TIME_Pause();
			DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
			          MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc);
			ARX_TIME_UnPause();
		}

		else
			PRECALC = (CreateDialogParam((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
			                             MAKEINTRESOURCE(IDD_PRECALC), danaeApp.m_hWnd, PrecalcProc, 0));
	}

	THREAD_MINX = minx;
	THREAD_MINZ = minz;
	THREAD_MAXX = maxx;
	THREAD_MAXZ = maxz;

	LIGHTTHREAD = (HANDLE)CreateThread(
	                  NULL,												//pointer to security attributes
	                  0,												// initial thread stack size
	                  (LPTHREAD_START_ROUTINE) EERIE_LIGHT_LightProc,	// pointer to thread function
	                  (LPVOID)&args,									// argument for new thread
	                  0,                     // creation flags
	                  (LPDWORD)&id										// pointer to receive thread ID
	              );

}

long SYNTAXCHECKING = 0;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK PrecalcProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;

	switch (uMsg)
	{
		case WM_MOVE:

			if (danaeApp.m_pDeviceInfo->bWindowed)
			{
				RECT rect1, rect2;
				GetWindowRect(danaeApp.m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_LightPrecalc_POSX", posx);
				Danae_Registry_WriteValue("WND_LightPrecalc_POSY", posy);
			}

			break;
		case WM_TIMER:
			thWnd = GetDlgItem(hWnd, IDC_PROGRESS);
			float t;
			t = (float)PROGRESS_COUNT / (float)PROGRESS_TOTAL * 1000.f;
			SendMessage(thWnd, PBM_SETRANGE , 0, MAKELPARAM(0, 1000));
			SendMessage(thWnd, PBM_SETPOS , (long)t, 0);

			thWnd = GetDlgItem(hWnd, IDC_STATICC);

			if (PAUSED_PRECALC) SetWindowText(thWnd, "Paused");
			else if (LIGHTTHREAD != NULL)
			{
				char tex[32];
				t *= DIV10;
				sprintf(tex, "Working... ( %d%% )", (long)t);
				SetWindowText(thWnd, tex);
			}
			else
			{
				SetWindowText(thWnd, "Idle...");
				PROGRESS_COUNT = PROGRESS_TOTAL;
				SendMessage(thWnd, PBM_SETPOS , 1000, 0);

				if (danaeApp.m_pFramework->m_bIsFullscreen)
				{
					PRECALC = NULL;
					KillTimer(hWnd, 1);
					EndDialog(hWnd, TRUE);
					return FALSE;
				}
			}

			break;
		case WM_COMMAND:

			if (ID_PAUSE == LOWORD(wParam))
			{
				if (PAUSED_PRECALC)
				{
					thWnd = GetDlgItem(hWnd, ID_PAUSE);
					SetWindowText(thWnd, "Pause");
					PAUSED_PRECALC = 0;
				}
				else
				{
					thWnd = GetDlgItem(hWnd, ID_PAUSE);
					SetWindowText(thWnd, "Resume");
					PAUSED_PRECALC = 1;
				}
			}

			if (ID_STOP == LOWORD(wParam))
			{
				if (LIGHTTHREAD != NULL)
				{
					TerminateThread(LIGHTTHREAD, 1);
					LIGHTTHREAD = NULL;
				}
			}

			if (ID_RECALCULATE == LOWORD(wParam))
			{
				LaunchLightThread(0, 0, 999999, 9999999);
			}

			if (ID_AROUND == LOWORD(wParam))
			{
				RecalcLightZone(player.pos.x, player.pos.y, player.pos.z, 2);
			}

			break;
		case WM_CLOSE:
			PRECALC = NULL;
			KillTimer(hWnd, 1);
			EndDialog(hWnd, TRUE);
			return FALSE;
			break;
		case WM_INITDIALOG:

			if (danaeApp.m_pDeviceInfo->bWindowed)
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_LightPrecalc_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_LightPrecalc_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > 0)
				        && (posy < 1000) && (posy > 0)
				   )
				{
					RECT rect1;
					GetWindowRect(danaeApp.m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			PRECALC = hWnd;
			SetTimer(hWnd, 1, 100, NULL);
			thWnd = GetDlgItem(hWnd, IDC_PROGRESS);
			SendMessage(thWnd, PBM_SETRANGE , 0, MAKELPARAM(0, 1000));
			SendMessage(thWnd, PBM_SETPOS , 0, 0);
			SendMessage(thWnd, PBM_SETBKCOLOR , 0, 0);
			SendMessage(thWnd, PBM_SETBARCOLOR , 0, 0xFF0000FF);
			thWnd = GetDlgItem(hWnd, IDC_STATICC);
			SetWindowText(thWnd, "Idle");
			return TRUE;
			break;
	}

	return FALSE;
}

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK GaiaTextEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	HWND thWnd;

	if (WM_COMMAND == uMsg)
	{
		if (IDOK == LOWORD(wParam))
		{
			thWnd = GetDlgItem(hWnd, IDC_TEXTEDIT);
			GetWindowText(thWnd, GTE_TEXT, GTE_SIZE - 1);
			EndDialog(hWnd, TRUE);
		}

		if (IDCANCEL == LOWORD(wParam))
			EndDialog(hWnd, TRUE);
	}

	if (uMsg == WM_INITDIALOG)
	{
		SetWindowText(hWnd, GTE_TITLE);
		thWnd = GetDlgItem(hWnd, IDC_TEXTEDIT);
		SetWindowText(thWnd, GTE_TEXT);
		return TRUE;
	}

	return FALSE;
}

void ExitProc();

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK StartProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	long val;
	HWND thWnd;

	switch (uMsg)
	{
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_LAUNCHDEMO:
					LaunchDemo = 1;

				case IDOK:

					if (IsChecked(hWnd, IDC_FASTLOADING)) FASTLOADS = 1;
					else FASTLOADS = 0;

					if (IsChecked(hWnd, IDC_NODIRCREATION)) NODIRCREATION = 1;
					else NODIRCREATION = 0;

					if (IsChecked(hWnd, IDC_SOUND))
					{
						Project.soundmode &= ~ARX_SOUND_ON;
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), FALSE);
						SetCheck(hWnd, IDC_REVERB, UNCHECK);
					}
					else
					{
						Project.soundmode |= ARX_SOUND_ON;
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), TRUE);

						if (IsChecked(hWnd, IDC_REVERB)) Project.soundmode |= ARX_SOUND_REVERB;
						else Project.soundmode &= ~ARX_SOUND_REVERB;
					}

					Project.bits = 32;

					if (IsChecked(hWnd, IDC_SYNTAXCHECKING)) SYNTAXCHECKING = 1;
					else SYNTAXCHECKING = 0;

					if (IsChecked(hWnd, IDC_NOCHECKSUM)) NOCHECKSUM = 1;
					else NOCHECKSUM = 0;

					if (IsChecked(hWnd, IDC_LOADDEMO))	Project.demo = LEVELDEMO;

					if (IsChecked(hWnd, IDC_LOADDEMO2))	Project.demo = LEVELDEMO2;

					if (IsChecked(hWnd, IDC_LOADDEMO3))	Project.demo = LEVELDEMO3;

					if (IsChecked(hWnd, IDC_LOADDEMO4))	Project.demo = LEVELDEMO4;

					if (IsChecked(hWnd, IDC_TLEVEL0))	Project.demo = LEVEL0;

					if (IsChecked(hWnd, IDC_TLEVEL1))	Project.demo = LEVEL1;

					if (IsChecked(hWnd, IDC_TLEVEL2))	Project.demo = LEVEL2;

					if (IsChecked(hWnd, IDC_TLEVEL3))	Project.demo = LEVEL3;

					if (IsChecked(hWnd, IDC_TLEVEL4))	Project.demo = LEVEL4;

					if (IsChecked(hWnd, IDC_TLEVEL5))	Project.demo = LEVEL5;

					if (IsChecked(hWnd, IDC_TLEVEL6))	Project.demo = LEVEL6;

					if (IsChecked(hWnd, IDC_TLEVEL7))	Project.demo = LEVEL7;

					if (IsChecked(hWnd, IDC_TLEVEL8))	Project.demo = LEVEL8;

					if (IsChecked(hWnd, IDC_TLEVEL9))	Project.demo = LEVEL9;

					if (IsChecked(hWnd, IDC_TLEVEL10))	Project.demo = LEVEL10;

					if (IsChecked(hWnd, IDC_TLEVEL11))	Project.demo = LEVEL11;

					if (IsChecked(hWnd, IDC_TLEVEL12))	Project.demo = LEVEL12;

					if (IsChecked(hWnd, IDC_TLEVEL13))	Project.demo = LEVEL13;

					if (IsChecked(hWnd, IDC_TLEVEL14))	Project.demo = LEVEL14;

					if (IsChecked(hWnd, IDC_TLEVEL15))	Project.demo = LEVEL15;

					if (IsChecked(hWnd, IDC_TLEVEL16))	Project.demo = LEVEL16;

					if (IsChecked(hWnd, IDC_TLEVEL17))	Project.demo = LEVEL17;

					if (IsChecked(hWnd, IDC_TLEVEL18))	Project.demo = LEVEL18;

					if (IsChecked(hWnd, IDC_TLEVEL19))	Project.demo = LEVEL19;

					if (IsChecked(hWnd, IDC_TLEVEL20))	Project.demo = LEVEL20;

					if (IsChecked(hWnd, IDC_TLEVEL21))	Project.demo = LEVEL21;

					if (IsChecked(hWnd, IDC_TLEVEL22))	Project.demo = LEVEL22;

					if (IsChecked(hWnd, IDC_TLEVEL23))	Project.demo = LEVEL23;

					if (IsChecked(hWnd, IDC_TLEVEL24))	Project.demo = LEVEL24;

					if (IsChecked(hWnd, IDC_TRACEMEMORY)) HERMES_KEEP_MEMORY_TRACE = 1;
					else HERMES_KEEP_MEMORY_TRACE = 0;

					if (IsChecked(hWnd, IDC_NEED_ANCHOR)) NEED_ANCHORS = 1;
					else NEED_ANCHORS = 0;


					if (IsChecked(hWnd, IDC_MULTIPLAYER)) Project.multiplayer = 1;
					else Project.multiplayer = 0;

					if (IsChecked(hWnd, IDC_COMPATIBILITY)) Project.compatibility = 1;
					else Project.compatibility = 0;

					if (IsChecked(hWnd, IDC_OTHERSERVER))
					{
						thWnd = GetDlgItem(hWnd, IDC_OTHERSERVER);
						GetWindowText(thWnd, Project.workingdir, 256);
					}
					else strcpy(Project.workingdir, "\\\\ARKANESERVER\\Public\\Arx\\");

					char tteexx[512];
					strcpy(tteexx, Project.workingdir);
					strcat(tteexx, "GRAPH\\LEVELS\\");
					_chdir(tteexx);

					thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
					val = SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					switch (val)
					{
						case 1:
							Project.TextureSize = 32;
							break;
						case 2:
							Project.TextureSize = 64;
							break;
						case 3:
							Project.TextureSize = 96;
							break;
						case 4:
							Project.TextureSize = 128;
							break;
						case 5:
							Project.TextureSize = 192;
							break;
						case 6:
							Project.TextureSize = 256;
							break;
						case 7:
							Project.TextureSize = 2;
							break;
						default:
							Project.TextureSize = 0;
					}

					if (IsChecked(hWnd, IDC_TEX16)) Project.TextureBits = 16;
					else Project.TextureBits = 32;

					Danae_Registry_Write("LastWorkingDir", Project.workingdir);
					EndDialog(hWnd, TRUE);
					break;
				case IDQUIT:
					EndDialog(hWnd, TRUE);
					ExitProc();
					break;
				case IDC_CHOOSEDIR:
					thWnd = GetDlgItem(hWnd, IDC_OTHERSERVER);
					SendMessage(thWnd, BM_CLICK, 0, 0);
					HERMESFolderSelector(Project.workingdir, "Choose Working Folder");

					if (!strcmp(Project.workingdir, "A:\\\\")) strcpy(Project.workingdir, "A:\\");

					if (!strcmp(Project.workingdir, "B:\\\\")) strcpy(Project.workingdir, "B:\\");

					if (!strcmp(Project.workingdir, "C:\\\\")) strcpy(Project.workingdir, "C:\\");

					if (!strcmp(Project.workingdir, "D:\\\\")) strcpy(Project.workingdir, "D:\\");

					if (!strcmp(Project.workingdir, "E:\\\\")) strcpy(Project.workingdir, "E:\\");

					if (!strcmp(Project.workingdir, "F:\\\\")) strcpy(Project.workingdir, "F:\\");

					if (!strcmp(Project.workingdir, "G:\\\\")) strcpy(Project.workingdir, "G:\\");

					if (!strcmp(Project.workingdir, "H:\\\\")) strcpy(Project.workingdir, "H:\\");

					if (!strcmp(Project.workingdir, "I:\\\\")) strcpy(Project.workingdir, "I:\\");

					if (!strcmp(Project.workingdir, "J:\\\\")) strcpy(Project.workingdir, "J:\\");

					if (!strcmp(Project.workingdir, "K:\\\\")) strcpy(Project.workingdir, "K:\\");

					if (!strcmp(Project.workingdir, "L:\\\\")) strcpy(Project.workingdir, "L:\\");

					if (!strcmp(Project.workingdir, "M:\\\\")) strcpy(Project.workingdir, "M:\\");

					if (!strcmp(Project.workingdir, "N:\\\\")) strcpy(Project.workingdir, "N:\\");

					if (!strcmp(Project.workingdir, "O:\\\\")) strcpy(Project.workingdir, "O:\\");

					if (!strcmp(Project.workingdir, "P:\\\\")) strcpy(Project.workingdir, "P:\\");

					if (!strcmp(Project.workingdir, "Q:\\\\")) strcpy(Project.workingdir, "Q:\\");

					if (!strcmp(Project.workingdir, "R:\\\\")) strcpy(Project.workingdir, "R:\\");

					if (!strcmp(Project.workingdir, "S:\\\\")) strcpy(Project.workingdir, "S:\\");

					if (!strcmp(Project.workingdir, "T:\\\\")) strcpy(Project.workingdir, "T:\\");

					if (!strcmp(Project.workingdir, "U:\\\\")) strcpy(Project.workingdir, "U:\\");

					if (!strcmp(Project.workingdir, "V:\\\\")) strcpy(Project.workingdir, "V:\\");

					if (!strcmp(Project.workingdir, "W:\\\\")) strcpy(Project.workingdir, "W:\\");

					if (!strcmp(Project.workingdir, "X:\\\\")) strcpy(Project.workingdir, "X:\\");

					if (!strcmp(Project.workingdir, "Y:\\\\")) strcpy(Project.workingdir, "Y:\\");

					if (!strcmp(Project.workingdir, "Z:\\\\")) strcpy(Project.workingdir, "Z:\\");

					SetWindowText(thWnd, Project.workingdir);
					Danae_Registry_Write("LastWorkingDir", Project.workingdir);
					break;
			}

			break;
		case WM_NOTIFY:
			long val;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
			val = SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATICC);

			switch (val)
			{
				case 1:
					sprintf(temp, "Texture Size: 32x32");
					break;
				case 2:
					sprintf(temp, "Texture Size: 64x64");
					break;
				case 3:
					sprintf(temp, "Texture Size: 96x96");
					break;
				case 4:
					sprintf(temp, "Texture Size: 128x128");
					break;
				case 5:
					sprintf(temp, "Texture Size: 192x192");
					break;
				case 6:
					sprintf(temp, "Texture Size: 256x256");
					break;
				case 7:
					sprintf(temp, "Texture Size: DIV2");
					break;
				default:
					sprintf(temp, "Texture Size: ANY");
			}

			SetWindowText(thWnd, temp);
			break;
		case WM_INITDIALOG:
			HWND thWnd;
			char tex[128];

			thWnd = GetDlgItem(hWnd, IDC_NODIRCREATION);

			if (NODIRCREATION) SendMessage(thWnd, BM_CLICK, 0, 0);

			thWnd = GetDlgItem(hWnd, IDC_VERSION);
			sprintf(tex, "Ver.%2.3f", DANAE_VERSION);
			SetWindowText(thWnd, tex);
			Danae_Registry_Read("LastWorkingDir", Project.workingdir, "c:\\arx\\", 256);
			thWnd = GetDlgItem(hWnd, IDC_OTHERSERVER);
			SetWindowText(thWnd, Project.workingdir);

			if (HERMES_KEEP_MEMORY_TRACE) SetClick(hWnd, IDC_TRACEMEMORY);

			SetClick(hWnd, IDC_OTHERSERVER);

			if (!(Project.soundmode & ARX_SOUND_ON)) SetClick(hWnd, IDC_SOUND);

			SetClick(hWnd, IDC_LOADDEMO);

			if (NOCHECKSUM) SetClick(hWnd, IDC_NOCHECKSUM);

			if (SYNTAXCHECKING)	SetClick(hWnd, IDC_SYNTAXCHECKING);

			SetClick(hWnd, IDC_TEX16);
			thWnd = GetDlgItem(hWnd, IDC_TEXTUREPRECISION);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 8));
			long t = 8;
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			if (NEED_ANCHORS) SetClick(hWnd, IDC_NEED_ANCHOR);

			return TRUE;
			break;
	}

	return FALSE;
}
char SCRIPT_SEARCH_TEXT[256];
BOOL CALLBACK ScriptSearchProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	if (WM_COMMAND == uMsg)
	{
		switch (LOWORD(wParam))
		{
			case IDOK:
				HWND thWnd;
				thWnd = GetDlgItem(hWnd, IDC_SEARCHEDIT);
				GetWindowText(thWnd, SCRIPT_SEARCH_TEXT, 255);
				EndDialog(hWnd, TRUE);
				break;
			case IDCANCEL:
				EndDialog(hWnd, TRUE);
				break;
		}
	}

	if (uMsg == WM_INITDIALOG)
	{
		SCRIPT_SEARCH_TEXT[0] = 0;
		HWND thWnd;
		thWnd = GetDlgItem(hWnd, IDC_SEARCHEDIT);
		SetFocus(thWnd); 
		return TRUE;
	}

	return FALSE;
}

//*************************************************************************************
// AboutProc()
//  message proc function for the about box
//*************************************************************************************
BOOL CALLBACK AboutProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	if (WM_COMMAND == uMsg)
		if (IDOK == LOWORD(wParam) || IDCANCEL == LOWORD(wParam))
			EndDialog(hWnd, TRUE);

	if (uMsg == WM_INITDIALOG)
	{
		HWND thWnd;
		char tex[128];

		thWnd = GetDlgItem(hWnd, IDC_ABOUT_VERSION);
		sprintf(tex, "Ver.%2.3f", DANAE_VERSION);
		SetWindowText(thWnd, tex);
		return TRUE;
	}

	return FALSE;
}
extern long DEBUGSYS;
extern long DBGSETTEXTURE;
extern long USEINTERNORM;
long oml;
extern float BIGLIGHTPOWER;
extern long DEBUGCODE;
extern long TRUEFIGHT;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK OptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;
	static long wuz;

	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			ARX_SOUND_MixerPause(ARX_SOUND_MixerGame);

			oml = ModeLight;
			char tex[64];
			thWnd = GetDlgItem(hWnd, IDC_EDITNBINTERPOLATIONS);
			sprintf(tex, "%d", MOLLESS_Nb_Interpolations);
			SetWindowText(thWnd, tex);

			if (ARX_DEMO)					SetClick(hWnd, IDC_ARXDEMO);

			if (SYNTAXCHECKING)				SetClick(hWnd, IDC_SYNTAXCHECK);

			if (ZMAPMODE)					SetClick(hWnd, IDC_ZMAPMODE);

			if (DEBUGCODE)					SetClick(hWnd, IDC_DEBUGCODE);

			if (HIPOLY)						SetClick(hWnd, IDC_HPO);

			if (Project.interpolatemouse)	SetClick(hWnd, IDC_INTERPOLATEMOUSE);

			if (Project.vsync)				SetClick(hWnd, IDC_VSYNC);

			if (TRUECLIPPING)				SetClick(hWnd, IDC_TRUECLIPPING);

			if (SHOWSHADOWS)				SetClick(hWnd, IDC_SHOWSHADOWS);

			if (A_FLARES)					SetClick(hWnd, IDC_FLARES);

			if (LIGHTPOWERUP)				SetClick(hWnd, IDC_LIGHTPOWERUP);

			if (INVERTMOUSE)				SetClick(hWnd, IDC_INVERTMOUSE);

			if (Project.bits == 16)			SetClick(hWnd, IDC_FULLRENDER16BITS);

			if (Project.bits == 32)			SetClick(hWnd, IDC_FULLRENDER32BITS);

			if (DEBUG1ST)					SetClick(hWnd, IDC_DEBUG1ST);

			if (DEBUGSYS)					SetClick(hWnd, IDC_DEBUGSYS);

			if (DEBUGNPCMOVE)				SetClick(hWnd, IDC_DEBUGNPCMOVE);

			if (DEBUG_MOLLESS)				SetClick(hWnd, IDC_DEBUGMOLLESS);

			if (DYNAMIC_NORMALS)			SetClick(hWnd, IDC_DYNAMICNORMALS);

			if (DBGSETTEXTURE)				SetClick(hWnd, IDC_SETTEXTURE);

			if (ViewMode & VIEWMODE_WIRE)	SetClick(hWnd, IDC_WIREFRAME);

			if (TRUEFIGHT)					SetClick(hWnd, IDC_TRUEFIGHT);

			if (USE_D3DFOG)					SetClick(hWnd, IDC_USED3DFOG);

			if (ModeLight & MODE_STATICLIGHT)
			{
				SetClick(hWnd, IDC_SHOWLIGHTSNSHADOWS);
				wuz = 1;
			}
			else wuz = 0;


			if (ModeLight & MODE_NORMALS)	SetClick(hWnd, IDC_ILLUMNORMAL);

			if (ModeLight & MODE_RAYLAUNCH)	SetClick(hWnd, IDC_ILLUMRAYLAUNCH);

			if (ModeLight & MODE_SMOOTH)	SetClick(hWnd, IDC_ILLUMSMOOTH);

			if (ModeLight & MODE_DYNAMICLIGHT)	SetClick(hWnd, IDC_TORCHHALO);

			if (USE_COLLISIONS)				SetClick(hWnd, IDC_COLLISIONS);

			if (USE_PLAYERCOLLISIONS)		SetClick(hWnd, IDC_PLAYERCOLLISIONS);

			if (SHOW_TORCH)					SetClick(hWnd, IDC_TORCHHALO2);

			if (ViewMode & VIEWMODE_NORMALS)	SetClick(hWnd, IDC_SHOWNORMALS);

			if (ModeLight & MODE_DEPTHCUEING)	SetClick(hWnd, IDC_SHOWDEPTH);

			if (ViewMode & VIEWMODE_INFOTEXT)	SetClick(hWnd, IDC_INFOTEXT);

			if (ViewMode & VIEWMODE_FLAT)		SetClick(hWnd, IDC_NOTEXTURES);

			if (Cross)						SetClick(hWnd, IDC_RAY);

			if (USEINTERNORM)				SetClick(hWnd, IDC_INTERNORM);

			if (EXTERNALVIEWING)			SetClick(hWnd, IDC_THIRDPERSON);

			if (DebugLvl[0])				SetClick(hWnd, IDC_LEVELNONE);

			if (DebugLvl[1])				SetClick(hWnd, IDC_LEVEL1);

			if (DebugLvl[2])				SetClick(hWnd, IDC_LEVEL2);

			if (DebugLvl[3])				SetClick(hWnd, IDC_LEVEL3);

			if (DebugLvl[4])				SetClick(hWnd, IDC_LEVEL4);

			if (DebugLvl[5])				SetClick(hWnd, IDC_LEVEL5);

			if (Bilinear == 0)				SetClick(hWnd, IDC_FILTERPOINT);
			else if (Bilinear == 1)			SetClick(hWnd, IDC_FILTERLINEAR);
			else if (Bilinear == 2)			SetClick(hWnd, IDC_FILTERANISOTROPIC);

			if (MAPUPDATE)					SetClick(hWnd, IDC_MAPUPDATE);

			if (BLURTEXTURES) SetClick(hWnd, IDC_BLURTEXTURES);

			if (NOMIPMAPS)    SetClick(hWnd, IDC_NOMIPMAPS);

			if (POINTINTERPOLATION) SetClick(hWnd, IDC_POINTINTERPOLATION);

			if (ALLOW_MESH_TWEAKING) SetClick(hWnd, IDC_MESHTWEAK);

			thWnd = GetDlgItem(hWnd, IDC_SLIDERDEPTH);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1000, 8000));
			long t = (long)subj.cdepth;
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_POWERSLIDER);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 10));
			t = (long)LPpower;
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_POWERSLIDER2);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 50));
			t = (long)(BIGLIGHTPOWER * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 200));
			t = (long)(TIMEFACTOR * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));


			thWnd = GetDlgItem(hWnd, IDC_D3DTRANSFORM);

			if (D3DTRANSFORM) SendMessage(thWnd, BM_CLICK, 0, 0);

			if (!ARX_SOUND_IsEnabled())
			{
				SetCheck(hWnd, IDC_DISABLESOUND, CHECK);
				EnableWindow(GetDlgItem(hWnd, IDC_REVERB), FALSE);
			}
			else if (ARX_SOUND_IsReverbEnabled()) SetCheck(hWnd, IDC_REVERB, CHECK);

			return TRUE;
		}

		case WM_DESTROY :
			ARX_SOUND_MixerResume(ARX_SOUND_MixerGame);
			break;

		case WM_COMMAND :
		{
			switch (LOWORD(wParam))
			{
				case IDC_TIMEFACTOR :
				{
					TIMEFACTOR = 1.f;
					long t = (long)(TIMEFACTOR * 100.f);
					thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
					SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));
					break;
				}

				case IDC_BKGCOLOR :
				{
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = hWnd;
					cc.hInstance = 0; //Ignored
					cc.rgbResult = ((subj.bkgcolor >> 16 & 255))
					               | ((subj.bkgcolor >> 8 & 255) << 8)
					               | ((subj.bkgcolor & 255) << 16);
					cc.lpCustColors = custcr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					ChooseColor(&cc);
					subj.bkgcolor = ((cc.rgbResult >> 16 & 255))
					                | ((cc.rgbResult >> 8 & 255) << 8)
					                | ((cc.rgbResult & 255) << 16);
					break;
				}

				case IDC_DISABLESOUND :

					if (IsChecked(hWnd, IDC_DISABLESOUND))
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), FALSE);
					else
						EnableWindow(GetDlgItem(hWnd, IDC_REVERB), TRUE);

					break;

				case IDC_DEPTHDEFAULT :
					SendMessage(GetDlgItem(hWnd, IDC_SLIDERDEPTH), TBM_SETPOS, TRUE, (LPARAM)(2800));
					break;

				case IDOK :
				{
					if (IsChecked(hWnd, IDC_DISABLESOUND)) ARX_SOUND_Release();
					else
					{
						if (!ARX_SOUND_IsEnabled()) ARX_SOUND_Init(danaeApp.m_hWnd);

						ARX_SOUND_EnableReverb(IsChecked(hWnd, IDC_REVERB) ? 1 : 0);
					}

					long restoretex = 0;

					if (IsChecked(hWnd, IDC_BLURTEXTURES))
					{
						if (BLURTEXTURES != 1)
						{
							BLURTEXTURES = 1;
							restoretex = 1;
						}
					}
					else if (BLURTEXTURES != 0)
					{
						BLURTEXTURES = 0;
						restoretex = 1;
					}

					if (IsChecked(hWnd, IDC_NOMIPMAPS))
					{
						if (NOMIPMAPS != 1)
						{
							NOMIPMAPS = 1;
							restoretex = 1;
						}
					}
					else if (NOMIPMAPS != 0)
					{
						NOMIPMAPS = 0;
						restoretex = 1;
					}

					if (restoretex) D3DTextr_RestoreAllTextures(GDevice);

					if (IsChecked(hWnd, IDC_POINTINTERPOLATION)) POINTINTERPOLATION = 1;
					else POINTINTERPOLATION = 0;

					if (IsChecked(hWnd, IDC_INTERPOLATEMOUSE)) Project.interpolatemouse = 1;
					else Project.interpolatemouse = 0;

					if (IsChecked(hWnd, IDC_USED3DFOG)) USE_D3DFOG = 1;
					else USE_D3DFOG = 0;

					if (IsChecked(hWnd, IDC_TRUEFIGHT)) TRUEFIGHT = 1;
					else TRUEFIGHT = 0;

					if (IsChecked(hWnd, IDC_DEBUGCODE)) DEBUGCODE = 1;
					else DEBUGCODE = 0;

					if (IsChecked(hWnd, IDC_HPO)) HIPOLY = 1;
					else HIPOLY = 0;

					if (IsChecked(hWnd, IDC_ARXDEMO)) ARX_DEMO = 1;
					else ARX_DEMO = 0;

					if (IsChecked(hWnd, IDC_SYNTAXCHECK)) SYNTAXCHECKING = 1;
					else SYNTAXCHECKING = 0;

					if (IsChecked(hWnd, IDC_VSYNC)) Project.vsync = 1;
					else Project.vsync = 0;

					if (IsChecked(hWnd, IDC_ZMAPMODE)) ZMAPMODE = 1;
					else ZMAPMODE = 0;

					if (IsChecked(hWnd, IDC_LIGHTPOWERUP)) LIGHTPOWERUP = 1;
					else LIGHTPOWERUP = 0;

					if (IsChecked(hWnd, IDC_SHOWSHADOWS)) SHOWSHADOWS = 1;
					else SHOWSHADOWS = 0;

					if (IsChecked(hWnd, IDC_TRUECLIPPING)) TRUECLIPPING = 1;
					else TRUECLIPPING = 0;

					if (IsChecked(hWnd, IDC_FULLRENDER16BITS)) Project.bits = 16;
					else Project.bits = 32;

					danaeApp.m_pFramework->bitdepth = Project.bits;

					if (IsChecked(hWnd, IDC_DEBUG1ST)) DEBUG1ST = 1;
					else DEBUG1ST = 0;

					if (IsChecked(hWnd, IDC_DEBUGSYS)) DEBUGSYS = 1;
					else DEBUGSYS = 0;

					if (IsChecked(hWnd, IDC_SETTEXTURE)) DBGSETTEXTURE = 1;
					else DBGSETTEXTURE = 0;

					if (IsChecked(hWnd, IDC_DEBUGNPCMOVE)) DEBUGNPCMOVE = 1;
					else DEBUGNPCMOVE = 0;

					if (IsChecked(hWnd, IDC_DEBUGMOLLESS)) DEBUG_MOLLESS = 1;
					else DEBUG_MOLLESS = 0;

					if (IsChecked(hWnd, IDC_DYNAMICNORMALS)) DYNAMIC_NORMALS = 1;
					else DYNAMIC_NORMALS = 0;

					if (IsChecked(hWnd, IDC_WIREFRAME)) ViewMode |= VIEWMODE_WIRE;
					else ViewMode &= ~VIEWMODE_WIRE;

					if (IsChecked(hWnd, IDC_ILLUMNORMAL))
					{
						if (!(oml & MODE_NORMALS)) wuz = 0;

						ModeLight |= MODE_NORMALS;
					}
					else
					{
						if ((oml & MODE_NORMALS)) wuz = 0;

						ModeLight &= ~MODE_NORMALS;
					}

					if (IsChecked(hWnd, IDC_ILLUMRAYLAUNCH))
					{
						if (!(oml & MODE_RAYLAUNCH)) wuz = 0;

						ModeLight |= MODE_RAYLAUNCH;
					}
					else
					{
						if ((oml & MODE_RAYLAUNCH)) wuz = 0;

						ModeLight &= ~MODE_RAYLAUNCH;
					}

					if (IsChecked(hWnd, IDC_ILLUMSMOOTH))
					{
						if (!(oml & MODE_SMOOTH)) wuz = 0;

						ModeLight |= MODE_SMOOTH;
					}
					else
					{
						if ((oml & MODE_SMOOTH)) wuz = 0;

						ModeLight &= ~MODE_SMOOTH;
					}



					if (IsChecked(hWnd, IDC_SHOWLIGHTSNSHADOWS) && !wuz)
					{
						ModeLight |= MODE_STATICLIGHT;
						EERIERemovePrecalcLights();
						LaunchLightThread(0, 0, 999999, 999999);
					}
					else if (!IsChecked(hWnd, IDC_SHOWLIGHTSNSHADOWS) && wuz)
					{
						ModeLight &= ~MODE_STATICLIGHT;
						EERIERemovePrecalcLights();
					}

					if (IsChecked(hWnd, IDC_TORCHHALO)) ModeLight |= MODE_DYNAMICLIGHT;
					else ModeLight &= ~MODE_DYNAMICLIGHT;

					if (IsChecked(hWnd, IDC_TORCHHALO2))
					{
						SHOW_TORCH = 1;
						ARX_SOUND_Stop(SND_TORCH_LOOP);
						ARX_SOUND_PlaySFX(SND_TORCH_LOOP, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
					}
					else
					{
						ARX_SOUND_Stop(SND_TORCH_LOOP);
						SHOW_TORCH = 0;
					}

					if (IsChecked(hWnd, IDC_COLLISIONS)) USE_COLLISIONS = 1;
					else USE_COLLISIONS = 0;

					if (IsChecked(hWnd, IDC_FLARES)) A_FLARES = 1;
					else A_FLARES = 0;

					if (IsChecked(hWnd, IDC_INVERTMOUSE)) INVERTMOUSE = 1;
					else INVERTMOUSE = 0;

					if (IsChecked(hWnd, IDC_INTERNORM)) USEINTERNORM = 1;
					else USEINTERNORM = 0;

					if (IsChecked(hWnd, IDC_THIRDPERSON)) EXTERNALVIEWING = 1;
					else EXTERNALVIEWING = 0;

					if (IsChecked(hWnd, IDC_PLAYERCOLLISIONS))
					{
						USE_PLAYERCOLLISIONS = 1;
						EERIEPOLY * ep = BCCheckInPoly(player.pos.x, player.pos.y, player.pos.z);

						if (ep != NULL) player.pos.y = ep->max.y + PLAYER_BASE_HEIGHT;
					}
					else USE_PLAYERCOLLISIONS = 0;

					if (IsChecked(hWnd, IDC_SHOWNORMALS)) ViewMode |= VIEWMODE_NORMALS;
					else ViewMode &= ~VIEWMODE_NORMALS;

					if (IsChecked(hWnd, IDC_SHOWDEPTH)) ModeLight |= MODE_DEPTHCUEING;
					else ModeLight &= ~MODE_DEPTHCUEING;

					if (IsChecked(hWnd, IDC_INFOTEXT)) ViewMode |= VIEWMODE_INFOTEXT;
					else ViewMode &= ~VIEWMODE_INFOTEXT;

					if (IsChecked(hWnd, IDC_NOTEXTURES)) ViewMode |= VIEWMODE_FLAT;
					else ViewMode &= ~VIEWMODE_FLAT;

					if (IsChecked(hWnd, IDC_RAY)) Cross = 1;
					else Cross = 0;

					if (IsChecked(hWnd, IDC_LEVELNONE))
					{
						DebugLvl[0] = 1;
						DEBUGG = 0;
					}
					else
					{
						DebugLvl[0] = 0;
						DEBUGG = 1;
					}

					if (IsChecked(hWnd, IDC_LEVEL1))  DebugLvl[1] = 1;
					else DebugLvl[1] = 0;

					if (IsChecked(hWnd, IDC_LEVEL2))  DebugLvl[2] = 1;
					else DebugLvl[2] = 0;

					if (IsChecked(hWnd, IDC_LEVEL3))  DebugLvl[3] = 1;
					else DebugLvl[3] = 0;

					if (IsChecked(hWnd, IDC_LEVEL4))  DebugLvl[4] = 1;
					else DebugLvl[4] = 0;

					if (IsChecked(hWnd, IDC_LEVEL5))  DebugLvl[5] = 1;
					else DebugLvl[5] = 0;

					if (IsChecked(hWnd, IDC_MESHTWEAK))  ALLOW_MESH_TWEAKING = 1;
					else ALLOW_MESH_TWEAKING = 0;


					if (IsChecked(hWnd, IDC_D3DTRANSFORM)) D3DTRANSFORM = 1;
					else D3DTRANSFORM = 0;

					if (IsChecked(hWnd, IDC_FILTERANISOTROPIC))	Bilinear = 2;

					if (IsChecked(hWnd, IDC_FILTERLINEAR))		Bilinear = 1;

					if (IsChecked(hWnd, IDC_FILTERPOINT))		Bilinear = 0;

					if (IsChecked(hWnd, IDC_MAPUPDATE)) MAPUPDATE = 1;
					else MAPUPDATE = 0;

					thWnd = GetDlgItem(hWnd, IDC_SLIDERDEPTH);
					long t = SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
					EERIE_CAMERA * oldcam = ACTIVECAM;
					SetActiveCamera(&subj);
					SetCameraDepth((float)t);
					SetActiveCamera(oldcam);

					thWnd = GetDlgItem(hWnd, IDC_POWERSLIDER);
					LPpower = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_POWERSLIDER2);
					BIGLIGHTPOWER = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

					thWnd = GetDlgItem(hWnd, IDC_TIMESLIDER);
					TIMEFACTOR = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

					thWnd = GetDlgItem(hWnd, IDC_EDITNBINTERPOLATIONS);
					char tex[64];
					GetWindowText(thWnd, tex, 63);
					MOLLESS_Nb_Interpolations = atoi(tex);

					EndDialog(hWnd, TRUE);
					break;
				}

				case IDCANCEL :
					EndDialog(hWnd, TRUE);
					break;
			}

			break;
		}
	}

	return FALSE;
}

long WATERFX = 0;
long REFLECTFX = 0;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK OptionsProc_2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HRESULT hr;

	if (WM_COMMAND == uMsg)
	{

		if (IDC_MODE  == LOWORD(wParam))
		{
			if (!danaeApp.m_pDeviceInfo->bWindowed)
			{
				if (SUCCEEDED(D3DEnum_UserChangeDevice(&danaeApp.m_pDeviceInfo)))
				{
					ARX_Text_Close();

					if (FAILED(hr = danaeApp.Change3DEnvironment()))
					{
						ShowPopup("Error Changing Environment");
						return 0;
					}

					GDevice = danaeApp.m_pd3dDevice;
					ARX_Text_Init();
				}
				else ShowPopup("Error Changing Device");
			}
		}

		if (IDC_INTERCOLOR  == LOWORD(wParam))
		{
			CHOOSECOLOR cc;
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = danaeApp.m_hWnd;
			cc.hInstance = 0; //Ignored
			cc.rgbResult = (((long)(float)(Project.interfacergb.r * 255.f) & 255))
			               | (((long)(float)(Project.interfacergb.g * 255.f) & 255) << 8)
			               | (((long)(float)(Project.interfacergb.b * 255.f) & 255)) << 16;
			cc.lpCustColors = custcr;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				Project.interfacergb.b = (float)((cc.rgbResult >> 16 & 255)) * DIV255;
				Project.interfacergb.g = (float)((cc.rgbResult >> 8 & 255)) * DIV255;
				Project.interfacergb.r = (float)((cc.rgbResult & 255)) * DIV255;
			}
		}

		if (IDC_INTERCOLOR2  == LOWORD(wParam))
		{
			CHOOSECOLOR cc;
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = danaeApp.m_hWnd;
			cc.hInstance = 0; //Ignored
			cc.rgbResult = (((long)(float)(Project.torch.r * 255.f) & 255))
			               | (((long)(float)(Project.torch.g * 255.f) & 255) << 8)
			               | (((long)(float)(Project.torch.b * 255.f) & 255)) << 16;
			cc.lpCustColors = custcr;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc))
			{
				Project.torch.b = (float)((cc.rgbResult >> 16 & 255)) * DIV255;
				Project.torch.g = (float)((cc.rgbResult >> 8 & 255)) * DIV255;
				Project.torch.r = (float)((cc.rgbResult & 255)) * DIV255;
			}
		}

		if (IDOK == LOWORD(wParam))
		{
			if (IsChecked(hWnd, IDC_HIDESPEECH))		HIDESPEECH = 1;
			else HIDESPEECH = 0;

			if (IsChecked(hWnd, IDC_WATERFX))		WATERFX = 1;
			else WATERFX = 0;

			if (IsChecked(hWnd, IDC_REFLECTFX))		REFLECTFX = 1;
			else REFLECTFX = 0;

			if (IsChecked(hWnd, IDC_FORCEIO))		ForceIODraw = 1;
			else ForceIODraw = 0;

			if (IsChecked(hWnd, IDC_TREATALLIO))		TreatAllIO = 1;
			else TreatAllIO = 0;


			if (IsChecked(hWnd, IDC_HIDEMAGICDUST))	HIDEMAGICDUST = 1;
			else HIDEMAGICDUST = 0;

			if (IsChecked(hWnd, IDC_HIDEANCHORS))	HIDEANCHORS = 1;
			else HIDEANCHORS = 0;

			if (IsChecked(hWnd, IDC_HIDEBACKGROUND))	Project.hide |= HIDE_BACKGROUND;
			else Project.hide &= ~HIDE_BACKGROUND;

			if (IsChecked(hWnd, IDC_HIDENPC))		Project.hide |= HIDE_NPC;
			else Project.hide &= ~HIDE_NPC;

			if (IsChecked(hWnd, IDC_HIDEFIXINTER))	Project.hide |= HIDE_FIXINTER;
			else Project.hide &= ~HIDE_FIXINTER;

			if (IsChecked(hWnd, IDC_HIDEITEMS))		Project.hide |= HIDE_ITEMS;
			else Project.hide &= ~HIDE_ITEMS;

			if (IsChecked(hWnd, IDC_HIDEPARTICLES))	Project.hide |= HIDE_PARTICLES;
			else Project.hide &= ~HIDE_PARTICLES;

			if (IsChecked(hWnd, IDC_HIDECAMERAS))	Project.hide |= HIDE_CAMERAS;
			else Project.hide &= ~HIDE_CAMERAS;

			if (IsChecked(hWnd, IDC_HIDEINTERFACE))	Project.hide |= HIDE_INTERFACE;
			else Project.hide &= ~HIDE_INTERFACE;

			if (IsChecked(hWnd, IDC_HIDENODES))		Project.hide |= HIDE_NODES;
			else Project.hide &= ~HIDE_NODES;


			EndDialog(hWnd, TRUE);
		}

		if (IDCANCEL == LOWORD(wParam))
		{
			EndDialog(hWnd, TRUE);
		}
	}

	if (WM_INITDIALOG == uMsg)
	{
		if (HIDESPEECH)						SetClick(hWnd, IDC_HIDESPEECH);

		if (Project.hide & HIDE_BACKGROUND) SetClick(hWnd, IDC_HIDEBACKGROUND);

		if (Project.hide & HIDE_NPC)		SetClick(hWnd, IDC_HIDENPC);

		if (Project.hide & HIDE_FIXINTER)	SetClick(hWnd, IDC_HIDEFIXINTER);

		if (Project.hide & HIDE_ITEMS)		SetClick(hWnd, IDC_HIDEITEMS);

		if (Project.hide & HIDE_PARTICLES)	SetClick(hWnd, IDC_HIDEPARTICLES);

		if (Project.hide & HIDE_CAMERAS)	SetClick(hWnd, IDC_HIDECAMERAS);

		if (Project.hide & HIDE_INTERFACE)	SetClick(hWnd, IDC_HIDEINTERFACE);

		if (Project.hide & HIDE_NODES)		SetClick(hWnd, IDC_HIDENODES);

		if (HIDEANCHORS)					SetClick(hWnd, IDC_HIDEANCHORS);

		if (HIDEMAGICDUST)					SetClick(hWnd, IDC_HIDEMAGICDUST);

		if (WATERFX)						SetClick(hWnd, IDC_WATERFX);

		if (REFLECTFX)						SetClick(hWnd, IDC_REFLECTFX);

		if (TreatAllIO)						SetClick(hWnd, IDC_TREATALLIO);


		return TRUE;
	}

	return FALSE;
	return WM_INITDIALOG == uMsg ? TRUE : FALSE;
}
extern long CHANGE_LEVEL_PROC_RESULT;

BOOL CALLBACK ChangeLevelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_COMMAND == uMsg)
	{
		if (IDC_GOTOPOLY == LOWORD(wParam))
		{
			ARX_PLAYER_GotoAnyPoly();
			CHANGE_LEVEL_PROC_RESULT = -1;
			EndDialog(hWnd, TRUE);
		}
		else if (IDOK == LOWORD(wParam))
		{
			if (IsChecked(hWnd, IDC_C_LEVEL0))	CHANGE_LEVEL_PROC_RESULT = 0;

			if (IsChecked(hWnd, IDC_C_LEVEL1))	CHANGE_LEVEL_PROC_RESULT = 1;

			if (IsChecked(hWnd, IDC_C_LEVEL2))	CHANGE_LEVEL_PROC_RESULT = 2;

			if (IsChecked(hWnd, IDC_C_LEVEL3))	CHANGE_LEVEL_PROC_RESULT = 3;

			if (IsChecked(hWnd, IDC_C_LEVEL4))	CHANGE_LEVEL_PROC_RESULT = 4;

			if (IsChecked(hWnd, IDC_C_LEVEL5))	CHANGE_LEVEL_PROC_RESULT = 5;

			if (IsChecked(hWnd, IDC_C_LEVEL6))	CHANGE_LEVEL_PROC_RESULT = 6;

			if (IsChecked(hWnd, IDC_C_LEVEL7))	CHANGE_LEVEL_PROC_RESULT = 7;

			if (IsChecked(hWnd, IDC_C_LEVEL8))	CHANGE_LEVEL_PROC_RESULT = 8;

			if (IsChecked(hWnd, IDC_C_LEVEL9))	CHANGE_LEVEL_PROC_RESULT = 9;

			if (IsChecked(hWnd, IDC_C_LEVEL10))	CHANGE_LEVEL_PROC_RESULT = 10;

			if (IsChecked(hWnd, IDC_C_LEVEL11))	CHANGE_LEVEL_PROC_RESULT = 11;

			if (IsChecked(hWnd, IDC_C_LEVEL12))	CHANGE_LEVEL_PROC_RESULT = 12;

			if (IsChecked(hWnd, IDC_C_LEVEL13))	CHANGE_LEVEL_PROC_RESULT = 13;

			if (IsChecked(hWnd, IDC_C_LEVEL14))	CHANGE_LEVEL_PROC_RESULT = 14;

			if (IsChecked(hWnd, IDC_C_LEVEL15))	CHANGE_LEVEL_PROC_RESULT = 15;

			if (IsChecked(hWnd, IDC_C_LEVEL16))	CHANGE_LEVEL_PROC_RESULT = 16;

			if (IsChecked(hWnd, IDC_C_LEVEL17))	CHANGE_LEVEL_PROC_RESULT = 17;

			if (IsChecked(hWnd, IDC_C_LEVEL18))	CHANGE_LEVEL_PROC_RESULT = 18;

			if (IsChecked(hWnd, IDC_C_LEVEL19))	CHANGE_LEVEL_PROC_RESULT = 19;

			if (IsChecked(hWnd, IDC_C_LEVEL20))	CHANGE_LEVEL_PROC_RESULT = 20;

			if (IsChecked(hWnd, IDC_C_LEVEL21))	CHANGE_LEVEL_PROC_RESULT = 21;

			if (IsChecked(hWnd, IDC_C_LEVEL22))	CHANGE_LEVEL_PROC_RESULT = 22;

			if (IsChecked(hWnd, IDC_C_LEVEL23))	CHANGE_LEVEL_PROC_RESULT = 23;

			EndDialog(hWnd, TRUE);
		}
		else if (IDCANCEL == LOWORD(wParam))
		{

			EndDialog(hWnd, TRUE);
		}
	}

	if (WM_INITDIALOG == uMsg)
	{
		CHANGE_LEVEL_PROC_RESULT = -1;
		SetClick(hWnd, IDC_C_LEVEL0);
		return TRUE;
	}

	return FALSE;
	return WM_INITDIALOG == uMsg ? TRUE : FALSE;
}



EERIE_LIGHT lightparam;
EERIE_LIGHT lightcopy;
extern HWND CDP_LIGHTOptions;
extern EERIE_LIGHT * CDP_EditLight;
long CONSTANTUPDATELIGHT = 0;

void LightApply(HWND hWnd)
{
	HWND thWnd;

	if (CDP_EditLight != NULL)
	{
		lightparam.extras &= EXTRAS_FIREPLACE;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
		lightparam.fallstart = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
		lightparam.fallend = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
		lightparam.intensity = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) / 100.f;

		if (IsChecked(hWnd, IDC_SEMIDYNAMIC))	lightparam.extras |= EXTRAS_SEMIDYNAMIC;
		else lightparam.extras &= ~EXTRAS_SEMIDYNAMIC;

		if (IsChecked(hWnd, IDC_EXTINGUISH))		lightparam.extras |= EXTRAS_EXTINGUISHABLE;
		else lightparam.extras &= ~EXTRAS_EXTINGUISHABLE;

		if (IsChecked(hWnd, IDC_EXTINGUISH2))	lightparam.extras |= EXTRAS_STARTEXTINGUISHED;
		else lightparam.extras &= ~EXTRAS_STARTEXTINGUISHED;

		if (IsChecked(hWnd, IDC_NO_IGNIT))	lightparam.extras |= EXTRAS_NO_IGNIT;
		else lightparam.extras &= ~EXTRAS_NO_IGNIT;

		if (IsChecked(hWnd, IDC_SPAWNFIRE))		lightparam.extras |= EXTRAS_SPAWNFIRE;
		else lightparam.extras &= ~EXTRAS_SPAWNFIRE;

		if (IsChecked(hWnd, IDC_SPAWNSMOKE))		lightparam.extras |= EXTRAS_SPAWNSMOKE;
		else lightparam.extras &= ~EXTRAS_SPAWNSMOKE;

		if (IsChecked(hWnd, IDC_CAST_SHADOWS))	lightparam.extras |= EXTRAS_NOCASTED;
		else lightparam.extras &= ~EXTRAS_NOCASTED;

		if (IsChecked(hWnd, IDC_FIXFLARESIZE))	lightparam.extras |= EXTRAS_FIXFLARESIZE;
		else lightparam.extras &= ~EXTRAS_FIXFLARESIZE;

		if (IsChecked(hWnd, IDC_COLORLEGACY))	lightparam.extras |= EXTRAS_COLORLEGACY;
		else lightparam.extras &= ~EXTRAS_COLORLEGACY;

		if (IsChecked(hWnd, IDC_FLARE))	lightparam.extras |= EXTRAS_FLARE;
		else lightparam.extras &= ~EXTRAS_FLARE;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
		lightparam.ex_flicker.r = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
		lightparam.ex_flicker.g = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
		lightparam.ex_flicker.b = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
		lightparam.ex_radius = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

		thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
		lightparam.ex_frequency = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
		lightparam.ex_size = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV10;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
		lightparam.ex_speed = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;

		thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
		lightparam.ex_flaresize = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

		lightparam.pos.x = CDP_EditLight->pos.x;
		lightparam.pos.y = CDP_EditLight->pos.y;
		lightparam.pos.z = CDP_EditLight->pos.z;
		lightparam.tl = CDP_EditLight->tl;

		memcpy(CDP_EditLight, &lightparam, sizeof(EERIE_LIGHT));

		if (CDP_EditLight->tl != -1) DynLight[CDP_EditLight->tl].exist = 0;

		if (!(lightparam.extras & EXTRAS_SEMIDYNAMIC))
		{
			RecalcLight(CDP_EditLight);
			RecalcLightZone(CDP_EditLight->pos.x, CDP_EditLight->pos.y, CDP_EditLight->pos.z, (long)(CDP_EditLight->fallend * ACTIVEBKG->Xmul) + 1);
		}
	}
	}

long INITT = 0;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK LightOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;
	long l;

	switch (uMsg)
	{
		case WM_MOVE:

			if (danaeApp.m_pDeviceInfo->bWindowed)
			{
				RECT rect1, rect2;
				GetWindowRect(danaeApp.m_hWnd, &rect1);
				GetWindowRect(hWnd, &rect2);
				long posx = rect2.left - rect1.left;
				long posy = rect2.top - rect1.top;
				Danae_Registry_WriteValue("WND_LightOptions_POSX", posx);
				Danae_Registry_WriteValue("WND_LightOptions_POSY", posy);
			}

			break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_CONSTANTUPDATE:
					PrecalcIOLighting(NULL, 0, 1);

					if (CONSTANTUPDATELIGHT)
					{
						CONSTANTUPDATELIGHT = 0;
						thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
						SetWindowText(thWnd, "No Real-Time Update");
					}
					else
					{
						CONSTANTUPDATELIGHT = 1;
						thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
						SetWindowText(thWnd, "Real-Time Update");
						SendMessage(hWnd, WM_COMMAND, IDAPPLY, 0);
					}

					break;
				case IDC_SEMIDYNAMIC:
				case IDC_EXTINGUISH:
				case IDC_EXTINGUISH2:
				case IDC_SPAWNFIRE:
				case IDC_SPAWNSMOKE:
				case IDC_COLORLEGACY:
				case IDC_CAST_SHADOWS:
				case IDC_FIXFLARESIZE:
				case IDC_SLIDER11:
				case IDC_SLIDER12:
				case IDC_SLIDER13:
				case IDC_SLIDER14:
				case IDC_SLIDER15:
				case IDC_SLIDER16:
				case IDC_SLIDER17:
				case IDC_SLIDER18:
				case IDC_SLIDER19:
				case IDC_SLIDER20:
				case IDC_SLIDER21:
				{
					PrecalcIOLighting(NULL, 0, 1);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);
				}
				break;
				case IDC_LIGHTCOLOR:
					PrecalcIOLighting(NULL, 0, 1);
					CHOOSECOLOR cc;
					cc.lStructSize = sizeof(CHOOSECOLOR);
					cc.hwndOwner = danaeApp.m_hWnd;
					cc.hInstance = 0; //Ignored
					cc.rgbResult = (((long)(float)(lightparam.rgb.r * 255.f) & 255))
					               | (((long)(float)(lightparam.rgb.g * 255.f) & 255) << 8)
					               | (((long)(float)(lightparam.rgb.b * 255.f) & 255)) << 16;
					cc.lpCustColors = custcr;
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

					if (ChooseColor(&cc))
					{
						lightparam.rgb.b = (float)((cc.rgbResult >> 16 & 255)) * DIV255;
						lightparam.rgb.g = (float)((cc.rgbResult >> 8 & 255)) * DIV255;
						lightparam.rgb.r = (float)((cc.rgbResult & 255)) * DIV255;
					}

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDAPPLY:
				{
					PrecalcIOLighting(NULL, 0, 1);
					LightApply(hWnd);
				}
				break;
				case IDCOPY:
					PrecalcIOLighting(NULL, 0, 1);
					memcpy(&lightcopy, &lightparam, sizeof(EERIE_LIGHT));
					break;
				case IDPASTE:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.fallstart = lightcopy.fallstart;
					lightparam.fallend = lightcopy.fallend;
					lightparam.intensity = lightcopy.intensity;
					lightparam.rgb.r = lightcopy.rgb.r;
					lightparam.rgb.g = lightcopy.rgb.g;
					lightparam.rgb.b = lightcopy.rgb.b;
					lightparam.type = lightcopy.type;
					RecalcLight(&lightparam);
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);
					break;
				case IDRESET:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.fallstart = 500.f;
					lightparam.fallend = 600.f;
					lightparam.intensity = 1.2f;
					lightparam.rgb.r = 1.f;
					lightparam.rgb.g = 0.f;
					lightparam.rgb.b = 0.f;
					RecalcLight(&lightparam);
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);
					break;
				case IDC_SET_FIRE:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.rgb.r = 0.71f;
					lightparam.rgb.g = 0.43f;
					lightparam.rgb.b = 0.29f;
					lightparam.intensity = 4.f;
					lightparam.fallstart = 250.f;
					lightparam.fallend = 450.f;
					lightparam.ex_flicker.r = 0.25f;
					lightparam.ex_flicker.g = 0.25f;
					lightparam.ex_flicker.b = 0.25f;
					lightparam.ex_radius = 1;
					lightparam.ex_frequency = 0.7f;
					lightparam.ex_size = 0.8f;
					lightparam.ex_speed = 0.7f;
					lightparam.ex_flaresize = 40.f;
					lightparam.extras =	EXTRAS_SEMIDYNAMIC | EXTRAS_EXTINGUISHABLE |
					                    EXTRAS_SPAWNFIRE   | EXTRAS_SPAWNSMOKE | EXTRAS_FLARE;
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDC_SET_FIRE2:
					PrecalcIOLighting(NULL, 0, 1);
					lightparam.rgb.r = 0.71f;
					lightparam.rgb.g = 0.43f;
					lightparam.rgb.b = 0.29f;
					lightparam.intensity = 4.f;
					lightparam.fallstart = 350.f;
					lightparam.fallend = 520.f;
					lightparam.ex_flicker.r = 0.25f;
					lightparam.ex_flicker.g = 0.25f;
					lightparam.ex_flicker.b = 0.25f;
					lightparam.ex_radius = 20;
					lightparam.ex_frequency = 0.8f;
					lightparam.ex_size = 1.f;
					lightparam.ex_speed = 0.65f;
					lightparam.ex_flaresize = 95.f;
					lightparam.extras =	EXTRAS_SEMIDYNAMIC | EXTRAS_EXTINGUISHABLE |
					                    EXTRAS_SPAWNFIRE   | EXTRAS_SPAWNSMOKE | EXTRAS_FIREPLACE | EXTRAS_FLARE;;
					SendMessage(hWnd, WM_INITDIALOG, 0, 0);

					if (!INITT) if (CONSTANTUPDATELIGHT) LightApply(hWnd);

					break;
				case IDOK:
					PrecalcIOLighting(NULL, 0, 1);
					LightApply(hWnd);
					CDP_LIGHTOptions = NULL;

					EndDialog(hWnd, TRUE);
					break;
				case IDCANCEL:
					PrecalcIOLighting(NULL, 0, 1);
					CDP_LIGHTOptions = NULL;

					if ((CDP_EditLight) && (CDP_EditLight->tl != -1)) DynLight[CDP_EditLight->tl].exist = 0;

					EndDialog(hWnd, TRUE);
					break;
			}
		}
		break;
		case WM_INITDIALOG:

			if (danaeApp.m_pDeviceInfo->bWindowed)
			{
				long posx, posy;
				Danae_Registry_ReadValue("WND_LightOptions_POSX", &posx, 0);
				Danae_Registry_ReadValue("WND_LightOptions_POSY", &posy, 0);

				if ((posx != -1) && (posy != -1)
				        && (posx < 1000) && (posx > -1000)
				        && (posy < 1000) && (posy > -1000)
				   )
				{
					RECT rect1;
					GetWindowRect(danaeApp.m_hWnd, &rect1);
					posx = rect1.left + posx;
					posy = rect1.left + posy;

					if (posx < 0) posx = 0;

					if (posy < 0) posy = 0;

					SetWindowPos(hWnd, NULL, posx, posy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			INITT = 1;

			if (CONSTANTUPDATELIGHT)
			{
				thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
				SetWindowText(thWnd, "Real-Time Update");
			}
			else
			{
				thWnd = GetDlgItem(hWnd, IDC_CONSTANTUPDATE);
				SetWindowText(thWnd, "No Real-Time Update");
			}

			if ((CDP_EditLight) && (CDP_EditLight->tl != -1))
			{
				DynLight[CDP_EditLight->tl].exist = 0;
				CDP_EditLight->tl = NULL;
			}

			thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 2000));
			l = (long)(float)(lightparam.fallstart);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 2000));
			l = (long)(float)(lightparam.fallend);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 500));
			l = (long)(float)(lightparam.intensity * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			if (lightparam.extras & EXTRAS_NOCASTED) SetCheck(hWnd, IDC_CAST_SHADOWS, CHECK);
			else SetCheck(hWnd, IDC_CAST_SHADOWS, UNCHECK);

			if (lightparam.extras & EXTRAS_SEMIDYNAMIC) SetCheck(hWnd, IDC_SEMIDYNAMIC, CHECK);
			else SetCheck(hWnd, IDC_SEMIDYNAMIC, UNCHECK);

			if (lightparam.extras & EXTRAS_EXTINGUISHABLE) SetCheck(hWnd, IDC_EXTINGUISH, CHECK);
			else SetCheck(hWnd, IDC_EXTINGUISH, UNCHECK);

			if (lightparam.extras & EXTRAS_STARTEXTINGUISHED) SetCheck(hWnd, IDC_EXTINGUISH2, CHECK);
			else SetCheck(hWnd, IDC_EXTINGUISH2, UNCHECK);

			if (lightparam.extras & EXTRAS_NO_IGNIT) SetCheck(hWnd, IDC_NO_IGNIT, CHECK);
			else SetCheck(hWnd, IDC_NO_IGNIT, UNCHECK);


			if (lightparam.extras & EXTRAS_SPAWNFIRE) SetCheck(hWnd, IDC_SPAWNFIRE, CHECK);
			else SetCheck(hWnd, IDC_SPAWNFIRE, UNCHECK);

			if (lightparam.extras & EXTRAS_SPAWNSMOKE) SetCheck(hWnd, IDC_SPAWNSMOKE, CHECK);
			else SetCheck(hWnd, IDC_SPAWNSMOKE, UNCHECK);

			if (lightparam.extras & EXTRAS_COLORLEGACY) SetCheck(hWnd, IDC_COLORLEGACY, CHECK);
			else SetCheck(hWnd, IDC_COLORLEGACY, UNCHECK);

			if (lightparam.extras & EXTRAS_FLARE) SetCheck(hWnd, IDC_FLARE, CHECK);
			else SetCheck(hWnd, IDC_FLARE, UNCHECK);

			if (lightparam.extras & EXTRAS_FIXFLARESIZE) SetCheck(hWnd, IDC_FIXFLARESIZE, CHECK);
			else SetCheck(hWnd, IDC_FIXFLARESIZE, UNCHECK);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.r * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.g * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_flicker.b * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			l = (long)(float)(lightparam.ex_radius);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 100));
			l = (long)((float)(lightparam.ex_frequency * 100.f));
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 30));
			l = (long)((float)(lightparam.ex_size * 10.f));
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			l = (long)((float)(lightparam.ex_speed) * 100.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 200));
			l = (long)((float)(lightparam.ex_flaresize));
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(l));
			INITT = 0;
			return TRUE;
			break;
		case WM_NOTIFY:
			float val, val1, val2;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_SLIDER11);
			val1 = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC11);
			sprintf(temp, "%4.0f", val1);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
			val2 = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC12);
			sprintf(temp, "%4.0f", val2);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER13);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV100;
			thWnd = GetDlgItem(hWnd, IDC_STATIC13);
			sprintf(temp, "%2.2f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER14);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC14);
			sprintf(temp, "%3d%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER15);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC15);
			sprintf(temp, "%3d%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER16);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC16);
			sprintf(temp, "%3d%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER17);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC17);
			sprintf(temp, "%3dcm", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER18);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC18);
			sprintf(temp, "%3d%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER19);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC19);
			sprintf(temp, "%3d", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER20);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC20);
			sprintf(temp, "%3d%%", (long)val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER21);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC21);
			sprintf(temp, "%3d", (long)val);
			SetWindowText(thWnd, temp);

			if (val2 < val1)
			{
				thWnd = GetDlgItem(hWnd, IDC_SLIDER12);
				SendMessage(thWnd, TBM_SETPOS, FALSE, (LPARAM)(val1));
			}

			if (!INITT)
				if (((int) wParam == IDC_SLIDER11)
				        ||	((int) wParam == IDC_SLIDER12)
				        ||	((int) wParam == IDC_SLIDER13)
				        ||	((int) wParam == IDC_SLIDER14)
				        ||	((int) wParam == IDC_SLIDER15)
				        ||	((int) wParam == IDC_SLIDER16)
				        ||	((int) wParam == IDC_SLIDER17)
				        ||	((int) wParam == IDC_SLIDER18)
				        ||	((int) wParam == IDC_SLIDER19)
				        ||	((int) wParam == IDC_SLIDER20)
				        ||	((int) wParam == IDC_SLIDER21))
					if (CONSTANTUPDATELIGHT) LightApply(hWnd);

			break;
	}

	return FALSE;
}

FOG_DEF fogparam;
FOG_DEF fogcopy;
extern HWND CDP_FogOptions;
extern FOG_DEF * CDP_EditFog;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK FogOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;
	float t;

	switch (uMsg)
	{
		case WM_COMMAND:
		{
			if (IDC_BUTTON_COLOR == LOWORD(wParam))
			{
				CHOOSECOLOR cc;
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = danaeApp.m_hWnd;
				cc.hInstance = 0; //Ignored
				cc.rgbResult = (((long)(float)(fogparam.rgb.r * 255.f) & 255))
				               | (((long)(float)(fogparam.rgb.g * 255.f) & 255) << 8)
				               | (((long)(float)(fogparam.rgb.b * 255.f) & 255)) << 16;
				cc.lpCustColors = custcr;
				cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

				if (ChooseColor(&cc))
				{
					fogparam.rgb.b = (float)((cc.rgbResult >> 16 & 255)) * DIV255;
					fogparam.rgb.g = (float)((cc.rgbResult >> 8 & 255)) * DIV255;
					fogparam.rgb.r = (float)((cc.rgbResult & 255)) * DIV255;
				}
			}

			if (IDAPPLY == LOWORD(wParam))
			{
				if (CDP_EditFog != NULL)
				{
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
					fogparam.rotatespeed = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) / 1000.f;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
					fogparam.speed = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
					fogparam.size = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
					fogparam.scale = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
					fogparam.tolive = (long)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * 100;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
					fogparam.frequency = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_DIRECTIONAL);

					if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED)
						fogparam.special |= FOG_DIRECTIONAL;

					fogparam.pos.x = CDP_EditFog->pos.x;
					fogparam.pos.y = CDP_EditFog->pos.y;
					fogparam.pos.z = CDP_EditFog->pos.z;
					memcpy(CDP_EditFog, &fogparam, sizeof(FOG_DEF));
				}
			}

			if (IDCOPY == LOWORD(wParam))
			{
				memcpy(&fogcopy, &fogparam, sizeof(FOG_DEF));
			}

			if (IDPASTE == LOWORD(wParam))
			{
				fogparam.angle.a = fogcopy.angle.a;
				fogparam.angle.b = fogcopy.angle.b;
				fogparam.angle.g = fogcopy.angle.g;
				fogparam.blend = fogcopy.blend;
				fogparam.frequency = fogcopy.frequency;
				fogparam.move.x = fogcopy.move.x;
				fogparam.move.y = fogcopy.move.y;
				fogparam.move.z = fogcopy.move.z;
				fogparam.rgb.r = fogcopy.rgb.r;
				fogparam.rgb.g = fogcopy.rgb.g;
				fogparam.rgb.b = fogcopy.rgb.b;
				fogparam.rotatespeed = fogcopy.rotatespeed;
				fogparam.scale = fogcopy.scale;
				fogparam.size = fogcopy.size;
				fogparam.special = fogcopy.special;
				fogparam.speed = fogcopy.speed;
				fogparam.tolive = fogcopy.tolive;
				SendMessage(hWnd, WM_INITDIALOG, 0, 0);
			}

			if (IDRESET == LOWORD(wParam))
			{
				fogparam.angle.a = 0.f;
				fogparam.angle.b = 0.f;
				fogparam.angle.g = 0.f;
				fogparam.move.x = 0.f;
				fogparam.move.y = 0.f;
				fogparam.move.z = 0.f;
				fogparam.special = (long)0.f;
				fogparam.blend = (long)0.f;
				fogparam.frequency = 17.f;
				fogparam.rgb.r = 0.3f;
				fogparam.rgb.g = 0.3f;
				fogparam.rgb.b = 0.5f;
				fogparam.rotatespeed = 0.001f;
				fogparam.scale = 8.f;
				fogparam.size = 80.f;
				fogparam.speed = 1.f;
				fogparam.tolive = 4500;
				SendMessage(hWnd, WM_INITDIALOG, 0, 0);
			}

			if (IDOK == LOWORD(wParam))
			{
				if (CDP_EditFog != NULL)
				{
					thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
					fogparam.rotatespeed = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) / 1000.f;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
					fogparam.speed = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
					fogparam.size = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
					fogparam.scale = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
					fogparam.tolive = (long)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * 100;

					thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
					fogparam.frequency = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);

					thWnd = GetDlgItem(hWnd, IDC_DIRECTIONAL);

					if (SendMessage(thWnd, BM_GETSTATE, 0, 0) == BST_CHECKED)
						fogparam.special |= FOG_DIRECTIONAL;

					memcpy(CDP_EditFog, &fogparam, sizeof(FOG_DEF));
				}

				CDP_FogOptions = NULL;
				EndDialog(hWnd, TRUE);
			}

			if (IDCANCEL == LOWORD(wParam))
			{
				CDP_FogOptions = NULL;
				EndDialog(hWnd, TRUE);
			}
		}
		break;
		case WM_INITDIALOG:
		{

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(-100, 100));
			t = (float)(long)(fogparam.rotatespeed * 1000.f);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(0, 100));
			t = (float)(long)(fogparam.speed);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 80));
			t = (float)(long)(fogparam.size);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(-80, 80));
			t = (float)(long)(fogparam.scale);
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 400));
			t = (float)(long)(fogparam.tolive) / 100;
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
			SendMessage(thWnd, TBM_SETRANGE, TRUE, (LPARAM) MAKELONG(1, 99));
			t = (float)(long)fogparam.frequency;
			SendMessage(thWnd, TBM_SETPOS, TRUE, (LPARAM)(t));



			if (fogparam.special & FOG_DIRECTIONAL) SetClick(hWnd, IDC_DIRECTIONAL);

			return TRUE;
		}
		case WM_NOTIFY:
			float val;
			char temp[64];
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_SLIDER_FREQUENCY_TEXT);
			sprintf(temp, "%2.0f%%", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_ROTATIONSPEED);
			val = ((float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0)) / 1000.f;
			thWnd = GetDlgItem(hWnd, IDC_STATIC2);
			sprintf(temp, "%1.3f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_MOVESPEED);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC3);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_INITSIZE);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC4);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_SCALING);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0);
			thWnd = GetDlgItem(hWnd, IDC_STATIC5);
			sprintf(temp, "%3.0f", val);
			SetWindowText(thWnd, temp);

			thWnd = GetDlgItem(hWnd, IDC_SLIDER_DURATION);
			val = (float)SendMessage(thWnd, TBM_GETPOS, TRUE, 0) * DIV10;
			thWnd = GetDlgItem(hWnd, IDC_STATIC6);
			sprintf(temp, "%4.2fs", val);
			SetWindowText(thWnd, temp);
			break;
	}

	return FALSE;
}
long SHOWWARNINGS = 0;
extern HWND CDP_IOOptions;
extern INTERACTIVE_OBJ * CDP_EditIO;
#define MAX_SCRIPT_SIZE 128000
char text1[MAX_SCRIPT_SIZE+1];
char text2[MAX_SCRIPT_SIZE+1];
 
//*************************************************************************************
//*************************************************************************************
 
extern HWND LastErrorPopupNO2;
extern HWND LastErrorPopupNO1;

UINT uFindReplaceMsg;

long IOScript_X = -1;
long IOScript_Y = -1;
long IOScript_XX = -1;
long IOScript_YY = -1;

#if _ARX_CEDITOR_
CEditor * edit1 = NULL;
CEditor * edit2 = NULL;
#endif

long edit_lin1 = 0;
long edit_lin2 = 0;
INTERACTIVE_OBJ * edit_io = NULL;

//*************************************************************************************
//*************************************************************************************

BOOL CALLBACK IOOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND thWnd;

	switch (uMsg)
	{
		case WM_SIZE:
			RECT trec, rec, rec2, wndrect;
			GetWindowRect(hWnd, &wndrect);
			GetClientRect(hWnd, &trec);
			long space;
			space = ((trec.bottom - 60) / 2) - 25;

			rec.left = trec.left + 4;
			rec.right = trec.right - 6;
			rec.top = 60;
			rec.bottom = space;

			rec2.left = rec.left;
			rec2.right = rec.right;
			rec2.top = rec.top + rec.bottom + 25;
			rec2.bottom = space;


			thWnd = GetDlgItem(hWnd, IDC_EDIT1);
			SetWindowPos(thWnd, HWND_TOP, rec.left, rec.top, rec.right, rec.bottom, SWP_NOZORDER);
			UpdateWindow(thWnd);
			thWnd = GetDlgItem(hWnd, IDC_EDIT2);
			SetWindowPos(thWnd, HWND_TOP, rec2.left, rec2.top, rec2.right, rec2.bottom, SWP_NOZORDER);
			UpdateWindow(thWnd);

			// Primary win
			long px, py;
			px = rec.left;
			py = rec.top - 22;
			thWnd = GetDlgItem(hWnd, IDC_LOCSCR1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICCOL1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_COL1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICLINE1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_LINE1);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			// Secondary win
			px = rec2.left;
			py = rec2.top - 22;
			thWnd = GetDlgItem(hWnd, IDC_LOCSCR2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICCOL2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_COL2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_STATICLINE2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;

			thWnd = GetDlgItem(hWnd, IDC_LINE2);
			SetWindowPos(thWnd, HWND_TOP, px, py, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			InvalidateRect(thWnd, NULL, TRUE);
			UpdateWindow(thWnd);
			GetClientRect(thWnd, &rec);
			px += rec.right + 5;


			UpdateWindow(hWnd);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			KillLightThread();

#if _ARX_CEDITOR_
			if ((CDP_EditIO) && edit1 && edit2)
			{
				edit_io = CDP_EditIO;
				edit_lin1 = LOWORD(edit1->GetCurrentPos());
				edit_lin2 = LOWORD(edit2->GetCurrentPos());
			}

			CDP_IOOptions = NULL;
			SAFE_DELETE(edit1);
			SAFE_DELETE(edit2);
#endif
			CDP_IOOptions = NULL;
			RECT _wndrect;
			GetWindowRect(hWnd, &_wndrect);
			IOScript_X = _wndrect.left;
			IOScript_Y = _wndrect.top;
			IOScript_XX = _wndrect.right - _wndrect.left;
			IOScript_YY = _wndrect.bottom - _wndrect.top;
			EndDialog(hWnd, TRUE);
			break;
		case WM_COMMAND:
		{
			if (IDSYNTAX == LOWORD(wParam))
			{
				if (CDP_EditIO != NULL)
				{
					long SC = SYNTAXCHECKING;
					SYNTAXCHECKING = 1;
					SHOWWARNINGS = 1;
					CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

					if (CheckScriptSyntax(CDP_EditIO) != TRUE) CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
					else CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

					SHOWWARNINGS = 0;
					RECT rec;
					GetWindowRect(hWnd, &rec);

					if (LastErrorPopupNO2 != NULL)
					{
						SetWindowPos(LastErrorPopupNO2, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top + 200, 0, 0, SWP_NOSIZE);
						SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
						LastErrorPopupNO2 = NULL;
					}

					if (LastErrorPopupNO1 != NULL)
					{
						SetWindowPos(LastErrorPopupNO1, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top, 0, 0, SWP_NOSIZE);
						SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
						LastErrorPopupNO1 = NULL;
					}

					SYNTAXCHECKING = SC;
				}
			}
			else if (IDOK == LOWORD(wParam))
			{
				if (CDP_EditIO != NULL)
				{
					edit_io = CDP_EditIO;

					if (IsChecked(hWnd, IDC_FREEZESCRIPT))	CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
					else CDP_EditIO->ioflags &= ~IO_FREEZESCRIPT;

#if _ARX_CEDITOR_
					edit_lin1 = LOWORD(edit1->GetCurrentPos());
					edit_lin2 = LOWORD(edit2->GetCurrentPos());
					edit1->GetText(text1, MAX_SCRIPT_SIZE);
					edit2->GetText(text2, MAX_SCRIPT_SIZE);
#endif
					long i = 0;

					if (CDP_EditIO->script.data != NULL)
					{
						int n = strcmp(text1, CDP_EditIO->script.data);

						if (n) i += 1;
					}
					else if (strlen(text1) > 0) i += 1;

					if ((CDP_EditIO->over_script.data != NULL)
					        && (CDP_EditIO->ident != -1))
					{
						int n = strcmp(text2, CDP_EditIO->over_script.data);

						if (n) i += 2;
					}
					else if (strlen(text2) > 0) i += 2;

					switch (i)
					{
						case 3:

							if (OKBox("Save Changes to LOCAL & CLASS script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->script.data != NULL)
								{
									free(CDP_EditIO->script.data);
									CDP_EditIO->script.data = NULL;
								}

								CDP_EditIO->script.size = strlen(text1) + 1;
								CDP_EditIO->script.data = (char *)malloc(CDP_EditIO->script.size);
								strcpy(CDP_EditIO->script.data, text1);

								if (CDP_EditIO->over_script.data != NULL)
								{
									free(CDP_EditIO->over_script.data);
									CDP_EditIO->over_script.data = NULL;
								}

								CDP_EditIO->over_script.size = strlen(text2) + 1;
								CDP_EditIO->over_script.data = (char *)malloc(CDP_EditIO->over_script.size);
								strcpy(CDP_EditIO->over_script.data, text2);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 1);
								SaveIOScript(CDP_EditIO, 2);
							}

							break;
						case 2:

							if (OKBox("Save Changes to LOCAL script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->over_script.data != NULL)
								{
									free(CDP_EditIO->over_script.data);
									CDP_EditIO->over_script.data = NULL;
								}

								CDP_EditIO->over_script.size = strlen(text2) + 1;
								CDP_EditIO->over_script.data = (char *)malloc(CDP_EditIO->over_script.size);
								strcpy(CDP_EditIO->over_script.data, text2);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 2);
							}

							break;
						case 1:

							if (OKBox("Save Changes to CLASS script ?", "SAVE Confirmation"))
							{
								if (CDP_EditIO->script.data != NULL)
								{
									free(CDP_EditIO->script.data);
									CDP_EditIO->script.data = NULL;
								}

								CDP_EditIO->script.size = strlen(text1) + 1;
								CDP_EditIO->script.data = (char *)malloc(CDP_EditIO->script.size);
								strcpy(CDP_EditIO->script.data, text1);

								if (CDP_EditIO->script.data != NULL)
									CDP_EditIO->over_script.master = (void *)&CDP_EditIO->script;
								else CDP_EditIO->over_script.master = NULL;

								SaveIOScript(CDP_EditIO, 1);
							}

							break;
					}

					if (!(CDP_EditIO->ioflags & IO_FREEZESCRIPT))
						if (CheckScriptSyntax(CDP_EditIO) != TRUE) CDP_EditIO->ioflags |= IO_FREEZESCRIPT;
				}

				CDP_IOOptions = NULL;
#if _ARX_CEDITOR_
				SAFE_DELETE(edit1);
				SAFE_DELETE(edit2);
#endif			
				RECT _wndrect;
				GetWindowRect(hWnd, &_wndrect);
				IOScript_X = _wndrect.left;
				IOScript_Y = _wndrect.top;
				IOScript_XX = _wndrect.right - _wndrect.left;
				IOScript_YY = _wndrect.bottom - _wndrect.top;
				EndDialog(hWnd, TRUE);
			}

			if (IDCANCEL == LOWORD(wParam))
			{

			#if _ARX_CEDITOR_
				if ((CDP_EditIO) && (edit1) && (edit2))
				{
					edit_io = CDP_EditIO;
					edit_lin1 = LOWORD(edit1->GetCurrentPos());
					edit_lin2 = LOWORD(edit2->GetCurrentPos());
				}

				SAFE_DELETE(edit1);
				SAFE_DELETE(edit2);		
			#endif
			
				CDP_IOOptions = NULL;
				RECT _wndrect;
				GetWindowRect(hWnd, &_wndrect);
				IOScript_X = _wndrect.left;
				IOScript_Y = _wndrect.top;
				IOScript_XX = _wndrect.right - _wndrect.left;
				IOScript_YY = _wndrect.bottom - _wndrect.top;
				EndDialog(hWnd, TRUE);
			}
		}
		break;
		case WM_INITDIALOG:
		{
			uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

			if (CDP_EditIO)
			{
				// Isis Athena
				char temp[256];
				thWnd = GetDlgItem(hWnd, IDC_OBJNAME);
				strcpy(temp, GetName(CDP_EditIO->filename));
				sprintf(temp, "%s_%04d", temp, CDP_EditIO->ident);
				SetWindowText(thWnd, temp);

				thWnd = GetDlgItem(hWnd, IDC_EDIT1);

#if _ARX_CEDITOR_
				edit1 = new CEditor(thWnd, SW_SHOW);

				if (CDP_EditIO->script.data)
				{
					memcpy(text1, CDP_EditIO->script.data, CDP_EditIO->script.size);
					text1[CDP_EditIO->script.size] = 0;

					for (long k = CDP_EditIO->script.size; k > 0; k--)
					{
						if (text1[k] <= 32) text1[k] = 0;
						else break;
					}
				}
				else strcpy(text1, "");

				edit1->SetText(text1);
				edit1->SetBackgroundColor(0x00FFFFFF);
				HWND h1, h2;
				h1 = GetDlgItem(hWnd, IDC_LINE1);
				h2 = GetDlgItem(hWnd, IDC_COL1);
				edit1->SetHwndWindowPos(h1, h2);

				thWnd = GetDlgItem(hWnd, IDC_EDIT2);
				edit2 = new CEditor(thWnd, SW_SHOW);

				if (CDP_EditIO->over_script.data)
				{
					memcpy(text2, CDP_EditIO->over_script.data, CDP_EditIO->over_script.size);
					text2[CDP_EditIO->over_script.size] = 0;

					for (long k = CDP_EditIO->over_script.size; k > 0; k--)
					{
						if (text2[k] <= 32) text2[k] = 0;
						else break;
					}
				}
				else strcpy(text2, "");

				edit2->SetText(text2);

				if (CDP_EditIO->ident == -1)
					edit2->SetBackgroundColor(0x000000FF);
				else
					edit2->SetBackgroundColor(0x00FFFFFF);

				h1 = GetDlgItem(hWnd, IDC_LINE2);
				h2 = GetDlgItem(hWnd, IDC_COL2);
				edit2->SetHwndWindowPos(h1, h2);

				if ((edit_io == CDP_EditIO) && (edit_io != NULL))
				{
					edit1->SetCurrentLinePos(edit_lin1);
					edit2->SetCurrentLinePos(edit_lin2);
				}

#endif

				if (CDP_EditIO->ioflags & IO_FREEZESCRIPT) SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);

				RECT rec;
				GetWindowRect(hWnd, &rec);

				if (LastErrorPopupNO2 != NULL)
				{
					SetWindowPos(LastErrorPopupNO2, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top + 200, 0, 0, SWP_NOSIZE);
					SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
				}

				if (LastErrorPopupNO1 != NULL)
				{
					SetWindowPos(LastErrorPopupNO1, HWND_TOPMOST, rec.left + rec.right - rec.left, rec.top, 0, 0, SWP_NOSIZE);
					SetCheck(hWnd, IDC_FREEZESCRIPT, CHECK);
				}
			}

			if (IOScript_X != -1)
			{
				SetWindowPos(hWnd, NULL, IOScript_X, IOScript_Y, IOScript_XX, IOScript_YY, SWP_NOZORDER);
			}

			return TRUE;
		}

	}

	return FALSE;

}
extern HWND CDP_SOUNDOptions;

//*************************************************************************************
//*************************************************************************************
 
BOOL CALLBACK LanguageOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	if (WM_COMMAND == uMsg)
		switch (LOWORD(wParam))
		{
			case IDOK:

				if (IsChecked(hWnd, IDC_LANGUAGE1))
				{
					strcpy(Project.localisationpath, "english");
				}

				if (IsChecked(hWnd, IDC_LANGUAGE2))
				{
					strcpy(Project.localisationpath, "fr");
				}

				EndDialog(hWnd, TRUE);
				break;
			case IDCANCEL:
				EndDialog(hWnd, TRUE);
				break;
		}

	return FALSE;
}

//*************************************************************************************
// Creates A Text Box
//*************************************************************************************
void TextBox(char * title, char * text, long size)
{
	ARX_TIME_Pause();
	GTE_TITLE = title;
	GTE_TEXT = text;
	GTE_SIZE = size;
	DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
	          MAKEINTRESOURCE(IDD_GAIATEXTEDIT), danaeApp.m_hWnd, GaiaTextEdit);
	ARX_TIME_UnPause();
}

extern long LastSelectedLight;

//-----------------------------------------------------------------------------

void launchlightdialog()
{
	if ((LastSelectedLight != -1)
	        && (GLight[LastSelectedLight] != NULL)
	        && (GLight[LastSelectedLight]->exist))
	{
		if (!CDP_LIGHTOptions)
		{
			memcpy(&lightparam, GLight[LastSelectedLight], sizeof(EERIE_LIGHT));
			CDP_EditLight = GLight[LastSelectedLight];

			if (danaeApp.m_pFramework->m_bIsFullscreen)
			{
				ARX_TIME_Pause();
				danaeApp.Pause(TRUE);
				DialogBox((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
				          MAKEINTRESOURCE(IDD_LIGHTDIALOG), danaeApp.m_hWnd, LightOptionsProc);
				danaeApp.Pause(FALSE);
				ARX_TIME_UnPause();
			}
			else
				CDP_LIGHTOptions = (CreateDialogParam((HINSTANCE)GetWindowLong(danaeApp.m_hWnd, GWL_HINSTANCE),
				                                      MAKEINTRESOURCE(IDD_LIGHTDIALOG), danaeApp.m_hWnd, LightOptionsProc, 0));
		}
	}
}
