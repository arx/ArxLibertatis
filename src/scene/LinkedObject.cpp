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
	
	if(!obj)
		return;
	
	obj->linked.clear();
}

void EERIE_LINKEDOBJ_UnLinkObjectFromObject(EERIE_3DOBJ * obj, const EERIE_3DOBJ * tounlink) {
	
	if(!obj || !tounlink) {
		return;
	}
	
	for(size_t k = 0; k < obj->linked.size(); k++) {
		if(obj->linked[k].lgroup && obj->linked[k].obj == tounlink) {
			obj->linked.erase(obj->linked.begin() + k);
			return;
		}
	}
	
}

void EERIE_LINKEDOBJ_LinkObjectToObject(EERIE_3DOBJ * obj, EERIE_3DOBJ * tolink, std::string_view actiontext,
                                        std::string_view actiontext2, Entity * io) {
	
	if(!obj || !tolink) {
		return;
	}
	
	VertexId ni = getNamedVertex(obj, actiontext);
	if(!ni) {
		return;
	}
	
	VertexGroupId group = getGroupForVertex(obj, ni);
	if(!group) {
		return;
	}
	
	VertexId ni2 = getNamedVertex(tolink, actiontext2);
	if(!ni2) {
		return;
	}
	
	EERIE_LINKED link;
	link.lidx2 = ni2;
	link.lidx = ni;
	link.lgroup = group;
	link.obj = tolink;
	link.io = io;
	
	obj->linked.push_back(link);
}

void linkEntities(Entity & master, std::string_view masterVertex,
                  Entity & slave, std::string_view slaveVertex) {
	
	arx_assert(master.obj && slave.obj);
	
	removeFromInventories(&slave);
	slave.show = SHOW_FLAG_LINKED;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(master.obj, slave.obj);
	EERIE_LINKEDOBJ_LinkObjectToObject(master.obj, slave.obj, masterVertex, slaveVertex, &slave);
	
}

void ARX_INTERACTIVE_Detach(EntityHandle n_source, EntityHandle n_target)
{
	Entity * source = entities.get(n_source);
	Entity * target = entities.get(n_target);
	
	if(!source || !target)
		return;

	removeFromInventories(source);
	source->show = SHOW_FLAG_IN_SCENE;
	EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, source->obj);
}

void IO_UnlinkAllLinkedObjects(Entity * io) {
	
	if(!io || !io->obj) {
		return;
	}
	
	for(size_t k = 0; k < io->obj->linked.size(); k++) {
		
		Entity * linked = io->obj->linked[k].io;
		if(!ValidIOAddress(linked)) {
			continue;
		}
		
		linked->angle = Anglef(Random::getf(340.f, 380.f), Random::getf(0.f, 360.f), 0.f);
		linked->soundtime = 0;
		linked->soundcount = 0;
		linked->gameFlags |= GFLAG_NO_PHYS_IO_COL;
		removeFromInventories(io);
		linked->show = SHOW_FLAG_IN_SCENE;
		linked->no_collide = io->index();
		
		Vec3f pos = io->obj->vertexWorldPositions[io->obj->linked[k].lidx].v;
		
		Vec3f vector = angleToVectorXZ(linked->angle.getYaw()) * 0.5f;
		
		vector.y = std::sin(glm::radians(linked->angle.getPitch()));
		
		EERIE_PHYSICS_BOX_Launch(linked->obj, pos, linked->angle, vector);
		
	}
	
	EERIE_LINKEDOBJ_ReleaseData(io->obj);
	
}
