/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GUI_MENU_H
#define ARX_GUI_MENU_H

#include <string>

class TextureContainer;

#define MAX_FLYOVER 32

struct MENU_DYNAMIC_DATA {
	
	TextureContainer * Background;
	TextureContainer * BookBackground;
	TextureContainer * pTexCredits;
	float creditspos;
	float creditstart;
	std::string flyover[MAX_FLYOVER];
	std::string credits;
	// New Quest Buttons Strings
	std::string str_button_quickgen;
	std::string str_button_skin;
	std::string str_button_done;
	
	MENU_DYNAMIC_DATA() : Background(NULL), BookBackground(NULL),
	  pTexCredits(NULL), creditspos(0), creditstart(0) { }
	
};

// Possible values for ARXmenu.currentmode
enum MenuMode {
	AMCM_OFF,
	AMCM_MAIN,
	AMCM_CREDITS,
	AMCM_NEWQUEST
};

// ARX_MENU_DATA contains all Menu-datas
struct ARX_MENU_DATA {
	MenuMode currentmode;
	MENU_DYNAMIC_DATA * mda;
};

extern ARX_MENU_DATA ARXmenu;

void ARX_Menu_Manage();
bool ARX_Menu_Render();
void ARX_MENU_Launch(bool allowResume);
void ARX_MENU_Clicked_QUIT_GAME();
void ARX_Menu_Resources_Create();
void ARX_Menu_Resources_Release(bool _bNoSound = true);
void ARX_MENU_Clicked_CREDITS();

#endif // ARX_GUI_MENU_H
