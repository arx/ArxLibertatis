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

#ifndef ARX_GUI_HUD_PLAYERINVENTORY_H
#define ARX_GUI_HUD_PLAYERINVENTORY_H

#include "math/Vector.h"

class Entity;

extern bool bInventorySwitch;
extern float fDecPulse;

void playerInventoryInit();
void playerInventoryUpdate();
bool inventoryGuiupdateInputPROXY();
void playerInventoryDraw();

void playerInventoryNextBag();
void playerInventoryPreviousBag();

bool InPlayerInventoryPos(const Vec2s & pos);
Entity * GetInventoryObj(const Vec2s & pos);

bool playerInventoryDropEntity();

#endif // ARX_GUI_HUD_PLAYERINVENTORY_H
