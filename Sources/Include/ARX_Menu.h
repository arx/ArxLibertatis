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
// ARX_Menu
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Menu Management
//
// Updates: (date)		(person)	(update)
//			22/01/2001	Cyril		Created Basic functionalities
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_MENU_H
#define ARX_MENU_H

#include <tchar.h>
#include "eerietexture.h"
#include "ARX_input.h"

//-----------------------------------------------------------------------------
#define MAX_FLYOVER	32
typedef struct
{
	TextureContainer	* Background;
	TextureContainer	* BookBackground;
	TextureContainer	* pTexCredits;
	float				creditspos;
	float				creditstart;
	_TCHAR		*		flyover[MAX_FLYOVER];
	_TCHAR		*		str_cre_credits;
	// New Quest Buttons Strings
	_TCHAR		*		str_button_quickgen;
	_TCHAR		*		str_button_skin;
	_TCHAR		*		str_button_done;
} MENU_DYNAMIC_DATA;

// ARX_MENU_DATA contains all Menu-datas
typedef struct
{
	long				currentmode; 
	long				mainmenupos;
	long				optionspos;
	long				inputpos;
	long				custompos;
	MENU_DYNAMIC_DATA	* mda;
} ARX_MENU_DATA;
extern ARX_MENU_DATA ARXmenu;

typedef struct
{
	long				num;
	char				name[256];
	long				level;
	float				version;
	SYSTEMTIME			stime;
} SaveGame;

//-----------------------------------------------------------------------------
// Possible values for ARXmenu.currentmode
#define AMCM_OFF		0
#define AMCM_MAIN		1
#define AMCM_CREDITS	2
#define AMCM_NEWQUEST	3
#define AMCM_CDNOTFOUND	4

//-----------------------------------------------------------------------------
extern SaveGame * save_l;
extern long save_c;
extern long save_p;
extern char LOCAL_SAVENAME[64];

//-----------------------------------------------------------------------------
void ARX_Menu_Manage(LPDIRECT3DDEVICE7 m_pd3dDevice);
BOOL ARX_Menu_Render(LPDIRECT3DDEVICE7 m_pd3dDevice);
void ARX_MENU_Launch(LPDIRECT3DDEVICE7 m_pd3dDevice);
void ARX_MENU_Clicked_QUIT_GAME();
void ARX_Menu_Resources_Create(LPDIRECT3DDEVICE7 m_pd3dDevice);
void ARX_Menu_Resources_Release(bool _bNoSound = true);
void ARX_Menu_Release_Text(void * a);

//-----------------------------------------------------------------------------
void LoadSaveGame(const long & i);
void UpdateSaveGame(const long & i);
void CreateSaveGameList();
void FreeSaveGameList();

#endif
