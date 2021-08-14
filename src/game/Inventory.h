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

#ifndef ARX_GAME_INVENTORY_H
#define ARX_GAME_INVENTORY_H

#include <stddef.h>
#include <limits>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "game/GameTypes.h"
#include "math/Types.h"
#include "script/Script.h"
#include "util/Range.h"

class Entity;

struct INVENTORY_SLOT {
	
	Entity * io;
	bool show;
	
	INVENTORY_SLOT()
		: io(nullptr)
		, show(false)
	{}
	
};

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
		: io(io_), bag(bag_), x(x_), y(y_)
	{ }
	
	InventoryPos(EntityHandle io_, Vec3s pos)
		: io(io_), bag(pos.z), x(pos.x), y(pos.y)
	{ }
	
	//! \return true if this is a valid position
	operator bool() const {
		return (io != EntityHandle());
	}
	
	operator Vec3s() const {
		return { x, y, bag };
	}
	
};

class INVENTORY_DATA {
	
	Entity & m_owner;
	const Vec2s m_size;
	size_t m_bags;
	std::vector<INVENTORY_SLOT> m_slots;
	
	//! Tile data accessor
	template <bool Mutable>
	class SlotView {
		
		typedef std::conditional_t<Mutable, INVENTORY_DATA, const INVENTORY_DATA> Base;
		
		std::conditional_t<Mutable, INVENTORY_SLOT, const INVENTORY_SLOT> & m_data;
		
	public:
		
		std::conditional_t<Mutable, Entity *, Entity * const> & entity;
		std::conditional_t<Mutable, bool, const bool> & show;
		
		SlotView(Base * inventory, size_t index) noexcept
			: m_data(inventory->m_slots[index])
			, entity(m_data.io)
			, show(m_data.show)
		{ }
		
		[[nodiscard]] operator std::conditional_t<Mutable, INVENTORY_SLOT, const INVENTORY_SLOT> &() const noexcept {
			return m_data;
		}
		
	};
	
	//! Tile data accessor
	template <bool Mutable>
	class GridSlotView : public SlotView<Mutable> {
		
		typedef std::conditional_t<Mutable, INVENTORY_DATA, const INVENTORY_DATA> Base;
		
		const EntityHandle m_owner;
		
	public:
		
		const s16 bag;
		const s16 x;
		const s16 y;
		
		GridSlotView(Base * inventory, Vec3s slot) noexcept
			: SlotView<Mutable>(inventory, inventory->index(slot))
			, m_owner(inventory->owner())
			, bag(slot.z)
			, x(slot.x)
			, y(slot.y)
		{ }
		
		[[nodiscard]] Vec3s slot() const noexcept {
			return { x, y, bag };
		}
		[[nodiscard]] operator Vec3s() const noexcept {
			return slot();
		}
		[[nodiscard]] operator Vec2s() const noexcept {
			return slot();
		}
		[[nodiscard]] explicit operator Vec2f() const noexcept {
			return slot();
		}
		
		[[nodiscard]] operator InventoryPos() const noexcept {
			return { m_owner, slot() };
		}
		
	};
	
	[[nodiscard]] size_t index(Vec3s slot) const noexcept {
		arx_assume(m_size.x > 0 && m_size.y > 0);
		return (size_t(slot.z) * size_t(m_size.x) + size_t(slot.x)) * size_t(m_size.y) + slot.y;
	}
	
	[[nodiscard]] size_t slotCount() const noexcept {
		return m_bags * size_t(m_size.x) * size_t(m_size.y);
	}
	
	//! Returns an iterable over all slots of an item
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto slotsInArea(Vec3s firstSlot, Vec2s size) noexcept {
		const Vec2s start = firstSlot;
		const s16 bag = firstSlot.z;
		arx_assume(m_bags > 0 && bag >= 0 && size_t(bag) < m_bags);
		arx_assume(start.x >= 0 && size.x > 0 && start.y >= 0 && size.y > 0);
		arx_assume(start.x + size.x <= m_size.x && start.y + size.y <= m_size.y);
		return util::transform(util::GridRange<Vec2s, Iterator>(start, start + size), [bag, this](Vec2s slot) {
			arx_assume(slot.x >= 0 && slot.x < m_size.x && slot.y >= 0 && slot.y < m_size.y);
			return GridSlotView<true>(this, Vec3s(slot, bag));
		});
	}
	
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto slotsForItem(Vec3s firstSlot, const Entity & item) noexcept {
		return slotsInArea(firstSlot, getInventorySize(item));
	}
	
	[[nodiscard]] static Vec2s getInventorySize(const Entity & item) noexcept;
	
public:
	
	bool insertGold(Entity * item);
	
	bool insertIntoStackAt(Entity & item, Vec3s pos, bool identify = false);
	
	InventoryPos insertIntoStack(Entity & item);
	
	bool insertIntoNewSlotAt(Entity & item, Vec3s pos);
	
	InventoryPos insertIntoNewSlot(Entity & item);
	
