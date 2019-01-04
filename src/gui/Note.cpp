/*
 * Copyright 2012-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/Note.h"

#include <limits>
#include <cctype>

#include "core/Core.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "gui/Cursor.h"
#include "gui/Interface.h"
#include "gui/Text.h"
#include "input/Input.h"
#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "scene/GameSound.h"

void Note::setData(Type type, const std::string & text) {
	
	deallocate();
	
	m_type = type;
	m_text = text;
	
	allocate();
}

void Note::clear() {
	deallocate();
	m_text.clear();
	m_type = Undefined;
}

void Note::deallocate() {
	
	// Don't bother actually deleting the textures, we'll just need them again!
	m_background = NULL, m_prevPage = NULL, m_nextPage = NULL;
	
	m_currentRatio = Vec2f(0.f);
	
	m_area = Rectf::ZERO;
	m_prevPageButton = Rectf::ZERO;
	m_nextPageButton = Rectf::ZERO;
	
	m_pages.clear();
}

void Note::loadTextures() {
	switch(m_type) {
		
		// TODO this information should not be hardcoded
		
		case Notice: {
			m_background = TextureContainer::LoadUI("graph/interface/book/notice");
			break;
		}
		
		case SmallNote: {
			m_background = TextureContainer::LoadUI("graph/interface/book/bignote");
			break;
		}
		
		case BigNote: {
			m_background = TextureContainer::LoadUI("graph/interface/book/very_bignote");
			break;
		}
		
		case Book: {
			m_background = TextureContainer::LoadUI("graph/interface/book/ingame_books");
			m_prevPage = TextureContainer::LoadUI("graph/interface/book/left_corner");
			m_nextPage = TextureContainer::LoadUI("graph/interface/book/right_corner");
			break;
		}
		
		case QuestBook: {
			m_background = TextureContainer::LoadUI("graph/interface/book/questbook");
			m_prevPage = TextureContainer::LoadUI("graph/interface/book/left_corner_original");
			m_nextPage = TextureContainer::LoadUI("graph/interface/book/right_corner_original");
			break;
		}
		
		case Undefined: arx_unreachable(); // Cannot handle notes of undefined type.
	}
}

void Note::calculateLayout() {
	
	Vec2f newPos;
	Vec2f newTextStart;
	Vec2f newTextEnd;
	
	Vec2f prevButtonOffset(0.f);
	Vec2f nextButtonOffset(0.f);
	
	Vec2f scale = Vec2f(g_playerBook.getScale());
	
	float noteY = 0.0f;
	if(m_type != QuestBook) {
		noteY = (float(m_background->m_size.y) * minSizeRatio() / 2.f + 47.f * minSizeRatio())
		        - float(m_background->m_size.y) * scale.y / 2.0f;
	}
	
	switch(m_type) {
		
		// TODO this information should not be hardcoded
		
		case Notice: {
			newPos = Vec2f(320 * g_sizeRatio.x - m_background->m_size.x * 0.5f * scale.x, noteY);
			newTextStart = Vec2f(50.f, 50.f);
			newTextEnd = Vec2f(m_background->size()) - Vec2f(50.f, 50.f);
			m_maxPages = 1;
			break;
		}
		
		case SmallNote: {
			newPos = Vec2f(320 * g_sizeRatio.x - m_background->m_size.x * 0.5f * scale.x, noteY);
			newTextStart = Vec2f(30.f, 30.f);
			newTextEnd = Vec2f(m_background->size()) - Vec2f(30.f, 40.f);
			m_maxPages = 1;
			break;
		}
		
		case BigNote: {
			newPos = Vec2f(320 * g_sizeRatio.x - m_background->m_size.x * 0.5f * scale.x, noteY);
			newTextStart = Vec2f(40.f, 40.f);
			newTextEnd = Vec2f(m_background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 40.f);
			m_maxPages = 2;
			break;
		}
		
		case Book: {
			newPos = Vec2f(320 * g_sizeRatio.x - m_background->m_size.x * 0.5f * scale.x, noteY);
			newTextStart = Vec2f(40.f, 20.f);
			newTextEnd = Vec2f(m_background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 40.f);
			m_maxPages = std::numeric_limits<size_t>::max();
			prevButtonOffset = Vec2f(8.f, -6.f);
			nextButtonOffset = Vec2f(-15.f, -6.f);
			break;
		}
		
		case QuestBook: {
			newPos = g_playerBook.getArea().topLeft();
			newTextStart = Vec2f(40.f, 30.f);
			newTextEnd = Vec2f(m_background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 45.f);
			m_maxPages = std::numeric_limits<size_t>::max();
			prevButtonOffset = Vec2f(8.f, -6.f);
			nextButtonOffset = Vec2f(-15.f, -6.f);
			break;
		}
		
		case Undefined: arx_unreachable(); // Cannot handle notes of undefined type.
	}
	
	if(m_type == QuestBook) {
		m_area = g_playerBook.getArea();
	} else {
		m_area = Rectf(newPos, float(m_background->m_size.x) * scale.x, float(m_background->m_size.y) * scale.y);
	}
	m_textArea = Rect(Vec2i(newTextStart * scale), Vec2i(newTextEnd * scale));
	m_pageSpacing = s32(20 * scale.x);
	if(m_prevPage) {
		Vec2f pos = Vec2f(0.f, m_background->m_size.y - m_prevPage->m_size.y) + prevButtonOffset;
		pos *= scale;
		m_prevPageButton = Rectf(newPos + pos, m_prevPage->m_size.x * scale.x, m_prevPage->m_size.y * scale.y);
	}
	if(m_nextPage) {
		Vec2f pos = Vec2f(m_background->size() - m_nextPage->size()) + nextButtonOffset;
		pos *= scale;
		m_nextPageButton = Rectf(newPos + pos, m_nextPage->m_size.x * scale.x, m_nextPage->m_size.y * scale.y);
	}
}

bool Note::splitTextToPages() {
	
	std::string::const_iterator txtbegin = m_text.begin();
	std::string::const_iterator txtend = m_text.end();
	
	while(txtbegin != txtend) {
		
		// Change the note type if the text is too long.
		if(m_pages.size() >= m_maxPages) {
			switch(m_type) {
				case Notice: m_type = SmallNote; break;
				case SmallNote: m_type = BigNote; break;
				case BigNote: m_type = Book; break;
				default: arx_unreachable();
			}
			return false;
		}
		
		long pageSize = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, txtbegin, txtend, m_textArea, true);
		if(pageSize <= 0) {
			LogWarning << "Error splitting note text into pages";
			m_pages.push_back(std::string(txtbegin, txtend));
			break;
		}
		
		m_pages.push_back(std::string(txtbegin, txtbegin + pageSize));
		
		// Skip whitespace at the start of pages.
		while(pageSize < txtend - txtbegin && std::isspace(*(txtbegin + pageSize))) {
			pageSize++;
		}
		
		txtbegin += pageSize;
	}
	return true;
}

bool Note::allocate() {
	
	float bookScale = g_playerBook.getScale();
	
	if(m_currentRatio == g_sizeRatio && m_currentScale == bookScale
	   && m_currentFontSize == config.interface.fontSize && m_currentFontWeight == config.interface.fontWeight) {
		return m_background != NULL;
	}
	m_currentRatio = g_sizeRatio;
	m_currentScale = bookScale;
	m_currentFontSize = config.interface.fontSize;
	m_currentFontWeight = config.interface.fontWeight;
	
	ARX_Text_scaleNoteFont(bookScale * config.interface.fontSize, config.interface.fontWeight);
	
	do {
		deallocate();
		
		// Allocate textures and calculate sizes
		loadTextures();
		
		if(m_background) {
			calculateLayout();
		}
		
		if(!m_background) {
			return false;
		}
	} while(!splitTextToPages());
	
	// Clamp the current page to a valid page.
	setPage(m_page);
	
	return true;
}

void Note::setPage(size_t page) {
	m_page = m_pages.empty() ? 0 : std::min(m_pages.size() - 1, page);
	m_page &= ~1; // Only allow even pages!
}

void Note::render() {
	
	if(!allocate()) {
		return;
	}
	
	UseRenderState state(render2D());
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	float z = 0.000001f;
	
	if(m_background) {
		EERIEDrawBitmap(m_area, z, m_background, Color::white);
	}
	
	if(m_pages.empty()) {
		return;
	}
	
	arx_assert(m_page < m_pages.size());
	
	// Draw the "previous page" button
	Rectf prevRect = prevPageButton();
	if(!prevRect.empty()) {
		arx_assert(m_prevPage != NULL);
		EERIEDrawBitmap(prevRect, z, m_prevPage, Color::white);
	}
	
	// Draw the "next page" button
	Rectf nextRect = nextPageButton();
	if(!nextRect.empty()) {
		arx_assert(m_nextPage != NULL);
		EERIEDrawBitmap(nextRect, z, m_nextPage, Color::white);
	}
	
	Font * font = hFontInGameNote;
	
	// Draw the left page
	{
		ARX_UNICODE_DrawTextInRect(
			font,
			Vec2f(m_area.left + m_textArea.left, m_area.top + m_textArea.top),
			m_area.left + m_textArea.right,
			m_pages[m_page],
			Color::none
		);
	}
	
	// Draw the right page
	if(m_page + 1 < m_pages.size()) {
		ARX_UNICODE_DrawTextInRect(
			font,
			Vec2f(m_area.left + m_textArea.right + m_pageSpacing, m_area.top + m_textArea.top),
			m_area.left + m_textArea.right + m_pageSpacing + m_textArea.width(),
			m_pages[m_page + 1],
			Color::none
		);
	}
	
}

/*!
* Manage forward and backward buttons on notes and the quest book.
* \return true if the note was clicked
*/
bool Note::manageActions() {
	
	if(prevPageButton().contains(Vec2f(DANAEMouse))) {
		cursorSetInteraction();
		if(eeMouseUp1()) {
			ARX_SOUND_PlayInterface(g_snd.BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			arx_assert(page() >= 2);
			setPage(page() - 2);
		}
		
	} else if(nextPageButton().contains(Vec2f(DANAEMouse))) {
		cursorSetInteraction();
		if(eeMouseUp1()) {
			ARX_SOUND_PlayInterface(g_snd.BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			setPage(page() + 2);
		}
		
	} else if(area().contains(Vec2f(DANAEMouse))) {
		if((eeMouseDown1() && TRUE_PLAYER_MOUSELOOK_ON) || eeMouseDown2()) {
			return true;
		}
	}
	
	return false;
}
