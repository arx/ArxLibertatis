/*
 * Copyright 2014-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <memory>
#include <array>

#include "game/magic/Rune.h"
#include "graphics/GraphicsTypes.h"

namespace gui {

struct ARX_NECKLACE {
	
	std::unique_ptr<EERIE_3DOBJ> lacet;
	std::array<std::unique_ptr<EERIE_3DOBJ>, RUNE_COUNT> runes;
	std::array<TextureContainer *, RUNE_COUNT> pTexTab;
	std::array<Anglef, RUNE_COUNT> runeAngles;
	
};

extern ARX_NECKLACE necklace;

void NecklaceInit();
void ReleaseNecklace();

void ARX_INTERFACE_ManageOpenedBook_Finish(const Vec2f & mousePos, Rectf rect, float scale);

} // namespace gui

#endif // ARX_GUI_BOOK_NECKLACE_H
