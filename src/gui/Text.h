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
// ARX_Text
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Text Management
//
// Updates: (date) (person) (update)
//
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_TEXT_H
#define ARX_TEXT_H

#include <string>
#include <vector>
#include "gui/TextManager.h"
#include "core/Application.h"
#include "graphics/GraphicsTypes.h"

extern TextManager * pTextManage;
extern HFONT hFontMainMenu;
extern HFONT hFontMenu;
extern HFONT hFontControls;
extern HFONT hFontCredits;
extern HFONT InBookFont;
extern HFONT hFontInGame;
extern HFONT hFontInGameNote;

long ARX_TEXT_Draw(HFONT ef, float x, float y, long spacingy, const std::string& car, COLORREF colo, COLORREF bcol = 0x00FF00FF);
long ARX_TEXT_DrawRect(HFONT ef, float x, float y, float maxx, float maxy, const std::string& car, COLORREF colo, HRGN hRgn = NULL, COLORREF bcol = 0x00FF00FF);
float DrawBookTextInRect(float x, float y, float maxx, float maxy, const std::string& text, COLORREF col, COLORREF col2, HFONT font);
void DrawBookTextCenter(float x, float y, const std::string& text, COLORREF col, COLORREF col2, HFONT font);
long UNICODE_ARXDrawTextCenter(float x, float y, const std::string& str, COLORREF col, COLORREF bcol, HFONT font);
 
long UNICODE_ARXDrawTextCenteredScroll(float x, float y, float x2, const std::string& str, COLORREF col, COLORREF bcol, HFONT font, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut = INT_MAX);
long ARX_UNICODE_ForceFormattingInRect(HFONT _hFont, const std::string& _lpszUText, int _iSpacingY, RECT _rRect);
long ARX_UNICODE_DrawTextInRect(float x, float y,
								float maxx,
								const std::string& _lpszUText,
								COLORREF col, COLORREF bcol,
								HFONT font,
								HRGN hRgn = NULL,
								HDC hHDC = NULL);

void ARX_Allocate_Text( std::string& dest, const std::string& id_string);
std::string GetFontName( const std::string& );
void _ShowText(char * text);
void ARX_Text_Init();
void ARX_Text_Close();
void FontRenderText(HFONT _hFont, EERIE_3D pos, const std::string& _pText, COLORREF _c);

#endif
