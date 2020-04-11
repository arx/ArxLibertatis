/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

InventoryPos g_draggedItemPreviousPosition;

INVENTORY_DATA::~INVENTORY_DATA() {
	
	for(long nj = 0; nj < m_size.y; nj++) {
		for(long ni = 0; ni < m_size.x; ni++) {
			arx_assert(slot[ni][nj].io == NULL);
			arx_assert(slot[ni][nj].show == false);
		}
	}
	
}

/*!
 * Declares an IO as entering into player Inventory
 * Sends appropriate INVENTORYIN Event to player AND concerned io.
 */
static void ARX_INVENTORY_Declare_InventoryIn(Entity * io, EntityHandle container) {
	
	arx_assert(io);
	
	if(io->ignition > 0) {
		
		lightHandleDestroy(io->ignit_light);
		
		ARX_SOUND_Stop(io->ignit_sound);
		io->ignit_sound = audio::SourcedSample();
		
		io->ignition = 0;
	}
	
	SendIOScriptEvent(io, entities.get(container), SM_INVENTORYIN);
	if(container == EntityHandle_Player) {
		// Items only receive the event when they are put in the player inventory because scripts expect that
		SendIOScriptEvent(entities.player(), io, SM_INVENTORYIN);
	}
	
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
	
	if(DRAGINTER == io) {
		Set_DragInter(NULL);
	}
	
	removeFromInventories(io);
	
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
		if(a->_itemdata->count != b->_itemdata->count) {
			return (a->_itemdata->count > b->_itemdata->count);
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
	
	EntityHandle io;
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
	
	bool insertGold(Entity * item) {
		
		if(io == EntityHandle_Player && item != NULL && (item->ioflags & IO_GOLD)) {
			ARX_PLAYER_AddGold(item);
			return true;
		}
		
		return false;
	}
	
	Pos insertImpl(Entity * item, const Pos & pos = Pos()) {
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
	
	// Move via diagonal lines thorough the rect made by start and end
	static void advance(Pos & p, Vec2s start, Vec2s end) {
		p.x++;
		p.y--;
		if(p.y < start.y || p.x >= end.x) {
			p.y = p.y + (p.x - start.x) + 1;
			p.x = start.x;
		}
	}
	
	Pos insertAtImpl(Entity * item, index_type bag, Vec2f pos, const Pos & fallback) {
		
		Vec2s start(pos + Vec2f(1.f / 3));
		Vec2s size = item->m_inventorySize;
		if(pos.x - float(start.x) > 1.f / 3) {
			size.x++;
		}
		if(pos.y - float(start.y) > 1.f / 3) {
			size.y++;
		}
		if(start.x < 0) {
			size.x += start.x;
			start.x = 0;
		}
		if(start.y < 0) {
			size.y += start.y;
			start.y = 0;
		}
		start = glm::min(start, Vec2s(width - 1, height - 1));
		Vec2s end = start + glm::clamp(size, Vec2s(1), Vec2s(width, height) - start);
		
		for(Pos p(io, bag, start.x, start.y); p.y < end.y; advance(p, start, end)) {
			if(insertIntoStackAt(item, p, true)) {
				return p;
			}
		}
		
		for(Pos p(io, bag, start.x, start.y); p.y < end.y; advance(p, start, end)) {
			if(insertIntoNewSlotAt(item, p)) {
				return p;
			}
		}
		
		return insertImpl(item, fallback);
	}
	
	bool insertIntoNewSlotAt(Entity * item, const Pos & pos) {
		
		if(!pos || pos.bag >= bags) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		arx_assert(pos.io == io);
		
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
	
		if(DRAGINTER == item) {
			Set_DragInter(NULL);
		}
		
		removeFromInventories(item);
		
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
		item->_itemdata->m_inventoryPos = pos;
		item->show = SHOW_FLAG_IN_INVENTORY;
		
		return true;
	}
	
	Pos insertIntoNewSlot(Entity * item) {
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		index_type maxi = width + 1 - size_type(item->m_inventorySize.x);
		index_type maxj = height + 1 - size_type(item->m_inventorySize.y);
		if(MaxHeight >= MaxWidth) {
			std::swap(maxi, maxj);
		}
		
		for(index_type bag = 0; bag < bags; bag++) {
			for(index_type i = 0; i < maxi; i++) {
				for(index_type j = 0; j < maxj; j++) {
					Pos pos = (MaxHeight < MaxWidth) ? Pos(io, bag, i, j) : Pos(io, bag, j, i);
					
					// Ignore already used inventory slots
					if(index(pos).io != NULL) {
						continue;
					}
					
					if(insertIntoNewSlotAt(item, pos)) {
						return pos;
					}
				}
			}
		}
		
		return Pos();
	}
	
	bool insertIntoStackAt(Entity * item, const Pos & pos, bool identify = false) {
		
		if(!pos || pos.bag >= bags) {
			return false;
		}
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		if(pos.x + item->m_inventorySize.x > width || pos.y + item->m_inventorySize.y > height) {
			return false;
		}
		
		Entity * oldItem = index(pos).io;
		
		if(oldItem && identify) {
			ARX_INVENTORY_IdentifyIO(oldItem);
		}
		
		if(item == oldItem) {
			return true;
		}
		
		// Ignore empty slots and different or non-stackeable items
		if(!oldItem || oldItem->_itemdata->playerstacksize <= 1 || !IsSameObject(item, oldItem)) {
			return false;
		}
		
		// Ignore full stacks
		if(oldItem->_itemdata->count >= oldItem->_itemdata->playerstacksize) {
			return false;
		}
		
		if((oldItem->ioflags & IO_GOLD) && (item->ioflags & IO_GOLD)) {
			oldItem->_itemdata->price += item->_itemdata->price;
			item->destroy();
			return true;
		}
		
		// Get the number of items to add to the stack
		short int remainingSpace = oldItem->_itemdata->playerstacksize - oldItem->_itemdata->count;
		short int count = std::min(item->_itemdata->count, remainingSpace);
		
		LogDebug(" - " << pos << " " << oldItem->idString()
		         << " [" << oldItem->_itemdata->count << '/'
		         << oldItem->_itemdata->playerstacksize << "] += "
		         << item->idString() << " x" << count << '/' << item->_itemdata->count);
		
		oldItem->scale = oldItem->scale * float(oldItem->_itemdata->count) + item->scale * float(count);
		
		oldItem->_itemdata->count += count, item->_itemdata->count -= count;
		
		oldItem->scale /= float(oldItem->_itemdata->count);
		
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
		
		index_type maxi = width + 1 - size_type(item->m_inventorySize.x);
		index_type maxj = height + 1 - size_type(item->m_inventorySize.y);
		if(MaxHeight >= MaxWidth) {
			std::swap(maxi, maxj);
		}
		
		// Try to add the items to an existing stack
		for(index_type bag = 0; bag < bags; bag++) {
			for(index_type i = 0; i < maxi; i++) {
				for(index_type j = 0; j < maxj; j++) {
					Pos pos = (MaxHeight < MaxWidth) ? Pos(io, bag, i, j) : Pos(io, bag, j, i);
					if(insertIntoStackAt(item, pos)) {
						return pos;
					}
				}
			}
		}
		
		return Pos();
	}
	
public:
	
	Inventory(EntityHandle io_, array_type & data_, size_type bags_, size_type width_, size_type height_)
		: io(io_), data(data_), bags(bags_), width(width_), height(height_) { }
	
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
	bool insert(Entity * item, const Pos & pos = Pos()) {
		
		if(insertGold(item)) {
			return true;
		}
		
		if(item && (item->ioflags & IO_ITEM)) {
			if(Pos newPos = insertImpl(item, pos)) {
				ARX_INVENTORY_Declare_InventoryIn(get(newPos), io);
				return true;
			}
		}
		
		return false;
	}
	
	/*!
	 * Insert an item into the inventory at a specified position
	 * The item will be inserted near the specified position if possible.
	 * Otherwise, the item will be added to existing stacks if possible.
	 * Otherwise, the item will be inserted at the specified fallback position.
	 * If that fails, the first empty slot will be used.
	 *
	 * Does not check if the item is already in the inventory!
	 *
	 * \param item the item to insert
	 * \param pos position where to insert the item
	 *
	 * \return true if the item was inserted, false otherwise
	 */
	bool insertAt(Entity * item, index_type bag, Vec2f pos, const Pos & fallback = Pos()) {
		
		if(insertGold(item)) {
			return true;
		}
		
		if(item && (item->ioflags & IO_ITEM)) {
			if(Pos newPos = insertAtImpl(item, bag, pos, fallback)) {
				ARX_SOUND_PlayInterface(g_snd.INVSTD);
				ARX_INVENTORY_Declare_InventoryIn(get(newPos), io);
				return true;
			}
		}
		
		return false;
	}
	
	bool insertAtNoEvent(Entity * item, const Pos & pos) {
		
		if(insertGold(item)) {
			return true;
		}
		
		if(!item || !(item->ioflags & IO_ITEM)) {
			return false;
		}
		
		if(insertIntoNewSlotAt(item, pos)) {
			return true;
		}
		
		return insertImpl(item, pos);
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
						remove(slot.io);
					}
					arx_assert(!slot.io && !slot.show);
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
	
	void remove(Entity * item) {
		
		arx_assert(item != NULL && (item->ioflags & IO_ITEM));
		
		InventoryPos pos = locateInInventories(item);
		arx_assert(pos.io == io);
		arx_assert(pos.x + item->m_inventorySize.x <= width);
		arx_assert(pos.y + item->m_inventorySize.y <= height);
		arx_assert(index(pos).show == true);
		arx_assert(item->show == SHOW_FLAG_IN_INVENTORY);
		
		LogDebug(" - " << pos << " remove " << item->idString()
		         << " [" << item->_itemdata->count << '/'
		         << item->_itemdata->playerstacksize << "]: "
		         << int(item->m_inventorySize.x) << 'x' << int(item->m_inventorySize.y));
		
		index(pos).show = false;
		for(index_type j = pos.y; j < (pos.y + size_type(item->m_inventorySize.y)); j++) {
			for(index_type i = pos.x; i < (pos.x + size_type(item->m_inventorySize.x)); i++) {
				arx_assert(index(pos.bag, i, j).io == item);
				arx_assert(index(pos.bag, i, j).show == false);
				index(pos.bag, i, j).io = NULL;
			}
		}
		
		item->_itemdata->m_inventoryPos = InventoryPos();
	}
	
	Entity * get(const Pos & pos) const {
		return pos ? index(pos).io : NULL;
	}
	
};

Inventory<INVENTORY_BAGS, INVENTORY_X, INVENTORY_Y> getPlayerInventory() {
	return Inventory<INVENTORY_BAGS, INVENTORY_X, INVENTORY_Y>(EntityHandle_Player, g_inventory, player.m_bags,
	                                              INVENTORY_X, INVENTORY_Y);
}

Inventory<1, 20, 20> getIoInventory(Entity * io) {
	arx_assert(io != NULL && io->inventory != NULL);
	INVENTORY_DATA * inv = io->inventory;
	return Inventory<1, 20, 20>(io->index(), inv->slot, 1, inv->m_size.x, inv->m_size.y);
}

Inventory<1, 20, 20> getIoInventory(EntityHandle id) {
	return getIoInventory(entities.get(id));
}

} // anonymous namespace

/*!
 * \brief Cleans Player inventory
 */
void CleanInventory() {
	
	for(size_t bag = 0; bag < INVENTORY_BAGS; bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		if(slot.io) {
			getPlayerInventory().remove(slot.io);
		}
		arx_assert(!slot.io && !slot.show);
	}
	
	g_playerInventoryHud.setCurrentBag(0);
}

void optimizePlayerInventory() {
	getPlayerInventory().optimize();
}

bool giveToPlayer(Entity * item) {
	if(getPlayerInventory().insert(item)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

bool giveToPlayer(Entity * item, const InventoryPos & pos) {
	if(getPlayerInventory().insert(item, pos)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

InventoryPos removeFromInventories(Entity * item) {
	
	if(!item || !(item->ioflags & IO_ITEM)) {
		return InventoryPos();
	}
	
	InventoryPos pos = locateInInventories(item);
	if(!pos) {
		return InventoryPos();
	}
	
	if(pos.io == EntityHandle_Player) {
		getPlayerInventory().remove(item);
	} else {
		getIoInventory(pos.io).remove(item);
	}
	
	return pos;
}

InventoryPos locateInInventories(const Entity * item) {
	
	if(!item || !(item->ioflags & IO_ITEM)) {
		return InventoryPos();
	}
	
	return item->_itemdata->m_inventoryPos;
}

bool insertIntoInventory(Entity * item, const InventoryPos & pos) {
	
	if(pos.io == EntityHandle_Player) {
		return getPlayerInventory().insert(item, pos);
	}
	
	if(ValidIONum(pos.io) && entities[pos.io]->inventory) {
		if(getIoInventory(pos.io).insert(item, pos)) {
			return true;
		}
	}
	
	return false;
}

bool insertIntoInventory(Entity * item, Entity * container) {
	
	if(container == entities.player()) {
		return getPlayerInventory().insert(item);
	}
	
	return getIoInventory(container).insert(item);
}

bool insertIntoInventoryAt(Entity * item, Entity * container, InventoryPos::index_type bag, Vec2f pos,
                           const InventoryPos & previous) {
	
	InventoryPos fallback;
	if(previous.io == container->index()) {
		fallback = previous;
	}
	
	if(container == entities.player()) {
		return getPlayerInventory().insertAt(item, bag, pos, fallback);
	}
	
	return getIoInventory(container).insertAt(item, bag, pos, fallback);
}

bool insertIntoInventoryAtNoEvent(Entity * item, const InventoryPos & pos) {
	
	if(pos.io == EntityHandle_Player) {
		return getPlayerInventory().insertAtNoEvent(item, pos);
	}
	
	return getIoInventory(pos.io).insertAtNoEvent(item, pos);
}

/*!
 * \brief Returns IO under position xx,yy in any INVENTORY or NULL if no IO was found
 */
Entity * GetFromInventory(const Vec2s & pos) {
	
	if(g_secondaryInventoryHud.containsPos(pos)) {
		Entity * result = g_secondaryInventoryHud.getObj(pos);
		if(result) {
			return result;
		}
	}
	
	if(g_playerInventoryHud.containsPos(pos)) {
		Entity * result = g_playerInventoryHud.getObj(pos);
		if(result) {
			return result;
		}
	}
	
	return NULL;
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
	
	if(DRAGINTER == io) {
		// Set position to approximate center of player.
		return player.pos + Vec3f(0.f, 80.f, 0.f);
	}

	// Not in scene ?
	if(io->show != SHOW_FLAG_IN_SCENE) {
		
		if(IsEquipedByPlayer(io)) {
			return player.pos + Vec3f(0.f, 80.f, 0.f);
		}
		
		InventoryPos pos = locateInInventories(io);
		if(pos) {
			if(pos.io == EntityHandle_Player) {
				return player.pos + Vec3f(0.f, 80.f, 0.f);
			} else {
				arx_assert(entities.get(pos.io) != NULL);
				return entities.get(pos.io)->pos;
			}
		}
		
	}
	
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
			return ARX_PLAYER_FrontPos();
		}
		
		InventoryPos pos = locateInInventories(io);
		if(pos) {
			if(pos.io == EntityHandle_Player) {
				return ARX_PLAYER_FrontPos();
			} else {
				arx_assert(entities.get(pos.io) != NULL);
				return entities.get(pos.io)->pos;
			}
		}
		
	}
	
	return io->pos;
}

/*!
 * \brief Takes an object from an inventory (be it player's or secondary inventory)
 * at screen position "xx,yy" and Puts that object in player's "hand" (cursor)
 *
 * \return true if an object was taken
 */
bool TakeFromInventory(const Vec2s & pos) {
	
	Entity * item = GetFromInventory(pos);
	if(!item) {
		return false;
	}
	
	arx_assert(item->ioflags & IO_ITEM);
	InventoryPos p = locateInInventories(item);
	if(p.io == EntityHandle_Player) {
		g_playerInventoryHud.dragEntity(item);
	} else {
		arx_assert(SecondaryInventory);
		arx_assert(SecondaryInventory->io->index() == p.io);
		g_secondaryInventoryHud.dragEntity(item);
	}
	
	return true;
}

bool IsInPlayerInventory(Entity * io) {
	return locateInInventories(io).io == EntityHandle_Player;
}

bool IsInSecondaryInventory(Entity * io) {
	return SecondaryInventory && locateInInventories(io).io == SecondaryInventory->io->index();
}

// TODO don't use texture name to find entity
void SendInventoryObjectCommand(const std::string & _lpszText, ScriptMessage _lCommand) {
	
	arx_assert(player.m_bags >= 0);
	arx_assert(player.m_bags <= 3);
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		const INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		
		if(!slot.io || !slot.show || !slot.io->obj)
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

Entity * getInventoryItemWithLowestDurability(const std::string & className, float minDurability) {
	
	Entity * io = NULL;
	
	for(size_t bag = 0; bag < size_t(player.m_bags); bag++)
	for(size_t y = 0; y < INVENTORY_Y; y++)
	for(size_t x = 0; x < INVENTORY_X; x++) {
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		if(slot.io && slot.show && slot.io->className() == className && slot.io->durability >= minDurability) {
			if(!io || slot.io->durability < io->durability) {
				io = slot.io;
			}
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
		INVENTORY_SLOT & slot = g_inventory[bag][x][y];
		if(slot.io && slot.show) {
			ARX_INVENTORY_IdentifyIO(slot.io);
		}
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
			
			if(getPlayerInventory().insert(io)) {
				bSound = true;
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
		
		getIoInventory(TSecondaryInventory->io).remove(io);
		
		InventoryPos pos(TSecondaryInventory->io->index(), 0, 0, 0);
		bool inserted = getIoInventory(TSecondaryInventory->io).insertAtNoEvent(io, pos);
		arx_assert(inserted), ARX_UNUSED(inserted);
	}
}
