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


bool TextManager::AddText(Font * font, std::string && text, const Rect & bounds, Color color,
                          PlatformDuration displayTime, PlatformDuration scrollDelay,
                          float scrollSpeed, int maxLines) {
	
	if(text.empty()) {
		return false;
	}
	
	if(!font) {
		LogWarning << "Adding text with NULL font.";
		return false;
	}
	
	arx_assert(!bounds.empty());
	
	ManagedText & entry = m_entries.emplace_back();
	
	entry.font = font;
	entry.text = std::move(text);
	entry.bounds = bounds;
	entry.color = color;
	entry.scrollDelay = scrollDelay;
	entry.scrollPosition = 0.f;
	entry.scrollSpeed = scrollSpeed;
	entry.displayTime = displayTime;
	entry.clipRect = entry.bounds;
	
	if(maxLines) {
		entry.clipRect.bottom = entry.bounds.top + font->getTextSize(entry.text).height() * maxLines;
	}
	
	return true;
}

bool TextManager::AddText(Font * font, std::string && text, Vec2i pos, Color color) {
	Rect bounds;
	bounds.left = pos.x;
	bounds.top = pos.y;
	bounds.right = Rect::Limits::max();
	bounds.bottom = Rect::Limits::max();
	return AddText(font, std::move(text), bounds, color);
}

void TextManager::Update(PlatformDuration delta) {
	
	m_entries.erase(std::remove_if( m_entries.begin(), m_entries.end(), [](const ManagedText & text) {
		return text.displayTime < 0;
	}), m_entries.end());
	
	for(ManagedText & entry : m_entries ) {
		
		entry.displayTime -= delta;
		
		if(entry.scrollDelay >= 0) {
			entry.scrollDelay -= delta;
			continue;
		}
		
		if(entry.scrollPosition < float(entry.bounds.bottom - entry.clipRect.bottom)) {
			entry.scrollPosition += entry.scrollSpeed * toMs(delta);
			if(entry.scrollPosition >= entry.bounds.bottom - entry.clipRect.bottom) {
				entry.scrollPosition = static_cast<float>(entry.bounds.bottom - entry.clipRect.bottom);
			}
		}
		
	}
	
}

void TextManager::Render() {
	
	for(ManagedText & entry : m_entries ) {
		
		const Rect * clipRect = nullptr;
		if(entry.clipRect.right != Rect::Limits::max() || entry.clipRect.bottom != Rect::Limits::max()) {
			clipRect = &entry.clipRect;
		}
		
		float maxx;
		if(entry.bounds.right == Rect::Limits::max()) {
			maxx = std::numeric_limits<float>::infinity();
		} else {
			maxx = static_cast<float>(entry.bounds.right);
		}
		
		Vec2f pos(entry.bounds.left, entry.bounds.top - entry.scrollPosition);
		long height = ARX_UNICODE_DrawTextInRect(entry.font, pos, maxx, entry.text, entry.color, clipRect);
		
		entry.bounds.bottom = entry.bounds.top + height;
		
	}
	
}

void TextManager::Clear() {
	m_entries.clear();
}

bool TextManager::Empty() const {
	return m_entries.empty();
}
