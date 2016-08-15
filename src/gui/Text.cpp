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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#include "gui/Text.h"

#include <sstream>

#include <boost/lexical_cast.hpp>

#include "core/Localisation.h"
#include "core/Config.h"
#include "core/Core.h"

#include "gui/Interface.h"
#include "gui/TextManager.h"

#include "graphics/Renderer.h"
#include "graphics/Math.h"
#include "graphics/font/Font.h"
#include "graphics/font/FontCache.h"

#include "io/resource/PakReader.h"
#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "util/Unicode.h"

TextManager * pTextManage = NULL;
TextManager * pTextManageFlyingOver = NULL;

Font * hFontInBook = NULL;
Font * hFontMainMenu = NULL;
Font * hFontMenu = NULL;
Font * hFontControls = NULL;
Font * hFontCredits = NULL;
Font * hFontInGame = NULL;
Font * hFontInGameNote = NULL;
Font * hFontDebug = NULL;

static void ARX_UNICODE_FormattingInRect(Font * font, const std::string & text,
                                         const Rect & rect, Color col,
                                         long * textHeight = 0, long * numChars = 0,
                                         bool computeOnly = false) {
	
	std::string::const_iterator itLastLineBreak = text.begin();
	std::string::const_iterator itLastWordBreak = text.begin();
	std::string::const_iterator it = text.begin();
	
	int maxLineWidth;
	if(rect.right == Rect::Limits::max()) {
		maxLineWidth = std::numeric_limits<int>::max();
	} else {
		maxLineWidth = rect.width();
	}
	arx_assert(maxLineWidth > 0);
	int penY = rect.top;
	
	if(textHeight) {
		*textHeight = 0;
	}
	
	if(numChars) {
		*numChars = 0;
	}
	
	// Ensure we can at least draw one line...
	if(penY + font->getLineHeight() > rect.bottom) {
		return;
	}
	
	std::string::const_iterator next = it;
	for(it = text.begin(); it != text.end(); it = next) {
		
		next = util::UTF8::next(it, text.end());
		
		// Line break ?
		bool isLineBreak = false;
		if(*it == '\n' || *it == '*') {
			isLineBreak = true;
		} else {
			
			// Word break ?
			if(*it == ' ' || *it == '\t') {
				itLastWordBreak = it;
			}
			
			// Check length of string up to this point
			Vec2i size = font->getTextSize(itLastLineBreak, next);
			if(size.x > maxLineWidth) { // Too long ?
				isLineBreak = true;
				if(itLastWordBreak > itLastLineBreak) {
					// Draw a line from the last line break up to the last word break
					it = itLastWordBreak;
					next = util::UTF8::next(it, text.end());
				} else if(it == itLastLineBreak) {
					// Not enough space to render even one character!
					break;
				} else {
					// The current word is too long to fit on a line, force a line break
					// Don't skip a character after the forced line break.
					next = it;
				}
			}
		}
		
		// If we have to draw a line
		//  OR
		// This is the last character of the string
		if(isLineBreak || next == text.end()) {
			
			std::string::const_iterator itTextStart = itLastLineBreak;
			std::string::const_iterator itTextEnd;
			
			itTextEnd = (isLineBreak) ? it : next;
			
			// Draw the line
			if(!computeOnly) {
				font->draw(rect.left, penY, itTextStart, itTextEnd, col);
			}
			
			itLastLineBreak = next;
			
			penY += font->getLineHeight();
			
			// Validate that the new line will fit inside the rect...
			if(penY + font->getLineHeight() > rect.bottom) {
				break;
			}
		}
	}
	
	// Return text height
	if(textHeight) {
		*textHeight = penY - rect.top;
	}
	
	// Return num characters displayed
	if(numChars) {
		*numChars = it - text.begin();
	}
}

long ARX_UNICODE_ForceFormattingInRect(Font * font, const std::string & text,
                                       const Rect & rect) {
	long numChars;
	ARX_UNICODE_FormattingInRect(font, text, rect, Color::none, 0, &numChars, true);
	return numChars;
}

