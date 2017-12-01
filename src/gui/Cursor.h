/*
 * Copyright 2013-2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_CURSOR_H
#define ARX_GUI_CURSOR_H

class Entity;
class TextureContainer;

enum ARX_INTERFACE_CURSOR_MODE
{
	CURSOR_UNDEFINED,
	CURSOR_FIREBALLAIM,
	CURSOR_INTERACTION_ON,
	CURSOR_REDIST,
	CURSOR_COMBINEON,
	CURSOR_COMBINEOFF,
	CURSOR_READY_WEAPON
};

extern ARX_INTERFACE_CURSOR_MODE SpecialCursor;

bool cursorIsSpecial();
void cursorSetInteraction();

extern TextureContainer * cursorMovable;

enum EntityMoveCursor {
	EntityMoveCursor_Throw = -1,
	EntityMoveCursor_Ok = 0,
	EntityMoveCursor_Invalid = 1
};

extern EntityMoveCursor CANNOT_PUT_IT_HERE;

void cursorTexturesInit();

bool Manage3DCursor(Entity * io, bool simulate);

void ARX_INTERFACE_RenderCursor(bool flag);

#endif // ARX_GUI_CURSOR_H
