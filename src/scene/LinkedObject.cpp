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
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "scene/LinkedObject.h"

#include <cstdlib>
#include <cstring>

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "scene/Interactive.h"
#include "scene/Object.h"


/*!
 * \brief Releases Data for linked objects
 */
void EERIE_LINKEDOBJ_ReleaseData(EERIE_3DOBJ * obj) {
	
	if(!obj) {
		return;
	}
	
	for(EERIE_LINKED & link : obj->linked) {
		if(Entity * slave = link.io) {
			link.io = nullptr;
			slave->updateOwner();
		}
	}
	
	obj->linked.clear();
	
}

static void linkObjects(EERIE_3DOBJ & master, std::string_view masterVertex,
                        EERIE_3DOBJ & slave, std::string_view slaveVertex, Entity * slaveEntity) {
	
	VertexId masterVertexIndex = getNamedVertex(&master, masterVertex);
	if(!masterVertexIndex) {
		return;
	}
	
	VertexGroupId masterVertexGroup = getGroupForVertex(&master, masterVertexIndex);
	if(!masterVertexGroup) {
		return;
	}
	
	VertexId slaveVertexIndex = getNamedVertex(&slave, slaveVertex);
	if(!slaveVertexIndex) {
		return;
	}
	
	EERIE_LINKED link;
	link.lidx = masterVertexIndex;
	link.lgroup = masterVertexGroup;
	link.lidx2 = slaveVertexIndex;
	link.obj = &slave;
	link.io = slaveEntity;
	
	master.linked.push_back(link);
	
	return;
}

#ifdef ARX_DEBUG
static Entity * entityForObject(const EERIE_3DOBJ & object) {
	for(Entity & entity : entities) {
		if(entity.obj == &object) {
			return &entity;
		}
	}
	return nullptr;
}
#endif

void EERIE_LINKEDOBJ_UnLinkObjectFromObject(EERIE_3DOBJ * obj, const EERIE_3DOBJ * tounlink) {
	
	if(!obj || !tounlink) {
		return;
	}
	
	arx_assert(!entityForObject(*tounlink));
	
	for(size_t k = 0; k < obj->linked.size(); k++) {
		if(obj->linked[k].obj == tounlink) {
			obj->linked.erase(obj->linked.begin() + k);
			return;
		}
	}
	
}

void EERIE_LINKEDOBJ_LinkObjectToObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tolink,
                                        std::string_view actiontext, std::string_view actiontext2) {
	
	if(!obj || !tolink) {
		return;
	}
	
	arx_assert(!entityForObject(*tolink));
	
	linkObjects(*obj, actiontext, *tolink, actiontext2, nullptr);
	
}

void linkEntities(Entity & master, std::string_view masterVertex,
                  Entity & slave, std::string_view slaveVertex) {
	
	arx_assert(master.obj && slave.obj);
	
	unlinkEntity(slave);
	
	removeFromInventories(&slave);
	
	linkObjects(*master.obj, masterVertex, *slave.obj, slaveVertex, &slave);
	
	slave.setOwner(&master);
	
}

void unlinkEntity(Entity & slave) {
	
	if(!slave.owner() || !slave.owner()->obj) {
		return;
	}
	
	EERIE_3DOBJ * obj = slave.owner()->obj;
	
	for(size_t k = obj->linked.size(); k > 0; k--) {
		if(obj->linked[k - 1].io == &slave) {
			obj->linked.erase(obj->linked.begin() + (k - 1));
			slave.updateOwner();
			return;
		}
	}
	
}

bool isEntityLinked(Entity & slave) {
	
	if(!slave.owner() || !slave.owner()->obj) {
		return false;
	}
	
	for(const EERIE_LINKED & link : slave.owner()->obj->linked) {
		if(link.io == &slave) {
			return true;
		}
	}
	
	return false;
}

void IO_UnlinkAllLinkedObjects(Entity * io) {
	
	if(!io || !io->obj) {
		return;
	}
	
	while(!io->obj->linked.empty()) {
		
		if(!io->obj->linked.back().io) {
			io->obj->linked.pop_back();
			continue;
		}
		
		Entity * linked = io->obj->linked.back().io;
		
		arx_assert(ValidIOAddress(linked));
		
		linked->setOwner(nullptr);
		
		linked->angle = Anglef(Random::getf(340.f, 380.f), Random::getf(0.f, 360.f), 0.f);
		linked->soundtime = 0;
		linked->soundcount = 0;
		linked->gameFlags |= GFLAG_NO_PHYS_IO_COL;
		linked->show = SHOW_FLAG_IN_SCENE;
		linked->no_collide = io->index();
		
		Vec3f pos = io->obj->vertexWorldPositions[io->obj->linked.back().lidx].v;
		
		Vec3f vector = angleToVectorXZ(linked->angle.getYaw()) * 0.5f;
		
		vector.y = std::sin(glm::radians(linked->angle.getPitch()));
		
		EERIE_PHYSICS_BOX_Launch(linked->obj, pos, linked->angle, vector);
		
	}
	
	io->obj->linked.clear();
	
}
