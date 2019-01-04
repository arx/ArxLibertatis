/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "game/Inventory.h"

#include <algorithm>
#include <set>
#include <vector>

#include <boost/foreach.hpp>

#include "ai/Paths.h"

#include "core/Application.h"
#include "core/Config.h"

#include "game/EntityManager.h"
#include "game/Item.h"
#include "game/Player.h"

#include "gui/Hud.h"
#include "gui/Interface.h"
#include "gui/hud/PlayerInventory.h"
#include "gui/hud/SecondaryInventory.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector.h"

#include "physics/Physics.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "script/Script.h"


void ARX_INVENTORY_ReOrder();

INVENTORY_SLOT g_inventory[INVENTORY_BAGS][INVENTORY_X][INVENTORY_Y];
INVENTORY_DATA * SecondaryInventory = NULL;
Entity * DRAGINTER = NULL;
Entity * ioSteal = NULL;

// 1 player 2 secondary
short sInventory = -1;
Vec2s sInventoryPos = Vec2s(-1, -1);

/*!
 * Declares an IO as entering into player Inventory
 * Sends appropriate INVENTORYIN Event to player AND concerned io.
 */
void ARX_INVENTORY_Declare_InventoryIn(Entity * io) {
	if(!io)
		return;
	
	io->show = SHOW_FLAG_IN_INVENTORY;
	
	if(io->ignition > 0) {
		
		lightHandleDestroy(io->ignit_light);
		
		ARX_SOUND_Stop(io->ignit_sound);
		io->ignit_sound = audio::SourcedSample();
		
		io->ignition = 0;
	}
	
	SendIOScriptEvent(io, entities.player(), SM_INVENTORYIN);
	SendIOScriptEvent(entities.player(), io, SM_INVENTORYIN);
	
}

/*!
 * \brief Cleans Player inventory
 */
void CleanInventory() {
	
	for(size_t bag = 0; bag < INVENTORY_BAGS; bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		slot.io = NULL;
		slot.show = true;
	}
	
	g_playerInventoryHud.setCurrentBag(0);
}

Entity * GetInventoryObj_INVENTORYUSE(const Vec2s & pos) {
	
	std::pair<Entity *, int> result = GetFromInventory(pos);
	
	if(result.first) {
		if(result.second == 2 && SecondaryInventory) {
			Entity * temp = SecondaryInventory->io;
			if(temp->ioflags & IO_SHOP) {
				return NULL;
			}
		}
		return result.first;
	}

	if(InInventoryPos(pos))
		return NULL;
	
	return InterClick(pos);
}

//! Puts an IO in front of the player
void PutInFrontOfPlayer(Entity * io)
{
	if(!io)
		return;
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getYaw()) * 80.f;
	io->pos += Vec3f(0.f, 20.f, 0.f);
	
	io->angle = Anglef();
	io->show = SHOW_FLAG_IN_SCENE;

	if(io->obj && io->obj->pbox) {
		Vec3f vector = Vec3f(0.f, 100.f, 0.f);
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, vector);
	}
}

std::ostream & operator<<(std::ostream & strm, const InventoryPos & p) {
	return strm << '(' << p.io.handleData() << ", " << p.bag << ", " << p.x << ", " << p.y << ')';
}

namespace {

// Glue code to access both player and IO inventories in a uniform way.
template <size_t MaxBags, size_t MaxWidth, size_t MaxHeight>
struct InventoryArray {
	typedef INVENTORY_SLOT type[MaxBags][MaxWidth][MaxHeight];
	typedef InventoryPos::index_type index_type;
	static INVENTORY_SLOT & index(type & data, index_type bag,
	                       index_type x, index_type y) {
		return data[bag][x][y];
	}
	static const INVENTORY_SLOT & index(const type & data, index_type bag,
	                             index_type x, index_type y) {
		return data[bag][x][y];
	}
};
template <size_t MaxWidth, size_t MaxHeight>
struct InventoryArray<1, MaxWidth, MaxHeight> {
	typedef INVENTORY_SLOT type[MaxWidth][MaxHeight];
	typedef InventoryPos::index_type index_type;
	static INVENTORY_SLOT & index(type & data, index_type bag,
	                       index_type x, index_type y) {
		ARX_UNUSED(bag);
		return data[x][y];
	}
	static const INVENTORY_SLOT & index(const type & data, index_type bag,
	                             index_type x, index_type y) {
		ARX_UNUSED(bag);
		return data[x][y];
	}
};

//! Compare items by their size and name
struct ItemSizeComaparator {
	bool operator()(const Entity * a, const Entity * b) const {
		int sizea = a->m_inventorySize.x * a->m_inventorySize.y * a->m_inventorySize.y;
		int sizeb = b->m_inventorySize.x * b->m_inventorySize.y * b->m_inventorySize.y;
		if(sizea != sizeb) {
			return (sizea > sizeb);
		}
		int locname = a->locname.compare(b->locname);
		if(locname != 0) {
			return (locname < 0);
		}
		int name = a->idString().compare(b->idString());
		if(name != 0) {
			return (name < 0);
		}
		return (a < b);
	}
};

// TODO this class should probably be used directly in player and IO objects
template <size_t MaxBags, size_t MaxWidth, size_t MaxHeight>
class Inventory {
	
