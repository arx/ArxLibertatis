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
#ifndef ARX_GUI_NOTEGUIDATA_H
#define ARX_GUI_NOTEGUIDATA_H

#include <string>
#include "graphics/data/TextureContainer.h"

namespace gui {
/*!
* Wraps up GUI data for notes.
* This is intended to be used as a pair for note types (each note type has its own NoteGuiData).
* In the case of a data-driven design with a parser feeding data from an external config file, the parser
* would feed NoteGuiData(s).
*/
class NoteGuiData {
public:
	NoteGuiData(const char* backgroundPath, const char* prevPagePath, const char* nextPagePath);
	~NoteGuiData();
	TextureContainer* getBackground() const;
	TextureContainer* getPrevPage() const;
	TextureContainer* getNextPage() const;

private:
	TextureContainer* m_background;
	TextureContainer* m_prevPage;
	TextureContainer* m_nextPage;
};

} //namespace gui

#endif //ARX_GUI_NOTEGUIDATA_H