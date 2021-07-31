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

#include "game/EntityId.h"
#include "game/GameTypes.h"

class Entity;

class EntityManager {
	
	typedef std::vector<Entity *> Entries;
	typedef Entries::iterator miterator;
	
	class Sentinel { };
	
	template <typename Filter>
	class EntityIterator : private Filter {
		
		const EntityManager * m_manager;
		size_t m_i = 0;
		
		bool check(Entity * entity) {
			return entity && (*this)(*entity);
		}
		
	public:
		
		EntityIterator(const EntityManager * manager, Filter filter)
			: Filter(std::move(filter))
			, m_manager(manager)
		{
			if(m_i != m_manager->entries.size() && !check(m_manager->entries[m_i])) {
				operator++();
			}
		}
		
		Entity & operator*() const {
			return *m_manager->entries[m_i];
		}
		
		void operator++() {
			do {
				++m_i;
			} while(m_i != m_manager->entries.size() && !check(m_manager->entries[m_i]));
		}
		
		bool operator!=(Sentinel /* sentinel */) {
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
	
	EntityHandle getById(std::string_view idString) const;
	EntityHandle getById(const EntityId & id) const;
	
	Entity * getById(std::string_view idString, Entity * self) const;
	
	Entity * operator[](EntityHandle index) const {
		return entries[index.handleData()];
	}
	
	Entity * get(EntityHandle handle) const {
		
		if(handle.handleData() < 0 || handle.handleData() >= long(size())) {
			return nullptr;
		}
		
		return entries[handle.handleData()];
	}
	
	
	//! Get the player entity
	Entity * player() const {
		return entries[0];
	}
	
	/*!
	 * Get the total number of valid indices in this entity manager.
	 *
	 * The maximum valid index for operator[] is size() - 1, however
	 * some of these indices might be unused, so the actual number of
	 * entities in existence may be less.
	 */
	size_t size() const { return entries.size(); }
	
	auto begin() const { return EntityIterator( this, [](Entity & /* entity */) { return true; } ); }
	auto end() const { return Sentinel(); }
	
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