	typedef InventoryPos Pos;
	
public:
	
	typedef InventoryArray<MaxBags, MaxWidth, MaxHeight> Array;
	typedef typename Array::type array_type;
	typedef unsigned short size_type;
	typedef InventoryPos::index_type index_type;
	
private:
	
	long io;
	array_type & data;
	size_type bags;
	size_type width;
	size_type height;
	
	INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) {
		return Array::index(data, bag, x, y);
	}
	const INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) const {
		return Array::index(data, bag, x, y);
	}
	
	INVENTORY_SLOT & index(const InventoryPos & pos) {
		return index(pos.bag, pos.x, pos.y);
	}
	const INVENTORY_SLOT & index(const InventoryPos & pos) const {
		return index(pos.bag, pos.x, pos.y);
	}
	
	Pos insertImpl(Entity * item) {
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		if(Pos pos = insertIntoStack(item)) {
			return pos;
		}
		return insertIntoNewSlot(item);
	}
	
	Pos insertImpl(Entity * item, const Pos & pos) {
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		if(insertIntoStackAt(item, pos)) {
			return pos;
		}
		if(Pos newPos = insertIntoStack(item)) {
			return newPos;
		}
		if(insertIntoNewSlotAt(item, pos)) {
			return pos;
		}
		return insertIntoNewSlot(item);
	}
	
	void removeAt(const Entity * item, const Pos & pos) {
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		arx_assert(pos.x + item->m_inventorySize.x <= width);
		arx_assert(pos.y + item->m_inventorySize.y <= height);
		arx_assert(index(pos).io == item);
		
		LogDebug(" - " << pos << " remove " << item->idString()
		         << " [" << item->_itemdata->count << '/'
		         << item->_itemdata->playerstacksize << "]: "
		         << int(item->m_inventorySize.x) << 'x' << int(item->m_inventorySize.y));
		
		for(index_type j = pos.y; j < (pos.y + size_type(item->m_inventorySize.y)); j++) {
			for(index_type i = pos.x; i < (pos.x + size_type(item->m_inventorySize.x)); i++) {
				index(pos.bag, i, j).io = NULL;
				index(pos.bag, i, j).show = false;
			}
		}
	}
	
	bool insertIntoNewSlotAt(Entity * item, const Pos & pos) {
		
		if(!pos || pos.bag >= bags) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		if(pos.x + item->m_inventorySize.x > width || pos.y + item->m_inventorySize.y > height) {
			return false;
		}
		
		// Check if the whole area required by this item is empty
		for(index_type j = pos.y; j < pos.y + item->m_inventorySize.y; j++) {
			for(index_type i = pos.x; i < pos.x + item->m_inventorySize.x; i++) {
				if(index(pos.bag, i, j).io != NULL) {
					return false;
				}
			}
		}
		
		LogDebug(" - " << pos << " := " << item->idString()
		         << " [" << item->_itemdata->count << '/'
		         << item->_itemdata->playerstacksize << "]: "
		         << int(item->m_inventorySize.x) << 'x' << int(item->m_inventorySize.y));
		
		// Insert the item at the found position
		for(index_type j = pos.y; j < (pos.y + item->m_inventorySize.y); j++) {
			for (index_type i = pos.x; i < (pos.x + item->m_inventorySize.x); i++) {
				index(pos.bag, i, j).io = item;
				index(pos.bag, i, j).show = false;
			}
		}
		index(pos).show = true;
		
		return true;
	}
	
	Pos insertIntoNewSlot(Entity * item) {
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		for(index_type bag = 0; bag < bags; bag++) {
			for(index_type i = 0; i < width + 1 - item->m_inventorySize.x; i++) {
				for(index_type j = 0; j < height + 1 - item->m_inventorySize.y; j++) {
					
					// Ignore already used inventory slots
					if(index(bag, i, j).io != NULL) {
						continue;
					}
					
					Pos pos(io, bag, i, j);
					if(insertIntoNewSlotAt(item, pos)) {
						return pos;
					}
				}
			}
		}
		
		return Pos();
	}
	
	bool insertIntoStackAt(Entity * item, const Pos & pos) {
		
		if(!pos || pos.bag >= bags) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		if(pos.x + item->m_inventorySize.x > width || pos.y + item->m_inventorySize.y > height) {
			return false;
		}
		
		Entity * oldItem = index(pos).io;
		
		// Ignore empty slots and different or non-stackeable items
		if(!oldItem || oldItem->_itemdata->playerstacksize <= 1 || !IsSameObject(item, oldItem)) {
			return false;
		}
		
		// Ignore full stacks
		if(oldItem->_itemdata->count >= oldItem->_itemdata->playerstacksize) {
			return false;
		}
		
		// Get the number of items to add to the stack
		short int remainingSpace = oldItem->_itemdata->playerstacksize - oldItem->_itemdata->count;
		short int count = std::min(item->_itemdata->count, remainingSpace);
		
		LogDebug(" - " << pos << " " << oldItem->idString()
		         << " [" << oldItem->_itemdata->count << '/'
		         << oldItem->_itemdata->playerstacksize << "] += "
		         << item->idString() << " x" << count << '/' << item->_itemdata->count);
		
		oldItem->_itemdata->count += count, item->_itemdata->count -= count;
		
		if(item->_itemdata->count != 0) {
			// We inserted some of the items into the stack, but there was not enough
			// space for all of them.
			return false;
		}
		
		// Delete the old item
		item->destroy();
		return true;
	}
	
	Pos insertIntoStack(Entity * item) {
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		// Try to add the items to an existing stack
		for(index_type bag = 0; bag < bags; bag++) {
			for(index_type i = 0; i < width + 1 - size_type(item->m_inventorySize.x); i++) {
				for(index_type j = 0; j < height + 1 - size_type(item->m_inventorySize.y); j++) {
					Pos pos(io, bag, i, j);
					if(insertIntoStackAt(item, pos)) {
						return pos;
					}
				}
			}
		}
		
		return Pos();
	}
	
