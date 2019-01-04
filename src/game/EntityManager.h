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

#ifndef ARX_GAME_ENTITYMANAGER_H
#define ARX_GAME_ENTITYMANAGER_H

#include <stddef.h>
#include <string>
#include <vector>

#include "game/EntityId.h"
#include "game/GameTypes.h"

class Entity;

class EntityManager {
	
	typedef std::vector<Entity *> Entries;
	typedef Entries::iterator miterator;
	
public:
	
	EntityManager();
	~EntityManager();
	
	//! Reserve at least one entry for the player entity
	void init();
	
	//! Free all entities except for the player
	void clear();
	
	EntityHandle getById(const std::string & idString) const;
	EntityHandle getById(const EntityId & id) const;
	
	Entity * getById(const std::string & idString, Entity * self) const;
	
	Entity * operator[](EntityHandle index) const {
		return entries[index.handleData()];
	}
	
	Entity * get(EntityHandle handle) const {
		
		if(handle.handleData() < 0 || handle.handleData() >= long(size())) {
			return NULL;
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
	
	typedef Entries::const_iterator iterator;
	typedef Entries::const_iterator const_iterator;
	
	iterator begin() const { return entries.begin(); }
	iterator end() const { return entries.end(); }
	
	typedef bool (*AutocompleteHandler)(void * context, const std::string & suggestion);
	void autocomplete(const std::string & prefix, AutocompleteHandler handler, void * context);
	
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
