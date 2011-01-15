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

#ifndef DANAEDLG_H
#define DANAEDLG_H

#include "danae.h"
#define IOTVTYPE_PATH 1
#define IOTVTYPE_NODE 2
#define IOTVTYPE_PLAYER 3

#define _ARX_FINAL_VERSION_ " SC-1£ µ*[aAv|( $!1^;.|(2çà1çà]_______"

extern short	Cross;
extern long Bilinear;
extern long DEBUG1ST;
extern long DEBUGNPCMOVE;
extern long FASTLOADS;

extern HWND CDP_PATHWAYS_Options;
extern HWND CDP_IOOptions;

void LaunchInteractiveObjectsApp(HWND hwnd);
void InterTreeViewItemRemove(INTERACTIVE_OBJ * io, char * name = NULL, long type = 0);
void InterTreeViewItemAdd(INTERACTIVE_OBJ * io, char * name = NULL, long type = 0);
void SetWindowTitle(HWND hWnd, char * tex);
void KillInterTreeView();
HWND ShowErrorPopup(char * title, char * tex);
BOOL CALLBACK PathwayOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM);
BOOL CALLBACK StartProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM);
BOOL CALLBACK OptionsProc_2(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChangeLevelProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT  CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK OptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GaiaTextEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern COLORREF custcr[16];
BOOL CALLBACK FogOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK LightOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK IOOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
 
BOOL CALLBACK LanguageOptionsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PrecalcProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK PassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK IDDErrorLogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MeshReductionProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                LPARAM lParam);
BOOL CALLBACK ScriptSearchProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam);

void LaunchSnapShotParamApp(HWND hwnd);

#define LEVEL0		0
#define LEVEL1		1
#define LEVEL2		2
#define LEVEL3		3
#define LEVEL4		4
#define LEVEL5		5
#define LEVEL6		6
#define LEVEL7		7
#define LEVEL8		8
#define LEVEL9		9
#define LEVEL10		10
#define LEVEL11		11
#define LEVEL12		12
#define LEVEL13		13
#define LEVEL14		14
#define LEVEL15		15
#define LEVEL16		16
#define LEVEL17		17
#define LEVEL18		18
#define LEVEL19		19
#define LEVEL20		20
#define LEVEL21		21
#define LEVEL22		22
#define LEVEL23		23
#define LEVEL24		24
#define LEVEL25		25
#define LEVEL26		26
#define LEVEL27		27
#define LEVELDEMO	28

#define LEVELDEMO2	29
#define LEVELDEMO3	30
#define LEVELDEMO4	31
#define NOLEVEL		32

 
void TextBox(char * title, char * text, long size);
void launchlightdialog();

#endif
