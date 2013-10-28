/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
#include <ostream>

#include "math/Types.h"
#include "script/Script.h"

class Entity;

struct INVENTORY_SLOT {
	Entity * io;
	long show;
};

struct INVENTORY_DATA {
	Entity * io;
	long sizex;
	long sizey;
	INVENTORY_SLOT slot[20][20];
};

const size_t INVENTORY_X = 16;
const size_t INVENTORY_Y = 3;

// TODO this should be completely wrapped in PlayerInventory!
extern INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];

extern INVENTORY_DATA * SecondaryInventory;
extern INVENTORY_DATA * TSecondaryInventory;
extern Entity * DRAGINTER;
extern Entity * ioSteal;
extern long InventoryY;

struct InventoryPos {
	
	typedef unsigned short index_type;
	
	long io;
	index_type bag;
	index_type x;
	index_type y;
	
	InventoryPos() : io(-1) { }
	
	InventoryPos(long io, index_type bag, index_type x, index_type y)
		: io(io), bag(bag), x(x), y(y) { }
	
	//! @return true if this is a valid position
	operator bool() const {
		return (io != -1);
	}
	
};

std::ostream & operator<<(std::ostream & strm, const InventoryPos & pos);

class PlayerInventory {
	
	typedef InventoryPos Pos;
	
public:
	
	/*!
	 * Insert an item into the player inventory
	 * The item will be added to existing stacks if possible.
	 * Otherwise the first empty slot will be used.
	 *
	 * Does not check if the item is already in the inventory!
	 *
	 * @param item the item to insert
	 *
	 * @return true if the item was inserted, false otherwise
	 */
	static bool insert(Entity * item);
	
	/*!
	 * Insert an item into the player inventory
	 * The item will be added to existing stacks if possible.
	 * Otherwise, the item will be inserted at the specified position.
	 * If that fails, the first empty slot will be used.
	 *
	 * Does not check if the item is already in the inventory!
	 *
	 * @param item the item to insert
	 *
	 * @return true if the item was inserted, false otherwise
	 */
	static bool insert(Entity * item, const Pos & pos);
	
	//! Sort the inventory and stack duplicate items
	static void optimize();
	
	/*!
	 * Get the position of an item in the inventory.
	 *
	 * @return the position of the item
	 */
	static Pos locate(const Entity * item);
	
	/*!
	 * Remove an item from the inventory.
	 * The item is not deleted.
	 *
	 * @return the old position of the item
	 */
	static Pos remove(const Entity * item);
	
	static Entity * get(const Pos & pos) {
		return pos ? inventory[pos.bag][pos.x][pos.y].io : NULL;
	}
	
};

extern PlayerInventory playerInventory;

/*!
 * Insert an item into the player inventory
 * The item will be added to existing stacks if possible.
 * Otherwise a the first empty slot will be used.
 * If no slot was available, the item is dropped in front of the player
 *
 * @param item the item to insert
 *
 * @return true if the item was added to the inventory, false if it was dropped
 */
bool giveToPlayer(Entity * item);

/*!
 * Insert an item into the player inventory
 * The item will be added to existing stacks if possible.
 * Otherwise, the item will be inserted at the specified position.
 * If that fails, a the first empty slot will be used.
 * If no slot was available, the item is dropped in front of the player
 *
 * @param item the item to insert
 *
 * @return true if the item was added to the inventory, false if it was dropped
 */
bool giveToPlayer(Entity * item, const InventoryPos & pos);

/*!
 * Insert an item into an inventory
 * The item will be added to existing stacks if possible.
 * Otherwise, the item will be inserted at the specified position.
 * If that fails, the first empty slot will be used.
 *
 * Does not check if the item is already in the inventory!
 *
 * @param item the item to insert
 *
 * @return true if the item was inserted, false otherwise
 */
bool insertIntoInventory(Entity * item, const InventoryPos & pos);

/*!
 * Get the position of an item in the inventory.
 *
 * @return the position of the item
 */
InventoryPos locateInInventories(const Entity * item);

/*!
 * Remove an item from all inventories.
 * The item is not deleted.
 *
 * @return the old position of the item
 */
InventoryPos removeFromInventories(Entity * item);

/*!
 * Insert an item into an NPC's inventory
 * The item will be added to existing stacks if possible.
 * Otherwise, the item will be inserted at the specified position.
 * If that fails, a the first empty slot will be used.
 * If no slot was available, the item is dropped in front of the player
 *
 * @param item the item to insert
 *
 * @return true if the item was added to the inventory, false if it was dropped
 */
bool putInInventory(Entity * item, const InventoryPos & pos);

void PutInFrontOfPlayer(Entity * io);

bool GetItemWorldPosition(Entity * io, Vec3f * pos);
bool GetItemWorldPositionSound(const Entity * io, Vec3f * pos);

Entity * GetInventoryObj_INVENTORYUSE(Vec2s * pos);
void CheckForInventoryReplaceMe(Entity * io, Entity * old);

bool InSecondaryInventoryPos(Vec2s * pos);
bool InPlayerInventoryPos(Vec2s * pos);
bool CanBePutInSecondaryInventory(INVENTORY_DATA * id, Entity * io, long * xx, long * yy);

void CleanInventory();
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand);
bool PutInInventory();
bool TakeFromInventory(Vec2s * pos);
Entity * GetFromInventory(Vec2s * pos);
bool IsFlyingOverInventory(Vec2s * pos);
bool IsInPlayerInventory(Entity * io);
bool IsInSecondaryInventory(Entity * io);
bool InInventoryPos(Vec2s * pos);
void RemoveFromAllInventories(const Entity * io);
Entity * ARX_INVENTORY_GetTorchLowestDurability();
void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_OpenClose(Entity * io);
void ARX_INVENTORY_TakeAllFromSecondaryInventory();

void IO_Drop_Item(Entity * io_src, Entity * io);

#endif // ARX_GAME_INVENTORY_H
