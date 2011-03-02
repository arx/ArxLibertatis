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

#include <climits>
#include <string>

#include "gui/TextManager.h"
#include "core/Application.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/font/FontCache.h"

extern TextManager * pTextManage;
extern Font* hFontMainMenu;
extern Font* hFontMenu;
extern Font* hFontControls;
extern Font* hFontCredits;
extern Font* hFontInBook;
extern Font* hFontRedist;
extern Font* hFontInGame;
extern Font* hFontInGameNote;

void ARX_TEXT_Draw(Font* ef, float x, float y, const std::string & car, COLORREF colo, COLORREF bcol = 0x00FF00FF);
long ARX_TEXT_DrawRect(Font* ef, float x, float y, float maxx, const std::string& car, COLORREF colo, HRGN hRgn = NULL, COLORREF bcol = 0x00FF00FF);
float DrawBookTextInRect(Font* font, float x, float y, float maxx, const std::string& text, COLORREF col, COLORREF col2);
void DrawBookTextCenter(Font* font, float x, float y, const std::string& text, COLORREF col, COLORREF col2);
long UNICODE_ARXDrawTextCenter(Font* font, float x, float y, const std::string& str, COLORREF col, COLORREF bcol);
 
long UNICODE_ARXDrawTextCenteredScroll(Font* font, float x, float y, float x2, const std::string& str, COLORREF col, COLORREF bcol, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut = INT_MAX);
long ARX_UNICODE_ForceFormattingInRect(Font* _hFont, const std::string& _lpszUText, int _iSpacingY, RECT _rRect);
long ARX_UNICODE_DrawTextInRect(Font* font,
                                float x, float y,
								float maxx,
								const std::string& _lpszUText,
								COLORREF col, COLORREF bcol,
								HRGN hRgn = NULL);

void ARX_Allocate_Text( std::string& dest, const std::string& id_string);
std::string GetFontName( const std::string& );
void _ShowText( const char * text);
void ARX_Text_Init();
void ARX_Text_Close();
void FontRenderText(Font* _pFont, EERIE_3D pos, const std::string& _pText, COLORREF _c);

#endif
