/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_GAME_INVENTORY_H
#define ARX_GAME_INVENTORY_H

#include <stddef.h>
#include <string>

#include "math/MathFwd.h"
#include "script/Script.h"

struct INTERACTIVE_OBJ;

struct INVENTORY_SLOT {
	INTERACTIVE_OBJ * io;
	long show;
};

struct INVENTORY_DATA {
	INTERACTIVE_OBJ * io;
	long sizex;
	long sizey;
	INVENTORY_SLOT slot[20][20];
};

const size_t INVENTORY_X = 16;
const size_t INVENTORY_Y = 3;

extern INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];
extern INVENTORY_DATA * SecondaryInventory;
extern INVENTORY_DATA * TSecondaryInventory;
extern INTERACTIVE_OBJ * DRAGINTER;
extern INTERACTIVE_OBJ * ioSteal;
extern long InventoryY;

void PutInFrontOfPlayer(INTERACTIVE_OBJ * io);
bool CanBePutInInventory(INTERACTIVE_OBJ * io);

bool GetItemWorldPosition(INTERACTIVE_OBJ * io, Vec3f * pos);
bool GetItemWorldPositionSound(const INTERACTIVE_OBJ * io, Vec3f * pos);

INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(Vec2s * pos);
void CheckForInventoryReplaceMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * old);

bool InSecondaryInventoryPos(Vec2s * pos);
bool InPlayerInventoryPos(Vec2s * pos);
bool CanBePutInSecondaryInventory(INVENTORY_DATA * id, INTERACTIVE_OBJ * io, long * xx, long * yy);

void CleanInventory();
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand);
bool PutInInventory();
bool TakeFromInventory(Vec2s * pos);
INTERACTIVE_OBJ * GetFromInventory(Vec2s * pos);
bool IsFlyingOverInventory(Vec2s * pos);
void ForcePlayerInventoryObjectLevel(long level);
bool IsInPlayerInventory(INTERACTIVE_OBJ * io);
bool IsInSecondaryInventory(INTERACTIVE_OBJ * io);
bool InInventoryPos(Vec2s * pos);
void ReplaceInAllInventories(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void RemoveFromAllInventories(const INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * ARX_INVENTORY_GetTorchLowestDurability();
void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_OpenClose(INTERACTIVE_OBJ * io);
void ARX_INVENTORY_TakeAllFromSecondaryInventory();

void IO_Drop_Item(INTERACTIVE_OBJ * io_src, INTERACTIVE_OBJ * io);

#endif // ARX_GAME_INVENTORY_H
