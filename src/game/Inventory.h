/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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
#include <utility>

#include "game/GameTypes.h"
#include "math/Types.h"
#include "script/Script.h"

class Entity;

struct INVENTORY_SLOT {
	
	Entity * io;
	bool show;
	
	INVENTORY_SLOT()
		: io(NULL)
		, show(false)
	{}
	
};

struct INVENTORY_DATA {
	
	Entity * io;
	Vec2s m_size;
	INVENTORY_SLOT slot[20][20];
	
	INVENTORY_DATA()
		: io(NULL)
		, m_size(Vec2s(0, 0))
	{}
	
	~INVENTORY_DATA();
	
};

const size_t INVENTORY_BAGS = 3;
const size_t INVENTORY_X = 16;
const size_t INVENTORY_Y = 3;

// TODO this should be completely wrapped in PlayerInventory!
extern INVENTORY_SLOT g_inventory[INVENTORY_BAGS][INVENTORY_X][INVENTORY_Y];

extern INVENTORY_DATA * SecondaryInventory;
extern INVENTORY_DATA * TSecondaryInventory;
extern Entity * DRAGINTER;
extern Entity * ioSteal;

inline Vec2s inventorySizeFromTextureSize(Vec2i size) {
	return Vec2s(glm::clamp((size + Vec2i(31, 31)) / Vec2i(32, 32), Vec2i(1, 1), Vec2i(3, 3)));
}

struct InventoryPos {
	
	typedef unsigned short index_type;
	
	EntityHandle io;
	index_type bag;
	index_type x;
	index_type y;
	
	InventoryPos()
		: bag(0)
		, x(0)
		, y(0)
	{ }
	
	InventoryPos(EntityHandle io_, index_type bag_, index_type x_, index_type y_)
		: io(io_), bag(bag_), x(x_), y(y_) { }
	
	//! \return true if this is a valid position
	operator bool() const {
		return (io != EntityHandle());
	}
	
};

extern InventoryPos g_draggedItemPreviousPosition;

std::ostream & operator<<(std::ostream & strm, const InventoryPos & pos);

//! Sort the inventory and stack duplicate items
void optimizeInventory(Entity * container);

/*!
 * Insert an item into the player inventory
 * The item will be added to existing stacks if possible.
 * Otherwise a the first empty slot will be used.
 * If no slot was available, the item is dropped in front of the player
 *
 * \param item the item to insert
 *
 * \return true if the item was added to the inventory, false if it was dropped
 */
bool giveToPlayer(Entity * item);

/*!
 * Insert an item into the player inventory
 * The item will be added to existing stacks if possible.
 * Otherwise, the item will be inserted at the specified position.
 * If that fails, a the first empty slot will be used.
 * If no slot was available, the item is dropped in front of the player
 *
 * \param item the item to insert
 *
 * \return true if the item was added to the inventory, false if it was dropped
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
 * \param item the item to insert
 *
 * \return true if the item was inserted, false otherwise
 */
bool insertIntoInventory(Entity * item, const InventoryPos & pos);

/*!
 * Insert an item into an inventory
 * The item will be added to existing stacks if possible.
 * Otherwise a the first empty slot will be used.
 * If that fails, the first empty slot will be used.
 *
 * Does not check if the item is already in the inventory!
 *
 * \param item the item to insert
 *
 * \return true if the item was inserted, false otherwise
 */
bool insertIntoInventory(Entity * item, Entity * container);

/*!
 * Insert an item into the inventory at a specified position
 * The item will be inserted near the specified position if possible.
 * Otherwise, the item will be added to existing stacks if possible.
 * Otherwise, the item will be inserted at the specified previous position.
 * If that fails, the first empty slot will be used.
 *
 * Does not check if the item is already in the inventory!
 *
 * \param item the item to insert
 *
 * \return true if the item was inserted, false otherwise
 */
bool insertIntoInventoryAt(Entity * item, Entity * container, InventoryPos::index_type bag, Vec2f pos,
                           const InventoryPos & previous = InventoryPos());

/*!
 * Insert an item into the inventory at a specified position without firing script events
 * The item will be inserted at the specified previous position.
 * Otherwise, the item will be added to existing stacks if possible.
 * If that fails, the first empty slot will be used.
 *
 * Does not check if the item is already in the inventory!
 *
 * \param item the item to insert
 *
 * \return true if the item was inserted, false otherwise
 */
bool insertIntoInventoryAtNoEvent(Entity * item, const InventoryPos & pos);

/*!
 * Get the position of an item in the inventory.
 *
 * \return the position of the item
 */
InventoryPos locateInInventories(const Entity * item);

/*!
 * Remove an item from all inventories.
 * The item is not deleted.
 *
 * \return the old position of the item
 */
InventoryPos removeFromInventories(Entity * item);

void PutInFrontOfPlayer(Entity * io);

Vec3f GetItemWorldPosition(const Entity * io);
Vec3f GetItemWorldPositionSound(const Entity * io);

void CleanInventory();
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand);
bool IsInPlayerInventory(Entity * io);
Entity * getInventoryItemWithLowestDurability(const std::string & className, float minDurability = 0.f);

void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_OpenClose(Entity * io);

void ARX_INVENTORY_IdentifyIO(Entity * _pIO);

#endif // ARX_GAME_INVENTORY_H
