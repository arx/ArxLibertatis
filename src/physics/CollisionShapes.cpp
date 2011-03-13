/*
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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIECollisionSpheres
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		CollisionSpheres creation/handling
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "physics/CollisionShapes.h"

#include "graphics/Math.h"
#include "graphics/data/MeshManipulation.h"

using std::max;
using std::vector;

void EERIE_COLLISION_Cylinder_Create(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return;

	EERIE_3DOBJ * obj = io->obj;

	if (!obj) return;

	if (obj->vertexlist.empty())
	{
		io->physics.cyl.height = 0.f;
		return;
	}

	io->physics.cyl.origin.x = obj->vertexlist[obj->origin].v.x;
	io->physics.cyl.origin.y = obj->vertexlist[obj->origin].v.y;
	io->physics.cyl.origin.z = obj->vertexlist[obj->origin].v.z;
	float dist = 0.f;
	float height = 0.f;

	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		if ((i != (size_t)obj->origin) && (EEfabs(io->physics.cyl.origin.y - obj->vertexlist[i].v.y) < 20.f))
		{
			dist = max(dist, TRUEDistance3D(io->physics.cyl.origin.x, io->physics.cyl.origin.y, io->physics.cyl.origin.z,
											  obj->vertexlist[i].v.x, obj->vertexlist[i].v.y, obj->vertexlist[i].v.z));
		}

		height = max(height, io->physics.cyl.origin.y - obj->vertexlist[i].v.y);
	}

	if ((dist == 0.f) || (height == 0.f))
	{
		io->physics.cyl.height = 0.f;
		return;
	}

	io->original_radius = dist * 1.2f;
	io->original_height = -height;
	io->physics.cyl.origin.x = io->pos.x;
	io->physics.cyl.origin.y = io->pos.y;
	io->physics.cyl.origin.z = io->pos.z;

	if (io->original_height > -40)
	{
		float v = (-io->original_height) * ( 1.0f / 40 );
		io->original_radius *= (0.5f + v * 0.5f);
	}

	if (io->original_height > -40) io->original_height = -40;

	if (io->original_height < -165) io->original_height = -165;

	if (io->original_radius > 40.f) io->original_radius = 40.f;

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

long GetFirstChildGroup(EERIE_3DOBJ * obj, long group)
{
	if (obj->nbgroups < group + 2) return -1;

	for (long k = group + 1; k < obj->nbgroups; k++)
	{
		for (size_t i = 0; i < obj->grouplist[group].indexes.size(); i++)
		{
			if (obj->grouplist[group].indexes[i] == obj->grouplist[k].origin)
			{
				return k;
			}
		}
	}

	return -1;
}
 
bool IsExclusiveGroupMember(EERIE_3DOBJ * obj, long idx, long group)
{

	for (long i = group + 1; i < obj->nbgroups; i++)
	{
		for (size_t j = 0; j < obj->grouplist[i].indexes.size(); j++)
		{
			if (idx == obj->grouplist[i].indexes[j])
			{
				return false;
			}
		}
	}

	return true;
}

float GetSphereRadiusForGroup(EERIE_3DOBJ * obj, EERIE_3D * center, EERIE_3D * dirvect, long group, float maxi)
{
	float curradius = 0.f;
	float maxf = 0.f;
	float div = 0.f;
	long sel = -1;

	for (size_t i = 0; i < obj->selections.size(); i++)
	{ // TODO iterator
		if (!strcasecmp(obj->selections[i].name.c_str(), "mou"))
		{
			sel = i;
			break;
		}
	}

	for (size_t i = 0; i < obj->grouplist[group].indexes.size(); i++)
	{
		if (!IsExclusiveGroupMember(obj, obj->grouplist[group].indexes[i], group)) continue;

		if ((sel > -1) && (IsInSelection(obj, obj->grouplist[group].indexes[i], sel) >= 0)) continue;

		EERIE_3D target;
		target.x = obj->vertexlist[obj->grouplist[group].indexes[i]].v.x;
		target.y = obj->vertexlist[obj->grouplist[group].indexes[i]].v.y;
		target.z = obj->vertexlist[obj->grouplist[group].indexes[i]].v.z;
		float distance = Distance3D(center->x, center->y, center->z,
									target.x, target.y, target.z);

		if (distance < 2.f) continue;

		if (distance < maxf) continue;

		EERIE_3D targvect;
		targvect.x = target.x - center->x;
		targvect.y = target.y - center->y;
		targvect.z = target.z - center->z;

		float divdist = 1.f / distance;
		targvect.x *= divdist;
		targvect.y *= divdist;
		targvect.z *= divdist;
		float val = Vector_DotProduct(dirvect, &targvect);

		if (EEfabs(val) < 1.2f)
		{
			if (distance > maxi) distance = maxi;

			curradius += distance;
			div += 1.f;
			maxf = max(maxf, distance);
		}
	}

	if (div > 0.f)
	{
		curradius /= div;
	}

	return (curradius);
}

long AddVertexToVertexList(EERIE_3DOBJ * obj, EERIE_3D * center, long group)
{
	if (obj->vertexlist.empty()) return -1;

	for (size_t i = 0; i < obj->vertexlist.size(); i++)
	{
		if ((center->x == obj->vertexlist[i].v.x)
				&&	(center->y == obj->vertexlist[i].v.y)
				&&	(center->z == obj->vertexlist[i].v.z))
		{
			if (IsVertexIdxInGroup(obj, i, group) == false)
			{
				if ((obj->vertexlist[i].norm.x == 50.f)
						&&	(obj->vertexlist[i].norm.y == 50.f)
						&&	(obj->vertexlist[i].norm.z == 50.f)
				   )
					AddVertexIdxToGroup(obj, group, i);
			}

			return i;
		}
	}
	
	size_t nvertex = obj->vertexlist.size();

	// Must create a new one...
	obj->vertexlist.resize(nvertex + 1);

	memset(&obj->vertexlist[nvertex], 0, sizeof(EERIE_VERTEX));

	if (obj->pdata)
	{
		obj->pdata = (PROGRESSIVE_DATA *)
					 realloc(obj->pdata, sizeof(PROGRESSIVE_DATA) * (nvertex + 1));

		memset(&obj->pdata[nvertex], 0, sizeof(PROGRESSIVE_DATA));
		obj->pdata[nvertex].collapse_candidate = -1;
		obj->pdata[nvertex].collapse_cost = 1000000;
	}

	obj->vertexlist[nvertex].v.x = center->x;
	obj->vertexlist[nvertex].v.y = center->y;
	obj->vertexlist[nvertex].v.z = center->z;
	obj->vertexlist[nvertex].norm.x = 50.f;
	obj->vertexlist[nvertex].norm.y = 50.f;
	obj->vertexlist[nvertex].norm.z = 50.f;

	obj->vertexlist3.push_back(obj->vertexlist[nvertex]);
	
	AddVertexIdxToGroup(obj, group, nvertex);

	return nvertex;
}
void EERIE_COLLISION_SPHERES_Create(EERIE_3DOBJ * obj)
{
	if (obj == NULL) return;

	EERIE_COLLISION_SPHERES_Release(obj);
	obj->sdata = new COLLISION_SPHERES_DATA();

	for (long k = 1; k < obj->nbgroups; k++)
	{
		long workon;
		workon = GetFirstChildGroup(obj, k);

		if (workon != -1)
		{
			EERIE_3D center; // Group origin pos
			center.x = obj->vertexlist[obj->grouplist[k].origin].v.x;
			center.y = obj->vertexlist[obj->grouplist[k].origin].v.y;
			center.z = obj->vertexlist[obj->grouplist[k].origin].v.z;
			EERIE_3D dest;  // Destination Group origin pos
			dest.x = obj->vertexlist[obj->grouplist[workon].origin].v.x;
			dest.y = obj->vertexlist[obj->grouplist[workon].origin].v.y;
			dest.z = obj->vertexlist[obj->grouplist[workon].origin].v.z;
			EERIE_3D dirvect; // Direction Vector from Origin Group to Destination Origin Group
			dirvect.x = dest.x - center.x;
			dirvect.y = dest.y - center.y;
			dirvect.z = dest.z - center.z;
			// Distance between those 2 pos
			float dista = Distance3D(dest.x, dest.y, dest.z, center.x, center.y, center.z);
			float tot = dista;
			float divdist = 1.f / dista;
			// Vector Normalization
			dirvect.x *= divdist;
			dirvect.y *= divdist;
			dirvect.z *= divdist;
	
			while (dista >= 0.f) // Iterate along the whole distance
			{
				// Compute required radius for this group and that pos
				float val = GetSphereRadiusForGroup(obj, &center, &dest, k, tot * 1.4f);

				if (val > 0.2f)
				{
					long idx = AddVertexToVertexList(obj, &center, k);

					if (idx > -1)
						AddCollisionSphere(obj, idx, val);

					val *= ( 1.0f / 2 );
				}
				else val = 0.15f;
			
				float inc = val;
				dista -= inc;
			
				center.x += dirvect.x * inc;
				center.y += dirvect.y * inc;
				center.z += dirvect.z * inc;
			}
		}
		else AddCollisionSphere(obj, obj->grouplist[k].origin, obj->grouplist[k].siz * 18.f);
	

	}

	if (obj->sdata->spheres.empty() == 0) EERIE_COLLISION_SPHERES_Release(obj);
}