public:
	
	Inventory(long io_, array_type & data_, size_type bags_, size_type width_, size_type height_)
		: io(io_), data(data_), bags(bags_), width(width_), height(height_) { }
	
	/*!
	 * Insert an item into the inventory
	 * The item will be added to existing stacks if possible.
	 * Otherwise the first empty slot will be used.
	 *
	 * Does not check if the item is already in the inventory!
	 *
	 * \param item the item to insert
	 *
	 * \return true if the item was inserted, false otherwise
	 */
	bool insert(Entity * item) {
		if(item && (item->ioflags & IO_ITEM)) {
			if(Pos pos = insertImpl(item)) {
				ARX_INVENTORY_Declare_InventoryIn(get(pos));
				return true;
			}
		}
		return false;
	}
	
	/*!
	 * Insert an item into the inventory
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
	bool insert(Entity * item, const Pos & pos) {
		if(item && (item->ioflags & IO_ITEM)) {
			if(Pos newPos = insertImpl(item, pos)) {
				ARX_INVENTORY_Declare_InventoryIn(get(newPos));
				return true;
			}
		}
		return false;
	}
	
	//! Sort the inventory and stack duplicate items
	void optimize() {
		
		LogDebug("collecting items");
		
		// Collect all inventory items
		std::vector<Entity *> items;
		for(size_t bag = 0; bag < bags; bag++) {
			for(size_t j = 0 ; j < height; j++) {
				for(size_t i = 0 ; i < width; i++) {
					INVENTORY_SLOT & slot = index(bag, i, j);
					if(slot.io && slot.show) {
						items.push_back(slot.io);
						removeAt(slot.io, Pos(io, bag, i, j));
					}
					slot.io = NULL;
					slot.show = false;
				}
			}
		}
		
		// Sort the items by their size and name
		std::sort(items.begin(), items.end(), ItemSizeComaparator());
		
		LogDebug("sorting");
		#ifdef ARX_DEBUG
		BOOST_FOREACH(const Entity * item, items) {
			LogDebug(" - " << item->idString() << ": "
			         << int(item->m_inventorySize.x) << 'x' << int(item->m_inventorySize.y));
		}
		#endif
		
		LogDebug("putting back items");
		
		// Now put the items back into the inventory
		BOOST_FOREACH(Entity * item, items) {
			if(!insertImpl(item)) {
				// TODO: oops - there was no space for the item
				// ideally this should not happen, but the current sorting algorithm does not
				// guarantee that the resulting order is at least as good as the existing one
				LogDebug("could not insert " << item->idString() << " after sorting inventory");
				PutInFrontOfPlayer(item);
			}
		}
	}
	
	/*!
	 * Get the position of an item in the inventory.
	 *
	 * \return the position of the item
	 */
	Pos locate(const Entity * item) const {
		for(size_t bag = 0; bag < bags; bag++) {
			for(size_t i = 0; i < width; i++) {
				for(size_t j = 0; j < height; j++) {
					if(index(bag, i, j).io == item) {
						return Pos(io, bag, i, j);
					}
				}
			}
		}
		return Pos();
	}
	
	/*!
	 * Remove an item from the inventory.
	 * The item is not deleted.
	 *
	 * \return the old position of the item
	 */
	Pos remove(const Entity * item) {
		Pos pos = locate(item);
		if(pos) {
			/* TODO There was a bug cauing corrupted inventories where an item
			 * takes up more slots than specified by its size. Ideally, this should
			 * be fixed while loading old save games, but until then we just fix it
			 * here when removing items.
			removeAt(item, pos);
			 */
			for(size_t j = 0; j < height; j++) {
				for(size_t i = 0; i < width; i++) {
					if(index(pos.bag, i, j).io == item) {
						index(pos.bag, i, j).io = NULL;
						index(pos.bag, i, j).show = false;
					}
				}
			}
		}
		return pos;
	}
	
	Entity * get(const Pos & pos) const {
		return pos ? index(pos).io : NULL;
	}
	
};

