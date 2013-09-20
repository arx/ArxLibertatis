/*
 * Copyright 2012 Arx Libertatis Team (see the AUTHORS file)
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
#include "NoteGuiData.h"

namespace gui {

NoteGuiData::NoteGuiData(const char* backgroundPath, const char* prevPagePath, const char* nextPagePath) 
	: m_background(0)
    , m_prevPage(0)
	, m_nextPage(0) 
{
	if(backgroundPath != 0) {
		this->m_background = TextureContainer::LoadUI(backgroundPath);
	}
	if(prevPagePath != 0) {
		this->m_prevPage = TextureContainer::LoadUI(prevPagePath);
	}
	if(nextPagePath != 0) {
		this->m_nextPage = TextureContainer::LoadUI(nextPagePath);
	}
}

TextureContainer* NoteGuiData::getBackground() const {
	return this->m_background;
}

TextureContainer* NoteGuiData::getPrevPage() const {
	return this->m_prevPage;
}

TextureContainer* NoteGuiData::getNextPage() const {
	return this->m_nextPage;
}

NoteGuiData::~NoteGuiData() {
	this->m_background = 0;
	this->m_prevPage = 0;
	this->m_nextPage = 0;
}

} //namespace gui 
