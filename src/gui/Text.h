/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_TEXT_H
#define ARX_GUI_TEXT_H

#include <string>
#include <climits>

#include "core/TimeTypes.h"
#include "graphics/Color.h"
#include "math/Types.h"

class TextManager;
class Font;

extern TextManager * pTextManage;
extern Font * hFontMainMenu;
extern Font * hFontMenu;
extern Font * hFontControls;
extern Font * hFontCredits;
extern Font * hFontInBook;
extern Font * hFontInGame;
extern Font * hFontInGameNote;
extern Font * hFontDebug;

void UNICODE_ARXDrawTextCenter(Font * font, const Vec2f & pos, const std::string & str, Color col);
 
void UNICODE_ARXDrawTextCenteredScroll(Font * font, float x, float y,
                                       float x2, const std::string & str, Color col,
                                       PlatformDuration iTimeScroll, float fSpeed,
                                       int iNbLigne, PlatformDuration iTimeOut = PlatformDurationMs(INT_MAX));

long ARX_UNICODE_ForceFormattingInRect(Font * font, std::string::const_iterator txtbegin,
                                       std::string::const_iterator txtend, const Rect & rect,
                                       bool noOneLineParagraphs = false);
long ARX_UNICODE_DrawTextInRect(Font * font, const Vec2f & pos, float maxx, const std::string & text, Color col, const Rect * pClipRect = NULL);

bool ARX_Text_Init();
void ARX_Text_scaleBookFont(float scale, int weight);
void ARX_Text_scaleNoteFont(float scale, int weight);
void ARX_Text_Close();

/*!
 * Draw text centered (both x an y) at a given position.
 *
 * \param font   The font to use
 * \param center The position to center the text at
 * \param text   The text to draw
 * \param color  The text color to use
 */
void drawTextCentered(Font * font, Vec2f center, const std::string & text,
                      Color color = Color::white);

/*!
 * Draw text centered (both x an y) at a position in given in 3D space
 *
 * \param font   The font to use
 * \param pos    The position to draw the text at
 * \param text   The text to draw
 * \param color  The text color to use
 * \param text2  An optional secend line of text - this does not affect the position
 *               of the first line
 * \param color2 The text color to use for the second line
 */
void drawTextAt(Font * font, const Vec3f & pos, const std::string & text,
                Color color = Color::white, const std::string & text2 = std::string(),
                Color color2 = Color::white);

#endif // ARX_GUI_TEXT_H
