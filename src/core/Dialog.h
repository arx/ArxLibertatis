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
// DanaeDlg.H
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

#ifndef ARX_CORE_DIALOG_H
#define ARX_CORE_DIALOG_H

#include <windows.h>

struct INTERACTIVE_OBJ;

#define IOTVTYPE_PATH 1
#define IOTVTYPE_PLAYER 3

extern long FASTLOADS;

void LaunchInteractiveObjectsApp(HWND hwnd);
void InterTreeViewItemRemove(INTERACTIVE_OBJ * io, const char * name = NULL);
void InterTreeViewItemAdd(INTERACTIVE_OBJ * io, const char * name = NULL, long type = 0);
void SetWindowTitle(HWND hWnd, const char * tex);
void KillInterTreeView();
INT_PTR CALLBACK PathwayOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM);
INT_PTR CALLBACK StartProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM);
INT_PTR CALLBACK OptionsProc_2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ChangeLevelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK OptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FogOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IOOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LanguageOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PrecalcProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MeshReductionProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ScriptSearchProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void LaunchSnapShotParamApp(HWND hwnd);

enum LevelNumber {
	LEVEL0     = 0,
	LEVEL1     = 1,
	LEVEL2     = 2,
	LEVEL3     = 3,
	LEVEL4     = 4,
	LEVEL5     = 5,
	LEVEL6     = 6,
	LEVEL7     = 7,
	LEVEL8     = 8,
	LEVEL9     = 9,
	LEVEL10    = 10,
	LEVEL11    = 11,
	LEVEL12    = 12,
	LEVEL13    = 13,
	LEVEL14    = 14,
	LEVEL15    = 15,
	LEVEL16    = 16,
	LEVEL17    = 17,
	LEVEL18    = 18,
	LEVEL19    = 19,
	LEVEL20    = 20,
	LEVEL21    = 21,
	LEVEL22    = 22,
	LEVEL23    = 23,
	LEVEL24    = 24,
	LEVEL25    = 25,
	LEVEL26    = 26,
	LEVEL27    = 27,
	LEVELDEMO  = 28,
	LEVELDEMO2 = 29,
	LEVELDEMO3 = 30,
	LEVELDEMO4 = 31,
	NOLEVEL    = 32
};
 
void TextBox(const char * title, char * text, long size);
void launchlightdialog();

#endif // ARX_CORE_DIALOG_H
