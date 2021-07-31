/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <vector>
#include <utility>

#include "ai/Paths.h"

#include "core/Application.h"
#include "core/Config.h"

#include "game/EntityManager.h"
#include "game/Item.h"
#include "game/Player.h"

#include "gui/Dragging.h"
#include "gui/Interface.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"

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


INVENTORY_SLOT g_inventory[INVENTORY_BAGS][INVENTORY_X][INVENTORY_Y];
INVENTORY_DATA * SecondaryInventory = nullptr;
Entity * ioSteal = nullptr;

INVENTORY_DATA::~INVENTORY_DATA() {
	
	for(long nj = 0; nj < m_size.y; nj++) {
		for(long ni = 0; ni < m_size.x; ni++) {
			arx_assert(slot[ni][nj].io == nullptr);
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
void PutInFrontOfPlayer(Entity * io) {
	
	if(!io) {
		return;
	}
	
	if((io->ioflags & IO_ITEM) && io->_itemdata->count > 1) {
		while(io->_itemdata->count > 1) {
			Entity * unstackedEntity = CloneIOItem(io);
			unstackedEntity->scriptload = 1;
			unstackedEntity->_itemdata->count = 1;
			unstackedEntity->pos = io->pos;
			unstackedEntity->angle = io->angle;
			PutInFrontOfPlayer(unstackedEntity);
			io->_itemdata->count--;
		}
	}
	
	io->angle = Anglef();
	
	if(g_draggedEntity == io) {
		setDraggedEntity(nullptr);
	}
	
	io->show = SHOW_FLAG_IN_SCENE;
	
	removeFromInventories(io);
	
	Sphere limit(player.pos + Vec3f(0.f, 20.f, 0.f), 80);
	Vec3f dir = angleToVectorXZ(player.angle.getYaw());
	EntityDragResult result = findSpotForDraggedEntity(limit.origin, dir, io, limit);
	
	if(!result.foundSpot) {
		ARX_INTERACTIVE_Teleport(io, player.pos, true);
		return;
	}
	
	ARX_INTERACTIVE_Teleport(io, result.pos - Vec3f(result.offset.x, 0.f, result.offset.z), true);
	
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
struct PlayerInventoryAccess {
	
	typedef InventoryPos::index_type index_type;
	
	const index_type m_bags;
	
	explicit PlayerInventoryAccess(Entity * entity)
		: m_bags(player.m_bags)
	{
		ARX_UNUSED(entity);
	}
	
	[[nodiscard]] INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) {
		return g_inventory[bag][x][y];
	}
	
	[[nodiscard]] const INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) const {
		return g_inventory[bag][x][y];
	}
	
	[[nodiscard]] index_type bags() const {
		arx_assume(m_bags <= INVENTORY_BAGS);
		return m_bags;
	}
	
	[[nodiscard]] index_type width() const {
		return INVENTORY_X;
	}
	
	[[nodiscard]] index_type height() const {
		return INVENTORY_Y;
	}
	
	[[nodiscard]] EntityHandle handle() const {
		return EntityHandle_Player;
	}
	
	[[nodiscard]] bool swap() const {
		return false;
	}
	
	bool insertGold(Entity * item) {
		
		if(item != nullptr && (item->ioflags & IO_GOLD)) {
			ARX_PLAYER_AddGold(item);
			return true;
		}
		
		return false;
	}
	
};

struct EntityInventoryAccess {
	
	typedef InventoryPos::index_type index_type;
	
	INVENTORY_DATA * const m_data;
	const EntityHandle m_entity;
	const Vec2s m_size;
	
	explicit EntityInventoryAccess(Entity * entity)
		: m_data(entity->inventory)
		, m_entity(entity->index())
		, m_size(entity->inventory->m_size)
	{ }
	
	[[nodiscard]] INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) {
		ARX_UNUSED(bag);
		return m_data->slot[x][y];
	}
	
	[[nodiscard]] const INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) const {
		ARX_UNUSED(bag);
		return  m_data->slot[x][y];
	}
	
	[[nodiscard]] index_type bags() const {
		return 1;
	}
	
	[[nodiscard]] index_type width() const {
		arx_assume(m_size.x > 0);
		arx_assume(m_size.x <= 20);
		return index_type(m_size.x);
	}
	
	[[nodiscard]] index_type height() const {
		arx_assume(m_size.y > 0);
		arx_assume(m_size.y <= 20);
		return index_type(m_size.y);
	}
	
	[[nodiscard]] EntityHandle handle() const {
		return m_entity;
	}
	
	[[nodiscard]] bool swap() const {
		return true;
	}
	
	bool insertGold(Entity * item) {
		ARX_UNUSED(item);
		return false;
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

template <typename InventoryAccess>
class Inventory : private InventoryAccess {
	
	typedef InventoryPos Pos;
	
public:
	
	typedef unsigned short size_type;
	typedef InventoryPos::index_type index_type;
	
private:
	
	INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) {
		return InventoryAccess::index(bag, x, y);
	}
	
	const INVENTORY_SLOT & index(index_type bag, index_type x, index_type y) const {
		return InventoryAccess::index(bag, x, y);
	}
	
	INVENTORY_SLOT & index(const InventoryPos & pos) {
		return InventoryAccess::index(pos.bag, pos.x, pos.y);
	}
	
	const INVENTORY_SLOT & index(const InventoryPos & pos) const {
		return InventoryAccess::index(pos.bag, pos.x, pos.y);
	}
	
	index_type bags() {
		return InventoryAccess::bags();
	}
	
	index_type width() {
		return InventoryAccess::width();
	}
	
	index_type height() {
		return InventoryAccess::height();
	}
	
	EntityHandle handle() {
		return InventoryAccess::handle();
	}
	
	bool swap() {
		return InventoryAccess::swap();
	}
	
	bool insertGold(Entity * item) {
		return InventoryAccess::insertGold(item);
	}
	
	Pos insertImpl(Entity * item, const Pos & pos = Pos()) {
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
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
		start = glm::min(start, Vec2s(width() - 1, height() - 1));
		Vec2s end = start + glm::clamp(size, Vec2s(1), Vec2s(width(), height()) - start);
		
		for(Pos p(handle(), bag, start.x, start.y); p.y < end.y; advance(p, start, end)) {
			if(insertIntoStackAt(item, p, true)) {
				return p;
			}
		}
		
		for(Pos p(handle(), bag, start.x, start.y); p.y < end.y; advance(p, start, end)) {
			if(insertIntoNewSlotAt(item, p)) {
				return p;
			}
		}
		
		Vec2f diff(glm::clamp(pos, Vec2f(0.f), Vec2f(width() - 1, height() - 1)) - Vec2f(start));
		Vec2s neighbor(diff.x < 0 ? start.x - 1 : start.x + 1, diff.y < 0 ? start.y - 1 : start.y + 1);
		neighbor = glm::clamp(neighbor, Vec2s(0), Vec2s(width() - 1, height() - 1));
		for(int i = 0; i < 3; i++) {
			bool xfirst = glm::abs(diff.x) > glm::abs(diff.y);
			Pos::index_type x = (xfirst ? (i == 1) : (i == 0)) ? start.x : neighbor.x;
			Pos::index_type y = (xfirst ? (i == 0) : (i == 1)) ? start.y : neighbor.y;
			Pos p(handle(), bag, x, y);
			if(insertIntoStackAt(item, p, true)) {
				return p;
			}
			if(insertIntoNewSlotAt(item, p)) {
				return p;
			}
		}
		
		return insertImpl(item, fallback);
	}
	
	bool insertIntoNewSlotAt(Entity * item, const Pos & pos) {
		
		if(!pos || pos.bag >= bags()) {
			return false;
		}
		
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
		
		arx_assert(pos.io == handle());
		
		if(pos.x + item->m_inventorySize.x > width() || pos.y + item->m_inventorySize.y > height()) {
			return false;
		}
		
		// Check if the whole area required by this item is empty
		for(index_type j = pos.y; j < pos.y + item->m_inventorySize.y; j++) {
			for(index_type i = pos.x; i < pos.x + item->m_inventorySize.x; i++) {
				if(index(pos.bag, i, j).io != nullptr) {
					return false;
				}
			}
		}
		
		if(g_draggedEntity == item) {
			setDraggedEntity(nullptr);
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
		
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
		
		if(size_type(item->m_inventorySize.x) > width() || size_type(item->m_inventorySize.y) > height()) {
			return Pos();
		}
		
		index_type maxi = width() + 1 - size_type(item->m_inventorySize.x);
		index_type maxj = height() + 1 - size_type(item->m_inventorySize.y);
		if(swap()) {
			std::swap(maxi, maxj);
		}
		
		for(index_type bag = 0; bag < bags(); bag++) {
			for(index_type i = 0; i < maxi; i++) {
				for(index_type j = 0; j < maxj; j++) {
					Pos pos = swap() ? Pos(handle(), bag, j, i) : Pos(handle(), bag, i, j);
					
					// Ignore already used inventory slots
					if(index(pos).io != nullptr) {
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
		
		if(!pos || pos.bag >= bags()) {
			return false;
		}
		
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
		
		if(pos.x + item->m_inventorySize.x > width() || pos.y + item->m_inventorySize.y > height()) {
			return false;
		}
		
		Entity * oldItem = index(pos).io;
		if(!oldItem) {
			return false;
		}
		
		if(identify) {
			ARX_INVENTORY_IdentifyIO(oldItem);
		}
		
		if(item == oldItem) {
			return true;
		}
		
		// Don't allow stacking non-interactive items
		// While non-interactive items can't be picked up, they can be made non-interactive while being dragged.
		if(!(item->gameFlags & GFLAG_INTERACTIVITY) || !(oldItem->gameFlags & GFLAG_INTERACTIVITY)) {
			return false;
		}
		
		return combineItemStacks(oldItem, item);
	}
	
	Pos insertIntoStack(Entity * item) {
		
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
		
		if(size_type(item->m_inventorySize.x) > width() || size_type(item->m_inventorySize.y) > height()) {
			return Pos();
		}
		
		index_type maxi = width() + 1 - size_type(item->m_inventorySize.x);
		index_type maxj = height() + 1 - size_type(item->m_inventorySize.y);
		if(swap()) {
			std::swap(maxi, maxj);
		}
		
		// Try to add the items to an existing stack
		for(index_type bag = 0; bag < bags(); bag++) {
			for(index_type i = 0; i < maxi; i++) {
				for(index_type j = 0; j < maxj; j++) {
					Pos pos = swap() ? Pos(handle(), bag, j, i) : Pos(handle(), bag, i, j);
					if(insertIntoStackAt(item, pos)) {
						return pos;
					}
				}
			}
		}
		
		return Pos();
	}
	
public:
	
	explicit Inventory(Entity * entity)
		: InventoryAccess(entity) { }
	
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
		
		if(item && (item->ioflags & IO_ITEM) && !(item->ioflags & IO_MOVABLE)) {
			if(Pos newPos = insertImpl(item, pos)) {
				ARX_INVENTORY_Declare_InventoryIn(get(newPos), handle());
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
		
		if(item && (item->ioflags & IO_ITEM) && !(item->ioflags & IO_MOVABLE)) {
			if(Pos newPos = insertAtImpl(item, bag, pos, fallback)) {
				ARX_SOUND_PlayInterface(g_snd.INVSTD);
				ARX_INVENTORY_Declare_InventoryIn(get(newPos), handle());
				return true;
			}
		}
		
		return false;
	}
	
	bool insertAtNoEvent(Entity * item, const Pos & pos) {
		
		if(insertGold(item)) {
			return true;
		}
		
		if(!item || !(item->ioflags & IO_ITEM) || (item->ioflags & IO_MOVABLE)) {
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
		std::vector<Pos> positions;
		for(size_t bag = 0; bag < bags(); bag++) {
			for(size_t j = 0 ; j < height(); j++) {
				for(size_t i = 0 ; i < width(); i++) {
					INVENTORY_SLOT & slot = index(bag, i, j);
					if(slot.io && slot.show) {
						items.push_back(slot.io);
						positions.push_back(Pos(handle(), bag, i, j));
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
		for(const Entity * item : items) {
			LogDebug(" - " << item->idString() << ": "
			         << int(item->m_inventorySize.x) << 'x' << int(item->m_inventorySize.y));
		}
		#endif
		
		LogDebug("putting back items");
		
		// Now put the items back into the inventory
		std::vector<Entity *> remaining;
		for(Entity * item : items) {
			if(!remaining.empty() || !insertImpl(item)) {
				remaining.push_back(item);
			}
		}
		
		// If we failed to insert any items, put all items back at their original positions
		// Note that some items might have already been merged
		if(!remaining.empty()) {
			LogWarning << "Failed to optimize " << entities.get(handle())->idString() << " inventory";
			
			for(size_t bag = 0; bag < bags(); bag++) {
				for(size_t j = 0 ; j < height(); j++) {
					for(size_t i = 0 ; i < width(); i++) {
						INVENTORY_SLOT & slot = index(bag, i, j);
						if(slot.io && slot.show) {
							remaining.push_back(slot.io);
							remove(slot.io);
						}
						arx_assert(!slot.io && !slot.show);
					}
				}
			}
			
			for(Entity * item : remaining) {
				Pos pos;
				for(size_t i = 0; i < items.size(); i++) {
					if(items[i] == item) {
						pos = positions[i];
						break;
					}
				}
				if(!insertImpl(item, pos)) {
					LogWarning << "Could not restory original position of " << item->idString() << " in "
					           << entities.get(handle())->idString() << " inventory";
					PutInFrontOfPlayer(item);
				}
			}
			
		}
		
	}
	
	void remove(Entity * item) {
		
		arx_assert(item != nullptr && (item->ioflags & IO_ITEM));
		
		InventoryPos pos = locateInInventories(item);
		arx_assert(pos.io == handle());
		arx_assert(pos.x + item->m_inventorySize.x <= width());
		arx_assert(pos.y + item->m_inventorySize.y <= height());
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
				index(pos.bag, i, j).io = nullptr;
			}
		}
		
		item->_itemdata->m_inventoryPos = InventoryPos();
	}
	
	Entity * get(const Pos & pos) const {
		return pos ? index(pos).io : nullptr;
	}
	
};

Inventory<PlayerInventoryAccess> getPlayerInventory() {
	return Inventory<PlayerInventoryAccess>(nullptr);
}

Inventory<EntityInventoryAccess> getEntityInventory(Entity * entity) {
	arx_assert(entity != nullptr && entity->inventory != nullptr);
	return Inventory<EntityInventoryAccess>(entity);
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
	
}

void optimizeInventory(Entity * container) {
	if(container == entities.player()) {
		getPlayerInventory().optimize();
	} else {
		getEntityInventory(container).optimize();
	}
}

bool giveToPlayer(Entity * item) {
	ARX_INVENTORY_IdentifyIO(item);
	if(getPlayerInventory().insert(item)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

bool giveToPlayer(Entity * item, const InventoryPos & pos) {
	ARX_INVENTORY_IdentifyIO(item);
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
		getEntityInventory(entities.get(pos.io)).remove(item);
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
	
	if(Entity * container = entities.get(pos.io); container && container->inventory) {
		if(getEntityInventory(container).insert(item, pos)) {
			return true;
		}
	}
	
	return false;
}

bool insertIntoInventory(Entity * item, Entity * container) {
	
	if(container == entities.player()) {
		return getPlayerInventory().insert(item);
	}
	
	return getEntityInventory(container).insert(item);
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
	
	return getEntityInventory(container).insertAt(item, bag, pos, fallback);
}

bool insertIntoInventoryAtNoEvent(Entity * item, const InventoryPos & pos) {
	
	if(pos.io == EntityHandle_Player) {
		return getPlayerInventory().insertAtNoEvent(item, pos);
	} else if(Entity * container = entities.get(pos.io)) {
		return getEntityInventory(container).insertAtNoEvent(item, pos);
	}
	
	return false;
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
	
	if(g_draggedEntity == io) {
		// Set position to approximate center of player.
		// This is done even if the entity is being dragged in the world in order to avoid triggering
		// any events until the entity is dropped.
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
				arx_assert(entities.get(pos.io) != nullptr);
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
	
	if(g_draggedEntity == io) {
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
				arx_assert(entities.get(pos.io) != nullptr);
				return entities.get(pos.io)->pos;
			}
		}
		
	}
	
	return io->pos;
}

bool IsInPlayerInventory(Entity * io) {
	// Dragged stacks are still considered to be in the player inventory because cooking scripts
	// are not designed to handle them in the scene.
	// Maybe this should also be the case for individual items being dragged but AF 1.21 did not do that.
	return locateInInventories(io).io == EntityHandle_Player
	       || (io && g_draggedEntity == io && (io->ioflags & IO_ITEM) && io->_itemdata->count > 1);
}

Entity * getInventoryItemWithLowestDurability(std::string_view className, float minDurability) {
	
	Entity * io = nullptr;
	
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

void useInventoryItemWithLowestDurability(std::string_view className, float minDurability) {
	
	Entity * item = getInventoryItemWithLowestDurability(className, minDurability);
	if(item && (item->gameFlags & GFLAG_INTERACTIVITY)) {
		SendIOScriptEvent(entities.player(), item, SM_INVENTORYUSE);
	}
	
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

bool combineItemStacks(Entity * target, Entity * source) {
	
	arx_assert(target && (target->ioflags & IO_ITEM) && (target->gameFlags & GFLAG_INTERACTIVITY));
	arx_assert(source && (source->ioflags & IO_ITEM) && (source->gameFlags & GFLAG_INTERACTIVITY));
	arx_assert(source != target);
	
	// Cannot merge into full stacks
	if(target->_itemdata->count >= target->_itemdata->playerstacksize) {
		return false;
	}
	
	// Cannot merge stacks of different items or non-stackeable items
	if(target->_itemdata->playerstacksize <= 1 || !IsSameObject(source, target)) {
		return false;
	}
	
	// Gold stacks use price instead of count
	if((target->ioflags & IO_GOLD) && (source->ioflags & IO_GOLD)) {
		target->_itemdata->price += source->_itemdata->price;
		source->destroy();
		return true;
	}
	
	// Get the number of items to add to the stack
	short int remainingSpace = target->_itemdata->playerstacksize - target->_itemdata->count;
	short int count = std::min(source->_itemdata->count, remainingSpace);
	
	LogDebug(target->idString() << " [" << target->_itemdata->count << '/'
	         << target->_itemdata->playerstacksize << "] += "
	         << source->idString() << " x" << count << '/' << source->_itemdata->count);
	
	target->scale = target->scale * float(target->_itemdata->count) + source->scale * float(count);
	
	target->_itemdata->count += count, source->_itemdata->count -= count;
	
	target->scale /= float(target->_itemdata->count);
	
	if(source->_itemdata->count != 0) {
		// We inserted some of the items into the stack, but there was not enough
		// space for all of them.
		return false;
	}
	
	// Delete the old item
	source->destroy();
	return true;
}
