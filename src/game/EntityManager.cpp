/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "game/Entity.h"
#include "platform/Platform.h"

struct EntityManager::Impl {
	
	size_t m_minfree; // first unused index (value == NULL)
	
	typedef boost::unordered_map<std::string, Entity *> Index;
	Index m_index;
	
	Impl() : m_minfree(0) { }
	
	EntityHandle getById(const std::string & idString) const {
		Impl::Index::const_iterator i = m_index.find(idString);
		return (i != m_index.end()) ? i->second->index() : EntityHandle();
	}
	
};

EntityManager entities;

EntityManager::EntityManager() : m_impl(new Impl) { }

EntityManager::~EntityManager() {
	
#ifdef ARX_DEBUG
	for(size_t i = 0; i < size(); i++) {
		arx_assert_msg(entries[i] == NULL, "object %lu not cleared", (unsigned long)i);
	}
#endif
	
	delete m_impl;
	
}

void EntityManager::init() {
	arx_assert(size() == 0);
	entries.resize(1);
	entries[0] = NULL;
	m_impl->m_minfree = 0;
}

void EntityManager::clear() {
	
	// Free all entities, ignoring the player.
	for(size_t i = 1; i < size(); i++) {
		delete entries[i];
		arx_assert(entries[i] == NULL);
	}
	
	entries.resize(1);
	m_impl->m_minfree = 0;
}

EntityHandle EntityManager::getById(const std::string & idString) const {
	
	if(idString.empty() || idString == "none") {
		return EntityHandle();
	}
	if(idString == "self" || idString == "me") {
		return EntityHandle_Self;
	}
	if(idString == "player") {
		return EntityHandle_Player;
	}
	
	return m_impl->getById(idString);
}

EntityHandle EntityManager::getById(const EntityId & id) const {
	
	if(id.isSpecial()) {
		if(id.className().empty()) {
			return EntityHandle();
		}
		if(id.className() == "self" || id.className() == "me") {
			return EntityHandle_Self;
		}
		if(id.className() == "player") {
			return EntityHandle_Player;
		}
	}
	
	return m_impl->getById(id.string());
}

Entity * EntityManager::getById(const std::string & idString, Entity * self) const {
	
	EntityHandle handle = getById(idString);
	if(handle == EntityHandle()) {
		return NULL;
	}
	if(handle == EntityHandle_Self) {
		return self;
	}
	return entries[handle.handleData()];
}

void EntityManager::autocomplete(const std::string & prefix, AutocompleteHandler handler, void * context) {
	
	std::string check = boost::to_lower_copy(prefix);
	
	// TODO we don't need to iterate over all entities if we have per-class indices
	BOOST_FOREACH(Entity * entity, entries) {
		if(entity) {
			std::string id = entity->idString();
			if(boost::starts_with(id, check)) {
				if(!handler(context, id)) {
					return;
				}
			}
		}
	}
	
}

size_t EntityManager::add(Entity * entity) {
	
	m_impl->m_index[entity->idString()] = entity;
	
	for(size_t i = m_impl->m_minfree; i < size(); i++) {
		if(entries[i] == NULL) {
			entries[i] = entity;
			m_impl->m_minfree = i + 1;
			return i;
		}
	}
	
	size_t i = size();
	entries.push_back(entity);
	m_impl->m_minfree = i + 1;
	return i;
}

void EntityManager::remove(size_t index) {
	
	arx_assert_msg(index < size() && entries[index] != NULL,
	               "double free or memory corruption detected: index=%lu", (unsigned long)index);
	
	m_impl->m_index.erase(entries[index]->idString());
	
	if(index < m_impl->m_minfree) {
		m_impl->m_minfree = index;
	}
	
	entries[index] = NULL;
}
