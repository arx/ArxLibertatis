/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

void ARX_TEXT_Draw(Font * ef, float x, float y, const std::string & car, Color colo);
float DrawBookTextInRect(Font * font, float x, float y, float maxx, const std::string & text, Color col);
void DrawBookTextCenter(Font * font, float x, float y, const std::string & text, Color col);
long UNICODE_ARXDrawTextCenter(Font * font, float x, float y, const std::string & str, Color col);
 
long UNICODE_ARXDrawTextCenteredScroll(Font * font, float x, float y, float x2, const std::string & str, Color col, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut = INT_MAX);
long ARX_UNICODE_ForceFormattingInRect(Font * font, const std::string & text, const Rect & _rRect);
long ARX_UNICODE_DrawTextInRect(Font * font, float x, float y, float maxx, const std::string & text, Color col, const Rect * pClipRect = NULL);

bool ARX_Text_Init();
void ARX_Text_Close();

#endif // ARX_GUI_TEXT_H
