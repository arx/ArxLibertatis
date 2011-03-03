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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "gui/Text.h"

#include <cassert>
#include <sstream>

#include "core/Localization.h"
#include "core/Core.h"

#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/Frame.h"
#include "graphics/effects/Fog.h"

#include "io/Filesystem.h"
#include "io/Logger.h"

using std::string;

//-----------------------------------------------------------------------------
TextManager * pTextManage;
TextManager * pTextManageFlyingOver;

//-----------------------------------------------------------------------------
Font* hFontInBook	= NULL;
Font* hFontRedist	= NULL;
Font* hFontMainMenu = NULL;
Font* hFontMenu		= NULL;
Font* hFontControls = NULL;
Font* hFontCredits	= NULL;
Font* hFontInGame	= NULL;
Font* hFontInGameNote = NULL;
 

extern long CHINESE_VERSION;


//-----------------------------------------------------------------------------
string FontError() {
	LPVOID lpMsgBuf;
	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER |
	    FORMAT_MESSAGE_FROM_SYSTEM |
	    FORMAT_MESSAGE_IGNORE_INSERTS,
	    NULL,
	    GetLastError(),
	    0, // Default language
	    (LPTSTR) &lpMsgBuf,
	    0,
	    NULL);
	return string("Font Error: ") + (LPCSTR)lpMsgBuf;
}
//-----------------------------------------------------------------------------


