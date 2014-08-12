/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_MAINMENU_H
#define ARX_GUI_MAINMENU_H

#include <string>

#include "math/Types.h"

// FIXME remove this
const std::string AUTO_RESOLUTION_STRING = "Automatic";

void Menu2_Render_NewQuest(Vec2i posBack, Vec2i size, Vec2i offset);
void Menu2_Render_EditQuest(Vec2i size, Vec2i offset, float fPosX1, Vec2i posBack);
void Menu2_Render_Options(Vec2i size, float fPosX1, Vec2i offset, Vec2i posBack, Vec2i posNext);
void Menu2_Render_Quit(Vec2i posBack, Vec2i size, Vec2i offset);

#endif // ARX_GUI_MAINMENU_H
