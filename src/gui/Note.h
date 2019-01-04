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

#ifndef ARX_GUI_NOTE_H
#define ARX_GUI_NOTE_H

#include <stddef.h>
#include <string>
#include <vector>

#include "math/Vector.h"
#include "math/Rectangle.h"

class TextureContainer;

struct Note {
	
	enum Type {
		Undefined,
		Notice,
		SmallNote,
		BigNote,
		Book,
		QuestBook
	};
	
	Note()
		: m_type(Undefined)
		, m_currentRatio(0.f)
		, m_currentScale(0.f)
		, m_currentFontSize(0.f)
		, m_currentFontWeight(0)
		, m_page(0)
		, m_pageSpacing(20)
		, m_maxPages(1)
		, m_background(NULL)
		, m_prevPage(NULL)
		, m_nextPage(NULL)
	{}
	
	void setData(Type type, const std::string & text);
	void clear();
	
	bool isOpen() { return m_type != Undefined; }
	
	void render();
	bool manageActions();
	
	const Type & type() { return m_type; }
	const std::string & text() { return m_text; }
	
	size_t page() const { return m_page; }
	size_t pageCount() const { return m_pages.size(); }
	void setPage(size_t page);
	
	const Rectf & area() const { return m_area; }
	Rectf prevPageButton() const { return m_page > 0 ? m_prevPageButton : Rectf::ZERO; }
	Rectf nextPageButton() const { return m_page + 2 < m_pages.size() ? m_nextPageButton : Rectf::ZERO; }
	
private:
	
	//! Allocate note textures and split text into pages.
	bool allocate();
	void deallocate();
	
	void loadTextures();
	void calculateLayout();
	bool splitTextToPages();
	
	Type m_type;
	std::string m_text;
	
	Vec2f m_currentRatio;
	float m_currentScale;
	float m_currentFontSize;
	int m_currentFontWeight;
	
	std::vector<std::string> m_pages;
	size_t m_page;
	
	Rectf m_area;
	Rect m_textArea;
	Rectf m_prevPageButton;
	Rectf m_nextPageButton;
	s32 m_pageSpacing;

	size_t m_maxPages;
	
	TextureContainer * m_background;
	TextureContainer * m_prevPage;
	TextureContainer * m_nextPage;
	
};
#endif // ARX_GUI_NOTE_H
