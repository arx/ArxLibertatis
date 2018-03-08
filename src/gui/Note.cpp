/*
 * Copyright 2012-2015 Arx Libertatis Team (see the AUTHORS file)
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
	_text = text;
	
	allocate();
}

void Note::clear() {
	deallocate();
	_text.clear();
	m_type = Undefined;
	m_isCleared = true;
}

void Note::deallocate() {
	
	// Don't bother actually deleting the textures, we'll just need them again!
	background = NULL, prevPage = NULL, nextPage = NULL;
	
	m_currentRatio = Vec2f_ZERO;
	
	_area = Rectf::ZERO;
	_prevPageButton = Rectf::ZERO;
	_nextPageButton = Rectf::ZERO;
	
	pages.clear();
}

void Note::loadTextures() {
	switch(m_type) {
		
		// TODO this information should not be hardcoded
		
		case Notice: {
			background = TextureContainer::LoadUI("graph/interface/book/notice");
			break;
		}
		
		case SmallNote: {
			background = TextureContainer::LoadUI("graph/interface/book/bignote");
			break;
		}
		
		case BigNote: {
			background = TextureContainer::LoadUI("graph/interface/book/very_bignote");
			break;
		}
		
		case Book: {
			background = TextureContainer::LoadUI("graph/interface/book/ingame_books");
			prevPage = TextureContainer::LoadUI("graph/interface/book/left_corner");
			nextPage = TextureContainer::LoadUI("graph/interface/book/right_corner");
			break;
		}
		
		case QuestBook: {
			background = TextureContainer::LoadUI("graph/interface/book/questbook");
			prevPage = TextureContainer::LoadUI("graph/interface/book/left_corner_original");
			nextPage = TextureContainer::LoadUI("graph/interface/book/right_corner_original");
		}
		
		case Undefined: break; // Cannot handle notes of undefined type.
	}
}

void Note::calculateLayout() {
	
	Vec2f newPos;
	Vec2f newTextStart;
	Vec2f newTextEnd;
	
	Vec2f prevButtonOffset;
	Vec2f nextButtonOffset;
	
	Vec2f scale = Vec2f(minSizeRatio(), minSizeRatio());
	
	if(m_type == QuestBook) {
		// TODO change this once the aspect ratio in character screen, spell book, etc. is fixed.
		scale = g_sizeRatio;
	}
	
	switch(m_type) {
		
		// TODO this information should not be hardcoded
		
		case Notice: {
			newPos = Vec2f(320 * g_sizeRatio.x - background->m_size.x * 0.5f * scale.x, 47.f * scale.y);
			newTextStart = Vec2f(50.f, 50.f);
			newTextEnd = Vec2f(background->size()) - Vec2f(50.f, 50.f);
			m_maxPages = 1;
			break;
		}
		
		case SmallNote: {
			newPos = Vec2f(320 * g_sizeRatio.x - background->m_size.x * 0.5f * scale.x, 47.f * scale.y);
			newTextStart = Vec2f(30.f, 30.f);
			newTextEnd = Vec2f(background->size()) - Vec2f(30.f, 40.f);
			m_maxPages = 1;
			break;
		}
		
		case BigNote: {
			newPos = Vec2f(320 * g_sizeRatio.x - background->m_size.x * 0.5f * scale.x, 47.f * scale.y);
			newTextStart = Vec2f(40.f, 40.f);
			newTextEnd = Vec2f(background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 40.f);
			m_maxPages = 2;
			break;
		}
		
		case Book: {
			newPos = Vec2f(320 * g_sizeRatio.x - background->m_size.x * 0.5f * scale.x, 47.f * scale.y);
			newTextStart = Vec2f(40.f, 20.f);
			newTextEnd = Vec2f(background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 40.f);
			m_maxPages = std::numeric_limits<size_t>::max();
			prevButtonOffset = Vec2f(8.f, -6.f);
			nextButtonOffset = Vec2f(-15.f, -6.f);
			break;
		}
		
		case QuestBook: {
			newPos = Vec2f(97, 64) * scale;
			newTextStart = Vec2f(40.f, 40.f);
			newTextEnd = Vec2f(background->size()) * Vec2f(0.5f, 1.f) - Vec2f(10.f, 65.f);
			m_maxPages = std::numeric_limits<size_t>::max();
			prevButtonOffset = Vec2f(8.f, -6.f);
			nextButtonOffset = Vec2f(-15.f, -6.f);
		}
		
		case Undefined: break; // Cannot handle notes of undefined type.
	}
	
	_area = Rectf(newPos, background->m_size.x * scale.x, background->m_size.y * scale.y);
	_textArea = Rect(Vec2i(newTextStart * scale), Vec2i(newTextEnd * scale));
	_pageSpacing = s32(20 * scale.x);
	if(prevPage) {
		Vec2f pos = Vec2f(0.f, background->m_size.y - prevPage->m_size.y) + prevButtonOffset;
		pos *= scale;
		_prevPageButton = Rectf(newPos + pos, prevPage->m_size.x * scale.x, prevPage->m_size.y * scale.y);
	}
	if(nextPage) {
		Vec2f pos = Vec2f(background->size() - nextPage->size()) + nextButtonOffset;
		pos *= scale;
		_nextPageButton = Rectf(newPos + pos, nextPage->m_size.x * scale.x, nextPage->m_size.y * scale.y);
	}
}

bool Note::splitTextToPages() {
	
	std::string::const_iterator txtbegin = _text.begin();
	std::string::const_iterator txtend = _text.end();
	
	while(txtbegin != txtend) {
		
		// Change the note type if the text is too long.
		if(pages.size() >= m_maxPages) {
			switch(m_type) {
				case Notice: m_type = SmallNote; break;
				case SmallNote: m_type = BigNote; break;
				case BigNote: m_type = Book; break;
				default: ARX_DEAD_CODE(); break;
			}
			return false;
		}
		
		long pageSize = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, txtbegin, txtend, _textArea);
		if(pageSize <= 0) {
			LogWarning << "Error splitting note text into pages";
			pages.push_back(std::string(txtbegin, txtend));
			break;
		}
		
		pages.push_back(std::string(txtbegin, txtbegin + pageSize));
		
		// Skip whitespace at the start of pages.
		while(pageSize < txtend - txtbegin && std::isspace(*(txtbegin + pageSize))) {
			pageSize++;
		}
		
		txtbegin += pageSize;
	}
	return true;
}

bool Note::allocate() {
	
	if(m_currentRatio == g_sizeRatio) {
		return background != NULL;
	}
	
	do {
		deallocate();
		
		// Allocate textures and calculate sizes
		loadTextures();
		
		if(background) {
			calculateLayout();
		}
		
		if(!background) {
			m_currentRatio = g_sizeRatio;
			return false;
		}
	} while(!splitTextToPages());
	
	// Clamp the current page to a valid page.
	setPage(m_page);
	
	m_currentRatio = g_sizeRatio;
	m_isCleared = false;
	
	return true;
}

void Note::setPage(size_t page) {
	m_page = pages.empty() ? 0 : std::min(pages.size() - 1, page);
	m_page &= ~1; // Only allow even pages!
}

void Note::render() {
	
	if(!allocate()) {
		return;
	}
	
	UseRenderState state(render2D());
	
	float z = 0.000001f;
	
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);
	
	if(background) {
		EERIEDrawBitmap(_area, z, background, Color::white);
	}
	
	if(pages.empty()) {
		return;
	}
	
	arx_assert(m_page < pages.size());
	
	// Draw the "previous page" button
	Rectf prevRect = prevPageButton();
	if(!prevRect.empty()) {
		arx_assert(prevPage != NULL);
		EERIEDrawBitmap(prevRect, z, prevPage, Color::white);
	}
	
	// Draw the "next page" button
	Rectf nextRect = nextPageButton();
	if(!nextRect.empty()) {
		arx_assert(nextPage != NULL);
		EERIEDrawBitmap(nextRect, z, nextPage, Color::white);
	}
	
	Font * font = hFontInGameNote;
	
	// Draw the left page
	{
		ARX_UNICODE_DrawTextInRect(
			font,
			Vec2f(_area.left + _textArea.left, _area.top + _textArea.top),
			_area.left + _textArea.right,
			pages[m_page],
			Color::none
		);
	}
	
	// Draw the right page
	if(m_page + 1 < pages.size()) {
		ARX_UNICODE_DrawTextInRect(
			font,
			Vec2f(_area.left + _textArea.right + _pageSpacing, _area.top + _textArea.top),
			_area.left + _textArea.right + _pageSpacing + _textArea.width(),
			pages[m_page + 1],
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
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			arx_assert(page() >= 2);
			setPage(page() - 2);
		}
		
	} else if(nextPageButton().contains(Vec2f(DANAEMouse))) {
		cursorSetInteraction();
		if(eeMouseUp1()) {
			ARX_SOUND_PlayInterface(SND_BOOK_PAGE_TURN, Random::getf(0.9f, 1.1f));
			setPage(page() + 2);
		}
		
	} else if(area().contains(Vec2f(DANAEMouse))) {
		if((eeMouseDown1() && TRUE_PLAYER_MOUSELOOK_ON) || eeMouseDown2()) {
			return true;
		}
	}
	
	return false;
}
