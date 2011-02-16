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
// Nuky - 10-02-11 - cleaned whole file
#ifndef ARX_TEXT_H
#define ARX_TEXT_H

#include <vector>
#include "EERIEApp.h"
#include "EERIETypes.h"
#include <tchar.h>

using std::vector;

//-----------------------------------------------------------------------------

void ARX_Text_Init();
void ARX_Text_Close();

//-----------------------------------------------------------------------------

class ARXText;

class CARXTextManager
{
public:
	CARXTextManager();
	~CARXTextManager();

public:
	void AddText(HFONT font, const _TCHAR* str, const RECT& rect, long fgcolor = -1, long bgcolor = 0, long timeout = 0, long scrolltime = 0, float scrollspeed = 0.f, int maxlines = 0);
	void AddText(HFONT font, const _TCHAR* str, long x, long y, long fgcolor);
	void Update(float diffFrame);
	void Render();
	void Clear();
	bool HasText() const;

private:
	std::vector<ARXText*> vText_;

	// No copy
	CARXTextManager(const CARXTextManager&);
	CARXTextManager& operator=(const CARXTextManager&);
};

//-----------------------------------------------------------------------------

void ARX_TEXT_Draw(HFONT ef, float x, float y, const _TCHAR* car, COLORREF colo, COLORREF bcol = 0x00FF00FF);
long ARX_TEXT_DrawRect(HFONT ef, float x, float y, float maxx, float maxy, const _TCHAR* car,
					   COLORREF colo, HRGN hRgn = NULL, COLORREF bcol = 0x00FF00FF);

void DrawBookTextInRect(HFONT font, float x, float y, float maxx, float maxy, const _TCHAR* text,
						COLORREF col, COLORREF col2);
void DrawBookTextCenter(HFONT font, float x, float y, const _TCHAR* text, COLORREF col, COLORREF col2);
		
void UNICODE_ARXDrawTextCenter(HFONT font, float x, float y, const _TCHAR* str, COLORREF col, COLORREF bcol);
void UNICODE_ARXDrawTextCenteredScroll(HFONT font, float x, float y, float x2, const _TCHAR* str, COLORREF col,
									   COLORREF bcol, int iTimeScroll, float fSpeed,
									   int iNbLigne, int iTimeOut = INT_MAX);

long ARX_UNICODE_ForceFormattingInRect(HFONT _hFont, const _TCHAR* _lpszUText, int _iSpacingY, RECT _rRect);

void ARX_Allocate_Text(_TCHAR*& dest, const _TCHAR* id_string);
void _ShowText(const char* text);

//-----------------------------------------------------------------------------

extern CARXTextManager * pTextManage;

extern HFONT hFontMainMenu;
extern HFONT hFontMenu;
extern HFONT hFontControls;
extern HFONT hFontCredits;
extern HFONT hFontInBook;
extern HFONT hFontRedist;
extern HFONT hFontInGame;
extern HFONT hFontInGameNote;

#endif
