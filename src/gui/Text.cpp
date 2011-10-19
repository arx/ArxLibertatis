/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "gui/Text.h"

#include <sstream>

#include "core/Localisation.h"
#include "core/Core.h"

#include "gui/Interface.h"
#include "gui/TextManager.h"

#include "graphics/Renderer.h"
#include "graphics/Math.h"
#include "graphics/font/FontCache.h"

#include "io/Filesystem.h"
#include "io/log/Logger.h"
#include "io/FilePath.h"

using std::string;

//-----------------------------------------------------------------------------
TextManager * pTextManage;
TextManager * pTextManageFlyingOver;

//-----------------------------------------------------------------------------
Font* hFontInBook	= NULL;
Font* hFontMainMenu = NULL;
Font* hFontMenu		= NULL;
Font* hFontControls = NULL;
Font* hFontCredits	= NULL;
Font* hFontInGame	= NULL;
Font* hFontInGameNote = NULL;

//-----------------------------------------------------------------------------
void ARX_UNICODE_FormattingInRect(Font* pFont, const std::string& text, const Rect & _rRect, Color col, long* textHeight = 0, long* numChars = 0, bool computeOnly = false)
{
	std::string::const_iterator itLastLineBreak = text.begin();
	std::string::const_iterator itLastWordBreak = text.begin();
	std::string::const_iterator it = text.begin();
	
	int maxLineWidth = (_rRect.right == Rect::Limits::max() ? std::numeric_limits<int>::max() : _rRect.width());
	arx_assert(maxLineWidth > 0);
	int penY = _rRect.top;

	if(textHeight)
		*textHeight = 0;

	if(numChars)
		*numChars = 0;

	// Ensure we can at least draw one line...
	if(penY + pFont->GetLineHeight() > _rRect.bottom)
		return;

	for(it = text.begin(); it != text.end(); ++it)
	{
		bool bDrawLine = false;

		// Line break ?
		if((*it == '\n') || (*it == '*'))
		{
			bDrawLine = true;
		}
		else
		{
			// Word break ?
			if((*it == ' ') || (*it == '\t'))
			{
				itLastWordBreak = it;
			}

			// Check length of string up to this point
			Vec2i size = pFont->GetTextSize(itLastLineBreak, it+1);
			if(size.x > maxLineWidth)	// Too long ?
			{
				bDrawLine = true;		// Draw a line from the last line break up to the last word break
				it = itLastWordBreak;
			}			
		}
		
		// If we have to draw a line 
		//  OR
		// This is the last character of the string
		if(bDrawLine || (it+1 == text.end()))
		{
			// Draw the line
			if(!computeOnly)
				pFont->Draw(_rRect.left, penY, itLastLineBreak, it+1, col);
			
			itLastLineBreak = it+1;

			penY += pFont->GetLineHeight();

			// Validate that the new line will fit inside the rect...
			if(penY + pFont->GetLineHeight() > _rRect.bottom)
				break;
		}
	}

	// Return text height
	if(textHeight)
		*textHeight = penY - _rRect.top;

	// Return num characters displayed
	if(numChars)
		*numChars = it - text.begin();
}

