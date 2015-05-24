/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"

#include "input/Input.h"

#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector.h"

#include "physics/Box.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/Interactive.h"
#include "scene/GameSound.h"

#include "script/Script.h"

extern float InventoryX;
extern float InventoryDir;

void ARX_INVENTORY_ReOrder();
void ARX_INVENTORY_IdentifyIO(Entity * _pIO);

//------------------------------------------------------------------------------------
//CInventory Inventory;
INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];
INVENTORY_DATA * SecondaryInventory = NULL;
Entity * DRAGINTER = NULL;
Entity * ioSteal = NULL;
long InventoryY = 100;
static long HERO_OR_SECONDARY = 0;
short sActiveInventory = 0;

// 1 player 2 secondary
short sInventory = -1;
Vec2s sInventoryPos = Vec2s(-1, -1);

/*!
 * Declares an IO as entering into player Inventory
 * Sends appropriate INVENTORYIN Event to player AND concerned io.
 */
static void ARX_INVENTORY_Declare_InventoryIn(Entity * io) {
	if(!io)
		return;
	
	io->show = SHOW_FLAG_IN_INVENTORY;
	
	if(io->ignition > 0) {
		
		lightHandleDestroy(io->ignit_light);
		
		if (io->ignit_sound != audio::INVALID_ID) {
			ARX_SOUND_Stop(io->ignit_sound);
			io->ignit_sound = audio::INVALID_ID;
		}
		
		io->ignition = 0;
	}
	
	EVENT_SENDER = io;
	SendIOScriptEvent(entities.player(), SM_INVENTORYIN);
	EVENT_SENDER = entities.player();
	SendIOScriptEvent(io, SM_INVENTORYIN);
	EVENT_SENDER = NULL;
}

/*!
 * \brief Cleans Player inventory
 */
void CleanInventory() {
	
	for(size_t bag = 0; bag < 3; bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		slot.io	 = NULL;
		slot.show = 1;
	}
	
	sActiveInventory = 0;
}

extern Vec2s DANAEMouse;

extern Rect g_size;

static Entity * GetInventoryObj(const Vec2s & pos) {
	
	Vec2f anchorPos = getInventoryGuiAnchorPosition();
	
	Vec2i iPos = Vec2i(anchorPos);
	
	if(player.Interface & INTER_INVENTORY) {
		long tx = pos.x - iPos.x; //-4
		long ty = pos.y - iPos.y; //-2

		if(tx >= 0 && ty >= 0) {
			tx = checked_range_cast<long>(tx / INTERFACE_RATIO(32));
			ty = checked_range_cast<long>(ty / INTERFACE_RATIO(32));

			if((tx >= 0) && ((size_t)tx < INVENTORY_X) && (ty >= 0) && ((size_t)ty < INVENTORY_Y)) {
				Entity *result = inventory[sActiveInventory][tx][ty].io;

				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					HERO_OR_SECONDARY = 1;
					return result;
				}
			}

			return NULL;
		}
	} else if(player.Interface & INTER_INVENTORYALL) {

		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);

		int iY = checked_range_cast<int>(fBag);

		for(size_t bag = 0; bag < size_t(player.bag); bag++) {
			long tx = pos.x - iPos.x;
			long ty = pos.y - iPos.y - iY;

			tx = checked_range_cast<long>(tx / INTERFACE_RATIO(32));
			ty = checked_range_cast<long>(ty / INTERFACE_RATIO(32));

			if(tx >= 0 && (size_t)tx < INVENTORY_X && ty >= 0 && (size_t)ty < INVENTORY_Y) {
				Entity *result = inventory[bag][tx][ty].io;

				if(result && (result->gameFlags & GFLAG_INTERACTIVITY)) {
					HERO_OR_SECONDARY = 1;
					return result;
				}

				return NULL;
			}

			iY += checked_range_cast<int>(INTERFACE_RATIO(121));
		}
	}

	return NULL;
}

Entity * GetInventoryObj_INVENTORYUSE(const Vec2s & pos)
{
	Entity * io = GetFromInventory(pos);

	if(io) {
		if(HERO_OR_SECONDARY == 2) {
			if(SecondaryInventory) {
				Entity *temp = SecondaryInventory->io;

				if(temp->ioflags & IO_SHOP)
					return NULL;
			}
		}

		return io;
	}

	if(InInventoryPos(pos))
		return NULL;

	io = InterClick(pos);

	return io;
}

/*!
 * \brief Puts an IO in front of the player
 * \param io
 */
