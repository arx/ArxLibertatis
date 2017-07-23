/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_BOOK_BOOK_H
#define ARX_GUI_BOOK_BOOK_H

#include "gui/Note.h"

enum ARX_INTERFACE_BOOK_MODE
{
	BOOKMODE_STATS = 0,
	BOOKMODE_SPELLS,
	BOOKMODE_MINIMAP,
	BOOKMODE_QUESTS
};

extern long BOOKZOOM;

void ARX_INTERFACE_BookOpen();
void ARX_INTERFACE_BookClose();
void ARX_INTERFACE_BookToggle();

void forceBookPage(ARX_INTERFACE_BOOK_MODE page);
ARX_INTERFACE_BOOK_MODE currentBookPage();
ARX_INTERFACE_BOOK_MODE nextBookPage();
ARX_INTERFACE_BOOK_MODE prevBookPage();
void openBookPage(ARX_INTERFACE_BOOK_MODE newPage, bool toggle = false);

void updateQuestBook();

#endif // ARX_GUI_BOOK_BOOK_H
