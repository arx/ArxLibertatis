/*
 * Copyright 2014-2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_BOOK_NECKLACE_H
#define ARX_GUI_BOOK_NECKLACE_H

#include "game/magic/Rune.h"
#include "graphics/GraphicsTypes.h"

namespace gui {

struct ARX_NECKLACE {
	EERIE_3DOBJ * lacet;
	EERIE_3DOBJ * runes[RUNE_COUNT];
	TextureContainer * pTexTab[RUNE_COUNT];
};

extern ARX_NECKLACE necklace;

void NecklaceInit();
void ReleaseNecklace();

void ARX_INTERFACE_ManageOpenedBook_Finish(const Vec2f & mousePos);

} // namespace gui

#endif // ARX_GUI_BOOK_NECKLACE_H