void PutInFrontOfPlayer(Entity * io)
{
	if(!io)
		return;
	
	io->pos = player.pos;
	io->pos += angleToVectorXZ(player.angle.getPitch()) * 80.f;
	io->pos += Vec3f(0.f, 20.f, 0.f);
	
	io->velocity.y = 0.3f;
	io->velocity.x = 0; 
	io->velocity.z = 0; 
	io->angle = Anglef::ZERO;
	io->stopped = 0;
	io->show = SHOW_FLAG_IN_SCENE;

	if(io->obj && io->obj->pbox) {
		Vec3f vector = Vec3f(0.f, 100.f, 0.f);
		io->soundtime = 0;
		io->soundcount = 0;
		EERIE_PHYSICS_BOX_Launch(io->obj, io->pos, io->angle, vector);
	}
}

std::ostream & operator<<(std::ostream & strm, const InventoryPos & p) {
	return strm << '(' << p.io << ", " << p.bag << ", " << p.x << ", " << p.y << ')';
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
				index(pos.bag, i, j).show = 0;
			}
		}
	}
	
	bool insertIntoNewSlotAt(Entity * item, const Pos & pos) {
		
		if(!pos || pos.bag >= bags) {
			return false;
		}
		
		if(pos.x + item->m_inventorySize.x > width || pos.y + item->m_inventorySize.y > height) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
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
				index(pos.bag, i, j).show = 0;
			}
		}
		index(pos).show = 1;
		
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
		
		if(pos.x + item->m_inventorySize.x > width || pos.y + item->m_inventorySize.y > height) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		Entity * io = index(pos).io;
		
		// Ignore empty slots and different or non-stackeable items
		if(!io || io->_itemdata->playerstacksize <= 1 || !IsSameObject(item, io)) {
			return false;
		}
		
		// Ignore full stacks
		if(io->_itemdata->count >= io->_itemdata->playerstacksize) {
			return false;
		}
		
		// Get the number of items to add to the stack
		short int remainingSpace = io->_itemdata->playerstacksize - io->_itemdata->count;
		short int count = std::min(item->_itemdata->count, remainingSpace);
		
		LogDebug(" - " << pos << " " << io->idString()
		         << " [" << io->_itemdata->count << '/'
		         << io->_itemdata->playerstacksize << "] += "
		         << item->idString() << " x" << count << '/' << item->_itemdata->count);
		
		io->_itemdata->count += count, item->_itemdata->count -= count;
		
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
	
	Inventory(long io, array_type & data, size_type bags, size_type width, size_type height)
		: io(io), data(data), bags(bags), width(width), height(height) { }
		
	Inventory(const Inventory & o)
		: io(o.io), data(o.data), bags(o.bags), width(o.width), height(o.height) { }
	
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
					slot.show = 0;
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
						index(pos.bag, i, j).show = 0;
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

Inventory<3, INVENTORY_X, INVENTORY_Y> getPlayerInventory() {
	return Inventory<3, INVENTORY_X, INVENTORY_Y>(0, inventory, player.bag,
	                                              INVENTORY_X, INVENTORY_Y);
}

