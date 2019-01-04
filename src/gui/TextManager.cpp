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

#include "gui/TextManager.h"

#include "gui/Text.h"
#include "graphics/Math.h"
#include "graphics/font/Font.h"
#include "io/log/Logger.h"

struct TextManager::ManagedText {
	
	Font * pFont;
	Rect rRect;
	Rect rRectClipp;
	std::string lpszUText;
	float fDeltaY;
	float fSpeedScrollY;
	Color lCol;
	PlatformDuration lTimeScroll;
	PlatformDuration lTimeOut;
	
};

TextManager::TextManager() { }

TextManager::~TextManager() {
	Clear();
}

bool TextManager::AddText(Font * font, const std::string & text,
                          const Rect & bbox, Color color,
                          PlatformDuration displayTime, PlatformDuration scrollTime,
                          float scrollSpeed, int nLineClipp) {
	
	if(text.empty()) {
		return false;
	}
	
	if(!font) {
		LogWarning << "Adding text with NULL font.";
		return false;
	}
	
	ManagedText * pArxText = new ManagedText();
	if(!pArxText) {
		return false;
	}
	
	arx_assert(!bbox.empty());
	
	pArxText->pFont = font;
	pArxText->lpszUText = text;
	pArxText->rRect = bbox;
	pArxText->lCol = color;
	pArxText->lTimeScroll = scrollTime;
	pArxText->fDeltaY = 0.f;
	pArxText->fSpeedScrollY = scrollSpeed;
	pArxText->lTimeOut = displayTime;
	pArxText->rRectClipp = pArxText->rRect;
	
	if(nLineClipp) {
		Vec2i sSize = font->getTextSize(pArxText->lpszUText);
		sSize.y *= nLineClipp;
		pArxText->rRectClipp.bottom = pArxText->rRect.top + sSize.y;
	}
	
	entries.push_back(pArxText);
	
	return true;
}

bool TextManager::AddText(Font * font, const std::string & text, Vec2i pos, Color color) {
	Rect bbox;
	bbox.left = pos.x;
	bbox.top = pos.y;
	bbox.right = Rect::Limits::max();
	bbox.bottom = Rect::Limits::max();
	return AddText(font, text, bbox, color);
}

void TextManager::Update(PlatformDuration _iDiffFrame) {
	
	std::vector<ManagedText *>::iterator itManage;
	for(itManage = entries.begin(); itManage != entries.end();) {
		
		ManagedText * pArxText = *itManage;
		
		if(pArxText->lTimeOut < 0) {
			delete pArxText;
			itManage = entries.erase(itManage);
			continue;
		}
		
		pArxText->lTimeOut -= _iDiffFrame;
		
		if(pArxText->lTimeScroll < 0
		   && pArxText->fDeltaY < float(pArxText->rRect.bottom - pArxText->rRectClipp.bottom)) {
			pArxText->fDeltaY += pArxText->fSpeedScrollY * toMs(_iDiffFrame);
			
			if(pArxText->fDeltaY >= (pArxText->rRect.bottom - pArxText->rRectClipp.bottom)) {
				pArxText->fDeltaY = static_cast<float>(pArxText->rRect.bottom - pArxText->rRectClipp.bottom);
			}
		} else {
			pArxText->lTimeScroll -= _iDiffFrame;
		}
		
		++itManage;
	}
	
}

void TextManager::Render() {
	std::vector<ManagedText *>::const_iterator itManage = entries.begin();
	for(; itManage != entries.end(); ++itManage) {
		
		ManagedText * pArxText = *itManage;
		
		Rect * pRectClip = NULL;
		if(pArxText->rRectClipp.right != Rect::Limits::max() || pArxText->rRectClipp.bottom != Rect::Limits::max()) {
			pRectClip = &pArxText->rRectClipp;
		}
		
		float maxx;
		if(pArxText->rRect.right == Rect::Limits::max()) {
			maxx = std::numeric_limits<float>::infinity();
		} else {
			maxx = static_cast<float>(pArxText->rRect.right);
		}

		long height = ARX_UNICODE_DrawTextInRect(pArxText->pFont,
		                                         Vec2f(pArxText->rRect.left, pArxText->rRect.top - pArxText->fDeltaY),
		                                         maxx,
		                                         pArxText->lpszUText, pArxText->lCol, pRectClip);
		
		pArxText->rRect.bottom = pArxText->rRect.top + height;
	}
}

void TextManager::Clear() {
	
	std::vector<ManagedText *>::iterator itManage;
	for(itManage = entries.begin(); itManage < entries.end(); ++itManage) {
		delete *itManage;
	}
	
	entries.clear();
}

bool TextManager::Empty() const {
	return entries.empty();
}
