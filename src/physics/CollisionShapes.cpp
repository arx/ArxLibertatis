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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "physics/CollisionShapes.h"

#include <cstring>

#include "game/Entity.h"
#include "graphics/Math.h"
#include "graphics/data/MeshManipulation.h"

using std::max;
using std::vector;
using std::memset;

void EERIE_COLLISION_Cylinder_Create(Entity * io)
{
	if(!io)
		return;

	EERIE_3DOBJ * obj = io->obj;

	if(!obj)
		return;

	if(obj->vertexlist.empty()) {
		io->physics.cyl.height = 0.f;
		return;
	}
	
	io->physics.cyl.origin = obj->vertexlist[obj->origin].v;
	
	float d = 0.f;
	float height = 0.f;
	for(size_t i = 0; i < obj->vertexlist.size(); i++) {
		if(i != (size_t)obj->origin && EEfabs(io->physics.cyl.origin.y - obj->vertexlist[i].v.y) < 20.f) {
			d = max(d, glm::distance(io->physics.cyl.origin, obj->vertexlist[i].v));
		}
		height = max(height, io->physics.cyl.origin.y - obj->vertexlist[i].v.y);
	}

	if(d == 0.f || height == 0.f) {
		io->physics.cyl.height = 0.f;
		return;
	}
	
	io->original_radius = d * 1.2f;
	io->original_height = -height;
	io->physics.cyl.origin = io->pos;
	
	if(io->original_height > -40) {
		float v = (-io->original_height) * ( 1.0f / 40 );
		io->original_radius *= (0.5f + v * 0.5f);
	}

	if(io->original_height > -40)
		io->original_height = -40;

	if(io->original_height < -165)
		io->original_height = -165;

	if(io->original_radius > 40.f)
		io->original_radius = 40.f;

	io->physics.cyl.radius = io->original_radius * io->scale;
	io->physics.cyl.height = io->original_height * io->scale;
}

void EERIE_COLLISION_SPHERES_Release(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->sdata) {
		return;
	}
	
	delete obj->sdata;
	obj->sdata = NULL;
}

void AddCollisionSphere(EERIE_3DOBJ * obj, long idx, float radius) {
	
	if(radius < 1.f) {
		return;
	}
	
	for(vector<COLLISION_SPHERE>::iterator i = obj->sdata->spheres.begin(); i < obj->sdata->spheres.end(); ++i) {
		
		if(i->idx == idx) {
			if(radius >= i->radius) {
				i->radius = radius;
			}
			return;
		}
	}
	
	COLLISION_SPHERE newSphere;
	newSphere.idx = (short)idx;
	newSphere.radius = radius;
	newSphere.flags = 0;
	obj->sdata->spheres.push_back(newSphere);
}

long GetFirstChildGroup(EERIE_3DOBJ * obj, long group) {

	if(obj->nbgroups < group + 2)
		return -1;

	for(long k = group + 1; k < obj->nbgroups; k++) {
		for(size_t i = 0; i < obj->grouplist[group].indexes.size(); i++) {
			if(obj->grouplist[group].indexes[i] == obj->grouplist[k].origin) {
				return k;
			}
		}
	}

	return -1;
}
 
bool IsExclusiveGroupMember(EERIE_3DOBJ * obj, long idx, long group) {

	for(long i = group + 1; i < obj->nbgroups; i++) {
		for(size_t j = 0; j < obj->grouplist[i].indexes.size(); j++) {
			if(idx == obj->grouplist[i].indexes[j]) {
				return false;
			}
		}
	}

	return true;
}