Inventory<1, 20, 20> getIoInventory(Entity * io) {
	arx_assert(io != NULL && io->inventory != NULL);
	INVENTORY_DATA * inv = io->inventory;
	return Inventory<1, 20, 20>(io->index(), inv->slot, 1, inv->m_size.x, inv->m_size.y);
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
	
	if(pos.io == 0) {
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

// TODO replace remaining uses of this with playerInventory.insert()
/*!
 * \brief tries to put an object in player inventory
 */
static bool CanBePutInInventory(Entity * io) {
	
	if(io == NULL)
		return false;
	
	if(io->ioflags & IO_MOVABLE)
		return false;
	
	if(io->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(io);
		return true;
	}
	
	const Vec2s s = io->m_inventorySize;
	
	// on essaie de le remettre à son ancienne place --------------------------
	if(   sInventory == 1
	   && sInventoryPos.x >= 0
	   && sInventoryPos.x <= short(INVENTORY_X) - s.x
	   && sInventoryPos.y >= 0
	   && sInventoryPos.y <= short(INVENTORY_Y) - s.y
	) {
		long j = sInventoryPos.y;
		long i = sInventoryPos.x;
		
		arx_assert(player.bag >= 0);
		arx_assert(player.bag <= 3);
		
		// first try to stack -------------------------------------------------
		for(size_t bag = 0; bag < size_t(player.bag); bag++) {
				
				Entity * ioo = inventory[bag][i][j].io;
				
				if(ioo && ioo->_itemdata->playerstacksize > 1 && IsSameObject(io, ioo)) {
					if(ioo->_itemdata->count < ioo->_itemdata->playerstacksize) {
						ioo->_itemdata->count += io->_itemdata->count;
						
						if(ioo->_itemdata->count > ioo->_itemdata->playerstacksize) {
							io->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
							ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
						} else {
							io->_itemdata->count = 0;
						}
						if(!io->_itemdata->count) {
							io->destroy();
						}
						
						ARX_INVENTORY_Declare_InventoryIn(ioo);
						sInventory = -1;
						return true;
					}
				}
		}
		
		arx_assert(player.bag >= 0);
		arx_assert(player.bag <= 3);
		
		for(size_t bag = 0; bag < size_t(player.bag); bag++) {
				if(inventory[bag][i][j].io == NULL) {
					bool valid = true;
					
					if(s.x == 0 || s.y == 0)
						valid = false;
					
					for(long k = j; k < j + s.y; k++)
					for(long l = i; l < i + s.x; l++) {
						if(inventory[bag][l][k].io != NULL)
							valid = false;
					}
					
					if(valid) {
						for(long k = j; k < j + s.y; k++)
						for(long l = i; l < i + s.x; l++) {
							inventory[bag][l][k].io = io;
							inventory[bag][l][k].show = 0;
						}
						
						inventory[bag][i][j].show = 1;
						ARX_INVENTORY_Declare_InventoryIn(io);
						sInventory = -1;
						return true;
					}
				}
		}
	}
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t x = 0; x <= INVENTORY_X - s.x; x++)
	for(size_t y = 0; y <= INVENTORY_Y - s.y; y++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(!slot.io || !IsSameObject(io, slot.io))
			continue;
		
		IO_ITEMDATA * slotItem = slot.io->_itemdata;
		
		if(slotItem->playerstacksize > 1 && slotItem->count < slotItem->playerstacksize) {
			
			slotItem->count += io->_itemdata->count;
			
			if(slotItem->count > slotItem->playerstacksize) {
				io->_itemdata->count = slotItem->count - slotItem->playerstacksize;
				slotItem->count = slotItem->playerstacksize;
			} else {
				io->_itemdata->count = 0;
			}
			if(!io->_itemdata->count) {
				io->destroy();
				ARX_INVENTORY_Declare_InventoryIn(slot.io);
				return true;
			}
		}
	}
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t x = 0; x <= INVENTORY_X - s.x; x++)
	for(size_t y = 0; y <= INVENTORY_Y - s.y; y++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(!slot.io)
			continue;
				
		bool valid = true;
		
		if(s.x == 0 || s.y == 0)
			valid = false;
		
		for(size_t k = y; k < y + s.y; k++)
		for(size_t l = x; l < x + s.x; l++) {
			if(inventory[bag][l][k].io != NULL)
				valid = false;
		}
		
		if(valid) {
			for(size_t k = y; k < y + s.y; k++)
			for(size_t l = x; l < x + s.x; l++) {
				inventory[bag][l][k].io = io;
				inventory[bag][l][k].show = 0;
			}
			
			slot.show = 1;
			ARX_INVENTORY_Declare_InventoryIn(io);
			return true;
		}
	}
	
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
		long y = sInventoryPos.y;
		long x = sInventoryPos.x;
		// first try to stack
		
		Entity * ioo = id->slot[x][y].io;
		
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
		
		ioo = id->slot[x][y].io;
		
		if(!ioo) {
			long valid = 1;
			
			if(s.x == 0 || s.y == 0)
				valid = 0;
			
			for(long y2 = y; y2 < y + s.y; y2++)
			for(long x2 = x; x2 < x + s.x; x2++) {
				if(id->slot[x2][y2].io != NULL) {
					valid = 0;
					break;
				}
			}
			
			if(valid) {
				for(long y2 = y; y2 < y + s.y; y2++)
				for(long x2 = x; x2 < x + s.x; x2++) {
					id->slot[x2][y2].io = io;
					id->slot[x2][y2].show = 0;
				}
				
				id->slot[x][y].show = 1;
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
					id->slot[x2][y2].show = 0;
				}
				
				id->slot[x][y].show = 1;
				return true;
			}
		}
	}
	
	return false;
}

/*!
 * \brief Try to put DRAGINTER object in an inventory
 * \return
 */
