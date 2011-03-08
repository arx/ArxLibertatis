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

#include <windef.h>

class INTERACTIVE_OBJ;

#define IOTVTYPE_PATH 1
#define IOTVTYPE_PLAYER 3

extern long FASTLOADS;

void LaunchInteractiveObjectsApp(HWND hwnd);
void InterTreeViewItemRemove(INTERACTIVE_OBJ * io, const char * name = NULL);
void InterTreeViewItemAdd(INTERACTIVE_OBJ * io, const char * name = NULL, long type = 0);
void SetWindowTitle(HWND hWnd, const char * tex);
void KillInterTreeView();
HWND ShowErrorPopup(char * title, char * tex);
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

#define LEVEL0     0
#define LEVEL1     1
#define LEVEL2     2
#define LEVEL3     3
#define LEVEL4     4
#define LEVEL5     5
#define LEVEL6     6
#define LEVEL7     7
#define LEVEL8     8
#define LEVEL9     9
#define LEVEL10    10
#define LEVEL11    11
#define LEVEL12    12
#define LEVEL13    13
#define LEVEL14    14
#define LEVEL15    15
#define LEVEL16    16
#define LEVEL17    17
#define LEVEL18    18
#define LEVEL19    19
#define LEVEL20    20
#define LEVEL21    21
#define LEVEL22    22
#define LEVEL23    23
#define LEVEL24    24
#define LEVEL25    25
#define LEVEL26    26
#define LEVEL27    27
#define LEVELDEMO  28
#define LEVELDEMO2 29
#define LEVELDEMO3 30
#define LEVELDEMO4 31
#define NOLEVEL    32
 
void TextBox(const char * title, char * text, long size);
void launchlightdialog();

#endif // ARX_CORE_DIALOG_H