Inventory<INVENTORY_BAGS, INVENTORY_X, INVENTORY_Y> getPlayerInventory() {
	return Inventory<INVENTORY_BAGS, INVENTORY_X, INVENTORY_Y>(0, g_inventory, player.m_bags,
	                                              INVENTORY_X, INVENTORY_Y);
}

Inventory<1, 20, 20> getIoInventory(Entity * io) {
	arx_assert(io != NULL && io->inventory != NULL);
	INVENTORY_DATA * inv = io->inventory;
	return Inventory<1, 20, 20>(io->index().handleData(), inv->slot, 1, inv->m_size.x, inv->m_size.y);
}

} // anonymous namespace

PlayerInventory playerInventory;

PlayerInventory::Pos PlayerInventory::locate(const Entity * item) {
	return getPlayerInventory().locate(item);
}

PlayerInventory::Pos PlayerInventory::remove(const Entity * item) {
	return getPlayerInventory().remove(item);
}

bool PlayerInventory::insert(Entity * item) {
	if(item && (item->ioflags & IO_GOLD)) {
		ARX_PLAYER_AddGold(item);
		return true;
	}
	return getPlayerInventory().insert(item);
}

bool PlayerInventory::insert(Entity * item, const Pos & pos) {
	if(item && (item->ioflags & IO_GOLD)) {
		ARX_PLAYER_AddGold(item);
		return true;
	}
	return getPlayerInventory().insert(item, pos);
}

void PlayerInventory::optimize() {
	getPlayerInventory().optimize();
}