bool PutInInventory() {
	// Check Validity
	if(!DRAGINTER || (DRAGINTER->ioflags & IO_MOVABLE))
		return false;
	
	Vec2s t = Vec2s_ZERO;
	
	Vec2s s = DRAGINTER->m_inventorySize;
	
	const Rect backpackMouseTestRect(
	g_size.width() - 35,
	g_size.height() - 113,
	g_size.width() - 35 + 32,
	g_size.height() - 113 + 32
	);
	
	// Check for backpack Icon
	if(backpackMouseTestRect.contains(Vec2i(DANAEMouse))) {
		if(CanBePutInInventory(DRAGINTER)) {
			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
		}
		return false;
	}
	
	// First Look for Identical Item...
	if(SecondaryInventory && InSecondaryInventoryPos(DANAEMouse)) {
		Entity * io = SecondaryInventory->io;
		
		float fprice = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f; //>>1;
		long price = checked_range_cast<long>(fprice);
		price *= DRAGINTER->_itemdata->count;
		fprice = price + price * ((float)player.m_skillFull.intuition) * 0.005f;
		price = checked_range_cast<long>(fprice);
		
		// SHOP
		if(io->ioflags & IO_SHOP) {
			if(!io->shop_category.empty() && DRAGINTER->groups.find(io->shop_category) == DRAGINTER->groups.end())
				return false;
			
			if(price <= 0)
				return false;
			
			// Check shop group
			for(long j = 0; j < SecondaryInventory->m_size.y; j++) {
			for(long i = 0; i < SecondaryInventory->m_size.x; i++) {
				Entity * ioo = SecondaryInventory->slot[i][j].io;
				
				if(!ioo || !IsSameObject(DRAGINTER, ioo))
					continue;
				
				ioo->_itemdata->count += DRAGINTER->_itemdata->count;
				ioo->scale = 1.f;
				
				DRAGINTER->destroy();
				
				ARX_PLAYER_AddGold(price);
				ARX_SOUND_PlayInterface(SND_GOLD);
				ARX_SOUND_PlayInterface(SND_INVSTD);
				return true;
			}
			}
		}
		
		t.x = DANAEMouse.x + static_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		t.y = DANAEMouse.y - SHORT_INTERFACE_RATIO(13);
		t.x = t.x / SHORT_INTERFACE_RATIO(32);
		t.y = t.y / SHORT_INTERFACE_RATIO(32);
		
		if(t.x <= SecondaryInventory->m_size.x - s.x && t.y <= SecondaryInventory->m_size.y - s.y) {
			
			float fprice = ARX_INTERACTIVE_GetPrice(DRAGINTER, io) / 3.0f;
			long price = checked_range_cast<long>(fprice);
			price *= DRAGINTER->_itemdata->count;
			fprice = price + price * ((float)player.m_skillFull.intuition) * 0.005f;
			price = checked_range_cast<long>(fprice);
			
			for(long j = 0; j < s.y; j++) {
			for(long i = 0; i < s.x; i++) {
				Entity * ioo = SecondaryInventory->slot[t.x+i][t.y+j].io;
				
				if(!ioo)
					continue;
				
				DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
				
				if(   ioo->_itemdata->playerstacksize > 1
				   && IsSameObject(DRAGINTER, ioo)
				   && ioo->_itemdata->count < ioo->_itemdata->playerstacksize
				) {
					ioo->_itemdata->count += DRAGINTER->_itemdata->count;
					
					if(ioo->_itemdata->count > ioo->_itemdata->playerstacksize) {
						DRAGINTER->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
						ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
					} else {
						DRAGINTER->_itemdata->count = 0;
					}
				}
				
				if(DRAGINTER->_itemdata->count) {
					if(CanBePutInSecondaryInventory(SecondaryInventory, DRAGINTER)) {
						// SHOP
						if(io->ioflags & IO_SHOP) {
							ARX_PLAYER_AddGold(price);
							ARX_SOUND_PlayInterface(SND_GOLD);
						}
					} else {
						return false;
					}
				}
				
				ARX_SOUND_PlayInterface(SND_INVSTD);
				Set_DragInter(NULL);
				return true;
			}
			}
			
			if(DRAGINTER->ioflags & IO_GOLD) {
				ARX_PLAYER_AddGold(DRAGINTER);
				Set_DragInter(NULL);
				return true;
			}

			for(long j = 0; j < s.y; j++) {
			for(long i = 0; i < s.x; i++) {
				SecondaryInventory->slot[t.x+i][t.y+j].io = DRAGINTER;
				SecondaryInventory->slot[t.x+i][t.y+j].show = 0;
			}
			}
			
			// SHOP
			if(io->ioflags & IO_SHOP) {
				player.gold += price;
				ARX_SOUND_PlayInterface(SND_GOLD);
			}

			SecondaryInventory->slot[t.x][t.y].show = 1;
			DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
			ARX_SOUND_PlayInterface(SND_INVSTD);
			Set_DragInter(NULL);
			return true;
		}
	}
	
	if(!(player.Interface & INTER_INVENTORY) && !(player.Interface & INTER_INVENTORYALL))
		return false;
	
	if(InventoryY != 0)
		return false;
	
	if(!InPlayerInventoryPos(DANAEMouse))
		return false;
	
	int bag = 0;
	
	Vec2f anchorPos = getInventoryGuiAnchorPosition();
	
	float fCenterX	= anchorPos.x;
	float fSizY		= anchorPos.y;
	
	short iPosX = checked_range_cast<short>(fCenterX);
	short iPosY = checked_range_cast<short>(fSizY);
	
	if(player.Interface & INTER_INVENTORY) {
		t.x = DANAEMouse.x - iPosX;
		t.y = DANAEMouse.y - iPosY;
		t.x = t.x / SHORT_INTERFACE_RATIO(32); 
		t.y = t.y / SHORT_INTERFACE_RATIO(32); 
		
		if((t.x >= 0) && (t.x <= 16 - s.x) && (t.y >= 0) && (t.y <= 3 - s.y)) {
			bag = sActiveInventory;
		} else {
			return false;
		}
	} else {
		bool bOk = false;
		
		float fBag	= (player.bag - 1) * INTERFACE_RATIO(-121);
		
		short iY = checked_range_cast<short>(fBag);
		
		//We must enter the for-loop to initialyze tx/ty
		arx_assert(0 < player.bag);
		
		for(int i = 0; i < player.bag; i++) {
			t.x = DANAEMouse.x - iPosX;
			t.y = DANAEMouse.y - iPosY - iY; 
			
			if((t.x >= 0) && (t.y >= 0)) {
				t.x = t.x / SHORT_INTERFACE_RATIO(32); 
				t.y = t.y / SHORT_INTERFACE_RATIO(32); 
				
				if((t.x >= 0) && (t.x <= 16 - s.x) && (t.y >= 0) && (t.y <= 3 - s.y)) {
					bOk = true;
					bag = i;
					break;
				}
			}
			
			float fRatio = INTERFACE_RATIO(121);
			
			iY += checked_range_cast<short>(fRatio);
		}
		
		if(!bOk)
			return false;
	}
	
	if(DRAGINTER->ioflags & IO_GOLD) {
		ARX_PLAYER_AddGold(DRAGINTER);
		Set_DragInter(NULL);
		return true;
	}

	for(long j = 0; j < s.y; j++)
	for(long i = 0; i < s.x; i++) {
		Entity * ioo = inventory[bag][t.x+i][t.y+j].io;
		
		if(!ioo)
			continue;
		
		ARX_INVENTORY_IdentifyIO(ioo);

		if(   ioo->_itemdata->playerstacksize > 1
		   && IsSameObject(DRAGINTER, ioo)
		   && ioo->_itemdata->count < ioo->_itemdata->playerstacksize
		) {
			ioo->_itemdata->count += DRAGINTER->_itemdata->count;
			
			if(ioo->_itemdata->count > ioo->_itemdata->playerstacksize) {
				DRAGINTER->_itemdata->count = ioo->_itemdata->count - ioo->_itemdata->playerstacksize;
				ioo->_itemdata->count = ioo->_itemdata->playerstacksize;
			} else {
				DRAGINTER->_itemdata->count = 0;
			}
			
			ioo->scale = 1.f;
			ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
			
			if(!DRAGINTER->_itemdata->count) {
				DRAGINTER->destroy();
			}
			
			ARX_SOUND_PlayInterface(SND_INVSTD);
			return true;
		}
		
		return false;
	}
	
	for(long j = 0; j < s.y; j++) {
	for(long i = 0; i < s.x; i++) {
		inventory[bag][t.x+i][t.y+j].io = DRAGINTER;
		inventory[bag][t.x+i][t.y+j].show = 0;
	}
	}
	
	inventory[bag][t.x][t.y].show = 1;
	
	ARX_INVENTORY_Declare_InventoryIn(DRAGINTER);
	ARX_SOUND_PlayInterface(SND_INVSTD);
	DRAGINTER->show = SHOW_FLAG_IN_INVENTORY;
	Set_DragInter(NULL);
	return true;
}