long ARX_UNICODE_ForceFormattingInRect(Font* _pFont, const std::string& _lpszUText, int _iSpacingY, RECT _rRect)
{
	int iTemp = 0;

	int			iLenght	= _lpszUText.length();
	int			iHeight	= 0;
	Vector2i	sSize;
	int			iOldTemp;
	bool		bWrite;

	sSize.x = sSize.y = 0 ;

	while (1)
	{
		bWrite			= true;
		int iLenghtCurr	= _rRect.left;
		iOldTemp		= iTemp;

		ARX_CHECK(iTemp < iLenght);

		for (; iTemp < iLenght ; iTemp++)
		{
			sSize = _pFont->GetCharSize(_lpszUText[iTemp]);

			if ((_lpszUText[iTemp] == '\n') || (_lpszUText[iTemp] == '*'))
			{
				iHeight		+= _iSpacingY + sSize.y;
				bWrite		 = false;
				_rRect.top	+= _iSpacingY + sSize.y;
				iTemp++;
				break;
			}


					iLenghtCurr	+= sSize.x;

					if (iLenghtCurr > _rRect.right)
					{
						iHeight += _iSpacingY + sSize.y;

						if (CHINESE_VERSION)
						{
							iTemp--;
						}
						else
						{
							while ((_lpszUText[iTemp] != ' ') && (iTemp > 0)) iTemp--;
						}

						bWrite		 = false;
				_rRect.top	+= _iSpacingY + sSize.y;
				iTemp++;
				break;
			}
		}

		if ((iTemp == iLenght) || ((_rRect.top + sSize.y) > _rRect.bottom))
		{
			break;
		}
	}

	return iTemp;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_FormattingInRect(Font* font, const std::string& text, int _iSpacingY, RECT & _rRect, COLORREF col)
{
	std::string txtOut = text;
	size_t	iLenght = txtOut.length();
	int iHeight = 0;
	Vector2i sSize;
	size_t iOldTemp;
	bool bWrite;
	sSize.x = sSize.y = 0;

	size_t iTemp = 0;

	while (1)
	{
		bWrite = true;
		int iLenghtCurr = _rRect.left;
		iOldTemp = iTemp;

		for (; iTemp < iLenght; iTemp++)
		{
			sSize = font->GetCharSize(txtOut[iTemp]);

			if ((txtOut[iTemp] == '\n') || (txtOut[iTemp] == '*'))
			{
				iHeight += _iSpacingY + sSize.y;
				txtOut[iTemp] = '\0';
				bWrite = false;

				font->Draw(_rRect.left, _rRect.top, &txtOut[iOldTemp], col);
				_rRect.top += _iSpacingY + sSize.y;
				iTemp++;
				break;
			}
			
			iLenghtCurr += sSize.x;

			if (iLenghtCurr > _rRect.right)
			{
				iHeight += _iSpacingY + sSize.y;

				if (CHINESE_VERSION)
				{
					char * ptexttemp = (char *)malloc(iTemp - iOldTemp + 1);
					strncpy(ptexttemp, &txtOut[iOldTemp], iTemp - iOldTemp);
					ptexttemp[iTemp-iOldTemp] = '\0';

					font->Draw(_rRect.left, _rRect.top, ptexttemp, col);
					free((void *)ptexttemp);
					ptexttemp = NULL;
					iTemp--;
				}
				else
				{
					while ((txtOut[iTemp] != ' ') && (iTemp > 0)) iTemp--;

					txtOut[iTemp] = '\0';

					font->Draw(_rRect.left, _rRect.top, &txtOut[iOldTemp], col);					
				}

				bWrite = false;
				_rRect.top += _iSpacingY + sSize.y;
				iTemp++;
				break;
			}


		}

		if (iTemp == iLenght) break;

		if (iTemp == iOldTemp) break;
	}

	if (bWrite)
	{
		iHeight += _iSpacingY + sSize.y;
		font->Draw(_rRect.left, _rRect.top, &txtOut[iOldTemp], col);
	}

	return iHeight;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_DrawTextInRect(Font* font,
                                float x, float y,
                                float maxx,
                                const std::string& _text,
                                COLORREF col,
                                COLORREF bcol,
                                HRGN hRgn
                               )
{
	if (hRgn)
	{
		// TODO-FONT: glScissor / IDirect3DDevice7::SetClipStatus
		//SelectClipRgn(hDC, hRgn);
	}

	if (bcol != 0x00FF00FF) // TODO-FONT: Transparent!?
	{
		// TODO-FONT: Draw background
		//SetBkMode(hDC, OPAQUE);
		//SetBkColor(hDC, bcol);
	}
	
	RECT rect;
	rect.top	= (long)y;
	rect.left	= (long)x;
	rect.right	= (long)maxx;
	// TODO-FONT: rect.bottom ?? maxy ??

	long n = ARX_UNICODE_FormattingInRect(font, _text, 0, rect, col);

	if (hRgn)
	{
		// TODO-FONT: Undo glScissor / IDirect3DDevice7::SetClipStatus
	}

	return n;
}

void ARX_TEXT_Draw(Font* ef,
                   float x, float y,
                   const std::string& car,
                   COLORREF colo, COLORREF bcol) {
	
	if (car.empty() || car[0] == 0)
		return;

	ARX_UNICODE_DrawTextInRect(ef, x, y, 9999.f, car, colo, bcol);
}

long ARX_TEXT_DrawRect(Font* ef,
                       float x, float y,
                       float maxx,
                       const string & car,
                       COLORREF colo,
                       HRGN _hRgn,
                       COLORREF bcol) {
	
	bcol = RGB((bcol >> 16) & 255, (bcol >> 8) & 255, (bcol) & 255);

	colo = RGB((colo >> 16) & 255, (colo >> 8) & 255, (colo) & 255);
	return ARX_UNICODE_DrawTextInRect(ef, x, y, maxx, car, colo, bcol, _hRgn);
}

float DrawBookTextInRect(Font* font, float x, float y, float maxx, const std::string& text, COLORREF col, COLORREF col2) {
	return (float)ARX_TEXT_DrawRect(font, (BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, (BOOKDECX + maxx) * Xratio, text, col, NULL, col2);
}

//-----------------------------------------------------------------------------
void DrawBookTextCenter( Font* font, float x, float y, const std::string& text, COLORREF col, COLORREF col2 )
{
	UNICODE_ARXDrawTextCenter(font, (BOOKDECX + x)*Xratio, (BOOKDECY + y)*Yratio, text, col, col2);
}

//-----------------------------------------------------------------------------

long UNICODE_ARXDrawTextCenter( Font* font, float x, float y, const std::string& str, COLORREF col, COLORREF bcol)
{
	if (bcol != 0x00FF00FF)
	{
		// TODO-FONT: Draw background
		//SetBkMode(hDC, OPAQUE);
		//SetBkColor(hDC, bcol);
	}

	Vector2i size = font->GetTextSize(str);
	int drawX = ((int)x) - (size.x / 2);
	int drawY = (int)y;

	font->Draw(drawX, drawY, str, col);

	return size.x;
}



long UNICODE_ARXDrawTextCenteredScroll( Font* font, float x, float y, float x2, const std::string& str, COLORREF col, COLORREF bcol, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut)
{
	RECT rRect;
	ARX_CHECK_LONG(y);
	ARX_CHECK_LONG(x + x2);   //IF OK, x - x2 cannot overflow
	rRect.left = ARX_CLEAN_WARN_CAST_LONG(x - x2);
	rRect.top = ARX_CLEAN_WARN_CAST_LONG(y);
	rRect.right = ARX_CLEAN_WARN_CAST_LONG(x + x2);


	if (pTextManage)
	{
		pTextManage->AddText(font,
							 str,
							 rRect,
							 col,
							 bcol,
							 iTimeOut,
							 iTimeScroll,
							 fSpeed,
							 iNbLigne
							);

		return ARX_CLEAN_WARN_CAST_LONG(x2);
	}

	return 0;
}

//-----------------------------------------------------------------------------
void ARX_Allocate_Text( std::string& dest, const std::string& id_string) {
	std::string output;
	PAK_UNICODE_GetPrivateProfileString(id_string, "default", output);
	dest = output;
}

//-----------------------------------------------------------------------------
struct _FONT_HEADER
{
	ULONG   ulVersion;
	USHORT  usNumTables;
	USHORT  usSearchRange;
	USHORT  usEntrySelector;
	USHORT  usRangeShift;
};

//-----------------------------------------------------------------------------
struct _FONT_TABLE_HEADER
{
	ULONG	ulTag;
	ULONG	ulCheckSum;
	ULONG	ulOffset;
	ULONG	ulLength;
};

//-----------------------------------------------------------------------------
struct _FONT_NAMING_HEADER
{
	USHORT	usFormat;
	USHORT	usNbNameRecords;
	USHORT	usOffsetStorage;	//(from start of table)
};

//-----------------------------------------------------------------------------
struct _FONT_NAMING_NAMERECORD
{
	USHORT	usPlatformID;
	USHORT	usPlatformSpecificEncodingID;
	USHORT	usLanguageID;
	USHORT	usNameID;
	USHORT	usStringLength;
	USHORT	usStringOffset;		//from start of storage area (in bytes)
};

ULONG LilEndianLong(ULONG ulValue) {
	return MAKELONG(
			MAKEWORD(HIBYTE(HIWORD(ulValue)), LOBYTE(HIWORD(ulValue))),
			MAKEWORD(HIBYTE(LOWORD(ulValue)), LOBYTE(LOWORD(ulValue)))
	);
}

USHORT LilEndianShort(USHORT ulValue) {
	return MAKEWORD(HIBYTE(ulValue), LOBYTE(ulValue));
}

int Traffic(int iFontSize)
{
	iFontSize = (int)(float)(iFontSize * Yratio);

	if (CHINESE_VERSION)
	{
		if (iFontSize < 14)
			iFontSize = 12;
		else if (iFontSize < 15)
			iFontSize = 14;
		else if (iFontSize < 18)
			iFontSize = 15;
		else if (iFontSize <= 29)
			iFontSize = 18;
		else
			iFontSize = 30;
	}

	return iFontSize;
}

Font* _CreateFont(std::string fontFace, std::string fontProfileName, unsigned int fontSize)
{
	std::stringstream ss;

	std::string szFontSize;
	ss << fontSize;
	ss >> szFontSize;
	ss.clear();

	std::string szUT;
	PAK_UNICODE_GetPrivateProfileString(fontProfileName, szFontSize, szUT);
	ss << szUT;
	ss >> fontSize;
	ss.clear();

	fontSize = Traffic(fontSize);

	Font* newFont = FontCache::GetFont(fontFace, fontSize);
	if(!newFont) {
		LogError << "error loading font: " << fontFace << " of size " << fontSize;
	}
	
	return newFont;
}

string getFontFile() {
	string tx= "misc" PATH_SEPERATOR_STR "Arx.ttf";
	if(!FileExist(tx.c_str())) {
		tx = "misc" PATH_SEPERATOR_STR "ARX_default.ttf"; // Full path
	}
	return tx;
}

//-----------------------------------------------------------------------------
void ARX_Text_Init()
{	
	ARX_Text_Close();

	Localisation_Init();
	
	std::string strInGameFont = getFontFile();
	std::string strInMenuFont = strInGameFont;

	pTextManage = new TextManager();
	pTextManageFlyingOver = new TextManager();

	FontCache::Initialize();

	hFontMainMenu = _CreateFont(strInMenuFont, "system_font_mainmenu_size", 58);
	LogInfo << "Created hFontMainMenu, size " << hFontMainMenu->GetSize();

	hFontMenu	  = _CreateFont(strInMenuFont, "system_font_menu_size", 32);
	LogInfo << "Created hFontMenu, size " << hFontMenu->GetSize();

	hFontControls = _CreateFont(strInMenuFont, "system_font_menucontrols_size", 22);
	LogInfo << "Created hFontControls, size " << hFontControls->GetSize();

	hFontCredits  = _CreateFont(strInMenuFont, "system_font_menucredits_size", 36);
	LogInfo << "Created hFontCredits, size " << hFontCredits->GetSize();

	hFontRedist   = _CreateFont(strInGameFont, "system_font_redist_size", 18);
	LogInfo << "Created hFontRedist, size " << hFontRedist->GetSize();

	// NEW QUEST
	if (Yratio > 1.f)
	{
		Yratio *= .8f;
	}

	hFontInGame     = _CreateFont(strInGameFont, "system_font_book_size", 18);
	LogInfo << "Created hFontInGame, size " << hFontInGame->GetSize();

	hFontInGameNote = _CreateFont(strInGameFont, "system_font_note_size", 18);
	LogInfo << "Created hFontInGameNote, size " << hFontInGameNote->GetSize();

	hFontInBook		= _CreateFont(strInGameFont, "system_font_book_size", 18);
	LogInfo << "Created InBookFont, size " << hFontInBook->GetSize();
}

//-----------------------------------------------------------------------------
void ARX_Text_Close()
{
	Localisation_Close();

	delete pTextManage;
	pTextManage = NULL;

	delete pTextManageFlyingOver;
	pTextManageFlyingOver = NULL;

	FontCache::ReleaseFont(hFontInBook);
	hFontInBook = NULL;
	
	FontCache::ReleaseFont(hFontRedist);
	hFontRedist = NULL;
	
	FontCache::ReleaseFont(hFontMainMenu);
	hFontMainMenu = NULL;

	FontCache::ReleaseFont(hFontMenu);
	hFontMenu = NULL;
	
	FontCache::ReleaseFont(hFontControls);
	hFontControls = NULL;
	
	FontCache::ReleaseFont(hFontCredits);
	hFontCredits = NULL;
	
	FontCache::ReleaseFont(hFontInGame);
	hFontInGame = NULL;
	
	FontCache::ReleaseFont(hFontInGameNote);
	hFontInGameNote = NULL;

	FontCache::Shutdown();
}