bool giveToPlayer(Entity * item) {
	if(playerInventory.insert(item)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

bool giveToPlayer(Entity * item, const InventoryPos & pos) {
	if(playerInventory.insert(item, pos)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

InventoryPos removeFromInventories(Entity * item) {
	
	// TODO the item should know the inventory it is in and position
	
	InventoryPos oldPos = playerInventory.remove(item);
	if(oldPos) {
		return oldPos;
	}
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->inventory) {
			oldPos = getIoInventory(e).remove(item);
			if(oldPos) {
				return oldPos;
			}
		}
	}
	
	return InventoryPos();
}

InventoryPos locateInInventories(const Entity * item) {
	
	InventoryPos pos = playerInventory.locate(item);
	if(pos) {
		return pos;
	}
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->inventory) {
			pos = getIoInventory(e).locate(item);
			if(pos) {
				return pos;
			}
		}
	}
	
	return InventoryPos();
}

bool insertIntoInventory(Entity * item, const InventoryPos & pos) {
	
	if(pos.io == EntityHandle_Player) {
		return giveToPlayer(item, pos);
	}
	
	if(ValidIONum(pos.io) && entities[pos.io]->inventory) {
		if(getIoInventory(entities[pos.io]).insert(item, pos)) {
			return true;
		}
	}
	
	PutInFrontOfPlayer(item);
	return false;
}

/*!
 * \brief Tries to put an object in secondary inventory
 */
bool CanBePutInSecondaryInventory(INVENTORY_DATA * id, Entity * io)
{
	if(!id || !io)
		return false;
	
	if(io->ioflags & IO_MOVABLE)
		return false;
	
	const Vec2s s = io->m_inventorySize;
	
	// on essaie de le remettre à son ancienne place
	if(   sInventory == 2
	   && sInventoryPos.x >= 0
	   && sInventoryPos.x <= id->m_size.x - s.x
	   && sInventoryPos.y >= 0
	   && sInventoryPos.y <= id->m_size.y - s.y
	) {
		Vec2s pos = sInventoryPos;
		// first try to stack
		
		Entity * ioo = id->slot[pos.x][pos.y].io;
		
		if(   ioo
		   && ioo->_itemdata->playerstacksize > 1
		   && IsSameObject(io, ioo)
		   && ioo->_itemdata->count < ioo->_itemdata->playerstacksize
		   && ioo->durability == io->durability
		) {
			if(io->ioflags & IO_GOLD){
				ioo->_itemdata->price += io->_itemdata->price;
			} else {
				ioo->_itemdata->count += io->_itemdata->count;
				ioo->scale = 1.f;
			}
			
			io->destroy();
			
			sInventory = -1;
			return true;
		}
		
		ioo = id->slot[pos.x][pos.y].io;
		
		if(!ioo) {
			long valid = 1;
			
			if(s.x == 0 || s.y == 0)
				valid = 0;
			
			for(long y2 = pos.y; y2 < pos.y + s.y; y2++)
			for(long x2 = pos.x; x2 < pos.x + s.x; x2++) {
				if(id->slot[x2][y2].io != NULL) {
					valid = 0;
					break;
				}
			}
			
			if(valid) {
				for(long y2 = pos.y; y2 < pos.y + s.y; y2++)
				for(long x2 = pos.x; x2 < pos.x + s.x; x2++) {
					id->slot[x2][y2].io = io;
					id->slot[x2][y2].show = false;
				}
				
				id->slot[pos.x][pos.y].show = true;
				sInventory = -1;
				return true;
			}
		}
	}
	
	for(long y = 0; y <= id->m_size.y - s.y; y++)
	for(long x = 0; x <= id->m_size.x - s.x; x++) {
		Entity * ioo = id->slot[x][y].io;
		
		if(   ioo
		   && ioo->_itemdata->playerstacksize > 1
		   && IsSameObject(io, ioo)
		   && ioo->_itemdata->count < ioo->_itemdata->playerstacksize
		   && ioo->durability == io->durability
		) {
			if (io->ioflags & IO_GOLD) {
				ioo->_itemdata->price += io->_itemdata->price;
			} else {
				ioo->_itemdata->count += io->_itemdata->count;
				ioo->scale = 1.f;
			}
			
			io->destroy();
			
			return true;
		}
	}
	
	for(long y = 0; y <= id->m_size.y - s.y; y++)
	for(long x = 0; x <= id->m_size.x - s.x; x++) {
		Entity * ioo = id->slot[x][y].io;
		
		if(!ioo) {
			long valid = 1;
			
			if(s.x == 0 || s.y == 0)
				valid = 0;
			
			for(long y2 = y; y2 < y + s.y; y2++)
			for(long x2 = x; x2 < x + s.x; x2++) {
				if (id->slot[x2][y2].io != NULL) {
					valid = 0;
					break;
				}
			}
			
			if(valid) {
				for(long y2 = y; y2 < y + s.y; y2++)
				for(long x2 = x; x2 < x + s.x; x2++) {
					id->slot[x2][y2].io = io;
					id->slot[x2][y2].show = false;
				}
				
				id->slot[x][y].show = true;
				return true;
			}
		}
	}
	
	return false;
}

