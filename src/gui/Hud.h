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

#ifndef ARX_GUI_HUD_H
#define ARX_GUI_HUD_H

#include "math/Types.h"

extern bool bIsAiming;

namespace gui {

void InventoryFaderUpdate();

void CloseSecondaryInventory();

}

void hudElementsInit();

void setHudScale(float scale);

void purseIconGuiRequestHalo();

void mecanismIconReset();
//! Show the quick save indicator for a short time
void showQuickSaveIcon();
void hitStrengthGaugeRequestFlash(float flashIntensity);
void bookIconGuiRequestFX();
void bookIconGuiRequestHalo();
//! Hide the quick save indicator
void hideQuickSaveIcon();

void hudUpdateInput();
void manageEditorControlsHUD2();

bool inventoryGuiupdateInputPROXY();

Vec2f getInventoryGuiAnchorPosition();

#endif // ARX_GUI_HUD_H
