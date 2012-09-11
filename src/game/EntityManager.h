/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

struct Entity;

class EntityManager {
	
public:
	
	EntityManager();
	~EntityManager();
	
	//! Reserve at least one entry for the player entity
	void init();
	
	//! Free all entities except for the player
	void clear();
	
	long nbmax;
	
	long getById(const std::string & name);
	Entity * getById(const std::string & name, Entity * self);
	
	Entity * operator[](size_t index) {
		return iobj[index];
	}
	
	//! Get the player entity
	Entity * player() {
		return iobj[0];
	}
	
private:
	
	size_t minfree;
	
	Entity ** iobj;
	
	size_t add(Entity * entity);
	
	void remove(size_t index);
	
	friend struct Entity;
};

extern EntityManager entities;

#endif // ARX_GAME_ENTITYMANAGER_H
