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
	NoteGuiData::NoteGuiData(const char* backgroundPath, const char* prevPagePath, const char* nextPagePath) : background(0),
																											   prevPage(0),
																											   nextPage(0) {
		if(backgroundPath != 0) {
			this->background = TextureContainer::LoadUI(backgroundPath);
		}
		if(prevPagePath != 0) {
			this->prevPage = TextureContainer::LoadUI(prevPagePath);
		}
		if(nextPagePath != 0) {
			this->nextPage = TextureContainer::LoadUI(nextPagePath);
		}
	}

	TextureContainer* NoteGuiData::getBackground() const {
		return this->background;
	}

	TextureContainer* NoteGuiData::getPrevPage() const {
		return this->prevPage;
	}

	TextureContainer* NoteGuiData::getNextPage() const {
		return this->nextPage;
	}

	NoteGuiData::~NoteGuiData() {
		this->background = 0;
		this->prevPage = 0;
		this->nextPage = 0;
	}
}