/*!
 * \brief Returns true if xx,yy is a position in secondary inventory
 */
bool InSecondaryInventoryPos(const Vec2s & pos) {
	if(SecondaryInventory != NULL) {
		short tx, ty;

		tx = pos.x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		ty = pos.y - SHORT_INTERFACE_RATIO(13);
		tx = tx / SHORT_INTERFACE_RATIO(32);
		ty = ty / SHORT_INTERFACE_RATIO(32);

		if(tx < 0 || tx >= SecondaryInventory->m_size.x)
			return false;

		if(ty < 0 || ty >= SecondaryInventory->m_size.y)
			return false;

		return true;
	}

	return false;
}

/*!
 * \brief Returns true if xx,yy is a position in player inventory
 */
bool InPlayerInventoryPos(const Vec2s & pos) {
	Vec2f anchorPos = getInventoryGuiAnchorPosition();
	
	Vec2s iPos = Vec2s(anchorPos);

	if(player.Interface & INTER_INVENTORY) {
		Vec2s t = pos - iPos;
		
		if(t.x >= 0 && t.y >= 0) {
			t.x = t.x / SHORT_INTERFACE_RATIO(32);
			t.y = t.y / SHORT_INTERFACE_RATIO(32);

			if(   t.x >= 0
			   && (size_t)t.x <= INVENTORY_X
			   && t.y >= 0
			   && (size_t)t.y < INVENTORY_Y
			)
				return true;
			else
				return false;
		}
	} else if(player.Interface & INTER_INVENTORYALL) {
		float fBag = (player.bag - 1) * INTERFACE_RATIO(-121);

		short iY = checked_range_cast<short>(fBag);

		if(   pos.x >= iPos.x
		   && pos.x <= iPos.x + INVENTORY_X * INTERFACE_RATIO(32)
		   && pos.y >= iPos.y + iY
		   && pos.y <= g_size.height()
		) {
			return true;
		}

		for(int i = 0; i < player.bag; i++) {
			Vec2s t = pos - iPos;
			t.y -= iY;
			
			if(t.x >= 0 && t.y >= 0) {
				t.x = t.x / SHORT_INTERFACE_RATIO(32);
				t.y = t.y / SHORT_INTERFACE_RATIO(32);

				if(   t.x >= 0
				   && (size_t)t.x <= INVENTORY_X
				   && t.y >= 0
				   && (size_t)t.y < INVENTORY_Y
				) {
					return true;
				}
			}

			float fRatio	= INTERFACE_RATIO(121);

			iY = checked_range_cast<short>(iY + fRatio);
		}
	}

	return false;
}