long ARX_UNICODE_ForceFormattingInRect(Font* pFont, const string & text, const Rect & _rRect) {
	long numChars;
	ARX_UNICODE_FormattingInRect(pFont, text, _rRect, Color::none, 0, &numChars, true);
	return numChars;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_DrawTextInRect(Font* font,
                                float x, float y,
                                float maxx,
                                const std::string& _text,
                                Color col,
                                const Rect * pClipRect
                               ) {
	
	Rect previousViewport;
	if(pClipRect) {
		previousViewport = GRenderer->GetViewport();
		GRenderer->SetViewport(*pClipRect); 
	}

	Rect rect((Rect::Num)x, (Rect::Num)y, (Rect::Num)maxx, Rect::Limits::max());
	if(maxx == std::numeric_limits<float>::infinity()) {
		rect.right = Rect::Limits::max();
	}

	long height;
	ARX_UNICODE_FormattingInRect(font, _text, rect, col, &height);

	if(pClipRect) {
		GRenderer->SetViewport(previousViewport);
	}

	return height;
}

void ARX_TEXT_Draw(Font* ef,
                   float x, float y,
                   const std::string& car,
                   Color col) {
	
	if (car.empty() || car[0] == 0)
		return;

	ARX_UNICODE_DrawTextInRect(ef, x, y, std::numeric_limits<float>::infinity(), car, col);
}

long ARX_TEXT_DrawRect(Font* ef,
                       float x, float y,
                       float maxx,
                       const string & car,
                       Color col,
                       const Rect * pClipRect) {
	return ARX_UNICODE_DrawTextInRect(ef, x, y, maxx, car, col, pClipRect);
}

float DrawBookTextInRect(Font* font, float x, float y, float maxx, const std::string& text, Color col) {
	return (float)ARX_TEXT_DrawRect(font, (BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, (BOOKDECX + maxx) * Xratio, text, col);
}

//-----------------------------------------------------------------------------
void DrawBookTextCenter( Font* font, float x, float y, const std::string& text, Color col )
{
	UNICODE_ARXDrawTextCenter(font, (BOOKDECX + x)*Xratio, (BOOKDECY + y)*Yratio, text, col);
}

//-----------------------------------------------------------------------------

long UNICODE_ARXDrawTextCenter( Font* font, float x, float y, const std::string& str, Color col )
{
	Vec2i size = font->GetTextSize(str);
	int drawX = ((int)x) - (size.x / 2);
	int drawY = (int)y;

	font->Draw(drawX, drawY, str, col);

	return size.x;
}



long UNICODE_ARXDrawTextCenteredScroll( Font* font, float x, float y, float x2, const std::string& str, Color col, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut) {
	
	Rect::Num _x = checked_range_cast<Rect::Num>(x - x2);
	Rect::Num _y = checked_range_cast<Rect::Num>(y);
	Rect::Num w = checked_range_cast<Rect::Num>(x + x2);
	
	Rect rRect(_x, _y, w, Rect::Limits::max());
	
	if (pTextManage)
	{
		pTextManage->AddText(font,
							 str,
							 rRect,
							 col,
							 iTimeOut,
							 iTimeScroll,
							 fSpeed,
							 iNbLigne
							);

		return static_cast<long>(x2);
	}

	return 0;
}

static Font * _CreateFont(const string & fontFace, const string & fontProfileName, unsigned int fontSize, float scaleFactor = Yratio) {
	
	std::stringstream ss;

	std::string szFontSize;
	ss << fontSize;
	ss >> szFontSize;
	ss.clear();

	std::string szUT;
	szUT = getLocalised(fontProfileName, szFontSize);
	ss << szUT;
	ss >> fontSize;
	ss.clear();

	fontSize *= scaleFactor;

	Font* newFont = FontCache::GetFont(fontFace, fontSize);
	if(!newFont) {
		LogError << "error loading font: " << fontFace << " of size " << fontSize;
	}
	
	return newFont;
}

bool ARX_Text_Init() {
	
	ARX_Text_Close();
	
	string fontFile = "misc/arx.ttf";
	if(!fs::exists(fontFile)) {
		fontFile = "misc/arx_default.ttf"; // Full path
		if(!fs::exists(fontFile)) {
			LogError << "missing font file: need either misc/arx.ttf or misc/arx_default.ttf";
			return false;
		}
	}
	
	pTextManage = new TextManager();
	pTextManageFlyingOver = new TextManager();

	FontCache::Initialize();

	hFontMainMenu = _CreateFont(fontFile, "system_font_mainmenu_size", 58);
	LogDebug("Created hFontMainMenu, size " << hFontMainMenu->GetSize());

	hFontMenu = _CreateFont(fontFile, "system_font_menu_size", 32);
	LogDebug("Created hFontMenu, size " << hFontMenu->GetSize());

	hFontControls = _CreateFont(fontFile, "system_font_menucontrols_size", 22);
	LogDebug("Created hFontControls, size " << hFontControls->GetSize());

	hFontCredits = _CreateFont(fontFile, "system_font_menucredits_size", 36);
	LogDebug("Created hFontCredits, size " << hFontCredits->GetSize());

	// Keep small font small when increasing resolution
	float smallFontRatio = Yratio > 1.0f ? Yratio * 0.8f : Yratio;

	hFontInGame = _CreateFont(fontFile, "system_font_book_size", 18, smallFontRatio);
	LogDebug("Created hFontInGame, size " << hFontInGame->GetSize());

	hFontInGameNote = _CreateFont(fontFile, "system_font_note_size", 18, smallFontRatio);
	LogDebug("Created hFontInGameNote, size " << hFontInGameNote->GetSize());

	hFontInBook = _CreateFont(fontFile, "system_font_book_size", 18, smallFontRatio);
	LogDebug("Created InBookFont, size " << hFontInBook->GetSize());
	
	LogInfo << "Loaded font " << fontFile << " with sizes " << hFontMainMenu->GetSize() << ", " << hFontMenu->GetSize() << ", " << hFontControls->GetSize() << ", " << hFontCredits->GetSize() << ", " << hFontInGame->GetSize() << ", " << hFontInGameNote->GetSize() << ", " << hFontInBook->GetSize();
	
	return true;
}

//-----------------------------------------------------------------------------
void ARX_Text_Close() {

	delete pTextManage;
	pTextManage = NULL;

	delete pTextManageFlyingOver;
	pTextManageFlyingOver = NULL;

	FontCache::ReleaseFont(hFontInBook);
	hFontInBook = NULL;
	
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
