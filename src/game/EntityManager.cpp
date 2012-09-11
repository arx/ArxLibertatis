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

#include "game/EntityManager.h"

#include <cstdlib>
#include <algorithm>

#include "game/Entity.h"
#include "platform/Platform.h"

EntityManager entities;

EntityManager::EntityManager() : minfree(0) { }

EntityManager::~EntityManager() {
	
#ifdef _DEBUG
	for(size_t i = 0; i < size(); i++) {
		arx_assert_msg(entries[i] == NULL,
		               "object " PRINT_SIZE_T " not cleared", i);
	}
#endif // _DEBUG
	
}

void EntityManager::init() {
	arx_assert(size() == 0);
	entries.resize(1);
	entries[0] = NULL;
	minfree = 0;
}

void EntityManager::clear() {
	
	// Free all entities, ignoring the player.
	for(size_t i = 1; i < size(); i++) {
		delete entries[i];
		arx_assert(entries[i] == NULL);
	}
	
	entries.resize(1);
	minfree = 0;
}

long EntityManager::getById(const std::string & name) const {
	
	if(name.empty() || name == "none") {
		return -1;
	} else if(name == "self" || name == "me") {
		return -2;
	} else if(name == "player") {
		return 0; // player is an IO with index 0
	}
	
	for(size_t i = 0 ; i < size() ; i++) {
		if(entries[i] != NULL && entries[i]->ident > -1) {
			// TODO this check is inefficient!
			if(name == entries[i]->long_name()) {
				return i;
			}
		}
	}
	
	return -1;
}

Entity * EntityManager::getById(const std::string & name, Entity * self) const {
	long index = getById(name);
	return (index == -1) ? NULL : (index == -2) ? self : entries[index]; 
}

size_t EntityManager::add(Entity * entity) {
	
	for(size_t i = minfree; i < size(); i++) {
		if(entries[i] == NULL) {
			entries[i] = entity;
			minfree = i + 1;
			return i;
		}
	}
	
	size_t i = size();
	entries.push_back(entity);
	minfree = i + 1;
	return i;
}

void EntityManager::remove(size_t index) {
	
	arx_assert_msg(index < size() && entries[index] != NULL,
	               "double free or memory corruption detected: index="
	               PRINT_SIZE_T, index);
	
	if(index < minfree) {
		minfree = index;
	}
	
	entries[index] = NULL;
}