/*!
 * \brief Returns true if "pos" is a position in player inventory or in SECONDARY inventory
 */
bool InInventoryPos(const Vec2s & pos) {
	if(InSecondaryInventoryPos(pos))
		return true;

	return InPlayerInventoryPos(pos);
}

/*!
 * \brief returns true if cursor is flying over any inventory
 */
bool IsFlyingOverInventory(const Vec2s & pos) {
	//	if(eMouseState==MOUSE_IN_WORLD) return false;

	if(SecondaryInventory != NULL) {
		short tx = pos.x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		short ty = pos.y - SHORT_INTERFACE_RATIO(13);
		tx /= SHORT_INTERFACE_RATIO(32);
		ty /= SHORT_INTERFACE_RATIO(32);

		if(   tx >= 0
		   && tx <= SecondaryInventory->m_size.x
		   && ty >= 0
		   && ty <= SecondaryInventory->m_size.y
		) {
			return true;
		}
	}

	return InPlayerInventoryPos(pos);
}

/*!
 * \brief Returns IO under position xx,yy in any INVENTORY or NULL if no IO was found
 */
Entity * GetFromInventory(const Vec2s & pos) {
	HERO_OR_SECONDARY = 0;

	if(!IsFlyingOverInventory(pos))
		return NULL;

	if(SecondaryInventory != NULL) {
		short tx = pos.x + checked_range_cast<short>(InventoryX) - SHORT_INTERFACE_RATIO(2);
		short ty = pos.y - SHORT_INTERFACE_RATIO(13);

		if(tx >= 0 && ty >= 0) {
			tx = tx / SHORT_INTERFACE_RATIO(32); 
			ty = ty / SHORT_INTERFACE_RATIO(32); 

			if(   tx >= 0
			   && tx <= SecondaryInventory->m_size.x
			   && ty >= 0
			   && ty <= SecondaryInventory->m_size.y
			) {
				if(SecondaryInventory->slot[tx][ty].io == NULL)
					return NULL;

				if(   (player.Interface & INTER_STEAL)
				   && !ARX_PLAYER_CanStealItem(SecondaryInventory->slot[tx][ty].io)
				) {
					return NULL;
				}

				Entity * io = SecondaryInventory->slot[tx][ty].io;

				if(!(io->gameFlags & GFLAG_INTERACTIVITY))
					return NULL;

				HERO_OR_SECONDARY = 2;
				return io;
			}
		}
	}

	return GetInventoryObj(pos);
}

/*!
 * \brief Gets real world position for an IO (can be used for non items)
 * (even in an inventory or being dragged)
 *
 * Put the position in "pos". returns true if position was found
 * or false if object is invalid, or position not defined.
 */