//! Try to put DRAGINTER object in an inventory
void PutInInventory() {
	// Check Validity
	if(!DRAGINTER || (DRAGINTER->ioflags & IO_MOVABLE))
		return;
	
	g_secondaryInventoryHud.dropEntity();
	
	g_playerInventoryHud.dropEntity();
}

/*!
 * \brief Returns true if "pos" is a position in player inventory or in SECONDARY inventory
 */
bool InInventoryPos(const Vec2s & pos) {
	if(g_secondaryInventoryHud.containsPos(pos))
		return true;

	return g_playerInventoryHud.containsPos(pos);
}

/*!
 * \brief Returns IO under position xx,yy in any INVENTORY or NULL if no IO was found
 */
std::pair<Entity *, int> GetFromInventory(const Vec2s & pos) {
	
	if(!InInventoryPos(pos)) {
		return std::pair<Entity *, int>(NULL, 0);
	}
	
	Entity * result = g_secondaryInventoryHud.getObj(pos);
	if(result) {
		return std::pair<Entity *, int>(result, 2);
	}
	
	result = g_playerInventoryHud.getObj(pos);
	if(result) {
		return std::pair<Entity *, int>(result, 1);
	}
	
	return std::pair<Entity *, int>(NULL, 0);
}

/*!
 * \brief Gets real world position for an IO (can be used for non items)
 * (even in an inventory or being dragged)
 *
 * Put the position in "pos". returns true if position was found
 * or false if object is invalid, or position not defined.
 */
Vec3f GetItemWorldPosition(const Entity * io) {
	arx_assert(io);
	
	// Is this object being Dragged by player ?
	if(DRAGINTER == io) {
		// Set position to approximate center of player.
		return player.pos + Vec3f(0.f, 80.f, 0.f);
	}

	// Not in scene ?
	if(io->show != SHOW_FLAG_IN_SCENE) {
		// Is it equiped ?
		if(IsEquipedByPlayer(io)) {
			// in player inventory
			return player.pos + Vec3f(0.f, 80.f, 0.f);
		}
		
		arx_assert(player.m_bags >= 0);
		arx_assert(player.m_bags <= 3);
		
		// Is it in any player inventory ?
		for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
		for(size_t y = 0; y < INVENTORY_Y; y++)
		for(size_t x = 0; x < INVENTORY_X; x++) {
			const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
			
			if(slot.io == io) {
				return player.pos + Vec3f(0.f, 80.f, 0.f);
			}
		}

		// Is it in any other IO inventory ?
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * ioo = entities[handle];
			
			if(!ioo || !ioo->inventory)
				continue;
			
			INVENTORY_DATA * id = ioo->inventory;
			for(long j = 0; j < id->m_size.y; j++) {
			for(long k = 0; k < id->m_size.x; k++) {
				if(id->slot[k][j].io == io) {
					return ioo->pos;
				}
			}
			}
		}
	}

	// Default position.
	return io->pos;
}

/*!
 * \brief Gets real world position for an IO to spawn a sound
 */
Vec3f GetItemWorldPositionSound(const Entity * io) {
	
	arx_assert(io);
	
	if(DRAGINTER == io) {
		return ARX_PLAYER_FrontPos();
	}
	
	if(io->show != SHOW_FLAG_IN_SCENE) {
		
		if(IsEquipedByPlayer(io)) {
			// in player inventory
			return ARX_PLAYER_FrontPos();
		}
		
		arx_assert(player.m_bags >= 0);
		arx_assert(player.m_bags <= 3);
		
		for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
		for(size_t y = 0; y < INVENTORY_Y; y++)
		for(size_t x = 0; x < INVENTORY_X; x++) {
			const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
			
			if(slot.io == io) {
				// in player inventory
				return ARX_PLAYER_FrontPos();
			}
		}
		
		for(size_t i = 0; i < entities.size(); i++) {
			const EntityHandle handle = EntityHandle(i);
			Entity * ioo = entities[handle];
			
			if(!ioo || !ioo->inventory)
				continue;
			
			INVENTORY_DATA * id = ioo->inventory;
			for(long j = 0; j < id->m_size.y; j++) {
			for(long k = 0; k < id->m_size.x; k++) {
				if(id->slot[k][j].io == io) {
					return ioo->pos;
				}
			}
			}
		}
	}
	
	return io->pos;
}

