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

#include <tchar.h>
#include <list>
#include <vector>
#include "eerieapp.h"
#include "eerietypes.h"

using namespace std;

//-----------------------------------------------------------------------------
typedef enum _ARX_TEXT_TYPE
{
	ARX_TEXT_ONCE,
	ARX_TEXT_STAY
} ARX_TEXT_TYPE;

//-----------------------------------------------------------------------------
typedef struct _ARX_TEXT
{
	ARX_TEXT_TYPE	eType;
	HFONT			hFont;
	RECT			rRect;
	RECT			rRectClipp;
	_TCHAR		*	lpszUText;
	float			fDeltaY;
	float			fSpeedScrollY;
	long			lCol;
	long			lBkgCol;
	long			lTimeScroll;
	long			lTimeOut;
	long			lTailleLigne;
	int				iNbLineClip;
} ARX_TEXT;

//-----------------------------------------------------------------------------
void ARX_Text_Init(ARX_TEXT *);

//-----------------------------------------------------------------------------
class CARXTextManager
{
	public:
		vector<ARX_TEXT *>	vText;
	public:
		CARXTextManager();
		~CARXTextManager();
	public:
		bool AddText(HFONT, _TCHAR *, RECT &, long _lCol = -1, long _lBkgCol = 0, long _lTimeOut = 0, long _lTimeScroll = 0, float _fSpeedScroll = 0.f, int iNbLigneClipp = 0);
		bool AddText(ARX_TEXT *);
		void Update(float);
		void Render();
		void Clear();
};

//-----------------------------------------------------------------------------
extern CARXTextManager * pTextManage;
extern _TCHAR * lpszFontMenu;
extern _TCHAR * lpszFontIngame;

extern HFONT hFontMainMenu;
extern HFONT hFontMenu;
extern HFONT hFontControls;
extern HFONT hFontCredits;
extern HFONT InBookFont;
extern HFONT hFontRedist;
extern HFONT hFontInGame;
extern HFONT hFontInGameNote;

//-----------------------------------------------------------------------------
long ARX_TEXT_Draw(LPDIRECT3DDEVICE7 pd3dDevice, HFONT ef, float x, float y, long spacingx, long spacingy, _TCHAR * car, COLORREF colo, COLORREF bcol = 0x00FF00FF);
long ARX_TEXT_DrawRect(LPDIRECT3DDEVICE7 pd3dDevice, HFONT ef, float x, float y, long spacingx, long spacingy, float maxx, float maxy, _TCHAR * car, COLORREF colo, HRGN hRgn = NULL, COLORREF bcol = 0x00FF00FF, long flags = 0);
float DrawBookTextInRect(float x, float y, float maxx, float maxy, _TCHAR * text, COLORREF col, COLORREF col2, HFONT font);
void DrawBookTextCenter(float x, float y, _TCHAR * text, COLORREF col, COLORREF col2, HFONT font);
long UNICODE_ARXDrawTextCenter(float x, float y, _TCHAR * str, COLORREF col, COLORREF bcol, HFONT font);
 
long UNICODE_ARXDrawTextCenteredScroll(float x, float y, float x2, _TCHAR * str, COLORREF col, COLORREF bcol, HFONT font, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut = INT_MAX);
long ARX_UNICODE_ForceFormattingInRect(HFONT _hFont, _TCHAR * _lpszUText, int _iSpacingY, RECT _rRect);
long ARX_UNICODE_DrawTextInRect(float x, float y,
                                float maxx, float maxy,
                                _TCHAR * _lpszUText,
                                COLORREF col, COLORREF bcol,
                                HFONT font,
                                HRGN hRgn = NULL,
                                HDC hHDC = NULL);

void ARX_Allocate_Text(_TCHAR *&dest, _TCHAR * id_string);
_TCHAR * GetFontName(char *);
void _ShowText(char * text);

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileString(_TCHAR	*	sectionname,
                                     _TCHAR	*	t_keyname,
                                     _TCHAR	*	defaultstring,
                                     _TCHAR	*	destination,
                                     unsigned long	maxsize,
                                     _TCHAR	*	datastream,
                                     long			lastspeech);

//-----------------------------------------------------------------------------

void ARX_Text_Init();
void ARX_Text_Close();
void FontRenderText(HFONT _hFont, EERIE_3D pos, _TCHAR * _pText, COLORREF _c);

#endif