Vec3f GetItemWorldPosition(Entity * io) {
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
		
		arx_assert(player.bag >= 0);
		arx_assert(player.bag <= 3);
		
		// Is it in any player inventory ?
		for(size_t bag = 0; bag < size_t(player.bag); bag++)
		for(size_t y = 0; y < INVENTORY_Y; y++)
		for(size_t x = 0; x < INVENTORY_X; x++) {
			const INVENTORY_SLOT & slot = inventory[bag][x][y];
			
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
		
		arx_assert(player.bag >= 0);
		arx_assert(player.bag <= 3);
		
		for(size_t bag = 0; bag < size_t(player.bag); bag++)
		for(size_t y = 0; y < INVENTORY_Y; y++)
		for(size_t x = 0; x < INVENTORY_X; x++) {
			const INVENTORY_SLOT & slot = inventory[bag][x][y];
			
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
				id->slot[k][j].show = 1;
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
	Entity * io = GetFromInventory(pos);
	
	if(io == NULL)
		return false;
	
	if(SecondaryInventory != NULL) {
		if(InSecondaryInventoryPos(pos) && (io->ioflags & IO_ITEM)) {
			Entity * ioo = SecondaryInventory->io;
			
			if(ioo->ioflags & IO_SHOP) {
				long cos = ARX_INTERACTIVE_GetPrice(io, ioo);
				
				float fcos	= cos - cos * ((float)player.m_skillFull.intuition) * 0.005f;
				cos = checked_range_cast<long>(fcos);
				
				if(player.gold < cos) {
					return false;
				}
				
				ARX_SOUND_PlayInterface(SND_GOLD);
				player.gold -= cos;
				
				if(io->_itemdata->count > 1) {
					Entity * ioo = CloneIOItem(io);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					return true;
				}
			} else if(io->_itemdata->count > 1) {
				
				if(!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
					Entity * ioo = CloneIOItem(io);
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->scriptload = 1;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					sInventory = 2;
					
					
					float fCalcX = (pos.x + InventoryX - INTERFACE_RATIO(2)) / INTERFACE_RATIO(32);
					float fCalcY = (pos.y - INTERFACE_RATIO(13)) / INTERFACE_RATIO(32);
					
					sInventoryPos.x = checked_range_cast<short>(fCalcX);
					sInventoryPos.y = checked_range_cast<short>(fCalcY);
					
					//ARX_INVENTORY_Object_Out(SecondaryInventory->io, ioo);
					
					ARX_INVENTORY_IdentifyIO(ioo);
					return true;
				}
			}
		}
		
		for(long j = 0; j < SecondaryInventory->m_size.y; j++)
		for(long i = 0; i < SecondaryInventory->m_size.x; i++) {
			INVENTORY_SLOT & slot = SecondaryInventory->slot[i][j];
			
			if(slot.io != io)
				continue;
			
			slot.io = NULL;
			slot.show = 1;
			sInventory = 2;
			
			float fCalcX = (pos.x + InventoryX - INTERFACE_RATIO(2)) / INTERFACE_RATIO(32);
			float fCalcY = (pos.y - INTERFACE_RATIO(13)) / INTERFACE_RATIO(32);
			
			sInventoryPos.x = checked_range_cast<short>(fCalcX);
			sInventoryPos.y = checked_range_cast<short>(fCalcY);
		}
	}
	
	Vec2f anchorPos = getInventoryGuiAnchorPosition();
	
	Vec2i iPos = Vec2i(anchorPos);
	
	if(InPlayerInventoryPos(pos)) {
		if(!GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
			if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
				if(io->_itemdata->count - 1 > 0) {
					
					Entity * ioo = AddItem(io->classPath());
					ioo->show = SHOW_FLAG_NOT_DRAWN;
					ioo->_itemdata->count = 1;
					io->_itemdata->count--;
					ioo->scriptload = 1;
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(ioo);
					RemoveFromAllInventories(ioo);
					sInventory = 1;
					
					float fX = (pos.x - iPos.x) / INTERFACE_RATIO(32);
					float fY = (pos.y - iPos.y) / INTERFACE_RATIO(32);
					
					sInventoryPos.x = checked_range_cast<short>(fX);
					sInventoryPos.y = checked_range_cast<short>(fY);
					
					SendInitScriptEvent(ioo);
					ARX_INVENTORY_IdentifyIO(ioo);
					return true;
				}
			}
		}
	}
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(slot.io == io) {
			slot.io = NULL;
			slot.show = 1;
			sInventory = 1;
			
			float fX = (pos.x - iPos.x) / INTERFACE_RATIO(32);
			float fY = (pos.y - iPos.y) / INTERFACE_RATIO(32);
			
			sInventoryPos.x = checked_range_cast<short>(fX);
			sInventoryPos.y = checked_range_cast<short>(fY);
		}
	}
	
	Set_DragInter(io);
	
	RemoveFromAllInventories(io);
	ARX_INVENTORY_IdentifyIO(io);
	return true;
}