long ARX_UNICODE_DrawTextInRect(Font* font,
                                const Vec2f & pos,
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

	Rect rect((Rect::Num)pos.x, (Rect::Num)pos.y, (Rect::Num)maxx, Rect::Limits::max());
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


void UNICODE_ARXDrawTextCenter(Font* font, const Vec2f & pos, const std::string& str, Color col) {
	
	Vec2i size = font->getTextSize(str);
	font->draw(Vec2i(pos.x - (size.x / 2), pos.y), str, col);
}

void UNICODE_ARXDrawTextCenteredScroll(Font* font, float x, float y, float x2, const std::string& str, Color col, PlatformDuration iTimeScroll, float fSpeed, int iNbLigne, PlatformDuration iTimeOut) {
	
	Rect::Num _x = checked_range_cast<Rect::Num>(x - x2);
	Rect::Num _y = checked_range_cast<Rect::Num>(y);
	Rect::Num w = checked_range_cast<Rect::Num>(x + x2);
	
	Rect rRect(_x, _y, w, Rect::Limits::max());
	
	if(pTextManage) {
		pTextManage->AddText(font,
							 str,
							 rRect,
							 col,
							 iTimeOut,
							 iTimeScroll,
							 fSpeed,
							 iNbLigne
							);
	}
}

static Font * createFont(const res::path & fontFace,
                         const std::string & configSizeKey, unsigned int fontSize,
                         float scaleFactor) {

	arx_assert(fontSize > 0);
	arx_assert(scaleFactor > 0.f);
	arx_assert(scaleFactor < 1000.f); // TODO better maximum
	
	try {
		std::string szFontSize = boost::lexical_cast<std::string>(fontSize);
		std::string configSize = getLocalised(configSizeKey, szFontSize);
		fontSize = boost::lexical_cast<unsigned int>(configSize);
	} catch(const boost::bad_lexical_cast &) {
		LogError << "Invalid font size for: " << configSizeKey;
	}
	
	fontSize *= scaleFactor;

	Font * newFont = FontCache::getFont(fontFace, fontSize);
	if(!newFont) {
		LogError << "Error loading font: " << fontFace << " of size " << fontSize;
	}
	
	return newFont;
}

static float created_font_scale = 0.f;

static bool getFontFile(res::path & result) {
	
	if(!config.language.empty()) {
		result = "misc/arx_" + config.language + ".ttf";
		if(resources->hasFile(result)) {
			return true;
		}
	}
	
	result = "misc/arx_default.ttf";
	if(resources->hasFile(result)) {
		return true;
	}
	
	result = "misc/arx.ttf";
	if(resources->hasFile(result)) {
		return true;
	}
	
	if(!config.language.empty()) {
		LogCritical << "Missing font file: need either misc/arx_" << config.language
		            << ".ttf, misc/arx_default.ttf or misc/arx.ttf.";
	} else {
		LogCritical << "Missing font file: need either misc/arx_default.ttf or misc/arx.ttf.";
	}
	return false;
}

bool ARX_Text_Init() {
	
	res::path file;
	if(!getFontFile(file)) {
		return false;
	}
	
	res::path debugFontFile = "misc/dejavusansmono.ttf";
	
	float scale = std::max(std::min(g_sizeRatio.y, g_sizeRatio.x), .001f);
	if(scale == created_font_scale) {
		return true;
	}
	created_font_scale = scale;
	
	// Keep small font small when increasing resolution
	// TODO font size jumps around scale = 1
	float small_scale = scale > 1.0f ? scale * 0.8f : scale;
	
	delete pTextManage;
	pTextManage = new TextManager();
	delete pTextManageFlyingOver;
	pTextManageFlyingOver = new TextManager();
	
	FontCache::initialize();
	
	Font * nFontMainMenu   = createFont(file, "system_font_mainmenu_size", 58, scale);
	Font * nFontMenu       = createFont(file, "system_font_menu_size", 32, scale);
	Font * nFontControls   = createFont(file, "system_font_menucontrols_size", 22, scale);
	Font * nFontCredits    = createFont(file, "system_font_menucredits_size", 36, scale);
	Font * nFontInGame     = createFont(file, "system_font_book_size", 18, small_scale);
	Font * nFontInGameNote = createFont(file, "system_font_note_size", 18, small_scale);
	Font * nFontInBook     = createFont(file, "system_font_book_size", 18, small_scale);
	Font * nFontDebug      = FontCache::getFont(debugFontFile, 14);
	
	// Only release old fonts after creating new ones to allow same fonts to be cached.
	FontCache::releaseFont(hFontMainMenu);
	FontCache::releaseFont(hFontMenu);
	FontCache::releaseFont(hFontControls);
	FontCache::releaseFont(hFontCredits);
	FontCache::releaseFont(hFontInGame);
	FontCache::releaseFont(hFontInGameNote);
	FontCache::releaseFont(hFontInBook);
	FontCache::releaseFont(hFontDebug);
	
	hFontMainMenu = nFontMainMenu;
	hFontMenu = nFontMenu;
	hFontControls = nFontControls;
	hFontCredits = nFontCredits;
	hFontInGame = nFontInGame;
	hFontInGameNote = nFontInGameNote;
	hFontInBook = nFontInBook;
	hFontDebug = nFontDebug;
	
	if(!hFontMainMenu
	   || !hFontMenu
	   || !hFontControls
	   || !hFontCredits
	   || !hFontInGame
	   || !hFontInGameNote
	   || !hFontInBook
	) {
		LogCritical << "Could not load font " << file << " for scale " << scale
		            << " / small scale " << small_scale;
		return false;
	}
	
	if(!hFontDebug) {
		LogCritical << "Could not load font " << debugFontFile;
		return false;
	}
	
	LogInfo << "Loaded font " << file << " with sizes " << hFontMainMenu->getSize()
			<< ", " << hFontMenu->getSize()
			<< ", " << hFontControls->getSize()
			<< ", " << hFontCredits->getSize()
			<< ", " << hFontInGame->getSize()
			<< ", " << hFontInGameNote->getSize()
			<< ", " << hFontInBook->getSize()
			<< ", " << hFontDebug->getSize();
	
	return true;
}

void ARX_Text_Close() {
	
	created_font_scale = 0.f;
	
	delete pTextManage;
	pTextManage = NULL;
	
	delete pTextManageFlyingOver;
	pTextManageFlyingOver = NULL;
	
	FontCache::releaseFont(hFontInBook);
	hFontInBook = NULL;
	
	FontCache::releaseFont(hFontMainMenu);
	hFontMainMenu = NULL;
	
	FontCache::releaseFont(hFontMenu);
	hFontMenu = NULL;
	
	FontCache::releaseFont(hFontControls);
	hFontControls = NULL;
	
	FontCache::releaseFont(hFontCredits);
	hFontCredits = NULL;
	
	FontCache::releaseFont(hFontInGame);
	hFontInGame = NULL;
	
	FontCache::releaseFont(hFontInGameNote);
	hFontInGameNote = NULL;

	FontCache::releaseFont(hFontDebug);
	hFontDebug = NULL;
	
	FontCache::shutdown();
}

void drawTextCentered(Font * font, Vec2f center, const std::string & text, Color color) {
	Vec2i size = font->getTextSize(text);
	Vec2f corner = center - Vec2f(size) / 2.f;
	font->draw(corner.x, corner.y, text, color);
}

void drawTextAt(Font * font, const Vec3f & pos, const std::string & text, Color color,
                const std::string & text2, Color color2) {
	
	// Project the 3d coordinates to get an on-screen position
	TexturedVertex out;
	EE_RTP(pos, out);
	if(out.p.z < 0.f) {
		// Don't draw text behind the camera!
		return;
	}
	
	Vec2f pos2d = Vec2f(out.p.x, out.p.y);
	drawTextCentered(font, pos2d, text, color);
	
	if(!text2.empty()) {
		pos2d.y += font->getLineHeight() + 2;
		drawTextCentered(font, pos2d, text2, color2);
	}
	
}
