/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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
	entry.bounds = Rectf(bounds);
	if(bounds.right == Rect::Limits::max()) {
		entry.bounds.right = std::numeric_limits<float>::infinity();
	}
	if(bounds.bottom == Rect::Limits::max()) {
		entry.bounds.bottom = std::numeric_limits<float>::infinity();
	}
	entry.color = color;
	entry.scrollDelay = scrollDelay;
	entry.scrollPosition = 0.f;
	entry.scrollSpeed = scrollSpeed;
	entry.displayTime = displayTime;
	entry.clipRect = bounds;
	
	if(maxLines) {
		entry.clipRect.bottom = entry.clipRect.top + font->getTextSize(entry.text).height() * maxLines;
	}
	
	return true;
}

void TextManager::Update(PlatformDuration delta) {
	
	arx_assume(delta >= 0);
	
	m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(), [](const ManagedText & text) {
		return text.displayTime < 0;
	}), m_entries.end());
	
	for(ManagedText & entry : m_entries) {
		
		entry.displayTime -= delta;
		
		if(entry.scrollDelay >= 0) {
			entry.scrollDelay -= delta;
			continue;
		}
		
		entry.scrollPosition = std::min(entry.scrollPosition + entry.scrollSpeed * toMsf(delta),
		                                entry.bounds.bottom - float(entry.clipRect.bottom));
		
	}
	
}

void TextManager::Render() {
	
	for(ManagedText & entry : m_entries) {
		
		const Rect * clipRect = nullptr;
		if(entry.clipRect.right != Rect::Limits::max() || entry.clipRect.bottom != Rect::Limits::max()) {
			clipRect = &entry.clipRect;
		}
		
		Vec2f pos = entry.bounds.topLeft() - Vec2f(0.f, entry.scrollPosition);
		long height = ARX_UNICODE_DrawTextInRect(entry.font, pos, entry.bounds.right, entry.text, entry.color,
		                                         clipRect);
		
		entry.bounds.bottom = entry.bounds.top + float(height);
		
	}
	
}

void TextManager::Clear() {
	m_entries.clear();
}

bool TextManager::Empty() const {
	return m_entries.empty();
}