bool IsInPlayerInventory(Entity * io) {
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = inventory[bag][x][y];
		
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
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(!slot.io || !slot.io->obj)
			continue;
		
		for(size_t lTex = 0; lTex < slot.io->obj->texturecontainer.size(); lTex++) {
			if(   !slot.io->obj->texturecontainer.empty()
			   && slot.io->obj->texturecontainer[lTex]
			   && slot.io->obj->texturecontainer[lTex]->m_texName == _lpszText
			) {
				if(slot.io->gameFlags & GFLAG_INTERACTIVITY) {
					SendIOScriptEvent(slot.io, _lCommand);
				}
				return;
			}
		}
	}
}

Entity * ARX_INVENTORY_GetTorchLowestDurability() {
	
	Entity * io = NULL;
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = inventory[bag][x][y];
		
		if(!slot.io || slot.io->locname != "description_torch")
			continue;
		
		if(!io || slot.io->durability < io->durability) {
			io = slot.io;
		}
	}
	
	return io;
}

void ARX_INVENTORY_IdentifyIO(Entity * _pIO) {
	if(_pIO && (_pIO->ioflags & IO_ITEM) && _pIO->_itemdata->equipitem) {
		if(player.m_skillFull.objectKnowledge + player.m_attributeFull.mind
		   >= _pIO->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value) {
			SendIOScriptEvent(_pIO, SM_IDENTIFY);
		}
	}
}

void ARX_INVENTORY_IdentifyAll() {
	
	arx_assert(player.bag >= 0);
	arx_assert(player.bag <= 3);
	
	for(size_t bag = 0; bag < size_t(player.bag); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		Entity * io = inventory[bag][x][y].io;
		if(!io || !(io->ioflags & IO_ITEM) || !io->_itemdata->equipitem)
			continue;
		
		const float identifyValue = io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value;
		
		if(player.m_skillFull.objectKnowledge + player.m_attributeFull.mind >= identifyValue) {
			SendIOScriptEvent(io, SM_IDENTIFY);
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_INVENTORY_OpenClose(Entity * _io)
{
	// CLOSING
	if(!_io || SecondaryInventory == _io->inventory) {
		if(SecondaryInventory && SecondaryInventory->io)
			SendIOScriptEvent(SecondaryInventory->io, SM_INVENTORY2_CLOSE);

		InventoryDir = -1;
		TSecondaryInventory = SecondaryInventory;
		SecondaryInventory = NULL;
		EERIEMouseButton &= ~4;
		DRAGGING = false;
	} else {
		if(TSecondaryInventory && TSecondaryInventory->io)
			SendIOScriptEvent(TSecondaryInventory->io, SM_INVENTORY2_CLOSE);

		InventoryDir = 1;
		TSecondaryInventory = SecondaryInventory = _io->inventory;

		if(SecondaryInventory && SecondaryInventory->io != NULL) {
			if(SendIOScriptEvent(SecondaryInventory->io, SM_INVENTORY2_OPEN) == REFUSE) {
				InventoryDir = -1;
				TSecondaryInventory = SecondaryInventory = NULL;
				return;
			}
		}

		if(player.Interface & INTER_COMBATMODE) {
			ARX_INTERFACE_Combat_Mode(0);
		}

		if(!config.input.autoReadyWeapon) {
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if(SecondaryInventory && SecondaryInventory->io && (SecondaryInventory->io->ioflags & IO_SHOP))
			ARX_INVENTORY_ReOrder();

		EERIEMouseButton &= ~4;
		DRAGGING = false;
	}

	if(player.Interface & INTER_INVENTORYALL) {
		ARX_SOUND_PlayInterface(SND_BACKPACK, 0.9F + 0.2F * rnd());
		bInventoryClosing = true;
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
		ARX_SOUND_PlayInterface(SND_INVSTD);
	else
		ARX_SOUND_PlayInterface(SND_INVSTD, 0.1f);
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
		sInventoryPos = Vec2s_ZERO;
		
		if(CanBePutInSecondaryInventory(TSecondaryInventory, io)) {
		} else{
			sInventory = 2;
			
			sInventoryPos = Vec2s(i, j);
			
			CanBePutInSecondaryInventory(TSecondaryInventory, io);
		}
	}
}