/*!
 * \brief Seeks an IO in all Inventories to remove it
 */
void RemoveFromAllInventories(const Entity * io) {
	
	if(!io) {
		return;
	}
	
	// Seek IO in Player Inventory/ies
	playerInventory.remove(io);
	
	// Seek IO in Other IO's Inventories
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(!e || !e->inventory)
			continue;
		
		INVENTORY_DATA * id = e->inventory;
		for(long j = 0; j < id->m_size.y; j++) {
		for(long k = 0; k < id->m_size.x; k++) {
			if(id->slot[k][j].io == io) {
				id->slot[k][j].io = NULL;
				id->slot[k][j].show = true;
			}
		}
		}
	}
}

/*!
 * \brief Seeks an IO in all Inventories to replace it by another IO
 */
void CheckForInventoryReplaceMe(Entity * io, Entity * old) {
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(!e || !e->inventory)
			continue;
		
		INVENTORY_DATA * id = e->inventory;
		
		for(long j = 0; j < id->m_size.y; j++) {
		for(long k = 0; k < id->m_size.x; k++) {
			if(id->slot[k][j].io == old) {
				if(CanBePutInSecondaryInventory(id, io)) {
					return;
				}
				PutInFrontOfPlayer(io);
				return;
			}
		}
		}
	}
}


/*!
 * \brief Takes an object from an inventory (be it player's or secondary inventory)
 * at screen position "xx,yy" and Puts that object in player's "hand" (cursor)
 *
 * \return true if an object was taken
 */
bool TakeFromInventory(const Vec2s & pos) {
	
	std::pair<Entity *, int> result = GetFromInventory(pos);
	
	if(result.first == NULL) {
		return false;
	}
	
	switch(result.second) {
		case 1:
			g_playerInventoryHud.dragEntity(result.first, pos);
			break;
		case 2:
			g_secondaryInventoryHud.dragEntity(result.first, pos);
			break;
		default:
			arx_unreachable();
	}
	
	return true;
}

bool IsInPlayerInventory(Entity * io) {
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(slot.io == io) {
			return true;
		}
	}
	
	return false;
}

bool IsInSecondaryInventory(Entity * io) {
	
	if(!SecondaryInventory)
		return false;
	
	for(long y = 0; y < SecondaryInventory->m_size.y; y++)
	for(long x = 0; x < SecondaryInventory->m_size.x; x++) {
		if(SecondaryInventory->slot[x][y].io == io) {
			return true;
		}
	}
	
	return false;
}

// TODO don't use texture name to find entity
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand) {
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(!slot.io || !slot.io->obj)
			continue;
		
		for(size_t lTex = 0; lTex < slot.io->obj->texturecontainer.size(); lTex++) {
			if(   !slot.io->obj->texturecontainer.empty()
			   && slot.io->obj->texturecontainer[lTex]
			   && slot.io->obj->texturecontainer[lTex]->m_texName == _lpszText
			) {
				if(slot.io->gameFlags & GFLAG_INTERACTIVITY) {
					SendIOScriptEvent(entities.player(), slot.io, _lCommand);
				}
				return;
			}
		}
	}
}

Entity * ARX_INVENTORY_GetTorchLowestDurability() {
	
	Entity * io = NULL;
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(!slot.io || slot.io->locname != "description_torch")
			continue;
		
		if(!io || slot.io->durability < io->durability) {
			io = slot.io;
		}
	}
	
	return io;
}

long Player_Arrow_Count() {
	
	long count = 0;
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(slot.io && slot.io->className() == "arrows" && slot.io->durability >= 1.f) {
			count += checked_range_cast<long>(slot.io->durability);
		}
	}
	
	return count;
}

