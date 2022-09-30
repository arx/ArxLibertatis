/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <utility>

#include "gui/Text.h"
#include "graphics/Math.h"
#include "graphics/font/Font.h"
#include "io/log/Logger.h"


bool TextManager::AddText(Font * font, std::string && text, const Rect & bbox, Color color,
                          PlatformDuration displayTime, PlatformDuration scrollTime,
                          float scrollSpeed, int nLineClipp) {
	
	if(text.empty()) {
		return false;
	}
	
	if(!font) {
		LogWarning << "Adding text with NULL font.";
		return false;
	}

	arx_assert(!bbox.empty());
	
	ManagedText & entry = m_entries.emplace_back();
	
	entry.pFont = font;
	entry.lpszUText = std::move(text);
	entry.rRect = bbox;
	entry.lCol = color;
	entry.lTimeScroll = scrollTime;
	entry.fDeltaY = 0.f;
	entry.fSpeedScrollY = scrollSpeed;
	entry.lTimeOut = displayTime;
	entry.rRectClipp = entry.rRect;
	
	if(nLineClipp) {
		Vec2i sSize = font->getTextSize(entry.lpszUText);
		sSize.y *= nLineClipp;
		entry.rRectClipp.bottom = entry.rRect.top + sSize.y;
	}
	
	return true;
}

bool TextManager::AddText(Font * font, std::string && text, Vec2i pos, Color color) {
	Rect bbox;
	bbox.left = pos.x;
	bbox.top = pos.y;
	bbox.right = Rect::Limits::max();
	bbox.bottom = Rect::Limits::max();
	return AddText(font, std::move(text), bbox, color);
}

void TextManager::Update(PlatformDuration delta) {
	
	m_entries.erase(std::remove_if( m_entries.begin(), m_entries.end(), [](const ManagedText & text) {
		return text.lTimeOut < 0;
	}), m_entries.end());
	
	for(ManagedText & entry : m_entries ) {
		
		entry.lTimeOut -= delta;
		
		if(entry.lTimeScroll < 0 && entry.fDeltaY < float(entry.rRect.bottom - entry.rRectClipp.bottom)) {
			entry.fDeltaY += entry.fSpeedScrollY * toMs(delta);
			if(entry.fDeltaY >= (entry.rRect.bottom - entry.rRectClipp.bottom)) {
				entry.fDeltaY = static_cast<float>(entry.rRect.bottom - entry.rRectClipp.bottom);
			}
		} else {
			entry.lTimeScroll -= delta;
		}
		
	}
	
}

void TextManager::Render() {
	
	for(ManagedText & entry : m_entries ) {
		
		const Rect * pRectClip = nullptr;
		if(entry.rRectClipp.right != Rect::Limits::max() || entry.rRectClipp.bottom != Rect::Limits::max()) {
			pRectClip = &entry.rRectClipp;
		}
		
		float maxx;
		if(entry.rRect.right == Rect::Limits::max()) {
			maxx = std::numeric_limits<float>::infinity();
		} else {
			maxx = static_cast<float>(entry.rRect.right);
		}
		
		long height = ARX_UNICODE_DrawTextInRect(entry.pFont,
		                                         Vec2f(entry.rRect.left, entry.rRect.top - entry.fDeltaY),
		                                         maxx, entry.lpszUText, entry.lCol, pRectClip);
		
		entry.rRect.bottom = entry.rRect.top + height;
		
	}
	
}

void TextManager::Clear() {
	m_entries.clear();
}

bool TextManager::Empty() const {
	return m_entries.empty();
}
