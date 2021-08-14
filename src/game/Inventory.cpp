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


Entity * ioSteal = nullptr;

Inventory::~Inventory() {
	
	#ifdef ARX_DEBUG
	for(auto slot : slots()) {
		arx_assert(slot.entity == nullptr);
		arx_assert(slot.show == false);
	}
	#endif
	
}

void Inventory::setBags(size_t newBagCount) {
	
	#ifdef ARX_DEBUG
	for(size_t bag = newBagCount; bag < bags(); bag++) {
		for(auto slot : slotsInBag(bag)) {
			arx_assert(slot.entity == nullptr);
			arx_assert(slot.show == false);
		}
	}
	#endif
	
	m_bags = newBagCount;
	m_slots.resize(slotCount());
	
}

EntityHandle Inventory::owner() const noexcept {
	return m_owner.index();
}

Vec2s Inventory::getInventorySize(const Entity & item) noexcept {
	return item.m_inventorySize;
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
	
	
	if(g_draggedEntity == io) {
		setDraggedEntity(nullptr);
	}
	
	removeFromInventories(io);
	
	io->angle = Anglef();
	io->show = SHOW_FLAG_IN_SCENE;
	
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

std::ostream & operator<<(std::ostream & strm, InventoryPos p) {
	return strm << '(' << p.io.handleData() << ", " << p.bag << ", " << p.x << ", " << p.y << ')';
}

bool Inventory::insertGold(Entity * item) {
	
	if(&m_owner == entities.player() && item && (item->ioflags & IO_GOLD)) {
		ARX_PLAYER_AddGold(item);
		return true;
	}
	
	return false;
}

bool Inventory::insertIntoStackAt(Entity & item, Vec3s pos, bool identify) {
	
	arx_assume(pos.x >= 0 && pos.y >= 0 && pos.z >= 0);
	if(size_t(pos.z) >= bags()) {
		return false;
	}
	
	arx_assert(item.ioflags & IO_ITEM);
	
	if(pos.x + item.m_inventorySize.x > width() || pos.y + item.m_inventorySize.y > height()) {
		return false;
	}
	
	Entity * oldItem = get(pos).entity;
	if(!oldItem) {
		return false;
	}
	
	if(identify) {
		ARX_INVENTORY_IdentifyIO(oldItem);
	}
	
	if(item == *oldItem) {
		return true;
	}
	
	// Don't allow stacking non-interactive items
	// While non-interactive items can't be picked up, they can be made non-interactive while being dragged.
	if(!(item.gameFlags & GFLAG_INTERACTIVITY) || !(oldItem->gameFlags & GFLAG_INTERACTIVITY)) {
		return false;
	}
	
	return combineItemStacks(oldItem, &item);
}

InventoryPos Inventory::insertIntoStack(Entity & item) {
	
	arx_assert(item.ioflags & IO_ITEM);
	
	if(item.m_inventorySize.x > width() || item.m_inventorySize.y > height()) {
		return { };
	}
	
	for(auto slot : slotsInOrder(item.m_inventorySize)) {
		if(slot.show && insertIntoStackAt(item, slot)) {
			return slot;
		}
	}
	
	return { };
}

bool Inventory::insertIntoNewSlotAt(Entity & item, Vec3s pos) {
	
	arx_assume(pos.x >= 0 && pos.y >= 0 && pos.z >= 0);
	if(size_t(pos.z) >= bags()) {
		return false;
	}
	
	arx_assert(item.ioflags & IO_ITEM);
	
	if(pos.x + item.m_inventorySize.x > width() || pos.y + item.m_inventorySize.y > height()) {
		return false;
	}
	
	// Check if the whole area required by this item is empty
	for(auto slot : slotsForItem(pos, item)) {
		if(slot.entity) {
			return false;
		}
	}
	
	if(g_draggedEntity == &item) {
		setDraggedEntity(nullptr);
	}
	
	removeFromInventories(&item);
	
	LogDebug(" - " << '(' << m_owner.idString() << ", " << pos.z << ", " << pos.x << ", " << pos.y << ')'
	         << " := " << item.idString()
	         << " [" << item._itemdata->count << '/' << item._itemdata->playerstacksize << "]: "
	         << int(item.m_inventorySize.x) << 'x' << int(item.m_inventorySize.y));
	
	// Insert the item at the found position
	for(auto slot : slotsForItem(pos, item)) {
		slot.entity = &item;
		slot.show = false;
	}
	set(pos).show = true;
	item._itemdata->m_inventoryPos = { owner(), pos };
	item.show = SHOW_FLAG_IN_INVENTORY;
	
	return true;
}

InventoryPos Inventory::insertIntoNewSlot(Entity & item) {
	
	arx_assert(item.ioflags & IO_ITEM);
	
	if(item.m_inventorySize.x > width() || item.m_inventorySize.y > height()) {
		return { };
	}
	
	for(auto slot : slotsInOrder(item.m_inventorySize)) {
		if(!slot.entity && insertIntoNewSlotAt(item, slot)) {
			return slot;
		}
	}
	
	return { };
}

InventoryPos Inventory::insertImpl(Entity & item, InventoryPos pos) {
	
	arx_assert(item.ioflags & IO_ITEM);
	
	if(pos.io == owner() && insertIntoStackAt(item, pos)) {
		return pos;
	}
	
	if(InventoryPos newPos = insertIntoStack(item)) {
		return newPos;
	}
	
	if(pos.io == owner() && insertIntoNewSlotAt(item, pos)) {
		return pos;
	}
	
	return insertIntoNewSlot(item);
}

InventoryPos Inventory::insertAtImpl(Entity & item, s16 bag, Vec2f pos, InventoryPos fallback) {
	
	Vec2s start(pos + Vec2f(1.f / 3));
	Vec2s size = item.m_inventorySize;
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
	size = glm::clamp(size, Vec2s(1), Vec2s(width(), height()) - start);
	
	for(auto slot : slotsInArea<util::GridDiagonalIterator>(Vec3s(start, bag), size)) {
		if(insertIntoStackAt(item, slot, true)) {
			return slot;
		}
	}
	
	for(auto slot : slotsInArea<util::GridDiagonalIterator>(Vec3s(start, bag), size)) {
		if(insertIntoNewSlotAt(item, slot)) {
			return slot;
		}
	}
	
	Vec2f diff(glm::clamp(pos, Vec2f(0.f), Vec2f(width() - 1, height() - 1)) - Vec2f(start));
	Vec2s neighbor(diff.x < 0 ? start.x - 1 : start.x + 1, diff.y < 0 ? start.y - 1 : start.y + 1);
	neighbor = glm::clamp(neighbor, Vec2s(0), Vec2s(width() - 1, height() - 1));
	for(int i = 0; i < 3; i++) {
		bool xfirst = glm::abs(diff.x) > glm::abs(diff.y);
		s16 x = (xfirst ? (i == 1) : (i == 0)) ? start.x : neighbor.x;
		s16 y = (xfirst ? (i == 0) : (i == 1)) ? start.y : neighbor.y;
		auto slot = get(Vec3s(x, y, bag));
		if(insertIntoStackAt(item, slot, true)) {
			return slot;
		}
		if(insertIntoNewSlotAt(item, slot)) {
			return slot;
		}
	}
	
	return insertImpl(item, fallback);
}

bool Inventory::insert(Entity * item, InventoryPos pos) {
	
	if(insertGold(item)) {
		return true;
	}
	
	if(item && (item->ioflags & IO_ITEM) && !(item->ioflags & IO_MOVABLE)) {
		if(InventoryPos newPos = insertImpl(*item, pos)) {
			ARX_INVENTORY_Declare_InventoryIn(get(newPos).entity, owner());
			return true;
		}
	}
	
	return false;
}

bool Inventory::insertAt(Entity * item, s16 bag, Vec2f pos, InventoryPos fallback) {
	
	if(insertGold(item)) {
		return true;
	}
	
	if(item && (item->ioflags & IO_ITEM) && !(item->ioflags & IO_MOVABLE)) {
		if(InventoryPos newPos = insertAtImpl(*item, bag, pos, fallback)) {
			ARX_SOUND_PlayInterface(g_snd.INVSTD);
			ARX_INVENTORY_Declare_InventoryIn(get(newPos).entity, owner());
			return true;
		}
	}
	
	return false;
}

bool Inventory::insertAtNoEvent(Entity * item, InventoryPos pos) {
	
	if(insertGold(item)) {
		return true;
	}
	
	if(!item || !(item->ioflags & IO_ITEM) || (item->ioflags & IO_MOVABLE)) {
		return false;
	}
	
	if(insertIntoNewSlotAt(*item, pos)) {
		return true;
	}
	
	return insertImpl(*item, pos);
}

void Inventory::remove(Entity & item) {
	
	arx_assert(item.ioflags & IO_ITEM);
	arx_assert(item._itemdata->m_inventoryPos.io == owner());
	
	Vec3s pos = item._itemdata->m_inventoryPos;
	arx_assert(pos.x + item.m_inventorySize.x <= width());
	arx_assert(pos.y + item.m_inventorySize.y <= height());
	arx_assert(get(pos).show == true);
	arx_assert(item.show == SHOW_FLAG_IN_INVENTORY);
	
	LogDebug(" - " << '(' << m_owner.idString() << ", " << pos.z << ", " << pos.x << ", " << pos.y << ')'
	         << " remove " << item.idString()
	         << " [" << item._itemdata->count << '/' << item._itemdata->playerstacksize << "]: "
	         << int(item.m_inventorySize.x) << 'x' << int(item.m_inventorySize.y));
	
	set(pos).show = false;
	for(auto slot : slotsForItem(pos, item)) {
		arx_assert(slot.entity == &item);
		arx_assert(slot.show == false);
		slot.entity = nullptr;
	}
	
	item._itemdata->m_inventoryPos = { };
}

namespace {

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

} // anonymous namespace

//! Sort the inventory and stack duplicate items
void Inventory::optimize() {
	
	LogDebug("collecting items");
	
	// Collect all inventory items
	std::vector<Entity *> items;
	std::vector<Vec3s> positions;
	for(auto slot : slotsInGrid()) {
		if(slot.entity && slot.show) {
			items.push_back(slot.entity);
			positions.push_back(slot);
			remove(*slot.entity);
		}
		arx_assert(!slot.entity && !slot.show);
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
		if(!remaining.empty() || !insertImpl(*item)) {
			remaining.push_back(item);
		}
	}
	
	// If we failed to insert any items, put all items back at their original positions
	// Note that some items might have already been merged
	if(!remaining.empty()) {
		LogWarning << "Failed to optimize " << m_owner.idString() << " inventory";
		
		for(auto slot : slots()) {
			if(slot.entity && slot.show) {
				remaining.push_back(slot.entity);
				remove(*slot.entity);
			}
			arx_assert(!slot.entity && !slot.show);
		}
		
		for(Entity * item : remaining) {
			Vec3s pos;
			for(size_t i = 0; i < items.size(); i++) {
				if(items[i] == item) {
					pos = positions[i];
					break;
				}
			}
			if(!insertImpl(*item, { owner(), pos })) {
				LogWarning << "Could not restory original position of " << item->idString() << " in "
				           << m_owner.idString() << " inventory";
				PutInFrontOfPlayer(item);
			}
		}
		
	}
	
}

/*!
 * \brief Cleans Player inventory
 */
void CleanInventory() {
	
	if(!entities.player()) {
		return;
	}
	
	for(auto slot : entities.player()->inventory->slots()) {
		if(slot.entity) {
			entities.player()->inventory->remove(*slot.entity);
		}
		arx_assert(!slot.entity && !slot.show);
	}
	
}

void optimizeInventory(Entity * container) {
	
	arx_assert(container && container->inventory);
	
	container->inventory->optimize();
	
}

bool giveToPlayer(Entity * item) {
	ARX_INVENTORY_IdentifyIO(item);
	if(entities.player()->inventory->insert(item)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

bool giveToPlayer(Entity * item, InventoryPos pos) {
	ARX_INVENTORY_IdentifyIO(item);
	if(entities.player()->inventory->insert(item, pos)) {
		return true;
	} else {
		PutInFrontOfPlayer(item);
		return false;
	}
}

InventoryPos removeFromInventories(Entity * item) {
	
	if(!item || !(item->ioflags & IO_ITEM)) {
		return { };
	}
	
	InventoryPos pos = locateInInventories(item);
	if(!pos) {
		return { };
	}
	
	Entity * container = entities.get(pos.io);
	
	arx_assert(container && container->inventory);
	
	container->inventory->remove(*item);
	
	return pos;
}

InventoryPos locateInInventories(const Entity * item) {
	
	if(!item || !(item->ioflags & IO_ITEM)) {
		return { };
	}
	
	return item->_itemdata->m_inventoryPos;
}

bool insertIntoInventory(Entity * item, InventoryPos pos) {
	
	if(Entity * container = entities.get(pos.io); container && container->inventory) {
		if(container->inventory->insert(item, pos)) {
			return true;
		}
	}
	
	return false;
}

bool insertIntoInventoryAt(Entity * item, Entity * container, InventoryPos::index_type bag, Vec2f pos,
                           InventoryPos previous) {
	
	arx_assert(container && container->inventory);
	
	InventoryPos fallback;
	if(previous.io == container->index()) {
		fallback = previous;
	}
	
	return container->inventory->insertAt(item, bag, pos, fallback);
}

bool insertIntoInventoryAtNoEvent(Entity * item, InventoryPos pos) {
	
	if(Entity * container = entities.get(pos.io)) {
		arx_assert(container->inventory);
		return container->inventory->insertAtNoEvent(item, pos);
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
	
	Entity * item = nullptr;
	
	for(auto slot : entities.player()->inventory->slotsInOrder()) {
		if(slot.entity && slot.show && slot.entity->className() == className
		   && slot.entity->durability >= minDurability
		   && (!item || slot.entity->durability < item->durability)) {
			item = slot.entity;
		}
	}
	
	return item;
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
	
	for(auto slot : entities.player()->inventory->slots()) {
		if(slot.entity && slot.show) {
			ARX_INVENTORY_IdentifyIO(slot.entity);
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