float GetSphereRadiusForGroup(EERIE_3DOBJ * obj, Vec3f * center, Vec3f * dirvect, long group, float maxi) {

	float curradius = 0.f;
	float maxf = 0.f;
	float div = 0.f;
	long sel = -1;

	for(size_t i = 0; i < obj->selections.size(); i++) { // TODO iterator
		if(obj->selections[i].name == "mou") {
			sel = i;
			break;
		}
	}

	for(size_t i = 0; i < obj->grouplist[group].indexes.size(); i++) {
		if(!IsExclusiveGroupMember(obj, obj->grouplist[group].indexes[i], group))
			continue;

		if(sel > -1 && IsInSelection(obj, obj->grouplist[group].indexes[i], sel) >= 0)
			continue;

		Vec3f target = obj->vertexlist[obj->grouplist[group].indexes[i]].v;
		float distance = fdist(*center, target);

		if(distance < 2.f)
			continue;

		if(distance < maxf)
			continue;

		Vec3f targvect = (target - *center) * 1.f / distance;
		float val = glm::dot(*dirvect, targvect);

		if(fabs(val) < 1.2f) {
			if(distance > maxi)
				distance = maxi;

			curradius += distance;
			div += 1.f;
			maxf = max(maxf, distance);
		}
	}

	if(div > 0.f) {
		curradius /= div;
	}

	return curradius;
}

long AddVertexToVertexList(EERIE_3DOBJ * obj, Vec3f * center, long group) {

	if(obj->vertexlist.empty())
		return -1;

	for(size_t i = 0; i < obj->vertexlist.size(); i++) {
		if(center->x == obj->vertexlist[i].v.x
		   && center->y == obj->vertexlist[i].v.y
		   && center->z == obj->vertexlist[i].v.z
		) {
			if(IsVertexIdxInGroup(obj, i, group) == false) {
				if(obj->vertexlist[i].norm.x == 50.f
				   && obj->vertexlist[i].norm.y == 50.f
				   && obj->vertexlist[i].norm.z == 50.f
				) {
					AddVertexIdxToGroup(obj, group, i);
				}
			}

			return i;
		}
	}
	
	size_t nvertex = obj->vertexlist.size();

	// Must create a new one...
	obj->vertexlist.resize(nvertex + 1);

	memset(&obj->vertexlist[nvertex], 0, sizeof(EERIE_VERTEX));

	if(obj->pdata) {
		obj->pdata = (PROGRESSIVE_DATA *)
					 realloc(obj->pdata, sizeof(PROGRESSIVE_DATA) * (nvertex + 1));

		memset(&obj->pdata[nvertex], 0, sizeof(PROGRESSIVE_DATA));
		obj->pdata[nvertex].collapse_candidate = -1;
		obj->pdata[nvertex].collapse_cost = 1000000;
	}

	obj->vertexlist[nvertex].v = *center;
	obj->vertexlist[nvertex].norm = Vec3f(50.f);

	obj->vertexlist3.push_back(obj->vertexlist[nvertex]);
	
	AddVertexIdxToGroup(obj, group, nvertex);

	return nvertex;
}

void EERIE_COLLISION_SPHERES_Create(EERIE_3DOBJ * obj) {

	if(obj == NULL)
		return;

	EERIE_COLLISION_SPHERES_Release(obj);
	obj->sdata = new COLLISION_SPHERES_DATA();

	for(long k = 1; k < obj->nbgroups; k++) {
		long workon = GetFirstChildGroup(obj, k);

		if(workon != -1) {
			// Group origin pos
			Vec3f center = obj->vertexlist[obj->grouplist[k].origin].v;
			
			// Destination Group origin pos
			Vec3f dest = obj->vertexlist[obj->grouplist[workon].origin].v;
			
			// Direction Vector from Origin Group to Destination Origin Group
			Vec3f dirvect = dest - center;
			
			// Distance between those 2 pos
			float dista = fdist(dest, center);
			float tot = dista;
			float divdist = 1.f / dista;
			// Vector Normalization
			dirvect *= divdist;
	
			while(dista >= 0.f) { // Iterate along the whole distance
				// Compute required radius for this group and that pos
				float val = GetSphereRadiusForGroup(obj, &center, &dest, k, tot * 1.4f);

				if(val > 0.2f) {
					long idx = AddVertexToVertexList(obj, &center, k);

					if(idx > -1)
						AddCollisionSphere(obj, idx, val);

					val *= ( 1.0f / 2 );
				} else {
					val = 0.15f;
				}
			
				float inc = val;
				dista -= inc;
			
				center += dirvect * inc;
			}
		} else {
			AddCollisionSphere(obj, obj->grouplist[k].origin, obj->grouplist[k].siz * 18.f);
		}
	}

	if(obj->sdata->spheres.empty() == 0)
		EERIE_COLLISION_SPHERES_Release(obj);
}