Entity * Player_Arrow_Count_Decrease() {
	
	Entity * io = NULL;
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(slot.io && slot.io->className() == "arrows" && slot.io->durability >= 1.f) {
			if(!io || io->durability > slot.io->durability)
				io = slot.io;
		}
	}
	
	return io;
}

void ARX_INVENTORY_IdentifyIO(Entity * _pIO) {
	if(_pIO && (_pIO->ioflags & IO_ITEM) && _pIO->_itemdata->equipitem) {
		if(player.m_skillFull.objectKnowledge + player.m_attributeFull.mind
		   >= _pIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) {
			SendIOScriptEvent(entities.player(), _pIO, SM_IDENTIFY);
		}
	}
}

void ARX_INVENTORY_IdentifyAll() {
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * io = g_inventory[bag][x][y].io;
		
		ARX_INVENTORY_IdentifyIO(io);
	}
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_OpenClose(Entity * _io)
{
	// CLOSING
	if(!_io || SecondaryInventory == _io->inventory) {
		if(SecondaryInventory && SecondaryInventory->io)
			SendIOScriptEvent(entities.player(), SecondaryInventory->io, SM_INVENTORY2_CLOSE);

		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
		DRAGGING = false;
	} else {
		if(TSecondaryInventory && TSecondaryInventory->io)
			SendIOScriptEvent(entities.player(), TSecondaryInventory->io, SM_INVENTORY2_CLOSE);

		g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_right;
		TSecondaryInventory = SecondaryInventory = _io->inventory;

		if(SecondaryInventory && SecondaryInventory->io != NULL) {
			if(SendIOScriptEvent(entities.player(), SecondaryInventory->io, SM_INVENTORY2_OPEN) == REFUSE) {
				g_secondaryInventoryHud.m_fadeDirection = SecondaryInventoryHud::Fade_left;
				TSecondaryInventory = SecondaryInventory = NULL;
				return;
			}
		}

		if(player.Interface & INTER_COMBATMODE) {
			ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);
		}

		if(config.input.autoReadyWeapon != AlwaysAutoReadyWeapon) {
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if(SecondaryInventory && SecondaryInventory->io && (SecondaryInventory->io->ioflags & IO_SHOP))
			ARX_INVENTORY_ReOrder();
		
		DRAGGING = false;
	}

	if(player.Interface & INTER_INVENTORYALL) {
		ARX_SOUND_PlayInterface(g_snd.BACKPACK, Random::getf(0.9f, 1.1f));
		g_playerInventoryHud.close();
	}
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_TakeAllFromSecondaryInventory() {
	bool bSound = false;

	if(TSecondaryInventory) {
		for(long j = 0; j < TSecondaryInventory->m_size.y; j++)
		for(long i = 0; i < TSecondaryInventory->m_size.x; i++) {
			INVENTORY_SLOT & slot = TSecondaryInventory->slot[i][j];
			
			if(!slot.io || !slot.show)
				continue;
			
			Entity * io = slot.io;
			
			if(!(io->ioflags & IO_GOLD))
				RemoveFromAllInventories(io);
			
			if(playerInventory.insert(io)) {
				bSound = true;
			} else {
				sInventory = 2;
				sInventoryPos = Vec2s(i, j);
				
				CanBePutInSecondaryInventory(TSecondaryInventory, io);
			}
		}
	}

	if(bSound)
		ARX_SOUND_PlayInterface(g_snd.INVSTD);
	else
		ARX_SOUND_PlayInterface(g_snd.INVSTD, 0.1f);
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_ReOrder() {
	
	if(!TSecondaryInventory)
		return;
	
	for(long j = 0; j < TSecondaryInventory->m_size.y; j++)
	for(long i = 0; i < TSecondaryInventory->m_size.x; i++) {
		INVENTORY_SLOT & slot = TSecondaryInventory->slot[i][j];
		
		if(!slot.io || !slot.show)
			continue;
		
		Entity * io = slot.io;
		
		RemoveFromAllInventories(io);
		sInventory = 2;
		sInventoryPos = Vec2s(0);
		
		if(CanBePutInSecondaryInventory(TSecondaryInventory, io)) {
		} else{
			sInventory = 2;
			
			sInventoryPos = Vec2s(i, j);
			
			CanBePutInSecondaryInventory(TSecondaryInventory, io);
		}
	}
}
