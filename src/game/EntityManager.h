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

#ifndef ARX_GAME_ENTITYMANAGER_H
#define ARX_GAME_ENTITYMANAGER_H

#include <stddef.h>
#include <string_view>
#include <vector>
#include <utility>

#include "game/Entity.h"
#include "game/EntityId.h"
#include "game/GameTypes.h"
#include "util/Range.h"

class EntityManager {
	
	typedef std::vector<Entity *> Entries;
	typedef Entries::iterator miterator;
	
	class Sentinel { };
	
	class EntityIterator {
		
		const EntityManager * m_manager;
		size_t m_i = 0;
		
	public:
		
		explicit EntityIterator(const EntityManager * manager)
			: m_manager(manager)
		{
			if(m_i != m_manager->entries.size() && !m_manager->entries[m_i]) {
				operator++();
			}
		}
		
		[[nodiscard]] Entity & operator*() const {
			return *m_manager->entries[m_i];
		}
		
		void operator++() {
			do {
				++m_i;
			} while(m_i != m_manager->entries.size() && !m_manager->entries[m_i]);
		}
		
		[[nodiscard]] bool operator==(Sentinel /* sentinel */) const {
			return m_i == m_manager->entries.size();
		}
		
		[[nodiscard]] bool operator!=(Sentinel /* sentinel */) const {
			return m_i != m_manager->entries.size();
		}
		
	};
	
public:
	
	EntityManager();
	~EntityManager();
	
	//! Reserve at least one entry for the player entity
	void init();
	
	//! Free all entities except for the player
	void clear();
	
	[[nodiscard]] EntityHandle getIndexById(std::string_view idString) const;
	
	[[nodiscard]] Entity * getById(const EntityId & id, Entity * self = nullptr) const;
	[[nodiscard]] Entity * getById(std::string_view idString, Entity * self = nullptr) const;
	
	[[nodiscard]] Entity * operator[](EntityHandle index) const {
		return entries[index.handleData()];
	}
	
	[[nodiscard]] Entity * get(EntityHandle handle) const {
		
		if(handle.handleData() < 0 || handle.handleData() >= long(size())) {
			return nullptr;
		}
		
		return entries[handle.handleData()];
	}
	
	
	//! Get the player entity
	[[nodiscard]] Entity * player() const {
		return entries[0];
	}
	
	/*!
	 * Get the total number of valid indices in this entity manager.
	 *
	 * The maximum valid index for operator[] is size() - 1, however
	 * some of these indices might be unused, so the actual number of
	 * entities in existence may be less.
	 */
	[[nodiscard]] size_t size() const { return entries.size(); }
	
	[[nodiscard]] auto begin() const { return EntityIterator(this); }
	[[nodiscard]] auto end() const { return Sentinel(); }
	
	[[nodiscard]] auto operator()(EntityFlags flags) const {
		return util::filter(*this, [flags](const Entity & entity) {
			return entity.ioflags & flags;
		});
	}
	
	[[nodiscard]] auto inScene(EntityFlags flags = EntityFlags::all()) const {
		return util::filter(*this, [flags](const Entity & entity) {
			return entity.show == SHOW_FLAG_IN_SCENE && (entity.ioflags & flags);
		});
	}
	
	typedef bool (*AutocompleteHandler)(void * context, std::string_view suggestion);
	void autocomplete(std::string_view prefix, AutocompleteHandler handler, void * context);
	
private:
	
	Entries entries;
	
	struct Impl;
	Impl * m_impl;
	
	size_t add(Entity * entity);
	
	void remove(size_t index);
	
	friend class Entity;
};

extern EntityManager entities;

#endif // ARX_GAME_ENTITYMANAGER_H