	InventoryPos insertImpl(Entity & item, InventoryPos pos = { });
	
	InventoryPos insertAtImpl(Entity & item, s16 bag, Vec2f pos, InventoryPos fallback);
	
public:
	
	INVENTORY_DATA(Entity * owner, Vec2s size)
		: m_owner(*owner)
		, m_size(size)
		, m_bags(1)
	{
		arx_assume(size.x > 0 && size.y > 0 && m_bags > 0);
		setBags(1);
	}
	
	~INVENTORY_DATA();
	
	void setBags(size_t newBagCount);
	
	[[nodiscard]] EntityHandle owner() const noexcept;
	
	[[nodiscard]] Vec3s size() const noexcept {
		arx_assume(m_size.x > 0 && m_size.y > 0 && m_bags > 0 && m_bags <= std::numeric_limits<s16>::max());
		return { m_size, s16(m_bags) };
	}
	
	[[nodiscard]] s16 width() const noexcept {
		arx_assume(m_size.x > 0);
		return m_size.x;
	}
	
	[[nodiscard]] s16 height() const noexcept {
		arx_assume(m_size.y > 0);
		return m_size.y;
	}
	
	[[nodiscard]] size_t bags() const noexcept {
		arx_assume(m_bags > 0);
		return m_bags;
	}
	
	[[nodiscard]] GridSlotView<true> get(Vec3s slot) noexcept {
		arx_assume(slot.x >= 0 && slot.x < m_size.x && slot.y >= 0 && slot.y < m_size.y
		           && slot.z >= 0 && size_t(slot.z) < m_bags);
		return { this, slot };
	}
	
	[[nodiscard]] GridSlotView<false> get(Vec3s slot) const noexcept {
		arx_assume(slot.x >= 0 && slot.x < m_size.x && slot.y >= 0 && slot.y < m_size.y
		           && slot.z >= 0 && size_t(slot.z) < m_bags);
		return { this, slot };
	}
	
	//! Returns an iterable over all slots in any order
	[[nodiscard]] auto slots() const noexcept {
		return util::transform(util::IntRange(slotCount()), [this](size_t index) {
			return SlotView<false>(this, index);
		});
	}
	
	//! Returns an iterable over all slots in grid order
	template <template <typename T> typename Iterator = util::GridZXYIterator>
	[[nodiscard]] auto slotsInGrid() const noexcept {
		return util::transform(util::GridRange<Vec3s, Iterator>(Vec3s(0), size()), [this](Vec3s slot) {
			return get(slot);
		});
	}
	
	//! Returns an iterable over all slots in grid order with the inner loop over the smaller dimension
	template <template <typename T> typename Iterator = util::GridZXYIterator>
	[[nodiscard]] auto slotsInOrder(Vec2s itemSize = Vec2s(1)) const noexcept {
		arx_assume(itemSize.x > 0 && itemSize.x <= m_size.x && itemSize.y > 0 && itemSize.y <= m_size.y);
		const bool swap = (m_size.y > m_size.x);
		Vec3s max = size() - Vec3s(itemSize - Vec2s(1), 0);
		if(swap) {
			std::swap(max.x, max.y);
		}
		arx_assume(max.z > 0);
		return util::transform(util::GridRange<Vec3s, Iterator>(Vec3s(0), max), [swap, this](Vec3s slot) {
			if(swap) {
				std::swap(slot.x, slot.y);
			}
			return get(slot);
		});
	}
	
	//! Returns an iterable over all slots
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto slotsInBag(size_t bag) const noexcept {
		arx_assume(m_bags > 0 && bag < m_bags && bag <= std::numeric_limits<s16>::max());
		return util::transform(util::GridRange<Vec2s, Iterator>(Vec2s(0), m_size), [bag, this](Vec2s slot) {
			return get(Vec3s(slot, s16(bag)));
		});
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
	bool insert(Entity * item, InventoryPos pos = { });
	
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
	bool insertAt(Entity * item, s16 bag, Vec2f pos, InventoryPos fallback = { });
	
	bool insertAtNoEvent(Entity * item, InventoryPos pos = { });
	
	void remove(Entity & item);
	
};

extern Entity * ioSteal;

inline Vec2s inventorySizeFromTextureSize(Vec2i size) {
	return Vec2s(glm::clamp((size + Vec2i(31, 31)) / Vec2i(32, 32), Vec2i(1, 1), Vec2i(3, 3)));
}

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
bool IsInPlayerInventory(Entity * io);
Entity * getInventoryItemWithLowestDurability(std::string_view className, float minDurability = 0.f);
void useInventoryItemWithLowestDurability(std::string_view className, float minDurability = 0.f);

void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_IdentifyIO(Entity * _pIO);

/*!
 * Tries to move as much of the stack in source to the stack in target.
 * Combining stacks is only possible if the stacks have the same item type.
 *
 * \return true iff the source stack has been completely merged and the entity was destroyed
 */
bool combineItemStacks(Entity * target, Entity * source);

#endif // ARX_GAME_INVENTORY_H
